//! \file test_us_astfem_rsa_threads.cpp
// test_us_astfem_rsa_threads.cpp - ASTFEM solves that share a process:
//  solves in threads must not share scratch storage, and a solve without a
//  loaded timestate must use the speed profile of its own experiment.
#include "qt_test_base.h"
#include "us_astfem_math.h"
#include "us_astfem_rsa.h"
#include "us_constants.h"
#include "us_dataIO.h"
#include "us_model.h"
#include "us_settings.h"
#include "us_simparms.h"
#include "us_time_state.h"
#include <QDir>
#include <QStringList>
#include <QVector>
#include <cmath>
#include <cstring>
#include <thread>
#include <vector>

namespace
{
//! One single-speed experiment and one species, simulated without a loaded
//!  timestate
struct Experiment
{
      int    rpm;
      double hours;
      double sval;
      double dval;
      bool   fixed_grid;
};

// Speed step, scans and simulation parameters of an experiment;  no
//  timestate is loaded.  Channel 'A' (not a simulation):  the timestate
//  then derives the acceleration from the scans, as for measured data,
//  and a speed profile of another experiment changes the result.
void setup( const Experiment& exp, US_SimulationParameters& sp, US_DataIO::RawData& rdata )
{
   const int    nscans = 20;
   const double delay  = 240.0;
   const double durat  = exp.hours * 3600.0;

   sp                   = US_SimulationParameters();
   sp.simpoints         = 200;
   sp.radial_resolution = 0.005;
   sp.meniscus          = 5.8;
   sp.bottom            = 7.2;
   sp.bottom_position   = 7.2;
   sp.temperature       = NORMAL_TEMP;
   sp.sim               = true;
   sp.gridType          = exp.fixed_grid ? US_SimulationParameters::FIXED : US_SimulationParameters::MOVING;

   US_SimulationParameters::SpeedProfile& step = sp.speed_step[0];
   step.duration_hours                         = (int) exp.hours;
   step.duration_minutes                       = ( exp.hours - (int) exp.hours ) * 60.0;
   step.delay_hours                            = 0;
   step.delay_minutes                          = delay / 60.0;
   step.scans                                  = nscans;
   step.rotorspeed                             = exp.rpm;
   step.set_speed                              = exp.rpm;
   step.avg_speed                              = exp.rpm;
   step.acceleration                           = 400;
   step.acceleration_flag                      = true;

   rdata         = US_DataIO::RawData();
   rdata.type[0] = 'R';
   rdata.type[1] = 'A';
   memset( rdata.rawGUID, 0, sizeof( rdata.rawGUID ) );
   rdata.cell        = 1;
   rdata.channel     = 'A';
   rdata.description = "Synthetic";
   for ( double rr = sp.meniscus; rr <= sp.bottom + 1.0e-9; rr += sp.radial_resolution )
   {
      rdata.xvalues << rr;
   }

   for ( int ss = 0; ss < nscans; ss++ )
   {
      US_DataIO::Scan scan;
      scan.temperature = NORMAL_TEMP;
      scan.rpm         = exp.rpm;
      scan.seconds     = (double) qRound( delay + ( durat - delay ) * ( ss + 1 ) / nscans );
      scan.omega2t     = US_AstfemMath::calc_omega2t( 0.0, 0.0, 0.0, exp.rpm, step.acceleration, scan.seconds );
      scan.wavelength  = 280.0;
      scan.plateau     = 0.0;
      scan.delta_r     = sp.radial_resolution;
      scan.nz_stddev   = false;
      scan.rvalues.fill( 0.0, rdata.xvalues.size() );
      rdata.scanData << scan;
   }
   US_SimulationParameters::computeSpeedSteps( &rdata.scanData, sp.speed_step );
}

// All simulated values of an experiment, scan-major
QVector<double> simulate( const Experiment& exp )
{
   US_Model model;
   model.components.resize( 1 );
   US_Model::SimulationComponent& sc = model.components[0];
   sc.s                              = exp.sval;
   sc.D                              = exp.dval;
   sc.vbar20                         = TYPICAL_VBAR;
   sc.f_f0                           = 0.0;
   sc.mw                             = 0.0;
   sc.f                              = 0.0;
   sc.signal_concentration           = 1.0;

   US_SimulationParameters sp;
   US_DataIO::RawData      rdata;
   setup( exp, sp, rdata );

   US_Astfem_RSA astfem( model, sp );
   astfem.set_debug_flag( 0 );
   astfem.calculate( rdata );

   QVector<double> vals;
   for ( int ss = 0; ss < rdata.scanCount(); ss++ )
   {
      for ( int rr = 0; rr < rdata.pointCount(); rr++ )
      {
         vals << rdata.value( ss, rr );
      }
   }
   return vals;
}

// Index of the first value that differs bitwise, or -1
int firstDifference( const QVector<double>& aa, const QVector<double>& bb )
{
   if ( aa.size() != bb.size() )
   {
      return 0;
   }
   for ( int ii = 0; ii < aa.size(); ii++ )
   {
      if ( std::memcmp( &aa[ii], &bb[ii], sizeof( double ) ) != 0 )
      {
         return ii;
      }
   }
   return -1;
}

// Where a temporary timestate goes in this build variant
QString timestateBase()
{
#ifdef NO_DB
   return QDir::tempPath();
#else
   return US_Settings::tmpDir();
#endif
}

QStringList timestateDirs()
{
   return QDir( timestateBase() ).entryList( QStringList{ "us3-tmst-*" }, QDir::AllEntries | QDir::NoDotAndDotDot );
}

// Experiments with different speed profiles, both moving and fixed grids,
//  and a floating species (moving mesh to the left)
const std::vector<Experiment> experiments = {
   { 50000, 2.0, 4.0e-13, 6.0e-7, false },
   { 30000, 5.0, 4.0e-13, 6.0e-7, false },
   { 40000, 2.5, 7.0e-13, 4.0e-7, true },
   { 45000, 2.0, -2.0e-13, 5.0e-7, false },
};
} // namespace

