#include "../include/us_pm.h"

// to add a new type
//  define model generation routine: 
//     bool US_PM::"object"( int model_pos, vector < double > & params, vector < double > & params_left, set < pm_point > & model )

// insert into:
// enum objects
//   init_objects()
//   create_1_model()
//   best_md0()
//   also: best_"object"()

// handling routines:

// finds best one model
bool US_PM::best_md0( 
                     vector < double > & params, 
                     vector < double > & low_fparams, 
                     vector < double > & high_fparams, 
                     set < pm_point >  & model 
                     )
{
   if ( params.size() < 1 )
   {
      error_msg = "best_md0: params[0] must be set";
      return false;
   }
   int this_type = (int) params[ 0 ];
   if ( this_type >= (int) object_names.size() ||
        this_type < 0 )
   {
      error_msg = "best_md0: params[0] invalid param";
      return false;
   }

   if ( !low_fparams.size() )
   {
      if ( us_log )
      {
         us_log->log( "best_md0: notice, setting default range\n" );
      }
      zero_md0_params( params );
      set_limits( params, low_fparams, high_fparams );
   }

   if ( (int)low_fparams.size() != object_m0_parameters[ this_type ] ||
        (int)high_fparams.size() != object_m0_parameters[ this_type ] )
   {
      error_msg = "best_md0: low_fparams and/or high_fparams incorrect size";
      return false;
   }

   for ( int i = 0; i < (int)low_fparams.size(); ++i )
   {
      if ( low_fparams[ i ] > high_fparams[ i ] )
      {
         error_msg = "best_md0: fparams low > high";
         return false;
      }
   }

   switch( this_type )
   {
   case SPHERE        : return best_sphere       ( params, low_fparams, high_fparams, model ); break;
   case CYLINDER      : return best_cylinder     ( params, low_fparams, high_fparams, model ); break;
   case SPHEROID      : return best_spheroid     ( params, low_fparams, high_fparams, model ); break;
   case ELLIPSOID     : return best_ellipsoid    ( params, low_fparams, high_fparams, model ); break;
   case TORUS         : return best_torus        ( params, low_fparams, high_fparams, model ); break;
      // case TORUS_SEGMENT : return best_torus_segment( params, low_fparams, high_fparams, model ); break;
   default: error_msg = "best_md0: invalid type"; return false; break;
   }
   return false;
}

