//! \file us_sim_optics.cpp
#include "us_sim_optics.h"
#include "us_math2.h"

#include <QtMath>

#include <algorithm>
#include <cmath>

namespace
{
// Shared resize for both transforms: values on xvalues, no stddevs, nothing
// interpolated.
void resizeScans( US_DataIO::RawData& data, int points, double delta_r )
{
   for ( US_DataIO::Scan& scan : data.scanData )
   {
      scan.rvalues.resize( points );
      scan.stddevs.clear();
      scan.nz_stddev = false;
      scan.delta_r   = delta_r;
      scan.interpolated.fill( '\0', ( points + 7 ) / 8 );
   }
}

// Linear interpolation on ascending xs, clamped at both ends.
double interpolate( const QVector<double>& xs, const QVector<double>& ys, double x )
{
   if ( x <= xs.first() )
   {
      return ys.first();
   }
   if ( x >= xs.last() )
   {
      return ys.last();
   }
   const int    hi = static_cast<int>( std::upper_bound( xs.begin(), xs.end(), x ) - xs.begin() );
   const int    lo = hi - 1;
   const double f  = ( x - xs[lo] ) / ( xs[hi] - xs[lo] );
   return ys[lo] + ( ys[hi] - ys[lo] ) * f;
}

// A uniform deviate on (0, 1), drawn from the noise stream's normal deviates.
double uniformDeviate()
{
   return 0.5 * std::erfc( -US_Math2::box_muller( 0.0, 1.0 ) / std::sqrt( 2.0 ) );
}
}

QString US_SimOptics::intensityProblem( const Intensity& settings )
{
   if ( !( settings.source_counts > 0.0 ) || !std::isfinite( settings.source_counts ) )
   {
      return QString( "intensity source_counts must be a number greater than 0, got %1" ).arg( settings.source_counts );
   }
   if ( !( settings.read_noise >= 0.0 ) || !( settings.shot_noise >= 0.0 ) || !( settings.dark >= 0.0 ) )
   {
      return QString( "intensity read_noise, shot_noise and dark must be 0 or more" );
   }
   if ( settings.dark >= settings.source_counts )
   {
      return QString( "intensity dark (%1) must be below source_counts (%2)" ).arg( settings.dark ).arg( settings.source_counts );
   }
   return QString();
}

QString US_SimOptics::windowProblem( const Window& settings, double meniscus, double bottom )
{
   if ( !( settings.resolution > 0.0 ) || !( settings.end > settings.start ) )
   {
      return QString( "window needs start < end and a resolution greater than 0" );
   }
   if ( !( settings.meniscus_artifact >= 0.0 ) )
   {
      return QString( "window meniscus_artifact must be 0 or more" );
   }
   if ( meniscus <= settings.start || bottom >= settings.end )
   {
      return QString( "window %1-%2 cm does not hold the cell, meniscus %3 to bottom %4 cm" )
         .arg( settings.start )
         .arg( settings.end )
         .arg( meniscus )
         .arg( bottom );
   }
   return QString();
}

QVector<double> US_SimOptics::windowRadii( const Window& settings )
{
   const int       points = qRound( ( settings.end - settings.start ) / settings.resolution );
   QVector<double> radii( qMax( points, 0 ) );
   for ( int jj = 0; jj < radii.size(); jj++ )
   {
      radii[jj] = settings.start + jj * settings.resolution;
   }
   return radii;
}

double US_SimOptics::sourceFactor( double wavelength )
{
   // A xenon flash lamp falls off toward 180 nm; floor it so the far UV is
   // dim but not dark.
   return qMax( 0.02, 1.0 - std::exp( -( wavelength - 180.0 ) / 30.0 ) );
}

double US_SimOptics::sourceIntensity( const Intensity& settings, double radius, double wavelength )
{
   double profile = 1.0;
   if ( settings.curved )
   {
      const double u = ( radius - profileCenter ) / profileHalfWidth;
      profile        = 1.0 - profileCurvature * u * u;
   }
   return settings.source_counts * sourceFactor( wavelength ) * profile;
}

