#ifndef US_2DPLOT_H
#define US_2DPLOT_H

#include <qwt_plot.h>
#include "us_extern.h"
class US_EXTERN US_2dPlot : public QwtPlot
{
	Q_OBJECT
	
	public:
		US_2dPlot(QWidget *p=0, const char *name="us_2dplot");
		~US_2dPlot();
				
	protected slots:
		void closeEvent(QCloseEvent *);

	signals:
		void plotClosed();
};

#endif

