//! \file us_calc_validation.cpp
#include "us_calc_validation.h"
#include "us_calc_fp.h" // first:  floating-point policy

#include <cmath>

namespace US_Calc
{
bool has_error( const std::vector<Diagnostic>& diagnostics )
{
   for ( const Diagnostic& d : diagnostics )
   {
      if ( d.severity == Severity::Error )
      {
         return true;
      }
   }
   return false;
}

std::vector<Diagnostic> validate_model( const Model& model )
{
   std::vector<Diagnostic> out;
   if ( model.components.empty() )
   {
      out.push_back( { Issue::EmptyModel, Severity::Error, "the model has no components" } );
      return out;
   }
   bool reported_negative = false;
   for ( std::size_t i = 0; i < model.components.size(); ++i )
   {
      const Component& c      = model.components[i];
      const double     vals[] = { c.s, c.D, c.mw, c.f, c.f_f0, c.vbar20, c.signal_concentration };
      for ( double v : vals )
      {
         if ( !std::isfinite( v ) )
         {
            out.push_back(
               { Issue::NonFiniteValue, Severity::Error, "component " + std::to_string( i ) + " has a non-finite value" } );
            break;
         }
      }
      if ( c.signal_concentration < 0.0 && !reported_negative )
      {
         out.push_back( { Issue::NegativeConcentration,
                          Severity::Warning,
                          "component " + std::to_string( i ) + " has a negative concentration" } );
         reported_negative = true;
      }
   }
   return out;
}

std::vector<Diagnostic> validate_range( const Range& range )
{
   std::vector<Diagnostic> out;
   if ( !std::isfinite( range.low ) || !std::isfinite( range.high ) )
   {
      out.push_back( { Issue::InvalidRange, Severity::Error, "a range bound is not finite" } );
   }
   else if ( range.low > range.high )
   {
      out.push_back( { Issue::InvalidRange, Severity::Error, "the range has low > high" } );
   }
   return out;
}
} // namespace US_Calc