double US_SimOptics::countsSigma( const Intensity& settings, double counts )
{
   return std::sqrt( settings.read_noise * settings.read_noise + settings.shot_noise * qMax( counts, 0.0 ) );
}

double US_SimOptics::recordedCounts( const Intensity& settings, double counts, double deviate )
{
   const double level = qMax( counts, settings.dark );
   return qMax( 0.0, level + deviate * countsSigma( settings, level ) );
}

double US_SimOptics::bufferOD( const US_Buffer& buffer, double wavelength, double pathlength )
{
   const QMap<double, double>& spectrum = buffer.extinction;
   if ( spectrum.isEmpty() || wavelength < spectrum.firstKey() || wavelength > spectrum.lastKey() )
   {
      return 0.0;
   }
   QVector<double> xs;
   QVector<double> ys;
   for ( auto it = spectrum.constBegin(); it != spectrum.constEnd(); ++it )
   {
      xs << it.key();
      ys << it.value();
   }
   return interpolate( xs, ys, wavelength ) * pathlength;
}

char US_SimOptics::referenceChannel( char sample )
{
   switch ( sample )
   {
      case 'A':
         return 'B';
      case 'C':
         return 'D';
      case 'E':
         return 'F';
      case 'G':
         return 'H';
      default:
         return 0;
   }
}

void US_SimOptics::toWindow( US_DataIO::RawData& data,
                             const Window&       settings,
                             double              meniscus,
                             double              bottom,
                             double              loading,
                             double              air_sigma,
                             double              liquid_offset,
                             double              past_bottom,
                             double              ceiling )
{
   const QVector<double> source = data.xvalues;
   const QVector<double> radii  = windowRadii( settings );
   const int             points = radii.size();

   // The first window point at or past the meniscus.
   int                   menisc = 0;
   while ( menisc < points && radii[menisc] < meniscus )
   {
      menisc++;
   }

   for ( US_DataIO::Scan& scan : data.scanData )
   {
      const QVector<double> values = scan.rvalues;
      QVector<double>       window( points );

      for ( int jj = 0; jj < points; jj++ )
      {
         const double r = radii[jj];
         if ( r < meniscus )
         {
            window[jj] = ( air_sigma > 0.0 ) ? US_Math2::box_muller( 0.0, air_sigma ) : 0.0;
         }
         else if ( r <= bottom )
         {
            window[jj] = interpolate( source, values, r ) + liquid_offset;
         }
         else
         {
            window[jj] = past_bottom;
         }
      }

      // The meniscus refracts light out of the beam: a spike a few points
      // wide, slightly different in every scan.
      if ( settings.meniscus_artifact > 0.0 && loading > 0.0 )
      {
         const double height = settings.meniscus_artifact * loading * ( 1.0 + 0.15 * qAbs( US_Math2::box_muller( 0.0, 1.0 ) ) );
         const int    half   = 1 + qMin( 2, static_cast<int>( qAbs( US_Math2::box_muller( 0.0, 1.0 ) ) ) );
         for ( int kk = -half; kk <= half; kk++ )
         {
            const int jj = menisc + kk;
            if ( jj >= 0 && jj < points && radii[jj] <= bottom )
            {
               window[jj] += height * ( 1.0 - qAbs( kk ) / ( half + 1.0 ) );
            }
         }
      }

      if ( ceiling > 0.0 )
      {
         for ( double& value : window )
         {
            value = qMin( value, ceiling );
         }
      }

      scan.rvalues = window;
   }

   data.xvalues = radii;
   resizeScans( data, points, settings.resolution );
}

