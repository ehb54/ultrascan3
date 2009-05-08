#ifndef US_CLIPDATA_H
#define US_CLIPDATA_H

#include <qlabel.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qwt_counter.h>
#include <qframe.h>
#include <qmessagebox.h>
#include "us.h"
#include "us_util.h"


class US_EXTERN US_ClipData : public QDialog
{
   Q_OBJECT
   
   public:
      US_ClipData(float *, float *, float, float, QWidget *p=0, const char *name = 0);
      ~US_ClipData();

      QLabel *lbl_info;
      QLabel *lbl_conc;
      QLabel *lbl_loading;
      QwtCounter *ct_conc;
      QLabel *lbl_rad;
      QwtCounter *ct_rad;
      QPushButton *pb_cancel;
      QPushButton *pb_ok;
      QPushButton *pb_help;
      US_Config *USglobal;

      float *conc, *rad, meniscus, loading;
   
   public slots:
   
      void cancel();
      void check();
      void help();
      void update_conc(double);
      void update_rad(double);
      
   protected slots:
      void closeEvent(QCloseEvent *);
};

#endif

