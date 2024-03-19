#ifndef US_HYDRODYN_MALS_SAXS_H
#define US_HYDRODYN_MALS_SAXS_H

// QT defs:

#include <qsplitter.h>
#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
//#include <q3frame.h>
#include <qcheckbox.h>
#include <qtextedit.h>
#include <qprogressbar.h>
#include <qmenubar.h>
#include <qfileinfo.h>
#include <qprinter.h>
#include <qradiobutton.h>
#include <qtablewidget.h>
#include <qwt_plot_zoomer.h>
#include <qwt_wheel.h>
//Added by qt3to4:
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QCloseEvent>
#if QT_VERSION < 0x040000
# include "../3dplot/mesh2mainwindow.h"
#endif

#include "qwt_plot_marker.h"
#include "qwt_symbol.h"

#include "us_util.h"
#include "us_plot_util.h"

//standard C and C++ defs:

#include <map>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "us_hydrodyn_saxs.h"
#include "us_hydrodyn_mals_saxs_conc.h"
#include "qwt/scrollbar.h"
#include "qwt/scrollzoomer.h"
#include "us_saxs_util.h"
#include "us_lud.h"

using namespace std;

#ifdef WIN32
# define use_erf(x) US_Saxs_Util::spline_erf(x)
#else
# define use_erf(x) erf(x)
#endif

#ifndef M_SQRT2PI
# define M_SQRT2PI 2.50662827463e0
#endif

#ifndef M_ONE_OVER_SQRT2PI
# define M_ONE_OVER_SQRT2PI 3.98942280402e-1
#endif

#ifndef M_SQRT2
# define M_SQRT2   1.41421356237e0
#endif
#ifndef M_ONE_OVER_SQRT2
# define M_ONE_OVER_SQRT2   7.07106781188e-1
#endif

struct mals_saxs_stack_data
{
   map < QString, vector < QString > > f_qs_string;
   map < QString, vector < double > >  f_qs;
   map < QString, vector < double > >  f_Is;
   map < QString, vector < double > >  f_errors;
   map < QString, vector < double > >  f_gaussians;
   map < QString, unsigned int >       f_pos;

   map < QString, QString >            f_name;
   map < QString, QString >            f_header;
   map < QString, bool >               f_is_time;
   map < QString, double >             f_psv;
   map < QString, double >             f_I0se;
   map < QString, double >             f_conc;
   map < QString, double >             f_extc;
   map < QString, double >             f_time;

   map < QString, bool >               created_files_not_saved;
   QStringList                         files;
   map < QString, bool >               selected_files;
   QStringList                         created_files;
   map < QString, bool >               created_selected_files;
   vector < double >                   gaussians;

   set < QString >                     conc_files;
};

class MALS_SAXS_Angle {
 public:

   double  angle;
   double  angle_ri_corr;
   double  gain;
   double  norm_coef;
   bool    has_gain;
   bool    has_angle_ri_corr;
   bool    has_norm_coef;
   
   QString list();
   QString list_csv();
   QString list_rich( double lambda = 0, double n = 0 ); // set lambda & n non-zero to include q values
   bool    populate( const QStringList & );
};

class MALS_SAXS_Angles {
 public:
   map < int, MALS_SAXS_Angle > mals_saxs_angle;
   QString                 list();
   QString list_rich( double lambda = 0, double n = 0 ); // set lambda & n non-zero to include q values
   QStringList             list_active();
   void                    clear();
   QString                 loaded_filename;
   bool                    populate( const QString & filename, const QStringList & );
   bool                    load( const QString & filename, const QStringList & csvlines , QString & errormsg );
   bool                    load( const QString & filename, QString & errormsg );
   bool                    save( const QString & filename, QString & errormsg );
};

class US_EXTERN US_Hydrodyn_Mals_Saxs : public QFrame
{
   Q_OBJECT

      friend class US_Hydrodyn;
      friend class US_Hydrodyn_Batch;
      friend class US_Hydrodyn_Saxs;
      friend class US_Hydrodyn_Mals_Saxs_Conc;
      friend class US_Hydrodyn_Mals_Saxs_Fit;
      friend class US_Hydrodyn_Mals_Saxs_Fit_Global;
      friend class US_Hydrodyn_Mals_Saxs_Options;
      friend class US_Hydrodyn_Mals_Saxs_Gauss_Mode;
      friend class US_Hydrodyn_Mals_Saxs_Nth;
      friend class US_Hydrodyn_Mals_Saxs_Svd;
      friend class US_Hydrodyn_Mals_Saxs_Movie;
      friend class US_Hydrodyn_Mals_Saxs_Baseline_Best;
      friend class US_Hydrodyn_Mals_Saxs_Simulate;

   public:
      US_Hydrodyn_Mals_Saxs(
                              csv csv1,
                              void *us_hydrodyn, 
                              QWidget *p = 0
                             );
      ~US_Hydrodyn_Mals_Saxs();

      void add_plot( QString           name,
                     vector < double > q,
                     vector < double > I,
                     bool              is_time = false,
                     bool              replot  = true );

      void add_plot( QString           name,
                     vector < double > q,
                     vector < double > I,
                     vector < double > errors,
                     bool              is_time = false,
                     bool              replot  = true );

      void add_plot_gaussian(
                             const QString     & file,
                             const QString     & tag
                             );

   private:
#if QT_VERSION < 0x040000
      Mesh2MainWindow *plot3d_window;
#endif
      bool             plot3d_flag;

      void * mals_saxs_options_widget;

      // "main plot options ", "plot selections", "plot properties",  "cropping", "",

      // new:
      // main plot options:
      // pb_rescale
      // pb_rescale_y
      // pb_axis_x
      // pb_axis_y
      // cb_eb
      // pb_legend
      // pb_pp

      // plot selections:
      // pb_select_vis
      // pb_remove_vis
      // pb_movie

      // plot cropping
      // pb_crop_common
      // pb_crop_vis
      // pb_crop_zero
      // pb_crop_left
      // pb_crop_undo
      // pb_crop_right

      // plot properties
      // pb_line_width
      // pb_color_rotate

      // currently in line 1
      // pb_select_vis
      // pb_remove_vis
      // pb_crop_common
      // pb_crop_vis
      // pb_crop_zero
      // pb_crop_left
      // pb_crop_undo
      // pb_crop_right
      // pb_legend

      // currently in line 2
      // pb_rescale
      // pb_axis_x
      // pb_axis_y
      // cb_eb
      // pb_movie
      // pb_line_width
      // pb_color_rotate
      // pb_pp

      enum pbmodes {
         PBMODE_MAIN,
         PBMODE_SEL,
         PBMODE_CROP,
         PBMODE_CONC,
         PBMODE_SD,
         PBMODE_FASTA,
         PBMODE_MALS_SAXS,
         PBMODE_Q_EXCLUDE,
         PBMODE_NONE
      };

      QLabel                * lbl_pbmode;
      QButtonGroup          * bg_pbmode;
      QRadioButton          * rb_pbmode_main;
      QRadioButton          * rb_pbmode_sel;
      QRadioButton          * rb_pbmode_crop;
      QRadioButton          * rb_pbmode_conc;
      QRadioButton          * rb_pbmode_sd;
      QRadioButton          * rb_pbmode_fasta;
      QRadioButton          * rb_pbmode_mals_saxs;
      QRadioButton          * rb_pbmode_q_exclude;
      QRadioButton          * rb_pbmode_none;
      void                    pbmode_select( pbmodes mode );

 private slots:

      void set_pbmode_main( );
      void set_pbmode_sel( );
      void set_pbmode_crop( );
      void set_pbmode_conc( );
      void set_pbmode_sd( );
      void set_pbmode_fasta( );
      void set_pbmode_mals_saxs( );
      void set_pbmode_q_exclude( );
      void set_pbmode_none( );

 private:

      MALS_SAXS_Angles   mals_saxs_angles;
      bool          mals_saxs_load( const QString & filename, const QStringList & qsl, QString & errormsg ); 

      double        mals_saxs_param_lambda;
      double        mals_saxs_param_n;
      double        mals_saxs_param_g_dndc;
      double        mals_saxs_param_g_extinction_coef;
      double        mals_saxs_param_g_conc;
      int           mals_saxs_param_DLS_detector;

      void          deselect_conc_file();
      
      csv           csv1;

      void          *us_hydrodyn;

      US_Config     *USglobal;

      QLabel        *lbl_title;

      QProgressBar  *progress;

      mQLabel       *lbl_files;
      QCheckBox     *cb_lock_dir;
      mQLineEdit    *le_dir;
      QPushButton   *pb_add_files;
      QPushButton   *pb_add_dir;
      QPushButton   *pb_similar_files;
      QPushButton   *pb_conc;
      QPushButton   *pb_clear_files;

      QPushButton   *pb_regex_load;
      QLineEdit     *le_regex;
      QLineEdit     *le_regex_args;
      QPushButton   *pb_save_state;

