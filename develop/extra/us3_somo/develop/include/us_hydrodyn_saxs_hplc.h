#ifndef US_HYDRODYN_SAXS_HPLC_H
#define US_HYDRODYN_SAXS_HPLC_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <q3frame.h>
#include <qcheckbox.h>
#include <q3textedit.h>
#include <q3progressbar.h>
#include <qmenubar.h>
#include <qfileinfo.h>
#include <qprinter.h>
#include <qradiobutton.h>
#include <q3table.h>
#include <qwt_plot_zoomer.h>
#include <qwt_wheel.h>
//Added by qt3to4:
#include <Q3BoxLayout>
#include <QMouseEvent>
#include <QCloseEvent>
#include "../3dplot/mesh2mainwindow.h"

#ifdef QT4
#include "qwt_plot_marker.h"
#include "qwt_symbol.h"
#endif

#include "us_util.h"

//standard C and C++ defs:

#include <map>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "us_hydrodyn_saxs.h"
#include "us_hydrodyn_saxs_hplc_conc.h"
#include "qwt/scrollbar.h"
#include "qwt/scrollzoomer.h"
#include "us_saxs_util.h"

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

class ga_individual
{
 public:
   vector < double > v;
   double            fitness;
   bool operator < (const ga_individual & objIn) const
   {
      return fitness < objIn.fitness;
   }

   bool operator == (const ga_individual & objIn) const
   {
      return v == objIn.v;
   }
   bool operator != (const ga_individual & objIn) const
   {
      return v != objIn.v;
   }
};

struct hplc_stack_data
{
#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( disable: 4251 )
# endif
#endif
   map < QString, vector < QString > > f_qs_string;
   map < QString, vector < double > >  f_qs;
   map < QString, vector < double > >  f_Is;
   map < QString, vector < double > >  f_errors;
   map < QString, vector < double > >  f_gaussians;
   map < QString, unsigned int >       f_pos;

   map < QString, QString >            f_name;
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

#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( default: 4251 )
# endif
#endif
};

class US_EXTERN US_Hydrodyn_Saxs_Hplc : public Q3Frame
{
   Q_OBJECT

      friend class US_Hydrodyn_Batch;
      friend class US_Hydrodyn_Saxs;
      friend class US_Hydrodyn_Saxs_Hplc_Conc;
      friend class US_Hydrodyn_Saxs_Hplc_Fit;
      friend class US_Hydrodyn_Saxs_Hplc_Fit_Global;
      friend class US_Hydrodyn_Saxs_Hplc_Options;
      friend class US_Hydrodyn_Saxs_Hplc_Nth;
      friend class US_Hydrodyn_Saxs_Hplc_Svd;
      friend class US_Hydrodyn_Saxs_Hplc_Movie;

   public:
      US_Hydrodyn_Saxs_Hplc(
                              csv csv1,
                              void *us_hydrodyn, 
                              QWidget *p = 0, 
                              const char *name = 0
                             );
      ~US_Hydrodyn_Saxs_Hplc();

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

   private:
      Mesh2MainWindow *plot3d_window;
      bool             plot3d_flag;

      csv           csv1;

      void          *us_hydrodyn;

      US_Config     *USglobal;

      QLabel        *lbl_title;

      Q3ProgressBar  *progress;

      mQLabel       *lbl_files;
      QCheckBox     *cb_lock_dir;
      mQLabel       *lbl_dir;
      QPushButton   *pb_add_files;
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
      QPushButton   *pb_rescale;

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

      Q3ListBox      *lb_files;
      QLabel        *lbl_selected;
      // QPushButton   *pb_plot_files;
      QPushButton   *pb_conc_avg;
      QPushButton   *pb_normalize;
      QPushButton   *pb_add;
      QPushButton   *pb_avg;
      QPushButton   *pb_smooth;
      QPushButton   *pb_repeak;
      QPushButton   *pb_svd;
      QPushButton   *pb_create_i_of_t;
      QPushButton   *pb_create_i_of_q;


      QPushButton   *pb_conc_file;
      QLabel        *lbl_conc_file;

      QPushButton   *pb_detector;

      QPushButton   *pb_set_hplc;
      QLabel        *lbl_hplc;

      QPushButton   *pb_set_empty;
      QLabel        *lbl_empty;

      QPushButton   *pb_set_signal;
      QLabel        *lbl_signal;

      QLabel        *lbl_created_files;
      mQLabel       *lbl_created_dir;
      Q3ListBox      *lb_created_files;
      QLabel        *lbl_selected_created;

