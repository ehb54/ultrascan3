#ifndef US_FIT_MENISCUS_H
#define US_FIT_MENISCUS_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_editor.h"
#include "us_plot.h"

class US_EXTERN US_FitMeniscus : public US_Widgets
{
	Q_OBJECT

	public:
		US_FitMeniscus();

	private:
      QLineEdit*    le_fit;
      QLineEdit*    le_rms_error;

      US_Help       showHelp;
      
      US_Editor*    te_data;

      QSpinBox*     sb_order;

      QwtPlot*      meniscus_plot;
      QwtPlotCurve* raw_curve;
      QwtPlotCurve* fit_curve;
      QwtPlotCurve* minimum_curve;
	
	public slots:
      void plot_data( void );
      void plot_data( int );

		void help     ( void )
      { showHelp.show_help( "manual/fit_meniscus.html" ); };
};
#endif
