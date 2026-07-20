// somo_perceiver.h -- perceive SOMO hybrid types from element + 3D coordinates.
//
// Isolated, Qt-free. Input: a flat list of atoms (element + xyz, plus optional passthrough
// identity used only for output grouping, never for classification). Output: per-atom hybrid
// type + physics (mw / vdW radius / electrons) via the HybridTable, or synthesized for novel
// types. This is the "chemical perception" core; SOMO would feed its parsed atoms in.
#pragma once
#include <string>
#include <utility>
#include <vector>
#include "us_hydrodyn_perceive_hybrid.h"

namespace somo_perceive {

struct InAtom {
    std::string element;                 // "C","N","O",... (may be inferred upstream)
    double x = 0, y = 0, z = 0;
    int  serial = 0;                     // PDB serial (only used to resolve CONECT records)
    // passthrough identity (NOT used by the classifier):
    std::string name, resName, chain;
    int  resSeq = 0;
    bool hetatm = false;
};

struct OutAtom {
    std::string hybrid;                  // e.g. "C4H3","N3H1","FE+2"
    std::string element;
    int  heavy_neighbors = 0;
    int  n_h = 0;                        // inferred implicit hydrogens
    int  formal_charge = 0;
    // physics:
    double mw = 0, vdw_radius = 0;
    int    num_elect = 0;
    bool   in_table = false;             // hybrid found in somo.hybrid.new
    int    ambiguity = 0;                // 0 confident, 1 heuristic (see note)
    std::string note;
};

// adjacency: nb[i] = indices of heavy-atom neighbors of atom i.
// aromatic[i]    != 0 if atom i belongs to a planar, conjugated 5- or 6-membered ring.
// ring_double[i] != 0 if atom i was assigned a ring double bond by Kekule perception
//                 (used to tell pyridine-type ring N (no H) from pyrrole-type (has H)).
struct Bonds {
    std::vector<std::vector<int>> nb;
    std::vector<char> aromatic;
    std::vector<char> ring_double;
};

struct Params {
    double bond_tol      = 0.45;   // dist < r_cov(i)+r_cov(j)+tol  => bonded
    double bond_min      = 0.40;   // reject closer than this (clashes)
    double planar_thresh = 0.30;   // A out-of-plane dist (A) below => planar/sp2 (3 neighbors)
    double sp_angle      = 150.0;  // 2-coord angle above => sp (linear)
    double sp2_angle     = 115.0;  // 2-coord angle above (and below sp) => sp2, else sp3
    bool   metals_isolated = true; // don't bond-perceive monatomic ions
};

class Perceiver {
public:
    explicit Perceiver(const HybridTable& tbl, Params p = {}) : tbl_(tbl), p_(p) {}

    // explicit_bonds: optional authoritative connectivity as ATOM INDEX pairs (e.g. resolved from
    // PDB CONECT records). Merged with distance-based perception; a metal that appears here is NOT
    // treated as an isolated ion, since its ligand bonds are real covalent/cluster bonds.
    std::vector<OutAtom> perceive(const std::vector<InAtom>& atoms,
                                  const std::vector<std::pair<int,int>>& explicit_bonds = {}) const;

    // Emit a somo.residue-style entry for one residue plus any somo.hybrid lines for perceived
    // types not already in the table. Chemistry (atom name, hybrid type, mass, radius, electrons)
    // is perceived; modeling fields (bead assignment, ASA) are stubbed and molvol/vbar estimated.
    struct Emitted {
        std::string residue_block;             // ready to append to somo.residue
        std::vector<std::string> new_hybrids;  // ready to append to somo.hybrid
    };
    // chemical_name: optional full name (e.g. from the PDB HETNAM header record) used as the
    // entry's comment line, which is far more informative than the bare 3-letter code.
    Emitted emit_residue(const std::string& resname,
                         const std::vector<InAtom>& atoms,
                         const std::vector<OutAtom>& perceived,
                         const std::string& chemical_name = "") const;

    const Params& params() const { return p_; }

private:
    const HybridTable& tbl_;
    Params p_;

    // adjacency + geometry
    Bonds perceive_bonds(const std::vector<InAtom>& a,
                         const std::vector<std::pair<int,int>>& explicit_bonds) const;

    OutAtom classify(int i, const std::vector<InAtom>& a, const Bonds& b) const;
    void    finalize_physics(OutAtom& o) const;

    // per-element classifiers; set o.hybrid, o.n_h, o.formal_charge, o.ambiguity, o.note
    void classify_C(int i, const std::vector<InAtom>&, const Bonds&, OutAtom&) const;
    void classify_N(int i, const std::vector<InAtom>&, const Bonds&, OutAtom&) const;
    void classify_O(int i, const std::vector<InAtom>&, const Bonds&, OutAtom&) const;
    void classify_S(int i, const std::vector<InAtom>&, const Bonds&, OutAtom&) const;
    void classify_P(int i, const std::vector<InAtom>&, const Bonds&, OutAtom&) const;
};

} // namespace somo_perceive