void US_PM::init_objects()
{
   object_names          .clear( );
   object_m0_parameters  .clear( );
   object_parameter_types.clear( );
   object_name_map       .clear( );

   vector < vector < parameter_type > > tmp_ptv;
   vector < parameter_type >            tmp_pt;


   object_name_map[ SPHERE ] = "sphere";
   object_names        .push_back( "sphere" );
   object_m0_parameters.push_back( 1 ); // radius
   {
      tmp_pt .clear( );
      tmp_ptv.clear( );
      {
         // model 0
         tmp_pt .push_back( RADIUS );
         tmp_ptv.push_back( tmp_pt );
         // model 1
         tmp_pt .push_back( COORD );
         tmp_ptv.push_back( tmp_pt );
         // model 2
         tmp_pt .push_back( COORD );
         tmp_ptv.push_back( tmp_pt );
         // model 3 and above
         tmp_pt .push_back( COORD );
         tmp_ptv.push_back( tmp_pt );
      }
      object_parameter_types.push_back( tmp_ptv );
   }


   object_name_map[ CYLINDER ] = "cylinder";
   object_names        .push_back( "cylinder" );
   object_m0_parameters.push_back( 2 ); // height length
   {
      tmp_pt .clear( );
      tmp_ptv.clear( );
      {
         // model 0
         tmp_pt .push_back( RADIUS );
         tmp_pt .push_back( COORD );
         tmp_ptv.push_back( tmp_pt );
         // model 1
         tmp_pt .push_back( COORD );
         tmp_pt .push_back( COORD );
         tmp_pt .push_back( COORD );
         tmp_ptv.push_back( tmp_pt );
         // model 2 and above
         tmp_pt .push_back( COORD );
         tmp_pt .push_back( COORD );
         tmp_ptv.push_back( tmp_pt );
      }
      object_parameter_types.push_back( tmp_ptv );
   }

   object_name_map[ SPHEROID ] = "spheroid";
   object_names        .push_back( "spheroid" );
   object_m0_parameters.push_back( 2 ); // a, b
   {
      tmp_pt .clear( );
      tmp_ptv.clear( );
      {
         // model 0
         tmp_pt .push_back( RADIUS );
         tmp_pt .push_back( RADIUS );
         tmp_ptv.push_back( tmp_pt );
         // model 1 and above
         tmp_pt .push_back( COORD );
         tmp_pt .push_back( COORD );
         tmp_pt .push_back( COORD );
         tmp_pt .push_back( NORM );
         tmp_pt .push_back( NORM );
         tmp_pt .push_back( NORM );
         tmp_pt .push_back( ANGLE );
         tmp_ptv.push_back( tmp_pt );
      }
      object_parameter_types.push_back( tmp_ptv );
   }

   object_name_map[ ELLIPSOID ] = "ellipsoid";
   object_names        .push_back( "ellipsoid" );
   object_m0_parameters.push_back( 3 ); // a, b, c
   {
      tmp_pt .clear( );
      tmp_ptv.clear( );
      {
         // model 0
         tmp_pt .push_back( RADIUS );
         tmp_pt .push_back( RADIUS );
         tmp_pt .push_back( RADIUS );
         tmp_ptv.push_back( tmp_pt );
         // model 1 and above
         tmp_pt .push_back( COORD );
         tmp_pt .push_back( COORD );
         tmp_pt .push_back( COORD );
         tmp_pt .push_back( NORM );
         tmp_pt .push_back( NORM );
         tmp_pt .push_back( NORM );
         tmp_pt .push_back( ANGLE );
         tmp_ptv.push_back( tmp_pt );
      }
      object_parameter_types.push_back( tmp_ptv );
   }

   object_name_map[ TORUS ] = "torus";
   object_names        .push_back( "torus" );
   object_m0_parameters.push_back( 2 ); // radius1, radius2
   {
      tmp_pt .clear( );
      tmp_ptv.clear( );
      {
         // model 0
         tmp_pt .push_back( RADIUS );
         tmp_pt .push_back( RADIUS );
         tmp_ptv.push_back( tmp_pt );
         // model 1 and above
         tmp_pt .push_back( COORD );
         tmp_pt .push_back( COORD );
         tmp_pt .push_back( COORD );
         tmp_pt .push_back( NORM );
         tmp_pt .push_back( NORM );
         tmp_pt .push_back( NORM );
         tmp_pt .push_back( ANGLE );
         tmp_ptv.push_back( tmp_pt );
      }
      object_parameter_types.push_back( tmp_ptv );
   }

   /* later turn on torus_segment
   object_name_map[ TORUS_SEGMENT ] = "torus segment";
   object_names        .push_back( "torus_segment" );
   object_m0_parameters.push_back( 3 ); // radius1, radius2, end theta
   object_best_f       .push_back( &US_PM::best_torus_segment );
   {
      tmp_pt .clear( );
      tmp_ptv.clear( );
      {
         // model 0
         tmp_pt .push_back( RADIUS );
         tmp_pt .push_back( RADIUS );
         tmp_pt .push_back( ANGLE );
         tmp_ptv.push_back( tmp_pt );
         // model 1 and above
         tmp_pt .push_back( COORD );
         tmp_pt .push_back( COORD );
         tmp_pt .push_back( COORD );
         tmp_pt .push_back( NORM );
         tmp_pt .push_back( NORM );
         tmp_pt .push_back( NORM );
         tmp_pt .push_back( ANGLE );
         tmp_ptv.push_back( tmp_pt );
      }
      object_parameter_types.push_back( tmp_ptv );
   }
   */

   object_type_name.clear( );
   object_type_name[ COORD  ] = "Coord";
   object_type_name[ NORM   ] = "Norm";
   object_type_name[ RADIUS ] = "Radius";
   object_type_name[ ANGLE  ] = "Angle";
}

QString US_PM::get_name( vector < int > & types )
{
   QString result;

   for ( int i = 0; i < (int) types.size(); ++i )
   {
      if ( object_name_map.count( (objects) types[ i ] ) )
      {
         result += "_" + object_name_map[ (objects) types[ i ] ];
      } else {
         result += "_unknown";
      }
   }
   return result;
}

bool US_PM::create_1_model( int model_pos, vector < double > & params, vector < double > & params_left, set < pm_point > & model )
{
   debug( 1, "create_1_model" );

   if ( !params.size() )
   {
      error_msg = "create_1_model called with no params!";
      return false;
   }

   switch( ( int )params[ 0 ] )
   {
   case SPHERE : 
      return sphere         ( model_pos, params, params_left, model );
      break;
   case CYLINDER : 
      return cylinder       ( model_pos, params, params_left, model );
      break;
   case SPHEROID :
      return spheroid       ( model_pos, params, params_left, model );
      break;
   case ELLIPSOID :
      return ellipsoid      ( model_pos, params, params_left, model );
      break;
   case TORUS :
      return torus          ( model_pos, params, params_left, model );
      break;
      /*
   case TORUS_SEGMENT :
      return torus_segment  ( model_pos, params, params_left, model );
      break;
      */

   default:
      error_msg = QString( "US_PM::create_1_model: object type %1 not defined" ).arg( ( int )params[ 0 ] );
      return false;
      break;
   }
   return false;
}

// objects themselves:

