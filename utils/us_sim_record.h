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
    Composes experiment and solution records for simulated scan data.
*/
class US_UTIL_EXTERN US_SimRecord
{
   public:

      /*! \brief Compose an experiment record for a simulated run.
          Uses default lab and instrument IDs and a fixed calibration date.
          Unspecified fields retain US_Experiment::clear() defaults.
          \param rotor     Simulation rotor.
          \param simparams Calibration, stretch coefficients, and temperature.
          \param runID     Run ID and label.
          \param runType   Two-character optical data type.
          \param guidSeed  Reproducible GUID seed; empty generates random GUIDs.
          \returns         An experiment ready for saveToDisk().
      */
      static US_Experiment experiment( const US_Rotor::Rotor&         rotor,
                                       const US_SimulationParameters& simparams,
                                       const QString&                 runID,
                                       const QString&                 runType,
                                       const QString&                 guidSeed = QString() );

      /*! \brief Generate a random or reproducible GUID.
          A nonempty seed produces a version 5 UUID from the seed and key.
          \param seed Run-level seed; empty generates a random GUID.
          \param key  Identifier unique within the run, such as "raw.0".
          \returns    A 36-character GUID.
      */
      static QString guid( const QString& seed, const QString& key );

      /*! \brief Compose the solution shared by a run's datasets.
          Component identities and amounts come from the first model.
          Extinction spectra combine the values from all model wavelengths.
          Wavelengths are explicit because derived models may store them only
          in their descriptions.
          \param models      Models in wavelength order.
          \param wavelengths One wavelength per model.
          \param buffer      Simulation buffer.
          \returns           A solution, or an empty solution for empty or
                             unequal-length inputs.
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
