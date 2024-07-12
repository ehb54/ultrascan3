#ifndef US_HYDRODYN_H
#define US_HYDRODYN_H

// QT defs:

#include <qfile.h>
#include <qfiledialog.h>
//#include <q3textstream.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
//#include <q3frame.h>
#include <qprogressbar.h>
#include <qprocess.h>
#include <qdir.h>
#include <qtextedit.h>
#include <qmenubar.h>
//#include <q3popupmenu.h>
#include <qprinter.h>
#include "qwt_counter.h"
#include <qtreewidget.h>
//Added by qt3to4:
#include <QCloseEvent>
#include <QHeaderView>

#include "us.h"
#include "us_timer.h"
#include "us_extern.h"
#include "us_util.h"
#include "us_math.h"
#include "us_json.h"
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
#include "us_hydrodyn_hydro_zeno.h"
#include "us_hydrodyn_misc.h"
#include "us_hydrodyn_grid.h"
#include "us_hydrodyn_results.h"
#include "us_hydrodyn_pdb_visualization.h"
#include "us_hydrodyn_pdb_parsing.h"
#include "us_hydrodyn_sas_options_saxs.h"
#include "us_hydrodyn_sas_options_sans.h"
#include "us_hydrodyn_sas_options_bead_model.h"
#include "us_hydrodyn_sas_options_curve.h"
#include "us_hydrodyn_sas_options_hydration.h"
#include "us_hydrodyn_sas_options_guinier.h"
#include "us_hydrodyn_sas_options_xsr.h"
#include "us_hydrodyn_sas_options_misc.h"
#include "us_hydrodyn_sas_options_experimental.h"
#include "us_hydrodyn_saxs_options.h"
#include "us_hydrodyn_saxs.h"
#include "us_hydrodyn_saxs_screen.h"
#include "us_hydrodyn_saxs_search.h"
#include "us_hydrodyn_saxs_buffer.h"
#include "us_hydrodyn_saxs_hplc.h"
#include "us_hydrodyn_dad.h"
#include "us_hydrodyn_mals.h"
#include "us_hydrodyn_mals_saxs.h"
#include "us_hydrodyn_saxs_1d.h"
#include "us_hydrodyn_saxs_2d.h"
#include "us_hydrodyn_advanced_config.h"
#include "us_hydrodyn_batch.h"
#include "us_hydrodyn_save.h"
#include "us_hydrodyn_file.h"
#include "us_hydrodyn_file2.h"
#include "us_hydrodyn_dammin_opts.h"
#include "us_hydrodyn_bd_load_results_opts.h"
#include "us_hydrodyn_bd.h"
#include "us_hydrodyn_batch_movie_opts.h"
#include "us_hydrodyn_comparative.h"
#include "us_hydrodyn_pdb_tool.h"
#include "us_hydrodyn_cluster.h"
#include "us_saxs_util.h"
// #include "us_hydrodyn_fractal_dimension_options.h"
#include "us_container_grpy.h"

// #include "us_hydrodyn_pat.h"

#include "us3i_gui_settings.h"
#include "us3i_editor.h"

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

struct _vdwf {
   double mw;
   double ionized_mw_delta;
   double r;
   double w;
   double e;
   int color;
};

class US_EXTERN US_Hydrodyn : public QFrame
{
   Q_OBJECT

   public:
      friend class US_Hydrodyn_Batch;
      friend class US_Hydrodyn_Cluster;
      friend class US_Hydrodyn_Cluster_Dmd;
      friend class US_Hydrodyn_Dad;
      friend class US_Hydrodyn_Dad_Svd;
      friend class US_Hydrodyn_Dad_Options;
      friend class US_Hydrodyn_Dad_Baseline_Best;
      friend class US_Hydrodyn_Dad_Parameters;
      friend class US_Hydrodyn_Mals;
      friend class US_Hydrodyn_Mals_Svd;
      friend class US_Hydrodyn_Mals_Options;
      friend class US_Hydrodyn_Mals_Baseline_Best;
      friend class US_Hydrodyn_Mals_Parameters;
      friend class US_Hydrodyn_Mals_Saxs;
      friend class US_Hydrodyn_Mals_Saxs_Svd;
      friend class US_Hydrodyn_Mals_Saxs_Options;
      friend class US_Hydrodyn_Mals_Saxs_Baseline_Best;
      friend class US_Hydrodyn_Saxs;
      friend class US_Hydrodyn_Hydro;
      friend class US_Hydrodyn_Saxs_Screen;
      friend class US_Hydrodyn_Saxs_Search;
      friend class US_Hydrodyn_Saxs_Buffer;
      friend class US_Hydrodyn_Saxs_Hplc;
      friend class US_Hydrodyn_Saxs_Hplc_Svd;
      friend class US_Hydrodyn_Saxs_Hplc_Options;
      friend class US_Hydrodyn_Saxs_1d;
      friend class US_Hydrodyn_Saxs_2d;
      friend class US_Hydrodyn_Misc;
      friend class US_Hydrodyn_Pdb_Tool;
      friend class US_Hydrodyn_Pdb_Tool_Merge;
      friend class US_Hydrodyn_SaxsOptions;
      friend class US_Hydrodyn_Zeno;
      friend class US_Hydrodyn_Xsr;
      friend class US_Hydrodyn_Cluster_Additional;
      friend class US_Hydrodyn_SasOptionsExperimental;
      friend class US_Hydrodyn_SasOptionsGuinier;
      friend class US_Hydrodyn_SasOptionsSans;
      friend class US_Hydrodyn_Hplc;
      friend class US_Hydrodyn_Saxs_Cormap;
      friend class US_Hydrodyn_Saxs_Hplc_Baseline_Best;
      friend class US_Hydrodyn_AdvancedConfig;

