#include "us_hydrodyn_hydration.h"
#include <cstdio>

#include <algorithm>
#include <set>
#include <string>
#include <cmath>

namespace somo_hydration {

Table Table::from_observations(const std::vector<std::pair<std::string, double>>& obs,
                               double confident_at) {
    Table t;
    std::map<std::string, std::map<double, int>> counts;   // hybrid -> waters -> times seen
    for (const auto& o : obs) counts[o.first][o.second]++;

    for (const auto& kv : counts) {
        int total = 0, best_n = 0;
        double best_v = 0;
        for (const auto& c : kv.second) {
            total += c.second;
            // Ties break toward the smaller water count: under-hydrating a proposal the user
            // will edit is safer than over-hydrating one they may accept unchanged.
            if (c.second > best_n || (c.second == best_n && c.first < best_v)) {
                best_n = c.second;
                best_v = c.first;
            }
        }
        Entry e;
        e.waters = best_v;
        e.n_obs = total;
        e.agreement = total ? double(best_n) / total : 0.0;
        e.confident = e.agreement >= confident_at;
        t.by_hybrid_[kv.first] = e;
    }
    return t;
}

const Entry* Table::find(const std::string& hybrid) const {
    auto it = by_hybrid_.find(hybrid);
    return it == by_hybrid_.end() ? nullptr : &it->second;
}

Proposal propose(const Table& t,
                 const std::vector<somo_perceive::InAtom>& atoms,
                 const std::vector<somo_perceive::OutAtom>& perceived,
                 const std::vector<int>& idx) {
    Proposal p;
    if (atoms.size() != perceived.size() || idx.empty()) return p;

    for (int i : idx) {
        const std::string& hyb = perceived[i].hybrid;
        const Entry* e = t.find(hyb);
        if (!e) {
            p.per_atom.push_back(0.0);
            p.review.push_back(atoms[i].name + " (" + hyb +
                               "): type never seen in somo.residue, hydration left at 0");
            continue;
        }
        p.per_atom.push_back(e->waters);
        p.total += e->waters;
        if (!e->confident) {
            char buf[64];
            std::snprintf(buf, sizeof(buf), "%.0f%% of %d", 100 * e->agreement, e->n_obs);
            p.review.push_back(atoms[i].name + " (" + hyb + "): proposed " +
                               std::to_string((long) std::lround(e->waters)) +
                               " but the coded residues agree only " + buf + " -- please check");
        }
    }
    p.ok = true;
    return p;
}


// ---- pH 7 chemistry rules ---------------------------------------------------------------
namespace {

using somo_perceive::Bonds;
using somo_perceive::InAtom;
using somo_perceive::OutAtom;

bool elem_is(const OutAtom& o, const char* e) { return o.element == e; }

// A carbon bearing both a doubly-bonded O and a second O: a carboxyl/carboxylate carbon.
// `other_o` receives the partner oxygen so the caller can tell the two apart.
bool carboxyl_carbon(int c, const std::vector<OutAtom>& p, const Bonds& b) {
    int n_o = 0, n_carbonyl = 0;
    for (int n : b.nb[c]) {
        if (!elem_is(p[n], "O")) continue;
        ++n_o;
        if (p[n].n_h == 0 && b.nb[n].size() == 1) ++n_carbonyl;
    }
    return elem_is(p[c], "C") && n_o == 2 && n_carbonyl >= 1;
}

// Number of sp3 carbons between a carboxyl carbon and whatever the group hangs off, walking
// away from the carboxyl until the chain stops being a plain aliphatic carbon. Asp gives 1 and
// Glu 2, which is what separates their hydration (5 versus 6 waters).
int chain_len_from(int carboxyl_c, int from, const std::vector<OutAtom>& p, const Bonds& b) {
    int n = 0, cur = from, prev = carboxyl_c;
    while (n < 8) {
        if (!elem_is(p[cur], "C") || p[cur].heavy_neighbors > 3) break;
        bool aliphatic = true;
        for (int x : b.nb[cur]) if (!elem_is(p[x], "C") && x != prev) aliphatic = false;
        if (!aliphatic) break;              // reached a backbone or a heteroatom-bearing carbon
        ++n;
        int next = -1;
        for (int x : b.nb[cur]) if (x != prev && elem_is(p[x], "C")) { next = x; break; }
        if (next < 0) break;
        prev = cur;
        cur = next;
    }
    return n;
}

bool in_ring(int a, const std::vector<int>& r) { return std::find(r.begin(), r.end(), a) != r.end(); }

bool in_any_ring(int a, const Bonds& b) {
    for (const auto& r : b.rings) if (in_ring(a, r)) return true;
    return false;
}

// Arg's CZ. Guanine's C2 also carries three nitrogens, but it is a ring atom, and its N1 and N3
// follow the ring nitrogen rules instead.
bool guanidinium_carbon(int c, const std::vector<OutAtom>& p, const Bonds& b) {
    if (!elem_is(p[c], "C") || in_any_ring(c, b)) return false;
    int nn = 0;
    for (int x : b.nb[c]) if (elem_is(p[x], "N")) ++nn;
    return nn >= 3;
}

bool in_ring_of_size(int a, size_t n, const Bonds& b) {
    for (const auto& r : b.rings) if (r.size() == n && in_ring(a, r)) return true;
    return false;
}

// A carbon of a six-membered ring that also holds a nitrogen -- a nucleobase's or flavin's ring,
// as against the five-membered lactam of pyroglutamate or imide of succinimide (both stored 0).
bool in_six_ring_with_n(int c, const std::vector<OutAtom>& p, const Bonds& b) {
    for (const auto& r : b.rings) {
        if (r.size() != 6 || !in_ring(c, r)) continue;
        for (int x : r) if (elem_is(p[x], "N")) return true;
    }
    return false;
}

// A ring carbon carrying a terminal, hydrogen-free oxygen.
bool ring_carbonyl_carbon(int c, const std::vector<OutAtom>& p, const Bonds& b) {
    if (!elem_is(p[c], "C") || !in_any_ring(c, b)) return false;
    for (int x : b.nb[c])
        if (elem_is(p[x], "O") && p[x].n_h == 0 && b.nb[x].size() == 1) return true;
    return false;
}

bool sp3_carbon(int c, const std::vector<OutAtom>& p) {
    return elem_is(p[c], "C") && p[c].heavy_neighbors + p[c].n_h == 4;
}

// The N1 of a purine: a nitrogen of the six-membered ring of a 6-5 fused bicycle that touches
// neither fusion atom. N3, beside the fusion carbon C4, is the one that is hydrated.
bool purine_n1(int n, const Bonds& b) {
    for (const auto& r6 : b.rings) {
        if (r6.size() != 6 || !in_ring(n, r6)) continue;
        for (const auto& r5 : b.rings) {
            if (r5.size() != 5) continue;
            std::vector<int> fusion;
            for (int x : r6) if (in_ring(x, r5)) fusion.push_back(x);
            if (fusion.size() != 2) continue;
            bool touches = in_ring(n, fusion);
            for (int x : b.nb[n]) if (in_ring(x, fusion)) touches = true;
            if (!touches) return true;
        }
    }
    return false;
}

// ---- phosphate oxygens ----------------------------------------------------------------------
// Every oxygen on a phosphorus is valued from the phosphate's topology. somo.residue's
// nucleotide entries (2026-10, from MD) give the P=O oxygen one water and the charged ones four
// or five, but to a geometric perceiver those are the same atom, and atom names do not settle it
// either: the CCD mostly calls the P=O O1, yet its own LNC entry puts it on O2A/O2N, and an FAD
// file in use carries the charges on O1P and O1A. So the first free oxygen by name takes the P=O
// value and the others the charged ones; in a bead model only the sum matters (Mattia,
// 2026-10-26). ATP's own alpha (1, 4) and gamma (1, 3, 3) come out lower than ADP's for the same
// topologies; the ADP values are used.
struct PTemplate {
    int n_free, n_bridge, n_ester;
    double free[3];                       // by name: the P=O first, then the charged oxygens
    double ester;                         // each P-O-C oxygen; a P-O-P bridge is always 2
    const char* what;
};
const PTemplate P_TEMPLATES[] = {
    {2, 0, 2, {1, 4, 0}, 1, "phosphodiester, as in the nucleic-acid backbone"},
    {2, 1, 1, {1, 5, 0}, 0, "pyrophosphate half, as in ADP alpha and NAD"},
    {2, 2, 0, {1, 4, 0}, 0, "middle phosphate of a triphosphate, as in ATP beta"},
    {3, 0, 1, {1, 5, 5}, 0, "terminal phosphate monoester, as in AMP"},
    {3, 1, 0, {1, 4, 5}, 0, "terminal phosphate of an anhydride, as in ADP beta"},
};

struct PhosphateO {
    double waters;
    const char* why;
};

// Values for the oxygens of every phosphorus in the residue, and of a neighbouring residue's
// phosphorus where one of this residue's oxygens is its ester (a nucleotide's O3'). One review
// line per phosphorus of the residue itself.
std::map<int, PhosphateO> phosphate_oxygens(const std::vector<InAtom>& atoms,
                                            const std::vector<OutAtom>& p, const Bonds& b,
                                            const std::vector<int>& idx,
                                            std::vector<std::string>& review) {
    std::map<int, PhosphateO> out;
    std::set<int> mine(idx.begin(), idx.end()), phos;
    for (int i : idx) {
        if (elem_is(p[i], "P")) phos.insert(i);
        if (elem_is(p[i], "O")) for (int n : b.nb[i]) if (elem_is(p[n], "P")) phos.insert(n);
    }
    for (int ph : phos) {
        std::vector<int> fr, br, es;
        int other = 0;
        for (int o : b.nb[ph]) {
            // Across a tight P-O-P bridge the two phosphorus atoms can come within the P-P bond
            // cutoff (2.59 A; CCD ideal geometry puts them at 2.585). That contact is never a
            // bond in these ligands, so it must not knock the phosphate off its template.
            if (elem_is(p[o], "P")) continue;
            if (!elem_is(p[o], "O")) { ++other; continue; }
            int np = 0, nx = 0;
            for (int x : b.nb[o]) {
                if (x == ph) continue;
                if (elem_is(p[x], "P")) ++np;
                else ++nx;
            }
            if (np == 0 && nx == 0)      fr.push_back(o);
            else if (np == 1 && nx == 0) br.push_back(o);
            else if (np == 0 && nx == 1) es.push_back(o);
            else ++other;
        }
        const PTemplate* t = 0;
        for (const PTemplate& c : P_TEMPLATES)
            if (!other && c.n_free == (int) fr.size() && c.n_bridge == (int) br.size()
                && c.n_ester == (int) es.size())
                t = &c;
        if (!t) {
            if (mine.count(ph))
                review.push_back(atoms[ph].name + ": a phosphorus with " + std::to_string(fr.size())
                                 + " free, " + std::to_string(br.size()) + " bridging and "
                                 + std::to_string(es.size()) + " ester oxygens matches no phosphate "
                                 "template -- its oxygens fall to the generic rules");
            continue;
        }
        std::sort(fr.begin(), fr.end(),
                  [&atoms](int x, int y) { return atoms[x].name < atoms[y].name; });
        std::string line = atoms[ph].name + ": " + t->what + " --";
        for (size_t k = 0; k < fr.size(); ++k) {
            out[fr[k]] = {t->free[k], k ? "charged phosphate oxygen" : "phosphate P=O oxygen"};
            line += " " + atoms[fr[k]].name + " " + std::to_string(std::lround(t->free[k]));
        }
        for (int o : br) {
            out[o] = {2.0, "P-O-P bridging oxygen"};
            line += ", bridge " + atoms[o].name + " 2";
        }
        for (int o : es) {
            out[o] = {t->ester, "phosphate ester oxygen"};
            line += ", ester " + atoms[o].name + " " + std::to_string(std::lround(t->ester));
        }
        if (mine.count(ph))
            review.push_back(line + " (the first free oxygen by name is taken as the P=O, "
                             "which a PDB file does not record)");
    }
    return out;
}

} // namespace

Proposal propose_by_rules(const std::vector<InAtom>& atoms,
                          const std::vector<OutAtom>& perceived,
                          const Bonds& bonds,
                          const std::vector<int>& idx,
                          const Table* fallback) {
    Proposal p;
    if (atoms.size() != perceived.size() || bonds.nb.size() != atoms.size() || idx.empty())
        return p;

    const std::map<int, PhosphateO> phos = phosphate_oxygens(atoms, perceived, bonds, idx, p.review);

    for (int i : idx) {
        const OutAtom& o = perceived[i];
        double w = -1.0;                    // -1 = no rule matched
        const char* why = 0;
        Alternate alt;                      // stays empty unless a rule finds an ionizable group
        const auto ph = phos.find(i);

        if (ph != phos.end()) {
            // already reported, one line per phosphorus
            w = ph->second.waters;
            why = ph->second.why;
        } else if (elem_is(o, "O")) {
            // find a carboxyl carbon this oxygen belongs to
            int cc = -1;
            for (int n : bonds.nb[i]) if (carboxyl_carbon(n, perceived, bonds)) cc = n;
            if (cc >= 0) {
                // At pH 7 the group is -COO-. The oxygen that carries the proton in the neutral
                // form is the one that becomes the charged, hydrated oxygen; its partner is the
                // carbonyl and takes none.
                if (o.n_h >= 1 || o.formal_charge < 0) {
                    int chain = 0;
                    for (int n : bonds.nb[cc])
                        if (elem_is(perceived[n], "C"))
                            chain = std::max(chain, chain_len_from(cc, n, perceived, bonds));
                    // The waters belong to the DEPROTONATED alternate, not to this line. ASP
                    // writes OD2 as "O2H1 ... 0 | O1H0- ... 5": the neutral -COOH carries none
                    // and the carboxylate carries five. Emitting 5 against an O2H1 primary --
                    // which is what this did -- produces a row that exists nowhere in the table
                    // and double-states the ionisation (Mattia, 2026-08-10, on citrate).
                    w = 0.0;
                    why = "carboxyl -OH";
                    alt.hybrid = "O1H0-";
                    alt.waters = chain >= 2 ? 6.0 : 5.0;
                    alt.pKa    = chain >= 2 ? 4.25 : 3.67;   // GLU / ASP convention
                    alt.why    = "carboxylate";
                } else {
                    w = 0.0;
                    why = "carbonyl oxygen of a carboxylate";
                }
            } else if (o.n_h >= 1) {
                bool phenolic = false;
                for (int n : bonds.nb[i]) if (bonds.aromatic[n]) phenolic = true;
                w = phenolic ? 0.0 : 1.0;
                why = phenolic ? "phenolic hydroxyl" : "aliphatic hydroxyl";
            } else if (o.formal_charge == 0) {
                // No hydrogen, no charge, not part of a carboxyl or a phosphate: a carbonyl,
                // ester or ether oxygen. The backbone carbonyl, the amide and acetyl carbonyls
                // and the pyranose ring O5 all carry 0 in somo.residue, so 0 is the confident
                // default -- without it the backbone carbonyl fell through to "no pH 7 rule for
                // this group" and was flagged on essentially every entry. The nucleotide entries
                // (2026-10) hydrate two kinds: the ring carbonyls of the bases (O2/O4/O6), and
                // the furanose ring O4'. Flavin's ring carbonyls follow the bases (Mattia,
                // 2026-10-26); five-membered lactams and imides (pyroglutamate, succinimide)
                // stay at 0, as stored.
                const int c = bonds.nb[i].size() == 1 ? bonds.nb[i][0] : -1;
                if (c >= 0 && elem_is(perceived[c], "C") && in_six_ring_with_n(c, perceived, bonds)) {
                    w = 1.0;
                    why = "ring carbonyl of a nucleobase-like ring";
                } else if (bonds.nb[i].size() == 2 && in_ring_of_size(i, 5, bonds)) {
                    w = 1.0;
                    why = "five-membered ring ether oxygen (furanose O4')";
                } else {
                    w = 0.0;
                    why = "carbonyl / ether oxygen";
                }
            }
        } else if (elem_is(o, "N")) {
            bool guanid = false;
            for (int n : bonds.nb[i]) if (guanidinium_carbon(n, perceived, bonds)) guanid = true;
            bool sp3_sub = false, beside_carbonyl = false;
            for (int n : bonds.nb[i]) {
                if (sp3_carbon(n, perceived)) sp3_sub = true;
                if (ring_carbonyl_carbon(n, perceived, bonds)) beside_carbonyl = true;
            }
            if (guanid) {
                // only the terminal nitrogens of the group are hydrated
                w = bonds.nb[i].size() == 1 ? 1.0 : 0.0;
                why = "guanidinium nitrogen";
            } else if (bonds.aromatic[i] && sp3_sub) {
                // The nucleotide entries (2026-10) leave the glycosidic N9 / pyrimidine N1 dry,
                // and so flavin's N10 on its ribityl (Mattia, 2026-10-26). NAD's nicotinamide
                // N1N, on its ribose, was already stored 0.
                w = 0.0;
                why = "ring nitrogen carrying a glycosidic or other sp3 substituent";
            } else if (bonds.aromatic[i] && beside_carbonyl) {
                // pyrimidine N3 and guanine N1 in the nucleotide entries, flavin N1/N3 by analogy
                w = 0.0;
                why = "ring nitrogen beside a ring carbonyl";
            } else if (bonds.aromatic[i] && purine_n1(i, bonds)) {
                w = 0.0;
                why = "purine N1";
            } else if (bonds.aromatic[i]) {
                // purine N3 and N7, His, Trp, the heme pyrroles
                w = o.formal_charge > 0 ? 2.0 : 1.0;
                why = "aromatic ring nitrogen";
            } else if (o.n_h >= 3 || (o.formal_charge > 0 && o.n_h >= 2)) {
                w = 3.0;
                why = "terminal ammonium at pH 7";
            } else {
                // Amide nitrogen, tested by its carbonyl neighbour rather than by H count: a
                // tertiary ring amide such as proline's carries no hydrogen and is still
                // hydrated, so an "n_h >= 1" test would miss it.
                bool amide = false;
                for (int n : bonds.nb[i])
                    if (elem_is(perceived[n], "C") && carboxyl_carbon(n, perceived, bonds) == false)
                        for (int m : bonds.nb[n])
                            if (elem_is(perceived[m], "O") && perceived[m].n_h == 0 &&
                                bonds.nb[m].size() == 1)
                                amide = true;
                if (amide) {
                    w = 1.0;
                    why = "amide nitrogen";
                } else if (o.n_h >= 1) {
                    w = 1.0;
                    why = "amine nitrogen";
                }
            }
        } else if (elem_is(o, "S")) {
            // A thioether (Met SD, C-S-C) is hydrated; a thiol or a disulfide is not. Counting
            // neighbours alone does not separate them, since a disulfide sulfur also has two --
            // the partner has to be carbon on both sides.
            bool thioether = o.n_h == 0 && bonds.nb[i].size() == 2;
            for (int n : bonds.nb[i]) if (!elem_is(perceived[n], "C")) thioether = false;
            w = thioether ? 1.0 : 0.0;
            why = thioether ? "thioether" : "thiol / disulfide";
        } else if (elem_is(o, "C")) {
            w = 0.0;
            why = "carbon";
        } else if (elem_is(o, "P")) {
            w = 0.0;
            why = "phosphorus";
        }

        if (w < 0) {
            // No rule. Fall back to what the coded residues do on average, and say so.
            const Entry* e = fallback ? fallback->find(o.hybrid) : 0;
            w = e ? e->waters : 0.0;
            p.review.push_back(atoms[i].name + " (" + o.hybrid + "): no pH 7 rule for this group"
                               + (e ? ", using the coded-residue average" : ", left at 0"));
        } else if (!alt.hybrid.empty()) {
            // An ionizable group: say what the alternate is and what pKa is being proposed, since
            // the pKa is a convention value the user is expected to check.
            char buf[48];
            std::snprintf(buf, sizeof(buf), "%.2f", alt.pKa);
            p.review.push_back(atoms[i].name + " (" + o.hybrid + "): ionizable -> " + alt.hybrid
                               + ", " + std::to_string((long) std::lround(alt.waters))
                               + " waters, proposed pKa " + buf + " (convention from the coded residues -- check it)");
        } else if (why && (w >= 5.0) && ph == phos.end()) {
            // Large values dominate a residue's total, so make them visible even when confident.
            // Phosphate oxygens are already listed, one line per phosphorus.
            p.review.push_back(atoms[i].name + " (" + o.hybrid + "): " + why + ", "
                               + std::to_string((long) std::lround(w)) + " waters");
        }
        p.per_atom.push_back(w);
        p.alternate.push_back(alt);
        p.total += w;
        p.total_ionized += alt.hybrid.empty() ? w : alt.waters;
    }
    p.ok = true;
    return p;
}

} // namespace somo_hydration
