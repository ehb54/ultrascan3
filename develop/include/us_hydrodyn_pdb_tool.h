#ifndef US_HYDRODYN_PDB_TOOL_H
#define US_HYDRODYN_PDB_TOOL_H

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
#include <qlistview.h>

#include "us_util.h"

//standard C and C++ defs:

#include <map>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "us_hydrodyn_comparative.h"
#include "qwt_wheel.h"

using namespace std;

class US_EXTERN US_Hydrodyn_Pdb_Tool : public QFrame
{
   Q_OBJECT

   public:
      US_Hydrodyn_Pdb_Tool(
                             csv csv1,
                             void *us_hydrodyn, 
                             QWidget *p = 0, 
                             const char *name = 0
                             );
      ~US_Hydrodyn_Pdb_Tool();

   private:
      csv           csv1;
      csv           csv2;

      void          *us_hydrodyn;

      US_Config     *USglobal;

      QLabel        *lbl_title;

      QFont         ft;
      QTextEdit     *editor;
      QMenuBar      *m;

      QLabel        *lbl_csv;
      QListView     *lv_csv;
      QTextEdit     *te_csv;
      QPushButton   *pb_csv_load;
      QPushButton   *pb_csv_visualize;
      QPushButton   *pb_csv_save;
      QPushButton   *pb_csv_undo;
      QPushButton   *pb_csv_cut;
      QPushButton   *pb_csv_copy;
      QPushButton   *pb_csv_paste;
      QPushButton   *pb_csv_merge;
      QPushButton   *pb_csv_reseq;

      QLabel        *lbl_csv2;
      QListView     *lv_csv2;
      QTextEdit     *te_csv2;
      QwtWheel      *qwtw_wheel;
      QLabel        *lbl_pos_range;
      QPushButton   *pb_csv2_load;
      QPushButton   *pb_csv2_visualize;
      QPushButton   *pb_csv2_dup;
      QPushButton   *pb_csv2_undo;
      QPushButton   *pb_csv2_cut;
      QPushButton   *pb_csv2_copy;
      QPushButton   *pb_csv2_paste;
      QPushButton   *pb_csv2_merge;
      QPushButton   *pb_csv2_reseq;

      QPushButton   *pb_help;
      QPushButton   *pb_cancel;

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

      vector < csv >  csv_undos;
      vector < csv >  csv2_undos;

      map < QString, unsigned int > csv_selected_element_counts;
      map < QString, unsigned int > csv2_selected_element_counts;

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

      void          update_enables();
      void          update_enables_csv();
      void          update_enables_csv2();
      void          editor_msg( QString color, QString msg );
      void          csv_msg   ( QString color, QString msg );
      void          csv2_msg  ( QString color, QString msg );

      bool          selection_since_count_csv1;
      unsigned int  last_count_csv1;
      bool          selection_since_count_csv2;
      unsigned int  last_count_csv2;

      unsigned int  count_selected  ( QListView *lv );
      bool          any_selected    ( QListView *lv );
      csv           to_csv          ( QListView *lv, csv &ref_csv, bool only_selected = false );
      bool          is_selected     ( QListViewItem *lvi );
      bool          child_selected  ( QListViewItem *lvi );
      QString       key             ( QListViewItem *lvi );
      void          csv_to_lv       ( csv &csv1, QListView *lv );
      void          csv_setup_keys  ( csv &csv1 );
      void          list_csv_keys   ( csv &csv1 );
      csv           csv_clipboard;

      bool          merge_ok();

   private slots:
      
      void setupGUI();

      void clear_display();
      void update_font();
      void save();

      void csv_selection_changed();
      void csv_load();
      void csv_save();
      void csv_cut();
      void csv_undo();
      void csv_copy();
      void csv_paste();
      void csv_merge();
      void csv_reseq();
      void csv_visualize();

      void csv2_selection_changed();
      void csv2_load();
      void csv2_dup();
      void csv2_undo();
      void csv2_cut();
      void csv2_copy();
      void csv2_paste();
      void csv2_merge();
      void csv2_reseq();
      void csv2_visualize();

      void adjust_wheel( double );

      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
   
};

#endif
