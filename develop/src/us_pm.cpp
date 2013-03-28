#include "../include/us_pm.h"

US_PM::US_PM( double grid_conversion_factor, double drho, vector < double > q, vector < double > I, vector < double > e, int debug_level )
{
   this->grid_conversion_factor = grid_conversion_factor;
   this->drho                   = drho;
   this->q                      = q;
   this->I                      = I;
   this->e                      = e;
   this->debug_level            = debug_level;

   one_over_grid_conversion_factor = 1e0 / grid_conversion_factor;

   cube_size   = grid_conversion_factor * grid_conversion_factor * grid_conversion_factor;

   // radius gives a sphere with equal size to the cube:
   bead_radius = pow( cube_size / M_PI, 1e0/3e0 );

   cout << QString( "US_PM:cube size   %1\n"
                    "US_PM:bead radius %2\n" )
      .arg( cube_size )
      .arg( bead_radius );
}

US_PM::~US_PM()
{
}

QString US_PM::qs_bead_model( set < pm_point > & model )
{
   // sphere should equal volume occupied by the cube
   // 

   QString qs;

   qs += QString( "%1 %2\n" ).arg( model.size() ).arg( .7 );

   for ( set < pm_point >::iterator it = model.begin();
         it != model.end();
         it++ )
   {
      qs += 
         QString( "%1 %2 %3 %4  20 8 Unk 10\n")
         .arg( it->x[ 0 ] * grid_conversion_factor )
         .arg( it->x[ 1 ] * grid_conversion_factor )
         .arg( it->x[ 2 ] * grid_conversion_factor )
         .arg( bead_radius )
         ;
   }
   return qs;
}

vector < PDB_atom > US_PM::bead_model( set < pm_point > & model )
{
   
   vector < PDB_atom > result;

   int i = 0;
   for ( set < pm_point >::iterator it = model.begin();
         it != model.end();
         it++ )
   {
      PDB_atom tmp_atom;
      tmp_atom.coordinate.axis[0]      = it->x[ 0 ] * grid_conversion_factor;
      tmp_atom.coordinate.axis[1]      = it->x[ 1 ] * grid_conversion_factor;
      tmp_atom.coordinate.axis[2]      = it->x[ 2 ] * grid_conversion_factor;
      tmp_atom.bead_coordinate.axis[0] = it->x[ 0 ] * grid_conversion_factor;
      tmp_atom.bead_coordinate.axis[1] = it->x[ 1 ] * grid_conversion_factor;
      tmp_atom.bead_coordinate.axis[2] = it->x[ 2 ] * grid_conversion_factor;
      tmp_atom.bead_computed_radius    = bead_radius;
      tmp_atom.bead_actual_radius      = bead_radius;
      tmp_atom.radius                  = bead_radius;
      tmp_atom.bead_mw                 = 20;
      tmp_atom.mw                      = 20;
      tmp_atom.bead_ref_volume         = 0;
      tmp_atom.bead_color              = 1;
      tmp_atom.serial                  = ++i;
      tmp_atom.exposed_code            = 1;
      tmp_atom.all_beads               .clear();
      tmp_atom.name                    = "PM";
      tmp_atom.resName                 = "PM";
      tmp_atom.iCode                   = "";
      tmp_atom.chainID                 = "";
      tmp_atom.chain                   = 1;
      tmp_atom.active                  = 1;
      tmp_atom.normalized_ot_is_valid  = false;

      result.push_back( tmp_atom );
   }

   return result;
}

bool US_PM::create_model( vector < double > params, set < pm_point > & model )
{
   debug( 1, "craete_model" );
   model.clear();

   vector < double > params_left;

   int model_pos = 0;

   while ( params.size() )
   {
      debug( 2, QString( "create_model, params size %1" ).arg( params.size() ) );
      if ( !create_1_model( model_pos, params, params_left, model ) )
      {
         return false;
      }
      params = params_left;
      ++model_pos;
   }      
   return true;
}

vector < double > US_PM::compute_I( set < pm_point > & /* model */ )
{
   vector < double > result( I.size() );
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
   case 0 : // sphere
      return sphere   ( model_pos, params, params_left, model );
      break;
   case 1 : // cylinder
      return cylinder ( model_pos, params, params_left, model );
      break;
   case 2 : // ellipsoid
      return ellipsoid( model_pos, params, params_left, model );
      break;
   case 3 : // torus

   default:
      error_msg = QString( "US_PM::create_1_model: object type %1 not defined" ).arg( ( int )params[ 0 ] );
      return false;
      break;
   }
   return false;
}


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

