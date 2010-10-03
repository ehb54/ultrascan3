#ifndef US_ROTOR_STRETCH_H
#define US_ROTOR_STRETCH_H

// QT includes:
#include <qwidget.h>
#include <qframe.h>
#include <qpushbt.h>
#include <qlabel.h>
#include <qfont.h>
#include <qlistbox.h>
#include <qmessagebox.h>


// QWT includes:

#include <qwt_counter.h>

// UltraScan includes:
#include "us_util.h"
#include "us_math.h"

extern int global_Xpos;
extern int global_Ypos;

class US_EXTERN US_RotorStretch : public QFrame
{
   Q_OBJECT
   
   public:
   
      US_RotorStretch(QWidget *p=0 , const char *name=0);
      ~US_RotorStretch();
      double speed;
      unsigned int rotor, centerpiece;
      US_Config *USglobal;
      QPushButton *pb_help;
      QPushButton *pb_close;
      QListBox *lb_rotor;
      QListBox *lb_centerpiece;
      QLabel *lbl_banner1;
      QLabel *lbl_rotor;
      QLabel *lbl_centerpiece;
      QLabel *lbl_stretch1;
      QLabel *lbl_stretch2;
      QLabel *lbl_bottom1;
      QLabel *lbl_bottom2;
      QLabel *lbl_speed;
      QwtCounter *cnt_speed;

#ifdef WIN32
#pragma warning ( disable: 4251 )
#endif

      vector < rotorInfo > rotor_list;
      vector <struct centerpieceInfo> cp_list;

#ifdef WIN32
#pragma warning ( default: 4251 )
#endif

   public slots:
      void setup_GUI();
      void update();
      void set_speed(double);
      void update_rotor(int);
      void update_centerpiece(int);
      void help();
};


#endif

