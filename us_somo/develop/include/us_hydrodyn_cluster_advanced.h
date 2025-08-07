#ifndef US_HYDRODYN_CLUSTER_ADVANCED_H
#define US_HYDRODYN_CLUSTER_ADVANCED_H

// QT defs:

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qstring.h>
//#include <q3frame.h>
#include <qcheckbox.h>
#include <qmenubar.h>
#include <qprinter.h>
#include <qtablewidget.h>
#include <qtextedit.h>
//Added by qt3to4:
#include <QCloseEvent>

#include "us_hydrodyn_batch.h"
#include "us_hydrodyn_comparative.h"
#include "us_hydrodyn_pdbdefs.h"
#include "us_util.h"

//standard C and C++ defs:

#include <iostream>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

using namespace std;

class US_EXTERN US_Hydrodyn_Cluster_Advanced : public QDialog {
      Q_OBJECT

      friend class US_Hydrodyn_Cluster;

   public:
      US_Hydrodyn_Cluster_Advanced(csv &csv1, void *us_hydrodyn, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_Cluster_Advanced();

   private:
      csv csv1;
      csv *original_csv1;

      void *us_hydrodyn;
      void *cluster_window;

      US_Config *USglobal;

      QLabel *lbl_title;

      QTableWidget *t_csv;

      QFont ft;
      QTextEdit *editor;
      QMenuBar *m;

      QPushButton *pb_ok;
      QPushButton *pb_help;
      QPushButton *pb_cancel;

      void editor_msg(QString color, QString msg);

      csv current_csv();
      void recompute_interval_from_points();
      void recompute_points_from_interval();
      unsigned int interval_starting_row;
      void reset_csv();

   private slots:

      void setupGUI();

      void table_value(int, int);

      void clear_display();
      void update_font();
      void save();

      void ok();
      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
};

#endif
