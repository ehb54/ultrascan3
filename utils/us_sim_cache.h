//! \file us_sim_cache.h
#ifndef US_SIM_CACHE_H
#define US_SIM_CACHE_H

#include <QtCore>
#include <atomic>

#include "us_dataIO.h"
#include "us_extern.h"
#include "us_model.h"
#include "us_simparms.h"

//! \brief Cache of single-component simulations, shared by the threads of a
//!        fit
//!
//! A single-component simulation depends only on the component values, the
//! simulation parameters and the grid of the experiment (radii and scans), not
//! on the data values.  A fit simulates the same solutes again and again:
//! the survivors of each subgrid at every merge depth, and whole grids again
//! in refinement and Monte Carlo iterations.  US_SolveSim reuses simulations
//! from a cache it is given (see US_SolveSim::set_sim_cache).
//!
//! Simulations are stored exactly (as computed) under a key of their inputs,
//! so a fit gives the same result with and without a cache.  The key is a
//! byte array built from values only (no pointers), so it is the same in every
//! process and can be sent to other processes.
//!
//! Memory is capped.  Survivors (simulations of solutes with non-zero
//! concentration after a NNLS) are kept for the life of the cache; other
//! simulations are stored while there is room and are evicted, newest first,
//! to make room for survivors.
//!
//! All methods are thread-safe.
class US_UTIL_EXTERN US_SimCache
{
   public:
      //! \brief Counts of cache use
      class US_UTIL_EXTERN Stats
      {
         public:
            qint64 hits;      //!< Fetches that found a simulation
            qint64 misses;    //!< Fetches that found none
            qint64 stored;    //!< Simulations stored
            qint64 evicted;   //!< Simulations evicted to make room for survivors
            qint64 rejected;  //!< Simulations not stored for lack of room
            qint64 entries;   //!< Simulations now in the cache
            qint64 survivors; //!< ... of which survivors
            qint64 bytes;     //!< Approximate bytes now used
            qint64 max_bytes; //!< Memory cap in bytes
      };

      //! \brief Create a cache
      //! \param max_bytes  Memory cap in bytes
      explicit US_SimCache( qint64 max_bytes );

      //! \brief Create a cache with the default memory cap, unless debug
      //!        text turns caching off
      //!
      //! Debug text:  "SimCache=off" disables the cache (a null pointer is
      //! returned);  "SimCacheMB=N" sets the cap to N megabytes.  The default
      //! cap is 25% of the memory now available, at least 256 MB and at
      //! most 4 GB.
      //! \returns  A new cache owned by the caller, or a null pointer
      static US_SimCache* create_default();

      //! \brief The default memory cap in bytes (0 if caching is off)
      static qint64       default_max_bytes();

      //! \brief Key of the inputs of a simulation besides its component:
      //!        the simulation parameters and the experiment's grid
      //! \param simparams  Simulation parameters of the data set
      //! \param edata      Experiment data whose grid is simulated
      //! \returns          Key (a hash of the values)
      static QByteArray   dataset_key( const US_SimulationParameters& simparams, const US_DataIO::EditedData& edata );

      //! \brief Key of a simulation
      //! \param dskey  Key of the data set (from dataset_key)
      //! \param comp   Simulated component (experimental space)
      //! \returns      Key
      static QByteArray   sim_key( const QByteArray& dskey, const US_Model::SimulationComponent& comp );

      //! \brief Get a simulation
      //! \param key     Simulation key
      //! \param simdat  Returned simulation, if found
      //! \returns       Flag whether found
      bool                fetch( const QByteArray& key, US_DataIO::RawData& simdat );

      //! \brief Store a simulation, or mark a stored one as a survivor
      //! \param key       Simulation key
      //! \param simdat    Simulation as computed
      //! \param survivor  Flag the solute survived a NNLS:  keep it, evicting
      //!                  other simulations if need be
      //! \returns         Flag whether the simulation is now in the cache
      bool                store( const QByteArray& key, const US_DataIO::RawData& simdat, bool survivor );

      //! \brief Remove all simulations (the counts are kept)
      void                clear();

      //! \brief Counts of cache use
      Stats               stats() const;

      //! \brief One-line text of the counts
      QString             stats_text() const;

      //! \brief Approximate bytes a simulation takes in the cache
      static qint64       sim_bytes( const US_DataIO::RawData& );

   private:
      class Entry
      {
         public:
            US_DataIO::RawData simdat;
            qint64             bytes;
            bool               survivor;
      };

      // Evict non-survivors, newest first, until "need" bytes fit
      void                     make_room( qint64 );

      mutable QReadWriteLock   lock;
      QHash<QByteArray, Entry> sims;
      QList<QByteArray>        evictable; // Non-survivors, stored order
      qint64                   max_bytes;
      qint64                   nbytes;
      qint64                   nsurvivors;
      qint64                   nstored;
      qint64                   nevicted;
      qint64                   nrejected;
      std::atomic<qint64>      nhits;
      std::atomic<qint64>      nmisses;
};
#endif
