//! \file us_spectrodata.cpp

#include "us_spectrodata.h"
#include "us_defines.h"
#include "us_settings.h"

#define LO_DTERM 0.2500    // low decay-term point (1/4)

// Provides raster data for QwtSpectrogram
US_SpectrogramData::US_SpectrogramData() : QwtRasterData()
{
   rdata.clear();
   xreso    = 300.0;
   yreso    = 300.0;
   resol    = 90.0;
   zfloor   = 100.0;
   nxpsc    = qRound( xreso );
   nyscn    = qRound( yreso );
   nxypt    = nxpsc * nyscn;
   dbg_level = US_Settings::us_debug();
}

#if QT_VERSION < 0x050000
QwtRasterData* US_SpectrogramData::copy() const
{
   US_SpectrogramData* newdat = new US_SpectrogramData;
   newdat->xreso  = xreso;
   newdat->yreso  = yreso;
   newdat->resol  = resol;
   newdat->zfloor = zfloor;
   newdat->nxpsc  = nxpsc;
   newdat->nyscn  = nyscn;
   newdat->nxypt  = nxypt;
   newdat->rdata  = rdata;
   return newdat;
}

QwtDoubleInterval US_SpectrogramData::range() const
{
   return QwtDoubleInterval( zmin, zmax );
}
#else
QwtInterval US_SpectrogramData::range() const
{
   return QwtInterval( zmin, zmax );
}
#endif

// Initialize raster: get x,y ranges and image pixel size
void US_SpectrogramData::initRaster( QRectF& drect, QSize& rsiz )
{
   drect = QRectF( xmin, ymin, xrng, yrng );
   rsiz  = QSize( nxpsc, nyscn );
qDebug() << "specDat: initRas: drect" << drect;
qDebug() << "specDat: initRas: rsiz" << rsiz;
}

// Get x range
QwtInterval US_SpectrogramData::xrange()
{
   return QwtInterval( xmin, xmax );
}

// Get y range
QwtInterval US_SpectrogramData::yrange()
{
   return QwtInterval( ymin, ymax );
}

// Get x or y or z interval
QwtInterval US_SpectrogramData::interval( Qt::Axis axis )
{
   QwtInterval arange = QwtInterval( zmin, zmax );

   switch ( axis )
   {
      case Qt::XAxis:
         arange = xrange();
         break;
      case Qt::YAxis:
         arange = yrange();
         break;
      case Qt::ZAxis:
      default:
         break;
   }

   return arange;
}

// Necessary method to set up raster ranges: resolutions, floor fraction
void US_SpectrogramData::setRastRanges( double a_xres, double a_yres,
          double a_reso, double a_zfloor, QRectF a_drecti )
{
   xreso    = a_xres;
   yreso    = a_yres;
   resol    = a_reso;
   zfloor   = ( a_zfloor - 100.0 ) / 100.0;
   nxpsc    = qRound( xreso );
   nyscn    = qRound( yreso );
   nxypt    = nxpsc * nyscn;
   drecti   = a_drecti;
}

// Set constant Z range for use with manual or looping displays
void US_SpectrogramData::setZRange( double a_zmin, double a_zmax )
{
   zmin     = a_zmin;
   zmax     = a_zmax;
   zminr    = zmin - ( ( zmax - zmin ) * zfloor );
#if QT_VERSION > 0x050000
   setInterval( Qt::ZAxis, QwtInterval( zmin, zmax ) );
#endif
}

// Method called by QwtPlotSpectrogram for each raster point.
// This version gets or interpolates a point from the raster buffer.
double US_SpectrogramData::value(double x, double y) const
{
   double rx  = ( x - xmin ) * xinc;   // real x pixel position
   double ry  = ( ymax - y ) * yinc;   // real y pixel position
   int jx     = (int)rx;               // integral x pixel
   int jy     = (int)ry;               // integral y pixel
   int jr1    = jy * nxpsc + jx;       // overall raster position
   int mxr    = nxypt - 1;             // max raster position index

   if ( jr1 < 0  ||  jr1 > mxr )
   {  // for x,y outside raster, return z-minimum
      return zmin;
   }

   double rv1 = rdata.at( jr1 );       // possible output value

   double dx  = rx - (double)jx;       // fractional part of x
   double dy  = ry - (double)jy;       // fractional part of y

   if ( dx > 0.1  ||  dy > 0.1 )
   {   // if either fraction significant, interpolate output value
      int jr2    = jr1 + 1;            // x+1,y
      int jr3    = jr1 + nxpsc;        //   x,y+1
      int jr4    = jr3 + 1;            // x+1,y+1
      jr2        = ( jr2 > mxr ) ? jr1 : jr2;
      jr3        = ( jr3 > mxr ) ? jr2 : jr3;
      jr4        = ( jr4 > mxr ) ? jr3 : jr4;
      double rv2 = rdata.at( jr2 );    // surrounding input values
      double rv3 = rdata.at( jr3 );
      double rv4 = rdata.at( jr4 );

      // interpolate between the four points
      rv2        = ( dx * rv2 ) + ( ( 1.0 - dx ) * rv1 );
      rv4        = ( dx * rv4 ) + ( ( 1.0 - dx ) * rv3 );
      rv1        = ( dy * rv4 ) + ( ( 1.0 - dy ) * rv2 );
   }

   return rv1;   // return raster value or interpolated value
}

