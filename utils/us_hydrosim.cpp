//! \file us_hydrosim.cpp
//
#include "us_hydrosim.h"
#include "us_constants.h"
#include "us_math2.h"
#include "us_model.h"

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
   viscosity   = VISC_20W;
   vbar        = TYPICAL_VBAR;
   temperature = 20.0;
   temperature = 25.0;
   axial_ratio = 10.0;
   analyteGUID = QString();
}

void US_Hydrosim::calculate( double temperature )
{
   US_Math2::SolutionData solution;
   double vbar20      = vbar + 4.25e-4 * ( temperature - 20.0 );
   solution.density   = density;
   solution.viscosity = viscosity;
   solution.vbar      = vbar;
   solution.vbar20    = vbar20;
//qDebug() << "    Hydrosim Calc dens visc vbar temp vbar20"
//   << density << viscosity << vbar << temperature << vbar20;
   
   US_Math2::data_correction( temperature, solution );

   US_Model::SimulationComponent sc_sph;
   US_Model::SimulationComponent sc_obl;
   US_Model::SimulationComponent sc_pro;
   US_Model::SimulationComponent sc_rod;

//qDebug() << "     Math2 visc visc_wt visc_tb"
//   << solution.viscosity << solution.viscosity_wt << solution.viscosity_tb;
   double vol_per_molecule = solution.vbar * mw / AVOGADRO;
   double rad_sphere       =
                 pow( vol_per_molecule * 3.0 / ( 4.0 * M_PI ), 1.0 / 3.0 );

   sphere.f_f0 = 1.0;
   sphere.a    = 1.0e+08 * rad_sphere;

   // Recaluclate volume to put into cubic angstroms:
   vol_per_molecule = ( 4.0 / 3.0 ) * M_PI * pow( rad_sphere * 1.0e+08, 3.0 );

   // Sphere:
   sc_sph.mw     = mw;
   sc_sph.f_f0   = sphere.f_f0;
   sc_sph.s      = 0.0;
   sc_sph.D      = 0.0;
   sc_sph.f      = 0.0;
   sc_sph.vbar20 = vbar20;
   US_Model::calc_coefficients( sc_sph );

   sphere.s      = sc_sph.s;
   sphere.D      = sc_sph.D;
   sphere.f      = sc_sph.f;
   sphere.b      = sphere.a;
   sphere.volume = vol_per_molecule;

   // Oblate ellipsoid:
   oblate.f_f0   = sqrt( sq( axial_ratio ) - 1.0 ) /
      ( pow( axial_ratio, 2.0 / 3.0 ) * atan( sqrt( sq( axial_ratio ) - 1.0 )));

   double bo = 1.0e+08 * rad_sphere / pow( axial_ratio, 2.0 / 3.0 );
   double ao = axial_ratio * bo;

   sc_obl.mw     = mw;
   sc_obl.f_f0   = oblate.f_f0;
   sc_obl.s      = 0.0;
   sc_obl.D      = 0.0;
   sc_obl.f      = 0.0;
   sc_obl.vbar20 = vbar20;
   US_Model::calc_coefficients( sc_obl );

   oblate.s      = sc_obl.s;
   oblate.D      = sc_obl.D;
   oblate.f      = sc_obl.f;
   oblate.a      = ao;
   oblate.b      = bo;
   oblate.volume = vol_per_molecule;

   // Prolate ellipsoid, ratio = ap/bp  (a = semi-major axis)

   prolate.f_f0 = pow( axial_ratio, -1.0 / 3.0 ) 
                * sqrt( sq( axial_ratio ) - 1.0 )
                / log( axial_ratio + sqrt( sq( axial_ratio ) - 1.0 ) );

   double ap = 1.0e+08 * ( rad_sphere * pow( axial_ratio, 2.0 / 3.0 ) );
   double bp = ap / axial_ratio;

   sc_pro.mw     = mw;
   sc_pro.f_f0   = prolate.f_f0;
   sc_pro.s      = 0.0;
   sc_pro.D      = 0.0;
   sc_pro.f      = 0.0;
   sc_pro.vbar20 = vbar20;
   US_Model::calc_coefficients( sc_pro );

   prolate.s      = sc_pro.s;
   prolate.D      = sc_pro.D;
   prolate.f      = sc_pro.f;
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

   sc_rod.mw     = mw;
   sc_rod.f_f0   = rod.f_f0;
   sc_rod.s      = 0.0;
   sc_rod.D      = 0.0;
   sc_rod.f      = 0.0;
   sc_rod.vbar20 = vbar20;
   US_Model::calc_coefficients( sc_rod );

   rod.s         = sc_rod.s;
   rod.D         = sc_rod.D;
   rod.f         = sc_rod.f;
   rod.a         = ar;
   rod.b         = br;
   rod.volume    = vol_per_molecule;
//qDebug() << "SPHERE  hyd  ff0 mw" << sphere.f_f0 << mw
//  << " s D f" << sphere.s << sphere.D << sphere.f;
//qDebug() << "SPHERE  mod  ff0 mw" << sc_sph.f_f0 << sc_sph.mw
//  << " s D f" << sc_sph.s << sc_sph.D << sc_sph.f;
//qDebug() << "OBLATE  hyd  ff0 mw" << oblate.f_f0 << mw
//  << " s D f" << oblate.s << oblate.D << oblate.f;
//qDebug() << "OBLATE  mod  ff0 mw" << sc_obl.f_f0 << sc_obl.mw
//  << " s D f" << sc_obl.s << sc_obl.D << sc_obl.f;
//qDebug() << "PROLATE hyd  ff0 mw" << prolate.f_f0 << mw
//  << " s D f" << prolate.s << prolate.D << prolate.f;
//qDebug() << "PROLATE mod  ff0 mw" << sc_pro.f_f0 << sc_pro.mw
//  << " s D f" << sc_pro.s << sc_pro.D << sc_pro.f;
//qDebug() << "ROD     hyd  ff0 mw" << rod.f_f0 << mw
//  << " s D f" << rod.s << rod.D << rod.f;
//qDebug() << "ROD     mod  ff0 mw" << sc_rod.f_f0 << sc_rod.mw
//  << " s D f" << sc_rod.s << sc_rod.D << sc_rod.f;
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
