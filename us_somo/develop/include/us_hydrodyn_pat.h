#ifndef US_HYDRODYN_PAT_H
#define US_HYDRODYN_PAT_H

#if defined(USE_MPI)
#include <mpi.h>
#endif

#include "../include/us_hydrodyn_supc_hydro.h"

struct dati1_pat {
      float x, y, z; /* coordinate del centro della sfera */
      float rg, r; /* raggio della sfera                */
      float m; /* massa                             */
      int col; /* colore                            */
};

int us_hydrodyn_pat_main(int use_nmax, int in_nat, dati1_supc *in_dt, int *out_nat, dati1_pat *out_dt);

#define US_HYDRODYN_PAT_ERR_MEMORY_ALLOC -10

#endif