      QPushButton   *pb_select_all;
      QPushButton   *pb_invert;
      QPushButton   *pb_adjacent;
      QPushButton   *pb_select_nth;
      // QPushButton   *pb_join;
      QPushButton   *pb_to_saxs;
      QPushButton   *pb_view;
      QPushButton   *pb_movie;
      QPushButton   *pb_ag;
      QCheckBox     *cb_eb;
      QCheckBox     *cb_dots;
      QPushButton   *pb_rescale;
      QPushButton   *pb_rescale_y;

      QPushButton   *pb_stack_push_all;
      QPushButton   *pb_stack_push_sel;
      QLabel        *lbl_stack;
      QPushButton   *pb_stack_copy;
      QPushButton   *pb_stack_pcopy;
      QPushButton   *pb_stack_paste;
      QPushButton   *pb_stack_drop;
      QPushButton   *pb_stack_clear;
      QPushButton   *pb_stack_join;
      QPushButton   *pb_stack_swap;
      QPushButton   *pb_stack_rot_up;
      QPushButton   *pb_stack_rot_down;

      QListWidget      *lb_files;
      QLabel        *lbl_selected;
      // QPushButton   *pb_plot_files;
      QPushButton   *pb_conc_avg;
      QPushButton   *pb_normalize;
      QPushButton   *pb_add;
      QPushButton   *pb_avg;
      QPushButton   *pb_bin;
      QPushButton   *pb_smooth;
      QPushButton   *pb_repeak;
      QPushButton   *pb_svd;
      QPushButton   *pb_common_time;
      QPushButton   *pb_scroll_pair;
      QPushButton   *pb_scale_pair;
      QPushButton   *pb_join_by_time;
      QPushButton   *pb_create_i_of_t;
      QPushButton   *pb_test_i_of_t;
      QPushButton   *pb_create_i_of_q;
      QPushButton   *pb_create_ihash_t;
      QPushButton   *pb_create_istar_q;

      QPushButton   *pb_fasta_file;
      QLabel        *lbl_fasta_value;
      QLineEdit     *le_fasta_value;

      QPushButton   *pb_mals_saxs_angles_save;
      QLabel        *lbl_mals_saxs_angles_data;
      
 private slots:
      void           fasta_file();
 private:

      QPushButton   *pb_load_conc;
      QPushButton   *pb_conc_file;
      QLabel        *lbl_conc_file;

      QPushButton   *pb_detector;

      QPushButton   *pb_set_mals_saxs;
      QLabel        *lbl_mals_saxs;

 private slots:
      bool           mals_saxs_angles_save();
 private:

      QPushButton   *pb_set_empty;
      QLabel        *lbl_empty;

      QPushButton   *pb_set_signal;
      QLabel        *lbl_signal;

      QLabel        *lbl_created_files;
      mQLineEdit    *le_created_dir;
      QListWidget      *lb_created_files;
      QLabel        *lbl_selected_created;

      // models
      map < QString, QString > models;
      set < QString >          models_not_saved;

      QLabel        *lbl_model_files;
      QListWidget      *lb_model_files;
      QLabel        *lbl_model_created;

      QPushButton   * pb_model_select_all;
      QPushButton   * pb_model_save;
      QPushButton   * pb_model_text;
      QPushButton   * pb_model_view;
      QPushButton   * pb_model_remove;

      QPushButton   *pb_select_all_created;
      QPushButton   *pb_invert_all_created;
      QPushButton   *pb_adjacent_created;
      QPushButton   *pb_remove_created;
      QPushButton   *pb_save_created_csv;
      QPushButton   *pb_save_created;
      QPushButton   *pb_show_created;
      QPushButton   *pb_show_only_created;

      void           model_enables();
      void           model_remove ( QStringList files );
      bool           model_save   ( QStringList files );
      bool           model_save   ( QString file, bool & cancel, bool & overwrite_all );
      void           model_view   ( QStringList files );
      void           model_text   ( QStringList files );

      mQLabel     * lbl_editor;
      QFont         ft;
      QTextEdit     *editor;
      QMenuBar      *m;

      QPushButton   *pb_help;
      QPushButton   *pb_options;
      QPushButton   *pb_cancel;

      QSplitter     *qs_plots;
      QBoxLayout    *vbl_plot_group;
      mQwtPlot      *plot_dist;
      US_Plot       *usp_plot_dist;

      void          conc_info( const QString & msg = "" );
      void          dndc_info( const QString & msg = "" );
      void          extc_info( const QString & msg = "" );      

   private slots:
      void usp_config_plot_dist( const QPoint & );

   private:
      ScrollZoomer  *plot_dist_zoomer;
      QwtPlotGrid   *grid_saxs;
      bool          legend_vis;

      mQwtPlot      *plot_errors;
      US_Plot       *usp_plot_errors;
   private slots:
      void usp_config_plot_errors( const QPoint & );

   private:
      ScrollZoomer  *plot_errors_zoomer;
      QwtPlotGrid   *grid_errors;
      mQwtPlot      *plot_ref;
      US_Plot       *usp_plot_ref;
   private slots:
      void usp_config_plot_ref( const QPoint & );

   private:
      QwtPlotGrid   *grid_ref;

      QCheckBox     *cb_plot_errors_rev;
      QCheckBox     *cb_plot_errors_sd;
      QCheckBox     *cb_plot_errors_pct;
      QCheckBox     *cb_plot_errors_group;

      QBoxLayout    *l_plot_errors;

      QPushButton   *pb_timeshift;
      QPushButton   *pb_timescale;
      QPushButton   *pb_p3d;
      QLabel        *lbl_blank1;
      QLabel        *lbl_wheel_pos;
      QLabel        *lbl_blank2;
      QLabel        *lbl_wheel_pos_below;
      QLabel        *lbl_wheel_Pcolor;
      QImage        *qi_green;
      QImage        *qi_yellow;
      QImage        *qi_red;
      QPushButton   *pb_wheel_inc;
      QPushButton   *pb_wheel_dec;
      QwtWheel      *qwtw_wheel;
      QPushButton   *pb_ref;
      QPushButton   *pb_errors;
      QPushButton   *pb_cormap;
      QPushButton   *pb_ggqfit;
      QPushButton   *pb_pp;
      QPushButton   *pb_wheel_cancel;
      QPushButton   *pb_wheel_save;

      QPushButton   *pb_gauss_mode;
      QPushButton   *pb_gauss_start;
      QPushButton   *pb_gauss_clear;
      QPushButton   *pb_gauss_new;
      QPushButton   *pb_gauss_delete;
      QPushButton   *pb_gauss_prev;
      QLabel        *lbl_gauss_pos;
      QPushButton   *pb_gauss_next;
      mQLineEdit    *le_gauss_pos;
      mQLineEdit    *le_gauss_pos_width;
      mQLineEdit    *le_gauss_pos_height;
      mQLineEdit    *le_gauss_pos_dist1;
      mQLineEdit    *le_gauss_pos_dist2;
      QCheckBox     *cb_sd_weight;
      QCheckBox     *cb_fix_width;
      QCheckBox     *cb_fix_dist1;
      QCheckBox     *cb_fix_dist2;
      QPushButton   *pb_gauss_fit;
      QLabel        *lbl_gauss_fit;
      mQLineEdit    *le_gauss_fit_start;
      mQLineEdit    *le_gauss_fit_end;
      QPushButton   *pb_gauss_save;

      QPushButton   *pb_gauss_local_caruanas;
      QPushButton   *pb_gauss_local_guos;
      QLineEdit     *le_gauss_local_pts;
      bool          data_point_window( vector < double > &q, vector < double > &I );

      QCheckBox     *cb_gauss_match_amplitude;

      QCheckBox     *cb_ggauss_scroll;
      QCheckBox     *cb_ggauss_scroll_p_green;
      QCheckBox     *cb_ggauss_scroll_p_yellow;
      QCheckBox     *cb_ggauss_scroll_p_red;
      QCheckBox     *cb_ggauss_scroll_smoothed;
      QCheckBox     *cb_ggauss_scroll_oldstyle;

      QPushButton   *pb_ggauss_start;
      QPushButton   *pb_ggauss_rmsd;
      QPushButton   *pb_ggauss_results;

      QPushButton   *pb_gauss_as_curves;
      QPushButton   *pb_ggauss_as_curves;

      QPushButton   *pb_blanks_start;
      QPushButton   *pb_blanks_params;
      QPushButton   *pb_baseline_blanks;
      QPushButton   *pb_bb_cm_inc;
      QPushButton   *pb_baseline_start;
      QPushButton   *pb_baseline_test;

      QCheckBox     *cb_baseline_start_zero;
      mQLineEdit    *le_baseline_start_s;
      mQLineEdit    *le_baseline_start;
      mQLineEdit    *le_baseline_start_e;
      mQLineEdit    *le_baseline_end_s;
      mQLineEdit    *le_baseline_end;
      mQLineEdit    *le_baseline_end_e;
      QCheckBox     *cb_baseline_fix_width;
      mQLineEdit    *le_baseline_width;
      QPushButton   *pb_baseline_best;
      QPushButton   *pb_baseline_apply;

