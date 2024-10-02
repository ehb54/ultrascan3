#include "../include/us_pm.h"
//Added by qt3to4:
#include <QTextStream>

#define MAKE_MODEL    0

#define RANDOM_TEST   0
#define RANDOM_CYJ    0

#define TEST_CYLINDER 0

#define BEST_SPHERE   0
#define BEST_CYLINDER 0
#define BEST_SPHEROID 0
#define BEST_TORUS    0
#define BEST          ( BEST_SPHERE || BEST_CYLINDER || BEST_SPHEROID || BEST_TORUS )


#define BEST_MD0_SPHERE   0
#define BEST_MD0_CYLINDER 0
#define BEST_MD0_SPHEROID 0
#define BEST_MD0_TORUS    0
#define BEST_MD0          ( BEST_MD0_SPHERE || BEST_MD0_CYLINDER || BEST_MD0_SPHEROID || BEST_MD0_TORUS )

#define BEST_MD0_GA_SPHERE   0
#define BEST_MD0_GA_CYLINDER 0
#define BEST_MD0_GA_SPHEROID 1
#define BEST_MD0_GA_TORUS    0
#define BEST_MD0_GA          ( BEST_MD0_GA_SPHERE || BEST_MD0_GA_CYLINDER || BEST_MD0_GA_SPHEROID || BEST_MD0_GA_TORUS )

#define SPHEROID_SPEC_TEST 0
#define GRID_SEARCH_SPHEROID 0

#define MAKE_SPHERE_AND_SPHEROID 0

#define LEAK_CHECK    0
#define STD_MODEL     0
#define PERF_TEST     0

#define NEW_TEST      0

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

