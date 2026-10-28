//! \file test_us_sim_cache.cpp
// test_us_sim_cache.cpp - Unit tests for US_SimCache
#include "qt_test_base.h"
#include "us_astfem_math.h"
#include "us_settings.h"
#include "us_sim_cache.h"
#include <atomic>
#include <cstring>
#include <memory>

namespace
{
// A simulation of a 4-scan, 10-point grid, filled with values from a seed
US_DataIO::RawData makeSim( double seed )
{
   US_DataIO::RawData simdat;
   simdat.cell    = 1;
   simdat.channel = 'S';
   for ( int rr = 0; rr < 10; rr++ )
   {
      simdat.xvalues << 6.0 + 0.1 * rr;
   }
   for ( int ss = 0; ss < 4; ss++ )
   {
      US_DataIO::Scan scan;
      scan.seconds = 100.0 * ( ss + 1 );
      for ( int rr = 0; rr < 10; rr++ )
      {
         scan.rvalues << seed + 0.001 * ( ss * 10 + rr ) / 3.0;
      }
      simdat.scanData << scan;
   }
   return simdat;
}

// Bitwise equality of the readings of two simulations
bool sameValues( const US_DataIO::RawData& adat, const US_DataIO::RawData& bdat )
{
   if ( adat.scanData.size() != bdat.scanData.size() || adat.xvalues != bdat.xvalues )
   {
      return false;
   }
   for ( int ss = 0; ss < adat.scanData.size(); ss++ )
   {
      const QVector<double>& av = adat.scanData[ss].rvalues;
      const QVector<double>& bv = bdat.scanData[ss].rvalues;
      if ( av.size() != bv.size() || memcmp( av.constData(), bv.constData(), av.size() * sizeof( double ) ) != 0 )
      {
         return false;
      }
   }
   return true;
}

QByteArray key( int ii )
{
   return QByteArray( "key" ) + QByteArray::number( ii );
}
} // namespace

class TestUSSimCache : public QtTestBase
{
   protected:
      void SetUp() override
      {
         QtTestBase::SetUp();
         savedDebug = US_Settings::debug_text();
         sbytes     = US_SimCache::sim_bytes( makeSim( 0.0 ) );
      }

      void TearDown() override
      {
         US_Settings::set_debug_text( savedDebug );
         QtTestBase::TearDown();
      }

      // Simulation parameters and edited data of a small grid
      void makeDataSet( US_SimulationParameters& simparams, US_DataIO::EditedData& edata )
      {
         simparams.meniscus        = 5.9;
         simparams.bottom          = 7.2;
         simparams.bottom_position = 7.2;
         edata.cell                = "1";
         edata.channel             = "A";
         edata.description         = "Test";
         for ( int rr = 0; rr < 10; rr++ )
         {
            edata.xvalues << 6.0 + 0.1 * rr;
         }
         for ( int ss = 0; ss < 4; ss++ )
         {
            US_DataIO::Scan scan;
            scan.temperature = 20.0;
            scan.rpm         = 50000.0;
            scan.seconds     = 100.0 * ( ss + 1 );
            scan.omega2t     = 1.0e9 * ( ss + 1 );
            scan.wavelength  = 280.0;
            scan.plateau     = 1.0;
            scan.delta_r     = 0.1;
            scan.rvalues.fill( 0.5, 10 );
            edata.scanData << scan;
         }
      }

      QStringList savedDebug;
      qint64      sbytes = 0;
};

TEST_F( TestUSSimCache, FetchReturnsStoredSimulationExactly )
{
   US_SimCache        cache( 1024 * 1024 );
   US_DataIO::RawData simdat = makeSim( 0.1 );
   US_DataIO::RawData fetched;

   EXPECT_FALSE( cache.fetch( key( 1 ), fetched ) );
   EXPECT_TRUE( cache.store( key( 1 ), simdat, false ) );
   ASSERT_TRUE( cache.fetch( key( 1 ), fetched ) );
   EXPECT_TRUE( sameValues( fetched, simdat ) );
   EXPECT_FALSE( cache.fetch( key( 2 ), fetched ) );

   // Changing a fetched simulation does not change the cached one
   fetched.scanData[0].rvalues[0] = -1.0;
   US_DataIO::RawData again;
   ASSERT_TRUE( cache.fetch( key( 1 ), again ) );
   EXPECT_TRUE( sameValues( again, simdat ) );

   US_SimCache::Stats st = cache.stats();
   EXPECT_EQ( st.hits, 2 );
   EXPECT_EQ( st.misses, 2 );
   EXPECT_EQ( st.stored, 1 );
   EXPECT_EQ( st.entries, 1 );
   EXPECT_EQ( st.survivors, 0 );
   EXPECT_EQ( st.bytes, sbytes );
}

