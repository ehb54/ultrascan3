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
#include <qprogressbar.h>

#include "../include/us_hydrodyn.h"
#if !defined(USE_OLD_ZENO) && __cplusplus >= 201103L
int zeno_cxx_main(int argc, char **argv, const char * fname, bool cmdline_temp, US_Udp_Msg * zeno_us_udp_msg );
#endif

#if !defined(USE_OLD_ZENO) && QT_VERSION >= 0x040000 && __cplusplus < 201103L
# error C++11 or greater is required
#endif

class US_Hydrodyn_Zeno
{
 public:
   US_Hydrodyn_Zeno( 
                    hydro_options *         options,
                    hydro_results *         results,
                    mQProgressBar *         use_progress,
                    US_Hydrodyn *           us_hydrodyn
                    );

   static bool    test();
   bool    run(
               QString                 filename,
               vector < PDB_atom > *   bead_model,
               double              &   sum_mass,
               double              &   sum_volume,
               const double        &   Rg,
               mQProgressBar *         use_progress,
               bool                    keep_files = false,
               bool                    zeno_cxx = false,
               int                     threads = 1
               );
   QString error_msg;
   void        update_progress         ( int pos, int total );

 private:
   QString                 filename;
   hydro_options *         options;
   hydro_results *         results;
   vector < PDB_atom > *   bead_model;
   US_Hydrodyn *           us_hydrodyn;
   bool                    keep_files;

 signals:
    void                   progress_updated( int, int );
};

#endif
