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
   bead_radius        = pow( cube_size / M_PI, 1e0/3e0 );
   bead_radius_over_2 = bead_radius * 5e-1;

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
   debug( 1, "create_model" );
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

QString US_PM::test( QString name )
{
   vector < double > q;
   vector < double > I;
   vector < double > e;

   vector < double > params;

   QFile f( name );

   double grid_conversion_factor = 1e0;
   double drho = 1e-1;

   if ( f.exists() && f.open( IO_ReadOnly ) )
   {
      QTextStream ts( &f );
      grid_conversion_factor = ts.readLine().stripWhiteSpace().toDouble();
      drho                   = ts.readLine().stripWhiteSpace().toDouble();
      while( !ts.atEnd() )
      {
         QString qs = ts.readLine().stripWhiteSpace();
         if ( !qs.isEmpty() )
         {
            params.push_back( qs.toDouble() );
         }
      }
   }
   f.close();


   US_Vector::printvector( "read params", params );
   
   US_PM test_pm( grid_conversion_factor, drho, q, I, e, 5 );

   if ( 0 )
   {
      // sphere test
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
   }

   if ( 0 )
   {
      // cylinder test

      params.push_back( 1e0 );  // cylinder 0
      params.push_back( 2e0 );  // radius in grid coordinates
      params.push_back( 10e0 ); // length

      params.push_back( 1e0 );  // cylinder 1
      params.push_back( 3e0 );  // radius in grid coordinates
      params.push_back( 0e0 ); // basex
      params.push_back( -10e0 ); // basey
      params.push_back( 0e0 ); // endx
      params.push_back( 12e0 ); // endy

      params.push_back( 1e0 );  // cylinder 2
      params.push_back( 4e0 );  // radius in grid coordinates
      params.push_back( 0e0 ); // basex
      params.push_back( 0e0 ); // basey
      params.push_back( -15e0 ); // basez
      params.push_back( 0e0 ); // endx
      params.push_back( 0e0 ); // endy
      params.push_back( 0e0 ); // endz
   }

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

   return test_pm.qs_bead_model( model );
}
