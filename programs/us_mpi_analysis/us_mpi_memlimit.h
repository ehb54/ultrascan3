//! \file us_mpi_memlimit.h
#ifndef US_MPI_MEMLIMIT_H
#define US_MPI_MEMLIMIT_H

#include <QtGlobal>

#include <string>

//! \brief The memory a job may use on this node
//!
//! A job's limit is not the node's memory:  the scheduler gives it a share
//! (Slurm's --mem and --mem-per-cpu) and may enforce that with a cgroup.  This
//! class finds the limit by reading the environment and the proc and cgroup
//! files;  it starts no process (some MPI interconnect stacks do not allow a
//! fork after MPI_Init).  On a system without these files no limit is found.
//! The paths can be replaced for tests.
class US_MPI_MemLimit
{
   public:
      //! \brief Where the system files are
      class Paths
      {
         public:
            Paths() : proc( "/proc" ), cgroup( "/sys/fs/cgroup" )
            {
            }

            std::string proc;   //!< proc file system
            std::string cgroup; //!< cgroup file system
      };

      //! \brief Limit from the Slurm environment, in bytes (0: none):
      //!        SLURM_MEM_PER_NODE, or SLURM_MEM_PER_CPU times the CPUs of
      //!        this node (SLURM_CPUS_ON_NODE, else node_ranks)
      static qint64 slurm_limit( int node_ranks );

      //! \brief The tightest memory limit of this process's cgroup and its
      //!        parents, in bytes (0: none found).  cgroup v1 and v2.
      static qint64 cgroup_limit( const Paths& paths = Paths() );

      //! \brief The tightest of the Slurm and cgroup limits (0: none)
      static qint64 job_limit( int node_ranks, const Paths& paths = Paths() );

      //! \brief Read the node's total and available bytes (/proc/meminfo)
      static bool   node_memory( qint64& total, qint64& available, const Paths& paths = Paths() );

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
};

#endif
