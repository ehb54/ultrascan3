#ifndef US_HYDRODYN_DAD_PARAMETERS_H
#define US_HYDRODYN_DAD_PARAMETERS_H

#include "us_hydrodyn_dad.h"
#include "qlabel.h"
#include "qstring.h"
#include "qlayout.h"
#include "qvalidator.h"
//Added by qt3to4:
#include <QCloseEvent>

using namespace std;

class US_EXTERN US_Hydrodyn_Dad_Parameters : public QDialog
{
   Q_OBJECT

   public:
      US_Hydrodyn_Dad_Parameters(
                                 void                     *              us_hydrodyn_dad,
                                 map < QString, QString > *              parameters,
                                 QWidget *                               p = 0,
                                 const char *                            name = 0
                                 );

      ~US_Hydrodyn_Dad_Parameters();

   private:

      US_Config *                             USglobal;

      QLabel *                                lbl_title;

      QLabel *                                lbl_dad_param_lambda;
      QLineEdit *                             le_dad_param_lambda;

      QLabel *                                lbl_dad_param_n;
      QLineEdit *                             le_dad_param_n;

      QLabel *                                lbl_dad_param_g_dndc;
      QLineEdit *                             le_dad_param_g_dndc;

      QLabel *                                lbl_dad_param_g_extinction_coef;
      QLineEdit *                             le_dad_param_g_extinction_coef;

      QLabel *                                lbl_dad_param_g_conc;
      QLineEdit *                             le_dad_param_g_conc;

      QLabel *                                lbl_dad_param_DLS_detector;
      QLineEdit *                             le_dad_param_DLS_detector;

      QPushButton *                           pb_help;
      QPushButton *                           pb_quit;
      QPushButton *                           pb_keep;
      QPushButton *                           pb_save;

      void                     *              us_hydrodyn_dad;
      map < QString, QString > *              parameters;

      void                                    setupGUI();

      set < QWidget * >                       always_hide_widgets;

   private slots:

      void                                    set_dad_param_lambda            ( const QString & );
      void                                    set_dad_param_n                 ( const QString & );
      void                                    set_dad_param_g_dndc            ( const QString & );
      void                                    set_dad_param_g_extinction_coef ( const QString & );
      void                                    set_dad_param_g_conc            ( const QString & );
      void                                    set_dad_param_DLS_detector      ( const QString & );

      void                                    help();
      void                                    quit();
      void                                    keep();
      void                                    save();

   protected slots:

      void                                    closeEvent( QCloseEvent * );
};

#endif