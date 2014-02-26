#ifndef US_HYDRODYN_PDB_TOOL_H
#define US_HYDRODYN_PDB_TOOL_H

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
#include <q3listview.h>
//Added by qt3to4:
#include <QCloseEvent>

#include "us_util.h"
#include "us_saxs_util.h"
#include "us_pdb_util.h"

//standard C and C++ defs:

#include <map>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "us_hydrodyn_comparative.h"
#include "us_hydrodyn_pdb_tool_merge.h"
#include "us_hydrodyn_pdb_tool_renum.h"
#include "us_hydrodyn_pdb_tool_sort.h"
#include "us_mqt.h"
#include "qwt_wheel.h"

using namespace std;

#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( disable: 4251 )
# endif
#endif

class US_EXTERN US_Hydrodyn_Pdb_Tool : public Q3Frame
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
      Q3TextEdit     *editor;
      QMenuBar      *m;

      QPushButton   *pb_split_pdb;
      QPushButton   *pb_join_pdbs;
      QPushButton   *pb_merge;
      QPushButton   *pb_renum_pdb;
      QPushButton   *pb_hybrid_split;
      QPushButton   *pb_h_to_chainX;

      mQLabel        *lbl_csv;
      Q3ListView     *lv_csv;
      Q3TextEdit     *te_csv;
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
      QPushButton   *pb_csv_angle;
      QPushButton   *pb_csv_reseq;
      QPushButton   *pb_csv_check;
      QPushButton   *pb_csv_sort;
      QPushButton   *pb_csv_find_alt;
      QPushButton   *pb_csv_bm;
      QPushButton   *pb_csv_clash_report;
      QPushButton   *pb_csv_sel;
      QPushButton   *pb_csv_sel_clear;
      QPushButton   *pb_csv_sel_clean;
      QPushButton   *pb_csv_sel_invert;
      QPushButton   *pb_csv_sel_chain;
      QPushButton   *pb_csv_sel_nearest_atoms;
      QPushButton   *pb_csv_sel_nearest_residues;
      QLabel        *lbl_csv_sel_msg;

      mQLabel        *lbl_csv2;
      Q3ListView     *lv_csv2;
      Q3TextEdit     *te_csv2;
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
      QPushButton   *pb_csv2_angle;
      QPushButton   *pb_csv2_reseq;
      QPushButton   *pb_csv2_check;
      QPushButton   *pb_csv2_sort;
      QPushButton   *pb_csv2_find_alt;
      QPushButton   *pb_csv2_bm;
      QPushButton   *pb_csv2_clash_report;
      QPushButton   *pb_csv2_sel;
      QPushButton   *pb_csv2_sel_clear;
      QPushButton   *pb_csv2_sel_clean;
      QPushButton   *pb_csv2_sel_invert;
      QPushButton   *pb_csv2_sel_chain;
      QPushButton   *pb_csv2_sel_nearest_atoms;
      QPushButton   *pb_csv2_sel_nearest_residues;
      QLabel        *lbl_csv2_sel_msg;

      QPushButton   *pb_help;
      QPushButton   *pb_cancel;


      vector < csv >                csv2;
      vector < csv >                csv_undos;
      vector < vector < csv > >     csv2_undos;

      map < QString, unsigned int > csv_selected_element_counts;
      map < QString, unsigned int > csv2_selected_element_counts;

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

      pdb_sel_count count_selected         ( Q3ListView *lv );
      bool          any_selected           ( Q3ListView *lv );
      void          clean_selection        ( Q3ListView *lv );
      void          sel                    ( Q3ListView *lv );
      void          invert_selection       ( Q3ListView *lv );
      QString       pdb_sel_count_msg      ( pdb_sel_count &counts );
      csv           to_csv                 ( Q3ListView *lv, csv &ref_csv, bool only_selected = false );
      bool          is_selected            ( Q3ListViewItem *lvi );
      bool          child_selected         ( Q3ListViewItem *lvi );
      bool          all_children_selected  ( Q3ListViewItem *lvi );
      QString       key                    ( Q3ListViewItem *lvi );
      void          csv_to_lv              ( csv &csv1, Q3ListView *lv );
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
      void          save_csv               ( Q3ListView *lv );

      void          visualize              ( Q3ListView *lv );

      void          load                   ( Q3ListView *lv, QString &filename, bool only_first_nmr = false );
      void          load_from_qsl          ( Q3ListView *lv, QStringList &pdb_text, QString title );

      void          csv2_redisplay         ( unsigned int pos );
      void          csv2_push              ( bool save_current = false );

      void          csv_sel_msg            ();
      void          csv2_sel_msg           ();

      void          sel_nearest_atoms      ( Q3ListView *lv );
      void          sel_nearest_residues   ( Q3ListView *lv );
      double        pair_dist              ( Q3ListViewItem *item1, Q3ListViewItem *item2 );

      // compute minimum pair distance between chains return respective keys in key_1, key_2
      double        minimum_pair_distance  ( Q3ListView *lv,
                                             QString    chain_1,
                                             QString    chain_2,
                                             QString  & key_1,
                                             QString  & key_2 );

      US_Hydrodyn_Pdb_Tool_Merge   *pdb_tool_merge_window;
      bool          pdb_tool_merge_widget;

      QString       check_csv              ( csv &csv1, vector < QString > &error_keys );
      QString       check_csv_for_alt      ( csv &csv1, QStringList &alt_residues );
      QString       errormsg;
      US_Saxs_Util  *usu;

      void          select_these           ( Q3ListView *lv, vector < QString > &error_keys );

      void          replace_selected_residues ( Q3ListView *lv, csv &csv_use, QString from, QString to );
      void          distances              ( Q3ListView *lv );
      QString       get_atom_name          ( Q3ListViewItem *lvi );
      QString       get_atom_number        ( Q3ListViewItem *lvi );
      QString       get_chain_id           ( Q3ListViewItem *lvi );
      QString       get_model_id           ( Q3ListViewItem *lvi );
      QString       get_residue_name       ( Q3ListViewItem *lvi );
      QString       get_residue_number     ( Q3ListViewItem *lvi );
      QStringList   atom_set               ( Q3ListView *lv ); // returns a list of selected atoms
      QStringList   chain_set              ( Q3ListView *lv ); // returns a list of selected chains
      QStringList   model_set              ( Q3ListView *lv ); // returns a list of selected models
      QStringList   atom_sel_rasmol        ( Q3ListView *lv ); // returns a list of selected atoms with chain and atom
      void          select_model           ( Q3ListView *lv, QString model ); // selects just that model
      void          select_model           ( Q3ListView *lv, const set < QString > & models );
      void          select_chain           ( Q3ListView *lv ); // selected a set of chains
      void          select_chain           ( Q3ListView *lv, QStringList chains ); // selected a set of chains
      void          select_chain           ( Q3ListView *lv, QString chain ); // selected one chain
      void          compute_angle          ( Q3ListView *lv );

      csv           reseq_csv              ( Q3ListView *lv, csv &ref_csv, bool only_selected = false );

      void          split_pdb_by_residue   ( QFile &f );

      void          select_residues_with_atoms_selected( Q3ListView *lv );

      set    < Q3ListViewItem * >            get_exposed_set         ( Q3ListView *lv, 
                                                                      double max_asa,
                                                                      bool only_selected = false );
      set    < Q3ListViewItem * >            get_exposed_set_naccess ( Q3ListView *lv, 
                                                                      double max_asa, 
                                                                      bool either_sc_or_mc,
                                                                      bool only_selected = false );
      
      vector < QString >                    get_models              ( Q3ListView *lv );
      vector < vector < Q3ListViewItem * > > separate_models         ( Q3ListView *lv );
      vector < QStringList >                separate_models         ( csv &ref_csv );

      Q3Process  * naccess;
      QString     naccess_last_pdb;
      QStringList naccess_result_data;
      bool        naccess_running;
      bool        naccess_run                    ( QString pdb );
      
      void        do_sort( Q3ListView *lv );

      void        do_bm( Q3ListView *lv );

      vector < QWidget * > panel1_widgets;
      vector < QWidget * > panel2_widgets;

      void hide_widgets( vector < QWidget * >, bool do_hide = true );

      bool        bm_active;

   private slots:
      
      void setupGUI();

      void clear_display();
      void update_font();
      void save();

      void split_pdb();
      void join_pdbs();
      void merge();
      void renum_pdb();
      void hybrid_split();
      void h_to_chainX();

      void hide_csv();

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
      void csv_angle();
      void csv_reseq();
      void csv_check();
      void csv_sort();
      void csv_find_alt();
      void csv_bm();
      void csv_clash_report();
      void csv_visualize();
      void csv_sel();
      void csv_sel_clear();
      void csv_sel_clean();
      void csv_sel_invert();
      void csv_sel_chain();
      void csv_sel_nearest_atoms();
      void csv_sel_nearest_residues();

      void hide_csv2();

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
      void csv2_angle();
      void csv2_reseq();
      void csv2_check();
      void csv2_sort();
      void csv2_find_alt();
      void csv2_bm();
      void csv2_clash_report();
      void csv2_visualize();
      void csv2_sel();
      void csv2_sel_clear();
      void csv2_sel_clean();
      void csv2_sel_invert();
      void csv2_sel_chain();
      void csv2_sel_nearest_atoms();
      void csv2_sel_nearest_residues();

      void adjust_wheel( double );

      void        naccess_readFromStdout         ();
      void        naccess_readFromStderr         ();
      void        naccess_launchFinished         ();
      void        naccess_processExited          ();

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
