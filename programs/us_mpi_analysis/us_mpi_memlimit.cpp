//! \file us_mpi_memlimit.cpp
#include "us_mpi_memlimit.h"

#include <cstdio>
#include <cstdlib>

// Limits at or above this mean "none" (cgroup v1 reports a huge number)
static const qint64 NO_LIMIT = (qint64) 1 << 60;
static const qint64 MB       = 1024 * 1024;

// Read a whole small file (empty if it cannot be read, as on a system without
//  these files, where then no limit is found)
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
   const std::string text = read_text( path );

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
   const qint64 number = strtoll( text, &end, 10 );
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
qint64 US_MPI_MemLimit::cgroup_limit( const Paths& paths )
{
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
      return 0;
   }

   const std::string base       = have_v1 ? paths.cgroup + "/memory" : paths.cgroup;
   std::string       path       = have_v1 ? v1_path : v2_path;
   const std::string limit_name = have_v1 ? "memory.limit_in_bytes" : "memory.max";
   qint64            tightest   = 0;

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

      if ( limit > 0 && limit < NO_LIMIT && ( tightest == 0 || limit < tightest ) )
      {
         tightest = limit;
      }

      if ( path == "/" || path.empty() )
      {
         break;
      }

      size_t slash = path.rfind( '/' );
      path         = ( slash == 0 || slash == std::string::npos ) ? std::string( "/" ) : path.substr( 0, slash );
   }

   return tightest;
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

// The node's total and available bytes
bool US_MPI_MemLimit::node_memory( qint64& total, qint64& available, const Paths& paths )
{
   const std::string text = read_text( paths.proc + "/meminfo" );
   total                  = keyed_value( text, "MemTotal" ) * 1024;
   available              = keyed_value( text, "MemAvailable" ) * 1024;
   return ( total > 0 && available >= 0 );
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
