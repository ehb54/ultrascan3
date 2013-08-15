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
                                map < QString, int >  &selected_files,
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
      QPushButton           * pb_to_hplc;
      QPushButton           * pb_replot;

      // ------ editor section 

      mQLabel               * lbl_editor;
      QFont                   ft;
      QTextEdit             * editor;
      QMenuBar              * m;

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

      QLabel                * lbl_t_range;
      mQLineEdit            * le_t_start;
      mQLineEdit            * le_t_end;

      QCheckBox             * cb_random;
      QLineEdit             * le_random;

      QLabel                * lbl_ev_count;
      QLineEdit             * le_ev_count;
      QLabel                * lbl_ev;
      QListBox              * lb_ev;

      QPushButton           * pb_svd;
      QPushButton           * pb_recon;


      // ------- bottom section

      QPushButton           * pb_help;
      QProgressBar          * progress;
      QPushButton           * pb_cancel;


      void                    editor_msg( QString color, QString msg );

      QString                 errormsg;

      map < QString, int >    selected_files;

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

      int                          count_selected();
      void                         clean_selected();

   private slots:


      // ------ data section 

      void data_selection_changed();
      void to_hplc();

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

      void t_start_text ( const QString & );
      void t_end_text   ( const QString & );

      void ev_count_text( const QString & );
      void ev_selection_changed();

      void svd();
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
