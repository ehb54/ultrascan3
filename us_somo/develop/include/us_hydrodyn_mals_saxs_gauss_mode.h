#ifndef US_HYDRODYN_MALS_SAXS_GAUSS_MODE_H
#define US_HYDRODYN_MALS_SAXS_GAUSS_MODE_H

#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_mals_saxs.h"
//Added by qt3to4:
#include <QLabel>
#include <QCloseEvent>

using namespace std;

class US_EXTERN US_Hydrodyn_Mals_Saxs_Gauss_Mode : public QDialog
{
   Q_OBJECT

   public:
      US_Hydrodyn_Mals_Saxs_Gauss_Mode(
                                    map < QString, QString > * parameters,
                                    US_Hydrodyn              * us_hydrodyn,
                                    QWidget                  * p = 0
                                    );
      ~US_Hydrodyn_Mals_Saxs_Gauss_Mode();

   private:
      void         * mals_saxs_win;

#ifdef WIN32
# if QT_VERSION < 0x040000
  #pragma warning ( disable: 4251 )
# endif
#endif
      map < QString, QString > *              parameters;
#ifdef WIN32
# if QT_VERSION < 0x040000
  #pragma warning ( default: 4251 )
# endif
#endif

      US_Hydrodyn *                           us_hydrodyn;

      US_Config *                             USglobal;

      QLabel *                                lbl_gaussian_type;

      QButtonGroup *                          bg_gaussian_type;

      QRadioButton *                          rb_gauss;
      QRadioButton *                          rb_gmg;
      QRadioButton *                          rb_emg;
      QRadioButton *                          rb_emggmg;
      
      QPushButton  *                          pb_clear_gauss;

      QPushButton  *                          pb_quit;
      QPushButton  *                          pb_help;
      QPushButton  *                          pb_ok;

      void                                    setupGUI();

   private slots:

      void                                    quit();
      void                                    help();
      void                                    ok();

      void                                    clear_gauss();

      void                                    update_enables();

   protected slots:

      void closeEvent(QCloseEvent *);
   
};

#endif
