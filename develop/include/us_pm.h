#ifndef US_PM_H
#define US_PM_H

#include <iostream>
#include <stdint.h>
#include <math.h>
#include <qstring.h>
#include <set>
#include <vector>
#include "us_hydrodyn_pdbdefs.h"

class pm_point
{
 public:
   int16_t x[ 3 ];
   bool operator < ( const pm_point & objIn ) const
   {
      return
         x[ 0 ] < objIn.x[ 0 ] ||
         ( x[ 0 ] == objIn.x[ 0 ] &&
           x[ 1 ] <  objIn.x[ 1 ] ) ||
         ( x[ 0 ] == objIn.x[ 0 ] &&
           x[ 1 ] == objIn.x[ 1 ] &&
           x[ 2 ] <  objIn.x[ 2 ] );
   }
};

class US_PM
{
 private:

   // stuff for spherical harmonics

   double cube_size;
   double bead_radius;
   double grid_conversion_factor;
   double one_over_grid_conversion_factor;

   // active grid:
   vector < double >   q;
   vector < double >   I;
   vector < double >   e;

 public:
   US_PM               ( double grid_conversion_factor, vector < double > q, vector < double > I, vector < double > e, int debug_level = 0 );
   ~US_PM              ();

   QString             error_msg;

   int                 debug_level;


   // params = model type, params required for model type, model type etc.

   // build a model from params:
   bool                create_model     ( vector < double > params, set < pm_point > & model );

   // build a single model and return remaining points
   bool                create_1_model   ( int model_pos, vector < double > & params, vector < double > & params_left,  set < pm_point > & model );

   // models themselves:

   bool                sphere           ( int model_pos, vector < double > & params, vector < double > & params_left,  set < pm_point > & model );
   bool                ellipsoid        ( int model_pos, vector < double > & params, vector < double > & params_left,  set < pm_point > & model );
   bool                cylinder         ( int model_pos, vector < double > & params, vector < double > & params_left,  set < pm_point > & model );

   // output a bead model
   vector < PDB_atom > bead_model       ( set < pm_point > & model );
   QString             qs_bead_model    ( set < pm_point > & model );

   // compute I of model 
   //      using spherical harmonics cached upon the grid
   //      new "on" points are cached
   vector < double >   compute_I        ( set < pm_point > & model );

   static QString      test             ();

   void                debug            ( int level, QString qs );

   set < pm_point >    recenter         ( set < pm_point > & model );
   // bool             = is_connected( set < pm_point > & model );
};

#endif
