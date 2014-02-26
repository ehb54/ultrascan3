#ifndef US_HYDRODYN_SAXS_IQQ_RESIDUALS_H
#define US_HYDRODYN_SAXS_IQQ_RESIDUALS_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qcolor.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <q3frame.h>
#include <qcheckbox.h>
#include <qwt_plot.h>
#include <qwt_counter.h>
#include <q3textedit.h>
#include <qpen.h>
//Added by qt3to4:
#include <QCloseEvent>

#include "us_util.h"

#ifdef QT4
#include "qwt_plot_grid.h"
#include "qwt_plot_curve.h"
#endif
#include "qwt/scrollbar.h"
#include "qwt/scrollzoomer.h"

//standard C and C++ defs:

#include <vector>

using namespace std;

class US_EXTERN US_Hydrodyn_Saxs_Iqq_Residuals : public Q3Frame
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
                                     double avg_std_dev_frac,
                                     vector < double > std_dev_frac,
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
# if !defined( QT4 )
  #pragma warning ( disable: 4251 )
# endif
#endif
      vector < vector < double > > qs;

      vector < vector < double > > differences;
      vector < vector < double > > differences_mult_avg_sd;
      vector < vector < double > > differences_mult_sd;
      vector < vector < double > > differences_no_errors;
      vector < vector < double > > log_differences;

      vector < vector < double > > targets;
      vector < vector < double > > log_targets;

      vector < vector < double > > difference_pcts;
      vector < vector < double > > differences_no_errors_pcts;
      vector < vector < double > > log_difference_pcts;

      vector < QColor > plot_colors;

      vector < double >            std_dev_frac;

#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( default: 4251 )
# endif
#endif
      double            avg_std_dev_frac;

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
      QCheckBox         *cb_plot_mult_avg_sd_frac;
      QCheckBox         *cb_plot_mult_sd_frac;

#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( disable: 4251 )
# endif
#endif

      QPushButton        *pb_help;
      QPushButton        *pb_cancel;

      void update_plot();

      unsigned int       pen_width;

   private slots:
      
      void setupGUI();

      void set_plot_log();
      void set_plot_difference();
      void set_plot_as_percent();
      void set_plot_mult_avg_sd_frac();
      void set_plot_mult_sd_frac();

      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
};

#endif
