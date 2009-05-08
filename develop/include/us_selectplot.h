#ifndef US_SELECTPLOT_H
#define US_SELECTPLOT_H

#include <qlabel.h>
#include <qcheckbox.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qstring.h>
#include "us.h"
#include "us_util.h"

class US_EXTERN US_SelectPlot : public QDialog
{
   Q_OBJECT
   
   public:
      US_SelectPlot(bool *temp_plotFlag, bool *temp_print_bw, bool *temp_print_inv, 
      bool temp_change, QString, QString, QWidget *p=0, const char *name = 0);
      US_SelectPlot(bool *temp_print_bw, bool *temp_print_inv, 
      QWidget *p=0, const char *name = 0);
      ~US_SelectPlot();
      
      bool *plotFlag;
      bool *print_bw;
      bool *print_inv;
      bool change;
      QString plot1_name, plot2_name;
      QLabel *lbl_info;
      QLabel *lbl_analysis;
      QLabel *lbl_edit;
      QLabel *lbl_bw;
      QLabel *lbl_color;
      QLabel *lbl_inv;
      QPushButton *pb_ok;
      QCheckBox *cb_analysis;
      QCheckBox *cb_edit;
      QCheckBox *cb_color;
      QCheckBox *cb_bw;
      QCheckBox *cb_inv;
      US_Config *USglobal;
      
   public slots:
      void check();
      void change_analysis();
      void change_edit();
      void change_color();
      void change_bw();
      void change_inv();

   protected slots:
      void closeEvent(QCloseEvent *);
};

#endif

