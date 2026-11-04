// us_zeno_lib.cpp -- compiles the NIST ZENO library vendored in zeno/ as one
// translation unit, so the settings in zeno/us_somo_zeno_config.h apply to all
// of it (in particular, no MPI even in the MPI programs).  The SOMO driver is
// us_zeno_cxx.cpp.  See zeno/README.US-SOMO.

#include "../zeno/us_somo_zeno_config.h"

#include "../zeno/libzeno/src/ParametersInteriorSampling.cc"
#include "../zeno/libzeno/src/ParametersResults.cc"
#include "../zeno/libzeno/src/ParametersWalkOnSpheres.cc"
#include "../zeno/libzeno/src/RandomNumber/LeapFrog.cc"
#include "../zeno/libzeno/src/Results.cc"
#include "../zeno/libzeno/src/ResultsCompiler.cc"
#include "../zeno/libzeno/src/ResultsInterior.cc"
#include "../zeno/libzeno/src/ResultsZeno.cc"
#include "../zeno/libzeno/src/Timer.cc"
#include "../zeno/libzeno/src/Units.cc"
#include "../zeno/libzeno/src/Zeno.cc"

#include "../zeno/zeno-cli/src/ParametersLocal.cc"
#include "../zeno/zeno-cli/src/cmdline.c"
