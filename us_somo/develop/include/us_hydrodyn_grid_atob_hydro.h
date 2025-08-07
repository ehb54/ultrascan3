#ifndef US_HYDRODYN_GRID_ATOB_H
#define US_HYDRODYN_GRID_ATOB_H

#include "../include/us_file_util.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_grid.h"
#include "../include/us_hydrodyn_pdbdefs.h"
#include "../include/us_json.h"

#ifdef OSX
#include <sys/malloc.h>
#endif

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <values.h>
#include <list>

typedef float REAL;

typedef struct pdb_entry {
      REAL x, y, z, occ, bval;
      struct pdb_entry *next;
      int atnum;
      int resnum;
      char junk[ 8 ];
      char atnam[ 8 ];
      char resnam[ 8 ];
      char insert[ 8 ];
      char chain[ 8 ];
} PDB;

typedef struct physical_properties {
      REAL f, rVW; /*electron density */
      REAL mass;
      REAL si;
      struct physical_properties *next;
      float saxs_excl_vol;
} PHYSPROP;

vector<PDB_atom> us_hydrodyn_grid_atob_hydro(
   vector<PDB_atom> *bead_model, grid_options *our_grid_options, US_Log *us_log, US_Udp_Msg *us_udp_msg,
   QString *accumulated_msgs);
/* QProgressBar *progress, */
/* QTextEdit *editor, */
/* US_Hydrodyn* us_hydrodyn */
/* ); */

#endif
