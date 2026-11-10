//! \file us_calc_coefficients.h
//! \brief Hydrodynamic coefficients of a model component.
#ifndef US_CALC_COEFFICIENTS_H
#define US_CALC_COEFFICIENTS_H

#include "us_calc_types.h"

namespace US_Calc
{
//! \brief Complete the coefficients of a component from any valid pair of
//! s, D, mw, f and f/f0, at 20 C in water.
//!
//! This is the calculation of US_Model::calc_coefficients, which calls it.
//! See doc/calc_model_coefficient_documentation.pdf for the derivation.
//!
//! - A vbar20 <= 0 is replaced by TYPICAL_VBAR.
//! - Returns false, leaving the other values unchanged, when the sign of s
//!   does not match the buoyancy, or when the buoyancy is zero and the pair
//!   starts from s or mw (both divide by it).
//! - Returns false when no valid pair is present.
//! - When the frictional coefficient changes by more than 10 %, f takes the
//!   new value; if the concentration was zero it takes the old f.
US_UTIL_EXTERN bool calc_coefficients( Component& component );
} // namespace US_Calc

#endif
