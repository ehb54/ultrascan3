#ifndef US_HYDRODYN_H
#define US_HYDRODYN_H

// QT defs:

#include <qfile.h>
#include <qtextstream.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qframe.h>
#include <qprogressbar.h>

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

//standard C and C++ defs:

#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
using namespace std;

class US_EXTERN US_Hydrodyn : public QFrame
{
	Q_OBJECT

	public:
		US_Hydrodyn(QWidget *p = 0, const char *name = 0);
		~US_Hydrodyn();

	private:
		bool 	residue_widget,
				atom_widget,
				hybrid_widget,
				asa_widget,
				overlap_widget,
				bead_output_widget,
				hydro_widget,
				misc_widget;
		unsigned int current_model;
		QString residue_filename, bead_model_file;
		struct asa_options asa;
		struct misc_options misc;
		struct hydro_options hydro;
		struct bead_output_options bead_output;
		struct residue new_residue;
		struct atom new_atom;
		struct bead new_bead;
		struct overlap_reduction sidechain_overlap;
		struct overlap_reduction mainchain_overlap;
		struct overlap_reduction buried_overlap;
		double overlap_tolerance;

		vector <PDB_atom> bead_model;

		US_Config *USglobal;

		QLabel *lbl_info;
		QLabel *lbl_pdb_file;
		QLabel *lbl_model;
		QLabel *lbl_table;
		QLabel *lbl_tabletabs;
		QLabel *lbl_options;
		QLabel *lbl_somo;

		QLineEdit *le_bead_model_file;

		QPushButton *pb_save;
		QPushButton *pb_reset;
		QPushButton *pb_select_residue_file;
		QPushButton *pb_load_pdb;
		QPushButton *pb_atom;
		QPushButton *pb_residue;
		QPushButton *pb_hybrid;
		QPushButton *pb_help;
		QPushButton *pb_cancel;
		QPushButton *pb_somo;
		QPushButton *pb_visualize;
		QPushButton *pb_show_asa;
		QPushButton *pb_show_overlap;
		QPushButton *pb_show_bead_output;
		QPushButton *pb_show_hydro;
		QPushButton *pb_show_misc;
		QPushButton *pb_load_bead_model;
		QPushButton *pb_calc_hydro;
		QPushButton *pb_show_hydro_results;

		QProgressBar *progress;

		QListBox *lb_model;

		US_AddAtom *addAtom;
		US_AddResidue *addResidue;
		US_AddHybridization *addHybrid;
		US_Hydrodyn_ASA *asa_window;
		US_Hydrodyn_Overlap *overlap_window;
		US_Hydrodyn_Bead_Output *bead_output_window;
		US_Hydrodyn_Hydro *hydro_window;
		US_Hydrodyn_Misc *misc_window;

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

		vector <struct residue>   residue_list;
		vector <struct PDB_model> model_vector;

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

	private slots:
		void load_pdb();
		void load_bead_model();
		void read_pdb(const QString &);
		void setupGUI();
		void select_residue_file();
		void clear_temp_chain(struct PDB_chain *);
	   void assign_atom(const QString &, struct PDB_chain *);
		void cancel();
		void help();
		void atom();
		void hybrid();
		void residue();
		void select_model(int);
		void calc_bead_mw(struct residue *); // calculate the molecular weight of all beads in residue
		int calc_somo(); // build bead model
		int compute_asa(); // calculate maximum accessible surface area
		void show_asa();
		void show_overlap();
		void show_bead_output();
		void show_hydro();
		void show_misc();
		void bead_check(); // recheck beads
		void read_config();
		void write_config();
		void reset();
		void visualize();
		void update_bead_model_file(const QString &);
		void calc_hydro();
		void show_hydro_results();
		bool create_beads_normally;             // true = normal, false = atoms are beads
		void write_bead_tsv(QString, vector <PDB_atom> *);
		void write_bead_ebf(QString, vector <PDB_atom> *);
		void write_bead_spt(QString, vector <PDB_atom> *);

	protected slots:

		void closeEvent(QCloseEvent *);
};

#endif
