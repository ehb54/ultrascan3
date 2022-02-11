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

#define UPU_EB_PREFIX "#e#"

class US_EXTERN US_Plot_Util
{
 public:
   static bool printtofile( QString basename,
                            map < QString, QwtPlot *> plots,
                            QString & errors,
                            QString & messages
                            );
   static void eb_handling( 
                           vector < QString >                   titles,
                           vector < vector < double > >         x,
                           vector < vector < double > >         y,
                           map < QString, vector < double > > & e
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
                       bool only_scale_y = true,
                       bool rescale_rounding = false
                       );

   static void rescale(
                       map < QString, QwtPlot *> plots,
                       map < QwtPlot *, ScrollZoomer * > plot_to_zoomer,
                       map < QwtPlot *, double >         plot_limit_x_range_min,
                       map < QwtPlot *, double >         plot_limit_x_range_max,
                       bool only_scale_y = true,
                       bool rescale_rounding = false
                       );

   static void rescale(
                       const vector < QwtPlot * > & plots,
                       map < QwtPlot *, ScrollZoomer * > plot_to_zoomer,
                       bool only_scale_y = true,
                       bool rescale_rounding = false
                       );

   static void rescale(
                       const vector < QwtPlot * > & plots,
                       map < QwtPlot *, ScrollZoomer * > plot_to_zoomer,
                       map < QwtPlot *, double >         plot_limit_x_range_min,
                       map < QwtPlot *, double >         plot_limit_x_range_max,
                       bool only_scale_y = true,
                       bool rescale_rounding = false
                       );

   static void rescale(
                       QwtPlot * plot,
                       ScrollZoomer * zoomer,
                       bool only_scale_y = true,
                       bool rescale_rounding = false
                       );

   static void rescale(
                       QwtPlot * plot,
                       ScrollZoomer * zoomer,
                       double limit_x_range_min,
                       double limit_x_range_max,
                       bool only_scale_y = true,
                       bool rescale_rounding = false
                       );

   static double round_digits(
                              double value,
                              int digits = 2
                              );
                             
};

#endif