      QPushButton   *pb_line_width;
      QPushButton   *pb_color_rotate;

      QPushButton   *pb_select_vis;
      QPushButton   *pb_remove_vis;
      QPushButton   *pb_crop_zero;
      QPushButton   *pb_crop_vis;
      QPushButton   *pb_crop_to_vis;
      QPushButton   *pb_crop_common;
      QPushButton   *pb_crop_left;
      QPushButton   *pb_crop_undo;
      QPushButton   *pb_crop_right;
      QPushButton   *pb_legend;
      QPushButton   *pb_axis_x;
      QPushButton   *pb_axis_y;

      QLabel        *lbl_mode_title;

      // q_exclude

      QPushButton   *pb_q_exclude_vis;
      QPushButton   *pb_q_exclude_left;
      QPushButton   *pb_q_exclude_right;
      QPushButton   *pb_q_exclude_clear;
      QLabel        *lbl_q_exclude;
      QLabel        *lbl_q_exclude_detail;

      void          q_exclude_update_lbl();
      void          q_exclude_opt_remove_unreferenced();
      
      set < double > q_exclude;
      
      // wyatt errors

      QPushButton   *pb_wyatt_start;

      QCheckBox     *cb_wyatt_2;
      mQLineEdit    *le_wyatt_start;
      mQLineEdit    *le_wyatt_end;
      mQLineEdit    *le_wyatt_start2;
      mQLineEdit    *le_wyatt_end2;
      QPushButton   *pb_wyatt_apply;

      void           wyatt_enables();
      void           wyatt_init_markers();
      void           replot_wyatt();
      bool           org_wyatt_2;
      double         org_wyatt_start;
      double         org_wyatt_end;
      double         org_wyatt_start2;
      double         org_wyatt_end2;

      // scale

      QPushButton   *pb_scale;

      QLabel       * lbl_scale_low_high;
      QRadioButton * rb_scale_low;
      QRadioButton * rb_scale_high;
      QButtonGroup * bg_scale_low_high;
      QCheckBox    * cb_scale_sd;
      QCheckBox    * cb_scale_save_intp;
      QCheckBox    * cb_scale_scroll;
      QLabel       * lbl_scale_q_range;
      mQLineEdit   * le_scale_q_start;
      mQLineEdit   * le_scale_q_end;
      QPushButton  * pb_scale_q_reset;
      QPushButton  * pb_scale_reset;
      QPushButton  * pb_scale_apply;
      QPushButton  * pb_scale_create;

      // testiq

      QPushButton  * pb_testiq;

      QLabel       * lbl_testiq_q_range;
      mQLineEdit   * le_testiq_q_start;
      mQLineEdit   * le_testiq_q_end;
      QPushButton  * pb_testiq_visrange;
      QPushButton  * pb_testiq_testset;

      QCheckBox    * cb_testiq_from_gaussian;

      QLabel                *    lbl_testiq_gaussians;
      QButtonGroup          *    bg_testiq_gaussians;
      QRadioButton          *    rb_testiq_from_i_t;
      QHBoxLayout           *    hbl_testiq_gaussians;
      vector < QRadioButton * >  rb_testiq_gaussians;
      
      bool           testiq_ggaussian_ok;

      set < QString > testiq_selected;

      bool           testiq_make();
      bool           testiq_active;

      vector < QString >                 testiq_created_names;
      map < QString, double >            testiq_created_t;
      map < QString, vector < double > > testiq_created_q;
      map < QString, vector < double > > testiq_created_I;
      map < QString, vector < double > > testiq_created_e;

      set < QString >                    testiq_original_selection;
      set < QString >                    testiq_created_scale_names;

      QString                            testiq_it_selected;
      double                             testiq_it_selected_Imin;
      double                             testiq_it_selected_Imax;

      // scroll pair
      
      vector < QStringList >             scroll_pair_names;
      vector < double >                  scroll_pair_times;
      map < double, vector < QString > > scroll_pair_time_to_names;
      set < QString >                    scroll_pair_org_selected;
      int                                scroll_pair_scroll_pos;
      void                               scroll_pair_scroll_highlight( int pos );
      void                               scroll_pair_enables();

      // scale pair
      
      vector < QStringList >             scale_pair_names;
      vector < double >                  scale_pair_times;
      map < double, vector < QString > > scale_pair_time_to_names;
      set < QString >                    scale_pair_org_selected;
      int                                scale_pair_scroll_pos;
      void                               scale_pair_scroll_highlight( int pos );
      void                               scale_pair_enables();

      set < QString >                    scale_pair_created_names;
      map < double, QString >            scale_pair_created_time_to_name;
      map < QString, vector < double > > scale_pair_created_q;
      map < QString, vector < double > > scale_pair_created_I;
      map < QString, vector < double > > scale_pair_created_e;

      QColor                             scale_pair_color_q1;
      QColor                             scale_pair_color_q2;

      QLabel                           * lbl_scale_pair_msg;

      QLabel                           * lbl_scale_pair_q1_range;
      mQLineEdit                       * le_scale_pair_q1_start;
      mQLineEdit                       * le_scale_pair_q1_end;
      QLabel                           * lbl_scale_pair_q2_range;
      mQLineEdit                       * le_scale_pair_q2_start;
      mQLineEdit                       * le_scale_pair_q2_end;
      QLabel                           * lbl_scale_pair_fit_curve;
      QLabel                           * lbl_scale_pair_sd_scale;
      QLineEdit                        * le_scale_pair_sd_scale;
      QLabel                           * lbl_scale_pair_scale;
      mQLineEdit                       * le_scale_pair_scale;
      QLabel                           * lbl_scale_pair_time;
      mQLineEdit                       * le_scale_pair_time;

      QPushButton                      * pb_scale_pair_fit;
      QPushButton                      * pb_scale_pair_minimize;
      QPushButton                      * pb_scale_pair_reset;
      QPushButton                      * pb_scale_pair_create_scaled_curves;

      set < QString >                    scale_pair_original_selection;
      double                             scale_pair_original_scale;
      double                             scale_pair_original_sd_scale;

      double                             scale_pair_q1_min;
      double                             scale_pair_q1_max;
      double                             scale_pair_q2_min;
      double                             scale_pair_q2_max;

      vector < QwtPlotMarker * >         scale_pair_markers;

      void                               scale_pair_add_marker  (
                                                                 QwtPlot * plot,
                                                                 double pos, 
                                                                 QColor color, 
                                                                 QString text, 
                                                                 Qt::Alignment align = Qt::AlignRight | Qt::AlignTop
                                                                 );
      void                               scale_pair_delete_markers();

      int                                scale_pair_time_pos;

      bool                               scale_pair_use_errors;

      enum scale_pair_fit_curves : int {
         SCALE_PAIR_FIT_CURVE_P2 = 2,
         SCALE_PAIR_FIT_CURVE_P3 = 3,
         SCALE_PAIR_FIT_CURVE_P4 = 4,
         SCALE_PAIR_FIT_CURVE_P5 = 5,
         SCALE_PAIR_FIT_CURVE_P6 = 6,
         SCALE_PAIR_FIT_CURVE_P7 = 7,
         SCALE_PAIR_FIT_CURVE_P8 = 8
      };

      vector < vector < double > >       scale_pair_qgrids;
      int                                scale_pair_mals_set;
      int                                scale_pair_saxs_set;

      void                               scale_pair_created_init();
      void                               scale_pair_created_update();
      void                               scale_pair_created_remove();

      set < QString >                    scale_pair_save_names;
      
      QComboBox                        * cb_scale_pair_fit_curve;
      QComboBox                        * cb_scale_pair_fit_alg;
      QComboBox                        * cb_scale_pair_fit_alg_weight;

      QCheckBox                        * cb_scale_pair_scale_saxs;

      QwtPlotCurve                     * scale_pair_fit_curve;

      void                               scale_pair_fit_clear( bool replot = true );

      bool                               check_files_selected_paired();
      bool                               check_files_selected_paired( const QStringList & files );
      vector < QStringList >             saved_nth_pair_names;
      QStringList                        saved_nth_last_paired_selections;
      bool                               saved_nth_last_paired_valid();
      set < QString >                    qsl_to_set( const QStringList & qsl );
      QStringList                        set_to_qsl( const set < QString > & qsset );

      void                               scale_pair_minimize_clear();

   private slots:

      void scale_pair                     ( bool no_store_original = false );

      void scale_pair_fit();
      void scale_pair_reset();
      void scale_pair_minimize();
      void scale_pair_create_scaled_curves();
      void scale_pair_sd_scale            ( const QString & );
      void scale_pair_scale               ( const QString & );
      void scale_pair_scale_focus         ( bool );
      
