#ifndef US_HYDRODYN_ADDATOM_H
#define US_HYDRODYN_ADDATOM_H

#include <qwidget.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qwidget.h>
#include <QComboBox>
#include <qcheckbox.h>
#include <qpushbutton.h>
//#include <q3frame.h>
//Added by qt3to4:
#include <QCloseEvent>

#include "us.h"
#include "us_hydrodyn_pdbdefs.h"
#include "us_hydrodyn_addhybrid.h"
#include "us_extern.h"
#include "us_util.h"


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
      QPushButton *pb_help;
      QPushButton *pb_delete;
      QPushButton *pb_select_atom_file;
      QPushButton *pb_select_hybrid_file;
      QPushButton *pb_select_saxs_file;

      QLabel *lbl_info;
      QLabel *lbl_hybrid_table;
      QLabel *lbl_atom_table;
      QLabel *lbl_saxs_table;
      QLabel *lbl_name;
      QLabel *lbl_saxs_name1;
      QLabel *lbl_saxs_name2;
      QLabel *lbl_saxs_excl_vol1;
      QLabel *lbl_saxs_excl_vol2;
      QLabel *lbl_hybrid1;
      QLabel *lbl_hybrid2;
      QLabel *lbl_mw1;
      QLabel *lbl_mw2;
      QLabel *lbl_radius1;
      QLabel *lbl_radius2;
      QLabel *lbl_number_of_atoms;
      QLabel *lbl_number_of_hybrids;

      QComboBox *cmb_atom;
      QComboBox *cmb_hybrid;
      QCheckBox *cb_excl_vol;
      QLineEdit *le_name;
      QLineEdit *le_excl_vol;
      struct atom current_atom;
      struct hybridization current_hybrid;
      struct saxs current_saxs;
      vector <struct atom> atom_list;
      vector <struct hybridization> hybrid_list;
      vector <struct saxs> saxs_list;
      QString atom_filename;
      QString hybrid_filename;
      QString saxs_filename;

   private slots:
      void add();
      void help();
      void select_atom_file();
      void select_hybrid_file();
      void select_saxs_file();
      void setupGUI();
      void update_name(const QString &);
      void update_excl_vol(const QString &);
      void update_hybridization_name(const QString &);
      void select_hybrid(int);
      void select_atom(int);
      void delete_atom();
      void write_atom_file();
      void sort_atoms();
      void set_excl_vol();

   protected slots:
      void closeEvent(QCloseEvent *);
};

#endif
