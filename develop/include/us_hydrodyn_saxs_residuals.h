#ifndef US_HYDRODYN_SAXS_RESIDUALS_H
#define US_HYDRODYN_SAXS_RESIDUALS_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qwt_plot.h>
#include <qwt_counter.h>
#include <qtextedit.h>

#include "us_util.h"

//standard C and C++ defs:

#include <vector>

using namespace std;

class US_EXTERN US_Hydrodyn_Saxs_Residuals : public QFrame
{
   Q_OBJECT

   public:
      US_Hydrodyn_Saxs_Residuals(
                                 bool *saxs_residuals_widget,
                                 QString title,
                                 vector < double > r,
                                 vector < double > difference,
                                 vector < double > residuals,
                                 bool plot_residuals,
                                 bool plot_difference,
                                 QWidget *p = 0, 
                                 const char *name = 0
                                 );
      ~US_Hydrodyn_Saxs_Residuals();

   private:

      QString title;
      bool *saxs_residuals_widget;

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif
      vector < double > r;
      vector < double > difference;
      vector < double > residuals;
#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

      bool              plot_residuals;
      bool              plot_difference;

      US_Config         *USglobal;

      QLabel            *lbl_title;

      QwtPlot           *plot;

      QCheckBox         *cb_plot_residuals;
      QCheckBox         *cb_plot_difference;

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

      QPushButton        *pb_help;
      QPushButton        *pb_cancel;

      void               update_plot();

   private slots:
      
      void setupGUI();

      void set_plot_residuals();
      void set_plot_difference();

      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
};

#endif
