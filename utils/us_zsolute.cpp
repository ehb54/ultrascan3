//! \file us_zsolute.cpp
#include "us_zsolute.h"
#include "us_math2.h"

US_ZSolute::US_ZSolute( double x0, double y0, double z0, double c0 )
{
   x = x0;
   y = y0;
   z = z0;
   c = c0;
}

// Public function to put a model component attribute value
void US_ZSolute::put_mcomp_attr( US_Model::SimulationComponent& comp,
                                 double& aval, const int a_type )
{
   switch ( a_type )
   {
      default:
      case ATTR_S:                 // Sedimentation Coefficient
         comp.s       = aval;
         break;
      case ATTR_K:                 // Frictional Ratio
         comp.f_f0    = aval;
         break;
      case ATTR_W:                 // Molecular Weight
         comp.mw      = aval;
         break;
      case ATTR_V:                 // Vbar 20W
         comp.vbar20  = aval;
         break;
      case ATTR_D:                 // Diffusion Coefficient
         comp.D       = aval;
         break;
      case ATTR_C:                 // Concentrations
         comp.signal_concentration  = aval;
         break;
   }
}

// Public function to put a solute attribute value
void US_ZSolute::put_solute_attr( US_ZSolute& solute,
                                  double& aval, const int a_type )
{
   switch ( a_type )
   {
      default:
      case 0:                      // X value
         solute.x     = aval;
         break;
      case 1:                      // Y value
         solute.y     = aval;
         break;
      case 2:                      // Z value
         solute.z     = aval;
         break;
      case 9:                      // C value
         solute.c     = aval;
         break;
   }
}

// Public function to get a model component attribute value
void US_ZSolute::get_mcomp_attr( US_Model::SimulationComponent& comp,
                                 double& aval, const int a_type )
{
   switch ( a_type )
   {
      default:
      case ATTR_S:                 // Sedimentation Coefficient
         aval         = comp.s;
         break;
      case ATTR_K:                 // Frictional Ratio
         aval         = comp.f_f0;
         break;
      case ATTR_W:                 // Molecular Weight
         aval         = comp.mw;
         break;
      case ATTR_V:                 // Vbar 20W
         aval         = comp.vbar20;
         break;
      case ATTR_D:                 // Diffusion Coefficient
         aval         = comp.D;
         break;
      case ATTR_C:                 // Concentration
         aval         = comp.signal_concentration;
         break;
   }
}

// Public function to get a solute attribute value
void US_ZSolute::get_solute_attr( US_ZSolute& solute,
                                  double& aval, const int a_type )
{
   switch ( a_type )
   {
      default:
      case 0:                      // X value
         aval         = solute.x;
         break;
      case 1:                      // Y value
         aval         = solute.y;
         break;
      case 2:                      // Z value
         aval         = solute.z;
         break;
      case 9:                      // C value
         aval         = solute.c;
         break;
   }
}

// Public function to set model component values from a solute
void US_ZSolute::set_mcomp_values( US_Model::SimulationComponent& comp,
                                   US_ZSolute& solute, const int s_type,
                                   const bool concv )
{
   int attr_x   = ( s_type >> 6 ) & 7;
   int attr_y   = ( s_type >> 3 ) & 7;
   int attr_z   = s_type & 7;
   double aval  = 0.0;

   get_solute_attr( solute, aval, 0 );
   put_mcomp_attr ( comp,   aval, attr_x );

   get_solute_attr( solute, aval, 1 );
   put_mcomp_attr ( comp,   aval, attr_y );

   get_solute_attr( solute, aval, 2 );
   put_mcomp_attr ( comp,   aval, attr_z );

   if ( concv )
   {  // Copy concentration value, too
      get_solute_attr( solute, aval, 9 );
      put_mcomp_attr ( comp,   aval, 9 );
   }
}

// Public function to set solute values from a model component
void US_ZSolute::set_solute_values( US_Model::SimulationComponent& comp,
                                    US_ZSolute& solute, const int s_type )
{
   int attr_x   = ( s_type >> 6 ) & 7;
   int attr_y   = ( s_type >> 3 ) & 7;
   int attr_z   = s_type & 7;
   double aval  = 0.0;

   get_mcomp_attr ( comp,   aval, attr_x );
   put_solute_attr( solute, aval, 0 );

   get_mcomp_attr ( comp,   aval, attr_y );
   put_solute_attr( solute, aval, 1 );

   get_mcomp_attr ( comp,   aval, attr_z );
   put_solute_attr( solute, aval, 2 );

   get_mcomp_attr ( comp,   aval, 9 );
   put_solute_attr( solute, aval, 9 );
}

