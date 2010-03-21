#ifndef US_MENISCUS_H
#define US_MENISCUS_H


#include <qframe.h>
#include <qlabel.h>
#include <qtextedit.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qrect.h>
#include <qregexp.h>
#include <qwt_counter.h>
#include <qwt_plot.h>
#include "us_util.h"
#include "us_math.h"
#include "us_matrix.h"

class US_EXTERN US_Meniscus : public QFrame
{
   Q_OBJECT
   
   public:
   
      US_Meniscus(QWidget *p = 0, const char *name = 0);
      ~US_Meniscus();
      
   private:
      unsigned int order, fit_points;
      QLabel *lbl_minimum;
      QLabel *lbl_rmsd;
      QLabel *lbl_order;
      QwtCounter *ct_order;
      QTextEdit *editor;
      QwtPlot *plot_meniscus;
      QLineEdit *le_minimum;
      QLineEdit *le_rmsd;
      QPushButton *pb_close;
      QPushButton *pb_plot;
      QPushButton *pb_reset;
      QPushButton *pb_help;
      US_Config *USglobal;

   private slots:
      void setupGUI();
      void update_order(double);
      void update_plot();
      void reset();
      void help();
      int find_index(double, int, int, double *);
      
};


#endif

