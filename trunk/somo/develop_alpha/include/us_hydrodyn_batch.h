#ifndef US_HYDRODYN_BATCH_H
#define US_HYDRODYN_BATCH_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <q3frame.h>
#include <qcheckbox.h>
#include <qwt_counter.h>
#include <q3buttongroup.h>
#include <q3textedit.h>
#include <q3progressbar.h>
#include <qmenubar.h>
#include <qfileinfo.h>
#include <qprinter.h>
#include <q3listbox.h>
#include <q3dragobject.h>
#include <q3widgetstack.h>
//Added by qt3to4:
#include <QCloseEvent>
#include <QDropEvent>
#include <QResizeEvent>
#include <QDragEnterEvent>

#include "us_util.h"
#include "us_hydrodyn_comparative.h"
#include "us_hydrodyn_pdb_parsing.h"

//standard C and C++ defs:

#include <map>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace std;

struct batch_info 
{
   vector < QString > file;
   int missing_atoms;      // 0 = Halt
                           // 1 = Skip
                           // 2 = use approximate method
   int missing_residues;   // 0 = Halt
                           // 1 = Skip
                           // 2 = use automatic bead builder (approximate method, default)
   bool mm_first;
   bool mm_all;
   bool dmd;
   bool somo;
   bool grid;
   bool iqq;
   bool prr;
   bool csv_saxs;
   bool create_native_saxs;
   QString csv_saxs_name;
   bool hydro;
   bool zeno;
   bool avg_hydro;
   QString avg_hydro_name;
   int height;
   int width;
   bool compute_iq_avg;
   bool compute_iq_only_avg;
   bool compute_iq_std_dev;
   bool compute_prr_avg;
   bool compute_prr_std_dev;
   bool hydrate;
   bool saxs_search;
   bool equi_grid;
};

class US_EXTERN US_Hydrodyn_Batch : public Q3Frame
{
   Q_OBJECT

   public:
      friend class US_Hydrodyn;
      friend class US_Hydrodyn_Cluster;
      friend class US_Hydrodyn_Cluster_Dmd;

      US_Hydrodyn_Batch(batch_info *batch, 
                        bool *batch_widget, 
                        void *us_hydrodyn, 
                        QWidget *p = 0, 
                        const char *name = 0);
      ~US_Hydrodyn_Batch();

      QCheckBox     *cb_saveParams;
      bool          save_batch_active;
      bool          batch_job_running;

   protected:

      void dragEnterEvent(QDragEnterEvent *event);
      void dropEvent(QDropEvent *event);

   private:

      batch_info    *batch;
      bool          *batch_widget;
      void          *us_hydrodyn;

      US_Config     *USglobal;

      QLabel        *lbl_selection;
      Q3ListBox      *lb_files;
      QPushButton   *pb_add_files;
      QPushButton   *pb_select_all;
      QPushButton   *pb_remove_files;
      QPushButton   *pb_load_somo;
      QPushButton   *pb_load_saxs;
      QPushButton   *pb_make_movie;

      QLabel        *lbl_total_files;
      QLabel        *lbl_selected;

      QLabel        *lbl_screen;
      Q3ButtonGroup  *bg_residues;
      QCheckBox     *cb_residue_stop;
      QCheckBox     *cb_residue_skip;
      QCheckBox     *cb_residue_auto;
      Q3ButtonGroup  *bg_atoms;
      QCheckBox     *cb_atom_stop;
      QCheckBox     *cb_atom_skip;
      QCheckBox     *cb_atom_auto;
      QPushButton   *pb_screen;

      QLabel        *lbl_process;
      QCheckBox     *cb_mm_first;
      QCheckBox     *cb_mm_all;
      QCheckBox     *cb_dmd;
      QCheckBox     *cb_grid;
      QCheckBox     *cb_equi_grid;
      QCheckBox     *cb_somo;
      QCheckBox     *cb_iqq;
      QCheckBox     *cb_saxs_search;
      QCheckBox     *cb_prr;
      QCheckBox     *cb_csv_saxs;
      QLineEdit     *le_csv_saxs_name;
      QCheckBox     *cb_create_native_saxs;
      QCheckBox     *cb_hydrate;
      QCheckBox     *cb_compute_iq_avg;
      QCheckBox     *cb_compute_iq_only_avg;
      QCheckBox     *cb_compute_iq_std_dev;
      QCheckBox     *cb_compute_prr_avg;
      QCheckBox     *cb_compute_prr_std_dev;
      QCheckBox     *cb_hydro;
      QCheckBox     *cb_zeno;
      QCheckBox     *cb_avg_hydro;
      QLineEdit     *le_avg_hydro_name;

