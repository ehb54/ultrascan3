#ifndef US_2DPLOT_H
#define US_2DPLOT_H

#include <QtGui>
#include <QApplication>

#include "us_extern.h"
#include "us_widgets.h"
#include "us_plot.h"

class US_2dPlot : public US_Widgets
{
	Q_OBJECT

	public:
  		US_2dPlot();

	private:

      QVector <double>   x, y;

      double             resolution;
      double             s_min;
      double             s_max;
      double             f_min;
      double             f_max;
      double             par1;
      double             par2;
      double             par3;
      double             par4;
      int		 model;

      QPushButton*       pb_calculate;
      QPushButton*       pb_close;

      QLabel*            lbl_par1;
      QLabel*            lbl_par2;
      QLabel*            lbl_par3;
      QLabel*            lbl_par4;
      QLabel*            lbl_resolution;
      QLabel*            lbl_model;
            
      QwtPlot*           data_plot;
      QwtPlotCurve*      f_curve;
      QwtCounter*        ct_par1;
      QwtCounter*        ct_par2;
      QwtCounter*        ct_par3;
      QwtCounter*        ct_par4;
      QwtCounter*        ct_resolution;
      QwtCounter*        ct_model;
      

      US_Plot*           plot;
      
   private slots:
      void       calculate         ( void );
      void       update_par1       ( double );
      void       update_par2       ( double );
      void       update_par3       ( double );
      void       update_par4       ( double );
      void       update_resolution ( double );
      void       update_model      ( double );
};
#endif
