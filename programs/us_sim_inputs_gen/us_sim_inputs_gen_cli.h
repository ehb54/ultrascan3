//! \file us_sim_inputs_gen_cli.h
//! \brief Command-line definitions us_sim_inputs_gen shares with its tests.
#ifndef US_SIM_INPUTS_GEN_CLI_H
#define US_SIM_INPUTS_GEN_CLI_H

#include <QStringList>

#include "us_sim_species.h"

//! \brief Error formats us_sim_inputs_gen prints; fill them with QString::arg().
namespace US_SimInputsGenMessages
{
//! A negative --delay-hrs or --delay-mins.
constexpr const char* negativeDelay       = "--delay-hrs and --delay-mins must be zero or greater; "
                                            "omit both to use the acceleration time";
//! A --cell outside 1 to 8.
constexpr const char* badCell             = "--cell must be 1 to 8";
//! A --channel that is not a channel letter: %1 the accepted letters.
constexpr const char* badChannel          = "--channel must be one of %1";
//! A --wavelength that is not three unsigned digits.
constexpr const char* badWavelength       = "--wavelength must be a 3-digit number";
//! An unknown --component key: %1 the key, %2 the accepted keys.
constexpr const char* unknownComponentKey = "--component key \"%1\" is not one of %2";
}

//! \brief Keys accepted in a --component value.
inline const QStringList& component_keys()
{
   static const QStringList keys = []
   {
      QStringList names;
      for ( const US_SimSpecies::Coefficient& coeff : US_SimSpecies::coefficients() )
      {
         names << coeff.name;
      }
      names << "vbar20" << "conc" << "extinction" << "type" << "guid" << "name";
      return names;
   }();
   return keys;
}

#endif