      QPushButton   *pb_select_all_created;
      QPushButton   *pb_invert_all_created;
      QPushButton   *pb_adjacent_created;
      QPushButton   *pb_remove_created;
      QPushButton   *pb_save_created_csv;
      QPushButton   *pb_save_created;
      QPushButton   *pb_show_created;
      QPushButton   *pb_show_only_created;

      mQLabel     * lbl_editor;
      QFont         ft;
      Q3TextEdit     *editor;
      QMenuBar      *m;

      QPushButton   *pb_help;
      QPushButton   *pb_options;
      QPushButton   *pb_cancel;

      QwtPlot       *plot_dist;
      ScrollZoomer  *plot_dist_zoomer;
#ifdef QT4
      QwtPlotGrid   *grid_saxs;
      bool          legend_vis;
#endif

      QwtPlot       *plot_errors;
      ScrollZoomer  *plot_errors_zoomer;
#ifdef QT4
      QwtPlotGrid   *grid_errors;
#endif
      QwtPlot       *plot_ref;
#ifdef QT4
      QwtPlotGrid   *grid_ref;
#endif

      QCheckBox     *cb_plot_errors_rev;
      QCheckBox     *cb_plot_errors_sd;
      QCheckBox     *cb_plot_errors_pct;
      QCheckBox     *cb_plot_errors_group;

      Q3BoxLayout    *l_plot_errors;

      QPushButton   *pb_wheel_start;
      QPushButton   *pb_p3d;
      QLabel        *lbl_wheel_pos;
      QwtWheel      *qwtw_wheel;
      QPushButton   *pb_ref;
      QPushButton   *pb_errors;
      QPushButton   *pb_wheel_cancel;
      QPushButton   *pb_wheel_save;

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

      QPushButton   *pb_ggauss_start;
      QPushButton   *pb_ggauss_rmsd;
      QPushButton   *pb_ggauss_results;

      QPushButton   *pb_gauss_as_curves;

      QPushButton   *pb_baseline_start;
      mQLineEdit    *le_baseline_start_s;
      mQLineEdit    *le_baseline_start;
      mQLineEdit    *le_baseline_start_e;
      mQLineEdit    *le_baseline_end_s;
      mQLineEdit    *le_baseline_end;
      mQLineEdit    *le_baseline_end_e;
      QPushButton   *pb_baseline_apply;

      QPushButton   *pb_line_width;
      QPushButton   *pb_color_rotate;

      QPushButton   *pb_select_vis;
      QPushButton   *pb_remove_vis;
      QPushButton   *pb_crop_zero;
      QPushButton   *pb_crop_vis;
      QPushButton   *pb_crop_common;
      QPushButton   *pb_crop_left;
      QPushButton   *pb_crop_undo;
      QPushButton   *pb_crop_right;
      QPushButton   *pb_legend;
      QPushButton   *pb_axis_x;
      QPushButton   *pb_axis_y;

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

#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( disable: 4251 )
# endif
#endif
      vector < double >                  plot_errors_grid;
      vector < double >                  plot_errors_target;
      vector < double >                  plot_errors_fit;
      vector < double >                  plot_errors_errors;

      vector < hplc_stack_data >         stack_data;

      vector < QColor >                  plot_colors;

      map < QString, vector < QString > > f_qs_string;
      map < QString, vector < double > >  f_qs;
      map < QString, vector < double > >  f_Is;
      map < QString, vector < double > >  f_errors;
      map < QString, vector < double > >  f_gaussians;
      map < QString, unsigned int >       f_pos;

      map < QString, QString >            f_name;
      map < QString, bool >               f_is_time;
      map < QString, double >             f_psv;
      map < QString, double >             f_I0se;
      map < QString, double >             f_conc;
      map < QString, double >             f_extc;
      map < QString, double >             f_time;

      map < QString, bool >               created_files_not_saved;

      map < QString, double >             current_concs( bool quiet = false );
      map < QString, double >             window_concs();

      vector < crop_undo_data >           crop_undos;

      bool                                is_nonzero_vector( vector < double > &v );
      bool                                is_zero_vector( vector < double > &v );

      vector < double >                   union_q( QStringList files );

#ifdef QT4
      map < QString, QwtPlotCurve * >     plotted_curves;
      vector < QwtPlotMarker * >          plotted_markers;
      vector < QwtPlotCurve * >           plotted_gaussians;
      vector < QwtPlotCurve * >           plotted_gaussian_sum;
      vector < QwtPlotCurve * >           plotted_baseline;
      vector < QwtPlotCurve * >           plotted_hlines;
      QwtPlotMarker *                     ref_marker;
#else
      map < QString, long >               plotted_curves;
      vector < long >                     plotted_markers;
      vector < long >                     plotted_gaussians;
      vector < long >                     plotted_gaussian_sum;
      vector < long >                     plotted_baseline;
      vector < long >                     plotted_hlines;
      long                                ref_marker;
#endif
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

