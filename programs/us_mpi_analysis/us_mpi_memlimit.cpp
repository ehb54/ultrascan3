//! \file us_mpi_memlimit.cpp
#include "us_mpi_memlimit.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef Q_OS_UNIX
#include <sys/statvfs.h>
#endif

#ifdef Q_OS_LINUX
#include <linux/magic.h>
#include <sys/vfs.h>
#endif

// Shares of a limit at which to stop adding to the cache and to empty it.
//  They leave room for what the ranks allocate between two looks at the use.
static const double SOFT_SHARE = 0.75;
static const double HARD_SHARE = 0.88;

// Shares of the node's memory available at which to do the same
static const double NODE_SOFT = 0.05;
static const double NODE_HARD = 0.025;

// Limits at or above this mean "none" (cgroup v1 reports a huge number)
static const qint64 NO_LIMIT = (qint64) 1 << 60;
static const qint64 MB       = 1024 * 1024;

// Read a whole small file (empty if it cannot be read, as on a system without
//  these files, where then no limit or pressure is found)
static std::string  read_text( const std::string& path )
{
   std::string text;
   FILE*       file = fopen( path.c_str(), "r" );

   if ( file == nullptr )
   {
      return text;
   }

   char   buffer[4096];
   size_t count;

   while ( ( count = fread( buffer, 1, sizeof( buffer ), file ) ) > 0 )
   {
      text.append( buffer, count );
   }

   fclose( file );
   return text;
}

// Number in a file:  -1 if it cannot be read or does not start with a digit
//  (e.g. "max" in cgroup v2)
static qint64 read_number( const std::string& path )
{
   std::string text = read_text( path );

   if ( text.empty() || text[0] < '0' || text[0] > '9' )
   {
      return -1;
   }

   return strtoll( text.c_str(), nullptr, 10 );
}

// Value after a key at the start of a line:  "key value" or "key: value"
static qint64 keyed_value( const std::string& text, const std::string& key )
{
   size_t pos = 0;

   while ( pos < text.size() )
   {
      if ( text.compare( pos, key.size(), key ) == 0 )
      {
         const char* after = text.c_str() + pos + key.size();

         if ( *after == ' ' || *after == ':' || *after == '\t' )
         {
            while ( *after == ' ' || *after == ':' || *after == '\t' )
            {
               after++;
            }

            return strtoll( after, nullptr, 10 );
         }
      }

      pos = text.find( '\n', pos );
      pos = ( pos == std::string::npos ) ? text.size() : pos + 1;
   }

   return -1;
}

// Positive number from the environment (0 if not set or not valid)
static qint64 env_number( const char* name )
{
   const char* text = getenv( name );

   if ( text == nullptr )
   {
      return 0;
   }

   char*  end    = nullptr;
   qint64 number = strtoll( text, &end, 10 );
   return ( end != text && number > 0 ) ? number : 0;
}

// Limit from the Slurm environment.  A value of 0 ("--mem=0") means all the
//  memory of the node, so no limit here.
qint64 US_MPI_MemLimit::slurm_limit( int node_ranks )
{
   const qint64 per_node = env_number( "SLURM_MEM_PER_NODE" );

   if ( per_node > 0 )
   {
      return per_node * MB;
   }

   const qint64 per_cpu = env_number( "SLURM_MEM_PER_CPU" );
   qint64       cpus    = env_number( "SLURM_CPUS_ON_NODE" );
   cpus                 = ( cpus > 0 ) ? cpus : node_ranks;

   return ( per_cpu > 0 ) ? per_cpu * cpus * MB : 0;
}

