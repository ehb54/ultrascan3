// ---------------------------------------------------------------------------------------------
// --------------- WARNING: this code is generated by an automatic code generator --------------
// ---------------------------------------------------------------------------------------------
// -------------- WARNING: any modifications made to this code will be overwritten -------------
// ---------------------------------------------------------------------------------------------

#ifndef US_HYDRODYN_SAXS_CONC_CSV_FRAMES_H
#define US_HYDRODYN_SAXS_CONC_CSV_FRAMES_H

#include "us_hydrodyn.h"
//Added by qt3to4:
#include <QLabel>
#include <QCloseEvent>

using namespace std;

class US_EXTERN US_Hydrodyn_Dad_Conc_Csv_Frames : public QDialog
{
   Q_OBJECT

   public:
      US_Hydrodyn_Dad_Conc_Csv_Frames(
                       void                     *              us_hydrodyn,
                       map < QString, QString > *              parameters,
                       QWidget *                               p = 0,
                       const char *                            name = ""
                       );

      ~US_Hydrodyn_Dad_Conc_Csv_Frames();

   private:

      US_Config *                             USglobal;
      void                                    setupGUI();
      map < QString, QString > *              parameters;
      void *                                  us_hydrodyn;
      vector < QPushButton * >                bottom_row_buttons;
      map < QWidget *, vector < QWidget * > > repeats;
      void                                    showhide( vector < QWidget * > &, bool isChecked );

      QPushButton *                           pb_help;
      QPushButton *                           pb_cancel;

      QLabel *                                lbl_label_0;

      QLabel *                                lbl_starttime;
      QLineEdit *                             le_starttime;

      QLabel *                                lbl_endtime;
      QLineEdit *                             le_endtime;

      QLabel *                                lbl_pointcount;
      QLineEdit *                             le_pointcount;

      QLabel *                                lbl_frame1t;
      QLineEdit *                             le_frame1t;

      QLabel *                                lbl_time2frame;
      QLineEdit *                             le_time2frame;

      QCheckBox *                             cb_startframe;
      QLabel *                                lbl_startframenumber;
      QLineEdit *                             le_startframenumber;

      QCheckBox *                             cb_endframe;
      QLabel *                                lbl_endframenumber;
      QLineEdit *                             le_endframenumber;

      QPushButton *                           pb_go;

   private slots:

      void                                    help();
      void                                    cancel();

      void                                    update_starttime( const QString & );
      void                                    update_endtime( const QString & );
      void                                    update_pointcount( const QString & );
      void                                    update_frame1t( const QString & );
      void                                    update_time2frame( const QString & );
      void                                    set_startframe();
      void                                    update_startframenumber( const QString & );
      void                                    set_endframe();
      void                                    update_endframenumber( const QString & );
      void                                    go();

   protected slots:

      void                                    closeEvent( QCloseEvent * );
};

#endif