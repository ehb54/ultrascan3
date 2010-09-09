#ifndef US_HYDRODYN_H
#define US_HYDRODYN_H

// QT defs:

#include <qfile.h>
#include <qfiledialog.h>
#include <qtextstream.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qframe.h>
#include <qprogressbar.h>
#include <qprocess.h>
#include <qdir.h>
#include <qtextedit.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qprinter.h>

#include "us.h"
#include "us_extern.h"
#include "us_util.h"
#include "us_hydrodyn_pdbdefs.h"
#include "us_hydrodyn_addatom.h"
#include "us_hydrodyn_addresidue.h"
#include "us_hydrodyn_addsaxs.h"
#include "us_hydrodyn_anaflex_options.h"
#include "us_hydrodyn_overlap.h"
#include "us_hydrodyn_bead_output.h"
#include "us_hydrodyn_asa.h"
#include "us_hydrodyn_bd_options.h"
#include "us_hydrodyn_dmd_options.h"
#include "us_hydrodyn_hydro.h"
#include "us_hydrodyn_misc.h"
#include "us_hydrodyn_grid.h"
#include "us_hydrodyn_results.h"
#include "us_hydrodyn_pdb_visualization.h"
#include "us_hydrodyn_pdb_parsing.h"
#include "us_hydrodyn_saxs_options.h"
#include "us_hydrodyn_saxs.h"
#include "us_hydrodyn_advanced_config.h"
#include "us_hydrodyn_batch.h"
#include "us_hydrodyn_save.h"
#include "us_hydrodyn_file.h"
#include "us_hydrodyn_dammin_opts.h"
#include "us_hydrodyn_bd_load_results_opts.h"
#include "us_hydrodyn_bd.h"
#include "us_hydrodyn_batch_movie_opts.h"

//standard C and C++ defs:

#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <float.h>
#include <iostream>
#include <list>
#include <map>

#define START_RASMOL
using namespace std;

class US_EXTERN US_Hydrodyn : public QFrame
{
   Q_OBJECT

   public:
      US_Hydrodyn(vector < QString >,
                  QWidget *p = 0, 
                  const char *name = 0);
      ~US_Hydrodyn();
      int get_color(PDB_atom *);
      BD_Options bd_options;
      BD_Options default_bd_options;
      Anaflex_Options anaflex_options;
      Anaflex_Options default_anaflex_options;
      DMD_Options dmd_options;
      DMD_Options default_dmd_options;
      struct misc_options misc;
      struct misc_options default_misc;
      struct advanced_config advanced_config;
      double overlap_tolerance;
      double default_overlap_tolerance;
      bool stopFlag;
      bool errorFlag;
      bool calcAutoHydro;
      bool setSuffix;
      bool overwrite;
      bool saveParams;
      bool guiFlag;
      QLabel *lbl_core_progress;
      void set_disabled();
      QTextEdit *editor;
      void play_sounds(int);
      struct pdb_parsing pdb_parse;
      bool bead_model_from_file;
      batch_info batch;
      save_info save_params;
      QString somo_dir;
      QString somo_pdb_dir;
      bool screen_pdb(QString, bool display_pdb = false);
      bool screen_bead_model(QString);
      QPushButton *pb_stop_calc;
      struct hydro_options hydro;
      QCheckBox *cb_saveParams;
      QListBox *lb_model;
      US_Hydrodyn_Save *save_util; // to enable calling of save routines regardless of whether or not widget is available
      bool batch_widget;
      US_Hydrodyn_Batch *batch_window;
      QString residue_filename;
      QLabel *lbl_table;
      void do_reset();
      unsigned int numThreads; // local copy of USglobal->config_list.numThreads
      static QString fileNameCheck( QString filename, int mode = 0 ); 
      static QString fileNameCheck( QString *path, QString *base, QString *ext, int mode = 0 ); 
      // checks to see if file name exists, and if it does, according to 'mode'
      // mode == 0, stop and ask with the option for an new filename, mode == 1 auto increment, 
      void setSomoGridFile(bool); // checks for file existance and resets suffix accordingly, true for somo
      void setHydroFile(); // checks for file existance and resets suffix accordingly
      QCheckBox *cb_overwrite;
      QString last_hydro_res;
      void bd_anaflex_enables( bool flag ); // turns buttons on/off based upon current status
#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif
      vector < QString > movie_text;
#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

