#ifndef US_HYDRODYN_SAXS_HPLC_BL_H
#define US_HYDRODYN_SAXS_HPLC_BL_H

#include "qlabel.h"
#include "qlayout.h"
#include "qstring.h"
#include "qvalidator.h"
#include "us_hydrodyn_saxs_hplc.h"
//Added by qt3to4:
#include <QCloseEvent>

using namespace std;

class US_EXTERN US_Hydrodyn_Saxs_Hplc_Bl : public QDialog {
      Q_OBJECT

   public:
      US_Hydrodyn_Saxs_Hplc_Bl(
         void *us_hydrodyn_saxs_hplc, map<QString, QString> *parameters, QWidget *p = 0, const char *name = 0);

      ~US_Hydrodyn_Saxs_Hplc_Bl();

   private:
      US_Config *USglobal;

      QLabel *lbl_title;

      QGroupBox *bg_bl_type;

      QRadioButton *rb_linear;
      QRadioButton *rb_integral;

      QLabel *lbl_smooth;
      QLineEdit *le_smooth;

      QLabel *lbl_reps;
      QLineEdit *le_reps;

      QCheckBox *cb_save_bl;

      QPushButton *pb_help;
      QPushButton *pb_quit;
      QPushButton *pb_go;

      void *us_hydrodyn_saxs_hplc;
      map<QString, QString> *parameters;

      void setupGUI();

   private slots:

      void go();
      void quit();
      void help();

      void update_enables();

   protected slots:

      void closeEvent(QCloseEvent *);
};

#endif
