#include "../include/us_pm.h"

// note: this program uses cout and/or cerr and this should be replaced
// #define USPMGA_DEBUG

unsigned int US_PM::ga_pop_selection( unsigned int size )
{
   // exponential ranking selection
   int pos;
   double beta = size / 8e0;

   pos = ( int )( - log(1e0 - drand48() ) * beta );
   if ( pos >= ( int ) size )
   {
      pos = size - 1;
   }
   if ( pos < 0 )
   {
      pos = 0;
   }

   return ( unsigned int ) pos;
}

bool US_PM::ga_fitness( pm_ga_individual & individual )
{
   ga_delta_to_fparams( individual.v, ga_fparams );
   join( ga_params, ga_types, ga_fparams );
   individual.fitness = model_fit( ga_params, individual.model, ga_I_result );
   
   //    cout << US_Vector::qs_vector( "ga_fitness: v", individual.v );
   //    cout << US_Vector::qs_vector( "ga_fitness: params", ga_params );
   //    cout << QString( "ga_fitness: fitness %1 beads %2\n" )
   //       .arg( individual.fitness )
   //       .arg( individual.model.size() );

   return true;
}

bool US_PM::ga( pm_ga_individual & best_individual )
{
   cout << ga_info();
   pm_ga_individual      individual;

   individual.v.resize( ga_fparams_size );

   // init population
   for ( unsigned int i = 0; i < ga_population; i++ )
   {
      for ( unsigned int j = 0; j < ga_fparams_size; j++ )
      {
         individual.v[ j ] = int( ((double)ga_points * drand48() + 0.5e0 ) );
      }
      ga_fitness( individual );
      ga_pop.push_back( individual );
   }

   // sort by fitness
   double       last_best_fitness        = 1e99;
   unsigned int gens_with_no_improvement = 0;

   for ( unsigned int g = 0; g < ga_generations; g++ )
   {
      ga_pop.sort();
      ga_pop.unique();
      
      map < unsigned int, bool > has_been_duplicated;

      if ( !g )
      {
         last_best_fitness = ga_pop.front().fitness;
      } else {
         if ( last_best_fitness > ga_pop.front().fitness )
         {
            last_best_fitness = ga_pop.front().fitness;
            gens_with_no_improvement = 0;
         } else {
            gens_with_no_improvement++;
            if ( gens_with_no_improvement >= ga_early_termination )
            {
               cout << "early termination\n";
               break;
            }
         }
      }
            
      cout << QString( "ga_: gen %1 best individual fitness %2 beads %3\n" )
         .arg( g )
         .arg( ga_pop.front().fitness )
         .arg( ga_pop.front().model.size() );

      {
         ga_delta_to_fparams( ga_pop.front().v, ga_fparams );
         join( ga_params, ga_types, ga_fparams );
         cout << US_Vector::qs_vector( "ga_fitness: v", ga_pop.front().v );
         cout << US_Vector::qs_vector( "ga_fitness: params", ga_params );
      }

      unsigned int elitism_count   = 0;
      unsigned int crossover_count = 0;
      unsigned int mutate_count    = 0;
      unsigned int duplicate_count = 0;
      unsigned int random_count    = 0;

      vector < pm_ga_individual > last_pop;
      for ( list < pm_ga_individual >::iterator it = ga_pop.begin();
            it != ga_pop.end();
            it++ )
      {
         last_pop.push_back( *it );
      }

      cout << QString( "start: ga_pop.size() %1\n" ).arg( last_pop.size() );

      ga_pop.clear();

      for ( unsigned int i = 0; i < ga_population; i++ )
      {
         if ( i < ga_elitism )
         {
            // cout << "elitism\n";
            ga_fitness( last_pop[ i ] );
            ga_pop.push_back( last_pop[ i ] );
            elitism_count++;
            continue;
         }

         if ( drand48() < ga_mutate )
         {
            // cout << "mutate\n";
            individual = last_pop[ ga_pop_selection( last_pop.size() ) ];
            unsigned int pos = ( unsigned int )( drand48() * ga_fparams_size );
            individual.v[ pos ] = int( ((double)ga_points * drand48() ) + 0.5e0 );
            ga_fitness( individual );
            ga_pop.push_back( individual );
            mutate_count++;
            continue;
         }
      
         if ( drand48() < ga_crossover )
         {
            // cout << "crossover\n";
            individual                   = last_pop[ ga_pop_selection( last_pop.size() ) ];
            pm_ga_individual individual2 = last_pop[ ga_pop_selection( last_pop.size() ) ];
            unsigned int pos = ( unsigned int )( drand48() * ga_fparams_size );
            for ( unsigned int j = pos; j < ga_fparams_size ; j++ )
            {
               individual.v[ j ] = individual2.v[ j ];
            }
            ga_fitness( individual );
            ga_pop.push_back( individual );
            crossover_count++;
            continue;
         }

         unsigned int pos = ga_pop_selection( last_pop.size() );
         if ( has_been_duplicated.count( pos ) )
         {
            for ( unsigned int j = 0; j < ga_fparams_size; j++ )
            {
               individual.v[ j ] = int( ((double)ga_points * drand48() + 0.5e0 ) );
            }
            ga_fitness( individual );
            ga_pop.push_back( individual );
            random_count++;
         } else {
            has_been_duplicated[ pos ] = true;
            individual = last_pop[ ga_pop_selection( last_pop.size() ) ];
            ga_fitness( individual );
            ga_pop.push_back( individual );
            duplicate_count++;
         }
      }

      cout << QString( 
                      "summary counts:\n"
                      " elitism   %1\n"
                      " mutate    %2\n"
                      " crossover %3\n"
                      " duplicate %4\n" 
                      " random    %5\n" 
                      " total     %6\n" 
                      )
         .arg( elitism_count )
         .arg( mutate_count )
         .arg( crossover_count )
         .arg( duplicate_count )
         .arg( random_count )
         .arg( elitism_count + mutate_count + crossover_count + duplicate_count + random_count );
   }

   ga_pop.sort();
   best_individual = ga_pop.front();

   return true;
}

