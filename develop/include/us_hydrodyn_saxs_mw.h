#ifndef US_HYDRODYN_SAXS_MW_H
#define US_HYDRODYN_SAXS_MW_H

// QT defs:

#include <qlabel.h>
#include <qpushbutton.h>
#include <qdialog.h>

#include "us_util.h"

using namespace std;

// set mw for saxs normalized p(r) computations

class US_EXTERN US_Hydrodyn_Saxs_Mw : public QDialog
{
   Q_OBJECT

   public:

      US_Hydrodyn_Saxs_Mw(
                          QString msg,
                          float *mw,
                          bool *remember,
                          bool *use_partial,
                          QString *partial,
                          QWidget *p = 0, 
                          const char *name = 0
                          );
      ~US_Hydrodyn_Saxs_Mw();

   private:

      QLabel *lbl_info;
      QLabel *lbl_mw;

      QLineEdit *le_mw;
      QLineEdit *le_partial;

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

      void update_mw(const QString &);
      void update_partial(const QString &);

      void set_remember();
      void set_use_partial();

      void cancel();
      void help();
};

#endif
