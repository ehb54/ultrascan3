#ifndef US_HYDRODYN_CLUSTER_ADDITIONAL_H
#define US_HYDRODYN_CLUSTER_ADDITIONAL_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qtextedit.h>
#include <qmenubar.h>
#include <qprinter.h>

#include "us_util.h"

//standard C and C++ defs:

#include <map>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace std;

class US_EXTERN US_Hydrodyn_Cluster_Additional : public QDialog
{
   Q_OBJECT

   public:
      US_Hydrodyn_Cluster_Additional(
                                     void                         * us_hydrodyn, 
                                     QWidget                      * p = 0, 
                                     const char                   * name = 0
                                     );
      ~US_Hydrodyn_Cluster_Additional();

   private:
      void          *us_hydrodyn;
      void          *cluster_window;

      US_Config     *USglobal;

      QLabel        *lbl_title;

      QCheckBox     *cb_bfnb;
      QPushButton   *pb_bfnb;
      QCheckBox     *cb_bfnb_nsa;
      QPushButton   *pb_bfnb_nsa;
      QCheckBox     *cb_csa;
      QPushButton   *pb_csa;
      QCheckBox     *cb_dammin;
      QPushButton   *pb_dammin;
      QCheckBox     *cb_dammif;
      QPushButton   *pb_dammif;
      QCheckBox     *cb_gasbor;
      QPushButton   *pb_gasbor;

      QPushButton   *pb_ok;
      QPushButton   *pb_help;
      QPushButton   *pb_cancel;

#ifdef Q_WS_WIN
  #pragma warning ( disable: 4251 )
#endif
      map < QString, bool >                     options_active;
      map < QString, map < QString, QString > > options_selected;
#ifdef Q_WS_WIN
  #pragma warning ( default: 4251 )
#endif

      void update_enables();

   private slots:

      void setupGUI();

      void set_bfnb();
      void bfnb();
      void set_bfnb_nsa();
      void bfnb_nsa();
      void set_csa();
      void csa();
      void set_dammin();
      void dammin();
      void set_dammif();
      void dammif();
      void set_gasbor();
      void gasbor();

      void ok();
      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
   
};

#endif
