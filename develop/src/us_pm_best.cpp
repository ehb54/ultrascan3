#include "../include/us_pm.h"

#define USPM_BEST_DELTA_MIN 1e-3

// #define USPM_USE_CA 0

void US_PM::set_best_delta( 
                           double best_delta_start,
                           double best_delta_divisor,
                           double best_delta_min
                           )
{
   this->best_delta_start   = best_delta_start;
   this->best_delta_divisor = best_delta_divisor;
   this->best_delta_min     = best_delta_min;

   if ( this->best_delta_min < USPM_BEST_DELTA_MIN )
   {
      this->best_delta_min = USPM_BEST_DELTA_MIN;
      cout << QString( "Notice: best delta mininimum requested %1 is below hard limit %2, reset to %3\n" )
         .arg( best_delta_min )
         .arg( USPM_BEST_DELTA_MIN )
         .arg( this->best_delta_min )
         ;
   }
   best_delta_size_min = 5e-1;
   best_delta_size_max = max_dimension_d;
}

bool US_PM::best_vary_one_param(
                                unsigned int        param_to_vary,
                                vector < double > & params, 
                                set < pm_point >  & model,
                                double            & best_fitness
                                )
{
   // use compute_delta_I
   double best_fit = 1e99;
   double prev_fit = 1e99;
   double this_fit;

   double delta      = best_delta_start;
   double prev_size;
   double best_size;

   double low_limit  = best_delta_size_min;
   double high_limit = best_delta_size_max;

   vector < double > I_result( q_points );

   set < pm_point > this_model;
   set < pm_point > prev_model;
   vector < vector < complex < float > > > Av;

   map < double, double > fitnesses;

   while ( delta >= best_delta_min )
   {
      double last_fitness_1_pos = -1e0;
      double last_fitness_2_pos = -1e0;
      double last_fitness_3_pos = -1e0;
      prev_fit = 1e99;

      prev_model.clear();
      Av.clear();

      cout << QString( "this limit %1 %2 delta %3\n" ).arg( low_limit ).arg( high_limit ).arg( delta );
      unsigned int steps_without_change = 0;
      for ( params[ param_to_vary ] = low_limit; params[ param_to_vary ] <= high_limit; params[ param_to_vary ] += delta )
      {
         bool skip = false;
         create_model( params, this_model );
         cout << QString( "create model size %1 prev model size %2\n" ).arg( this_model.size() ).arg( prev_model.size() );
         if ( this_model.size() &&  prev_model.size() != this_model.size() )
         {
            //             if ( USPM_USE_CA )
            //             {
            //                compute_I( this_model, I_result );
            //             } else {
            compute_delta_I( this_model, prev_model, Av, I_result );
            //             }
            this_fit = fitness2( I_result );
            fitnesses[ params[ param_to_vary ] ] = this_fit;
         } else {
            skip = true;
         }
         if ( !skip )
         {
            // write_model( tmp_name( "", params ), this_model );
            QString qs = 
               QString( "%1 fitness^2 %2 beads %3 params:" )
               .arg( object_names[ (int) params[ 0 ] ] )
               .arg( this_fit, 0, 'g', 8 )
               .arg( this_model.size() )
               ;
            for ( int i = 1; i <= (int)object_m0_parameters[ (int) params[ 0 ] ]; ++i )
            {
               qs += QString( " %1" ).arg( params[ i ] );
            }
            cout << qs.ascii() << endl;

            last_fitness_3_pos = last_fitness_2_pos;
            last_fitness_2_pos = last_fitness_1_pos;
            last_fitness_1_pos = params[ param_to_vary ];

            if ( this_fit < prev_fit )
            {
               best_fit = this_fit;
               best_size = params[ param_to_vary ];
               model = this_model;
            }
            if ( this_fit > prev_fit )
            {
               break;
            }
            prev_fit = this_fit;
            prev_size = params[ param_to_vary ];
            steps_without_change = 0;
         } else {
            QString qs = 
               QString( "skipping %1 (empty or identical to previous) beads %2 prev %3 params:" )
               .arg( object_names[ (int) params[ 0 ] ] )
               .arg( this_model.size() )
               .arg( prev_model.size() )
               ;
            for ( int i = 1; i <= (int)object_m0_parameters[ (int) params[ 0 ] ]; ++i )
            {
               qs += QString( " %1" ).arg( params[ i ] );
            }
            cout << qs.ascii() << endl;
            if ( steps_without_change > 100 )
            {
               cout << "To many steps without model change, terminating inner loop\n";
               break;
            }
            steps_without_change++;
         }            
         prev_model = this_model;
      }
      if ( last_fitness_3_pos < 0e0 ||
           last_fitness_1_pos < 0e0 )
      {
         cout << "Best found nothing to recenter on, terminating outer loop\n";
         break;
      }
         
      low_limit  = last_fitness_3_pos;
      high_limit = last_fitness_1_pos;
      delta /= best_delta_divisor;
   }
   best_fitness    = best_fit;
   return true;
}