TEST_F( TestUSSimCache, FullCacheRefusesNonSurvivors )
{
   US_SimCache cache( 3 * sbytes );
   for ( int ii = 0; ii < 3; ii++ )
   {
      EXPECT_TRUE( cache.store( key( ii ), makeSim( ii ), false ) );
   }
   EXPECT_FALSE( cache.store( key( 3 ), makeSim( 3 ), false ) );

   US_DataIO::RawData fetched;
   EXPECT_TRUE( cache.fetch( key( 0 ), fetched ) ); // Earlier ones are kept
   EXPECT_FALSE( cache.fetch( key( 3 ), fetched ) );
   EXPECT_EQ( cache.stats().rejected, 1 );
}

TEST_F( TestUSSimCache, SurvivorsEvictNewestNonSurvivors )
{
   US_SimCache cache( 3 * sbytes );
   for ( int ii = 0; ii < 3; ii++ )
   {
      cache.store( key( ii ), makeSim( ii ), false );
   }

   // Two new survivors evict the two newest non-survivors
   EXPECT_TRUE( cache.store( key( 10 ), makeSim( 10 ), true ) );
   EXPECT_TRUE( cache.store( key( 11 ), makeSim( 11 ), true ) );

   US_DataIO::RawData fetched;
   EXPECT_TRUE( cache.fetch( key( 0 ), fetched ) );
   EXPECT_FALSE( cache.fetch( key( 1 ), fetched ) );
   EXPECT_FALSE( cache.fetch( key( 2 ), fetched ) );
   EXPECT_TRUE( cache.fetch( key( 10 ), fetched ) );
   EXPECT_TRUE( cache.fetch( key( 11 ), fetched ) );

   // A third evicts the last non-survivor; a fourth finds no room
   EXPECT_TRUE( cache.store( key( 12 ), makeSim( 12 ), true ) );
   EXPECT_FALSE( cache.fetch( key( 0 ), fetched ) );
   EXPECT_FALSE( cache.store( key( 13 ), makeSim( 13 ), true ) );

   US_SimCache::Stats st = cache.stats();
   EXPECT_EQ( st.entries, 3 );
   EXPECT_EQ( st.survivors, 3 );
   EXPECT_EQ( st.evicted, 3 );
   EXPECT_EQ( st.rejected, 1 );
   EXPECT_LE( st.bytes, st.max_bytes );
}

TEST_F( TestUSSimCache, StoredSimulationBecomesSurvivor )
{
   US_SimCache cache( 2 * sbytes );
   cache.store( key( 0 ), makeSim( 0 ), false );
   cache.store( key( 1 ), makeSim( 1 ), false );

   // Marking a stored simulation a survivor keeps it from eviction
   EXPECT_TRUE( cache.store( key( 1 ), makeSim( 1 ), true ) );
   EXPECT_EQ( cache.stats().survivors, 1 );
   EXPECT_TRUE( cache.store( key( 2 ), makeSim( 2 ), true ) );

   US_DataIO::RawData fetched;
   EXPECT_FALSE( cache.fetch( key( 0 ), fetched ) );
   EXPECT_TRUE( cache.fetch( key( 1 ), fetched ) );
   EXPECT_TRUE( sameValues( fetched, makeSim( 1 ) ) );
   EXPECT_TRUE( cache.fetch( key( 2 ), fetched ) );
   EXPECT_EQ( cache.stats().survivors, 2 );
}

TEST_F( TestUSSimCache, ClearRemovesAllSimulations )
{
   US_SimCache cache( 4 * sbytes );
   cache.store( key( 0 ), makeSim( 0 ), false );
   cache.store( key( 1 ), makeSim( 1 ), true );
   cache.clear();

   US_DataIO::RawData fetched;
   EXPECT_FALSE( cache.fetch( key( 0 ), fetched ) );
   EXPECT_FALSE( cache.fetch( key( 1 ), fetched ) );
   US_SimCache::Stats st = cache.stats();
   EXPECT_EQ( st.entries, 0 );
   EXPECT_EQ( st.survivors, 0 );
   EXPECT_EQ( st.bytes, 0 );
   EXPECT_TRUE( cache.store( key( 2 ), makeSim( 2 ), false ) );
}

