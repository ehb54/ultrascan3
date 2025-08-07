#ifndef US_HYDRODYN_SAXS_PDB_TOOL_MERGE_H
#define US_HYDRODYN_SAXS_PDB_TOOL_MERGE_H

// QT defs:

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qstring.h>
//#include <q3frame.h>
#include <qcheckbox.h>
#include <qfileinfo.h>
#include <qmenubar.h>
#include <qprinter.h>
#include <qprogressbar.h>
#include <qtablewidget.h>
#include <qtextedit.h>
#include <qtreewidget.h>
//Added by qt3to4:
#include <QCloseEvent>

#include "us_hydrodyn_comparative.h"
#include "us_saxs_util.h"
#include "us_util.h"

//standard C and C++ defs:

#include <iostream>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

using namespace std;

class range_entry {
   public:
      QString chain;
      unsigned int start;
      unsigned int end;
      map<unsigned int, bool> gaps;
      map<unsigned int, bool> residues;
      friend ostream &operator<<(ostream &, const range_entry &);
};

struct pdb_sel_count {
      unsigned int models;
      unsigned int chains;
      unsigned int residues;
      unsigned int atoms;
      bool model_partial;
      bool chain_partial;
      bool residue_partial;
      unsigned int not_selected_atoms;
      unsigned int SOLs;
};

class US_EXTERN US_Hydrodyn_Pdb_Tool_Merge : public QFrame {
      Q_OBJECT
      friend class US_Hydrodyn_Pdb_Tool;

   public:
      US_Hydrodyn_Pdb_Tool_Merge(void *us_hydrodyn, void *pdb_tool_window, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_Pdb_Tool_Merge();

   private:
      void *us_hydrodyn;
      void *pdb_tool_window;

      US_Saxs_Util *usu;

      US_Config *USglobal;

      QLabel *lbl_title;

      QTableWidget *t_csv;
      // csv layout for cut/splice
      //
      // chain|cut start|cut end|fit start|fit end|merge start|merge end

      // ? duplicate lines
      // ? reorder lines
      // set selected from cut, fit, merge
      // set selected to cut, fit, merge

      QPushButton *pb_sel_auto;
      QPushButton *pb_sel_from_to_merge;
      QPushButton *pb_sel_from_to_fit;
      QPushButton *pb_sel_to_to_fit;
      QPushButton *pb_sel_to_to_cut;

      QPushButton *pb_extra_chains;
      QPushButton *pb_only_closest;
      QPushButton *pb_delete_row;

      QPushButton *pb_clear;
      QPushButton *pb_load;
      QPushButton *pb_validate;
      QPushButton *pb_csv_save;

      QPushButton *pb_chains_from;
      QLineEdit *le_chains_from;

      QPushButton *pb_chains_to;
      QLineEdit *le_chains_to;

      QPushButton *pb_target;
      QLineEdit *le_target;

      QCheckBox *cb_filter;

      QProgressBar *progress;

      QPushButton *pb_start;
      QPushButton *pb_trial;
      QPushButton *pb_stop;

      QFont ft;
      QTextEdit *editor;
      QMenuBar *m;

      QPushButton *pb_help;
      QPushButton *pb_pdb_tool;
      QPushButton *pb_cancel;

      void editor_msg(QString color, QString msg);

      bool running;
      bool filtered;

      csv csv_commands;
      csv csv_from;
      csv csv_to;
      csv csv_target;

      void reset_csv_commands();

      QTreeWidget *lv_csv_from;
      QTreeWidget *lv_csv_to;
      void sel_to_range(QTreeWidget *lv, vector<range_entry> &ranges, bool just_selected = true);

#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(disable : 4251)
#endif
#endif

      map<QString, unsigned int> csv_chain_map;
      vector<QString> extra_chains_list;

      vector<range_entry> cache_from_ranges;
      vector<range_entry> cache_to_ranges;
      map<QString, unsigned int> cache_from_range_pos;
      map<QString, unsigned int> cache_to_range_pos;
      map<QString, bool> cache_use_start;

#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(default : 4251)
#endif
#endif

      bool cache_range_ok;

      void make_csv_chain_map();
      void update_t_csv_range(vector<range_entry> &ranges, unsigned int col_start, unsigned int col_end);

      void update_csv_commands_from_table();
      void update_t_csv_data();
      QStringList csv_parse_line(QString qs);

      bool validate_commands();
      void run_one();
      bool extra_chains_done;

      void update_cache_range();
      void recalc_from_merge(int row, int col);
      void recalc_from_fit(int row, int col);
      void recalc_from_cut(int row, int col);

      unsigned int residue_length(bool use_from, QString chain, unsigned int start_residue, unsigned int end_residue);

      unsigned int residue_offset_position(bool use_from, QString chain, unsigned int start_residue, int offset);

      bool get_chains(QString chain, QString &fit_chain, QString &cross_chain);
      bool get_chains(QString chain);

      void fill_csv_empty_items();
      void info_csv(const QString &msg);

   private slots:

      void setupGUI();

      void table_value(int, int);

      void update_enables();

      void sel_auto();
      void sel_from_to_merge();
      void sel_from_to_fit();
      void sel_to_to_fit();
      void sel_to_to_cut();

      void extra_chains();
      void only_closest();
      void delete_row();

      void clear();
      void load();
      void validate();
      void csv_save();

      void chains_from();
      void chains_to();
      void target();

      void start();
      void trial();
      void stop();

      void clear_display();
      void update_font();
      void save();

      void cancel();
      void pdb_tool();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
};

#endif
