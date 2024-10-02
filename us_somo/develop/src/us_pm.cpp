#include "../include/us_pm.h"
//Added by qt3to4:
#include <QTextStream>

bool US_PM::set_grid_size( double grid_conversion_factor, bool quiet )
{
   /*
#if defined( USE_MPI )
   if ( us_log )
   {
      us_log->log( QString( "%1: Set grid size: %2\n" ).arg( myrank ).arg( grid_conversion_factor ) );
   }
#endif
   */

   if ( grid_conversion_factor < 0.5 )
   {
      error_msg = QString( "set_grid_size requested size %1 is less than the minimum allowed 0.5" ).arg( grid_conversion_factor );
      return false;
   }

   // be careful with this routine!
   // have to clear because any rtp data is now invalid
   clear( );
   this->grid_conversion_factor = grid_conversion_factor;
   one_over_grid_conversion_factor = 1e0 / grid_conversion_factor;
   cube_size   = grid_conversion_factor * grid_conversion_factor * grid_conversion_factor;

   // radius gives a sphere with equal size to the cube:
   bead_radius              = pow( cube_size / M_PI, 1e0/3e0 );
   bead_radius_over_2gcf     = ( bead_radius * 5e-1 ) / grid_conversion_factor;

   if ( !quiet &&  us_log )
   {
      us_log->log( QString( "US_PM:cube size   %1\n"
                            "US_PM:bead radius %2\n" )
                   .arg( cube_size )
                   .arg( bead_radius )
                   );
   }

   double conv_F = cube_size / org_cube_size;
   for ( int i = 0; i < (int)F.size(); ++i )
   {
      F[ i ] = org_F[ i ] * conv_F;
   }
   max_dimension = org_max_dimension / grid_conversion_factor;
   return true;
}

bool US_PM::reset_grid_size( bool quiet )
{
   return set_grid_size( org_conversion_factor, quiet );
}

US_PM::US_PM( 
             double            grid_conversion_factor, 
             int               max_dimension, 
             unsigned int      max_harmonics,
             vector < double > F, 
             vector < double > q, 
             vector < double > I, 
             vector < double > e,
             unsigned int      max_mem_in_MB,
             int               debug_level,
             bool              quiet,
             US_Log          * us_log,
             US_Udp_Msg      * us_udp_msg
             )
{
   // this->grid_conversion_factor = grid_conversion_factor;

   this->max_dimension          = abs( max_dimension );
   //    this->drho                   = drho;
   //    this->buffer_e_density       = buffer_e_density;
   //    this->ev                     = ev;
   this->max_harmonics          = max_harmonics;
   // this->fibonacci_grid         = fibonacci_grid;
   this->F                      = F;
   this->q                      = q;
   this->I                      = I;
   this->e                      = e;
   this->max_mem_in_MB          = max_mem_in_MB;
   this->debug_level            = debug_level;
   this->quiet                  = quiet;
   this->us_log                 = us_log;
   this->us_udp_msg             = us_udp_msg;

   us_log_started               = false;
   if ( !us_log )
   {
      this->us_log = new US_Log( "pmlog.txt", true, QIODevice::Append );
      us_log_started = true;
   }
   this->us_log->datetime( "start us_pm" );

   this->max_dimension   = this->max_dimension < USPM_MAX_VAL ? this->max_dimension : USPM_MAX_VAL;
   if ( this->max_dimension != max_dimension )
   {
      if ( us_log )
      {
         us_log->log( QString( "Warning: maximum dimension requested %1 exceedes maximum allowed %2, reset to maximum allowed\n" )
                      .arg( max_dimension )
                      .arg( this->max_dimension ) );
      }
   }

   org_F                        = F;
   org_conversion_factor        = grid_conversion_factor;
   org_cube_size                = grid_conversion_factor * grid_conversion_factor * grid_conversion_factor;
   org_max_dimension            = max_dimension;
   set_grid_size                ( grid_conversion_factor, quiet );

   max_dimension_d = ( double ) this->max_dimension;

   // replaced by set_grid_size()
   //    one_over_grid_conversion_factor = 1e0 / grid_conversion_factor;
   //    cube_size   = grid_conversion_factor * grid_conversion_factor * grid_conversion_factor;
   //    // radius gives a sphere with equal size to the cube:
   //    bead_radius        = pow( cube_size / M_PI, 1e0/3e0 );
   //    bead_radius_over_2 = bead_radius * 5e-1;
   //    cout << QString( "US_PM:cube size   %1\n"
   //                     "US_PM:bead radius %2\n" )
   //       .arg( cube_size )
   //       .arg( bead_radius )
   //       ;

   q_points           = ( unsigned int ) q.size();

   // fib grid is used for hydration
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
   i_k.clear( );
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
   if ( !quiet && us_log )
   {
      us_log->log( 
                  QString( "q_Y_points %1\n" ).arg( q_Y_points ) +
                  QString( "A1v0 size  %1\n" ).arg( A1v0.size() )
                   );
   }

   ccY.resize( Y_points );
   ccJ.resize( J_points );
   ccA1v = A1v0;

   use_errors = e.size() == q.size();

   if ( use_errors )
   {
      bool any_non_zero = false;
      for ( unsigned int i = 0; i < e.size(); i++ )
      {
         if ( e[ i ] )
         {
            any_non_zero = true;
            break;
         }
      }
      for ( unsigned int i = 0; i < e.size(); i++ )
      {
         if ( !e[ i ] )
         {
            use_errors = false;
            break;
         }
      }
      if ( !quiet && us_log && !use_errors && any_non_zero )
      {
         us_log->log( "Notice: SD's provided but some were zero, so SD fitting is turned off\n" );
      }
   }

   if ( use_errors )
   {
      oneoversd2.resize( q_points );
      for ( unsigned int i = 0; i < q_points; ++i )
      {
         oneoversd2[ i ] = 1e0 / ( e[ i ] * e[ i ] );
      }
   } else {
      oneoversd2.clear( );
   }

   // memory computations

   bytes_per_pm_data =
      sizeof( pm_data ) +
      J_points * sizeof ( double ) +
      Y_points * sizeof ( float ) * 2;

   bytes_per_pmc_data =
      sizeof( pmc_data ) +
      q_Y_points * sizeof ( float ) * 2;

   unsigned int base_mem = 20;

   max_beads_CYJ =  ( 1024 * 1024 * ( max_mem_in_MB - base_mem ) ) / bytes_per_pm_data;
   max_beads_CA  =  ( 1024 * 1024 * ( max_mem_in_MB - base_mem ) ) / bytes_per_pmc_data;

   if ( !quiet && us_log )
   {
      us_log->log( QString( "bytes per pm_data %1\n" ).arg( bytes_per_pm_data ).toLatin1().data() );
      us_log->log( QString( "bytes per pmc data %1\n" ).arg( bytes_per_pmc_data ).toLatin1().data() );

      us_log->log( QString( "Memory max %1 MB\n" ).arg( max_mem_in_MB ).toLatin1().data() );
      us_log->log( QString( "Memory available %1 MB\n" ).arg( max_mem_in_MB - base_mem ).toLatin1().data() );
      us_log->log( QString( "max beads CYJ %1\n" ).arg( max_beads_CYJ ).toLatin1().data() );
      us_log->log( QString( "max beads CA %1\n" ).arg( max_beads_CA ).toLatin1().data() );
   }

   use_CYJ = false;

   set_best_delta();
   init_objects();
   if ( !quiet && us_log )
   {
      us_log->log( list_object_info() );
   }
   last_best_rmsd_ok = false;

   ga_I_result.resize( q_points );
   ga_set_params();
   shs = (SHS_USE *)0;
   shs = new SHS_USE( max_harmonics );
   pm_ga_fitness_secs = 0e0;
   pm_ga_fitness_calls = 0;
   ga_delta_ok = false;
}

