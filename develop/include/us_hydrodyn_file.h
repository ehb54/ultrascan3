#ifndef US_HYDRODYN_FILE_H
#define US_HYDRODYN_FILE_H

// QT defs:

#include <qlabel.h>
#include <qpushbutton.h>
#include <qdialog.h>

#include "us_util.h"

using namespace std;

class US_EXTERN US_Hydrodyn_File : public QDialog
{
   Q_OBJECT

   public:

      US_Hydrodyn_File(QString *dir,
                       QString *base,
                       QString *ext,
                       QWidget *p = 0, 
                       const char *name = 0);
      ~US_Hydrodyn_File();

   private:

      QString *dir;
      QString *base;
      QString *ext;

      QLabel *lbl_info;
      QLabel *lbl_info2;

      QLineEdit *le_dir;
      QLineEdit *le_base;
      QLineEdit *le_ext;

      QPushButton *pb_overwrite;
      QPushButton *pb_auto_inc;
      QPushButton *pb_try_again;
      QPushButton *pb_help;

      void setupGUI();

      US_Config *USglobal;

   public slots:

   private slots:

      void overwrite();
      void auto_inc();
      void try_again();
      void help();
};

#endif
