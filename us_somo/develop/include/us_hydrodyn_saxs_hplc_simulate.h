#ifndef US_HYDRODYN_SAXS_HPLC_SIMULATE_H
#define US_HYDRODYN_SAXS_HPLC_SIMULATE_H

#include "qfile.h"
#include "qfiledialog.h"
#include "qfontmetrics.h"
#include "qlabel.h"
#include "qlayout.h"
#include "qlineedit.h"
#include "qprogressbar.h"
#include "qstring.h"
#include "qtextedit.h"
//Added by qt3to4:
#include <QCloseEvent>
#include <QFrame>

#include <map>
#include <set>
#include <vector>
#include "us_util.h"

using namespace std;

#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(disable : 4251)
#endif
#endif

struct uhshs_data {
      QLabel *lbl_name;

      QLabel *lbl_i_mult;
      QLineEdit *le_i_mult;

      QLabel *lbl_center;
      QLineEdit *le_center;

      QLabel *lbl_width;
      QLineEdit *le_width;

      QCheckBox *cb_alpha;
      QLineEdit *le_alpha;
};

class US_EXTERN US_Hydrodyn_Saxs_Hplc_Simulate : public QFrame {
      Q_OBJECT

   public:
      US_Hydrodyn_Saxs_Hplc_Simulate(
         void *us_hydrodyn, void *hplc_win, QStringList files, map<QString, vector<double>> q,
         map<QString, vector<double>> I, map<QString, vector<double>> e, QWidget *p = 0, const char *name = 0);

      ~US_Hydrodyn_Saxs_Hplc_Simulate();

   private:
      US_Config *USglobal;
      void *us_hydrodyn;
      void *hplc_win;

      QStringList files;
      map<QString, vector<double>> q;
      map<QString, vector<double>> I;
      map<QString, vector<double>> e;

      QLabel *lbl_title;

      QLabel *lbl_name;
      QLineEdit *le_name;

      QLabel *lbl_frames;
      QLineEdit *le_frames;

      QCheckBox *cb_noise;
      QLineEdit *le_noise;

      QCheckBox *cb_slope;
      QLineEdit *le_slope;

      QCheckBox *cb_alpha;
      QLineEdit *le_alpha;

      QCheckBox *cb_i_power;
      QLineEdit *le_i_power;

      QCheckBox *cb_gamma;
      QLineEdit *le_gamma;

      QCheckBox *cb_exponent;
      QLineEdit *le_exponent;

      vector<uhshs_data> data;

      QPushButton *pb_simulate;

      QPushButton *pb_help;
      QPushButton *pb_close;


      void setupGUI();
      void displayData();

   private slots:

      void update_enables();
      void simulate();

      void help();
      void cancel();

   protected slots:

      void closeEvent(QCloseEvent *);
};

#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(default : 4251)
#endif
#endif

#endif