// Set up raster values from solution distribution set.
void US_SpectrogramData::setRaster( QList< S_Solute >* solu )
{
   double xval;
   double yval;
   double zval;

   int nsol    = solu->size();          // number Solute points
   if ( nsol < 1 )
      return;

   xmin        = drecti.left ();
   xmax        = drecti.right();

   if ( xmin == xmax )
   {  // Auto-limits:  calculate x,y ranges
      xmin        = solu->at( 0 ).s;    // initial minima,maxima
      ymin        = solu->at( 0 ).k;
      zmin        = solu->at( 0 ).c;
      xmax        = xmin;
      ymax        = ymin;
      zmax        = zmin;
zmin=0.0;

      // scan solute distribution for ranges

      for ( int ii = 1; ii < nsol; ii++ )
      {
         xval    = solu->at( ii ).s;
         yval    = solu->at( ii ).k;
         zval    = solu->at( ii ).c; 
         xmin    = qMin( xval, xmin );
         xmax    = qMax( xval, xmax );
         ymin    = qMin( yval, ymin );
         ymax    = qMax( yval, ymax );
         zmin    = qMin( zval, zmin );
         zmax    = qMax( zval, zmax );
      }

      xrng    = xmax - xmin;          // initial ranges and pixel/data ratios
      xinc    = ( xreso - 1.0 ) / xrng;
      yrng    = ymax - ymin;
      if ( yrng == 0.0 )
      {
         yrng    = 0.02;
         ymin    = (double)( qFloor( ymin * 100.0 ) ) * 0.01;
         ymax    = ymin + yrng;
      }
      yinc    = ( yreso - 1.0 ) / yrng;

      xmin   -= ( 4.0 / xinc );       // adjust for padding, then recalc ranges
      xmax   += ( 4.0 / xinc );
      ymin   -= ( 4.0 / yinc );
      ymax   += ( 4.0 / yinc );
   }

   else
   {  // Given x,y ranges
      ymin    = drecti.top   ();
      ymax    = drecti.bottom();
   }

   if ( nsol == 1 )
   {
      zmin   *= zfloor;
      xmin   *= 0.90;
      xmax   *= 1.10;
      ymin   *= 0.90;
      ymax   *= 1.10;
   }

   xrng    = xmax - xmin;
   yrng    = ymax - ymin;
   xinc    = ( xreso - 1.0 ) / xrng;
   yinc    = ( yreso - 1.0 ) / yrng;
   zminr   = zmin - ( ( zmax - zmin ) * zfloor );

   // Set bounding rectangle for raster plot
#if QT_VERSION < 0x050000
   setBoundingRect( QRectF( xmin, ymin, xrng, yrng ) );
#else
qDebug() << "sRaDa: setBounding... xmin xmax" << xmin << xmax;
qDebug() << "sRaDa: setBounding... ymin ymax" << ymin << ymax;
qDebug() << "sRaDa: setBounding... zminr zmax" << zminr << zmax;
   setInterval( Qt::XAxis, QwtInterval( xmin, xmax ) );
   setInterval( Qt::YAxis, QwtInterval( ymin, ymax ) );
   setInterval( Qt::ZAxis, QwtInterval( zmin, zmax ) );
#endif

   // Initialize raster to zmin (zero)
   rdata.clear();

   for ( int ii = 0; ii < nxypt; ii++ )
   {
      rdata.append( 0.0 );
   }

   // Populate raster with z values derived from a Gaussian distribution
   //  around each distribution point.

   double ssig  = xrng / ( 2.0 * resol );         // sigma values
   double fsig  = yrng / ( 2.0 * resol );
   double sssc  = -1.0 / ( 4.0 * ssig * ssig );   // sig scale factors
   double fssc  = -1.0 / ( 4.0 * fsig * fsig );

   // calculate radius of decay-to-zero in Gaussian distribution
   double dmin  = sqrt( -log( 1.0e-5 ) ) * 2.0;    // terms very low
   double xdif  = dmin * ssig * xinc;
   double ydif  = dmin * fsig * yinc;
   int    nxd   = qRound( xdif ) + 2;              // radius points with pad
   int    nyd   = qRound( ydif ) + 2;
   int    hixd  = nxpsc / 4;                       // maximum radii
   int    hiyd  = nyscn / 4;
   nxd          = ( nxd < 10 ) ? 10 : ( ( nxd > hixd ) ? hixd : nxd );
   nyd          = ( nyd < 10 ) ? 10 : ( ( nyd > hiyd ) ? hiyd : nyd );

   if ( resol != 100.0 )
   {
      for ( int kk = 0; kk < nsol; kk++ )
      {   // spread z values for each distribution point
         xval    = solu->at( kk ).s;                   // x,y,z
         yval    = solu->at( kk ).k;
         zval    = solu->at( kk ).c - zminr;           // use z in 0,zrng range
         zval    = qMax( 0.0, zval );

         int rx  = (int)( ( xval - xmin ) * xinc );    // x index of this point
         int fx  = rx - nxd;                           // first reasonable x
         int lx  = rx + nxd;                           // last reasonable x
         fx      = ( fx > 0 )     ? fx : 0;
         lx      = ( lx < nxpsc ) ? lx : nxpsc;
         int ry  = (int)( ( ymax - yval ) * yinc );    // y index of this point
         int fy  = ry - nyd;                           // first reasonable y
         int ly  = ry + nyd;                           // last reasonable y
         fy      = ( fy > 0 )     ? fy : 0;
         ly      = ( ly < nyscn ) ? ly : nyscn;

         for ( int ii = fy; ii < ly; ii++ )
         {   // calculate y-term and x range for each y
            double yras   = ymax - ( (double)ii / yinc );
            double ydif   = yras - yval;
            double yterm  = exp( ydif * ydif * fssc ); // y term
            double zterm  = zval * yterm;              // combine z-value,y-term
            int kr        = ii * nxpsc + fx;           // start rast point index

            for ( int jj = fx; jj < lx; jj++ )
            {  // calculate x-term then calculate z-value for raster point
               double xras   = (double)jj / xinc + xmin;
               double xdif   = xras - xval;
               double xterm  = exp( xdif * xdif * sssc );

               double zin    = rdata.at( kr );         // current value there

               // Output value according to Gaussian distribution factor.
               // Note that the expression below adds zmin back in to a
               // value that is really:
               //   zval * exp( -pow( xdif, 2.0 ) / pow( 2 * ssigma, 2.0 ) )
               //        * exp( -pow( ydif, 2.0 ) / pow( 2 * fsigma, 2.0 ) )
               double zout   = zmin + zterm * xterm;

               // only replace input if new value is greater
               if ( zout > zin )
                  rdata.replace( kr, zout );

               kr++;                                   // bump rast point index
            }
         }
      }
   }
   else
   {   // for resolution=100, make all points in circle have zval value
      for ( int kk = 0; kk < nsol; kk++ )
      {   // spread z values for each distribution point
         xval    = solu->at( kk ).s;                   // x,y,z
         yval    = solu->at( kk ).k;
         zval    = solu->at( kk ).c;

         int rx  = (int)( ( xval - xmin ) * xinc );    // x index of this point
         int fx  = rx - nxd;                           // first reasonable x
         int lx  = rx + nxd;                           // last reasonable x
         fx      = ( fx > 0 )     ? fx : 0;
         lx      = ( lx < nxpsc ) ? lx : nxpsc;
         int ry  = (int)( ( ymax - yval ) * yinc );    // y index of this point
         int fy  = ry - nyd;                           // first reasonable y
         int ly  = ry + nyd;                           // last reasonable y
         fy      = ( fy > 0 )     ? fy : 0;
         ly      = ( ly < nyscn ) ? ly : nyscn;

         for ( int ii = fy; ii < ly; ii++ )
         {   // calculate y-term and x range for each y
            double yras   = ymax - ( (double)ii / yinc );
            double ydif   = yras - yval;
            double yterm  = exp( ydif * ydif * fssc ); // y term
            int kr        = ii * nxpsc + fx;           // start rast point index

            for ( int jj = fx; jj < lx; jj++ )
            {  // calculate x-term then calculate z-value for raster point
               double xras   = (double)jj / xinc + xmin;
               double xdif   = xras - xval;
               double xterm  = exp( xdif * xdif * sssc );

               double zin    = rdata.at( kr );         // current value there
               double zdecay = yterm * xterm;          // composite decay term

               // replace input if within distr radius and zout>zin
               if ( zdecay > LO_DTERM  &&  zval > zin)
                  rdata.replace( kr, zval );

               kr++;                                   // bump rast point index
            }
         }
      }
   }
qDebug() << "SD:sRaDa: RETURN:";
}

void US_SpectrogramData::value( int x, int y, double &x_out, double &y_out, double &z_out ) const {
   x_out  = (double)x / xinc + xmin ;   // real x pixel position
   y_out = ymax - (double)y / yinc;   // real y pixel position
   int jx     = (int)x;               // integral x pixel
   int jy     = (int)y;               // integral y pixel
   int jr1    = jy * nxpsc + jx;       // overall raster position
   int mxr    = nxypt - 1;             // max raster position index

   if ( jr1 < 0  ||  jr1 > mxr )
   {  // for x,y outside raster, return z-minimum
      z_out = 0.0;
      return;
   }

   z_out = rdata.at( jr1 );       // possible output value
}