      void scale_pair_q1_start_text       ( const QString & );
      void scale_pair_q1_start_focus      ( bool );
      void scale_pair_q1_end_text         ( const QString & );
      void scale_pair_q1_end_focus        ( bool );
      void scale_pair_q2_start_text       ( const QString & );
      void scale_pair_q2_start_focus      ( bool );
      void scale_pair_q2_end_text         ( const QString & );
      void scale_pair_q2_end_focus        ( bool );

      void scale_pair_time_focus          ( bool );

      // qt doc says int argument for the signal, but actually QString
      void scale_pair_fit_alg_index       (); // int index );
      void scale_pair_fit_alg_weight_index(); // int index );
      void scale_pair_fit_curve_index     (); // int index );

      void scale_pair_scale_saxs          ();

      bool scale_pair_fit_at_time( double time
                                   ,int degree
                                   ,const set < double > & q1
                                   ,const set < double > & q2
                                   ,double & chi2 );

   private:

      // Guinier

      QPushButton   *pb_guinier;

      QCheckBox    * cb_guinier_scroll;

      QLabel       * lbl_guinier_q_range;
      mQLineEdit   * le_guinier_q_start;
      mQLineEdit   * le_guinier_q_end;
      QLabel       * lbl_guinier_q2_range;
      mQLineEdit   * le_guinier_q2_start;
      mQLineEdit   * le_guinier_q2_end;
      QLabel       * lbl_guinier_delta_range;
      mQLineEdit   * le_guinier_delta_start;
      mQLineEdit   * le_guinier_delta_end;
      QCheckBox    * cb_guinier_qrgmax;
      QLineEdit    * le_guinier_qrgmax;
      QCheckBox    * cb_guinier_sd;
      QLabel       * lbl_guinier_stats;

      QRadioButton * rb_guinier_resid_diff;
      QRadioButton * rb_guinier_resid_sd;
      QRadioButton * rb_guinier_resid_pct;
      QButtonGroup * bg_guinier_resid_type;

      QPushButton  * pb_guinier_plot_rg;
      QPushButton  * pb_guinier_plot_mw;

      QLabel       * lbl_guinier_rg_t_range;
      mQLineEdit   * le_guinier_rg_t_start;
      mQLineEdit   * le_guinier_rg_t_end;

      QLabel       * lbl_guinier_rg_rg_range;
      mQLineEdit   * le_guinier_rg_rg_start;
      mQLineEdit   * le_guinier_rg_rg_end;

      QCheckBox    * cb_guinier_lock_rg_range;
      QPushButton  * pb_guinier_replot;

      QLabel       * lbl_guinier_mw_t_range;
      mQLineEdit   * le_guinier_mw_t_start;
      mQLineEdit   * le_guinier_mw_t_end;

      QLabel       * lbl_guinier_mw_mw_range;
      mQLineEdit   * le_guinier_mw_mw_start;
      mQLineEdit   * le_guinier_mw_mw_end;

      QCheckBox    * cb_guinier_lock_mw_range;
      QPushButton  * pb_guinier_mw_replot;

      // QCheckBox    * cb_guinier_repeat;
      // QLineEdit    * le_guinier_repeat_sd_limit;
      // QCheckBox    * cb_guinier_search;

      mQwtPlot     * guinier_plot;
      US_Plot      * usp_guinier_plot;
   private slots:
      void usp_config_guinier_plot( const QPoint & );

   private:
      ScrollZoomer * guinier_plot_zoomer;
      QwtPlotGrid  * guinier_plot_grid;

      mQwtPlot     * guinier_plot_errors;
      US_Plot      * usp_guinier_plot_errors;
   private slots:
      void usp_config_guinier_plot_errors( const QPoint & );

   private:
      ScrollZoomer * guinier_plot_errors_zoomer;
      QwtPlotGrid  * guinier_plot_errors_grid;

      mQwtPlot     * guinier_plot_rg;
      US_Plot      * usp_guinier_plot_rg;
   private slots:
      void usp_config_guinier_plot_rg( const QPoint & );

   private:
      ScrollZoomer * guinier_plot_rg_zoomer;
      QwtPlotGrid  * guinier_plot_rg_grid;

      mQwtPlot     * guinier_plot_mw;
      US_Plot      * usp_guinier_plot_mw;
   private slots:
      void usp_config_guinier_plot_mw( const QPoint & );

   private:
      ScrollZoomer * guinier_plot_mw_zoomer;
      QwtPlotGrid  * guinier_plot_mw_grid;

      mQwtPlot     * guinier_plot_summary;
      US_Plot      * usp_guinier_plot_summary;
   private slots:
      void usp_config_guinier_plot_summary( const QPoint & );

   private:

      int                                 guinier_scroll_pos;
      void                                guinier_scroll_highlight( int pos );

      vector < QString >                  guinier_names;
      map < QString, vector < double > >  guinier_q;
      map < QString, vector < double > >  guinier_q2;
      map < QString, vector < double > >  guinier_I;
      map < QString, vector < double > >  guinier_e;
      map < QString, vector < double > >  guinier_x;
      map < QString, vector < double > >  guinier_y;
      map < QString, double >             guinier_a;
      map < QString, double >             guinier_b;
      map < QString, double >             guinier_t;
      vector < double >                   guinier_it_t;
      vector < double >                   guinier_it_I;
      vector < double >                   guinier_it_pg_t;
      vector < double >                   guinier_it_pg_I;
      double                              guinier_it_Imin;
      double                              guinier_it_Imax;
      double                              guinier_it_Irange;
      map < QString, QString >            guinier_report;

      map < QString, QwtPlotCurve * >     guinier_curves;
      vector < QwtPlotMarker * >          guinier_markers;
      map < QString, QwtPlotCurve * >     guinier_fit_lines; 
      map < QString, QwtPlotCurve * >     guinier_error_curves;
      map < QString, vector < QwtPlotCurve * > >  guinier_errorbar_curves;
      map < QString, QwtPlotCurve * >     guinier_rg_curves;
      map < QString, QwtPlotCurve * >     guinier_mw_curves;
      map < QString, QwtPlotMarker * >    guinier_mwt_markers;
      map < QString, QwtPlotMarker * >    guinier_mwc_markers;
      map < QString, QColor >             guinier_colors;

      void           guinier_analysis     ();
      void           guinier_residuals    ( bool reset = false );
      void           guinier_range        ();
      void           guinier_range        ( 
                                           double minq2, 
                                           double maxq2,
                                           double minI, 
                                           double maxI
                                            );

      void           guinier_add_marker   (
                                           QwtPlot * plot,
                                           double pos, 
                                           QColor color, 
                                           QString text, 
                                           Qt::Alignment align = Qt::AlignRight | Qt::AlignTop
                                           );
      void           guinier_delete_markers();
      vector < QWidget * > guinier_errors_widgets;
      vector < QWidget * > guinier_rg_widgets;
      vector < QWidget * > guinier_mw_widgets;

      double         guinier_minq;
      double         guinier_maxq;
      double         guinier_minq2;
      double         guinier_maxq2;
      double         guinier_mint;
      double         guinier_maxt;

      // rgc util

      QPushButton  * pb_rgc;
      QLabel       * lbl_rgc_mw;
      QLineEdit    * le_rgc_mw;
      QLabel       * lbl_rgc_vol;
      QLineEdit    * le_rgc_vol;
      QLabel       * lbl_rgc_rho;
      QLineEdit    * le_rgc_rho;
      QLabel       * lbl_rgc_axis;
      QRadioButton * rb_rgc_shape_sphere;
      QRadioButton * rb_rgc_shape_oblate;
      QRadioButton * rb_rgc_shape_prolate;
      QRadioButton * rb_rgc_shape_ellipsoid;
      QButtonGroup * bg_rgc_shape;

      QLineEdit    * le_rgc_axis_b;
      QLineEdit    * le_rgc_axis_c;
      QLabel       * lbl_rgc_rg;
      QLineEdit    * le_rgc_rg;
      QLabel       * lbl_rgc_extents;
      QLineEdit    * le_rgc_extents;
      QLabel       * lbl_rgc_g_qrange;
      QLineEdit    * le_rgc_g_qrange;

      void           rgc_calc_rg();

      // simulate
      QPushButton  * pb_simulate;

      // pm

      QPushButton  * pb_pm;
      QRadioButton * rb_pm_shape_sphere;
      QRadioButton * rb_pm_shape_spheroid;
      QRadioButton * rb_pm_shape_ellipsoid;
      QRadioButton * rb_pm_shape_cylinder;
      QRadioButton * rb_pm_shape_torus;
      QButtonGroup * bg_pm_shape;

      QCheckBox    * cb_pm_sd;
      QCheckBox    * cb_pm_q_logbin;
      QLabel       * lbl_pm_q_range;
      mQLineEdit   * le_pm_q_start;
      mQLineEdit   * le_pm_q_end;
      QPushButton  * pb_pm_q_reset;
      QLabel       * lbl_pm_samp_e_dens;
      QLineEdit    * le_pm_samp_e_dens;
      QLabel       * lbl_pm_buff_e_dens;
      QLineEdit    * le_pm_buff_e_dens;
      QLabel       * lbl_pm_grid_size;
      QLineEdit    * le_pm_grid_size;
      QLabel       * lbl_pm_q_pts;
      QLineEdit    * le_pm_q_pts;
      QPushButton  * pb_pm_run;
      