bool US_PM::sphere( int model_pos, vector < double > & params, vector < double > & params_left, set < pm_point > & model )
{
   debug( 1, "sphere" );
   int ofs = 1;
   double radius = params[ ofs++ ];

   double centerx;
   double centery;
   double centerz;

   switch( model_pos )
   {
   case 0 :
      centerx = 0e0;
      centery = 0e0;
      centerz = 0e0;
      break;
   case 1 :
      centerx = params[ ofs++ ];
      centery = 0e0;
      centerz = 0e0;
      break;
   case 2 :
      centerx = params[ ofs++ ];
      centery = params[ ofs++ ];
      centerz = 0e0;
      break;
   default:
      centerx = params[ ofs++ ];
      centery = params[ ofs++ ];
      centerz = params[ ofs++ ];
      break;
   }

   params_left.clear( );
   for ( int i = ofs; i < ( int ) params.size(); i++ )
   {
      params_left.push_back( params[ i ] );
   }

   // make a bounding box

   double one_over_radius2 = 1e0 / ( ( radius + bead_radius_over_2gcf ) * ( radius + bead_radius_over_2gcf ) );

   debug( 2, QString( "sphere @ %1 %2 %3 radius %4" ).arg( centerx ).arg( centery ).arg( centerz ).arg( radius ) );

   int minx = (int) ( centerx - radius - 1 );
   int maxx = (int) ( centerx + radius + 1 );
   int miny = (int) ( centery - radius - 1 );
   int maxy = (int) ( centery + radius + 1 );
   int minz = (int) ( centerz - radius - 1 );
   int maxz = (int) ( centerz + radius + 1 );

   pm_point pmp;

   for ( int x = minx; x <= maxx; ++x )
   {
      for ( int y = miny; y <= maxy; ++y )
      {
         for ( int z = minz; z <= maxz; ++z )
         {
            if ( one_over_radius2 * (double)( ( x - centerx ) * ( x - centerx ) +
                                              ( y - centery ) * ( y - centery ) +
                                              ( z - centerz ) * ( z - centerz ) ) LE_OR_LT 1e0 )
            {
               pmp.x[ 0 ] = ( int16_t )x;
               pmp.x[ 1 ] = ( int16_t )y;
               pmp.x[ 2 ] = ( int16_t )z;
               model.insert( pmp );
            }
         }
      }
   }
   return true;
}

