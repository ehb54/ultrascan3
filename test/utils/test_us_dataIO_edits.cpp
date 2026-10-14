//! \file test_us_dataIO_edits.cpp
// Edit file round trips through US_DataIO::writeEdits and readEdits.

#include "qt_test_base.h"
#include "us_dataIO.h"

#include <QTemporaryDir>

namespace
{
US_DataIO::EditValues velocityEdits()
{
   US_DataIO::EditValues ev;
   ev.expType    = "Velocity";
   ev.runID      = "demo-run";
   ev.cell       = "2";
   ev.channel    = "B";
   ev.wavelength = "280";
   ev.editGUID   = "11111111-2222-4333-8444-555555555555";
   ev.dataGUID   = "66666666-7777-4888-8999-aaaaaaaaaaaa";
   ev.meniscus   = 5.912345;
   ev.bottom     = 7.18;
   ev.rangeLeft  = 6.0;
   ev.rangeRight = 7.05;
   ev.plateau    = 6.9;
   ev.baseline   = 7.1;
   ev.ODlimit    = 1.25;
   ev.excludes << 0 << 3 << 7;
   ev.noiseOrder         = 2;
   ev.removeSpikes       = true;
   ev.invert             = -1.0;
   ev.floatingData       = true;
   ev.bl_corr_slope      = 0.125;
   ev.bl_corr_yintercept = -0.5;

   US_DataIO::EditedPoint ep;
   ep.scan   = 4;
   ep.radius = 17;
   ep.value  = 0.4375;
   ev.editedPoints << ep;
   return ev;
}

US_DataIO::EditValues roundTrip( const US_DataIO::EditValues& in, int* status )
{
   QTemporaryDir dir;
   const QString path = dir.path() + "/run.e1.RI.2.B.280.xml";
   *status            = US_DataIO::writeEdits( path, in );

   US_DataIO::EditValues out;
   if ( *status == US_DataIO::OK )
   {
      *status = US_DataIO::readEdits( path, out );
   }
   return out;
}
}

TEST( EditFileRoundTrip, VelocityValuesSurvive )
{
   const US_DataIO::EditValues in     = velocityEdits();
   int                         status = -1;
   const US_DataIO::EditValues out    = roundTrip( in, &status );
   ASSERT_EQ( status, US_DataIO::OK );

   EXPECT_EQ( out.expType, in.expType );
   EXPECT_EQ( out.runID, in.runID );
   EXPECT_EQ( out.cell, in.cell );
   EXPECT_EQ( out.channel, in.channel );
   EXPECT_EQ( out.wavelength, in.wavelength );
   EXPECT_EQ( out.editGUID, in.editGUID );
   EXPECT_EQ( out.dataGUID, in.dataGUID );
   EXPECT_DOUBLE_EQ( out.meniscus, in.meniscus );
   EXPECT_DOUBLE_EQ( out.bottom, in.bottom );
   EXPECT_DOUBLE_EQ( out.rangeLeft, in.rangeLeft );
   EXPECT_DOUBLE_EQ( out.rangeRight, in.rangeRight );
   EXPECT_DOUBLE_EQ( out.plateau, in.plateau );
   EXPECT_DOUBLE_EQ( out.baseline, in.baseline );
   EXPECT_DOUBLE_EQ( out.ODlimit, in.ODlimit );
   EXPECT_EQ( out.excludes, in.excludes );
   EXPECT_EQ( out.noiseOrder, in.noiseOrder );
   EXPECT_TRUE( out.removeSpikes );
   EXPECT_DOUBLE_EQ( out.invert, -1.0 );
   EXPECT_TRUE( out.floatingData );
   EXPECT_DOUBLE_EQ( out.bl_corr_slope, in.bl_corr_slope );
   EXPECT_DOUBLE_EQ( out.bl_corr_yintercept, in.bl_corr_yintercept );

   ASSERT_EQ( out.editedPoints.size(), 1 );
   EXPECT_EQ( out.editedPoints[0].scan, 4 );
   EXPECT_EQ( out.editedPoints[0].radius, 17 );
   EXPECT_DOUBLE_EQ( out.editedPoints[0].value, 0.4375 );
}

