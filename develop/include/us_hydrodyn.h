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
#include "us_hydrodyn_overlap.h"
#include "us_hydrodyn_bead_output.h"
#include "us_hydrodyn_asa.h"
#include "us_hydrodyn_hydro.h"
#include "us_hydrodyn_misc.h"
#include "us_hydrodyn_grid.h"
#include "us_hydrodyn_results.h"
#include "us_hydrodyn_pdb_visualization.h"
#include "us_hydrodyn_pdb_parsing.h"

//standard C and C++ defs:

#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <map>

#define START_RASMOL
using namespace std;

class US_EXTERN US_Hydrodyn : public QFrame
{
	Q_OBJECT

	public:
		US_Hydrodyn(QWidget *p = 0, const char *name = 0);
		~US_Hydrodyn();
		int get_color(PDB_atom *);
		struct misc_options misc;
		double overlap_tolerance;
		bool stopFlag;

	private:
		bool 	residue_widget,
				atom_widget,
				hybrid_widget,
				asa_widget,
				overlap_widget,
				bead_output_widget,
				grid_widget,
				hydro_widget,
				results_widget,
				misc_widget,
				pdb_parsing_widget,
				pdb_visualization_widget;
		QMenuBar *m;
		QPrinter printer;
		QFont ft;

		bool create_beads_normally; // true = normal, false = atoms are beads
		bool regular_N_handling; // true = PB methed, false = MC method
		unsigned int current_model;
		QString residue_filename, bead_model_file;
		struct residue current_residue;
		struct asa_options asa;
		struct pdb_visualization pdb_vis;
		struct pdb_parsing pdb_parse;
		struct hydro_options hydro;
		struct grid_options grid;
		struct hydro_results results;
		struct bead_output_options bead_output;
		struct residue new_residue;
		struct atom new_atom;
		struct bead new_bead;
		struct overlap_reduction sidechain_overlap;
		struct overlap_reduction mainchain_overlap;
		struct overlap_reduction buried_overlap;
		struct overlap_reduction grid_overlap;
		QString project;	// name of the current project - derived from the prefix of the pdb filename
		QString bead_model_prefix;
		QString somo_dir;
		QString somo_pdb_dir;
		QString somo_tmp_dir;

		point last_molecular_cog;

		US_Config *USglobal;

		QLabel *lbl_info;
		QLabel *lbl_pdb_file;
		QLabel *lbl_model;
		QLabel *lbl_table;
		QLabel *lbl_somo;
		QLabel *lbl_bead_model_prefix;

		QPopupMenu *lookup_tables;
		QPopupMenu *somo_options;
		QPopupMenu *pdb_options;
		QPopupMenu *configuration;
		QMenuBar *menu;
		
		QLineEdit *le_bead_model_file;
		QLineEdit *le_bead_model_prefix;

		QPushButton *pb_save;
		QPushButton *pb_select_residue_file;
		QPushButton *pb_load_pdb;
		QPushButton *pb_help;
		QPushButton *pb_cancel;
		QPushButton *pb_somo;
		QPushButton *pb_visualize;
		QPushButton *pb_view_pdb;
		QPushButton *pb_load_bead_model;
		QPushButton *pb_calc_hydro;
		QPushButton *pb_show_hydro_results;
		QPushButton *pb_grid;
		QPushButton *pb_view_asa;
		QPushButton *pb_stop_calc;


		QProgressBar *progress;
		QTextEdit *editor;
		TextEdit *e;

		QListBox *lb_model;

		US_AddAtom *addAtom;
		US_AddResidue *addResidue;
		US_AddHybridization *addHybrid;
		US_Hydrodyn_ASA *asa_window;
		US_Hydrodyn_Overlap *overlap_window;
		US_Hydrodyn_Bead_Output *bead_output_window;
		US_Hydrodyn_Hydro *hydro_window;
		US_Hydrodyn_Misc *misc_window;
		US_Hydrodyn_Results *results_window;
		US_Hydrodyn_Grid *grid_window;
		US_Hydrodyn_PDB_Visualization *pdb_visualization_window;
		US_Hydrodyn_PDB_Parsing *pdb_parsing_window;
		QProcess *rasmol;

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

		vector <PDB_atom>        bead_model;
		vector < vector <PDB_atom> >  bead_models;
		vector <PDB_atom *>      active_atoms;
		vector <struct residue>   residue_list;
		map < QString, vector <int> > multi_residue_map; // maps residue to index of residue_list
		map < QString, vector <int> > valid_atom_map; // maps resName|atomName|pos 
		//                                               in multi_residue_map to index of atoms
		map < QString, int > atom_counts;
		map < QString, int > has_OXT;
		map < QString, int > bead_exceptions; // 1 == ok, 2 == skip, 3 == use automatic bead builder
		vector <struct PDB_model> model_vector;
		bool bead_model_from_file;
		vector <int> somo_processed;
		QString options_log;

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

	private slots:
		void load_pdb();
		void load_bead_model();
		void read_pdb(const QString &);
		int read_bead_model(QString);
		void setupGUI();
		void select_residue_file();
		void read_residue_file();
		void clear_temp_chain(struct PDB_chain *);
		// The next function returns a boolean to indicate whether the atom that was just assigned belongs
		// to a new residue or is part of a previously assigned residue
		// true: new residue sequence number, false: still the same
		// the integer argument contains the last sequence number (last_resSeq)
		bool assign_atom(const QString &, struct PDB_chain *, QString *);
		void cancel();
		void help();
		void atom();
		void hybrid();
		void residue();
		void select_model(int);
		void calc_bead_mw(struct residue *); // calculate the molecular weight of all beads in residue
		int calc_somo();    // compute asa and then refine bead_model
		int calc_grid();    // compute grid model
		int create_beads(QString *error_string); // turn pdb/atom model into bead_model
		void get_atom_map(PDB_model *);
		int check_for_missing_atoms(QString *error_string, PDB_model *);
		int overlap_check(bool sc, bool mc, bool buried); // check for overlaps
		int compute_asa(); // calculate maximum accessible surface area
		void show_asa();
		void show_overlap();
		void show_bead_output();
		void show_hydro();
		void show_misc();
		void show_grid(); // show grid options
		void view_pdb(); // show pdb file in editor
		void pdb_parsing(); // pdb parsing options
		void pdb_visualization(); // pdb visualization options
		void view_asa(); // show asa file in editor
		void stop_calc(); // stop some operations
		void view_file(const QString &); // call editor to view a file
		void bead_check(); // recheck beads
		void read_config(const QString &);
		void load_config();
		void write_config();
		void write_config(const QString &);
		void reset();
		void visualize();
		void update_bead_model_file(const QString &);
		void update_bead_model_prefix(const QString &);
		void calc_hydro();
		void radial_reduction();
		void show_hydro_results();
		void write_bead_asa(QString, vector <PDB_atom> *);
		void write_bead_tsv(QString, vector <PDB_atom> *);
		void write_bead_ebf(QString, vector <PDB_atom> *);
		void write_bead_spt(QString, vector <PDB_atom> *);
		void write_bead_model(QString, vector <PDB_atom> *);
		void printError(const QString &);
		void closeAttnt(QProcess *, QString);
		void calc_vbar(struct PDB_model *);
		void update_vbar(); // update the results.vbar everytime something changes the vbar in options or calculation
		void append_options_log_somo(); // append somo options to options_log 
		void append_options_log_atob(); // append atob options to options_log 

		// editor functions:
		void save();
		void print();
		void update_font();
		void clear_display();



	protected slots:

		void closeEvent(QCloseEvent *);
};

#endif
