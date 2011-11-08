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
#include "qwt/scrollbar.h"
#include "qwt/scrollzoomer.h"

using namespace std;

class US_EXTERN US_Hydrodyn_Saxs_Buffer : public QFrame
{
   Q_OBJECT

      friend class US_Hydrodyn_Batch;
      friend class US_Hydrodyn_Saxs;

   public:
      US_Hydrodyn_Saxs_Buffer(
                              csv csv1,
                              void *us_hydrodyn, 
                              QWidget *p = 0, 
                              const char *name = 0
                             );
      ~US_Hydrodyn_Saxs_Buffer();

   private:
      csv           csv1;

      void          *us_hydrodyn;

      US_Config     *USglobal;

      QLabel        *lbl_title;

      QTable        *t_csv;             

      QProgressBar  *progress;

      QLabel        *lbl_files;
      QPushButton   *pb_add_files;
      QPushButton   *pb_select_all;
      QPushButton   *pb_invert;
      QPushButton   *pb_clear_files;
      QListBox      *lb_files;
      // QPushButton   *pb_plot_files;
      QPushButton   *pb_save_avg;

      QPushButton   *pb_set_buffer;
      QLabel        *lbl_buffer;

      QPushButton   *pb_set_signal;
      QLabel        *lbl_signal;

      QLabel        *lbl_created_files;
      QListBox      *lb_created_files;

      QPushButton   *pb_select_all_created;
      QPushButton   *pb_save_created;

      QCheckBox     *cb_save_to_csv;
      QLineEdit     *le_csv_filename;
      QCheckBox     *cb_individual_files;

      QPushButton   *pb_replot_saxs;
      QPushButton   *pb_save_saxs_plot;
      QPushButton   *pb_set_target;
      QLabel        *lbl_current_target;

      QPushButton   *pb_start;
      QPushButton   *pb_run_current;
      QPushButton   *pb_run_best;
      QPushButton   *pb_stop;

      QFont         ft;
      QTextEdit     *editor;
      QMenuBar      *m;

      QPushButton   *pb_help;
      QPushButton   *pb_cancel;

      QwtPlot       *plot_dist;
      ScrollZoomer  *plot_dist_zoomer;

      bool          order_ascending;

      void          editor_msg( QString color, QString msg );
      void          editor_msg_qc( QColor qcolor, QString msg );

      bool          running;
      void          update_enables();

      bool          validate();
      bool          any_to_run();

      US_Hydrodyn_Saxs *saxs_window;
      bool             *saxs_widget;
      bool             activate_saxs_window();
      bool             validate_saxs_window();
      void             run_one();

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

      vector < vector < double > >       qs;
      vector < vector < double > >       Is;
      vector < vector < double > >       I_errors;
      vector < QString >                 names;

      vector < QString >                 csv_source_name_iqq;
      vector < double >                  saxs_q;
      vector < vector < double > >       saxs_iqq;
      map < QString, bool >              created_files_not_saved;
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

      bool disable_updates;

      QString qstring_common_head( QString s1, 
                                   QString s2 );
      QString qstring_common_tail( QString s1, 
                                   QString s2 );

      QString qstring_common_head( QStringList qsl );
      QString qstring_common_tail( QStringList qsl );

      QString last_load_dir;
      bool    save_files( QStringList files );
      bool    save_file( QString file );

   private slots:

      void setupGUI();

      void update_files();
      void update_created_files();
      void add_files();
      void clear_files();
      void select_all();
      void invert();
      void save_avg();
      void set_buffer();
      void set_signal();
      void select_all_created();
      void save_created();

      void table_value( int, int );

      void save_to_csv();

      void replot_saxs();
      void save_saxs_plot();
      void set_target();

      void start();
      void run_current();
      void run_best();
      void stop();

      void clear_display();
      void update_font();
      void save();

      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
   
};

#endif