void US_SimOptics::toIntensity( const US_DataIO::RawData& absorbance,
                                const Intensity&          settings,
                                double                    meniscus,
                                double                    bottom,
                                double                    buffer_od,
                                US_DataIO::RawData&       sample,
                                US_DataIO::RawData&       reference )
{
   sample            = absorbance;
   reference         = absorbance;
   reference.channel = referenceChannel( absorbance.channel );

   const QVector<double>& xs = absorbance.xvalues;
   const int              np = xs.size();
   const double           bf = std::pow( 10.0, -buffer_od );

   for ( int js = 0; js < absorbance.scanData.size(); js++ )
   {
      const US_DataIO::Scan& scan = absorbance.scanData[js];

      for ( int jj = 0; jj < np; jj++ )
      {
         const double r  = xs[jj];
         const double i0 = sourceIntensity( settings, r, scan.wavelength );
         double       is = 0.0;
         double       ir = 0.0;

         if ( r < meniscus )
         { // Air: both sectors see the source.
            is = i0;
            ir = i0;
         }
         else if ( r <= bottom )
         {
            ir = i0 * bf;
            is = ir * std::pow( 10.0, -scan.rvalues[jj] );
         }

         // Sample first, then reference, point by point: a fixed draw order.
         sample.scanData[js].rvalues[jj]    = recordedCounts( settings, is, US_Math2::box_muller( 0.0, 1.0 ) );
         reference.scanData[js].rvalues[jj] = recordedCounts( settings, ir, US_Math2::box_muller( 0.0, 1.0 ) );
      }
   }

   resizeScans( sample, np, sample.scanData.isEmpty() ? 0.0 : sample.scanData[0].delta_r );
   resizeScans( reference, np, reference.scanData.isEmpty() ? 0.0 : reference.scanData[0].delta_r );
}

QString US_SimOptics::interferenceProblem( const Interference& settings )
{
   if ( settings.points < 2 || !( settings.resolution > 0.0 ) )
   {
      return "interference needs at least 2 points and a resolution greater than 0";
   }
   if ( !( settings.scale > 0.0 ) )
   {
      return "interference scale must be greater than 0";
   }
   if ( !( settings.random >= 0.0 ) || !( settings.offset >= 0.0 ) || !( settings.structure >= 0.0 ) ||
        !( settings.meniscus_artifact >= 0.0 ) )
   {
      return "interference random, offset, structure and meniscus_artifact must be 0 or more";
   }
   if ( !( settings.jump_rate >= 0.0 && settings.jump_rate <= 1.0 ) ||
        !( settings.step_fraction >= 0.0 && settings.step_fraction <= 1.0 ) )
   {
      return "interference jump_rate and step_fraction must be between 0 and 1";
   }
   return QString();
}

QString US_SimOptics::interferenceGeometryProblem( const Interference& settings, double meniscus, double bottom )
{
   const QVector<double> radii = interferenceRadii( settings );
   if ( radii.isEmpty() || !( radii.first() < meniscus && radii.last() > meniscus ) )
   {
      return QString( "interference grid %1-%2 cm does not hold the meniscus at %3 cm (cell to %4 cm)" )
         .arg( radii.value( 0 ) )
         .arg( radii.isEmpty() ? 0.0 : radii.last() )
         .arg( meniscus )
         .arg( bottom );
   }
   return QString();
}

QVector<double> US_SimOptics::interferenceRadii( const Interference& settings )
{
   QVector<double> radii( qMax( settings.points, 0 ) );
   for ( int jj = 0; jj < radii.size(); jj++ )
   {
      radii[jj] = settings.start + jj * settings.resolution;
   }
   return radii;
}

