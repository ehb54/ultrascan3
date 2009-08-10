#ifndef US_HYDRODYN_SAXS_H
#define US_HYDRODYN_SAXS_H

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

#include <qwt_plot.h>

#include "us_util.h"
#include "us_hydrodyn_pdbdefs.h"

//standard C and C++ defs:

#include <map>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>


using namespace std;

struct saxs_atom
{
   QString saxs_name;
   float pos[3];
   float excl_vol;
};

class US_EXTERN US_Hydrodyn_Saxs : public QFrame
{
   Q_OBJECT

   public:
      US_Hydrodyn_Saxs(
                       bool                           *saxs_widget, 
                       saxs_options                   *our_saxs_options,
                       QString                        pdb_file, 
                       vector < residue >             residue_list,
                       vector < PDB_model >           model_vector,
                       vector < unsigned int >        selected_models,
                       map < QString, vector <int> >  multi_residue_map,
                       map < QString, QString >       residue_atom_hybrid_map,
                       int                            source, 
                       QWidget                        *p = 0, 
                       const char                     *name = 0
                       );
      ~US_Hydrodyn_Saxs();

   public:
      bool *saxs_widget, initial;
      int source;
      US_Config *USglobal;

      QLabel *lbl_info;
      QLabel *lbl_filename1;
      QLabel *lbl_filename2;
      QLabel *lbl_hybrid_table;
      QLabel *lbl_atom_table;
      QLabel *lbl_saxs_table;
      QLabel *lbl_core_progress;

      QPrinter printer;

      QPushButton *pb_plot_pr;
      QPushButton *pb_plot_saxs;
      QPushButton *pb_select_atom_file;
      QPushButton *pb_select_hybrid_file;
      QPushButton *pb_select_saxs_file;
      QPushButton *pb_stop;
      QPushButton *pb_help;
      QPushButton *pb_cancel;

      QFont ft;

      QTextEdit *editor;

      QMenuBar *m;

      QwtPlot *plot_pr;
      QwtPlot *plot_saxs;

      QProgressBar *progress_pr;
      QProgressBar *progress_saxs;

      struct atom current_atom;
      struct hybridization current_hybrid;
      struct saxs current_saxs;
      saxs_options *our_saxs_options;

#ifdef WIN32
     #pragma warning ( disable: 4251 )
#endif
      vector < atom >          atom_list;
      vector < hybridization > hybrid_list;
      vector < saxs >          saxs_list;
      vector < residue >       residue_list;
      vector < PDB_model >     model_vector;
      vector < unsigned int >  selected_models;
#ifdef WIN32
     #pragma warning ( default: 4251 )
#endif      

      map < QString, saxs >          saxs_map;
      map < QString, hybridization > hybrid_map;
      map < QString, atom >          atom_map;
      map < QString, vector <int> >  multi_residue_map;
      map < QString, QString >       residue_atom_hybrid_map;

      unsigned int current_model;

      QString atom_filename;
      QString hybrid_filename;
      QString saxs_filename;
      
      bool stopFlag;

      QProgressBar *progress;

   private slots:

      void setupGUI();
      void show_plot_pr();
      void show_plot_saxs();
      void cancel();
      void help();
      void stop();
      void clear_display();
      void print();
      void update_font();
      void save();
      void select_atom_file();
      void select_hybrid_file();
      void select_saxs_file();
      void select_atom_file(const QString &);
      void select_hybrid_file(const QString &);
      void select_saxs_file(const QString &);

   protected slots:

      void closeEvent(QCloseEvent *);
};



#include <qthread.h>
#include <qwaitcondition.h>
#include <qmutex.h>

class saxs_Iq_thr_t : public QThread
{
 public:
  saxs_Iq_thr_t(int);
  void saxs_Iq_thr_setup(
                         vector < saxs_atom > *atoms,
                         vector < vector < double > > *f,
                         vector < double > *I,
                         vector < double > *q,
                         unsigned int threads,
                         QProgressBar *progress,
                         QLabel *lbl_core_progress,
                         bool *stopFlag
                         );
  void saxs_Iq_thr_shutdown();
  void saxs_Iq_thr_wait();
  int saxs_Iq_thr_work_status();
  virtual void run();

 private:

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

  vector < saxs_atom > *atoms;
  vector < vector < double > > *f;
  vector < double > *I;
  vector < double > *q;

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif
  unsigned int threads;
  QProgressBar *progress;
  QLabel *lbl_core_progress;
  bool *stopFlag;

  int thread;
  QMutex work_mutex;
  int work_to_do;
  QWaitCondition cond_work_to_do;
  int work_done;
  QWaitCondition cond_work_done;
  int work_to_do_waiters;
  int work_done_waiters;
};


#endif

