#ifndef US_ZENO_H
#define US_ZENO_H

int zeno_main(int argc, char const* argv[]);

#include "../include/us_file_util.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_json.h"
#include "qdatetime.h"
#include "qdir.h"
#include "qfile.h"
#include "qstring.h"
#include "qtimer.h"

#if !defined(USE_OLD_ZENO) && __cplusplus >= 201103L
int zeno_cxx_main(int argc, char** argv, const char* fname, bool cmdline_temp,
                  US_Udp_Msg* zeno_us_udp_msg);
#endif

class US_Hydrodyn_Zeno {
 public:
  US_Hydrodyn_Zeno(hydro_options* options, hydro_results* results,
                   US_Log* us_log, US_Udp_Msg* us_udp_msg,
                   QString* accumulated_msgs);
  /* US_Hydrodyn *           us_hydrodyn */
  /* ); */

  static bool test();
  bool run(QString filename, vector<PDB_atom>* bead_model,
           bool keep_files = false);
  bool run(QString filename, vector<PDB_atom>* bead_model, double& sum_mass,
           double& sum_volume, bool keep_files, bool zeno_cxx, int threads);

  QString error_msg;

 private:
  QString filename;
  hydro_options* options;
  hydro_results* results;
  vector<PDB_atom>* bead_model;
  //   US_Hydrodyn *           us_hydrodyn;
  US_Log* us_log;
  US_Udp_Msg* us_udp_msg;
  QString* accumulated_msgs;
  bool keep_files;
};

#endif
