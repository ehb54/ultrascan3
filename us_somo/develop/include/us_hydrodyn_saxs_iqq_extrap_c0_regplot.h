#ifndef US_HYDRODYN_SAXS_IQQ_EXTRAP_C0_REGPLOT_H
#define US_HYDRODYN_SAXS_IQQ_EXTRAP_C0_REGPLOT_H

#include "qlabel.h"
#include "qstring.h"
#include "qstringlist.h"
#include "qpushbutton.h"
//Added by qt3to4:
#include <QFrame>
#include <QCloseEvent>
#include <QCheckBox>
#include <QLineEdit>
#include <QRadioButton>
#include <QTimer>
#include <QSet>

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

// An extrapolated curve waiting in the per-q fit viewer: added to the SAS plot when the user keeps it
// (US_Hydrodyn_Saxs::extrap_c0_commit), dropped on Discard. Also carries what the CSV export needs.
struct US_Hydrodyn_Saxs_Extrap_C0_Pending
{
   QString           base_name;
   vector < double > q;
   vector < double > I;
   vector < double > I_err;
   double            conc_tag   = 1e0;
   double            psv        = 0e0;
   double            i0se       = 0e0;
   QString           hdr_extra;
   QString           msg_tail;
   bool              offer_save = false;
   QStringList       curve_names;               // dequoted; index = the reg_ci / reg_sci curve index
   vector < double > curve_concs;               // <= 0 => not in the fit
   QString           model_desc;
};

// Pop-up viewer for the per-q linear regressions behind an extrapolation to zero
// concentration. One q-value is shown at a time: the (concentration, y) data points
// with error bars, the fitted line, and the c=0 intercept. A qwt wheel (plus
// prev/next) scrolls through the q grid; the movie controls step through it on a
// timer and can save every frame shown as an image.

class US_EXTERN US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot : public QFrame
{
   Q_OBJECT

   public:
      US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot(
                                              void *                          us_hydrodyn,
                                              QString                         y_axis_title,
                                              double                          merge_q,
                                              int                             fit_broaden,
                                              int                             model,
                                              vector < double >               reg_q,
                                              vector < vector < double > >    reg_x,
                                              vector < vector < double > >    reg_y,
                                              vector < vector < double > >    reg_e,
                                              vector < double >               reg_a,
                                              vector < double >               reg_b,
                                              vector < double >               reg_c,
                                              vector < double >               reg_siga,
                                              double                          excl_conc,
                                              vector < double >               reg_excl_y,
                                              QString                         excl_name,
                                              vector < vector < int > >       reg_ci,
                                              vector < vector < double > >    reg_sx,
                                              vector < vector < double > >    reg_sy,
                                              vector < vector < double > >    reg_se,
                                              vector < vector < int > >       reg_sci,
                                              US_Hydrodyn_Saxs_Extrap_C0_Pending pending,
                                              void *                          saxs_window,
                                              QWidget *                       p = 0,
                                              const char *                    name = 0
                                              );
      ~US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot();

   private:

      US_Config *                     USglobal;
      void *                          us_hydrodyn;

      QString                         y_axis_title;
      double                          merge_q;      // absolute-scale merging q (0 => pure extrapolation / Zimm)
      int                             fit_broaden;  // additive slope-smoothing q-window (0 => off)
      int                             model;        // 0 additive (I/c), 1 reciprocal (c/I), 2 2nd-virial
      vector < double >               reg_q;
      vector < vector < double > >    reg_x;
      vector < vector < double > >    reg_y;
      vector < vector < double > >    reg_e;
      vector < double >               reg_a;
      vector < double >               reg_b;
      vector < double >               reg_c;        // quadratic coeff (2nd-virial), 0 for linear fits
      vector < double >               reg_siga;
      double                          excl_conc;    // QC-excluded curve's concentration (< 0 => none)
      vector < double >               reg_excl_y;   // its per-q y on the plot axis (NaN where none)
      QString                         excl_name;    // its (dequoted) name, for the info note
      vector < vector < int > >       reg_ci;       // curve index of each fitted point
      vector < vector < double > >    reg_sx;       // measured values of replaced spikes: conc, y,
      vector < vector < double > >    reg_sy;       //   y-error, curve index (drawn in magenta)
      vector < vector < double > >    reg_se;
      vector < vector < int > >       reg_sci;
      US_Hydrodyn_Saxs_Extrap_C0_Pending pending;
      bool                            pending_open; // the curve is still waiting for Keep / Discard
      void *                          saxs_window;  // US_Hydrodyn_Saxs that adds a kept curve

      int                             cur_index;

      QLabel *                        lbl_info;
      QLabel *                        lbl_results;
      QwtWheel *                      qwtw_wheel;
      QPushButton *                   pb_prev;
      QPushButton *                   pb_next;
      QPushButton *                   pb_help;
      QPushButton *                   pb_csv;
      QPushButton *                   pb_discard;
      QPushButton *                   pb_keep;

      // movie: steps through q on a timer, optionally saving each frame (plot + results line) as an
      // image, as the HPLC/KIN movie does; frames go to <inputs folder>/<prefix>_frames/
      QPushButton *                   pb_movie_front;
      QPushButton *                   pb_movie_slower;
      QPushButton *                   pb_movie_start;
      QPushButton *                   pb_movie_faster;
      QPushButton *                   pb_movie_end;
      QLabel *                        lbl_movie_state;
      QCheckBox *                     cb_save_frames;
      QLineEdit *                     le_save_prefix;
      QRadioButton *                  rb_save_png;
      QRadioButton *                  rb_save_jpeg;
      QRadioButton *                  rb_save_bmp;
      QCheckBox *                     cb_save_overwrite;
      QTimer *                        movie_timer;
      int                             movie_timer_ms;
      int                             n_frames_saved;
      int                             n_frames_kept;    // existing frame files not overwritten
      QString                         frames_dir_used;
      QSet < QString >                run_files;        // frame files met since the last report

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
      void                            after_redraw();
      void                            clear_plot_items();
      void                            fit_quality( int i, double & r2, double & chi2, double & red_chi2,
                                                   int & n_err, int & nparam );
      QString                         frames_dir();
      bool                            save_frame();
      void                            movie_stop();
      void                            report_frames();
      void                            update_movie_state();

   private slots:

      void                            wheel_changed( double );
      void                            prev();
      void                            next();
      void                            movie_front();
      void                            movie_slower();
      void                            movie_start();
      void                            movie_faster();
      void                            movie_end();
      void                            movie_tick();
      void                            set_save_frames();
      void                            help();
      void                            save_csv();
      void                            discard();
      void                            keep();
      void                            usp_config_plot( const QPoint & );

   protected slots:

      void                            closeEvent( QCloseEvent * );
};

#endif
