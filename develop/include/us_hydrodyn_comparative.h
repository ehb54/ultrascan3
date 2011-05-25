#ifndef US_HYDRODYN_COMPARATIVE_H
#define US_HYDRODYN_COMPARATIVE_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qwt_counter.h>
#include <qbuttongroup.h>
#include <qtextedit.h>
#include <qprogressbar.h>
#include <qmenubar.h>
#include <qfileinfo.h>
#include <qprinter.h>
#include <qlistbox.h>
#include <qtabwidget.h>

#include "us_util.h"

//standard C and C++ defs:

#include <map>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "../include/us_hydrodyn_results.h"
#include "../include/us_hydrodyn_hydro.h"

using namespace std;

struct comparative_entry
{
   QString name;
   bool    active;
   double  target;  // we could later setup an enum for varying target types
   
   // for simple csv sort priority
   int     rank;

   // weighted ranking   
   bool   include_in_weight;
   double weight;

   // for 'bin'ning
   int     buckets; 
   double  min;
   double  max;

   // csv output
   bool   store_abs_diff;
   bool   store_diff;
   bool   store_reference;

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

struct comparative_info
{
   bool rank; // set for simple sorting by difference, then rank
   bool weight_controls; // set for sorting by weighted computation

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

class US_EXTERN US_Hydrodyn_Comparative : public QFrame
{
   Q_OBJECT

   public:
      US_Hydrodyn_Comparative(
                              comparative_info *comparative,      
                              void *us_hydrodyn, 
                              bool *comparative_widget = 0,  // no comparative widget implies non-gui
                              QWidget *p = 0, 
                              const char *name = 0
                              );
      ~US_Hydrodyn_Comparative();

      static comparative_entry      empty_comparative_entry( QString name );
      static comparative_info       empty_comparative_info();
      // probably should make comparative_entry,_info classes and define == 
      static bool                   comparative_entry_equals(comparative_entry ce1, 
                                                             comparative_entry ce2);
      static bool                   comparative_info_equals(comparative_info ci1, 
                                                            comparative_info ci2);

   private:

      comparative_info              *comparative;
      bool                          *comparative_widget;
      void                          *us_hydrodyn;

      US_Config                     *USglobal;

      QLabel                        *lbl_title_param;
      QLabel                        *lbl_title_csv;

      QLabel                        *lbl_active;
      QLabel                        *lbl_target;

      QLabel                        *lbl_sort;

      QCheckBox                     *cb_rank;

      QCheckBox                     *cb_weight_controls;
      QLabel                        *lbl_include_in_weight;
      QLabel                        *lbl_weight;

      QLabel                        *lbl_ec;
      QLabel                        *lbl_buckets;
      QLabel                        *lbl_min;
      QLabel                        *lbl_max;

      QLabel                        *lbl_csv_controls;
      QLabel                        *lbl_store_reference;
      QLabel                        *lbl_store_diff;
      QLabel                        *lbl_store_abs_diff;

      QCheckBox                     *cb_active_s;
      QLineEdit                     *le_target_s;
      QLineEdit                     *le_rank_s;
      QCheckBox                     *cb_include_in_weight_s;
      QLineEdit                     *le_weight_s;
      QLineEdit                     *le_buckets_s;
      QLineEdit                     *le_min_s;
      QLineEdit                     *le_max_s;
      QCheckBox                     *cb_store_reference_s;
      QCheckBox                     *cb_store_diff_s;
      QCheckBox                     *cb_store_abs_diff_s;

      QCheckBox                     *cb_active_D;
      QLineEdit                     *le_target_D;
      QLineEdit                     *le_rank_D;
      QCheckBox                     *cb_include_in_weight_D;
      QLineEdit                     *le_weight_D;
      QLineEdit                     *le_buckets_D;
      QLineEdit                     *le_min_D;
      QLineEdit                     *le_max_D;
      QCheckBox                     *cb_store_reference_D;
      QCheckBox                     *cb_store_diff_D;
      QCheckBox                     *cb_store_abs_diff_D;

