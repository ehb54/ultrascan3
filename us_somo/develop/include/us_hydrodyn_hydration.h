#ifndef US_HYDRODYN_HYDRATION_H
#define US_HYDRODYN_HYDRATION_H

// Proposed per-atom hydration for a residue somo.residue does not code, Qt-free.
//
// WHAT THE SOURCE DATA ACTUALLY IS -- read before trusting any number from here.
// somo.residue stores hydration per atom (field 8 of each atom line). Summed per residue it
// reproduces the classical Kuntz numbers (Durchschlag 1986 Table 1, pH 6-8): 14 of the 20 amino
// acids agree within 0.5 water, and the two large misses, Asp (1 vs 6) and Glu (1 vs 7.5), are
// exactly the residues SOMO stores PROTONATED and which therefore carry no charged-carboxylate
// water. So the per-atom values are a hand distribution of a per-residue total, not an
// independently derived per-atom rule, and the same chemical group is sometimes hydrated in one
// entry and not in another (ADP.O2' = 1 but A.O2' = 0; MET.SD = 1 but CYS.SG = 0).
//
// Consequently this is a STARTING POINT FOR INTERACTIVE EDITING, exactly as specified, and must
// never be presented to the user as a physical prediction. Two numbers are worth showing: the
// per-atom proposal, and the residue TOTAL -- the total is the quantity with literature backing.
//
// The table is derived at run time from whatever somo.residue is loaded, so it cannot drift from
// the user's own tables. Types whose observations disagree are marked not-confident and must be
// surfaced for review rather than silently defaulted.
//
// NOTE (Mattia, 2026-08-08): hydration in somo.residue is pH dependent, and so are the
// hybridizations of the atoms involved. Everything here is the neutral-pH picture.

#include <map>
#include <string>
#include <vector>

#include "us_hydrodyn_perceive.h"

namespace somo_hydration {

struct Entry {
    double waters = 0;       // most common value observed for this hybrid type
    int    n_obs = 0;
    double agreement = 0;    // fraction of observations equal to `waters`
    bool   confident = false;
};

class Table {
public:
    // obs: (hybrid type, waters) for every atom of every coded residue.
    static Table from_observations(const std::vector<std::pair<std::string, double>>& obs,
                                   double confident_at = 0.75);

    // Returns nullptr when the type was never observed.
    const Entry* find(const std::string& hybrid) const;
    size_t size() const { return by_hybrid_.size(); }
    const std::map<std::string, Entry>& all() const { return by_hybrid_; }

private:
    std::map<std::string, Entry> by_hybrid_;
};

struct Proposal {
    std::vector<double> per_atom;          // parallel to the idx passed in
    double total = 0;
    std::vector<std::string> review;       // atoms whose type is unknown or not confident
    bool ok = false;
};

Proposal propose(const Table& t,
                 const std::vector<somo_perceive::InAtom>& atoms,
                 const std::vector<somo_perceive::OutAtom>& perceived,
                 const std::vector<int>& idx);

} // namespace somo_hydration

#endif
