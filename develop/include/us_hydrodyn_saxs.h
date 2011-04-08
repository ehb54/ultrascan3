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
#include <qradiobutton.h>
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
   float b;           // scattering factor b for p(r) calcs
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
                       vector < vector <PDB_atom> >   bead_models,
                       vector < unsigned int >        selected_models,
                       map < QString, vector <int> >  multi_residue_map,
                       map < QString, QString >       residue_atom_hybrid_map,
                       int                            source, 
                       bool                           create_native_saxs,
                       void                           *us_hydrodyn,
                       QWidget                        *p = 0, 
                       const char                     *name = 0
                       );
      ~US_Hydrodyn_Saxs();

      void refresh(
                   QString                        pdb_file, 
                   vector < residue >             residue_list,
                   vector < PDB_model >           model_vector,
                   vector < vector <PDB_atom> >   bead_models,
                   vector < unsigned int >        selected_models,
                   map < QString, vector <int> >  multi_residue_map,
                   map < QString, QString >       residue_atom_hybrid_map,
                   int                            source,
                   bool                           create_native_saxs
                   );
      
   public:
      bool *saxs_widget, initial;
      int source;
      US_Config *USglobal;

      QLabel *lbl_info;
      QLabel *lbl_filename1;
      QLabel *lbl_hybrid_table;
      QLabel *lbl_atom_table;
      QLabel *lbl_saxs_table;
      QLabel *lbl_core_progress;
      QLabel *lbl_info_prr;
      QLabel *lbl_bin_size;
      QLabel *lbl_guinier_cutoff;

      QTextEdit *te_filename2;

      QPrinter printer;

      QPushButton *pb_plot_saxs_sans;
      QPushButton *pb_load_saxs_sans;
      QPushButton *pb_clear_plot_saxs;
      QPushButton *pb_plot_pr;
      QPushButton *pb_load_pr;
      QPushButton *pb_clear_plot_pr;
      QPushButton *pb_load_gnom;
      QPushButton *pb_guinier_analysis;
      QPushButton *pb_select_atom_file;
      QPushButton *pb_select_hybrid_file;
      QPushButton *pb_select_saxs_file;
      QPushButton *pb_stop;
      QPushButton *pb_options;
      QPushButton *pb_help;
      QPushButton *pb_cancel;

      QwtCounter *cnt_bin_size;

      QButtonGroup *bg_saxs_sans;
      QRadioButton *rb_saxs;
      QRadioButton *rb_sans;

      QCheckBox *cb_normalize;

      QwtCounter *cnt_guinier_cutoff;

      QCheckBox *cb_guinier;
      QLineEdit *le_guinier_lowq2;
      QLineEdit *le_guinier_highq2;

      QCheckBox *cb_user_range;
      QLineEdit *le_user_lowq;
      QLineEdit *le_user_highq;

      QButtonGroup *bg_curve;
      QRadioButton *rb_curve_raw;
      QRadioButton *rb_curve_saxs_dry;
      QRadioButton *rb_curve_saxs;
      QRadioButton *rb_curve_sans;

      QCheckBox   *cb_pr_contrib;
      QLineEdit   *le_pr_contrib_low;
      QLineEdit   *le_pr_contrib_high;
      QPushButton *pb_pr_contrib;

      QCheckBox *cb_create_native_saxs;

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
      QStringList  qsl_plotted_iq_names;
      QStringList  qsl_plotted_pr_names;
#ifdef WIN32
     #pragma warning ( disable: 4251 )
#endif
      map    < QString, bool >                        dup_plotted_iq_name_check;
      map    < QString, bool >                        dup_plotted_pr_name_check;
      vector < atom >                                 atom_list;
      vector < hybridization >                        hybrid_list;
      vector < saxs >                                 saxs_list;
      vector < residue >                              residue_list;
      vector < PDB_model >                            model_vector;
      vector < vector <PDB_atom> >                    bead_models;
      vector < unsigned int >                         selected_models;
      vector < QColor >                               plot_colors;
      vector < long >                                 plotted_Iq;  // curve keys
      vector < vector < double > >                    plotted_q;
      vector < vector < double > >                    plotted_q2;  // q^2 for guinier plots
      vector < vector < double > >                    plotted_I;

      map    < unsigned int, long >                   plotted_Gp;  // guinier points
      map    < unsigned int, bool >                   plotted_guinier_valid;
      map    < unsigned int, bool >                   plotted_guinier_plotted;
      map    < unsigned int, double >                 plotted_guinier_lowq2;
      map    < unsigned int, double >                 plotted_guinier_highq2;
      map    < unsigned int, double >                 plotted_guinier_a;           // y = a + b*x
      map    < unsigned int, double >                 plotted_guinier_b;
      map    < unsigned int, vector < double > >      plotted_guinier_x;
      map    < unsigned int, vector < double > >      plotted_guinier_y;

      vector < vector < double > >                    plotted_pr;
      vector < vector < double > >                    plotted_pr_not_normalized;
      vector < vector < double > >                    plotted_r;

      map < QString, saxs >          saxs_map;
      map < QString, hybridization > hybrid_map;
      map < QString, atom >          atom_map;
      map < QString, vector <int> >  multi_residue_map;
      map < QString, QString >       residue_atom_hybrid_map;

      map < QString, vector < double > > nnls_A;
      map < QString,  double >           nnls_x;
      vector < double >                  nnls_B;
      vector < double >                  nnls_r;
      double                             nnls_rmsd;

