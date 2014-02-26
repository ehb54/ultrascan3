#include "../include/us_pixmap.h"


US_Pixmap::US_Pixmap(QWidget* parent, const char* name) : QWidget(parent, name)
{
}

US_Pixmap::~US_Pixmap()
{
}

void US_Pixmap::save_file(QString fileName, QPixmap p)
{
   QImage image;
   image = p.convertToImage();
   QRgb frame_pixel = image.pixel(0, 0);


   QBitmap mask(p.size());
   mask.fill(Qt::color1);

   QPainter pic(&mask);
   pic.setPen(Qt::color0);

   for (int y=0; y<image.height(); y++)
   {
      for ( int x=0; x<image.width(); x++ )
      {
         QRgb rgb = image.pixel(x, y);
         if (rgb == frame_pixel) // we want the frame transparent
         {
            pic.drawPoint( x, y );
         }
      }
   }
   pic.end();
   p.setMask(mask);
   if(!p.save(fileName + "png", "PNG"))
   {
      QMessageBox::information(this, tr("Disk Error:"),
      tr("Please note:\n\nUltraScan is unable to write the image for:\n\n" +
         fileName + ".png\n\nto disk. Please make sure the directory exists!"),
         //1,0,0);
      QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
   }
}