TEST( EditFileRoundTrip, DefaultsWriteNoOptionalElements )
{
   US_DataIO::EditValues in = velocityEdits();
   in.excludes.clear();
   in.editedPoints.clear();
   in.noiseOrder         = 0;
   in.removeSpikes       = false;
   in.invert             = 1.0;
   in.floatingData       = false;
   in.bl_corr_slope      = 0.0;
   in.bl_corr_yintercept = 0.0;

   int                         status = -1;
   const US_DataIO::EditValues out    = roundTrip( in, &status );
   ASSERT_EQ( status, US_DataIO::OK );

   EXPECT_TRUE( out.excludes.isEmpty() );
   EXPECT_TRUE( out.editedPoints.isEmpty() );
   EXPECT_EQ( out.noiseOrder, 0 );
   EXPECT_FALSE( out.removeSpikes );
   EXPECT_DOUBLE_EQ( out.invert, 1.0 );
   EXPECT_FALSE( out.floatingData );
   EXPECT_DOUBLE_EQ( out.airGapLeft, 0.0 );
   EXPECT_DOUBLE_EQ( out.airGapRight, 9.0 );
}

TEST( EditFileRoundTrip, AirGapAndLambdasSurvive )
{
   US_DataIO::EditValues in = velocityEdits();
   in.wavelength            = "250-280";
   in.lambdas << 250 << 260 << 280;
   in.airGapLeft   = 5.81;
   in.airGapRight  = 5.86;
   in.gapTolerance = 0.03;

   int                         status = -1;
   const US_DataIO::EditValues out    = roundTrip( in, &status );
   ASSERT_EQ( status, US_DataIO::OK );

   EXPECT_EQ( out.lambdas, in.lambdas );
   EXPECT_DOUBLE_EQ( out.airGapLeft, in.airGapLeft );
   EXPECT_DOUBLE_EQ( out.airGapRight, in.airGapRight );
   EXPECT_DOUBLE_EQ( out.gapTolerance, in.gapTolerance );
}

TEST( EditFileRoundTrip, WavelengthRangeWithoutLambdasSurvives )
{
   US_DataIO::EditValues in = velocityEdits();
   in.wavelength            = "250-280";

   int                         status = -1;
   const US_DataIO::EditValues out    = roundTrip( in, &status );
   ASSERT_EQ( status, US_DataIO::OK );

   EXPECT_TRUE( out.lambdas.isEmpty() );
   EXPECT_EQ( out.wavelength, in.wavelength );
   EXPECT_DOUBLE_EQ( out.meniscus, in.meniscus );
   EXPECT_EQ( out.excludes, in.excludes );
   EXPECT_EQ( out.noiseOrder, in.noiseOrder );
}

TEST( EditFileRoundTrip, EquilibriumSpeedStepsSurvive )
{
   US_DataIO::EditValues in = velocityEdits();
   in.expType               = "Equilibrium";

   for ( int ii = 0; ii < 2; ii++ )
   {
      US_DataIO::SpeedData sd;
      sd.speed      = 20000.0 + 5000.0 * ii;
      sd.first_scan = 1 + 8 * ii;
      sd.scan_count = 8;
      sd.meniscus   = 5.9 + 0.01 * ii;
      sd.dataLeft   = 6.0 + 0.01 * ii;
      sd.dataRight  = 6.2 + 0.01 * ii;
      in.speedData << sd;
   }

   int                         status = -1;
   const US_DataIO::EditValues out    = roundTrip( in, &status );
   ASSERT_EQ( status, US_DataIO::OK );

   ASSERT_EQ( out.speedData.size(), 2 );
   for ( int ii = 0; ii < 2; ii++ )
   {
      SCOPED_TRACE( ii );
      EXPECT_DOUBLE_EQ( out.speedData[ii].speed, in.speedData[ii].speed );
      EXPECT_EQ( out.speedData[ii].first_scan, in.speedData[ii].first_scan );
      EXPECT_EQ( out.speedData[ii].scan_count, in.speedData[ii].scan_count );
      EXPECT_DOUBLE_EQ( out.speedData[ii].meniscus, in.speedData[ii].meniscus );
      EXPECT_DOUBLE_EQ( out.speedData[ii].dataLeft, in.speedData[ii].dataLeft );
      EXPECT_DOUBLE_EQ( out.speedData[ii].dataRight, in.speedData[ii].dataRight );
   }
}

TEST( EditFileRoundTrip, UnwritablePathReportsCantOpen )
{
   EXPECT_EQ( US_DataIO::writeEdits( "/nonexistent-dir/x.xml", velocityEdits() ), US_DataIO::CANTOPEN );
}
