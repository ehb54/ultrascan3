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

#include "../include/us_hydrodyn.h"
#include "../include/us_file_util.h"
#include "../include/us_json.h"

class US_Hydrodyn_Zeno
{
  public:
       US_Hydrodyn_Zeno( 
         hydro_options *         options,
	 hydro_results *         results,
	 US_Log *                us_log,
	 US_Udp_Msg *            us_udp_msg,
	 QString *               accumulated_msgs
			 );
/* US_Hydrodyn *           us_hydrodyn */
/* ); */

   static bool    test();
   bool    run(
               QString                 filename,
               vector < PDB_atom > *   bead_model,
               bool                    keep_files = false
               );
   QString error_msg;

 private:
   QString                 filename;
   hydro_options *         options;
   hydro_results *         results;
   vector < PDB_atom > *   bead_model;
   //   US_Hydrodyn *           us_hydrodyn;
   US_Log *                us_log;
   US_Udp_Msg *            us_udp_msg;
   QString *               accumulated_msgs;
   bool                    keep_files;
};

#endif
