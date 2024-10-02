#ifndef US_HYDRODYN_ADDRESIDUE_H
#define US_HYDRODYN_ADDRESIDUE_H

#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qwidget.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qwidget.h>
#include <qpushbutton.h>
//#include <q3frame.h>
#include <QComboBox>
#include <qcheckbox.h>
#include <qlistwidget.h>
#include <qmessagebox.h>
#include <qpixmap.h>
#include <qwt_counter.h>
//Added by qt3to4:
#include <QCloseEvent>
#include <math.h>

#include "us.h"
#include "us_hydrodyn_pdbdefs.h"
#include "us_extern.h"
#include "us_util.h"


class US_AddResidue : public QWidget
{
   Q_OBJECT

   public:
      US_AddResidue(bool *, const double, QWidget *p=0, const char *name=0);
      ~US_AddResidue();

   private:

      US_Config *USglobal;
      US_Help *online_help;
      bool *widget_flag;
      bool existing_residue;
      bool position_flag; // 0 = not involved, 1 = determines position
      bool hydration_flag; // 0 = use atoms', 1 = use manual value
      bool pKa_flag; // 0 = not involved, 1 = determines position
      unsigned int current_atom;
      unsigned int current_bead;
      unsigned int atom_hydration;
      double hydrovol;
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
      QLabel *lbl_residue_comment;
      QLabel *lbl_numatoms;
      QLabel *lbl_numbeads;
      QLabel *lbl_type;
      QLabel *lbl_molvol;
      QLabel *lbl_vbar;
      QLabel *lbl_vbar2;
      QLabel *lbl_pKa;
      QLabel *lbl_asa;
      QLabel *lbl_beadchain;
      QLabel *lbl_bead_volume;
      QLabel *lbl_bead_mw;
      QLabel *lbl_bead_hydro_from_atom;
      QLabel *lbl_bead_hydrovol;
      QLabel *lbl_bead_hydrorad;

      QLabel *lbl_info2;
      QLabel *lbl_r_atoms;
      QLabel *lbl_define_atom;
      QLabel *lbl_define_hybrid;
      QLabel *lbl_define_hybrid2;
      QLabel *lbl_atom_hydration;
      QLabel *lbl_atom_hydration2;

      QLabel *lbl_info3;
      QLabel *lbl_r_beads;
      QLabel *lbl_bead_color;
      QLabel *lbl_hydration;
      QLabel *lbl_enable_pKa;
      QLabel *lbl_select_atom;
      QLabel *lbl_placing;
      QLabel *lbl_select_beadatom;
      QLabel *lbl_list_beadatom;
      QLabel *lbl_positioning;

      QComboBox *cmb_type;
      QComboBox *cmb_r_atoms;
      QComboBox *cmb_atoms;
      QComboBox *cmb_hybrids;
      QComboBox *cmb_hybrids2;
      QComboBox *cmb_r_beads;
      QComboBox *cmb_bead_color;
      QComboBox *cmb_placing;

      QCheckBox *cb_positioning;
      QCheckBox *cb_hydration;
      QCheckBox *cb_enable_pKa;

      QRadioButton *rb_sidechain;
      QRadioButton *rb_backbone;
      QGroupBox *bg_chain;

      QListWidget *lb_residues;
      QListWidget *lb_select_beadatom;
      QListWidget *lb_list_beadatom;

      QwtCounter *cnt_atom_hydration;
      QwtCounter *cnt_atom_hydration2;
      QwtCounter *cnt_hydration;
      QwtCounter *cnt_numbeads;
      QwtCounter *cnt_numatoms;

      QLineEdit *le_molvol;
      QLineEdit *le_vbar;
      QLineEdit *le_vbar2;
      QLineEdit *le_pKa;
      QLineEdit *le_asa;
      QLineEdit *le_residue_name;
      QLineEdit *le_residue_comment;
      QLineEdit *le_bead_volume;
      QLineEdit *le_bead_mw;
      QLineEdit *le_bead_hydro_from_atom;
      QLineEdit *le_bead_hydrovol;
      QLineEdit *le_bead_hydrorad;
      struct residue new_residue;
      struct atom new_atom;
      struct bead new_bead;
      vector <struct residue> residue_list;
      vector <struct atom> atom_list;
      vector <struct bead> bead_list;
      QString atom_filename, residue_filename;
      bool update_pKas( int atomno );
      void info_residue( const QString & msg );
      void update_bead_hydrated_info();
      void clear_bead_hydrated_info();

      bool enable_assign_hydration;

   private slots:
      void add();
      void reset( bool reselect = true );
      void help();
      void select_atom_file();
      void select_residue_file();
      void read_residue_file(const QString &);
      void select_residue(int);
      void residue_selection_changed();
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
      void update_comment(const QString &);
      void update_molvol(const QString &);
      void update_vbar(const QString &);
      // void update_vbar2(const QString &);
      void update_pKa(const QString &);
      void update_asa(const QString &);
      void update_bead_volume(const QString &);
      void update_atom_hydration(double);
      void update_atom_hydration2(double);
      void update_hydration(double);
      void update_numatoms(double);
      void update_numbeads(double);
      void update_hybrid(int);
      void set_positioning();
      void set_hydration();
      void set_chain();
      void set_enable_pKa();
      void set_chain(int);
      void atom_continue();
      void write_residue_file();
      void calc_bead_mw(struct residue *);
      void enable_area_1(bool);
      void enable_area_2(bool);
      void enable_area_3(bool);

   protected slots:
      void closeEvent(QCloseEvent *);
};
#endif
