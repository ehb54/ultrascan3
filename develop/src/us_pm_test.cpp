#include "../include/us_pm.h"

#define BEST_SPHERE   0
#define BEST_CYLINDER 1
#define BEST          ( BEST_SPHERE || BEST_CYLINDER )

#define LEAK_CHECK    0
#define STD_MODEL     0
#define PERF_TEST     0

QString US_PM::test( QString name, QString oname )
{
   vector < double > q;
   vector < double > I;
   vector < double > e;
   vector < double > F;

   vector < double > params;

   QFile f( name );

   double       grid_conversion_factor = 1e0;
   int          max_dimension          = USPM_MAX_VAL;
   double       drho                   = 1e-1;
   double       buffer_e_density       = 0e0;
   double       ev                     = 0e0;
   unsigned int max_harmonics          = 12;
   // unsigned int fibonacci_grid         = 17;
   bool         only_last_model        = false;

   if ( f.exists() && f.open( IO_ReadOnly ) )
   {
      QTextStream ts( &f );
      grid_conversion_factor = ts.readLine().stripWhiteSpace().toDouble();
      max_dimension          = ts.readLine().stripWhiteSpace().toInt   ();
      drho                   = ts.readLine().stripWhiteSpace().toDouble();
      buffer_e_density       = ts.readLine().stripWhiteSpace().toDouble();
      ev                     = ts.readLine().stripWhiteSpace().toDouble();
      max_harmonics          = ts.readLine().stripWhiteSpace().toUInt();
      // fibonacci_grid         = ts.readLine().stripWhiteSpace().toUInt();
      only_last_model        = ts.readLine().stripWhiteSpace().lower() == "only_last";

      while( !ts.atEnd() )
      {
         QString qs = ts.readLine().stripWhiteSpace();
         if ( qs == "__END__" )
         {
            cout << "end in data\n";
            break;
         }
         if ( qs == "__data__" )
         {
            while( !ts.atEnd() )
            {
               qs = ts.readLine().stripWhiteSpace();
               if ( !qs.isEmpty() )
               {
                  if ( qs == "__data_end__" )
                  {
                     break;
                  }
                  QStringList qsl = QStringList::split( QRegExp( "\\s+" ), qs );
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

   if ( BEST )
   {
      US_PM sphere_pm( grid_conversion_factor, 
                       max_dimension, 
                       drho, 
                       buffer_e_density, 
                       ev, 
                       max_harmonics, 
                       // fibonacci_grid,
                       F, 
                       q, 
                       I, 
                       e, 
                       1024,
                       0 );

      set < pm_point >    model;

      if ( BEST_SPHERE )
      {
         sphere_pm.best_sphere( model );
      
         // output bead model
         {
            QString outfile = QString( "%1_sh%2_best_sphere" ).arg( oname ).arg( max_harmonics );
      
            if ( !outfile.contains( QRegExp( "\\.bead_model$" ) ) )
            {
               outfile += ".bead_model";
            }

            cout << "Creating:" << outfile << "\n";
            QFile of( outfile );
            if ( !of.open( IO_WriteOnly ) )
            {
               return "could not create output file";
            }
   
            QTextStream ts( &of );
            ts << sphere_pm.qs_bead_model( model );
            of.close();
         }
         
         {
            vector < double >   I_result( q.size() );
            sphere_pm.compute_CYJ_I( model, I_result );
            QString outfile = QString( "%1_sh%2_best_sphere" ).arg( oname ).arg( max_harmonics );
      
            if ( !outfile.contains( QRegExp( "\\.dat$" ) ) )
            {
               outfile += ".dat";
            }

            cout << "Creating:" << outfile << "\n";
            QFile of( outfile );
            if ( !of.open( IO_WriteOnly ) )
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

      }

      if ( BEST_CYLINDER )
      {
         sphere_pm.best_cylinder( model );
      
         // output bead model
         {
            QString outfile = QString( "%1_sh%2_best_cylinder" ).arg( oname ).arg( max_harmonics );
      
            if ( !outfile.contains( QRegExp( "\\.bead_model$" ) ) )
            {
               outfile += ".bead_model";
            }

            cout << "Creating:" << outfile << "\n";
            QFile of( outfile );
            if ( !of.open( IO_WriteOnly ) )
            {
               return "could not create output file";
            }
   
            QTextStream ts( &of );
            ts << sphere_pm.qs_bead_model( model );
            of.close();
         }
         
         {
            vector < double >   I_result( q.size() );
            sphere_pm.compute_CYJ_I( model, I_result );
            QString outfile = QString( "%1_sh%2_best_cylinder" ).arg( oname ).arg( max_harmonics );
      
            if ( !outfile.contains( QRegExp( "\\.dat$" ) ) )
            {
               outfile += ".dat";
            }

            cout << "Creating:" << outfile << "\n";
            QFile of( outfile );
            if ( !of.open( IO_WriteOnly ) )
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
      }
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
                          drho, 
                          buffer_e_density, 
                          ev, 
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
               sphere_pm.log = "";
               model.clear();
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
                          drho, 
                          buffer_e_density, 
                          ev, 
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
               sphere_pm.log = "";
               model.clear();
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
                          drho, 
                          buffer_e_density, 
                          ev, 
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
               sphere_pm.log = "";
               model.clear();
               sphere_pm.create_model( params, model );
               sphere_pm.compute_CYJ_I( model, I_result );
            }
         }
         sphere_pm.us_timers.end_timer( "PC rep" );
         cout << sphere_pm.us_timers.list_times();
      }
   }


   if ( PERF_TEST ) {
      double spheretest_min = 6e0;
      double spheretest_max = 14e0;
      int    dups           = 2;
      QString                 log;
   
      if ( 1 ) 
      { // 
         log += QString( "fully cached ascending sphere test %1 to %2\n" ).arg( spheretest_min ).arg( spheretest_max );

         US_PM sphere_pm( grid_conversion_factor, 
                          max_dimension, 
                          drho, 
                          buffer_e_density, 
                          ev, 
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

         for ( params[ 1 ] = spheretest_min; params[ 1 ] <= spheretest_max; ++params[ 1 ] )
         {
            for ( int d = 0; d < dups; d++ )
            {
               cout << QString( "---------- fully cached Sphere radius %1 -----------\n" ).arg( params[ 1 ] ).ascii();
               /*
                 US_PM sphere_pm( grid_conversion_factor, 
                 max_dimension, 
                 drho, 
                 buffer_e_density, 
                 ev, 
                 max_harmonics, 
                 // fibonacci_grid,
                 F, 
                 q, 
                 I, 
                 e, 
                 5 );
               */
               model.clear();
               sphere_pm.create_model( params, model );
               puts( "test:ccI call" );
               sphere_pm.compute_CA_I( model, I_result );
               puts( "test:ccI return" );

               cout << QString( "sphere radius %1 model bead count %2\n" ).arg( params[ 1 ] ).arg( model.size() );

               // output bead model
               {
                  QString outfile = QString( "%1_sh%2_FC_sphere_asc_x%2" ).arg( oname ).arg( max_harmonics ).arg( params[ 1 ] );
      
                  if ( !outfile.contains( QRegExp( "\\.bead_model$" ) ) )
                  {
                     outfile += ".bead_model";
                  }

                  cout << "Creating:" << outfile << "\n";
                  QFile of( outfile );
                  if ( !of.open( IO_WriteOnly ) )
                  {
                     return "could not create output file";
                  }
   
                  QTextStream ts( &of );
                  ts << sphere_pm.qs_bead_model( model );
                  of.close();
               }
         
               {
                  QString outfile = QString( "%1_sh%2_FC_sphere_asc_x%2" ).arg( oname ).arg( max_harmonics ).arg( params[ 1 ] );
      
                  if ( !outfile.contains( QRegExp( "\\.dat$" ) ) )
                  {
                     outfile += ".dat";
                  }

                  cout << "Creating:" << outfile << "\n";
                  QFile of( outfile );
                  if ( !of.open( IO_WriteOnly ) )
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
            }
         }
         log += sphere_pm.log;
      }

      if ( 0 ) { // ascending sphere delta test
         log += QString( "delta ascending sphere test %1 to %2\n" ).arg( spheretest_min ).arg( spheretest_max );
         US_PM sphere_pm( grid_conversion_factor, 
                          max_dimension, 
                          drho, 
                          buffer_e_density, 
                          ev, 
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

         for ( params[ 1 ] = spheretest_min; params[ 1 ] <= spheretest_max; ++params[ 1 ] )
         {
            cout << QString( "---------- delta Sphere radius %1 -----------\n" ).arg( params[ 1 ] ).ascii();
            model.clear();
            sphere_pm.create_model( params, model );
            sphere_pm.compute_delta_I( model, prev_model, Av, I_result );
            prev_model = model;

            cout << QString( "sphere radius %1 model bead count %2\n" ).arg( params[ 1 ] ).arg( model.size() );

            // output bead model
            {
               QString outfile = QString( "%1_sh%2_AscDelta_sphere_x%2" ).arg( oname ).arg( max_harmonics ).arg( params[ 1 ] );
      
               if ( !outfile.contains( QRegExp( "\\.bead_model$" ) ) )
               {
                  outfile += ".bead_model";
               }

               cout << "Creating:" << outfile << "\n";
               QFile of( outfile );
               if ( !of.open( IO_WriteOnly ) )
               {
                  return "could not create output file";
               }
   
               QTextStream ts( &of );
               ts << sphere_pm.qs_bead_model( model );
               of.close();
            }
         
            {
               QString outfile = QString( "%1_sh%2_AscDelta_sphere_x%2" ).arg( oname ).arg( max_harmonics ).arg( params[ 1 ] );
      
               if ( !outfile.contains( QRegExp( "\\.dat$" ) ) )
               {
                  outfile += ".dat";
               }

               cout << "Creating:" << outfile << "\n";
               QFile of( outfile );
               if ( !of.open( IO_WriteOnly ) )
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
         }
         log += sphere_pm.log;
      }

      if ( 1 ) 
      { // ascending sphere non-delta test
         log += QString( "partially cached sphere test %1 to %2\n" ).arg( spheretest_min ).arg( spheretest_max );
         US_PM sphere_pm( grid_conversion_factor, 
                          max_dimension, 
                          drho, 
                          buffer_e_density, 
                          ev, 
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

         for ( params[ 1 ] = spheretest_min; params[ 1 ] <= spheretest_max; ++params[ 1 ] )
         {
            for ( int d = 0; d < dups; d++ )
            {
               cout << QString( "---------- standard Sphere radius %1 -----------\n" ).arg( params[ 1 ] ).ascii();
               model.clear();
               sphere_pm.create_model( params, model );
               sphere_pm.compute_CYJ_I( model, I_result );

               cout << QString( "sphere radius %1 model bead count %2\n" ).arg( params[ 1 ] ).arg( model.size() );

               // output bead model
               {
                  QString outfile = QString( "%1_sh%2_STD_sphere_x%2" ).arg( oname ).arg( max_harmonics ).arg( params[ 1 ] );
      
                  if ( !outfile.contains( QRegExp( "\\.bead_model$" ) ) )
                  {
                     outfile += ".bead_model";
                  }

                  cout << "Creating:" << outfile << "\n";
                  QFile of( outfile );
                  if ( !of.open( IO_WriteOnly ) )
                  {
                     return "could not create output file";
                  }
   
                  QTextStream ts( &of );
                  ts << sphere_pm.qs_bead_model( model );
                  of.close();
               }
         
               {
                  QString outfile = QString( "%1_sh%2_STD_sphere_x%2" ).arg( oname ).arg( max_harmonics ).arg( params[ 1 ] );
      
                  if ( !outfile.contains( QRegExp( "\\.dat$" ) ) )
                  {
                     outfile += ".dat";
                  }

                  cout << "Creating:" << outfile << "\n";
                  QFile of( outfile );
                  if ( !of.open( IO_WriteOnly ) )
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
            }
         }
         log += sphere_pm.log;
      }

      if ( 0 ) { // descending sphere delta test
         US_PM sphere_pm( grid_conversion_factor, 
                          max_dimension, 
                          drho, 
                          buffer_e_density, 
                          ev, 
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

         for ( params[ 1 ] = spheretest_max; params[ 1 ] >= spheretest_min; --params[ 1 ] )
         {
            cout << QString( "---------- delta descending Sphere radius %1 -----------\n" ).arg( params[ 1 ] ).ascii();
            model.clear();
            sphere_pm.create_model( params, model );
            sphere_pm.compute_delta_I( model, prev_model, Av, I_result );
            prev_model = model;

            cout << QString( "sphere radius %1 model bead count %2\n" ).arg( params[ 1 ] ).arg( model.size() );

            // output bead model
            {
               QString outfile = QString( "%1_sh%2_DscDelta_sphere_x%2" ).arg( oname ).arg( max_harmonics ).arg( params[ 1 ] );
      
               if ( !outfile.contains( QRegExp( "\\.bead_model$" ) ) )
               {
                  outfile += ".bead_model";
               }

               cout << "Creating:" << outfile << "\n";
               QFile of( outfile );
               if ( !of.open( IO_WriteOnly ) )
               {
                  return "could not create output file";
               }
   
               QTextStream ts( &of );
               ts << sphere_pm.qs_bead_model( model );
               of.close();
            }
         
            {
               QString outfile = QString( "%1_sh%2_DscDelta_sphere_x%2" ).arg( oname ).arg( max_harmonics ).arg( params[ 1 ] );
      
               if ( !outfile.contains( QRegExp( "\\.dat$" ) ) )
               {
                  outfile += ".dat";
               }

               cout << "Creating:" << outfile << "\n";
               QFile of( outfile );
               if ( !of.open( IO_WriteOnly ) )
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
         }
         log += sphere_pm.log;
      }
      cout << log.ascii();
   }

   if ( STD_MODEL ) // standard model test
   {

      US_PM test_pm( grid_conversion_factor, 
                     max_dimension, 
                     drho, 
                     buffer_e_density, 
                     ev, 
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
         if ( !of.open( IO_WriteOnly ) )
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
         if ( !of.open( IO_WriteOnly ) )
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
      output = test_pm.qs_bead_model( model );

      // output bead model
      {
         QString outfile = oname;
      
         if ( !outfile.contains( QRegExp( "\\.bead_model$" ) ) )
         {
            outfile += ".bead_model";
         }

         cout << "Creating:" << outfile << "\n";
         QFile of( outfile );
         if ( !of.open( IO_WriteOnly ) )
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
            return "compute_CYJ_I error:" + test_pm.error_msg;
         }

         QString outfile = QString( "%1_sh%2" ).arg( oname ).arg( max_harmonics );
      
         if ( !outfile.contains( QRegExp( "\\.dat$" ) ) )
         {
            outfile += ".dat";
         }

         cout << "Creating:" << outfile << "\n";
         QFile of( outfile );
         if ( !of.open( IO_WriteOnly ) )
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
   }

   return "ok";
}
