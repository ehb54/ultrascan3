#include "../include/us_pm.h"

// to add a new type
//  define model generation routine: 
//     bool US_PM::"object"( int model_pos, vector < double > & params, vector < double > & params_left, set < pm_point > & model )

// insert into:
//   create_1_model()
//   split()
//   join()

// handling routines:

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
   case 0 : // sphere
      return sphere           ( model_pos, params, params_left, model );
      break;
   case 1 : // cylinder
      return cylinder         ( model_pos, params, params_left, model );
      break;
   case 2 : // spheroid
      return spheroid  ( model_pos, params, params_left, model );
      break;
   case 3 : // ellipsoid
      return spheroid  ( model_pos, params, params_left, model );
      break;
   case 4 : // torus

   default:
      error_msg = QString( "US_PM::create_1_model: object type %1 not defined" ).arg( ( int )params[ 0 ] );
      return false;
      break;
   }
   return false;
}

bool US_PM::split( vector < double > & params, vector < int > & types, vector < double > & fparams )
{
   vector < double > params_left;

   int model_pos = 0;
   int ofs       = 0;

   types  .clear();
   fparams.clear();

   while ( ofs < ( int ) params.size() )
   {
      types.push_back( ( int )params[ ofs++ ] );
      if ( ( int ) params.size() <= ofs )
      {
         error_msg = QString( "split: error insufficient params for type %2" ).arg( types.back() );
         return false;
      }
      fparams.push_back( params[ ofs++ ] );
      switch( types.back() )
      {
      case 0 : // sphere
         {
            switch( model_pos )
            {
            case 0 :
               break;
            case 1 :
               if ( ( int ) params.size() <= ofs )
               {
                  error_msg = QString( "split: error insufficient params for type %2" ).arg( types.back() );
                  return false;
               }
               fparams.push_back( params[ ofs++ ] );
               break;
            case 2 :
               if ( ( int ) params.size() <= ofs + 1 )
               {
                  error_msg = QString( "split: error insufficient params for type %2" ).arg( types.back() );
                  return false;
               }
               fparams.push_back( params[ ofs++ ] );
               fparams.push_back( params[ ofs++ ] );
               break;
            default:
               if ( ( int ) params.size() <= ofs + 2 )
               {
                  error_msg = QString( "split: error insufficient params for type %2" ).arg( types.back() );
                  return false;
               }
               fparams.push_back( params[ ofs++ ] );
               fparams.push_back( params[ ofs++ ] );
               fparams.push_back( params[ ofs++ ] );
               break;
            }
         }
         break;
      case 1 : // cylinder
         {
            switch( model_pos )
            {
            case 0 :
               if ( ( int ) params.size() <= ofs )
               {
                  error_msg = QString( "split: error insufficient params for type %2" ).arg( types.back() );
                  return false;
               }
               fparams.push_back( params[ ofs++ ] );
               break;
            case 1 :
               if ( ( int ) params.size() <= ofs + 3 )
               {
                  error_msg = QString( "split: error insufficient params for type %2" ).arg( types.back() );
                  return false;
               }
               fparams.push_back( params[ ofs++ ] );
               fparams.push_back( params[ ofs++ ] );
               fparams.push_back( params[ ofs++ ] );
               fparams.push_back( params[ ofs++ ] );
               break;
            default:
               if ( ( int ) params.size() <= ofs + 5 )
               {
                  error_msg = QString( "split: error insufficient params for type %2" ).arg( types.back() );
                  return false;
               }
               fparams.push_back( params[ ofs++ ] );
               fparams.push_back( params[ ofs++ ] );
               fparams.push_back( params[ ofs++ ] );
               fparams.push_back( params[ ofs++ ] );
               fparams.push_back( params[ ofs++ ] );
               fparams.push_back( params[ ofs++ ] );
               break;
            }
         }
         break;
      case 2 : // spheroid
         {
            switch( model_pos )
            {
            case 0 :
               if ( ( int ) params.size() <= ofs )
               {
                  error_msg = QString( "split: error insufficient params for type %2" ).arg( types.back() );
                  return false;
               }
               fparams.push_back( params[ ofs++ ] );
               break;
            default:
               if ( ( int ) params.size() <= ofs + 6 )
               {
                  error_msg = QString( "split: error insufficient params for type %2" ).arg( types.back() );
                  return false;
               }
               fparams.push_back( params[ ofs++ ] );
               fparams.push_back( params[ ofs++ ] );
               fparams.push_back( params[ ofs++ ] );
               fparams.push_back( params[ ofs++ ] );
               fparams.push_back( params[ ofs++ ] );
               fparams.push_back( params[ ofs++ ] );
               fparams.push_back( params[ ofs++ ] );
               break;
            }
         }
         break;
      case 3 : // ellispoid
         {
            switch( model_pos )
            {
            case 0 :
               if ( ( int ) params.size() <= ofs + 1 )
               {
                  error_msg = QString( "split: error insufficient params for type %2" ).arg( types.back() );
                  return false;
               }
               fparams.push_back( params[ ofs++ ] );
               fparams.push_back( params[ ofs++ ] );
               break;
            default:
               if ( ( int ) params.size() <= ofs + 7 )
               {
                  error_msg = QString( "split: error insufficient params for type %2" ).arg( types.back() );
                  return false;
               }
               fparams.push_back( params[ ofs++ ] );
               fparams.push_back( params[ ofs++ ] );
               fparams.push_back( params[ ofs++ ] );
               fparams.push_back( params[ ofs++ ] );
               fparams.push_back( params[ ofs++ ] );
               fparams.push_back( params[ ofs++ ] );
               fparams.push_back( params[ ofs++ ] );
               fparams.push_back( params[ ofs++ ] );
               break;
            }
         }
         break;
      case 4 : // torus
      default:
         error_msg = QString( "split: object type %1 not defined" ).arg( types.back() );
         return false;
         break;
      }
      ++model_pos;
   }
   return true;
}