   private:
      bool residue_widget;
      bool atom_widget;
      bool hybrid_widget;
      bool saxs_widget;
      bool asa_widget;
      bool overlap_widget;
      bool grid_overlap_widget;
      bool bead_output_widget;
      bool grid_widget;
      bool hydro_widget;
      bool results_widget;
      bool misc_widget;
      bool pdb_parsing_widget;
      bool pdb_visualization_widget;
      bool saxs_options_widget;
      bool bd_widget;
      bool bd_options_widget;
      bool anaflex_options_widget;
      bool dmd_options_widget;
      bool saxs_plot_widget;
      bool advanced_config_widget;
      bool save_widget;

      QMenuBar *m;
      QPrinter printer;
      QFont ft;

      bool create_beads_normally; // true = normal, false = atoms are beads
      bool alt_method; // true = new bead method, false = original bead method
      unsigned int current_model;
      QString bead_model_file;
      QString pdb_file;
      struct residue current_residue;
      struct asa_options asa;
      struct asa_options default_asa;
      struct pdb_visualization pdb_vis;
      struct pdb_visualization default_pdb_vis;
      struct pdb_parsing default_pdb_parse;
      struct hydro_options default_hydro;
      struct grid_options grid;
      struct grid_options default_grid;
      struct saxs_options saxs_options;
      struct saxs_options default_saxs_options;
      struct bead_output_options bead_output;
      struct bead_output_options default_bead_output;
      hydro_results results;
      batch_info default_batch;
      save_info default_save_params;
      struct residue new_residue;
      struct atom new_atom;
      struct bead new_bead;
      bool replicate_o_r_method_somo;
      bool replicate_o_r_method_grid;
      struct overlap_reduction sidechain_overlap;
      struct overlap_reduction mainchain_overlap;
      struct overlap_reduction buried_overlap;
      struct overlap_reduction grid_exposed_overlap;
      struct overlap_reduction grid_buried_overlap;
      struct overlap_reduction grid_overlap;
      struct overlap_reduction default_sidechain_overlap;
      struct overlap_reduction default_mainchain_overlap;
      struct overlap_reduction default_buried_overlap;
      struct overlap_reduction default_grid_exposed_overlap;
      struct overlap_reduction default_grid_buried_overlap;
      struct overlap_reduction default_grid_overlap;
      QString project;   // name of the current project - derived from the prefix of the pdb filename
      QString bead_model_prefix;
      QString bead_model_suffix;
      QString somo_tmp_dir;

      QString bead_model_selected_filter;

      point last_molecular_cog;

      US_Config *USglobal;

      QLabel *lbl_info1;
      QLabel *lbl_info2;
      QLabel *lbl_info3;
      QLabel *lbl_pdb_file;
      QLabel *lbl_model;
      QLabel *lbl_somo;
      QLabel *lbl_bead_model_prefix;

      QCheckBox *cb_calcAutoHydro;
      QCheckBox *cb_setSuffix;

      QPopupMenu *lookup_tables;
      QPopupMenu *somo_options;
      QPopupMenu *md_options;
      QPopupMenu *pdb_options;
      QPopupMenu *configuration;
      QMenuBar *menu;

      QLineEdit *le_bead_model_file;
      QLineEdit *le_bead_model_prefix;
      QLineEdit *le_bead_model_suffix;

