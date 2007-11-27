#include "../include/us_pixmap.h"


US_Pixmap::US_Pixmap()
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
	p.save(fileName + "png", "PNG");
}

