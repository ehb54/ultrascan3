//! \file us_mpi_sim_cache.h
#ifndef US_MPI_SIM_CACHE_H
#define US_MPI_SIM_CACHE_H

#include <mpi.h>

#include "us_mpi_memlimit.h"
#include "us_sim_cache.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

//! \brief Simulation cache in memory shared by the MPI ranks of a node
//!
//! One copy per node replaces a copy per process:  every rank of the node
//! (master, workers, parallel-masters supervisor) reads and adds simulations
//! in an MPI-3 shared-memory window allocated by the node's first rank.
//!
//! Layout of the window:  a header, a table of slots, and a data area of
//! simulations serialized with US_SimCache::pack (so they are restored
//! bit-exactly).  A slot is found by open addressing on the SHA-256 of the
//! simulation key and filled lock-free:  a rank claims an empty slot with a
//! compare-and-swap, writes the key digest, appends the data and then marks
//! the slot ready.  A slot being filled by another rank counts as a miss.
//!
//! Two policies decide what is kept once the cache is full.  The survivor
//! flag of store() is ignored by both:  in a GA nearly every solute of a
//! gene has a nonzero concentration, so the flag does not single out the
//! simulations worth keeping.
//!
//! KEEP_FIRST (2DSA, PCSA):  simulations are kept in the order they arrive;
//! when the data area or the table is full, further simulations are
//! computed but not stored.  These methods repeat the same sequence of
//! solutes (refinement and Monte Carlo iterations, meniscus and bottom
//! points), and for a cyclic sequence longer than the cache keeping its first
//! part reuses a share capacity/sequence of it, while discarding the oldest
//! simulations would reuse none.
//!
//! RECENT (GA):  the cache is split into two generations of half the size.
//! Simulations go into the current generation; a hit in the previous one is
//! copied into the current one.  When the current generation is full, the
//! previous one is emptied and becomes current.  The cache so holds the
//! simulations used most recently, which is what a GA population, drifting
//! toward the fit, reuses.  A rank empties a generation only when no rank
//! reads or writes it:  ranks announce their use of a generation in its
//! user count, and leave a generation being emptied alone.
//!
//! Room:  the cache may hold less than its window.  Its room (bytes it may
//! hold now) starts where the job's predicted peak leaves space below its
//! memory limit, and is set again (fit_room_to_peak) once the watch has seen
//! the job's real peak, e.g. after the first refinement iteration.  A cache
//! full at its room takes more simulations again when the room grows.
//!
//! Memory pressure:  the cache gives way to the job's own memory.  A thread on
//! the node's first rank (start_watch) compares the job's use with its limit,
//! the node's available memory and the free shared memory.  Under soft
//! pressure no rank adds to the cache, except that RECENT empties its previous
//! generation to make room.  Under hard pressure the thread empties the cache.
//! Emptying returns the pages to the system, so that the use really falls.
//! The next simulations then have to be computed again, but never differ.
//!
//! The window lives where the MPI implementation puts it:  a file in /dev/shm
//! (MPICH), a file in a session directory that may be on a disk (Open MPI),
//! System V shared memory, or a section of the paging file (MS-MPI).  Its
//! size is fitted to the free space of the file system that really holds it
//! (see probe_backing), as touching a page beyond that kills the process.
//!
//! The constructor and destructor are collective over the node communicator.
//! fetch(), store() and the counts are thread-safe, for any number of threads
//! in each rank.  clear() may only be called while no other rank or thread of
//! the node uses the cache.
class US_MPI_NodeSimCache : public US_SimCache
{
   public:
      //! \brief What to keep once the cache is full
      enum Policy
      {
         KEEP_FIRST = 0, //!< Keep the first simulations; store no more when full
         RECENT     = 1  //!< Keep the most recently used simulations
      };

