//! \file us_model_editor.h
#ifndef US_MODEL_EDITOR_H
#define US_MODEL_EDITOR_H

#include "us_help.h"
#include "us_predict1.h"
#include "us_widgets_dialog.h"

//! \brief A window to edit a model for simulation

class US_ModelEditor : public US_WidgetsDialog {
      Q_OBJECT

   public:
      //! \param current_model - Location of model structure for editing
      //! \param p             - Parent widget, normally not specified
      //! \param f             - Window flags, normally not specified
      US_ModelEditor(ModelSystem &, QWidget * = 0, Qt::WindowFlags = 0);

   private:
      US_Help showhelp;

      int component;
      int shape;
      enum { PROLATE, OBLATE, ROD, SPHERE };

      QLabel *lb_header;

      QLineEdit *le_sed;
      QLineEdit *le_diff;
      QLineEdit *le_extinction;
      QLineEdit *le_vbar;
      QLineEdit *le_mw;
      QLineEdit *le_f_f0;
      QLineEdit *le_conc;
      QLineEdit *le_c0;
      QLineEdit *le_koff;
      QLineEdit *le_keq;
      QLineEdit *le_sigma;
      QLineEdit *le_delta;
      QLineEdit *le_stoich;

      QComboBox *cmb_component1;
      QListWidget *cmb_component2;

      QSpinBox *sb_count;

      QPushButton *pb_vbar;
      QPushButton *pb_load_c0;

      QRadioButton *rb_rod;
      QRadioButton *rb_oblate;
      QRadioButton *rb_sphere;
      QRadioButton *rb_prolate;

      QString c0_file;

      ModelSystem &model;
      US_Hydrosim simcomp;

      void update_sD(void);
      bool verify_model(void);
      void error(const QString &);
      void update_component(void);

   private slots:

      void help(void) { showhelp.show_help("manual/astfem_component.html"); };

      void update_shape(void);
      void change_component1(int);
      void change_spinbox(int);
      void get_vbar(void);
      void simulate_component(void);

      void select_shape(int);
      void load_c0(void);
      void load_model(void);
      void save_model(void);
      void accept_model(void);

      void update_sed(const QString &);
      void update_diff(const QString &);
      void update_extinction(const QString &);
      void update_vbar(const QString &);
      void update_vbar(double);
      void update_mw(const QString &);
      void update_f_f0(const QString &);
      void update_conc(const QString &);
      void update_keq(const QString &);
      void update_koff(const QString &);
      void update_sigma(const QString &);
      void update_delta(const QString &);
};
#endif