bool US_PM::best_vary_two_param( 
                                unsigned int        param_to_vary_1,
                                unsigned int        param_to_vary_2,
                                vector < double > & params, 
                                set < pm_point >  & model,
                                double            & best_fitness
                                )
{
   set < pm_point > this_model;
   use_CYJ = false;

   QString qs_timer = QString( "%1 2 params to vary" ).arg( object_names[ (int) params[ 0 ] ] );

   us_timers.clear_timers();
   us_timers.init_timer  ( qs_timer );
   us_timers.start_timer ( qs_timer );

   double delta      = best_delta_start;
   double low_limit  = best_delta_size_min;
   double high_limit = best_delta_size_max;

   double prev_size;
   double best_size;

   double best_fit = 1e99;
   double prev_fit = 1e99;
   double this_fit;

   map < double, double > fitnesses;
   while ( delta > best_delta_min )
   {
      double last_fitness_1_pos = -1e0;
      double last_fitness_2_pos = -1e0;
      double last_fitness_3_pos = -1e0;
      prev_fit = 1e99;

      cout << QString( "this limit %1 %2 delta %3\n" ).arg( low_limit ).arg( high_limit ).arg( delta );
      for ( params[ param_to_vary_2 ] = low_limit; params[ param_to_vary_2 ] <= high_limit; params[ param_to_vary_2 ] += delta )
      {
         best_vary_one_param( param_to_vary_1, params, this_model, this_fit );
         fitnesses[ params[ param_to_vary_2 ] ] = this_fit;

         last_fitness_3_pos = last_fitness_2_pos;
         last_fitness_2_pos = last_fitness_1_pos;
         last_fitness_1_pos = params[ param_to_vary_2 ];

         if ( this_fit < prev_fit )
         {
            best_fit = this_fit;
            best_size = params[ param_to_vary_2 ];
            model = this_model;
         }
         if ( this_fit > prev_fit )
         {
            break;
         }
         prev_fit = this_fit;
         prev_size = params[ param_to_vary_2 ];

      }
      low_limit  = last_fitness_3_pos;
      high_limit = last_fitness_1_pos;
      delta /= best_delta_divisor;
   }
   us_timers.end_timer( qs_timer );
   cout << us_timers.list_times();

   best_fitness   = best_fit;
   return true;
}

// set params[ 0 ] and run

