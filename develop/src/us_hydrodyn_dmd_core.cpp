// us_hydrodyn.cpp contains class creation & gui connected functions
// us_hydrodyn_core.cpp contains the main computational routines
// us_hydrodyn_bd_core.cpp contains the main computational routines for brownian dynamic browflex computations
// us_hydrodyn_anaflex_core.cpp contains the main computational routines for brownian dynamic (anaflex) computations
// (this) us_hydrodyn_dmd_core.cpp contains the main computational routines for molecular dynamic (dmd) computations
// us_hydrodyn_other.cpp contains other routines such as file i/o

// includes and defines need cleanup

#include "../include/us_hydrodyn.h"
#include <qregexp.h>
#include <values.h>

#ifndef WIN32
#   include <unistd.h>
#   define SLASH "/"
#else
#   include <direct.h>
#   include <io.h>
#   define SLASH "\\"
#endif

