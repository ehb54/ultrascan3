//! \file us_calc_range.h
//! \brief Range sums, fractions and weighted means of a model.
//!
//! Algorithm "range_summary".  The results equal those of the GMP reporter
//! (us_reporter_gmp.cpp:  distrib_info and velmwl_range_fraction) for the
//! same model:  bounds are scaled and compared as there, and every sum runs
//! in component order with the same double operations.  Where the reporter
//! divides by zero, the library leaves the value empty and adds a
//! diagnostic.
#ifndef US_CALC_RANGE_H
#define US_CALC_RANGE_H

#include "us_calc_types.h"

#include <QVector>
#include <optional>

namespace US_Calc
{
//! \brief Concentration of a model within a range.
struct RangeResult
{
      Range                 range;
      int                   component_count     = 0;   //!< Components in the model
      int                   selected_count      = 0;   //!< Components within the range
      double                total_concentration = 0.0; //!< C:  sum of all concentrations
      double                range_concentration = 0.0; //!< R:  sum within the range
      //! 100 * R / C, computed as (R / C) * 100; empty when C is zero.
      std::optional<double> fraction_percent;
      //! sum(x * c) / R over the range, x being the range quantity in model
      //! units; empty when R is zero.
      std::optional<double> within_range_mean;
      QVector<Diagnostic>   diagnostics;
      AlgorithmVersion      algorithm;
};

//! \brief Concentration-weighted means of a whole model, in model units:
//! sum(x * c) / C for each quantity; empty when C is zero.
struct ModelMeans
{
      int                   component_count     = 0;
      double                total_concentration = 0.0;
      std::optional<double> s;
      std::optional<double> D;
      std::optional<double> mw;
      std::optional<double> f_f0;
      std::optional<double> vbar20;
      QVector<Diagnostic>   diagnostics;
      AlgorithmVersion      algorithm;
};

//! \brief The concentration of a model within a range.
//!
//! A component is within the range when low <= value <= high, the bounds
//! converted with to_model_units.  Supported quantities are s, D, MW and
//! f/f0, the reporter's range types; vbar gives an UnsupportedQuantity
//! error.  An invalid range or model gives an error and no fraction.
RangeResult range_summary( const Model& model, const Range& range );

//! \brief The concentration-weighted means of a whole model.
ModelMeans  model_means( const Model& model );
} // namespace US_Calc

#endif
