#ifndef US_HYDRODYN_SAXS_HPLC_PARAMETERS_H
#define US_HYDRODYN_SAXS_HPLC_PARAMETERS_H

#include "us_hydrodyn_saxs_hplc.h"
#include "qlabel.h"
#include "qstring.h"
#include "qlayout.h"
#include "qvalidator.h"
//Added by qt3to4:
#include <QCloseEvent>

using namespace std;

class US_EXTERN US_Hydrodyn_Saxs_Hplc_Parameters : public QDialog
{
   Q_OBJECT

   public:
      US_Hydrodyn_Saxs_Hplc_Parameters(
                                 void                     *              us_hydrodyn_saxs_hplc,
                                 map < QString, QString > *              parameters,
                                 QWidget *                               p = 0,
                                 const char *                            name = 0
                                 );

      ~US_Hydrodyn_Saxs_Hplc_Parameters();

   private:

      US_Config *                             USglobal;

      QLabel *                                lbl_title;

      QLabel *                                lbl_saxs_hplc_param_frame_interval;
      QLineEdit *                             le_saxs_hplc_param_frame_interval;

      QLabel *                                lbl_saxs_hplc_param_g_conc;
      QLineEdit *                             le_saxs_hplc_param_g_conc;

      QLabel *                                lbl_saxs_hplc_param_g_psv;
      QLineEdit *                             le_saxs_hplc_param_g_psv;
      
      QLabel *                                lbl_saxs_hplc_param_I0_exp;
      QLineEdit *                             le_saxs_hplc_param_I0_exp;

      QLabel *                                lbl_saxs_hplc_param_I0_theo;
      QLineEdit *                             le_saxs_hplc_param_I0_theo;

      QLabel *                                lbl_saxs_hplc_param_diffusion_len;
      QLineEdit *                             le_saxs_hplc_param_diffusion_len;

      QLabel *                                lbl_saxs_hplc_param_electron_nucleon_ratio;
      QLineEdit *                             le_saxs_hplc_param_electron_nucleon_ratio;

      QLabel *                                lbl_saxs_hplc_param_nucleon_mass;
      QLineEdit *                             le_saxs_hplc_param_nucleon_mass;

      QLabel *                                lbl_saxs_hplc_param_solvent_electron_density;
      QLineEdit *                             le_saxs_hplc_param_solvent_electron_density;

      QPushButton *                           pb_help;
      QPushButton *                           pb_quit;
      QPushButton *                           pb_keep;
      QPushButton *                           pb_save;

      void                     *              us_hydrodyn_saxs_hplc;
      map < QString, QString > *              parameters;

      void                                    setupGUI();

   private slots:

      void                                    set_saxs_hplc_param_frame_interval           ( const QString & );
      void                                    set_saxs_hplc_param_g_conc                   ( const QString & );
      void                                    set_saxs_hplc_param_g_psv                    ( const QString & );
      void                                    set_saxs_hplc_param_I0_exp                   ( const QString & );
      void                                    set_saxs_hplc_param_I0_theo                  ( const QString & );
      void                                    set_saxs_hplc_param_diffusion_len            ( const QString & );
      void                                    set_saxs_hplc_param_electron_nucleon_ratio   ( const QString & );
      void                                    set_saxs_hplc_param_nucleon_mass             ( const QString & );
      void                                    set_saxs_hplc_param_solvent_electron_density ( const QString & );

      void                                    help();
      void                                    quit();
      void                                    keep();
      void                                    save();

   protected slots:

      void                                    closeEvent( QCloseEvent * );
};

#endif