void US_SimOptics::toInterference( US_DataIO::RawData& data,
                                   const Interference& settings,
                                   double              meniscus,
                                   double              bottom,
                                   double              loading,
                                   double              ceiling,
                                   QVector<double>&    structure,
                                   QVector<double>&    offsets )
{
   const QVector<double> source = data.xvalues;
   const QVector<double> radii  = interferenceRadii( settings );
   const int             points = radii.size();

   int                   menisc = 0;
   while ( menisc < points && radii[menisc] < meniscus )
   {
      menisc++;
   }

   // A fixed radial structure: a few slow waves across the window, scaled to
   // the requested spread. Shared by every scan, and by every speed step of a
   // run once it has been drawn.
   const bool drawn = structure.size() == points;
   if ( !drawn )
   {
      structure.fill( 0.0, points );
   }
   if ( !drawn && settings.structure > 0.0 )
   {
      const double span = radii.last() - radii.first();
      for ( int kk = 0; kk < 6; kk++ )
      {
         const double period    = span * ( 0.05 + 0.45 * uniformDeviate() );
         const double phase     = 2.0 * M_PI * uniformDeviate();
         const double amplitude = US_Math2::box_muller( 0.0, 1.0 );
         for ( int jj = 0; jj < points; jj++ )
         {
            structure[jj] += amplitude * std::sin( 2.0 * M_PI * ( radii[jj] - radii.first() ) / period + phase );
         }
      }
      double sum = 0.0, sum2 = 0.0;
      for ( double value : structure )
      {
         sum += value;
         sum2 += value * value;
      }
      const double mean   = sum / points;
      const double spread = std::sqrt( qMax( sum2 / points - mean * mean, 1e-30 ) );
      for ( double& value : structure )
      {
         value = ( value - mean ) * settings.structure / spread;
      }
   }

   offsets.clear();
   for ( US_DataIO::Scan& scan : data.scanData )
   {
      const QVector<double> values = scan.rvalues;
      QVector<double>       fringes( points );
      for ( int jj = 0; jj < points; jj++ )
      {
         const double r = radii[jj];
         if ( r < meniscus )
         {
            fringes[jj] = 0.0;
         }
         else if ( r <= bottom )
         {
            fringes[jj] = settings.scale * interpolate( source, values, r );
         }
         else
         {
            fringes[jj] = ( ceiling > 0.0 ) ? ceiling : settings.scale * values.last();
         }
      }

      // The meniscus: a spike a few points wide, above the plateau.
      if ( settings.meniscus_artifact > 0.0 && loading > 0.0 )
      {
         const double height =
            settings.meniscus_artifact * settings.scale * loading * ( 1.0 + 0.15 * qAbs( US_Math2::box_muller( 0.0, 1.0 ) ) );
         const int half = 2 + qMin( 3, static_cast<int>( qAbs( US_Math2::box_muller( 0.0, 1.0 ) ) * 2.0 ) );
         for ( int kk = -half; kk <= half; kk++ )
         {
            const int jj = menisc + kk;
            if ( jj >= 0 && jj < points )
            {
               fringes[jj] += height * ( 1.0 - qAbs( kk ) / ( half + 1.0 ) );
            }
         }
      }

      // Per-scan baseline offset, and now and then a whole-fringe jump.
      double offset  = US_Math2::box_muller( 0.0, settings.offset );
      int    step_at = points;
      double step    = 0.0;
      if ( uniformDeviate() < settings.jump_rate )
      {
         const double size = ( uniformDeviate() < 0.5 ? 1.0 : 2.0 ) * ( uniformDeviate() < 0.5 ? -1.0 : 1.0 );
         if ( uniformDeviate() < settings.step_fraction )
         {
            step_at = static_cast<int>( uniformDeviate() * points );
            step    = size;
         }
         else
         {
            offset += size;
         }
      }
      offsets << offset;

      for ( int jj = 0; jj < points; jj++ )
      {
         double value =
            fringes[jj] + structure[jj] + offset + ( jj >= step_at ? step : 0.0 ) + US_Math2::box_muller( 0.0, settings.random );
         if ( ceiling > 0.0 )
         {
            value = qMin( value, ceiling );
         }
         fringes[jj] = value;
      }
      scan.rvalues = fringes;
   }

   data.xvalues = radii;
   resizeScans( data, points, settings.resolution );
}