      QLineEdit    * le_dummy;

      set < QString >     scale_selected;
      vector < QString >  scale_selected_names;
      vector < QString >  scale_scroll_selected;
      int                 scale_scroll_pos;
      void                scale_scroll_highlight( int pos );

      bool          order_ascending;

      void          editor_msg( QString color, QString msg );
      void          editor_msg_qc( QColor qcolor, QString msg );

      bool          running;

      US_Hydrodyn_Saxs *saxs_window;
      bool             *saxs_widget;
      bool             activate_saxs_window();

      void             do_replot_saxs();

      QString          saxs_header_iqq;

      QString          errormsg;

      vector < double >                  plot_errors_grid;
      vector < double >                  plot_errors_target;
      vector < double >                  plot_errors_fit;
      vector < double >                  plot_errors_errors;
      QColor                             plot_errors_color;

      vector < mals_saxs_stack_data >         stack_data;

      vector < QColor >                  plot_colors;
      set < QString >                    conc_files;

      map < QString, vector < QString > > f_qs_string;
      map < QString, vector < double > >  f_qs;
      map < QString, vector < double > >  f_Is;
      map < QString, vector < double > >  f_errors;
      map < QString, vector < double > >  f_gaussians;
      map < QString, unsigned int >       f_pos;
      map < QString, QString >            f_name;
      map < QString, QString >            f_header;
      map < QString, bool >               f_is_time;
      map < QString, double >             f_psv;
      map < QString, double >             f_I0se;
      map < QString, double >             f_conc;
      map < QString, double >             f_extc;
      map < QString, double >             f_time;
      map < QString, double >             f_diffusion_len;
      map < QString, double >             f_e_nucleon_ratio;
      map < QString, double >             f_nucleon_mass;
      map < QString, double >             f_solvent_e_dens;
      map < QString, double >             f_I0st;
      map < QString, double >             f_g_dndc;
      map < QString, double >             f_dndc;
      map < QString, QString >            f_conc_units;
      map < QString, double >             f_ri_corr;
      map < QString, QString >            f_ri_corrs;
      map < QString, double >             f_ref_index;
      map < QString, QString >            f_fit_curve;
      map < QString, QString >            f_fit_method;
      map < QString, QString >            f_fit_q_ranges;
      map < QString, double >             f_fit_chi2;
      map < QString, double >             f_fit_sd_scale;

      // for displaying last smoothing in gg scroll mode
      map < QString, vector < double > >  f_qs_smoothed;
      map < QString, vector < double > >  f_Is_smoothed;
      map < QString, int >                f_best_smoothed_smoothing;
      void                                clear_smoothed(); // resets the smoothing data
      void                                list_smoothed();  // reports on smoothing
      void                                add_smoothed(
                                                       const QString            & name
                                                       ,const vector < double > & q
                                                       ,const vector < double > & I
                                                       ,int                       best_smoothing
                                                       );

      // for displaying oldstyle in gg scroll mode
      map < QString, vector < double > >  f_qs_oldstyle;
      map < QString, vector < double > >  f_Is_oldstyle;
      void                                clear_oldstyle(); // resets the oldstyle data
      void                                list_oldstyle();  // reports on oldstyle
      void                                add_oldstyle(
                                                       const QString            & name
                                                       ,const vector < double > & q
                                                       ,const vector < double > & I
                                                       );

      map < QString, bool >               created_files_not_saved;

      map < QString, double >             current_concs( bool quiet = false );
      map < QString, double >             window_concs();

      vector < crop_undo_data >           crop_undos;

      bool                                is_nonzero_vector( vector < double > &v );
      bool                                is_zero_vector( vector < double > &v );

      vector < double >                   union_q( QStringList files );

      map < QString, QwtPlotCurve * >     plotted_curves;
      vector < QwtPlotMarker * >          plotted_markers;
      vector < QwtPlotCurve * >           plotted_gaussians;
      vector < QwtPlotCurve * >           plotted_gaussian_sum;
      vector < QwtPlotCurve * >           plotted_baseline;
      vector < QwtPlotCurve * >           plotted_wyatt;
      vector < QwtPlotCurve * >           plotted_hlines;
      QwtPlotMarker *                     ref_marker;
      // always a multiple of 3 { a e^-[((x-b)/c)^2]/2 }, a, b, c
      // the b values are fixed by the user
      // a & c must be > 0
      vector < double >                   gaussians;  
      vector < double >                   org_gaussians;  
      vector < double >                   gaussian( double * g );
      vector < double >                   compute_gaussian_sum( vector < double > t, vector < double > g );
      double                              compute_gaussian_peak( QString file, vector < double > g );

      vector < double >                   unified_ggaussian_params; // global centers first, then height, width for each file
      vector < unsigned int >             unified_ggaussian_param_index; // index into params
      vector < double >                   unified_ggaussian_q;
      vector < double >                   unified_ggaussian_jumps;
      vector < double >                   unified_ggaussian_I;
      vector < double >                   unified_ggaussian_e;
      vector < double >                   unified_ggaussian_t;
      vector < double >                   unified_ggaussian_qvals;

      vector < unsigned int >             unified_ggaussian_q_start;        // the start q pos - one for each curve
      vector < unsigned int >             unified_ggaussian_q_end;          // the end q pos   - one for each curve

      int                                 ggaussian_sel_no_gaussian_count();

      map < QString, vector < double > >  org_f_gaussians;

      vector < double >                   compute_gaussian( vector < double > t, vector < double > g );
      vector < double >                   compute_ggaussian_gaussian_sum();
      void                                add_ggaussian_curve( QString name, vector < double > y );
      void                                gauss_add_hline( double center, double width );

      bool                                gg_fit_vector( 
                                                        vector < double > & fit,
                                                        vector < double > & pfit
                                                         );  // returns a vector of the chi^2 or rmsd and their q values &
      void                                ggauss_scroll_set_selected();
      void                                ggauss_scroll_highlight( int pos );
      vector < int >                      ggauss_scroll_set;
      bool                                ggauss_scroll_save_group;

      // the cormap p values from pairwise comparisons

      bool                                pvalue( const vector < double > &q, vector < double > &I, vector < double > &G, double &P, QString &errormsg ); // compute pvalue comparing 2 curves
      bool                                pvalue( const QString & file, double &P, QString &errormsg ); // compute pvalue comparing I with Gaussians
      
      map < QString, bool >               all_files_map();

      void                                update_plot_errors( vector < double > &grid, 
                                                              vector < double > &target, 
                                                              vector < double > &fit,
                                                              vector < double > &errors,
                                                              QColor            use_color = Qt::red
                                                              );

      void                                update_plot_errors_group();

      void                                hide_widgets( vector < QWidget *> widgets, bool hide );

      vector < QWidget * >                plot_errors_widgets;
      vector < QWidget * >                files_widgets;
      vector < QWidget * >                files_expert_widgets;
      vector < QWidget * >                created_files_widgets;
      vector < QWidget * >                created_files_expert_widgets;
      vector < QWidget * >                editor_widgets;
      vector < QWidget * >                model_widgets;
      set < QWidget * >                   always_hide_widgets;

      mQwtPlot                          * ggqfit_plot;
      US_Plot                           * usp_ggqfit_plot;

      void                                plot_debug();

   private slots:
      void usp_config_ggqfit_plot( const QPoint & );      

   private:
      ScrollZoomer                      * ggqfit_plot_zoomer;
      QwtPlotGrid                       * ggqfit_plot_grid;

      QCheckBox                         * cb_ggq_plot_chi2;
      QCheckBox                         * cb_ggq_plot_P;

      // "mode" widgets

      vector < QWidget * >                gaussian_widgets;
      vector < QWidget * >                gaussian_4var_widgets;
      vector < QWidget * >                gaussian_5var_widgets;
      vector < QWidget * >                ggaussian_widgets;
      vector < QWidget * >                ggaussian_4var_widgets;
      vector < QWidget * >                ggaussian_5var_widgets;
      vector < QWidget * >                ggqfit_widgets;
      vector < QWidget * >                wheel_below_widgets;

