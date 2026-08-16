//! \file us_sim_inputs.h
#ifndef US_SIM_INPUTS_H
#define US_SIM_INPUTS_H

#include "us_extern.h"
#include "us_model.h"
#include "us_buffer.h"
#include "us_simparms.h"
#include "us_sim_species.h"

//! \brief Generate minimal, valid inputs for headless simulations.
//! Uses each input class's defaults and serializer, without hand-written XML,
//! GUI interaction, or database access.
class US_UTIL_EXTERN US_SimInputs
{
   public:
      //! \brief Run conditions for a single-speed simulation.
      //! Shared fields inherit US_SimulationParameters defaults; run-specific
      //! fields match us_astfem_sim.
      class US_UTIL_EXTERN Params
      {
         public:
            Params();

            double  rpm;                 //!< Rotor speed, also used to derive
                                         //!< the meniscus and bottom positions
            int     duration_hours;
            double  duration_minutes;

            //! Delay before the first scan. Leave both negative to derive it
            //! from the time needed to reach speed; set them to align scan
            //! times across runs at different speeds.
            int     delay_hours;
            double  delay_minutes;

            int     scans;
            double  acceleration;
            int     simpoints;
            double  radial_resolution;
            US_SimulationParameters::MeshType meshType;
            US_SimulationParameters::GridType gridType;
            double  rnoise;
            double  lrnoise;
            double  tinoise;
            double  rinoise;
            double  baseline;
            bool    band_forming;
            double  band_volume;

            //! Rotor calibration ID. "0" selects zero stretch correction.
            QString rotor_calibr;

            int     centerpiece;         //!< Centerpiece list index
            int     centerpiece_channel; //!< Row within that centerpiece; not
                                         //!< an instrument channel label
      };

      //! \brief Validate run conditions before constructing parameters.
      //! \return An empty string if valid; otherwise, an error message.
      static QString validateParams( const Params& p );

      //! \brief Create parameters for a single-speed simulation.
      //! Geometry is stored at rest; simulation consumers apply rotor stretch.
      //! Validation is enforced here rather than left to the caller.
      //! \param p      Run conditions.
      //! \param params Parameters populated only on success.
      //! \param error  Error message on failure; empty on success.
      static bool simParams( const Params& p, US_SimulationParameters& params,
                             QString& error );

      //! \brief Create default single-speed simulation parameters.
      //! Construction reads hardware definitions from disk, so it can fail.
      //! \param params Parameters populated only on success.
      //! \param error  Error message on failure; empty on success.
      static bool simParams( US_SimulationParameters& params, QString& error );

      //! \brief Create a default buffer using water properties at 20 C.
      static US_Buffer buffer();

      //! \brief Write the default simulation parameters, model, and buffer.
      //! Creates sp_default.xml, model_default.xml, and buffer_default.xml
      //! using each class's serializer.
      //! \param dir   Existing output directory.
      //! \param error Error message on failure; empty on success.
      //! \return true on success; otherwise, false.
      static bool writeAll( const QString& dir, QString& error );
};

#endif // US_SIM_INPUTS_H
