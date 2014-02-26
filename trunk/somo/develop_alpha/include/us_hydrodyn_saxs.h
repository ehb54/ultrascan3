#ifndef US_HYDRODYN_SAXS_H
#define US_HYDRODYN_SAXS_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <q3frame.h>
#include <qcheckbox.h>
#include <qwt_counter.h>
#include <qbuttongroup.h>
#include <q3textedit.h>
#include <q3progressbar.h>
#include <qradiobutton.h>
#include <qmenubar.h>
#include <qfileinfo.h>
#include <qprinter.h>
#include <qregexp.h>
#include <qwt_wheel.h>
#include <qtimer.h>
#include <qmutex.h>

#include "qwt/scrollbar.h"
#include "qwt/scrollzoomer.h"

#include <qwt_plot.h>
//Added by qt3to4:
#include <QCloseEvent>
#ifdef QT4
# include "qwt_legend.h"
# include "qwt_plot_grid.h"
# include "qwt_plot_curve.h"
# include "qwt_scale_engine.h"
#endif

#include "us_util.h"
#include "us_hydrodyn_pdbdefs.h"
// #ifndef WIN32
#include "us_hydrodyn_saxs_iqq_residuals.h"
// #endif
#include "us_hydrodyn_saxs_residuals.h"
#include "us_hydrodyn_comparative.h"
#include "us_vector.h"

//standard C and C++ defs:

#include <set>
#include <map>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include <time.h>
#include <qstringlist.h>
#include <qinputdialog.h>

using namespace std;

#if defined( WIN32 ) && !defined( MINGW )
typedef _int16 int16_t;
typedef unsigned _int32 uint32_t;
#else
#include <stdint.h>
#endif

#include <fstream>

struct shd_point
{
   float   x[ 3 ];
   int16_t ff_type;
};

struct shd_input_data
{
   uint32_t max_harmonics;
   uint32_t model_size;
   uint32_t q_size;
   uint32_t F_size;
};

#include <complex>

struct shd_data
{
   float rtp[ 3 ];
   vector < complex < float > > A1v;
};

#include "us_mqt.h"

struct crop_undo_data
{
#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( disable: 4251 )
# endif
#endif
   bool               is_left;
   vector < QString > files;
   vector < QString > q_string;
   vector < double >  q;
   vector < double >  I;
   vector < bool >    has_e;
   vector < double >  e;

   bool               is_common;

   map < QString, vector < QString > > f_qs_string;
   map < QString, vector < double > >  f_qs;
   map < QString, vector < double > >  f_Is;
   map < QString, vector < double > >  f_errors;
#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( default: 4251 )
# endif
#endif
};

class US_EXTERN US_Hydrodyn_Saxs : public Q3Frame
{
   Q_OBJECT

      friend class US_Hydrodyn_Batch;
      // friend class US_Hydrodyn_Saxs_Options;

      friend class US_Hydrodyn_SasOptionsBeadModel;
      friend class US_Hydrodyn_SasOptionsCurve;
      friend class US_Hydrodyn_SasOptionsExperimental;
      friend class US_Hydrodyn_SasOptionsGuinier;
      friend class US_Hydrodyn_SasOptionsHydration;
      friend class US_Hydrodyn_SasOptionsMisc;
      friend class US_Hydrodyn_SasOptionsSans;
      friend class US_Hydrodyn_SasOptionsSaxs;

      friend class US_Hydrodyn_Saxs_Iqq_Residuals;
      friend class US_Hydrodyn;
      friend class US_Hydrodyn_Saxs_Search;
      friend class US_Hydrodyn_Saxs_Screen;
      friend class US_Hydrodyn_Saxs_Buffer;
      friend class US_Hydrodyn_Saxs_Buffer_Conc;
      friend class US_Hydrodyn_Saxs_Hplc;
      friend class US_Hydrodyn_Saxs_Hplc_Conc;
      friend class US_Hydrodyn_Saxs_Hplc_Svd;
      friend class US_Hydrodyn_Saxs_1d;
      friend class US_Hydrodyn_Saxs_2d;
      friend class US_Hydrodyn_Saxs_Conc;

      friend class US_Hydrodyn_Xsr;

   public:

      US_Hydrodyn_Saxs(
                       bool                           *saxs_widget, 
                       saxs_options                   *our_saxs_options,
                       QString                        pdb_file, 
                       QString                        pdb_filepath, 
                       vector < residue >             residue_list,
                       vector < PDB_model >           model_vector,
                       vector < vector <PDB_atom> >   bead_models,
                       vector < unsigned int >        selected_models,
                       map < QString, vector <int> >  multi_residue_map,
                       map < QString, QString >       residue_atom_hybrid_map,
                       int                            source, 
                       bool                           create_native_saxs,
                       void                           *us_hydrodyn,
                       QWidget                        *p = 0, 
                       const char                     *name = 0
                       );
      ~US_Hydrodyn_Saxs();

      void refresh(
                   QString                        pdb_file, 
                   QString                        pdb_filepath, 
                   vector < residue >             residue_list,
                   vector < PDB_model >           model_vector,
                   vector < vector <PDB_atom> >   bead_models,
                   vector < unsigned int >        selected_models,
                   map < QString, vector <int> >  multi_residue_map,
                   map < QString, QString >       residue_atom_hybrid_map,
                   int                            source,
                   bool                           create_native_saxs
                   );
      
   public:
      bool *saxs_widget, initial;
      int source;
      US_Config *USglobal;

      mQLabel *lbl_iq;
      mQLabel *lbl_settings;
      QLabel *lbl_filename1;
      QLabel *lbl_hybrid_table;
      QLabel *lbl_atom_table;
      QLabel *lbl_saxs_table;
      QLabel *lbl_core_progress;
      mQLabel *lbl_pr;
      QLabel *lbl_bin_size;
      QLabel *lbl_smooth;
      QLabel *lbl_guinier_cutoff;

      QLineEdit *te_filename2;

      QPrinter printer;

      QButtonGroup *bg_saxs_sans;
      QRadioButton *rb_saxs;
      QRadioButton *rb_sans;

      QButtonGroup *bg_saxs_iq;
      QRadioButton *rb_saxs_iq_native_debye;
      QRadioButton *rb_saxs_iq_native_sh;
      QRadioButton *rb_saxs_iq_native_hybrid;
      QRadioButton *rb_saxs_iq_native_hybrid2;
      QRadioButton *rb_saxs_iq_native_hybrid3;
      QRadioButton *rb_saxs_iq_native_fast;
      QRadioButton *rb_saxs_iq_foxs;
      QRadioButton *rb_saxs_iq_sastbx;
      QRadioButton *rb_saxs_iq_crysol;

      QButtonGroup *bg_sans_iq;
      QRadioButton *rb_sans_iq_native_debye;
      QRadioButton *rb_sans_iq_native_sh;
      QRadioButton *rb_sans_iq_native_hybrid;
      QRadioButton *rb_sans_iq_native_hybrid2;
      QRadioButton *rb_sans_iq_native_hybrid3;
      QRadioButton *rb_sans_iq_native_fast;
      QRadioButton *rb_sans_iq_cryson;

      QLabel       *lbl_iqq_suffix;
      QLineEdit    *le_iqq_manual_suffix;
      QLineEdit    *le_iqq_full_suffix;

      QPushButton *pb_plot_saxs_sans;
      QPushButton *pb_load_saxs_sans;
      QPushButton *pb_load_plot_saxs;
      QPushButton *pb_set_grid;
      QPushButton *pb_clear_plot_saxs;
      QPushButton *pb_saxs_legend;
      QPushButton *pb_plot_pr;
      QPushButton *pb_load_pr;
      QPushButton *pb_load_plot_pr;
      QPushButton *pb_clear_plot_pr;
      QPushButton *pb_pr_legend;
      QPushButton *pb_load_gnom;
      QPushButton *pb_ift;
      QPushButton *pb_saxs_search;
      QPushButton *pb_saxs_screen;
      QPushButton *pb_saxs_buffer;
      QPushButton *pb_saxs_hplc;
      QPushButton *pb_saxs_xsr;
      QPushButton *pb_saxs_1d;
      QPushButton *pb_saxs_2d;
      QPushButton *pb_guinier_analysis;
      QPushButton *pb_guinier_cs;
      QPushButton *pb_select_atom_file;
      QPushButton *pb_select_hybrid_file;
      QPushButton *pb_select_saxs_file;
      QPushButton *pb_stop;
      QPushButton *pb_options;
      QPushButton *pb_help;
      QPushButton *pb_cancel;

      QwtCounter *cnt_bin_size;
      QwtCounter *cnt_smooth;

      QCheckBox *cb_normalize;
      QCheckBox *cb_smooth;

      QwtCounter *cnt_guinier_cutoff;

