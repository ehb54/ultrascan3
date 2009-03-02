//! \file us_constants.h
#ifndef US_CONSTANTS_H
#define US_CONSTANTS_H

#include <QtCore>

#include "us_extern.h"

#define AVOGADRO 6.022045e+23 //!< Avogadro's number
#define R        8.314472e+07 //!< Ideal gas constant
#define K0       273.15       //!< 0 degrees C in Kelvin
#define K20      293.15       //!< 20 degrees C in Kelvin
#define VISC_20W 0.0100194    //!< Viscosity of water at 20 degrees C
#define DENS_20W 0.998234     //!< Density of water at 20 degrees C

namespace US_ConstantModels
{
   //! \brief The predefined simulation model types
   enum models 
   {
      Ideal1Comp, 
      Ideal2Comp, 
      Ideal3Comp, 
      Fixed, 
      MonoDimer, 
      MonoTrimer, 
      MonoTetramer,
      MonoPentamer, 
      MonoHexamer, 
      MonoHeptamer, 
      UserMonoNmer, 
      MonoDiTrimer, 
      MonoDiTetramer, 
      UserMonoNmerNmer, 
      TwoComponent,
      UserHetero, 
      UserMonoIncompMono, 
      UserMonoIncompNmer, 
      UserIrreversible,
      UserMonoPlusContaminant
   };
}

//! \brief Fetch preset values.  All functions are static.
class US_EXTERN US_Constants
{
  public:
     
   //! \brief Get defined model strings
   static QStringList modelStrings( void );
};
#endif
