//! \file us_calc_report_item.cpp
#include "us_calc_report_item.h"
#include "us_calc_fp.h" // floating-point policy

#include "us_calc_matching.h"
#include "us_calc_tolerance.h"
#include "us_calc_units.h"
#include "us_calc_version.h"

namespace US_Calc::Gmp
{
ReportItemResult evaluate_report_item( const Model& model, const ReportItem& item )
{
   ReportItemResult result;
   result.algorithm        = algorithm( "report_item" );
   const Quantity quantity = quantity_from_report_type( item.type, &result.range_type );
   result.range            = range_summary( model, { quantity, item.low, item.high } );

   // C in component order, as range_summary and model_means sum it.
   double c_total = 0.0;
   for ( const Component& c : model.components )
   {
      c_total += c.signal_concentration;
   }
   result.total_concentration = c_total;
   result.range_concentration = result.range.range_concentration;
   result.within_range_mean   = result.range.within_range_mean;
   result.fraction_percent    = result.range_type ? result.range.fraction_percent : ( 0.0 / c_total ) * 100.0;
   result.passed =
      passes( result.fraction_percent, { ToleranceRule::AbsolutePercentagePoints, item.target_percent, item.tolerance } );
   return result;
}

VelMwlItemResult evaluate_velmwl_item( const Model& model, const ReportItem& item )
{
   VelMwlItemResult result;
   result.algorithm = algorithm( "velmwl_report_item" );
   if ( model.components.isEmpty() )
   {
      result.passed =
         passes( result.fraction_percent, { ToleranceRule::AbsolutePercentagePoints, item.target_percent, item.tolerance } );
      return result;
   }
   const Quantity quantity = quantity_from_report_type( item.type, &result.supported );
   if ( !result.supported )
   {
      return result;
   }
   result.range            = range_summary( model, { quantity, item.low, item.high } );
   result.fraction_percent = ( result.range.total_concentration > 0.0 ) ? result.range.fraction_percent : 0.0;
   result.passed =
      passes( result.fraction_percent, { ToleranceRule::AbsolutePercentagePoints, item.target_percent, item.tolerance } );
   return result;
}

bool any_method_matches( const QStringList& item_methods, const QString& model_description )
{
   for ( const QString& method : item_methods )
   {
      if ( method_matches( method, model_description ) )
      {
         return true;
      }
   }
   return false;
}
} // namespace US_Calc::Gmp
