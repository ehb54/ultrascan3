#ifndef US_HYDRODYN_SUPC_H
#define US_HYDRODYN_SUPC_H

#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_hydro.h"
#include "../include/us_hydrodyn_pdbdefs.h"
#include <qlistbox.h>

#include <stdio.h>
#include <math.h>
#ifdef OSX
#include <sys/malloc.h>
#endif
#include <string.h>

int
us_hydrodyn_supc_main(hydro_results *hydro_results, 
		      hydro_options *hydro, 
		      vector < vector <PDB_atom> > *bead_models, 
		      vector <int> *somo_processed,
		      QListBox *lb_model,
		      const char *filename,
		      const char *res_filename,
		      QProgressBar *progress,
		      QTextEdit *editor);

#define US_HYDRODYN_SUPC_ERR_MEMORY_ALLOC   -1
#define US_HYDRODYN_SUPC_FILE_NOT_FOUND     -5
#define US_HYDRODYN_SUPC_OVERLAPS_EXIST     -6
#define US_HYDRODYN_SUPC_NO_SEL_MODELS      -7

#endif
