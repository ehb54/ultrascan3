#ifndef US_HYDRODYN_SAXS_PDB_TOOL_MERGE_H
#define US_HYDRODYN_SAXS_PDB_TOOL_MERGE_H

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
#include <qlistview.h>

#include "us_util.h"
#include "us_hydrodyn_comparative.h"

//standard C and C++ defs:

#include <map>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace std;

class range_entry
{
 public: 
   QString      chain;
   unsigned int start;
   unsigned int end;
   friend ostream& operator<<(ostream&, const range_entry&);
};

struct pdb_sel_count
{
   unsigned int models;
   unsigned int chains;
   unsigned int residues;
   unsigned int atoms;
   bool         model_partial;
   bool         chain_partial;
   bool         residue_partial;
   unsigned int not_selected_atoms;
};

class US_EXTERN US_Hydrodyn_Pdb_Tool_Merge : public QFrame
{
   Q_OBJECT
      friend class US_Hydrodyn_Pdb_Tool;

   public:
      US_Hydrodyn_Pdb_Tool_Merge(
                                 void *us_hydrodyn, 
                                 void *pdb_tool_window,
                                 QWidget *p = 0, 
                                 const char *name = 0
                                 );
      ~US_Hydrodyn_Pdb_Tool_Merge();

   private:

      void                 *us_hydrodyn;
      void                 *pdb_tool_window;

      US_Config            *USglobal;

      QLabel               *lbl_title;

      QTable               *t_csv;
      // csv layout for cut/splice
      // 
      // chain|cut start|cut end|fit start|fit end|merge start|merge end

      // ? duplicate lines
      // ? reorder lines
      // set selected from cut, fit, merge
      // set selected to cut, fit, merge

      QPushButton          *pb_sel_auto;
      QPushButton          *pb_sel_from_to_merge;
      QPushButton          *pb_sel_from_to_fit;
      QPushButton          *pb_sel_to_to_fit;
      QPushButton          *pb_sel_to_to_cut;

      QPushButton          *pb_clear;
      QPushButton          *pb_load;
      QPushButton          *pb_validate;
      QPushButton          *pb_csv_save;

      QPushButton          *pb_chains_from;
      QLineEdit            *le_chains_from;

      QPushButton          *pb_chains_to;
      QLineEdit            *le_chains_to;

      QPushButton          *pb_target;
      QLineEdit            *le_target;

      QProgressBar         *progress;

      QPushButton          *pb_start;
      QPushButton          *pb_stop;

      QFont                ft;
      QTextEdit            *editor;
      QMenuBar             *m;

      QPushButton          *pb_help;
      QPushButton          *pb_cancel;

      void                 editor_msg( QString color, QString msg );

      bool                 running;
      void                 update_enables();

      csv                  csv_commands;
      csv                  csv_from;
      csv                  csv_to;
      csv                  csv_target;

      void                 reset_csv_commands();

      QListView            *lv_csv_from;
      QListView            *lv_csv_to;
      void                 sel_to_range( QListView *lv, 
                                         vector < range_entry > &ranges );

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

      map < QString, unsigned int > csv_chain_map;

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

      void make_csv_chain_map();
      void update_t_csv_range( vector < range_entry > &ranges,
                               unsigned int col_start,
                               unsigned int col_end );

   private slots:

      void setupGUI();

      void sel_auto();
      void sel_from_to_merge();
      void sel_from_to_fit();
      void sel_to_to_fit();
      void sel_to_to_cut();

      void clear();
      void load();
      void validate();
      void csv_save();

      void chains_from();
      void chains_to();
      void target();

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
