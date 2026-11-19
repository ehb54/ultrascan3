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

namespace
{
US_SimOptics::Interference quietInterference()
{
   US_SimOptics::Interference settings;
   settings.random            = 0.0;
   settings.offset            = 0.0;
   settings.structure         = 0.0;
   settings.jump_rate         = 0.0;
   settings.meniscus_artifact = 0.0;
   return settings;
}

US_SimOptics::WavelengthScan smallScan()
{
   US_SimOptics::WavelengthScan settings;
   settings.given           = true;
   settings.start           = 250.0;
   settings.end             = 270.0;
   settings.spectrum[250.0] = 500.0;
   settings.spectrum[260.0] = 1000.0;
   settings.spectrum[270.0] = 500.0;
   return settings;
}
}

TEST( US_SimOpticsTest, InterferenceUsesItsOwnGridAndScale )
{
   US_DataIO::RawData         data = absorbance( 5.87, 7.2, 2, plateau );
   QVector<double>            structure;
   QVector<double>            offsets;
   US_SimOptics::Interference settings = quietInterference();
   US_SimOptics::toInterference( data, settings, 5.87, 7.2, 1.0, 0.0, structure, offsets );

   ASSERT_EQ( data.xvalues.size(), 2048 );
   EXPECT_DOUBLE_EQ( data.xvalues.first(), 5.80 );
   EXPECT_NEAR( data.xvalues.last(), 5.80 + 2047 * 0.00068, 1e-9 );
   EXPECT_EQ( data.scanData[0].rvalues.size(), 2048 );
   EXPECT_DOUBLE_EQ( data.scanData[0].delta_r, 0.00068 );
   EXPECT_DOUBLE_EQ( data.scanData[0].rvalues[data.xindex( 5.83 )], 0.0 );       // before the meniscus
   EXPECT_NEAR( data.scanData[0].rvalues[data.xindex( 6.5 )], 0.8 * 3.3, 1e-9 ); // fringes
   EXPECT_EQ( offsets, QVector<double>( { 0.0, 0.0 } ) );
}

TEST( US_SimOpticsTest, InterferenceNoiseHasTheRequestedSizes )
{
   US_DataIO::RawData         data = absorbance( 5.87, 7.2, 400, plateau );
   QVector<double>            structure;
   QVector<double>            offsets;
   US_SimOptics::Interference settings = quietInterference();
   settings.random                     = 0.002;
   settings.offset                     = 0.02;
   settings.structure                  = 0.01;
   US_Math2::randomize( 3 );
   US_SimOptics::toInterference( data, settings, 5.87, 7.2, 1.0, 0.0, structure, offsets );

   double sum = 0.0, sum2 = 0.0;
   for ( double value : structure )
   {
      sum += value;
      sum2 += value * value;
   }
   EXPECT_NEAR( std::sqrt( sum2 / structure.size() - std::pow( sum / structure.size(), 2 ) ), 0.01, 1e-9 );

   double osum2 = 0.0;
   for ( double value : offsets )
   {
      osum2 += value * value;
   }
   EXPECT_NEAR( std::sqrt( osum2 / offsets.size() ), 0.02, 0.003 );

   // What is left on the plateau once structure and offset are taken out is the random noise.
   const US_DataIO::Scan& scan  = data.scanData[10];
   double                 dsum2 = 0.0;
   int                    count = 0;
   for ( int jj = data.xindex( 6.4 ); jj < data.xindex( 6.9 ); jj++ )
   {
      const double here = scan.rvalues[jj] - structure[jj];
      const double next = scan.rvalues[jj + 1] - structure[jj + 1];
      dsum2 += ( next - here ) * ( next - here );
      count++;
   }
   EXPECT_NEAR( std::sqrt( dsum2 / count / 2.0 ), 0.002, 0.0003 );
}

TEST( US_SimOpticsTest, InterferenceJumpsAreWholeFringes )
{
   US_DataIO::RawData         data = absorbance( 5.87, 7.2, 50, plateau );
   QVector<double>            structure;
   QVector<double>            offsets;
   US_SimOptics::Interference settings = quietInterference();
   settings.jump_rate                  = 1.0;
   settings.step_fraction              = 0.0; // every jump is a whole-scan offset
   US_Math2::randomize( 11 );
   US_SimOptics::toInterference( data, settings, 5.87, 7.2, 1.0, 0.0, structure, offsets );
   for ( double offset : offsets )
   {
      EXPECT_TRUE( offset == 1.0 || offset == -1.0 || offset == 2.0 || offset == -2.0 ) << offset;
   }
}

TEST( US_SimOpticsTest, InterferenceClipsInFringes )
{
   US_DataIO::RawData data = absorbance( 5.87, 7.2, 1, plateau );
   QVector<double>    structure;
   QVector<double>    offsets;
   US_SimOptics::toInterference( data, quietInterference(), 5.87, 7.2, 1.0, 2.0, structure, offsets );
   for ( double value : data.scanData[0].rvalues )
   {
      EXPECT_LE( value, 2.0 );
   }
}

