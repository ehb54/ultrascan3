//! \file us_sim_record.h
#ifndef US_SIM_RECORD_H
#define US_SIM_RECORD_H

#include "us_extern.h"
#include "us_experiment.h"
#include "us_solution.h"
#include "us_model.h"
#include "us_buffer.h"
#include "us_rotor.h"
#include "us_simparms.h"
#include "us_dataIO.h"

/*! \class US_SimRecord
    Composes the records a simulated run exports alongside its scan data.

    A simulator produces .auc files, but a US3 import directory is only
    loadable if it also carries the documents that say what was in the cell:
    the US_Scandata experiment record and the solution it references. Both
    simulators build those the same way, from the same inputs, so the
    composition lives here rather than being written out twice.

    Nothing here touches the database or the simulation itself. These are
    pure factories: they take what a run already knows and return a value.
*/
class US_UTIL_EXTERN US_SimRecord
{
   public:

      /*! \brief Compose the experiment record for a simulated run.

          Fields a simulation cannot supply are left at the values
          US_Experiment::clear() produces. The ones that follow have no real
          counterpart and take fixed values instead: the lab and instrument
          are the first of each, and the calibration date is fixed rather
          than read from the clock, so two runs of the same inputs differ
          only in the GUIDs they mint.

          \param rotor     The rotor the run was simulated on.
          \param simparams The run's simulation parameters; supplies the
                           calibration ID, the stretch coefficients, and the
                           temperature.
          \param runID     The run ID, used as both runID and label.
          \param runType   Two-character optical data type tag ("RA", "RI",
                           "IP", "FI", "WA", "WI").
          \param guidSeed  Seed for a reproducible expGUID; empty mints a
                           random one, which is what the desktop does.
          \returns         An experiment ready for saveToDisk.
      */
      static US_Experiment experiment( const US_Rotor::Rotor&         rotor,
                                       const US_SimulationParameters& simparams,
                                       const QString&                 runID,
                                       const QString&                 runType,
                                       const QString&                 guidSeed = QString() );

      /*! \brief A GUID that is reproducible when a seed is given.

          With an empty seed this is US_Util::new_guid(), so nothing that does
          not ask for reproducibility changes behaviour. With a seed it is a
          version 5 (name based) UUID over the seed and the key, so simulating
          the same inputs again exports the same identities and the archive
          built from them stops changing for reasons no one can review.

          The seed names the run and must be unique across runs; the key names
          what within the run is being identified, so it only has to be unique
          within one.

          \param seed Run-level seed; empty for the previous behaviour.
          \param key  What is being identified, for example "raw.0".
          \returns    A 36-character GUID.
      */
      static QString guid( const QString& seed, const QString& key );

      /*! \brief Compose the solution a simulated run's datasets share.

          Every dataset of a simulated run holds the same sample, so there is
          one solution: the components of the first model, in the run's
          buffer. Amounts come from that first model as well, because a
          solution records what was loaded rather than what each dataset
          measured.

          Extinction is the exception. A component collects one entry per
          model, keyed by that model's wavelength, which is what makes a
          multi-wavelength run's analyte a spectrum instead of a single
          reading. A single-model run yields a one-entry map, matching what
          a single-wavelength simulation has always written.

          Wavelengths are passed in rather than read from US_Model::wavelength
          because the per-wavelength models a multi-wavelength run derives
          leave that member at zero and carry the wavelength in their
          description instead.

          \param models      The run's models, first one authoritative for
                             component identity and amount.
          \param wavelengths One wavelength per model, in the same order.
          \param buffer      The buffer the components were simulated in.
          \returns           A solution with a description of the form
                             "name name ... | buffer description". Empty if
                             the inputs are empty or of unequal length.
      */
      static US_Solution solution( const QList< US_Model >& models,
                                   const QList< double >&   wavelengths,
                                   const US_Buffer&         buffer );

      /*! \brief The largest value in a dataset, the OD ceiling an edit file
                 records and RMSD calculations read.
          \param data The dataset to scan.
          \returns    The maximum, or zero for an empty dataset.
      */
      static double maxOD( US_DataIO::RawData& data );
};

#endif
