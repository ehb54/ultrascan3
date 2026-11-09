//! \file us_calc_constants.h
//! \brief Physical constants shared by the calculation library and the rest
//! of UltraScan.
//!
//! This is their only definition.  utils/us_constants.h includes this file,
//! so programs keep using the same macros.
#ifndef US_CALC_CONSTANTS_H
#define US_CALC_CONSTANTS_H

#define AVOGADRO 6.022140857e+23 //!< Avogadro's number
#define R_GC 8.314472e+07        //!< Ideal gas constant
#define K0 273.15                //!< 0 degrees C in Kelvin
#define K20 293.15               //!< 20 degrees C in Kelvin
#define VISC_20W 1.001600        //!< Viscosity of water at 20 degrees C
#define DENS_20W 0.998213        //!< Density of water at 20 degrees C
#define COMP_25W 4.6e-10         //!< Compressibility of water at 25 degrees C
#define WATER_MW 18.01           //!< Molecular weight of water
#define WATER_PH 7.0             //!< pH of water
#define TYPICAL_VBAR 0.7200      //!< Typical vbar for a protein
#define NORMAL_TEMP 20.0         //!< Normal temperature for an experiment
#define ODLIM_PLFAC 1.2          //!< ODlimit-exceeded plot scale factor

#endif
