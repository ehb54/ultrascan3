#ifndef US_PLOT_ZOOM_H
#define US_PLOT_ZOOM_H

#include <qstack.h>
#include <qstring.h>
#include <qwt_plot.h>

#include <QRectF>

#include "qwt/scrollzoomer.h"
#include "us_extern.h"

using namespace std;

class US_EXTERN US_Plot_Zoom {
 public:
  US_Plot_Zoom(QwtPlot* plot, ScrollZoomer* zoomer);

  QString info(const QString& msg = "");
  void save();
  void restore(bool replot = false);

 private:
  QwtPlot* plot;
  ScrollZoomer* zoomer;
  uint currentIndex;
  QStack<QRectF> stack;
};

#endif
