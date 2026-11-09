//! \file us_calc_statistics.h
//! \brief Line fit and concentration-weighted solute statistics.
#ifndef US_CALC_STATISTICS_H
#define US_CALC_STATISTICS_H

#include "us_calc_types.h"

#include <cstddef>
#include <optional>
#include <vector>

namespace US_Calc
{
//! \brief Least-squares line through (x, y).  The calculation of
//! US_Math2::linefit, which calls it.
struct LineFit
{
      double slope       = 0.0;
      double intercept   = 0.0;
      double sigma       = 0.0; //!< sqrt( sum of squared deviations of y ) / n
      double correlation = 0.0;
      double average     = 0.0; //!< mean of y
};

//! \brief Fit a line to n points.  Like US_Math2::linefit, this does not
//! check its input:  fewer than two distinct x, or a constant y, gives
//! non-finite values.
LineFit linefit( const double* x, const double* y, int n );

//! \brief Concentration-weighted statistics of one solute attribute
//! (algorithm "solute_statistics"), as US_SoluteData::outputStats
//! (us_ga_init) calculates them; outputStats prints these values.
//!
//! Values that the legacy calculation leaves undefined (division by zero,
//! or a confidence index before the first value) are empty and explained
//! by diagnostics.
struct SoluteStatistics
{
      std::size_t             count    = 0;
      double                  minimum  = 0.0;
      double                  maximum  = 0.0;
      //! True when |(maximum - minimum) / minimum| < 1e-4.
      bool                    constant = false;
      std::optional<double>   mean; //!< sum(v*c) / sum(c)
      //! (minimum + maximum) / 2.  The desktop output labels it "Median".
      std::optional<double>   midrange;
      //! Moments with the weighted bias factor sum(c^2) / (sum(c)^2 - sum(c^2))
      std::optional<double>   variance;
      std::optional<double>   standard_deviation;
      std::optional<double>   standard_error; //!< standard deviation / sqrt(count)
      std::optional<double>   skew;
      std::optional<double>   kurtosis; //!< excess kurtosis
      //! Correlation of the values with their index (a line fit)
      std::optional<double>   correlation;
      //! Highest of 50 equal bins between minimum and maximum (low, high, center)
      std::optional<double>   mode_low;
      std::optional<double>   mode_high;
      std::optional<double>   mode_center;
      //! Sum of bin concentrations times bin width.  Every value is in
      //! exactly one bin, the last including the maximum, so this is the
      //! total concentration times the bin width.
      std::optional<double>   area;
      int                     bins = 50;
      //! Weighted quantiles:  the first value where the cumulative
      //! concentration reaches 2.5 % (0.5 %), and the value before the one
      //! where it exceeds 97.5 % (99.5 %).
      std::optional<double>   confidence95_low;
      std::optional<double>   confidence95_high;
      std::optional<double>   confidence99_low;
      std::optional<double>   confidence99_high;
      std::vector<Diagnostic> diagnostics;
      AlgorithmVersion        algorithm;
};

//! \brief Statistics of values weighted by concentrations of equal length.
SoluteStatistics solute_statistics( const std::vector<double>& values, const std::vector<double>& concentrations );
} // namespace US_Calc

#endif
