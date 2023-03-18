#ifndef US_HYDRODYN_MALS_CONC_H
#define US_HYDRODYN_MALS_CONC_H

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

class US_EXTERN US_Hydrodyn_Mals_Conc : public QFrame
{
   Q_OBJECT

      friend class US_Hydrodyn_Mals;

   public:
      US_Hydrodyn_Mals_Conc(
                             csv &csv1,
                             void *mals_window, 
                             QWidget *p = 0, 
                             const char *name = 0
                             );
      ~US_Hydrodyn_Mals_Conc();
      void refresh( csv &csv1 );

   private:
      csv           csv1;
      csv           *org_csv;

      void          *mals_window;

      US_Config     *USglobal;

      QLabel        *lbl_title;

      QTableWidget        *t_csv;             

      QPushButton   *pb_load;
      QPushButton   *pb_save;

      QPushButton   *pb_copy;
      QPushButton   *pb_paste;
      QPushButton   *pb_paste_all;

      QPushButton   *pb_help;
      QPushButton   *pb_cancel;
      QPushButton   *pb_set_ok;

      bool          order_ascending;

      void          setupGUI();
      csv           current_csv();
      csv           csv_copy;

      bool          disable_updates;
      void          reload_csv();

      QString       csv_to_qstring( csv from_csv);
      QStringList   csv_parse_line( QString qs );
      void          *us_hydrodyn;

   private slots:

      void update_enables();
      void row_header_released( int row );
      void sort_column( int );

      void load();
      void save();

      void copy();
      void paste();
      void paste_all();

      void cancel();
      void help();
      void set_ok();

   protected slots:

      void closeEvent(QCloseEvent *);
   
};

#endif
