#ifndef US_HYDRODYN_MALS_SAXS_SVD_H
#define US_HYDRODYN_MALS_SAXS_SVD_H

// QT defs:

#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_efa.h"
#include "us_plot_util.h"

//Added by qt3to4:
#include <QCloseEvent>
#include <QFrame>
#include <QLabel>
#include <qwt_counter.h>

using namespace std;

class US_EXTERN US_Hydrodyn_Mals_Saxs_Svd : public QFrame
{
   Q_OBJECT

   public:
      US_Hydrodyn_Mals_Saxs_Svd(
                                US_Hydrodyn_Mals_Saxs *mals_saxs_win, 
                                vector < QString > mals_saxs_selected_files,
                                QWidget *p = 0
                                );

      ~US_Hydrodyn_Mals_Saxs_Svd();


   private:

      US_Config             * USglobal;

      US_Hydrodyn_Mals_Saxs * mals_saxs_win;
      US_Hydrodyn           * ush_win;

      // ------ data section 

      mQLabel               * lbl_data;
      QTreeWidget           * lv_data;
      QPushButton           * pb_clear;
      QPushButton           * pb_to_mals_saxs;
      QPushButton           * pb_save_plots;
      QPushButton           * pb_color_rotate;
      QPushButton           * pb_replot;

      // ------ editor section 

      mQLabel               * lbl_editor;
      QFont                   ft;
      QTextEdit             * editor;
      QMenuBar              * mb_editor;

      // ------ mode controls section

      QLabel                * lbl_modes;
      QButtonGroup          * bg_modes;
      QRadioButton          * rb_mode_iqit;
      QRadioButton          * rb_mode_svd;
      QRadioButton          * rb_mode_efa;
      QRadioButton          * rb_mode_efa_decomp;

   private slots:
      void                     set_mode_iqit();
      void                     set_mode_svd();
      void                     set_mode_efa();
      void                     set_mode_efa_decomp();

   private:

      // ------ plot section

      // ----------- plot data

      mQwtPlot               * plot_data;
      US_Plot               * usp_plot_data;
   private slots:
      void usp_config_plot_data( const QPoint & );

   private:
      ScrollZoomer          * plot_data_zoomer;
      QwtPlotGrid           * grid_data;

      // ----------- plot errors

      mQwtPlot               * plot_errors;
      US_Plot               * usp_plot_errors;
   private slots:
      void usp_config_plot_errors( const QPoint & );

   private:
      ScrollZoomer          * plot_errors_zoomer;
      QwtPlotGrid           * grid_errors;

      QCheckBox             * cb_plot_errors;
      QCheckBox             * cb_plot_errors_rev;
      QCheckBox             * cb_plot_errors_sd;
      QCheckBox             * cb_plot_errors_pct;
      QCheckBox             * cb_plot_errors_ref;
      QCheckBox             * cb_plot_errors_group;

      QPushButton           * pb_iq_it;
      QPushButton           * pb_axis_x;
      QPushButton           * pb_axis_y;


      // ----------- plot svd
      
      mQwtPlot               * plot_svd;
      US_Plot               * usp_plot_svd;
   private slots:
      void usp_config_plot_svd( const QPoint & );

   private:
      ScrollZoomer          * plot_svd_zoomer;
      QwtPlotGrid           * grid_svd;

      void                    set_number_of_svs_for_efa();

      // ----------- plot autocor

      mQwtPlot               * plot_ac;
      US_Plot               * usp_plot_ac;
   private slots:
      void usp_config_plot_ac( const QPoint & );

   private:
      ScrollZoomer          * plot_ac_zoomer;
      QwtPlotGrid           * grid_ac;

      // ----------- plot efa
      mQwtPlot               * plot_lefa;
      US_Plot               * usp_plot_lefa;
   private slots:
      void usp_config_plot_lefa( const QPoint & );

   private:
      ScrollZoomer          * plot_lefa_zoomer;
      QwtPlotGrid           * grid_lefa;

      mQwtPlot               * plot_refa;
      US_Plot               * usp_plot_refa;

   private slots:
      void usp_config_plot_refa( const QPoint & );

   private:
      ScrollZoomer          * plot_refa_zoomer;
      QwtPlotGrid           * grid_refa;

      // ----------- plot efa_decomp

      mQwtPlot               * plot_efa_decomp;
      US_Plot               * usp_plot_efa_decomp;
   private slots:
      void usp_config_plot_efa_decomp( const QPoint & );
      void                    efa_decomp_to_mals_saxs_saxs();
      void                    set_efa_decomp_force_positive();

   private:
      ScrollZoomer          * plot_efa_decomp_zoomer;
      QwtPlotGrid           * grid_efa_decomp;

