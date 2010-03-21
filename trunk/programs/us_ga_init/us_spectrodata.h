//! \file us_spectrodata.h
#ifndef US_SPECDATA_H
#define US_SPECDATA_H

#include <QtCore>

#include <qwt_color_map.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_layout.h>

//! \brief Structure for values of each solution distribution point
typedef struct solute_s
{
   double s;
   double k;
   double c;
} Solute;

//! \brief Class derived from QwtRasterData to supply QwtPlotSpectrogram data
class US_SpectrogramData: public QwtRasterData
{

public:

   US_SpectrogramData();
   US_SpectrogramData( QwtDoubleRect& );

   virtual QwtRasterData *copy() const;

   virtual QwtDoubleInterval range() const;

   /*! \brief Called to get data ranges and raster size in pixels
       \param drect Data rectangle (x-min,y-min,x-range,y-range)
       \param rsiz  Raster size (nbr-x-per-scan,nbr-y-scans)
   */
   virtual void initRaster( QwtDoubleRect&, QSize& );

   /*! \brief Fetch the X data range
       \return Interval( x-min, x-max )
   */
   QwtDoubleInterval xrange();

   /*! \brief Fetch the Y data range
       \return Interval( y-min, y-max )
   */
   QwtDoubleInterval yrange();

   /*! \brief Sets up ranges and controls for the raster data to come
       \param a_xres   X resolution, the real X extent in pixels.
       \param a_yres   Y resolution, the real Y extent in pixels.
       \param a_reso   Resolution, the factor used in Gaussian (default=90.0).
       \param a_zfloor Floor percent of Z-range to add below Z-minimum.
   */
   void setRastRanges( double, double, double, double );

   /*! \brief Called by QwtPlot to get the Z-value at each X,Y pixel location
       \param x  The real X pixel location for which to fetch Z.
       \param y  The real Y pixel location for which to fetch Z.
       \return   The real Z value for raster location X,Y.
   */
   virtual double value( double, double ) const;

   /*! \brief Sets up the internal raster, based on a set of Solute points.
       \param solu  The list of solution points for the current distribution.
   */
   void setRaster( QList< Solute >* );

private:

   QList< double > rdata;        // Raster data: z-values at each pixel

   double          xmin;         // X minimum
   double          xmax;         // X maximum
   double          xrng;         // X data range (xmax-xmin)
   double          xinc;         // X increment (xreso-1)/xrng
   double          ymin;         // Y minimum
   double          ymax;         // Y maximum
   double          yrng;         // Y data range (ymax-ymin)
   double          yinc;         // Y increment (yreso-1)/xrng
   double          zmin;         // Z minimum
   double          zmax;         // Z maximum
   double          zrng;         // Z data values range
   double          xreso;        // number of X pixels per scan
   double          yreso;        // number of Y scans
   double          resol;        // resolution parameter for Gaussian
   double          zfloor;       // floor percent below z minimum

   int             nxpsc;        // integral number of X pixels per scan
   int             nyscn;        // integral number of Y scans
   int             nxypt;        // total number of X,Y points in raster
};

#endif
