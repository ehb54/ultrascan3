#ifndef US_HYDRODYN_DAD_BASELINE_BEST_H
#define US_HYDRODYN_DAD_BASELINE_BEST_H

#include "qlabel.h"
#include "qstring.h"
#include "qlayout.h"
#include "qlineedit.h"
#include "qfontmetrics.h"
#include "qfile.h"
#include "qfiledialog.h"
#include "qtextedit.h"
#include "qprogressbar.h"
#include "qsplitter.h"
//Added by qt3to4:
#include <QFrame>
#include <QCloseEvent>

#include "us_util.h"
#include <map>
#include <set>
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

#include "us_mqt.h"

using namespace std;

#ifdef WIN32
# if QT_VERSION < 0x040000
     #pragma warning ( disable: 4251 )
# endif
#endif      

class US_EXTERN US_Hydrodyn_Dad_Baseline_Best : public QFrame
{
   Q_OBJECT

   public:
      US_Hydrodyn_Dad_Baseline_Best(
                                     void                         *          us_hydrodyn,
                                     map < QString, QString >                parameters,
                                     map < QString, double >                 dparameters,
                                     map < QString, vector < double > >      vdparameters,
                                     QWidget *                               p = 0,
                                     const char *                            name = 0
                                     );

      ~US_Hydrodyn_Dad_Baseline_Best();

   private:

      US_Config *                             USglobal;

      QSplitter *                             qs_left;
      QSplitter *                             qs_right;

      QLabel *                                lbl_title;
      QLabel *                                lbl_hb_title;

      QPushButton *                           pb_set_best;
      QPushButton *                           pb_set_hb_best;

      QCheckBox *                             cb_show_hb;

      QPushButton *                           pb_help;
      QPushButton *                           pb_close;

      void                     *              us_hydrodyn;
      map < QString, QString >                parameters;
      map < QString, double >                 dparameters;
      map < QString, vector < double > >      vdparameters;

      void                                    setupGUI();
      void                                    displayData();

      mQwtPlot      *plot;
      US_Plot       *usp_plot;
   private slots:
      void usp_config_plot( const QPoint & );

   private:
      ScrollZoomer  *plot_zoomer;
#if QT_VERSION >= 0x040000
      QwtPlotGrid   *plot_grid;
#endif

      mQwtPlot      *hb_plot;
      US_Plot       *usp_hb_plot;
   private slots:
      void usp_config_hb_plot( const QPoint & );

   private:
      ScrollZoomer  *hb_plot_zoomer;
#if QT_VERSION >= 0x040000
      QwtPlotGrid   *hb_plot_grid;
#endif

      QFont         ft;
      QTextEdit     *editor;
      // QMenuBar      *m;
      void          editor_msg( QString color, QString msg, QTextEdit *e = (QTextEdit *)0 );
      void          editor_ec_msg( QString msg, QTextEdit *e = (QTextEdit *)0 );

      QTextEdit     *hb_editor;
      // QMenuBar      *hb_m;

      void          editor_hb_msg( QString color, QString msg );
      void          editor_hb_ec_msg( QString msg );

      QString                                 plot_name;
      QString                                 hb_plot_name;

   private slots:

      void                                    clear_display();
      void                                    update_font();
      void                                    save();

      void                                    update_enables();

      void                                    set_best();
      void                                    set_hb_best();

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