QString US_SimOptics::wavelengthScanProblem( const WavelengthScan& settings, double meniscus, double bottom )
{
   if ( !( settings.step > 0.0 ) || !( settings.end > settings.start ) )
   {
      return "wavelength_scan needs start < end and a step greater than 0";
   }
   if ( settings.spectrum.isEmpty() )
   {
      return "wavelength_scan needs a spectrum: <point wavelength=... extinction=...> elements";
   }
   if ( settings.start < settings.spectrum.firstKey() || settings.end > settings.spectrum.lastKey() )
   {
      return QString( "wavelength_scan %1-%2 nm is outside its spectrum, %3-%4 nm" )
         .arg( settings.start )
         .arg( settings.end )
         .arg( settings.spectrum.firstKey() )
         .arg( settings.spectrum.lastKey() );
   }
   double peak = 0.0;
   for ( double value : extinctionAt( settings, scanWavelengths( settings ) ) )
   {
      peak = qMax( peak, value );
   }
   if ( !( peak > 0.0 ) )
   {
      return "wavelength_scan spectrum has no positive extinction over the scan";
   }
   if ( meniscus > 0.0 && !( settings.position > meniscus && settings.position < bottom ) )
   {
      return QString( "wavelength_scan position %1 cm is outside the cell, %2-%3 cm" )
         .arg( settings.position )
         .arg( meniscus )
         .arg( bottom );
   }
   return QString();
}

QVector<double> US_SimOptics::scanWavelengths( const WavelengthScan& settings )
{
   QVector<double> wavelengths;
   const int       count = static_cast<int>( std::floor( ( settings.end - settings.start ) / settings.step + 1e-9 ) ) + 1;
   for ( int jj = 0; jj < count; jj++ )
   {
      wavelengths << settings.start + jj * settings.step;
   }
   return wavelengths;
}

QVector<double> US_SimOptics::extinctionAt( const WavelengthScan& settings, const QVector<double>& wavelengths )
{
   QVector<double> xs;
   QVector<double> ys;
   for ( auto it = settings.spectrum.constBegin(); it != settings.spectrum.constEnd(); ++it )
   {
      xs << it.key();
      ys << it.value();
   }
   QVector<double> values;
   for ( double wl : wavelengths )
   {
      values << ( xs.isEmpty() ? 0.0 : interpolate( xs, ys, wl ) );
   }
   return values;
}

void US_SimOptics::toWavelengthScan( const US_DataIO::RawData& absorbance,
                                     const Intensity&          intensity,
                                     const WavelengthScan&     settings,
                                     const QVector<double>&    buffer_od,
                                     US_DataIO::RawData&       sample,
                                     US_DataIO::RawData&       reference )
{
   const QVector<double> wavelengths = scanWavelengths( settings );
   const QVector<double> epsilon     = extinctionAt( settings, wavelengths );
   double                peak        = 0.0;
   for ( double value : epsilon )
   {
      peak = qMax( peak, value );
   }

   sample            = absorbance;
   reference         = absorbance;
   reference.channel = referenceChannel( absorbance.channel );
   sample.xvalues    = wavelengths;
   reference.xvalues = wavelengths;
   const int count   = wavelengths.size();

   for ( int js = 0; js < absorbance.scanData.size(); js++ )
   {
      const US_DataIO::Scan& scan = absorbance.scanData[js];
      const double           a    = interpolate( absorbance.xvalues, scan.rvalues, settings.position );
      QVector<double>        is( count );
      QVector<double>        ir( count );
      for ( int jj = 0; jj < count; jj++ )
      {
         const double i0     = sourceIntensity( intensity, settings.position, wavelengths[jj] );
         const double buffer = jj < buffer_od.size() ? buffer_od[jj] : 0.0;
         const double shape  = peak > 0.0 ? epsilon[jj] / peak : 0.0;
         is[jj] = recordedCounts( intensity, i0 * std::pow( 10.0, -( a * shape + buffer ) ), US_Math2::box_muller( 0.0, 1.0 ) );
         ir[jj] = recordedCounts( intensity, i0 * std::pow( 10.0, -buffer ), US_Math2::box_muller( 0.0, 1.0 ) );
      }
      sample.scanData[js].rvalues       = is;
      reference.scanData[js].rvalues    = ir;
      sample.scanData[js].wavelength    = settings.position;
      reference.scanData[js].wavelength = settings.position;
   }

   resizeScans( sample, count, settings.step );
   resizeScans( reference, count, settings.step );
}