QString US_PM::test( QString name, QString oname )
{
   vector < double > q;
   vector < double > I;
   vector < double > e;
   vector < double > F;

   vector < double > params;

   if ( MAKE_MODEL ) {
      // special model
      double gcf = 1;

      US_PM test_pm( gcf,
                     200,
                     //                        drho, 
                     //                        buffer_e_density, 
                     //                        ev, 
                     10,
                     // fibonacci_grid,
                     F, 
                     q, 
                     I, 
                     e, 
                     1024,
                     0 );

      cout << "testpm" << endl;

      params.clear( );

      params.push_back( 0e0 );  // sphere 0
      params.push_back( 8e0 / gcf );  // radius in grid coordinates

      params.push_back( 1e0 );  // cylinder 1
      params.push_back( 8e0 / gcf );  // radius in grid coordinates
      params.push_back( 88e0 / gcf );  // -x
      params.push_back( -37e0 / gcf ); // -y
      params.push_back( 88e0 / gcf );  // +x
      params.push_back( 37e0 / gcf ); // +y

      us_qdebug( "mm0" );
      set < pm_point > model;
      us_qdebug( "mm1" );
      test_pm.create_model( params, model );
      us_qdebug( "mm2" );
      test_pm.write_model( oname, model );
      
      cout << "testpm wrote model" << oname;
      exit( 0 );
   }

   QFile f( name );

   double       grid_conversion_factor = 1e0;
   int          max_dimension          = USPM_MAX_VAL;
   double       drho                   = 1e-1;
   // double       buffer_e_density       = 0e0;
   // double       ev                     = 0e0;
   unsigned int max_harmonics          = 12;
   // unsigned int fibonacci_grid         = 17;
   bool         only_last_model        = false;

   if ( f.exists() && f.open( QIODevice::ReadOnly ) )
   {
      QTextStream ts( &f );
      grid_conversion_factor = ts.readLine().trimmed().toDouble();
      max_dimension          = ts.readLine().trimmed().toInt   ();
      drho                   = ts.readLine().trimmed().toDouble();
      /* buffer_e_density       = */ ts.readLine().trimmed().toDouble();
      /* ev                     = */ ts.readLine().trimmed().toDouble();
      max_harmonics          = ts.readLine().trimmed().toUInt();
      // fibonacci_grid         = ts.readLine().trimmed().toUInt();
      only_last_model        = ts.readLine().trimmed().toLower() == "only_last";

      while( !ts.atEnd() )
      {
         QString qs = ts.readLine().trimmed();
         if ( qs == "__END__" )
         {
            cout << "end in data\n";
            break;
         }
         if ( qs == "__data__" )
         {
            while( !ts.atEnd() )
            {
               qs = ts.readLine().trimmed();
               if ( !qs.isEmpty() )
               {
                  if ( qs == "__data_end__" )
                  {
                     break;
                  }
                  QStringList qsl = (qs ).split( QRegExp( "\\s+" ) , Qt::SkipEmptyParts );
                  if ( qsl.size() < 2 )
                  {
                     cout << QString( "error: data has only %1 parameters\n" ).arg( qsl.size() );
                     return "error";
                  }
                  q.push_back( qsl[ 0 ].toDouble() );
                  I.push_back( qsl[ 1 ].toDouble() );
                  e.push_back( qsl.size() > 2 ? qsl[ 1 ].toDouble() : 0e0 );
               }
            }
            continue;
         }
         if ( !qs.isEmpty() )
         {
            params.push_back( qs.toDouble() );
         }
      }
   }
   f.close();

   {
      US_Saxs_Util usu;
      if ( !usu.compute_rayleigh_structure_factors( 
                                                   pow( pow( grid_conversion_factor, 3e0 ) / M_PI, 1e0/3e0 ),
                                                   drho,
                                                   q,
                                                   F
                                                   ) )
      {
         cout << "could not compute rayleigh structure factors " << usu.errormsg << endl;
         return "error";
      }
   }

   US_Vector::printvector( "rayleigh structure factors", F );

   US_Vector::printvector( "read params", params );

   if ( NEW_TEST )
   {
      US_PM sphere_pm( grid_conversion_factor, 
                       max_dimension, 
                       //                        drho, 
                       //                        buffer_e_density, 
                       //                        ev, 
                       max_harmonics, 
                       // fibonacci_grid,
                       F, 
                       q, 
                       I, 
                       e, 
                       1024,
                       0 );

      vector < int > types;
      types.push_back( 0 );
      types.push_back( 1 );
      types.push_back( 2 );
      types.push_back( 3 );
      types.push_back( 4 );
      types.push_back( 0 );
      types.push_back( 1 );
      types.push_back( 2 );
      types.push_back( 3 );
      types.push_back( 4 );
      types.push_back( 0 );
      types.push_back( 1 );
      types.push_back( 2 );
      types.push_back( 3 );
      types.push_back( 4 );

      vector < double > org_params;
      vector < double > params;
      sphere_pm.random_params    ( params    , types );

      US_Vector::printvector( "params", params );
      US_Vector::printvector( "types", types );

      cout << sphere_pm.list_params( params );

      vector < int >    types_new;
      vector < double > fparams_new;

      if ( !sphere_pm.split    ( params, types_new, fparams_new ) )
      {
         cout << sphere_pm.error_msg << endl;
      }

      vector < double > jparams;

      if ( !sphere_pm.join    ( jparams    , types_new, fparams_new ) )
      {
         cout << sphere_pm.error_msg << endl;
      }         

      if ( params == jparams )
      {
         cout << "join exact match\n";
      } else {
         cout << "join ERROR\n";
         US_Vector::printvector2( "params original after join", params, jparams );
      }


      return "";
   }         

   if ( RANDOM_TEST )
   {
      US_PM sphere_pm( grid_conversion_factor, 
                       max_dimension, 
                       //                        drho, 
                       //                        buffer_e_density, 
                       //                        ev, 
                       max_harmonics, 
                       // fibonacci_grid,
                       F, 
                       q, 
                       I, 
                       e, 
                       1024,
                       0 );

      srand48( -1 );

      set < pm_point >   model;
      vector < double >  I_result( q.size() );
      US_Timer           us_timers;
      us_timers          .clear_timers();
      us_timers.init_timer( "create_model" );
      us_timers.init_timer( "compute I" );

      for ( int i = 0; i < RANDOM_TEST; i++ )
      {
         sphere_pm.random_md0_params  ( params, 5 );
         cout << QString( "Random test %1 of %2:\n" ).arg( i+1 ).arg( RANDOM_TEST ) << sphere_pm.list_params ( params ) << endl;

         us_timers.start_timer        ( "create_model" );
         sphere_pm.create_model       ( params, model );
         us_timers.end_timer          ( "create_model" );

         cout << QString( "\tbeads: %1\n" ).arg( model.size() );

         us_timers.start_timer        ( "compute I" );
         if ( RANDOM_CYJ )
         {
            sphere_pm.compute_CYJ_I      ( model, I_result );
         } else {
            sphere_pm.compute_CA_I       ( model, I_result );
         }
            
         us_timers.end_timer          ( "compute I" );
      }
      cout << us_timers.list_times();
   }

   if ( TEST_CYLINDER )
   {
      US_PM sphere_pm( grid_conversion_factor, 
                       max_dimension, 
                       //                        drho, 
                       //                        buffer_e_density, 
                       //                        ev, 
                       max_harmonics, 
                       // fibonacci_grid,
                       F, 
                       q, 
                       I, 
                       e, 
                       1024,
                       0 );


      params.clear( );

      params.push_back( 1e0 );  // cylinder 0
      params.push_back( 1e-1 );  // radius in grid coordinates
      params.push_back( 1e-1 ); // length

      set < pm_point > model;
      for ( params[ 2 ] = 2e0; params[ 2 ] < 15e0; params[ 2 ] += 1e0 )
      {
         sphere_pm.create_model( params, model );
         QString tmp_name = sphere_pm.tmp_name( "", params );
         sphere_pm.write_model( tmp_name, model );
      }
   }

   if ( BEST )
   {
      US_PM sphere_pm( grid_conversion_factor, 
                       max_dimension, 
                       //                        drho, 
                       //                        buffer_e_density, 
                       //                        ev, 
                       max_harmonics, 
                       // fibonacci_grid,
                       F, 
                       q, 
                       I, 
                       e, 
                       1024,
                       0 );

      set < pm_point >    model;

      US_Timer           us_timers;
      us_timers          .clear_timers();
      us_timers.init_timer( "BEST" );
      us_timers.start_timer( "BEST" );

      if ( BEST_SPHERE )
      {
         cout << "starting best sphere\n";
         sphere_pm.best_sphere( model );

         us_timers.stop_all();
      
         QString outname = QString( "%1_sh%2_best_sphere" ).arg( oname ).arg( max_harmonics );
         sphere_pm.write_model( outname, model );
         sphere_pm.write_I    ( outname, model );
      
         us_timers.start_all();

         cout << "ending best sphere\n";
      }

      if ( BEST_CYLINDER )
      {
         // sphere_pm.clear( );
         cout << "starting best cylinder\n";
         sphere_pm.best_cylinder( model );
         us_timers.stop_all();
      
         QString outname = QString( "%1_sh%2_best_cylinder" ).arg( oname ).arg( max_harmonics );
         sphere_pm.write_model( outname, model );
         sphere_pm.write_I    ( outname, model );

         us_timers.start_all();
         cout << "ending best cylinder\n";
      }

      if ( BEST_SPHEROID )
      {
         // sphere_pm.clear( );
         cout << "starting best spheroid\n";
         sphere_pm.best_spheroid( model );
         us_timers.stop_all();
      
         QString outname = QString( "%1_sh%2_best_spheroid" ).arg( oname ).arg( max_harmonics );
         sphere_pm.write_model( outname, model );
         sphere_pm.write_I    ( outname, model );
      
         us_timers.start_all();
         cout << "ending best spheroid\n";
      }

      if ( BEST_TORUS )
      {
         // sphere_pm.clear( );
         cout << "starting best torus\n";
         sphere_pm.best_torus( model );
         us_timers.stop_all();

         QString outname = QString( "%1_sh%2_best_torus" ).arg( oname ).arg( max_harmonics );
         sphere_pm.write_model( outname, model );
         sphere_pm.write_I    ( outname, model );

         us_timers.start_all();
         cout << "ending best torus\n";
      }
      us_timers.end_timer          ( "BEST" );
      cout << us_timers.list_times();
   }

   if ( MAKE_SPHERE_AND_SPHEROID )
   {
      // spheroid should be a sphere when a == b

      US_PM sphere_pm( grid_conversion_factor, 
                       max_dimension, 
                       //                        drho, 
                       //                        buffer_e_density, 
                       //                        ev, 
                       max_harmonics, 
                       // fibonacci_grid,
                       F, 
                       q, 
                       I, 
                       e, 
                       2048,
                       1 );

      QString tag = "MAKE_SPHERE_AND_SPHEROID";
      cout << "starting " << tag << endl;

      set < pm_point >   model;

      double radius = 5e0;

      // sphere

      vector < double > params(2);

      params      [ 0 ] = 0e0;
      params      [ 1 ] = radius;

      sphere_pm.create_model( params, model );

      QString outname = QString( "%1_sh%2_sphere_%3" ).arg( oname ).arg( max_harmonics ).arg( radius );
      sphere_pm.write_model( outname, model, params );
      sphere_pm.write_I    ( outname, model);
      
      params.resize( 3 );
      params[ 0 ] = 2e0;
      params[ 1 ] = params[ 1 ];
      params[ 2 ] = params[ 1 ];

      sphere_pm.create_model( params, model );
      outname = QString( "%1_sh%2_spheroid_%3" ).arg( oname ).arg( max_harmonics ).arg( radius );
      sphere_pm.write_model( outname, model, params );
      sphere_pm.write_I    ( outname, model);

      params.resize( 4 );
      params[ 0 ] = 3e0;
      params[ 1 ] = params[ 1 ];
      params[ 2 ] = params[ 1 ];
      params[ 3 ] = params[ 1 ];

      sphere_pm.create_model( params, model );
      outname = QString( "%1_sh%2_ellipsoid_%3" ).arg( oname ).arg( max_harmonics ).arg( radius );
      sphere_pm.write_model( outname, model, params );
      sphere_pm.write_I    ( outname, model);

      params.resize( 3 );
      params[ 0 ] = 4e0;
      params[ 1 ] = 0e0;
      params[ 2 ] = params[ 2 ];

      sphere_pm.create_model( params, model );
      outname = QString( "%1_sh%2_torus_%3" ).arg( oname ).arg( max_harmonics ).arg( radius );
      sphere_pm.write_model( outname, model, params );
      sphere_pm.write_I    ( outname, model);

      params.resize( 3 );
      params[ 0 ] = 4e0;
      params[ 1 ] = 7e0;
      params[ 2 ] = 2e0;

      sphere_pm.create_model( params, model );
      outname = QString( "%1_sh%2_torus_2_%3" ).arg( oname ).arg( max_harmonics ).arg( radius );
      sphere_pm.write_model( outname, model, params );
      sphere_pm.write_I    ( outname, model);

      cout << "ending best grid search spheroid\n";
   }


   if ( GRID_SEARCH_SPHEROID )
   {
      US_PM sphere_pm( grid_conversion_factor, 
                       max_dimension, 
                       //                        drho, 
                       //                        buffer_e_density, 
                       //                        ev, 
                       max_harmonics, 
                       // fibonacci_grid,
                       F, 
                       q, 
                       I, 
                       e, 
                       2048,
                       0 );

      cout << "starting best grid search spheroid\n";

      set < pm_point >   model;

      US_Timer           us_timers;
      us_timers         .clear_timers();
      us_timers         .init_timer( "SPHEROID_GRID_SEARCH" );
      us_timers         .start_all();

      vector < double > params(3);
      vector < double > delta(2);
      vector < double > low_fparams(2);
      vector < double > high_fparams(2);

      params      [ 0 ] = 2e0;
      delta       [ 0 ] = 1e-1;
      delta       [ 1 ] = 1e-1;
      low_fparams [ 0 ] = 4e0;
      low_fparams [ 1 ] = 4e0;
      high_fparams[ 0 ] = 11e0;
      high_fparams[ 1 ] = 11e0;

      sphere_pm.grid_search( params, delta, low_fparams, high_fparams, model );
      
      us_timers        .end_all();
      cout << us_timers.list_times();

      QString outname = QString( "%1_sh%2_best_grid_search_spheroid" ).arg( oname ).arg( max_harmonics );
      sphere_pm.write_model( outname, model, params );
      sphere_pm.write_I    ( outname, model );

      cout << "ending best grid search spheroid\n";
   }

   if ( BEST_MD0_GA )
   {
      US_PM sphere_pm( grid_conversion_factor, 
                       max_dimension, 
                       //                        drho, 
                       //                        buffer_e_density, 
                       //                        ev, 
                       max_harmonics, 
                       // fibonacci_grid,
                       F, 
                       q, 
                       I, 
                       e, 
                       1024,
                       0 );

      set < pm_point >    model;

      US_Timer           us_timers;
      us_timers          .clear_timers();
      us_timers.init_timer( "BEST_MD0_GA" );
      us_timers.start_timer( "BEST_MD0_GA" );

      vector < double > params(1);

      unsigned int steps    = 1;
      unsigned int pts_max  = 150;
      // double       finest   = grid_conversion_factor;
      double       coarse   = 10e0;
      double       ref_pct  = 7.5e0;
      double       conv_div = 2.5e0;

      unsigned int pop      = 500;
      unsigned int gen      = 100;
      double       mutate   = .45;
      double       cross    = .45;
      unsigned int elitism  = 2;
      unsigned int early_term = 5;

      double       best_delta_start   = 1e0;
      double       best_delta_divisor = 10e0;
      double       best_delta_min     = 1e-2;

      sphere_pm.ga_set_params( pop, gen, mutate, cross, elitism, early_term );
      sphere_pm.set_best_delta( best_delta_start, best_delta_divisor, best_delta_min );
      if ( BEST_MD0_GA_SPHERE )
      {
         cout << "starting best sphere\n";
         params[ 0 ] = 0e0;
         sphere_pm.best_md0_ga( params, model, steps, pts_max, grid_conversion_factor, coarse, ref_pct, conv_div );
         us_timers.stop_all();
      
         QString outname = QString( "%1_sh%2_best_MD0_GA_sphere" ).arg( oname ).arg( max_harmonics );
         sphere_pm.write_model( outname, model, params );
         sphere_pm.write_I    ( outname, model );
      
         us_timers.start_all();
         cout << "ending best sphere\n";
      }

      if ( BEST_MD0_GA_CYLINDER )
      {
         // sphere_pm.clear( );
         cout << "starting best cylinder\n";
         params[ 0 ] = 1e0;
         sphere_pm.best_md0_ga( params, model, steps, pts_max, grid_conversion_factor, coarse, ref_pct, conv_div );
         us_timers.stop_all();

         QString outname = QString( "%1_sh%2_best_MD0_GA_cylinder" ).arg( oname ).arg( max_harmonics );
         sphere_pm.write_model( outname, model, params );
         sphere_pm.write_I    ( outname, model );
      
         us_timers.start_all();
         cout << "ending best cylinder\n";
      }

      if ( BEST_MD0_GA_SPHEROID )
      {
         // sphere_pm.clear( );
         cout << "starting best spheroid\n";
         params[ 0 ] = 2e0;
         sphere_pm.best_md0_ga( params, model, steps, pts_max, grid_conversion_factor, coarse, ref_pct, conv_div );

         us_timers.stop_all();
            
         QString outname = QString( "%1_sh%2_best_MD0_GA_spheroid" ).arg( oname ).arg( max_harmonics );
         sphere_pm.write_model( outname, model, params );
         sphere_pm.write_I    ( outname, model );

         us_timers.start_all();
         cout << "ending best spheroid\n";
      }

      if ( BEST_MD0_GA_TORUS )
      {
         // sphere_pm.clear( );
         cout << "starting best torus\n";
         params[ 0 ] = 4e0;
         sphere_pm.best_md0_ga( params, model, steps, pts_max, grid_conversion_factor, coarse, ref_pct, conv_div );
         us_timers.stop_all();

         QString outname = QString( "%1_sh%2_best_MD0_GA_torus" ).arg( oname ).arg( max_harmonics );
         sphere_pm.write_model( outname, model, params );
         sphere_pm.write_I    ( outname, model );

         us_timers.start_all();
         cout << "ending best torus\n";
      }
      us_timers.end_timer          ( "BEST_MD0_GA" );
      cout << us_timers.list_times();
   }

   if ( LEAK_CHECK )
   {
      double spheretest_min = 3e0;
      double spheretest_max = 5e0;
      int     reps          = 20;
      int     i;
      if ( 1 ) 
      { // fully cached
         US_PM sphere_pm( grid_conversion_factor, 
                          max_dimension, 
                          //                           drho, 
                          //                           buffer_e_density, 
                          //                           ev, 
                          max_harmonics, 
                          // fibonacci_grid,
                          F, 
                          q, 
                          I, 
                          e, 
                          1024,
                          0 );


         vector < double >                       I_result( q.size() );
         set < pm_point >                        model;
         vector < double > params( 2 );
         params[ 0 ] = 0e0;

         sphere_pm.us_timers.clear_timers();
         sphere_pm.us_timers.init_timer( "FC rep" );
         sphere_pm.us_timers.start_timer( "FC rep" );
         for ( i = 0; i < reps; i++ )
         {
            if ( !( i % 10 ) )
            {
               cout << QString( "FC rep %1\n" ).arg( i );
            }
            for ( params[ 1 ] = spheretest_min; params[ 1 ] <= spheretest_max; ++params[ 1 ] )
            {
               sphere_pm.msg_log = "";
               model.clear( );
               sphere_pm.create_model( params, model );
               sphere_pm.compute_CA_I( model, I_result );
            }
         }
         sphere_pm.us_timers.end_timer( "FC rep" );
         cout << sphere_pm.us_timers.list_times();
      }

      if ( 1 ) 
      { // delta 
         US_PM sphere_pm( grid_conversion_factor, 
                          max_dimension, 
                          //                           drho, 
                          //                           buffer_e_density, 
                          //                           ev, 
                          max_harmonics, 
                          // fibonacci_grid,
                          F, 
                          q, 
                          I, 
                          e, 
                          1024,
                          0 );

         vector < double >                       I_result( q.size() );
         set < pm_point >                        model;
         set < pm_point >                        prev_model;
         vector < vector < complex < float > > > Av;
         vector < double > params( 2 );
         params[ 0 ] = 0e0;

         sphere_pm.us_timers.clear_timers();
         sphere_pm.us_timers.init_timer( "Delta rep" );
         sphere_pm.us_timers.start_timer( "Delta rep" );

         for ( i = 0; i < reps; i++ )
         {
            if ( !( i % 10 ) )
            {
               cout << QString( "Delta rep %1\n" ).arg( i );
            }
            for ( params[ 1 ] = spheretest_min; params[ 1 ] <= spheretest_max; ++params[ 1 ] )
            {
               sphere_pm.msg_log = "";
               model.clear( );
               sphere_pm.create_model( params, model );
               sphere_pm.compute_delta_I( model, prev_model, Av, I_result );
               prev_model = model;
            }
         }
         sphere_pm.us_timers.end_timer( "Delta rep" );
         cout << sphere_pm.us_timers.list_times();
      }

      if ( 1 ) 
      { // partially cached
         US_PM sphere_pm( grid_conversion_factor, 
                          max_dimension, 
                          //                           drho, 
                          //                           buffer_e_density, 
                          //                           ev, 
                          max_harmonics, 
                          // fibonacci_grid,
                          F, 
                          q, 
                          I, 
                          e, 
                          1024,
                          0 );

         vector < double >                       I_result( q.size() );
         set < pm_point >                        model;
         vector < double > params( 2 );
         params[ 0 ] = 0e0;

         sphere_pm.us_timers.clear_timers();
         sphere_pm.us_timers.init_timer( "PC rep" );
         sphere_pm.us_timers.start_timer( "PC rep" );

         for ( i = 0; i < reps; i++ )
         {
            if ( !( i % 10 ) )
            {
               cout << QString( "PC rep %1\n" ).arg( i );
            }
            for ( params[ 1 ] = spheretest_min; params[ 1 ] <= spheretest_max; ++params[ 1 ] )
            {
               sphere_pm.msg_log = "";
               model.clear( );
               sphere_pm.create_model( params, model );
               sphere_pm.compute_CYJ_I( model, I_result );
            }
         }
         sphere_pm.us_timers.end_timer( "PC rep" );
         cout << sphere_pm.us_timers.list_times();
      }
   }


   if ( PERF_TEST ) {
      oname += "_alt_sh_alt_sph";

      double spheretest_min = 6e0;
      double spheretest_max = 10e0;
      int    dups           = 2;
      QString                 log;
      US_Timer           us_timers;
      QString                 tlog;
   
      if ( 1 ) 
      { // 
         log += QString( "fully cached ascending sphere test %1 to %2\n" ).arg( spheretest_min ).arg( spheretest_max );

         US_PM sphere_pm( grid_conversion_factor, 
                          max_dimension, 
                          //                           drho, 
                          //                           buffer_e_density, 
                          //                           ev, 
                          max_harmonics, 
                          // fibonacci_grid,
                          F, 
                          q, 
                          I, 
                          e, 
                          1024,
                          5 );


         vector < double >                       I_result( q.size() );
         set < pm_point >                        model;
         vector < double > params( 2 );
         params[ 0 ] = 0e0;

         sphere_pm.us_timers.clear_timers();

         us_timers.init_timer( "asc sph FC" );
         us_timers.start_timer( "asc sph FC" );

         for ( params[ 1 ] = spheretest_min; params[ 1 ] <= spheretest_max; ++params[ 1 ] )
         {
            for ( int d = 0; d < dups; d++ )
            {
               cout << QString( "---------- fully cached Sphere radius %1 -----------\n" ).arg( params[ 1 ] ).toLatin1().data();
               /*
                 US_PM sphere_pm( grid_conversion_factor, 
                 max_dimension, 
                 //                  drho, 
                 //                  buffer_e_density, 
                 //                  ev, 
                 max_harmonics, 
                 // fibonacci_grid,
                 F, 
                 q, 
                 I, 
                 e, 
                 5 );
               */
               model.clear( );
               sphere_pm.create_model( params, model );
               puts( "test:ccI call" );
               sphere_pm.compute_CA_I( model, I_result );
               puts( "test:ccI return" );

               cout << QString( "sphere radius %1 model bead count %2\n" ).arg( params[ 1 ] ).arg( model.size() );

               QString outname = QString( "%1_sh%2_FC_sphere_asc_x%2" ).arg( oname ).arg( max_harmonics ).arg( params[ 1 ] );
               sphere_pm.write_model( outname, model );
               sphere_pm.write_I    ( outname, model );
            }
         }
         us_timers.end_timer( "asc sph FC" );
         log += sphere_pm.msg_log;
      }

      if ( 1 ) { // ascending sphere delta test

         log += QString( "delta ascending sphere test %1 to %2\n" ).arg( spheretest_min ).arg( spheretest_max );
         US_PM sphere_pm( grid_conversion_factor, 
                          max_dimension, 
                          //                           drho, 
                          //                           buffer_e_density, 
                          //                           ev, 
                          max_harmonics, 
                          // fibonacci_grid,
                          F, 
                          q, 
                          I, 
                          e, 
                          1024,
                          5 );

         vector < double >                       I_result( q.size() );
         vector < vector < complex < float > > > Av;
         set < pm_point >                        model;
         set < pm_point >                        prev_model;
         vector < double > params( 2 );
         params[ 0 ] = 0e0;

         us_timers.init_timer( "asc sphere delta" );
         us_timers.start_timer( "asc sphere delta" );

         for ( params[ 1 ] = spheretest_min; params[ 1 ] <= spheretest_max; ++params[ 1 ] )
         {
            cout << QString( "---------- delta Sphere radius %1 -----------\n" ).arg( params[ 1 ] ).toLatin1().data();
            model.clear( );
            sphere_pm.create_model( params, model );
            sphere_pm.compute_delta_I( model, prev_model, Av, I_result );
            prev_model = model;

            cout << QString( "sphere radius %1 model bead count %2\n" ).arg( params[ 1 ] ).arg( model.size() );

            QString outname = QString( "%1_sh%2_AscDelta_sphere_x%2" ).arg( oname ).arg( max_harmonics ).arg( params[ 1 ] );
            sphere_pm.write_model( outname, model );
            sphere_pm.write_I    ( outname, model );
         }
         us_timers.end_timer( "asc sphere delta" );
         log += sphere_pm.msg_log;
      }

      if ( 1 ) 
      { // ascending sphere non-delta test
         log += QString( "partially cached sphere test %1 to %2\n" ).arg( spheretest_min ).arg( spheretest_max );
         US_PM sphere_pm( grid_conversion_factor, 
                          max_dimension, 
                          //                           drho, 
                          //                           buffer_e_density, 
                          //                           ev, 
                          max_harmonics, 
                          // fibonacci_grid,
                          F, 
                          q, 
                          I, 
                          e, 
                          1024,
                          5 );

         vector < double >                       I_result( q.size() );
         set < pm_point >                        model;
         vector < double > params( 2 );
         params[ 0 ] = 0e0;

         us_timers.init_timer( "asc sphere CYJ" );
         us_timers.start_timer( "asc sphere CYJ" );

         for ( params[ 1 ] = spheretest_min; params[ 1 ] <= spheretest_max; ++params[ 1 ] )
         {
            for ( int d = 0; d < dups; d++ )
            {
               cout << QString( "---------- standard Sphere radius %1 -----------\n" ).arg( params[ 1 ] ).toLatin1().data();
               model.clear( );
               sphere_pm.create_model( params, model );
               sphere_pm.compute_CYJ_I( model, I_result );

               cout << QString( "sphere radius %1 model bead count %2\n" ).arg( params[ 1 ] ).arg( model.size() );

               QString outname = QString( "%1_sh%2_STD_sphere_x%2" ).arg( oname ).arg( max_harmonics ).arg( params[ 1 ] );
               sphere_pm.write_model( outname, model );
               sphere_pm.write_I    ( outname, model );
            }
         }
         us_timers.end_timer( "asc sphere CYJ" );
         log += sphere_pm.msg_log;
      }

      if ( 0 ) { // descending sphere delta test
         US_PM sphere_pm( grid_conversion_factor, 
                          max_dimension, 
                          //                           drho, 
                          //                           buffer_e_density, 
                          //                           ev, 
                          max_harmonics, 
                          // fibonacci_grid,
                          F, 
                          q, 
                          I, 
                          e, 
                          1024,
                          5 );

         vector < double >                       I_result( q.size() );
         set < pm_point >                        model;
         vector < vector < complex < float > > > Av;
         set < pm_point >                        prev_model;
         vector < double > params( 2 );
         params[ 0 ] = 0e0;

         us_timers.init_timer( "desc sphere delta" );
         us_timers.start_timer( "deeesc sphere delta" );

         for ( params[ 1 ] = spheretest_max; params[ 1 ] >= spheretest_min; --params[ 1 ] )
         {
            cout << QString( "---------- delta descending Sphere radius %1 -----------\n" ).arg( params[ 1 ] ).toLatin1().data();
            model.clear( );
            sphere_pm.create_model( params, model );
            sphere_pm.compute_delta_I( model, prev_model, Av, I_result );
            prev_model = model;

            cout << QString( "sphere radius %1 model bead count %2\n" ).arg( params[ 1 ] ).arg( model.size() );

            QString outname = QString( "%1_sh%2_DscDelta_sphere_x%2" ).arg( oname ).arg( max_harmonics ).arg( params[ 1 ] );            sphere_pm.write_model( outname, model );
            sphere_pm.write_I    ( outname, model );
            sphere_pm.write_I    ( outname, model );
         }
         us_timers.end_timer( "deeesc sphere delta" );
         log += sphere_pm.msg_log;
      }
      cout << log.toLatin1().data();
      cout << us_timers.list_times();
   }

   if ( STD_MODEL ) // standard model test
   {

      US_PM test_pm( grid_conversion_factor, 
                     max_dimension, 
                     //                      drho, 
                     //                      buffer_e_density, 
                     //                      ev, 
                     max_harmonics, 
                     // fibonacci_grid,
                     F, 
                     q, 
                     I, 
                     e, 
                     5 );

      vector < double > low_fparams;
      vector < double > high_fparams;
      test_pm.set_limits( params, low_fparams, high_fparams );
      US_Vector::printvector2( "fparam limits", low_fparams, high_fparams );

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

      if ( !test_pm.create_model( params, model, only_last_model ) )
      {
         cout << test_pm.error_msg.toLatin1().data() << endl;
      }

      vector < int    > types;
      vector < double > fparams;
      US_Vector::printvector( "before split: params", params ); 
      if ( !test_pm.split( params, types, fparams ) )
      {
         cout << test_pm.error_msg.toLatin1().data() << endl;
      }
      US_Vector::printvector( "split: types", types ); 
      US_Vector::printvector( "split: fparams", fparams ); 

      vector < double > new_params;
      if ( !test_pm.join( new_params, types, fparams ) )
      {
         cout << test_pm.error_msg.toLatin1().data() << endl;
      }
      US_Vector::printvector( "after join: new_params", new_params ); 
      if ( new_params != params )
      {
         cout << "ERROR: new_params != params" << endl;
      }
      cout << QString( "bead model contains %1 beads\n" ).arg( model.size() );

      QString output = test_pm.qs_bead_model( model );

      oname += QString( "_gs%1" ).arg( grid_conversion_factor );

      // output bead model
      {
         QString outfile = oname;
      
         if ( !outfile.contains( QRegExp( "\\.bead_model$" ) ) )
         {
            outfile += ".bead_model";
         }

         cout << "Creating:" << outfile << "\n";
         QFile of( outfile );
         if ( !of.open( QIODevice::WriteOnly ) )
         {
            return "could not create output file";
         }
   
         QTextStream ts( &of );
         ts << output;
         of.close();
      }

      // compute I(q)
      if ( q.size() )
      {
         cout << "compute_CYJ_I\n";
         vector < double > I_result( q.size() );
         if ( !test_pm.compute_CYJ_I( model, I_result ) )
         {
            return "compute_CYJ_I error:" + test_pm.error_msg;
         }
         if ( !test_pm.compute_CYJ_I( model, I_result ) )
         {
            return "2nd compute_CYJ_I error:" + test_pm.error_msg;
         }
         QString outfile = QString( "%1_sh%2" ).arg( oname ).arg( max_harmonics );
      
         if ( !outfile.contains( QRegExp( "\\.dat$" ) ) )
         {
            outfile += ".dat";
         }

         cout << "Creating:" << outfile << "\n";
         QFile of( outfile );
         if ( !of.open( QIODevice::WriteOnly ) )
         {
            return "could not create output file";
         }
   
         QTextStream ts( &of );
         ts << "# US-SOMO PM .dat file containing I(q) computed on bead model\n";
         for ( unsigned int i = 0; i < ( unsigned int ) q.size(); i++ )
         {
            ts << QString( "%1\t%2\n" ).arg( q[ i ], 0, 'e', 6 ).arg( I_result[ i ], 0, 'e', 6 );
         }
         of.close();
      }

      oname += "_recentered";

      model = test_pm.recenter( model );
      QString outname = QString( "%1_sh%2" ).arg( oname ).arg( max_harmonics );
      test_pm.write_model( outname, model );
      test_pm.write_I( outname, model );
   }

   return "ok";
}
