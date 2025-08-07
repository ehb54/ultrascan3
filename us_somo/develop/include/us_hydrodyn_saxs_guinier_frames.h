#ifndef US_HYDRODYN_SAXS_GUINIER_FRAMES_H
#define US_HYDRODYN_SAXS_GUINIER_FRAMES_H

#include "qfile.h"
#include "qfiledialog.h"
#include "qfontmetrics.h"
#include "qlabel.h"
#include "qlayout.h"
#include "qlineedit.h"
#include "qstring.h"
//Added by qt3to4:
#include <QCloseEvent>
#include <QFrame>

#include <map>
#include <vector>
#include "us_util.h"

#include <qwt_plot.h>
#include "us3i_plot.h"

#if QT_VERSION >= 0x040000
#include "qwt/scrollbar.h"
#include "qwt/scrollzoomer.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_grid.h"
#else
#include "qwt/scrollbar.h"
#include "qwt/scrollzoomer.h"
#endif

using namespace std;

#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(disable : 4251)
#endif
#endif

class US_EXTERN US_Hydrodyn_Saxs_Guinier_Frames : public QFrame {
      Q_OBJECT

   public:
      US_Hydrodyn_Saxs_Guinier_Frames(
         void *us_hydrodyn, map<QString, QString> parameters, map<QString, vector<vector<double>>> plots,
         QWidget *p = 0, const char *name = 0);

      ~US_Hydrodyn_Saxs_Guinier_Frames();

   private:
      US_Config *USglobal;

      // QLabel *                                lbl_title;
      // QLabel *                                lbl_credits_1;

      QPushButton *pb_reffile;
      QLineEdit *le_reffile;

      QCheckBox *cb_yright1;
      QCheckBox *cb_yright2;

      QLabel *lbl_xbottom;
      QLineEdit *le_xbottom_min;
      QLineEdit *le_xbottom_max;

      QLabel *lbl_yleft;
      QLineEdit *le_yleft_min;
      QLineEdit *le_yleft_max;

      QLabel *lbl_yright;
      QLineEdit *le_yright_min;
      QLineEdit *le_yright_max;

      QPushButton *pb_help;
      QPushButton *pb_close;
      void *us_hydrodyn;
      map<QString, QString> parameters;
      map<QString, vector<vector<double>>> plots;

      mQwtPlot *plot;
      US_Plot *usp_plot;
   private slots:
      void usp_config_plot(const QPoint &);

   private:
      ScrollZoomer *plot_zoomer;
#if QT_VERSION >= 0x040000
      QwtPlotGrid *grid;
#endif

      void setupGUI();

      void update_plot();

      vector<double> ref_t;
      vector<double> ref_I;
      vector<double> ref_e;

   private slots:

      void set_yright1();
      void set_yright2();

      void set_reffile();
      void update_reffile(const QString &);

      void update_xbottom_min(const QString &);
      void update_xbottom_max(const QString &);
      void update_yleft_min(const QString &);
      void update_yleft_max(const QString &);
      void update_yright_min(const QString &);
      void update_yright_max(const QString &);

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
