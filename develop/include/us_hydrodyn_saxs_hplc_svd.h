#ifndef US_HYDRODYN_SAXS_HPLC_SVD_H
#define US_HYDRODYN_SAXS_HPLC_SVD_H

// QT defs:

#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_saxs.h"

using namespace std;

#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( disable: 4251 )
# endif
#endif

class US_EXTERN US_Hydrodyn_Saxs_Hplc_Svd : public QFrame
{
   Q_OBJECT

   public:
      US_Hydrodyn_Saxs_Hplc_Svd(
                                US_Hydrodyn_Saxs_Hplc *hplc_win, 
                                map < QString, int >  &hplc_selected_files,
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
      QListView             * lv_data;
      QPushButton           * pb_clear;
      QPushButton           * pb_to_hplc;
      QPushButton           * pb_color_rotate;
      QPushButton           * pb_replot;

      // ------ editor section 

      mQLabel               * lbl_editor;
      QFont                   ft;
      QTextEdit             * editor;
      QMenuBar              * mb_editor;

      // ------ plot section

      QwtPlot               * plot_data;
      ScrollZoomer          * plot_data_zoomer;
#ifdef QT4
      QwtPlotGrid           * grid_data;
#endif
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

      QLabel                * lbl_ev;
      QListBox              * lb_ev;

      QPushButton           * pb_svd;
      QPushButton           * pb_svd_plot;
      QPushButton           * pb_recon;


      // ------- bottom section

      QPushButton           * pb_help;
      QProgressBar          * progress;
      QPushButton           * pb_cancel;


      void                    editor_msg( QString color, QString msg );

      QString                 errormsg;

      map < QString, int >    hplc_selected_files;

      map < QString, unsigned int >       f_pos;

      map < QString, vector < QString > > f_qs_string;
      map < QString, vector < double > >  f_qs;

      map < QString, vector < double > >  f_Is;
      map < QString, vector < double > >  f_errors;
      map < QString, bool >               f_is_time;

      vector < QWidget * >                data_widgets;
      vector < QWidget * >                editor_widgets;
      vector < QWidget * >                process_widgets;

      void                                hide_widgets( vector < QWidget *> widgets, bool hide );

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

      bool                         axis_x_log;
      bool                         axis_y_log;

      void                         disable_all();

      mQLineEdit                   *le_last_focus;

      QStringList                  selected_files();
      set < QString >              get_current_files();
      set < QString >              get_sources();
      set < QString >              get_selected_sources();
      QListViewItem *              get_source_item( QString source );

      int                          selected_sources();
      void                         clean_selected();
      bool                         is_selected( QListViewItem *lvi );
      bool                         all_children_selected( QListViewItem *lvi );
      bool                         plotted_matches_selected();

      bool                         iq_it_state;

      void                         axis_x_title();
      void                         axis_y_title();

#ifdef QT4
      map < QString, QwtPlotCurve * >     plotted_curves;
#else
      map < QString, long >               plotted_curves;
#endif
      void                         add_i_of_t( QString source, QStringList files );
      void                         rescale();

      vector < vector < double > > svd_U;
      vector < vector < double > > svd_V;
      vector < double >            svd_D;
      vector < int >               svd_index;

      vector < double >            svd_x;
      vector < double >            svd_y;
      
      bool                         ev_plot;
      QStringList                  last_svd_data;
      QListViewItem *              lvi_last_depth( int d );

      vector < QColor >            plot_colors;
      QColorGroup                  cg_red;

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

      void plot_data_zoomed( const QwtDoubleRect &rect );

      void axis_x();
      void axis_y();
      void iq_it();

      // ------ process section 

      void q_start_text ( const QString & );
      void q_end_text   ( const QString & );

      // void t_start_text ( const QString & );
      // void t_end_text   ( const QString & );

      void ev_selection_changed();

      void svd();
      void svd_plot();
      void recon();
      
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
