//! \file us_calc_validation.cpp
#include "us_calc_validation.h"
#include "us_calc_fp.h" // first:  floating-point policy

#include <cmath>

namespace US_Calc
{
bool has_error( const QVector<Diagnostic>& diagnostics )
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

QVector<Diagnostic> validate_model( const Model& model )
{
   QVector<Diagnostic> out;
   if ( model.components.isEmpty() )
   {
      out.push_back( { Issue::EmptyModel, Severity::Error, "the model has no components" } );
      return out;
   }
   bool reported_negative = false;
   for ( int i = 0; i < model.components.size(); ++i )
   {
      const Component& c      = model.components[i];
      const double     vals[] = { c.s, c.D, c.mw, c.f, c.f_f0, c.vbar20, c.signal_concentration };
      for ( double v : vals )
      {
         if ( !std::isfinite( v ) )
         {
            out.push_back( { Issue::NonFiniteValue, Severity::Error, QString( "component %1 has a non-finite value" ).arg( i ) } );
            break;
         }
      }
      if ( c.signal_concentration < 0.0 && !reported_negative )
      {
         out.push_back(
            { Issue::NegativeConcentration, Severity::Warning, QString( "component %1 has a negative concentration" ).arg( i ) } );
         reported_negative = true;
      }
   }
   return out;
}

QVector<Diagnostic> validate_range( const Range& range )
{
   QVector<Diagnostic> out;
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