      QCheckBox *cb_guinier;
      QCheckBox *cb_cs_guinier;
      QCheckBox *cb_Rt_guinier;
      QLineEdit *le_guinier_lowq2;
      QLineEdit *le_guinier_highq2;

      QCheckBox *cb_user_range;
      QCheckBox *cb_kratky;
      QLineEdit *le_user_lowq;
      QLineEdit *le_user_highq;
      QLineEdit *le_user_lowI;
      QLineEdit *le_user_highI;

      QButtonGroup *bg_curve;
      QRadioButton *rb_curve_raw;
      QRadioButton *rb_curve_saxs_dry;
      QRadioButton *rb_curve_saxs;
      QRadioButton *rb_curve_sans;

      QCheckBox   *cb_pr_contrib;
      QLineEdit   *le_pr_contrib_low;
      QLineEdit   *le_pr_contrib_high;
      QPushButton *pb_pr_contrib;

      QCheckBox *cb_create_native_saxs;

      QFont ft;

      Q3TextEdit *editor;

      QMenuBar *m;

      QwtPlot       *plot_pr;
      ScrollZoomer  *plot_pr_zoomer;
      QwtPlot       *plot_saxs;
      ScrollZoomer  *plot_saxs_zoomer;
      QwtPlot       *plot_resid;
      ScrollZoomer  *plot_resid_zoomer;
#ifdef QT4
      QwtPlotGrid  *grid_pr;
      QwtPlotGrid  *grid_saxs;
      QwtPlotGrid  *grid_resid;
#endif
      QCheckBox   *cb_resid_pct;
      QCheckBox   *cb_manual_guinier;
      QCheckBox   *cb_resid_sd;
      QCheckBox   *cb_resid_show_errorbars;
      QCheckBox   *cb_resid_show;

      mQLineEdit  * le_manual_guinier_fit_start;
      mQLineEdit  * le_manual_guinier_fit_end;
      QwtWheel    * qwtw_wheel;
      QPushButton * pb_manual_guinier_process;

      Q3ProgressBar *progress_pr;
      Q3ProgressBar *progress_saxs;

      struct atom current_atom;
      struct hybridization current_hybrid;
      struct saxs current_saxs;
      saxs_options *our_saxs_options;
      QStringList  qsl_plotted_iq_names;
      QStringList  qsl_plotted_pr_names;
#ifdef WIN32
# if !defined( QT4 )
     #pragma warning ( disable: 4251 )
# endif
#endif
      map    < QString, bool >                        dup_plotted_iq_name_check;
      map    < QString, unsigned int >                plotted_iq_names_to_pos;
      map    < QString, bool >                        dup_plotted_pr_name_check;
      vector < atom >                                 atom_list;
      vector < hybridization >                        hybrid_list;
      vector < saxs >                                 saxs_list;
      vector < residue >                              residue_list;
      vector < PDB_model >                            model_vector;
      vector < vector <PDB_atom> >                    bead_models;
      vector < unsigned int >                         selected_models;
      vector < QColor >                               plot_colors;
#ifndef QT4
      vector < long >                                 plotted_Iq;  // curve keys
#else
      vector < QwtPlotCurve * >                       plotted_Iq_curves;
#endif
      vector < double >                               exact_q;
      vector < vector < double > >                    plotted_q;
      vector < vector < double > >                    plotted_q2;  // q^2 for guinier plots
      vector < vector < double > >                    plotted_I;
      vector < vector < double > >                    plotted_I_error; 

#ifndef QT4
      map    < unsigned int, long >                   plotted_Gp;  // guinier points
      map    < unsigned int, long >                   plotted_cs_Gp;  // cs guinier points
      map    < unsigned int, long >                   plotted_Rt_Gp;  // Rt guinier points
      map    < unsigned int, long >                   plotted_Gp_full;  // guinier points
      map    < unsigned int, long >                   plotted_cs_Gp_full;  // cs guinier points
      map    < unsigned int, long >                   plotted_Rt_Gp_full;  // Rt guinier points
      vector < long >                                 plotted_manual_guinier_fit;
      vector < long >                                 plotted_guinier_error_bars;
#else
      map    < unsigned int, QwtPlotCurve * >         plotted_Gp_curves;  // guinier points
      map    < unsigned int, QwtPlotCurve * >         plotted_cs_Gp_curves;  // cs guinier points
      map    < unsigned int, QwtPlotCurve * >         plotted_Rt_Gp_curves;  // Rt guinier points
      map    < unsigned int, QwtPlotCurve * >         plotted_Gp_curves_full;  // guinier points
      map    < unsigned int, QwtPlotCurve * >         plotted_cs_Gp_curves_full;  // cs guinier points
      map    < unsigned int, QwtPlotCurve * >         plotted_Rt_Gp_curves_full;  // Rt guinier points
      vector < QwtPlotCurve * >                       plotted_manual_guinier_fit;
      vector < QwtPlotCurve * >                       plotted_guinier_error_bars; 
#endif
      map    < unsigned int, bool >                   plotted_guinier_valid;
      map    < unsigned int, bool >                   plotted_guinier_plotted;
      map    < unsigned int, double >                 plotted_guinier_lowq2;
      map    < unsigned int, double >                 plotted_guinier_highq2;
      map    < unsigned int, double >                 plotted_guinier_a;           // y = a + b*x
      map    < unsigned int, double >                 plotted_guinier_b;
      map    < unsigned int, vector < double > >      plotted_guinier_x;
      map    < unsigned int, vector < double > >      plotted_guinier_y;
      map    < unsigned int, map < double, double > > plotted_guinier_pts_removed;

