//! \file us_calc_version.h
//! \brief Interface and algorithm versions of the US_Calc calculations.
#ifndef US_CALC_VERSION_H
#define US_CALC_VERSION_H

#include "us_calc_types.h"

#include <QString>
#include <QVector>
#include <optional>

namespace US_Calc
{
//! \brief Version of the US_Calc interface (major.minor.patch).
//!
//! The major number changes when the interface changes incompatibly.  It
//! is independent of the algorithm versions, which record the rules.
US_UTIL_EXTERN QString library_version();

//! \brief Every US_Calc algorithm with its current version.
US_UTIL_EXTERN QVector<AlgorithmVersion> algorithms();

//! \brief The current version of the named algorithm, if it exists.
US_UTIL_EXTERN std::optional<AlgorithmVersion> algorithm( const QString& name );
} // namespace US_Calc

#endif
