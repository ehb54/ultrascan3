#ifndef US_HYDRODYN_ADDATOM_H
#define US_HYDRODYN_ADDATOM_H

#include "us_hydrodyn_pdbdefs.h"
#include "us_hydrodyn_addhybrid.h"
#include "us.h"
#include "us_extern.h"
#include "us_util.h"

#include <qwidget.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qwidget.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qframe.h>

class US_AddAtom : public QWidget
{
	Q_OBJECT

	public:
		US_AddAtom(bool *, QWidget *p=0, const char *name=0);
		~US_AddAtom();

	private:

		US_Config *USglobal;
		bool *widget_flag;

		QPushButton *pb_add;
		QPushButton *pb_close;
		QPushButton *pb_delete;
		QPushButton *pb_select_atom_file;
		QPushButton *pb_select_hybrid_file;

		QLabel *lbl_info;
		QLabel *lbl_hybrid_table;
		QLabel *lbl_atom_table;
		QLabel *lbl_name;
		QLabel *lbl_hybrid1;
		QLabel *lbl_hybrid2;
		QLabel *lbl_chain;
		QLabel *lbl_mw1;
		QLabel *lbl_mw2;
		QLabel *lbl_radius1;
		QLabel *lbl_radius2;
		QLabel *lbl_number_of_atoms;
		QLabel *lbl_number_of_hybrids;

		QComboBox *cmb_atom;
		QComboBox *cmb_hybrid;
		QComboBox *cmb_chain;
		QLineEdit *le_name;
		struct atom current_atom;
		struct hybridization current_hybrid;
		vector <struct atom> atom_list;
		vector <struct hybridization> hybrid_list;
		QString atom_filename;
		QString hybrid_filename;

	private slots:
		void add();
		void select_atom_file();
		void select_hybrid_file();
		void setupGUI();
		void update_name(const QString &);
		void update_hybridization_name(const QString &);
		void select_hybrid(int);
		void select_atom(int);
		void select_chain(int);
		void delete_atom();
		void write_atom_file();
		void sort_atoms();

	protected slots:
		void closeEvent(QCloseEvent *);
};

#endif