bool US_PM::best_md0( 
                     vector < double > & params, 
                     set < pm_point >  & model, 
                     double              finest_conversion,
                     double              coarse_conversion,
                     double              refinement_range_pct,
                     double              conversion_divisor
                     )
{
   if ( !zero_md0_params( params ) )
   {
      error_msg = "best_md0: " + error_msg;
      return false;
   }

   set_grid_size( coarse_conversion );

   vector < double > low_fparams;
   vector < double > high_fparams;
   set_limits( params, low_fparams, high_fparams );
   vector < double > next_low_fparams  = low_fparams;
   vector < double > next_high_fparams = high_fparams;

   double new_grid_conversion_factor = grid_conversion_factor;
   
   while ( grid_conversion_factor >= finest_conversion )
   {
      // need params[] from best model and ability to set limits for best model
      // have to add to best_*, maybe add general best_model( params )

      cout << "----------------------------------------------------------------------\n";
      cout << QString( "best_md0: grid cube side: %1\n" ).arg( grid_conversion_factor);
      cout << "----------------------------------------------------------------------\n";
      
      clip_limits( next_low_fparams , low_fparams, high_fparams );
      clip_limits( next_high_fparams, low_fparams, high_fparams );
      low_fparams  = next_low_fparams;
      high_fparams = next_high_fparams;

      US_Vector::printvector2( "param limits:", low_fparams, high_fparams );

      if ( !best_md0( params, low_fparams, high_fparams, model ) )
      {
         return false;
      }

      // convert limits to new factor

      if ( grid_conversion_factor == finest_conversion )
      {
         break;
      }

      new_grid_conversion_factor = grid_conversion_factor / conversion_divisor;
      if ( new_grid_conversion_factor < finest_conversion )
      {
         new_grid_conversion_factor = finest_conversion;
      }
 
      US_Vector::printvector ( "result params:", params );
      US_Vector::printvector2( "previous limits before rescaling:", low_fparams, high_fparams );

      for ( int i = 0; i < (int)low_fparams.size(); i++ )
      {
         next_low_fparams [ i ] = params[ i + 1 ] - grid_conversion_factor * refinement_range_pct / 100e0;
         next_high_fparams[ i ] = params[ i + 1 ] + grid_conversion_factor * refinement_range_pct / 100e0;

         next_low_fparams [ i ] *= grid_conversion_factor / new_grid_conversion_factor;
         next_high_fparams[ i ] *= grid_conversion_factor / new_grid_conversion_factor;
         low_fparams      [ i ] *= grid_conversion_factor / new_grid_conversion_factor;
         high_fparams     [ i ] *= grid_conversion_factor / new_grid_conversion_factor;
      }

      US_Vector::printvector2( "previous limits after rescaling:", low_fparams, high_fparams );
      US_Vector::printvector2( "new limits after rescaling:", next_low_fparams, next_high_fparams );

      set_grid_size( new_grid_conversion_factor );
   }
   return true;
}