bool US_PM::cylinder( int model_pos, vector < double > & params, vector < double > & params_left, set < pm_point > & model )
{
   debug( 1, "cylinder----------------------------------------" );
   int ofs = 1;
   double radius = params[ ofs++ ];

   double basex;
   double basey;
   double basez;
   double endx;
   double endy;
   double endz;

   switch( model_pos )
   {
   case 0 :
      // centered along x axis
      basex = params[ ofs++ ];
      basey = 0e0;
      basez = 0e0;
      endx = -basex;
      endy = 0e0;
      endz = 0e0;
      if ( endx < basex )
      {
         double tmp = basex;
         basex      = endx;
         endx       = tmp;
      }
      break;
   case 1 :
      // in x-y plane
      basex = params[ ofs++ ];
      basey = params[ ofs++ ];
      basez = 0e0;
      endx = params[ ofs++ ];
      endy = params[ ofs++ ];
      endz = 0e0;
      if ( endx < basex )
      {
         double tmp = basex;
         basex      = endx;
         endx       = tmp;
      }
      if ( endy < basey )
      {
         double tmp = basey;
         basey      = endy;
         endy       = tmp;
      }
      break;
   default:
      // arbitrary
      basex = params[ ofs++ ];
      basey = params[ ofs++ ];
      basez = params[ ofs++ ];
      endx = params[ ofs++ ];
      endy = params[ ofs++ ];
      endz = params[ ofs++ ];
      if ( endx < basex )
      {
         double tmp = basex;
         basex      = endx;
         endx       = tmp;
      }
      if ( endy < basey )
      {
         double tmp = basey;
         basey      = endy;
         endy       = tmp;
      }
      if ( endz < basez )
      {
         double tmp = basez;
         basez      = endz;
         endz       = tmp;
      }
      break;
   }

   params_left.clear( );
   for ( int i = ofs; i < ( int ) params.size(); i++ )
   {
      params_left.push_back( params[ i ] );
   }

   debug( 1, QString( "cylinder radius %1 start %2,%3,%4 end %5,%6,%7" )
          .arg( radius )
          .arg( basex )
          .arg( basey )
          .arg( basez )
          .arg( endx )
          .arg( endy )
          .arg( endz ) 
          );

   // make a bounding box

   int minx = (int) ( basex - radius - 1 );
   int maxx = (int) ( endx  + radius + 1 );
   int miny = (int) ( basey - radius - 1 );
   int maxy = (int) ( endy  + radius + 1 );
   int minz = (int) ( basez - radius - 1 );
   int maxz = (int) ( endz  + radius + 1 );

   debug( 2, QString( "cylinder:bounding box %1,%2,%3 %4,%5,%6" )
          .arg( minx )
          .arg( miny )
          .arg( minz )
          .arg( maxx )
          .arg( maxy )
          .arg( maxz )
          );

   pm_point pmp;

   // p . ( end - base ) < r^2
   // if so
   //  d(p,b)^2 <=  h^2 - r^2 &&
   // d(p,b)^2 <= h^2 - r^2
   // also:
   //   do prolate & oblate spheroids

   double vh[ 3 ] =
      {
         endx - basex,
         endy - basey,
         endz - basez 
      };

   double h2 = 
      vh[ 0 ] * vh[ 0 ] + 
      vh[ 1 ] * vh[ 1 ] + 
      vh[ 2 ] * vh[ 2 ];

   double lh2 = sqrt( h2 );

   double nvh[ 3 ] =
      {
         vh[ 0 ] / lh2,
         vh[ 1 ] / lh2,
         vh[ 2 ] / lh2 
      };

   double radius2 = radius * radius;
   debug( 1, QString( "cylinder: normal %1,%2,%3 length^2 %4 radius^2 %5" )
          .arg( nvh[ 0 ] )
          .arg( nvh[ 1 ] )
          .arg( nvh[ 2 ] )
          .arg( h2 )
          .arg( radius2 ) );

   double r2;

   for ( int x = minx; x <= maxx; ++x )
   {
      for ( int y = miny; y <= maxy; ++y )
      {
         for ( int z = minz; z <= maxz; ++z )
         {
            // compute distance2 from point to line
            
            double p_b[ 3 ] =
               {
                  ( double ) x - basex,
                  ( double ) y - basey,
                  ( double ) z - basez
               };
            
            double dot_p_b_d =
               p_b[ 0 ] * nvh[ 0 ] +
               p_b[ 1 ] * nvh[ 1 ] +
               p_b[ 2 ] * nvh[ 2 ];
               
            double d_dot_p_b_d[ 3 ] =
               {
                  nvh[ 0 ] * dot_p_b_d,
                  nvh[ 1 ] * dot_p_b_d,
                  nvh[ 2 ] * dot_p_b_d
               };

            double p_b_minus_d_dot_p_b_d[ 3 ] =
               {
                  p_b[ 0 ] - d_dot_p_b_d[ 0 ],
                  p_b[ 1 ] - d_dot_p_b_d[ 1 ],
                  p_b[ 2 ] - d_dot_p_b_d[ 2 ]
               };

            r2 = 
               p_b_minus_d_dot_p_b_d[ 0 ] * p_b_minus_d_dot_p_b_d[ 0 ] +
               p_b_minus_d_dot_p_b_d[ 1 ] * p_b_minus_d_dot_p_b_d[ 1 ] +
               p_b_minus_d_dot_p_b_d[ 2 ] * p_b_minus_d_dot_p_b_d[ 2 ];

            if ( r2 LE_OR_LT radius2 + bead_radius_over_2gcf )
            {
               if ( 
                   p_b[ 0 ] * p_b[ 0 ] +
                   p_b[ 1 ] * p_b[ 1 ] +
                   p_b[ 2 ] * p_b[ 2 ] LE_OR_LT h2 + r2 + bead_radius_over_2gcf 
                   )
               {
                  double p_e[ 3 ] = 
                     {
                        ( double ) x - endx,
                        ( double ) y - endy,
                        ( double ) z - endz
                     };
                  
                  if ( 
                      p_e[ 0 ] * p_e[ 0 ] +
                      p_e[ 1 ] * p_e[ 1 ] +
                      p_e[ 2 ] * p_e[ 2 ] LE_OR_LT h2 + r2 + bead_radius_over_2gcf 
                      )
                  {
                     pmp.x[ 0 ] = ( int16_t )x;
                     pmp.x[ 1 ] = ( int16_t )y;
                     pmp.x[ 2 ] = ( int16_t )z;
                     model.insert( pmp );
                  }
               }
            }
         }
      }
   }
   return true;
}

