//! \file us_mpi_sim_cache.cpp
#include "us_mpi_sim_cache.h"

#include <atomic>
#include <chrono>
#include <cstring>
#include <new>
#include <thread>

#include <QCoreApplication>

#ifdef Q_OS_LINUX
#include <sys/mman.h>
#include <unistd.h>
#endif

// Shared state of a generation
class US_MPI_NodeSimCache::Generation
{
   public:
      std::atomic<qint64> data_used; // Bytes of the data area taken
      std::atomic<qint64> entries;   // Simulations stored
      std::atomic<int>    full;      // Flag:  no room left for more
      std::atomic<int>    epoch;     // Odd while the generation is emptied
      std::atomic<int>    users;     // Ranks reading or writing it
      int                 pad;       // (alignment)
};

// Shared header of the window
class US_MPI_NodeSimCache::Header
{
   public:
      std::atomic<int>    current;  // Generation simulations are stored in
      std::atomic<int>    rotating; // Flag:  a rank empties a generation
      std::atomic<int>    pressure; // Memory pressure:  no adding if nonzero
      std::atomic<qint64> releases; // Times emptied under pressure
      std::atomic<qint64> released; // Bytes returned to the system
      Generation          gen[2];
};

// Shared slot of the table
class US_MPI_NodeSimCache::Slot
{
   public:
      std::atomic<int> state;      // SlotState
      int              pad;        // (alignment)
      qint64           offset;     // Data offset of the serialized simulation
      qint64           size;       // Bytes of the serialized simulation
      unsigned char    digest[32]; // SHA-256 of the simulation key
};

// Ranks of different processes use the atomics in the shared window
static_assert( ATOMIC_INT_LOCK_FREE == 2, "lock-free int atomics are required" );
static_assert( ATOMIC_LLONG_LOCK_FREE == 2 && sizeof( qint64 ) == sizeof( long long ), "lock-free 64-bit atomics are required" );

// Results of store_in
enum StoreResult
{
   STORED,  // Stored, or stored already
   SKIPPED, // Known but not stored (no room when it came first)
   NO_ROOM  // Generation full
};

// Round up to a multiple of 8 bytes
static qint64 align8( qint64 bytes )
{
   return ( bytes + 7 ) & ~(qint64) 7;
}

