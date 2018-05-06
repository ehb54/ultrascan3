#ifndef US_PLOT_UTIL_H
#define US_PLOT_UTIL_H

#include <map>
#include <vector>
#include <set>
#include "us_file_util.h"
#include "us_vector.h"
#include <qwt_plot.h>
#include "us3i_plot.h"
#if QT_VERSION >= 0x040000
# include <qwt_plot_curve.h>
#endif

using namespace std;
#ifdef WIN32
# if QT_VERSION < 0x040000
  #pragma warning ( disable: 4251 )
# endif
#endif


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
};

#ifdef WIN32
# if QT_VERSION < 0x040000
  #pragma warning ( default: 4251 )
# endif
#endif



#endif