// The tightest limit of this process's cgroup and its parents
US_MPI_MemLimit::Cgroup US_MPI_MemLimit::find_cgroup( const Paths& paths )
{
   Cgroup      cgroup;
   std::string text = read_text( paths.proc + "/self/cgroup" );
   std::string v1_path;
   std::string v2_path;
   bool        have_v1 = false;
   bool        have_v2 = false;
   size_t      pos     = 0;

   while ( pos < text.size() )
   { // Lines "id:controllers:path"
      size_t end       = text.find( '\n', pos );
      end              = ( end == std::string::npos ) ? text.size() : end;
      std::string line = text.substr( pos, end - pos );
      pos              = end + 1;
      size_t col1      = line.find( ':' );
      size_t col2      = ( col1 == std::string::npos ) ? col1 : line.find( ':', col1 + 1 );

      if ( col2 == std::string::npos )
      {
         continue;
      }

      std::string names = "," + line.substr( col1 + 1, col2 - col1 - 1 ) + ",";

      if ( names == ",," )
      {
         have_v2 = true;
         v2_path = line.substr( col2 + 1 );
      }
      else if ( names.find( ",memory," ) != std::string::npos )
      {
         have_v1 = true;
         v1_path = line.substr( col2 + 1 );
      }
   }

   if ( !have_v1 && !have_v2 )
   {
      return cgroup;
   }

   const std::string base       = have_v1 ? paths.cgroup + "/memory" : paths.cgroup;
   std::string       path       = have_v1 ? v1_path : v2_path;
   const std::string limit_name = have_v1 ? "memory.limit_in_bytes" : "memory.max";
   const std::string usage_name = have_v1 ? "memory.usage_in_bytes" : "memory.current";

   // From this process's level up to the root.  In a container the path of
   //  the host may not exist: such levels are passed over.
   while ( true )
   {
      while ( path.size() > 1 && path.back() == '/' )
      {
         path.pop_back();
      }

      const std::string dir   = base + ( path == "/" ? std::string() : path );
      const qint64      limit = read_number( dir + "/" + limit_name );

      if ( limit > 0 && limit < NO_LIMIT && ( cgroup.limit == 0 || limit <= cgroup.limit ) )
      {
         cgroup.limit      = limit;
         cgroup.usage_file = dir + "/" + usage_name;
         cgroup.stat_file  = dir + "/memory.stat";
         cgroup.inactive   = have_v1 ? "total_inactive_file" : "inactive_file";
      }

      if ( path == "/" || path.empty() )
      {
         break;
      }

      size_t slash = path.rfind( '/' );
      path         = ( slash == 0 || slash == std::string::npos ) ? std::string( "/" ) : path.substr( 0, slash );
   }

   return cgroup;
}

// The tightest limit of this process's cgroup and its parents
qint64 US_MPI_MemLimit::cgroup_limit( const Paths& paths )
{
   return find_cgroup( paths ).limit;
}

// The tightest of the Slurm and cgroup limits
qint64 US_MPI_MemLimit::job_limit( int node_ranks, const Paths& paths )
{
   const qint64 slurm  = slurm_limit( node_ranks );
   const qint64 cgroup = cgroup_limit( paths );

   if ( slurm > 0 && cgroup > 0 )
   {
      return qMin( slurm, cgroup );
   }

   return ( slurm > 0 ) ? slurm : cgroup;
}

// Bytes used at the cgroup level, without reclaimable file pages
qint64 US_MPI_MemLimit::cgroup_usage( const Cgroup& cgroup )
{
   if ( cgroup.limit <= 0 )
   {
      return -1;
   }

   const qint64 usage = read_number( cgroup.usage_file );

   if ( usage < 0 )
   {
      return -1;
   }

   const qint64 inactive = keyed_value( read_text( cgroup.stat_file ), cgroup.inactive );
   return qMax( (qint64) 0, usage - qMax( (qint64) 0, inactive ) );
}

// Bytes free in the shared memory file system
qint64 US_MPI_MemLimit::shm_free( const Paths& paths )
{
   return fs_free( paths.shm );
}

