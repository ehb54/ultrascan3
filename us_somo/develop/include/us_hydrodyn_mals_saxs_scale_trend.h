#ifndef US_HYDRODYN_MALS_SAXS_SCALE_TREND_H
#define US_HYDRODYN_MALS_SAXS_SCALE_TREND_H

#include "qlabel.h"
#include "qstring.h"
#include "qlayout.h"
#include "qlineedit.h"
#include "qfontmetrics.h"
#include "qfile.h"
#include "qfiledialog.h"
//Added by qt3to4:
#include <QFrame>
#include <QCloseEvent>

#include "us_util.h"
#include <map>
#include <vector>

#include <qwt_plot.h>
#include "us3i_plot.h"

#if QT_VERSION >= 0x040000
#include "qwt_plot_grid.h"
#include "qwt_plot_curve.h"
#include "qwt/scrollbar.h"
#include "qwt/scrollzoomer.h"
#else
#  include "qwt/scrollbar.h"
#  include "qwt/scrollzoomer.h"
#endif

using namespace std;

#ifdef WIN32
# if QT_VERSION < 0x040000
     #pragma warning ( disable: 4251 )
# endif
#endif      

class US_EXTERN US_Hydrodyn_Mals_Saxs_Scale_Trend : public QFrame
{
   Q_OBJECT

   public:
      US_Hydrodyn_Mals_Saxs_Scale_Trend(
                                         void                     *              us_hydrodyn,
                                         map < QString, QString >                parameters,
                                         map < QString, vector < vector < double > > > plots,
                                         QWidget *                               p = 0,
                                         const char *                            name = 0
                                         );

      ~US_Hydrodyn_Mals_Saxs_Scale_Trend();

   private:

      US_Config *                             USglobal;

      // QLabel *                                lbl_title;
      // QLabel *                                lbl_credits_1;

      QPushButton *                           pb_help;
      QPushButton *                           pb_close;
      void                     *              us_hydrodyn;
      map < QString, QString >                parameters;
      map < QString, vector < vector < double > > > plots;

      mQwtPlot *                              plot;
      US_Plot *                               usp_plot;
   private slots:
      void usp_config_plot( const QPoint & );

   private:
      ScrollZoomer *                          plot_zoomer;
#if QT_VERSION >= 0x040000
      QwtPlotGrid *                           grid;
#endif

      QLabel *                                lbl_results;

      void                                    setupGUI();

      void                                    analysis();

      void                                    update_plot();

      vector < double >                       slopes_x;
      vector < double >                       slopes;
      vector < double >                       slopes_e;

      double                                  fit_a;
      double                                  fit_b;
      double                                  fit_siga;
      double                                  fit_sigb;
      double                                  fit_chi2;

      bool                                    use_qmin;
      bool                                    use_qmax;
      double                                  qmin;
      double                                  qmax;

   private slots:

      void                                    help();
      void                                    cancel();

   protected slots:

      void                                    closeEvent( QCloseEvent * );
};

#ifdef WIN32
# if QT_VERSION < 0x040000
  #pragma warning ( default: 4251 )
# endif
#endif

#endif
