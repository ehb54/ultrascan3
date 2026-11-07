//! \file test_us_sim_optics.cpp
#include "qt_test_base.h"
#include "us_math2.h"
#include "us_sim_optics.h"
#include "us_simparms.h"

#include <QFile>
#include <QTemporaryDir>

#include <cmath>

namespace
{
// An absorbance dataset on a meniscus-start grid: value( r ) at every point.
US_DataIO::RawData absorbance( double meniscus, double bottom, int scans, double ( *value )( double ) )
{
   US_DataIO::RawData data;
   data.type[0]     = 'R';
   data.type[1]     = 'I';
   data.cell        = 1;
   data.channel     = 'A';
   const int points = qRound( ( bottom - meniscus ) / 0.001 ) + 1;
   for ( int jj = 0; jj < points; jj++ )
   {
      data.xvalues << meniscus + jj * 0.001;
   }
   for ( int js = 0; js < scans; js++ )
   {
      US_DataIO::Scan scan;
      scan.temperature = 20.0;
      scan.rpm         = 45000.0;
      scan.seconds     = 100.0 * ( js + 1 );
      scan.omega2t     = 0.0;
      scan.wavelength  = 280.0;
      scan.plateau     = 0.0;
      scan.delta_r     = 0.001;
      scan.nz_stddev   = false;
      for ( double r : data.xvalues )
      {
         scan.rvalues << value( r );
      }
      scan.interpolated.fill( '\0', ( points + 7 ) / 8 );
      data.scanData << scan;
   }
   return data;
}

double plateau( double )
{
   return 0.8;
}

US_SimOptics::Intensity quiet()
{
   US_SimOptics::Intensity settings;
   settings.read_noise = 0.0;
   settings.shot_noise = 0.0;
   settings.dark       = 0.0;
   return settings;
}
}

TEST( US_SimOpticsTest, WindowMatchesTheInstrumentGrid )
{
   const QVector<double> radii = US_SimOptics::windowRadii( US_SimOptics::Window() );
   ASSERT_EQ( radii.size(), 1500 );
   EXPECT_DOUBLE_EQ( radii.first(), 5.75 );
   EXPECT_NEAR( radii.last(), 7.249, 1e-9 );
}

TEST( US_SimOpticsTest, WindowMustHoldTheCell )
{
   const US_SimOptics::Window window;
   EXPECT_TRUE( US_SimOptics::windowProblem( window, 5.87, 7.2 ).isEmpty() );
   EXPECT_FALSE( US_SimOptics::windowProblem( window, 5.7, 7.2 ).isEmpty() );
   EXPECT_FALSE( US_SimOptics::windowProblem( window, 5.87, 7.3 ).isEmpty() );

   US_SimOptics::Window backwards;
   backwards.end = backwards.start;
   EXPECT_FALSE( US_SimOptics::windowProblem( backwards, 5.87, 7.2 ).isEmpty() );
}

TEST( US_SimOpticsTest, IntensitySettingsAreChecked )
{
   EXPECT_TRUE( US_SimOptics::intensityProblem( US_SimOptics::Intensity() ).isEmpty() );
   US_SimOptics::Intensity settings;
   settings.source_counts = 0.0;
   EXPECT_FALSE( US_SimOptics::intensityProblem( settings ).isEmpty() );
   settings      = US_SimOptics::Intensity();
   settings.dark = settings.source_counts;
   EXPECT_FALSE( US_SimOptics::intensityProblem( settings ).isEmpty() );
   settings            = US_SimOptics::Intensity();
   settings.read_noise = -1.0;
   EXPECT_FALSE( US_SimOptics::intensityProblem( settings ).isEmpty() );
}

TEST( US_SimOpticsTest, SourceIsDimInTheFarUvAndCurvedAcrossTheCell )
{
   EXPECT_LT( US_SimOptics::sourceFactor( 200.0 ), 0.6 );
   EXPECT_GT( US_SimOptics::sourceFactor( 280.0 ), 0.95 );
   EXPECT_GE( US_SimOptics::sourceFactor( 150.0 ), 0.02 );

   US_SimOptics::Intensity settings;
   const double            center = US_SimOptics::sourceIntensity( settings, 6.5, 400.0 );
   const double            edge   = US_SimOptics::sourceIntensity( settings, 7.25, 400.0 );
   EXPECT_NEAR( edge / center, 1.0 - US_SimOptics::profileCurvature, 1e-9 );

   settings.curved = false;
   EXPECT_DOUBLE_EQ( US_SimOptics::sourceIntensity( settings, 7.25, 400.0 ), center );
}