// Create the cache (collective over node_comm)
US_MPI_NodeSimCache::US_MPI_NodeSimCache( MPI_Comm node_comm, qint64 max_bytes, qint64 sim_bytes, Policy policy ) :
    US_SimCache( 0 ),
    comm( node_comm ),
    window( MPI_WIN_NULL ),
    base( nullptr ),
    header( nullptr ),
    table( nullptr ),
    data( nullptr ),
    nslots( 0 ),
    data_size( 0 ),
    keep( policy ),
    ngens( ( policy == RECENT ) ? 2 : 1 ),
    gen_slots( 0 ),
    gen_size( 0 ),
    store_on( true ),
    watch_stop( false ),
    rank_hits( 0 ),
    rank_misses( 0 ),
    rank_stored( 0 ),
    rank_rejected( 0 ),
    rank_evicted( 0 )
{
   int node_rank = 0;
   MPI_Comm_rank( comm, &node_rank );

   // Table of a generation at most half full when its data area is
   gen_size            = align8( qMax( (qint64) 0, max_bytes ) / ngens );
   gen_slots           = 2 * ( gen_size / qMax( (qint64) 1024, sim_bytes ) ) + 1024;
   data_size           = gen_size * ngens;
   nslots              = gen_slots * ngens;
   qint64 header_bytes = align8( sizeof( Header ) );
   qint64 table_bytes  = align8( nslots * (qint64) sizeof( Slot ) );
   qint64 total_bytes  = header_bytes + table_bytes + data_size;

   // The node's first rank allocates all of the window; report a failure
   //  instead of aborting, so that every rank can go on without a cache
   MPI_Comm_set_errhandler( comm, MPI_ERRORS_RETURN );
   char* mybase = nullptr;
   int   rc = MPI_Win_allocate_shared( ( node_rank == 0 ) ? (MPI_Aint) total_bytes : 0, 1, MPI_INFO_NULL, comm, &mybase, &window );
   int   ok = ( rc == MPI_SUCCESS ) ? 1 : 0;
   int   all_ok = 0;
   MPI_Allreduce( &ok, &all_ok, 1, MPI_INT, MPI_MIN, comm );

   if ( all_ok == 0 )
   {
      if ( rc == MPI_SUCCESS )
      {
         MPI_Win_free( &window );
      }
      window = MPI_WIN_NULL;
      MPI_Comm_set_errhandler( comm, MPI_ERRORS_ARE_FATAL );
      return;
   }

   MPI_Comm_set_errhandler( comm, MPI_ERRORS_ARE_FATAL );
   MPI_Aint size = 0;
   int      disp = 1;
   char*    ptr  = nullptr;
   MPI_Win_shared_query( window, 0, &size, &disp, &ptr );

   // Passive access for the life of the cache;  ranks use the memory directly
   MPI_Win_lock_all( MPI_MODE_NOCHECK, window );

   if ( node_rank == 0 )
   { // Where the MPI implementation keeps the window (for the memory watch)
      backing_dir = US_MPI_MemLimit::mapped_dir( ptr );
   }

   header = reinterpret_cast<Header*>( ptr );
   table  = reinterpret_cast<Slot*>( ptr + header_bytes );
   data   = ptr + header_bytes + table_bytes;

   if ( node_rank == 0 )
   { // Empty header and table
      memset( ptr, 0, header_bytes + table_bytes );
      new ( header ) Header();
      header->current.store( 0 );
      header->rotating.store( 0 );
      header->pressure.store( 0 );
      header->releases.store( 0 );
      header->released.store( 0 );

      for ( int gg = 0; gg < 2; gg++ )
      {
         header->gen[gg].data_used.store( 0 );
         header->gen[gg].entries.store( 0 );
         header->gen[gg].full.store( 0 );
         header->gen[gg].epoch.store( 0 );
         header->gen[gg].users.store( 0 );
      }

      for ( qint64 ii = 0; ii < nslots; ii++ )
      {
         new ( &table[ii].state ) std::atomic<int>( EMPTY );
      }
   }

   MPI_Win_sync( window );
   MPI_Barrier( comm );
   MPI_Win_sync( window );
   base = ptr;

   // The memory watch of the node's first rank reads the use of all its ranks
   int  node_size = 1;
   long my_pid    = (long) QCoreApplication::applicationPid();
   MPI_Comm_size( comm, &node_size );
   node_pids.resize( node_size );
   MPI_Allgather( &my_pid, 1, MPI_LONG, node_pids.data(), 1, MPI_LONG, comm );
}

// Directory of the file system of a shared window (collective over
//  node_comm).  The window is a page from the node's first rank, mapped only
//  to read its place in the process's memory map.
std::string US_MPI_NodeSimCache::probe_backing( MPI_Comm node_comm )
{
   int node_rank = 0;
   MPI_Comm_rank( node_comm, &node_rank );
   MPI_Comm_set_errhandler( node_comm, MPI_ERRORS_RETURN );

   std::string dir;
   MPI_Win     probe  = MPI_WIN_NULL;
   char*       mybase = nullptr;
   const int   rc     = MPI_Win_allocate_shared( ( node_rank == 0 ) ? 4096 : 0, 1, MPI_INFO_NULL, node_comm, &mybase, &probe );
   int         ok     = ( rc == MPI_SUCCESS ) ? 1 : 0;
   int         all_ok = 0;
   MPI_Allreduce( &ok, &all_ok, 1, MPI_INT, MPI_MIN, node_comm );

   if ( all_ok != 0 && node_rank == 0 )
   {
      dir = US_MPI_MemLimit::mapped_dir( mybase );
   }

   if ( rc == MPI_SUCCESS )
   {
      MPI_Win_free( &probe );
   }

   MPI_Comm_set_errhandler( node_comm, MPI_ERRORS_ARE_FATAL );
   return dir;
}

// Free the shared window (collective over the node communicator)
US_MPI_NodeSimCache::~US_MPI_NodeSimCache()
{
   if ( watcher.joinable() )
   {
      {
         std::lock_guard<std::mutex> guard( watch_lock );
         watch_stop = true;
      }
      watch_wake.notify_all();
      watcher.join();
   }

   if ( window != MPI_WIN_NULL )
   {
      if ( base != nullptr )
      {
         MPI_Win_unlock_all( window );
      }
      MPI_Win_free( &window );
   }
}