bool US_PM::ga_state_ok()
{
   if ( ga_low_fparams.size() != ga_high_fparams.size() )
   {
      error_msg = "ga_state_ok: not ok, fparams not equal";
      return false;
   }

   for ( int i = 0; i < (int) ga_low_fparams.size(); i++ )
   {
      if ( ga_low_fparams[ i ] > ga_high_fparams[ i ] )
      {
         error_msg = "ga_state_ok: not ok, low fparams > high fparams";
         return false;
      }
   }
   return true;
}

bool US_PM::ga_compute_delta( unsigned int points )
{
   if ( !ga_state_ok() )
   {
      return false;
   }
   if ( points < 2 )
   {
      error_msg = "ga_compute_delta: too few points";
      return false;
   }
   ga_delta.resize( ga_low_fparams.size() );
   for ( int i = 0; i < (int) ga_low_fparams.size(); i++ )
   {
      if ( ga_low_fparams[ i ] == ga_high_fparams[ i ] )
      {
         ga_delta[ i ] = 0e0;
      } else {
         ga_delta[ i ] = ( ga_high_fparams[ i ] - ga_low_fparams[ i ] ) / (double) ( points - 1 );
      }
   }
   US_Vector::printvector( "ga_compute_delta: deltas", ga_delta );
   return true;
}

bool US_PM::ga_refine_limits( unsigned int top_count, 
                              bool needs_sort_unique,
                              unsigned int extend_deltas )
{
   if ( needs_sort_unique )
   {
      ga_pop.sort();
      ga_pop.unique();
   }

   if ( (unsigned int) ga_pop.size() < top_count )
   {
      error_msg = "ga_refine_limits: insufficien population";
      return false;
   }

   vector < int > mins( ga_fparams_size );
   vector < int > maxs( ga_fparams_size );

   unsigned int i = 0;
   for ( list < pm_ga_individual >::iterator it = ga_pop.begin();
         it != ga_pop.end() && i < top_count;
         ++it, ++i )
   {
      if ( i )
      {
         for ( unsigned int j = 0; j < ga_fparams_size; ++j )
         {
            if ( mins[ j ] > it->v[ j ] )
            {
               mins[ j ] = it->v[ j ];
            }
            if ( maxs[ j ] < it->v[ j ] )
            {
               maxs[ j ] =  it->v[ j ];
            }
         }
      } else {
         mins = it->v;
         maxs = it->v;
      }
   }

   for ( unsigned int i = 0; i < ga_fparams_size; ++i )
   {
      mins[ i ] -= extend_deltas;
      maxs[ i ] += extend_deltas;
   }

   US_Vector::printvector2( "ga_refine_limits: limits before", ga_low_fparams, ga_high_fparams );

   ga_delta_to_fparams( mins, ga_low_fparams  );
   ga_delta_to_fparams( maxs, ga_high_fparams );

   US_Vector::printvector2( "ga_refine_limits: limits after", ga_low_fparams, ga_high_fparams );

   clip_limits( ga_low_fparams, ga_min_low_fparams, ga_max_high_fparams );
   clip_limits( ga_high_fparams, ga_min_low_fparams, ga_max_high_fparams );

   US_Vector::printvector2( "ga_refine_limits: limits after clipping", ga_low_fparams, ga_high_fparams );

   ga_pop.clear(); // or recompute deltas for those that fall within (?)

   return ga_compute_delta( ga_points );
}
   
