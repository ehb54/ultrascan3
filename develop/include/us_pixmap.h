#ifndef US_PIXMAP_H
#define US_PIXMAP_H

#include <qpixmap.h>
#include <qstring.h>
#include <qimage.h>
#include <qbitmap.h>
#include <qpainter.h>

class US_Pixmap
{
   public:
   US_Pixmap();
   ~US_Pixmap();
   void save_file(QString, QPixmap);
};

#endif

