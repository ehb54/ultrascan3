#ifndef US_HYDRODYN_ASAB1_H
#define US_HYDRODYN_ASAB1_H

#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_hydro.h"
#include "../include/us_hydrodyn_pdbdefs.h"


#include <stdio.h>
#ifdef OSX
# include <sys/malloc.h>
#endif
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

int
us_hydrodyn_asab1_main(vector <PDB_atom> *bead_model, 
		       QProgressBar *progress,
		       QTextEdit *editor);

#define US_HYDRODYN_ASAB1_ERR_MEMORY_ALLOC   -1

#endif
