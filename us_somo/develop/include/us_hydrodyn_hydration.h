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

// An atom's deprotonated alternate, for entries that need to be pH-aware the way the coded
// residues are. somo.residue writes these as a 16-field atom line: the first 8 fields are the
// PROTONATED species and fields 10-16 the DEPROTONATED one, with the pKa in the residue header.
// Verified against the table --
//     ASP  OD2  O2H1  ...0  |  O1H0-  ...5      COOH  -> COO-
//     LYS  NZ   N4H3+ ...3  |  N3H2   ...1      NH3+  -> NH2
//     ARG  NH2  N3H2+ ...1  |  N3H1   ...0      guanidinium+ -> neutral
// so "primary = protonated, alternate = deprotonated" holds for acids and bases alike.
//
// pKa is a CONVENTION value taken from whichever coded residue carries the same group, not a
// prediction for this molecule: citrate's three carboxyls all come out short-chain and would be
// proposed at ASP's 3.67, where the real values are ~3.13/4.76/6.40. It is offered for the user
// to accept or change (Mattia, 2026-08-10), never presented as measured.
struct Alternate {
    std::string hybrid;                    // empty = this atom has no ionizable alternate
    double waters = 0;
    double pKa    = 0;
    std::string why;                       // group that was recognised, for the review line
};

struct Proposal {
    std::vector<double> per_atom;          // parallel to the idx passed in
    std::vector<Alternate> alternate;      // parallel to per_atom; hybrid empty when not ionizable
    double total = 0;
    double total_ionized = 0;              // the same sum with every alternate applied
    std::vector<std::string> review;       // atoms whose type is unknown or not confident
    bool ok = false;
};

// ---- pH 7 chemistry rules -------------------------------------------------------------------
// Rules from Mattia (2026-08-08), extended with the cases the coded residues cover but he did
// not enumerate. At pH 7 the ionisable groups are assumed to be in their charged form -- an
// acidic side chain is -COO- and an aliphatic amine is protonated -- WITHOUT building any
// general pH machinery, which is deliberately out of scope for now.
//
//   aliphatic carboxylate, the hydroxyl-side O   5 waters, or 6 when >= 2 sp3 carbons
//                                                separate it from the residue's attachment
//   the carbonyl-side O of the same group        0
//   terminal ammonium      (N4H3+ / sp3 NH3)     3
//   guanidinium terminal N (N3H2+)               1, and 0 for the internal one
//   aliphatic hydroxyl     (O2H1)                1
//   phenolic hydroxyl      (O2H1 on aromatic C)  0
//   amide -NH- and -NH2    (N3H1 / N3H2)         1
//   aromatic ring NH                             1, and 2 when protonated
//   thioether S                                  1, thiol S 0
//
// Anything the rules do not recognise falls back to the observed-majority Table and is flagged,
// so a novel group is never given a confident-looking number by accident.
Proposal propose_by_rules(const std::vector<somo_perceive::InAtom>& atoms,
                          const std::vector<somo_perceive::OutAtom>& perceived,
                          const somo_perceive::Bonds& bonds,
                          const std::vector<int>& idx,
                          const Table* fallback = nullptr);

Proposal propose(const Table& t,
                 const std::vector<somo_perceive::InAtom>& atoms,
                 const std::vector<somo_perceive::OutAtom>& perceived,
                 const std::vector<int>& idx);

} // namespace somo_hydration

#endif
