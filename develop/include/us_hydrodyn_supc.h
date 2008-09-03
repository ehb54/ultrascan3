#ifndef US_HYDRODYN_SUPC_H
#define US_HYDRODYN_SUPC_H

#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_hydro.h"
#include "../include/us_hydrodyn_pdbdefs.h"


#include <stdio.h>
#include <math.h>
#ifdef OSX
#include <sys/malloc.h>
#endif
#include <string.h>


struct dati1
{
    float x, y, z;		/* coordinates of the bead center    */
    float r;			/* hydrated radius of the bead       */
    float ru;			/* unhydrated radius of the bead     */
    int m;			/* mass of the bead                  */
    int col;			/* color of the bead                 */
    char *cor;			/* correspondence between beads and AA */
};

int
us_hydrodyn_supc_main(hydro_results *hydro_results, 
		      hydro_options *hydro, 
		      vector <PDB_atom> *bead_model, 
		      const char *filename,
		      QProgressBar *progress,
		      QTextEdit *editor);

#define US_HYDRODYN_SUPC_ERR_MEMORY_ALLOC   -1
#define US_HYDRODYN_SUPC_FILE_NOT_FOUND     -5
#define US_HYDRODYN_SUPC_OVERLAPS_EXIST     -6

#endif