      US_Hydrodyn(vector < QString >,
                  QString gui_script_file = "",
                  bool init_configs_silently = false,
                  QWidget *p = 0, 
                  const char *name = 0);

      ~US_Hydrodyn();
      int get_color(const PDB_atom *);
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
      bool overwrite_hydro;
      bool saveParams;
      bool guiFlag;
      QLabel *lbl_core_progress;
      void set_disabled( bool clear_bead_model_file = true );
      void set_enabled();
      mQTextEdit *editor;
      void play_sounds(int);
      struct pdb_parsing pdb_parse;
      bool bead_model_from_file;
      batch_info batch;
      save_info save_params;
      comparative_info comparative;
      QString somo_dir;
      QString somo_pdb_dir;
      bool screen_pdb( QString, bool display_pdb = false, bool skip_clear_issue = false );
      bool screen_bead_model(QString);
      QPushButton *pb_stop_calc;
      struct hydro_options hydro;
      QCheckBox *cb_saveParams;
      QListWidget *lb_model;
      US_Hydrodyn_Save *save_util; // to enable calling of save routines regardless of whether or not widget is available
      bool batch_widget;
      US_Hydrodyn_Batch *batch_window;
      bool comparative_widget;
      US_Hydrodyn_Comparative *comparative_window;
      bool best_widget;
      QWidget *best_window;
      QString residue_filename;
      QLabel *lbl_table;
      void do_reset();
      unsigned int numThreads; // local copy of USglobal->config_list.numThreads
      static QString fileNameCheck( QString filename, int mode = 0, QWidget *p = 0 ); 
      static QString fileNameCheck( QString *path, QString *base, QString *ext, int mode = 0, QWidget *p = 0 ); 
      static QString fileNameCheck2( QString filename, bool &cancel, bool &overwrite_all, int mode = 0, QWidget *p = 0 ); 
      static QString fileNameCheck2( QString *path, QString *base, QString *ext, bool &cancel, bool &overwrite_all, int mode = 0, QWidget *p = 0 ); 
      // checks to see if file name exists, and if it does, according to 'mode'
      // mode == 0, stop and ask with the option for an new filename, mode == 1 auto increment, 
      void setSomoGridFile(bool); // checks for file existance and resets suffix accordingly, true for somo
      void setHydroFile(); // checks for file existance and resets suffix accordingly
      static void sizeArrows( QwtCounter* );
      static void fixWinButtons( QWidget* );
      QCheckBox *cb_overwrite;
      QString last_hydro_res;
      void bd_anaflex_enables( bool flag ); // turns buttons on/off based upon current status
      US_Saxs_Util *saxs_util;

      vector < QString > movie_text;
      QString last_saxs_header;

      vector < double > last_saxs_q;
      vector < double > last_saxs_iqq;
      vector < double > last_saxs_iqqa;
      vector < double > last_saxs_iqqc;
      vector < double > last_saxs_r;
      vector < double > last_saxs_prr;
      vector < double > last_saxs_prr_norm;
      float last_saxs_prr_mw;

      map < QString, float > dammix_remember_mw;
      map < QString, float > dammix_match_remember_mw;
      map < QString, QString > dammix_remember_mw_source;

      map < QString, saxs > extra_saxs_coefficients;
      bool is_dammin_dammif(QString filename);

      QString saxs_sans_ext(); // returns correct extension based upon current saxs/sans mode

      // save file paths
      QString path_load_pdb;
      QString path_view_pdb;
      QString path_load_bead_model;
      QString path_view_asa_res;
      QString path_view_bead_model;
      QString path_open_hydro_res;

      QString pdb_file;

      US_Hydrodyn_Saxs *saxs_plot_window;
      bool saxs_plot_widget;

      map < QString, QString >            gparams;
      QString                             gparam_value( const QString & param ); // convenience function, returns empty string if not defined

      US_Config *USglobal;

      // view a pdb or bead model file running in directory of file

      void model_viewer( QString file,
                         QString prefix = "",
                         bool nodisplay = false );  

      double use_solvent_visc();                     // temperature solvent viscosity - checks manual flag
      double use_solvent_dens();                     // temperature solvent density   - checks manual flag
      double use_vbar( double vbar );                // temperature vbar
      
      struct asa_options asa;

      void write_dati1_supc_bead_model( QString filename,
                                        int size,
                                        void * dt );

      void write_dati1_pat_bead_model( QString filename,
                                       int size,
                                       void * dtp,
                                       void * dts );

   private:
      bool                grpy_parallel_pulled;
      US_Container_Grpy * us_container_grpy;
      
      set < QString > residues_with_atomic_vs_bead_hydration_differences;
      void            compute_residues_with_atomic_vs_bead_hydration_differences( const vector < struct residue > & rl = {} );
      bool            residue_atomic_vs_bead_hydration_differences( const struct residue & r );
      QString         list_residues_with_atomic_vs_bead_hydration_differences();
      void            info_residue( struct residue & r, const QString & msg = "" );
      bool            model_vector_has_hydration_differences( const vector < struct PDB_model > & m );

      bool     gui_script;
      QString  gui_script_file;
      void     gui_script_msg  ( int line, QString arg, QString msg );
      void     gui_script_error( int line, QString arg, QString msg, bool doexit = true );

      bool     init_configs_silently;
      
      map < QString, struct atom * > residue_atom_map( struct residue & residue_entry );
      map < QString, struct atom * > first_residue_atom_map( struct PDB_chain & chain );
      map < QString, struct atom * > last_residue_atom_map( struct PDB_chain & chain );
      map < QString, int >           first_residue_PDB_atom_map( struct PDB_chain & chain );
      
