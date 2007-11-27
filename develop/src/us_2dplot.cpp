#include "../include/us_2dplot.h"

US_2dPlot::US_2dPlot(QWidget *p, const char *name) : QwtPlot( p, name)
{
}

US_2dPlot::~US_2dPlot()
{
}

void US_2dPlot::closeEvent(QCloseEvent *e)
{
	emit plotClosed();
	e->accept();
}

