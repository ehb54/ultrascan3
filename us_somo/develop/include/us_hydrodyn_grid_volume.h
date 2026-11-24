#ifndef US_HYDRODYN_GRID_VOLUME_H
#define US_HYDRODYN_GRID_VOLUME_H

// Grid-based molecular volume (van der Waals / solvent-excluded), Qt-free.
//
// Replaces the external dependency on 3V (vossvolvox) `Volume.exe` for filling the
// "Residue anhydrous mol. vol." field of somo.residue. Reproduces that program's numbers --
// see tests/grid_volume.cpp, which checks against analytic shells and against values measured
// with Volume.exe itself.
//
// DEFINITION (this is the part that matters; SOMO's tabulated residue volumes are on the
// probe = 1.4 A basis, NOT probe = 0 -- see DECISIONS.md):
//   accessible A = { v : |v - c_i| > r_i + probe  for every atom i }   (probe-CENTRE positions)
//   excluded   E = { v : v not in A, and dist(v, A) > probe }
//   volume       = |E| * grid^3
//
// probe = 0    -> E is the bare union of van der Waals spheres.
// probe = 1.4  -> E is the solvent-excluded volume: vdW plus the crevices a water centre
//                 cannot reach.
//
// Interior cavities are NOT counted: a sealed void large enough to hold a probe centre is in A
// by the local test above, so it is excluded from E. That is deliberate -- it is what makes
// V(complex) - V(complex minus ligand) measure a bound ligand's volume in context. (3V's
// separate `VolumeNoCav.exe` fills cavities by flood-filling from outside first; we match plain
// `Volume.exe`.)

#include <cstdint>
#include <vector>

namespace somo_volume {

struct Sphere {
    double x = 0, y = 0, z = 0, r = 0;
};

struct Result {
    double volume = 0;      // A^3
    double probe  = 0;
    double grid   = 0;
    long   voxels = 0;      // occupied voxels in E
    long   total  = 0;      // voxels in the bounding box (sanity/memory check)
    bool   ok     = false;
};

struct Options {
    double grid  = 0.25;    // A. 0.25 reproduces Volume.exe to ~0.5%; 0.5 is ~2% and much faster.
    double probe = 1.4;     // A. 1.4 = water. Use 0.0 for the bare vdW volume.
    // Hard ceiling on allocation so a bad grid/box cannot exhaust memory. Two bytes per voxel.
    long   max_voxels = 400L * 1000L * 1000L;
};

// Returns ok=false (volume 0) if the input is empty or the requested grid would exceed
// max_voxels -- callers must check, never silently treat that as "zero volume".
Result grid_volume(const std::vector<Sphere>& atoms, const Options& opt = {});

} // namespace somo_volume

#endif
