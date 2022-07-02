#include "../include/us_saxs_util.h"
//Added by qt3to4:
#include <QTextStream>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}
// #define USUNG_DEBUG

#if defined( USUNG_DEBUG )
# if defined( USE_MPI ) 
   extern int myrank;
# endif
#endif

unsigned int US_Saxs_Util::nsa_pop_selection( unsigned int size )
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

bool US_Saxs_Util::nsa_ga_fitness( nsa_ga_individual & individual )
{
   
#if defined( USUNG_DEBUG )
# if defined( USE_MPI )
   debug_mpi( QString( "%1: entering nsa_ga_fitness individual size %2, var size %3\n" )
              .arg( myrank )
              .arg( individual.v.size() )
              .arg( nsa_var_ref.size() ) );
# else
   cout << QString( "entering nsa_ga_fitness individual size %1\n" ).arg( individual.v.size() ) << fflush;
# endif
#endif

   our_vector *vi = new_our_vector( nsa_var_ref.size() );
   for ( unsigned int i = 0; i < nsa_var_ref.size(); i++ )
   {
      vi->d[ i ] = individual.v[ i ];
   }

   double selection = drand48();
   QString method = "cg";
   if ( selection > .333 )
   {
      method = "sd";
   } 
   if ( selection > .667 )
   {
      method = "ih";
   }

#if defined( USUNG_DEBUG )
# if defined( USE_MPI )
   debug_mpi( QString( "%1: in nsa_ga_fitness: method %2\n" )
              .arg( myrank )
              .arg( method ) );
# else
   cout << QString( "in nsa_ga_fitness: method %1\n" )
      .arg( method )
      << fflush;
# endif
#endif
   nsa_gsm( individual.fitness, 
            vi,
            method );

   for ( unsigned int i = 0; i < nsa_var_ref.size(); i++ )
   {
       individual.v[ i ] = vi->d[ i ];
   }

   free_our_vector( vi );
#if defined( USUNG_DEBUG )
# if defined( USE_MPI )
   debug_mpi( QString( "%1: leaving nsa_ga_fitness individual size %2\n" ).arg( myrank ).arg( nsa_var_ref.size() ) );
# else
   cout << QString( "leaving nsa_ga_fitness individual size %1\n" ).arg( nsa_var_ref.size() ) << fflush;
# endif
#endif
   return true;
}

