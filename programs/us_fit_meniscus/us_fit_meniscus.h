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
      enum { QUADRATIC, CUBIC };

      int           fit_order;

      QLineEdit*    le_fit;

      US_Help       showHelp;
      
      US_Editor*    te_data;

      QwtPlot*      meniscus_plot;
      QwtPlotCurve* raw_curve;
      QwtPlotCurve* fit_curve;
      QwtPlotCurve* minimum_curve;
	
	public slots:

      void fit_type ( int  );
      void plot_data( void );

		void help     ( void )
      { showHelp.show_help( "manual/fit_meniscus.html" ); };
};
#endif
