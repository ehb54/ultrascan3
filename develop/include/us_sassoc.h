#ifndef US_SASSOC_H
#define US_SASSOC_H

// QT includes:
#include <qwidget.h>
#include <qframe.h>
#include <qpushbt.h>
#include <qfile.h>
#include <qlabel.h>
#include <qtextstream.h>
#include <qfont.h>
#include <qlineedit.h>
#include <qcolor.h>
#include <qpoint.h>
#include <qpixmap.h>
#include <qtextedit.h>
#include <qlayout.h>

// QWT includes:
#include <qwt_counter.h>
#include <qwt_plot.h>

// UltraScan includes:
#include "us_util.h"
#include "us_pixmap.h"
#include "us_math.h"
#include "us_selectplot.h"
#include "us_printfilter.h"

extern int global_Xpos;
extern int global_Ypos;

// C/C++ includes
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <locale.h>


class US_SassocLegend : public QFrame
{
   Q_OBJECT
   
   public:
   
      US_SassocLegend(QWidget *p=0 , const char *name=0);
      ~US_SassocLegend();
      US_Config *USglobal;

      int xpos, buttonh, column1, column2, ypos, spacing, border;
      QLabel *lbl_species1a;
      QLabel *lbl_species2a;
      QLabel *lbl_species3a;
      QLabel *lbl_species4a;
      QLabel *lbl_species1b;
      QLabel *lbl_species2b;
      QLabel *lbl_species3b;
      QLabel *lbl_species4b;
      QLabel *lbl_instructions;
      void setup_GUI();
};

class US_EXTERN US_Sassoc : public QFrame
{
   Q_OBJECT
   
   public:
   
      US_Sassoc(float, float, float, float, QString, int, bool, QWidget *p=0 , const char *name=0);
      ~US_Sassoc();
      US_Config *USglobal;

      QwtPlot *data_plot;
      QLabel *lbl_banner1;
      QLabel *lbl_banner2;
      QLabel *lbl_banner3;
      QLabel *lbl_banner4;
      QLabel *lbl_banner5;
      QLabel *lbl_eq1;
      QLabel *lbl_eq2;
      QLabel *lbl_stoich1;
      QLabel *lbl_stoich2;
      QLabel *lbl_project;
      
      QTextEdit *mle_model;
      QPushButton *pb_cancel;
      QPushButton *pb_write;
      QPushButton *pb_help;
      QPushButton *pb_print;
      QPushButton *pb_recalc;
      QwtCounter *cnt_eq1;
      QwtCounter *cnt_eq2;
      QLineEdit *le_stoich1;
      QLineEdit *le_stoich2;
      QLineEdit *le_eq1;
      QLineEdit *le_eq2;
      QPoint point;
      US_SassocLegend *sas_l;
      US_Pixmap *pm;
      
   private:
   
      int xpos, buttonh, buttonw, ypos, spacing, border, model;
      unsigned int ARRAY_SIZE;
      unsigned int curve[3];
      double **data;
      float eq[2], stoich[2];
      bool status;
      QString project;
      QString htmlDir;
      
   protected slots:
      void setup_GUI();
      void closeEvent(QCloseEvent *);
//      void resizeEvent(QResizeEvent *e);
      void mouseMoved(const QMouseEvent &e);
      void mousePressed(const QMouseEvent &e);
      void mouseReleased(const QMouseEvent &e);
   
   public slots:

      void print();
      void recalc();
      bool createHtmlDir();
      void write_data();
      void help();
      void cancel();
      double monomer_root(double);
      double polynomial(double, double);
      void update_stoich1(const QString &);
      void update_stoich2(const QString &);
      void update_eq1(const QString &);
      void update_eq2(const QString &);
      void update_eq1Count(double);
      void update_eq2Count(double);
};

#endif