      QCheckBox                     *cb_active_sr;
      QLineEdit                     *le_target_sr;
      QLineEdit                     *le_rank_sr;
      QCheckBox                     *cb_include_in_weight_sr;
      QLineEdit                     *le_weight_sr;
      QLineEdit                     *le_buckets_sr;
      QLineEdit                     *le_min_sr;
      QLineEdit                     *le_max_sr;
      QCheckBox                     *cb_store_reference_sr;
      QCheckBox                     *cb_store_diff_sr;
      QCheckBox                     *cb_store_abs_diff_sr;

      QCheckBox                     *cb_active_fr;
      QLineEdit                     *le_target_fr;
      QLineEdit                     *le_rank_fr;
      QCheckBox                     *cb_include_in_weight_fr;
      QLineEdit                     *le_weight_fr;
      QLineEdit                     *le_buckets_fr;
      QLineEdit                     *le_min_fr;
      QLineEdit                     *le_max_fr;
      QCheckBox                     *cb_store_reference_fr;
      QCheckBox                     *cb_store_diff_fr;
      QCheckBox                     *cb_store_abs_diff_fr;

      QCheckBox                     *cb_active_rg;
      QLineEdit                     *le_target_rg;
      QLineEdit                     *le_rank_rg;
      QCheckBox                     *cb_include_in_weight_rg;
      QLineEdit                     *le_weight_rg;
      QLineEdit                     *le_buckets_rg;
      QLineEdit                     *le_min_rg;
      QLineEdit                     *le_max_rg;
      QCheckBox                     *cb_store_reference_rg;
      QCheckBox                     *cb_store_diff_rg;
      QCheckBox                     *cb_store_abs_diff_rg;

      QCheckBox                     *cb_active_tau;
      QLineEdit                     *le_target_tau;
      QLineEdit                     *le_rank_tau;
      QCheckBox                     *cb_include_in_weight_tau;
      QLineEdit                     *le_weight_tau;
      QLineEdit                     *le_buckets_tau;
      QLineEdit                     *le_min_tau;
      QLineEdit                     *le_max_tau;
      QCheckBox                     *cb_store_reference_tau;
      QCheckBox                     *cb_store_diff_tau;
      QCheckBox                     *cb_store_abs_diff_tau;

      QCheckBox                     *cb_active_eta;
      QLineEdit                     *le_target_eta;
      QLineEdit                     *le_rank_eta;
      QCheckBox                     *cb_include_in_weight_eta;
      QLineEdit                     *le_weight_eta;
      QLineEdit                     *le_buckets_eta;
      QLineEdit                     *le_min_eta;
      QLineEdit                     *le_max_eta;
      QCheckBox                     *cb_store_reference_eta;
      QCheckBox                     *cb_store_diff_eta;
      QCheckBox                     *cb_store_abs_diff_eta;

      QPushButton                   *pb_load_param;
      QPushButton                   *pb_reset_param;
      QPushButton                   *pb_save_param;

      QPushButton                   *pb_load_csv;
      QPushButton                   *pb_process_csv;
      QPushButton                   *pb_save_csv;

      QFont                         ft;
      QTextEdit                     *editor;
      QMenuBar                      *m;
      QPrinter                      printer;

      QLabel                        *lbl_heat_map;

      QPushButton                   *pb_help;
      QPushButton                   *pb_cancel;

      void                          update_enables();
      void                          refresh();

      QString                       serial_error;
      QStringList                   serialize_params();
      QString                       serialize_comparative_entry( comparative_entry ce );
      comparative_entry             deserialize_comparative_entry( QString qs );
      QString                       serialize_comparative_info( comparative_info ci );
      comparative_info              deserialize_comparative_info( QString qs );

   private slots:
      
      void setupGUI();

      void set_rank();
      void set_weight_controls();

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
