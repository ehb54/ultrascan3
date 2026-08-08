#include "us_hydrodyn_psv.h"

#include <algorithm>
#include <map>
#include <set>

namespace somo_psv {

using somo_perceive::Bonds;
using somo_perceive::InAtom;
using somo_perceive::OutAtom;

namespace inc {
double ring_decrement(int n) {
    switch (n) {
        case 3:  return 2.1;
        case 4:  return 4.1;
        case 5:  return 6.1;
        case 6:  return 8.1;
        case 7:  return 10.1;
        case 8:  return 12.1;
        default: return n >= 9 ? 14.1 : 0.0;
    }
}
} // namespace inc

namespace {

bool is_elem(const OutAtom& o, const char* e) { return o.element == e; }

// Does carbon `c` carry a double-bonded oxygen (i.e. is it a carbonyl carbon)?
// `except` excludes one oxygen from the search, so that asking "is my own carbon a carbonyl
// carbon" cannot be answered by pointing back at the querying atom.
bool carbon_has_carbonyl_O(int c, const std::vector<OutAtom>& p, const Bonds& b, int except = -1) {
    for (int n : b.nb[c]) {
        if (n == except) continue;
        if (is_elem(p[n], "O") && p[n].n_h == 0 && (int) b.nb[n].size() == 1) return true;
    }
    return false;
}

// A carboxyl/phosphate/sulfate/sulfonate oxygen: attached to a C that ALSO bears a carbonyl O,
// or to P or S. Its OH (or O-) takes 0.4 rather than the 2.3 of an ordinary first hydroxyl.
// Only ever asked of an oxygen bearing H or a negative charge -- a neutral =O is a carbonyl and
// takes 5.5 regardless of what else hangs off its carbon.
bool is_acid_oxygen(int o, const std::vector<OutAtom>& p, const Bonds& b) {
    for (int n : b.nb[o]) {
        if (is_elem(p[n], "P") || is_elem(p[n], "S")) return true;
        if (is_elem(p[n], "C") && carbon_has_carbonyl_O(n, p, b, o)) return true;
    }
    return false;
}

// Guanidinium: N bonded to a carbon carrying three nitrogens. Table 1's footnote is explicit
// that the 8.0 increment "refers to each N in a guanidinium group; in Arg only to the two
// terminal N" -- so the internal one (arginine's NE, which also bonds CD) is excluded here and
// falls through to the amine value.
bool is_guanidinium_N(int n, const std::vector<OutAtom>& p, const Bonds& b) {
    if (b.nb[n].size() != 1) return false;                  // terminal nitrogens only
    for (int c : b.nb[n]) {
        if (!is_elem(p[c], "C")) continue;
        int nn = 0;
        for (int x : b.nb[c]) if (is_elem(p[x], "N")) ++nn;
        if (nn >= 3) return true;
    }
    return false;
}

bool bonded_to_carbonyl_C(int n, const std::vector<OutAtom>& p, const Bonds& b) {
    for (int c : b.nb[n])
        if (is_elem(p[c], "C") && carbon_has_carbonyl_O(c, p, b)) return true;
    return false;
}

} // namespace

Result compute(const std::vector<InAtom>& atoms,
               const std::vector<OutAtom>& perceived,
               const Bonds& bonds,
               const std::vector<int>& idx,
               const Options& opt) {
    Result r;
    if (atoms.size() != perceived.size() || bonds.nb.size() != atoms.size() || idx.empty())
        return r;

    std::set<int> in_res(idx.begin(), idx.end());

    // ---- 1. hydroxyl oxygens, and which of them are NEIGHBOURING ---------------------------
    // D&Z charge 2.3 for a "1st OH" but only 0.4 for a "2nd or further NEIGHBOURING OH". That
    // is topological, not a running count: verified against their own Table 5, where
    // 1,2-ethanediol (adjacent OH) is 2.3 + 0.4 = 53.5 and 1,8-octanediol (isolated OH) is
    // 2.3 + 2.3 = 152.0, both exactly the published calculated volumes.
    std::vector<int> oh;
    for (int i : idx) {
        const OutAtom& o = perceived[i];
        if (is_elem(o, "O") && o.n_h >= 1 && o.formal_charge == 0 &&
            !is_acid_oxygen(i, perceived, bonds))
            oh.push_back(i);
    }
    // Cluster hydroxyls whose carrier atoms are bonded to each other; one 2.3 per cluster.
    std::map<int, int> oh_rank;                       // atom index -> 0 = first in its cluster
    {
        std::set<int> unassigned(oh.begin(), oh.end());
        while (!unassigned.empty()) {
            const int seed = *unassigned.begin();
            std::vector<int> cluster{seed};
            unassigned.erase(seed);
            for (size_t k = 0; k < cluster.size(); ++k) {
                for (int other : std::vector<int>(unassigned.begin(), unassigned.end())) {
                    bool adjacent = false;
                    for (int ca : bonds.nb[cluster[k]])
                        for (int cb : bonds.nb[other])
                            if (ca == cb || std::find(bonds.nb[ca].begin(), bonds.nb[ca].end(), cb)
                                                != bonds.nb[ca].end())
                                adjacent = true;
                    if (adjacent) { cluster.push_back(other); unassigned.erase(other); }
                }
            }
            std::sort(cluster.begin(), cluster.end());
            for (size_t k = 0; k < cluster.size(); ++k) oh_rank[cluster[k]] = (int) k;
        }
    }

    // ---- 2. per-atom increments ------------------------------------------------------------
    for (int i : idx) {
        const OutAtom& o = perceived[i];
        const std::string& e = o.element;
        double v = o.n_h * inc::H;
        r.mw += o.mw;
        if (o.formal_charge > 0) r.n_pos += o.formal_charge;
        if (o.formal_charge < 0) r.n_neg += -o.formal_charge;

        if (e == "C")       v += inc::C;
        else if (e == "S")  v += inc::S;
        else if (e == "F")  v += inc::F;
        else if (e == "CL") v += inc::CL;
        else if (e == "BR") v += inc::BR;
        else if (e == "I")  v += inc::I;
        else if (e == "P")  v += inc::P5;      // P(V); P(III) is rare in biomolecules
        else if (e == "O") {
            // Only a hydroxyl or an oxyanion can be an "acid" oxygen at 0.4. A neutral oxygen
            // with no H is a carbonyl, ether, ester, ring, amine-oxide or nitro O, all 5.5 --
            // testing it for acidity would let a carboxyl's own =O answer "yes" via its carbon.
            if (o.n_h >= 1 || o.formal_charge < 0) {
                if (is_acid_oxygen(i, perceived, bonds))
                    v += inc::O_ACID;
                else
                    v += (oh_rank.count(i) && oh_rank[i] == 0) ? inc::O_OH_FIRST : inc::O_OH_NEXT;
            } else {
                v += inc::O_CARBONYL;
            }
        } else if (e == "N") {
            if (bonds.aromatic[i])                              v += inc::N_AROMATIC;
            else if (is_guanidinium_N(i, perceived, bonds))     v += inc::N_GUANIDINIUM;
            else if (o.formal_charge > 0)                       v += inc::N_AMMONIUM_1;
            // Amide N, including the peptide backbone. Table 1 lists secondary amide N at 4.0,
            // but the peptide-group increment of Table 4 (33.5 = N + 2C + 2H + O) requires 2.0,
            // and 2.0 is what reproduces the tabulated residue volumes; 4.0 biases them +4.8%.
            // Table 1's "alpha- and omega-amino groups in aminoacids and peptides = 2.0" is the
            // entry that applies.
            else if (bonded_to_carbonyl_C(i, perceived, bonds)) v += inc::N_AMIDE;
            else                                                v += inc::N_AMINE;
        } else {
            // No published increment for this element (metals, Se, ...). Additive schemes do not
            // work for metal centres; say so rather than invent a number.
            r.review.push_back(atoms[i].name + " (" + e + "): no volume increment for this element");
        }
        r.atomic_sum += v;
    }

    // ---- 3. rings lying entirely within the residue -----------------------------------------
    for (const std::vector<int>& ring : bonds.rings) {
        bool inside = true;
        for (int a : ring) if (!in_res.count(a)) { inside = false; break; }
        if (!inside) continue;
        const double d = inc::ring_decrement((int) ring.size());
        if (d == 0.0) continue;
        r.ring_decrement += d;
        ++r.n_rings;
    }

    // ---- 3b. optional pH 7 ionisation --------------------------------------------------------
    if (opt.assume_ph7_ionization) {
        for (int i : idx) {
            const OutAtom& o = perceived[i];
            if (o.formal_charge != 0) continue;             // already charged by perception
            if (is_elem(o, "O") && o.n_h >= 1) {
                // a carboxyl -OH is deprotonated at pH 7
                for (int n : bonds.nb[i]) {
                    if (!is_elem(perceived[n], "C")) continue;
                    if (carbon_has_carbonyl_O(n, perceived, bonds, i)) { ++r.n_neg; break; }
                }
            } else if (is_elem(o, "N") && bonds.nb[i].size() == 1 &&
                       is_guanidinium_N(i, perceived, bonds)) {
                // one positive charge per guanidinium group, not per nitrogen
                bool first = true;
                for (int n : bonds.nb[i])
                    for (int m : bonds.nb[n])
                        if (is_elem(perceived[m], "N") && m < i && bonds.nb[m].size() == 1)
                            first = false;
                if (first) ++r.n_pos;
            }
        }
    }

    // ---- 4. covolume and electrostriction ---------------------------------------------------
    r.covolume = opt.free_molecule ? inc::COVOLUME : 0.0;
    r.electrostriction = r.n_pos * inc::ES_POS + r.n_neg * inc::ES_NEG;

    r.molar_volume = r.atomic_sum + r.covolume - r.ring_decrement - r.electrostriction;
    r.vbar = r.mw > 0 ? r.molar_volume / r.mw : 0.0;
    r.ok = r.mw > 0;
    return r;
}

} // namespace somo_psv
