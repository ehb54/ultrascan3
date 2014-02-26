#ifndef US_HYDRODYN_SAXS_SEARCH_H
#define US_HYDRODYN_SAXS_SEARCH_H

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
#include <q3table.h>
//Added by qt3to4:
#include <QCloseEvent>

#include "us_util.h"

//standard C and C++ defs:

#include <map>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "us_hydrodyn_saxs.h"

#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( disable: 4251 )
# endif
#endif

using namespace std;


class US_EXTERN US_Hydrodyn_Saxs_Search : public Q3Frame
{
   Q_OBJECT

      friend class US_Hydrodyn_Batch;
      friend class US_Hydrodyn_Saxs;

   public:
      US_Hydrodyn_Saxs_Search(
                              csv csv1,
                              void *us_hydrodyn, 
                              QWidget *p = 0, 
                              const char *name = 0
                             );
      ~US_Hydrodyn_Saxs_Search();

   private:
      csv           csv1;

      void          *us_hydrodyn;

      US_Config     *USglobal;

      QLabel        *lbl_title;

      Q3Table        *t_csv;             

      Q3ProgressBar  *progress;

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
      Q3TextEdit     *editor;
      QMenuBar      *m;

      QPushButton   *pb_help;
      QPushButton   *pb_cancel;

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

      QString saxs_header_iqq;

      vector < vector < double > > qs;
      vector < vector < double > > Is;
      vector < vector < double > > I_errors;
      vector < QString >           names;

      vector < QString >           csv_source_name_iqq;
      vector < double >            saxs_q;
      vector < vector < double > > saxs_iqq;
      void save_csv_saxs_iqq();

      csv  current_csv();

      void recompute_interval_from_points();
      void recompute_points_from_interval();

      bool not_active_warning();

   private slots:

      void setupGUI();

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

#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( default: 4251 )
# endif
#endif

#endif