bool US_PM::ellipsoid( int model_pos, vector < double > & params, vector < double > & params_left, set < pm_point > & model )
{
   debug( 1, "ellipsoid" );
   int ofs = 1;
   double radiusa = params[ ofs++ ];
   double radiusb = params[ ofs++ ];
   double radiusc = params[ ofs++ ];

   double centerx;
   double centery;
   double centerz;
   double normx;
   double normy;
   double normz;
   double theta;

   switch( model_pos )
   {
   case 0 :
      centerx = 0e0;
      centery = 0e0;
      centerz = 0e0;
      normx   = 1e0;
      normy   = 0e0;
      normz   = 0e0;
      theta   = 0e0;
      break;
   default:
      centerx = params[ ofs++ ];
      centery = params[ ofs++ ];
      centerz = params[ ofs++ ];
      normx   = params[ ofs++ ];
      normy   = params[ ofs++ ];
      normz   = params[ ofs++ ];
      theta   = params[ ofs++ ];
      break;
   }

   params_left.clear( );
   for ( int i = ofs; i < ( int ) params.size(); i++ )
   {
      params_left.push_back( params[ i ] );
   }

   double magnorminv = 1e0 / sqrt( normx * normx + normy * normy + normz * normz );
   normx *= magnorminv;
   normy *= magnorminv;
   normz *= magnorminv;

   // make a bounding box

   debug( 1, QString( "ellipsoid: center %1,%2,%3 norm vector %4,%5,%6" )
          .arg( centerx )
          .arg( centery )
          .arg( centerz )
          .arg( normx )
          .arg( normy )
          .arg( normz )
          );

   double radiusmax = radiusa > radiusb ? radiusa : radiusb;
   radiusmax = radiusmax > radiusc ? radiusmax : radiusc;
   
   debug( 1, QString( "ellipsoid: radiusa %1 radiusb %2 radiusmax %3" )
          .arg( radiusa )
          .arg( radiusb )
          .arg( radiusmax )
          );

   int minx = (int) ( centerx - 2 * radiusmax - 2 );
   int maxx = (int) ( centerx + 2 * radiusmax + 2 );
   int miny = (int) ( centery - 2 * radiusmax - 2 );
   int maxy = (int) ( centery + 2 * radiusmax + 2 );
   int minz = (int) ( centerz - 2 * radiusmax - 2 );
   int maxz = (int) ( centerz + 2 * radiusmax + 2 );

   debug( 2, QString( "ellispoid:bounding box %1,%2,%3 %4,%5,%6\n" )
          .arg( minx )
          .arg( miny )
          .arg( minz )
          .arg( maxx )
          .arg( maxy )
          .arg( maxz )
          );

   pm_point pmp;

   //    double radiusa2   = radiusa * radiusa;
   //    double radiusb2   = radiusb * radiusb;
   //    double radiusc2   = radiusc * radiusc;

   double one_over_radiusa2 = 1e0 / ( ( radiusa + bead_radius_over_2gcf ) * ( radiusa + bead_radius_over_2gcf ) );
   double one_over_radiusb2 = 1e0 / ( ( radiusb + bead_radius_over_2gcf ) * ( radiusb + bead_radius_over_2gcf ) );
   double one_over_radiusc2 = 1e0 / ( ( radiusc + bead_radius_over_2gcf ) * ( radiusc + bead_radius_over_2gcf ) );

   if ( model_pos && theta )
   {
      vector < vector < double > > rm;
      rotation_matrix( normx, normy, normz, theta, rm );

      double newx;
      double newy;
      double newz;
      for ( int x = minx; x <= maxx; ++x )
      {
         for ( int y = miny; y <= maxy; ++y )
         {
            for ( int z = minz; z <= maxz; ++z )
            {
               apply_rotation_matrix( rm, x, y, z, newx, newy, newz );
               double v_p_c[ 3 ] =
                  {
                     newx - (double) centerx,
                     newy - (double) centery,
                     newz - (double) centerz,
                  };
                         
               if ( 
                   one_over_radiusa2 * ( v_p_c[ 0 ] * v_p_c[ 0 ] ) +
                   one_over_radiusb2 * ( v_p_c[ 1 ] * v_p_c[ 1 ] ) +
                   one_over_radiusc2 * ( v_p_c[ 2 ] * v_p_c[ 2 ] ) LE_OR_LT 1e0 )
               {
                  pmp.x[ 0 ] = ( int16_t )x;
                  pmp.x[ 1 ] = ( int16_t )y;
                  pmp.x[ 2 ] = ( int16_t )z;
                  model.insert( pmp );
               }
            }
         }
      }
   } else {
      for ( int x = minx; x <= maxx; ++x )
      {
         for ( int y = miny; y <= maxy; ++y )
         {
            for ( int z = minz; z <= maxz; ++z )
            {
               double v_p_c[ 3 ] =
                  {
                     (double)x - centerx,
                     (double)y - centery,
                     (double)z - centerz
                  };
                         
               if ( 
                   one_over_radiusa2 * ( v_p_c[ 0 ] * v_p_c[ 0 ] ) +
                   one_over_radiusb2 * ( v_p_c[ 1 ] * v_p_c[ 1 ] ) +
                   one_over_radiusc2 * ( v_p_c[ 2 ] * v_p_c[ 2 ] ) LE_OR_LT 1e0 )
               {
                  pmp.x[ 0 ] = ( int16_t )x;
                  pmp.x[ 1 ] = ( int16_t )y;
                  pmp.x[ 2 ] = ( int16_t )z;
                  model.insert( pmp );
               }
            }
         }
      }
   }      

   return true;
}

