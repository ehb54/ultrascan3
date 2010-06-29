#ifndef US_HYDRODYN_DAMMIN_OPTS_H
#define US_HYDRODYN_DAMMIN_OPTS_H

// QT defs:

#include <qlabel.h>
#include <qpushbutton.h>
#include <qdialog.h>

#include "us_util.h"

using namespace std;

// set psv & mw for DAMMIN/DAMMIF file loads

class US_EXTERN US_Hydrodyn_Dammin_Opts : public QDialog
{
   Q_OBJECT

   public:

      US_Hydrodyn_Dammin_Opts(
                              QString msg,
                              float *psv,
                              float *mw,
                              bool *write_bead_model,
                              QWidget *p = 0, 
                              const char *name = 0
                              );
      ~US_Hydrodyn_Dammin_Opts();

   private:

      QLabel *lbl_info;
      QLabel *lbl_psv;
      QLabel *lbl_mw;

      QLineEdit *le_psv;
      QLineEdit *le_mw;

      QCheckBox *cb_write_bead_model;

      QPushButton *pb_cancel;
      QPushButton *pb_help;

      void setupGUI();

      US_Config *USglobal;
      
      QString msg;
      float *psv;
      float *mw;
      bool *write_bead_model;

   public slots:

   private slots:

      void update_psv(const QString &);
      void update_mw(const QString &);

      void set_write_bead_model();

      void cancel();
      void help();
};

#endif
