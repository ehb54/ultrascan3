#ifndef US_HYDRODYN_SAXS_HPLC_BASELINE_BEST_H
#define US_HYDRODYN_SAXS_HPLC_BASELINE_BEST_H

#include "qlabel.h"
#include "qstring.h"
#include "qlayout.h"
#include "qlineedit.h"
#include "qfontmetrics.h"
#include "qfile.h"
#include "qfiledialog.h"
#include "q3textedit.h"
#include "q3progressbar.h"
//Added by qt3to4:
#include <Q3Frame>
#include <QCloseEvent>

#include "us_util.h"
#include <map>
#include <set>
#include <vector>

#include <qwt_plot.h>

#ifdef QT4
#include "qwt_plot_grid.h"
#include "qwt_plot_curve.h"
#include "qwt/scrollbar.h"
#include "qwt/scrollzoomer.h"
#else
#  include "qwt/scrollbar.h"
#  include "qwt/scrollzoomer.h"
#endif

#include "us_mqt.h"

using namespace std;

#ifdef WIN32
# if !defined( QT4 )
     #pragma warning ( disable: 4251 )
# endif
#endif      

class US_EXTERN US_Hydrodyn_Saxs_Hplc_Baseline_Best : public Q3Frame
{
   Q_OBJECT

   public:
      US_Hydrodyn_Saxs_Hplc_Baseline_Best(
                                     void                         *          us_hydrodyn,
                                     map < QString, QString >                parameters,
                                     map < QString, double >                 dparameters,
                                     map < QString, vector < double > >      vdparameters,
                                     QWidget *                               p = 0,
                                     const char *                            name = 0
                                     );

      ~US_Hydrodyn_Saxs_Hplc_Baseline_Best();

   private:

      US_Config *                             USglobal;

      QLabel *                                lbl_title;

      QPushButton *                           pb_set_best;

      QPushButton *                           pb_help;
      QPushButton *                           pb_close;
      void                     *              us_hydrodyn;
      map < QString, QString >                parameters;
      map < QString, double >                 dparameters;
      map < QString, vector < double > >      vdparameters;

      void                                    setupGUI();
      void                                    displayData();

      QwtPlot       *plot;
      ScrollZoomer  *plot_zoomer;
#ifdef QT4
      QwtPlotGrid   *plot_grid;
#endif

      QFont         ft;
      Q3TextEdit     *editor;
      QMenuBar      *m;
      void          editor_msg( QString color, QString msg );
      void          editor_ec_msg( QString msg );

   private slots:

      void                                    clear_display();
      void                                    update_font();
      void                                    save();

      void                                    set_best();

      void                                    help();
      void                                    cancel();

   protected slots:

      void                                    closeEvent( QCloseEvent * );
};

#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( default: 4251 )
# endif
#endif

#endif
