#ifndef US_HYDRODYN_RESIDUE_BUILDER_H
#define US_HYDRODYN_RESIDUE_BUILDER_H

// Ties the pieces together into one complete somo.residue entry for a residue the table does
// not code: perceive -> psv -> anhydrous volume -> hydration -> emit. Qt-free.
//
// Each part is validated independently (tests/psv.cpp, tests/grid_volume.cpp,
// tests/hydration.cpp); this only orchestrates, and its own test checks that a fully populated
// entry comes out and that every unset field is visibly unset.
//
// Nothing here writes to somo.residue. Storing a confirmed entry is a separate, user-driven
// step by design.

#include <string>
#include <vector>

#include "us_hydrodyn_grid_volume.h"
#include "us_hydrodyn_hydration.h"
#include "us_hydrodyn_perceive.h"
#include "us_hydrodyn_psv.h"

namespace somo_residue_builder {

struct Options {
    // Anhydrous volume. 1.4 A, not 0: the coded residues' volumes are solvent-excluded, and a
    // 0 A probe gives the bare van der Waals volume, about a third smaller.
    double volume_probe = 1.4;
    double volume_grid  = 0.25;
    bool   compute_volume = true;

    // Scales an isolated-molecule solvent-excluded volume onto the convention the coded
    // residues use, which is void-inclusive (a Voronoi-style partition of packed protein) and
    // so ~20-25% larger than anything an isolated measurement can see.
    //
    // Calibrated with THIS pipeline -- the perceiver's own vdW radii and the native grid volume
    // -- over all 20 amino acids, 6 instances each, from 1HEL/2AAS/1AO6. Residual ~3%.
    //
    // The factor is radius-set specific: calibrating against 3V's default radii instead gives
    // 1.204/1.131, about 5% low here because those radii are larger. Recalibrate if the radii
    // in somo.hybrid change.
    //
    // Two caveats on the shape of this correction. It tracks POLARITY at least as much as size
    // -- the hydrophobics cluster near 1.26-1.29 and the polar and charged residues near
    // 1.16-1.21 -- which is why the residual is ~3% rather than the ~1% the per-band spread
    // suggests, and why a smooth fit in volume alone would not do better. And the split is a
    // step through a continuum, so it is placed at 110 A^3 raw, below the hydrophobic cluster,
    // rather than at the midpoint where it would separate Leu (127.4) from Ile (128.4).
    double convention_factor_large = 1.248;
    double convention_factor_small = 1.179;
    double convention_split = 110.0;

    // TEMPERATURE. The Durchschlag & Zipper increments are for 25 C, but somo.residue is a 20 C
    // table -- SOMO converts with tc_vbar() (us_hydrodyn_load.cpp) using this same coefficient.
    // Emitting the raw 25 C value put every generated entry ~0.3% high, which is small enough to
    // look like method noise and is not: it is 5 K x 4.25e-4 exactly. Verified against Mattia
    // Rocco's independent recalculation, whose 25 C and 20 C columns imply 4.2500e-04 for all
    // 20 residues with zero spread; matching the reference brings 14 of 20 residues to within
    // +-0.07% of his values.
    double psv_increment_temperature = 25.0;   // what D&Z's increments describe
    double psv_table_temperature     = 20.0;   // what somo.residue stores
    double psv_dvdt                  = 4.25e-4;  // cm^3 g^-1 K^-1, SOMO's tc_vbar coefficient

    bool compute_psv = true;
    bool compute_hydration = true;
    int  bead_color = somo_perceive::DEFAULT_BEAD_COLOR;
};

struct Built {
    std::string residue_block;                 // ready to append to somo.residue
    std::vector<std::string> new_hybrids;      // any somo.hybrid lines the types need
    somo_psv::Result psv;
    somo_hydration::Proposal hydration;
    double molvol = 0;                         // A^3, after the convention factor
    double raw_volume = 0;                     // A^3, before it
    bool   ok = false;
};

// atoms/perceived/bonds cover the whole structure; idx selects the residue. hyd may be null,
// in which case hydration is left unset rather than guessed.
Built build(const std::string& resname,
            const std::vector<somo_perceive::InAtom>& atoms,
            const std::vector<somo_perceive::OutAtom>& perceived,
            const somo_perceive::Bonds& bonds,
            const std::vector<int>& idx,
            const somo_perceive::Perceiver& perc,
            const somo_hydration::Table* hyd,
            const std::string& chemical_name = "",
            const Options& opt = {});

} // namespace somo_residue_builder

#endif