      QPushButton           * pb_efa_decomp_to_mals_saxs_saxs;
      QCheckBox             * cb_efa_decomp_force_positive;

      // ----------- plot end

      QLabel                * lbl_efas;
      QwtCounter            * qwtc_efas;

      vector < vector < double > > efa_lsv;
      vector < vector < double > > efa_rsv;

      vector < QLabel * >          efa_range_labels;
      vector < QwtCounter * >      efa_range_start;
      vector < QLabel * >          efa_range_labels2;
      vector < QwtCounter * >      efa_range_end;

      vector < int >               plotted_efa_range_start;
      vector < int >               plotted_efa_range_end;
      void                         update_efas_ranges();
      void                         efa_info( QString tag );
      void                         init_efa_values();
      void                         make_plotted_efas();

   private slots:
      void                         update_efa_range_start( double );
      void                         update_efa_range_end( double );

   private:
      vector < double >            plotted_efa_x;
      vector < vector < double > > plotted_efa_lsv;
      vector < vector < double > > plotted_efa_rsv;
      vector < QwtPlotMarker * >   qwtpm_lsv_range_marker;
      vector < QwtPlotMarker * >   qwtpm_rsv_range_marker;

   private slots:
      void update_efas( double val );

   private:

      vector < double >            efa_decomp_x;
      vector < vector < double > > efa_decomp_Ct;

      // ------ norm utils
      enum                     norms {
         NO_NORM
         ,NORM_NOT
         ,NORM_PW
         ,NORM_AVG
      };

      // ------ process section

      mQLabel               * lbl_process;
      // QLabel                * lbl_wheel_pos;
      // QwtWheel              * qwtw_wheel;

      QCheckBox             * cb_norm_pw;
      QCheckBox             * cb_norm_avg;

      QLabel                * lbl_q_range;
      mQLineEdit            * le_q_start;
      mQLineEdit            * le_q_end;

      // QLabel                * lbl_t_range;
      // mQLineEdit            * le_t_start;
      // mQLineEdit            * le_t_end;

      // QCheckBox             * cb_random;
      // QLineEdit             * le_random;

      QPushButton           * pb_svd;
      QPushButton           * pb_efa;
      QPushButton           * pb_efa_decomp;
      QPushButton           * pb_stop;

      QLabel                * lbl_ev;
      QListWidget           * lb_ev;

      QPushButton           * pb_svd_plot;
      QPushButton           * pb_svd_save;
      QPushButton           * pb_recon;

      QPushButton           * pb_inc_rmsd_plot;
      QPushButton           * pb_rmsd_save;
      QPushButton           * pb_inc_chi_plot;
      QPushButton           * pb_inc_recon;
      QPushButton           * pb_indiv_recon;

      // ------- bottom section

      QPushButton           * pb_help;
      QProgressBar          * progress;
      QPushButton           * pb_cancel;


      void                    editor_msg( QString color, QString msg );

      QString                 errormsg;

      vector < QString >      mals_saxs_selected_files;

      map < QString, unsigned int >       f_pos;

      map < QString, vector < QString > > f_qs_string;
      map < QString, vector < double > >  f_qs;

      map < QString, vector < double > >  f_Is;
      map < QString, vector < double > >  f_Is_norm_pw;
      map < QString, vector < double > >  f_Is_norm_avg;
      vector < double >                   org_avg_errors;
      map < QString, vector < double > >  f_errors;
      map < QString, bool >               f_is_time;

      // -------- rhs widgets
      vector < QWidget * >                iqit_widgets;
      vector < QWidget * >                errors_widgets;
      vector < QWidget * >                svd_widgets;
      vector < QWidget * >                efa_widgets;
      vector < QWidget * >                efa_decomp_widgets;

      // -------- lhs widgets
      vector < QWidget * >                data_widgets;
      vector < QWidget * >                editor_widgets;
      vector < QWidget * >                process_widgets;

      void                                hide_widgets( vector < QWidget *> widgets, bool hide );

      void plot_files();
      void plot_files( QStringList files, norms norm_mode = NO_NORM );

      bool plot_file( QString file,
                      double &minx,
                      double &maxx,
                      double &miny,
                      double &maxy,
                      norms norm_mode = NO_NORM );

      bool get_min_max( QString file,
                        double &minx,
                        double &maxx,
                        double &miny,
                        double &maxy,
                        norms norm_mode = NO_NORM );


      bool                         running;

      bool                         axis_x_log;
      bool                         axis_y_log;
      bool                         last_axis_x_log;
      bool                         last_axis_y_log;

      void                         disable_all();

      mQLineEdit                   *le_last_focus;

