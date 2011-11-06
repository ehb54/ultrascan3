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
#include "us_saxs_util.h"

//standard C and C++ defs:

#include <map>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "us_hydrodyn_comparative.h"
#include "us_hydrodyn_pdb_tool_merge.h"
#include "qwt_wheel.h"

using namespace std;

class US_EXTERN US_Hydrodyn_Pdb_Tool : public QFrame
{

   Q_OBJECT

      friend class US_Hydrodyn_Pdb_Tool_Merge;

   public:
      US_Hydrodyn_Pdb_Tool(
                             csv csv1,
                             void *us_hydrodyn, 
                             QWidget *p = 0, 
                             const char *name = 0
                             );
      ~US_Hydrodyn_Pdb_Tool();

   private:

      void          *us_hydrodyn;

      US_Config     *USglobal;

      QLabel        *lbl_title;

      QFont         ft;
      QTextEdit     *editor;
      QMenuBar      *m;

      QPushButton   *pb_split_pdb;
      QPushButton   *pb_join_pdbs;
      QPushButton   *pb_merge;

      QLabel        *lbl_csv;
      QListView     *lv_csv;
      QTextEdit     *te_csv;
      QPushButton   *pb_csv_load_1;
      QPushButton   *pb_csv_load;
      QPushButton   *pb_csv_visualize;
      QPushButton   *pb_csv_save;
      QPushButton   *pb_csv_undo;
      QPushButton   *pb_csv_clear;
      QPushButton   *pb_csv_cut;
      QPushButton   *pb_csv_copy;
      QPushButton   *pb_csv_paste;
      QPushButton   *pb_csv_paste_new;
      QPushButton   *pb_csv_merge;
      QPushButton   *pb_csv_reseq;
      QPushButton   *pb_csv_check;
      QPushButton   *pb_csv_find_alt;
      QPushButton   *pb_csv_sel_clear;
      QPushButton   *pb_csv_sel_clean;
      QPushButton   *pb_csv_sel_invert;
      QPushButton   *pb_csv_sel_nearest_atoms;
      QPushButton   *pb_csv_sel_nearest_residues;
      QLabel        *lbl_csv_sel_msg;

      QLabel        *lbl_csv2;
      QListView     *lv_csv2;
      QTextEdit     *te_csv2;
      QwtWheel      *qwtw_wheel;
      QLabel        *lbl_pos_range;
      QPushButton   *pb_csv2_load_1;
      QPushButton   *pb_csv2_load;
      QPushButton   *pb_csv2_visualize;
      QPushButton   *pb_csv2_dup;
      QPushButton   *pb_csv2_save;
      QPushButton   *pb_csv2_undo;
      QPushButton   *pb_csv2_clear;
      QPushButton   *pb_csv2_cut;
      QPushButton   *pb_csv2_copy;
      QPushButton   *pb_csv2_paste;
      QPushButton   *pb_csv2_paste_new;
      QPushButton   *pb_csv2_merge;
      QPushButton   *pb_csv2_reseq;
      QPushButton   *pb_csv2_check;
      QPushButton   *pb_csv2_find_alt;
      QPushButton   *pb_csv2_sel_clear;
      QPushButton   *pb_csv2_sel_clean;
      QPushButton   *pb_csv2_sel_invert;
      QPushButton   *pb_csv2_sel_nearest_atoms;
      QPushButton   *pb_csv2_sel_nearest_residues;
      QLabel        *lbl_csv2_sel_msg;

      QPushButton   *pb_help;
      QPushButton   *pb_cancel;

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

      vector < csv >                csv2;
      vector < csv >                csv_undos;
      vector < vector < csv > >     csv2_undos;

      map < QString, unsigned int > csv_selected_element_counts;
      map < QString, unsigned int > csv2_selected_element_counts;

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif
      csv           csv1;
      unsigned int  csv2_pos;

      void          update_enables         ();
      void          update_enables_csv     ();
      void          update_enables_csv2    ();
      void          editor_msg             ( QString color, QString msg );
      void          csv_msg                ( QString color, QString msg );
      void          csv2_msg               ( QString color, QString msg );

