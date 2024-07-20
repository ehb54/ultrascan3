#ifndef US_HYDRODYN_DAD_CIQ_H
#define US_HYDRODYN_DAD_CIQ_H

#include "us_hydrodyn_dad.h"
#include "qlabel.h"
#include "qstring.h"
#include "qlayout.h"
#include "qvalidator.h"
//Added by qt3to4:
#include <QCloseEvent>

using namespace std;

class US_EXTERN US_Hydrodyn_Dad_Ciq : public QDialog
{
   Q_OBJECT

   public:
      US_Hydrodyn_Dad_Ciq(
                                void                     *              us_hydrodyn_dad,
                                map < QString, QString > *              parameters,
                                QWidget *                               p = 0,
                                const char *                            name = 0
                                );

      ~US_Hydrodyn_Dad_Ciq();

   private:

      US_Config *                             USglobal;

      QLabel *                                lbl_title;

      QCheckBox *                             cb_add_bl;
      QCheckBox *                             cb_save_as_pct_iq;
      QCheckBox *                             cb_save_sum;

      QCheckBox *                             cb_makeiq_avg_peaks;
      QLineEdit *                             le_makeiq_avg_peaks;

      QCheckBox *                             cb_sd_source;


      QLabel *                                lbl_sd_zeros_found;
      QCheckBox *                             cb_sd_zero_avg_local_sd;
      QCheckBox *                             cb_sd_zero_keep_as_zeros;
      QCheckBox *                             cb_sd_zero_set_to_pt1pct;

      mQLabel *                               lbl_zeros_found;
      QCheckBox *                             cb_zero_drop_points;
      QCheckBox *                             cb_zero_avg_local_sd;
      QCheckBox *                             cb_zero_keep_as_zeros;

      QCheckBox *                             cb_I0se;
      QLineEdit *                             le_I0se;

      QCheckBox *                             cb_normalize;

      QLabel *                                lbl_error;
      
      QCheckBox *                             cb_conc_to_saxs;
      QLabel *                                lbl_conc_to_saxs_info1;
      QLabel *                                lbl_conc_to_saxs_info2;
      QLabel *                                lbl_conc_to_saxs_info3;

      QLabel *                                lbl_conc;
      QLabel *                                lbl_gaussian;
      QLabel *                                lbl_conv;
      QLabel *                                lbl_dndc;

#ifdef WIN32
# if QT_VERSION < 0x040000
  #pragma warning ( disable: 4251 )
# endif
#endif
      vector < QLabel * >                     lbl_gaussian_id;
      vector < QLineEdit * >                  le_conv;
      vector < QLineEdit * >                  le_dndc;
#ifdef WIN32
# if QT_VERSION < 0x040000
  #pragma warning ( default: 4251 )
# endif
#endif

      QPushButton *                           pb_global;

      QPushButton *                           pb_help;
      QPushButton *                           pb_quit;
      QPushButton *                           pb_create_ng;
      QPushButton *                           pb_go;

      void                     *              us_hydrodyn_dad;
      map < QString, QString > *              parameters;

      void                                    setupGUI();

#ifdef WIN32
# if QT_VERSION < 0x040000
  #pragma warning ( disable: 4251 )
# endif
#endif
      void                                    ws_hide( vector < QWidget * >, bool hide = true );
      vector < QWidget * >                    ws_zeros;
      vector < QWidget * >                    ws_sd_zeros;
#ifdef WIN32
# if QT_VERSION < 0x040000
  #pragma warning ( default: 4251 )
# endif
#endif


   private slots:

      void                                    set_add_bl();
      void                                    set_save_as_pct_iq();
      void                                    set_sd_source();

      void                                    set_sd_zero_avg_local_sd();
      void                                    set_sd_zero_keep_as_zeros();
      void                                    set_sd_zero_set_to_pt1pct();

      void                                    zeros_found();
      void                                    set_zero_drop_points();
      void                                    set_zero_avg_local_sd();
      void                                    set_zero_keep_as_zeros();

      void                                    set_normalize();

      void                                    set_I0se();

      void                                    update_enables();

      void                                    global();

      void                                    help();
      void                                    quit();
      void                                    create_ng();
      void                                    go();

   protected slots:

      void                                    closeEvent( QCloseEvent * );
};

#endif