      QPushButton *pb_batch;
      QPushButton *pb_batch2;
      QPushButton *pb_save;
      QPushButton *pb_select_residue_file;
      QPushButton *pb_load_pdb;
      QPushButton *pb_pdb_saxs;
      QPushButton *pb_bead_saxs;
      QPushButton *pb_help;
      QPushButton *pb_cancel;
      QPushButton *pb_somo;
      QPushButton *pb_visualize;
      QPushButton *pb_view_pdb;
      QPushButton *pb_load_bead_model;
      QPushButton *pb_calc_hydro;
      QPushButton *pb_show_hydro_results;
      QPushButton *pb_open_hydro_results;
      QPushButton *pb_select_save_params;
      QPushButton *pb_grid_pdb;
      QPushButton *pb_grid;
      QPushButton *pb_view_asa;
      QPushButton *pb_view_bead_model;

      QPushButton *pb_dmd_run;

      QPushButton *pb_bd;

      //      QPushButton *pb_bd_prepare;
      //      QPushButton *pb_bd_load;
      //      QPushButton *pb_bd_edit;
      //      QPushButton *pb_bd_run;
      //      QPushButton *pb_bd_load_results;

      //      QPushButton *pb_anaflex_prepare;
      //      QPushButton *pb_anaflex_load;
      //      QPushButton *pb_anaflex_edit;
      //      QPushButton *pb_anaflex_run;
      //      QPushButton *pb_anaflex_load_results;

      QProgressBar *progress;
      TextEdit *e;

      US_AddAtom *addAtom;
      US_AddResidue *addResidue;
      US_AddHybridization *addHybrid;
      US_AddSaxs *addSaxs;
      US_Hydrodyn_ASA *asa_window;
      US_Hydrodyn_Overlap *overlap_window;
      US_Hydrodyn_Overlap *grid_overlap_window;
      US_Hydrodyn_Bead_Output *bead_output_window;
      US_Hydrodyn_BD *bd_window;
      US_Hydrodyn_BD_Options *bd_options_window;
      US_Hydrodyn_Anaflex_Options *anaflex_options_window;
      US_Hydrodyn_DMD_Options *dmd_options_window;
      US_Hydrodyn_Hydro *hydro_window;
      US_Hydrodyn_Misc *misc_window;
      US_Hydrodyn_Results *results_window;
      US_Hydrodyn_Grid *grid_window;
      US_Hydrodyn_PDB_Visualization *pdb_visualization_window;
      US_Hydrodyn_PDB_Parsing *pdb_parsing_window;
      US_Hydrodyn_Saxs *saxs_plot_window;
      US_Hydrodyn_SaxsOptions *saxs_options_window;
      US_Hydrodyn_AdvancedConfig *advanced_config_window;
      US_Hydrodyn_Save *save_window;
      QProcess *rasmol;

      QString getExtendedSuffix(bool prerun = true, bool somo = true); 
      
#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif
      vector < QString >              batch_file;
      vector < PDB_atom >             bead_model;
      vector < vector < PDB_atom > >  bead_models;
      vector < vector < PDB_atom > >  bead_models_as_loaded;
      vector < PDB_atom * >           active_atoms;
      vector < struct residue >       residue_list;
      vector < struct residue >       residue_list_no_pbr;
      map < QString, vector < int > > multi_residue_map; // maps residue to index of residue_list
      map < QString, vector < int > > valid_atom_map;    // maps resName|atomName|pos
      //                                                  in multi_residue_map to index of atoms
      map < QString, QString >        residue_atom_hybrid_map;
      //                                       maps resName|atomName to hybrid_name
      map < QString, int > atom_counts;     // maps molecule #|resName|resSeq to count
      //                                       counts how many atoms are in each residue
      map < QString, int > has_OXT;         // maps molecule #|resName|resSeq to {0,1}
      map < QString, int > bead_exceptions; // maps molecule #|resName|resSeq to count
      //                                       1 == ok
      //                                       2 == skip
      //                                       3 == use automatic bead builder for residue
      //                                       4 == use automatic bead builder for missing atom
      //                                       5 == not ok, duplicate or non-coded atom

