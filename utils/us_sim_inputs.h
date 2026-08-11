//! \file us_sim_inputs.h
#ifndef US_SIM_INPUTS_H
#define US_SIM_INPUTS_H

#include "us_extern.h"
#include "us_model.h"
#include "us_buffer.h"
#include "us_simparms.h"

//! \brief Minimal, valid model/buffer/simparams defaults for bootstrapping
//! headless/CLI simulation inputs, built entirely from each class's own
//! native defaults and serializers -- no hand-authored XML, no GUI/DB access
//! required.
class US_UTIL_EXTERN US_SimInputs
{
   public:
      //! \brief A single-speed simulation parameter set. Every argument
      //! defaults to the original fixed values this function used to
      //! hardcode (45000 rpm, 2h30m run, 30 scans, matching what
      //! us_astfem_sim builds at construction time), so calling with no
      //! arguments is unchanged from before. Note: us_mwl_species_sim's own
      //! default enforces a 20-minute delay_minutes floor via qMax() that
      //! this does not -- the two GUI programs' defaults already disagree
      //! on that point.
      //! \param rpm Rotor speed. meniscus/bottom are computed from this
      //!        value (via calc_bottom()), so it must be passed in here --
      //!        overriding rotorspeed on the returned object afterward
      //!        would leave meniscus/bottom stale for the wrong speed.
      //! \param rotor_calibr Rotor calibration ID passed to setHardware().
      //!        "0" is a built-in "zero stretch correction" entry (see
      //!        US_Hardware::readRotorMap()), not a real physical rotor.
      //! \param centerpiece Centerpiece list index passed to setHardware().
      //! \param centerpiece_channel Channel index within that centerpiece,
      //!        passed to setHardware() -- unrelated to any instrument
      //!        channel label (e.g. "1A") used elsewhere in this project.
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

      //! \brief Check that centerpiece/centerpiece_channel are valid indices
      //! into the centerpiece list read from etc/abstractCenterpieces.xml
      //! (or the DB, if connected) -- the same list
      //! US_SimulationParameters::setHardware() indexes into unchecked, so
      //! callers that accept these indices from the CLI should validate
      //! them here first rather than risk an out-of-bounds access.
      //! \return Empty string if valid, otherwise a human-readable error.
      static QString validateCenterpiece( int centerpiece, int centerpiece_channel );

      //! \brief A default single-component model: one
      //! US_Model::SimulationComponent with its own library defaults
      //! (mw=50000, vbar20=typical protein, f_f0=1.25), ABSORBANCE/MANUAL.
      static US_Model model();

      //! \brief A default buffer: US_Buffer's own constructor defaults
      //! (20C water density/viscosity, compressibility 0).
      static US_Buffer buffer();

      //! \brief Write simParams()/model()/buffer() to
      //! <dir>/{sp_default.xml,model_default.xml,buffer_default.xml} using
      //! each class's native serializer.
      //! \param dir Output directory (must already exist)
      //! \return true on success
      static bool writeAll( const QString& dir );
};

#endif // US_SIM_INPUTS_H
