//! \file test_us_sim_schedule.cpp
#include "qt_test_base.h"
#include "us_sim_schedule.h"
#include "us_simparms.h"

#include <QTemporaryDir>

#include <cmath>

namespace
{
US_SimSchedule::Schedule threeCells( int slot )
{
   US_SimSchedule::Schedule settings;
   settings.given        = true;
   settings.slot_seconds = { 23.0, 23.0, 23.0 };
   settings.slot         = slot;
   return settings;
}

// Accelerate at 400 rpm/s to 45,000 rpm, then hold, for the given seconds.
QVector<US_SimSchedule::Row> physics( int seconds )
{
   QVector<US_SimSchedule::Row> rows;
   double                       omega2t = 0.0;
   for ( int tt = 1; tt <= seconds; tt++ )
   {
      US_SimSchedule::Row row;
      row.time      = tt;
      row.raw_speed = qMin( 45000.0, 400.0 * tt );
      row.set_speed = qRound( row.raw_speed / 100.0 ) * 100;
      omega2t += std::pow( row.raw_speed * M_PI / 30.0, 2.0 );
      row.omega2t     = omega2t;
      row.temperature = 20.0;
      row.step        = 1;
      rows << row;
   }
   return rows;
}
}

TEST( US_SimScheduleTest, SlotsStaggerCellsWithinCycles )
{
   EXPECT_DOUBLE_EQ( US_SimSchedule::cycleSeconds( threeCells( 0 ) ), 69.0 );
   EXPECT_EQ( US_SimSchedule::scanTime( threeCells( 0 ), 113.0, 0 ), 113 );
   EXPECT_EQ( US_SimSchedule::scanTime( threeCells( 1 ), 113.0, 0 ), 136 );
   EXPECT_EQ( US_SimSchedule::scanTime( threeCells( 2 ), 113.0, 2 ), 113 + 2 * 69 + 46 );
   EXPECT_EQ( US_SimSchedule::scanTime( threeCells( 1 ), 113.0, 0, 0, 2 ), 159 ); // another slot

   US_SimSchedule::Schedule paced = threeCells( 0 );
   paced.interval                 = 100.0;
   EXPECT_DOUBLE_EQ( US_SimSchedule::cycleSeconds( paced ), 100.0 );
   EXPECT_EQ( US_SimSchedule::scanTime( paced, 113.0, 1 ), 213 );
}

TEST( US_SimScheduleTest, WavelengthsFollowEachOtherWithinTheirSlot )
{
   US_SimSchedule::Schedule settings;
   settings.slot_seconds = { 120.0 };
   EXPECT_EQ( US_SimSchedule::scanTime( settings, 113.0, 0, 2 ), 193 );
   EXPECT_TRUE( US_SimSchedule::scheduleProblem( settings, 3 ).isEmpty() );
   EXPECT_FALSE( US_SimSchedule::scheduleProblem( settings, 4 ).isEmpty() );
}

TEST( US_SimScheduleTest, UnusableSchedulesAreReported )
{
   EXPECT_TRUE( US_SimSchedule::scheduleProblem( threeCells( 2 ) ).isEmpty() );
   EXPECT_FALSE( US_SimSchedule::scheduleProblem( threeCells( 3 ) ).isEmpty() );
   EXPECT_FALSE( US_SimSchedule::scheduleProblem( US_SimSchedule::Schedule() ).isEmpty() );
   US_SimSchedule::Schedule tight = threeCells( 0 );
   tight.interval                 = 60.0; // shorter than its 69 s of slots
   EXPECT_FALSE( US_SimSchedule::scheduleProblem( tight ).isEmpty() );
}

TEST( US_SimScheduleTest, RunMarkersNumberCyclesForEverySlot )
{
   const QMap<int, int> markers = US_SimSchedule::runMarkers( threeCells( 1 ), { 113.0 }, { 2 } );
   const QMap<int, int> expected{ { 113, 1 }, { 136, 1 }, { 159, 1 }, { 182, 2 }, { 205, 2 }, { 228, 2 } };
   EXPECT_EQ( markers, expected );

   QVector<US_SimSchedule::Row> rows = physics( 300 );
   US_SimSchedule::applyMarkers( rows, markers );
   EXPECT_EQ( rows[135].scan, 1 ); // second 136
   EXPECT_EQ( rows[136].scan, 0 );
}

TEST( US_SimScheduleTest, TelemetryJittersHeldSpeedAndWandersTemperature )
{
   const QVector<US_SimSchedule::Row> clean = physics( 3600 );
   QVector<US_SimSchedule::Row>       rows  = clean;
   US_SimSchedule::Telemetry          settings;
   settings.given = true;
   settings.seed  = 5;
   US_SimSchedule::applyTelemetry( rows, settings );

   double lo = 1e9, hi = -1e9, tlo = 1e9, thi = -1e9;
   for ( int jj = 0; jj < rows.size(); jj++ )
   {
      if ( clean[jj].raw_speed < 45000.0 )
      { // Acceleration is recorded as simulated.
         EXPECT_DOUBLE_EQ( rows[jj].raw_speed, clean[jj].raw_speed ) << "second " << rows[jj].time;
      }
      else
      {
         lo = qMin( lo, rows[jj].raw_speed );
         hi = qMax( hi, rows[jj].raw_speed );
      }
      tlo = qMin( tlo, rows[jj].temperature );
      thi = qMax( thi, rows[jj].temperature );
      EXPECT_EQ( rows[jj].set_speed, clean[jj].set_speed );
   }
   EXPECT_GE( lo, 45000.0 - settings.rpm_jitter );
   EXPECT_LE( hi, 45000.0 + settings.rpm_jitter );
   EXPECT_LT( lo, 45000.0 );
   EXPECT_GT( hi, 45000.0 );
   EXPECT_LT( tlo, 20.0 );
   EXPECT_GT( thi, 20.0 );
   EXPECT_LT( thi - tlo, 1.0 );

   // omega2t stays the integral of the recorded speed, one second at a time.
   for ( int jj = 200; jj < 210; jj++ )
   {
      const double step = rows[jj].omega2t - rows[jj - 1].omega2t;
      EXPECT_NEAR( step, std::pow( rows[jj].raw_speed * M_PI / 30.0, 2.0 ), 1e-6 * step );
   }
}