bool US_Saxs_Util::nsa_ga( double & nrmsd )
{
   list < nsa_ga_individual > nsa_pop;
   nsa_ga_individual          individual;

   individual.v.resize( nsa_var_ref.size() );

   // init population
   for ( unsigned int i = 0; i < control_parameters[ "nsapopulation" ].toUInt(); i++ )
   {
      for ( unsigned int j = 0; j < nsa_var_ref.size(); j++ )
      {
         individual.v[ j ] = nsa_var_min[ j ] + ( nsa_var_max[ j ] - nsa_var_min[ j ] ) * drand48();
      }
      nsa_ga_fitness( individual );
      nsa_pop.push_back( individual );
   }

   // sort by fitness
   double       last_best_fitness        = 1e99;
   unsigned int gens_with_no_improvement = 0;

   for ( unsigned int g = 0; g < control_parameters[ "nsagenerations" ].toUInt(); g++ )
   {
      nsa_pop.sort();
      nsa_pop.unique();
      
      map < unsigned int, bool > has_been_duplicated;

      if ( !g )
      {
         last_best_fitness = nsa_pop.front().fitness;
      } else {
         if ( last_best_fitness > nsa_pop.front().fitness )
         {
            last_best_fitness = nsa_pop.front().fitness;
            gens_with_no_improvement = 0;
         } else {
            gens_with_no_improvement++;
            if ( control_parameters.count( "nsaearlytermination" ) &&
                 gens_with_no_improvement >= control_parameters[ "nsaearlytermination" ].toUInt() )
            {
               cout << "early termination\n";
               break;
            }
         }
      }
            
      cout << QString( "nsa: gen %1 best individual fitness %2\n" )
         .arg( g )
         .arg( nsa_pop.front().fitness );

      unsigned int elitism_count   = 0;
      unsigned int crossover_count = 0;
      unsigned int mutate_count    = 0;
      unsigned int duplicate_count = 0;
      unsigned int random_count    = 0;

      vector < nsa_ga_individual > last_pop;
      for ( list < nsa_ga_individual >::iterator it = nsa_pop.begin();
            it != nsa_pop.end();
            it++ )
      {
         last_pop.push_back( *it );
      }

      cout << QString( "start: nsa_pop.size() %1\n" ).arg( last_pop.size() );

      nsa_pop.clear( );

      for ( unsigned int i = 0; i < control_parameters[ "nsapopulation" ].toUInt(); i++ )
      {
         if ( control_parameters.count( "nsaelitism" ) &&
              i < control_parameters[ "nsaelitism" ].toUInt() )
         {
            // cout << "elitism\n";
            nsa_ga_fitness( last_pop[ i ] );
            nsa_pop.push_back( last_pop[ i ] );
            elitism_count++;
            continue;
         }

         if ( control_parameters.count( "nsamutate" ) &&
              drand48() < control_parameters[ "nsamutate" ].toDouble() )
         {
            // cout << "mutate\n";
            individual = last_pop[ nsa_pop_selection( last_pop.size() ) ];
            unsigned int pos = ( unsigned int )( drand48() * nsa_var_ref.size() );
            individual.v[ pos ] = nsa_var_min[ pos ] + ( nsa_var_max[ pos ] - nsa_var_min[ pos ] ) * drand48();
            nsa_ga_fitness( individual );
            nsa_pop.push_back( individual );
            mutate_count++;
            continue;
         }
      
         if ( control_parameters.count( "nsacrossover" ) &&
              drand48() < control_parameters[ "nsacrossover" ].toDouble() )
         {
            // cout << "crossover\n";
            individual                    = last_pop[ nsa_pop_selection( last_pop.size() ) ];
            nsa_ga_individual individual2 = last_pop[ nsa_pop_selection( last_pop.size() ) ];
            unsigned int pos = ( unsigned int )( drand48() * nsa_var_ref.size() );
            for ( unsigned int j = pos; j < nsa_var_ref.size() ; j++ )
            {
               individual.v[ j ] = individual2.v[ j ];
            }
            nsa_ga_fitness( individual );
            nsa_pop.push_back( individual );
            crossover_count++;
            continue;
         }

         unsigned int pos = nsa_pop_selection( last_pop.size() );
         if ( has_been_duplicated.count( pos ) )
         {
            for ( unsigned int j = 0; j < nsa_var_ref.size(); j++ )
            {
               individual.v[ j ] = nsa_var_min[ j ] + ( nsa_var_max[ j ] - nsa_var_min[ j ] ) * drand48();
            }
            nsa_ga_fitness( individual );
            nsa_pop.push_back( individual );
            random_count++;
         } else {
            has_been_duplicated[ pos ] = true;
            individual = last_pop[ nsa_pop_selection( last_pop.size() ) ];
            nsa_ga_fitness( individual );
            nsa_pop.push_back( individual );
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

   nsa_pop.sort();
   nsa_pop.unique();

   for ( unsigned int i = 0; i < nsa_var_ref.size(); i++ )
   {
      *( nsa_var_ref[ i ] ) = nsa_pop.front().v[ i ];
   }

   nrmsd = nsa_fitness();

   return true;
}

#if !defined( USE_MPI )

bool US_Saxs_Util::nsa_run()
{
   QString save_outputfile = control_parameters[ "outputfile" ];
   if ( !nsa_validate() )
   {
      return false;
   }
   setup_saxs_options();
   if ( !sgp_init_sgp() )
   {
      return false;
   }

   double nrmsd;
      
   unsigned int startloop = 1;
   unsigned int endloop   = control_parameters[ "nsaspheres" ].toUInt();

   {
      QRegExp rx( "^(\\d+)\\s+(\\d+)$" );
      if ( rx.indexIn( control_parameters[ "nsaspheres" ] ) != -1 )
      {
         startloop = rx.cap( 1 ).toUInt();
         endloop   = rx.cap( 2 ).toUInt();
      }
   }
   cout << QString( "nsa run: %1 to %2\n" ).arg( startloop ).arg( endloop );

   for ( unsigned int i = startloop; i <= endloop; i++ )
   {
      if ( !nsa_fitness_setup( i ) )
      {
         return false;
      }
      cout << QString( "running nsa for size %1\n" ).arg( i );
      control_parameters[ "sgp_running" ] = "yes";
      if ( control_parameters.count( "nsasga" ) )
      {
         if ( !nsa_sga( nrmsd ) )
         {
            control_parameters.erase( "sgp_running" );
            return false;
         }
      } else {
         if ( control_parameters.count( "nsaga" ) )
         {
            if ( !nsa_ga( nrmsd ) )
            {
               control_parameters.erase( "sgp_running" );
               return false;
            }
         } else {
            if ( !nsa_gsm( nrmsd ) )
            {
               control_parameters.erase( "sgp_running" );
               return false;
            }
         }
      }

      control_parameters.erase( "sgp_running" );

      QString outname = 
         QString( "%1sa-%2%3%4%5" )
         .arg( i )
         .arg( control_parameters.count( "nsaga" ) ?
               "" : (control_parameters[ "nsagsm" ] + "-" ) )
         .arg( control_parameters.count( "nsaess" ) ?
               "ess-" : "" )
         .arg( control_parameters.count( "nsaexcl" ) ?
               "excl-" : "" )
         .arg( control_parameters[ "nsaiterations" ] );

      QFile f( QString( "%1.bead_model" ).arg( outname ) );

      if ( f.open( QIODevice::WriteOnly ) )
      {
         QTextStream ts( &f );
         ts << nsa_qs_bead_model();
         ts << nsa_physical_stats();
         ts << 
            QString(
                    "\n"
                    "nsa parameters:\n"
                    " gsm method       %1\n"
                    " max iterations   %2\n"
                    " epsilon          %3\n"
                    " scaling          %4\n"
                    )
            .arg( control_parameters[ "nsagsm" ] )
            .arg( control_parameters[ "nsaiterations" ] )
            .arg( control_parameters[ "nsaepsilon" ] )
            .arg( nsa_use_scaling_fit ? "Yes" : "No" )
            ;


         ts <<
            QString( 
                    " distance quantum %1\n"
                    " distance range   %2 %3\n"
                    " radius range     %4 %5\n"
                    )
            .arg( sgp_params[ "distancequantum" ] )
            .arg( sgp_params[ "distancemin" ] * sgp_params[ "distancequantum" ] )
            .arg( sgp_params[ "distancemax" ] * sgp_params[ "distancequantum" ] )
            .arg( sgp_params[ "radiusmin" ]   * sgp_params[ "distancequantum" ] )
            .arg( sgp_params[ "radiusmax" ]   * sgp_params[ "distancequantum" ] )
            ;

         ts << 
            QString( 
                    " target curve     %1\n" 
                    " target curve sd  %2\n" 
                    " fitness          %3\n" )
            .arg( control_parameters[ "experimentgrid" ] )
            .arg( sgp_use_e ? "present" : "not present or not useable" )
            .arg( nrmsd )
            ;

         // create json info
         {
            // should also add all the nsa_physical_params 
            map < QString, QString > out_params = control_parameters;
            out_params[ "result nrmsd" ] = QString( "%1" ).arg( nrmsd );
            for ( map < QString, QString >::iterator it = nsa_physical_stats_map.begin();
                  it !=  nsa_physical_stats_map.end();
                  it++ )
            {
               out_params[ it->first ] = it->second;
            }

            out_params[ "results vector" ] = "";
            for ( unsigned int i = 0; i < nsa_var_ref.size(); i++ )
            {
               out_params[ "results vector" ] += QString( "%1%2" )
                  .arg( i ? " " : "" )
                  .arg( *( nsa_var_ref[ i ] ) );
            }

            if ( nsa_use_scaling_fit )
            {
               out_params[ "result last scaling" ] = QString( "%1" ).arg( nsa_last_scaling );
               double delta_rho = control_parameters[ "targetedensity" ].toDouble() - our_saxs_options.water_e_density;
               if ( fabs(delta_rho) < 1e-5 )
               {
                  delta_rho = 0e0;
               }
               double delta_rho_prime = sqrt( nsa_last_scaling * delta_rho * delta_rho );
               out_params[ "result computed delta rho from scaling"  ] = QString( "%1" ).arg( delta_rho_prime );
               out_params[ "result computed target rho from scaling" ] = QString( "%1" ).arg( delta_rho_prime - delta_rho );
            }
            ts << Qt::endl << "__json:" << US_Json::compose( out_params ) << Qt::endl;
         }

         f.close();
         cout << QString( "written: %1\n" ).arg( f.fileName() );
         output_files << f.fileName();
      }
      control_parameters[ "outputfile" ] = outname;
      nsa_fitness();
      control_parameters[ "outputfile" ] = save_outputfile;
   }      

   return true;
}

#endif