      vector < PDB_atom > exposed_model( const vector < PDB_atom > & model );
      
      bool mm_mode;
      bool pat_model( vector < PDB_atom > & model );
      bool compute_asa_rgs( const vector < PDB_atom > & model );

      void set_bead_colors( vector < PDB_atom * > use_model );
      void set_bead_colors( vector < PDB_atom > & use_model );
      bool batch_avg_hydro_active();
      bool batch_active();
      double model_mw( const vector < PDB_atom > & use_model );
      double model_mw( const vector < PDB_atom * > use_model );

      int grpy_used_beads_count( const vector < PDB_atom > & use_model );
      int grpy_used_beads_count( const vector < PDB_atom * > use_model );

      int total_beads_count( const vector < PDB_atom > & use_model );
      // protons at pH
      double protons_at_pH( double pH, const struct PDB_model & model );
      double compute_isoelectric_point( const struct PDB_model & model );

      QString model_summary_msg( const QString &msg, struct PDB_model *model );    // summary info about model
      bool model_summary_csv( struct PDB_model *model, const QString & filename = "model_load_summary.csv" ); 
      bool model_summary_csv( const QString & filename = "model_load_summary.csv" ); 
      double tc_vbar( double vbar );                           // temperature corrected vbar
      QString pH_msg();                                        // vbar message
      QString vbar_msg( double vbar, bool only_used = false ); // vbar message
      double tc_solvent_visc();                                // temperature solvent viscosity
      double tc_solvent_dens();                                // temperature solvent density
      QString visc_dens_msg( bool only_used = false );         // return density and viscosity message

   // distance threshold check support
      void SS_setup();                              // called once to setup any persistant distance threshold structures
      void SS_init();                               // called during load pdb to setup any processing structures
      void SS_apply( struct PDB_model & model,
                     const QString & ssbondfile );  // called during load pdb to process any adjustments (CYS->CYH etc) *** per model! ***
      void SS_change_residue(                       // change the residue of an entry to target_residue
                             struct PDB_model & model
                             ,const QString & line
                             ,const QString target_residue
                                                );  
      
      void update_model_chain_ionization( struct PDB_model & model, bool quiet = true ); // progagate ionization
      
      set < QString >                                           cystine_residues;
      set < QString >                                           sulfur_atoms;

      vector < QString >                                        sulfur_pdb_line;
      vector < point   >                                        sulfur_coordinates;

      map < QString, map < QString, vector < unsigned int > > > sulfur_pdb_chain_atom_idx;
      map < QString, vector < unsigned int > >                  sulfur_pdb_chain_idx;

      map < int, int >                                          sulfur_paired;
      
      // end distance threshold check support

      map < QString, double >                                   hybrid_to_protons;   // for calculating net charge
      map < QString, double >                                   hybrid_to_electrons; // for calculating net charge
      void read_hybrid_file( QString filename );
      
      void fix_N1_non_pbr( struct PDB_model & model );

      // info routines (in us_hydrodyn_info.cpp

      void info_bead_models_mw( const QString & msg, const vector < vector < PDB_atom > > & b_models );
      void info_bead_models_mw( const QString & msg, const vector < PDB_atom > & b_model );
      void info_model_vector( const QString & msg, const vector <struct PDB_model> & models, const set < QString > only_atoms = {} );
      void info_model_residues( const QString & msg, struct PDB_model & model );
      void info_model_vector_mw  ( const QString & msg, const vector <struct PDB_model> & models, bool detail = false );
      void info_model_vector_vbar( const QString & msg, const vector <struct PDB_model> & models );
      void info_residue_p_residue( struct PDB_model & model ); // consistency report
      void info_compare_residues( struct residue * res1, struct residue * res2 ); // compare residues
      void info_residue_vector( const QString & msg, vector < struct residue > & residue_v, bool only_pKa_dependent = true );
      void info_model_p_residue( const QString & msg, const struct PDB_model & model, bool only_pKa_dependent );
      void info_mw( const QString & msg, const vector < struct PDB_model > & models, bool detail = false );
      void info_mw( const QString & msg, const struct PDB_model & model, bool detail = false );
      void info_residue_protons_electrons_at_pH( double pH, const struct PDB_model & model );
      QString info_cite( const QString & package );
      QStringList citation_stack;
      void citation_clear();
      QString citation_cleanup( const QString & qs );
      bool citation_stack_contains_type( const QString & type );
      void citation_load_pdb();
      void citation_load_bead_model( const QString & filename );
      void info_citation_stack();
      void citation_build_bead_model( const QString & type );
      QString split_and_prepend( const QString & qs, const QString & prepend );

      void set_ionized_residue_vector( vector < struct residue > & residue_v ); // apply ionization to complete residue vector
      void reset_ionized_residue_vectors(); // for saxs hplc options

      // in attic: vector < struct residue * > model_residue_v( const struct PDB_model & model ); // returns a residue * vector based upon PDB_atoms's p_residue

      // end info routines

      map < QString, double >  res_vbar; // for fasta 
      // map < QString, double >  res_vbar2; 
      // map < QString, double >  res_pKa;
      map < QString, double >  res_mw;
      // map < QString, double >  res_ionization_mass_change;
      map < QString, double >  fasta_vbar;
      map < QString, double >  fasta_mw;
      void create_fasta_vbar_mw();
      bool calc_fasta_vbar( QStringList & seq_chars, double &result, QString &msgs );

      bool load_vdwf_json( QString filename );
      map < QString, _vdwf > vdwf;

