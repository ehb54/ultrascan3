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
		bool residue_widget, atom_widget, hybrid_widget;

		US_Config *USglobal;
		
		QLabel *lbl_info;
		QLabel *lbl_table;
		QLabel *lbl_tabletabs;
		QLabel *lbl_pdb_file;
		
		QPushButton *pb_save;
		QPushButton *pb_select_residue_file;
		QPushButton *pb_load_pdb;
		QPushButton *pb_atom;
		QPushButton *pb_residue;
		QPushButton *pb_hybrid;
		QPushButton *pb_help;
		QPushButton *pb_cancel;
		
		QTabWidget *Tab;
		
		US_AddAtom *addAtom;
		US_AddResidue *addResidue;
		US_AddHybridization *addHybrid;
		
#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

		vector <struct PDB_model> model_vector;

		int compute_asa();

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

	private slots:
		void load_pdb();
		void read_pdb(const QString &);
		void setupGUI();
		void select_residue_file();
		void read_table(const QString &);
	   void clear_temp_chain(struct PDB_chain *);
	   void assign_atom(const QString &, struct PDB_chain *);
		void cancel();
		void help();
		void atom();
		void hybrid();
		void residue();

	protected slots:
	
		void closeEvent(QCloseEvent *);		
};

#endif

