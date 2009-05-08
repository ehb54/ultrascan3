#include <fstream>

#include <qfileinfo.h>
#include <qfiledialog.h>
#include <qlayout.h>
#include <qpainter.h>
#include <iostream>
#include "colormapreader.h"

using namespace Qwt3D;
using namespace std;

ColorMapPreview::ColorMapPreview( QWidget *parent ) 
   : QFrame( parent ) 
{
   label_ = new QLabel(this);
   setFrameShape( QFrame::StyledPanel );
   setFrameShadow( QFrame::Sunken );
   QGridLayout* layout = new QGridLayout( this, 1, 1, 10); 

   layout->addWidget( label_, 0, 0, Qt::AlignJustify );
}

void ColorMapPreview::previewUrl( const QUrl &u )
{
   QString path = u.path();
   QFileInfo fi( path );
   if (fi.extension() != "map" && fi.extension() != "MAP")
      label_->setText( "No color map" );
   else
   {
      if ( open(path) )
         label_->setPixmap( pix_ );
   }
}

bool ColorMapPreview::open(QString fname)
{   
   ifstream file((const char*)fname.local8Bit());
   
   RGBA rgb;
   cv.clear();
   
   while ( file ) 
   {      
      file >> rgb.r >> rgb.g >> rgb.b;
      file.ignore(10000,'\n');
      if (!file.good())
         break;
      else
      {
         rgb.a = 1;
         rgb.r /= 255;
         rgb.g /= 255;
         rgb.b /= 255;
         //std::cout << "new color\n";
         cv.push_back(rgb);   
      }
   }

   pix_.resize(80, cv.size());
   QPainter p( &pix_ );
   p.translate( 0, cv.size()-1 );
   for (unsigned i=0; i!=cv.size(); ++i)
   {
      rgb = cv[i];
      p.setPen(GL2Qt(rgb.r,rgb.g,rgb.b));
      p.drawLine(QPoint(0,0),QPoint(pix_.width(),0));   
      p.translate( 0, -1 );
   }
   p.end();

   return true;
}