// Slot of a key digest in generation gg:  the slot holding it, or else the
//  first empty slot of its probe sequence (null if the table is full)
US_MPI_NodeSimCache::Slot* US_MPI_NodeSimCache::find( int gg, const QByteArray& digest, bool& found )
{
   Slot*   gtable = table + gg * gen_slots;
   quint64 start  = 0;
   memcpy( &start, digest.constData(), sizeof( start ) );
   found = false;

   for ( qint64 nn = 0; nn < gen_slots; nn++ )
   {
      Slot* slot  = &gtable[( start + (quint64) nn ) % (quint64) gen_slots];
      int   state = slot->state.load( std::memory_order_acquire );

      while ( state == CLAIMED )
      { // Another rank is about to write the digest
         std::this_thread::yield();
         state = slot->state.load( std::memory_order_acquire );
      }

      if ( state == EMPTY )
      {
         return slot;
      }

      if ( memcmp( slot->digest, digest.constData(), sizeof( slot->digest ) ) == 0 )
      {
         found = true;
         return slot;
      }
   }

   return nullptr;
}

// Announce this rank's use of generation gg; fails while it is emptied.
//  With the sequentially consistent order of users and epoch, either this
//  rank sees the odd epoch, or the emptying rank sees this rank's use.
bool US_MPI_NodeSimCache::enter( int gg )
{
   Generation& gen = header->gen[gg];
   gen.users.fetch_add( 1, std::memory_order_seq_cst );

   if ( ( gen.epoch.load( std::memory_order_seq_cst ) & 1 ) != 0 )
   {
      gen.users.fetch_sub( 1, std::memory_order_seq_cst );
      return false;
   }

   return true;
}

// End this rank's use of generation gg
void US_MPI_NodeSimCache::leave( int gg )
{
   header->gen[gg].users.fetch_sub( 1, std::memory_order_seq_cst );
}

// Get a simulation from generation gg
bool US_MPI_NodeSimCache::fetch_in( int gg, const QByteArray& digest, US_DataIO::RawData& simdat )
{
   bool  found = false;
   Slot* slot  = find( gg, digest, found );

   return ( found && slot->state.load( std::memory_order_acquire ) == READY &&
            unpack( data + gg * gen_size + slot->offset, slot->size, simdat ) );
}

// Get a simulation
bool US_MPI_NodeSimCache::fetch( const QByteArray& key, US_DataIO::RawData& simdat )
{
   QByteArray digest = QCryptographicHash::hash( key, QCryptographicHash::Sha256 );
   int        cur    = header->current.load( std::memory_order_acquire );

   for ( int nn = 0; nn < ngens; nn++ )
   {
      int gg = ( cur + nn ) % ngens;

      if ( !enter( gg ) )
      {
         continue;
      }

      bool hit = fetch_in( gg, digest, simdat );
      leave( gg );

      if ( hit )
      {
         rank_hits.fetch_add( 1, std::memory_order_relaxed );

         if ( gg != cur && store_on.load( std::memory_order_relaxed ) )
         { // Keep a simulation of the previous generation in use (after
            //  leaving it:  the copy may empty it)
            store( key, simdat, true );
         }

         return true;
      }
   }

   rank_misses.fetch_add( 1, std::memory_order_relaxed );
   return false;
}

// Store a simulation in generation gg, unless it is stored (or being stored)
//  already
int US_MPI_NodeSimCache::store_in( int gg, const QByteArray& digest, const US_DataIO::RawData& simdat )
{
   Generation& gen = header->gen[gg];

   while ( true )
   {
      bool  found = false;
      Slot* slot  = find( gg, digest, found );

      if ( found )
      { // Stored, or being stored by another rank
         int state = slot->state.load( std::memory_order_acquire );
         return ( state == READY || state == FILLING ) ? STORED : SKIPPED;
      }

      if ( slot == nullptr || gen.full.load( std::memory_order_relaxed ) != 0 )
      { // No room
         gen.full.store( 1, std::memory_order_relaxed );
         return NO_ROOM;
      }

      int expected = EMPTY;

      if ( !slot->state.compare_exchange_strong( expected, CLAIMED, std::memory_order_acq_rel ) )
      {
         continue; // Another rank took the slot:  look again
      }

      memcpy( slot->digest, digest.constData(), sizeof( slot->digest ) );
      slot->state.store( FILLING, std::memory_order_release );

      QByteArray   packed = pack( simdat );
      const qint64 size   = packed.size();
      const qint64 room   = align8( size );
      qint64       offset = gen.data_used.load( std::memory_order_relaxed );

      // Reserve the bytes only if they fit, so that data_used never passes
      //  the end of the generation
      do
      {
         if ( offset + room > gen_size )
         { // Data area full:  keep the digest, so the key is not tried again
            gen.full.store( 1, std::memory_order_relaxed );
            slot->state.store( DEAD, std::memory_order_release );
            return NO_ROOM;
         }
      } while ( !gen.data_used.compare_exchange_weak( offset, offset + room, std::memory_order_relaxed ) );

      memcpy( data + gg * gen_size + offset, packed.constData(), size );
      slot->offset = offset;
      slot->size   = size;
      slot->state.store( READY, std::memory_order_release );
      gen.entries.fetch_add( 1, std::memory_order_relaxed );
      rank_stored.fetch_add( 1, std::memory_order_relaxed );
      return STORED;
   }
}

