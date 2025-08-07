//! \file us_constraints_edit.h
#ifndef US_CONSTRAINTS_EDIT_H
#define US_CONSTRAINTS_EDIT_H

#include "us_analyte_gui.h"
#include "us_buffer.h"
#include "us_dmga_constr.h"
#include "us_extern.h"
#include "us_help.h"
#include "us_model.h"
#include "us_predict1.h"
#include "us_widgets_dialog.h"

#define O_CONSTRAINTS US_dmGA_Constraints
#define C_CONSTRAINT US_dmGA_Constraints::Constraint
#define C_ATYPE US_dmGA_Constraints::AttribType
#define C_ATYPE_VBAR US_dmGA_Constraints::ATYPE_VBAR
#define C_ATYPE_MW US_dmGA_Constraints::ATYPE_MW
#define C_ATYPE_FF0 US_dmGA_Constraints::ATYPE_FF0
#define C_ATYPE_S US_dmGA_Constraints::ATYPE_S
#define C_ATYPE_D US_dmGA_Constraints::ATYPE_D
#define C_ATYPE_F US_dmGA_Constraints::ATYPE_F
#define C_ATYPE_CONC US_dmGA_Constraints::ATYPE_CONC
#define C_ATYPE_EXT US_dmGA_Constraints::ATYPE_EXT
#define C_ATYPE_KD US_dmGA_Constraints::ATYPE_KD
#define C_ATYPE_KOFF US_dmGA_Constraints::ATYPE_KOFF

//! \brief A window to edit a discreteGA constraints model

class US_ConstraintsEdit : public US_WidgetsDialog {
  Q_OBJECT

 public:
  //! \param current_model - Location of model structure for editing
  US_ConstraintsEdit(US_Model&);

 signals:
  //! A signal that the class is complete and the passed model reference
  //! has been updated.
  void done(void);

  //! A signal to indicate that the current disk/db selection has changed.
  //! /param DB True if DB is the new selection
  void use_db(bool DB);

 private:
  // Passed parameters
  US_Model& cmodel;

  US_Help showhelp;

  O_CONSTRAINTS constraints;

  QVector<C_CONSTRAINT> attribs;
  QVector<C_CONSTRAINT> flt_attrs;
  QVector<C_CONSTRAINT> wrk_attrs;

  int investigator;
  int oldRow;
  int db_access;
  int dbg_level;
  int crow;
  int arow;

  bool inUpdate;
  bool chgStoi;

  US_Analyte analyte;

  enum { MW, S, D, F, F_F0 } check_type;

  US_Hydrosim hydro_data;
  US_Hydrosim working_data;

  QIcon check;

  QPalette gray;
  QPalette normal;

  QListWidget* lw_comps;
  QListWidget* lw_assocs;

  QList<int> lcompx;

  QPushButton* pb_accept;
  QPushButton* pb_load_c0;

  QLineEdit* le_val_vbar;
  QLineEdit* le_min_vbar;
  QLineEdit* le_max_vbar;
  QLineEdit* le_val_mw;
  QLineEdit* le_min_mw;
  QLineEdit* le_max_mw;
  QLineEdit* le_val_s;
  QLineEdit* le_min_s;
  QLineEdit* le_max_s;
  QLineEdit* le_val_D;
  QLineEdit* le_min_D;
  QLineEdit* le_max_D;
  QLineEdit* le_val_ff0;
  QLineEdit* le_min_ff0;
  QLineEdit* le_max_ff0;
  QLineEdit* le_val_f;
  QLineEdit* le_min_f;
  QLineEdit* le_max_f;
  QLineEdit* le_val_conc;
  QLineEdit* le_min_conc;
  QLineEdit* le_max_conc;
  QLineEdit* le_extinction;
  QLineEdit* le_wavelength;
  QLineEdit* le_oligomer;
  QLineEdit* le_analyteConc;
  QLineEdit* le_molar;
  QLineEdit* le_lbl_kd;
  QLineEdit* le_val_kd;
  QLineEdit* le_min_kd;
  QLineEdit* le_max_kd;
  QLineEdit* le_lbl_koff;
  QLineEdit* le_val_koff;
  QLineEdit* le_min_koff;
  QLineEdit* le_max_koff;
  QLineEdit* le_sigma;
  QLineEdit* le_delta;
  QLineEdit* le_description;

  QCheckBox* ck_sel_vbar;
  QCheckBox* ck_sel_mw;
  QCheckBox* ck_sel_ff0;
  QCheckBox* ck_sel_s;
  QCheckBox* ck_sel_D;
  QCheckBox* ck_sel_f;
  QCheckBox* ck_sel_conc;
  QCheckBox* ck_flt_vbar;
  QCheckBox* ck_flt_mw;
  QCheckBox* ck_flt_ff0;
  QCheckBox* ck_flt_s;
  QCheckBox* ck_flt_D;
  QCheckBox* ck_flt_f;
  QCheckBox* ck_flt_conc;
  QCheckBox* ck_flt_kd;
  QCheckBox* ck_flt_koff;
  QCheckBox* ck_log_mw;
  QCheckBox* ck_log_kd;
  QCheckBox* ck_log_koff;
  QCheckBox* ck_co_sed;
  QCheckBox* ck_isreact;
  QCheckBox* ck_isprod;

 private slots:
  void check_mw(bool);
  void check_ff0(bool);
  void check_s(bool);
  void check_D(bool);
  void check_f(bool);
  void float_vbar(bool);
  void float_mw(bool);
  void float_ff0(bool);
  void float_s(bool);
  void float_D(bool);
  void float_f(bool);
  void float_conc(bool);
  void float_kd(bool);
  void float_koff(bool);
  void logsc_mw(bool);
  void logsc_kd(bool);
  void logsc_koff(bool);

  void comps_connect(bool);
  void assocs_connect(bool);
  int count_checks(void);
  void check_selects(void);
  void acceptProp(void);
  void load_c0(void);
  void co_sed(int);
  void set_molar(void);
  void source_changed(bool);

  void component_select(int);
  void association_select(int);
  void save_comp_settings(int, QVector<C_CONSTRAINT>&);
  void save_assoc_settings(int, QVector<C_CONSTRAINT>&);

  void float_par(bool, QLineEdit*, QLineEdit*, QLineEdit*);
  void check_value(const C_CONSTRAINT, QLineEdit*, QLineEdit*, QLineEdit*);
  double constr_value(const C_ATYPE, QVector<C_CONSTRAINT>&);
  void help(void) { showhelp.show_help("dmga_init_constr.html"); };
};
#endif
