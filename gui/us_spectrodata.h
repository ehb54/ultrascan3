//! \file us_spectrodata.h
#ifndef US_SPECDATA_H
#define US_SPECDATA_H

#include <QtCore>
#if QT_VERSION > 0x050000
#include <QtWidgets>
#else
#include <QtGui>
#define setSingleStep(a) setStep(a)
#define setMinorPen(a) setMinPen(a)
#define setMajorPen(a) setMajPen(a)
#define QwtInterval    QwtDoubleInterval
#endif

#include <us_extern.h>
#include <qwt_color_map.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_layout.h>

#define DbgLv(a) if(dbg_level>=a)qDebug()  //!< debug-level-conditioned qDebug()

//! \brief Simple structure for essential values of each solution distribution point
typedef struct solute_s
{
   double s;    //!< sedimentation coefficient (or current X)
   double k;    //!< frictional ratio (or current Y)
   double c;    //!< concentration (Z or Z percent)
   double w;    //!< molecular weight
   double v;    //!< vbar
   double d;    //!< diffusion coefficient
   double f;    //!< frictional coefficient
   double si;   //!< saved initial s
   double ki;   //!< saved initial k
} S_Solute;

//! \brief Class derived from QwtRasterData to supply QwtPlotSpectrogram data
class US_GUI_EXTERN US_SpectrogramData : public QwtRasterData
{
public:

   //! \brief SpectroGram Data constructor
   US_SpectrogramData();
   //! \brief SpectroGram Data constructor
   US_SpectrogramData( QRectF& );

#if QT_VERSION < 0x050000
   //! \brief Return a copy of SpectroGram Data
   virtual QwtRasterData *copy() const;

   //! \brief Return the Z range (minimum,maximum pair)
   virtual QwtDoubleInterval range() const;
#else
   //! \brief Return the Z range (minimum,maximum pair)
   virtual QwtInterval range() const;
#endif

   //! \brief Called to get data ranges and raster size in pixels
   //! \param drect Data rectangle (x-min,y-min,x-range,y-range)
   //! \param rsiz  Raster size (nbr-x-per-scan,nbr-y-scans)
   virtual void initRaster( QRectF&, QSize& );

   //! \brief Fetch the X data range
   //! \return Interval( x-min, x-max )
   QwtInterval xrange();

   //! \brief Fetch the Y data range
   //! \return Interval( y-min, y-max )
   QwtInterval yrange();

   //! \brief Fetch the X/Y/Z range
   //~ \return Interval( min, max )
   virtual QwtInterval interval( Qt::Axis );

   //! \brief Sets up ranges and controls for the raster data to come
   //! \param a_xres   X resolution, the real X extent in pixels.
   //! \param a_yres   Y resolution, the real Y extent in pixels.
   //! \param a_reso   Resolution, the factor used in Gaussian (default=90.0).
   //! \param a_zfloor Floor percent of Z-range to add below Z-minimum.
   //! \param a_drecti Data plot value ranges
   void setRastRanges( double, double, double, double, QRectF );

   //! \brief Set constant Z range for manual scale
   //! \param a_zmin   Z minimum constant value.
   //! \param a_zmax   Z maximum constant value.
   void setZRange( double, double );

   //! \brief Called by QwtPlot to get the Z-value at each X,Y pixel location
   //! \param x  The real X pixel location for which to fetch Z.
   //! \param y  The real Y pixel location for which to fetch Z.
   //! \return   The real Z value for raster location X,Y.
   virtual double value( double, double ) const;

   //! \brief Called by QwtPlot to get the Z-value at each X,Y pixel location
   //! \param x  The X pixel location for which to fetch Z.
   //! \param y  The Y pixel location for which to fetch Z.
   //! \param x_out  The real X value for raster location XY
   //! \param y_out  The real Y value for raster location XY
   //! \param z_out  The real Z value for raster location XY
   void value( int, int, double&, double&, double& ) const;

   //! \brief Sets up the internal raster, based on a set of Solute points.
   //! \param solu  Pointer to list of solution points for the current distribution.
   void setRaster( QList< S_Solute >* );

private:

   QList< double > rdata;        //!< Raster data: z-values at each pixel
   QRectF          drecti;       //!< Data rectangle for x,y plot ranges

   double          xmin;         //!< X minimum
   double          xmax;         //!< X maximum
   double          xrng;         //!< X data range (xmax-xmin)
   double          xinc;         //!< X increment (xreso-1)/xrng
   double          ymin;         //!< Y minimum
   double          ymax;         //!< Y maximum
   double          yrng;         //!< Y data range (ymax-ymin)
   double          yinc;         //!< Y increment (yreso-1)/xrng
   double          zmin;         //!< Z minimum
   double          zmax;         //!< Z maximum
   double          zminr;        //!< Z minimum in raster w floor adjust
   double          xreso;        //!< number of X pixels per scan
   double          yreso;        //!< number of Y scans
   double          resol;        //!< resolution parameter for Gaussian
   double          zfloor;       //!< floor percent below z minimum

   int             nxpsc;        //!< integral number of X pixels per scan
   int             nyscn;        //!< integral number of Y scans
   int             nxypt;        //!< total number of X,Y points in raster
   int             dbg_level;    //!< debug level value for conditional qDebug()
};

#endif