      bool          selection_since_count_csv1;
      bool          selection_since_clean_csv1;
      pdb_sel_count last_count_csv1;
      bool          selection_since_count_csv2;
      bool          selection_since_clean_csv2;
      pdb_sel_count last_count_csv2;

      pdb_sel_count count_selected         ( QListView *lv );
      bool          any_selected           ( QListView *lv );
      void          clean_selection        ( QListView *lv );
      void          invert_selection       ( QListView *lv );
      QString       pdb_sel_count_msg      ( pdb_sel_count &counts );
      csv           to_csv                 ( QListView *lv, csv &ref_csv, bool only_selected = false );
      bool          is_selected            ( QListViewItem *lvi );
      bool          child_selected         ( QListViewItem *lvi );
      bool          all_children_selected  ( QListViewItem *lvi );
      QString       key                    ( QListViewItem *lvi );
      void          csv_to_lv              ( csv &csv1, QListView *lv );
      void          csv_setup_keys         ( csv &csv1 );
      void          list_csv_keys          ( csv &csv1 );
      csv           csv_clipboard;
      csv           merge_csvs             ( csv &csv1, csv &csv2 );
      csv           merge_csvs_dup_keys    ( csv &csv1, csv &csv2 );
      bool          no_dup_keys            ( csv &csv1, csv &csv2 );
      QString       list_keys              ( csv &csv1 );
      bool          merge_ok               ();
      
      QString       csv_to_pdb             ( csv &csv1, bool only_atoms = false );
      QStringList   csv_to_pdb_qsl         ( csv &csv1, bool only_atoms = false );
      QString       data_to_key            ( vector < QString > &data );
      QString       key_to_bottom_key      ( csv &csv1 );
      void          save_csv               ( QListView *lv );

      void          visualize              ( QListView *lv );

      void          load                   ( QListView *lv, QString &filename, bool only_first_nmr = false );
      void          load_from_qsl          ( QListView *lv, QStringList &pdb_text, QString title );

      void          csv2_redisplay         ( unsigned int pos );
      void          csv2_push              ( bool save_current = false );

      void          csv_sel_msg            ();
      void          csv2_sel_msg           ();

      void          sel_nearest_atoms      ( QListView *lv );
      double        pair_dist              ( QListViewItem *item1, QListViewItem *item2 );

      US_Hydrodyn_Pdb_Tool_Merge   *pdb_tool_merge_window;
      bool          pdb_tool_merge_widget;

      QString       check_csv              ( csv &csv1, vector < QString > &error_keys );
      QString       check_csv_for_alt      ( csv &csv1, QStringList &alt_residues );
      QString       errormsg;
      US_Saxs_Util  *usu;

      void          select_these           ( QListView *lv, vector < QString > &error_keys );

   private slots:
      
      void setupGUI();

      void clear_display();
      void update_font();
      void save();

      void split_pdb();
      void join_pdbs();
      void merge();

      void csv_selection_changed();
      void csv_load_1();
      void csv_load();
      void csv_save();
      void csv_cut();
      void csv_undo();
      void csv_clear();
      void csv_copy();
      void csv_paste();
      void csv_paste_new();
      void csv_merge();
      void csv_reseq();
      void csv_check();
      void csv_find_alt();
      void csv_visualize();
      void csv_sel_clear();
      void csv_sel_clean();
      void csv_sel_invert();
      void csv_sel_nearest_atoms();
      void csv_sel_nearest_residues();

      void csv2_selection_changed();
      void csv2_load_1();
      void csv2_load();
      void csv2_dup();
      void csv2_save();
      void csv2_undo();
      void csv2_clear();
      void csv2_cut();
      void csv2_copy();
      void csv2_paste();
      void csv2_paste_new();
      void csv2_merge();
      void csv2_reseq();
      void csv2_check();
      void csv2_find_alt();
      void csv2_visualize();
      void csv2_sel_clear();
      void csv2_sel_clean();
      void csv2_sel_invert();
      void csv2_sel_nearest_atoms();
      void csv2_sel_nearest_residues();

      void adjust_wheel( double );

      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
};

#endif
