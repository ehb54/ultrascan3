#ifndef US_EDSCAN_H
#define US_EDSCAN_H

#include <qpushbutton.h>
#include <qlabel.h>
#include <qwt_plot.h>
#include <qwidget.h>
#include <qframe.h>
#include <qpoint.h>
#include <qlayout.h>
#include "us_util.h"
#include "us_math.h"

#include <qwt_math.h>

class edit_single_F : public QFrame
{
   Q_OBJECT
   
   public:
      QPushButton *pb_accept;
      QPushButton *pb_cancel;
      QPushButton *pb_help;
      QPushButton *pb_zoom;
      QLabel *lbl_instruct1;
      QLabel *lbl_instruct2;
      edit_single_F(QWidget *p = 0, const char *name = 0);
      ~edit_single_F();
      US_Config *USglobal;
   
   public slots:
      void help();
      
   signals: 
      void clicked();
};

class edit_single_Win : public QWidget
{
   Q_OBJECT

   private:
      QwtPlot *scan_plt;
      uint scan;
      int points, points2, index;
      double **temp_rad, xval, yval;
      double **temp_abs;
      double *radius;
      double *absorbance;
      double *radius2;
      double *absorbance2;
      bool move_point;
      bool zoom_enabled;
      QPoint p1, p2;
      US_Config *USglobal;

   public:   
      edit_single_F *frm;
      edit_single_Win(double **, double **, int, QWidget *p = 0, const char *name = 0);
      ~edit_single_Win();

   protected:
      //   void resizeEvent(QResizeEvent *e);
      
   signals:
      void clicked();

   private slots:
      void plotMousePressed(const QMouseEvent &e);
      void plotMouseReleased(const QMouseEvent &e);
      void zoom();
      void update_plot();
      void accept();
      void cancel();
      void closeEvent(QCloseEvent *);
};

class US_ZoomFrame : public QFrame
{
   Q_OBJECT
   
   public:
      QPushButton *pb_accept;
      QPushButton *pb_cancel;
      QPushButton *pb_help;
      QPushButton *pb_zoom;
      QPushButton *pb_reset_zoom;
      QLabel *lbl_instruct1;
      QLabel *lbl_instruct2;
      US_ZoomFrame(QWidget *p = 0, const char *name = 0);
      ~US_ZoomFrame();
      US_Config *USglobal;
   
   public slots:
      void help();
   signals: 
      void clicked();
};

class US_ZoomWin : public QWidget
{
   Q_OBJECT

   private:
      QwtPlot *scan_plt;
      uint *scan;
      int points, points2, index, scans;
      double ***temp_rad, xval, yval, xlimit_left, xlimit_right;
      double ***temp_abs;
      double **radius;
      double **absorbance;
      double **radius2;
      double **absorbance2;
      float *radius_val;
      bool move_point;
      bool zoom_enabled;
      QPoint p1, p2;

   public:   
      US_ZoomFrame *frm;
      US_ZoomWin(double ***, double ***, int, int, float *, QWidget *p = 0, const char *name = 0);
      ~US_ZoomWin();
      US_Config *USglobal;

   protected:
      void resizeEvent(QResizeEvent *e);
      
   signals:
      void clicked();

   private slots:
      void plotMousePressed(const QMouseEvent &e);
      void plotMouseReleased(const QMouseEvent &e);
      void zoom();
      void update_plot();
      void accept();
      void cancel();
      void reset_zoom();
};
#endif

