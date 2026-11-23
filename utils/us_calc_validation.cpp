//! \file us_calc_validation.cpp
#include "us_calc_validation.h"
#include "us_calc_fp.h" // floating-point policy

#include "us_calc_units.h"

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

QVector<Diagnostic> validate_values( const Model& model, const QVector<Quantity>& quantities )
{
   QVector<Diagnostic> out;
   if ( model.components.isEmpty() )
   {
      out.push_back( { Issue::EmptyModel, Severity::Error, "the model has no components" } );
      return out;
   }
   bool         reported_conc     = false;
   bool         reported_negative = false;
   QVector<int> reported_quantity;
   for ( int i = 0; i < model.components.size(); ++i )
   {
      const Component& c = model.components[i];
      if ( !std::isfinite( c.signal_concentration ) && !reported_conc )
      {
         out.push_back(
            { Issue::NonFiniteValue, Severity::Error, QString( "component %1 has a non-finite concentration" ).arg( i ) } );
         reported_conc = true;
      }
      for ( Quantity q : quantities )
      {
         if ( !std::isfinite( component_value( c, q ) ) && !reported_quantity.contains( static_cast<int>( q ) ) )
         {
            out.push_back( { Issue::NonFiniteValue,
                             Severity::Warning,
                             QString( "component %1 has a non-finite %2" ).arg( i ).arg( quantity_name( q ) ) } );
            reported_quantity.push_back( static_cast<int>( q ) );
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
      out.push_back( { Issue::InvalidRange, Severity::Warning, "a range bound is not finite" } );
   }
   else if ( range.low > range.high )
   {
      out.push_back( { Issue::InvalidRange, Severity::Warning, "the range has low > high and selects nothing" } );
   }
   return out;
}
} // namespace US_Calc