      //! \brief Create the cache (collective over node_comm)
      //! \param node_comm  Communicator of the ranks of one node
      //! \param max_bytes  Capacity of the data area in bytes
      //! \param sim_bytes  Expected bytes of one serialized simulation, to size
      //!                   the slot table
      //! \param policy     What to keep once the cache is full (the same on
      //!                   every rank of the node)
      //! \param room_bytes Bytes the cache may hold at first (negative: all
      //!                   of max_bytes); taken from the node's first rank
      //! \param space_bytes Free bytes of the file system that will hold the
      //!                   window (negative: unknown).  The node's first rank
      //!                   checks before the collective allocation that the
      //!                   window fits it and the node's memory:  otherwise
      //!                   no rank allocates and the cache is not valid().
      US_MPI_NodeSimCache( MPI_Comm node_comm,
                           qint64   max_bytes,
                           qint64   sim_bytes,
                           Policy   policy      = KEEP_FIRST,
                           qint64   room_bytes  = -1,
                           qint64   space_bytes = -1 );

      //! \brief Bytes of the whole window of a cache (header, slot table and
      //!        data) of the given capacity
      static qint64 window_bytes( qint64 max_bytes, qint64 sim_bytes, Policy policy );

      //! \brief Largest capacity, at most wanted, whose whole window fits 90 %
      //!        of the free bytes of its file system (negative: unknown)
      static qint64 fit_window( qint64 wanted, qint64 space_bytes, qint64 sim_bytes, Policy policy );

      //! \brief Free the shared window (collective over the node communicator)
      ~US_MPI_NodeSimCache() override;

      //! \brief Directory of the file system that holds a shared window of
      //!        this MPI implementation on this node (collective over
      //!        node_comm:  the node's first rank maps a small window to see).
      //!        Empty on other ranks, and where the window is not a file of a
      //!        file system (anonymous or System V shared memory, Windows).
      static std::string probe_backing( MPI_Comm node_comm );

      //! \brief Directory of the file system holding this cache's window, as
      //!        found by the node's first rank (empty if not a file or unknown)
      const std::string& backing() const
      {
         return backing_dir;
      }

      //! \brief Flag whether the shared window could be allocated on every rank
      //!        of the node (the cache must not be used otherwise)
      bool valid() const
      {
         return base != nullptr;
      }

      bool   fetch( const QByteArray& key, US_DataIO::RawData& simdat ) override;
      bool   store( const QByteArray& key, const US_DataIO::RawData& simdat, bool survivor ) override;
      void   clear() override;
      Stats  stats() const override;

      //! \brief Policy of the cache
      Policy policy() const
      {
         return keep;
      }

      //! \brief Turn storing on or off for this rank (fetching stays on).
      //!        Off for evaluations unlikely to be repeated, e.g. the
      //!        off-grid points of the GA gradient search.
      void set_store_enabled( bool enabled )
      {
         store_on.store( enabled, std::memory_order_relaxed );
      }

      //! \brief Settings of the memory watch
      class Watch
      {
         public:
            qint64                  job_limit = 0;  //!< The job's bytes on this node (0: none)
            US_MPI_MemLimit::Cgroup cgroup;         //!< Cgroup level of the job
            bool                    stress = false; //!< Test:  empty the cache at every look
            std::string             shm_dir;        //!< File system of the window (empty: /dev/shm)
      };

      //! \brief Watch the memory use from a thread (on the node's first rank)
      void   start_watch( const Watch& settings );

      //! \brief Set the bytes the cache may hold (at most its window's data
      //!        area).  Any rank may call it at any time; less room keeps what
      //!        is stored, but adds nothing until the cache is below it.
      void   set_room( qint64 bytes );

      //! \brief Bytes the cache may hold now
      qint64 room() const;

      //! \brief Highest use of the job's own memory on this node that the
      //!        watch has seen, without the cache (0: none seen)
      qint64 job_peak() const;

      //! \brief Set the room from the job's peak the watch has seen:  the
      //!        cache and the peak times margin stay below the limit at which
      //!        the watch stops adding (node's first rank, with the watch).
      //!        Returns the new room, or -1 if unchanged (no limit, no peak).
      qint64 fit_room_to_peak( double margin );

      //! \brief Number of times the cache was emptied for lack of memory
      qint64 releases() const;

