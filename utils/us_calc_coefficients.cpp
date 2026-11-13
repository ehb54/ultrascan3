//! \file us_calc_coefficients.cpp
#include "us_calc_coefficients.h"
#include "us_calc_fp.h" // floating-point policy

#include "us_constants.h"

#include <QtMath>

namespace US_Calc
{
// Ideal gas constant times 20 degrees C in Kelvin
constexpr double kRT      = R_GC * K20;
// Factors of the sphere volume and radius:  4/3 pi r^3 = volume, f0 = 6 pi eta r
constexpr double vol_fac  = 0.75 / M_PI;
constexpr double onethird = 1.0 / 3.0;
constexpr double rsph_fac = 0.06 * M_PI * VISC_20W;

bool             calc_coefficients( Component& component )
{
   bool   ok = true;

   // A vbar we can use:  a missing vbar20 becomes the typical vbar.
   double vbar;
   if ( component.vbar20 <= 0.0 )
   {
      vbar             = TYPICAL_VBAR;
      component.vbar20 = TYPICAL_VBAR;
   }
   else
   {
      vbar = component.vbar20;
   }

   double       s    = component.s; // component coefficient values
   double       D    = component.D;
   double       mw   = component.mw;
   double       f    = component.f;
   double       f_f0 = component.f_f0;
   const double c    = component.signal_concentration;
   double       fv   = f; // frictional coefficient (working value)
   double       f0;       // f-zero
   double       volume;
   double       radius_sphere;

   // s must have the sign of the buoyancy.  The pairs that start from s or
   // from mw divide by the buoyancy, so with a zero buoyancy (vbar20 equal
   // to 1 / DENS_20W) they cannot be completed; D or f with f/f0 still give
   // s = 0.  Sign tests rather than s * buoyancy > 0, which can underflow.
   const double buoyancyb = 1.0 - ( vbar * DENS_20W );

   if ( ( s < 0.0 && buoyancyb > 0.0 ) || ( s > 0.0 && buoyancyb < 0.0 ) )
   {
      return false;
   }
   if ( buoyancyb == 0.0 && ( s != 0.0 || component.mw != 0.0 ) )
   {
      return false;
   }

   // Start with s if possible
   if ( s != 0.0 )
   {
      const double s20w = s;

      if ( f_f0 != 0.0 ) // s and f_f0
      {
         // calc_model_coefficient_documentation.pdf, equation 14, with
         // adjustments for units in poise.
         double numer = 0.02 * vbar * s * VISC_20W * f_f0;
         f0           = 0.09 * VISC_20W * M_PI * sqrt( numer / buoyancyb );
         fv           = f_f0 * f0;
         D            = kRT / ( AVOGADRO * fv );
         mw           = s * kRT / ( D * buoyancyb );
      }

      else if ( D != 0.0 ) // s and D
      {
         mw            = s * kRT / ( D * buoyancyb );
         volume        = vbar * mw / AVOGADRO;
         radius_sphere = pow( volume * vol_fac, onethird );
         f0            = radius_sphere * rsph_fac;
         fv            = mw * buoyancyb / ( s20w * AVOGADRO );
         double ff0sv  = f_f0;
         f_f0          = fv / f0;
         double ffdif  = qAbs( ff0sv - f_f0 );
         f_f0          = ( ffdif < 1.e-5 ) ? ff0sv : f_f0;
      }

      else if ( mw != 0.0 ) // s and mw
      {
         D             = s * kRT / ( buoyancyb * mw );
         fv            = mw * buoyancyb / ( s20w * AVOGADRO );
         volume        = vbar * mw / AVOGADRO;
         radius_sphere = pow( volume * vol_fac, onethird );
         f0            = radius_sphere * rsph_fac;
         f_f0          = fv / f0;
      }

      else if ( f != 0.0 ) // s and f
      {
         D             = kRT / ( AVOGADRO * fv );
         mw            = s * kRT / ( D * buoyancyb );
         volume        = vbar * mw / AVOGADRO;
         radius_sphere = pow( volume * vol_fac, onethird );
         f0            = radius_sphere * rsph_fac;
         f_f0          = fv / f0;
      }

      else // do not have 2 valid coeffs
      {
         ok = false;
      }
   }

   else if ( component.mw != 0.0 )
   {
      volume        = vbar * mw / AVOGADRO;
      radius_sphere = pow( volume * vol_fac, onethird );
      f0            = radius_sphere * rsph_fac;

      if ( D != 0.0 ) // mw and D
      {
         s    = D * buoyancyb * mw / kRT;
         fv   = mw * buoyancyb / ( s * AVOGADRO );
         f_f0 = fv / f0;
      }

      else if ( f_f0 != 0.0 ) // mw and f_f0
      {
         fv = f_f0 * f0;
         s  = mw * buoyancyb / ( AVOGADRO * fv );
         D  = s * kRT / ( buoyancyb * mw );
      }

      else if ( f != 0.0 ) // mw and f
      {
         f_f0 = fv / f0;
         s    = mw * buoyancyb / ( AVOGADRO * fv );
         D    = s * kRT / ( buoyancyb * mw );
      }

      else // do not have 2 valid coeffs
      {
         ok = false;
      }
   }

   else if ( component.D != 0.0 )
   {
      if ( f_f0 >= 1.0 ) // D and f_f0
      {
         fv               = kRT / ( AVOGADRO * D );
         f0               = fv / f_f0;
         radius_sphere    = f0 / ( 0.06 * M_PI * VISC_20W );
         double volume_dk = ( 4.0 / 3.0 ) * M_PI * pow( radius_sphere, 3.0 );
         mw               = volume_dk * AVOGADRO / vbar;
         s                = mw * buoyancyb / ( AVOGADRO * fv );
      }

      else // D and f are not a valid pair
      {
         ok = false;
      }
   }

   else if ( fv > 0.0 && f_f0 >= 1.0 ) // f and f_f0
   {
      f0               = fv / f_f0;
      D                = kRT / ( AVOGADRO * fv );
      radius_sphere    = f0 / ( 0.06 * M_PI * VISC_20W );
      double volume_fk = ( 4.0 / 3.0 ) * M_PI * pow( radius_sphere, 3.0 );
      mw               = volume_fk * AVOGADRO / vbar;
      s                = mw * buoyancyb / ( AVOGADRO * fv );
   }

   else // do not have 2 valid coeffs
   {
      ok = false;
   }

   double df = qAbs( f - fv ) / fv;

   if ( df > 0.1 )
   { // Significant change in f:  replace and use old as concentration

      if ( c == 0.0 )
      {
         component.signal_concentration = f;
      }

      f = fv;
   }

   component.s      = s; // set component properties (some re-calculated)
   component.D      = D;
   component.mw     = mw;
   component.f_f0   = f_f0;
   component.f      = f;
   component.vbar20 = vbar;

   return ok;
}
} // namespace US_Calc
