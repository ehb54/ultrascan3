//! \file us_sim_inputs.h
#ifndef US_SIM_INPUTS_H
#define US_SIM_INPUTS_H

#include "us_extern.h"
#include "us_model.h"
#include "us_buffer.h"
#include "us_simparms.h"

//! \brief Generate minimal, valid inputs for headless simulations.
//! Uses each input class's defaults and serializer, without hand-written XML,
//! GUI interaction, or database access.
class US_UTIL_EXTERN US_SimInputs
{
   public:
      //! \brief Create parameters for a single-speed simulation.
      //! Defaults to 45,000 rpm for 2 hours 30 minutes with 30 scans, matching
      //! the values constructed by us_astfem_sim. Unlike us_mwl_species_sim,
      //! this function does not enforce a 20-minute minimum delay.
      //! \param rpm Rotor speed used to calculate the meniscus and bottom
      //!        positions. Changing rotorspeed on the returned object does not
      //!        recalculate those positions.
      //! \param rotor_calibr Rotor calibration ID passed to setHardware().
      //!        "0" is the built-in zero-stretch-correction entry (see
      //!        US_Hardware::readRotorMap()), not a real physical rotor.
      //! \param centerpiece Centerpiece list index passed to setHardware().
      //! \param centerpiece_channel Channel index within that centerpiece,
      //!        passed to setHardware(). It is unrelated to instrument channel
      //!        labels such as "1A".
      static US_SimulationParameters simParams(
         double    rpm                  = 45000.0,
         int       duration_hours       = 2,
         double    duration_minutes     = 30.0,
         int       scans                = 30,
         double    acceleration         = 400.0,
         int       simpoints             = 200,
         double    radial_resolution    = 0.001,
         US_SimulationParameters::MeshType meshType = US_SimulationParameters::ASTFEM,
         US_SimulationParameters::GridType gridType = US_SimulationParameters::MOVING,
         double    rnoise               = 0.0,
         double    lrnoise              = 0.0,
         double    tinoise              = 0.0,
         double    rinoise              = 0.0,
         double    baseline             = 0.0,
         bool      band_forming         = false,
         double    band_volume          = 0.015,
         QString   rotor_calibr         = "0",
         int       centerpiece          = 0,
         int       centerpiece_channel  = 0
      );

      //! \brief Validate centerpiece and channel indices.
      //! Checks the centerpiece list loaded from etc/abstractCenterpieces.xml
      //! or the database before setHardware() accesses it.
      //! \return An empty string if valid; otherwise, an error message.
      static QString validateCenterpiece( int centerpiece, int centerpiece_channel );

      //! \brief Create a default single-component absorbance model.
      //! Uses a molecular weight of 50,000, the typical protein vbar20, a
      //! frictional ratio of 1.25, and manual analysis.
      static US_Model model();

      //! \brief Create a default buffer using water properties at 20 C.
      static US_Buffer buffer();

      //! \brief Write the default simulation parameters, model, and buffer.
      //! Creates sp_default.xml, model_default.xml, and buffer_default.xml
      //! using each class's serializer.
      //! \param dir Existing output directory.
      //! \return true on success; otherwise, false.
      static bool writeAll( const QString& dir );
};

#endif // US_SIM_INPUTS_H
