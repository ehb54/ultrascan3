#ifndef US_HYDRODYN_ADDATOM_H
#define US_HYDRODYN_ADDATOM_H

#include "us_hydrodyn_pdbdefs.h"
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
		QPushButton *pb_select_file;

		QLabel *lbl_info;
		QLabel *lbl_table;
		QLabel *lbl_mw;
		QLabel *lbl_radius;
		QLabel *lbl_name;
		QLabel *lbl_number_of_atoms;
		QLabel *lbl_chain;

		QComboBox *cmb_atoms;
		QComboBox *cmb_chain;
		
		QLineEdit *le_name;
		QLineEdit *le_mw;
		QLineEdit *le_radius;
		struct atom new_atom;
		vector <struct atom> atom_list;
		QString atom_filename;
		
	private slots:
		void add();
		void select_file();
		void setupGUI();
		void update_name(const QString &);
		void update_mw(const QString &);
		void update_radius(const QString &);
		void select_atom(int);
		void select_chain(int);

	protected slots:
		void closeEvent(QCloseEvent *);
};

#endif