      // save_residue_list and save_multi_residue_map contain the results of 'read_residue_file'
      // the automatic bead builder adds temporary residues, so it uses these to reset the values
      // in lieu of re-reading the residue file...
      vector <struct residue>   save_residue_list;
      vector <struct residue>   save_residue_list_no_pbr;
      map < QString, vector <int> > save_multi_residue_map; // maps residue to index of residue_list
      map < QString, int > new_residues;    // maps resName|atom_count to {0,1} for duplicate checks
      map < QString, vector < QString > > molecules_residues_atoms;
      //                                  maps molecule #|resSeq to vector of atom names
      map < QString, QString > molecules_residue_name;
      //                                  maps molecule #|resSeq to residue name
      vector < QString > molecules_idx_seq; // vector of idx's
      map < QString, vector < QString > > molecules_residue_errors;
      //                                  maps molecule #|resSeq to vector of errors
      //                                  each element in the vector corresponds to
      //                                  the dup_residue_map pos for the residue
      map < QString, vector < int > >     molecules_residue_missing_counts;
      //                                  maps molecule #|resSeq to vector of missing count
      //                                  if any atoms errors that are "non-missing" i.e.
      //                                  duplicate or non-coded, then the value is set to -1
      map < QString, vector < vector < QString > > > molecules_residue_missing_atoms;
      //                                  maps molecule #|resSeq to vector of vector of missing atoms
      //                                  each vector in the vector corresponds to
      //                                  the dup_residue_map pos for the residue
      map < QString, vector < vector < unsigned int > > > molecules_residue_missing_atoms_beads;
      //                                  maps molecule #|resSeq to vector of vector of missing atoms beads
      //                                  each vector in the vector corresponds to
      //                                  the dup_residue_map pos for the residue
      map < QString, bool >               molecules_residue_missing_atoms_skip;
      //                                  maps molecule #|resSeq|multiresmappos|atom_no to flag
      //                                  if true, the atom should be skipped
      map < QString, int >                molecules_residue_min_missing;
      //                                  maps molecule #|resSeq to pos of entry with minimum missing count
      map < QString, bool >               broken_chain_end;
      //                                  maps molecule #|resSeq to flag indicating broken end of chain
      map < QString, bool >               broken_chain_head;
      //                                  maps molecule #|resSeq to flag indicating broken head of chain
      map < QString, bool >               unknown_residues;
      //                                  maps resName to flag for unknown residue
      
      QString last_abb_msgs; // automatic bead builder message log
      vector <struct PDB_model> model_vector;
      vector <struct PDB_model> model_vector_as_loaded;
      vector <int> somo_processed;
      QString options_log;

      bool install_new_version();         
      // checks for new versions of somo.config, .atom, .saxs_atom, .hybrid, .residue and backs up previous versions


      // bd private data
      bool bd_ready_to_run;
      bool anaflex_ready_to_run;

      //    next three are keyed upon string "bead#~bead#"
      map < QString, bool > connection_active;
      map < QString, vector < float > > connection_dists;
      map < QString, vector < float > > connection_dist_stats;  // 0 = min, 1 = max, 2 = avg
      map < QString, int > connection_pair_type; // 0 mc-mc, 1 mc-sc, 2 sc-sc
      map < QString, bool > connection_forced;  // chemical connections

      QProcess *browflex;
      QProcess *anaflex;

      QString bd_project;
      unsigned int bd_current_model;
      QString bd_last_file;
      QString anaflex_last_file;

      QString bd_last_traj_file;  // needed for anaflex and to load bd results
      QString bd_last_molec_file;  // needed to load bd results
      QString anaflex_last_log_file; // needed to load bd results
      QString anaflex_last_out1_file; // needed to load bd results

