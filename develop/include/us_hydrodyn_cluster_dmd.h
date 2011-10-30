#ifndef US_HYDRODYN_CLUSTER_DMD_H
#define US_HYDRODYN_CLUSTER_DMD_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qtextedit.h>
#include <qmenubar.h>
#include <qprinter.h>
#include <qtable.h>

#include "us_util.h"
#include "us_hydrodyn_pdbdefs.h"
#include "us_hydrodyn_batch.h"
#include "us_hydrodyn_comparative.h"

//standard C and C++ defs:

#include <map>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace std;

class US_EXTERN US_Hydrodyn_Cluster_Dmd : public QDialog
{
   Q_OBJECT

      friend class US_Hydrodyn_Cluster;

   public:
      US_Hydrodyn_Cluster_Dmd(
                              csv &csv1,
                              void *us_hydrodyn, 
                              QWidget *p = 0, 
                              const char *name = 0
                             );
      ~US_Hydrodyn_Cluster_Dmd();

   private:
      csv           csv1;
      csv           *original_csv1;
      csv           csv_copy;

      void          *us_hydrodyn;
      void          *cluster_window;

      US_Config     *USglobal;

      QLabel        *lbl_title;

      QTable        *t_csv;             

      QPushButton   *pb_select_all;
      QPushButton   *pb_copy;
      QPushButton   *pb_paste;
      QPushButton   *pb_dup;
      QPushButton   *pb_delete;
      QPushButton   *pb_load;
      QPushButton   *pb_save_csv;

      QFont         ft;
      QTextEdit     *editor;
      QMenuBar      *m;

      QPushButton   *pb_ok;
      QPushButton   *pb_help;
      QPushButton   *pb_cancel;

      void          editor_msg( QString color, QString msg );

      csv           current_csv();
      void          recompute_interval_from_points( unsigned int );
      void          recompute_points_from_interval( unsigned int );
      unsigned int  interval_starting_row;
      void          reset_csv();
      void          reload_csv();

      bool          disable_updates;

   private slots:

      void setupGUI();

      void table_value( int, int );
      void update_enables();

      void select_all();
      void copy();
      void paste();
      void dup();
      void delete_rows();
      void load();
      void save_csv();

      void clear_display();
      void update_font();
      void save();

      void ok();
      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
   
};

#endif
