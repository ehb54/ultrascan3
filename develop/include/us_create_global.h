#ifndef US_CREATE_GLOBAL_H
#define US_CREATE_GLOBAL_H

#include <vector>
#include <iostream>
#include <qframe.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qstring.h>
#include <qfile.h>
#include <qfiledlg.h>
#include <qlayout.h>
#include "us_util.h"


extern int global_Xpos;
extern int global_Ypos;

struct single_distro
{
   QString name;

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

   vector <QString> line;

#ifdef WIN32
#pragma warning ( default: 4251 )
#endif
};

class US_EXTERN US_CreateGlobal : public QFrame
{
   Q_OBJECT

   public:
      US_CreateGlobal( QWidget *p=0, const char *name=0 );
      ~US_CreateGlobal();

   private:

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

      vector <struct single_distro> distro;
      
#ifdef WIN32
#pragma warning ( default: 4251 )
#endif

    QLabel *lbl_info;
      QListBox *lb_distro;
      QPushButton *pb_add;
      QPushButton *pb_reset;
      QPushButton *pb_help;
      QPushButton *pb_save;
      QPushButton *pb_quit;
      US_Config *USglobal;
      unsigned int iterations;

   protected slots:
      void setup_GUI();
      void closeEvent(QCloseEvent *e);

   private slots:
      void add();
      void reset();
      void help();
      void save();
      void remove(int);
};

#endif

