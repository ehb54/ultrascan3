#ifndef US_SAXS_UTIL_ASAB1_H
#define US_SAXS_UTIL_ASAB1_H

#if defined(USE_MPI)
#include <mpi.h>
#endif

#include <stdio.h>

#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_hydro.h"
#include "../include/us_hydrodyn_pdbdefs.h"
#ifdef OSX
#include <sys/malloc.h>
#endif
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIN32
#include <unistd.h>
#endif

int us_saxs_util_asab1_main(vector<PDB_atom *> active_atoms,
                            asa_options *asa_opts, hydro_results *use_results,
                            bool use_recheck);

#define US_SAXS_UTIL_ASAB1_ERR_MEMORY_ALLOC -1

#endif
