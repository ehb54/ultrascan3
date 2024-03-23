#include "../include/us_hydrodyn_dad_fit.h"

// #define  UHSHFG_DEBUG
// #define  UHSHFG_DEBUG2

unsigned int US_Hydrodyn_Dad_Fit::pop_selection( unsigned int size )
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

bool US_Hydrodyn_Dad_Fit::ga_fitness( ga_individual & individual )
{
   
#if defined( UHSHFG_DEBUG )
   cout << QString( "entering ga_fitness individual size %1\n" ).arg( individual.v.size() ) << fflush;
#endif

   our_vector *vi = new_our_vector( DFIT::init_params.size() );
   for ( unsigned int i = 0; i < DFIT::init_params.size(); i++ )
   {
      vi->d[ i ] = individual.v[ i ];
   }

   individual.fitness = gsm_f( vi );

   for ( unsigned int i = 0; i < DFIT::init_params.size(); i++ )
   {
       individual.v[ i ] = vi->d[ i ];
   }

   free_our_vector( vi );
#if defined( UHSHFG_DEBUG )
   cout << QString( "leaving ga_fitness individual size %1\n" ).arg( var_ref.size() ) << fflush;
#endif
   return true;
}

bool US_Hydrodyn_Dad_Fit::ga_run( double & nrmsd )
{
   map < QString, QString > control_parameters;
   control_parameters[ "generations"      ] = le_iterations->text();
   control_parameters[ "population"       ] = le_population->text();
   control_parameters[ "crossover"        ] = "0.3";
   control_parameters[ "mutate"           ] = "0.5";
   control_parameters[ "earlytermination" ] = "5";
   control_parameters[ "elitism"          ] = "2";

   list < ga_individual > pop;
   ga_individual          individual;

   individual.v.resize( DFIT::init_params.size() );

   // init population
   for ( unsigned int i = 0; i < control_parameters[ "population" ].toUInt(); i++ )
   {
      for ( unsigned int j = 0; j < DFIT::init_params.size(); j++ )
      {
         individual.v[ j ] = DFIT::param_min[ j ] + ( DFIT::param_max[ j ] - DFIT::param_min[ j ] ) * drand48();
      }
      ga_fitness( individual );
      pop.push_back( individual );
   }

   // sort by fitness
   double       last_best_fitness        = 1e99;
   unsigned int gens_with_no_improvement = 0;

   for ( unsigned int g = 0; g < control_parameters[ "generations" ].toUInt(); g++ )
   {
      pop.sort();
      pop.unique();
      
      map < unsigned int, bool > has_been_duplicated;

      if ( !g )
      {
         last_best_fitness = pop.front().fitness;
      } else {
         if ( last_best_fitness > pop.front().fitness )
         {
            last_best_fitness = pop.front().fitness;
            gens_with_no_improvement = 0;
         } else {
            gens_with_no_improvement++;
            if ( control_parameters.count( "earlytermination" ) &&
                 gens_with_no_improvement >= control_parameters[ "earlytermination" ].toUInt() )
            {
#if defined( UHSHFG_DEBUG2 )
               cout << "early termination\n";
#endif
               break;
            }
         }
      }
            
#if defined( UHSHFG_DEBUG2 )
      cout << QString( ": gen %1 best individual fitness %2\n" )
         .arg( g )
         .arg( pop.front().fitness );
#endif

      unsigned int elitism_count   = 0;
      unsigned int crossover_count = 0;
      unsigned int mutate_count    = 0;
      unsigned int duplicate_count = 0;
      unsigned int random_count    = 0;

      vector < ga_individual > last_pop;
      for ( list < ga_individual >::iterator it = pop.begin();
            it != pop.end();
            it++ )
      {
         last_pop.push_back( *it );
      }

#if defined( UHSHFG_DEBUG2 )
      cout << QString( "start: pop.size() %1\n" ).arg( last_pop.size() );
#endif

      pop.clear( );

      for ( unsigned int i = 0; i < control_parameters[ "population" ].toUInt(); i++ )
      {
         if ( control_parameters.count( "elitism" ) &&
              i < control_parameters[ "elitism" ].toUInt() )
         {
            // cout << "elitism\n";
            ga_fitness( last_pop[ i ] );
            pop.push_back( last_pop[ i ] );
            elitism_count++;
            continue;
         }

         if ( control_parameters.count( "mutate" ) &&
              drand48() < control_parameters[ "mutate" ].toDouble() )
         {
            // cout << "mutate\n";
            individual = last_pop[ pop_selection( last_pop.size() ) ];
            unsigned int pos = ( unsigned int )( drand48() * DFIT::init_params.size() );
            individual.v[ pos ] = DFIT::param_min[ pos ] + ( DFIT::param_max[ pos ] - DFIT::param_min[ pos ] ) * drand48();
            ga_fitness( individual );
            pop.push_back( individual );
            mutate_count++;
            continue;
         }
      
         if ( control_parameters.count( "crossover" ) &&
              drand48() < control_parameters[ "crossover" ].toDouble() )
         {
            // cout << "crossover\n";
            individual                    = last_pop[ pop_selection( last_pop.size() ) ];
            ga_individual individual2 = last_pop[ pop_selection( last_pop.size() ) ];
            unsigned int pos = ( unsigned int )( drand48() * DFIT::init_params.size() );
            for ( unsigned int j = pos; j < DFIT::init_params.size() ; j++ )
            {
               individual.v[ j ] = individual2.v[ j ];
            }
            ga_fitness( individual );
            pop.push_back( individual );
            crossover_count++;
            continue;
         }

         unsigned int pos = pop_selection( last_pop.size() );
         if ( has_been_duplicated.count( pos ) )
         {
            for ( unsigned int j = 0; j < DFIT::init_params.size(); j++ )
            {
               individual.v[ j ] = DFIT::param_min[ j ] + ( DFIT::param_max[ j ] - DFIT::param_min[ j ] ) * drand48();
            }
            ga_fitness( individual );
            pop.push_back( individual );
            random_count++;
         } else {
            has_been_duplicated[ pos ] = true;
            individual = last_pop[ pop_selection( last_pop.size() ) ];
            ga_fitness( individual );
            pop.push_back( individual );
            duplicate_count++;
         }
      }

#if defined( UHSHFG_DEBUG2 )
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
#endif
   }

   pop.sort();
   pop.unique();

   for ( unsigned int i = 0; i < DFIT::init_params.size(); i++ )
   {
      DFIT::init_params[ i ] = pop.front().v[ i ];
   }

   nrmsd = pop.front().fitness;

   return true;
}