TEST( US_SimScheduleTest, TelemetryIsReproducibleFromItsSeed )
{
   US_SimSchedule::Telemetry settings;
   settings.seed                     = 9;
   QVector<US_SimSchedule::Row> one  = physics( 500 );
   QVector<US_SimSchedule::Row> two  = physics( 500 );
   QVector<US_SimSchedule::Row> four = physics( 500 );
   US_SimSchedule::applyTelemetry( one, settings );
   US_SimSchedule::applyTelemetry( two, settings );
   settings.seed = 4;
   US_SimSchedule::applyTelemetry( four, settings );
   bool differs = false;
   for ( int jj = 0; jj < one.size(); jj++ )
   {
      EXPECT_EQ( one[jj].raw_speed, two[jj].raw_speed );
      EXPECT_EQ( one[jj].temperature, two[jj].temperature );
      differs = differs || one[jj].temperature != four[jj].temperature;
   }
   EXPECT_TRUE( differs );
}

TEST( US_SimScheduleTest, RowsRoundTripThroughATimeStateAndIntoScans )
{
   QTemporaryDir dir;
   ASSERT_TRUE( dir.isValid() );
   const QString                path = dir.filePath( "run.time_state.tmst" );
   QVector<US_SimSchedule::Row> rows = physics( 400 );
   rows[199].scan                    = 3;
   const QStringList keys{ "Time", "RawSpeed", "SetSpeed", "Omega2T", "Temperature", "Step", "Scan" };
   const QStringList formats{ "I4", "F4", "I4", "F4", "F4", "I2", "I2" };
   ASSERT_TRUE( US_SimSchedule::writeRows( path, rows, keys, formats ) );

   QVector<US_SimSchedule::Row> read;
   QStringList                  read_keys;
   QStringList                  read_formats;
   ASSERT_TRUE( US_SimSchedule::readRows( path, read, read_keys, read_formats ) );
   ASSERT_EQ( read.size(), rows.size() );
   EXPECT_EQ( read_keys, keys );
   EXPECT_EQ( read[199].scan, 3 );
   EXPECT_FLOAT_EQ( read[199].raw_speed, rows[199].raw_speed );

   US_DataIO::RawData data;
   US_DataIO::Scan    scan;
   scan.seconds = 200;
   data.scanData << scan;
   ASSERT_TRUE( US_SimSchedule::recordScans( data, read ) );
   EXPECT_DOUBLE_EQ( data.scanData[0].rpm, read[199].raw_speed );
   EXPECT_DOUBLE_EQ( data.scanData[0].omega2t, read[199].omega2t );
   data.scanData[0].seconds = 900;
   EXPECT_FALSE( US_SimSchedule::recordScans( data, read ) );
}

TEST( US_SimScheduleTest, SimparamsRoundTripScheduleAndTelemetryOnlyWhenGiven )
{
   QTemporaryDir dir;
   ASSERT_TRUE( dir.isValid() );
   US_SimulationParameters given;
   given.schedule             = threeCells( 2 );
   given.schedule.interval    = 80.0;
   given.telemetry.given      = true;
   given.telemetry.seed       = 1234;
   given.telemetry.rpm_jitter = 1.5;
   const QString path         = dir.filePath( "given.xml" );
   ASSERT_EQ( given.save_simparms( path ), 0 );

   US_SimulationParameters loaded;
   ASSERT_EQ( loaded.load_simparms( path ), 0 );
   EXPECT_TRUE( loaded.schedule.given );
   EXPECT_EQ( loaded.schedule.slot_seconds, given.schedule.slot_seconds );
   EXPECT_EQ( loaded.schedule.slot, 2 );
   EXPECT_DOUBLE_EQ( loaded.schedule.interval, 80.0 );
   EXPECT_TRUE( loaded.telemetry.given );
   EXPECT_EQ( loaded.telemetry.seed, 1234u );
   EXPECT_DOUBLE_EQ( loaded.telemetry.rpm_jitter, 1.5 );

   US_SimulationParameters plain;
   const QString           plain_path = dir.filePath( "plain.xml" );
   ASSERT_EQ( plain.save_simparms( plain_path ), 0 );
   ASSERT_EQ( loaded.load_simparms( plain_path ), 0 );
   EXPECT_FALSE( loaded.schedule.given );
   EXPECT_FALSE( loaded.telemetry.given );
}
