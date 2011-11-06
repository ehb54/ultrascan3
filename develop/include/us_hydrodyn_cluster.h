#ifndef US_HYDRODYN_CLUSTER_H
#define US_HYDRODYN_CLUSTER_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qtextedit.h>
#include <qvalidator.h>
#include <qtimer.h>

#include "us_util.h"
#include "us_file_util.h"
#include "us_hydrodyn_pdbdefs.h"
#include "us_hydrodyn_batch.h"
#include "us_hydrodyn_comparative.h"
#include "us_hydrodyn_cluster_submit.h"
#include "us_hydrodyn_cluster_status.h"
#include "us_hydrodyn_cluster_results.h"
#include "us_hydrodyn_cluster_config.h"
#include "us_hydrodyn_cluster_config_server.h"
#include "us_hydrodyn_cluster_advanced.h"
#include "us_hydrodyn_cluster_dmd.h"

//standard C and C++ defs:

using namespace std;

class US_EXTERN US_Hydrodyn_Cluster : public QDialog
{
   Q_OBJECT

      friend class US_Hydrodyn_Batch;
      friend class US_Hydrodyn;
      friend class US_Hydrodyn_Cluster_Submit;
      friend class US_Hydrodyn_Cluster_Status;
      friend class US_Hydrodyn_Cluster_Config;
      friend class US_Hydrodyn_Cluster_Advanced;
      friend class US_Hydrodyn_Cluster_Dmd;
      friend class US_Hydrodyn_Cluster_Config_Server;

   public:
      US_Hydrodyn_Cluster(
                          void *us_hydrodyn,
                          QWidget *p = 0, 
                          const char *name = 0
                          );
      ~US_Hydrodyn_Cluster();

    private:
      void          *us_hydrodyn;
      
      US_Config     *USglobal;

      QLabel        *lbl_title;

      QLabel        *lbl_target;
      QPushButton   *pb_add_target;
      QPushButton   *pb_clear_target;
      QListBox      *lb_target_files;

      QLabel        *lbl_no_of_jobs;
      QLineEdit     *le_no_of_jobs;

      QLabel        *lbl_output_name;
      QLineEdit     *le_output_name;

      QCheckBox     *cb_for_mpi;
      QPushButton   *pb_dmd;
      QPushButton   *pb_advanced;

      QPushButton   *pb_create_pkg;
      QPushButton   *pb_submit_pkg;
      QPushButton   *pb_check_status;
      QPushButton   *pb_load_results;

      QFont         ft;
      QTextEdit     *editor;
      QMenuBar      *m;

      QPushButton   *pb_help;
      QPushButton   *pb_cluster_config;
      QPushButton   *pb_cancel;
      
      QStringList   selected_files;

      US_Hydrodyn_Batch *batch_window;

      saxs_options  *our_saxs_options;

      void          editor_msg( QString color, QString msg );

      QString       pkg_dir;
      QString       submitted_dir;
      QString       completed_dir;
      QString       results_dir;

      bool          copy_files_to_pkg_dir( QStringList &filenames );
      bool          remove_files( QStringList &filenames );
      QString       errormsg;
      QString       noticemsg;

      // cluster config info

      bool          read_config();
      bool          write_config();
      QString       list_config();

      bool          dup_in_submitted_or_completed();

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif
      map < QString, QString >                  cluster_config;
      map < QString, map < QString, QString > > cluster_systems;
      map < QString, QString >                  cluster_stage_to_system;

      map < QString, bool > submitted_jobs;
      map < QString, bool > completed_jobs;
      map < QString, bool > results_jobs;
#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif
      csv           csv_advanced;
      bool          any_advanced();
      QString       advanced_addition( QString outputfile );
      QString       advanced_addition_methods();

      csv           csv_dmd;
      QString       dmd_base_addition( QStringList &base_source_files );
      QString       dmd_file_addition( QString inputfile, QString outputfile );

      bool          validate_csv_dmd( unsigned int &number_active );

      void          update_enables();
      bool          create_enabled;
      QString       options_summary();

   private slots:

      void setupGUI();
   
      void add_target();
      void clear_target();

      void update_output_name( const QString & );

      void create_pkg();
      void submit_pkg();
      void check_status();
      void load_results();

      void for_mpi();
      void dmd();
      void advanced();

      void clear_display();
      void update_font();
      void save();

      void cancel();
      void config();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
   
};

#endif
