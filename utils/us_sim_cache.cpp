//! \file us_sim_cache.cpp
#include "us_sim_cache.h"
#include "us_memory.h"
#include "us_settings.h"

// Default memory cap:  this share of available memory, within limits (MB)
static const double            DEF_SHARE  = 0.25;
static const qint64            DEF_MAX_MB = 4096;
static const qint64            DEF_MIN_MB = 256;
static const qint64            MB         = 1024 * 1024;

// Append the bytes of a value to a key
template <class T> static void add_bytes( QByteArray& key, const T& value )
{
   key.append( reinterpret_cast<const char*>( &value ), sizeof( T ) );
}

// Append the values of a vector, preceded by its size, to a key
static void add_vector( QByteArray& key, const QVector<double>& values )
{
   add_bytes( key, (qint64) values.size() );
   key.append( reinterpret_cast<const char*>( values.constData() ), values.size() * (qsizetype) sizeof( double ) );
}

// Append a string, preceded by its size, to a key
static void add_string( QByteArray& key, const QString& text )
{
   QByteArray utf8 = text.toUtf8();
   add_bytes( key, (qint64) utf8.size() );
   key.append( utf8 );
}

// Create a cache with a memory cap in bytes
US_SimCache::US_SimCache( qint64 max_bytes ) :
    max_bytes( qMax( (qint64) 0, max_bytes ) ),
    nbytes( 0 ),
    nsurvivors( 0 ),
    nstored( 0 ),
    nevicted( 0 ),
    nrejected( 0 ),
    nhits( 0 ),
    nmisses( 0 )
{
}

// Create a cache with the default cap, unless debug text turns caching off
US_SimCache* US_SimCache::create_default()
{
   qint64 cap = default_max_bytes();

   return ( cap > 0 ) ? new US_SimCache( cap ) : nullptr;
}

// Default memory cap from debug text or available memory (0 for no cache)
qint64 US_SimCache::default_max_bytes()
{
   const QStringList dbgtxt = US_Settings::debug_text();

   for ( const QString& dtext : dbgtxt )
   {
      QString name  = dtext.section( "=", 0, 0 ).trimmed();
      QString value = dtext.section( "=", 1, 1 ).trimmed();

      if ( name.compare( "SimCache", Qt::CaseInsensitive ) == 0 )
      {
         if ( QStringList( { "off", "0", "false", "no" } ).contains( value, Qt::CaseInsensitive ) )
         {
            return 0;
         }
      }

      else if ( name.compare( "SimCacheMB", Qt::CaseInsensitive ) == 0 )
      {
         bool   ok    = false;
         qint64 capmb = value.toLongLong( &ok );

         if ( ok )
         {
            return qMax( (qint64) 0, capmb ) * MB;
         }
      }
   }

   int memavail = 0;
   US_Memory::memory_profile( &memavail );
   qint64 capmb = qRound64( (double) memavail * DEF_SHARE );

   return qBound( DEF_MIN_MB, capmb, DEF_MAX_MB ) * MB;
}

