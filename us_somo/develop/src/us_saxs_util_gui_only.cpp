#include "../include/us_saxs_util.h"

// not sure why this is suddenly needed ...

bool US_Saxs_Util::read_pdb_hydro( QString, bool ) {
   QTextStream( stderr ) << "US_Saxs_Util::read_pdb_hydro() called\n" << "\n";
   exit(-1);
   return false;
}
