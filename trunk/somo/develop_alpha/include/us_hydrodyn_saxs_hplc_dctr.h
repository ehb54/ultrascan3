#ifndef US_HYDRODYN_SAXS_HPLC_DCTR_H
#define US_HYDRODYN_SAXS_HPLC_DCTR_H

#include "us_hydrodyn_saxs_hplc.h"
#include "qlabel.h"
#include "qstring.h"
#include "qlayout.h"
#include "qvalidator.h"
//Added by qt3to4:
#include <QCloseEvent>

using namespace std;

class US_EXTERN US_Hydrodyn_Saxs_Hplc_Dctr : public QDialog
{
   Q_OBJECT

   public:
      US_Hydrodyn_Saxs_Hplc_Dctr(
                                 void                     *              us_hydrodyn_saxs_hplc,
                                 map < QString, QString > *              parameters,
                                 QWidget *                               p = 0,
                                 const char *                            name = 0
                                 );

      ~US_Hydrodyn_Saxs_Hplc_Dctr();

   private:

      US_Config *                             USglobal;

      QLabel *                                lbl_title;

      QLabel *                                lbl_type;
      QCheckBox *                             cb_uv;
      QLabel *                                lbl_uv_conv;
      QLineEdit *                             le_uv_conv;

      QCheckBox *                             cb_ri;
      QLabel *                                lbl_ri_conv;
      QLineEdit *                             le_ri_conv;

      QPushButton *                           pb_help;
      QPushButton *                           pb_quit;
      QPushButton *                           pb_keep;
      QPushButton *                           pb_save;

      void                     *              us_hydrodyn_saxs_hplc;
      map < QString, QString > *              parameters;


      void                                    setupGUI();

   private slots:

      void                                    set_uv();
      void                                    set_ri();

      void                                    ri_conv_text( const QString & );
      void                                    uv_conv_text( const QString & );

      void                                    help();
      void                                    quit();
      void                                    keep();
      void                                    save();

   protected slots:

      void                                    closeEvent( QCloseEvent * );
};

#endif
