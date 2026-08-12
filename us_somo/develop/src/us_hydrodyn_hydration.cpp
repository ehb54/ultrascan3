#include "us_hydrodyn_hydration.h"
#include <cstdio>

#include <algorithm>
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

bool guanidinium_carbon(int c, const std::vector<OutAtom>& p, const Bonds& b) {
    if (!elem_is(p[c], "C")) return false;
    int nn = 0;
    for (int x : b.nb[c]) if (elem_is(p[x], "N")) ++nn;
    return nn >= 3;
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

    for (int i : idx) {
        const OutAtom& o = perceived[i];
        double w = -1.0;                    // -1 = no rule matched
        const char* why = 0;
        Alternate alt;                      // stays empty unless a rule finds an ionizable group

        if (elem_is(o, "O")) {
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
                // No hydrogen, no charge, not part of a carboxyl: a carbonyl, ester, ether or
                // phosphate oxygen. EVERY O1H0 in somo.residue carries 0 waters -- 36 backbone
                // O, plus O2/O4/O6/OP1/OP2 and the rest -- so this is a confident zero, not a
                // fallback. Without it the backbone carbonyl fell through to "no pH 7 rule for
                // this group" and was flagged for review on essentially every entry.
                w = 0.0;
                why = "carbonyl / ether oxygen";
            }
        } else if (elem_is(o, "N")) {
            bool guanid = false;
            for (int n : bonds.nb[i]) if (guanidinium_carbon(n, perceived, bonds)) guanid = true;
            if (guanid) {
                // only the terminal nitrogens of the group are hydrated
                w = bonds.nb[i].size() == 1 ? 1.0 : 0.0;
                why = "guanidinium nitrogen";
            } else if (bonds.aromatic[i]) {
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
        } else if (why && (w >= 5.0)) {
            // Large values dominate a residue's total, so make them visible even when confident.
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
