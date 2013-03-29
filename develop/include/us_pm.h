#ifndef US_PM_H
#define US_PM_H

#include <iostream>
#include <math.h>
#include <qstring.h>
#include <qfile.h>
#include <qtextstream.h>
#include <set>
#include <vector>
#include "us_hydrodyn_pdbdefs.h"
#include "us_vector.h"

#ifdef WIN32
typedef _int16 int16_t;
#else
#include <stdint.h>
#endif

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

// based upon int16 right now
#define USPM_MAX_VAL   32767
#define USPM_MAX_VAL_D ( ( double ) USPM_MAX_VAL )
#define USPM_MIN_VAL   -32767
#define USPM_MIN_VAL_D ( ( double ) USPM_MIN_VAL )

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

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

class US_PM
{
 private:

   // stuff for spherical harmonics

   double grid_conversion_factor;
   double one_over_grid_conversion_factor;

   double drho;

   double cube_size;
   double bead_radius;
   double bead_radius_over_2;

   // active grid:
   vector < double >   q;
   vector < double >   I;
   vector < double >   e;

   // ideas:
   // best of each single model
   // best of each pair
   // best of each triple
   // etc
   // apply to various experimental & simulated data

 public:
   US_PM               ( double grid_conversion_factor, double drho, vector < double > q, vector < double > I, vector < double > e, int debug_level = 0 );
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
   bool                cylinder         ( int model_pos, vector < double > & params, vector < double > & params_left,  set < pm_point > & model );
   bool                spheroid         ( int model_pos, vector < double > & params, vector < double > & params_left,  set < pm_point > & model );
   bool                ellipsoid        ( int model_pos, vector < double > & params, vector < double > & params_left,  set < pm_point > & model );
   bool                torus            ( int model_pos, vector < double > & params, vector < double > & params_left,  set < pm_point > & model );
   bool                torus_segment    ( int model_pos, vector < double > & params, vector < double > & params_left,  set < pm_point > & model );

   // limits

   bool                set_limits       ( vector < double > & params, vector < double > & low_fparams, vector < double > & high_fparams );

   // output a bead model
   vector < PDB_atom > bead_model       ( set < pm_point > & model );
   QString             qs_bead_model    ( set < pm_point > & model );

   // compute I of model 
   //      using spherical harmonics cached upon the grid
   //      new "on" points are cached
   vector < double >   compute_I        ( set < pm_point > & model );

   static QString      test             ( QString name );

   void                debug            ( int level, QString qs );

   set < pm_point >    recenter         ( set < pm_point > & model );
   // bool             is_connected     ( set < pm_point > & model );
   // double           fitness          ( set < pm_point > & model ); // compute_I and compare rmsd

   // split vectors into a "types" list and the searchable double params
   bool                split            ( vector < double > & params, vector < int > & types, vector < double > & fparams );
   // join them back
   bool                join             ( vector < double > & params, vector < int > & types, vector < double > & fparams );

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif
};

#endif
