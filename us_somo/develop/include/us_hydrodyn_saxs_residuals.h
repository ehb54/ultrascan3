#ifndef US_HYDRODYN_SAXS_RESIDUALS_H
#define US_HYDRODYN_SAXS_RESIDUALS_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
//#include <q3frame.h>
#include <qcheckbox.h>
#include <qwt_plot.h>
#include "us3i_plot.h"
#include <qwt_counter.h>
#include <qtextedit.h>
#include <qpen.h>
//Added by qt3to4:
#include <QCloseEvent>

#include "us_util.h"

#if QT_VERSION >= 0x040000
#include "qwt_plot_grid.h"
#include "qwt_plot_curve.h"
#include "qwt/scrollbar.h"
#include "qwt/scrollzoomer.h"
#else
#  include "qwt/scrollbar.h"
#  include "qwt/scrollzoomer.h"
#endif

//standard C and C++ defs:

#include <vector>

using namespace std;

class US_EXTERN US_Hydrodyn_Saxs_Residuals : public QFrame
{
   Q_OBJECT

      friend class US_Hydrodyn_Saxs;

   public:
      US_Hydrodyn_Saxs_Residuals(
                                 bool *saxs_residuals_widget,
                                 unsigned int width,
                                 QString title,
                                 vector < double > r,
                                 // vector < double > difference,
                                 vector < double > residuals,
                                 vector < double > target,
                                 vector < double > error,
                                 // bool plot_residuals,
                                 // bool plot_difference,
                                 // bool plot_as_percent,
                                 // bool use_errors,
                                 unsigned int pen_width,
                                 QWidget *p = 0, 
                                 const char *name = 0
                                 );
      ~US_Hydrodyn_Saxs_Residuals();

   private:

      QString title;
      bool *saxs_residuals_widget;

      vector < double > r;
      // vector < double > difference;
      vector < double > residuals;
      vector < double > target;
      vector < double > error;

      // vector < double > difference_pct;
      vector < double > residuals_pct;

      // vector < double > difference_div_sd;
      vector < double > residuals_div_sd;

      bool              plot_residuals;
      bool              plot_difference;
      bool              plot_as_percent;
      bool              use_errors;

      US_Config         *USglobal;

      QLabel            *lbl_title;

      mQwtPlot          *plot;
      US_Plot           *usp_plot;


   private slots:
      void usp_config_plot( const QPoint & );

   private:
      ScrollZoomer      *plot_zoomer;
      QwtPlotGrid       *grid;

      QCheckBox         *cb_plot_residuals;
      QCheckBox         *cb_plot_difference;
      QCheckBox         *cb_plot_as_percent;

      QPushButton        *pb_help;
      QPushButton        *pb_cancel;

      void               update_plot();
      unsigned int       pen_width;

   private slots:
      
      void setupGUI();

      void set_plot_residuals();
      void set_plot_difference();
      void set_plot_as_percent();

      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
};

#endif