      QStringList                  selected_files();
      set < QString >              get_current_files();
      set < QString >              get_sources();
      set < QString >              get_selected_sources();
      QTreeWidgetItem *              get_source_item( QString source );

      int                          selected_sources();
      void                         clean_selected();
      bool                         is_selected( QTreeWidgetItem *lvi );
      bool                         all_children_selected( QTreeWidgetItem *lvi );
      bool                         plotted_matches_selected();

      bool                         iq_it_state;

      void                         axis_x_title();
      void                         axis_y_title();

      map < QString, QwtPlotCurve * >     plotted_curves;

      void                         add_i_of_q_or_t( QString source, QStringList files, bool do_update_enables = true );
      void                         rescale( bool do_update_enables = true );

      bool                         svd_F_nonzero;
      vector < vector < double > > svd_F;
      vector < vector < double > > svd_F_errors;
      vector < vector < double > > svd_U;
      vector < vector < double > > svd_V;
      vector < double >            svd_D;
      vector < int >               svd_index;

      vector < double >            svd_x;
      vector < double >            svd_y;

      vector < double >            svd_autocor_U;
      vector < double >            svd_autocor_V;

      vector < double >            rmsd_x;
      vector < double >            rmsd_y;
      vector < double >            chi_x;
      vector < double >            chi_y;
      
      bool                         sv_plot;
      bool                         rmsd_plot;
      bool                         chi_plot;

      set < QString >              subset_data;
      QStringList                  last_svd_data;
      QString                      last_svd_name;
      map < QString, QStringList > svd_data_map;

      QStringList                  last_efa_data;
      QString                      last_efa_name;
      map < QString, QStringList > efa_data_map;

      QTreeWidgetItem *            lvi_last_depth( int d );

      vector < QColor >            plot_colors;
      QPalette                     cg_red;

      void                         do_recon();
      QString                      last_recon_tag;
      QStringList                  last_recon_evs;
      double                       last_recon_rmsd;
      double                       last_recon_chi;
      double                       vmin( vector < double > &x );
      double                       vmax( vector < double > &x );

      unsigned int                 use_line_width;
      QString                      recon_mode;

      void                         update_plot_errors( bool do_update_enables = true );
      void                         do_plot_errors();
      void                         do_plot_errors_group();

      QStringList                  add_subset_data( QStringList files );
      QString                      get_related_source_name( QString name );
      QStringList                  get_files_by_name( QString name );
      bool                         get_plot_files( QStringList &use_list, QStringList &use_ref_list, norms &ref_norm_mode );

      vector < double >            plot_errors_jumps;
      void                         plot_errors_jump_markers();
      set < QString >              plot_errors_reference;

      bool                         setup_save( QString tag, QString & fname );

      bool                         mode_i_of_t;

      bool                         norm_ok;
      void                         check_norm( QStringList & files );
      QString                      get_name();
      double                       autocor1( vector < double > &x );
      vector < double >            autocor( vector < vector < double > > &A );
      vector < vector < double > > transpose( vector < vector < double > > &A );
      vector < vector < int > >    transpose( vector < vector < int > > &A );
      vector < vector < double > > dot( vector < vector < double > > &A,
                                        vector < vector < double > > &B ); 
      vector < vector < double > > vivd_pwmult( vector < vector < int > > &A,
                                                vector < vector < double > > &B );
      void                         vvd_cnorm( vector < vector < double > > &A );
      void                         vvd_cnorm( vector < vector < double > > &A,
                                              vector < vector < double > > &B );
      
      double                       vvd_min( vector < vector < double > > &A );
      double                       vvd_max( vector < vector < double > > &A );
      void                         vvd_smult( vector < vector < double > > &A, double x );

      void                         matrix_info( QString qs, vector < vector < double > > &A );
      void                         set_title( QwtPlot *plot, QString title );

      map < QString, QwtPlot *>    plot_info;

      enum                         modes
      {
         MODE_IQIT
         ,MODE_SVD
         ,MODE_EFA
         ,MODE_EFA_DECOMP
      };

      modes                        current_mode;
      void                         mode_select();
      void                         mode_select( modes mode );

      void                         clear_svd();
      void                         clear_efa();
      void                         clear_efa_decomp();
      vector < double >            gradient( vector < double > & );
      bool                         efa_range_processing;

      bool                         convert_it_to_iq( QStringList files, QStringList & created_files, QString & error_msg );

      // efa decomp
      enum                         efa_decomp_method
      {
         METHOD_HYBRID
         ,METHOD_ITERATIVE
         ,METHOD_EXPLICIT
      };

      

