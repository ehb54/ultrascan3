#ifndef US_HISTOGRAM_H
#define US_HISTOGRAM_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_globeq_data.h"
#include "us_plot.h"

//! \brief Simple Dialog to display a histogram plot

class US_EqHistogram : public US_WidgetsDialog
{
	Q_OBJECT
	
	public:
		US_EqHistogram( double, QVector< EqScanFit >&,
         QWidget* = 0, Qt::WindowFlags = 0 );

	private:
      double                 od_limit;  // OD Limit value
      QVector< EqScanFit >&  scanfits;  // Scan fits vector

      US_Plot*    hplot;      // Histogram plot layout
      QwtPlot*    hist_plot;  // The histogram plot

   private slots:
};
#endif