bool US_PM::spheroid( int model_pos, vector < double > & params, vector < double > & params_left, set < pm_point > & model )
{
   debug( 1, "spheroid" );
   int ofs = 1;
   double radiusa = params[ ofs++ ];
   double radiusb = params[ ofs++ ];

   double centerx;
   double centery;
   double centerz;
   double normx;
   double normy;
   double normz;
   double theta;

   switch( model_pos )
   {
   case 0 :
      centerx = 0e0;
      centery = 0e0;
      centerz = 0e0;
      normx   = 1e0;
      normy   = 0e0;
      normz   = 0e0;
      theta   = 0e0;
      break;
   default:
      centerx = params[ ofs++ ];
      centery = params[ ofs++ ];
      centerz = params[ ofs++ ];
      normx   = params[ ofs++ ];
      normy   = params[ ofs++ ];
      normz   = params[ ofs++ ];
      theta   = params[ ofs++ ];
      break;
   }

   params_left.clear( );
   for ( int i = ofs; i < ( int ) params.size(); i++ )
   {
      params_left.push_back( params[ i ] );
   }

   double magnorminv = 1e0 / sqrt( normx * normx + normy * normy + normz * normz );
   normx *= magnorminv;
   normy *= magnorminv;
   normz *= magnorminv;

   vector < vector < double > > rm;
   rotation_matrix( normx, normy, normz, theta, rm );

   // make a bounding box

   debug( 1, QString( "spheroid: center %1,%2,%3 norm vector %4,%5,%6" )
          .arg( centerx )
          .arg( centery )
          .arg( centerz )
          .arg( normx )
          .arg( normy )
          .arg( normz )
          );

   double radiusmax = radiusa > radiusb ? radiusa : radiusb;
   
   debug( 1, QString( "spheroid: radiusa %1 radiusb %2 radiusmax %3" )
          .arg( radiusa )
          .arg( radiusb )
          .arg( radiusmax )
          );

   int minx = (int) ( centerx - 2 * radiusmax - 2 );
   int maxx = (int) ( centerx + 2 * radiusmax + 2 );
   int miny = (int) ( centery - 2 * radiusmax - 2 );
   int maxy = (int) ( centery + 2 * radiusmax + 2 );
   int minz = (int) ( centerz - 2 * radiusmax - 2 );
   int maxz = (int) ( centerz + 2 * radiusmax + 2 );

   pm_point pmp;

   // double radiusa2   = radiusa * radiusa;
   // double radiusb2   = radiusb * radiusb;

   double one_over_radiusa2 = 1e0 / ( ( radiusa + bead_radius_over_2gcf ) * ( radiusa + bead_radius_over_2gcf ) );
   double one_over_radiusb2 = 1e0 / ( ( radiusb + bead_radius_over_2gcf ) * ( radiusb + bead_radius_over_2gcf ) );

   if ( model_pos && theta )
   {
      vector < vector < double > > rm;
      rotation_matrix( normx, normy, normz, theta, rm );

      double newx;
      double newy;
      double newz;
      for ( int x = minx; x <= maxx; ++x )
      {
         for ( int y = miny; y <= maxy; ++y )
         {
            for ( int z = minz; z <= maxz; ++z )
            {
               apply_rotation_matrix( rm, x, y, z, newx, newy, newz );
               double v_p_c[ 3 ] =
                  {
                     newx - centerx,
                     newy - centery,
                     newz - centerz,
                  };
               if ( 
                   one_over_radiusa2 * ( v_p_c[ 0 ] * v_p_c[ 0 ] + v_p_c[ 1 ] * v_p_c[ 1 ] ) +
                   one_over_radiusb2 * ( v_p_c[ 2 ] * v_p_c[ 2 ] ) LE_OR_LT 1e0 )
               {
                  pmp.x[ 0 ] = ( int16_t )x;
                  pmp.x[ 1 ] = ( int16_t )y;
                  pmp.x[ 2 ] = ( int16_t )z;
                  model.insert( pmp );
               }
            }
         }
      }
   } else {
      for ( int x = minx; x <= maxx; ++x )
      {
         for ( int y = miny; y <= maxy; ++y )
         {
            for ( int z = minz; z <= maxz; ++z )
            {
               double v_p_c[ 3 ] =
                  {
                     (double) x - centerx,
                     (double) y - centery,
                     (double) z - centerz
                  };
                         
               if ( 
                   one_over_radiusa2 * ( v_p_c[ 0 ] * v_p_c[ 0 ] + v_p_c[ 1 ] * v_p_c[ 1 ] ) +
                   one_over_radiusb2 * ( v_p_c[ 2 ] * v_p_c[ 2 ] ) LE_OR_LT 1e0 )
               {
                  pmp.x[ 0 ] = ( int16_t )x;
                  pmp.x[ 1 ] = ( int16_t )y;
                  pmp.x[ 2 ] = ( int16_t )z;
                  model.insert( pmp );
               }
            }
         }
      }
   }      

   return true;
}