      vector < QWidget * >                scroll_pair_widgets;
      vector < QWidget * >                scale_pair_widgets;
      vector < QWidget * >                wyatt_widgets;
      vector < QWidget * >                blanks_widgets;
      vector < QWidget * >                baseline_widgets;
      vector < QWidget * >                scale_widgets;
      vector < QWidget * >                timeshift_widgets;
      vector < QWidget * >                plot_widgets;
      vector < QWidget * >                pbmode_main_widgets;
      vector < QWidget * >                pbmode_sel_widgets;
      vector < QWidget * >                pbmode_crop_widgets;
      vector < QWidget * >                pbmode_conc_widgets;
      vector < QWidget * >                pbmode_sd_widgets;
      vector < QWidget * >                pbmode_fasta_widgets;
      vector < QWidget * >                pbmode_mals_saxs_widgets;
      vector < QWidget * >                pbmode_q_exclude_widgets;
      vector < QWidget * >                rgc_widgets;
      vector < QWidget * >                pm_widgets;
      vector < QWidget * >                guinier_widgets;
      vector < QWidget * >                testiq_widgets;
      vector < vector < QWidget * > >     pb_row_widgets;

      vector < double >                   conc_curve( vector < double > &t,
                                                      unsigned int peak,
                                                      double conv );
      unsigned int                        unified_ggaussian_curves;
      unsigned int                        unified_ggaussian_gaussians_size;
      QStringList                         unified_ggaussian_files;
      bool                                unified_ggaussian_use_errors;
      bool                                unified_ggaussian_errors_skip;
      bool                                unified_ggaussian_ok;

      bool                                initial_ggaussian_fit( QStringList & files, bool only_init_unset = false );
      bool                                create_unified_ggaussian_target( bool do_init = true, bool only_init_unset = false );
      bool                                create_unified_ggaussian_target( QStringList & files, bool do_init = true, bool only_init_unset = false );
      double                              ggaussian_rmsd( bool normchi = true );
      bool                                ggaussian_compatible( bool check_against_global = true );
      bool                                ggaussian_compatible( QStringList & files, bool check_against_global = true );
      bool                                ggaussian_compatible( set < QString > & selected, bool check_against_global = true );
      vector < int >                      ggaussian_last_pfit_N;
      vector < int >                      ggaussian_last_pfit_C;
      vector < int >                      ggaussian_last_pfit_S;
      vector < double >                   ggaussian_last_pfit_P;
      vector < double >                   ggaussian_last_chi2;
      vector < int >                      ggaussian_selected_file_index;
      vector < vector < double > >        ggaussian_last_gg;
      vector < vector < vector < double > > >       ggaussian_last_ggig;
      vector < vector < double > >        compute_gaussian_sum_partials;
      vector < vector < double > >        ggaussian_last_gg_t;
      vector < vector < double > >        ggaussian_last_I;
      vector < vector < double > >        ggaussian_last_e;
      map < QString, double >             ggaussian_last_pfit_map;
      vector < QwtPlotMarker * >          ggaussian_pts_chi2;
      vector < QwtPlotMarker * >          ggaussian_pts_pfit;
      set < QwtPlotMarker * >             ggaussian_pts_chi2_marked;
      set < QwtPlotMarker * >             ggaussian_pts_pfit_marked;

      unsigned int                        gaussian_pos;
      void                                update_gauss_pos();
      void                                plot_gaussian_sum();
      void                                replot_gaussian_sum();

      bool                                wheel_is_pressed;

      bool                                wheel_errors_ok;

      bool                                opt_repeak_gaussians( QString file );

      QStringList                         all_selected_files();
      set < QString >                     all_selected_files_set();
      void                                remove_files( set < QString > & fileset, bool replot = false );
      QStringList                         all_files();
      QString                             last_selected_file;

      void save_csv_saxs_iqq();

      csv  current_csv();

      bool load_file( QString file, bool load_conc = false );

      void plot_files( bool save_zoom_state = false );
      bool plot_file( QString file,
                      double &minx,
                      double &maxx,
                      double &miny,
                      double &maxy );

      bool get_min_max( QString file,
                        double &minx,
                        double &maxx,
                        double &miny,
                        double &maxy );

      bool disable_updates;

      QString qstring_common_head( QString s1, 
                                   QString s2 );
      QString qstring_common_tail( QString s1, 
                                   QString s2 );

      QString qstring_common_head( QStringList qsl, bool strip_digits = false );
      QString qstring_common_tail( QStringList qsl, bool strip_digits = false );
      vector < double > get_time_grid_from_namelist( const QStringList & files );

      QString last_load_dir;
      bool    save_files( QStringList files );
      bool    save_file ( QString file, bool &overwrite, bool &cancel );
      bool    save_files_csv( QStringList files );

      csv                          csv_conc;
      US_Hydrodyn_Mals_Saxs_Conc *conc_window;
      bool                         conc_widget;
      void                         update_csv_conc();
      bool                         all_selected_have_nonzero_conc();

      void                         delete_zoomer_if_ranges_changed();
      QString                      vector_double_to_csv( vector < double > vd );
      static QStringList           vector_qstring_to_qstringlist( vector < QString > vqs );

      bool                         adjacent_ok( QString name );

      void                         avg     ( QStringList files, QString suffix = "" );
      void                         conc_avg( QStringList files );
      void                         bin( QStringList files );
      void                         smooth( QStringList files );
      void                         repeak( QStringList files );
      void                         create_i_of_t( QStringList files );
      bool                         create_i_of_q( QStringList files,
                                                  double t_min = -1e99,
                                                  double t_max = 1e99 );
      bool                         create_i_of_q( set < QString > &fileset,
                                                  double t_min = -1e99,
                                                  double t_max = 1e99 );
      bool                         create_i_of_q_ng( QStringList files,
                                                     double t_min = -1e99,
                                                     double t_max = 1e99 );
      bool                         create_i_of_q_ng( set < QString > & fileset,
                                                     double t_min = -1e99,
                                                     double t_max = 1e99 );
      bool                         create_ihash_t( QStringList files );
      bool                         create_istar_q(
                                                  QStringList files
                                                  ,double t_min = -1e99
                                                  ,double t_max = 1e99
                                                  );
      bool                         create_istar_q_ng(
                                                     QStringList files
                                                     ,double t_min = -1e99
                                                     ,double t_max = 1e99
                                                     );

      QString                      last_created_file;
      void                         zoom_info();
      void                         clear_files( QStringList files, bool quiet = false );
      void                         to_created( QString file );
      void                         add_files( QStringList files );
      bool                         axis_x_log;
      bool                         axis_y_log;
      bool                         compatible_files( QStringList files );
      bool                         type_files( QStringList files );
      bool                         get_peak( QString file, double &peak, bool full = false );
      bool                         get_peak( QString file, double &peak, double &pos, bool full = false );

      QString                      wheel_file;
      QwtPlotCurve *               wheel_curve;
      void                         disable_all();

      // bool                         gaussian_mode;
      // bool                         ggaussian_mode;
      // bool                         baseline_mode;
      // bool                         timeshift_mode;

      enum                         modes
      {
         MODE_NORMAL
         ,MODE_GAUSSIAN
         ,MODE_GGAUSSIAN
         ,MODE_BLANKS
         ,MODE_BASELINE
         ,MODE_TIMESHIFT
         ,MODE_SCALE
         ,MODE_RGC
         ,MODE_PM
         ,MODE_GUINIER
         ,MODE_TESTIQ
         ,MODE_WYATT
         ,MODE_SCROLL_PAIR
         ,MODE_SCALE_PAIR
      };

      modes                        current_mode;
      void                         mode_select();
      void                         mode_title( QString title );
      void                         mode_select( modes mode );
      void                         mode_setup_widgets();

      void                         gaussian_enables();
      void                         ggaussian_enables();

      void                         gauss_add_marker( double pos, 
                                                     QColor color, 
                                                     QString text, 
                                                     Qt::Alignment
                                                     align
                                                     = Qt::AlignRight | Qt::AlignTop );
      void                         gauss_init_markers();
      void                         gauss_delete_markers();

      void                         gauss_add_gaussian( double * g, QColor color );
      void                         gauss_init_gaussians();
      void                         gauss_delete_gaussians();
      void                         gauss_replot_gaussian();
      
      double                       gauss_max_height;

      mQLineEdit                   *le_last_focus;

      void                         blanks_enables();
      QStringList                  default_blanks;
      QStringList                  default_blanks_files;
      set < QString >              default_blanks_set;
      set < QString >              blanks_temporary_load_set;

      QStringList                  baseline_blanks;
      bool                         blanks_in_baseline_mode;

      QStringList                  blanks_save_selected;
      set < QString >              blanks_save_selected_set;

      QStringList                  blanks_selected;
      QStringList                  blanks_created;
      vector < double >            blanks_created_q;
      set < QString >              blanks_created_set;
      set < QString >              blanks_selected_set;
      bool                         blanks_axis_y_was_log;

      map < QString, QString >     blanks_last_cormap_parameters;
      vector < vector < double > > blanks_last_cormap_pvaluepairs;
      map < QString, double >      blanks_last_brookesmap_sliding_results;
      double                       blanks_last_cormap_run_end_s;
      double                       blanks_last_cormap_run_end_e;

      vector < double >            blanks_last_q_range;

      map < QString, QString >     blanks_cormap_parameters;
      vector < vector < double > > blanks_cormap_pvaluepairs;
      map < QString, double >      blanks_brookesmap_sliding_results;

