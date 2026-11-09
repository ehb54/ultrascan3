//! \file us_calc_matching.h
//! \brief Which models a report item applies to (algorithm
//! "report_item_matching").
//!
//! As distrib_info in us_reporter_gmp.cpp:  a report item applies to a
//! model when every '-'-separated part of the item's method occurs in the
//! model's analysis label.  A method containing "PCSA" counts as "PCSA".
#ifndef US_CALC_MATCHING_H
#define US_CALC_MATCHING_H

#include <string>

namespace US_Calc
{
//! \brief QString::section( separator, start, end ) for a plain separator,
//! with negative positions counted from the end.
std::string text_section( const std::string& text, char separator, int start, int end );

//! \brief The analysis label of a model description:
//! description.section('.', -2, -2).section('_', 1, -1), or
//! description.section('.', 0, -2) when that is empty.
std::string analysis_label( const std::string& description );

//! \brief True when a report item's method applies to a model.
bool        method_matches( const std::string& item_method, const std::string& model_description );
} // namespace US_Calc

#endif
