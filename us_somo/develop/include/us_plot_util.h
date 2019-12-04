#ifndef US_PLOT_UTIL_H
#define US_PLOT_UTIL_H

#include <map>
#include <vector>
#include <set>
#include "us_file_util.h"
#include "us_vector.h"
#include <qwt_plot.h>
#include "us3i_plot.h"
#include "qwt/scrollbar.h"
#include "qwt/scrollzoomer.h"

#include <qwt_plot_curve.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>

using namespace std;

class US_EXTERN US_Plot_Util
{
 public:
   static bool printtofile( QString basename,
                            map < QString, QwtPlot *> plots,
                            QString & errors,
                            QString & messages
                            );
   static void plotinfo(
                        QString  name,
                        QwtPlot * plot
                        );
   static void align_plot_extents(
                                  const vector < QwtPlot * > & plots,
                                  bool scale_x_to_first = false
                                  );
   static void rescale(
                       map < QString, QwtPlot *> plots,
                       map < QwtPlot *, ScrollZoomer * > plot_to_zoomer,
                       bool only_scale_y = true
                       );

   static void rescale(
                       const vector < QwtPlot * > & plots,
                       map < QwtPlot *, ScrollZoomer * > plot_to_zoomer,
                       bool only_scale_y = true
                       );

   static void rescale(
                       QwtPlot * plot,
                       ScrollZoomer * zoomer,
                       bool only_scale_y = true
                       );
};

#endif