#ifdef WIN32
     #pragma warning ( default: 4251 )
#endif      
      QString nnls_B_name;
      QString nnls_header_tag;

      void *us_hydrodyn;

      unsigned int current_model;

      QString model_filename;

      QString atom_filename;
      QString hybrid_filename;
      QString saxs_filename;
      
      bool stopFlag;
      bool create_native_saxs;

      QProgressBar *progress;

   private:

      QProcess *rasmol;

#ifdef WIN32
     #pragma warning ( disable: 4251 )
#endif
      map < QString, float > *remember_mw;
      map < QString, float > *match_remember_mw;
      // map < QString, float > contrib;
      vector < vector < float > > contrib_array;
      vector < PDB_atom * >  contrib_pdb_atom;
#ifdef WIN32
     #pragma warning ( default: 4251 )
#endif      

      float contrib_delta;
      QString contrib_file;

      double get_mw(QString filename);

      vector < double > interpolate( vector < double > to_r, 
                                     vector < double > from_r, 
                                     vector < double > from_pr );

      vector < double > rescale( vector < double > x );

      QString vector_double_to_csv( vector < double > vd );
      void calc_nnls_fit( QString save_to_csv_name = "" );
      void plot_one_pr(vector < double > r, vector < double > pr, QString name);
      void plot_one_iqq(vector < double > q, vector < double > I, QString name);
      bool plotted;
      bool save_to_csv;
      QString csv_filename;
      double guinier_cutoff;

      int file_curve_type(QString filename);
      QString curve_type_string(int curve);

      bool guinier_analysis( unsigned int i, QString &csvlog );
      void crop_iq_data( vector < double > &q,
                         vector < double > &I );


      // sets lowq & highq based upon current, plot settings (could be q^2 if in guinier)
      void plot_domain(double &lowq, double &highq);  

      // sets lowI & highI based upon range
      void plot_range(double lowq, double highq, double &lowI, double &highI);
      
      void rescale_plot();

      double pr_contrib_low;
      double pr_contrib_high;

   public slots:
      void show_plot_saxs_sans();
      void show_plot_pr();

   private slots:

      void clear_guinier();
      void setupGUI();
      void set_saxs_sans(int);
      void load_saxs_sans();
      void show_plot_saxs();
      void load_saxs(QString filename = "");
      void clear_plot_saxs();
      void show_plot_sans();
      void load_sans();
      void update_bin_size(double);
      void update_guinier_cutoff(double);
      void show_pr_contrib();
      void update_pr_contrib_low(const QString &);
      void update_pr_contrib_high(const QString &);
      void set_curve(int);
      void load_pr();
      void clear_plot_pr();
      void cancel();
      void help();
      void options();
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
      void normalize_pr(vector < double >, vector < double > *, double mw = 1e0);
      void update_saxs_sans();
      void run_guinier_analysis();
      QString saxs_filestring();
      QString sprr_filestring();
      void set_create_native_saxs();
      void set_guinier();
      void set_pr_contrib();
      void set_user_range();
      void update_guinier_lowq2(const QString &);
      void update_guinier_highq2(const QString &);
      void update_user_lowq(const QString &);
      void update_user_highq(const QString &);
      void load_gnom();

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
                         vector < vector < double > > *fc,
                         vector < vector < double > > *fp,
                         vector < double > *I,
                         vector < double > *Ia,
                         vector < double > *Ic,
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
  vector < vector < double > > *fc;
  vector < vector < double > > *fp;
  vector < double > *I;
  vector < double > *Ia;
  vector < double > *Ic;
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

class saxs_pr_thr_t : public QThread
{
 public:
  saxs_pr_thr_t(int);
  void saxs_pr_thr_setup(
                         vector < saxs_atom > *atoms,
                         vector < float > *hist,
                         double delta,
                         unsigned int threads,
                         QProgressBar *progress,
                         QLabel *lbl_core_progress,
                         bool *stopFlag,
                         float b_bar_inv2
                         );
  void saxs_pr_thr_shutdown();
  void saxs_pr_thr_wait();
  int saxs_pr_thr_work_status();
  virtual void run();

 private:

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

  vector < saxs_atom > *atoms;
  vector < float > *hist;

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif
  double delta;
  unsigned int threads;
  QProgressBar *progress;
  QLabel *lbl_core_progress;
  bool *stopFlag;
  float b_bar_inv2;

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
