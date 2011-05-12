#ifndef US_HYDRODYN_SAXS_LOAD_CSV_H
#define US_HYDRODYN_SAXS_LOAD_CSV_H

// QT defs:

#include <qlabel.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qstringlist.h>
#include <qlistbox.h>
#include <qlineedit.h>

#include "us_util.h"

using namespace std;

class US_EXTERN US_Hydrodyn_Saxs_Load_Csv : public QDialog
{
   Q_OBJECT

   public:

      US_Hydrodyn_Saxs_Load_Csv(
                                QString msg,
                                QStringList *qsl_names,
                                QStringList *qsl_sel_names,
                                QStringList *qsl,
                                QString loaded_filename,
                                bool *create_avg,
                                bool *create_std_dev,
                                bool *only_plot_stats,
                                bool *save_to_csv,
                                QString *csv_filename,
                                bool *save_original_data,
                                bool *run_nnls,
                                QString *nnls_target,
                                bool expert_mode,
                                void *us_hydrodyn,
                                QWidget *p = 0, 
                                const char *name = 0
                                );
      ~US_Hydrodyn_Saxs_Load_Csv();

   private:

      QLabel *lbl_info;

      QListBox    *lb_names;

      QCheckBox   *cb_create_avg;
      QCheckBox   *cb_create_std_dev;
      QCheckBox   *cb_only_plot_stats;
      QCheckBox   *cb_save_to_csv;

      QLineEdit   *le_csv_filename;

      QCheckBox   *cb_save_original_data;

      QCheckBox   *cb_run_nnls;
      QLabel      *lbl_nnls_target;

      QPushButton *pb_select_all;
      QPushButton *pb_select_target;
      QPushButton *pb_transpose;
      QPushButton *pb_ok;
      QPushButton *pb_cancel;
      QPushButton *pb_help;

      void setupGUI();

      US_Config *USglobal;
      
      QString msg;
      QStringList *qsl_names;
      QStringList *qsl_sel_names;
      QStringList *qsl;
      QString loaded_filename;

      bool *create_avg;
      bool *create_std_dev;
      bool *only_plot_stats;
      bool *save_to_csv;
      QString *csv_filename;
      bool *save_original_data;
      bool *run_nnls;
      QString *nnls_target;
      bool expert_mode;

      void update_enables();

      void *us_hydrodyn;


   private slots:

      void update_selected();

      void set_create_avg();
      void set_create_std_dev();
      void set_only_plot_stats();
      void set_save_to_csv();
      void update_csv_filename(const QString &);
      void set_save_original_data();
      void set_run_nnls();

      void select_all();
      void select_target();
      void transpose();
      void ok();
      void cancel();
      void help();
};

#endif
