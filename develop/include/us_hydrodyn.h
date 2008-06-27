#ifndef US_HYDRODYN_H
#define US_HYDRODYN_H

// QT defs:

#include <qfile.h>
#include <qtextstream.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qframe.h>
#include <qtabwidget.h>

#include "us.h"
#include "us_extern.h"
#include "us_util.h"
#include "us_hydrodyn_pdbdefs.h"
#include "us_hydrodyn_addatom.h"
#include "us_hydrodyn_addresidue.h"
#include "us_hydrodyn_overlap_reduction.h"
#include "us_db_tbl_vbar.h"

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
		bool residue_widget, atom_widget, hybrid_widget, recheck_beads, asa_calculation;
		unsigned int current_model;
		int output; 				// 0 = SOMO
										// 1 = BEAMS
										// 2 = HYDRO
		int sequence;				// 0 = as in original pdb file
										// 1 = exposed sidechain -> exposed main chain -> buried
										// 2 = include bead-original residue correspondence
		bool compute_vbar;		// true = compute
		 								// false = use user specified value
		QString residue_filename, output_filename;
		struct residue new_residue;
		struct atom new_atom;
		struct bead new_bead;
		struct overlap_reduction sidechain_overlap;
		struct overlap_reduction mainchain_overlap;
		struct overlap_reduction buried_overlap;
		US_Hydrodyn_OR *sidechain_OR;
		US_Hydrodyn_OR *mainchain_OR;
		US_Hydrodyn_OR *buried_OR;
		US_Vbar_DB *vbar_dlg;
		double probe_radius, asa_threshold, asa_threshold_percent, vbar, hydrovol, overlap_tolerance;

		US_Config *USglobal;

		QLabel *lbl_info;
		QLabel *lbl_table;
		QLabel *lbl_tabletabs;
		QLabel *lbl_output;
		QLabel *lbl_sequence_format;
		QLabel *lbl_pdb_file;
		QLabel *lbl_model;
		QLabel *lbl_probe_radius;
		QLabel *lbl_asa_threshold;
		QLabel *lbl_asa_threshold_percent;
		QLabel *lbl_hydrovol;
		QLabel *lbl_overlap_tolerance;

		QLineEdit *le_vbar;
		QLineEdit *le_output_file;

		QCheckBox *cb_asa_calculation;
		QCheckBox *cb_bead_check;
		QCheckBox *cb_somo_output;
		QCheckBox *cb_beams_output;
		QCheckBox *cb_hydro_output;
		QCheckBox *cb_pdb_sequence;
		QCheckBox *cb_chain_sequence;
		QCheckBox *cb_correspondence_sequence;
		QCheckBox *cb_vbar;

		QButtonGroup *bg_output;
		QButtonGroup *bg_sequence;

		QPushButton *pb_save;
		QPushButton *pb_reset;
		QPushButton *pb_vbar;
		QPushButton *pb_select_residue_file;
		QPushButton *pb_select_output_file;
		QPushButton *pb_load_pdb;
		QPushButton *pb_atom;
		QPushButton *pb_residue;
		QPushButton *pb_hybrid;
		QPushButton *pb_help;
		QPushButton *pb_cancel;
		QPushButton *pb_somo;
		QPushButton *pb_visualize;

		QListBox *lb_model;

		QTabWidget *tw_overlap;

		US_AddAtom *addAtom;
		US_AddResidue *addResidue;
		US_AddHybridization *addHybrid;

		QwtCounter *cnt_probe_radius;
		QwtCounter *cnt_asa_threshold;
		QwtCounter *cnt_asa_threshold_percent;
		QwtCounter *cnt_hydrovol;
		QwtCounter *cnt_overlap_tolerance;

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
		void read_pdb(const QString &);
		void setupGUI();
		void select_residue_file();
		void select_output_file();
		void clear_temp_chain(struct PDB_chain *);
	   void assign_atom(const QString &, struct PDB_chain *);
		void cancel();
		void help();
		void atom();
		void hybrid();
		void residue();
		void select_model(int);
		void select_output(int);
		void select_sequence(int);
		void calc_bead_mw(struct residue *); // calculate the molecular weight of all beads in residue
		void update_probe_radius(double);
		void update_asa_threshold(double);
		void update_asa_threshold_percent(double);
		void update_hydrovol(double);
		void update_overlap_tolerance(double);
		void set_asa_calculation();
		void set_bead_check();
		int calc_somo(); // build bead model
		int compute_asa(); // calculate maximum accessible surface area
		void bead_check(); // recheck beads
		void read_config();
		void write_config();
		void reset();
		void visualize();
		void set_vbar();
		void select_vbar();
		void update_vbar(const QString &);
		void update_output_file(const QString &);
		void update_vbar_signal(float, float);

	protected slots:

		void closeEvent(QCloseEvent *);
};

#endif