bool US_PM::torus_segment( int model_pos, vector < double > & params, vector < double > & params_left, set < pm_point > & model )
{
   debug( 1, "torus_segment" );
   int ofs = 1;
   double radiusa = params[ ofs++ ];
   double radiusb = params[ ofs++ ];
   /* double enda    = */ params[ ofs++ ];  // a %'ge ? or an angle.  assume start with 0 moduli 2PI or 100% maybe require limits on params

   double centerx;
   double centery;
   double centerz;
   double normx;
   double normy;
   double normz;
   // to be added back if ever revisited
   // double theta;

   switch( model_pos )
   {
   case 0 :
      centerx = 0e0;
      centery = 0e0;
      centerz = 0e0;
      normx   = 1e0;
      normy   = 0e0;
      normz   = 0e0;
      // theta   = 0e0;
      break;
   default:
      centerx = params[ ofs++ ];
      centery = params[ ofs++ ];
      centerz = params[ ofs++ ];
      normx   = params[ ofs++ ];
      normy   = params[ ofs++ ];
      normz   = params[ ofs++ ];
      // theta   = params[ ofs++ ];
      break;
   }

   params_left.clear( );
   for ( int i = ofs; i < ( int ) params.size(); i++ )
   {
      params_left.push_back( params[ i ] );
   }

   double magnorminv = 1e0 / sqrt( normx * normx + normy * normy + normz * normz );
   normx *= magnorminv;
   normy *= magnorminv;
   normz *= magnorminv;

   // make a bounding box

   debug( 1, QString( "spheroid: center %1,%2,%3 norm vector %4,%5,%6" )
          .arg( centerx )
          .arg( centery )
          .arg( centerz )
          .arg( normx )
          .arg( normy )
          .arg( normz )
          );

   double radiusmax = radiusa > radiusb ? radiusa : radiusb;
   
   debug( 1, QString( "spheroid: radiusa %1 radiusb %2 radiusmax %3" )
          .arg( radiusa )
          .arg( radiusb )
          .arg( radiusmax )
          );

   int minx = (int) ( centerx - radiusmax - 1 );
   int maxx = (int) ( centerx + radiusmax + 1 );
   int miny = (int) ( centery - radiusmax - 1 );
   int maxy = (int) ( centery + radiusmax + 1 );
   int minz = (int) ( centerz - radiusmax - 1 );
   int maxz = (int) ( centerz + radiusmax + 1 );

   pm_point pmp;

   double radiusa2   = radiusa * radiusa;
   double radiusb2   = radiusb * radiusb;

   double one_over_radiusa2 = 1e0 / radiusa2;
   double one_over_radiusb2 = 1e0 / radiusb2;

   for ( int x = minx; x <= maxx; ++x )
   {
      for ( int y = miny; y <= maxy; ++y )
      {
         for ( int z = minz; z <= maxz; ++z )
         {
            double v_p_c[ 3 ] =
               {
                  (double)( x - centerx ),
                  (double)( y - centery ),
                  (double)( z - centerz )
               };
                         
            if ( 
                one_over_radiusa2 * ( v_p_c[ 0 ] * v_p_c[ 0 ] + v_p_c[ 1 ] * v_p_c[ 1 ] ) +
                one_over_radiusb2 * ( v_p_c[ 2 ] * v_p_c[ 2 ] ) LE_OR_LT 1e0 )
            {
               pmp.x[ 0 ] = ( int16_t )x;
               pmp.x[ 1 ] = ( int16_t )y;
               pmp.x[ 2 ] = ( int16_t )z;
               model.insert( pmp );
            }
         }
      }
   }
   return true;
}