      void                     initHybridEFA(
                                             vector < vector < int > >    & M,
                                             int                            num_sv,
                                             vector < vector < double > > & D,
                                             vector < vector < double > > & C,
                                             bool                         & converged,
                                             vector < vector < double > > & V_bar,
                                             int                            niter,
                                             double                         tol,
                                             vector < int >               & force_pos,
                                             // returns
                                             bool                         & failed,
                                             vector < vector < double > > & C_ret,
                                             vector < vector < double > > & T_ret
                                             );


      void                     initIterativeEFA(
                                                vector < vector < int > >    & M,
                                                int                            num_sv,
                                                vector < vector < double > > & D,
                                                vector < vector < double > > & C,
                                                bool                         & converged,
                                                vector < vector < double > > & V_bar,
                                                // returns
                                                bool                         & failed,
                                                vector < vector < double > > & C_ret
                                                );

      void                     initExplicitEFA(
                                               vector < vector < int > >    & M,
                                               int                            num_sv,
                                               vector < vector < double > > & D,
                                               vector < vector < double > > & C,
                                               bool                         & converged,
                                               vector < vector < double > > & V_bar,
                                               // returns
                                               bool                         & failed,
                                               vector < vector < double > > & T_ret
                                               );

      void                     runExplicitEFARotation(
                                                      vector < vector < int > >    & M,
                                                      vector < vector < double > > & D,
                                                      bool                         & failed, // also a return
                                                      vector < vector < double > > & C,
                                                      vector < vector < double > > & V_bar,
                                                      vector < vector < double > > & T,
                                                      int                            niter,
                                                      double                         tol,
                                                      vector < int >               & force_pos,
                                                      // returns
                                                      vector < vector < double > > & C_ret,
                                                      bool                         & converged
                                                      );

      void                     runIterativeEFARotation(
                                                       vector < vector < int > >    & M,
                                                       vector < vector < double > > & D,
                                                       bool                         & failed, // also a return
                                                       vector < vector < double > > & C,
                                                       vector < vector < double > > & V_bar,
                                                       vector < vector < double > > & T,
                                                       int                            niter,
                                                       double                         tol,
                                                       vector < int >               & force_pos,
                                                       // returns
                                                       vector < vector < double > > & C_ret,
                                                       bool                         & converged,
                                                       vector < double >            & dc,
                                                       int                          & k
                                                       );

      void                     EFAUpdateRotation(
                                                 vector < vector < int > >    & M,
                                                 vector < vector < double > > & C,
                                                 vector < vector < double > > & D,
                                                 vector < int >               & force_pos,
                                                 // returns
                                                 vector < vector < double > > & Cnew
                                                 );

      void                     EFAFirstRotation(
                                                vector < vector < int > >    & M,
                                                vector < vector < double > > & C,
                                                vector < vector < double > > & D,
                                                // returns
                                                vector < vector < double > > & Cnew
                                                );
      
      void                     efa_plot();
      void                     efa_decomp_plot();
      int                      efa_plot_count;

      map < norms, QString >   norm_name_map;

      bool                     get_name_norm_mode( QString fullname, QString &name, norms &norm_mode );

   private slots:

      // ------ data section 

      void data_selection_changed();
      void clear( );
      void to_mals_saxs();
      void save_plots();
      void color_rotate();
      void replot( bool keep_mode = false );

      void hide_data();

      // ------ editor section 

      void clear_display();
      void update_font();
      void save();

      void hide_editor();

      // ------ plot section 

      // void plot_data_zoomed( const QRectF &rect );
      // void plot_errors_zoomed( const QRectF &rect );

      void set_plot_errors();
      void set_plot_errors_rev();
      void set_plot_errors_sd();
      void set_plot_errors_pct();
      void set_plot_errors_ref();
      void set_plot_errors_group();

      void axis_x();
      void axis_y();
      void iq_it();

      // ------ process section 

      void set_norm_pw  ();
      void set_norm_avg ();

      void q_start_text ( const QString & );
      void q_end_text   ( const QString & );

      // void t_start_text ( const QString & );
      // void t_end_text   ( const QString & );

      void sv_selection_changed();

      void svd();
      void efa();
      void efa_decomp();
      void stop();

      void svd_plot( bool axis_change = true );
      void svd_save();
      void recon();

      void inc_rmsd_plot( bool axis_change = true );
      void rmsd_save();
      void inc_chi_plot ( bool axis_change = true );
      void inc_recon();
      void indiv_recon();
      
      void hide_process();

      // ----- bottom section


      void help();
      void cancel();

      // ----- util section

      void setup_norm();

      void setupGUI();

      void update_enables();

      // void adjust_wheel ( double );
      
   protected slots:

      void closeEvent(QCloseEvent *);
   
};

#endif