// Store a simulation.  US_SolveSim offers each simulation it computes first
//  as a non-survivor, and a surviving one again:  only the first offer counts
//  as rejected when there is no room.
bool US_MPI_NodeSimCache::store( const QByteArray& key, const US_DataIO::RawData& simdat, bool survivor )
{
   if ( !store_on.load( std::memory_order_relaxed ) )
   {
      return false;
   }

   if ( header->pressure.load( std::memory_order_relaxed ) != 0 )
   { // Memory is short.  RECENT gives up its older simulations for new ones;
      //  otherwise this one is not stored.
      if ( keep != RECENT || !rotate( header->current.load( std::memory_order_acquire ), true ) )
      {
         if ( !survivor )
         {
            rank_rejected.fetch_add( 1, std::memory_order_relaxed );
         }

         return false;
      }
   }

   QByteArray digest = QCryptographicHash::hash( key, QCryptographicHash::Sha256 );

   for ( int tries = 0; tries < 2; tries++ )
   {
      int gg = header->current.load( std::memory_order_acquire );

      if ( !enter( gg ) )
      {
         break;
      }

      int result = store_in( gg, digest, simdat );
      leave( gg );

      if ( result != NO_ROOM )
      {
         return ( result == STORED );
      }

      if ( keep != RECENT )
      {
         break;
      }

      rotate( gg ); // Make room, then try once more
   }

   if ( !survivor )
   {
      rank_rejected.fetch_add( 1, std::memory_order_relaxed );
   }

   return false;
}

// Empty the previous generation and make it current, if generation gg is
//  still current and no other rank does so already.  Waits only for ranks
//  within a single fetch or store of the previous generation.  With give_back
//  this frees memory:  a previous generation without simulations has none to
//  free, and the pages of an emptied one go back to the system.  The pressure
//  is then reset, for the watch to judge the new use before any more is given up.
bool US_MPI_NodeSimCache::rotate( int gg, bool give_back )
{
   int  idle    = 0;
   bool rotated = false;

   if ( !header->rotating.compare_exchange_strong( idle, 1, std::memory_order_acq_rel ) )
   {
      return false; // Another rank rotates
   }

   if ( header->current.load( std::memory_order_acquire ) == gg )
   {
      int         og  = 1 - gg;
      Generation& old = header->gen[og];

      if ( !give_back || old.entries.load( std::memory_order_relaxed ) > 0 )
      {
         old.epoch.fetch_add( 1, std::memory_order_seq_cst ); // Odd:  keep out

         while ( old.users.load( std::memory_order_seq_cst ) != 0 )
         {
            std::this_thread::yield();
         }

         rank_evicted.fetch_add( old.entries.load( std::memory_order_relaxed ), std::memory_order_relaxed );
         empty( og );

         if ( give_back )
         {
            release_pages( og );
            header->pressure.store( 0, std::memory_order_relaxed );
         }

         old.epoch.fetch_add( 1, std::memory_order_seq_cst ); // Even:  usable
         header->current.store( og, std::memory_order_release );
         rotated = true;
      }
   }

   header->rotating.store( 0, std::memory_order_release );
   return rotated;
}