      map    < unsigned int, bool >                   plotted_cs_guinier_valid;
      map    < unsigned int, bool >                   plotted_cs_guinier_plotted;
      map    < unsigned int, double >                 plotted_cs_guinier_lowq2;
      map    < unsigned int, double >                 plotted_cs_guinier_highq2;
      map    < unsigned int, double >                 plotted_cs_guinier_a;           // y = a + b*x
      map    < unsigned int, double >                 plotted_cs_guinier_b;
      map    < unsigned int, vector < double > >      plotted_cs_guinier_x;
      map    < unsigned int, vector < double > >      plotted_cs_guinier_y;
      map    < unsigned int, map < double, double > > plotted_cs_guinier_pts_removed;

      map    < unsigned int, bool >                   plotted_Rt_guinier_valid;
      map    < unsigned int, bool >                   plotted_Rt_guinier_plotted;
      map    < unsigned int, double >                 plotted_Rt_guinier_lowq2;
      map    < unsigned int, double >                 plotted_Rt_guinier_highq2;
      map    < unsigned int, double >                 plotted_Rt_guinier_a;           // y = a + b*x
      map    < unsigned int, double >                 plotted_Rt_guinier_b;
      map    < unsigned int, vector < double > >      plotted_Rt_guinier_x;
      map    < unsigned int, vector < double > >      plotted_Rt_guinier_y;
      map    < unsigned int, map < double, double > > plotted_Rt_guinier_pts_removed;

      vector < vector < double > >                    plotted_pr;
      vector < vector < double > >                    plotted_pr_not_normalized;
      vector < vector < double > >                    plotted_r;
      vector < float >                                plotted_pr_mw;

      map < QString, saxs >          saxs_map;
      map < QString, hybridization > hybrid_map;
      map < QString, atom >          atom_map;
      map < QString, vector <int> >  multi_residue_map;
      map < QString, QString >       residue_atom_hybrid_map;

      map < QString, vector < double > > nnls_A;
      map < QString,  double >           nnls_x;
      map < QString,  double >           nnls_mw;
      vector < double >                  nnls_B;
      vector < double >                  nnls_errors;
      vector < double >                  nnls_r;
      vector < double >                  nnls_q;
      double                             nnls_rmsd;

#ifdef WIN32
# if !defined( QT4 )
     #pragma warning ( default: 4251 )
# endif
#endif      
      QString nnls_B_name;
      QString nnls_header_tag;

      void *us_hydrodyn;

      unsigned int current_model;

      QString model_filename;
      QString model_filepathname;

      QString atom_filename;
      QString hybrid_filename;
      QString saxs_filename;
      
      bool stopFlag;
      bool create_native_saxs;

      Q3ProgressBar *progress;

      void set_current_method_buttons();
      void update_iqq_suffix();

   private:

      void  create_shd( vector < saxs_atom > & atoms,
                        vector < double >    & q,
                        vector < double >    & q2,
                        vector < double >    & q_over_4pi_2
                        );

      void plot_guinier_pts_removed( int i, bool cs = false, bool Rt = false );
      void plot_guinier_error_bars( int i, bool cs = false, bool Rt = false );
      void clear_guinier_error_bars();

      void do_plot_resid();
      void do_plot_resid( vector < double > & x, 
                          vector < double > & y, 
                          vector < double > & e, 
                          QColor qc );

