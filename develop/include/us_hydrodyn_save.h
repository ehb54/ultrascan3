#ifndef US_HYDRODYN_SAVE_H
#define US_HYDRODYN_SAVE_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qwt_counter.h>
#include <qbuttongroup.h>
#include <qtextedit.h>
#include <qprogressbar.h>
#include <qmenubar.h>
#include <qfileinfo.h>
#include <qprinter.h>
#include <qlistbox.h>
#include <qtabwidget.h>

#include "us_util.h"

//standard C and C++ defs:

#include <map>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace std;

struct save_info 
{
   QString file;
   vector < QString >    field;
   map < QString, bool > field_flag;
};

class US_EXTERN US_Hydrodyn_Save : public QFrame
{
   Q_OBJECT

   public:
      US_Hydrodyn_Save(save_info *save, 
                        bool *save_widget, 
                        void *us_hydrodyn, 
                        QWidget *p = 0, 
                        const char *name = 0);
      ~US_Hydrodyn_Save();

   private:

      save_info                     *save;
      bool                          *save_widget;
      void                          *us_hydrodyn;

      US_Config                     *USglobal;

      QLabel                        *lbl_possible;
      QLabel                        *lbl_selected;


      QTabWidget                    *tw_possible;
      QListBox                      *lb_selected;

      QPushButton                   *pb_add;
      QPushButton                   *pb_remove;

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif
      vector < QListBox * >         lb_possible;

      vector < QString >            field;
      vector < QString >            descriptive_name;
      vector < QString >            short_name;
      map < QString, unsigned int > field_to_pos;
      map < QString, unsigned int > descriptive_name_to_pos;
      map < QString, QString >      section_name;
      map < QString, bool >         row_break;
      map < QString, int >          descriptive_name_to_section;

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

      QPushButton   *pb_help;
      QPushButton   *pb_cancel;

   private slots:
      
      void setupGUI();

      void add();
      void remove();

      void rebuild();

      void tab_changed(QWidget *);

      void update_enables_selected();
      void update_enables_possible();

      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
   
};

#endif
