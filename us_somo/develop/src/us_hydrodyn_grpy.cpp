// (this) us_hydrodyn.cpp contains class creation & gui connected functions
// us_hydrodyn_core.cpp contains the main computational routines
// us_hydrodyn_bd_core.cpp contains the main computational routines for brownian dynamic browflex computations
// us_hydrodyn_anaflex_core.cpp contains the main computational routines for brownian dynamic (anaflex) computations
// us_hydrodyn_dmd_core.cpp contains the main computational routines for molecular dynamic (dmd) computations
// us_hydrodyn_other.cpp contains other routines such as file i/o
// us_hydrodyn_info.cpp contains code to report structures for debugging
// us_hydrodyn_util.cpp contains other various code, such as disulfide code
// us_hydrodyn_load.cpp contains code to load files 
// us_hydrodyn_grpy.cpp contains code for grpy interface

// includes and defines need cleanup
 
#include "../include/us_hydrodyn.h"

bool US_Hydrodyn::calc_grpy_hydro() {
   QMessageBox::warning( this, windowTitle() + ": Calculate RB Hydrodynamics: GRPY", "GRPY not yet integrated" );
   return false;
}
