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
#include "us_hydrodyn_cluster_submit.h"
#include "us_hydrodyn_cluster_status.h"
#include "us_hydrodyn_cluster_results.h"
#include "us_hydrodyn_cluster_config.h"

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
      QPushButton   *pb_set_target;
      QLineEdit     *le_target_file;

      QLabel        *lbl_no_of_jobs;
      QLineEdit     *le_no_of_jobs;

      QLabel        *lbl_output_name;
      QLineEdit     *le_output_name;

      QCheckBox     *cb_for_mpi;

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

      // cluster config info

      bool          read_config();
      bool          write_config();

      QString       cluster_id;
      QString       submit_url;
      QString       stage_url;

      bool          dup_in_submitted_or_completed();

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif
      map < QString, bool > submitted_jobs;
      map < QString, bool > completed_jobs;
      map < QString, bool > results_jobs;
#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

   private slots:

      void setupGUI();
   
      void set_target();

      void update_output_name( const QString & );

      void create_pkg();
      void submit_pkg();
      void check_status();
      void load_results();

      void for_mpi();

      void clear_display();
      void update_font();
      void save();

      void cancel();
      void cluster_config();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
   
};

#endif
