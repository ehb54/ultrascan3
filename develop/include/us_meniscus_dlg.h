#ifndef US_MENISCUS_DLG_H
#define US_MENISCUS_DLG_H

#include "us.h"
#include "us_extern.h"
#include "us_util.h"

#include <qlabel.h>
#include <qlineedit.h>
#include <qdialog.h>
#include <qstring.h>
#include <qlayout.h>
#include <qframe.h>

class US_EXTERN US_MeniscusDialog : public QDialog
{
   Q_OBJECT
   
   public:
      US_MeniscusDialog(float *, QWidget *p=0, const char *name = 0);
      ~US_MeniscusDialog();
      
      float *meniscus;

      QLabel *lbl_info;
      QLabel *lbl_meniscus1;
      QLabel *lbl_meniscus2;
      QLabel *lbl_meniscus3;
      QLabel *lbl_meniscus4;
      QLabel *lbl_meniscus5;
      QLabel *lbl_meniscus6;
      QLabel *lbl_meniscus7;
      QLabel *lbl_meniscus8;

      QLineEdit *le_meniscus1;
      QLineEdit *le_meniscus2;
      QLineEdit *le_meniscus3;
      QLineEdit *le_meniscus4;
      QLineEdit *le_meniscus5;
      QLineEdit *le_meniscus6;
      QLineEdit *le_meniscus7;
      QLineEdit *le_meniscus8;

      QPushButton *pb_save;
      QPushButton *pb_cancel;
      
      US_Config *USglobal;    /*!< A US_Config reference. */

   private slots:

      void cancel();
      void ok();
      void update_meniscus1(const QString &);
      void update_meniscus2(const QString &);
      void update_meniscus3(const QString &);
      void update_meniscus4(const QString &);
      void update_meniscus5(const QString &);
      void update_meniscus6(const QString &);
      void update_meniscus7(const QString &);
      void update_meniscus8(const QString &);

   protected slots:
      void setup_GUI();
};

#endif