// Empty generation gg (no rank may use it)
void US_MPI_NodeSimCache::empty( int gg )
{
   Slot* gtable = table + gg * gen_slots;

   for ( qint64 ii = 0; ii < gen_slots; ii++ )
   {
      gtable[ii].state.store( EMPTY, std::memory_order_relaxed );
   }

   Generation& gen = header->gen[gg];
   gen.data_used.store( 0, std::memory_order_relaxed );
   gen.entries.store( 0, std::memory_order_relaxed );
   gen.full.store( 0, std::memory_order_relaxed );
   std::atomic_thread_fence( std::memory_order_seq_cst );
}

// Return the pages of generation gg's data to the system.  Resetting the
//  counts alone leaves them in memory, charged to the job.  Should the system
//  not support this for the window, the pages just stay until used again.
bool US_MPI_NodeSimCache::release_pages( int gg )
{
#ifdef Q_OS_LINUX
   static const quintptr page  = (quintptr) sysconf( _SC_PAGESIZE );
   const quintptr        start = (quintptr) ( data + gg * gen_size );
   const quintptr        first = ( start + page - 1 ) & ~( page - 1 );
   const quintptr        last  = ( start + gen_size ) & ~( page - 1 );

   // MADV_REMOVE frees the pages of tmpfs (MPICH, Open MPI in /dev/shm),
   //  of System V shared memory and of a file on a disk file system that
   //  can punch holes (Open MPI's session directory on ext4, xfs, btrfs)
   return ( last <= first || madvise( reinterpret_cast<void*>( first ), last - first, MADV_REMOVE ) == 0 );
#else
   // (MS-MPI:  a paging file section has no way to drop its pages here)
   Q_UNUSED( gg );
   return false;
#endif
}

// Wait for the sole right to empty generations
void US_MPI_NodeSimCache::lock_maintenance()
{
   int idle = 0;

   while ( !header->rotating.compare_exchange_weak( idle, 1, std::memory_order_acq_rel ) )
   {
      idle = 0;
      std::this_thread::yield();
   }
}

// Remove all simulations.  Only while no other rank of the node uses the
//  cache (the counts are kept); the memory watch may be at work, though.
void US_MPI_NodeSimCache::clear()
{
   lock_maintenance();

   for ( int gg = 0; gg < ngens; gg++ )
   {
      empty( gg );
      release_pages( gg );
   }

   header->current.store( 0, std::memory_order_relaxed );
   header->rotating.store( 0, std::memory_order_release );
   std::atomic_thread_fence( std::memory_order_seq_cst );
   MPI_Win_sync( window );

   forget_dataset_keys();
}

// Empty every generation, while other ranks may use the cache:  they keep out
//  of a generation being emptied, and take that as a miss or as no room.
void US_MPI_NodeSimCache::empty_under_pressure( qint64 used )
{
   lock_maintenance();
   qint64 freed = 0;

   for ( int gg = 0; gg < ngens; gg++ )
   {
      Generation& gen = header->gen[gg];
      gen.epoch.fetch_add( 1, std::memory_order_seq_cst ); // Odd:  keep out

      while ( gen.users.load( std::memory_order_seq_cst ) != 0 )
      {
         std::this_thread::yield();
      }

      const qint64 held = gen.data_used.load( std::memory_order_relaxed );
      empty( gg );

      if ( release_pages( gg ) )
      {
         freed += held;
      }
      gen.epoch.fetch_add( 1, std::memory_order_seq_cst ); // Even:  usable
   }

   header->rotating.store( 0, std::memory_order_release );

   const qint64 count = header->releases.fetch_add( 1 ) + 1;
   header->released.fetch_add( freed );

   if ( count <= 5 && !watch.stress )
   {
      qDebug().noquote() << QString( "Simulation cache: emptied for lack of memory (use %1 MB, limit %2 MB, %3 MB freed)" )
                               .arg( used / ( 1024 * 1024 ) )
                               .arg( watch.job_limit / ( 1024 * 1024 ) )
                               .arg( freed / ( 1024 * 1024 ) );
   }
}

// Bytes the cache holds in memory now
qint64 US_MPI_NodeSimCache::bytes_held() const
{
   qint64 bytes = 0;

   for ( int gg = 0; gg < ngens; gg++ )
   {
      bytes += header->gen[gg].data_used.load( std::memory_order_relaxed );
   }

   return bytes;
}

