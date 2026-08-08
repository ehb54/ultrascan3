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
// rings = SSSR (smallest set of smallest rings), one entry per ring, atom indices in cycle order.
// Its size is the circuit rank |bonds| - |atoms| + |components|, so a fused bicyclic such as
// naphthalene or indole yields exactly 2 rings -- NOT the 3 simple cycles a full enumeration
// finds. That distinction matters wherever a quantity is charged once per ring, e.g. the
// Durchschlag & Zipper ring-formation volume decrement.
struct Bonds {
    std::vector<std::vector<int>> nb;
    std::vector<char> aromatic;
    std::vector<char> ring_double;
    std::vector<std::vector<int>> rings;
};

// Smallest set of smallest rings, from adjacency alone (no geometry). Fills b.rings.
// Rings longer than max_ring are not sought: the callers that need this charge a single
// "large ring" term for anything from 9 up, so exhaustive search of big macrocycles buys
// nothing and costs exponentially.
void find_sssr(Bonds& b, int max_ring = 12);

// ---- Bead colour policy for machine-generated (non-coded) residues -------------------------
// SINGLE POINT OF DEFINITION. Change DEFAULT_BEAD_COLOR here to change the policy everywhere;
// do not write bare colour numbers anywhere else.
//
// 10 = Light Green, which the SOMO manual (somo/doc/manual/somo/somo_residue.html, Panel 3)
// documents as "USED by the Automatic Bead Builder for non-coded residues". (eb, 2026-08-08.)
//
// Colour is NOT cosmetic -- it categorises the bead, and two values cause it to be dropped from
// the hydrodynamic computation entirely. Never emit a reserved colour:
//    0 black  RESERVED - very small beads, always EXCLUDED from computations
//    6 brown  RESERVED - buried beads, auto-assigned during model generation
//    7 white  USED for fused beads
//    8 grey   USED for beads previously buried but found exposed on re-check
// Selectable: 1 blue main-chain, 2 green acidic, 3 cyan hydrophobic, 4 red polar,
//    5 magenta non-polar, 9 light blue lipid tails/CO, 10 light green (this default),
//    11 light cyan DNA/RNA bases, 12 light red cofactors/prosthetic groups,
//    13 light magenta carbohydrates, 14 yellow basic, 15 bright white unassigned.
static constexpr int DEFAULT_BEAD_COLOR = 10;
static constexpr int MAX_BEAD_COLOR     = 15;

inline bool bead_color_is_reserved(int c) {
    return c == 0 || c == 6 || c == 7 || c == 8;
}
inline bool bead_color_is_selectable(int c) {
    return c >= 0 && c <= MAX_BEAD_COLOR && !bead_color_is_reserved(c);
}

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

    // Same, but also hands back the perceived bond/ring graph. Downstream consumers that need
    // topology rather than just per-atom types -- ring sizes and counts for volume increments,
    // neighbour walks to classify an N or O environment -- use this form.
    std::vector<OutAtom> perceive(const std::vector<InAtom>& atoms,
                                  Bonds& bonds_out,
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
    // Values the perceiver cannot derive from geometry alone, supplied by the caller so this
    // header stays free of the psv / volume / hydration headers (which include this one).
    // Anything left unset is emitted as 0 and called out in the entry's REVIEW block, so a
    // partially-filled entry is always visibly partial rather than quietly wrong.
    struct Properties {
        bool   have_vbar = false;    double vbar = 0;      // cm^3/g
        bool   have_molvol = false;  double molvol = 0;    // A^3, also the single bead's volume
        std::vector<double> hydration;                     // per atom; empty = unset
        std::vector<std::string> review;                   // extra lines for the REVIEW block
        int    bead_color = DEFAULT_BEAD_COLOR;
    };

    Emitted emit_residue(const std::string& resname,
                         const std::vector<InAtom>& atoms,
                         const std::vector<OutAtom>& perceived,
                         const std::string& chemical_name = "",
                         const Properties* props = nullptr) const;

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
