#ifndef US_HYDRODYN_SAXS_HPLC_SVD_H
#define US_HYDRODYN_SAXS_HPLC_SVD_H

// QT defs:

#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_saxs.h"
//Added by qt3to4:
#include <QCloseEvent>
#include <Q3Frame>
#include <QLabel>

using namespace std;

#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( disable: 4251 )
# endif
#endif

class US_EXTERN US_Hydrodyn_Saxs_Hplc_Svd : public Q3Frame
{
   Q_OBJECT

   public:
      US_Hydrodyn_Saxs_Hplc_Svd(
                                US_Hydrodyn_Saxs_Hplc *hplc_win, 
                                vector < QString > hplc_selected_files,
                                QWidget *p = 0, 
                                const char *name = 0
                                );

      ~US_Hydrodyn_Saxs_Hplc_Svd();

   private:
      US_Config             * USglobal;

      US_Hydrodyn_Saxs_Hplc * hplc_win;
      US_Hydrodyn           * ush_win;

      // ------ data section 

      mQLabel               * lbl_data;
      Q3ListView             * lv_data;
      QPushButton           * pb_clear;
      QPushButton           * pb_to_hplc;
      QPushButton           * pb_color_rotate;
      QPushButton           * pb_replot;

      // ------ editor section 

      mQLabel               * lbl_editor;
      QFont                   ft;
      Q3TextEdit             * editor;
      QMenuBar              * mb_editor;

      // ------ plot section

      QwtPlot               * plot_data;
      ScrollZoomer          * plot_data_zoomer;
#ifdef QT4
      QwtPlotGrid           * grid_data;
#endif

      QwtPlot               * plot_errors;
      ScrollZoomer          * plot_errors_zoomer;
#ifdef QT4
      QwtPlotGrid           * grid_errors;
#endif

      QCheckBox             * cb_plot_errors;
      QCheckBox             * cb_plot_errors_rev;
      QCheckBox             * cb_plot_errors_sd;
      QCheckBox             * cb_plot_errors_pct;
      QCheckBox             * cb_plot_errors_ref;
      QCheckBox             * cb_plot_errors_group;

      QPushButton           * pb_iq_it;
      QPushButton           * pb_axis_x;
      QPushButton           * pb_axis_y;

      // ------ process section

      mQLabel               * lbl_process;
      // QLabel                * lbl_wheel_pos;
      // QwtWheel              * qwtw_wheel;

      QLabel                * lbl_q_range;
      mQLineEdit            * le_q_start;
      mQLineEdit            * le_q_end;

      // QLabel                * lbl_t_range;
      // mQLineEdit            * le_t_start;
      // mQLineEdit            * le_t_end;

      // QCheckBox             * cb_random;
      // QLineEdit             * le_random;

      QPushButton           * pb_svd;
      QPushButton           * pb_stop;

      QLabel                * lbl_ev;
      Q3ListBox              * lb_ev;

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
      Q3ProgressBar          * progress;
      QPushButton           * pb_cancel;


      void                    editor_msg( QString color, QString msg );

      QString                 errormsg;

      vector < QString >      hplc_selected_files;

      map < QString, unsigned int >       f_pos;

      map < QString, vector < QString > > f_qs_string;
      map < QString, vector < double > >  f_qs;

      map < QString, vector < double > >  f_Is;
      map < QString, vector < double > >  f_errors;
      map < QString, bool >               f_is_time;

      vector < QWidget * >                data_widgets;
      vector < QWidget * >                editor_widgets;
      vector < QWidget * >                process_widgets;
      vector < QWidget * >                errors_widgets;

      void                                hide_widgets( vector < QWidget *> widgets, bool hide );

      void plot_files();
      void plot_files( QStringList files );
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
      Q3ListViewItem *              get_source_item( QString source );

      int                          selected_sources();
      void                         clean_selected();
      bool                         is_selected( Q3ListViewItem *lvi );
      bool                         all_children_selected( Q3ListViewItem *lvi );
      bool                         plotted_matches_selected();

      bool                         iq_it_state;

      void                         axis_x_title();
      void                         axis_y_title();

#ifdef QT4
      map < QString, QwtPlotCurve * >     plotted_curves;
#else
      map < QString, long >               plotted_curves;
#endif
      void                         add_i_of_t( QString source, QStringList files, bool do_update_enables = true );
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
      Q3ListViewItem *              lvi_last_depth( int d );

      vector < QColor >            plot_colors;
      QColorGroup                  cg_red;

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
      bool                         get_plot_files( QStringList &use_list, QStringList &use_ref_list );

      vector < double >            plot_errors_jumps;
      void                         plot_errors_jump_markers();
      set < QString >              plot_errors_reference;

      bool                         setup_save( QString tag, QString & fname );

   private slots:

      // ------ data section 

      void data_selection_changed();
      void clear();
      void to_hplc();
      void color_rotate();
      void replot();

      void hide_data();

      // ------ editor section 

      void clear_display();
      void update_font();
      void save();

      void hide_editor();

      // ------ plot section 

      // void plot_data_zoomed( const QwtDoubleRect &rect );
      // void plot_errors_zoomed( const QwtDoubleRect &rect );

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

      void q_start_text ( const QString & );
      void q_end_text   ( const QString & );

      // void t_start_text ( const QString & );
      // void t_end_text   ( const QString & );

      void sv_selection_changed();

      void svd();
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

      void setupGUI();

      void update_enables();

      // void adjust_wheel ( double );
      
   protected slots:

      void closeEvent(QCloseEvent *);
   
};

#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( default: 4251 )
# endif
#endif

#endif
