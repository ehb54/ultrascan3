#ifndef US_PRINTFILTER_H
#define US_PRINTFILTER_H

#include <qwt_plot_printfilter.h>
#include "us_extern.h"

class US_EXTERN PrintFilter : public QwtPlotPrintFilter
{
	public:
		PrintFilter();
		~PrintFilter();
		virtual QColor color(const QColor &, Item, int) const;
};

class US_EXTERN PrintFilterDark : public QwtPlotPrintFilter
{
	public:
		PrintFilterDark();
		~PrintFilterDark();
		virtual QColor color(const QColor &, Item, int) const;
};

class US_EXTERN PrintFilterDistro : public QwtPlotPrintFilter
{
	public:
		PrintFilterDistro();
		~PrintFilterDistro();
		virtual QColor color(const QColor &, Item, int) const;
};

#endif
