//! \file us_calc_range.h
//! \brief Range sums, fractions and weighted means of a model.
//!
//! Algorithm "range_summary".  The results equal those of the GMP reporter
//! (us_reporter_gmp.cpp:  distrib_info and distrib_info_velmwl) for the
//! same model:  bounds are scaled and compared as there, and every sum runs
//! in component order with the same double operations.  The divisions are
//! not guarded either:  where the reporter divides by zero the result is
//! the same NaN or infinity, with a diagnostic.  An inverted range or a NaN
//! bound selects nothing, and a non-finite value affects only the sums it
//! enters, as in the reporter; both are reported as warnings.
#ifndef US_CALC_RANGE_H
#define US_CALC_RANGE_H

#include "us_calc_types.h"

#include <QVector>
#include <QtNumeric>

namespace US_Calc
{
//! \brief Concentration of a model within a range.
struct RangeResult
{
      Range               range;
      int                 component_count     = 0;   //!< Components in the model
      int                 selected_count      = 0;   //!< Components within the range
      double              total_concentration = 0.0; //!< C:  sum of all concentrations
      double              range_concentration = 0.0; //!< R:  sum within the range
      //! 100 * R / C, computed as (R / C) * 100; not finite when C is zero.
      double              fraction_percent    = qQNaN();
      //! sum(x * c) / R over the range, x being the range quantity in model
      //! units; not finite when R is zero.
      double              within_range_mean   = qQNaN();
      QVector<Diagnostic> diagnostics;
      AlgorithmVersion    algorithm;
};

//! \brief Concentration-weighted means of a whole model, in model units:
//! sum(x * c) / C for each quantity; not finite when C is zero.
struct ModelMeans
{
      int                 component_count     = 0;
      double              total_concentration = 0.0;
      double              s                   = qQNaN();
      double              D                   = qQNaN();
      double              mw                  = qQNaN();
      double              f_f0                = qQNaN();
      double              vbar20              = qQNaN();
      QVector<Diagnostic> diagnostics;
      AlgorithmVersion    algorithm;
};

//! \brief The concentration of a model within a range.
//!
//! A component is within the range when low <= value <= high, the bounds
//! converted with to_model_units.  Supported quantities are s, D, MW and
//! f/f0, the reporter's range types; vbar gives an UnsupportedQuantity
//! error and no result.  Any other input is calculated as the reporter
//! calculates it (see the file description).
US_UTIL_EXTERN RangeResult range_summary( const Model& model, const Range& range );

//! \brief The concentration-weighted means of a whole model.
US_UTIL_EXTERN ModelMeans  model_means( const Model& model );
} // namespace US_Calc

#endif
