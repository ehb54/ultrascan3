#ifndef US_HYDRODYN_SAXS_HPLC_H
#define US_HYDRODYN_SAXS_HPLC_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qtextedit.h>
#include <qprogressbar.h>
#include <qmenubar.h>
#include <qfileinfo.h>
#include <qprinter.h>
#include <qradiobutton.h>
#include <qtable.h>
#include <qwt_plot_zoomer.h>
#include <qwt_wheel.h>

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

using namespace std;

class US_EXTERN US_Hydrodyn_Saxs_Hplc : public QFrame
{
   Q_OBJECT

      friend class US_Hydrodyn_Batch;
      friend class US_Hydrodyn_Saxs;
      friend class US_Hydrodyn_Saxs_Hplc_Conc;

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
                     bool              is_time = false );

      void add_plot( QString           name,
                     vector < double > q,
                     vector < double > I,
                     vector < double > errors,
                     bool              is_time = false );

   private:
      csv           csv1;

      void          *us_hydrodyn;

      US_Config     *USglobal;

      QLabel        *lbl_title;

      QProgressBar  *progress;

      QLabel        *lbl_files;
      QLabel        *lbl_dir;
      QPushButton   *pb_add_files;
      QPushButton   *pb_similar_files;
      QPushButton   *pb_conc;
      QPushButton   *pb_clear_files;

      QPushButton   *pb_regex_load;
      QLineEdit     *le_regex;
      QLineEdit     *le_regex_args;

      QPushButton   *pb_select_all;
      QPushButton   *pb_invert;
      QPushButton   *pb_adjacent;
      QPushButton   *pb_join;
      QPushButton   *pb_to_saxs;
      QPushButton   *pb_view;
      QPushButton   *pb_rescale;

      QListBox      *lb_files;
      QLabel        *lbl_selected;
      // QPushButton   *pb_plot_files;
      QPushButton   *pb_avg;
      QPushButton   *pb_normalize;
      QPushButton   *pb_conc_avg;
      QPushButton   *pb_smooth;
      QPushButton   *pb_repeak;
      QPushButton   *pb_create_i_of_t;


      QPushButton   *pb_set_hplc;
      QLabel        *lbl_hplc;

      QPushButton   *pb_set_empty;
      QLabel        *lbl_empty;

      QPushButton   *pb_set_signal;
      QLabel        *lbl_signal;

      QLabel        *lbl_created_files;
      QListBox      *lb_created_files;
      QLabel        *lbl_selected_created;

      QPushButton   *pb_select_all_created;
      QPushButton   *pb_adjacent_created;
      QPushButton   *pb_save_created_csv;
      QPushButton   *pb_save_created;
      QPushButton   *pb_show_created;
      QPushButton   *pb_show_only_created;

      QFont         ft;
      QTextEdit     *editor;
      QMenuBar      *m;

      QPushButton   *pb_help;
      QPushButton   *pb_cancel;

      QwtPlot       *plot_dist;
      ScrollZoomer  *plot_dist_zoomer;
#ifdef QT4
      QwtPlotGrid   *grid_saxs;
      bool          legend_vis;
#endif

      QPushButton   *pb_wheel_start;
      QwtWheel      *qwtw_wheel;
      QPushButton   *pb_wheel_cancel;
      QPushButton   *pb_wheel_save;

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
  #pragma warning ( disable: 4251 )
#endif
      vector < QColor >                  plot_colors;

      map < QString, vector < QString > > f_qs_string;
      map < QString, vector < double > >  f_qs;
      map < QString, vector < double > >  f_Is;
      map < QString, vector < double > >  f_errors;
      map < QString, unsigned int >       f_pos;

      map < QString, QString >            f_name;
      map < QString, bool >               f_is_time;

      map < QString, bool >               created_files_not_saved;

      map < QString, double >             current_concs( bool quiet = false );
      map < QString, double >             window_concs();

      vector < crop_undo_data >           crop_undos;

      bool                                is_nonzero_vector( vector < double > &v );

      vector < double >                   union_q( QStringList files );

#ifdef QT4
      map < QString, QwtPlotCurve * >     plotted_curves;
#else
      map < QString, long >               plotted_curves;
#endif

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif
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
      bool    save_file( QString file );
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

   private slots:

      void setupGUI();

      void update_enables();
      void update_files();
      void update_created_files();
      void add_files();
      void similar_files();
      void conc();
      void clear_files();
      void regex_load();

      void select_all();
      void invert();
      void join();
      void adjacent();
      void to_saxs();
      void view();
      void rescale();
      void avg();
      void normalize();
      void conc_avg();
      void smooth();
      void repeak();
      void create_i_of_t();
      void set_hplc();
      void set_empty();
      void set_signal();
      void select_all_created();
      void adjacent_created();
      void save_created_csv();
      void save_created();
      void show_created();
      void show_only_created();

      void wheel_start();
      void wheel_cancel();
      void wheel_save();

      void clear_display();
      void update_font();
      void save();

      void cancel();
      void help();

      void plot_zoomed( const QwtDoubleRect &rect );
      void plot_mouse ( const QMouseEvent &me );

      void adjust_wheel ( double );

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

      void rename_created( QListBoxItem *, const QPoint & );

   protected slots:

      void closeEvent(QCloseEvent *);
   
};

#endif
