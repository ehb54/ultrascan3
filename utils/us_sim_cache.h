//! \file us_sim_cache.h
#ifndef US_SIM_CACHE_H
#define US_SIM_CACHE_H

#include <QtCore>
#include <atomic>
#include <memory>

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
//! In float mode (debug text "SimFloat=on") the cache keeps the readings of
//! each simulation as floats, halving its memory.  US_SolveSim then rounds
//! every simulation it computes to float precision (see round_to_float), so
//! a fetched simulation is still exactly the one computed and a fit gives the
//! same result with and without the cache.  US_SolveSim takes simulations
//! with fetch_sim() and store_sim() in float form (Sim), shared with the
//! cache, so a cache hit copies no readings.
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
            bool   floats = false; //!< Flag readings are stored as floats
      };

      //! \brief A simulation as the cache keeps it:  implicitly shared, so
      //!        cheap to copy, and never modified once made
      //!
      //! In double form it holds the simulation.  In float form its readings
      //! are floats (rounded) and the simulation holds the rest (the scans
      //! without readings);  readers take each scan's readings as floats,
      //! which convert to the rounded doubles exactly.
      class US_UTIL_EXTERN Sim
      {
         public:
            //! \brief A simulation, in float form (readings rounded to float
            //!        precision) or double form (shared with simdat)
            static Sim from_raw( const US_DataIO::RawData& simdat, bool floats );

            //! \brief The simulation (in float form, readings from the floats)
            void       to_raw( US_DataIO::RawData& simdat ) const;

            //! \brief This simulation in float form (itself if it is)
            Sim        to_float() const;

            //! \brief Flag the readings are floats
            bool       is_float() const
            {
               return floats;
            }

            //! \brief Number of scans
            int scanCount() const
            {
               return simdat.scanData.size();
            }

            //! \brief Number of radial points
            int pointCount() const
            {
               return simdat.xvalues.size();
            }

            //! \brief Readings of a scan (double form)
            const double* dscan( int ss ) const
            {
               return simdat.scanData.at( ss ).rvalues.constData();
            }

            //! \brief Readings of a scan (float form)
            const float* fscan( int ss ) const
            {
               return fvalues.get() + offsets.at( ss );
            }

            //! \brief One reading
            double value( int ss, int rr ) const
            {
               return floats ? (double) fscan( ss )[rr] : dscan( ss )[rr];
            }

            //! \brief Approximate bytes the simulation takes in the cache
            qint64      bytes() const;

            //! \brief Serialize the simulation (bit-exact), e.g. for a cache
            //!        in memory shared between processes:  the float form
            //!        with its readings as floats (half the bytes), the
            //!        double form as US_SimCache::pack
            QByteArray  pack_bytes() const;

            //! \brief Restore a simulation serialized by pack_bytes (either
            //!        form) or by US_SimCache::pack (double form)
            //! \returns Flag whether the bytes were a complete simulation
            static bool unpack( const char* data, qint64 size, Sim& sim );

         private:
            US_DataIO::RawData           simdat; // All, or (float form) without readings
            // Readings of all scans (float form), in a block of their own
            //  (see alloc_floats in us_sim_cache.cpp), shared by the copies
            std::shared_ptr<const float> fvalues;
            qsizetype                    nfloats = 0; // Number of readings (float form)
            QVector<qsizetype>           offsets;     // Offset of each scan's readings (float form)
            bool                         floats = false;
      };

      //! \brief Create a cache
      //! \param max_bytes  Memory cap in bytes
      //! \param floats     Flag to store readings as floats (float mode;
      //!                   simulations stored should be rounded to float
      //!                   precision, see round_to_float).  Float mode uses
      //!                   this class's own storage:  a subclass that
      //!                   overrides fetch() and store() must not turn it on.
      explicit US_SimCache( qint64 max_bytes, bool floats = false );

      //! \brief Destroy the cache (subclasses may hold other storage)
      virtual ~US_SimCache() = default;

      //! \brief Create a cache with the default memory cap, unless debug
      //!        text turns caching off
      //!
      //! Debug text:  "SimCache=off" disables the cache (a null pointer is
      //! returned);  "SimCacheMB=N" sets the cap to N megabytes.  The default
      //! cap is 25% of the memory now available, at least 256 MB and at
      //! most 4 GB.  "SimFloat=on" turns on float mode.
      //! \returns  A new cache owned by the caller, or a null pointer
      static US_SimCache* create_default();

      //! \brief The default memory cap in bytes (0 if caching is off)
      static qint64       default_max_bytes();

      //! \brief Flag from debug text "SimFloat=on" that simulations are
      //!        rounded to float precision (and cached as floats)
      static bool         float_sims();

      //! \brief Round the readings of a simulation to float precision
      //! \param simdat  Simulation whose readings are rounded
      static void         round_to_float( US_DataIO::RawData& simdat );

      //! \brief Flag the cache stores readings as floats (float mode)
      bool                float_mode() const
      {
         return floats;
      }

      //! \brief Key of the inputs of a simulation besides its component:
      //!        the simulation parameters and the experiment's grid
      //! \param simparams  Simulation parameters of the data set
      //! \param edata      Experiment data whose grid is simulated
      //! \returns          Key (a hash of the values)
      static QByteArray   dataset_key( const US_SimulationParameters& simparams, const US_DataIO::EditedData& edata );

      //! \brief Key of a data set, as dataset_key;  with the memo on (see
      //!        set_key_memo), computed once for each fingerprint of its values
      //!
      //! The fingerprint holds all scalar values but only the address and size
      //! of each vector (radii, time states, scan interpolation flags), so it
      //! is cheap.
      //! \param simparams  Simulation parameters of the data set
      //! \param edata      Experiment data whose grid is simulated
      //! \returns          Key (a hash of the values)
      QByteArray          dataset_key_memo( const US_SimulationParameters& simparams, const US_DataIO::EditedData& edata );

      //! \brief Turn the memo of data set keys on or off (default off)
      //!
      //! The memo does not see vector values that change in place (same
      //! address and size), nor a freed vector whose address and size are
      //! reused with other values.  Turn it on only where the simulation
      //! parameters and experiment grids of the data sets are not changed so
      //! for the life of the cache (us_mpi_analysis GA:  data sets live for
      //! the whole job).
      //! \param on  Flag to use the memo
      void                set_key_memo( bool on );

      //! \brief Forget the memorized data set keys (clear() does, too).  Call
      //!        it when the data sets change while the cache lives on.
      void                forget_dataset_keys();

      //! \brief Turn least-recently-used mode on or off (default off)
      //!
      //! In this mode survivor flags are not used:  when the cap is reached,
      //! the least recently used (fetched or stored) simulations are evicted,
      //! down to 90% of the cap.  Suits a search that keeps simulating new
      //! solutes near those it reuses (GA), where nearly every solute
      //! survives some NNLS and the default policy fills the cache with
      //! survivors and then rejects all new simulations.
      //! \param on  Flag to use LRU eviction
      void                set_lru( bool on );

      //! \brief Key of a simulation
      //! \param dskey  Key of the data set (from dataset_key)
      //! \param comp   Simulated component (experimental space)
      //! \returns      Key
      static QByteArray   sim_key( const QByteArray& dskey, const US_Model::SimulationComponent& comp );

      //! \brief Get a simulation
      //! \param key     Simulation key
      //! \param simdat  Returned simulation, if found
      //! \returns       Flag whether found
      virtual bool        fetch( const QByteArray& key, US_DataIO::RawData& simdat );

      //! \brief Store a simulation, or mark a stored one as a survivor
      //! \param key       Simulation key
      //! \param simdat    Simulation as computed
      //! \param survivor  Flag the solute survived a NNLS:  keep it, evicting
      //!                  other simulations if need be
      //! \returns         Flag whether the simulation is now in the cache
      virtual bool        store( const QByteArray& key, const US_DataIO::RawData& simdat, bool survivor );

      //! \brief Get a simulation as the cache keeps it:  in float mode in
      //!        float form, shared with the cache (no copy);  otherwise as
      //!        fetch() gives it, in double form
      //! \param key  Simulation key
      //! \param sim  Returned simulation, if found
      //! \returns    Flag whether found
      //!
      //! A subclass with its own storage (e.g. a cache shared between
      //! processes) overrides fetch_sim and store_sim together with fetch and
      //! store;  in float mode it returns float-form simulations.
      virtual bool        fetch_sim( const QByteArray& key, Sim& sim );

      //! \brief Store a simulation (in either form), or mark a stored one as a
      //!        survivor, as store() does.  In float mode a float-form
      //!        simulation is kept shared (no copy).
      virtual bool        store_sim( const QByteArray& key, const Sim& sim, bool survivor );

      //! \brief Remove all simulations (the counts are kept)
      virtual void        clear();

      //! \brief Counts of cache use
      virtual Stats       stats() const;

      //! \brief One-line text of the counts
      QString             stats_text() const;

      //! \brief Approximate bytes a simulation takes in the cache
      //! \param floats  Flag readings are stored as floats
      static qint64       sim_bytes( const US_DataIO::RawData&, bool floats = false );

      //! \brief Serialize a simulation (all values, bit-exact), e.g. for a
      //!        cache in memory shared between processes
      //! \param simdat  Simulation
      //! \returns       Bytes from which unpack() restores the simulation
      static QByteArray   pack( const US_DataIO::RawData& simdat );

      //! \brief Restore a simulation serialized by pack()
      //! \param data    Serialized bytes
      //! \param size    Number of bytes
      //! \param simdat  Restored simulation
      //! \returns       Flag whether the bytes were a complete simulation
      static bool         unpack( const char* data, qint64 size, US_DataIO::RawData& simdat );

   private:
      class Entry
      {
         public:
            Sim    sim; // In float form in float mode
            qint64 bytes;
            bool   survivor;
            qint64 last_use = 0; // Use clock value (LRU mode)
      };

      // Get a simulation from this class's storage
      bool                          fetch_stored( const QByteArray&, Sim& );

      // Store a simulation in this class's storage (in float mode in float
      //  form), or mark a stored one as a survivor
      bool                          store_stored( const QByteArray&, const Sim&, bool );

      // Evict non-survivors, newest first, until "need" bytes fit
      void                     make_room( qint64 );

      // Evict the least recently used simulations (LRU mode)
      void                          evict_lru( qint64 );

      mutable QReadWriteLock   lock;
      QHash<QByteArray, QByteArray> dskey_memo; // Data set keys by fingerprint
      QHash<QByteArray, Entry> sims;
      QList<QByteArray>        evictable; // Non-survivors, stored order
      qint64                   max_bytes;
      bool                          floats; // Float mode
      qint64                   nbytes;
      qint64                   nsurvivors;
      qint64                   nstored;
      qint64                   nevicted;
      qint64                   nrejected;
      std::atomic<qint64>      nhits;
      std::atomic<qint64>      nmisses;
      bool                          key_memo;  // Memo of data set keys on
      bool                          lru;       // Least-recently-used mode
      qint64                        use_clock; // Use clock (LRU mode)
};
#endif