// Public function to initialize grid solutes
void US_ZSolute::init_grid_solutes( double x_min, double x_max, int x_res,
                                    double y_min, double y_max, int y_res,
                                    int    grid_reps, double* z_coeffs,
                                    QList< QVector< US_ZSolute > >& solutes )
{
   if ( grid_reps < 1 ) grid_reps = 1;

   int    nprx     = qMax( 1, ( ( x_res / grid_reps ) - 1 ) );
   int    npry     = qMax( 1, ( ( y_res / grid_reps ) - 1 ) );
   double x_step   = qAbs( x_max - x_min ) / (double)nprx;
   double y_step   = qAbs( y_max - y_min ) / (double)npry;
          x_step   = ( x_step  > 0.0 ) ? x_step : ( x_min * 1.001 );
          y_step   = ( y_step  > 0.0 ) ? y_step : ( y_min * 1.001 );
   double x_grid   = x_step / grid_reps;  
   double y_grid   = y_step / grid_reps;

   // Allow a 1% overscan
   x_max   += 0.01 * x_step;
   y_max   += 0.01 * y_step;

   solutes.reserve( sq( grid_reps ) );

   // Generate solutes for each grid repetition
   for ( int ii = 0; ii < grid_reps; ii++ )
   {
      for ( int jj = 0; jj < grid_reps; jj++ )
      {
         solutes << create_grid_solutes(
                       x_min + x_grid * ii, x_max, x_step,
                       y_min + y_grid * jj, y_max, y_step,
                       z_coeffs );
      }
   }
}

// Internal function to flag the type of Z function used
int US_ZSolute::zcoeff_flag( double* zcoeffs )
{
   int zcflg  = ( ( zcoeffs[ 1 ] == 0.0 ) ? 0 : 1 )   // =0 -> Constant
              + ( ( zcoeffs[ 2 ] == 0.0 ) ? 0 : 2 )   // =1 -> Linear
              + ( ( zcoeffs[ 3 ] == 0.0 ) ? 0 : 4 );  // >1 -> Polynomial

   return zcflg;
}

// Internal static function to create a single solute vector
QVector< US_ZSolute > US_ZSolute::create_grid_solutes(
        double x_min, double x_max, double x_step,
        double y_min, double y_max, double y_step,
        double* z_coeffs )
{
   QVector< US_ZSolute > solute_vector;
   double zz    = z_coeffs[ 0 ];
   bool x_is_s  = ( x_max < 1.0e-10 );
   bool y_is_s  = ( y_max < 1.0e-10 );

   int    zcflg = zcoeff_flag( z_coeffs );

   if ( zcflg == 0 )
   {  // Z is constant
      for ( double xx = x_min; xx <= x_max; xx += x_step )
      {
         if ( x_is_s  && xx >= -1.0e-14  &&  xx <= 1.0e-14 ) continue;

         for ( double yy = y_min; yy <= y_max; yy += y_step )
         {
            if ( y_is_s  && yy >= -1.0e-14  &&  yy <= 1.0e-14 ) continue;

            solute_vector << US_ZSolute( xx, yy, zz, 0.0 );
         }
      }
   }

   else if ( zcflg == 1 )
   {  // Z varies linearly with X
      for ( double xx = x_min; xx <= x_max; xx += x_step )
      {
         if ( x_is_s  && xx >= -1.0e-14  &&  xx <= 1.0e-14 ) continue;
         zz           = z_coeffs[ 0 ] + z_coeffs[ 1 ] * xx;

         for ( double yy = y_min; yy <= y_max; yy += y_step )
         {
            if ( y_is_s  && yy >= -1.0e-14  &&  yy <= 1.0e-14 ) continue;

            solute_vector << US_ZSolute( xx, yy, zz, 0.0 );
         }
      }
   }

   else
   {  // Z varies by ( a + bX + cX^2 + dX^3 )
      for ( double xx = x_min; xx <= x_max; xx += x_step )
      {
         if ( x_is_s  && xx >= -1.0e-14  &&  xx <= 1.0e-14 ) continue;
         zz           = z_coeffs[ 0 ] + z_coeffs[ 1 ] * xx 
                                      + z_coeffs[ 2 ] * xx * xx
                                      + z_coeffs[ 3 ] * xx * xx * xx;

         for ( double yy = y_min; yy <= y_max; yy += y_step )
         {
            if ( y_is_s  && yy >= -1.0e-14  &&  yy <= 1.0e-14 ) continue;

            solute_vector << US_ZSolute( xx, yy, zz, 0.0 );
         }
      }
   }

   return solute_vector;
}


