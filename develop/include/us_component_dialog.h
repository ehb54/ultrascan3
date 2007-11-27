#ifndef US_COMPONENT_DIALOG_H
#define US_COMPONENT_DIALOG_H

#include <iostream>
#include <vector>

#include <qframe.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qdialog.h>
#include <qlineedit.h>
#include <qfile.h>
#include <qfiledlg.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qwt_counter.h>

#include "us_util.h"
#include "us_hydro.h"
#include "us_femglobal.h"

using namespace std;

class US_Component_Dialog : public QDialog
{
	Q_OBJECT
	
	public:
		US_Component_Dialog(struct ModelSystem *, QWidget *parent=0, const char *name=0);
		~US_Component_Dialog();
		US_Config *USglobal;
		struct ModelSystem *system;
		unsigned int current_component, shape;
		struct hydrosim simcomp;
		QString c0_file;

	private:
	
		QwtCounter *cnt_item;

		QComboBox *cmb_component1;
		QComboBox *cmb_component2;

		QLineEdit *le_conc;
		QLineEdit *le_sed;
		QLineEdit *le_diff;
		QLineEdit *le_koff;
		QLineEdit *le_keq;
		QLineEdit *le_sigma;
		QLineEdit *le_delta;
		QLineEdit *le_stoich;
		
		QLabel *lbl_current;
		QLabel *lbl_model;
		QLabel *lbl_linked;
		QLabel *lbl_sed;
		QLabel *lbl_diff;
		QLabel *lbl_conc;
		QLabel *lbl_sigma;
		QLabel *lbl_delta;
		QLabel *lbl_keq;
		QLabel *lbl_koff;
		QLabel *lbl_vbar1;
		QLabel *lbl_vbar2;
		QLabel *lbl_f_f01;
		QLabel *lbl_f_f02;
		QLabel *lbl_mw1;
		QLabel *lbl_mw2;
		QLabel *lbl_stoich;
		QLabel *lbl_msg1;
		QLabel *lbl_load_c0;
		
		QPushButton *pb_simulateComponent;
		QPushButton *pb_cancel;
		QPushButton *pb_next;
		QPushButton *pb_help;
		QPushButton *pb_save;
		QPushButton *pb_accept;
		QPushButton *pb_load_c0;
		QPushButton *pb_load_model;

		QCheckBox *cb_rod, *cb_oblate, *cb_sphere, *cb_prolate;

	protected slots:
		void closeEvent(QCloseEvent *e);
	
	private slots:
		void simulate_component();
		void update_component();
		void select_component(double);
		void select_component(int);
		void update_conc(const QString &);
		void update_sed(const QString &);
		void update_diff(const QString &);
		void update_koff(const QString &);
		void update_keq(const QString &);
		void update_sigma(const QString &);
		void update_delta(const QString &);
		void savefile(const QString &);
		void savefile();
		void select_rod();
		void select_prolate();
		void select_oblate();
		void select_sphere();
		void setup_GUI();
		void help();
		void load_c0();
		void load_model();
		void load_model(const QString &);
		void printError(const int &);
		void save_model();
		void accept_model();
		bool verify_model();
};

#endif
