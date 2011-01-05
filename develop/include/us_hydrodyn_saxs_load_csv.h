#ifndef US_HYDRODYN_SAXS_LOAD_CSV_H
#define US_HYDRODYN_SAXS_LOAD_CSV_H

// QT defs:

#include <qlabel.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qstringlist.h>
#include <qlistbox.h>

#include "us_util.h"

using namespace std;

class US_EXTERN US_Hydrodyn_Saxs_Load_Csv : public QDialog
{
   Q_OBJECT

   public:

      US_Hydrodyn_Saxs_Load_Csv(
                                QString msg,
                                QStringList *qsl_names,
                                QStringList *qsl_sel_names,
                                QWidget *p = 0, 
                                const char *name = 0
                                );
      ~US_Hydrodyn_Saxs_Load_Csv();

   private:

      QLabel *lbl_info;

      QListBox *lb_names;

      QPushButton *pb_select_all;
      QPushButton *pb_ok;
      QPushButton *pb_cancel;
      QPushButton *pb_help;

      void setupGUI();

      US_Config *USglobal;
      
      QString msg;
      QStringList *qsl_names;
      QStringList *qsl_sel_names;

   private slots:

      void update_selected();

      void select_all();
      void ok();
      void cancel();
      void help();
};

#endif
