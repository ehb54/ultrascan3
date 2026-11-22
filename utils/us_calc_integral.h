//! \file us_calc_integral.h
//! \brief Integral (boundary-fraction) distribution of a model (algorithm
//! "integral_distribution").
//!
//! The distribution US_Integral plots:  the model's solutes ordered by one
//! quantity, their normalized concentrations accumulated, and at each of
//! divisions + 1 boundary-fraction levels the solute where the accumulated
//! concentration first reaches the level.
//!
//! It corrects the desktop reduction (ehb54/ultrascan-tickets#1214):
//! solutes with equal s, D, mw and f/f0 are merged wherever they appear,
//! nothing else is rescaled, and the order is total, so the accumulated concentration ends at 1 and the result does not
//! depend on component order or on the C++ library.
#ifndef US_CALC_INTEGRAL_H
#define US_CALC_INTEGRAL_H

#include "us_calc_types.h"

#include <QVector>

namespace US_Calc
{
//! \brief The quantity a distribution is ordered by (its x axis).
enum class DistributionAxis
{
   Sedimentation,
   FrictionalRatio,
   MolecularWeight,
   Diffusion,
   PartialSpecificVolume,
   HydrodynamicRadius
};

//! \brief The US_Integral controls, with their defaults.
struct IntegralSettings
{
      int    divisions               = 100;   //!< levels - 1; at least 1
      double boundary_start_percent  = 0.0;   //!< first level, percent
      double boundary_extent_percent = 100.0; //!< span of the levels, percent
      int    smoothing               = 1;     //!< Gaussian width; 1 or less for none
};

//! \brief One integral distribution.  Every series has one value per level,
//! in the units US_Integral plots:  s in S, D in 1e-7 cm^2/s, mw in g/mol,
//! f/f0, vbar20 in mL/g, and the hydrodynamic radius f / (6e-9 pi eta20w).
struct IntegralDistribution
{
      QVector<double>     boundary_fractions; //!< the levels
      QVector<double>     s;
      QVector<double>     D;
      QVector<double>     mw;
      QVector<double>     f_f0;
      QVector<double>     vbar20;
      QVector<double>     rh;
      QVector<Diagnostic> diagnostics;
      AlgorithmVersion    algorithm;
};

//! \brief The boundary fractions of the levels:  divisions + 1 values from
//! the start percent in steps of extent / divisions, accumulated as
//! US_Integral does.  Empty when divisions < 1.  They depend only on the
//! settings, so they are the same for every model.
US_UTIL_EXTERN QVector<double>      boundary_levels( const IntegralSettings& settings );

//! \brief The integral distribution of a model along an axis.
//!
//! The coefficients of every component are completed with
//! calc_coefficients.  (US_Model::update_coefficients, which US_Integral
//! used, stops at the first component it cannot complete and leaves the
//! later ones as stored.)  A model without components, with a non-finite
//! value or with a zero total concentration is an error and gives empty
//! series.  A smoothing width the levels cannot support is also an error;
//! the series are then left unsmoothed.
US_UTIL_EXTERN IntegralDistribution integral_distribution( const Model&            model,
                                                           DistributionAxis        axis,
                                                           const IntegralSettings& settings = {} );
} // namespace US_Calc

#endif
