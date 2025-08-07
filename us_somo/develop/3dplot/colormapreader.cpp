#include <fstream>

#include <q3filedialog.h>
#include <qfileinfo.h>
#include <qlayout.h>
#include <qpainter.h>
//Added by qt3to4:
#include <Q3Frame>
#include <Q3GridLayout>
#include <QLabel>
#include <iostream>
#include "colormapreader.h"

using namespace Qwt3D;
using namespace std;

ColorMapPreview::ColorMapPreview(QWidget *parent) : Q3Frame(parent) {
   label_ = new QLabel(this);
   setFrameShape(Q3Frame::StyledPanel);
   setFrameShadow(Q3Frame::Sunken);
   Q3GridLayout *layout = new Q3GridLayout(this, 1, 1, 10);

   layout->addWidget(label_, 0, 0, Qt::AlignJustify);
}

void ColorMapPreview::previewUrl(const Q3Url &u) {
   QString path = u.path();
   QFileInfo fi(path);
   if (fi.extension() != "map" && fi.extension() != "MAP")
      label_->setText("No color map");
   else {
      if (open(path))
         label_->setPixmap(pix_);
   }
}

bool ColorMapPreview::open(QString fname) {
   ifstream file(( const char * ) fname.local8Bit());

   RGBA rgb;
   cv.clear();

   while (file) {
      file >> rgb.r >> rgb.g >> rgb.b;
      file.ignore(10000, '\n');
      if (!file.good())
         break;
      else {
         rgb.a = 1;
         rgb.r /= 255;
         rgb.g /= 255;
         rgb.b /= 255;
         //std::cout << "new color\n";
         cv.push_back(rgb);
      }
   }

   pix_.resize(80, cv.size());
   QPainter p(&pix_);
   p.translate(0, cv.size() - 1);
   for (unsigned i = 0; i != cv.size(); ++i) {
      rgb = cv[ i ];
      p.setPen(GL2Qt(rgb.r, rgb.g, rgb.b));
      p.drawLine(QPoint(0, 0), QPoint(pix_.width(), 0));
      p.translate(0, -1);
   }
   p.end();

   return true;
}
