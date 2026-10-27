//! \file us_sim_inputs.h
#ifndef US_SIM_INPUTS_H
#define US_SIM_INPUTS_H

#include "us_buffer.h"
#include "us_extern.h"
#include "us_model.h"
#include "us_sim_species.h"
#include "us_simparms.h"

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

            double                            rpm; //!< Rotor speed, also used to derive
                                                   //!< the meniscus and bottom positions
            int                               duration_hours;
            double                            duration_minutes;

            //! Delay before the first scan. Leave both negative to derive it
            //! from the time needed to reach speed; set them to align scan
            //! times across runs at different speeds.
            int                               delay_hours;
            double                            delay_minutes;

            int                               scans;
            double                            acceleration;
            int                               simpoints;
            double                            radial_resolution;
            US_SimulationParameters::MeshType meshType;
            US_SimulationParameters::GridType gridType;
            double                            rnoise;
            double                            lrnoise;
            double                            tinoise;
            double                            rinoise;
            double                            baseline;
            bool                              band_forming;
            double                            band_volume;

            //! Rotor calibration ID. "0" selects zero stretch correction.
            QString                           rotor_calibr;

            int                               centerpiece;         //!< Centerpiece list index
            int                               centerpiece_channel; //!< Row within that centerpiece; not
                                                                   //!< an instrument channel label
      };

      //! \brief Validate run conditions before constructing parameters.
      //! \return An empty string if valid; otherwise, an error message.
      static QString               validateParams( const Params& p );

      //! \brief Error formats validateParams() and simParams() return, shared
      //!        with the tests; fill them with QString::arg().
      //! A value that must be positive: %1 its name, %2 the value.
      static constexpr const char* notPositiveMessage    = "%1 must be finite and greater than zero (got %2)";
      //! A value that must not be negative: %1 its name, %2 the value.
      static constexpr const char* negativeMessage       = "%1 must be finite and zero or greater (got %2)";
      //! Fewer than one scan: %1 the scan count.
      static constexpr const char* tooFewScansMessage    = "scans must be at least 1 (got %1)";
      //! Band forming without a band-forming centerpiece: %1 the centerpiece.
      static constexpr const char* notBandFormingMessage = "band forming requires a band-forming centerpiece "
                                                           "(centerpiece %1 is not band-forming)";

      //! \brief Create parameters for a single-speed simulation.
      //! Geometry is stored at rest; simulation consumers apply rotor stretch.
      //! Validates input first; returns false on failure.
      //! \param p      Run conditions.
      //! \param params Parameters populated only on success.
      //! \param error  Error message on failure; empty on success.
      static bool                  simParams( const Params& p, US_SimulationParameters& params, QString& error );

      //! \brief Create default single-speed simulation parameters.
      //! Construction reads hardware definitions from disk, so it can fail.
      //! \param params Parameters populated only on success.
      //! \param error  Error message on failure; empty on success.
      static bool                  simParams( US_SimulationParameters& params, QString& error );

      //! \brief Create a default buffer using water properties at 20 C.
      //! \param guidSeed Reproducible GUID seed; empty generates a random GUID.
      static US_Buffer             buffer( const QString& guidSeed = QString() );

      //! \brief Write the default simulation parameters, model, and buffer.
      //! Creates sp_default.xml, model_default.xml, and buffer_default.xml
      //! using each class's serializer.
      //! \param dir      Existing output directory.
      //! \param error    Error message on failure; empty on success.
      //! \param guidSeed Reproducible GUID seed for the model and buffer;
      //!                 empty generates random GUIDs.
      //! \return true on success; otherwise, false.
      static bool                  writeAll( const QString& dir, QString& error, const QString& guidSeed = QString() );
};

#endif // US_SIM_INPUTS_H
