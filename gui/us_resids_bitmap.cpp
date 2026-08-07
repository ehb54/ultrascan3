//! \file us_resids_bitmap.cpp

#include "us_resids_bitmap.h"
#include "us_settings.h"
#include "us_gui_settings.h"

#include <qwt_legend.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_matrix_raster_data.h>
#include <qwt_color_map.h>
#include <qwt_scale_widget.h>

// constructor:  residuals bitmap widget
US_ResidsBitmap::US_ResidsBitmap( QVector< QVector< double > >& resids,
   QWidget* wparent )
   : US_WidgetsDialog( wparent, Qt::WindowFlags() )
{
   setWindowTitle( tr( "Residuals Pixel Map" ) );
   setPalette( US_GuiSettings::frameColor() );

   QGridLayout* main = new QGridLayout( this );
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );

   US_Plot* usplot = new US_Plot( plot, tr( "Residuals Pixel Map" ),
                                  tr( "Value Index" ), tr( "Scan Index" ) );
   main->addLayout( usplot, 0, 0 );
   lb_bitmap         = new QLabel();      // will draw to a label

   main->addWidget( lb_bitmap );
   spectrogram = new QwtPlotSpectrogram();
   spectrogram->attach( plot );

   replot( resids );

   setAttribute( Qt::WA_DeleteOnClose, true );
}

void US_ResidsBitmap::replot( QVector< QVector< double > >& resids )
{
   // get standard deviation of residuals
   int    scanCount  = resids.size();
   int    valCount   = resids[ 0 ].size();
   double sigma      = 0.0;
   double kfactor    = 2.0;

   for ( int ii = 0; ii < scanCount; ii++ )
   {  // get sum of squares for sigma (std.dev.)
      for ( int jj = 0; jj < valCount; jj++ )
      {
         sigma    += pow( resids[ ii ][ jj ], 2.0 );
      }
   }

   sigma          = sqrt( sigma / (double)( scanCount * valCount ) );
   sigma         *= kfactor;    // standard deviation times k-factor
   int    red     = 0;
   int    green   = 0;
   int    blue    = 0;
   int    kres;
   double vres;

   QImage res_image( valCount, scanCount, QImage::Format_ARGB32 );
   // Build flat vector for QwtMatrixRasterData
   QVector< double > values;
   values.reserve( scanCount * valCount );
   for ( int ii = 0; ii < scanCount; ii++ )
   {
      for ( int jj = 0; jj < valCount; jj++ )
      {
         values.append( resids[ ii ][ jj ] );
         vres      = resids[ ii ][ jj ];
         kres      = qRound( 255.0 * vres / sigma );

         if ( kres >= 0  )
         {  // positive: scaled redish or red
            red       = kres < 255 ? kres : 255;
            green     = 0;
         }

         else
         {  // negative: scaled greenish or green
            kres      = -kres;
            red       = 0;
            green     = kres < 255 ? kres : 255;
         }

         // set pixel
         res_image.setPixel( jj, ii, QColor( red, green, blue ).rgb() );
      }
   }

   int    mwid    = valCount  < 600 ? valCount  : 600;
   int    mhgt    = scanCount < 100 ? scanCount : 100;

   // create the label pixmap from the image
   resize( mwid + 4, mhgt + 4 );
   lb_bitmap->resize( mwid, mhgt );
   QPixmap pmap( mwid, mhgt );
   pmap = pmap.fromImage( res_image.scaled( mwid, mhgt ) );

   lb_bitmap->setPixmap( pmap );

   QwtMatrixRasterData* data = new QwtMatrixRasterData();
   data->setValueMatrix( values, valCount );
   data->setInterval( Qt::XAxis, QwtInterval( 0, valCount ) );
   data->setInterval( Qt::YAxis, QwtInterval( 0, scanCount ) );
   data->setInterval( Qt::ZAxis, QwtInterval( -sigma, sigma ) );

   spectrogram->setData( data );

   // Setup color map: Green for negative, Black for zero, Red for positive
   QwtLinearColorMap* colorMap1 = new QwtLinearColorMap( Qt::green, Qt::red );
   colorMap1->addColorStop( 0.5, Qt::black );
   spectrogram->setColorMap( colorMap1 );

   // Enable the color scale axis
   plot->setAxisTitle( QwtPlot::yRight, tr( "Residual" ) );
   plot->enableAxis( QwtPlot::yRight );
   QwtScaleWidget* rightAxis = plot->axisWidget( QwtPlot::yRight );
   rightAxis->setColorBarEnabled( true );

   QwtLinearColorMap* colorMap2 = new QwtLinearColorMap( Qt::green, Qt::red );
   colorMap2->addColorStop( 0.5, Qt::black );
   rightAxis->setColorMap( QwtInterval( -sigma, sigma ), colorMap2 );

   plot->setAxisScale( QwtPlot::xBottom, 0, valCount );
   plot->setAxisScale( QwtPlot::yLeft,   0, scanCount );
   plot->setAxisScale( QwtPlot::yRight, -sigma, sigma );

   plot->replot();
}

