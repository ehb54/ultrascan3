//! \file us_calc_validation.h
//! \brief Input checks of the US_Calc calculations.
#ifndef US_CALC_VALIDATION_H
#define US_CALC_VALIDATION_H

#include "us_calc_types.h"

#include <QVector>

namespace US_Calc
{
//! \brief Diagnostics for a model before it is used.
//!
//! Errors:  no components, or a non-finite value in any component.
//! Warnings:  negative concentrations (the legacy calculations accept them;
//! a replicate group excludes members whose in-range concentration is
//! negative).
US_UTIL_EXTERN QVector<Diagnostic> validate_model( const Model& model );

//! \brief Diagnostics for the values a calculation uses.
//!
//! Checks only the signal concentration and the given quantities, so a
//! non-finite value in a field the calculation does not use changes
//! nothing.  Errors:  no components, or a non-finite concentration (the
//! total is then undefined).  Warnings:  a non-finite value of one of the
//! quantities (that component's term is not finite, or it is not
//! selected), and negative concentrations.  Each kind is reported once.
US_UTIL_EXTERN QVector<Diagnostic> validate_values( const Model& model, const QVector<Quantity>& quantities );

//! \brief Diagnostics for a range, all warnings:  low > high (the range
//! selects nothing), or a non-finite bound (a NaN bound selects nothing;
//! an infinite bound leaves that side open).  A range with low == high is
//! valid.  The legacy calculations accepted such ranges, and the range
//! calculations still give the legacy results for them.
US_UTIL_EXTERN QVector<Diagnostic> validate_range( const Range& range );
} // namespace US_Calc

#endif
