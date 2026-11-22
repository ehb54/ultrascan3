#ifndef US_HYDRODYN_SAXS_BUFFER_FRAMES_H
#define US_HYDRODYN_SAXS_BUFFER_FRAMES_H

#include "us_hydrodyn_saxs_buffer.h"
#include "us3i_plot.h"
#include "us_mqt.h"
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QCloseEvent>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_grid.h>
#include <qwt_wheel.h>

#include <vector>

using namespace std;

// SAS Data Utility "Buffer frames", for a SEC-SAXS run: the I(t) of the selected I(q) frames (each frame's mean
// intensity over a q range, against its frame number) with one or two regions set by dashed markers, the buffer
// frames before the elution and, optionally, after it. For each region it gives the drift of I(t) across it and
// how well each frame agrees with the mean of the region's other frames over its whole curve (reduced chi^2);
// with both regions, whether their average curves agree. It can suggest the regions, and selects the frames
// chosen in the Data Utility's file list. Nothing is added to the Data Utility.

class US_EXTERN US_Hydrodyn_Saxs_Buffer_Frames : public QFrame
{
   Q_OBJECT

   public:
      US_Hydrodyn_Saxs_Buffer_Frames( US_Hydrodyn_Saxs_Buffer * buffer, QStringList files, QWidget * p = 0 );
      ~US_Hydrodyn_Saxs_Buffer_Frames();

      struct region_stats
      {
         bool              ok          = false;
         int               i0          = 0;      // first and last frame, as indices into files
         int               i1          = 0;
         int               n           = 0;
         double            mean        = 0e0;    // mean of I(t)
         double            drift       = 0e0;    // change of I(t) across the region from its fitted slope
         double            drift_sd    = 0e0;
         double            line_a      = 0e0;    // the fitted line, I(t) = line_a + line_b ( frame - frame[ i0 ] )
         double            line_b      = 0e0;
         double            it_chi2r    = 0e0;    // reduced chi^2 of I(t) about the fitted line (with sd)
         bool              have_chi2   = false;  // the per-frame chi^2 below were computed
         vector < double > frame_chi2r;          // each frame against the mean of the others, over the whole curve
         int               n_q         = 0;      // points in each of those chi^2
         double            chi2_limit  = 0e0;    // upper 0.1% point of a reduced chi^2 of n_q points
         vector < int >    outliers;             // indices into files of the frames above the limit
         vector < double > avg;                  // the region's average curve and its variance (1/sd^2 weighted)
         vector < double > avg_var;
      };

   private:
      US_Hydrodyn_Saxs_Buffer *       buffer;
      US_Config *                     USglobal;
      QStringList                     files;          // in frame order
      vector < double >               frame;          // frame numbers, or positions 1..n when not numbered
      QStringList                     frame_text;     // as written in the names
      bool                            numbered;
      bool                            have_sd;        // every frame has sd > 0 at every point
      bool                            same_grid;      // all frames on one q grid
      vector < double >               It;
      vector < double >               It_sd;
      bool                            It_ok;          // every frame has points in the q range
      QString                         note;           // the outcome of the last suggestion
      region_stats                    r1;
      region_stats                    r2;

      QLabel *                        lbl_info;
      QLabel *                        lbl_q;
      QLineEdit *                     le_q_from;
      QLineEdit *                     le_q_to;
      QPushButton *                   pb_log_y;
      QLabel *                        lbl_r1;
      mQLineEdit *                    le_r1_from;
      mQLineEdit *                    le_r1_to;
      QCheckBox *                     cb_r2;
      mQLineEdit *                    le_r2_from;
      mQLineEdit *                    le_r2_to;
      QPushButton *                   pb_suggest;
      QPushButton *                   pb_wheel_dec;
      QwtWheel *                      qwtw_wheel;
      QPushButton *                   pb_wheel_inc;
      QLabel *                        lbl_wheel;
      mQwtPlot *                      plot;
      US_Plot *                       usp_plot;
      ScrollZoomer *                  plot_zoomer;
      mQwtPlot *                      plot_chi2;
      US_Plot *                       usp_plot_chi2;
      QLabel *                        lbl_stats;
      QPushButton *                   pb_help;
      QPushButton *                   pb_select_r1;
      QPushButton *                   pb_select_both;
      QPushButton *                   pb_close;

      mQLineEdit *                    le_wheel;       // the marker the wheel moves
      bool                            log_y;
      bool                            suppress;

      void                            setupGUI();
      void                            compute_It();
      int                             frame_index( const QString & text );
      bool                            region_range( QLineEdit * le_from, QLineEdit * le_to, int & i0, int & i1 );
      void                            compute_region( int i0, int i1, region_stats & r );
      QString                         region_text( const QString & name, const region_stats & r );
      void                            set_region_text( QLineEdit * le_from, QLineEdit * le_to, int i0, int i1 );
      void                            replot_It( bool rescale );
      void                            replot_chi2();
      void                            select_frames( bool both );
      void                            wheel_target( mQLineEdit * le );
      bool                            suggest_regions( int & a0, int & a1, int & b0, int & b1, QString & msg );

   private slots:
      void                            q_changed();
      void                            regions_changed();
      void                            suggest();
      void                            log_y_toggle();
      void                            adjust_wheel( double );
      void                            wheel_dec();
      void                            wheel_inc();
      void                            select_r1();
      void                            select_both();
      void                            help();
      void                            usp_config_plot( const QPoint & );
      void                            usp_config_plot_chi2( const QPoint & );

   protected slots:
      void                            closeEvent( QCloseEvent * );
};

#endif