bool US_PM::ga_delta_to_fparams( vector < int >    & delta, 
                                 vector < double > & fparams )
{
   for ( unsigned int i = 0; i < ga_fparams_size; ++i )
   {
      fparams[ i ] = ga_low_fparams[ i ] + (double)delta[ i ] * ga_delta[ i ];
   }
   return true;
}

bool US_PM::ga_run( 
                   vector < int >    & types, 
                   pm_ga_individual  & best_individual,
                   unsigned int        points,
                   vector < double > & low_fparams,
                   vector < double > & high_fparams
                   )
{
   ga_types        = types;
   ga_points       = points;
   ga_low_fparams  = low_fparams;
   ga_high_fparams = high_fparams;
   ga_fparams_size = low_fparams.size();
   ga_fparams      .resize( ga_fparams_size ); 

   zero_params( ga_params, types );
   set_limits ( ga_params, ga_min_low_fparams, ga_max_high_fparams );
   clip_limits( ga_params, ga_min_low_fparams, ga_max_high_fparams );

   if ( !ga_compute_delta( ga_points ) )
   {
      return false;
   }
   return ga( best_individual );
}

bool US_PM::ga_run( 
                   vector < int >    & types, 
                   pm_ga_individual  & best_individual,
                   unsigned int        points
                   )
{
   cerr << "Warning: ga_run() without limits can create huge models\n";
   vector < double > low_fparams;
   vector < double > high_fparams;
   zero_params( ga_params, types );
   set_limits ( ga_params, low_fparams, high_fparams );
   return ga_run( types, best_individual, points, low_fparams, high_fparams );
}

void US_PM::ga_set_params(
                          unsigned int        ga_population,
                          unsigned int        ga_generations,
                          double              ga_mutate,
                          double              ga_crossover,
                          unsigned int        ga_elitism,
                          unsigned int        ga_early_termination
                          )
{
   this->ga_population        = ga_population;
   this->ga_generations       = ga_generations;
   this->ga_mutate            = ga_mutate;
   this->ga_crossover         = ga_crossover;
   this->ga_elitism           = ga_elitism;
   this->ga_early_termination = ga_early_termination;
}

void US_PM::ga_set_params( map < QString, QString > control_parameters )
{
   ga_set_params();
   if ( control_parameters.count( "ga_population" ) )
   {
      ga_population = control_parameters[ "ga_population" ].toUInt();
   }
   if ( control_parameters.count( "ga_generations" ) )
   {
      ga_generations = control_parameters[ "ga_generations" ].toUInt();
   }
   if ( control_parameters.count( "ga_mutate" ) )
   {
      ga_mutate = control_parameters[ "ga_mutate" ].toDouble();
   }
   if ( control_parameters.count( "ga_crossover" ) )
   {
      ga_crossover = control_parameters[ "ga_crossover" ].toDouble();
   }
   if ( control_parameters.count( "ga_elitism" ) )
   {
      ga_elitism = control_parameters[ "ga_elitism" ].toUInt();
   }
   if ( control_parameters.count( "ga_early_termination" ) )
   {
      ga_early_termination = control_parameters[ "ga_early_termination" ].toUInt();
   }
}

QString US_PM::ga_info()
{
   QString qs;

   qs += QString( "ga_population %1\n" ).arg( ga_population );
   qs += QString( "ga_generations %1\n" ).arg( ga_generations );
   qs += QString( "ga_mutate %1\n" ).arg( ga_mutate );
   qs += QString( "ga_crossover %1\n" ).arg( ga_crossover );
   qs += QString( "ga_elitism %1\n" ).arg( ga_elitism );
   qs += QString( "ga_early_termination %1\n" ).arg( ga_early_termination );

   qs += QString( "ga_points %1\n" ).arg( ga_points );

   qs += US_Vector::qs_vector( "ga_types", ga_types );
   qs += US_Vector::qs_vector( "ga_delta", ga_delta );
   qs += US_Vector::qs_vector2( "ga_(low/high)_fparams", ga_low_fparams, ga_high_fparams );
   qs += US_Vector::qs_vector2( "ga_(min_low/max_high)_fparams", ga_min_low_fparams, ga_max_high_fparams );

   return qs;
}
