//! \file us_gui_util.cpp
#include <QtSvg>

#include "us_gui_util.h"

// Save SVG+PNG or PNG file
int US_GuiUtil::save_plot( const QString& filename, const QwtPlot* plot )
{
   int status = 0;

   if ( filename.endsWith( ".svg" ) )
   {  // Save the file as SVG, then save a PNG version
      save_svg( filename, plot );

      QString fnamepng  = QString( filename ).replace( ".svg", ".png" );
      save_png( fnamepng, plot );
   }

   else if ( filename.endsWith( ".png" ) )
   {  // Save the file as PNG
      save_png( filename, plot );
   }

   else
   {  // Flag an error:  file name does not end in ".svg" or ".png"
      status = 1;
   }

   return status;
}

// Save SVG file
int US_GuiUtil::save_svg( const QString& filename, const QwtPlot* plot )
{
   int status = 0;

   if ( filename.endsWith( ".svg" ) )
   {  // Save the file as SVG
      QSvgGenerator generator;

      // Set resolution to screen resolution
      double px  = (double)qApp->desktop()->width();
      double in  = (double)qApp->desktop()->widthMM() / 25.4;
      int    res = qRound( px / in );
      int    pw  = plot->width()  + res;
      int    ph  = plot->height() + res;

      generator.setResolution( res );
      generator.setFileName  ( filename );
      generator.setSize      ( plot->size() );
      generator.setViewBox   ( QRect( QPoint( 0, 0 ), QPoint( pw, ph ) ) );

      plot->print( generator );
   }

   else
   {
      status = 1;
   }

   return status;
}

// Save PNG file
int US_GuiUtil::save_png( const QString& filename, const QwtPlot* plot )
{
   int status = 0;


   if ( filename.endsWith( ".png" ) )
   {  // Save the file as a PNG version
      int pw  = plot->width();
      int ph  = plot->height();

      QPixmap pixmap = QPixmap::grabWidget( (QWidget*)plot, 0, 0, pw, ph );
      if ( ! pixmap.save( filename ) )
         status = 2;
   }

   else
   {  // Mark error:  filename does not end with ".png"
      status = 1;
   }

   return status;
}

