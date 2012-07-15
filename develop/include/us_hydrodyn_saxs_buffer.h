#ifndef US_HYDRODYN_SAXS_BUFFER_H
#define US_HYDRODYN_SAXS_BUFFER_H

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

#include "us_util.h"

//standard C and C++ defs:

#include <map>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "us_hydrodyn_saxs.h"
#include "us_hydrodyn_saxs_buffer_conc.h"
#include "qwt/scrollbar.h"
#include "qwt/scrollzoomer.h"

using namespace std;

struct crop_undo_data
{
#ifdef WIN32
  #pragma warning ( disable: 4251 )
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
  #pragma warning ( default: 4251 )
#endif
};

class US_EXTERN US_Hydrodyn_Saxs_Buffer : public QFrame
{
   Q_OBJECT

      friend class US_Hydrodyn_Batch;
      friend class US_Hydrodyn_Saxs;
      friend class US_Hydrodyn_Saxs_Buffer_Conc;

   public:
      US_Hydrodyn_Saxs_Buffer(
                              csv csv1,
                              void *us_hydrodyn, 
                              QWidget *p = 0, 
                              const char *name = 0
                             );
      ~US_Hydrodyn_Saxs_Buffer();

      void add_plot( QString           name,
                     vector < double > q,
                     vector < double > I );

      void add_plot( QString           name,
                     vector < double > q,
                     vector < double > I,
                     vector < double > errors );

   private:
      csv           csv1;

      void          *us_hydrodyn;

      US_Config     *USglobal;

      QLabel        *lbl_title;

      QTable        *t_csv;             

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

      QPushButton   *pb_set_buffer;
      QLabel        *lbl_buffer;

      QPushButton   *pb_set_empty;
      QLabel        *lbl_empty;

      QPushButton   *pb_set_signal;
      QLabel        *lbl_signal;

      QLabel        *lbl_np;
      QButtonGroup  *bg_np;
      QRadioButton  *rb_np_crop;
      QRadioButton  *rb_np_min;
      QRadioButton  *rb_np_ignore;
      QRadioButton  *rb_np_ask;

      QCheckBox     *cb_multi_sub;
      QCheckBox     *cb_multi_sub_avg;
      QCheckBox     *cb_multi_sub_conc_avg;

      QLabel        *lbl_created_files;
      QListBox      *lb_created_files;
      QLabel        *lbl_selected_created;

      QPushButton   *pb_select_all_created;
      QPushButton   *pb_adjacent_created;
      QPushButton   *pb_save_created_csv;
      QPushButton   *pb_save_created;
      QPushButton   *pb_show_created;
      QPushButton   *pb_show_only_created;

      QPushButton   *pb_start;
      QPushButton   *pb_run_current;
      QPushButton   *pb_run_divide;
      QPushButton   *pb_run_best;
      QPushButton   *pb_stop;

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

      QPushButton   *pb_select_vis;
      QPushButton   *pb_remove_vis;
      QPushButton   *pb_crop_vis;
      QPushButton   *pb_crop_common;
      QPushButton   *pb_crop_left;
      QPushButton   *pb_crop_undo;
      QPushButton   *pb_crop_right;
      QPushButton   *pb_legend;
      QPushButton   *pb_axis_x;
      QPushButton   *pb_axis_y;

      QCheckBox     *cb_guinier;
      QLabel        *lbl_guinier;

      bool          order_ascending;

      void          editor_msg( QString color, QString msg );
      void          editor_msg_qc( QColor qcolor, QString msg );

      bool          running;

      bool          validate();
      bool          any_to_run();

      US_Hydrodyn_Saxs *saxs_window;
      bool             *saxs_widget;
      bool             activate_saxs_window();
      void             run_one();
      void             run_one_divide();

      void             do_replot_saxs();

      double           best_fitness;

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

      map < QString, bool >               created_files_not_saved;

      map < QString, double >             current_concs( bool quiet = false );
      map < QString, double >             window_concs();

      vector < crop_undo_data >           crop_undos;

      bool                                is_nonzero_vector( vector < double > &v );

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif
      void save_csv_saxs_iqq();

      csv  current_csv();

      void recompute_interval_from_points();
      void recompute_points_from_interval();

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
      US_Hydrodyn_Saxs_Buffer_Conc *conc_window;
      bool                         conc_widget;
      void                         update_csv_conc();
      bool                         all_selected_have_nonzero_conc();

      void                         delete_zoomer_if_ranges_changed();
      QString                      vector_double_to_csv( vector < double > vd );

      bool                         adjacent_ok( QString name );

      void                         avg     ( QStringList files );
      void                         conc_avg( QStringList files );
      QString                      last_created_file;
      void                         zoom_info();
      void                         clear_files( QStringList files );
      void                         to_created( QString file );
      void                         add_files( QStringList files );
      bool                         axis_x_log;
      bool                         axis_y_log;

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
      void set_buffer();
      void set_empty();
      void set_signal();
      void select_all_created();
      void adjacent_created();
      void save_created_csv();
      void save_created();
      void show_created();
      void show_only_created();

      void table_value( int, int );

      void start();
      void run_current();
      void run_divide();
      void run_best();
      void stop();

      void clear_display();
      void update_font();
      void save();

      void cancel();
      void help();

      void plot_zoomed( const QwtDoubleRect &rect );
      void plot_mouse ( const QMouseEvent &me );

      void select_vis();
      void remove_vis();
      void crop_left();
      void crop_common();
      void crop_vis();
      void crop_undo();
      void crop_right();
      void legend();
      void axis_x();
      void axis_y();
      void legend_set();
      void guinier();

      void rename_created( QListBoxItem *, const QPoint & );

   protected slots:

      void closeEvent(QCloseEvent *);
   
};

#endif