      void do_plot_resid( vector < double > & x, 
                          vector < double > & y, 
                          vector < double > & e, 
                          map < double, double > & pts_removed,
                          QColor qc );

      void do_plot_resid( vector < double > & x_d, 
                          vector < double > & e_d, 
                          QColor qc );

      void set_plot_pr_range( double &minx, 
                              double &maxx,
                              double &miny,
                              double &maxy );

      Q3Process *rasmol;

      bool bead_model_ok_for_saxs;

      bool                           saxs_residuals_widget;
      US_Hydrodyn_Saxs_Residuals     *saxs_residuals_window;


#ifdef WIN32
# if !defined( QT4 )
     #pragma warning ( disable: 4251 )
# endif
#endif
      // #ifndef WIN32
      // target indexed iqq residuals windows
      map < QString, bool >                            saxs_iqq_residuals_widgets;
      map < QString, US_Hydrodyn_Saxs_Iqq_Residuals *> saxs_iqq_residuals_windows;
      // #endif
      map < QString, float > *remember_mw;
      map < QString, float > *match_remember_mw;
      map < QString, QString > *remember_mw_source;
      // map < QString, float > contrib;
      vector < vector < float > > contrib_array;
      vector < PDB_atom * >  contrib_pdb_atom;

      map < QString, unsigned int >   ff_table;
      vector < vector < double > >    ff_q;
      vector < vector < double > >    ff_ff;
      vector < vector < double > >    ff_y2;
      vector < double >               ff_ev;
      map < QString, bool >           ff_sent_msg1;

#ifdef WIN32
# if !defined( QT4 )
     #pragma warning ( default: 4251 )
# endif
#endif      
      bool                 ff_table_loaded;
      bool                 load_ff_table( QString filename );
      QString              ff_info();
      QString              last_ff_filename;
      double               get_ff_ev( QString res, QString atm );

      float contrib_delta;
      QString contrib_file;

      vector < double > interpolate( vector < double > to_r, 
                                     vector < double > from_r, 
                                     vector < double > from_pr );

      bool natural_spline_interpolate( vector < double > to_grid, 
                                       vector < double > from_grid, 
                                       vector < double > from_data,
                                       vector < double > &to_data );

      vector < double > rescale( vector < double > x );

      QString vector_double_to_csv( vector < double > vd );
      double compute_pr_area( vector < double > vd, vector < double > r );
      void calc_nnls_fit( 
                         QString title, 
                         QString save_to_csv_name = ""
                         );
      void calc_best_fit( 
                         QString title, 
                         QString save_to_csv_name = ""
                         );
      void plot_one_pr(
                       vector < double > r, 
                       vector < double > pr, 
                       QString name
                       );

      void calc_iqq_nnls_fit( 
                             QString title, 
                             QString save_to_csv_name = ""
                             );
      void calc_iqq_best_fit( 
                             QString title, 
                             QString save_to_csv_name = ""
                             );

      void plot_one_iqq(vector < double > q, vector < double > I, QString name);
      void plot_one_iqq(vector < double > q, vector < double > I, vector < double > I_error, QString name);
      bool plotted;
      bool save_to_csv;
      QString csv_filename;
      double guinier_cutoff;

      int file_curve_type(QString filename);
      QString curve_type_string(int curve);

      bool guinier_analysis( unsigned int i, QString &csvlog );
      bool cs_guinier_analysis( unsigned int i, QString &csvlog );
      bool Rt_guinier_analysis( unsigned int i, QString &csvlog );
      void crop_iq_data( vector < double > &q,
                         vector < double > &I );

      void crop_iq_data( vector < double > &q,
                         vector < double > &I,
                         vector < double > &I_errors );


      // sets lowq & highq based upon current, plot settings (could be q^2 if in guinier)
      void plot_domain(double &lowq, double &highq);  

      // sets lowI & highI based upon range
      void plot_range(double lowq, double highq, double &lowI, double &highI);
      
      void rescale_plot();

      double pr_contrib_low;
      double pr_contrib_high;

      double get_mw(QString filename, bool display_mw_msg = true);
      float last_used_mw;

      QString load_pr_selected_filter;
      QString load_saxs_sans_selected_filter;

#ifdef WIN32
# if !defined( QT4 )
     #pragma warning ( disable: 4251 )
# endif
#endif      
      void display_iqq_residuals( QString title,
                                  vector < double > q,
                                  vector < double > I1,
                                  vector < double > I2,
                                  QColor            plot_color,
                                  vector < double > I_errors );
#ifdef WIN32
# if !defined( QT4 )
     #pragma warning ( default: 4251 )
# endif
#endif      
      double last_rescaling_multiplier;
      double last_rescaling_offset;
      double last_rescaling_chi2;