      bool anaflex_return_to_bd_load_results;
      float bd_load_results_solvent_visc;
      float bd_load_results_temp;
      unsigned int bd_load_results_beads;
      float bd_load_results_mw;
      vector < float > bd_load_results_bead_radius;

      bool check_bead_model_for_nan();

      QString last_read_bead_model;

      QString last_spt_text;

      // needed 

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

   public slots:
      void display_default_differences();
      void clear_display();
      void reload_pdb();
      int calc_somo();     // compute asa and then refine bead_model
      int calc_grid_pdb(); // compute grid model from pdb
      int calc_grid();     // compute grid model from bead model
      int calc_hydro();
      void select_save_params();
      void show_saxs_options();
      void show_bd_options();
      void show_anaflex_options();
      void show_dmd_options();
      void read_residue_file();
      int read_config(const QString &);
      int read_config(QFile &);
      void show_misc();

      void bd_prepare();
      void bd_load();
      void bd_edit();
      void bd_run();
      void bd_load_results();

      void anaflex_prepare();
      void anaflex_load();
      void anaflex_edit();
      void anaflex_run();
      void anaflex_load_results();

      void stop_calc(); // stop some operations
      void visualize(bool movie_frame = false, QString dir = "", float scale = 1.0, bool black_background = false);

   private slots:
      void browflex_readFromStdout();
      void browflex_readFromStderr();
      void browflex_launchFinished();
      void browflex_processExited();

      void anaflex_readFromStdout();
      void anaflex_readFromStderr();
      void anaflex_launchFinished();
      void anaflex_processExited();

      void load_pdb();
      void show_batch();
      int read_pdb(const QString &);
      int read_bead_model(QString);
      void load_bead_model();
      void setupGUI();
      void select_residue_file();
      void clear_temp_chain(struct PDB_chain *);
      // The next function returns a boolean to indicate whether the atom that was just assigned belongs
      // to a new residue or is part of a previously assigned residue
      // true: new residue sequence number, false: still the same
      // the integer argument contains the last sequence number (last_resSeq)
      bool assign_atom(const QString &, struct PDB_chain *, QString *);
      void cancel();
      void help();
      void edit_atom();
      void hybrid();
      void residue();
      void saxs();
      void select_model(int);
      void calc_bead_mw(struct residue *); // calculate the molecular weight of all beads in residue
      int create_beads(QString *error_string); // turn pdb/atom model into bead_model
      void get_atom_map(PDB_model *);
      int check_for_missing_atoms(QString *error_string, PDB_model *);
      void build_molecule_maps(PDB_model *model); // sets up maps for molecule
      int overlap_check(bool sc, bool mc, bool buried, double tolerance); // check for overlaps
      int compute_asa(bool bd_mode = false); // calculate maximum accessible surface area
      void show_asa();
      void show_bd();
      void show_overlap();
      void show_grid_overlap();
      void show_bead_output();
      void show_hydro();
      void show_grid(); // show grid options
      void show_advanced_config();
      void view_pdb(); // show pdb file in editor
      void pdb_parsing(); // pdb parsing options
      void pdb_visualization(); // pdb visualization options
      void view_asa(); // show asa file in editor
      void view_bead_model(); // show bead model file in editor
      void view_file(const QString &, QString title = "SOMO editor"); // call editor to view a file
      void bead_check( bool use_threshold = false, bool message_type = false ); // recheck beads
      void load_config();
      void write_config();
      void write_config(const QString &);
      void reset();
      void set_default();
      void update_bead_model_file(const QString &);
      void update_bead_model_prefix(const QString &);
      void radial_reduction();
      void pdb_saxs();
      void bead_saxs();
      void show_hydro_results();
      void open_hydro_results();
      void write_bead_asa(QString, vector <PDB_atom> *);
      void write_bead_tsv(QString, vector <PDB_atom> *);
      void write_bead_ebf(QString, vector <PDB_atom> *);
      void write_bead_spt(QString, vector <PDB_atom> *, bool movie_frame = false, float scale = 1, bool black_background = false);
      void write_bead_model(QString, vector <PDB_atom> *);
      void write_corr(QString, vector <PDB_atom> *);
      bool read_corr(QString, vector <PDB_atom> *);
      void printError(const QString &);
      void closeAttnt(QProcess *, QString);
      void calc_vbar(struct PDB_model *);
      void update_vbar(); // update the results.vbar everytime something changes the vbar in options or calculation
      void append_options_log_somo(); // append somo options to options_log
      void append_options_log_atob(); // append atob options to options_log
      void list_model_vector(vector < PDB_model > *);
      QString default_differences_load_pdb();
      QString default_differences_somo();
      QString default_differences_grid();
      QString default_differences_hydro();
      QString default_differences_misc();
      QString default_differences_saxs_options();
      // editor functions:
      void save();
      void print();
      void update_font();
      void set_calcAutoHydro();
      void set_setSuffix();
      void set_overwrite();
      void set_saveParams();

