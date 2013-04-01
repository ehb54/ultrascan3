#include "../include/us_pm.h"

US_PM::US_PM( 
             double grid_conversion_factor, 
             int max_dimension, 
             double drho, 
             double buffer_e_density, 
             double ev, 
             unsigned int max_harmonics,
             // unsigned int fibonacci_grid, this is for hydration!
             vector < double > F, 
             vector < double > q, 
             vector < double > I, 
             vector < double > e, 
             int debug_level 
             )
{
   this->grid_conversion_factor = grid_conversion_factor;
   this->max_dimension          = abs( max_dimension );
   this->drho                   = drho;
   this->buffer_e_density       = buffer_e_density;
   this->ev                     = ev;
   this->max_harmonics          = max_harmonics;
   // this->fibonacci_grid         = fibonacci_grid;
   this->F                      = F;
   this->q                      = q;
   this->I                      = I;
   this->e                      = e;
   this->debug_level            = debug_level;

   this->max_dimension   = this->max_dimension < USPM_MAX_VAL ? this->max_dimension : USPM_MAX_VAL;

   if ( this->max_dimension != max_dimension )
   {
      cerr << QString( "Warning: maximum dimension requested %1 exceedes maximum allowed %2, reset to maximum allowed\n" )
         .arg( max_dimension )
         .arg( this->max_dimension )
         .ascii();
   }

   max_dimension_d = ( double ) this->max_dimension;
   one_over_grid_conversion_factor = 1e0 / grid_conversion_factor;

   cube_size   = grid_conversion_factor * grid_conversion_factor * grid_conversion_factor;

   // radius gives a sphere with equal size to the cube:
   bead_radius        = pow( cube_size / M_PI, 1e0/3e0 );
   bead_radius_over_2 = bead_radius * 5e-1;

   q_points           = ( unsigned int ) q.size();

   cout << QString( "US_PM:cube size   %1\n"
                    "US_PM:bead radius %2\n" )
      .arg( cube_size )
      .arg( bead_radius )
      .ascii();
   //    if ( fibonacci_grid > 2 )
   //    {
   //       cout << "Building Fibonacci grid\n";
   //       sh::build_grid( fib_grid, fibonacci_grid );
   //       printf( "Fibonacci of order %u is %u fib_grid.size() %u\n", 
   //               fibonacci_grid,
   //               sh::fibonacci( fibonacci_grid ),
   //               (unsigned int)fib_grid.size() 
   //               );
   //    }
   Z0 = complex < float > ( 0.0f, 0.0f );
   i_ = complex < float > ( 0.0f, 1.0f );

   i_l.resize( max_harmonics + 1 );
   for ( unsigned int l = 0; l <= max_harmonics; l++ )
   {
      i_l[ l ] = pow( i_, l );
   }

   Y_points = max_harmonics + 1 + ( max_harmonics ) * ( max_harmonics + 1 );
   J_points = ( 1 + max_harmonics ) * q_points;
   i_k.clear();
   for ( unsigned int l = 0; l <= max_harmonics; ++l )
   {
      for ( int m = - (int) l ; m <= (int) l; m++ )
      {
         i_k.push_back( i_l[ l ] );
      }
   }

   for ( unsigned int k = 0; k < Y_points; k++ )
   {
      A0.push_back( Z0 );
   }
   Av0.resize( q_points );
   I0.resize( q_points );
   for ( unsigned int i = 0; i < q_points; ++i )
   {
      Av0[ i ] = A0;
      I0 [ i ] = 0e0;
   }
   no_harmonics = max_harmonics + 1;

   q_Y_points = q_points * Y_points;

   for ( unsigned int i = 0; i < q_Y_points; ++i )
   {
      A1v0.push_back( Z0 );
   }
   cout << QString( "q_Y_points %1\n" ).arg( q_Y_points );
   cout << QString( "A1v0 size  %1\n" ).arg( A1v0.size() );

   ccY.resize( Y_points );
   ccJ.resize( J_points );
   ccA1v = A1v0;
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

bool US_PM::create_model( vector < double > params, set < pm_point > & model, bool only_last_model )
{
   debug( 1, QString( "create_model%1" ).arg( only_last_model ? ". Note: only last model saved" : "" ) );
   model.clear();

   vector < double > params_left;

   int model_pos = 0;

   while ( params.size() )
   {
      if ( only_last_model )
      {
         model.clear();
      }
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
   us_timers.init_timer( "recenter" );
   us_timers.start_timer( "recenter" );

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

   // cout << QString( "recenter: cx %1,%2,%3\n" ).arg( cx ).arg( cy ).arg( cz );

   set < pm_point > result;

   pm_point pmp;

   for ( set < pm_point >::iterator it = model.begin();
         it != model.end();
         it++ )
   {
      pmp.x[ 0 ] = it->x[ 0 ] - ( int16_t ) cx;
      pmp.x[ 1 ] = it->x[ 1 ] - ( int16_t ) cy;
      pmp.x[ 2 ] = it->x[ 2 ] - ( int16_t ) cz;
      result.insert( pmp );
   }
   us_timers.end_timer( "recenter" );
   cout << us_timers.list_time( "recenter" ).ascii();
   us_timers.clear_timer( "recenter" );
   return result;
}

bool US_PM::rotation_matrix( double l, double m, double n, double theta, vector < vector < double > > &rm )
{
   rm.clear();
   rm.resize( 3 );
   rm[ 0 ].resize( 3 );
   rm[ 1 ].resize( 3 );
   rm[ 2 ].resize( 3 );

   double c   = cos( theta );
   double s   = sin( theta );
   double omc = 1e0 - c;

   rm[ 0 ][ 0 ] = l * l * omc + c;
   rm[ 0 ][ 1 ] = m * l * omc - n * s;
   rm[ 0 ][ 2 ] = n * l * omc + m * s;

   rm[ 1 ][ 0 ] = l * m * omc + n * s;
   rm[ 1 ][ 1 ] = m * m * omc + c;
   rm[ 1 ][ 2 ] = n * m * omc - l * s;

   rm[ 2 ][ 0 ] = l * n * omc - m * s;
   rm[ 2 ][ 1 ] = m * n * omc + l * s;
   rm[ 2 ][ 2 ] = n * n * omc + c;
   return true;
}

bool US_PM::apply_rotation_matrix( vector < vector < double > > &rm, int x, int y, int z, double & newx, double & newy, double & newz )
{
   newx = ( double ) x * rm[ 0 ][ 0 ] + ( double ) y * rm[ 0 ][ 1 ] + ( double ) z * rm[ 0 ][ 2 ];
   newy = ( double ) x * rm[ 1 ][ 0 ] + ( double ) y * rm[ 1 ][ 1 ] + ( double ) z * rm[ 1 ][ 2 ];
   newz = ( double ) x * rm[ 2 ][ 0 ] + ( double ) y * rm[ 2 ][ 1 ] + ( double ) z * rm[ 2 ][ 2 ];

   return true;
}

bool US_PM::check_limits( vector < double > & fparams, vector < double > & low_fparams, vector < double > & high_fparams )
{
   for ( unsigned int i = 0; i < ( unsigned int )fparams.size(); i++ )
   {
      if ( fparams[ i ] < low_fparams[ i ] ||
           fparams[ i ] > high_fparams[ i ] )
      {
         return false;
      }
   }
   return true;
}

bool US_PM::clip_limits( vector < double > & fparams, vector < double > & low_fparams, vector < double > & high_fparams )
{
   bool clipped = false;
   for ( unsigned int i = 0; i < ( unsigned int )fparams.size(); i++ )
   {
      if ( fparams[ i ] < low_fparams[ i ] )
      {
         fparams[ i ] = low_fparams[ i ];
         clipped = true;
      }
      if ( fparams[ i ] > high_fparams[ i ] )
      {
         fparams[ i ] = high_fparams[ i ];
         clipped = true;
      }
   }
   return clipped;
}