      bool last_no_model_selected;

      bool residue_widget;
      bool atom_widget;
      bool hybrid_widget;
      bool asa_widget;
      bool overlap_widget;
      bool grid_overlap_widget;
      bool bead_output_widget;
      bool grid_widget;
      bool hydro_widget;
      bool hydro_zeno_widget;
      bool results_widget;
      bool misc_widget;
      bool pdb_parsing_widget;
      bool pdb_visualization_widget;
      bool saxs_widget;
      bool fractal_dimension_options_widget;
      bool vdw_overlap_widget;

      bool saxs_options_widget;
      bool sas_options_saxs_widget;
      bool sas_options_sans_widget;
      bool sas_options_curve_widget;
      bool sas_options_bead_model_widget;
      bool sas_options_hydration_widget;
      bool sas_options_guinier_widget;
      bool sas_options_xsr_widget;
      bool sas_options_misc_widget;
      bool sas_options_experimental_widget;

      bool bd_widget;
      bool bd_options_widget;
      bool anaflex_options_widget;
      bool dmd_options_widget;
      bool advanced_config_widget;
      bool save_widget;

      QMenuBar *m;
      QPrinter printer;
      QFont ft;

      bool create_beads_normally; // true = normal, false = atoms are beads
      bool alt_method; // true = new bead method, false = original bead method
      QString bead_model_file;
      struct residue current_residue;
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
      QString somo_tmp_dir;

      QString bead_model_selected_filter;

      point last_molecular_cog;

      QLabel *lbl_info1;
      QLabel *lbl_info2;
      QLabel *lbl_info3;
      mQLineEdit *le_pdb_file;
      QLabel *lbl_model;
      QLabel *lbl_rbh;
      QLabel *lbl_somo;
      QLabel *lbl_bead_model_prefix;

      QCheckBox *cb_calcAutoHydro;
      QCheckBox *cb_setSuffix;

 //      Q3PopupMenu *lookup_tables;
 //      Q3PopupMenu *somo_options;
 //      Q3PopupMenu *md_options;
 //      Q3PopupMenu *pdb_options;
 //      Q3PopupMenu *configuration;
      QMenuBar *menu;

      QLineEdit *le_bead_model_file;
      QLineEdit *le_bead_model_prefix;
      QTextEdit *le_bead_model_suffix;

      QPushButton *pb_batch;
      QPushButton *pb_batch2;
      QPushButton *pb_save;
      QPushButton *pb_select_residue_file;
      QPushButton *pb_load_pdb;
      QPushButton *pb_reload_pdb;
      QPushButton *pb_pdb_hydrate_for_saxs;
      QPushButton *pb_pdb_saxs;
      QPushButton *pb_bead_saxs;
      QPushButton *pb_rescale_bead_model;
      QPushButton *pb_equi_grid_bead_model;
      QPushButton *pb_help;
      QPushButton *pb_config;
      QPushButton *pb_cancel;
      QPushButton *pb_somo;
      QPushButton *pb_somo_o;
      QPushButton *pb_visualize;
      QPushButton *pb_view_pdb;
      QPushButton *pb_pdb_tool;
      QPushButton *pb_load_bead_model;
      QPushButton *pb_calc_hydro;
      QPushButton *pb_calc_zeno;
      QPushButton *pb_calc_grpy;
      QPushButton *pb_calc_hullrad;
      QPushButton *pb_show_hydro_results;
      QPushButton *pb_open_hydro_results;
      QPushButton *pb_select_save_params;
      QPushButton *pb_grid_pdb;
      QPushButton *pb_vdw_beads;
      QPushButton *pb_grid;
      QPushButton *pb_view_asa;
      QPushButton *pb_view_bead_model;

      QLabel      *lbl_temperature;
      QLineEdit   *le_temperature;
      QCheckBox   *cb_pH;
      QLineEdit   *le_pH;

      QPushButton *pb_dmd_run;

      QPushButton *pb_bd;

      QPushButton *pb_fractal_dimension;
      
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

      QPushButton *pb_comparative;
      QPushButton *pb_best;

      // TextEdit *e;

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
      US_Hydrodyn_Hydro_Zeno *hydro_zeno_window;
      US_Hydrodyn_Misc *misc_window;
      US_Hydrodyn_Results *results_window;
      US_Hydrodyn_Grid *grid_window;
      US_Hydrodyn_PDB_Visualization *pdb_visualization_window;
      US_Hydrodyn_PDB_Parsing *pdb_parsing_window;
      US_Hydrodyn_SaxsOptions *saxs_options_window;
      QFrame * fractal_dimension_options_window;
      QFrame * vdw_overlap_window;

      US_Hydrodyn_SasOptionsSaxs         * sas_options_saxs_window;
      US_Hydrodyn_SasOptionsSans         * sas_options_sans_window;
      US_Hydrodyn_SasOptionsCurve        * sas_options_curve_window;
      US_Hydrodyn_SasOptionsBeadModel    * sas_options_bead_model_window;
      US_Hydrodyn_SasOptionsHydration    * sas_options_hydration_window;
      US_Hydrodyn_SasOptionsGuinier      * sas_options_guinier_window;
      US_Hydrodyn_SasOptionsXsr          * sas_options_xsr_window;
      US_Hydrodyn_SasOptionsMisc         * sas_options_misc_window;
      US_Hydrodyn_SasOptionsExperimental * sas_options_experimental_window;

      US_Hydrodyn_AdvancedConfig *advanced_config_window;
      US_Hydrodyn_Save *save_window;
      QProcess *rasmol;

