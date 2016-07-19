#ifndef US_ZENO_H
#define US_ZENO_H

int
zeno_main(
  int argc,
  char const* argv[]);

#include "qstring.h"
#include "qdatetime.h"
#include "qfile.h"
#include "qdir.h"
#include "qtimer.h"

#include "../include/us_hydrodyn.h"
#if __cplusplus >= 201103L
int zeno_cxx_main(int argc, char **argv, const char * fname );
#endif

class US_Hydrodyn_Zeno
{
 public:
   US_Hydrodyn_Zeno( 
                    hydro_options *         options,
                    hydro_results *         results,
                    US_Hydrodyn *           us_hydrodyn
                    );

   static bool    test();
   bool    run(
               QString                 filename,
               vector < PDB_atom > *   bead_model,
               double              &   sum_mass,
               double              &   sum_volume,
               bool                    keep_files = false,
               bool                    zeno_cxx = false,
               int                     threads = 1
               );
   QString error_msg;

 private:
   QString                 filename;
   hydro_options *         options;
   hydro_results *         results;
   vector < PDB_atom > *   bead_model;
   US_Hydrodyn *           us_hydrodyn;
   bool                    keep_files;
};

#endif
