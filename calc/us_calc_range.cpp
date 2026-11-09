//! \file us_calc_range.cpp
#include "us_calc_range.h"
#include "us_calc_fp.h" // first:  floating-point policy

#include "us_calc_units.h"
#include "us_calc_validation.h"
#include "us_calc_version.h"

namespace US_Calc
{
namespace
{
bool range_quantity_supported( Quantity quantity )
{
   return quantity == Quantity::Sedimentation || quantity == Quantity::Diffusion || quantity == Quantity::MolecularWeight ||
          quantity == Quantity::FrictionalRatio;
}

void append( std::vector<Diagnostic>& to, const std::vector<Diagnostic>& from )
{
   to.insert( to.end(), from.begin(), from.end() );
}
} // namespace

RangeResult range_summary( const Model& model, const Range& range )
{
   RangeResult result;
   result.range           = range;
   result.component_count = model.components.size();
   result.algorithm       = *algorithm( "range_summary" );
   append( result.diagnostics, validate_model( model ) );
   append( result.diagnostics, validate_range( range ) );
   if ( !range_quantity_supported( range.quantity ) )
   {
      result.diagnostics.push_back(
         { Issue::UnsupportedQuantity, Severity::Error, quantity_name( range.quantity ) + " is not a range quantity" } );
   }
   if ( has_error( result.diagnostics ) )
   {
      return result;
   }

   // As distrib_info:  C over every component, then R and sum(x*c) over the
   // components within the range, each in component order.
   const double lo      = to_model_units( range.quantity, range.low );
   const double hi      = to_model_units( range.quantity, range.high );
   double       c_total = 0.0;
   double       r_total = 0.0;
   double       xc_sum  = 0.0;
   for ( const Component& c : model.components )
   {
      c_total += c.signal_concentration;
   }
   for ( const Component& c : model.components )
   {
      const double x = component_value( c, range.quantity );
      if ( x >= lo && x <= hi )
      {
         r_total += c.signal_concentration;
         xc_sum += x * c.signal_concentration;
         ++result.selected_count;
      }
   }
   result.total_concentration = c_total;
   result.range_concentration = r_total;

   if ( c_total == 0.0 )
   {
      result.diagnostics.push_back( { Issue::ZeroTotal, Severity::Error, "the total concentration is zero" } );
   }
   else
   {
      result.fraction_percent = ( r_total / c_total ) * 100.0;
   }
   if ( r_total == 0.0 )
   {
      result.diagnostics.push_back( { Issue::EmptyRange, Severity::Warning, "no concentration lies within the range" } );
   }
   else
   {
      result.within_range_mean = xc_sum / r_total;
   }
   return result;
}

ModelMeans model_means( const Model& model )
{
   ModelMeans result;
   result.component_count = model.components.size();
   result.algorithm       = *algorithm( "range_summary" );
   append( result.diagnostics, validate_model( model ) );
   if ( has_error( result.diagnostics ) )
   {
      return result;
   }

   // As distrib_info:  all sums in one pass in component order.
   double sum_c = 0.0, sum_s = 0.0, sum_D = 0.0, sum_mw = 0.0, sum_k = 0.0, sum_v = 0.0;
   for ( const Component& c : model.components )
   {
      const double conc = c.signal_concentration;
      sum_c += conc;
      sum_mw += c.mw * conc;
      sum_s += c.s * conc;
      sum_D += c.D * conc;
      sum_v += c.vbar20 * conc;
      sum_k += c.f_f0 * conc;
   }
   result.total_concentration = sum_c;
   if ( sum_c == 0.0 )
   {
      result.diagnostics.push_back( { Issue::ZeroTotal, Severity::Error, "the total concentration is zero" } );
      return result;
   }
   result.s      = sum_s / sum_c;
   result.D      = sum_D / sum_c;
   result.mw     = sum_mw / sum_c;
   result.f_f0   = sum_k / sum_c;
   result.vbar20 = sum_v / sum_c;
   return result;
}
} // namespace US_Calc
