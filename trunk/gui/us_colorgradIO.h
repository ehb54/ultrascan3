#ifndef US_COLORGRIO_H
#define US_COLORGRIO_H

#include <QtGui>
#include <QtXml>

class US_ColorGradIO
{
   public:

      static const int XFS_OK    = 0;
      static const int XFS_EROPN = 1;
      static const int XFS_ERXML = 2;
      static const int XFS_ERCST = 3;

      static int read_color_gradient( QString, QList< QColor >& );
};

#endif
