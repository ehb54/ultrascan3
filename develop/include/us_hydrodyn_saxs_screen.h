#ifndef US_HYDRODYN_SAXS_SCREEN_H
#define US_HYDRODYN_SAXS_SCREEN_H

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

#include "us_util.h"

//standard C and C++ defs:

#include <map>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "us_hydrodyn_saxs.h"

#include "qwt_wheel.h"

using namespace std;

class US_EXTERN US_Hydrodyn_Saxs_Screen : public QFrame
{
   Q_OBJECT

      friend class US_Hydrodyn_Batch;
      friend class US_Hydrodyn_Saxs;

   public:
      US_Hydrodyn_Saxs_Screen(
                              csv csv1,
                              void *us_hydrodyn, 
                              QWidget *p = 0, 
                              const char *name = 0
                             );
      ~US_Hydrodyn_Saxs_Screen();

   private:
      csv           csv1;

      void          *us_hydrodyn;

      US_Config     *USglobal;

      QLabel        *lbl_title;

      QTable        *t_csv;             

      QProgressBar  *progress;

      QCheckBox     *cb_save_to_csv;
      QLineEdit     *le_csv_filename;
      QPushButton   *pb_push;

      QPushButton   *pb_replot_saxs;
      QPushButton   *pb_save_saxs_plot;
      QPushButton   *pb_set_target;
      QLabel        *lbl_current_target;

      QPushButton   *pb_start;
      QPushButton   *pb_stop;

      QFont         ft;
      QTextEdit     *editor;
      QMenuBar      *m;

      QPushButton   *pb_help;
      QPushButton   *pb_cancel;

      QwtPlot       *plot_dist;
      QwtWheel      *qwtw_wheel;
      QLabel        *lbl_pos_range;
      QLabel        *lbl_message;
      QLabel        *lbl_message2;
      QLabel        *lbl_message3;
      QLabel        *lbl_message4;

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

      void             do_replot_saxs();

      double           best_fitness;

      QString saxs_header_iqq;

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif
      vector < vector < double > > qs;
      vector < vector < double > > Is;
      vector < vector < double > > I_errors;
      vector < QString >           names;

      vector < QString >           csv_source_name_iqq;
      vector < double >            saxs_q;
      vector < vector < double > > saxs_iqq;

      vector < QString >           messages;
      vector < QString >           messages2;
      vector < QString >           messages3;
      vector < QString >           messages4;
      vector < vector < double > > radiis;
      vector < vector < double > > intensitys;
      vector < double >            best_fit_radiuss;
      vector < double >            best_fit_delta_rhos;
      vector < double >            average_radiuss;
      vector < double >            average_delta_rhos;

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif
      void save_csv_saxs_iqq();

      csv  current_csv();

      void recompute_interval_from_points();
      void recompute_points_from_interval();


      void plot_one( 
                    QString           message,
                    QString           message2,
                    QString           message3,
                    QString           message4,
                    vector < double > radii,
                    vector < double > intensity,
                    double            best_fit_radius,
                    double            best_fit_delta_rho,
                    double            average_radius,
                    double            average_delta_rho
                    );

      void plot_pos( unsigned int );

      unsigned int last_plotted_pos;

      double max_y_range;

   private slots:

      void setupGUI();

      void table_value( int, int );

      void save_to_csv();
      void push();

      void replot_saxs();
      void save_saxs_plot();
      void set_target();

      void adjust_wheel( double );

      void start();
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
