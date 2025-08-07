#ifndef US_HYDRODYN_ASAB1_H
#define US_HYDRODYN_ASAB1_H

#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_hydro.h"
#include "../include/us_hydrodyn_pdbdefs.h"


#include <stdio.h>
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

int us_hydrodyn_asab1_main(
   vector<PDB_atom *> active_atoms, asa_options *asa_opts, hydro_results *use_results, bool use_recheck,
   QProgressBar *progress, QTextEdit *editor, US_Hydrodyn *use_us_hydrodyn);

#define US_HYDRODYN_ASAB1_ERR_MEMORY_ALLOC -1

#endif
