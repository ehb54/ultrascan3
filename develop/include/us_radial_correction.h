#ifndef US_RADIAL_CORRECTION_H
#define US_RADIAL_CORRECTION_H

// QT includes:
#include <qwidget.h>
#include <qframe.h>
#include <qpushbt.h>
#include <qfile.h>
#include <qlabel.h>
#include <qtextstream.h>
#include <qfont.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qstringlist.h>
#include <qprogressbar.h>

// QWT includes:

#include <qwt_counter.h>

// UltraScan includes:
#include "us_util.h"
#include "us_editor.h"

extern int global_Xpos;
extern int global_Ypos;

class US_EXTERN US_RadialCorrection : public QFrame
{
   Q_OBJECT
   
   public:
   
      US_RadialCorrection(QWidget *p=0 , const char *name=0);
      ~US_RadialCorrection();
      double correction;
      US_Config *USglobal;
      QPushButton *pb_load;
      QPushButton *pb_process;
      QPushButton *pb_help;
      QPushButton *pb_close;
      QPushButton *pb_reset;
      QLabel *lbl_banner1;
      QLabel *lbl_correction;
      QLabel *lbl_scans;
      QwtCounter *cnt_correction;
      QProgressBar *pgb_progress;

#ifdef WIN32
#pragma warning ( disable: 4251 )
#endif

      vector <QString> filenames;

#ifdef WIN32
#pragma warning ( default: 4251 )
#endif

   public slots:
      void setup_GUI();
      void process();
      void load();
      void help();
      void reset();
      void set_correction(double);

};


#endif

