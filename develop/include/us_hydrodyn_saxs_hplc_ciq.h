#ifndef US_HYDRODYN_SAXS_HPLC_CIQ_H
#define US_HYDRODYN_SAXS_HPLC_CIQ_H

#include "us_hydrodyn_saxs_hplc.h"
#include "qlabel.h"
#include "qstring.h"
#include "qlayout.h"

using namespace std;

class US_EXTERN US_Hydrodyn_Saxs_Hplc_Ciq : public QDialog
{
   Q_OBJECT

   public:
      US_Hydrodyn_Saxs_Hplc_Ciq(
                                void                     *              us_hydrodyn_saxs_hplc,
                                map < QString, QString > *              parameters,
                                QWidget *                               p = 0,
                                const char *                            name = 0
                                );

      ~US_Hydrodyn_Saxs_Hplc_Ciq();

   private:

      US_Config *                             USglobal;

      QLabel *                                lbl_title;

      QCheckBox *                             cb_add_bl;
      QCheckBox *                             cb_save_as_pct_iq;

      QPushButton *                           pb_help;
      QPushButton *                           pb_quit;
      QPushButton *                           pb_go;
      void                     *              us_hydrodyn_saxs_hplc;
      map < QString, QString > *              parameters;

      void                                    setupGUI();

   private slots:

      void                                    set_add_bl();
      void                                    set_save_as_pct_iq();

      void                                    help();
      void                                    quit();
      void                                    go();

   protected slots:

      void                                    closeEvent( QCloseEvent * );
};

#endif
