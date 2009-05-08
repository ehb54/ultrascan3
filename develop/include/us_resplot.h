#ifndef US_RESPLOT_H
#define US_RESPLOT_H


#include <iostream>
#include <qwidget.h>
#include <qpainter.h>
#include <qapplication.h>
#include <stdlib.h>
#include <qpushbutton.h>
#include <vector>
#include "us_mfem.h"


class US_ResidualPlot : public QWidget
{
   Q_OBJECT

   public:
       US_ResidualPlot(int, int, mfem_data *, QWidget *parent=0, const char *name=0 );
       US_ResidualPlot(int, int, double **, unsigned int, unsigned int, QWidget *parent=0, const char *name=0 );
       US_ResidualPlot(QWidget *parent=0, const char *name=0 );
      ~US_ResidualPlot();

   private:

      double **residuals;
      unsigned int scans, points;
      
   public slots:
      void setData(struct mfem_data *, int, int);
      void setData(double **, int, int, unsigned int, unsigned int);

   protected: 
      void paintEvent(QPaintEvent *);
};

#endif

