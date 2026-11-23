//! \file us_calc_matching.h
//! \brief Which models a report item applies to (algorithm
//! "report_item_matching").
//!
//! As distrib_info in us_reporter_gmp.cpp:  a report item applies to a
//! model when every '-'-separated part of the item's method occurs in the
//! model's analysis label.  A method containing "PCSA" counts as "PCSA".
#ifndef US_CALC_MATCHING_H
#define US_CALC_MATCHING_H

#include "us_extern.h"

#include <QString>

namespace US_Calc
{
//! \brief The analysis label of a model description:
//! description.section('.', -2, -2).section('_', 1, -1), or
//! description.section('.', 0, -2) when that is empty.
US_UTIL_EXTERN QString analysis_label( const QString& description );

//! \brief True when a report item's method applies to a model.
US_UTIL_EXTERN bool    method_matches( const QString& item_method, const QString& model_description );
} // namespace US_Calc

#endif
