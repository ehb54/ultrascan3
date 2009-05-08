#ifndef US_RTVRUNREQUEST_H
#define US_RTVRUNREQUEST_H

#include <qpushbutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qfiledialog.h>
#include <qtextstream.h>
#include <qfiledialog.h>
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <qsqlcursor.h>
#include <qlistbox.h>

#include "us_db_rtv_investigator.h"
#include "us_db_rtv_sample.h"

class US_EXTERN US_RunRequest : public US_DB
{
   Q_OBJECT
   
   public:
      US_RunRequest(int temp_RRID, QWidget *p = 0, const char *name = "us_runrequest");
      ~US_RunRequest();
      
      QLabel *lbl_description;
      QLabel *lbl_request;
      QLabel *lbl_runtype;
                  
   // for display value
      QLineEdit *le_description;
      QLineEdit *le_request;
      QLineEdit *le_runtype;
      QLineEdit *le_sample1;
      QLineEdit *le_sample2;
      QLineEdit *le_sample3;
      QLineEdit *le_sample4;
      QLineEdit *le_sample5;
      QLineEdit *le_sample6;
      QLineEdit *le_sample7;
      QLineEdit *le_sample8;
   
   //show detail
      QPushButton *pb_sample1;
      QPushButton *pb_sample2;
      QPushButton *pb_sample3;
      QPushButton *pb_sample4;
      QPushButton *pb_sample5;
      QPushButton *pb_sample6;
      QPushButton *pb_sample7;
      QPushButton *pb_sample8;      
      
      QPushButton *pb_help;
      QPushButton *pb_close;
   
      int s1,s2,s3,s4,s5,s6,s7,s8;
      protected slots:
      void load(int);
      void view_sample1();
      void view_sample2();
      void view_sample3();
      void view_sample4();
      void view_sample5();
      void view_sample6();
      void view_sample7();
      void view_sample8();
      void help();
      void quit();
   

};

class US_EXTERN US_DB_RtvRunRequest : public US_DB_RtvInvestigator
{
   Q_OBJECT
   
   public:
      US_DB_RtvRunRequest(QWidget *p=0, const char *name="us_rtvrunrequest");
      ~US_DB_RtvRunRequest();
      
      int *item_RunRequestID;
      int RunRequestID;
   
   public slots:
      void show_runrequest();
   protected slots:
      void checkRunRequest();   
      void check_runrequest(int);
      void select_runrequest(int);    
};

#endif