      void rescale_iqq_curve_using_last_rescaling( vector < double > &I, bool use_offset = false );

      void rescale_iqq_curve( QString scaling_target,
                              vector < double > &q,
                              vector < double > &I );

      void rescale_iqq_curve( QString scaling_target,
                              vector < double > &q,
                              vector < double > &I,
                              vector < double > &I2 );

      void rescale_iqq_curve( QString scaling_target,
                              vector < double > &q,
                              vector < double > &I,
                              QColor plot_color );

      void rescale_iqq_curve( QString scaling_target,
                              vector < double > &q,
                              vector < double > &I,
                              vector < double > &I2,
                              QColor plot_color );

      void editor_msg( QColor  color, QString msg );
      void editor_msg( QString color, QString msg );
      void editor_msg( const char *color, QString msg );

      void editor_msg( QColor  color, QColor  bgcolor, QString msg );
      void editor_msg( QString color, QColor  bgcolor, QString msg );
      void editor_msg( const char * color, QColor  bgcolor, QString msg );
      void editor_msg( QColor  color, QString bgcolor, QString msg );
      void editor_msg( QColor  color, const char * bgcolor, QString msg );
      void editor_msg( QString color, QString bgcolor, QString msg );
      void editor_msg( const char * color, const char * bgcolor, QString msg );

      int run_saxs_iq_foxs( QString pdb );
      int run_saxs_iq_crysol( QString pdb );
      int run_sans_iq_cryson( QString pdb );
      int run_saxs_iq_sastbx( QString pdb );

      Q3Process *foxs;
      Q3Process *crysol;
      Q3Process *cryson;
      Q3Process *sastbx;

      QStringList crysol_stdout;
      QStringList crysol_stderr;
      QString foxs_last_pdb;
      QString sastbx_last_pdb;
      QString crysol_last_pdb;
      QString crysol_last_pdb_base;
      QString cryson_last_pdb;
      QString cryson_last_pdb_base;
      bool crysol_manual_mode;
      QStringList crysol_manual_input;

      void calc_saxs_iq_native_debye();
      void calc_saxs_iq_native_sh();
      void calc_saxs_iq_native_hybrid2();
      void calc_saxs_iq_native_fast();

      void calc_saxs_iq_native_debye_bead_model();
      void calc_saxs_iq_native_sh_bead_model();
      void calc_saxs_iq_native_hybrid2_bead_model();
      void calc_saxs_iq_native_fast_bead_model();

      void set_bead_model_ok_for_saxs(); // checks current_model

      QString iqq_suffix();

      bool is_zero_vector( vector < double > &v );    // returns true if all zeros
      bool is_nonzero_vector( vector < double > &v ); // returns true if all non-zero
      void push_back_zero_I_error();
      QString Iq_plotted_summary();

      void ask_iq_target_grid( bool force = false );
      bool everything_plotted_has_same_grid();
      bool everything_plotted_has_same_grid_as_set();

      void load_iqq_csv( QString filename, bool just_plotted_curves = false );

      bool iq_plot_experimental_and_calculated_present();
      bool iq_plot_only_experimental_present();
      void clear_plot_saxs_and_replot_experimental();
      void clear_plot_saxs_data();

      bool select_from_directory_history( QString &dir, QWidget *parent = (QWidget *)0 );
      void add_to_directory_history( QString dir, bool accessed = true );
      
      csv  search_csv;
      void reset_search_csv();
      csv  screen_csv;
      void reset_screen_csv();
      csv  buffer_csv;
      void reset_buffer_csv();
      csv  hplc_csv;
      void reset_hplc_csv();

      csv  conc_csv;         // stores curve specific data
      void sync_conc_csv();  // removes deleted curves, adds non-extant curves with default
      void update_conc_csv( QString name, double conc, double psv, double I0_std );
      void update_conc_csv( QString name, double conc, double psv );
      void update_conc_csv( QString name, double conc );
      void update_conc_csv( QString name );
      void add_conc_csv( QString name, double conc, double psv, double I0_std );
      void add_conc_csv( QString name );
      bool get_conc_csv_values ( QString name, double &conc, double &psv, double &I0_std );

      void set_scaling_target( QString &scaling_target );

      void saxs_search_update_enables();

      bool external_running;

