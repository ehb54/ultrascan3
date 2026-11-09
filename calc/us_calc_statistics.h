//! \file us_calc_statistics.h
//! \brief Line fit and weighted statistics of one attribute.
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

//! \brief The two definitions of the weighted statistics in UltraScan.
enum class StatisticsConvention
{
   //! The values are a distribution weighted by concentration, as
   //! US_SoluteData::outputStats (us_ga_init) calculates it:  weighted
   //! moments with the bias factor sum(c^2) / (sum(c)^2 - sum(c^2)), weighted
   //! quantiles for the confidence intervals, and values within a relative
   //! 1e-4 counted as constant.
   ConcentrationDistribution,
   //! Each value is one Monte Carlo iteration, as
   //! US_DmgaMcStats::compute_statistics (us_fematch) calculates it:  only
   //! the mean is weighted; the moments are unweighted (divisor N) about
   //! that mean, the median is the median of the values, the confidence
   //! intervals are mean -/+ 1.960 and 2.576 standard deviations, and only
   //! equal values are constant.
   MonteCarloIterations
};

//! \brief Weighted statistics of one attribute (algorithm
//! "weighted_statistics").
//!
//! Values that the calculation leaves undefined (division by zero, or a
//! confidence index before the first value) are empty and explained by
//! diagnostics.
struct WeightedStatistics
{
      StatisticsConvention    convention = StatisticsConvention::ConcentrationDistribution;
      std::size_t             count      = 0;
      double                  minimum    = 0.0;
      double                  maximum    = 0.0;
      //! ConcentrationDistribution:  |(maximum - minimum) / minimum| < 1e-4.
      //! MonteCarloIterations:  minimum == maximum; the moments, the line
      //! fit, the area and the mode bounds are then 0, and the median, the
      //! mode center and the confidence intervals equal the mean.
      bool                    constant = false;
      std::optional<double>   mean;     //!< sum(v*c) / sum(c)
      std::optional<double>   midrange; //!< (minimum + maximum) / 2; outputStats prints it as "Median"
      //! The median of the values, unweighted (MonteCarloIterations only)
      std::optional<double>   median;
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
      //! ConcentrationDistribution:  the first value where the cumulative
      //! concentration reaches 2.5 % (0.5 %), and the value before the one
      //! where it exceeds 97.5 % (99.5 %).  MonteCarloIterations:  the mean
      //! -/+ 1.960 (2.576) standard deviations.
      std::optional<double>   confidence95_low;
      std::optional<double>   confidence95_high;
      std::optional<double>   confidence99_low;
      std::optional<double>   confidence99_high;
      std::vector<Diagnostic> diagnostics;
      AlgorithmVersion        algorithm;
};

//! \brief Statistics of values weighted by concentrations of equal length.
WeightedStatistics weighted_statistics( const std::vector<double>& values,
                                        const std::vector<double>& concentrations,
                                        StatisticsConvention       convention );
} // namespace US_Calc

#endif