TEST( US_SimOpticsTest, InterferenceSettingsAreChecked )
{
   EXPECT_TRUE( US_SimOptics::interferenceProblem( US_SimOptics::Interference() ).isEmpty() );
   US_SimOptics::Interference settings;
   settings.scale = 0.0;
   EXPECT_FALSE( US_SimOptics::interferenceProblem( settings ).isEmpty() );
   settings           = US_SimOptics::Interference();
   settings.jump_rate = 1.5;
   EXPECT_FALSE( US_SimOptics::interferenceProblem( settings ).isEmpty() );
}

TEST( US_SimOpticsTest, WavelengthScanGeometryIsChecked )
{
   US_SimOptics::WavelengthScan settings = smallScan();
   EXPECT_EQ( US_SimOptics::scanWavelengths( settings ).size(), 21 );
   EXPECT_TRUE( US_SimOptics::wavelengthScanProblem( settings, 5.87, 7.2 ).isEmpty() );
   EXPECT_FALSE( US_SimOptics::wavelengthScanProblem( settings, 6.6, 7.2 ).isEmpty() ); // radius in air
   settings.end = 280.0;                                                                // past the spectrum
   EXPECT_FALSE( US_SimOptics::wavelengthScanProblem( settings, 5.87, 7.2 ).isEmpty() );
   settings.spectrum.clear();
   EXPECT_FALSE( US_SimOptics::wavelengthScanProblem( settings, 5.87, 7.2 ).isEmpty() );

   US_SimOptics::WavelengthScan real = smallScan();
   real.start                        = 180.0;
   real.end                          = 800.0;
   real.spectrum[180.0]              = 900.0;
   real.spectrum[800.0]              = 0.0;
   EXPECT_EQ( US_SimOptics::scanWavelengths( real ).size(), 621 );
}

TEST( US_SimOpticsTest, WavelengthScanFollowsBeerLambertAtItsRadius )
{
   const US_DataIO::RawData           data     = absorbance( 5.87, 7.2, 2, plateau );
   const US_SimOptics::WavelengthScan settings = smallScan();
   US_DataIO::RawData                 sample;
   US_DataIO::RawData                 reference;
   US_SimOptics::toWavelengthScan( data, quiet(), settings, {}, sample, reference );

   ASSERT_EQ( sample.xvalues.size(), 21 );
   EXPECT_DOUBLE_EQ( sample.xvalues.first(), 250.0 );
   EXPECT_EQ( reference.channel, 'B' );
   EXPECT_DOUBLE_EQ( sample.scanData[0].wavelength, 6.5 ); // the radius, as an Optima wavelength scan stores it
   EXPECT_DOUBLE_EQ( sample.scanData[0].delta_r, 1.0 );
   const QVector<double>& is = sample.scanData[0].rvalues;
   const QVector<double>& ir = reference.scanData[0].rvalues;
   EXPECT_NEAR( std::log10( ir[10] / is[10] ), 0.8, 1e-9 );     // 260 nm, the spectrum's peak
   EXPECT_NEAR( std::log10( ir[0] / is[0] ), 0.8 * 0.5, 1e-9 ); // 250 nm, half the peak
   EXPECT_NEAR( ir[10], US_SimOptics::sourceIntensity( quiet(), 6.5, 260.0 ), 1e-9 );
}

TEST( US_SimOpticsTest, SimparamsRoundTripInterferenceAndWavelengthScan )
{
   QTemporaryDir dir;
   ASSERT_TRUE( dir.isValid() );
   US_SimulationParameters given;
   given.interference.given  = true;
   given.interference.scale  = 0.16;
   given.interference.points = 1024;
   given.wavelength_scan     = smallScan();
   const QString path        = dir.filePath( "given.xml" );
   ASSERT_EQ( given.save_simparms( path ), 0 );

   US_SimulationParameters loaded;
   ASSERT_EQ( loaded.load_simparms( path ), 0 );
   EXPECT_TRUE( loaded.interference.given );
   EXPECT_DOUBLE_EQ( loaded.interference.scale, 0.16 );
   EXPECT_EQ( loaded.interference.points, 1024 );
   EXPECT_TRUE( loaded.wavelength_scan.given );
   EXPECT_EQ( loaded.wavelength_scan.spectrum, given.wavelength_scan.spectrum );
   EXPECT_DOUBLE_EQ( loaded.wavelength_scan.end, 270.0 );
}

TEST( US_SimOpticsTest, InterferenceStructureIsSharedByEverySpeedStep )
{
   US_SimOptics::Interference settings = quietInterference();
   settings.structure                  = 0.01;
   QVector<double> structure;
   QVector<double> offsets;
   US_Math2::randomize( 5 );
   US_DataIO::RawData first = absorbance( 5.87, 7.2, 2, plateau );
   US_SimOptics::toInterference( first, settings, 5.87, 7.2, 1.0, 0.0, structure, offsets );
   const QVector<double> drawn  = structure;
   US_DataIO::RawData    second = absorbance( 5.87, 7.2, 2, plateau );
   US_SimOptics::toInterference( second, settings, 5.87, 7.2, 1.0, 0.0, structure, offsets );
   EXPECT_EQ( structure, drawn );
   EXPECT_EQ( first.scanData[0].rvalues[100], second.scanData[0].rvalues[100] );
}
