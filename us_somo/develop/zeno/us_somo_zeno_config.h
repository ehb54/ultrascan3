#ifndef US_SOMO_ZENO_CONFIG_H
#define US_SOMO_ZENO_CONFIG_H

// How US-SOMO builds the NIST ZENO code in this directory (see README.US-SOMO).
// Include this before any ZENO header, after any SOMO header.

// ZENO runs in a single process in every SOMO program, including the MPI ones,
// which define USE_MPI for their own code.
#undef USE_MPI

// SPRNG is not bundled.  LeapFrog is NIST ZENO's generator when SPRNG is off.
#define USE_LEAP_FROG_RNG

// Voxel (.fits.gz) input is not used by SOMO and would need zlib.
#define ZENO_NO_VOXELS

#endif // US_SOMO_ZENO_CONFIG_H
