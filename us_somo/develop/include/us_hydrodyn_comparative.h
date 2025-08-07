#ifndef US_HYDRODYN_COMPARATIVE_H
#define US_HYDRODYN_COMPARATIVE_H

// QT defs:

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qstring.h>
//#include <q3frame.h>
#include <qcheckbox.h>
#include <qfileinfo.h>
#include <qgroupbox.h>
#include <qlistwidget.h>
#include <qmenubar.h>
#include <qprinter.h>
#include <qprogressbar.h>
#include <qtextedit.h>
#include <qwt_counter.h>
//Added by qt3to4:
#include <QCloseEvent>

#include "us_util.h"

//standard C and C++ defs:

#include <iostream>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "../include/us_hydrodyn_hydro.h"
#include "../include/us_hydrodyn_results.h"

using namespace std;

struct csv {
      QString name;

      map<QString, int> header_map; // maps column names to positions

      vector<QString> header; // the first csv row
      vector<vector<QString>> data; // the entire csv past the first row
      vector<vector<double>> num_data; // toDoubles of the csv file

      vector<QString> prepended_names; // list of "model names" with QFileInfo(name).completeBaseName():

      // for pdb_tool:

      QString filename;
      QStringList header_text;
      QStringList title_text;

      map<QString, unsigned int> key; // used to map keys to data rows
      vector<bool> visible;
      vector<bool> selected;
      vector<bool> open;

      map<QString, unsigned int> nd_key; // used to map parents of data items to below
      vector<bool> nd_visible;
      vector<bool> nd_selected;
      vector<bool> nd_open;

      QString current_item_key;
};

#include "../include/us_hydrodyn_csv_viewer.h"

struct comparative_entry {
      QString name;
      bool active;
      double target; // we could later setup an enum for varying target types

      // for simple csv sort priority
      int rank;

      // weighted ranking
      bool include_in_weight;
      double weight;

      // for 'bin'ning
      int buckets;
      double min;
      double max;

      // csv output
      bool store_abs_diff;
      bool store_diff;
      bool store_reference;
};

/* current targets:
 * Sedimentation coefficient s [S]
 * Translational diffusion coefficient D [cm/sec**2]
 * Stokes radius [nm]
 * Frictional ratio
 * Radius of gyration [nm] (from bead model)
 * Relaxation Time, tau(h) [ns]
 * Intrinsic viscosity [cm**3/g] 
 */

struct comparative_info {
      bool by_pct;
      bool rank; // set for simple sorting by difference, then rank
      bool weight_controls; // set for sorting by weighted computation
      bool by_ec;

      comparative_entry ce_s;
      comparative_entry ce_D;
      comparative_entry ce_sr;
      comparative_entry ce_fr;
      comparative_entry ce_rg;
      comparative_entry ce_tau;
      comparative_entry ce_eta;

      QString path_param;
      QString path_csv;
};

class US_EXTERN US_Hydrodyn_Comparative : public QFrame {
      Q_OBJECT

