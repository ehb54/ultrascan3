//! \file us_hydrosim.cpp
//
#include "us_hydrosim.h"
#include "us_constants.h"
#include "us_math.h"

US_Hydrosim::US_Hydrocomp::US_Hydrocomp()
{
   s      = 0.0;
   D      = 0.0;
   f      = 0.0;
   f_f0   = 1.0;
   a      = 0.0;
   b      = 0.0;
   volume = 0.0;
}

US_Hydrosim::US_Hydrosim()
{
   mw          = 50000.0;
   density     = DENS_20W;
   viscosity   = VISC_20W * 100.0;
   vbar        = TYPICAL_VBAR;
   temperature = 20.0;
   axial_ratio = 10.0;
   guid        = QString();
}

void US_Hydrosim::calculate( double temperature )
{
   US_Math::SolutionData solution;
   solution.density   = density;
   solution.viscosity = viscosity;
   solution.vbar      = vbar;
   solution.vbar20    = vbar + 4.25e-4 * ( temperature - 20.0 );
   
   US_Math::data_correction( temperature, solution );

   double t                = temperature + K0;
   double vol_per_molecule = solution.vbar * mw / AVOGADRO;
   double rad_sphere       =
                 pow( vol_per_molecule * 3.0 / ( 4.0 * M_PI ), 1.0 / 3.0 );

   double f0 = rad_sphere * 6.0 * M_PI * solution.viscosity_tb * 0.01;

   // Recaluclate volume to put into cubic angstroms:
   vol_per_molecule = ( 4.0 / 3.0 ) * M_PI * pow( rad_sphere * 1.0e+08, 3.0 );

   // Sphere:
   double ss = mw * solution.buoyancyb / ( AVOGADRO * f0 );
   double Ds = ss * R * t / ( mw * solution.buoyancyb );

   sphere.s      = ss;
   sphere.D      = Ds;
   sphere.f      = f0;
   sphere.f_f0   = 1.0;
   sphere.a      = 1.0e+08 * rad_sphere;
   sphere.b      = sphere.a;
   sphere.volume = vol_per_molecule;

   // Oblate ellipsoid:
   oblate.f_f0   = sqrt( sq( axial_ratio ) - 1.0 ) /
      ( pow( axial_ratio, 2.0 / 3.0 ) * atan( sqrt( sq( axial_ratio ) - 1.0 )));

   double bo = 1.0e+08 * rad_sphere / pow( axial_ratio, 2.0 / 3.0 );
   double ao = axial_ratio * bo;
   double fo = oblate.f_f0 * f0;
   double so = mw * solution.buoyancyb / ( AVOGADRO * fo );
   double Do = so * R * t / ( mw * solution.buoyancyb );

   oblate.s      = so;
   oblate.D      = Do;
   oblate.f      = fo;
   oblate.a      = ao;
   oblate.b      = bo;
   oblate.volume = vol_per_molecule;

   // Prolate ellipsoid, ratio = ap/bp  (a = semi-major axis)

   prolate.f_f0 = pow( axial_ratio, -1.0 / 3.0 ) 
                * sqrt( sq( axial_ratio ) - 1.0 )
                / log( axial_ratio + sqrt( sq( axial_ratio ) - 1.0 ) );

   double ap = 1.0e+08 * ( rad_sphere * pow( axial_ratio, 2.0 / 3.0 ) );
   double bp = ap / axial_ratio;
   double fp = prolate.f_f0 * f0;
   double sp = mw * solution.buoyancyb / ( AVOGADRO * fp );
   double Dp = sp * R * t / ( mw * solution.buoyancyb );

   prolate.s      = sp;
   prolate.D      = Dp;
   prolate.f      = fp;
   prolate.a      = ap;
   prolate.b      = bp;
   prolate.volume = vol_per_molecule;


   // Long rod:
   rod.f_f0   =  pow( 2.0 / 3.0, 1.0 / 3.0 ) 
              *  pow( axial_ratio, 2.0 / 3.0 ) 
              /  ( log( 2.0 * axial_ratio ) - 0.3 );

   double br = 1.0e+08 * pow( 2.0 / ( 3.0 * axial_ratio ), 1.0 / 3.0 ) 
             * rad_sphere;
   
   double ar = axial_ratio * br;
   double fr = rod.f_f0 * f0;
   double sr = mw * solution.buoyancyb / ( AVOGADRO * fr );
   double Dr = sr * R * t / ( mw * solution.buoyancyb );

   rod.s      = sr;
   rod.D      = Dr;
   rod.f      = fr;
   rod.a      = ar;
   rod.b      = br;
   rod.volume = vol_per_molecule;
}

/*
// x is axial_ratio
   double x = 1.1;

   // From: K.E. van Holde, Biophysical Chemistry, 2nd edition, chapter 4.1
   // Calculate frictional ratio as a function of axial ratio
   for ( int i = 0; i < ARRAYSIZE; i++, x += 0.1 )
   {
      prolate[ i ] = pow( x, -1.0 / 3.0 ) * sqrt( sq( x ) - 1.0 ) /
                     log( x + sqrt( sq( x ) - 1.0 ) );

      oblate[ i ]  = sqrt( sq( x ) - 1.0 ) /
                     ( pow( x, 2.0 / 3.0 ) * atan( sqrt( sq( x ) - 1.0 ) ) );

      rod[ i ]     = pow( 2.0 / 3.0, 1.0 / 3.0 ) * pow( x, 2.0 / 3.0 ) /
                     ( log( 2.0 * x ) - 0.3 );

      ratio_x[ i ] = x;
   }
*/
