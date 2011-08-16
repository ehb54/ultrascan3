#ifndef US_HYDRODYN_SAXS_SEARCH_H
#define US_HYDRODYN_SAXS_SEARCH_H

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

using namespace std;

class US_EXTERN US_Hydrodyn_Saxs_Search : public QFrame
{
   Q_OBJECT

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

      QTable        *t_csv;             

      QProgressBar  *progress;

      QPushButton   *pb_replot_saxs;
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

      bool          order_ascending;

      void          editor_msg( QString color, QString msg );

      bool          running;
      void          update_enables();

      bool          validate();
      bool          any_to_run();

      US_Hydrodyn_Saxs *saxs_window;
      bool             *saxs_widget;
      bool             validate_saxs_window();
      void             run_one();

      void             do_replot_saxs();

      double           best_fitness;

      vector < vector < double > > qs;
      vector < vector < double > > Is;
      vector < vector < double > > I_errors;
      vector < QString >           names;

   private slots:

      void setupGUI();

      void sort_column( int );
      void table_updated( int, int );

      void replot_saxs();
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
