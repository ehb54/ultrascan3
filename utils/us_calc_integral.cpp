//! \file us_calc_integral.cpp
#include "us_calc_integral.h"
#include "us_calc_fp.h" // first:  floating-point policy

#include "us_calc_coefficients.h"
#include "us_calc_smoothing.h"
#include "us_calc_validation.h"
#include "us_calc_version.h"
#include "us_constants.h"

#include <QtMath>

#include <algorithm>
#include <map>
#include <tuple>

namespace US_Calc
{
namespace
{
// A solute in the units US_Integral uses (S_Solute):  s in S, d in 1e-7
// cm^2/s, w molar mass, k f/f0, v vbar20, r hydrodynamic radius, c
// concentration, f boundary fraction once accumulated.
struct Solute
{
      double s = 0, k = 0, c = 0, w = 0, v = 0, d = 0, f = 0, r = 0;
};

double axis_value( const Solute& x, DistributionAxis axis )
{
   switch ( axis )
   {
      case DistributionAxis::Sedimentation:
         return x.s;
      case DistributionAxis::FrictionalRatio:
         return x.k;
      case DistributionAxis::MolecularWeight:
         return x.w;
      case DistributionAxis::Diffusion:
         return x.d;
      case DistributionAxis::PartialSpecificVolume:
         return x.v;
      case DistributionAxis::HydrodynamicRadius:
         return x.r;
   }
   return x.s;
}
} // namespace

IntegralDistribution integral_distribution( const Model& model, DistributionAxis axis, const IntegralSettings& settings )
{
   IntegralDistribution out;
   out.algorithm   = *algorithm( "integral_distribution" );
   out.diagnostics = validate_model( model );
   if ( settings.divisions < 1 )
   {
      out.diagnostics.push_back( { Issue::InvalidRange, Severity::Error, "the division count must be at least 1" } );
   }
   if ( has_error( out.diagnostics ) )
   {
      return out;
   }

   // Complete the coefficients and convert, as US_Integral::load_distro.
   QVector<Solute> in;
   bool            complete = true;
   for ( Component c : model.components )
   {
      complete = calc_coefficients( c ) && complete;
      Solute x;
      x.s = c.s * 1.0e13;
      x.k = c.f_f0;
      x.c = c.signal_concentration;
      x.w = c.mw;
      x.v = c.vbar20;
      x.d = c.D * 1.0e7;
      x.f = c.f;
      x.r = c.f / ( 6e-9 * M_PI * VISC_20W );
      in.push_back( x );
   }
   if ( !complete )
   {
      out.diagnostics.push_back(
         { Issue::NonFiniteValue, Severity::Warning, "coefficients of some components could not be completed (no valid pair)" } );
   }

   // Merge solutes with equal s, D, mw and f/f0 wherever they appear:
   // concentrations add; vbar and radius are averaged.  First appearance
   // keeps its position for the stable sort below.
   QVector<Solute>                                              merged;
   QVector<int>                                                 count;
   std::map<std::tuple<double, double, double, double>, size_t> index;
   for ( const Solute& x : in )
   {
      const auto key = std::make_tuple( x.s, x.d, x.w, x.k );
      auto       it  = index.find( key );
      if ( it == index.end() )
      {
         index[key] = merged.size();
         merged.push_back( x );
         count.push_back( 1 );
      }
      else
      {
         Solute& m = merged[it->second];
         m.c += x.c;
         m.v += x.v;
         m.r += x.r;
         ++count[it->second];
      }
   }
   for ( size_t ii = 0; ii < merged.size(); ii++ )
   {
      merged[ii].v /= count[ii];
      merged[ii].r /= count[ii];
   }

   // A total order:  the axis value, then the remaining quantities.
   std::stable_sort( merged.begin(),
                     merged.end(),
                     [axis]( const Solute& a, const Solute& b )
                     {
                        return std::make_tuple( axis_value( a, axis ), a.s, a.d, a.w, a.k, a.v, a.r ) <
                               std::make_tuple( axis_value( b, axis ), b.s, b.d, b.w, b.k, b.v, b.r );
                     } );

   // The total in input order, as US_Integral::load_distro sums it.
   double total = 0.0;
   for ( const Solute& x : in )
   {
      total += x.c;
   }
   if ( total == 0.0 )
   {
      out.diagnostics.push_back( { Issue::ZeroTotal, Severity::Error, "the total concentration is zero" } );
      return out;
   }

   // Normalize and accumulate (the boundary distribution).
   double sum_co = 0.0;
   for ( Solute& x : merged )
   {
      x.c /= total;
      sum_co += x.c;
      x.f = sum_co;
   }

   // Select the solute at each level, as US_Integral::build_bf_distro.
   double    bfrac  = settings.boundary_start_percent * 0.01;
   double    bfextn = settings.boundary_extent_percent * 0.01;
   const int ksolbf = settings.divisions;
   const int nsolbf = ksolbf + 1;
   double    bfincr = bfextn / (double) ksolbf;
   const int nsolbo = static_cast<int>( merged.size() );
   int       j2     = 0;
   for ( int kk = 0; kk < nsolbf; kk++ )
   {
      while ( j2 < nsolbo )
      {
         if ( bfrac <= merged[j2].f || j2 == ( nsolbo - 1 ) )
         {
            break;
         }
         j2++;
      }
      const Solute& x = merged[j2];
      out.boundary_fractions.push_back( bfrac );
      out.s.push_back( x.s );
      out.D.push_back( x.d );
      out.mw.push_back( x.w );
      out.f_f0.push_back( x.k );
      out.vbar20.push_back( x.v );
      out.rh.push_back( x.r );
      bfrac += bfincr;
   }

   if ( settings.smoothing > 1 )
   {
      for ( QVector<double>* series : { &out.s, &out.D, &out.mw, &out.f_f0, &out.vbar20, &out.rh } )
      {
         QVector<Diagnostic> d = gaussian_smoothing( *series, settings.smoothing );
         if ( !d.isEmpty() )
         {
            out.diagnostics << d;
            break; // every series has the same length
         }
      }
   }
   return out;
}
} // namespace US_Calc
