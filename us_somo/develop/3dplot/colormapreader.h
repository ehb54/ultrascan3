#ifndef __colormapreader_2003_06_08_13_23__
#define __colormapreader_2003_06_08_13_23__

#include <q3frame.h>
#include <qlabel.h>
#include <qpixmap.h>

#include <qwt3d_types.h>


class ColorMapPreview : public Q3Frame, public Q3FilePreview {
   public:
      ColorMapPreview(QWidget *parent = 0);
      void previewUrl(const Q3Url &u);

   private:
      Qwt3D::ColorVector cv;
      QLabel *label_;
      QPixmap pix_;
      bool open(QString);
};


#endif
