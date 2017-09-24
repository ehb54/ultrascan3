#ifndef US_PLOT_UTIL_H
#define US_PLOT_UTIL_H

#include <map>
#include <vector>
#include <set>
#include "us_file_util.h"
#include "us_vector.h"
#include <qwt_plot.h>

using namespace std;
#ifdef WIN32
# if !defined( QT4 )
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
};

#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( default: 4251 )
# endif
#endif



#endif