TEST_F( TestUSSimCache, DatasetKeyCoversGridAndParameters )
{
   US_SimulationParameters simparams;
   US_DataIO::EditedData   edata;
   makeDataSet( simparams, edata );
   const QByteArray dskey = US_SimCache::dataset_key( simparams, edata );
   EXPECT_EQ( US_SimCache::dataset_key( simparams, edata ), dskey );

   // Data values (as in Monte Carlo iterations) do not matter
   US_DataIO::EditedData edata2  = edata;
   edata2.scanData[1].rvalues[3] = 0.7;
   EXPECT_EQ( US_SimCache::dataset_key( simparams, edata2 ), dskey );

   // Grid and simulation parameters do
   edata2 = edata;
   edata2.xvalues[4] += 1.0e-9;
   EXPECT_NE( US_SimCache::dataset_key( simparams, edata2 ), dskey );
   edata2 = edata;
   edata2.scanData[2].seconds += 1.0;
   EXPECT_NE( US_SimCache::dataset_key( simparams, edata2 ), dskey );
   edata2 = edata;
   edata2.scanData.removeLast();
   EXPECT_NE( US_SimCache::dataset_key( simparams, edata2 ), dskey );

   US_SimulationParameters simparams2 = simparams;
   simparams2.meniscus                = 5.91;
   EXPECT_NE( US_SimCache::dataset_key( simparams2, edata ), dskey );
   simparams2        = simparams;
   simparams2.bottom = 7.25;
   EXPECT_NE( US_SimCache::dataset_key( simparams2, edata ), dskey );
   simparams2           = simparams;
   simparams2.simpoints = simparams.simpoints + 10;
   EXPECT_NE( US_SimCache::dataset_key( simparams2, edata ), dskey );
   simparams2 = simparams;
   simparams2.speed_step << US_SimulationParameters::SpeedProfile();
   EXPECT_NE( US_SimCache::dataset_key( simparams2, edata ), dskey );

   // A user mesh matters only for the user mesh type
   simparams2             = simparams;
   simparams2.mesh_radius = { 6.0, 6.5, 7.0 };
   EXPECT_EQ( US_SimCache::dataset_key( simparams2, edata ), dskey );
   simparams2.meshType = US_SimulationParameters::USER;
   EXPECT_NE( US_SimCache::dataset_key( simparams2, edata ), dskey );
}

TEST_F( TestUSSimCache, SimKeyCoversComponent )
{
   const QByteArray              dskey( 32, 'd' );
   US_Model::SimulationComponent comp;
   comp.s                = 3.0e-13;
   comp.D                = 6.0e-7;
   comp.f_f0             = 1.3;
   const QByteArray skey = US_SimCache::sim_key( dskey, comp );
   EXPECT_EQ( US_SimCache::sim_key( dskey, comp ), skey );
   EXPECT_TRUE( skey.startsWith( dskey ) );

   US_Model::SimulationComponent comp2 = comp;
   comp2.s                             = 3.0000000000001e-13;
   EXPECT_NE( US_SimCache::sim_key( dskey, comp2 ), skey );
   comp2   = comp;
   comp2.D = 6.1e-7;
   EXPECT_NE( US_SimCache::sim_key( dskey, comp2 ), skey );
   comp2                      = comp;
   comp2.signal_concentration = 2.0;
   EXPECT_NE( US_SimCache::sim_key( dskey, comp2 ), skey );
   EXPECT_NE( US_SimCache::sim_key( QByteArray( 32, 'e' ), comp ), skey );

   // Names do not matter
   comp2      = comp;
   comp2.name = "other";
   EXPECT_EQ( US_SimCache::sim_key( dskey, comp2 ), skey );
}

TEST_F( TestUSSimCache, DebugTextControlsDefaultCache )
{
   US_Settings::set_debug_text( QStringList() << "SimCache=off" );
   EXPECT_EQ( US_SimCache::default_max_bytes(), 0 );
   EXPECT_EQ( US_SimCache::create_default(), nullptr );

   US_Settings::set_debug_text( QStringList() << "SimCacheMB=300" );
   EXPECT_EQ( US_SimCache::default_max_bytes(), 300LL * 1024 * 1024 );
   std::unique_ptr<US_SimCache> cache( US_SimCache::create_default() );
   ASSERT_NE( cache, nullptr );
   EXPECT_EQ( cache->stats().max_bytes, 300LL * 1024 * 1024 );

   // By default:  a share of available memory, from 256 MB to 4 GB
   US_Settings::set_debug_text( QStringList() );
   const qint64 defbytes = US_SimCache::default_max_bytes();
   EXPECT_GE( defbytes, 256LL * 1024 * 1024 );
   EXPECT_LE( defbytes, 4096LL * 1024 * 1024 );
}

TEST_F( TestUSSimCache, ConcurrentUseIsSafe )
{
   // Threads storing and fetching the same keys get the stored values
   US_SimCache      cache( 40 * sbytes );
   std::atomic<int> wrong{ 0 };
   QList<QThread*>  threads;
   for ( int tt = 0; tt < 6; tt++ )
   {
      threads << QThread::create(
         [&cache, &wrong, tt]()
         {
            for ( int ii = 0; ii < 400; ii++ )
            {
               const int          kk = ( ii * 7 + tt ) % 60;
               US_DataIO::RawData simdat;
               if ( cache.fetch( key( kk ), simdat ) )
               {
                  if ( !sameValues( simdat, makeSim( kk ) ) )
                  {
                     wrong++;
                  }
               }
               else
               {
                  cache.store( key( kk ), makeSim( kk ), ( kk % 5 ) == 0 );
               }
            }
         } );
   }
   for ( QThread* thr : threads )
   {
      thr->start();
   }
   for ( QThread* thr : threads )
   {
      thr->wait();
      delete thr;
   }

   EXPECT_EQ( wrong.load(), 0 );
   US_SimCache::Stats st = cache.stats();
   EXPECT_GT( st.hits, 0 );
   EXPECT_LE( st.bytes, st.max_bytes );
   EXPECT_EQ( st.survivors, 12 ); // Keys 0, 5, ..., 55 always fit
}
