#ifndef US_COLORGRADIENT_H
#define US_COLORGRADIENT_H

#include "us_extern.h"
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qcolor.h>
#include <qcolordialog.h>
#include <qstring.h>
#include <qcombobox.h>
#include <qevent.h>
#include <qframe.h>
#include <qlayout.h>
#include <qtranslator.h>
#include <qwt_counter.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qwidget.h>
#include "us_util.h"
#include <math.h>

struct GradientColorStep
{
   QColor color;
   unsigned int points;
};

struct Gradient
{
   QColor startcolor;
   vector <struct GradientColorStep> step;
};

class US_EXTERN US_GradientWidget : public QLabel
{
   Q_OBJECT

   public:
      US_GradientWidget(vector <QColor> *, QWidget *p=0, const char *name = 0);
      ~US_GradientWidget();
      vector <QColor> *color_array;

   public slots:
      virtual void paintEvent(QPaintEvent *);
};

class US_EXTERN US_ColorGradient : public QWidget
{
   Q_OBJECT

   public:
      US_ColorGradient(QWidget *p=0, const char *name = 0);
      ~US_ColorGradient();
      QLabel *lbl_banner1;
      QLabel *lbl_startcolor;
      QLabel *lbl_colors;
      QLabel *lbl_step;
      QLabel *lbl_points;
      QLabel *lbl_stepcolor;
      US_GradientWidget *lbl_gradient;
      QPushButton *pb_startcolor;
      QPushButton *pb_stepcolor;
      QPushButton *pb_save;
      QPushButton *pb_help;
      QPushButton *pb_reset;
      QPushButton *pb_close;
      QwtCounter *cnt_colors;
      QwtCounter *cnt_step;
      QwtCounter *cnt_points;
      struct Gradient gradient;
      unsigned int current_step;

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

      vector <QColor> color_array;

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

      US_Config *USglobal;

   public slots:
      void select_startcolor();
      void update_colors(double);
      void update_step(double);
      void update_points(double);
      void update_stepcolor();
      void update_array();
      void help();
      void reset();
      void save();
      void closeEvent(QCloseEvent *);
};

#endif
