#ifndef US_PIXMAP_H
#define US_PIXMAP_H

#include <qpixmap.h>
#include <qstring.h>
#include <qimage.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <qmessagebox.h>

#include "us_util.h"

class US_Pixmap : public QWidget
{
   Q_OBJECT
   public:
      US_Pixmap(QWidget * parent=0, const char * name=0);
      ~US_Pixmap();

   public:
      void save_file(QString, QPixmap);
};

#endif