// Look at the memory use until told to stop.  The job's cgroup is read often,
//  since it is what kills the job, the other sources less often.
void US_MPI_NodeSimCache::watch_loop()
{
   const std::chrono::milliseconds interval( 50 );
   std::unique_lock<std::mutex>    guard( watch_lock );
   int                             tick       = 0;
   qint64                          scanned    = -1;
   US_MPI_MemLimit::Pressure       slow_press = US_MPI_MemLimit::NONE;

   while ( !watch_wake.wait_for( guard, interval, [this] { return watch_stop; } ) )
   {
      guard.unlock();

      if ( tick++ % 4 == 0 )
      {
         scanned = US_MPI_MemLimit::private_bytes( node_pids ) + bytes_held();

         qint64 total = 0;
         qint64 avail = 0;
         slow_press =
            US_MPI_MemLimit::node_memory( total, avail ) ? US_MPI_MemLimit::node_pressure( total, avail ) : US_MPI_MemLimit::NONE;
         const qint64 shm_bytes = watch.shm_dir.empty() ? US_MPI_MemLimit::shm_free() : US_MPI_MemLimit::fs_free( watch.shm_dir );
         slow_press             = qMax( slow_press, US_MPI_MemLimit::shm_pressure( shm_bytes, data_size ) );
      }

      const qint64              used  = qMax( scanned, US_MPI_MemLimit::cgroup_usage( watch.cgroup ) );
      US_MPI_MemLimit::Pressure press = qMax( slow_press, US_MPI_MemLimit::limit_pressure( used, watch.job_limit ) );

      if ( watch.stress )
      { // Test:  let the ranks add for a while, then stop them, then empty
         press = ( tick % 4 == 0 ) ? US_MPI_MemLimit::HARD : ( tick % 4 == 3 ) ? US_MPI_MemLimit::SOFT : US_MPI_MemLimit::NONE;
      }

      header->pressure.store( ( press == US_MPI_MemLimit::NONE ) ? 0 : 1, std::memory_order_relaxed );

      if ( press == US_MPI_MemLimit::HARD && bytes_held() > 0 )
      {
         empty_under_pressure( used );
         scanned = -1;

         if ( !watch.stress )
         { // Look at everything again soon
            tick = 0;
         }
      }

      guard.lock();
   }
}

// Watch the memory use from a thread (on the node's first rank)
void US_MPI_NodeSimCache::start_watch( const Watch& settings )
{
   watch   = settings;
   watcher = std::thread( &US_MPI_NodeSimCache::watch_loop, this );
}

// Number of times the cache was emptied for lack of memory
qint64 US_MPI_NodeSimCache::releases() const
{
   return header->releases.load( std::memory_order_relaxed );
}

// Bytes returned to the system by emptying under pressure
qint64 US_MPI_NodeSimCache::released_bytes() const
{
   return header->released.load( std::memory_order_relaxed );
}

// Counts of this rank, with the node's entries and bytes
US_SimCache::Stats US_MPI_NodeSimCache::stats() const
{
   Stats st;
   st.hits      = rank_hits.load( std::memory_order_relaxed );
   st.misses    = rank_misses.load( std::memory_order_relaxed );
   st.stored    = rank_stored.load( std::memory_order_relaxed );
   st.evicted   = rank_evicted.load( std::memory_order_relaxed );
   st.rejected  = rank_rejected.load( std::memory_order_relaxed );
   st.entries   = 0;
   st.survivors = 0;
   st.bytes     = 0;
   st.max_bytes = data_size;

   for ( int gg = 0; gg < ngens; gg++ )
   {
      st.entries += header->gen[gg].entries.load( std::memory_order_relaxed );
      st.bytes += header->gen[gg].data_used.load( std::memory_order_relaxed );
   }

   return st;
}

// Counts of the whole node (collective over the node communicator)
US_SimCache::Stats US_MPI_NodeSimCache::node_stats() const
{
   Stats  st        = stats();
   qint64 mine[5]   = { st.hits, st.misses, st.stored, st.rejected, st.evicted };
   qint64 summed[5] = { 0, 0, 0, 0, 0 };
   MPI_Allreduce( mine, summed, 5, MPI_LONG_LONG, MPI_SUM, comm );
   st.hits     = summed[0];
   st.misses   = summed[1];
   st.stored   = summed[2];
   st.rejected = summed[3];
   st.evicted  = summed[4];
   return st;
}
