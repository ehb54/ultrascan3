#ifndef US_HYDRODYN_CLUSTER_RESULTS_H
#define US_HYDRODYN_CLUSTER_RESULTS_H

// QT defs:

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qstring.h>
//#include <q3frame.h>
#include <qcheckbox.h>
#include <qtreewidget.h>
//Added by qt3to4:
#include <QCloseEvent>

#include "us_hydrodyn_batch.h"
#include "us_hydrodyn_pdbdefs.h"
#include "us_util.h"

//standard C and C++ defs:

using namespace std;

class US_EXTERN US_Hydrodyn_Cluster_Results : public QDialog {
      Q_OBJECT
      friend class US_Hydrodyn_Cluster;

   public:
      US_Hydrodyn_Cluster_Results(void *us_hydrodyn, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_Cluster_Results();

   private:
      void *us_hydrodyn;

      US_Config *USglobal;

      QLabel *lbl_title;

      QLabel *lbl_files;
      QTreeWidget *lv_files;

      QPushButton *pb_select_all;
      QPushButton *pb_purge;
      QPushButton *pb_load_results;

      QFont ft;
      QTextEdit *editor;
      QMenuBar *m;

      QPushButton *pb_help;
      QPushButton *pb_cancel;

      QStringList files;

      void editor_msg(QString color, QString msg);

      QString pkg_dir;
      QString completed_dir;
      QString tmp_dir;
      QString results_dir;

      QString errormsg;
      bool disable_updates;

      unsigned int update_files(bool set_lv_files = true);

      bool clean_dir(QString dir);

      bool load_one_result(QString file);
      bool merge_csvs(QStringList &final_results);
      bool move_to_results(QString jobname, QStringList final_results);
      bool merge_this_csv(QString dest, vector<QString> csvs);

      bool are_any_selected(QTreeWidget *lv);

   private slots:

      void setupGUI();

      void update_enables();

      void select_all();
      void purge();
      void load_results();

      void clear_display();
      void update_font();
      void save();

      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
};

#endif
