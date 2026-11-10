//! \file us_calc_smoothing.h
//! \brief Gaussian smoothing of a series (algorithm "gaussian_smoothing").
#ifndef US_CALC_SMOOTHING_H
#define US_CALC_SMOOTHING_H

#include "us_calc_types.h"

#include <QVector>

namespace US_Calc
{
//! \brief The normal probability density with standard deviation sigma and
//! the given mean, at x.  A sigma of zero gives no finite value.
US_UTIL_EXTERN double normal_distribution( double sigma, double mean, double x );

//! \brief Smooth a series with a normalized Gaussian kernel reaching two
//! standard deviations over width points on each side.
//!
//! The calculation of US_Math2::gaussian_smoothing, which calls it:  the
//! first width points use an "appearing frame", the last width points a
//! "disappearing frame", and every point is normalized by the weights used.
//! A width of 1 or less leaves the series unchanged.  A series shorter
//! than 2*width-1 points cannot be smoothed with that width:  it is left
//! unchanged and a SeriesTooShort error is returned.
US_UTIL_EXTERN QVector<Diagnostic> gaussian_smoothing( QVector<double>& values, int width );
} // namespace US_Calc

#endif