      int                          baseline_max_window_size;
      int                          baseline_best_last_window_size;
      bool                         baseline_best_ok;
      bool                         baseline_ready_to_apply;

      double                       blanks_end_s;
      double                       blanks_end_e;

      void                         baseline_enables();
      void                         baseline_init_markers();
      void                         replot_baseline( QString qs = "" );
      void                         replot_baseline_integral();
      map < double, int >          wheel_double_to_pos;
      int                          input_double_to_pos( double d );

      bool                         ask_to_decimate( map < QString, QString > & parameters );
      bool                         ask_cormap_minq( map < QString, QString > & parameters );

      bool                         baseline_multi;
      bool                         baseline_integral;
      QStringList                  baseline_selected;
      set < QString >              baseline_selected_set;
      double                       baseline_intercept;
      double                       baseline_slope;

      set < QString >              baseline_test_added_set;
      vector < QString >           baseline_apply_created;

      bool                         org_baseline_start_zero;
      double                       org_baseline_start_s;
      double                       org_baseline_start;
      double                       org_baseline_start_e;
      double                       org_baseline_end_s;
      double                       org_baseline_end;
      double                       org_baseline_end_e;
      void                         baseline_apply( QStringList files, 
                                                   bool integral = false, 
                                                   int smoothing = 0,
                                                   bool save_bl = false,
                                                   unsigned int reps = 1,
                                                   bool do_replot = true,
                                                   bool quiet = false );

      map < QString, QString >     baseline_last_cormap_parameters;
      vector < vector < double > > baseline_last_cormap_pvaluepairs;
      // map < QString, double >      baseline_last_brookesmap_sliding_results;
      double                       baseline_last_cormap_run_end_s;
      double                       baseline_last_cormap_run_end_e;

      map < QString, QString >     baseline_cormap_parameters;
      vector < vector < double > > baseline_cormap_pvaluepairs;
      map < QString, double >      baseline_brookesmap_sliding_results;
      double                       baseline_cormap_run_end_s;
      double                       baseline_cormap_run_end_e;
      bool                         baseline_hb_only_mode;

      bool                         baseline_test_mode;
      void                         baseline_test_pos( int pos );

      bool                         compute_f_gaussians( QString file, QWidget *mals_saxs_fit_widget );
      bool                         compute_f_gaussians_trial( QString file, QWidget *mals_saxs_fit_widget );

      bool                         ggauss_recompute();

      US_Saxs_Util                 *usu;

      void                         push_back_color_if_ok( QColor bg, QColor set );

      QString                      pad_zeros( int val, int max );
      void                         plot_errors_jump_markers();
      bool                         check_fit_range();

      mals_saxs_stack_data              current_data( bool selected_only = false );
      mals_saxs_stack_data              clipboard;
      void                         set_current_data( mals_saxs_stack_data & data );

      bool                         errors_were_on;

      void                         redo_plot_errors();

      void                         stack_join( mals_saxs_stack_data & );

      bool                         all_have_f_gaussians( QStringList & files );

      bool                         detector_uv;
      double                       detector_uv_conv;

      bool                         detector_ri;
      double                       detector_ri_conv;

      bool                         adjacent_select( QListWidget *lb, QString match_pattern );

      enum                         gaussian_types
         {
            GAUSS,
            EMG,
            GMG,
            EMGGMG
         };

      gaussian_types               gaussian_type;
      int                          gaussian_type_size;
      QString                      gaussian_type_tag;

      void                         update_gauss_mode();

      bool                         dist1_active;
      bool                         dist2_active;

      bool                         unified_ggaussian_to_f_gaussians();

      unsigned int                 common_size;
      unsigned int                 per_file_size;

      vector < bool >              is_common; // is common maps the offsets to layout of the regular file specific gaussians
      vector < unsigned int >      offset;

      unsigned int                 use_line_width;

      QPalette                     cg_red;

      void                         errors_multi_file( QStringList files );

      bool                         compatible_grids( QStringList files );

      bool                         suppress_replot;
      bool                         suppress_plot;

      void                         update_ref();

      double                       tot_intensity( QString &file, double q_min = 0e0, double q_max = 6e0 );

      QString                      scale_get_target( bool do_msgs = false );
      bool                         scale_applied;
      double                       scale_applied_q_min;
      double                       scale_applied_q_max;

      set < QString >              scale_last_created;
      bool                         set_selected        ( const QStringList &, bool do_replot = true );
      bool                         set_selected        ( const set < QString > &, bool do_replot = true );
      bool                         set_created_selected( const set < QString > &, bool do_replot = true );

      map < QString, vector <double > > scale_q;
      map < QString, vector <double > > scale_I;
      map < QString, vector <double > > scale_e;
      QString                      scale_applied_target;
      void                         scale_update_plot_errors();

      vector < double >            scale_spline_x;
      vector < double >            scale_spline_y;
      vector < double >            scale_spline_y2;

      void                         scale_replot();

      map < QString, QwtPlotCurve * >     scale_plotted_errors;

      bool                         check_zi_window         ( QStringList & files, const QString & extra_text = "" );
      void                         check_discard_it_sd_mult( QStringList & files, bool optionally_discard = false );

      bool                         started_in_expert_mode;

      bool                         height_wheel_file( double &val, double pos );  // computes local height in gauss mode

      vector < double >            height_natural_spline_x;
      vector < double >            height_natural_spline_y;
      vector < double >            height_natural_spline_y2;

      QString                      describe_unified_common;
      QString                      describe_unified_per_file;

      void                         ggauss_msg_common( vector < map < int, set < double > > > & check_common );
      QStringList                  gaussian_param_text;

      QString                      select_conc_file( QString tag );

      void                         clear_gaussians();
      bool                         any_gaussians();

      double                       wyatt_errors( QString file,
                                                 double start_q,
                                                 double end_q );

      double                       wyatt_errors( QString file,
                                                 double start_q,
                                                 double end_q,
                                                 vector < double > & new_q,
                                                 vector < double > & new_I,
                                                 vector < double > & new_y );

      double                       wyatt_errors( const vector < double > & q,
                                                 const vector < double > & I,
                                                 double start_q,
                                                 double end_q );
      double                       wyatt_errors( const vector < double > & q,
                                                 const vector < double > & I,
                                                 double start_q,
                                                 double end_q,
                                                 vector < double > & new_q,
                                                 vector < double > & new_I,
                                                 vector < double > & new_y
                                                 );
      bool                         constant_e( QString file, double e );
      int                          slice( const vector < double > & q,
                                          const vector < double > & I,
                                          double start_q,
                                          double end_q,
                                          vector < double > & new_q,
                                          vector < double > & new_I
                                          );
      void                         wyatt_apply( const QStringList & files );

      void                         timescale  ( const QStringList & files );

      map < QString, QwtPlot *>    plot_info;
      map < QwtPlot *, ScrollZoomer * >    plot_to_zoomer;
      map < QwtPlot *, double >    plot_limit_x_range_min;
      map < QwtPlot *, double >    plot_limit_x_range_max;

      QStringList                  get_frames( QStringList files, QString head, QString tail );
      map < QString, QString >     ldata;

      void                         wheel_enables( bool enabled = true );

      void                         cormap( map < QString, QString > & parameters );

      // common time
      bool                         mals_params_interpolate( const vector < double > & times );
      bool                         mals_params_interpolate( const QStringList & filenames );
      bool                         mals_params_interpolate( const vector < double > & times, const QString & filename );

   signals:
      void do_resize_plots();
      void do_resize_guinier_plots();

   private slots:

      void setupGUI();

      void resize_plots();
      void resize_guinier_plots();

      void color_rotate();
      void line_width();
      void dir_pressed();
      void created_dir_pressed();

      void hide_files();
      void hide_created_files();

      void update_enables();
      void update_files();
      void update_created_files();
      void add_files( bool load_conc = false, bool from_dir = false );
      void add_dir();
      void similar_files();
      void conc();
      void clear_files();
      void regex_load();

      void select_all();
      void select_nth();
      void invert();
      void join();
      void adjacent();
      void to_saxs();
      void view();
      void movie();
      void set_eb();
      void set_dots();
      void rescale();
      void rescale_y();
      void rescale_y_plot_errors();
      void conc_avg();
      void normalize();
      void add();
      void avg();
      void bin();
      void smooth();
      void svd();
      void common_time();
      void scroll_pair();
      void join_by_time();
      void repeak();
      void create_i_of_t();
      void create_ihash_t();
      void create_istar_q();
      void test_i_of_t();
      void create_i_of_q();
      void load_conc();
      void set_conc_file( QString file = "" );
      void set_detector();
      void set_mals_saxs();
      void set_empty();
      void set_signal();
      void select_all_created();
      void invert_all_created();
      void adjacent_created();
      void remove_created();
      void save_created_csv();
      void save_created();
      void show_created();
      void show_only_created();

