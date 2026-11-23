//! \file us_calc_report_item.h
//! \brief A GMP report item evaluated on a model (algorithm "report_item").
//!
//! As the Integration Results table of the GMP reporter (distrib_info in
//! us_reporter_gmp.cpp):  the item's type picks the quantity, the range
//! result comes from range_summary, and the fraction passes when it is
//! within the tolerance of the target in percentage points.  A type that is
//! not a range type selects nothing and gives the fraction (0 / C) * 100.
#ifndef US_CALC_REPORT_ITEM_H
#define US_CALC_REPORT_ITEM_H

#include "us_calc_range.h"

#include <QString>
#include <QStringList>
#include <QtNumeric>

//! GMP report rules
namespace US_Calc::Gmp
{
//! \brief A report item as autoflowReportItem stores it.
struct ReportItem
{
      QString type;                 //!< "s", "D", "MW", "f/f0"; other types are not range types
      QString method;               //!< e.g. "2DSA-IT"; see method_matches
      double  low            = 0.0; //!< Range bounds in report units
      double  high           = 0.0;
      double  target_percent = 0.0; //!< Target fraction of the total, %
      double  tolerance      = 0.0; //!< Percentage points either side of the target
};

//! \brief A report item's result on one model.
struct ReportItemResult
{
      bool             range_type          = false; //!< The type is a range type
      double           total_concentration = 0.0;   //!< C over every component
      //! R:  the in-range concentration; 0 for a type that is not a range type.
      double           range_concentration = 0.0;
      //! (R / C) * 100; (0 / C) * 100 for a type that is not a range type.
      //! Not finite when C is zero, as the reporter divides.
      double           fraction_percent    = qQNaN();
      //! As RangeResult::within_range_mean; NaN for a type that is not a
      //! range type.
      double           within_range_mean   = qQNaN();
      //! |fraction - target| <= tolerance (ToleranceRule::AbsolutePercentagePoints).
      bool             passed              = false;
      RangeResult      range; //!< The range result, with its diagnostics
      AlgorithmVersion algorithm;
};

//! \brief Evaluates a report item on a model.
US_UTIL_EXTERN ReportItemResult evaluate_report_item( const Model& model, const ReportItem& item );

//! \brief A report item's result on one species model of a VELOCITY-MWL
//! channel (algorithm "velmwl_report_item").
struct VelMwlItemResult
{
      //! False for a type that is not a range type:  the reporter shows n/a
      //! and no pass/fail.  A model without components is never examined, so
      //! its result is supported whatever the type.
      bool             supported        = true;
      //! (R / C) * 100 when C > 0, otherwise 0; 0 for a model without
      //! components and for an unsupported type.
      double           fraction_percent = 0.0;
      //! |fraction - target| <= tolerance; not meaningful when unsupported.
      bool             passed           = false;
      RangeResult      range; //!< The range result (empty for a model without components or an unsupported type)
      AlgorithmVersion algorithm;
};

//! \brief Evaluates a report item on a VELOCITY-MWL species model, as
//! distrib_info_velmwl:  unlike evaluate_report_item, a total that is not
//! positive gives 0 rather than the undivided fraction.
US_UTIL_EXTERN VelMwlItemResult evaluate_velmwl_item( const Model& model, const ReportItem& item );

//! \brief True when any of the methods applies to the model (method_matches):
//! the reporter shows a model's Integration Results table only then.
US_UTIL_EXTERN bool             any_method_matches( const QStringList& item_methods, const QString& model_description );
} // namespace US_Calc::Gmp

#endif
