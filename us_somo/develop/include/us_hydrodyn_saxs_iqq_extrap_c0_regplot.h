#ifndef US_HYDRODYN_SAXS_IQQ_EXTRAP_C0_REGPLOT_H
#define US_HYDRODYN_SAXS_IQQ_EXTRAP_C0_REGPLOT_H

#include "qlabel.h"
#include "qstring.h"
#include "qpushbutton.h"
//Added by qt3to4:
#include <QFrame>
#include <QCloseEvent>

#include "us_util.h"
#include <map>
#include <vector>

#include <qwt_plot.h>
#include <qwt_wheel.h>
#include "us3i_plot.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_marker.h"
#include "qwt/scrollbar.h"
#include "qwt/scrollzoomer.h"

using namespace std;

// Pop-up viewer for the per-q linear regressions behind an extrapolation to zero
// concentration. One q-value is shown at a time: the (concentration, y) data points
// with error bars, the fitted line, and the c=0 intercept. A qwt wheel (plus
// prev/next) scrolls through the q grid.

class US_EXTERN US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot : public QFrame
{
   Q_OBJECT

   public:
      US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot(
                                              void *                          us_hydrodyn,
                                              QString                         y_axis_title,
                                              double                          merge_q,
                                              int                             fit_broaden,
                                              double                          gcv_edof,
                                              int                             model,
                                              vector < double >               reg_q,
                                              vector < vector < double > >    reg_x,
                                              vector < vector < double > >    reg_y,
                                              vector < vector < double > >    reg_e,
                                              vector < double >               reg_a,
                                              vector < double >               reg_b,
                                              vector < double >               reg_c,
                                              vector < double >               reg_siga,
                                              QWidget *                       p = 0,
                                              const char *                    name = 0
                                              );
      ~US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot();

   private:

      US_Config *                     USglobal;
      void *                          us_hydrodyn;

      QString                         y_axis_title;
      double                          merge_q;      // absolute-scale merging q (0 => pure extrapolation / Zimm)
      int                             fit_broaden;  // Zimm slope-smoothing q-window (0 => off)
      double                          gcv_edof;     // GCV effective slope dof (0 => GCV not used)
      int                             model;        // 0 additive (I/c), 1 reciprocal (c/I), 2 2nd-virial
      vector < double >               reg_q;
      vector < vector < double > >    reg_x;
      vector < vector < double > >    reg_y;
      vector < vector < double > >    reg_e;
      vector < double >               reg_a;
      vector < double >               reg_b;
      vector < double >               reg_c;        // quadratic coeff (2nd-virial), 0 for linear fits
      vector < double >               reg_siga;

      int                             cur_index;

      QLabel *                        lbl_info;
      QLabel *                        lbl_results;
      QwtWheel *                      qwtw_wheel;
      QPushButton *                   pb_prev;
      QPushButton *                   pb_next;
      QPushButton *                   pb_help;
      QPushButton *                   pb_close;

      mQwtPlot *                      plot;
      US_Plot *                       usp_plot;
      ScrollZoomer *                  plot_zoomer;
      QwtPlotGrid *                   grid;

      // transient plot items, rebuilt on every redraw
      vector < QwtPlotCurve * >       plot_curves;
      vector < QwtPlotMarker * >      plot_markers;

      void                            setupGUI();
      int                             clamp_index( int i );
      void                            set_index( int i );
      void                            update_plot();
      void                            clear_plot_items();

   private slots:

      void                            wheel_changed( double );
      void                            prev();
      void                            next();
      void                            help();
      void                            cancel();
      void                            usp_config_plot( const QPoint & );

   protected slots:

      void                            closeEvent( QCloseEvent * );
};

#endif