      void check_pr_grid( vector < double > &r, vector < double > &pr );
      vector < double > range_crop( vector < double > &q, vector < double > &I );

      bool started_in_expert_mode;

      QString specname;

      bool create_somo_ff();
      QString errormsg;

      bool compute_scale_excl_vol();

      void push_back_color_if_ok( QColor bg, QColor set );
      unsigned int pen_width;

   private:

      bool mw_from_I0 ( QString name, double I0_exp, double &MW, double &internal_contrast );
      bool ml_from_qI0( QString name, double I0_exp, double &ML, double &internal_contrast );
      bool ma_from_q2I0( QString name, double I0_exp, double &MA, double &internal_contrast );
      
#ifdef WIN32
# if !defined( QT4 )
     #pragma warning ( disable: 4251 )
# endif
#endif      
      vector < QWidget * > iq_widgets;
      vector < QWidget * > pr_widgets;
      vector < QWidget * > settings_widgets;
      vector < QWidget * > resid_widgets;
      vector < QWidget * > manual_guinier_widgets;

      void hide_widgets( vector < QWidget * >, bool do_hide = true );

#ifdef WIN32
# if !defined( QT4 )
     #pragma warning ( default: 4251 )
# endif
#endif      

#ifdef WIN32
# if !defined( QT4 )
     #pragma warning ( disable: 4251 )
# endif
#endif      
      map < QString, vector < point > >                                   hybrid_coords;
      map < QString, map < unsigned int, map < unsigned int, double > > > hybrid_r;

#ifdef WIN32
# if !defined( QT4 )
     #pragma warning ( default: 4251 )
# endif
#endif      

#if defined( UHSE_APP_RESPONSE_WAY )
      QStringList    crysol_app_text;
      QStringList    crysol_response;
      
      int            crysol_query_response_pos;
      bool           crysol_run_to_end;

      QTimer         crysol_timer;
      int            crysol_timer_delay_ms;
#endif
      void           crysol_finishup();

   private slots:

      void manual_guinier_process();
      void set_manual_guinier();
      void adjust_wheel( double );
      void manual_guinier_fit_start_text ( const QString & );
      void manual_guinier_fit_end_text   ( const QString & );
      void manual_guinier_fit_start_focus( bool );
      void manual_guinier_fit_end_focus  ( bool );

      void set_resid_pct();
      void set_resid_sd();
      void set_resid_show_errorbars();
      void set_resid_show();

      void hide_pr();
      void hide_iq();
      void hide_settings();

      void foxs_readFromStdout();
      void foxs_readFromStderr();
      void foxs_launchFinished();
      void foxs_processExited();

      void crysol_readFromStdout();
      void crysol_readFromStderr();
      void crysol_launchFinished();
      void crysol_processExited();
      void crysol_wroteToStdin();
      void crysol_timeout();

      void cryson_readFromStdout();
      void cryson_readFromStderr();
      void cryson_launchFinished();
      void cryson_processExited();

      void sastbx_readFromStdout();
      void sastbx_readFromStderr();
      void sastbx_launchFinished();
      void sastbx_processExited();

      double compute_ff(
                        saxs     &sa,     // gaussian decomposition for the main atom
                        saxs     &sh,     // gaussian decomposition for hydrogen
                        QString  &nr,     // name of residue
                        QString  &na,     // name of atom
                        QString  &naf,    // full name of atom
                        unsigned int h,   // number of hydrogens
                        double   q,
                        double   q_o_4pi2 
                        );

      double compute_ff_bead_model
         (
          saxs     &s,
          double   q_o_4pi2 
          );

      
      void fix_sas_options();

      void clear_guinier();
      void clear_cs_guinier();
      void clear_Rt_guinier();

   public slots:
      void show_plot_saxs_sans();
      void show_plot_pr();

   private slots:

      void setupGUI();
      void set_saxs_sans(int);
      void set_saxs_iq(int);
      void set_sans_iq(int);
      void load_saxs_sans();
      void load_plot_saxs();
      void set_grid();
      void show_plot_saxs();
      void load_saxs( QString filename = "", bool just_plotted_curves = false );
      void clear_plot_saxs( bool quiet = false );
      void show_plot_sans();
      void load_sans( QString filename = "", bool just_plotted_curves = false );
      void update_bin_size(double);
      void update_smooth(double);
      void update_guinier_cutoff(double);
      void show_pr_contrib();
      void update_pr_contrib_low(const QString &);
      void update_pr_contrib_high(const QString &);
      void set_curve(int);
      void load_pr( bool just_plotted_curves = false );
      void load_plot_pr();
      void clear_plot_pr();
      void cancel();
      void help();
      void options();
      void stop();
      void clear_display();
      void print();
      void update_font();
      void save();
      void select_atom_file();
      void select_hybrid_file();
      void select_saxs_file();
      void select_atom_file(const QString &);
      void select_hybrid_file(const QString &);
      void select_saxs_file(const QString &);
      void normalize_pr( vector < double >, 
                         vector < double > *, 
                         double mw = 1e0 );
      void update_saxs_sans();

