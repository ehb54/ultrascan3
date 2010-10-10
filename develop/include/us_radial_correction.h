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
#include <qlistbox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>

// QWT includes:

#include <qwt_counter.h>

// UltraScan includes:
#include "us_util.h"
#include "us_math.h"
#include "us_editor.h"

extern int global_Xpos;
extern int global_Ypos;

class US_EXTERN US_RadialCorrection : public QFrame
{
   Q_OBJECT
   
   public:
   
      US_RadialCorrection(QWidget *p=0 , const char *name=0);
      ~US_RadialCorrection();
      double correction, bottom_measured, top_measured, top_target, bottom_target, stretch;
      double speed;
      unsigned int rotor, centerpiece;
      bool scaled, trim;

      US_Config *USglobal;
      QRadioButton *rb_constant;
      QRadioButton *rb_scaled;
      QCheckBox *cb_trim;
      QButtonGroup *bg_correction;
      QPushButton *pb_load;
      QPushButton *pb_process;
      QPushButton *pb_help;
      QPushButton *pb_close;
      QPushButton *pb_reset;
      QListBox *lb_rotor;
      QListBox *lb_centerpiece;
      QLabel *lbl_banner1;
      QLabel *lbl_correction;
      QLabel *lbl_scans;
      QLabel *lbl_top_measured;
      QLabel *lbl_bottom_measured;
      QLabel *lbl_top_target;
      QLabel *lbl_rotor;
      QLabel *lbl_centerpiece;
      QLabel *lbl_stretch1;
      QLabel *lbl_stretch2;
      QLabel *lbl_bottom1;
      QLabel *lbl_bottom2;
      QLabel *lbl_speed;
      QwtCounter *cnt_speed;
      QwtCounter *cnt_correction;
      QwtCounter *cnt_top_measured;
      QwtCounter *cnt_top_target;
      QwtCounter *cnt_bottom_measured;
      QProgressBar *pgb_progress;

#ifdef WIN32
#pragma warning ( disable: 4251 )
#endif

      vector <QString> filenames;
      vector < rotorInfo > rotor_list;
      vector <struct centerpieceInfo> cp_list;

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
      void update();
      void set_speed(double);
      void set_top_measured(double);
      void set_top_target(double);
      void set_bottom_measured(double);
      void update_scaled(int);
      void update_rotor(int);
      void update_centerpiece(int);
      void update_trim();
      
};


#endif

