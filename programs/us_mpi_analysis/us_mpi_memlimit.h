//! \file us_mpi_memlimit.h
#ifndef US_MPI_MEMLIMIT_H
#define US_MPI_MEMLIMIT_H

#include <QtGlobal>

#include <string>
#include <vector>

//! \brief The memory a job may use on this node, and what it uses now
//!
//! A job's limit is not the node's memory:  the scheduler gives it a share
//! (Slurm's --mem and --mem-per-cpu) and may enforce that with a cgroup.  This
//! class finds the limit and reads the current use.  It makes no MPI or Qt
//! object calls, so a thread may use it next to the rank's MPI calls.  The
//! paths can be replaced for tests.
class US_MPI_MemLimit
{
   public:
      //! \brief Where the system files are
      class Paths
      {
         public:
            Paths() : proc( "/proc" ), cgroup( "/sys/fs/cgroup" ), shm( "/dev/shm" )
            {
            }

            std::string proc;   //!< proc file system
            std::string cgroup; //!< cgroup file system
            std::string shm;    //!< shared memory file system
      };

      //! \brief The cgroup level that limits this process
      class Cgroup
      {
         public:
            qint64      limit = 0;  //!< Limit in bytes (0: no limit found)
            std::string usage_file; //!< File with the bytes charged to the level
            std::string stat_file;  //!< File with the detailed counts of the level
            std::string inactive;   //!< Key in stat_file of reclaimable file pages
      };

      //! \brief Limit from the Slurm environment, in bytes (0: none):
      //!        SLURM_MEM_PER_NODE, or SLURM_MEM_PER_CPU times the CPUs of
      //!        this node (SLURM_CPUS_ON_NODE, else node_ranks)
      static qint64 slurm_limit( int node_ranks );

      //! \brief The tightest limit of this process's cgroup and its parents.
      //!        Of levels with the same limit, the highest is chosen:  all
      //!        ranks of the job are charged to it, not just this rank's task.
      static Cgroup      find_cgroup( const Paths& paths = Paths() );

      //! \brief The tightest memory limit of this process's cgroup and its
      //!        parents, in bytes (0: none found).  cgroup v1 and v2.
      static qint64 cgroup_limit( const Paths& paths = Paths() );

      //! \brief The tightest of the Slurm and cgroup limits (0: none)
      static qint64 job_limit( int node_ranks, const Paths& paths = Paths() );

      //! \brief Bytes used at the cgroup level, without reclaimable file
      //!        pages (-1 if unknown)
      static qint64      cgroup_usage( const Cgroup& cgroup );

      //! \brief Bytes free in the shared memory file system (-1 if unknown)
      static qint64      shm_free( const Paths& paths = Paths() );

      //! \brief Directory of the file whose mapping holds an address, from
      //!        the process's memory map:  where an MPI implementation keeps
      //!        a shared window (MPICH:  /dev/shm;  Open MPI:  its session
      //!        directory, maybe on a disk).  Empty for memory that is not a
      //!        file (anonymous or System V shared memory) or if unknown.
      static std::string mapped_dir( const void* address, const Paths& paths = Paths() );

      //! \brief Resident bytes of this process in shared mappings, from its
      //!        statm (-1 if unknown, e.g. not on Linux).  Cheap enough to
      //!        call for every message.
      static qint64      shared_rss( const Paths& paths = Paths() );

      //! \brief Bytes free in the file system of a directory (-1 if unknown)
      static qint64      fs_free( const std::string& dir );

      //! \brief Whether a directory is on a file system in memory (tmpfs and
      //!        the like):  1 yes, 0 no (a disk), -1 unknown
      static int         in_memory_fs( const std::string& dir );

      //! \brief Read the node's total and available bytes
      static bool   node_memory( qint64& total, qint64& available, const Paths& paths = Paths() );

      //! \brief Bytes of private memory of the given processes
      static qint64      private_bytes( const std::vector<long>& pids, const Paths& paths = Paths() );

      //! \brief Default cache capacity of one rank:  a share of the memory
      //!        the rank may use, at most max_bytes
      //!
      //! The rank's memory is its part (1 / node_ranks) of the job's limit
      //! on this node or of the node's available memory, whichever is less;
      //! the cache gets the given share of it.  If neither is known,
      //! fallback_bytes.
      //! \param job_limit       The job's limit on this node (0: none)
      //! \param node_available  The node's available bytes (negative: unknown)
      //! \param node_ranks      Ranks of the job on this node
      //! \param share           Share of the rank's memory for the cache
      //! \param max_bytes       Largest capacity
      //! \param fallback_bytes  Capacity if no memory size is known
      static qint64 default_cache_bytes( qint64 job_limit,
                                         qint64 node_available,
                                         int    node_ranks,
                                         double share,
                                         qint64 max_bytes,
                                         qint64 fallback_bytes );

      //! \brief Cache capacity allowed for a wanted capacity:  at most half of
      //!        the job's limit (0: none) and most of the free shared memory
      //!        (negative: unknown)
      static qint64      allowed_capacity( qint64 wanted, qint64 job_limit, qint64 shm_bytes );

      //! \brief Capacity that fits the free shared memory (negative: unknown)
      static qint64      fit_shared_memory( qint64 wanted, qint64 shm_bytes );

      //! \brief Predicted bytes one rank needs to solve a set of solutes (the
      //!        estimate of the 2DSA fit control, per thread):  the simulations
      //!        and matrices of the solutes, data and noise terms
      //! \param readings     Readings (scans times points) of the data
      //! \param solutes      Solutes solved at once
      //! \param noise_terms  1 for TI noise, 2 for RI noise, 3 for both
      static qint64      solve_bytes( qint64 readings, qint64 solutes, int noise_terms );

      //! \brief Bytes the cache may hold while the job's own use peaks at
      //!        job_peak:  the cache and the peak times margin stay below the
      //!        share of the limit at which the memory watch stops adding.
      //!        0 if there is no room; -1 if there is no limit.
      static qint64      cache_room( qint64 job_limit, qint64 job_peak, double margin );

      //! \brief Pressure from a job's use of its limit
      enum Pressure
      {
         NONE = 0, //!< No action
         SOFT = 1, //!< Stop adding to the cache
         HARD = 2  //!< Empty the cache
      };

      //! \brief Pressure for bytes used of a limit (limit 0: none)
      static Pressure limit_pressure( qint64 used, qint64 limit );

      //! \brief Pressure for the node's available memory
      static Pressure node_pressure( qint64 total, qint64 available );

      //! \brief Pressure for the free bytes in shared memory, given the
      //!        capacity of the cache
      static Pressure shm_pressure( qint64 shm_bytes, qint64 capacity );
};

#endif