class TestUSAstfemRsaThreads : public QtTestBase
{
};

// A solve without a loaded timestate gets the speed profile of its own
//  experiment, also right after another experiment in the same process
//  (within the same second)
TEST_F( TestUSAstfemRsaThreads, TimestateIsPerExperiment )
{
   const Experiment&     aa    = experiments[0];
   const Experiment&     bb    = experiments[1];
   const QVector<double> alone = simulate( bb );
   ASSERT_GT( alone.size(), 0 );

   for ( int pass = 0; pass < 3; pass++ )
   {
      simulate( aa );
      const QVector<double> after = simulate( bb );
      EXPECT_EQ( firstDifference( after, alone ), -1 ) << "pass " << pass;
   }
}

// The temporary timestate of a solve is not left behind
TEST_F( TestUSAstfemRsaThreads, TimestateFromSpeedstepsLeavesNoFiles )
{
   const QStringList       before = timestateDirs();

   US_SimulationParameters sp;
   US_DataIO::RawData      rdata;
   setup( experiments[0], sp, rdata );
   ASSERT_EQ( sp.tsobj, nullptr );

   EXPECT_GT( US_AstfemMath::timestate_from_speedsteps( sp, rdata ), 0 );
   ASSERT_NE( sp.tsobj, nullptr );
   EXPECT_GT( sp.tsobj->time_count(), 0 );
   EXPECT_EQ( sp.sim_speed_prof[0].rotorspeed, experiments[0].rpm );

   simulate( experiments[1] );
   EXPECT_EQ( timestateDirs(), before );
}

// Solves in threads, each of its own experiment and without a loaded
//  timestate, give the same values as the same solves one after another
TEST_F( TestUSAstfemRsaThreads, ThreadsMatchSerial )
{
   std::vector<QVector<double>> expected;
   for ( const Experiment& exp : experiments )
   {
      expected.push_back( simulate( exp ) );
      ASSERT_GT( expected.back().size(), 0 );
   }

   const int nrepeat = 3;
   const int nexps   = (int) experiments.size();
   for ( int pass = 0; pass < 2; pass++ )
   {
      std::vector<QVector<double>> results( nexps * nrepeat );
      std::vector<std::thread>     workers;
      for ( int tt = 0; tt < nexps * nrepeat; tt++ )
      {
         workers.emplace_back( [tt, nexps, &results]() { results[tt] = simulate( experiments[tt % nexps] ); } );
      }
      for ( std::thread& worker : workers )
      {
         worker.join();
      }

      for ( int tt = 0; tt < nexps * nrepeat; tt++ )
      {
         EXPECT_EQ( firstDifference( results[tt], expected[tt % nexps] ), -1 )
            << "pass " << pass << " thread " << tt << " experiment " << tt % nexps;
      }
   }
}