// Bytes free in the file system of a directory
qint64 US_MPI_MemLimit::fs_free( const std::string& dir )
{
#ifdef Q_OS_UNIX
   struct statvfs info;

   if ( dir.empty() || statvfs( dir.c_str(), &info ) != 0 || info.f_blocks == 0 )
   {
      return -1;
   }

   return (qint64) info.f_bavail * (qint64) info.f_frsize;
#else
   Q_UNUSED( dir );
   return -1;
#endif
}

// Whether a directory is on a file system in memory
int US_MPI_MemLimit::in_memory_fs( const std::string& dir )
{
#ifdef Q_OS_LINUX
   struct statfs info;

   if ( dir.empty() || statfs( dir.c_str(), &info ) != 0 )
   {
      return -1;
   }

   const unsigned long type = (unsigned long) info.f_type;
   return ( type == TMPFS_MAGIC || type == RAMFS_MAGIC || type == HUGETLBFS_MAGIC ) ? 1 : 0;
#else
   Q_UNUSED( dir );
   return -1;
#endif
}

// Directory of the file whose mapping holds an address.  Lines of the map:
//  "start-end perms offset dev inode [path]", the path maybe followed by
//  " (deleted)" (MPI implementations unlink the file once all ranks map it).
std::string US_MPI_MemLimit::mapped_dir( const void* address, const Paths& paths )
{
   const std::string text = read_text( paths.proc + "/self/maps" );
   const quintptr    addr = (quintptr) address;
   size_t            pos  = 0;

   while ( pos < text.size() )
   {
      size_t end       = text.find( '\n', pos );
      end              = ( end == std::string::npos ) ? text.size() : end;
      std::string line = text.substr( pos, end - pos );
      pos              = end + 1;

      char*          after = nullptr;
      const quintptr first = (quintptr) strtoull( line.c_str(), &after, 16 );
      if ( after == nullptr || *after != '-' )
      {
         continue;
      }
      const quintptr last = (quintptr) strtoull( after + 1, nullptr, 16 );

      if ( addr < first || addr >= last )
      {
         continue;
      }

      // The path is the first field starting with '/'
      const size_t slash = line.find( " /" );

      if ( slash == std::string::npos )
      {
         return std::string(); // Anonymous memory
      }

      std::string       path    = line.substr( slash + 1 );
      const std::string deleted = " (deleted)";

      if ( path.size() > deleted.size() && path.compare( path.size() - deleted.size(), deleted.size(), deleted ) == 0 )
      {
         path.resize( path.size() - deleted.size() );
      }

      // System V shared memory ("/SYSV<key>") and a shared anonymous mapping
      //  ("/dev/zero") are not files of a file system with a size
      if ( path.compare( 0, 5, "/SYSV" ) == 0 || path == "/dev/zero" )
      {
         return std::string();
      }

      const size_t last_slash = path.rfind( '/' );
      return ( last_slash == 0 ) ? std::string( "/" ) : path.substr( 0, last_slash );
   }

   return std::string();
}

// The node's total and available bytes
bool US_MPI_MemLimit::node_memory( qint64& total, qint64& available, const Paths& paths )
{
   const std::string text = read_text( paths.proc + "/meminfo" );
   total                  = keyed_value( text, "MemTotal" ) * 1024;
   available              = keyed_value( text, "MemAvailable" ) * 1024;
   return ( total > 0 && available >= 0 );
}

// Bytes of private memory of the given processes
qint64 US_MPI_MemLimit::private_bytes( const std::vector<long>& pids, const Paths& paths )
{
   qint64 bytes = 0;

   for ( long pid : pids )
   {
      const qint64 anon = keyed_value( read_text( paths.proc + "/" + std::to_string( pid ) + "/status" ), "RssAnon" );
      bytes += qMax( (qint64) 0, anon ) * 1024;
   }

   return bytes;
}

