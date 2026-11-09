//! \file us_calc_validation.h
//! \brief Input checks of the calculation library.
#ifndef US_CALC_VALIDATION_H
#define US_CALC_VALIDATION_H

#include "us_calc_types.h"

#include <vector>

namespace US_Calc
{
//! \brief Diagnostics for a model before it is used.
//!
//! Errors:  no components, or a non-finite value in any component.
//! Warnings:  negative concentrations (the legacy calculations accept them;
//! a replicate group excludes members whose in-range concentration is
//! negative).
std::vector<Diagnostic> validate_model( const Model& model );

//! \brief Diagnostics for a range:  an error for non-finite bounds or
//! low > high.  A range with low == high is valid.
std::vector<Diagnostic> validate_range( const Range& range );
} // namespace US_Calc

#endif
