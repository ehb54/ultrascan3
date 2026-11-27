//! \file us_sim_cache.cpp
#include "us_sim_cache.h"
#include "us_memory.h"
#include "us_settings.h"

#include <atomic>
#include <cstdio>
#include <cstring>

#ifdef Q_OS_LINUX
#include <sys/mman.h>
#endif

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
US_SimCache::US_SimCache( qint64 max_bytes, bool floats ) :
    max_bytes( qMax( (qint64) 0, max_bytes ) ),
    floats( floats ),
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

   return ( cap > 0 ) ? new US_SimCache( cap, float_sims() ) : nullptr;
}

// Flag from debug text that simulations are rounded to float precision
bool US_SimCache::float_sims()
{
   const QStringList dbgtxt = US_Settings::debug_text();

   for ( const QString& dtext : dbgtxt )
   {
      QString name  = dtext.section( "=", 0, 0 ).trimmed();
      QString value = dtext.section( "=", 1, 1 ).trimmed();

      if ( name.compare( "SimFloat", Qt::CaseInsensitive ) == 0 )
      {
         return QStringList( { "on", "1", "true", "yes" } ).contains( value, Qt::CaseInsensitive );
      }
   }

   return false;
}

// Round the readings of a simulation to float precision
void US_SimCache::round_to_float( US_DataIO::RawData& simdat )
{
   for ( US_DataIO::Scan& scan : simdat.scanData )
   {
      double*   vals  = scan.rvalues.data();
      const int nvals = scan.rvalues.size();

      for ( int ii = 0; ii < nvals; ii++ )
      {
         vals[ii] = (double) (float) vals[ii];
      }
   }
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

// Approximate cache bytes of a simulation's scans (headers, any readings
//  they hold) and of nfloats float readings with their scan offsets
static qint64 cache_bytes( const US_DataIO::RawData& simdat, qint64 nfloats )
{
   const qint64 obytes = ( nfloats > 0 ) ? (qint64) sizeof( qsizetype ) : 0;
   qint64       bytes  = 256 + nfloats * (qint64) sizeof( float ); // Entry, key and hash overhead

   for ( const US_DataIO::Scan& scan : simdat.scanData )
   {
      bytes += (qint64) sizeof( US_DataIO::Scan ) + 32 + obytes + (qint64) scan.rvalues.size() * (qint64) sizeof( double ) +
               scan.interpolated.size();
   }

   return bytes;
}

// Approximate bytes a simulation takes in the cache
qint64 US_SimCache::sim_bytes( const US_DataIO::RawData& simdat, bool floats )
{
   if ( !floats )
   {
      return cache_bytes( simdat, 0 );
   }

   // As cache_bytes of the float form:  the scans without their readings
   //  and the readings as floats
   qint64 nvals = 0;

   for ( const US_DataIO::Scan& scan : simdat.scanData )
   {
      nvals += scan.rvalues.size();
   }

   return cache_bytes( simdat, nvals ) - nvals * (qint64) sizeof( double );
}

#ifdef Q_OS_LINUX
// Most float blocks mapped at once:  a quarter of the kernel's limit of
//  mappings of a process (vm.max_map_count, 65530 by default), so the
//  blocks leave room for the libraries, threads and malloc's own mappings
//  and a full cache of any size never reaches the limit
static qint64 max_float_maps()
{
   static const qint64 cap = []()
   {
      long long limit = 65530;
      FILE*     file  = fopen( "/proc/sys/vm/max_map_count", "r" );

      if ( file != nullptr )
      {
         long long value = 0;

         if ( fscanf( file, "%lld", &value ) == 1 && value > 0 )
         {
            limit = value;
         }

         fclose( file );
      }

      return (qint64) ( limit / 4 );
   }();

   return cap;
}

// Float blocks mapped now
static std::atomic<qint64> float_maps{ 0 };
#endif

// A block for n float readings.  On Linux a large block is mapped from the
//  system, and unmapped when the last copy goes:  glibc's malloc raises its
//  mmap threshold as large blocks are freed, so it would put later blocks in
//  its arenas, where freed memory stays resident (measured:  ~1 GB more peak
//  memory in a us_2dsa fit with float simulations of 500 scans).  Beyond
//  max_float_maps() blocks are taken from malloc.
static std::shared_ptr<float> alloc_floats( qsizetype count )
{
#ifdef Q_OS_LINUX
   const size_t bytes = static_cast<size_t>( count ) * sizeof( float );

   if ( bytes >= 128 * 1024 && float_maps.fetch_add( 1 ) < max_float_maps() )
   {
      void* block = mmap( nullptr, bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0 );

      if ( block != MAP_FAILED )
      {
         // Should munmap fail (it can need a new mapping, to split one the
         //  kernel merged with a neighbour), the pages are still freed and
         //  only the address range stays, counted as mapped
         auto unmap = [bytes]( float* ptr )
         {
            if ( munmap( ptr, bytes ) == 0 )
            {
               float_maps.fetch_sub( 1 );
            }
            else
            {
               madvise( ptr, bytes, MADV_DONTNEED );
            }
         };

         return std::shared_ptr<float>( static_cast<float*>( block ), unmap );
      }
   }

   if ( bytes >= 128 * 1024 )
   {
      float_maps.fetch_sub( 1 ); // Not mapped
   }
#endif

   return std::shared_ptr<float>( new float[qMax( count, (qsizetype) 1 )], std::default_delete<float[]>() );
}

// A simulation in float form (readings rounded) or double form (shared)
US_SimCache::Sim US_SimCache::Sim::from_raw( const US_DataIO::RawData& simdat, bool floats )
{
   Sim sim;
   sim.simdat = simdat; // Shared
   sim.floats = floats;

   if ( floats )
   { // Readings of all scans as floats, the scans without them
      qsizetype nvals = 0;

      for ( const US_DataIO::Scan& scan : simdat.scanData )
      {
         nvals += scan.rvalues.size();
      }

      std::shared_ptr<float> block = alloc_floats( nvals );
      sim.fvalues                  = block;
      sim.nfloats                  = nvals;
      sim.offsets.reserve( simdat.scanData.size() );
      float*    fv   = block.get();
      qsizetype offs = 0;

      for ( US_DataIO::Scan& scan : sim.simdat.scanData )
      {
         const double* dv = scan.rvalues.constData();
         const int     nv = scan.rvalues.size();

         for ( int ii = 0; ii < nv; ii++ )
         {
            fv[offs + ii] = (float) dv[ii];
         }

         sim.offsets << offs;
         offs += nv;
         scan.rvalues = QVector<double>();
      }
   }

   return sim;
}

// The simulation (in float form, its readings from the floats)
void US_SimCache::Sim::to_raw( US_DataIO::RawData& raw ) const
{
   raw = simdat; // Shared

   if ( !floats )
   {
      return;
   }

   for ( int ss = 0; ss < raw.scanData.size(); ss++ )
   {
      const qsizetype  offs  = offsets.at( ss );
      const qsizetype  nvals = ( ( ss + 1 < offsets.size() ) ? offsets.at( ss + 1 ) : nfloats ) - offs;
      const float*     fv    = fvalues.get() + offs;
      QVector<double>& rvals = raw.scanData[ss].rvalues;
      rvals.resize( nvals );
      double* dv = rvals.data();

      for ( qsizetype ii = 0; ii < nvals; ii++ )
      {
         dv[ii] = (double) fv[ii];
      }
   }
}

// This simulation in float form
US_SimCache::Sim US_SimCache::Sim::to_float() const
{
   return floats ? *this : from_raw( simdat, true );
}

// Approximate bytes the simulation takes in the cache
qint64 US_SimCache::Sim::bytes() const
{
   return cache_bytes( simdat, nfloats );
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

// Mark of a serialized float-form simulation (a pack() of a double-form one
//  starts with the data type, two letters, and the raw GUID)
static const char FLOAT_SIM_MARK[8] = { 'U', 'S', 'S', 'I', 'M', 'F', '0', '1' };

// Serialize a simulation:  the float form as its mark, the pack() of the
//  scans without readings, the scans' reading offsets and the floats;  the
//  double form as pack()
QByteArray        US_SimCache::Sim::pack_bytes() const
{
   if ( !floats )
   {
      return US_SimCache::pack( simdat );
   }

   const QByteArray scans = US_SimCache::pack( simdat );
   QByteArray       data;
   data.reserve( sizeof( FLOAT_SIM_MARK ) + scans.size() + ( offsets.size() + 3 ) * 8 + nfloats * (qsizetype) sizeof( float ) );
   data.append( FLOAT_SIM_MARK, sizeof( FLOAT_SIM_MARK ) );
   add_bytes( data, (qint64) scans.size() );
   data.append( scans );
   add_bytes( data, (qint64) offsets.size() );

   for ( const qsizetype offs : offsets )
   {
      add_bytes( data, (qint64) offs );
   }

   add_bytes( data, (qint64) nfloats );
   data.append( reinterpret_cast<const char*>( fvalues.get() ), nfloats * (qsizetype) sizeof( float ) );
   return data;
}

// Restore a simulation serialized by pack_bytes() or pack()
bool US_SimCache::Sim::unpack( const char* data, qint64 size, Sim& sim )
{
   sim = Sim();

   if ( size < (qint64) sizeof( FLOAT_SIM_MARK ) || memcmp( data, FLOAT_SIM_MARK, sizeof( FLOAT_SIM_MARK ) ) != 0 )
   { // Double form
      return US_SimCache::unpack( data, size, sim.simdat );
   }

   Unpacker in( data + sizeof( FLOAT_SIM_MARK ), size - (qint64) sizeof( FLOAT_SIM_MARK ) );
   qint64   nscan_bytes = 0;
   in.get( nscan_bytes );

   if ( !in.ok || nscan_bytes < 0 || in.end - in.ptr < nscan_bytes || !US_SimCache::unpack( in.ptr, nscan_bytes, sim.simdat ) )
   {
      return false;
   }

   in.ptr += nscan_bytes;
   qint64 noffsets = 0;
   in.get( noffsets );

   if ( !in.ok || noffsets != sim.simdat.scanData.size() || ( in.end - in.ptr ) / 8 < noffsets )
   {
      return false;
   }

   sim.offsets.resize( noffsets );

   for ( qint64 ss = 0; ss < noffsets; ss++ )
   {
      qint64 offs = 0;
      in.get( offs );
      sim.offsets[ss] = offs;
   }

   qint64 nvals = 0;
   in.get( nvals );

   if ( !in.ok || nvals < 0 || ( in.end - in.ptr ) / (qint64) sizeof( float ) != nvals ||
        ( in.end - in.ptr ) % (qint64) sizeof( float ) != 0 )
   {
      return false;
   }

   for ( qint64 ss = 0; ss < noffsets; ss++ )
   { // Offsets ascending within the readings
      const qint64 next = ( ss + 1 < noffsets ) ? sim.offsets[ss + 1] : nvals;

      if ( sim.offsets[ss] < 0 || sim.offsets[ss] > next )
      {
         return false;
      }
   }

   std::shared_ptr<float> block = alloc_floats( nvals );
   memcpy( block.get(), in.ptr, nvals * sizeof( float ) );
   sim.fvalues = block;
   sim.nfloats = nvals;
   sim.floats  = true;
   return true;
}

// Get a simulation
bool US_SimCache::fetch( const QByteArray& key, US_DataIO::RawData& simdat )
{
   Sim sim;

   if ( !fetch_stored( key, sim ) )
   {
      return false;
   }

   sim.to_raw( simdat ); // Shared;  in float mode the readings converted
   return true;
}

// Store a simulation, or mark a stored one as a survivor
bool US_SimCache::store( const QByteArray& key, const US_DataIO::RawData& simdat, bool survivor )
{
   return store_stored( key, Sim::from_raw( simdat, false ), survivor );
}

// Get a simulation as the cache keeps it:  in float mode from this class's
//  storage (shared, in float form);  otherwise through fetch() (which a
//  subclass may override), in double form
bool US_SimCache::fetch_sim( const QByteArray& key, Sim& sim )
{
   if ( floats )
   {
      return fetch_stored( key, sim );
   }

   US_DataIO::RawData simdat;

   if ( !fetch( key, simdat ) )
   {
      return false;
   }

   sim = Sim::from_raw( simdat, false ); // Shared
   return true;
}

// Store a simulation as the cache keeps it:  in float mode in this class's
//  storage (in float form, shared if it is);  otherwise through store()
//  (which a subclass may override)
bool US_SimCache::store_sim( const QByteArray& key, const Sim& sim, bool survivor )
{
   if ( floats )
   {
      return store_stored( key, sim, survivor );
   }

   US_DataIO::RawData simdat;
   sim.to_raw( simdat );
   return store( key, simdat, survivor );
}

// Get a simulation from this class's storage:  a shared copy, made under
//  the lock (converting it is left to the caller, outside the lock)
bool US_SimCache::fetch_stored( const QByteArray& key, Sim& sim )
{
   if ( lru )
   { // Least-recently-used mode:  note the use
      QWriteLocker wlock( &lock );
      auto         it = sims.find( key );

      if ( it != sims.end() )
      {
         sim                 = it.value().sim;
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
      {
         sim = it.value().sim;
         nhits++;
         return true;
      }
   }

   nmisses++;
   return false;
}

// Store a simulation in this class's storage, or mark a stored one as a
//  survivor.  In float mode it is kept in float form:  converted (rounded)
//  only when it is new and in double form.
bool US_SimCache::store_stored( const QByteArray& key, const Sim& sim, bool survivor )
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

      Entry entry;
      entry.sim    = floats ? sim.to_float() : sim;
      qint64 bytes = entry.sim.bytes();

      if ( bytes > max_bytes )
      {
         nrejected++;
         return false;
      }

      if ( nbytes + bytes > max_bytes )
      {
         evict_lru( bytes );
      }

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

   Entry entry;
   entry.sim    = floats ? sim.to_float() : sim;
   qint64 bytes = entry.sim.bytes();

   if ( survivor )
   {
      make_room( bytes );
   }

   if ( nbytes + bytes > max_bytes )
   { // No room
      nrejected++;
      return false;
   }

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
   st.floats    = floats;
   return st;
}

// One-line text of the counts
QString US_SimCache::stats_text() const
{
   Stats  st     = stats();
   qint64 nfetch = st.hits + st.misses;
   double hitpc  = ( nfetch > 0 ) ? ( 100.0 * st.hits / (double) nfetch ) : 0.0;

   return QObject::tr( "Simulation cache:  %1 of %2 simulations reused (%3%);"
                       " %4 stored (%5 survivors) in %6 of %7 MB%10; %8 evicted, %9 rejected" )
      .arg( st.hits )
      .arg( nfetch )
      .arg( hitpc, 0, 'f', 1 )
      .arg( st.entries )
      .arg( st.survivors )
      .arg( (double) st.bytes / (double) MB, 0, 'f', 1 )
      .arg( st.max_bytes / MB )
      .arg( st.evicted )
      .arg( st.rejected )
      .arg( st.floats ? QObject::tr( " (floats)" ) : QString() );
}
