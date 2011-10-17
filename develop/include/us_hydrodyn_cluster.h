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

#include "us_util.h"
#include "us_hydrodyn_pdbdefs.h"
#include "us_hydrodyn_batch.h"

//standard C and C++ defs:

using namespace std;

class US_EXTERN US_Hydrodyn_Cluster : public QDialog
{
   Q_OBJECT

      friend class US_Hydrodyn_Batch;
      friend class US_Hydrodyn;

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

      QLabel        *lbl_jobs_per;
      QLineEdit     *le_jobs_per;

      QLabel        *lbl_output_name;
      QLineEdit     *le_output_name;

      QPushButton   *pb_create;
      QPushButton   *pb_create_pkg;

      QPushButton   *pb_help;
      QPushButton   *pb_cancel;
      
      QStringList   selected_files;

      US_Hydrodyn_Batch *batch_window;

      saxs_options  *our_saxs_options;

   private slots:

      void setupGUI();
   
      void set_target();
      void create();
      void create_pkg();

      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
   
};

#endif