US_PM::~US_PM()
{
   if ( shs )
   {
      delete shs;
   }
   if ( us_log )
   {
      us_log->datetime( "end us_pm" );
      if ( us_log_started )
      {
         delete us_log;
      }
   }
}

void US_PM::clear( )
{
   pcdata.clear( );
   pdata.clear( );
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

   qs += "\n";
   qs += physical_stats( model );
   if ( last_best_rmsd_ok )
   {
      qs += QString( "%1 to target data: %2\n" ).arg( use_errors ? "Chi^2" : "RMSD" ).arg( sqrt( last_best_rmsd2 ) );
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
      tmp_atom.all_beads               .clear( );
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

vector < point > US_PM::point_model( set < pm_point > & model )
{
   vector < point > result;

   for ( set < pm_point >::iterator it = model.begin();
         it != model.end();
         it++ )
   {
      point tmp_point;
      tmp_point.axis[0]      = it->x[ 0 ] * grid_conversion_factor;
      tmp_point.axis[1]      = it->x[ 1 ] * grid_conversion_factor;
      tmp_point.axis[2]      = it->x[ 2 ] * grid_conversion_factor;
      result.push_back( tmp_point );
   }

   return result;
}

bool US_PM::create_model( vector < double > params, set < pm_point > & model, bool only_last_model )
{
   debug( 1, QString( "create_model%1" ).arg( only_last_model ? ". Note: only last model saved" : "" ) );
   model.clear( );

   vector < double > params_left;

   int model_pos = 0;

   while ( params.size() )
   {
      if ( only_last_model )
      {
         model.clear( );
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
   if ( us_log && level <= debug_level )
   {
      us_log->log( qs.toLatin1().data() );
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
   if ( us_log )
   {
      us_log->log( us_timers.list_time( "recenter" ).toLatin1().data() );
   }
   us_timers.clear_timer( "recenter" );
   return result;
}

bool US_PM::rotation_matrix( double l, double m, double n, double theta, vector < vector < double > > &rm )
{
   rm.clear( );
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

bool US_PM::qstring_model( QString & out, set < pm_point > & model )
{
   double rg = Rg( model );
   out = qs_bead_model( model );
   out += "Model scale (10^-x m) (10 = Angstrom, 9 = nanometer), where x is : 10\n";
   out += QString( "Rg: %1\n" ).arg( rg );
   return true;
}

bool US_PM::qstring_model( QString & out, set < pm_point > & model, vector < double > & params )
{
   double rg = Rg( model );
   out = qs_bead_model( model );
   out += "Model scale (10^-x m) (10 = Angstrom, 9 = nanometer), where x is : 10\n";
   out += QString( "Rg: %1\n" ).arg( rg );
   out += list_params( params );
   return true;
}

bool US_PM::write_model( QString & filename, set < pm_point > & model, bool overwrite )
{
   if ( !overwrite )
   {
      int ext = 0;
      QString use_filename = QString( "%1.bead_model" ).arg( filename );
      while( QFile::exists( use_filename ) )
      {
         use_filename = QString( "%1-%2.bead_model" ).arg( filename ).arg( ++ext );
      }
      filename = use_filename;
   }

   if ( !filename.contains( QRegExp( "\\.bead_model$" ) ) )
   {
      filename += ".bead_model";
   }

   double rg = Rg( model );

   if ( us_log )
   {
      us_log->log( QString( "Creating (%1 beads, Rg %2): %3\n" ).arg( model.size() ).arg( rg ).arg( filename ) );
   }

   QFile of( filename );
   if ( !of.open( QIODevice::WriteOnly ) )
   {
      filename.replace( QRegExp( "\\.bead_model$" ), "" );
      return false;
   }
   
   QTextStream ts( &of );
   ts << qs_bead_model( model );
   ts << "Model scale (10^-x m) (10 = Angstrom, 9 = nanometer), where x is : 10\n";
   ts << QString( "Rg: %1\n" ).arg( rg );
   of.close();
   filename.replace( QRegExp( "\\.bead_model$" ), "" );
   return true;
}

bool US_PM::write_model( QString & filename, set < pm_point > & model, vector < double > & params, bool overwrite )
{
   if ( !overwrite )
   {
      int ext = 0;
      QString use_filename = QString( "%1.bead_model" ).arg( filename );
      while( QFile::exists( use_filename ) )
      {
         use_filename = QString( "%1-%2.bead_model" ).arg( filename ).arg( ++ext );
      }
      filename = use_filename;
   }

   if ( !filename.contains( QRegExp( "\\.bead_model$" ) ) )
   {
      filename += ".bead_model";
   }

   double rg = Rg( model );

   if ( us_log )
   {
      us_log->log( QString( "Creating (%1 beads, Rg %2): %3\n" ).arg( model.size() ).arg( rg ).arg( filename ) );
   }

   QFile of( filename );
   if ( !of.open( QIODevice::WriteOnly ) )
   {
      filename.replace( QRegExp( "\\.bead_model$" ), "" );
      return false;
   }
   
   QTextStream ts( &of );
   ts << qs_bead_model( model );
   ts << "Model scale (10^-x m) (10 = Angstrom, 9 = nanometer), where x is : 10\n";
   ts << QString( "Rg: %1\n" ).arg( rg );
   ts << list_params( params );
   of.close();
   filename.replace( QRegExp( "\\.bead_model$" ), "" );
   return true;
}

bool US_PM::write_I( QString & filename, set < pm_point > & model, bool overwrite )
{

   vector < double >   I_result( q.size() );
   if ( !compute_I( model, I_result ) )
   {
      if ( us_log )
      {
         us_log->log( "write_I:" + error_msg );
      }
      return false;
   }

   if ( !overwrite )
   {
      int ext = 0;
      QString use_filename = QString( "%1.dat" ).arg( filename );
      while( QFile::exists( use_filename ) )
      {
         use_filename = QString( "%1-%2.dat" ).arg( filename ).arg( ++ext );
      }
      filename = use_filename;
   }

   if ( !filename.contains( QRegExp( "\\.dat$" ) ) )
   {
      filename += ".dat";
   }

   if ( us_log )
   {
      us_log->log( "Creating:" + filename + "\n" );
   }
   QFile of( filename );

   if ( !of.open( QIODevice::WriteOnly ) )
   {
      filename.replace( QRegExp( "\\.dat$" ), "" );
      return false;
   }
   
   QTextStream ts( &of );
   ts << "# US-SOMO PM .dat file containing I(q) computed on bead model\n";
   for ( unsigned int i = 0; i < ( unsigned int ) q.size(); i++ )
   {
      ts << QString( "%1\t%2\n" ).arg( q[ i ], 0, 'e', 6 ).arg( I_result[ i ], 0, 'e', 6 );
   }

   last_written_I = I_result;

   of.close();
   filename.replace( QRegExp( "\\.dat$" ), "" );
   return true;
}

QString US_PM::tmp_name( QString basename, vector < double > &params )
{
   QString fname = QString( "/tmp/somo/%1%2_" ).arg( basename ).arg( object_names[ (int) params[ 0 ] ] );
   for ( int i = 1; i <= (int)object_m0_parameters[ (int) params[ 0 ] ]; ++i )
   {
      fname += QString( "_p%1" ).arg( params[ i ] ).replace( ".", "_" );
   }
   return fname;
}

QString US_PM::physical_stats( set < pm_point > & model )
{
   if ( !model.size() )
   {
      return "empty model\n";
   }

   // run pat? i.e. do a principal axis transformation

   vector < pm_point_f > adjusted_model;
   pm_point_f            tmp_point;

   {
      vector < dati1_supc > in_dt;

      for ( set < pm_point >::iterator it = model.begin();
            it != model.end();
            it++ )
      {
         dati1_supc dt;

         dt.x  = it->x[ 0 ] * grid_conversion_factor;
         dt.y  = it->x[ 1 ] * grid_conversion_factor;
         dt.z  = it->x[ 2 ] * grid_conversion_factor;
         dt.r  = bead_radius;
         dt.ru = bead_radius;
         dt.m  = ( 4.0 / 3.0 ) * M_PI * bead_radius * bead_radius * bead_radius;

         in_dt.push_back( dt );
      }

      int out_nat;
      vector < dati1_pat > out_dt( in_dt.size() + 1 );

      if ( !us_hydrodyn_pat_main( ( int ) in_dt.size(),
                                  ( int ) in_dt.size(),
                                  &( in_dt[ 0 ] ),
                                  &out_nat,
                                  &( out_dt[ 0 ] ) ) )
      {
         // cout << QString( "pat ok, out_nat %1\n" ).arg( out_nat );
         for ( unsigned int i = 0; i < model.size(); ++i )
         {
            tmp_point.x[ 0 ] = out_dt[ i ].x;
            tmp_point.x[ 1 ] = out_dt[ i ].y;
            tmp_point.x[ 2 ] = out_dt[ i ].z;
            adjusted_model.push_back( tmp_point );
         }
      }
   }

   // also compute intersection volumes & subtract?
   // every pair, remove intersection volume ?

   // compute average, center of electron density?

   double volume = 0e0;
   // double volume_intersection = 0e0;
   point pmin;
   point pmax;
   point prmin;
   point prmax;

   // just to stop silly compiler warning (may be used uninitialized in this function)
   for ( unsigned int j = 0; j < 3; j++ )
   {
      pmin.axis[ j ] =
         pmax.axis[ j ] =
         prmin.axis[ j ] =
         prmax.axis[ j ] = 0.0f;
   }

   // subtract each radius from min add to max?
   // possible alternate bounding box ...

   for ( unsigned int i = 0; i < ( unsigned int ) adjusted_model.size(); ++i )
   {
      if ( i )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( pmin.axis[ j ] > adjusted_model[ i ].x[ j ] )
            {
               pmin.axis[ j ] = adjusted_model[ i ].x[ j ];
            }
            if ( pmax.axis[ j ] < adjusted_model[ i ].x[ j ] )
            {
               pmax.axis[ j ] = adjusted_model[ i ].x[ j ];
            }

            if ( prmin.axis[ j ] > adjusted_model[ i ].x[ j ] - bead_radius )
            {
               prmin.axis[ j ] = adjusted_model[ i ].x[ j ] - bead_radius;
            }
            if ( prmax.axis[ j ] < adjusted_model[ i ].x[ j ] + bead_radius )
            {
               prmax.axis[ j ] = adjusted_model[ i ].x[ j ] + bead_radius;
            }
         }
      } else {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            pmin .axis[ j ] = adjusted_model[ i ].x[ j ];
            pmax .axis[ j ] = adjusted_model[ i ].x[ j ];
            prmin.axis[ j ] = adjusted_model[ i ].x[ j ];
            prmax.axis[ j ] = adjusted_model[ i ].x[ j ];
            prmin.axis[ j ] -= bead_radius;
            prmax.axis[ j ] += bead_radius;
         }
      }

      // not needed since each bead is of equal volume
      // volume += ( 4e0 / 3e0 ) * M_PI * bead_radius * bead_radius * bead_radius;
   }

   volume = (double) model.size() * cube_size;

   // no intersection volume needed since each bead fills the volume of the cube
   //    for ( unsigned int i = 0; i < ( unsigned int ) adjusted_model.size() - 1; ++i )
   //    {
   //       for ( unsigned int j = i + 1; j < ( unsigned int ) adjusted_model.size(); ++j )
   //       {
   //          float d = dist( bm[ i ].bead_coordinate , bm[ j ].bead_coordinate );
   //          if ( d < bead_radius + bead_radius )
   //          {
   //             float r1 = bead_radius;
   //             float r  = bead_radius;
   //             if ( d > 0.0 )
   //             {
   //                volume_intersection +=
   //                   M_PI * ( r1 + r - d ) * ( r1 + r - d ) *
   //                   ( d * d + 
   //                     2.0 * d * r + 2.0 * d * r1 
   //                     - 3.0 * r * r  - 3.0 * r1 * r1 
   //                     + 6.0 * r * r1 ) 
   //                   /
   //                   ( 12.0 * d );
   //             } else {
   //                // one is within the other:
   //                if ( r > r1 )
   //                {
   //                   r = r1;
   //                }
   //                volume_intersection +=
   //                   ( 4.0 / 3.0 ) * M_PI * r * r * r;
   //             }
   //          }
   //       }
   //    }


   last_physical_stats.clear( );

   last_physical_stats[ "result total volume"                ] = QString( "%1" ).arg( volume );
   // last_physical_stats[ "result intersection volume"         ] = QString( "%1" ).arg( volume_intersection );
   // last_physical_stats[ "result excluded volume"             ] = QString( "%1" ).arg( volume - volume_intersection );
   last_physical_stats[ "result centers bounding box size x" ] = QString( "%1" ).arg( pmax.axis[ 0 ] - pmin.axis[ 0 ] );
   last_physical_stats[ "result centers bounding box size y" ] = QString( "%1" ).arg( pmax.axis[ 1 ] - pmin.axis[ 1 ] );
   last_physical_stats[ "result centers bounding box size z" ] = QString( "%1" ).arg( pmax.axis[ 2 ] - pmin.axis[ 2 ] );

   double approx_Dmax = prmax.axis[ 0 ] - prmin.axis[ 0 ];
   for ( int i = 1; i < 3; ++i )
   {
      if ( approx_Dmax < prmax.axis[ i ] - prmin.axis[ i ] )
      {
         approx_Dmax = prmax.axis[ i ] - prmin.axis[ i ];
      }
   }
   last_physical_stats[ "approx. Dmax" ] = QString( "%1" ).arg( approx_Dmax );
   last_physical_stats[ "approx. Shannon channels" ] = QString( "%1" ).arg( approx_Dmax * q.back() / M_PI );

   last_physical_stats[ "result centers axial ratios x:z" ] = QString( "%1" )
      .arg( ( pmax.axis[ 0 ] - pmin.axis[ 0 ] ) / ( pmax.axis[ 2 ] - pmin.axis[ 2 ] ) );
   last_physical_stats[ "result centers axial ratios x:y" ] = QString( "%1" )
      .arg( ( pmax.axis[ 0 ] - pmin.axis[ 0 ] ) / ( pmax.axis[ 1 ] - pmin.axis[ 1 ] ) );
   last_physical_stats[ "result centers axial ratios y:z" ] = QString( "%1" )
      .arg( ( pmax.axis[ 1 ] - pmin.axis[ 1 ] ) / ( pmax.axis[ 2 ] - pmin.axis[ 2 ] ) );

   last_physical_stats[ "result radial extent bounding box size x" ] = QString( "%1" ).arg( prmax.axis[ 0 ] - prmin.axis[ 0 ] );
   last_physical_stats[ "result radial extent bounding box size y" ] = QString( "%1" ).arg( prmax.axis[ 1 ] - prmin.axis[ 1 ] );
   last_physical_stats[ "result radial extent bounding box size z" ] = QString( "%1" ).arg( prmax.axis[ 2 ] - prmin.axis[ 2 ] );

   last_physical_stats[ "result radial extent axial ratios x:z" ] = QString( "%1" )
      .arg( ( prmax.axis[ 0 ] - prmin.axis[ 0 ] ) / ( prmax.axis[ 2 ] - prmin.axis[ 2 ] ) );
   last_physical_stats[ "result radial extent axial ratios x:y" ] = QString( "%1" )
      .arg( ( prmax.axis[ 0 ] - prmin.axis[ 0 ] ) / ( prmax.axis[ 1 ] - prmin.axis[ 1 ] ) );
   last_physical_stats[ "result radial extent axial ratios y:z" ] = QString( "%1" )
      .arg( ( prmax.axis[ 1 ] - prmin.axis[ 1 ] ) / ( prmax.axis[ 2 ] - prmin.axis[ 2 ] ) );

   QString qs;

   qs += QString( 
                 "total        volume (A^3) %1\n" 
                 // "intersection volume (A^3) %2\n" 
                 // "excluded     volume (A^3) %3\n" 
                 )
      .arg( volume )
      // .arg( volume_intersection )
      // .arg( volume - volume_intersection );
      ;

   qs += QString( 
                  "centers bounding box size (A) %1 %2 %2\n" 
                  )
      .arg( pmax.axis[ 0 ] - pmin.axis[ 0 ] )
      .arg( pmax.axis[ 1 ] - pmin.axis[ 1 ] )
      .arg( pmax.axis[ 2 ] - pmin.axis[ 2 ] );

   qs += QString( 
                  "centers axial ratios: [x:z] = %1  [x:y] = %2  [y:z] = %3\n"
                  )
      .arg( ( pmax.axis[ 0 ] - pmin.axis[ 0 ] ) / ( pmax.axis[ 2 ] - pmin.axis[ 2 ] ) )
      .arg( ( pmax.axis[ 0 ] - pmin.axis[ 0 ] ) / ( pmax.axis[ 1 ] - pmin.axis[ 1 ] ) )
      .arg( ( pmax.axis[ 1 ] - pmin.axis[ 1 ] ) / ( pmax.axis[ 2 ] - pmin.axis[ 2 ] ) );

   qs += QString( 
                  "radial extent bounding box size (A) %1 %2 %2\n" 
                  )
      .arg( prmax.axis[ 0 ] - prmin.axis[ 0 ] )
      .arg( prmax.axis[ 1 ] - prmin.axis[ 1 ] )
      .arg( prmax.axis[ 2 ] - prmin.axis[ 2 ] );

   qs += QString( 
                  "radial extent axial ratios: [x:z] = %1  [x:y] = %2  [y:z] = %3\n"
                  )
      .arg( ( prmax.axis[ 0 ] - prmin.axis[ 0 ] ) / ( prmax.axis[ 2 ] - prmin.axis[ 2 ] ) )
      .arg( ( prmax.axis[ 0 ] - prmin.axis[ 0 ] ) / ( prmax.axis[ 1 ] - prmin.axis[ 1 ] ) )
      .arg( ( prmax.axis[ 1 ] - prmin.axis[ 1 ] ) / ( prmax.axis[ 2 ] - prmin.axis[ 2 ] ) );

   qs += QString( "approx. Dmax %1\n" ).arg( approx_Dmax );
   qs += QString( "approx. Shannon channels %1\n" ).arg( approx_Dmax * q.back() / M_PI );

   return qs;
}


bool US_PM::zero_md0_params( vector < double > & params, double max_d )
{
   
   if ( params.size() < 1 )
   {
      error_msg = "zero_md0_params: params[0] must be set";
      return false;
   }
   int this_type = (int) params[ 0 ];
   if ( this_type >= (int) object_names.size() ||
        this_type < 0 )
   {
      error_msg = "zero_md0_params: params[0] invalid param";
      return false;
   }
   
   params.resize( 1 + object_m0_parameters[ this_type ] );
   params[ 0 ] = (double) this_type;

   vector < double > low_fparams;
   vector < double > high_fparams;
   set_limits( params, low_fparams, high_fparams, max_d );

   for ( int i = 0; i < object_m0_parameters[ this_type ]; i++ )
   {
      if ( low_fparams[ i ] >= 0e0 )
      {
         params[ i + 1 ] = low_fparams[ i ];
      } else {
         if ( high_fparams[ i ] <= 0e0 )
         {
            params[ i + 1 ] = high_fparams[ i ];
         } else {
            params[ i + 1 ] = 0e0;
         }
      }
   }
   return true;
}      

void US_PM::random_md0_params( vector < double > & params, double max_d )
{
   
   int this_type = (int)( drand48() * ( double ) object_names.size() );

   
   vector < double > low_fparams;
   vector < double > high_fparams;
   
   params.resize( 1 + object_m0_parameters[ this_type ] );
   params[ 0 ] = (double) this_type;

   set_limits( params, low_fparams, high_fparams, max_d );

   for ( int i = 0; i < object_m0_parameters[ this_type ]; i++ )
   {
      params[ i + 1 ] = drand48() * ( high_fparams[ i ] - low_fparams[ i ] ) + low_fparams[ i ];
   }
}      

double US_PM::Rg( set < pm_point > & model )
{
   if ( !model.size() )
   {
      return 0e0;
   }
         
   vector < point > pmodel = point_model( model );

   double Rg2 = 0e0;

   point cm;
   cm.axis[ 0 ] = 0.0f;
   cm.axis[ 1 ] = 0.0f;
   cm.axis[ 2 ] = 0.0f;

   for ( int i = 0; i < (int) pmodel.size(); i++ )
   {
      cm.axis[ 0 ] += pmodel[ i ].axis[ 0 ];
      cm.axis[ 1 ] += pmodel[ i ].axis[ 1 ];
      cm.axis[ 2 ] += pmodel[ i ].axis[ 2 ];
   }

   cm.axis[ 0 ] /= ( float ) pmodel.size();
   cm.axis[ 1 ] /= ( float ) pmodel.size();
   cm.axis[ 2 ] /= ( float ) pmodel.size();


   for ( int i = 0; i < (int) pmodel.size(); i++ )
   {
      Rg2 +=  
         ( 
          ( pmodel[ i ].axis[ 0 ] - cm.axis[ 0 ]) *
          ( pmodel[ i ].axis[ 0 ] - cm.axis[ 0 ] ) +
          ( pmodel[ i ].axis[ 1 ] - cm.axis[ 1 ] ) *
          ( pmodel[ i ].axis[ 1 ] - cm.axis[ 1 ] ) +
          ( pmodel[ i ].axis[ 2 ] - cm.axis[ 2 ] ) *
          ( pmodel[ i ].axis[ 2 ] - cm.axis[ 2 ] ) 
          );
   }

   return sqrt( Rg2 / (double) pmodel.size() );
}

bool US_PM::rescale_params( vector < int >    & types, 
                            vector < double > & fparams, 
                            double              new_conversion_factor )
{
   double scale_factor = grid_conversion_factor / new_conversion_factor;
   int fpos = 0;
   for ( int i = 0; i < (int)types.size(); ++i )
   {
      int pos = i < (int)object_parameter_types[ types[ i ] ].size() ? i : object_parameter_types[ types[ i ] ].size() - 1;
      for ( int k = 0; k < (int)object_parameter_types[ types[ i ] ][ pos ].size(); ++k, ++fpos )
      {
         switch( object_parameter_types[ types[ i ] ][ pos ][ k ] )
         {
         case COORD:
            fparams [ fpos ] *= scale_factor;
            break;
         case RADIUS:
            fparams [ fpos ] += bead_radius_over_2gcf;
            fparams [ fpos ] *= scale_factor;
            break;
         default:
            // no conversion on NORM or ANGLE
            break;
         }
      }
   }
   return true;
}

bool US_PM::rescale_params( vector < double > & params,
                            vector < double > & low_fparams, 
                            vector < double > & high_fparams, 
                            double              new_conversion_factor,
                            double              refinement_range_pct )
{
   if ( us_log )
   {
      us_log->log( QString( "rescale_params: bead_radius            %1\n"
                    "                bead_radius_over_2gcf   %2\n"
                    "                grid_conversion_factor %3\n"
                    "                new_conversion_factor  %4\n" )
      .arg( bead_radius )
      .arg( bead_radius_over_2gcf )
      .arg( grid_conversion_factor )
      .arg( new_conversion_factor )
                    );
   }

   vector < int >    types;
   vector < double > fparams;
   if ( !split( params, types, fparams ) )
   {
      return false;
   }

   double scale_factor = grid_conversion_factor / new_conversion_factor;
   int fpos = 0;
   for ( int i = 0; i < (int)types.size(); ++i )
   {
      int pos = i < (int)object_parameter_types[ types[ i ] ].size() ? i : object_parameter_types[ types[ i ] ].size() - 1;
      for ( int k = 0; k < (int)object_parameter_types[ types[ i ] ][ pos ].size(); ++k, ++fpos )
      {
         switch( object_parameter_types[ types[ i ] ][ pos ][ k ] )
         {
         case COORD:
            low_fparams [ fpos ] = scale_factor * ( fparams[ fpos ] - grid_conversion_factor * refinement_range_pct / 100e0 );
            high_fparams[ fpos ] = scale_factor * ( fparams[ fpos ] + grid_conversion_factor * refinement_range_pct / 100e0 );

            break;

         case RADIUS:
            low_fparams [ fpos ] = scale_factor * ( ( fparams[ fpos ] - bead_radius_over_2gcf ) - grid_conversion_factor * refinement_range_pct / 100e0 );
            high_fparams[ fpos ] = scale_factor * ( ( fparams[ fpos ] + bead_radius_over_2gcf ) + grid_conversion_factor * refinement_range_pct / 100e0 );

            break;
         default:
            // no scale conversion on NORM or ANGLE
            low_fparams [ fpos ] = fparams[ fpos ] * ( 1e0 - refinement_range_pct / 100e0 );
            high_fparams[ fpos ] = fparams[ fpos ] * ( 1e0 + refinement_range_pct / 100e0 );
            break;
         }
         if ( low_fparams[ fpos ] > high_fparams[ fpos ] )
         {
            double tmp = low_fparams[ fpos ];
            low_fparams [ fpos ] = high_fparams[ fpos ];
            high_fparams[ fpos ] = tmp;
         }
      }
   }
   return true;
}

bool US_PM::zero_params( vector < double > & params, vector < int > & types )
{
   params.clear( );
   for ( int i = 0; i < (int)types.size(); ++i )
   {
      params.push_back( types[ i ] );
      int pos = i < (int)object_parameter_types[ types[ i ] ].size() ? i : object_parameter_types[ types[ i ] ].size() - 1;
      for ( int k = 0; k < (int)object_parameter_types[ types[ i ] ][ pos ].size(); ++k )
      {
         switch( object_parameter_types[ types[ i ] ][ pos ][ k ] )
         {
         case COORD :
            params .push_back( 0e0 );
            break;

         case NORM :
            params .push_back( 0e0 );
            break;

         case RADIUS :
            params .push_back( delta_min );
            break;

         case ANGLE :
            params .push_back( 0e0 );
            break;

         default :
            error_msg = "zero_params: unknown object parameter type";
            return false;
            break;
         }
      }
   }
   return true;
}

bool US_PM::random_params( vector < double > & params, vector < int > & types, double max_d )
{
   params.clear( );
   if ( !max_d )
   {
      max_d = max_dimension_d;
   }

   for ( int i = 0; i < (int)types.size(); ++i )
   {
      params.push_back( types[ i ] );
      int pos = i < (int)object_parameter_types[ types[ i ] ].size() ? i : object_parameter_types[ types[ i ] ].size() - 1;
      for ( int k = 0; k < (int)object_parameter_types[ types[ i ] ][ pos ].size(); ++k )
      {
         switch( object_parameter_types[ types[ i ] ][ pos ][ k ] )
         {
         case COORD :
            params .push_back( drand48() * max_d - max_d / 2e0 );
            break;

         case NORM :
            params .push_back( drand48() * 2e0 - 1e0 );
            break;

         case RADIUS :
            params .push_back( drand48() * max_d + delta_min );
            break;

         case ANGLE :
            params .push_back( drand48() * 1.9999e0 * M_PI );
            break;

         default :
            error_msg = "random_params: unknown object parameter type";
            return false;
            break;
         }
      }
   }
   return true;
}


QString US_PM::list_params( vector < double > & params )
{
   QString qs = QString( "model: grid conversion factor %1 " ).arg( grid_conversion_factor );
   vector < int >    types;
   vector < double > fparams;
   if ( !split( params, types, fparams ) )
   {
      return error_msg;
   }

   int fpos = 0;
   for ( int i = 0; i < (int)types.size(); ++i )
   {
      int pos = i < (int)object_parameter_types[ types[ i ] ].size() ? i : object_parameter_types[ types[ i ] ].size() - 1;
      qs += QString( "\n\t%1 (%2) : " ).arg( object_names[ types[ i ] ] ).arg( (int)object_parameter_types[ types[ i ] ][ pos ].size() );
      for ( int k = 0; k < (int)object_parameter_types[ types[ i ] ][ pos ].size(); ++k, ++fpos )
      {
         qs += QString( "%1 " ).arg( fparams[ fpos ] );
      }
   }
   qs += QString( "\n\ttotal types %1 fparams %2 total %3 \n" ).arg( types.size() ).arg( fpos ).arg( params.size() );
   return qs;
}

QString US_PM::list_object_info()
{
   QString qs;

   for ( int i = 0; i < (int)object_parameter_types.size(); ++i )
   {
      qs += QString( "object: %1 model positions %2\n" ).arg( object_names[ i ] ).arg( object_parameter_types[ i ].size() );
      for ( int j = 0; j < (int)object_parameter_types[ i ].size(); ++j )
      {
         qs += QString( "\tmodel position %1 parameters %2: " ).arg( j ).arg( object_parameter_types[ i ][ j ].size() );
         for ( int k = 0; k < (int)object_parameter_types[ i ][ j ].size(); ++k )
         {
            qs += QString( " %1" ).arg( object_type_name[ object_parameter_types[ i ][ j ][ k ] ] );
         }
         qs += "\n";
      }
   }
   return qs;
}


bool US_PM::set_limits( vector < double > & params, 
                        vector < double > & low_fparams, 
                        vector < double > & high_fparams, 
                        double max_d )
{
   vector < int > types;
   {
      vector < double > fparams;
      if ( !split( params, types, fparams ) )
      {
         return false;
      }
   }

   if ( !max_d )
   {
      max_d = max_dimension_d;
   }

   low_fparams.clear( );
   high_fparams.clear( );

   for ( int i = 0; i < (int)types.size(); ++i )
   {
      int pos = i < (int)object_parameter_types[ types[ i ] ].size() ? i : object_parameter_types[ types[ i ] ].size() - 1;
      for ( int k = 0; k < (int)object_parameter_types[ types[ i ] ][ pos ].size(); ++k )
      {
         switch( object_parameter_types[ types[ i ] ][ pos ][ k ] )
         {
         case COORD :
            low_fparams .push_back( -max_d );
            high_fparams.push_back( max_d );
            break;

         case NORM :
            low_fparams .push_back( -max_d );
            high_fparams.push_back( max_d );
            break;

         case RADIUS :
            low_fparams .push_back( delta_min );
            high_fparams.push_back( max_d );
            break;

         case ANGLE :
            low_fparams .push_back( 0e0 );
            high_fparams.push_back( M_PI * 1.9999e0 );
            break;
         default :
            error_msg = "set_limits: unknown object parameter type";
            return false;
            break;
         }
      }
   }
   return true;
}

bool US_PM::split( vector < double > & params, vector < int > & types, vector < double > & fparams )
{
   types  .clear( );
   fparams.clear( );

   for ( int i = 0; i < (int) params.size(); )
   {
      types.push_back( params[ i++ ] );
      if ( (int) params.size() <= i )
      {
         error_msg = QString( "split: no params for type %2" ).arg( types.back() );
         return false;
      }
      if ( (int)object_parameter_types.size() < types.back() )
      {
         error_msg = QString( "split: unknown object type %2" ).arg( types.back() );
         return false;
      }
      int pos = (int)types.size() - 1 < (int)object_parameter_types[ types.back() ].size() ? types.size() - 1 : object_parameter_types[ types.back() ].size() - 1;
      if ( params.size() < object_parameter_types[ types.back() ][ pos ].size() )
      {
         error_msg = QString( "split: error insufficient params for type %2" ).arg( types.back() );
         return false;
      }
      for ( int k = 0; k < (int)object_parameter_types[ types.back() ][ pos ].size(); ++k )
      {
         fparams.push_back( params[ i++ ] );
      }
   }
   return true;
}

bool US_PM::join( vector < double > & params, vector < int > & types, vector < double > & fparams )
{
   params.clear( );

   int fpos = 0;
   for ( int i = 0; i < (int)types.size(); ++i )
   {
      params.push_back( types[ i ] );
      if ( (int)object_parameter_types.size() < types[ i ] )
      {
         error_msg = QString( "split: unknown object type %2" ).arg( types[ i ] );
         return false;
      }
      int pos = i < (int)object_parameter_types[ types[ i ] ].size() ? i : object_parameter_types[ types[ i ] ].size() - 1;
      if ( (int)fparams.size() < fpos + (int)object_parameter_types[ types[ i ] ][ pos ].size() )
      {
         error_msg = QString( "join: error insufficient params for type %2" ).arg( types[ i ] );
         return false;
      }
      for ( int k = 0; k < (int)object_parameter_types[ types[ i ] ][ pos ].size(); ++k, ++fpos )
      {
         params.push_back( fparams[ fpos ] );
      }
   }
   return true;
}

static void combo_with_replacement(
                                   vector < int >            & elems,
                                   unsigned int                req_len,
                                   vector < int >            & pos,
                                   vector < vector < int > > & results,
                                   unsigned int                depth = 0,
                                   unsigned int                margin = 0
                                   )
{
   if ( depth >= req_len ) 
   {
      vector < int > result( pos.size() );
      for ( unsigned int ii = 0; ii < (unsigned int)pos.size(); ++ii )
      {
         result[ ii ] = elems[ pos[ ii ] ];
      }
      results.push_back( result );
      return;
   }

   for ( unsigned int ii = margin; ii < (unsigned int) elems.size(); ++ii ) 
   {
      pos[ depth ] = ii;
      combo_with_replacement( elems, req_len, pos, results, depth + 1, ii );
   }
   return;
}

bool  US_PM::expand_types(
                          vector < vector < double > > & types_vector,
                          QString                        types,
                          bool                           incrementally,
                          bool                           allcombinations 
                          )
{
   types_vector.clear( );
   vector < vector < int > > types_vector_int;
   bool result = expand_types( types_vector_int, types, incrementally, allcombinations );
   for ( int i = 0; i < (int) types_vector_int.size(); ++i )
   {
      vector < double > tmp_v;
      for ( int j = 0; j < (int) types_vector_int[ i ].size(); ++j )
      {
         tmp_v.push_back( (double) types_vector_int[ i ][ j ] );
      }
      types_vector.push_back( tmp_v );
   }
   return result;
}

bool  US_PM::expand_types(
                          vector < vector < int > > & types_vector,
                          QString                     types,
                          bool                        incrementally,
                          bool                        allcombinations 
                          )
{
   types_vector.clear( );

   error_msg = "";

   QStringList qsl_types;
   {
      QRegExp rx = QRegExp( "(\\s+|(\\s*(,|:)\\s*))" );
      qsl_types = (types ).split( rx , Qt::SkipEmptyParts );
   }

   for ( int i = 0; i < (int) qsl_types.size(); ++i )
   {
      if ( qsl_types[ i ].toInt() < OBJECTS_FIRST || 
           qsl_types[ i ].toInt() > OBJECTS_LAST )
      {
         error_msg += QString( " type out of range %1" ).arg( qsl_types[ i ] );
      }
   }
   if ( !error_msg.isEmpty() )
   {
      return false;
   }

   if ( incrementally )
   {
      if ( allcombinations )
      {
         set < int > types_set;

         for ( int i = 0; i < (int) qsl_types.size(); ++i )
         {
            types_set.insert( qsl_types[ i ].toInt() );
         }

         vector < int > typesv;
         for ( set < int >::iterator it = types_set.begin();
               it != types_set.end();
               it++ )
         {
            typesv.push_back( *it );
         }

         for ( int i = 1; i <= (int) qsl_types.size(); ++i )
         {
            vector < int > pos( i );
            combo_with_replacement( typesv,
                                    (int)pos.size(),
                                    pos,
                                    types_vector );
         }
      } else {
         for ( int i = 0; i < (int) qsl_types.size(); ++i )
         {
            vector < int > this_types;
            for ( int j = 0; j <= i; ++j )
            {
               this_types.push_back( qsl_types[ j ].toInt() );
            }
            types_vector.push_back( this_types );
         }
      }
   } else {
      if ( allcombinations )
      {
         set < int > types_set;

         for ( int i = 0; i < (int) qsl_types.size(); ++i )
         {
            types_set.insert( qsl_types[ i ].toInt() );
         }
         vector < int > typesv;
         for ( set < int >::iterator it = types_set.begin();
               it != types_set.end();
               it++ )
         {
            typesv.push_back( *it );
         }
         vector < int > pos( qsl_types.size() );
         combo_with_replacement( typesv,
                                 (int)pos.size(),
                                 pos,
                                 types_vector );
      } else {
         vector < int > this_types;
         for ( int i = 0; i < (int) qsl_types.size(); ++i )
         {
            this_types.push_back( qsl_types[ i ].toInt() );
         }
         types_vector.push_back( this_types );
      }
   }

   if ( us_log )
   {
      us_log->log( QString( "pm types string %1 %2 %3 size %4\n" )
                   .arg( types )
                   .arg( incrementally   ? "incrementally" : "" )
                   .arg( allcombinations ? "allcombinations" : "" )
                   .arg( types_vector.size() ) );
   }

   return true;
}