      QString getExtendedSuffix(bool prerun = true, bool somo = true, bool no_ovlp_removal = false, bool vdw = false ); 
      
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
      map < QString, QString >        residue_atom_abb_hybrid_map;
      //                                       maps atomName to hybrid_name for abb and nc usage
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

      map < QString, int  >               use_residue;
      //                                  maps molecule #|resSeq to correct residue in residue_list to use
      map < QString, bool  >              skip_residue;
      //                                  flags molecule "#|resSeq|residue_list entry" to be ignored
      
      map < QString, QString >            default_gparams;

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

      map < QString, float > dammix_remember_psv;

      map < QString, float > dammix_match_remember_psv;
      map < QString, bool >  dammix_match_remember_write_bead_model;

      map < QString, QChar > residue_short_names;

      enum load_state { NOTHING_LOADED, PDB_LOADED, BEAD_MODEL_LOADED };
      load_state state;
      void update_enables();

      enum issue_msg 
      { 
         ISSUE_RESPONSE_STOP
         ,ISSUE_RESPONSE_NC_SKIP
         ,ISSUE_RESPONSE_NC_REPLACE
         ,ISSUE_RESPONSE_MA_SKIP
         ,ISSUE_RESPONSE_MA_MODEL
      };

      int issue_non_coded   ( bool quiet = false );
      int issue_missing_atom( bool quiet = false );
      set < QString > issue_info; 

      bool restore_pb_rule;
      set < QString > pdb_info;
      set < QString > state_pdb_info;

      void clear_pdb_info( QString msg );
      void set_pdb_info  ( QString msg );
      void sync_pdb_info ( QString msg );

      void dna_rna_resolve();  // often DNA is encoded without the D, check each chain and ask about & adding it

      map < QString, bool >                     cluster_additional_methods_options_active;
      map < QString, map < QString, QString > > cluster_additional_methods_options_selected;

      void calc_mw();             // after pdb (bead_model?) loaded, go through and compute PDB_model.mw (& later PDB_chain.mw?)
      void calc_vol_for_saxs();   // run when there are errors
      void calc_bead_mw(struct residue *); // calculate the molecular weight of all beads in residue

      int pdb_asa_for_saxs_hydrate();

      QString list_exposed();
      void    view_exposed();

      QString validate_pointmap();

      map < QString, vector < vector < QString > > > dihedral_atoms;

      map < QString, vector < vector < QString > > > pointmap_atoms;
      map < QString, vector < vector < QString > > > pointmap_atoms_dest;
      map < QString, vector < QString > >            pointmap_atoms_ref_residue;

      map < QString, vector < rotamer > >            rotamers;
      map < QString, vector < vector < rotamer > > > rotated_rotamers;
      map < QString, map < QString, point > >        to_hydrate;
      map < QString, map < QString, point > >        to_hydrate_pointmaps;
      map < QString, vector < float > >              to_hydrate_dihedrals;
      map < QString, rotamer >                       best_fit_rotamer;
      map < QString, vector < rotamer > >            pointmap_rotamers;
      map < QString, vector < point > >              waters_to_add;
      map < QString, unsigned int >                  steric_clash_summary;
      vector < map < QString, vector < point > > >   alt_waters_to_add;
      vector < map < QString, unsigned int > >       alt_steric_clash_summary;

      map < QString, bool >                          hydrate_clash_map_structure;
      map < QString, bool >                          hydrate_clash_map_rtmr_water;
      map < QString, bool >                          hydrate_clash_map_pm_water;
      map < QString, vector < QString > >            waters_source;
      vector < map < QString, vector < QString > > > alt_waters_source;
      vector < map < QString, bool > >               alt_hydrate_clash_map_structure;
      vector < map < QString, bool > >               alt_hydrate_clash_map_rtmr_water;
      vector < map < QString, bool > >               alt_hydrate_clash_map_pm_water;

      map < QString, float >                         residue_asa;
      map < QString, float >                         residue_asa_sc;
      map < QString, float >                         residue_asa_mc;

      map < QString, unsigned int >                  hydrate_count;

      unsigned int hydrate_max_waters_no_asa;
      unsigned int count_waters;
      unsigned int count_waters_added;
      unsigned int count_waters_not_added;

      bool load_rotamer( QString &error_msg );
      QString list_rotamers( bool coords = false );

      QString list_pointmaps();

      QString list_rotated_rotamers();

      bool compute_rotamer_dihedrals( QString &error_msg );
      QString list_rotamer_dihedrals();

      bool compute_water_positioning_atoms( QString &error_msg );
      QString list_water_positioning_atoms();

      void build_to_hydrate();
      QString list_to_hydrate( bool coords = false );

      bool compute_to_hydrate_dihedrals( QString &error_msg );
      QString list_to_hydrate_dihedrals();

      bool compute_best_fit_rotamer( QString &error_msg );
      QString list_best_fit_rotamer();

      bool setup_pointmap_rotamers( QString &error_msg );
      QString list_pointmap_rotamers();

      bool compute_waters_to_add( QString &error_msg, bool quiet = false );
      bool compute_waters_to_add_alt( QString &error_msg, bool quiet = false );
      QString list_waters_to_add();

      bool    has_steric_clash( point p, bool summary = true );
      float   min_dist_to_waters( point p );
      float   min_dist_to_struct_and_waters( point p );
      QString list_steric_clash();
      QString list_steric_clash_recheck( bool quiet = false );

      bool write_pdb_with_waters( QString &error_msg, bool quiet = false );
      bool alt_write_pdb_with_waters( QString &error_msg, bool quiet = false );
      QString last_hydrated_pdb_name;
      QString last_hydrated_pdb_text;
      QString last_hydrated_pdb_header;

      QStringList hydrate_clash_log;

