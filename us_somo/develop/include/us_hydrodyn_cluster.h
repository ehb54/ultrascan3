#ifndef US_HYDRODYN_CLUSTER_H
#define US_HYDRODYN_CLUSTER_H

// QT defs:

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qstring.h>
//#include <q3frame.h>
#include <qcheckbox.h>
#include <qtcpsocket.h>
#include <qtextedit.h>
#include <qtimer.h>
#include <qvalidator.h>
//Added by qt3to4:
#include <QCloseEvent>

#include "us_file_util.h"
#include "us_hydrodyn_batch.h"
#include "us_hydrodyn_cluster_additional.h"
#include "us_hydrodyn_cluster_advanced.h"
#include "us_hydrodyn_cluster_config.h"
#include "us_hydrodyn_cluster_config_server.h"
#include "us_hydrodyn_cluster_dmd.h"
#include "us_hydrodyn_cluster_results.h"
#include "us_hydrodyn_cluster_status.h"
#include "us_hydrodyn_cluster_submit.h"
#include "us_hydrodyn_comparative.h"
#include "us_hydrodyn_pdbdefs.h"
#include "us_util.h"

//standard C and C++ defs:

#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(disable : 4251)
#endif
#endif

using namespace std;

class US_EXTERN US_Hydrodyn_Cluster : public QDialog {
      Q_OBJECT

      friend class US_Hydrodyn_Batch;
      friend class US_Hydrodyn;
      friend class US_Hydrodyn_Cluster_Submit;
      friend class US_Hydrodyn_Cluster_Status;
      friend class US_Hydrodyn_Cluster_Config;
      friend class US_Hydrodyn_Cluster_Advanced;
      friend class US_Hydrodyn_Cluster_Dmd;
      friend class US_Hydrodyn_Cluster_Config_Server;
      friend class US_Hydrodyn_Cluster_Additional;

   public:
      US_Hydrodyn_Cluster(void *us_hydrodyn, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_Cluster();

   private:
      void *us_hydrodyn;

      US_Config *USglobal;

      QLabel *lbl_title;

      QLabel *lbl_target;
      QPushButton *pb_add_target;
      QPushButton *pb_clear_target;
      QListWidget *lb_target_files;

      QLabel *lbl_no_of_jobs;
      QLineEdit *le_no_of_jobs;
      QValidator *le_no_of_jobs_qv;

      QLabel *lbl_output_name;
      QLineEdit *le_output_name;

      QCheckBox *cb_for_mpi;
      QCheckBox *cb_split_grid;
      QPushButton *pb_dmd;
      QPushButton *pb_additional;
      QPushButton *pb_advanced;

      QPushButton *pb_create_pkg;
      QPushButton *pb_submit_pkg;
      QPushButton *pb_check_status;
      QPushButton *pb_load_results;

      QFont ft;
      QTextEdit *editor;
      QMenuBar *m;

      QPushButton *pb_help;
      QPushButton *pb_cluster_config;
      QPushButton *pb_cancel;

      QStringList selected_files;

      US_Hydrodyn_Batch *batch_window;

      saxs_options *our_saxs_options;

      void editor_msg(QString color, QString msg);

      QString pkg_dir;
      QString submitted_dir;
      QString completed_dir;
      QString results_dir;

      bool copy_files_to_pkg_dir(QStringList &filenames);
      bool remove_files(QStringList &filenames);
      QString errormsg;
      QString noticemsg;

      // cluster config info

      bool read_config();
      bool corrupt_config();
      bool write_config();
      QString list_config();

      bool dup_in_submitted_or_completed();

      map<QString, QString> cluster_config;
      map<QString, map<QString, QString>> cluster_systems;
      map<QString, QString> cluster_stage_to_system;

      map<QString, bool> submitted_jobs;
      map<QString, bool> completed_jobs;
      map<QString, bool> results_jobs;

      map<QString, bool> *cluster_additional_methods_options_active;
      map<QString, map<QString, QString>> *cluster_additional_methods_options_selected;

      map<QString, bool> cluster_additional_methods_use_experimental_data;
      map<QString, bool> cluster_additional_methods_require_experimental_data;
      map<QString, bool> cluster_additional_methods_require_sleep;
      map<QString, bool> cluster_additional_methods_parallel_mpi;
      map<QString, bool> cluster_additional_methods_one_pdb_exactly;
      map<QString, bool> cluster_additional_methods_no_tgz_output;
      map<QString, bool> cluster_additional_methods_must_run_alone;
      map<QString, QString> cluster_additional_methods_prepend;
      map<QString, QString> cluster_additional_methods_add_selected_files;
      map<QString, unsigned int> cluster_additional_methods_job_multiplier;
      map<QString, map<QString, bool>> cluster_additional_methods_modes;

      csv csv_advanced;
      bool any_advanced();
      QStringList advanced_addition();
      QString advanced_addition_methods();

      csv csv_dmd;
      QString dmd_base_addition(QStringList &base_source_files, QString common_prefix);
      QString dmd_file_addition(QString inputfile, QString outputfile, int use_entry = -1);
      unsigned int dmd_entry_count(QString inputfile); // count entries for this file

      bool validate_csv_dmd(unsigned int &number_active);

      void update_enables();
      bool create_enabled;
      QString options_summary();

      void update_validator();

      QStringList active_additional_methods();
      bool active_additional_experimental_data();
      bool active_additional_mpi_mix_issue();
      bool active_additional_prepend_issue();
      QString job_prepend_name();

      QStringList additional_method_files(QString method);
      QString additional_method_package_text(QString method);

      void create_additional_methods_pkg(QString base_dir, QString filename, QString common_prefix, bool use_extension);

      void create_additional_methods_parallel_pkg(
         QString base_dir, QString filename, QString common_prefix, bool use_extension);

      void create_additional_methods_parallel_pkg_bfnb(QString filename);

      bool additional_processing(
         QString &out, QStringList &source_files, const QString &type, const QString &method, const QString &file = "");

   private slots:

      void setupGUI();

      void add_target();
      void clear_target();

      void update_output_name(const QString &);

      void create_pkg();
      void submit_pkg();
      void check_status();
      void load_results();

      void for_mpi();
      void split_grid();
      void dmd();
      void additional();
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

#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(default : 4251)
#endif
#endif
#endif
