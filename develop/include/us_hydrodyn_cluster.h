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

#include "us_util.h"

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

      QPushButton   *pb_create;
      QPushButton   *pb_create_pkg;

      QPushButton   *pb_help;
      QPushButton   *pb_cancel;
      
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
