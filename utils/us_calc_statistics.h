//! \file us_calc_statistics.h
//! \brief Line fit and weighted statistics of one attribute.
#ifndef US_CALC_STATISTICS_H
#define US_CALC_STATISTICS_H

#include "us_calc_types.h"

#include <QVector>
#include <QtNumeric>

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
US_UTIL_EXTERN LineFit linefit( const double* x, const double* y, int n );

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
//! confidence index before the first value) are NaN and explained by
//! diagnostics.
struct WeightedStatistics
{
      StatisticsConvention convention         = StatisticsConvention::ConcentrationDistribution;
      int                  count              = 0;
      double               minimum            = 0.0;
      double               maximum            = 0.0;
      //! ConcentrationDistribution:  |(maximum - minimum) / minimum| < 1e-4.
      //! MonteCarloIterations:  minimum == maximum; the moments, the line
      //! fit, the area and the mode bounds are then 0, and the median, the
      //! mode center and the confidence intervals equal the mean.
      bool                 constant           = false;
      double               mean               = qQNaN(); //!< sum(v*c) / sum(c)
      double               midrange           = qQNaN(); //!< (minimum + maximum) / 2; outputStats prints it as "Median"
      //! The median of the values, unweighted (MonteCarloIterations only)
      double               median             = qQNaN();
      double               variance           = qQNaN();
      double               standard_deviation = qQNaN();
      double               standard_error     = qQNaN(); //!< standard deviation / sqrt(count)
      double               skew               = qQNaN();
      double               kurtosis           = qQNaN(); //!< excess kurtosis
      //! Correlation of the values with their index (a line fit)
      double               correlation        = qQNaN();
      //! Highest of 50 equal bins between minimum and maximum (low, high, center)
      double               mode_low           = qQNaN();
      double               mode_high          = qQNaN();
      double               mode_center        = qQNaN();
      //! Sum of bin concentrations times bin width.  Every value is in
      //! exactly one bin, the last including the maximum, so this is the
      //! total concentration times the bin width.
      double               area               = qQNaN();
      int                  bins               = 50;
      //! ConcentrationDistribution:  with the values in ascending order (equal
      //! values by ascending concentration), the first value where the
      //! cumulative concentration reaches 2.5 % (0.5 %), and the value before
      //! the one where it exceeds 97.5 % (99.5 %).  MonteCarloIterations:  the mean
      //! -/+ 1.960 (2.576) standard deviations.
      double               confidence95_low   = qQNaN();
      double               confidence95_high  = qQNaN();
      double               confidence99_low   = qQNaN();
      double               confidence99_high  = qQNaN();
      QVector<Diagnostic>  diagnostics;
      AlgorithmVersion     algorithm;
};

//! \brief Statistics of values weighted by concentrations of equal length.
US_UTIL_EXTERN WeightedStatistics weighted_statistics( const QVector<double>& values,
                                                       const QVector<double>& concentrations,
                                                       StatisticsConvention   convention );
} // namespace US_Calc

#endif