      // atom align computes a transformation matrix 
      // on the transform_from to the transform_to
      // applies it to the apply_from and
      // leaves the result in result

      bool atom_align( vector < point > transform_from, 
                       vector < point > transform_to, 
                       vector < point > apply_from, 
                       vector < point > &result,
                       QString          &error_msg,
                       bool             force_zero_center = false );

      bool selected_models_contain( QString residue );

      static float mw_to_volume( float mw, float vbar );
      
      QString list_chainIDs( vector < PDB_model > &mv );

      // state copies

      vector < PDB_atom >                                 state_bead_model;
      vector < vector < PDB_atom > >                      state_bead_models;
      vector < vector < PDB_atom > >                      state_bead_models_as_loaded;
      vector < PDB_atom * >                               state_active_atoms;
      vector < struct residue >                           state_residue_list;
      vector < struct residue >                           state_residue_list_no_pbr;
      map < QString, vector < int > >                     state_multi_residue_map;
      map < QString, vector < int > >                     state_valid_atom_map;
      map < QString, QString >                            state_residue_atom_hybrid_map;
      map < QString, QString >                            state_residue_atom_abb_hybrid_map;
      map < QString, int >                                state_atom_counts;
      map < QString, int >                                state_has_OXT;
      map < QString, int >                                state_bead_exceptions;
      vector <struct residue>                             state_save_residue_list;
      vector <struct residue>                             state_save_residue_list_no_pbr;
      map < QString, vector <int> >                       state_save_multi_residue_map;
      map < QString, int >                                state_new_residues;
      map < QString, vector < QString > >                 state_molecules_residues_atoms;
      map < QString, QString >                            state_molecules_residue_name;
      vector < QString >                                  state_molecules_idx_seq;
      map < QString, vector < QString > >                 state_molecules_residue_errors;
      map < QString, vector < int > >                     state_molecules_residue_missing_counts;
      map < QString, vector < vector < QString > > >      state_molecules_residue_missing_atoms;
      map < QString, vector < vector < unsigned int > > > state_molecules_residue_missing_atoms_beads;
      map < QString, bool >                               state_molecules_residue_missing_atoms_skip;
      map < QString, int >                                state_molecules_residue_min_missing;
      map < QString, bool >                               state_broken_chain_end;
      map < QString, bool >                               state_broken_chain_head;
      map < QString, bool >                               state_unknown_residues;
      map < QString, int >                                state_use_residue;
      map < QString, bool >                               state_skip_residue;
      QString                                             state_last_abb_msgs;
      vector < PDB_model >                                state_model_vector;
      vector < PDB_model >                                state_model_vector_as_loaded;
      vector < int >                                      state_somo_processed;
      QString                                             state_options_log;
      QString                                             state_pdb_file;
      QString                                             state_project;

      QString                                             state_lbl_pdb_file;
      vector < QString >                                  state_lb_model_rows;
      unsigned int                                        state_current_model;

      void save_state();
      void restore_state();
      void clear_state();

      QStringList                       directory_history;
      map < QString, QDateTime >        directory_last_access;
      map < QString, QString >          directory_last_filetype;

      bool                     saxs_screen_widget;
      US_Hydrodyn_Saxs_Screen  *saxs_screen_window;
      csv                      last_saxs_screen_csv;

      bool                     saxs_buffer_widget;
      US_Hydrodyn_Saxs_Buffer  *saxs_buffer_window;
      csv                      last_saxs_buffer_csv;

      bool                     saxs_hplc_widget;
      US_Hydrodyn_Saxs_Hplc    *saxs_hplc_window;
      csv                      last_saxs_hplc_csv;

      bool                     dad_widget;
      US_Hydrodyn_Dad         *dad_window;
      csv                      last_dad_csv;
      
      bool                     mals_widget;
      US_Hydrodyn_Mals         *mals_window;
      csv                      last_mals_csv;
      
      bool                     mals_saxs_widget;
      US_Hydrodyn_Mals_Saxs    *mals_saxs_window;
      csv                      last_mals_saxs_csv;
      
      bool                     saxs_search_widget;
      US_Hydrodyn_Saxs_Search  *saxs_search_window;
      csv                      last_saxs_search_csv;

      bool                     saxs_1d_widget;
      US_Hydrodyn_Saxs_1d      *saxs_1d_window;

      bool                     saxs_2d_widget;
      US_Hydrodyn_Saxs_2d      *saxs_2d_window;

      double       total_volume_of_bead_model( vector < PDB_atom > &bead_model );
      unsigned int number_of_active_beads    ( vector < PDB_atom > &bead_model );
      bool         radii_all_equal           ( vector < PDB_atom > &bead_model );

      csv          pdb_to_csv  ( vector < PDB_model > &model );
      void         save_pdb_csv( csv &csv1 );

      QString      last_pdb_filename;
      QStringList  last_pdb_title;
      QStringList  last_pdb_header;
      QStringList  last_pdb_load_calc_mw_msg;
      QStringList  last_steric_clash_log;

      QString      last_bead_model;

      void show_zeno();
      bool calc_zeno();
      int do_calc_hydro();

      void         reset_chain_residues( PDB_model *);

      unsigned int multiply_iq_by_atomic_volume_last_water_multiplier;

      QString     sf_4term_notes;
      QString     sf_5term_notes;

      bool load_config_json ( QString &json );
      void hard_coded_defaults();

      QStringList msroll_radii;
      QStringList msroll_names;

      QString le_pdb_file_save_text;

      map < QString, map < QString, vector < QString > > > data_csv;
      QStringList                                          data_csv_headers;