      QPushButton   *pb_select_save_params;
      QPushButton   *pb_start;
      Q3ProgressBar  *progress;
      Q3WidgetStack  *ws_progress2;
      QLabel        *lbl_progress2;
      Q3ProgressBar  *progress2;
      QPushButton   *pb_stop;

      QPushButton   *pb_help;
      QPushButton   *pb_cluster;
      QPushButton   *pb_open_saxs_options;
      QPushButton   *pb_cancel;

      Q3TextEdit     *editor;

      QMenuBar      *m;
      QPrinter      printer;

      pdb_parsing   save_pdb_parse;
      bool          save_pb_rule_on;
      bool          save_calcAutoHydro;
      bool          save_disable_iq_scaling;

      bool          save_misc_supc;
      bool          save_misc_zeno;

      bool          stopFlag;
      bool          disable_updates;
      QFont ft;

      bool          any_pdb_in_list;
#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( disable: 4251 )
# endif
#endif

      map < QString, int > status; 

      // status values: 0 nothing done
      //                1 file missing
      //                2 screening now
      //                3 screen ok
      //                4 screen failed
      //                5 processing now
      //                6 processing ok
      //                7 processing failed

      vector < QString >   status_color;

      QString saxs_header_iqq;
      QString saxs_header_prr;
      vector < QString > csv_source_name_iqq;
      vector < QString > csv_source_name_prr;
      vector < double > saxs_q;
      vector < vector < double > > saxs_iqq;
      vector < vector < double > > saxs_iqqa;
      vector < vector < double > > saxs_iqqc;
      vector < double > saxs_r;
      vector < vector < double > > saxs_prr;
      vector < vector < double > > saxs_prr_norm;
      vector < float > saxs_prr_mw;

#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( default: 4251 )
# endif
#endif
      void save_csv_saxs_iqq( bool quiet = false );
      void save_csv_saxs_prr();
      QString vector_double_to_csv( vector < double > vd );
      double compute_pr_area( vector < double > vd, vector < double > r );

      // message utility
      void editor_msg( QString color, QString msg );
      QString iqq_suffix();
      bool activate_saxs_search_window();

      QString      cluster_no_of_jobs;
      QStringList  cluster_target_datafiles;
      QString      cluster_output_name;
      bool         cluster_for_mpi;
      bool         cluster_split_grid;
      bool         cluster_dmd;
      csv          cluster_csv_advanced;
      csv          cluster_csv_dmd;

   public :
      void add_file( QString filename );
      void add_files( vector < QString > filenames );
      int count_files(); // return current # of files
      void clear_files(); // reset all files

   private slots:
      
      void setupGUI();

      void add_files();
      void select_all();
      void remove_files();
      void load_somo();
      void load_saxs();

      void residue(int);
      void atom(int);
      void screen();

      void start( bool quiet = false );
      void set_mm_first();
      void set_mm_all();
      void set_dmd();
      void set_somo();
      void set_grid();
      void set_equi_grid();
      void set_hydro();
      void set_zeno();
      void set_iqq();
      void set_saxs_search();
      void set_prr();
      void set_avg_hydro();
      void update_avg_hydro_name(const QString &);
      void set_csv_saxs();
      void update_csv_saxs_name(const QString &);
      void set_create_native_saxs();
      void set_hydrate();
      void set_compute_iq_avg();
      void set_compute_iq_only_avg();
      void set_compute_iq_std_dev();
      void set_compute_prr_avg();
      void set_compute_prr_std_dev();
      void select_save_params();
      void set_saveParams();
      void stop();

      void make_movie();

      void cancel();
      void cluster();
      void open_saxs_options();
      void help();

      // editor functions:
      void save();
      void print();
      void update_font();
      void clear_display();

      bool screen_pdb(QString file, bool display_pdb = false);
      bool screen_bead_model(QString file);

      void save_us_hydrodyn_settings();
      void restore_us_hydrodyn_settings();
      
      void update_enables();
      void disable_after_start();
      void enable_after_stop();

      void set_counts();

      QString get_file_name(int i);
      void check_for_missing_files(bool display_messages);

   protected slots:

      void closeEvent(QCloseEvent *);
      void resizeEvent(QResizeEvent *);
   
};

#endif