bool US_PM::torus( int model_pos, vector < double > & params, vector < double > & params_left, set < pm_point > & model )
{
   debug( 1, "torus" );
   int ofs = 1;
   double radiusa = params[ ofs++ ];
   double radiusb = params[ ofs++ ];

   double centerx;
   double centery;
   double centerz;
   double normx;
   double normy;
   double normz;
   double theta;

   switch( model_pos )
   {
   case 0 :
      centerx = 0e0;
      centery = 0e0;
      centerz = 0e0;
      normx   = 1e0;
      normy   = 0e0;
      normz   = 0e0;
      theta   = 0e0;
      break;
   default:
      centerx = params[ ofs++ ];
      centery = params[ ofs++ ];
      centerz = params[ ofs++ ];
      normx   = params[ ofs++ ];
      normy   = params[ ofs++ ];
      normz   = params[ ofs++ ];
      theta   = params[ ofs++ ];
      break;
   }

   params_left.clear( );
   for ( int i = ofs; i < ( int ) params.size(); i++ )
   {
      params_left.push_back( params[ i ] );
   }

   double magnorminv = 1e0 / sqrt( normx * normx + normy * normy + normz * normz );
   normx *= magnorminv;
   normy *= magnorminv;
   normz *= magnorminv;

   // make a bounding box

   debug( 1, QString( "torus: center %1,%2,%3 norm vector %4,%5,%6" )
          .arg( centerx )
          .arg( centery )
          .arg( centerz )
          .arg( normx )
          .arg( normy )
          .arg( normz )
          );

   double radiusmax = radiusa > radiusb ? radiusa : radiusb;
   
   debug( 1, QString( "torus: radiusa %1 radiusb %2 radiusmax %3" )
          .arg( radiusa )
          .arg( radiusb )
          .arg( radiusmax )
          );

   int minx = (int) ( - 1 * ( radiusa + radiusb + 1 ) );
   int maxx = (int) ( 1 * ( radiusa + radiusb + 1 ) );
   int miny = (int) ( - 1 * ( radiusa + radiusb + 1 ) );
   int maxy = (int) ( 1 * ( radiusa + radiusb + 1 ) );
   int minz = (int) ( - 1 * ( radiusb + 1 ) );
   int maxz = (int) ( 1 * ( radiusb + 1 ) );

   pm_point pmp;

   double radiusa2 = ( radiusa + bead_radius_over_2gcf ) * ( radiusa + bead_radius_over_2gcf );
   // double oneoverradiusa2 = 1e0 / ( ( radiusa + bead_radius_over_2gcf ) * ( radiusa + bead_radius_over_2gcf ) );
   double oneoverradiusb2 = 1e0 / ( ( radiusb + bead_radius_over_2gcf ) * ( radiusb + bead_radius_over_2gcf ) );

   if ( model_pos && theta )
   {
      vector < vector < double > > rm;
      rotation_matrix( normx, normy, normz, theta, rm );

      double newx;
      double newy;
      double newz;

      double ofsx = 1;
      for ( double x = (double) minx; x <= (double) maxx; x += ofsx )
      {
         double dx = (double)x; // - centerx;
         double dx2 = dx * dx;
         for ( double y = (double)miny; y <= (double) maxy; y += ofsx )
         {
            // find the x-y plane center
            double dy = (double)y; // - centery;
            double dx2pdy2 = dx2 + dy * dy;
            if ( dx2pdy2 > 0 ) // exclude translated 0,0,0 from torus
            {
               double lenscale = radiusa / sqrt( dx2pdy2 );

               double c[ 3 ] =
                  {
                     dx * lenscale,
                     dy * lenscale,
                     0e0
                  };
         
               for ( double z = (double) minz; z <= (double) maxz; z += ofsx )
               {

                  if ( oneoverradiusb2 * ( ( (double) x - c[ 0 ] ) * ( (double) x - c[ 0 ] ) +
                                           ( (double) y - c[ 1 ] ) * ( (double) y - c[ 1 ] ) +
                                           ( (double) z - c[ 2 ] ) * ( (double) z - c[ 2 ] ) ) LE_OR_LT 1e0 )
                  {
                     apply_rotation_matrix( rm, x, y, z, newx, newy, newz );
                     pmp.x[ 0 ] = ( int16_t )( newx + centerx + 5e-1 );
                     pmp.x[ 1 ] = ( int16_t )( newy + centery + 5e-1 );
                     pmp.x[ 2 ] = ( int16_t )( newz + centerz + 5e-1 );
                     model.insert( pmp );
                  }
               }
            } else {
               for ( int z = minz; z <= maxz; ++z )
               {
                  if ( oneoverradiusb2 * ( radiusa2 + z * z ) LE_OR_LT 1e0 )
                  {
                     apply_rotation_matrix( rm, x, y, z, newx, newy, newz );
                     pmp.x[ 0 ] = ( int16_t )( newx + centerx + 5e-1 );
                     pmp.x[ 1 ] = ( int16_t )( newy + centery + 5e-1 );
                     pmp.x[ 2 ] = ( int16_t )( newz + centerz + 5e-1 );
                     model.insert( pmp );
                  }
               }
            }               
         }
      }
   } else {
      for ( int x = minx; x <= maxx; ++x )
      {
         double dx = (double)x; // - centerx;
         double dx2 = dx * dx;
         for ( int y = miny; y <= maxy; ++y )
         {
            // find the x-y plane center
            double dy = (double)y; // - centery;
            double dx2pdy2 = dx2 + dy * dy;

            if ( dx2pdy2 > 0 )
            {
               double lenscale = radiusa / sqrt( dx2pdy2 );

               double c[ 3 ] =
                  {
                     dx * lenscale,
                     dy * lenscale,
                     0e0
                  };
         
               for ( int z = minz; z <= maxz; ++z )
               {

                  if ( oneoverradiusb2 * ( ( (double) x - c[ 0 ] ) * ( (double) x - c[ 0 ] ) +
                                           ( (double) y - c[ 1 ] ) * ( (double) y - c[ 1 ] ) +
                                           ( (double) z - c[ 2 ] ) * ( (double) z - c[ 2 ] ) ) LE_OR_LT 1e0 )
                  {
                     pmp.x[ 0 ] = ( int16_t )( (double) x + centerx );
                     pmp.x[ 1 ] = ( int16_t )( (double) y + centery );
                     pmp.x[ 2 ] = ( int16_t )( (double) z + centerz );
                     model.insert( pmp );
                  }
               }
            } else {
               for ( int z = minz; z <= maxz; ++z )
               {
                  if ( oneoverradiusb2 * ( radiusa2 + z * z ) LE_OR_LT 1e0 )
                  {
                     pmp.x[ 0 ] = ( int16_t )( (double) x + centerx );
                     pmp.x[ 1 ] = ( int16_t )( (double) y + centery );
                     pmp.x[ 2 ] = ( int16_t )( (double) z + centerz );
                     model.insert( pmp );
                  }
               }
            }               
         }
      }
   }
   return true;
}
