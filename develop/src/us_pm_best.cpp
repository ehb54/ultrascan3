#include "../include/us_pm.h"

double US_PM::model_fit( vector < double > & params, set < pm_point > & model, vector < double > & I_result )
{
   create_model( params, model );
   compute_cached_I( model, I_result );
   return fitness2( I_result );
}

bool US_PM::best_sphere( set < pm_point > & model, double delta )
{
   // start with 1 sphere, slowly increase size until fitness drops

   double best_fit = 1e99;
   double prev_fit = 1e99;
   double this_fit;

   vector < double > params( 2 );
   params[ 0 ] = 0e0;
   vector < double > I_result( q_points );

   set < pm_point > this_model;

   us_timers.clear_timers();
   if ( 0 )
   {
      us_timers.init_timer( "sphere try with CA" );
      for ( params[ 1 ] = 0.1; params[ 1 ] <= max_dimension_d; params[ 1 ] += delta )
      {
         create_model( params, this_model );
         compute_cached_I( this_model, I_result );
         this_fit = fitness2( I_result );
         cout << QString( "radius %1 beads %2 fitness %3\n" ).arg( params[ 1 ] ).arg( model.size() ).arg( this_fit ).ascii();
         if ( this_fit < prev_fit )
         {
            best_fit = this_fit;
            model = this_model;
         }
         if ( this_fit > prev_fit )
         {
            break;
         }
         prev_fit = this_fit;
      }
      us_timers.end_timer( "sphere try with CA" );
   }


   if ( 1 )
   {
      us_timers.init_timer( "sphere try with CA bisection" );
      // find initial range
      prev_fit         = 1e99;

      double prev_size   = 1e-1;
      double left_size   = 1e-1;
      double left_fit    = 1e99;
      double center_size = 1e-1;
      double center_fit  = 1e99;
      double right_size  = -1e0;
      double right_fit   = 1e99;

      for ( params[ 1 ] = 0.1; params[ 1 ] <= max_dimension_d; params[ 1 ] += 2e0 )
      {
         this_fit = model_fit( params, model, I_result );
         cout << QString( "radius %1 beads %2 fitness %3\n" ).arg( params[ 1 ] ).arg( model.size() ).arg( this_fit ).ascii();
         if ( this_fit < prev_fit )
         {
            left_size = prev_size;
            left_fit  = prev_fit;
         }
         if ( this_fit > prev_fit )
         {
            center_size = prev_size;
            center_fit  = prev_fit;
            right_size   = params[ 1 ];
            right_fit    = this_fit;
            break;
         }
         prev_fit = this_fit;
         prev_size = params[ 1 ];
      }
      while ( right_size - left_size > delta )
      {
         cout << QString( "bisection bracket %1 %2 %3 delta %4\n" ).arg( left_size ).arg( center_size ).arg( right_size ).arg( delta );

         // compute parabolic fit

         double center_minus_left_size  = center_size - left_size;
         double center_minus_right_size = center_size - right_size;
         double center_minus_left_fit   = center_fit  - left_fit;
         double center_minus_right_fit  = center_fit  - right_fit;

         double trial_size = center_size - 5e-1 * 
            ( center_minus_left_size  * center_minus_left_size  * center_minus_right_fit -
              center_minus_right_size * center_minus_right_size * center_minus_left_fit  )
            / 
            ( center_minus_left_size  * center_minus_right_fit -
              center_minus_right_size * center_minus_left_fit  );

         params[ 1 ] = trial_size;
         double trial_fit = model_fit( params, model, I_result );
         cout << QString( "radius %1 beads %2 fitness %3\n" ).arg( params[ 1 ] ).arg( model.size() ).arg( trial_fit ).ascii();

         if ( trial_size < center_size )
         {
            right_fit  = center_fit;
            right_size = center_size;
         } else {
            left_fit  = center_fit;
            left_size = center_size;
         }
         center_size = trial_size;
         center_fit  = trial_fit;
         cout << QString( "end bisection bracket %1 %2 %3 delta %4\n" ).arg( left_size ).arg( center_size ).arg( right_size ).arg( delta );
      }         
         
      us_timers.end_timer( "sphere try with CA bisection" );
   }

   if ( 0 )
   {
      us_timers.init_timer( "sphere try with CA coarse, bisection" );
      us_timers.start_timer( "sphere try with Delta" );
      {
         prev_fit = 1e99;
         best_fit = 1e99;
         set < pm_point >                        prev_model;
         vector < vector < complex < float > > > Av;

         for ( params[ 1 ] = 0.1; params[ 1 ] <= max_dimension_d; params[ 1 ] += delta )
         {
            create_model( params, this_model );
            compute_delta_I( this_model, prev_model, Av, I_result );
            prev_model = this_model;
            this_fit = fitness2( I_result );
            cout << QString( "radius %1 beads %2 fitness %3\n" ).arg( params[ 1 ] ).arg( model.size() ).arg( this_fit ).ascii();
            if ( this_fit < prev_fit )
            {
               best_fit = this_fit;
               model = this_model;
            }
            if ( this_fit > prev_fit )
            {
               break;
            }
            prev_fit = this_fit;
         }
      }
      us_timers.end_timer( "sphere try with Delta" );
   }

   cout << us_timers.list_times();

   return false;
}


bool US_PM::best_cylinder( set < pm_point > & /* model */ )
{
   return false;
}
bool US_PM::best_spheroid( set < pm_point > & /* model */ )
{
   return false;
}
bool US_PM::best_ellipsoid( set < pm_point > & /* model */ )
{
   return false;
}
bool US_PM::best_torus( set < pm_point > & /* model */ )
{
   return false;
}
