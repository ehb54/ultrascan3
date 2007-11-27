#include "../include/us_printfilter.h"


PrintFilter::PrintFilter() : QwtPlotPrintFilter()
{
}

PrintFilter::~PrintFilter()
{
}

QColor PrintFilter::color(const QColor &c, Item item, int) const
{
	QColor c2 = c;
	switch(item)
	{
		case MajorGrid:
			return Qt::gray;
		case MinorGrid:
			return Qt::lightGray;
		case CanvasBackground:
			return Qt::white;

		default:;
	}
	c2.setRgb(0, 0, 0); // print everything in black
	return c2;
}


PrintFilterDark::PrintFilterDark() : QwtPlotPrintFilter()
{
}

PrintFilterDark::~PrintFilterDark()
{
}

QColor PrintFilterDark::color(const QColor &c, Item item, int) const
{
	QColor c2;
	int h, s, v;
	c.hsv(&h, &s, &v);
	
	if (h == 60)
	{
		h = 240;	// turn yellow into blue
		v = 150;
		s = 255;
	}
	if (h == 180)
	{
		h = 0;	// turn cyan into red
		v = 255;
		s = 100;
	}
	switch(item)
	{
		case MajorGrid:
			return Qt::gray;
		case MinorGrid:
			return Qt::lightGray;
		case Title:
			return Qt::black;
		case AxisScale:
			return Qt::black;
		case AxisTitle:
			return Qt::black;
		case Legend:
			return Qt::black;
		case CanvasBackground:
			return Qt::white;

		default:;
	}
	c2.setHsv(h, s, v); // print everything darker
	return c2;
}

PrintFilterDistro::PrintFilterDistro() : QwtPlotPrintFilter()
{
}

PrintFilterDistro::~PrintFilterDistro()
{
}

QColor PrintFilterDistro::color(const QColor &c, Item item, int) const
{
	QColor c2;
	int r, g, b;
	c.rgb(&r, &g,  &b);
	
	if (r == 255 && g == 255 && b == 255) // white
	{
		return Qt::black;
	}
	if (r == 255 && g == 255 && b == 0) // yellow
	{
		return Qt::blue;
	}
	if (r == 0 && g == 255 && b == 255) // cyan
	{
		c2.setRgb(120, 0 ,150); // purple
		return c2; // purple
	}
	switch(item)
	{
		case MajorGrid:
			return Qt::gray;
		case MinorGrid:
			return Qt::lightGray;
		case Title:
			return Qt::black;
		case AxisScale:
			return Qt::black;
		case AxisTitle:
			return Qt::black;
		case Legend:
			return Qt::black;
		case CanvasBackground:
			return Qt::white;
		default:;
	}
	if (r > 150)
	{
		r -= 70;
	}
	if (g > 150)
	{
		g -= 70;
	}
	if (b > 150)
	{
		b -= 70;
	}
	c2.setRgb(r, g, b);
	return c2;
}