// Key of the simulation parameters and experiment grid of a data set
QByteArray US_SimCache::dataset_key( const US_SimulationParameters& simparams, const US_DataIO::EditedData& edata )
{
   QByteArray vals;

   // Simulation parameters (a user mesh only matters for the USER mesh
   //  type;  programs keep other values in mesh_radius)
   if ( simparams.meshType == US_SimulationParameters::USER )
   {
      add_vector( vals, simparams.mesh_radius );
   }
   add_bytes( vals, (qint64) simparams.speed_step.size() );

   for ( const US_SimulationParameters::SpeedProfile& sp : simparams.speed_step )
   {
      add_bytes( vals, sp.duration_minutes );
      add_bytes( vals, sp.delay_minutes );
      add_bytes( vals, sp.w2t_first );
      add_bytes( vals, sp.w2t_last );
      add_bytes( vals, sp.avg_speed );
      add_bytes( vals, sp.speed_stddev );
      add_bytes( vals, sp.duration_hours );
      add_bytes( vals, sp.delay_hours );
      add_bytes( vals, sp.time_first );
      add_bytes( vals, sp.time_last );
      add_bytes( vals, sp.scans );
      add_bytes( vals, sp.rotorspeed );
      add_bytes( vals, sp.acceleration );
      add_bytes( vals, sp.set_speed );
      add_bytes( vals, sp.acceleration_flag );
   }

   add_bytes( vals, (qint64) simparams.sim_speed_prof.size() );

   for ( const US_SimulationParameters::SimSpeedProf& sp : simparams.sim_speed_prof )
   {
      add_bytes( vals, sp.acceleration );
      add_bytes( vals, sp.w2t_b_accel );
      add_bytes( vals, sp.w2t_e_accel );
      add_bytes( vals, sp.w2t_e_step );
      add_bytes( vals, sp.avg_speed );
      add_bytes( vals, sp.rotorspeed );
      add_bytes( vals, sp.duration );
      add_bytes( vals, sp.time_b_accel );
      add_bytes( vals, sp.time_e_accel );
      add_bytes( vals, sp.time_f_scan );
      add_bytes( vals, sp.time_l_scan );
      add_bytes( vals, sp.time_e_step );
      add_vector( vals, sp.rpm_timestate );
      add_vector( vals, sp.w2t_timestate );
   }

   add_bytes( vals, ( simparams.tsobj != nullptr ) );
   add_bytes( vals, (int) simparams.simpoints );
   add_bytes( vals, (int) simparams.meshType );
   add_bytes( vals, (int) simparams.gridType );
   add_bytes( vals, simparams.radial_resolution );
   add_bytes( vals, simparams.meniscus );
   add_bytes( vals, simparams.bottom );
   add_bytes( vals, simparams.temperature );
   add_bytes( vals, simparams.rnoise );
   add_bytes( vals, simparams.lrnoise );
   add_bytes( vals, simparams.tinoise );
   add_bytes( vals, simparams.rinoise );
   add_bytes( vals, simparams.baseline );
   add_bytes( vals, simparams.band_forming );
   add_bytes( vals, simparams.band_volume );
   add_bytes( vals, simparams.bottom_position );
   add_string( vals, simparams.rotorCalID );
   add_bytes( vals, simparams.rotorcoeffs[0] );
   add_bytes( vals, simparams.rotorcoeffs[1] );
   add_bytes( vals, simparams.firstScanIsConcentration );
   add_bytes( vals, simparams.sim );
   add_bytes( vals, simparams.cp_sector );
   add_bytes( vals, simparams.cp_pathlen );
   add_bytes( vals, simparams.cp_angle );
   add_bytes( vals, simparams.cp_width );

   // Experiment grid, as copied to a simulation by US_AstfemMath::initSimData
   add_string( vals, edata.cell );
   add_string( vals, edata.channel );
   add_string( vals, edata.description );
   add_vector( vals, edata.xvalues );
   add_bytes( vals, (qint64) edata.scanData.size() );

   for ( const US_DataIO::Scan& scan : edata.scanData )
   {
      add_bytes( vals, scan.temperature );
      add_bytes( vals, scan.rpm );
      add_bytes( vals, scan.seconds );
      add_bytes( vals, scan.omega2t );
      add_bytes( vals, scan.wavelength );
      add_bytes( vals, scan.plateau );
      add_bytes( vals, scan.delta_r );
      add_bytes( vals, (qint64) scan.interpolated.size() );
      vals.append( scan.interpolated );
   }

   return QCryptographicHash::hash( vals, QCryptographicHash::Sha256 );
}

// Key of a simulation:  data set key and component values
QByteArray US_SimCache::sim_key( const QByteArray& dskey, const US_Model::SimulationComponent& comp )
{
   QByteArray key = dskey;
   key.reserve( dskey.size() + 160 );
   add_bytes( key, comp.molar_concentration );
   add_bytes( key, comp.signal_concentration );
   add_bytes( key, comp.vbar20 );
   add_bytes( key, comp.mw );
   add_bytes( key, comp.s );
   add_bytes( key, comp.D );
   add_bytes( key, comp.f );
   add_bytes( key, comp.f_f0 );
   add_bytes( key, comp.extinction );
   add_bytes( key, comp.axial_ratio );
   add_bytes( key, comp.sigma );
   add_bytes( key, comp.delta );
   add_bytes( key, comp.oligomer );
   add_bytes( key, (int) comp.shape );
   add_bytes( key, comp.analyte_type );
   add_vector( key, comp.c0.radius );
   add_vector( key, comp.c0.concentration );
   return key;
}