bool US_PM::cylinder( int model_pos, vector < double > & params, vector < double > & params_left, set < pm_point > & /* model */ )
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

   // make a bounding box

   int minx = (int) ( basex - radius - 1 );
   int maxx = (int) ( endx  + radius + 1 );
   int miny = (int) ( basey - radius - 1 );
   int maxy = (int) ( endy  + radius + 1 );
   int minz = (int) ( basez - radius - 1 );
   int maxz = (int) ( endz  + radius + 1 );

   // pm_point pmp;

   // p . ( end - base ) < r^2
   // if so
   //  d(p,b)^2 <=  h^2 - r^2 &&
   // d(p,b)^2 <= h^2 - r^2
   // also:
   //   do prolate & oblate spheroids

   for ( int x = minx; x <= maxx; ++x )
   {
      for ( int y = miny; y <= maxy; ++y )
      {
         for ( int z = minz; z <= maxz; ++z )
         {
//             if ( sqrt( (double)( ( x - centerx ) * ( x - centerx ) +
//                                  ( y - centery ) * ( y - centery ) +
//                                  ( z - centerz ) * ( z - centerz ) ) ) < radius ) 
//             {
//                pmp.x[ 0 ] = ( int16_t )x;
//                pmp.x[ 1 ] = ( int16_t )y;
//                pmp.x[ 2 ] = ( int16_t )z;
//                model.insert( pmp );
//             }
         }
      }
   }
   return true;
}

bool US_PM::ellipsoid( int model_pos, vector < double > & params, vector < double > & params_left, set < pm_point > & model )
{
   debug( 1, "ellipsoid" );
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

void US_PM::debug( int level, QString qs )
{
   if ( level <= debug_level )
   {
      cout << qs.ascii() << endl;
   }
}

set < pm_point > US_PM::recenter( set < pm_point > & model )
{
   int cx = 0;
   int cy = 0;
   int cz = 0;

   for ( set < pm_point >::iterator it = model.begin();
         it != model.end();
         it++ )
   {
      cx += it->x[ 0 ];
      cy += it->x[ 1 ];
      cz += it->x[ 2 ];
   }

   cx /= model.size();
   cy /= model.size();
   cz /= model.size();


   set < pm_point > result;

   pm_point pmp;

   for ( set < pm_point >::iterator it = model.begin();
         it != model.end();
         it++ )
   {
      pmp.x[ 0 ] = it->x[ 0 ] + ( int16_t ) cx;
      pmp.x[ 1 ] = it->x[ 1 ] + ( int16_t ) cy;
      pmp.x[ 2 ] = it->x[ 2 ] + ( int16_t ) cz;
      result.insert( pmp );
   }
   return result;
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
      case 2 : // ellipsoid
      case 3 : // torus
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
      case 2 : // ellipsoid
      case 3 : // torus
      default:
         error_msg = QString( "join: object type %1 not defined" ).arg( types[ i ] );
         return false;
         break;
      }
   }
   return true;
}

QString US_PM::test()
{
   vector < double > q;
   vector < double > I;
   vector < double > e;

   US_PM test_pm( 1e0, 0.1e0, q, I, e, 5 );

   vector < double > params;

   params.push_back( 0e0 ); // sphere 0
   params.push_back( 5e0 ); // radius in grid coordinates

   params.push_back( 0e0 ); // sphere 1
   params.push_back( 5e0 ); // radius in grid coordinates
   params.push_back( 10e0 ); // centerx

   params.push_back( 0e0 ); // sphere 2
   params.push_back( 5e0 ); // radius in grid coordinates
   params.push_back( 10e0 ); // centerx
   params.push_back( 20e0 ); // centery

   params.push_back( 0e0 ); // sphere 3
   params.push_back( 5e0 ); // radius in grid coordinates
   params.push_back( -10e0 ); // centerx
   params.push_back( -20e0 ); // centery
   params.push_back( -20e0 ); // centerz

   set < pm_point > model;

   if ( !test_pm.create_model( params, model ) )
   {
      cout << test_pm.error_msg.ascii() << endl;
   }

   vector < int    > types;
   vector < double > fparams;
   US_Vector::printvector( "before split: params", params ); 
   if ( !test_pm.split( params, types, fparams ) )
   {
      cout << test_pm.error_msg.ascii() << endl;
   }
   US_Vector::printvector( "split: types", types ); 
   US_Vector::printvector( "split: fparams", fparams ); 

   vector < double > new_params;
   if ( !test_pm.join( new_params, types, fparams ) )
   {
      cout << test_pm.error_msg.ascii() << endl;
   }
   US_Vector::printvector( "after join: new_params", new_params ); 
   if ( new_params != params )
   {
      cout << "ERROR: new_params != params" << endl;
   }

   return ""; // test_pm.qs_bead_model( model );
}
