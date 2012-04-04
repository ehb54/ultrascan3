#ifndef US_HYDRODYN_SAXS_2D_H
#define US_HYDRODYN_SAXS_2D_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qtextedit.h>
#include <qprogressbar.h>
#include <qmenubar.h>
#include <qfileinfo.h>
#include <qprinter.h>
#include <qtable.h>
#include <qimage.h>

#include "us_util.h"

//standard C and C++ defs:

#include <map>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "us_hydrodyn_saxs.h"

using namespace std;

class US_EXTERN US_Hydrodyn_Saxs_2d : public QFrame
{
   Q_OBJECT

      friend class US_Hydrodyn_Saxs;

   public:
      US_Hydrodyn_Saxs_2d(
                          void *us_hydrodyn, 
                          QWidget *p = 0, 
                          const char *name = 0
                          );
      ~US_Hydrodyn_Saxs_2d();

   private:
      void          *us_hydrodyn;

      US_Config     *USglobal;

      QLabel        *lbl_title;

      QLabel        *lbl_detector_distance;
      QLineEdit     *le_detector_distance;

      QLabel        *lbl_detector_geometry;
      QLineEdit     *le_detector_height;
      QLineEdit     *le_detector_width;

      QLabel        *lbl_detector_pixels;
      QLineEdit     *le_detector_pixels_height;
      QLineEdit     *le_detector_pixels_width;

      QLabel        *lbl_beam_center;
      QLineEdit     *le_beam_center_height;
      QLineEdit     *le_beam_center_width;

      QLabel        *lbl_sample_rotations;
      QLineEdit     *le_sample_rotations;

      QProgressBar  *progress;

      QPushButton   *pb_integrate;

      QLabel        *lbl_2d;

      QPushButton   *pb_set_target;
      QLabel        *lbl_current_target;

      QPushButton   *pb_start;
      QPushButton   *pb_stop;

      QFont         ft;
      QTextEdit     *editor;
      QMenuBar      *m;

      QPushButton   *pb_help;
      QPushButton   *pb_cancel;

      void          editor_msg( QString color, QString msg );

      bool          running;
      void          update_enables();

      US_Hydrodyn_Saxs *saxs_window;
      bool             *saxs_widget;
      bool             activate_saxs_window();
      void             run_one();

      QImage        *i_2d;

      bool          validate();
      void          update_2d();

   private slots:

      void setupGUI();

      void set_target();

      void update_detector_distance       ( const QString & );
      void update_detector_height         ( const QString & );
      void update_detector_width          ( const QString & );
      void update_detector_pixels_height  ( const QString & );
      void update_detector_pixels_width   ( const QString & );
      void update_beam_center_height      ( const QString & );
      void update_beam_center_width       ( const QString & );
      void update_sample_rotations        ( const QString & );

      void integrate();

      void start();
      void stop();

      void clear_display();
      void update_font();
      void save();

      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
   
};

#endif
