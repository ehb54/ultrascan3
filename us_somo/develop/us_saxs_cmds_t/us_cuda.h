#ifndef _US_CUDA
#define _US_CUDA

extern bool cuda_hello_world();

extern bool cuda_reset();

// cuda debye, everything must be preallocated
extern bool cuda_debye(
    bool autocorrelate,
    unsigned int n,  // number of atoms
    float* pos,      // each atom will have 3 floats for coordinates,
    // so this will have 3 * n entries

    unsigned int q_points,
    float* q,  // this is the q grid

    float* fp,  // this has the scattering factors for each atom
    // it is 2d and it will have n * q_points entries
    // currently it is in atom, q order
    // it would be good to test q, atom order since we are currently
    // computing I(q) with q per thread

    // output:
    float* I,  // the computed debye output, q_points

    unsigned int threads_per_block);

#endif