      vector < unsigned int >             unified_ggaussian_q_start;        // the start q pos - one for each curve
      vector < unsigned int >             unified_ggaussian_q_end;          // the end q pos   - one for each curve

      map < QString, vector < double > >  org_f_gaussians;

      vector < double >                   compute_gaussian( vector < double > t, vector < double > g );
      vector < double >                   compute_ggaussian_gaussian_sum();
      void                                add_ggaussian_curve( QString name, vector < double > y );
      void                                gauss_add_hline( double center, double width );

      map < QString, bool >               all_files_map();

      void                                update_plot_errors( vector < double > &grid, 
                                                              vector < double > &target, 
                                                              vector < double > &fit,
                                                              vector < double > &errors );
      void                                update_plot_errors_group();

      void                                hide_widgets( vector < QWidget *> widgets, bool hide );

      vector < QWidget * >                plot_errors_widgets;
      vector < QWidget * >                files_widgets;
      vector < QWidget * >                files_expert_widgets;
      vector < QWidget * >                created_files_widgets;
      vector < QWidget * >                created_files_expert_widgets;
      vector < QWidget * >                editor_widgets;

      vector < double >                   conc_curve( vector < double > &t,
                                                      unsigned int peak,
                                                      double conv );
#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( default: 4251 )
# endif
#endif
      unsigned int                        unified_ggaussian_curves;
      unsigned int                        unified_ggaussian_gaussians_size;
      QStringList                         unified_ggaussian_files;
      bool                                unified_ggaussian_use_errors;
      bool                                unified_ggaussian_errors_skip;
      bool                                unified_ggaussian_ok;

      bool                                initial_ggaussian_fit( QStringList & files );
      bool                                create_unified_ggaussian_target( bool do_init = true );
      bool                                create_unified_ggaussian_target( QStringList & files, bool do_init = true );
      double                              ggaussian_rmsd();
      bool                                ggaussian_compatible( bool check_against_global = true );
      bool                                ggaussian_compatible( QStringList & files, bool check_against_global = true );

      unsigned int                        gaussian_pos;
      void                                update_gauss_pos();
      void                                plot_gaussian_sum();
      void                                replot_gaussian_sum();

      bool                                wheel_errors_ok;

      bool                                opt_repeak_gaussians( QString file );

      QStringList                         all_selected_files();
      QStringList                         all_files();
      QString                             last_selected_file;

      void save_csv_saxs_iqq();

      csv  current_csv();

      bool load_file( QString file );

      void plot_files();
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

      QString last_load_dir;
      bool    save_files( QStringList files );
      bool    save_file ( QString file, bool &overwrite, bool &cancel );
      bool    save_files_csv( QStringList files );

      csv                          csv_conc;
      US_Hydrodyn_Saxs_Hplc_Conc *conc_window;
      bool                         conc_widget;
      void                         update_csv_conc();
      bool                         all_selected_have_nonzero_conc();

      void                         delete_zoomer_if_ranges_changed();
      QString                      vector_double_to_csv( vector < double > vd );

      bool                         adjacent_ok( QString name );

      void                         avg     ( QStringList files );
      void                         conc_avg( QStringList files );
      void                         smooth( QStringList files );
      void                         repeak( QStringList files );
      void                         create_i_of_t( QStringList files );
      void                         create_i_of_q( QStringList files );
      void                         create_i_of_q_ng( QStringList files );
      QString                      last_created_file;
      void                         zoom_info();
      void                         clear_files( QStringList files );
      void                         to_created( QString file );
      void                         add_files( QStringList files );
      bool                         axis_x_log;
      bool                         axis_y_log;
      bool                         compatible_files( QStringList files );
      bool                         type_files( QStringList files );
      bool                         get_peak( QString file, double &peak );

      QString                      wheel_file;
#ifdef QT4
      QwtPlotCurve *               wheel_curve;
#else
      long                         wheel_curve;
#endif
      void                         disable_all();

      bool                         gaussian_mode;
      bool                         ggaussian_mode;
      bool                         baseline_mode;
      bool                         timeshift_mode;

      void                         gaussian_enables();
      void                         ggaussian_enables();

      void                         gauss_add_marker( double pos, 
                                                     QColor color, 
                                                     QString text, 
#ifndef QT4
                                                     int 
#else
                                                     Qt::Alignment
#endif
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

      void                         baseline_enables();
      void                         baseline_init_markers();
      void                         replot_baseline();

      double                       baseline_intercept;
      double                       baseline_slope;
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
                                                   unsigned int reps = 1 );

      bool                         compute_f_gaussians( QString file, QWidget *hplc_fit_widget );

      bool                         ggauss_recompute();

