#ifndef US_HYDRODYN_SAXS_IQQ_RESIDUALS_H
#define US_HYDRODYN_SAXS_IQQ_RESIDUALS_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qcolor.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qwt_plot.h>
#include <qwt_counter.h>
#include <qtextedit.h>
#include <qpen.h>

#include "us_util.h"

#ifdef QT4
# include "qwt_plot_grid.h"
# include "qwt_plot_curve.h"
#endif
#include "qwt/scrollbar.h"
#include "qwt/scrollzoomer.h"

//standard C and C++ defs:

#include <vector>

using namespace std;

class US_EXTERN US_Hydrodyn_Saxs_Iqq_Residuals : public QFrame
{
   Q_OBJECT

      friend class US_Hydrodyn_Saxs;

   public:
      US_Hydrodyn_Saxs_Iqq_Residuals(
                                     bool *saxs_iqq_residuals_widget,
                                     unsigned int width,
                                     QString title,
                                     vector < double > q,
                                     vector < double > difference,
                                     vector < double > difference_no_errors,
                                     vector < double > target,
                                     vector < double > log_difference,
                                     vector < double > log_target,
                                     QColor plot_color,
                                     bool use_errors,
                                     bool plot_log,
                                     bool plot_difference,
                                     bool plot_as_percent,
                                     QWidget *p = 0, 
                                     const char *name = 0
                                     );
      ~US_Hydrodyn_Saxs_Iqq_Residuals();

      void add(
               unsigned int width,
               vector < double > q,
               vector < double > difference,
               vector < double > difference_no_errors,
               vector < double > target,
               vector < double > log_difference,
               vector < double > log_target,
               QColor plot_color
               );

   private:

      QString title;
      bool *saxs_iqq_residuals_widget;

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif
      vector < vector < double > > qs;

      vector < vector < double > > differences;
      vector < vector < double > > differences_no_errors;
      vector < vector < double > > log_differences;

      vector < vector < double > > targets;
      vector < vector < double > > log_targets;

      vector < vector < double > > difference_pcts;
      vector < vector < double > > differences_no_errors_pcts;
      vector < vector < double > > log_difference_pcts;

      vector < QColor > plot_colors;

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

      bool              use_errors;

      bool              plot_log;
      bool              plot_difference;
      bool              plot_as_percent;

      bool              detached;

      US_Config         *USglobal;

      QLabel            *lbl_title;

      QwtPlot           *plot;
      ScrollZoomer      *plot_zoomer;
#ifdef QT4
      QwtPlotGrid       *grid;
#endif

      QCheckBox         *cb_plot_log;
      QCheckBox         *cb_plot_difference;
      QCheckBox         *cb_plot_as_percent;

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

      QPushButton        *pb_help;
      QPushButton        *pb_cancel;

      void               update_plot();

   private slots:
      
      void setupGUI();

      void set_plot_log();
      void set_plot_difference();
      void set_plot_as_percent();

      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
};

#endif