TEST( US_SimOpticsTest, CountsNoiseCombinesReadAndShotNoise )
{
   const US_SimOptics::Intensity settings; // read 12, shot 0.24
   EXPECT_DOUBLE_EQ( US_SimOptics::countsSigma( settings, 0.0 ), 12.0 );
   EXPECT_NEAR( US_SimOptics::countsSigma( settings, 3600.0 ), std::sqrt( 144.0 + 0.24 * 3600.0 ), 1e-9 );
   EXPECT_LE( US_SimOptics::countsSigma( settings, 14000.0 ), 60.0 );
}

TEST( US_SimOpticsTest, RecordedCountsFloorAtDarkAndNeverGoNegative )
{
   const US_SimOptics::Intensity settings;
   EXPECT_DOUBLE_EQ( US_SimOptics::recordedCounts( settings, 0.0, 0.0 ), settings.dark );
   EXPECT_DOUBLE_EQ( US_SimOptics::recordedCounts( settings, 5000.0, 0.0 ), 5000.0 );
   EXPECT_DOUBLE_EQ( US_SimOptics::recordedCounts( settings, 0.0, -10.0 ), 0.0 );
}

TEST( US_SimOpticsTest, ReferenceChannelPairsSampleSectors )
{
   EXPECT_EQ( US_SimOptics::referenceChannel( 'A' ), 'B' );
   EXPECT_EQ( US_SimOptics::referenceChannel( 'G' ), 'H' );
   EXPECT_EQ( US_SimOptics::referenceChannel( 'B' ), 0 );
   EXPECT_EQ( US_SimOptics::referenceChannel( 'S' ), 0 );
}

TEST( US_SimOpticsTest, BufferAbsorbanceInterpolatesItsSpectrum )
{
   US_Buffer buffer;
   EXPECT_DOUBLE_EQ( US_SimOptics::bufferOD( buffer, 280.0, 1.2 ), 0.0 );
   buffer.extinction[260.0] = 0.2;
   buffer.extinction[300.0] = 0.0;
   EXPECT_NEAR( US_SimOptics::bufferOD( buffer, 280.0, 1.2 ), 0.12, 1e-12 );
   EXPECT_DOUBLE_EQ( US_SimOptics::bufferOD( buffer, 400.0, 1.2 ), 0.0 );
}

TEST( US_SimOpticsTest, ToWindowFillsAirCellAndPastBottom )
{
   US_DataIO::RawData   data = absorbance( 5.87, 7.2, 3, plateau );
   US_SimOptics::Window window;
   US_Math2::randomize( 7 );
   US_SimOptics::toWindow( data, window, 5.87, 7.2, 1.0, 0.0, 0.05, 3.0, 2.5 );

   ASSERT_EQ( data.xvalues.size(), 1500 );
   for ( const US_DataIO::Scan& scan : data.scanData )
   {
      ASSERT_EQ( scan.rvalues.size(), 1500 );
      EXPECT_EQ( scan.interpolated.size(), ( 1500 + 7 ) / 8 );
      EXPECT_DOUBLE_EQ( scan.rvalues[data.xindex( 5.80 )], 0.0 );    // air
      EXPECT_NEAR( scan.rvalues[data.xindex( 6.50 )], 0.85, 1e-12 ); // liquid plus buffer
      EXPECT_DOUBLE_EQ( scan.rvalues[data.xindex( 7.23 )], 2.5 );    // past bottom, capped
      // The meniscus spike stands above the plateau, as the GMP automatic edit needs.
      double peak = 0.0;
      for ( int jj = data.xindex( 5.86 ); jj <= data.xindex( 5.88 ); jj++ )
      {
         peak = qMax( peak, scan.rvalues[jj] );
      }
      EXPECT_GT( peak, 0.85 + window.meniscus_artifact * 1.0 * 0.99 );
      EXPECT_LE( peak, 2.5 );
   }
}

