//! \file us_calc_coefficients.cpp
#include "us_calc_coefficients.h"
#include "us_calc_fp.h" // first:  floating-point policy

#include "us_calc_constants.h"

#include <QtMath>

namespace US_Calc
{
namespace
{
// Ideal gas constant times 20 degrees C in Kelvin
constexpr double kRT = R_GC * K20;
} // namespace

bool calc_coefficients( Component& component )
{
   bool   ok = true;
   double vbar;          // component vbar
   double volume;        // e.g., vbar * mw / AVOGADRO
   double vol_fac;       // volume factor, e.g., 0.75 / pi
   double radius_sphere; // e.g., pow( volume * vol_fac, 1/3 );
   double rsph_fac;      // radius_sphere factor; e.g., 0.06 * pi * VISC
   double onethird;      // one third ( 1.0 / 3.0 )
   double c;             // concentration
   double s;             // sedimentation coefficient
   double D;             // diffusion coefficient
   double mw;            // molecular weight
   double f;             // frictional coefficient
   double fv;            // frictional coefficient (working value)
   double f_f0;          // frictional ratio
   double f0;            // f-zero
   double s20w;
   double buoyancyb;

   // Ensure that we have a vbar we can use
   vbar = component.vbar20;

   if ( vbar <= 0.0 )
   {
      vbar             = TYPICAL_VBAR;
      component.vbar20 = vbar;
   }

   vol_fac  = 0.75 / M_PI; // various factors used in calcs below
   onethird = 1.0 / 3.0;
   rsph_fac = 0.06 * M_PI * VISC_20W;

   s    = component.s; // component coefficient values
   D    = component.D;
   mw   = component.mw;
   f    = component.f;
   f_f0 = component.f_f0;
   c    = component.signal_concentration;
   fv   = f;

   // Negative s-values require negative buoyancy.
   buoyancyb = 1.0 - ( vbar * DENS_20W );

   if ( ( s < 0.0 && buoyancyb > 0.0 ) || ( s > 0.0 && buoyancyb < 0.0 ) )
   {
      return false;
   }

   // Start with s if possible
   if ( s != 0.0 )
   {
      s20w = s;

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