bool US_PM::join( vector < double > & params, vector < int > & types, vector < double > & fparams )
{
   params.clear();

   int ofs = 0;

   for ( int i = 0; i < ( int ) types.size(); i++ )
   {
      params.push_back( ( double ) types[ i ] );
      if ( ( int ) fparams.size() <= ofs )
      {
         error_msg = QString( "join: error insufficient params for type %2" ).arg( types[ i ] );
         return false;
      }
      params.push_back( fparams[ ofs++ ] );
      switch( types[ i ] )
      {
      case 0 : // sphere
         {
            switch( i )
            {
            case 0 :
               break;
            case 1 :
               if ( ( int ) fparams.size() <= ofs )
               {
                  error_msg = QString( "join: error insufficient params for type %2" ).arg( types[ i ] );
                  return false;
               }
               params.push_back( fparams[ ofs++ ] );
               break;
            case 2 :
               if ( ( int ) fparams.size() <= ofs + 1 )
               {
                  error_msg = QString( "join: error insufficient params for type %2" ).arg( types[ i ] );
                  return false;
               }
               params.push_back( fparams[ ofs++ ] );
               params.push_back( fparams[ ofs++ ] );
               break;
            default:
               if ( ( int ) fparams.size() <= ofs + 2 )
               {
                  error_msg = QString( "join: error insufficient params for type %2" ).arg( types[ i ] );
                  return false;
               }
               params.push_back( fparams[ ofs++ ] );
               params.push_back( fparams[ ofs++ ] );
               params.push_back( fparams[ ofs++ ] );
               break;
            }
         }
         break;
      case 1 : // cylinder
         {
            switch( i )
            {
            case 0 :
               if ( ( int ) fparams.size() <= ofs )
               {
                  error_msg = QString( "join: error insufficient params for type %2" ).arg( types[ i ] );
                  return false;
               }
               params.push_back( fparams[ ofs++ ] );
               break;
            case 1 :
               if ( ( int ) fparams.size() <= ofs + 3 )
               {
                  error_msg = QString( "join: error insufficient params for type %2" ).arg( types[ i ] );
                  return false;
               }
               params.push_back( fparams[ ofs++ ] );
               params.push_back( fparams[ ofs++ ] );
               params.push_back( fparams[ ofs++ ] );
               params.push_back( fparams[ ofs++ ] );
               break;
            default:
               if ( ( int ) fparams.size() <= ofs + 5 )
               {
                  error_msg = QString( "join: error insufficient params for type %2" ).arg( types[ i ] );
                  return false;
               }
               params.push_back( fparams[ ofs++ ] );
               params.push_back( fparams[ ofs++ ] );
               params.push_back( fparams[ ofs++ ] );
               params.push_back( fparams[ ofs++ ] );
               params.push_back( fparams[ ofs++ ] );
               params.push_back( fparams[ ofs++ ] );
               break;
            }
         }
         break;
      case 2 : // spheroid
         {
            switch( i )
            {
            case 0 :
               if ( ( int ) fparams.size() <= ofs )
               {
                  error_msg = QString( "join: error insufficient params for type %2" ).arg( types[ i ] );
                  return false;
               }
               params.push_back( fparams[ ofs++ ] );
               break;
            default:
               if ( ( int ) fparams.size() <= ofs + 6 )
               {
                  error_msg = QString( "join: error insufficient params for type %2" ).arg( types[ i ] );
                  return false;
               }
               params.push_back( fparams[ ofs++ ] );
               params.push_back( fparams[ ofs++ ] );
               params.push_back( fparams[ ofs++ ] );
               params.push_back( fparams[ ofs++ ] );
               params.push_back( fparams[ ofs++ ] );
               params.push_back( fparams[ ofs++ ] );
               params.push_back( fparams[ ofs++ ] );
               break;
            }
         }
         break;
      case 3 : // ellispoid
         {
            switch( i )
            {
            case 0 :
               if ( ( int ) fparams.size() <= ofs + 1 )
               {
                  error_msg = QString( "join: error insufficient params for type %2" ).arg( types[ i ] );
                  return false;
               }
               params.push_back( fparams[ ofs++ ] );
               params.push_back( fparams[ ofs++ ] );
               break;
            default:
               if ( ( int ) fparams.size() <= ofs + 7 )
               {
                  error_msg = QString( "join: error insufficient params for type %2" ).arg( types[ i ] );
                  return false;
               }
               params.push_back( fparams[ ofs++ ] );
               params.push_back( fparams[ ofs++ ] );
               params.push_back( fparams[ ofs++ ] );
               params.push_back( fparams[ ofs++ ] );
               params.push_back( fparams[ ofs++ ] );
               params.push_back( fparams[ ofs++ ] );
               params.push_back( fparams[ ofs++ ] );
               params.push_back( fparams[ ofs++ ] );
               break;
            }
         }
         break;
      case 4 : // torus
      default:
         error_msg = QString( "join: object type %1 not defined" ).arg( types[ i ] );
         return false;
         break;
      }
   }
   return true;
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

   params_left.clear();
   for ( int i = ofs; i < ( int ) params.size(); i++ )
   {
      params_left.push_back( params[ i ] );
   }

   // make a bounding box

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
            if ( sqrt( (double)( ( x - centerx ) * ( x - centerx ) +
                                 ( y - centery ) * ( y - centery ) +
                                 ( z - centerz ) * ( z - centerz ) ) ) < radius ) 
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
   debug( 1, "cylinder" );
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

   params_left.clear();
   for ( int i = ofs; i < ( int ) params.size(); i++ )
   {
      params_left.push_back( params[ i ] );
   }

   debug( 1, QString( "cylinder radius %1 start %2,%3,%4 end %5,%6,%7\n" )
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

   debug( 1, QString( "cylinder:bounding box %1,%2,%3 %4,%5,%6\n" )
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

   debug( 1, QString( "cylinder: normal %1,%2,%3 length^2 %4" )
          .arg( nvh[ 0 ] )
          .arg( nvh[ 1 ] )
          .arg( nvh[ 2 ] )
          .arg( h2 ) );

   double r2;

   double radius2 = radius * radius;

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

            if ( r2 <= radius2 + bead_radius_over_2 )
            {
               if ( 
                   p_b[ 0 ] * p_b[ 0 ] +
                   p_b[ 1 ] * p_b[ 1 ] +
                   p_b[ 2 ] * p_b[ 2 ] <= h2 - r2 + bead_radius_over_2 
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
                      p_e[ 2 ] * p_e[ 2 ] <= h2 - r2 + bead_radius_over_2 
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

bool US_PM::torus( int, vector < double > &, vector < double > &, set < pm_point > & )
                  // int model_pos, vector < double > & params, vector < double > & params_left, set < pm_point > & model )
{
   debug( 1, "torus (nyi)" );
   error_msg = "nyi";
   return false;
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

   switch( model_pos )
   {
   case 0 :
      centerx = 0e0;
      centery = 0e0;
      centerz = 0e0;
      normx   = 1e0;
      normy   = 0e0;
      normz   = 0e0;
      break;
   default:
      centerx = params[ ofs++ ];
      centery = params[ ofs++ ];
      centerz = params[ ofs++ ];
      normx   = params[ ofs++ ];
      normy   = params[ ofs++ ];
      normz   = params[ ofs++ ];
      break;
   }

   params_left.clear();
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
   radiusmax = radiusmax > radiusc ? radiusmax : radiusc;
   
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
   double radiusc2   = radiusc * radiusc;

   double one_over_radiusa2 = 1e0 / radiusa2;
   double one_over_radiusb2 = 1e0 / radiusb2;
   double one_over_radiusc2 = 1e0 / radiusc2;

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
                one_over_radiusa2 * ( v_p_c[ 0 ] * v_p_c[ 0 ] ) +
                one_over_radiusb2 * ( v_p_c[ 1 ] * v_p_c[ 1 ] ) +
                one_over_radiusc2 * ( v_p_c[ 2 ] * v_p_c[ 2 ] ) < 1e0 + bead_radius_over_2 )
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

   switch( model_pos )
   {
   case 0 :
      centerx = 0e0;
      centery = 0e0;
      centerz = 0e0;
      normx   = 1e0;
      normy   = 0e0;
      normz   = 0e0;
      break;
   default:
      centerx = params[ ofs++ ];
      centery = params[ ofs++ ];
      centerz = params[ ofs++ ];
      normx   = params[ ofs++ ];
      normy   = params[ ofs++ ];
      normz   = params[ ofs++ ];
      break;
   }

   params_left.clear();
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
                one_over_radiusb2 * ( v_p_c[ 2 ] * v_p_c[ 2 ] ) < 1e0 + bead_radius_over_2 )
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