      //! \brief Bytes returned to the system by emptying under pressure
      qint64 released_bytes() const;

      //! \brief Bytes of memory the cache holds now:  the pages written and
      //!        not released (more than stats().bytes after a rotation, as
      //!        an emptied generation keeps its pages until refilled)
      qint64 resident_bytes() const;

      //! \brief Bytes to add to this rank's RSS so that the ranks of the node
      //!        together count the cache once:  each rank drops the pages of
      //!        the window it has mapped, and the node's first rank adds what
      //!        the cache holds.  A rank's pages of the window are the growth
      //!        of its shared memory since the window was made.  0 where
      //!        that is unknown (not on Linux).
      qint64 rss_correction() const;

      //! \brief Counts of the whole node:  this rank's fetch and store counts
      //!        summed over the node's ranks (collective over node_comm)
      Stats  node_stats() const;

   private:
      enum SlotState
      {
         EMPTY   = 0, // Never used
         CLAIMED = 1, // Taken by a rank; digest not yet written
         FILLING = 2, // Digest written; data being written
         READY   = 3, // Simulation stored
         DEAD    = 4  // Digest written, but no room for the data
      };

      class Header;
      class Generation;
      class Slot;

      // Slot of a key digest in generation gg:  the slot holding it, or else
      //  the first empty slot of its probe sequence (null if the table is full)
      Slot*                   find( int gg, const QByteArray& digest, bool& found );

      // Get or store a simulation in generation gg (entered by this rank)
      bool                    fetch_in( int gg, const QByteArray& digest, US_DataIO::RawData& simdat );
      int                     store_in( int gg, const QByteArray& digest, const US_DataIO::RawData& simdat );

      // Announce and end this rank's use of generation gg.  enter() fails
      //  while the generation is being emptied.
      bool                    enter( int gg );
      void                    leave( int gg );

      // Empty the previous generation and make it current, if generation gg
      //  is still current and no other rank does so already.  With give_back,
      //  a generation that holds simulations is emptied to free memory:
      //  returns whether that happened.
      bool                    rotate( int gg, bool give_back = false );

      // Empty generation gg (no rank may use it)
      void                    empty( int gg );

      // Return the pages of generation gg's data to the system:  returns
      //  whether the system took them
      bool                    release_pages( int gg );

      // Wait for the sole right to empty generations
      void                    lock_maintenance();

      // Empty every generation, while other ranks may use the cache
      void                    empty_under_pressure( qint64 used );

      // The memory watch
      void                    watch_loop();
      qint64                  bytes_held() const;

      MPI_Comm                comm;         // Node communicator
      MPI_Win                 window;       // Shared window
      char*                   base;         // Start of the window in this process
      Header*                 header;       // Shared header
      Slot*                   table;        // Shared slot table
      char*                   data;         // Shared data area
      qint64                  nslots;       // Number of slots
      qint64                  data_size;    // Bytes of the data area
      Policy                  keep;         // Policy
      int                     ngens;        // Generations:  1 (KEEP_FIRST) or 2 (RECENT)
      qint64                  gen_slots;    // Slots of a generation
      qint64                  gen_size;     // Bytes of the data area of a generation
      std::atomic<bool>       store_on;     // Flag:  this rank stores simulations
      std::string             backing_dir;  // File system of the window (node's first rank)
      qint64                  total_size;   // Bytes of the whole window
      bool                    first_rank;   // Flag:  the node's first rank
      qint64                  shared_start; // Rank's shared bytes before the window was used

      // Memory watch (only on the node's first rank)
      Watch                   watch;
      std::vector<long>       node_pids;
      std::thread             watcher;
      std::mutex              watch_lock;
      std::condition_variable watch_wake;
      bool                    watch_stop;

      // Counts of this rank (of all its threads)
      std::atomic<qint64>     rank_hits;
      std::atomic<qint64>     rank_misses;
      std::atomic<qint64>     rank_stored;
      std::atomic<qint64>     rank_rejected;
      std::atomic<qint64>     rank_evicted;
};

#endif
