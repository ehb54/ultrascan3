#ifndef US_HYDRODYN_CLUSTER_RESULTS_H
#define US_HYDRODYN_CLUSTER_RESULTS_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qlistview.h>

#include "us_util.h"
#include "us_hydrodyn_pdbdefs.h"
#include "us_hydrodyn_batch.h"

//standard C and C++ defs:

using namespace std;

class US_EXTERN US_Hydrodyn_Cluster_Results : public QDialog
{
   Q_OBJECT
      friend class US_Hydrodyn_Cluster;

   public:
     US_Hydrodyn_Cluster_Results(
                               void *us_hydrodyn,
                               QWidget *p = 0, 
                               const char *name = 0
                               );
      ~US_Hydrodyn_Cluster_Results();
      
    private:
      void          *us_hydrodyn;
      
      US_Config     *USglobal;

      QLabel        *lbl_title;

      QLabel        *lbl_files;
      QListView     *lv_files;

      QPushButton   *pb_select_all;
      QPushButton   *pb_purge;
      QPushButton   *pb_load_results;

      QFont         ft;
      QTextEdit     *editor;
      QMenuBar      *m;

      QPushButton   *pb_help;
      QPushButton   *pb_cancel;
      
      QStringList   files;

      void          editor_msg( QString color, QString msg );

      QString       pkg_dir;
      QString       completed_dir;

      QString       errormsg;
      bool          disable_updates;

      unsigned int  update_files( bool set_lv_files = true );

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
