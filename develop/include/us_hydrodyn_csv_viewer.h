#ifndef US_HYDRODYN_CSV_VIEWER_H
#define US_HYDRODYN_CSV_VIEWER_H

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

#include "us_util.h"

//standard C and C++ defs:

#include <map>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "us_hydrodyn_comparative.h"

using namespace std;

class US_EXTERN US_Hydrodyn_Csv_Viewer : public QFrame
{
   Q_OBJECT

   public:
      US_Hydrodyn_Csv_Viewer(
                             csv csv1,
                             void *us_hydrodyn, 
                             QWidget *p = 0, 
                             const char *name = 0
                             );
      ~US_Hydrodyn_Csv_Viewer();

   private:
      csv           csv1;

      void          *us_hydrodyn;

      US_Config     *USglobal;

      QLabel        *lbl_title;

      QTable        *t_csv;             

      QPushButton   *pb_help;
      QPushButton   *pb_cancel;

      bool          order_ascending;

   private slots:
      
      void setupGUI();

      void sort_column( int );

      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
   
};

#endif