   public:
      US_Hydrodyn_Comparative(
         comparative_info *comparative, void *us_hydrodyn,
         bool *comparative_widget = 0, // no comparative widget implies non-gui
         QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_Comparative();

      static comparative_entry empty_comparative_entry(QString name);
      static comparative_info empty_comparative_info();
      // probably should make comparative_entry,_info classes and define ==
      static bool comparative_entry_equals(comparative_entry ce1, comparative_entry ce2);
      static bool comparative_info_equals(comparative_info ci1, comparative_info ci2);

   private:
      comparative_info *comparative;
      bool *comparative_widget;
      void *us_hydrodyn;

      US_Config *USglobal;

      QLabel *lbl_title_param;
      QLabel *lbl_title_csv;

      QLabel *lbl_active;
      QLabel *lbl_target;

      QLabel *lbl_sort;

      QCheckBox *cb_by_pct;

      QCheckBox *cb_rank;
      QLabel *lbl_rank;

      QCheckBox *cb_weight_controls;
      QLabel *lbl_include_in_weight;
      QLabel *lbl_weight;

      QLabel *lbl_ec;
      QCheckBox *cb_by_ec;
      QLabel *lbl_buckets;
      QLabel *lbl_min;
      QLabel *lbl_max;

      QLabel *lbl_csv_controls;
      QLabel *lbl_store_reference;
      QLabel *lbl_store_diff;
      QLabel *lbl_store_abs_diff;

      QCheckBox *cb_active_s;
      QLineEdit *le_target_s;
      QLineEdit *le_rank_s;
      QCheckBox *cb_include_in_weight_s;
      QLineEdit *le_weight_s;
      QLineEdit *le_buckets_s;
      QLineEdit *le_min_s;
      QLineEdit *le_max_s;
      QCheckBox *cb_store_reference_s;
      QCheckBox *cb_store_diff_s;
      QCheckBox *cb_store_abs_diff_s;

      QCheckBox *cb_active_D;
      QLineEdit *le_target_D;
      QLineEdit *le_rank_D;
      QCheckBox *cb_include_in_weight_D;
      QLineEdit *le_weight_D;
      QLineEdit *le_buckets_D;
      QLineEdit *le_min_D;
      QLineEdit *le_max_D;
      QCheckBox *cb_store_reference_D;
      QCheckBox *cb_store_diff_D;
      QCheckBox *cb_store_abs_diff_D;

      QCheckBox *cb_active_sr;
      QLineEdit *le_target_sr;
      QLineEdit *le_rank_sr;
      QCheckBox *cb_include_in_weight_sr;
      QLineEdit *le_weight_sr;
      QLineEdit *le_buckets_sr;
      QLineEdit *le_min_sr;
      QLineEdit *le_max_sr;
      QCheckBox *cb_store_reference_sr;
      QCheckBox *cb_store_diff_sr;
      QCheckBox *cb_store_abs_diff_sr;

      QCheckBox *cb_active_fr;
      QLineEdit *le_target_fr;
      QLineEdit *le_rank_fr;
      QCheckBox *cb_include_in_weight_fr;
      QLineEdit *le_weight_fr;
      QLineEdit *le_buckets_fr;
      QLineEdit *le_min_fr;
      QLineEdit *le_max_fr;
      QCheckBox *cb_store_reference_fr;
      QCheckBox *cb_store_diff_fr;
      QCheckBox *cb_store_abs_diff_fr;

      QCheckBox *cb_active_rg;
      QLineEdit *le_target_rg;
      QLineEdit *le_rank_rg;
      QCheckBox *cb_include_in_weight_rg;
      QLineEdit *le_weight_rg;
      QLineEdit *le_buckets_rg;
      QLineEdit *le_min_rg;
      QLineEdit *le_max_rg;
      QCheckBox *cb_store_reference_rg;
      QCheckBox *cb_store_diff_rg;
      QCheckBox *cb_store_abs_diff_rg;

      QCheckBox *cb_active_tau;
      QLineEdit *le_target_tau;
      QLineEdit *le_rank_tau;
      QCheckBox *cb_include_in_weight_tau;
      QLineEdit *le_weight_tau;
      QLineEdit *le_buckets_tau;
      QLineEdit *le_min_tau;
      QLineEdit *le_max_tau;
      QCheckBox *cb_store_reference_tau;
      QCheckBox *cb_store_diff_tau;
      QCheckBox *cb_store_abs_diff_tau;

      QCheckBox *cb_active_eta;
      QLineEdit *le_target_eta;
      QLineEdit *le_rank_eta;
      QCheckBox *cb_include_in_weight_eta;
      QLineEdit *le_weight_eta;
      QLineEdit *le_buckets_eta;
      QLineEdit *le_min_eta;
      QLineEdit *le_max_eta;
      QCheckBox *cb_store_reference_eta;
      QCheckBox *cb_store_diff_eta;
      QCheckBox *cb_store_abs_diff_eta;

      QPushButton *pb_load_param;
      QPushButton *pb_reset_param;
      QPushButton *pb_save_param;

      QPushButton *pb_load_csv;
      QPushButton *pb_process_csv;
      QPushButton *pb_save_csv;

      QLabel *lbl_loaded;
      QListWidget *lb_loaded;
      QPushButton *pb_loaded_select_all;
      QPushButton *pb_loaded_view;
      QPushButton *pb_loaded_merge;
      QPushButton *pb_loaded_set_ranges;
      QPushButton *pb_loaded_remove;

      QLabel *lbl_selected;
      QListWidget *lb_selected;
      QPushButton *pb_selected_select_all;
      QPushButton *pb_selected_merge;
      QPushButton *pb_selected_set_ranges;
      QPushButton *pb_selected_remove;

      QFont ft;
      QTextEdit *editor;
      QMenuBar *m;
      QPrinter printer;

      QPushButton *pb_help;
      QPushButton *pb_cancel;

      void update_enables();
      void refresh();
      void update_lb_loaded_enables();
      void update_lb_selected_enables();
      void disable_updates();
      void enable_updates();
      bool updates_enabled;

      QString serial_error;
      QStringList serialize_params();
      QString serialize_comparative_entry(comparative_entry ce);
      comparative_entry deserialize_comparative_entry(QString qs);
      QString serialize_comparative_info(comparative_info ci);
      comparative_info deserialize_comparative_info(QString qs);

      // csv handling routines:


#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(disable : 4251)
#endif
#endif
      map<QString, csv> csvs; // all csvs loaded?
      csv csv_read(QString filename); // sets csv_error, csv_warn
      void csv_write(QString filename, csv &csv1); // use to save any csv in the loaded
      bool csv_process(csv &csv1); // processes csv
      QStringList csv_parse_line(QString qs); // split up line
      QString csv_error;
      QString csv_warn;
      csv csv_merge(csv &csv1, csv &csv2); // sets csv_error
      void csv_merge_loaded_selected(); // takes all loaded selected pdbs, merges them and puts it on the loaded list and selects it

      bool csv_merge_selected_selected(csv &csv_merged); // does not add to csvs, returns false if not created
      //                            ^^^^^^ note: this function disables_updates() and you must enable_updates() afterwards. (there are potential gui issues without this behaviour)

      bool csv_has_column_name(csv &csv1, QString name);
      void csv_remove_column(csv &csv1, QString name);

      void csv_remove(QString name, int i);

      void csv_sort(csv &csv1, vector<unsigned int> cols);

      QString csv_info(csv &csv1); // returns readable summary info (primarily for debugging)
      QStringList csv_model_names(csv &csv1); // returns a list of the names (primarily for updating lb_selected)
      bool csv_contains(comparative_entry ce, csv &csv1); // checks the csv for the column names by the comparative entry
      bool all_selected_csv_contain(
         comparative_entry ce); // checks all selected csvs for the column names by the comparative entry

      bool csv_get_min_max(
         double &min, double &max, unsigned int &rows_used_count, comparative_entry ce, csv &csv1,
         bool ignore_selected_selected = true);

      bool csv_get_loaded_min_max(double &min, double &max, comparative_entry ce);

      bool csv_get_selected_min_max(double &min, double &max, comparative_entry ce);

      void csv_make_unique_header_name(csv &csv1, QString name);
      //                            finds a unique name that does equal the originally supplied name
      //                            and then rename the header to that unique name

      void update_selected_map();
      map<QString, bool> selected_map; // needed for quick lookups, maps lb_selected_selected.text()

      // other utilities
      void editor_msg(QString color, QString msg);
      bool any_loaded_selected();
      bool one_loaded_selected();
      QString first_loaded_selected();
      bool any_selected_selected();
      bool one_selected_selected();
      bool any_params_enabled();
      bool ec_ready();

      bool csv_premerge_column_warning(csv &csv1, csv &csv2);
      bool csv_premerge_column_warning_all_loaded_selected();
      map<QString, bool> csv_premerge_missing_header_map; // keeps track of columns that will be added
      QStringList csv_premerge_missing_header_qsl; // string to display columns

      // utilities for keeping names unique

      map<QString, bool> loaded_csv_names;
      map<QString, bool> loaded_csv_row_prepended_names;
      void set_loaded_csv_row_prepended_names(csv &csv1);
      QString get_unique_csv_name(QString name);

      QString loaded_info();

      // cleaner logic structures

      void build_ce_names_map();
      vector<QString> ce_names;
      map<QString, comparative_entry *> ce_map;

#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(default : 4251)
#endif
#endif

   private slots:

      void setupGUI();

      void set_by_pct();
      void set_rank();
      void set_weight_controls();
      void set_by_ec();

      void set_active_s();
      void set_active_D();
      void set_active_sr();
      void set_active_fr();
      void set_active_rg();
      void set_active_tau();
      void set_active_eta();

      void update_target_s(const QString &);
      void update_rank_s(const QString &);
      void set_include_in_weight_s();
      void update_weight_s(const QString &);
      void update_buckets_s(const QString &);
      void update_min_s(const QString &);
      void update_max_s(const QString &);
      void set_store_reference_s();
      void set_store_diff_s();
      void set_store_abs_diff_s();

      void update_target_D(const QString &);
      void update_rank_D(const QString &);
      void set_include_in_weight_D();
      void update_weight_D(const QString &);
      void update_buckets_D(const QString &);
      void update_min_D(const QString &);
      void update_max_D(const QString &);
      void set_store_reference_D();
      void set_store_diff_D();
      void set_store_abs_diff_D();

      void update_target_sr(const QString &);
      void update_rank_sr(const QString &);
      void set_include_in_weight_sr();
      void update_weight_sr(const QString &);
      void update_buckets_sr(const QString &);
      void update_min_sr(const QString &);
      void update_max_sr(const QString &);
      void set_store_reference_sr();
      void set_store_diff_sr();
      void set_store_abs_diff_sr();

      void update_target_fr(const QString &);
      void update_rank_fr(const QString &);
      void set_include_in_weight_fr();
      void update_weight_fr(const QString &);
      void update_buckets_fr(const QString &);
      void update_min_fr(const QString &);
      void update_max_fr(const QString &);
      void set_store_reference_fr();
      void set_store_diff_fr();
      void set_store_abs_diff_fr();

      void update_target_rg(const QString &);
      void update_rank_rg(const QString &);
      void set_include_in_weight_rg();
      void update_weight_rg(const QString &);
      void update_buckets_rg(const QString &);
      void update_min_rg(const QString &);
      void update_max_rg(const QString &);
      void set_store_reference_rg();
      void set_store_diff_rg();
      void set_store_abs_diff_rg();

      void update_target_tau(const QString &);
      void update_rank_tau(const QString &);
      void set_include_in_weight_tau();
      void update_weight_tau(const QString &);
      void update_buckets_tau(const QString &);
      void update_min_tau(const QString &);
      void update_max_tau(const QString &);
      void set_store_reference_tau();
      void set_store_diff_tau();
      void set_store_abs_diff_tau();

      void update_target_eta(const QString &);
      void update_rank_eta(const QString &);
      void set_include_in_weight_eta();
      void update_weight_eta(const QString &);
      void update_buckets_eta(const QString &);
      void update_min_eta(const QString &);
      void update_max_eta(const QString &);
      void set_store_reference_eta();
      void set_store_diff_eta();
      void set_store_abs_diff_eta();

      void load_param();
      void reset_param();
      void save_param();

      void load_csv();
      void process_csv();
      void save_csv();

      void update_loaded();
      void loaded_select_all();
      void loaded_view();
      void loaded_merge();
      void loaded_set_ranges();
      void loaded_remove();

      void update_selected();
      void selected_select_all();
      void selected_merge();
      void selected_set_ranges();
      void selected_remove();

      void clear_display();
      void print();
      void update_font();
      void save();

      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
};
#endif
