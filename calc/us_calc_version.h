//! \file us_calc_version.h
//! \brief Library and algorithm versions of the calculation library.
#ifndef US_CALC_VERSION_H
#define US_CALC_VERSION_H

#include "us_calc_types.h"

#include <QString>
#include <QVector>
#include <optional>

namespace US_Calc
{
//! \brief Version of the library's public interface (major.minor.patch).
//!
//! The major number changes when the interface changes incompatibly.  It
//! is independent of the algorithm versions, which record the rules.
QString                         library_version();

//! \brief Every algorithm of the library with its current version.
QVector<AlgorithmVersion>       algorithms();

//! \brief The current version of the named algorithm, if it exists.
std::optional<AlgorithmVersion> algorithm( const QString& name );
} // namespace US_Calc

#endif
