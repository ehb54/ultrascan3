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
      //! \brief A default single-speed, 30-scan simulation parameter set
      //! (45000 rpm, 2h30m run), matching what us_astfem_sim builds at
      //! construction time. Note: us_mwl_species_sim's own default enforces
      //! a 20-minute delay_minutes floor via qMax() that this does not --
      //! the two GUI programs' defaults already disagree on that point.
      static US_SimulationParameters simParams();

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
