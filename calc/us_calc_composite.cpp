//! \file us_calc_composite.cpp
#include "us_calc_composite.h"
#include "us_calc_fp.h" // first:  floating-point policy

#include "us_calc_matching.h"
#include "us_calc_version.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace US_Calc
{
namespace
{
// US_Model::constant_vbar and constant_ff0:  the range of a value over the
// components is below a limit.
bool constant( const std::vector<Component>& comps, double Component::*value, double limit )
{
   double valmin = comps[0].*value;
   double valmax = comps[0].*value;
   for ( std::size_t ii = 1; ii < comps.size(); ii++ )
   {
      // qMin( valmin, v ) and qMax( valmax, v ), including their NaN results
      const double v = comps[ii].*value;
      valmin         = ( valmin < v ) ? valmin : v;
      valmax         = ( valmax < v ) ? v : valmax;
   }
   return ( valmax - valmin ) < limit;
}
} // namespace

namespace
{
// printf "%<width>.<decimals>f" as QString::asprintf writes it:  exact
// ties round away from zero, where the C library rounds them to even.  The
// value's exact decimal expansion is rounded by hand.
std::string fixed_like_qt( double value, int width, int decimals )
{
   char exact[1200];
   std::snprintf( exact, sizeof exact, "%.1100f", std::fabs( value ) );
   std::string                  digits( exact );
   const std::string::size_type point  = digits.find( '.' );
   std::string                  whole  = digits.substr( 0, point );
   std::string                  frac   = digits.substr( point + 1, decimals );
   const bool                   up     = digits.size() > point + 1 + decimals && digits[point + 1 + decimals] >= '5';
   std::string                  number = whole + frac;
   if ( up )
   {
      int i = static_cast<int>( number.size() ) - 1;
      for ( ; i >= 0 && number[i] == '9'; --i )
      {
         number[i] = '0';
      }
      if ( i < 0 )
      {
         number.insert( number.begin(), '1' );
      }
      else
      {
         number[i]++;
      }
   }
   std::string out = number.substr( 0, number.size() - decimals );
   if ( decimals > 0 )
   {
      out += "." + number.substr( number.size() - decimals );
   }
   if ( std::signbit( value ) )
   {
      out.insert( out.begin(), '-' );
   }
   if ( static_cast<int>( out.size() ) < width )
   {
      out.insert( out.begin(), width - out.size(), ' ' );
   }
   return out;
}
} // namespace

std::string mc_solute_key( double s, double second )
{
   return fixed_like_qt( s * 1.0e+13, 10, 4 ) + " " + fixed_like_qt( second, 8, 5 );
}

CompositeResult mc_composite( const std::vector<Model>& iterations )
{
   CompositeResult result;
   result.algorithm         = *algorithm( "mc_composite" );
   result.model.monte_carlo = true;
   if ( iterations.empty() )
   {
      result.diagnostics.push_back( { Issue::EmptyModel, Severity::Error, "there are no Monte Carlo iterations" } );
      return result;
   }

   int                    ncnstv = 0;
   int                    ncnstk = 0;
   std::vector<Component> all;
   for ( std::size_t ii = 0; ii < iterations.size(); ii++ )
   {
      const std::vector<Component>& comps = iterations[ii].components;
      if ( comps.empty() )
      {
         result.diagnostics.push_back(
            { Issue::EmptyModel, Severity::Error, "iteration " + std::to_string( ii + 1 ) + " has no components" } );
         return result;
      }
      if ( constant( comps, &Component::vbar20, 1.0e-4 ) )
      {
         ncnstv++;
      }
      if ( constant( comps, &Component::f_f0, 1.0e-3 ) )
      {
         ncnstk++;
      }
      all.insert( all.end(), comps.begin(), comps.end() );
   }

   const double             sclnrm  = 1.0 / (double) iterations.size();
   const bool               cnst_vb = ( ncnstv >= ncnstk );
   std::vector<std::string> sklist; // key of every component
   std::vector<std::string> skvals; // unique keys
   for ( const Component& c : all )
   {
      std::string key = mc_solute_key( c.s, cnst_vb ? c.f_f0 : c.vbar20 );
      if ( std::find( skvals.begin(), skvals.end(), key ) == skvals.end() )
      {
         skvals.push_back( key );
      }
      sklist.push_back( std::move( key ) );
   }
   std::sort( skvals.begin(), skvals.end() );

   for ( const std::string& key : skvals )
   {
      double      conc = 0.0;
      std::size_t last = 0;
      for ( std::size_t jj = 0; jj < sklist.size(); jj++ )
      {
         if ( sklist[jj] == key )
         {
            last = jj;
            conc += all[jj].signal_concentration;
         }
      }
      Component merged            = all[last];
      merged.signal_concentration = conc * sclnrm;
      result.model.components.push_back( merged );
      result.source_index.push_back( last );
   }

   const std::string& first = iterations.front().description;
   char               miter[32];
   std::snprintf( miter, sizeof miter, "_mcN%03d", static_cast<int>( iterations.size() ) );
   result.model.description = text_section( first, '.', 0, -3 ) + "." +
                              text_section( text_section( first, '.', -2, -2 ), '_', 0, -2 ) + miter + "." +
                              text_section( first, '.', -1, -1 );
   return result;
}
} // namespace US_Calc
