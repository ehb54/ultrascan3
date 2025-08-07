#ifndef US_HYDRODYN_SAXS_CONC_H
#define US_HYDRODYN_SAXS_CONC_H

// QT defs:

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qstring.h>
//#include <q3frame.h>
#include <qcheckbox.h>
#include <qfileinfo.h>
#include <qmenubar.h>
#include <qprinter.h>
#include <qprogressbar.h>
#include <qtablewidget.h>
#include <qtextedit.h>
//Added by qt3to4:
#include <QCloseEvent>

#include "us_util.h"

//standard C and C++ defs:

#include <iostream>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "us_hydrodyn_comparative.h"

using namespace std;

class US_EXTERN US_Hydrodyn_Saxs_Conc : public QDialog {
      Q_OBJECT

   public:
      US_Hydrodyn_Saxs_Conc(csv &csv1, void *saxs_window, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_Saxs_Conc();
      void refresh(csv &csv1);

   private:
      csv csv1;
      csv *org_csv;

      void *saxs_window;

      US_Config *USglobal;

      QLabel *lbl_title;

      QTableWidget *t_csv;

      QPushButton *pb_load;
      QPushButton *pb_save;

      QPushButton *pb_copy;
      QPushButton *pb_paste;
      QPushButton *pb_paste_all;

      QPushButton *pb_reset_to_defaults;

      QPushButton *pb_help;
      QPushButton *pb_cancel;
      QPushButton *pb_set_ok;

      bool order_ascending;

      void setupGUI();
      csv current_csv();
      csv csv_copy;

      bool disable_updates;

      QString csv_to_qstring(csv from_csv);
      QStringList csv_parse_line(QString qs);
      void *us_hydrodyn;

      void reload_csv();

   private slots:

      void update_enables();
      void row_header_released(int row);
      void sort_column(int);

      void load();
      void save();

      void reset_to_defaults();

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