      // grpy data & methods
      QProcess                            * grpy;
      void                                  grpy_process_next();
      void                                  grpy_finalize();
      QString                               grpy_prog;
      QStringList                           grpy_to_process;
      QStringList                           grpy_processed;
      QString                               grpy_last_processed;
      QString                               grpy_stdout;
      bool                                  grpy_running;
      QString                               grpy_filename;
      QVector < int >                       grpy_model_numbers;
      int                                   grpy_last_model_number;
      QVector < int >                       grpy_used_beads;
      QVector < QMap < QString, double > >  grpy_addl_params;
      QMap < QString, double >              grpy_addl_param;
      int                                   grpy_last_used_beads;
      bool                                  grpy_success;  // only valid if !grpy_running
      map < QString, vector < double > >    grpy_captures;

      bool                                  grpy_mm;
      QString                               grpy_mm_results;
      QString                               grpy_mm_name;
      save_info                             grpy_mm_save_params;
      bool                                  grpy_was_hydro_enabled;

      // zeno mm data
      bool                                  zeno_mm;
      QString                               zeno_mm_results;
      QString                               zeno_mm_name;
      save_info                             zeno_mm_save_params;
      

      US_Timer                              timers;

      hydro_results grpy_results;
      hydro_results grpy_results2;
      bool          grpy_vdw;

      QProcess *hullrad;
      void hullrad_process_next();
      void hullrad_finalize();
      QString hullrad_prog;
      QStringList hullrad_to_process;
      QStringList hullrad_processed;
      QString hullrad_last_processed;
      QString hullrad_stdout;
      bool hullrad_running;
      QString hullrad_filename;
      map < QString, vector < double > > hullrad_captures;

      QString get_somo_dir();

      // deprecated
      // double basic_fraction( float pH, float pKa );
      // double ionized_atom_mw( float bf, struct atom *atom );
      // double ionized_num_elect( float bf, struct atom *atom );
      // double ionized_hydrogens( float bf, struct atom *atom );
      // double ionized_residue_vbar( float bf, struct residue *res );
      // double ionized_residue_atom_mw( float bf, struct atom *atom );
      
      vector < double > basic_fractions    ( float pH, struct residue * res );
      double ionized_residue_vbar          ( vector < double > & fractions, struct residue * res );
      double ionized_residue_atom_mw       ( vector < double > & fractions, struct residue *res, struct atom *atom );
      double ionized_residue_atom_radius   ( vector < double > & fractions, struct residue *res, struct atom *atom );
      double ionized_residue_atom_hydration( vector < double > & fractions, struct residue *res, struct atom *atom );
      double ionized_residue_atom_protons  ( vector < double > & fractions, struct residue *res, struct atom *atom );
                                                                                                             
      bool dmd_failed_validation;
      map < QString, map < QString, QStringList > > dmd_all_pdb_prepare_reports;

      void clear_temp_dirs();
      
      // for vdw beads saxs excl vol
      vector < atom >                atom_list;
      map < QString, atom >          atom_map;
      void                           select_atom_file(const QString &filename);

   private slots:
      void hullrad_readFromStdout();
      void hullrad_readFromStderr();
      void hullrad_started();
      void hullrad_finished( int, QProcess::ExitStatus );
      void grpy_readFromStdout();
      void grpy_readFromStderr();
      void grpy_started();
      void grpy_finished( int, QProcess::ExitStatus );
      void gui_script_run();
      void fractal_dimension( bool from_parameters = false, save_info * fd_save_info = (save_info *)0 );
      
   public:
      void save_params_force_results_name( save_info & save );

      bool          cli_progress;
      mQProgressBar *progress;
      mQProgressBar *mprogress;

      void set_expert( bool );

      bool select_from_directory_history( QString &dir, QWidget *parent = (QWidget *)0, bool select_dir = false );
      void add_to_directory_history( QString dir, bool accessed = true );

      unsigned int current_model;
      QString project;   // name of the current project - derived from the prefix of the pdb filename
      QString bead_model_prefix;
      QString bead_model_suffix;

      QString      model_name( int val );

      bool rotamer_changed;  // toggles need for reloading rotamer file

      bool compute_structure_factors( QString filename, QString &error_msg );

      void editor_msg( QString color, QString msg );
      void editor_msg( QString color, const QFont &font, QString msg );

      saxs sf_factors;
      vector < saxs > sf_bead_factors;

      static QString fix_file_name( QString ); // remove spaces etc

      void do_update_progress( int, int );

   public slots:
      void update_progress( int, int );
      void show_zeno_options();
      void display_default_differences();
      void clear_display();
      void reload_pdb();
      int  calc_somo( bool no_ovlp_removal = false );     // compute asa and then refine bead_model
      int  calc_somo_o();   // somo model with overlaps
      int  calc_grid_pdb( bool no_ovlp_removal = false ); // compute grid model from pdb
      int  calc_vdw_beads(); // compute grid model from pdb
      int  calc_grid();     // compute grid model from bead model
      int  calc_hydro();
      bool calc_zeno_hydro();
      bool calc_grpy_hydro();

      int calc_iqq( bool bead_model, bool create_native_saxs = true, bool do_raise = true );      // bring up saxs window if needed and compute iqq curve
      int calc_prr( bool bead_model, bool create_native_saxs = true, bool do_raise = true );      // bring up saxs window if needed and compute prr curve
      void select_save_params();
      void show_saxs_options();
      void show_fractal_dimension_options();
      void show_vdw_overlap();
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
      void visualize(bool movie_frame = false, QString dir = "", float scale = 1.0, bool black_background = false, bool do_pat = false);

      void dmd_static_pairs();

