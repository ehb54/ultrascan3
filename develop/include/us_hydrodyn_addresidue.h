#ifndef US_HYDRODYN_ADDRESIDUE_H
#define US_HYDRODYN_ADDRESIDUE_H

#include "us_hydrodyn_pdbdefs.h"
#include "us.h"
#include "us_extern.h"
#include "us_util.h"

#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qwidget.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qwidget.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlistbox.h>
#include <qmessagebox.h>
#include <qpixmap.h>
#include <qwt_counter.h>

class US_AddResidue : public QWidget
{
	Q_OBJECT

	public:
		US_AddResidue(bool *, QWidget *p=0, const char *name=0);
		~US_AddResidue();

	private:

		US_Config *USglobal;
		US_Help *online_help;
		bool *widget_flag;
		bool existing_residue;
		bool position_flag; // 0 = not involved, 1 = determines position
		unsigned int current_atom;
		unsigned int current_bead;
		vector <QString> hybrids;
		vector <QString> atoms;

		QPushButton *pb_add;
		QPushButton *pb_accept_residue;
		QPushButton *pb_accept_atom;
		QPushButton *pb_atom_continue;
		QPushButton *pb_accept_bead;
		QPushButton *pb_close;
		QPushButton *pb_help;
		QPushButton *pb_reset;
		QPushButton *pb_select_residue_file;
		QPushButton *pb_select_atom_file;
		QPushButton *pb_delete_residue;

		QLabel *lbl_info1;
		QLabel *lbl_atom_file;
		QLabel *lbl_residue_file;
		QLabel *lbl_numresidues;
		QLabel *lbl_residue_name;
		QLabel *lbl_numatoms;
		QLabel *lbl_numbeads;
		QLabel *lbl_type;
		QLabel *lbl_molvol;
		QLabel *lbl_asa;
		QLabel *lbl_beadchain;
		QLabel *lbl_bead_volume;

		QLabel *lbl_info2;
		QLabel *lbl_r_atoms;
		QLabel *lbl_define_atom;
		QLabel *lbl_define_hybrid;

		QLabel *lbl_info3;
		QLabel *lbl_r_beads;
		QLabel *lbl_bead_color;
		QLabel *lbl_hydration;
		QLabel *lbl_select_atom;
		QLabel *lbl_placing;
		QLabel *lbl_select_beadatom;
		QLabel *lbl_list_beadatom;
		QLabel *lbl_positioning;

		QComboBox *cmb_type;
		QComboBox *cmb_r_atoms;
		QComboBox *cmb_atoms;
		QComboBox *cmb_hybrids;
		QComboBox *cmb_r_beads;
		QComboBox *cmb_bead_color;
		QComboBox *cmb_placing;

		QCheckBox *cb_positioning;

		QRadioButton *rb_sidechain;
		QRadioButton *rb_backbone;
		QButtonGroup *bg_chain;

		QListBox *lb_residues;
		QListBox *lb_select_beadatom;
		QListBox *lb_list_beadatom;

		QwtCounter *cnt_hydration;
		QwtCounter *cnt_numbeads;
		QwtCounter *cnt_numatoms;

		QLineEdit *le_molvol;
		QLineEdit *le_asa;
		QLineEdit *le_residue_name;
		QLineEdit *le_bead_volume;
		struct residue new_residue;
		struct atom new_atom;
		struct bead new_bead;
		vector <struct residue> residue_list;
		vector <struct atom> atom_list;
		vector <struct bead> bead_list;
		QString atom_filename, residue_filename;


	private slots:
		void add();
		void reset();
		void help();
		void select_atom_file();
		void select_residue_file();
		void select_residue(int);
		void select_r_atom(int);
		void select_r_bead(int);
		void select_bead_color(int);
		void select_placing_method(int);
		void select_type(int);
		void select_beadatom();
		void delete_residue();
		void print_residue(struct residue);
		void setupGUI();
		void accept_bead();
		void accept_atom();
		void accept_residue();
		void update_name(const QString &);
		void update_molvol(const QString &);
		void update_asa(const QString &);
		void update_bead_volume(const QString &);
		void update_hydration(double);
		void update_numatoms(double);
		void update_numbeads(double);
		void update_hybrid(int);
		void set_positioning();
		void set_chain(int);
		void atom_continue();
		void write_residue_file();

	protected slots:
		void closeEvent(QCloseEvent *);
};
#endif