bool US_PM::best_vary_one_param(
                                unsigned int        param_to_vary,
                                vector < double > & params,
                                vector < double > & low_fparams,
                                vector < double > & high_fparams,
                                set < pm_point >  & model,
                                double            & best_fitness )
{
   // use compute_delta_I
   double best_fit = 1e99;
   double prev_fit = 1e99;
   double this_fit;

   double delta      = best_delta_start;
   double prev_size;
   double best_size;

   double low_limit  = low_fparams [ param_to_vary - 1 ];
   double high_limit = high_fparams[ param_to_vary - 1 ];

   vector < double > I_result( q_points );

   if ( high_limit - low_limit <= best_delta_min )
   {
      cout << QString( "best_vary_one_param: parameter range %1:%2 is smaller than minimum delta %3, returning average model\n" )
         .arg( low_limit )
         .arg( high_limit )
         .arg( best_delta_min );
      params[ param_to_vary ] = (high_limit + low_limit) / 2e0;
      create_model( params, model );
      compute_I   ( model, I_result );
      best_fitness = fitness2( I_result );
      return true;
   }

   if ( ( high_limit - low_limit ) / 10e0 < delta )
   {
      delta = ( high_limit - low_limit ) / 10e0;
   }
   if ( delta < best_delta_min )
   {
      delta = best_delta_min;
   }

   vector < double > best_params;

   set < pm_point > this_model;
   set < pm_point > prev_model;
   vector < vector < complex < float > > > Av;

   map < double, double > fitnesses;

   while ( delta >= best_delta_min )
   {
      double last_fitness_1_pos = -1e0;
      double last_fitness_2_pos = -1e0;
      double last_fitness_3_pos = -1e0;
      prev_fit = 1e99;

      prev_model.clear();
      Av.clear();

      cout << QString( "best_vary_one_param: ------------ this limit %1 %2 delta %3\n" ).arg( low_limit ).arg( high_limit ).arg( delta );
      unsigned int steps_without_change = 0;
      for ( params[ param_to_vary ] = low_limit; params[ param_to_vary ] <= high_limit; params[ param_to_vary ] += delta )
      {
         bool skip = false;
         create_model( params, this_model );
         // cout << QString( "create model size %1 prev model size %2\n" ).arg( this_model.size() ).arg( prev_model.size() );
         if ( this_model.size() &&  prev_model.size() != this_model.size() )
         {
            //             if ( USPM_USE_CA )
            //             {
            //                compute_I( this_model, I_result );
            //             } else {
            compute_delta_I( this_model, prev_model, Av, I_result );
            //             }
            this_fit = fitness2( I_result );
            fitnesses[ params[ param_to_vary ] ] = this_fit;
         } else {
            skip = true;
         }
         if ( !skip )
         {
            // write_model( tmp_name( "", params ), this_model );
            //             QString qs = 
            //                QString( "%1 fitness^2 %2 beads %3 params:" )
            //                .arg( object_names[ (int) params[ 0 ] ] )
            //                .arg( this_fit, 0, 'g', 8 )
            //                .arg( this_model.size() )
            //                ;
            //             for ( int i = 1; i <= (int)object_m0_parameters[ (int) params[ 0 ] ]; ++i )
            //             {
            //                qs += QString( " %1" ).arg( params[ i ] );
            //             }
            //             cout << qs.ascii() << endl;
            US_Vector::printvector( QString( "best_vary_one_param: %1beads %2 fitness %3, params:" )
                                    .arg( this_fit < prev_fit ? "**" : "  " )
                                    .arg( this_model.size() )
                                    .arg( this_fit ), 
                                    params );

            last_fitness_3_pos = last_fitness_2_pos;
            last_fitness_2_pos = last_fitness_1_pos;
            last_fitness_1_pos = params[ param_to_vary ];

            if ( this_fit < prev_fit )
            {
               best_fit = this_fit;
               best_size = params[ param_to_vary ];
               model = this_model;
               best_params = params;
            }
            if ( this_fit > prev_fit )
            {
               break;
            }
            prev_fit = this_fit;
            prev_size = params[ param_to_vary ];
            steps_without_change = 0;
         } else {
            //             QString qs = 
            //                QString( "skipping %1 (empty or identical to previous) beads %2 prev %3 params:" )
            //                .arg( object_names[ (int) params[ 0 ] ] )
            //                .arg( this_model.size() )
            //                .arg( prev_model.size() )
            //                ;
            //             for ( int i = 1; i <= (int)object_m0_parameters[ (int) params[ 0 ] ]; ++i )
            //             {
            //                qs += QString( " %1" ).arg( params[ i ] );
            //             }
            //             cout << qs.ascii() << endl;
            if ( steps_without_change > 100 )
            {
               cout << "best_vary_one_param: Too many steps without model change, terminating inner loop\n";
               break;
            }
            steps_without_change++;
         }            
         prev_model = this_model;
      }
      if ( last_fitness_3_pos < 0e0 ||
           last_fitness_1_pos < 0e0 )
      {
         cout << "best_vary_one_param: found nothing to recenter on, terminating outer loop\n";
         break;
      }
         
      low_limit  = last_fitness_3_pos;
      high_limit = last_fitness_1_pos;
      delta /= best_delta_divisor;
   }
   best_fitness    = best_fit;
   params          = best_params;
   US_Vector::printvector( QString( "best_vary_one_param: ----end----- fitness %1, params:" ).arg( best_fitness ), params );
   return true;
}