      void stack_push_all();
      void stack_push_sel();
      void stack_copy();
      void stack_pcopy();
      void stack_paste();
      void stack_clear( );
      void stack_drop();
      void stack_join();
      void stack_rot_up();
      void stack_rot_down();
      void stack_swap();

      void timeshift();
      void timescale();
      void p3d();
      void ref();
      void errors();
      void ggqfit();
      void cormap();
      void pp();
      void wheel_cancel( bool from_wheel_save = false );
      void wheel_save();

      void clear_display();
      void update_font();
      void save();
      void hide_editor();

      void help();
      void options();
      void cancel();

      void plot_zoomed( const QRectF &rect );
      void plot_mouse ( const QMouseEvent &me );

      void plot_errors_zoomed( const QRectF &rect );

      void adjust_wheel ( double );

      void gauss_mode();
      void gauss_start();
      void gauss_clear( );
      void gauss_new();
      void gauss_delete();
      void gauss_prev();
      void gauss_next();
      void gauss_fit();
      void gauss_save();
      void gauss_local_caruanas();
      void gauss_local_guos();
      void gauss_pos_text              ( const QString & );
      void gauss_pos_width_text        ( const QString & );
      void gauss_pos_height_text       ( const QString & );
      void gauss_pos_dist1_text        ( const QString & );
      void gauss_pos_dist2_text        ( const QString & );
      void gauss_fit_start_text        ( const QString & );
      void gauss_fit_end_text          ( const QString & );
      void gauss_match_amplitude       ();

      void gauss_pos_focus             ( bool );
      void gauss_pos_width_focus       ( bool );
      void gauss_pos_height_focus      ( bool );
      void gauss_pos_dist1_focus       ( bool );
      void gauss_pos_dist2_focus       ( bool );
      void gauss_fit_start_focus       ( bool );
      void gauss_fit_end_focus         ( bool );

      void ggauss_start                ();
      void ggauss_rmsd                 ();
      void ggauss_results              ();

      void ggauss_scroll               ();
      void ggauss_scroll_p_green       ();
      void ggauss_scroll_p_yellow      ();
      void ggauss_scroll_p_red         ();
      void ggauss_scroll_smoothed      ();
      void ggauss_scroll_oldstyle      ();

      bool gg_fit_replot               ();

      void set_sd_weight               ();
      void set_fix_width               ();
      void set_fix_dist1               ();
      void set_fix_dist2               ();

      void gauss_as_curves             ();

      void wyatt_start                 ();
      void wyatt_apply                 ();
      void wyatt_2                     ();
      void wyatt_start_text            ( const QString & );
      void wyatt_start_focus           ( bool );
      void wyatt_end_text              ( const QString & );
      void wyatt_end_focus             ( bool );
      void wyatt_start2_text           ( const QString & );
      void wyatt_start2_focus          ( bool );
      void wyatt_end2_text             ( const QString & );
      void wyatt_end2_focus            ( bool );

      void blanks_start                ();
      bool blanks_params               ();
      void bb_cm_inc                   ();

      void baseline_start              ( bool from_blanks_mode_save = false );
      void baseline_test               ();
      void baseline_apply              ();
      void baseline_best               ();

      void set_baseline_start_zero     ();

      void baseline_start_s_text       ( const QString & );
      void baseline_start_text         ( const QString & );
      void baseline_start_e_text       ( const QString & );
      void baseline_end_s_text         ( const QString & );
      void baseline_end_text           ( const QString & );
      void baseline_end_e_text         ( const QString & );
      void set_baseline_fix_width      ();
      void baseline_width_text         ( const QString & );
      void baseline_start_s_focus      ( bool );
      void baseline_start_focus        ( bool );
      void baseline_start_e_focus      ( bool );
      void baseline_end_s_focus        ( bool );
      void baseline_end_focus          ( bool );
      void baseline_end_e_focus        ( bool );
      void baseline_width_focus        ( bool );

      void scale                       ();
      void scale_q_start_text          ( const QString & );
      void scale_q_start_focus         ( bool );
      void scale_q_end_text            ( const QString & );
      void scale_q_end_focus           ( bool );
      void scale_q_reset               ();
      void scale_apply                 ();
      void scale_reset                 ();
      void scale_create                ();
      void scale_enables               ();
      void scale_scroll                ();

      void guinier                     ();
      void guinier_replot              ();
      void guinier_q_start_text        ( const QString & );
      void guinier_q_start_focus       ( bool );
      void guinier_q_end_text          ( const QString & );
      void guinier_q_end_focus         ( bool );
      void guinier_q2_start_text       ( const QString & );
      void guinier_q2_start_focus      ( bool );
      void guinier_q2_end_text         ( const QString & );
      void guinier_q2_end_focus        ( bool );
      void guinier_delta_start_text    ( const QString & );
      void guinier_delta_start_focus   ( bool );
      void guinier_delta_end_text      ( const QString & );
      void guinier_delta_end_focus     ( bool );
      void guinier_qrgmax_text         ( const QString & );
      void guinier_qrgmax              ();
      void guinier_sd                  ();
      void guinier_enables             ();
      void guinier_residuals_update    ();
      void guinier_plot_rg_toggle      ();
      void guinier_plot_mw_toggle      ();
      void guinier_rg_t_start_text     ( const QString & );
      void guinier_rg_t_start_focus    ( bool );
      void guinier_rg_t_end_text       ( const QString & );
      void guinier_rg_t_end_focus      ( bool );
      void guinier_rg_rg_start_text    ( const QString & );
      void guinier_rg_rg_start_focus   ( bool );
      void guinier_rg_rg_end_text      ( const QString & );
      void guinier_rg_rg_end_focus     ( bool );
      void guinier_mw_t_start_text     ( const QString & );
      void guinier_mw_t_start_focus    ( bool );
      void guinier_mw_t_end_text       ( const QString & );
      void guinier_mw_t_end_focus      ( bool );
      void guinier_mw_mw_start_text    ( const QString & );
      void guinier_mw_mw_start_focus   ( bool );
      void guinier_mw_mw_end_text      ( const QString & );
      void guinier_mw_mw_end_focus     ( bool );
      void guinier_scroll              ();
      bool guinier_check_qmax          ( QString & report, bool show_message = true );
      bool guinier_check_qmax          ( bool show_message = true );

      void testiq                       ();
      void testiq_q_start_text         ( const QString & );
      void testiq_q_start_focus        ( bool );
      void testiq_q_end_text           ( const QString & );
      void testiq_q_end_focus          ( bool );
      void testiq_visrange             ();
      void testiq_testset              ();
      void testiq_gauss_line           ();
      void testiq_enables              ();

      void select_vis                  ();
      void remove_vis                  ();
      void crop_left                   ();
      void crop_common                 ();
      void crop_vis                    ();
      void crop_to_vis                 ();
      void crop_zero                   ();
      void crop_undo                   ();
      void crop_right                  ();
      void legend                      ();
      void axis_x                      ( bool nochange = false, bool no_replot = false );
      void axis_y                      ( bool nochange = false, bool no_replot = false );
      void legend_set                  ();

      void rename_created              ( QListWidgetItem *, const QPoint & );
      void rename_from_context         ( const QPoint & pos );

      void set_plot_errors_rev();
      void set_plot_errors_sd();
      void set_plot_errors_pct();
      void set_plot_errors_group();

      void rgc                      ();
      void rgc_enables              ();
      void rgc_mw_text              ( const QString & );
      void rgc_vol_text             ( const QString & );
      void rgc_rho_text             ( const QString & );
      void rgc_axis_b_text          ( const QString &, bool do_recompute = true );
      void rgc_axis_c_text          ( const QString &, bool do_recompute = true );
      void rgc_rg_text              ( const QString & );
      void rgc_shape                ();

      void simulate                 ();

      void pm                       ();
      void pm_enables               ();
      void pm_q_start_text          ( const QString & );
      void pm_q_end_text            ( const QString & );
      void pm_q_start_focus         ( bool );
      void pm_q_end_focus           ( bool );
      void pm_q_reset               ();

      void pm_samp_e_dens_text      ( const QString & );
      void pm_buff_e_dens_text      ( const QString & );
      void pm_grid_size_text        ( const QString & );
      void pm_q_pts_text            ( const QString & );

      void pm_run                   ();

      void save_state();

      void model_select_all         ();
      void model_save               ();
      void model_view               ();
      void model_text               ();
      void model_remove             ();

      void q_exclude_vis            ();
      void q_exclude_left           ();
      void q_exclude_right          ();
      void q_exclude_clear          ();

      void artificial_gaussians     ();

      void check_mwt_constants      ( bool force = false );

      void wheel_inc                ();
      void wheel_dec                ();

      void wheel_pressed            ();
      void wheel_released           ();

   protected slots:

      void closeEvent(QCloseEvent *);
   
};

#define UHSH_WHEEL_RES 10000000
#define Q_VAL_TOL 5e-6
#define UHSH_SHOW_DIGITS 6
#define UHSH_VAL_DEC 20
#endif
