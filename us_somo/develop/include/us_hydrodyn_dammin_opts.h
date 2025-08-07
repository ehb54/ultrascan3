#ifndef US_HYDRODYN_DAMMIN_OPTS_H
#define US_HYDRODYN_DAMMIN_OPTS_H

// QT defs:

#include <qdialog.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include "us_util.h"

using namespace std;

// set psv & mw for DAMMIN/DAMMIF file loads

class US_EXTERN US_Hydrodyn_Dammin_Opts : public QDialog {
      Q_OBJECT

   public:
      US_Hydrodyn_Dammin_Opts(
         QString msg, float *psv, float *mw, bool *write_bead_model, bool *remember, bool *use_partial,
         QString *partial, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_Dammin_Opts();

   private:
      QLabel *lbl_info;
      QLabel *lbl_psv;
      QLabel *lbl_mw;

      QLineEdit *le_psv;
      QLineEdit *le_mw;
      QLineEdit *le_partial;

      QCheckBox *cb_write_bead_model;
      QCheckBox *cb_remember;
      QCheckBox *cb_use_partial;

      QPushButton *pb_cancel;
      QPushButton *pb_help;

      void setupGUI();

      US_Config *USglobal;

      QString msg;
      float *psv;
      float *mw;
      bool *write_bead_model;
      bool *remember;
      bool *use_partial;
      QString *partial;

   public slots:

   private slots:

      void update_psv(const QString &);
      void update_mw(const QString &);
      void update_partial(const QString &);

      void set_write_bead_model();
      void set_remember();
      void set_use_partial();

      void cancel();
      void help();
};

#endif
