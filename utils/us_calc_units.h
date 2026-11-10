//! \file us_calc_units.h
//! \brief Units and quantity access of the US_Calc calculations.
#ifndef US_CALC_UNITS_H
#define US_CALC_UNITS_H

#include "us_calc_types.h"

#include <QString>
#include <optional>

namespace US_Calc
{
//! \brief The value of a quantity in model units:  value in report units
//! times the reporter's scale factor.
//!
//! The scale factors are those of the GMP reporter (us_reporter_gmp.cpp,
//! distrib_info and velmwl_range_fraction):  1e-13 for s, 1e-7 for D, 1e3
//! for MW and 1 otherwise.  The product is formed exactly as there, so a
//! component on a bound is included or excluded as before.
US_UTIL_EXTERN double to_model_units( Quantity quantity, double report_value );

//! \brief The scale factor from report units to model units.
US_UTIL_EXTERN double report_scale( Quantity quantity );

//! \brief The value of a quantity of a component, in model units.
US_UTIL_EXTERN double component_value( const Component& component, Quantity quantity );

//! \brief The quantity named by a report item type ("s", "D", "MW",
//! "f/f0"), as stored in autoflowReportItem.type.  Other types, including
//! "vbar", are not range types of the reporter and give no value.
US_UTIL_EXTERN std::optional<Quantity> quantity_from_report_type( const QString& type );

//! \brief The short name of a quantity ("s", "D", "MW", "f/f0", "vbar").
US_UTIL_EXTERN QString                 quantity_name( Quantity quantity );
} // namespace US_Calc

#endif
