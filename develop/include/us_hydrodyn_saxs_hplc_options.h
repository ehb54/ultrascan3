#ifndef US_HYDRODYN_SAXS_HPLC_OPTIONS_H
#define US_HYDRODYN_SAXS_HPLC_OPTIONS_H

#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_saxs_hplc.h"

using namespace std;

class US_EXTERN US_Hydrodyn_Saxs_Hplc_Options : public QDialog
{
   Q_OBJECT

   public:
      US_Hydrodyn_Saxs_Hplc_Options(
                                    map < QString, QString > * parameters,
                                    QWidget                  * p = 0, 
                                    const char               * name = 0
                                    );
      ~US_Hydrodyn_Saxs_Hplc_Options();

   private:
      void         * hplc_win;

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif
      map < QString, QString > *              parameters;
#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

      US_Config    * USglobal;

      QLabel       * lbl_title;

      QLabel       * lbl_gaussian_type;

      QButtonGroup * bg_gaussian_type;
      QRadioButton * rb_gauss;
      QRadioButton * rb_gmg;
      QRadioButton * rb_emg;
      QRadioButton * rb_emggmg;
      
      QLabel       * lbl_other_options;
      QCheckBox    * cb_csv_transposed;

      QPushButton  * pb_quit;
      QPushButton  * pb_help;
      QPushButton  * pb_ok;

      void           setupGUI();

   private slots:

      void quit();
      void help();
      void ok();

   protected slots:

      void closeEvent(QCloseEvent *);
   
};

#endif
