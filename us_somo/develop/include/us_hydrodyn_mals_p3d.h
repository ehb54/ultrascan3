#ifndef US_HYDRODYN_MALS_P3D_H
#define US_HYDRODYN_MALS_P3D_H

#include "us_hydrodyn_mals.h"
#include "qlabel.h"
#include "qstring.h"
#include "qlayout.h"
//Added by qt3to4:
#include <QCloseEvent>

using namespace std;

class US_EXTERN US_Hydrodyn_Mals_P3d : public QDialog
{
   Q_OBJECT

   public:
      US_Hydrodyn_Mals_P3d(
                                void                     *              us_hydrodyn_mals,
                                map < QString, QString > *              parameters,
                                QWidget *                               p = 0,
                                const char *                            name = 0
                                );

      ~US_Hydrodyn_Mals_P3d();

   private:

      US_Config *                             USglobal;

      QLabel *                                lbl_title;

      QLabel *                                lbl_plot_curves;
      QPushButton *                           pb_plot_all;

#ifdef WIN32
# if QT_VERSION < 0x040000
  #pragma warning ( disable: 4251 )
# endif
#endif
      vector < QCheckBox * >                  cb_plot_curves;
#ifdef WIN32
# if QT_VERSION < 0x040000
  #pragma warning ( default: 4251 )
# endif
#endif
      QPushButton *                           pb_help;
      QPushButton *                           pb_quit;
      QPushButton *                           pb_plot;

      void                     *              us_hydrodyn_mals;
      map < QString, QString > *              parameters;

      void                                    setupGUI();

   private slots:

      void                                    plot_all();

      void                                    plot();
      void                                    quit();
      void                                    help();

      void                                    update_enables();

   protected slots:

      void                                    closeEvent( QCloseEvent * );
};

#endif
