#ifndef US_HYDRODYN_SAXS_RESIDUALS_H
#define US_HYDRODYN_SAXS_RESIDUALS_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
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
#include "qwt/scrollbar.h"
#include "qwt/scrollzoomer.h"
#else
#  include "qwt/scrollbar.h"
#  include "qwt/scrollzoomer.h"
#endif

//standard C and C++ defs:

#include <vector>

using namespace std;

class US_EXTERN US_Hydrodyn_Saxs_Residuals : public Q3Frame
{
   Q_OBJECT

   public:
      US_Hydrodyn_Saxs_Residuals(
                                 bool *saxs_residuals_widget,
                                 unsigned int width,
                                 QString title,
                                 vector < double > r,
                                 vector < double > difference,
                                 vector < double > residuals,
                                 vector < double > target,
                                 bool plot_residuals,
                                 bool plot_difference,
                                 bool plot_as_percent,
                                 QWidget *p = 0, 
                                 const char *name = 0
                                 );
      ~US_Hydrodyn_Saxs_Residuals();

   private:

      QString title;
      bool *saxs_residuals_widget;

#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( disable: 4251 )
# endif
#endif
      vector < double > r;
      vector < double > difference;
      vector < double > residuals;
      vector < double > target;

      vector < double > difference_pct;
      vector < double > residuals_pct;

#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( default: 4251 )
# endif
#endif

      bool              plot_residuals;
      bool              plot_difference;
      bool              plot_as_percent;

      US_Config         *USglobal;

      QLabel            *lbl_title;

      QwtPlot           *plot;
      ScrollZoomer      *plot_zoomer;
#ifdef QT4
      QwtPlotGrid       *grid;
#endif

      QCheckBox         *cb_plot_residuals;
      QCheckBox         *cb_plot_difference;
      QCheckBox         *cb_plot_as_percent;

#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( disable: 4251 )
# endif
#endif

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