// Approximate bytes a simulation takes in the cache
qint64 US_SimCache::sim_bytes( const US_DataIO::RawData& simdat )
{
   qint64 bytes = 256; // Entry, key and hash overhead

   for ( const US_DataIO::Scan& scan : simdat.scanData )
   {
      bytes += (qint64) sizeof( US_DataIO::Scan ) + 32 + (qint64) scan.rvalues.size() * (qint64) sizeof( double ) +
               scan.interpolated.size();
   }

   return bytes;
}

// Get a simulation
bool US_SimCache::fetch( const QByteArray& key, US_DataIO::RawData& simdat )
{
   {
      QReadLocker rlock( &lock );
      auto        it = sims.constFind( key );

      if ( it != sims.constEnd() )
      { // Implicitly shared copy:  values are copied only if modified
         simdat = it.value().simdat;
         nhits++;
         return true;
      }
   }

   nmisses++;
   return false;
}

// Store a simulation, or mark a stored one as a survivor
bool US_SimCache::store( const QByteArray& key, const US_DataIO::RawData& simdat, bool survivor )
{
   QWriteLocker wlock( &lock );
   auto         it = sims.find( key );

   if ( it != sims.end() )
   { // Already stored:  possibly now a survivor (its key stays in evictable)
      if ( survivor && !it.value().survivor )
      {
         it.value().survivor = true;
         nsurvivors++;
      }
      return true;
   }

   qint64 bytes = sim_bytes( simdat );

   if ( survivor )
   {
      make_room( bytes );
   }

   if ( nbytes + bytes > max_bytes )
   { // No room
      nrejected++;
      return false;
   }

   Entry entry;
   entry.simdat   = simdat;
   entry.bytes    = bytes;
   entry.survivor = survivor;
   sims.insert( key, entry );
   nbytes += bytes;
   nstored++;

   if ( survivor )
   {
      nsurvivors++;
   }
   else
   {
      evictable << key;
   }

   return true;
}

// Evict non-survivors, newest first, until "need" more bytes fit.  The oldest
//  are the subgrid simulations of the first pass, which refinement and Monte
//  Carlo iterations simulate again;  the newest are the least likely reused.
void US_SimCache::make_room( qint64 need )
{
   while ( nbytes + need > max_bytes && !evictable.isEmpty() )
   {
      QByteArray key = evictable.takeLast();
      auto       it  = sims.find( key );

      if ( it == sims.end() || it.value().survivor )
      {
         continue; // Gone or now a survivor
      }

      nbytes -= it.value().bytes;
      sims.erase( it );
      nevicted++;
   }
}

// Remove all simulations
void US_SimCache::clear()
{
   QWriteLocker wlock( &lock );
   sims.clear();
   evictable.clear();
   nbytes     = 0;
   nsurvivors = 0;
}

// Counts of cache use
US_SimCache::Stats US_SimCache::stats() const
{
   QReadLocker rlock( &lock );
   Stats       st;
   st.hits      = nhits;
   st.misses    = nmisses;
   st.stored    = nstored;
   st.evicted   = nevicted;
   st.rejected  = nrejected;
   st.entries   = sims.size();
   st.survivors = nsurvivors;
   st.bytes     = nbytes;
   st.max_bytes = max_bytes;
   return st;
}

// One-line text of the counts
QString US_SimCache::stats_text() const
{
   Stats  st     = stats();
   qint64 nfetch = st.hits + st.misses;
   double hitpc  = ( nfetch > 0 ) ? ( 100.0 * st.hits / (double) nfetch ) : 0.0;

   return QObject::tr( "Simulation cache:  %1 of %2 simulations reused (%3%);"
                       " %4 stored (%5 survivors) in %6 of %7 MB" )
      .arg( st.hits )
      .arg( nfetch )
      .arg( hitpc, 0, 'f', 1 )
      .arg( st.entries )
      .arg( st.survivors )
      .arg( (double) st.bytes / (double) MB, 0, 'f', 1 )
      .arg( st.max_bytes / MB );
}
