#ifndef US_HYDRODYN_BD_LOAD_RESULTS_OPTS_H
#define US_HYDRODYN_BD_LOAD_RESULTS_OPTS_H

// QT defs:

#include <qdialog.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include "us_util.h"

using namespace std;

// set psv & mw for DAMMIN/DAMMIF file loads

class US_EXTERN US_Hydrodyn_BD_Load_Results_Opts : public QDialog {
      Q_OBJECT

   public:
      US_Hydrodyn_BD_Load_Results_Opts(
         QString msg, double *temperature, double *solvent_viscosity, double *solvent_density, QString *solvent_name,
         QString *solvent_acronym, double *psv, double browflex_temperature, double browflex_solvent_viscosity,
         bool *check_fix_overlaps, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_BD_Load_Results_Opts();

   private:
      QLabel *lbl_info;
      QLabel *lbl_param;
      QLabel *lbl_browflex;
      QLabel *lbl_to_use;
      QLabel *lbl_psv;
      QLabel *lbl_solvent_name;
      QLabel *lbl_solvent_defaults;
      QLabel *lbl_solvent_acronym;
      QLabel *lbl_temperature;
      QLabel *lbl_solvent_viscosity;
      QLabel *lbl_solvent_density;

      QLabel *lbl_browflex_temperature;
      QLabel *lbl_browflex_solvent_viscosity;

      QCheckBox *cb_solvent_defaults;
      QCheckBox *cb_set_to_browflex;
      QCheckBox *cb_check_fix_overlaps;

      QLineEdit *le_solvent_name;
      QLineEdit *le_solvent_acronym;
      QLineEdit *le_temperature;
      QLineEdit *le_solvent_viscosity;
      QLineEdit *le_solvent_density;
      QLineEdit *le_psv;


      QPushButton *pb_cancel;
      QPushButton *pb_help;

      void setupGUI();

      US_Config *USglobal;

      QString msg;
      double *temperature;
      double *solvent_viscosity;
      double *solvent_density;
      QString *solvent_name;
      QString *solvent_acronym;
      double *psv;

      double browflex_temperature;
      double browflex_solvent_viscosity;

      bool *check_fix_overlaps;

   public:
      int write_conditions_to_file(QString filename);
      void read_conditions_from_file(QString filename);

   public slots:

   private slots:

      void update_solvent_name(const QString &str);
      void update_solvent_acronym(const QString &str);
      void update_temperature(const QString &str);
      void update_solvent_viscosity(const QString &str);
      void update_solvent_density(const QString &str);
      void update_psv(const QString &);

      void set_to_browflex();
      void set_solvent_defaults();
      void set_check_fix_overlaps();

      void check_solvent_browflex_defaults();

      void cancel();
      void help();
};

#endif
