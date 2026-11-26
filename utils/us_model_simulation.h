//! \file us_model_simulation.h
#ifndef US_MODEL_SIMULATION_H
#define US_MODEL_SIMULATION_H

#include "ius_db2.h"
#include "us_buffer.h"
#include "us_dataIO.h"
#include "us_extern.h"
#include "us_model.h"
#include "us_simparms.h"

//! \brief The simulation of a model against edited data, as FEMatch, autoflow
//! analysis and the GMP reporter run it.
//!
//! prepare() sets the simulation parameters and the simulated data's time grid.
//! simulate() then solves in the calling thread; start() splits the model's
//! components among worker threads, adds their results into the simulated
//! data and emits finished().
class US_UTIL_EXTERN US_ModelSimulation : public QObject
{
      Q_OBJECT

   public:
      explicit US_ModelSimulation( QObject* parent = nullptr );

      //! \brief Set \a simparams and the time grid of \a sdata for simulating
      //! \a model against \a edata.
      //! \param db          Connection for the rotor and centerpiece, or null
      //! \param invID       Investigator who owns the run; -1 for the current one
      //! \param edata       Experimental data
      //! \param model       The adjusted model; its first component decides the mesh
      //! \param adv_vals    Advanced settings: simpoints, gridtype, bandvolume
      //! \param exp_steps   Use \a speed_steps, the experiment's; otherwise
      //!                    the steps come from the data
      //! \param speed_steps The experiment's speed steps
      //! \param tmst_fpath  The run's time state file
      //! \param rewrite_timestate Replace a time state that is not at
      //!                    1-second intervals with one that is
      //! \param simparams   Set
      //! \param sdata       Set to the data's time grid, with no readings
      //! \return The acceleration check's title, text and details; empty if
      //! the acceleration is acceptable.
      static QStringList prepare( IUS_DB2*                                              db,
                                  int                                                   invID,
                                  US_DataIO::EditedData&                                edata,
                                  const US_Model&                                       model,
                                  const QMap<QString, QString>&                         adv_vals,
                                  bool                                                  exp_steps,
                                  const QVector<US_SimulationParameters::SpeedProfile>& speed_steps,
                                  const QString&                                        tmst_fpath,
                                  bool                                                  rewrite_timestate,
                                  US_SimulationParameters&                              simparams,
                                  US_DataIO::RawData&                                   sdata );

      //! \brief Threads to use for \a ncomp components: the configured
      //! number, or 1 if any thread would get 25 components or fewer.
      static int         threadCount( int ncomp );

      //! \brief Solve \a model into \a sdata in this thread: the finite element
      //! solver for an ideal, non-co-sedimenting model without
      //! compressibility, otherwise the finite volume solver. Emits component()
      //! for finite element progress.
      //! \param buffer Set to \a compress and \a manual for the finite element
      //!               solver; neither solver is given it.
      void               simulate( US_Model&                model,
                                   US_SimulationParameters& simparams,
                                   US_DataIO::RawData&      sdata,
                                   US_Buffer&               buffer,
                                   double                   compress,
                                   bool                     manual,
                                   int                      dbg_level );

      //! \brief Split \a model's components round-robin among \a nthread
      //! worker threads and start them. Emits progress() with the components
      //! done in all threads, and finished() once their results are added
      //! into \a sdata, which must stay valid until then.
      //! \param buffer Set to \a compress and \a manual, and given to the workers
      void               start( US_Model&                model,
                                US_SimulationParameters& simparams,
                                US_DataIO::RawData&      sdata,
                                US_Buffer&               buffer,
                                double                   compress,
                                bool                     manual,
                                int                      nthread );

   signals:
      //! \brief simulate(): the finite element solver finished component \a icomp
      void component( int icomp );
      //! \brief start(): the components finished in all threads
      void progress( int kcomp );
      //! \brief start(): every thread is done and \a sdata holds the sum
      void finished();

   private slots:
      void thread_progress( int thr, int icomp );
      void thread_complete( int thr );

   private:
      US_DataIO::RawData*       sdata   = nullptr;
      int                       nthread = 0;
      int                       thrdone = 0;
      QVector<int>              kcomps;
      QList<US_DataIO::RawData> tsimdats;
      QList<US_Model>           tmodels;
};

#endif