      void guinier_window();
      void run_guinier_analysis();
      void run_guinier_cs();
      void run_guinier_Rt();
      QString saxs_filestring();
      QString sprr_filestring();
      void set_create_native_saxs();
      void set_guinier();
      void set_cs_guinier();
      void set_Rt_guinier();
      void set_pr_contrib();
      void set_user_range();
      void set_kratky();
      void update_guinier_lowq2(const QString &);
      void update_guinier_highq2(const QString &);
      void update_user_lowq(const QString &);
      void update_user_highq(const QString &);
      void update_user_lowI(const QString &);
      void update_user_highI(const QString &);
      void load_gnom();
      void ift();
      void saxs_search();
      void saxs_screen();
      void saxs_buffer();
      void saxs_hplc();
      void saxs_xsr();
      void saxs_1d();
      void saxs_2d();

      void plot_saxs_clicked( long );
      void plot_pr_clicked  ( long );
      void plot_saxs_item_clicked( QwtPlotItem* );
      void plot_pr_item_clicked  ( QwtPlotItem* );

      void saxs_legend();
      void pr_legend();

   protected slots:

      void closeEvent(QCloseEvent *);
};



#include <qthread.h>
#include <qwaitcondition.h>
#include <qmutex.h>

class saxs_Iq_thr_t : public QThread
{
 public:
  saxs_Iq_thr_t(int);
  void saxs_Iq_thr_setup(
                         vector < saxs_atom > *atoms,
                         vector < vector < double > > *f,
                         vector < vector < double > > *fc,
                         vector < vector < double > > *fp,
                         vector < double > *I,
                         vector < double > *Ia,
                         vector < double > *Ic,
                         vector < double > *q,
                         unsigned int threads,
                         Q3ProgressBar *progress,
                         QLabel *lbl_core_progress,
                         bool *stopFlag
                         );
  void saxs_Iq_thr_shutdown();
  void saxs_Iq_thr_wait();
  int saxs_Iq_thr_work_status();
  virtual void run();

 private:

#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( disable: 4251 )
# endif
#endif

  vector < saxs_atom > *atoms;
  vector < vector < double > > *f;
  vector < vector < double > > *fc;
  vector < vector < double > > *fp;
  vector < double > *I;
  vector < double > *Ia;
  vector < double > *Ic;
  vector < double > *q;

#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( default: 4251 )
# endif
#endif
  unsigned int threads;
  Q3ProgressBar *progress;
  QLabel *lbl_core_progress;
  bool *stopFlag;

  int thread;
  QMutex work_mutex;
  int work_to_do;
  QWaitCondition cond_work_to_do;
  int work_done;
  QWaitCondition cond_work_done;
  int work_to_do_waiters;
  int work_done_waiters;
};

class saxs_pr_thr_t : public QThread
{
 public:
  saxs_pr_thr_t(int);
  void saxs_pr_thr_setup(
                         vector < saxs_atom > *atoms,
                         vector < float > *hist,
                         double delta,
                         unsigned int threads,
                         Q3ProgressBar *progress,
                         QLabel *lbl_core_progress,
                         bool *stopFlag,
                         float b_bar_inv2
                         );
  void saxs_pr_thr_shutdown();
  void saxs_pr_thr_wait();
  int saxs_pr_thr_work_status();
  virtual void run();

 private:

#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( disable: 4251 )
# endif
#endif

  vector < saxs_atom > *atoms;
  vector < float > *hist;

#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( default: 4251 )
# endif
#endif
  double delta;
  unsigned int threads;
  Q3ProgressBar *progress;
  QLabel *lbl_core_progress;
  bool *stopFlag;
  float b_bar_inv2;

  int thread;
  QMutex work_mutex;
  int work_to_do;
  QWaitCondition cond_work_to_do;
  int work_done;
  QWaitCondition cond_work_done;
  int work_to_do_waiters;
  int work_done_waiters;
};

#endif
