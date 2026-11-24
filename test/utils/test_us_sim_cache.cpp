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
   US_DataIO::RawData simdat{};
   simdat.cell    = 1;
   simdat.channel = 'S';
   for ( int rr = 0; rr < 10; rr++ )
   {
      simdat.xvalues << 6.0 + 0.1 * rr;
   }
   for ( int ss = 0; ss < 4; ss++ )
   {
      US_DataIO::Scan scan{};
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
         ASSERT_NO_FATAL_FAILURE( QtTestBase::SetUp() );
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
            US_DataIO::Scan scan{};
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
   US_DataIO::RawData fetched{};

   EXPECT_FALSE( cache.fetch( key( 1 ), fetched ) );
   EXPECT_TRUE( cache.store( key( 1 ), simdat, false ) );
   ASSERT_TRUE( cache.fetch( key( 1 ), fetched ) );
   EXPECT_TRUE( sameValues( fetched, simdat ) );
   EXPECT_FALSE( cache.fetch( key( 2 ), fetched ) );

   // Changing a fetched simulation does not change the cached one
   fetched.scanData[0].rvalues[0] = -1.0;
   US_DataIO::RawData again{};
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

   US_DataIO::RawData fetched{};
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

   US_DataIO::RawData fetched{};
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

   US_DataIO::RawData fetched{};
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

   US_DataIO::RawData fetched{};
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
   US_DataIO::EditedData   edata{};
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

TEST_F( TestUSSimCache, DatasetKeyMemoGivesTheSameKeys )
{
   US_SimulationParameters simparams;
   US_DataIO::EditedData   edata{};
   makeDataSet( simparams, edata );
   const QByteArray dskey = US_SimCache::dataset_key( simparams, edata );

   US_SimCache      cache( 1024 * 1024 );
   EXPECT_EQ( cache.dataset_key_memo( simparams, edata ), dskey ); // Memo off
   cache.set_key_memo( true );
   EXPECT_EQ( cache.dataset_key_memo( simparams, edata ), dskey );
   EXPECT_EQ( cache.dataset_key_memo( simparams, edata ), dskey ); // Memorized

   // Scalars are compared by value
   simparams.meniscus += 0.01;
   const QByteArray menkey = US_SimCache::dataset_key( simparams, edata );
   EXPECT_NE( menkey, dskey );
   EXPECT_EQ( cache.dataset_key_memo( simparams, edata ), menkey );
   edata.scanData[2].seconds += 1.0;
   EXPECT_EQ( cache.dataset_key_memo( simparams, edata ), US_SimCache::dataset_key( simparams, edata ) );

   // Vectors by address:  a copy (new storage) gets its own key, and a
   //  vector changed in place needs forget_dataset_keys()
   US_DataIO::EditedData edata2 = edata;
   edata2.xvalues[4] += 1.0e-9; // Detaches from edata
   EXPECT_EQ( cache.dataset_key_memo( simparams, edata2 ), US_SimCache::dataset_key( simparams, edata2 ) );
   const QByteArray before = cache.dataset_key_memo( simparams, edata2 );
   edata2.xvalues[5] += 1.0e-9;                                      // In place
   EXPECT_EQ( cache.dataset_key_memo( simparams, edata2 ), before ); // Stale
   cache.forget_dataset_keys();
   EXPECT_EQ( cache.dataset_key_memo( simparams, edata2 ), US_SimCache::dataset_key( simparams, edata2 ) );
   EXPECT_NE( cache.dataset_key_memo( simparams, edata2 ), before );
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
}

TEST_F( TestUSSimCache, FloatModeReturnsRoundedSimulationExactly )
{
   // A simulation rounded to float precision is fetched exactly, and takes
   //  less room than as doubles
   for ( bool lru : { false, true } )
   {
      SCOPED_TRACE( lru ? "lru" : "default" );
      US_SimCache cache( 10 * sbytes, true );
      cache.set_lru( lru );
      US_DataIO::RawData simdat = makeSim( 0.1 );
      US_SimCache::round_to_float( simdat );
      EXPECT_FALSE( sameValues( simdat, makeSim( 0.1 ) ) );
      EXPECT_TRUE( cache.store( key( 0 ), simdat, false ) );

      US_DataIO::RawData fetched;
      ASSERT_TRUE( cache.fetch( key( 0 ), fetched ) );
      EXPECT_TRUE( sameValues( fetched, simdat ) );
      EXPECT_EQ( fetched.scanData[3].seconds, simdat.scanData[3].seconds );

      // Modifying a fetched copy changes neither the cache nor other copies
      US_DataIO::RawData again;
      fetched.scanData[0].rvalues[0] = -1.0;
      ASSERT_TRUE( cache.fetch( key( 0 ), again ) );
      EXPECT_TRUE( sameValues( again, simdat ) );

      const US_SimCache::Stats st = cache.stats();
      EXPECT_TRUE( st.floats );
      EXPECT_EQ( st.bytes, US_SimCache::sim_bytes( simdat, true ) );
      EXPECT_LT( st.bytes, sbytes );
   }
}

TEST_F( TestUSSimCache, FloatModeRoundsUnroundedSimulation )
{
   // An unrounded simulation is stored rounded:  fetched as round_to_float
   //  gives it
   US_SimCache        cache( 10 * sbytes, true );
   US_DataIO::RawData simdat = makeSim( 0.3 );
   EXPECT_TRUE( cache.store( key( 0 ), simdat, false ) );
   US_SimCache::round_to_float( simdat );

   US_DataIO::RawData fetched;
   ASSERT_TRUE( cache.fetch( key( 0 ), fetched ) );
   EXPECT_TRUE( sameValues( fetched, simdat ) );
}

TEST_F( TestUSSimCache, SimFormsHoldTheSameReadings )
{
   // Float form:  the readings rounded, read back exactly;  double form:
   //  the simulation itself.  Both give it back whole, and their bytes
   //  are what sim_bytes estimates.
   const US_DataIO::RawData simdat  = makeSim( 0.7 );
   US_DataIO::RawData       rounded = simdat;
   US_SimCache::round_to_float( rounded );

   const US_SimCache::Sim fsim = US_SimCache::Sim::from_raw( simdat, true );
   const US_SimCache::Sim dsim = US_SimCache::Sim::from_raw( simdat, false );
   EXPECT_TRUE( fsim.is_float() );
   EXPECT_FALSE( dsim.is_float() );
   ASSERT_EQ( fsim.scanCount(), simdat.scanData.size() );
   EXPECT_EQ( fsim.pointCount(), simdat.xvalues.size() );

   for ( int ss = 0; ss < fsim.scanCount(); ss++ )
   {
      for ( int rr = 0; rr < fsim.pointCount(); rr++ )
      {
         EXPECT_EQ( (double) fsim.fscan( ss )[rr], rounded.scanData[ss].rvalues[rr] );
         EXPECT_EQ( fsim.value( ss, rr ), rounded.scanData[ss].rvalues[rr] );
         EXPECT_EQ( dsim.value( ss, rr ), simdat.scanData[ss].rvalues[rr] );
      }
   }

   US_DataIO::RawData back;
   fsim.to_raw( back );
   EXPECT_TRUE( sameValues( back, rounded ) );
   EXPECT_EQ( back.scanData[2].seconds, simdat.scanData[2].seconds );
   dsim.to_raw( back );
   EXPECT_TRUE( sameValues( back, simdat ) );
   fsim.to_float().to_raw( back );
   EXPECT_TRUE( sameValues( back, rounded ) );
   dsim.to_float().to_raw( back );
   EXPECT_TRUE( sameValues( back, rounded ) );

   EXPECT_EQ( fsim.bytes(), US_SimCache::sim_bytes( simdat, true ) );
   EXPECT_EQ( dsim.bytes(), US_SimCache::sim_bytes( simdat, false ) );
}

TEST_F( TestUSSimCache, SimPackRestoresBothFormsExactly )
{
   // pack_bytes/unpack (the format of the MPI node cache's window):  each
   //  form comes back in its own form with the same readings and scans;  the
   //  float form takes about half the bytes;  a pack() of a RawData is read
   //  as the double form
   const US_DataIO::RawData simdat  = makeSim( 0.9 );
   US_DataIO::RawData       rounded = simdat;
   US_SimCache::round_to_float( rounded );

   const US_SimCache::Sim fsim   = US_SimCache::Sim::from_raw( simdat, true );
   const US_SimCache::Sim dsim   = US_SimCache::Sim::from_raw( simdat, false );
   const QByteArray       fbytes = fsim.pack_bytes();
   const QByteArray       dbytes = dsim.pack_bytes();
   EXPECT_EQ( dbytes, US_SimCache::pack( simdat ) );

   US_SimCache::Sim   back;
   US_DataIO::RawData raw;
   ASSERT_TRUE( US_SimCache::Sim::unpack( fbytes.constData(), fbytes.size(), back ) );
   EXPECT_TRUE( back.is_float() );
   back.to_raw( raw );
   EXPECT_TRUE( sameValues( raw, rounded ) );
   EXPECT_EQ( raw.scanData[3].seconds, simdat.scanData[3].seconds );
   EXPECT_EQ( back.bytes(), fsim.bytes() );

   ASSERT_TRUE( US_SimCache::Sim::unpack( dbytes.constData(), dbytes.size(), back ) );
   EXPECT_FALSE( back.is_float() );
   back.to_raw( raw );
   EXPECT_TRUE( sameValues( raw, simdat ) );

   // Larger readings:  the float form near half the bytes
   US_DataIO::RawData big = makeSim( 0.1 );
   for ( US_DataIO::Scan& scan : big.scanData )
   {
      scan.rvalues.fill( 0.25, 2000 );
   }
   big.xvalues.fill( 6.0, 2000 );
   const qsizetype nf = US_SimCache::Sim::from_raw( big, true ).pack_bytes().size();
   const qsizetype nd = US_SimCache::Sim::from_raw( big, false ).pack_bytes().size();
   EXPECT_LT( nf, nd * 3 / 4 );
}

TEST_F( TestUSSimCache, SimUnpackRejectsDamagedBytes )
{
   // Truncated or padded float-form bytes are refused (never read past the
   //  end):  a window slot holds exactly one serialized simulation
   const QByteArray bytes = US_SimCache::Sim::from_raw( makeSim( 0.4 ), true ).pack_bytes();
   US_SimCache::Sim sim;

   const qsizetype  size = bytes.size();

   for ( qsizetype len : { (qsizetype) 0, (qsizetype) 4, (qsizetype) 8, (qsizetype) 20, size / 2, size - 1 } )
   {
      EXPECT_FALSE( US_SimCache::Sim::unpack( bytes.constData(), len, sim ) ) << len;
   }

   const QByteArray padded = bytes + QByteArray( 4, '\0' );
   EXPECT_FALSE( US_SimCache::Sim::unpack( padded.constData(), padded.size(), sim ) );
   EXPECT_TRUE( US_SimCache::Sim::unpack( bytes.constData(), bytes.size(), sim ) );
}

TEST_F( TestUSSimCache, FloatModeSharesSimsWithoutCopies )
{
   // In float mode a stored float-form simulation is kept as it is, and
   //  every fetch_sim gives the same readings (no copy);  fetch() and
   //  fetch_sim() see the same values
   for ( bool lru : { false, true } )
   {
      SCOPED_TRACE( lru ? "lru" : "default" );
      US_SimCache cache( 10 * sbytes, true );
      cache.set_lru( lru );
      const US_SimCache::Sim sim = US_SimCache::Sim::from_raw( makeSim( 0.2 ), true );
      EXPECT_TRUE( cache.store_sim( key( 0 ), sim, false ) );

      US_SimCache::Sim first, second;
      ASSERT_TRUE( cache.fetch_sim( key( 0 ), first ) );
      ASSERT_TRUE( cache.fetch_sim( key( 0 ), second ) );
      EXPECT_TRUE( first.is_float() );
      EXPECT_EQ( first.fscan( 0 ), sim.fscan( 0 ) );
      EXPECT_EQ( second.fscan( 3 ), sim.fscan( 3 ) );

      US_DataIO::RawData fetched, expected;
      ASSERT_TRUE( cache.fetch( key( 0 ), fetched ) );
      sim.to_raw( expected );
      EXPECT_TRUE( sameValues( fetched, expected ) );

      // A double-form simulation is stored in float form (rounded)
      EXPECT_TRUE( cache.store_sim( key( 1 ), US_SimCache::Sim::from_raw( makeSim( 0.4 ), false ), false ) );
      US_SimCache::Sim stored;
      ASSERT_TRUE( cache.fetch_sim( key( 1 ), stored ) );
      EXPECT_TRUE( stored.is_float() );
      US_DataIO::RawData rounded = makeSim( 0.4 );
      US_SimCache::round_to_float( rounded );
      stored.to_raw( fetched );
      EXPECT_TRUE( sameValues( fetched, rounded ) );
      EXPECT_EQ( cache.stats().bytes, sim.bytes() + stored.bytes() );
   }
}

TEST_F( TestUSSimCache, DoubleModeSimsAreDoubleForm )
{
   // Without float mode, fetch_sim gives the stored simulation (double
   //  form, exactly) and store_sim of a float-form one stores its readings
   US_SimCache cache( 10 * sbytes );
   EXPECT_TRUE( cache.store_sim( key( 0 ), US_SimCache::Sim::from_raw( makeSim( 0.5 ), false ), false ) );
   US_SimCache::Sim sim;
   ASSERT_TRUE( cache.fetch_sim( key( 0 ), sim ) );
   EXPECT_FALSE( sim.is_float() );
   US_DataIO::RawData fetched;
   sim.to_raw( fetched );
   EXPECT_TRUE( sameValues( fetched, makeSim( 0.5 ) ) );

   EXPECT_TRUE( cache.store_sim( key( 1 ), US_SimCache::Sim::from_raw( makeSim( 0.6 ), true ), false ) );
   ASSERT_TRUE( cache.fetch( key( 1 ), fetched ) );
   US_DataIO::RawData rounded = makeSim( 0.6 );
   US_SimCache::round_to_float( rounded );
   EXPECT_TRUE( sameValues( fetched, rounded ) );
   EXPECT_FALSE( cache.fetch_sim( key( 2 ), sim ) );
}

TEST_F( TestUSSimCache, DebugTextControlsFloatMode )
{
   US_Settings::set_debug_text( QStringList() << "SimCacheMB=300" );
   EXPECT_FALSE( US_SimCache::float_sims() );
   std::unique_ptr<US_SimCache> dcache( US_SimCache::create_default() );
   ASSERT_NE( dcache, nullptr );
   EXPECT_FALSE( dcache->stats().floats );
   EXPECT_FALSE( dcache->float_mode() );

   US_Settings::set_debug_text( QStringList() << "SimCacheMB=300" << "SimFloat=on" );
   EXPECT_TRUE( US_SimCache::float_sims() );
   std::unique_ptr<US_SimCache> fcache( US_SimCache::create_default() );
   ASSERT_NE( fcache, nullptr );
   EXPECT_TRUE( fcache->stats().floats );
   EXPECT_TRUE( fcache->float_mode() );
   EXPECT_TRUE( fcache->stats_text().contains( "(floats)" ) );
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
               US_DataIO::RawData simdat{};
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

// --- Least-recently-used mode --------------------------------------------------

TEST_F( TestUSSimCache, LruEvictsLeastRecentlyUsedInABatch )
{
   US_SimCache cache( 10 * sbytes );
   cache.set_lru( true );
   for ( int ii = 0; ii < 10; ii++ )
   {
      ASSERT_TRUE( cache.store( key( ii ), makeSim( ii ), false ) );
   }

   // A fetch counts as a use, so key 0 is no longer the oldest
   US_DataIO::RawData fetched{};
   ASSERT_TRUE( cache.fetch( key( 0 ), fetched ) );

   // Room for one more means going down to 90% of the cap besides the new one
   ASSERT_TRUE( cache.store( key( 10 ), makeSim( 10 ), false ) );
   EXPECT_EQ( cache.stats().evicted, 2 );
   EXPECT_EQ( cache.stats().rejected, 0 );
   EXPECT_LE( cache.stats().bytes, cache.stats().max_bytes );

   EXPECT_FALSE( cache.fetch( key( 1 ), fetched ) );
   EXPECT_FALSE( cache.fetch( key( 2 ), fetched ) );
   for ( int ii : { 0, 3, 4, 5, 6, 7, 8, 9, 10 } )
   {
      ASSERT_TRUE( cache.fetch( key( ii ), fetched ) ) << ii;
      EXPECT_TRUE( sameValues( fetched, makeSim( ii ) ) ) << ii;
   }
}

TEST_F( TestUSSimCache, LruIgnoresSurvivorFlags )
{
   // The default policy would refuse the new simulations here
   US_SimCache cache( 4 * sbytes );
   cache.set_lru( true );
   for ( int ii = 0; ii < 12; ii++ )
   {
      EXPECT_TRUE( cache.store( key( ii ), makeSim( ii ), true ) ) << ii;
   }
   US_DataIO::RawData fetched{};
   EXPECT_TRUE( cache.fetch( key( 11 ), fetched ) );
   EXPECT_FALSE( cache.fetch( key( 0 ), fetched ) );
   EXPECT_EQ( cache.stats().rejected, 0 );
   EXPECT_GT( cache.stats().evicted, 0 );
   EXPECT_LE( cache.stats().bytes, cache.stats().max_bytes );
}

TEST_F( TestUSSimCache, LruStoringAgainCountsAsUse )
{
   US_SimCache cache( 10 * sbytes );
   cache.set_lru( true );
   for ( int ii = 0; ii < 10; ii++ )
   {
      ASSERT_TRUE( cache.store( key( ii ), makeSim( ii ), false ) );
   }
   ASSERT_TRUE( cache.store( key( 0 ), makeSim( 0 ), false ) );
   EXPECT_EQ( cache.stats().entries, 10 ); // Not stored twice

   ASSERT_TRUE( cache.store( key( 10 ), makeSim( 10 ), false ) );
   US_DataIO::RawData fetched{};
   EXPECT_TRUE( cache.fetch( key( 0 ), fetched ) );
   EXPECT_FALSE( cache.fetch( key( 1 ), fetched ) );
}

TEST_F( TestUSSimCache, LruRejectsASimulationLargerThanTheCap )
{
   US_SimCache cache( sbytes - 1 );
   cache.set_lru( true );
   EXPECT_FALSE( cache.store( key( 0 ), makeSim( 0 ), false ) );
   EXPECT_EQ( cache.stats().rejected, 1 );
   EXPECT_EQ( cache.stats().entries, 0 );
}

TEST_F( TestUSSimCache, StatsTextReportsEvictedAndRejected )
{
   US_SimCache cache( 2 * sbytes );
   cache.set_lru( true );
   for ( int ii = 0; ii < 4; ii++ )
   {
      cache.store( key( ii ), makeSim( ii ), false );
   }
   const QString text = cache.stats_text();
   EXPECT_TRUE( text.contains( QString( "%1 evicted" ).arg( cache.stats().evicted ) ) ) << text.toStdString();
   EXPECT_TRUE( text.contains( "0 rejected" ) ) << text.toStdString();
}

TEST_F( TestUSSimCache, LruConcurrentUseIsSafe )
{
   US_SimCache cache( 40 * sbytes );
   cache.set_lru( true );
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
               US_DataIO::RawData simdat{};
               if ( cache.fetch( key( kk ), simdat ) )
               {
                  if ( !sameValues( simdat, makeSim( kk ) ) )
                  {
                     wrong++;
                  }
               }
               else
               {
                  cache.store( key( kk ), makeSim( kk ), false );
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
   EXPECT_LE( cache.stats().bytes, cache.stats().max_bytes );
}

TEST_F( TestUSSimCache, FloatModeConcurrentUseIsSafe )
{
   // Threads storing and fetching the same keys in float mode get the
   //  rounded values, in the default and the LRU mode
   for ( bool lru : { false, true } )
   {
      SCOPED_TRACE( lru ? "lru" : "default" );
      US_SimCache cache( 20 * US_SimCache::sim_bytes( makeSim( 0.0 ), true ), true );
      cache.set_lru( lru );
      std::atomic<int> wrong{ 0 };
      QList<QThread*>  threads;
      for ( int tt = 0; tt < 6; tt++ )
      {
         threads << QThread::create(
            [&cache, &wrong, tt]()
            {
               for ( int ii = 0; ii < 400; ii++ )
               {
                  const int          kk      = ( ii * 7 + tt ) % 60;
                  US_DataIO::RawData rounded = makeSim( kk );
                  US_SimCache::round_to_float( rounded );
                  US_DataIO::RawData simdat;
                  if ( cache.fetch( key( kk ), simdat ) )
                  {
                     if ( !sameValues( simdat, rounded ) )
                     {
                        wrong++;
                     }
                     simdat.scanData[0].rvalues[0] = -1.0; // Must not reach the cache
                  }
                  else
                  {
                     cache.store( key( kk ), rounded, ( kk % 5 ) == 0 );
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
      EXPECT_GT( cache.stats().hits, 0 );
      EXPECT_LE( cache.stats().bytes, cache.stats().max_bytes );
   }
}

// --- Memo of data set keys -----------------------------------------------------

TEST_F( TestUSSimCache, KeyMemoReturnsTheDatasetKey )
{
   US_SimulationParameters simparams;
   US_DataIO::EditedData   edata{};
   makeDataSet( simparams, edata );
   const QByteArray full = US_SimCache::dataset_key( simparams, edata );

   US_SimCache      cache( 10 * sbytes );
   EXPECT_EQ( cache.dataset_key_memo( simparams, edata ), full ); // Memo off (default)
   cache.set_key_memo( true );
   EXPECT_EQ( cache.dataset_key_memo( simparams, edata ), full );
   EXPECT_EQ( cache.dataset_key_memo( simparams, edata ), full ); // From the memo
}

TEST_F( TestUSSimCache, KeyMemoSeesScalarAndResizedChanges )
{
   US_SimulationParameters simparams;
   US_DataIO::EditedData   edata{};
   makeDataSet( simparams, edata );
   US_SimCache cache( 10 * sbytes );
   cache.set_key_memo( true );
   const QByteArray first = cache.dataset_key_memo( simparams, edata );

   simparams.meniscus = 5.95;
   EXPECT_NE( cache.dataset_key_memo( simparams, edata ), first );
   EXPECT_EQ( cache.dataset_key_memo( simparams, edata ), US_SimCache::dataset_key( simparams, edata ) );
   simparams.meniscus = 5.9;
   EXPECT_EQ( cache.dataset_key_memo( simparams, edata ), first );

   edata.xvalues << 7.0;
   EXPECT_NE( cache.dataset_key_memo( simparams, edata ), first );
}

TEST_F( TestUSSimCache, KeyMemoLimitationAndClear )
{
   // Documented limit of the memo:  a value changed in place (same address and
   //  size) is not seen.  Without the memo, or after clear(), it is.
   US_SimulationParameters simparams;
   US_DataIO::EditedData   edata{};
   makeDataSet( simparams, edata );
   US_SimCache cache( 10 * sbytes );
   cache.set_key_memo( true );
   const QByteArray first = cache.dataset_key_memo( simparams, edata );

   edata.xvalues[3] += 0.001;
   EXPECT_EQ( cache.dataset_key_memo( simparams, edata ), first );

   cache.clear();
   EXPECT_NE( cache.dataset_key_memo( simparams, edata ), first );

   cache.set_key_memo( false );
   edata.xvalues[3] += 0.001;
   EXPECT_EQ( cache.dataset_key_memo( simparams, edata ), US_SimCache::dataset_key( simparams, edata ) );
}

// ---------------------------------------------------------------------------
// Serialization for caches shared between processes
// ---------------------------------------------------------------------------

namespace
{
// A simulation with every field set, including ones the simulator copies
//  from the experiment grid
US_DataIO::RawData makeFullSim()
{
   US_DataIO::RawData simdat = makeSim( 0.25 );
   memcpy( simdat.type, "RA", 2 );
   for ( int ii = 0; ii < 16; ii++ )
   {
      simdat.rawGUID[ii] = (char) ( 17 * ii + 3 );
   }
   simdat.description = QString::fromUtf8( "sim \xc2\xb5 test" );
   for ( int ss = 0; ss < simdat.scanData.size(); ss++ )
   {
      US_DataIO::Scan& scan = simdat.scanData[ss];
      scan.temperature      = 20.1 + ss;
      scan.rpm              = 50000.0;
      scan.omega2t          = 1.0e10 * ( ss + 1 ) / 3.0;
      scan.wavelength       = 280.0;
      scan.plateau          = 0.7 + ss / 7.0;
      scan.delta_r          = 0.001;
      scan.nz_stddev        = ( ss == 1 );
      scan.stddevs          = QVector<double>( scan.rvalues.size(), ss == 1 ? 0.01 : 0.0 );
      scan.interpolated     = QByteArray( 2, (char) ss );
   }
   return simdat;
}
} // namespace

TEST( SimCachePack, RoundTripRestoresEveryField )
{
   US_DataIO::RawData in    = makeFullSim();
   QByteArray         bytes = US_SimCache::pack( in );
   US_DataIO::RawData out{};

   ASSERT_TRUE( US_SimCache::unpack( bytes.constData(), bytes.size(), out ) );

   EXPECT_EQ( memcmp( out.type, in.type, 2 ), 0 );
   EXPECT_EQ( memcmp( out.rawGUID, in.rawGUID, 16 ), 0 );
   EXPECT_EQ( out.cell, in.cell );
   EXPECT_EQ( out.channel, in.channel );
   EXPECT_EQ( out.description, in.description );
   EXPECT_TRUE( sameValues( in, out ) );
   ASSERT_EQ( out.scanData.size(), in.scanData.size() );

   for ( int ss = 0; ss < in.scanData.size(); ss++ )
   {
      SCOPED_TRACE( ss );
      const US_DataIO::Scan& a = in.scanData[ss];
      const US_DataIO::Scan& b = out.scanData[ss];
      EXPECT_EQ( memcmp( &a.temperature, &b.temperature, sizeof( double ) ), 0 );
      EXPECT_EQ( memcmp( &a.omega2t, &b.omega2t, sizeof( double ) ), 0 );
      EXPECT_EQ( memcmp( &a.plateau, &b.plateau, sizeof( double ) ), 0 );
      EXPECT_EQ( a.rpm, b.rpm );
      EXPECT_EQ( a.seconds, b.seconds );
      EXPECT_EQ( a.wavelength, b.wavelength );
      EXPECT_EQ( a.delta_r, b.delta_r );
      EXPECT_EQ( a.nz_stddev, b.nz_stddev );
      EXPECT_EQ( a.stddevs, b.stddevs );
      EXPECT_EQ( a.interpolated, b.interpolated );
   }
}

TEST( SimCachePack, TruncatedOrPaddedBytesAreRejected )
{
   QByteArray         bytes = US_SimCache::pack( makeFullSim() );
   US_DataIO::RawData out{};

   for ( int cut : { 0, 1, 18, (int) bytes.size() / 2, (int) bytes.size() - 1 } )
   {
      SCOPED_TRACE( cut );
      EXPECT_FALSE( US_SimCache::unpack( bytes.constData(), cut, out ) );
   }

   QByteArray padded = bytes + QByteArray( 4, '\0' );
   EXPECT_FALSE( US_SimCache::unpack( padded.constData(), padded.size(), out ) );
}

TEST( SimCacheVirtual, SubclassOverridesFetchAndStore )
{
   // A store that only counts calls, used through a base-class pointer as
   //  US_SolveSim uses its cache
   class CountingCache : public US_SimCache
   {
      public:
         CountingCache() : US_SimCache( 0 )
         {
         }
         bool fetch( const QByteArray&, US_DataIO::RawData& ) override
         {
            fetches++;
            return false;
         }
         bool store( const QByteArray&, const US_DataIO::RawData&, bool ) override
         {
            stores++;
            return true;
         }
         int fetches = 0;
         int stores  = 0;
   };

   std::unique_ptr<CountingCache> counting( new CountingCache );
   US_SimCache*                   cache  = counting.get();
   US_DataIO::RawData             simdat = makeSim( 1.0 );

   EXPECT_FALSE( cache->fetch( "k", simdat ) );
   EXPECT_TRUE( cache->store( "k", simdat, false ) );
   EXPECT_EQ( counting->fetches, 1 );
   EXPECT_EQ( counting->stores, 1 );

   // fetch_sim and store_sim (as US_SolveSim uses them) go through the
   //  overrides too
   US_SimCache::Sim sim = US_SimCache::Sim::from_raw( simdat, true );
   EXPECT_FALSE( cache->fetch_sim( "k", sim ) );
   EXPECT_TRUE( cache->store_sim( "k", sim, true ) );
   EXPECT_EQ( counting->fetches, 2 );
   EXPECT_EQ( counting->stores, 2 );
}
