#include "../include/us_pm.h"

#define USPM_BEST_DELTA_MIN 1e-3


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
            compute_delta_I( this_model, prev_model, Av, I_result );
            this_fit = fitness2( I_result );
            fitnesses[ params[ param_to_vary ] ] = this_fit;
         } else {
            skip = true;
         }
         if ( !skip )
         {
            QString qs = 
               QString( "%1 fitness %2 beads %3 params:" )
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
            last_fitness_1_pos = params[ 1 ];

            if ( this_fit < prev_fit )
            {
               best_fit = this_fit;
               best_size = params[ 1 ];
               model = this_model;
            }
            if ( this_fit > prev_fit )
            {
               break;
            }
            prev_fit = this_fit;
            prev_size = params[ 1 ];
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
            if ( steps_without_change > 10 )
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
   best_fitness = best_fit;
   return true;
}
