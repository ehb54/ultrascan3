#ifndef US_MODEL_EDITOR_H
#define US_MODEL_EDITOR_H

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
#include "us_db_tbl_vbar.h"

using namespace std;

class US_ModelEditor : public QDialog
{
	Q_OBJECT
	
	public:
		US_ModelEditor(bool, struct ModelSystem *, QWidget *parent=0, const char *name=0);
		~US_ModelEditor();
		US_Config *USglobal;
		struct ModelSystem *system;
		unsigned int current_component, shape;
		struct hydrosim simcomp;
		QString c0_file;

		QwtCounter *cnt_item;

		QComboBox *cmb_component1;
		QComboBox *cmb_component2;

		QLineEdit *le_conc;
		QLineEdit *le_sed;
		QLineEdit *le_diff;
		QLineEdit *le_koff;
		QLineEdit *le_keq;
		QLineEdit *le_mw;
		QLineEdit *le_vbar;
		QLineEdit *le_f_f0;
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
		QLabel *lbl_f_f0;
		QLabel *lbl_mw;
		QLabel *lbl_stoich;
		QLabel *lbl_msg1;
		QLabel *lbl_load_c0;
		
		QPushButton *pb_simulateComponent;
		QPushButton *pb_cancel;
		QPushButton *pb_next;
		QPushButton *pb_help;
		QPushButton *pb_save;
		QPushButton *pb_vbar;
		QPushButton *pb_accept;
		QPushButton *pb_load_c0;
		QPushButton *pb_load_model;

		QCheckBox *cb_rod, *cb_oblate, *cb_sphere, *cb_prolate;

	protected slots:
		void closeEvent(QCloseEvent *e);
	
	public slots:
		void simulate_component();
		virtual void update_component();
		void select_component(double);
		void select_component(int);
		void update_conc(const QString &);
		virtual void update_sed(const QString &);
		virtual void update_diff(const QString &);
		void update_koff(const QString &);
		void update_keq(const QString &);
		void update_sigma(const QString &);
		void update_delta(const QString &);
		virtual void update_mw(const QString &);
		virtual void update_f_f0(const QString &);
		void savefile(const QString &);
		void savefile();
		void select_rod();
		void select_prolate();
		void select_oblate();
		void select_sphere();
		virtual void setup_GUI();
		virtual void help();
		void load_c0();
		void load_model();
		void load_model(const QString &);
		void get_vbar(const QString &);
		void get_vbar();
		void printError(const int &);
		void save_model();
		void accept_model();
		void update_vbar(float, float);
		virtual bool verify_model();

	signals:
		void componentChanged(unsigned int);
};

#endif