      void pdb_saxs( bool create_native_saxs = true, bool do_raise = true );
      void bead_saxs( bool create_native_saxs = true, bool do_raise = true );
      int pdb_hydrate_for_saxs( bool quiet = false );
      void open_hydro_results();
      
   private slots:
      bool calc_hullrad_hydro( QString filename = "" );

      void process_events();

      void le_pdb_file_changed( const QString & );
      void le_pdb_file_focus( bool );

      void         make_test_set();

      void rescale_bead_model();
      bool equi_grid_bead_model( double dR = 0e0 );

      void browflex_readFromStdout();
      void browflex_readFromStderr();
      void browflex_started();
      void browflex_finished( int, QProcess::ExitStatus );

      void anaflex_readFromStdout();
      void anaflex_readFromStderr();
      void anaflex_started();
      void anaflex_finished( int, QProcess::ExitStatus );

      void load_pdb();
      void show_batch();
      int read_pdb(const QString &);
      int read_bead_model( QString filename, bool &only_overlap );
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
      void config();
      void edit_atom();
      void hybrid();
      void residue();
      void do_saxs();
      void select_model( int val = 0 );
      void model_selection_changed();
      int create_beads(QString *error_string, bool quiet = false); // turn pdb/atom model into bead_model
      int create_vdw_beads(QString & error_string, bool quiet = false); // turn pdb/atom model into bead_model
      void get_atom_map(PDB_model *);
      int check_for_missing_atoms(QString *error_string, PDB_model *);
      void build_molecule_maps(PDB_model *model); // sets up maps for molecule
      int overlap_check(bool sc, bool mc, bool buried, double tolerance, int limit = 0, bool from_overlap_hydro = false ); // check for overlaps
      int compute_asa(bool bd_mode = false, bool no_ovlp_removal = false); // calculate maximum accessible surface area
      void show_asa();
      void show_bd();
      void show_overlap();
      void show_grid_overlap();
      void show_bead_output();
      void show_hydro();
      void show_grid(); // show grid options
      void show_advanced_config();
      void view_pdb(); // show pdb file in editor
      void pdb_tool(); // run pdb tool on current model
      void pdb_parsing(); // pdb parsing options
      void pdb_visualization(); // pdb visualization options
      void view_asa(); // show asa file in editor
      void view_bead_model(); // show bead model file in editor
      void view_file(const QString &, QString title = "SOMO editor"); // call editor to view a file
      void bead_check( bool use_threshold = false, bool message_type = false, bool vdw = false, bool only_asa = false ); // recheck beads
      void load_config();
      void write_config();
      void write_config(const QString &);
      void reset();
      void set_default();
      void set_pH();
      void update_temperature( const QString &, bool update_hydro = true );
      void update_pH( const QString & );
      void update_bead_model_file(const QString &);
      void update_bead_model_prefix(const QString &);
      void radial_reduction( bool from_grid = false );
      void show_hydro_results();
      void write_bead_asa( QString, vector <PDB_atom> * );
      void write_bead_tsv( QString, vector <PDB_atom> * );
      void write_bead_ebf( QString, vector <PDB_atom> * );
      void write_bead_spt( QString, vector <PDB_atom> *, bool movie_frame = false, float scale = 1, bool black_background = false );
      void write_bead_model( QString, vector <PDB_atom> *, QString extra_text = "" );
      void write_bead_model( QString, vector <PDB_atom> *, int bead_model_output, QString extra_text = "" );
      void write_corr( QString, vector <PDB_atom> * );
      bool read_corr( QString, vector <PDB_atom> * );
      void printError(const QString &);
      void closeAttnt(QProcess *, QString);
      void calc_vbar(struct PDB_model *, bool use_p_atom = false );
      double calc_vbar_updated( struct PDB_model & );
      void update_vbar(); // update the results.vbar everytime something changes the vbar in options or calculation
      void append_options_log_somo(); // append somo options to options_log
      void append_options_log_somo_ovlp(); // append somo options to options_log
      void append_options_log_atob(); // append atob options to options_log
      void append_options_log_atob_ovlp(); // append atob options to options_log
      void append_options_log_misc(); // append misc vbar info
      void list_model_vector(vector < PDB_model > *);
      QString default_differences_main();
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
      void select_comparative();
      void best_analysis();

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
      point plane( point p1, point p2, point p3 );  //( a3 - a2 ) x ( a1 - a2 )
      point average( vector < point > *v );    // returns an average vector
      float dist( point p1, point p2);         // sqrt( (p1 - p2) dot (p1 - p2) )

      int compute_bd_connections();            // computes intersected pb beads within tolerance map
      int write_pdb( QString fname, vector < PDB_atom > *model );


      bool write_pdb_from_model(
                                const PDB_model & model
                                ,QString & errors
                                ,QString & writtenname
                                ,const QString & headernote = ""
                                ,const QString & suffix     = ""
                                ,const QString & filename   = ""
                                );

      // ancillary pdb utils

      int write_contact_plot( QString fname, PDB_model *model, float thresh );
      int write_contact_plot( QString fname, vector < PDB_atom > *model, float thresh );

      // anaflex functions


      int create_anaflex_files();
      int create_anaflex_files( int use_mode, int sub_mode = 0 );
      int run_anaflex();
      int run_anaflex( int use_mode, int sub_mode = 0 );

      // admin
      void run_us_config();
      void run_us_admin();

   protected slots:

      void closeEvent(QCloseEvent *);

   signals:

      void                   progress_updated( int, int );

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

  vector <PDB_atom> *p_bead_model;
  vector <bool> *p_last_reduced;
  vector <bool> *p_reduced;
  vector <BPair> *p_my_pairs;

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