      // message utility
      void editor_msg( QString color, QString msg );

      // dmd functions:
      void dmd_run();

      // bd functions:
      //      void bd_mode();
      bool bd_valid_browflex_main( QString filename );
      void bd_load_error( QString filename );
      void bd_edit_util( QString dir, QString filename ); 
      void bd_load_results_after_anaflex();
      bool anaflex_valid_anaflex_main( QString filename );
      void anaflex_load_error( QString filename );

      int create_browflex_files();
      int run_browflex();
      int browflex_get_no_of_beads( QString filename ); // open main file, read molec file, return # of beads or zero if not found

      int compute_pb_normals();                // compute the normal vectors for each peptide bond plane
      int build_pb_structures( PDB_model * ) ; // create pb data structures
      point minus( point p1, point p2 );       // returns p1 - p2
      point cross( point p1, point p2 );       // returns p1 x p2
      float dot( point p1, point p2);          // p1 dot p2
      point normal( point p1 );                // normalized point
      point plane( PDB_atom *a1, PDB_atom *a2, PDB_atom *a3 );  //( a3 - a2 ) x ( a1 - a2 )
      point average( vector < point > *v );    // returns an average vector
      float dist( point p1, point p2);         // sqrt( (p1 - p2) dot (p1 - p2) )

      int compute_bd_connections();            // computes intersected pb beads within tolerance map
      int write_pdb( QString fname, vector < PDB_atom > *model );

      // ancillary pdb utils

      int write_contact_plot( QString fname, PDB_model *model, float thresh );
      int write_contact_plot( QString fname, vector < PDB_atom > *model, float thresh );

      // anaflex functions


      int create_anaflex_files();
      int create_anaflex_files( int use_mode, int sub_mode = 0 );
      int run_anaflex();
      int run_anaflex( int use_mode, int sub_mode = 0 );

   protected slots:

      void closeEvent(QCloseEvent *);
};


#include <qthread.h>
#include <qwaitcondition.h>
#include <qmutex.h>

struct BPair {
   int i;
   int j;
   float separation;
   bool active;
};


class radial_reduction_thr_t : public QThread
{
 public:
  radial_reduction_thr_t(int);
  void radial_reduction_thr_setup(
                                  unsigned int,
                                  vector <PDB_atom> *,
                                  vector <bool> *,
                                  vector <bool> *,
                                  vector <BPair> *,
                                  unsigned int,
                                  double
                                  );
  void radial_reduction_thr_shutdown();
  void radial_reduction_thr_wait();
  virtual void run();

 private:

  int methodk;

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

  vector <PDB_atom> *p_bead_model;
  vector <bool> *p_last_reduced;
  vector <bool> *p_reduced;
  vector <BPair> *p_my_pairs;

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif
  unsigned int threads;

  double overlap_tolerance;

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
