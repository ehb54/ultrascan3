#ifndef US_HYDRODYN_SAS_OPTIONS_GUINIER_H
#define US_HYDRODYN_SAS_OPTIONS_GUINIER_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qwt_counter.h>
#include <qbuttongroup.h>

#include "us_util.h"

//standard C and C++ defs:

#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace std;

class US_EXTERN US_Hydrodyn_SasOptionsGuinier : public QFrame
{
   Q_OBJECT

   public:
      friend class US_Hydrodyn;

      US_Hydrodyn_SasOptionsGuinier(struct saxs_options *, bool *, void *, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_SasOptionsGuinier();

   public:
      struct saxs_options *saxs_options;

      bool       *sas_options_guinier_widget;
      void       *us_hydrodyn;
      US_Config  *USglobal;

      QLabel     *lbl_info;

      QCheckBox  *cb_guinier_csv;
      QLineEdit  *le_guinier_csv_filename;

      QLabel     *lbl_qRgmax;
      QLabel     *lbl_qstart;
      QLabel     *lbl_qend;
      QLabel     *lbl_pointsmin;
      QLabel     *lbl_pointsmax;

      QwtCounter *cnt_qRgmax;
      QwtCounter *cnt_qstart;
      QwtCounter *cnt_qend;
      QwtCounter *cnt_pointsmin;
      QwtCounter *cnt_pointsmax;

      QPushButton *pb_help;
      QPushButton *pb_cancel;

   private slots:
      
      void setupGUI();

      void set_guinier_csv();
      void update_guinier_csv_filename(const QString &);
      void update_qRgmax(double);
      void update_qstart(double);
      void update_qend(double);
      void update_pointsmin(double);
      void update_pointsmax(double);

      void cancel();
      void help();
   
   protected slots:

      void closeEvent(QCloseEvent *);
};
#endif