TEST( US_SimOpticsTest, ToIntensityRecoversAbsorbanceWithoutNoise )
{
   US_DataIO::RawData data = absorbance( 5.87, 7.2, 2, plateau );
   US_DataIO::RawData sample;
   US_DataIO::RawData reference;
   US_SimOptics::toIntensity( data, quiet(), 5.9, 7.1, 0.1, sample, reference );

   EXPECT_EQ( sample.channel, 'A' );
   EXPECT_EQ( reference.channel, 'B' );
   const int mid = data.xindex( 6.5 );
   const int air = data.xindex( 5.88 );
   const int end = data.xindex( 7.15 );
   for ( int js = 0; js < 2; js++ )
   {
      const QVector<double>& is = sample.scanData[js].rvalues;
      const QVector<double>& ir = reference.scanData[js].rvalues;
      EXPECT_NEAR( std::log10( ir[mid] / is[mid] ), 0.8, 1e-12 );
      EXPECT_NEAR( ir[mid], US_SimOptics::sourceIntensity( quiet(), 6.5, 280.0 ) * std::pow( 10.0, -0.1 ), 1e-9 );
      EXPECT_DOUBLE_EQ( is[air], ir[air] ); // air: both see the source
      EXPECT_DOUBLE_EQ( is[end], 0.0 );     // past bottom: dark
      EXPECT_DOUBLE_EQ( ir[end], 0.0 );
   }
}

TEST( US_SimOpticsTest, ToIntensityIsReproducibleFromTheSeed )
{
   const US_DataIO::RawData data = absorbance( 5.87, 7.2, 2, plateau );
   US_DataIO::RawData       a1, b1, a2, b2;
   US_Math2::randomize( 42 );
   US_SimOptics::toIntensity( data, US_SimOptics::Intensity(), 5.87, 7.2, 0.0, a1, b1 );
   US_Math2::randomize( 42 );
   US_SimOptics::toIntensity( data, US_SimOptics::Intensity(), 5.87, 7.2, 0.0, a2, b2 );
   EXPECT_EQ( a1.scanData[1].rvalues, a2.scanData[1].rvalues );
   EXPECT_EQ( b1.scanData[1].rvalues, b2.scanData[1].rvalues );
   EXPECT_NE( a1.scanData[0].rvalues, a1.scanData[1].rvalues );
}

TEST( US_SimOpticsTest, SimparamsRoundTripTheSettingsOnlyWhenGiven )
{
   QTemporaryDir dir;
   ASSERT_TRUE( dir.isValid() );

   US_SimulationParameters plain;
   const QString           plain_path = dir.filePath( "plain.xml" );
   ASSERT_EQ( plain.save_simparms( plain_path ), 0 );
   QFile plain_file( plain_path );
   ASSERT_TRUE( plain_file.open( QIODevice::ReadOnly ) );
   const QByteArray plain_text = plain_file.readAll();
   EXPECT_FALSE( plain_text.contains( "<intensity" ) );
   EXPECT_FALSE( plain_text.contains( "<window" ) );

   US_SimulationParameters given;
   given.intensity.given         = true;
   given.intensity.source_counts = 12000.0;
   given.intensity.curved        = false;
   given.window.given            = true;
   given.window.end              = 7.3;
   const QString given_path      = dir.filePath( "given.xml" );
   ASSERT_EQ( given.save_simparms( given_path ), 0 );

   US_SimulationParameters loaded;
   ASSERT_EQ( loaded.load_simparms( given_path ), 0 );
   EXPECT_TRUE( loaded.intensity.given );
   EXPECT_DOUBLE_EQ( loaded.intensity.source_counts, 12000.0 );
   EXPECT_FALSE( loaded.intensity.curved );
   EXPECT_TRUE( loaded.window.given );
   EXPECT_DOUBLE_EQ( loaded.window.end, 7.3 );

   // Loading a file without them turns them off again.
   ASSERT_EQ( loaded.load_simparms( plain_path ), 0 );
   EXPECT_FALSE( loaded.intensity.given );
   EXPECT_FALSE( loaded.window.given );
}