// Capacity that fits the free shared memory:  touching a page beyond it
//  would kill the process with a bus error
qint64 US_MPI_MemLimit::fit_shared_memory( qint64 wanted, qint64 shm_bytes )
{
   return ( shm_bytes < 0 ) ? wanted : qMin( wanted, shm_bytes / 10 * 9 );
}

// Cache capacity allowed for a wanted capacity
qint64 US_MPI_MemLimit::allowed_capacity( qint64 wanted, qint64 job_limit, qint64 shm_bytes )
{
   qint64 allowed = fit_shared_memory( wanted, shm_bytes );
   return ( job_limit > 0 ) ? qMin( allowed, job_limit / 2 ) : allowed;
}

// Predicted bytes one rank needs to solve a set of solutes:  the estimate of
//  the 2DSA fit control (US_AnalysisControl2D::grid_change), a fixed 17.2 MB
//  and 2.28 times the bytes of the solutes' simulations, four data vectors
//  and the noise terms
qint64 US_MPI_MemLimit::solve_bytes( qint64 readings, qint64 solutes, int noise_terms )
{
   const double fixed_mb = 17.20;
   const double factor   = 2.28;
   const qint64 columns  = qMax( (qint64) 0, solutes ) + 4 + qMax( 0, noise_terms );
   const double data     = (double) qMax( (qint64) 0, readings ) * (double) columns * sizeof( double );
   return (qint64) ( fixed_mb * MB + factor * data );
}

// Bytes the cache may hold while the job's own use peaks at job_peak
qint64 US_MPI_MemLimit::cache_room( qint64 job_limit, qint64 job_peak, double margin )
{
   if ( job_limit <= 0 )
   {
      return -1;
   }

   const double room = SOFT_SHARE * (double) job_limit - margin * (double) qMax( (qint64) 0, job_peak );
   return ( room > 0.0 ) ? (qint64) room : 0;
}

// Pressure for bytes used of a limit
US_MPI_MemLimit::Pressure US_MPI_MemLimit::limit_pressure( qint64 used, qint64 limit )
{
   if ( limit <= 0 || used < 0 )
   {
      return NONE;
   }

   if ( (double) used > HARD_SHARE * (double) limit )
   {
      return HARD;
   }

   return ( (double) used > SOFT_SHARE * (double) limit ) ? SOFT : NONE;
}

// Pressure for the node's available memory
US_MPI_MemLimit::Pressure US_MPI_MemLimit::node_pressure( qint64 total, qint64 available )
{
   if ( total <= 0 || available < 0 )
   {
      return NONE;
   }

   if ( (double) available < NODE_HARD * (double) total )
   {
      return HARD;
   }

   return ( (double) available < NODE_SOFT * (double) total ) ? SOFT : NONE;
}

// Pressure for the free bytes in shared memory
US_MPI_MemLimit::Pressure US_MPI_MemLimit::shm_pressure( qint64 shm_bytes, qint64 capacity )
{
   if ( shm_bytes < 0 )
   {
      return NONE;
   }

   const qint64 margin = qMax( 64 * MB, capacity / 20 );

   if ( shm_bytes < margin / 2 )
   {
      return HARD;
   }

   return ( shm_bytes < margin ) ? SOFT : NONE;
}

// Default cache capacity of one rank
qint64 US_MPI_MemLimit::default_cache_bytes( const qint64 job_limit,
                                             const qint64 node_available,
                                             const int    node_ranks,
                                             const double share,
                                             const qint64 max_bytes,
                                             const qint64 fallback_bytes )
{
   qint64 memory = ( job_limit > 0 ) ? job_limit : -1;

   if ( node_available >= 0 )
   {
      memory = ( memory > 0 ) ? qMin( memory, node_available ) : node_available;
   }

   if ( memory < 0 )
   {
      return qMin( fallback_bytes, max_bytes );
   }

   const qint64 rank_bytes = memory / qMax( 1, node_ranks );
   return qMin( max_bytes, static_cast<qint64>( share * static_cast<double>( rank_bytes ) ) );
}
