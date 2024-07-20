#ifndef US_HYDRODYN_DAD_CONC_LOAD_H
#define US_HYDRODYN_DAD_CONC_LOAD_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
//#include <q3frame.h>
#include <qcheckbox.h>
#include <qtextedit.h>
#include <qprogressbar.h>
#include <qmenubar.h>
#include <qfileinfo.h>
#include <qprinter.h>
#include <qtablewidget.h>
//Added by qt3to4:
#include <QCloseEvent>

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

class US_EXTERN US_Hydrodyn_Dad_Conc_Load : public QDialog
{
   Q_OBJECT

      friend class US_Hydrodyn_Dad_Conc;

   public:
   // takes a text format file
   // lets users select columns for name & concentration 
   // and concentration units
   // tries to set values in csv1
      US_Hydrodyn_Dad_Conc_Load(
                                        QStringList &qsl_text,
                                        csv &csv1,
                                        QWidget *p = 0, 
                                        const char *name = 0
                                        );
      ~US_Hydrodyn_Dad_Conc_Load();

   private:
      QStringList   *text;
      csv           *csv1;
      csv           trial_csv;

      US_Config     *USglobal;

      QLabel        *lbl_title;

      QTableWidget        *t_csv;             

      QPushButton   *pb_del_row;

      QPushButton   *pb_set_name;
      QLabel        *lbl_name;

      QPushButton   *pb_adjust;
      
      QPushButton   *pb_set_conc;
      QLabel        *lbl_conc;
      
      QPushButton   *pb_trial;
      QLabel        *lbl_trial;

      QPushButton   *pb_help;
      QPushButton   *pb_cancel;
      QPushButton   *pb_set_ok;

      void          setupGUI();

      csv           disp_csv;
      csv           current_csv();
      void          reload_csv();

      bool          disable_updates;

      QString       csv_to_qstring( csv from_csv );

   private slots:

      void update_enables();
      void col_header_released( int col );

      void del_row();
      void set_name();
      void set_conc();
      void adjust();

      void trial();

      void cancel();
      void help();
      void set_ok();

   protected slots:

      void closeEvent(QCloseEvent *);
   
};

#endif