bool US_PM::best_vary_two_param( 
                                unsigned int        param_to_vary_1,
                                unsigned int        param_to_vary_2,
                                vector < double > & params, 
                                vector < double > & low_fparams,
                                vector < double > & high_fparams,
                                set < pm_point >  & model,
                                double            & best_fitness
                                )
{
   set < pm_point > this_model;
   use_CYJ = false;

   QString qs_timer = QString( "%1 2 params to vary" ).arg( object_names[ (int) params[ 0 ] ] );

   us_timers.clear_timers();
   us_timers.init_timer  ( qs_timer );
   us_timers.start_timer ( qs_timer );

   double delta      = best_delta_start;
   double prev_size;
   double best_size;

   double low_limit  = low_fparams [ param_to_vary_2 - 1 ];
   double high_limit = high_fparams[ param_to_vary_2 - 1 ];

   double best_fit = 1e99;
   double prev_fit = 1e99;
   double this_fit;

   if ( high_limit - low_limit <= best_delta_min )
   {
      cout << QString( "best_vary_two_param: parameter range %1:%2 is smaller than minimum delta %3, returning average model\n" )
         .arg( low_limit )
         .arg( high_limit )
         .arg( best_delta_min );
      params[ param_to_vary_2 ] = (high_limit + low_limit) / 2e0;
      return best_vary_one_param( param_to_vary_1, params, low_fparams, high_fparams, model, best_fitness );
   }

   if ( ( high_limit - low_limit ) / 10e0 < delta )
   {
      delta = ( high_limit - low_limit ) / 10e0;
   }
   if ( delta < best_delta_min )
   {
      delta = best_delta_min;
   }

   vector < double > best_params;
   map < double, double > fitnesses;

   while ( delta > best_delta_min )
   {
      double last_fitness_1_pos = -1e0;
      double last_fitness_2_pos = -1e0;
      double last_fitness_3_pos = -1e0;
      prev_fit = 1e99;

      cout << QString( "best_vary_two_param: ------------ this limit %1 %2 delta %3\n" ).arg( low_limit ).arg( high_limit ).arg( delta );
      for ( params[ param_to_vary_2 ] = low_limit; params[ param_to_vary_2 ] <= high_limit; params[ param_to_vary_2 ] += delta )
      {
         best_vary_one_param( param_to_vary_1, params, low_fparams, high_fparams, this_model, this_fit );
         fitnesses[ params[ param_to_vary_2 ] ] = this_fit;

         last_fitness_3_pos = last_fitness_2_pos;
         last_fitness_2_pos = last_fitness_1_pos;
         last_fitness_1_pos = params[ param_to_vary_2 ];

         US_Vector::printvector( QString( "best_vary_two_param: %1beads %2 fitness %3, params:" )
                                 .arg( this_fit < prev_fit ? "**" : "  " )
                                 .arg( this_model.size() )
                                 .arg( this_fit ), 
                                 params );

         if ( this_fit < prev_fit )
         {
            best_fit = this_fit;
            best_size = params[ param_to_vary_2 ];
            model = this_model;
            best_params = params;
         }
         if ( this_fit > prev_fit )
         {
            break;
         }
         prev_fit = this_fit;
         prev_size = params[ param_to_vary_2 ];

      }
      if ( last_fitness_3_pos < 0e0 ||
           last_fitness_1_pos < 0e0 )
      {
         cout << "best_vary_two_param: found nothing to recenter on, terminating outer loop\n";
         break;
      }
      low_limit  = last_fitness_3_pos;
      high_limit = last_fitness_1_pos;
      delta /= best_delta_divisor;
   }
   us_timers.end_timer( qs_timer );
   cout << us_timers.list_times();

   best_fitness   = best_fit;
   params         = best_params;
   US_Vector::printvector( QString( "best_vary_two_param: ----end----- fitness %1, params:" ).arg( best_fitness ), params );
   return true;
}