      US_Saxs_Util                 *usu;

      void                         push_back_color_if_ok( QColor bg, QColor set );

      QString                      pad_zeros( int val, int max );
      void                         plot_errors_jump_markers();
      bool                         check_fit_range();

      hplc_stack_data              current_data( bool selected_only = false );
      hplc_stack_data              clipboard;
      void                         set_current_data( hplc_stack_data & data );

      bool                         errors_were_on;

      void                         redo_plot_errors();

      void                         stack_join( hplc_stack_data & );

      bool                         all_have_f_gaussians( QStringList & files );

      bool                         detector_uv;
      double                       detector_uv_conv;

      bool                         detector_ri;
      double                       detector_ri_conv;

      bool                         adjacent_select( Q3ListBox *lb, QString match_pattern );

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

#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( disable: 4251 )
# endif
#endif
      vector < bool >              is_common; // is common maps the offsets to layout of the regular file specific gaussians
      vector < unsigned int >      offset;
#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( default: 4251 )
# endif
#endif
      unsigned int                 use_line_width;

      QColorGroup                  cg_red;

      void                         errors_multi_file( QStringList files );

      bool                         compatible_grids( QStringList files );

      bool                         suppress_replot;

      void                         update_ref();

   private slots:

      void setupGUI();

      void color_rotate();
      void line_width();
      void dir_pressed();
      void created_dir_pressed();

      void hide_files();
      void hide_created_files();

      void update_enables();
      void update_files();
      void update_created_files();
      void add_files();
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
      void rescale();
      void conc_avg();
      void normalize();
      void add();
      void avg();
      void smooth();
      void svd();
      void repeak();
      void create_i_of_t();
      void create_i_of_q();
      void set_conc_file();
      void set_detector();
      void set_hplc();
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
      void stack_clear();
      void stack_drop();
      void stack_join();
      void stack_rot_up();
      void stack_rot_down();
      void stack_swap();

      void wheel_start();
      void p3d();
      void ref();
      void errors();
      void wheel_cancel();
      void wheel_save();

      void clear_display();
      void update_font();
      void save();
      void hide_editor();

      void help();
      void options();
      void cancel();

      void plot_zoomed( const QwtDoubleRect &rect );
      void plot_mouse ( const QMouseEvent &me );

      void plot_errors_zoomed( const QwtDoubleRect &rect );

      void adjust_wheel ( double );

      void gauss_start();
      void gauss_clear();
      void gauss_new();
      void gauss_delete();
      void gauss_prev();
      void gauss_next();
      void gauss_fit();
      void gauss_save();
      void gauss_pos_text              ( const QString & );
      void gauss_pos_width_text        ( const QString & );
      void gauss_pos_height_text       ( const QString & );
      void gauss_pos_dist1_text        ( const QString & );
      void gauss_pos_dist2_text        ( const QString & );
      void gauss_fit_start_text        ( const QString & );
      void gauss_fit_end_text          ( const QString & );

      void gauss_pos_focus             ( bool );
      void gauss_pos_width_focus       ( bool );
      void gauss_pos_height_focus      ( bool );
      void gauss_pos_dist1_focus       ( bool );
      void gauss_pos_dist2_focus       ( bool );
      void gauss_fit_start_focus       ( bool );
      void gauss_fit_end_focus         ( bool );

      void ggauss_start();
      void ggauss_rmsd();
      void ggauss_results();

      void set_sd_weight();
      void set_fix_width();
      void set_fix_dist1();
      void set_fix_dist2();

      void gauss_as_curves();

      void baseline_start();
      void baseline_apply();

      void baseline_start_s_text       ( const QString & );
      void baseline_start_text         ( const QString & );
      void baseline_start_e_text       ( const QString & );
      void baseline_end_s_text         ( const QString & );
      void baseline_end_text           ( const QString & );
      void baseline_end_e_text         ( const QString & );
      void baseline_start_s_focus      ( bool );
      void baseline_start_focus        ( bool );
      void baseline_start_e_focus      ( bool );
      void baseline_end_s_focus        ( bool );
      void baseline_end_focus          ( bool );
      void baseline_end_e_focus        ( bool );

      void select_vis();
      void remove_vis();
      void crop_left();
      void crop_common();
      void crop_vis();
      void crop_zero();
      void crop_undo();
      void crop_right();
      void legend();
      void axis_x();
      void axis_y();
      void legend_set();

      void rename_created( Q3ListBoxItem *, const QPoint & );

      void set_plot_errors_rev();
      void set_plot_errors_sd();
      void set_plot_errors_pct();
      void set_plot_errors_group();

      void save_state();

   protected slots:

      void closeEvent(QCloseEvent *);
   
};

#endif
