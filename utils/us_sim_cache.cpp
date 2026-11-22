//! \file us_sim_cache.cpp
#include "us_sim_cache.h"
#include "us_memory.h"
#include "us_settings.h"

#include <cstring>

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
    nmisses( 0 ),
    key_memo( false ),
    lru( false ),
    use_clock( 0 )
{
}

// Turn the memo of data set keys on or off
void US_SimCache::set_key_memo( bool on )
{
   QWriteLocker wlock( &lock );
   key_memo = on;
   dskey_memo.clear();
}

// Forget the memorized data set keys
void US_SimCache::forget_dataset_keys()
{
   QWriteLocker wlock( &lock );
   dskey_memo.clear();
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

// Append a vector to a key:  its values, or (for a fingerprint) only its
//  address and size
static void add_vector( QByteArray& key, const QVector<double>& values, bool fprint )
{
   if ( fprint )
   {
      add_bytes( key, (qint64) values.size() );
      add_bytes( key, (quintptr) values.constData() );
   }
   else
   {
      add_vector( key, values );
   }
}

// Append a byte array to a key:  its bytes, or (for a fingerprint) only its
//  address and size
static void add_array( QByteArray& key, const QByteArray& values, bool fprint )
{
   add_bytes( key, (qint64) values.size() );

   if ( fprint )
   {
      add_bytes( key, (quintptr) values.constData() );
   }
   else
   {
      key.append( values );
   }
}

// Values of the simulation parameters and experiment grid of a data set, or
//  a fingerprint of them:  all scalar values, but only the address and size
//  of each vector
static void dataset_values( QByteArray&                    vals,
                            const US_SimulationParameters& simparams,
                            const US_DataIO::EditedData&   edata,
                            bool                           fprint )
{
   // Simulation parameters (a user mesh only matters for the USER mesh
   //  type;  programs keep other values in mesh_radius)
   if ( simparams.meshType == US_SimulationParameters::USER )
   {
      add_vector( vals, simparams.mesh_radius, fprint );
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
      add_vector( vals, sp.rpm_timestate, fprint );
      add_vector( vals, sp.w2t_timestate, fprint );
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
   add_vector( vals, edata.xvalues, fprint );
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
      add_array( vals, scan.interpolated, fprint );
   }
}

// Key of the simulation parameters and experiment grid of a data set
QByteArray US_SimCache::dataset_key( const US_SimulationParameters& simparams, const US_DataIO::EditedData& edata )
{
   QByteArray vals;
   dataset_values( vals, simparams, edata, false );

   return QCryptographicHash::hash( vals, QCryptographicHash::Sha256 );
}

// Key of a data set, computed once for each fingerprint of its values
QByteArray US_SimCache::dataset_key_memo( const US_SimulationParameters& simparams, const US_DataIO::EditedData& edata )
{
   if ( !key_memo )
   {
      return dataset_key( simparams, edata );
   }

   QByteArray fprint;
   dataset_values( fprint, simparams, edata, true );

   {
      QReadLocker rlock( &lock );
      auto        it = dskey_memo.constFind( fprint );

      if ( it != dskey_memo.constEnd() )
      {
         return it.value();
      }
   }

   QByteArray   key = dataset_key( simparams, edata );
   QWriteLocker wlock( &lock );

   if ( dskey_memo.size() >= 256 )
   {
      dskey_memo.clear();
   }

   dskey_memo.insert( fprint, key );
   return key;
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

// Serialize a simulation:  every field of the data and its scans
QByteArray US_SimCache::pack( const US_DataIO::RawData& simdat )
{
   QByteArray data;
   data.reserve( sim_bytes( simdat ) + simdat.description.size() * 4 );

   data.append( simdat.type, 2 );
   data.append( simdat.rawGUID, 16 );
   add_bytes( data, simdat.cell );
   add_bytes( data, simdat.channel );
   add_string( data, simdat.description );
   add_vector( data, simdat.xvalues );
   add_bytes( data, (qint64) simdat.scanData.size() );

   for ( const US_DataIO::Scan& scan : simdat.scanData )
   {
      add_bytes( data, scan.temperature );
      add_bytes( data, scan.rpm );
      add_bytes( data, scan.seconds );
      add_bytes( data, scan.omega2t );
      add_bytes( data, scan.wavelength );
      add_bytes( data, scan.plateau );
      add_bytes( data, scan.delta_r );
      add_bytes( data, scan.nz_stddev );
      add_vector( data, scan.rvalues );
      add_vector( data, scan.stddevs );
      add_bytes( data, (qint64) scan.interpolated.size() );
      data.append( scan.interpolated );
   }

   return data;
}

namespace
{
// Sequential reader of pack() bytes; any overrun makes it invalid
class Unpacker
{
   public:
      Unpacker( const char* data, qint64 size ) : ptr( data ), end( data + size ), ok( true )
      {
      }

      template <class T> void get( T& value )
      {
         raw( &value, sizeof( T ) );
      }

      void raw( void* dest, qint64 nbytes )
      {
         if ( !ok || nbytes < 0 || end - ptr < nbytes )
         {
            ok = false;
            return;
         }
         memcpy( dest, ptr, nbytes );
         ptr += nbytes;
      }

      void vector( QVector<double>& values )
      {
         qint64 count = 0;
         get( count );
         if ( !ok || count < 0 || ( end - ptr ) / (qint64) sizeof( double ) < count )
         {
            ok = false;
            return;
         }
         values.resize( count );
         raw( values.data(), count * (qint64) sizeof( double ) );
      }

      void bytes( QByteArray& array )
      {
         qint64 count = 0;
         get( count );
         if ( !ok || count < 0 || end - ptr < count )
         {
            ok = false;
            return;
         }
         array = QByteArray( ptr, count );
         ptr += count;
      }

      const char* ptr;
      const char* end;
      bool        ok;
};
} // namespace

// Restore a simulation serialized by pack()
bool US_SimCache::unpack( const char* data, qint64 size, US_DataIO::RawData& simdat )
{
   Unpacker   in( data, size );
   QByteArray text;
   qint64     nscans = 0;

   in.raw( simdat.type, 2 );
   in.raw( simdat.rawGUID, 16 );
   in.get( simdat.cell );
   in.get( simdat.channel );
   in.bytes( text );
   simdat.description = QString::fromUtf8( text );
   in.vector( simdat.xvalues );
   in.get( nscans );

   if ( !in.ok || nscans < 0 || nscans > size )
   {
      return false;
   }

   simdat.scanData.resize( nscans );

   for ( US_DataIO::Scan& scan : simdat.scanData )
   {
      in.get( scan.temperature );
      in.get( scan.rpm );
      in.get( scan.seconds );
      in.get( scan.omega2t );
      in.get( scan.wavelength );
      in.get( scan.plateau );
      in.get( scan.delta_r );
      in.get( scan.nz_stddev );
      in.vector( scan.rvalues );
      in.vector( scan.stddevs );
      in.bytes( scan.interpolated );
   }

   return in.ok && in.ptr == in.end;
}

// Get a simulation
bool US_SimCache::fetch( const QByteArray& key, US_DataIO::RawData& simdat )
{
   if ( lru )
   { // Least-recently-used mode:  note the use
      QWriteLocker wlock( &lock );
      auto         it = sims.find( key );

      if ( it != sims.end() )
      {
         simdat              = it.value().simdat;
         it.value().last_use = ++use_clock;
         nhits++;
         return true;
      }
   }
   else
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

   if ( lru )
   { // Least-recently-used mode:  survivor flags are not used
      if ( it != sims.end() )
      {
         it.value().last_use = ++use_clock;
         return true;
      }

      qint64 bytes = sim_bytes( simdat );

      if ( bytes > max_bytes )
      {
         nrejected++;
         return false;
      }

      if ( nbytes + bytes > max_bytes )
      {
         evict_lru( bytes );
      }

      Entry entry;
      entry.simdat   = simdat;
      entry.bytes    = bytes;
      entry.survivor = false;
      entry.last_use = ++use_clock;
      sims.insert( key, entry );
      nbytes += bytes;
      nstored++;
      return true;
   }

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

// Least-recently-used mode:  evict the least recently used simulations,
//  down to 90% of the cap besides "need" bytes (so evictions come in
//  batches, each costing one sort of the use times)
void US_SimCache::evict_lru( qint64 need )
{
   QVector<QPair<qint64, QByteArray>> uses;
   uses.reserve( sims.size() );

   for ( auto it = sims.constBegin(); it != sims.constEnd(); ++it )
   {
      uses << qMakePair( it.value().last_use, it.key() );
   }

   std::sort( uses.begin(),
              uses.end(),
              []( const QPair<qint64, QByteArray>& a, const QPair<qint64, QByteArray>& b ) { return a.first < b.first; } );

   const qint64 target = max_bytes - max_bytes / 10 - need;

   for ( const auto& use : uses )
   {
      if ( nbytes <= target )
      {
         break;
      }

      auto it = sims.find( use.second );
      nbytes -= it.value().bytes;
      sims.erase( it );
      nevicted++;
   }
}

// Turn least-recently-used mode on or off
void US_SimCache::set_lru( bool on )
{
   QWriteLocker wlock( &lock );
   lru = on;
}

// Remove all simulations
void US_SimCache::clear()
{
   QWriteLocker wlock( &lock );
   sims.clear();
   evictable.clear();
   dskey_memo.clear();
   nbytes     = 0;
   nsurvivors = 0;
   wlock.unlock();
   forget_dataset_keys();
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
                       " %4 stored (%5 survivors) in %6 of %7 MB; %8 evicted, %9 rejected" )
      .arg( st.hits )
      .arg( nfetch )
      .arg( hitpc, 0, 'f', 1 )
      .arg( st.entries )
      .arg( st.survivors )
      .arg( (double) st.bytes / (double) MB, 0, 'f', 1 )
      .arg( st.max_bytes / MB )
      .arg( st.evicted )
      .arg( st.rejected );
}
