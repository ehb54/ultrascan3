//! \file us3i_gui_util.cpp
#include <QtSvg>

#include "us3i_gui_util.h"
#include "us3i_gzip.h"
#include "us3i_settings.h"
#include "qwt_plot_renderer.h"

// Save SVG+PNG or PNG file
int US3i_GuiUtil::save_plot( const QString& filename, const QwtPlot* plot )
{
   int status = 0;

   if ( filename.contains( ".svg" ) )
   {  // Save the file as SVG, then save a PNG version
      save_svg( filename, plot );

      QString fnamepng  = QString( filename ).section( ".", 0, -2 ) + ".png";
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
int US3i_GuiUtil::save_svg( const QString& filename, QwtPlot* plot )
{
   int status = 0;

   if ( filename.contains( ".svg", Qt::CaseInsensitive ) ) {
      // Normalize extension to .svg
      const QString fnsvg = QString( filename ).section( '.', 0, -2 ) + ".svg";

      // --- Screen DPI (Qt5 & Qt6) ---
      QScreen* screen = QGuiApplication::primaryScreen();
      // Sensible default if no screen (headless) or info missing
      int dpi = 96;

      if ( screen ) {
         // Try to reproduce the old "px / inches" logic when physical size is available
         const double px_width = static_cast<double>( screen->geometry().width() );     // pixels
         const double mm_width = screen->physicalSize().width();                        // millimeters
         if ( mm_width > 0.0 ) {
            const double inches = mm_width / 25.4;
            if ( inches > 0.0 ) {
               dpi = qRound( px_width / inches );
            }
         }
         else
         {
            // Fall back to logical DPI reported by Qt
            dpi = qRound( screen->logicalDotsPerInch() );
         }
      }

      // Render using the current plot widget size (keeps behavior consistent with your Qt5 code)
      const QSizeF psize = plot->size();   // widget size; Qwt scales via 'dpi' below

      QwtPlotRenderer renderer;
      renderer.renderDocument( plot, fnsvg, psize, dpi );

      // Compress to SVGZ
      US3i_Gzip gz;
      gz.gzip( fnsvg );
   }
   else
   {
      status = 1; // not an .svg filename
   }

   return status;
}

// Const-friendly wrapper to preserve existing call sites
int US3i_GuiUtil::save_svg( const QString& filename, const QwtPlot* plot )
{
   return US3i_GuiUtil::save_svg( filename,
                                  const_cast<QwtPlot*>( plot ) );
}


// Save PNG file
int US3i_GuiUtil::save_png( const QString& filename, const QwtPlot* plot )
{
   int status = 0;


   if ( filename.endsWith( ".png" ) )
   {  // Save the file as a PNG version
      QPixmap pixmap = ((QWidget*)plot)->grab();
      if ( ! pixmap.save( filename ) )
         status = 2;
   }

   else
   {  // Mark error:  filename does not end with ".png"
      status = 1;
   }

   return status;
}

