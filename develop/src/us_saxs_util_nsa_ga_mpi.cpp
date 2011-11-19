#include "../include/us_saxs_util.h"

#include <mpi.h>
extern int npes;
extern int myrank;

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
   // cout << QString( "%1: nsa_mpi is %2\n" ).arg( myrank ).arg( nsa_mpi ? "true" : "false" );
   for ( unsigned int i = 1; i <= control_parameters[ "nsarun" ].toUInt(); i++ )
   {
      if ( !nsa_fitness_setup( i ) )
      {
         return false;
      }
      if ( !myrank )
      {
         cout << QString( "%1: running nsa for size %2\n" ).arg( myrank ).arg( i ) << flush;
      }

      control_parameters[ "sgp_running" ] = "yes";
      if ( control_parameters.count( "nsaga" ) )
      {
         if ( !nsa_mpi )
         {
            if ( !nsa_ga( nrmsd ) )
            {
               control_parameters.erase( "sgp_running" );
               return false;
            }
         } else {
            if ( myrank )
            {
               if ( !nsa_ga_worker() )
               {
                  return false;
               }
                  
            } else {
               if ( !nsa_ga_master( nrmsd ) )
               {
                  return false;
               }
            }
         }
      } else {
         if ( nsa_mpi )
         {
            errormsg = "nsagsm without ga not currently supported under nsa mpi";
            return false;
         }
         if ( !nsa_gsm( nrmsd ) )
         {
            control_parameters.erase( "sgp_running" );
            return false;
         }
      }

      control_parameters.erase( "sgp_running" );

      if ( !nsa_mpi || !myrank )
      {
         QString outname = 
            QString( "%1sa-%2%3" )
            .arg( i )
            .arg( control_parameters.count( "nsaga" ) ?
                  "" : (control_parameters[ "nsagsm" ] + "-" ) )
            .arg( control_parameters[ "nsaiterations" ] );

         QFile f( QString( "%1.bead_model" ).arg( outname ) );
         
         if ( f.open( IO_WriteOnly ) )
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
                       )
               .arg( control_parameters[ "nsagsm" ] )
               .arg( control_parameters[ "nsaiterations" ] )
               .arg( control_parameters[ "nsaepsilon" ] )
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
            
            f.close();
            cout << QString( "written: %1\n" ).arg( f.name() );
            output_files << f.name();
         }
         control_parameters[ "outputfile" ] = outname;
         nsa_fitness();
      }
      control_parameters[ "outputfile" ] = save_outputfile;
   }

   return true;
}

bool US_Saxs_Util::nsa_ga_worker()
{
   // cout << QString( "%1: nsa_ga_worker starting\n" ).arg( myrank ) << flush;
   int errorno = -23000;

   // cout << QString("%1: enter nsa ga initial barrier\n" ).arg( myrank ) << flush;
   if ( MPI_SUCCESS != MPI_Barrier( MPI_COMM_WORLD ) )
   {
      MPI_Abort( MPI_COMM_WORLD, errorno - myrank );
      exit( errorno - myrank );
   }         

   // cout << QString("%1: exit nsa ga initial barrier\n" ).arg( myrank ) << flush;
   
   MPI_Status mpi_status;

   vector < double > message_received( nsa_var_ref.size() + 2 );
   vector < double > message_sent    ( nsa_var_ref.size() + 2 );

   nsa_ga_individual individual;
   individual.v.resize( nsa_var_ref.size() );

   message_sent[ 0 ] = 0.0;

   errorno = -24000;

   if ( MPI_SUCCESS != MPI_Send( &( message_sent[ 0 ] ),
                                 message_sent.size(),
                                 MPI_DOUBLE, 
                                 0,
                                 0, 
                                 MPI_COMM_WORLD ) )
   {
      cout << QString( "%1: MPI send failed in nsa_ga_worker() initial registry\n" ).arg( myrank ) << flush;
      MPI_Abort( MPI_COMM_WORLD, errorno - myrank );
      exit( errorno - myrank );
   }

   errorno = -25000;

   do 
   {
      if ( MPI_SUCCESS != MPI_Recv( &( message_received[ 0 ] ),
                                    message_received.size(), 
                                    MPI_DOUBLE, 
                                    0, 
                                    MPI_ANY_TAG, 
                                    MPI_COMM_WORLD, 
                                    &mpi_status ) )
      {
         cout << QString( "%1: MPI Received failed in nsa_ga_worker()\n" ).arg( myrank ) << flush;
         MPI_Abort( MPI_COMM_WORLD, errorno - myrank );
         exit( errorno - myrank );
      }         
      
      if ( message_received[ 0 ] == 1.0 )
      {
         for ( unsigned int i = 0; i < nsa_var_ref.size(); i++ )
         {
            individual.v[ i ] = message_received[ i + 2 ];
         }
         if ( !nsa_ga_fitness( individual ) )
         {
            errorno -= 1000;
            cout << QString( "%1: nsa_ga_fitness() failed nsa_ga_worker()\n" ).arg( myrank ) << flush;
            MPI_Abort( MPI_COMM_WORLD, errorno - myrank );
            exit( errorno - myrank );
         }         
         
         // send results

         message_sent[ 0 ] = 1.0;
         message_sent[ 1 ] = individual.fitness;
         // QString vmsg;
         for ( unsigned int i = 0; i < nsa_var_ref.size(); i++ )
         {
            message_sent[ i + 2 ] = individual.v[ i ];
            // vmsg += QString( " %1" ).arg( individual.v[ i ] );
         }

         // cout << QString( "%1: worker sending result (%2): %3\n" )
         // .arg( myrank )
         // .arg( individual.fitness )
         // .arg( vmsg ) << flush;

         if ( MPI_SUCCESS != MPI_Send( &( message_sent[ 0 ] ),
                                       message_sent.size(),
                                       MPI_DOUBLE, 
                                       0,
                                       0, 
                                       MPI_COMM_WORLD ) )
         {
            errorno -= 2000;
            cout << QString( "%1: MPI send failed in nsa_ga_worker() returning results\n" ).arg( myrank ) << flush;
            MPI_Abort( MPI_COMM_WORLD, errorno );
            exit( errorno );
         }
      }
   } while ( message_received[ 0 ] != 0.0 );

   // cout << QString( "%1: nsa_ga_worker closing\n" ).arg( myrank ) << flush;
   return true;
}

bool US_Saxs_Util::nsa_ga_close_workers()
{
   int errorno = -22000;
   vector < double > message_sent    ( nsa_var_ref.size() + 2 );
   message_sent[ 0 ] = 0.0;

   for ( map < int, bool >::iterator it = waiting_workers.begin();
         it != waiting_workers.end();
         it++ )
   {
      // cout << QString( "%1: sending close request to worker %2\n" ).arg( myrank ).arg(  it->first ) << flush;
      if ( MPI_SUCCESS != MPI_Send( &( message_sent[ 0 ] ),
                                    message_sent.size(),
                                    MPI_DOUBLE, 
                                    it->first,
                                    0, 
                                    MPI_COMM_WORLD ) )
      {
         cout << QString( "%1: MPI send failed in nsa_ga_process_queue() closing waiting worker\n" ).arg( myrank ) << flush;
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }
   }
   // cout << QString( "%1: finished closing workers\n" ).arg( myrank ) << flush;
   return true;
}

bool US_Saxs_Util::nsa_ga_process_queue()
{
   // talk to workers
   // send jobs to fill up queue
   // retrieve results

   MPI_Status mpi_status;

   // messages:
   // 1st double code
      // msg[ 0 ] encodes function
      // msg[ 0 ] == 0 // exit from master, ready from worker
      // msg[ 0 ] == 1 // process request from master / results here from worker

   // 2nd double fitness
   // 3rd through last: individual.v

   vector < double > message_received( nsa_var_ref.size() + 2 );
   vector < double > message_sent    ( nsa_var_ref.size() + 2 );

   nsa_ga_individual individual;
   individual.v.resize( nsa_var_ref.size() );

   // clear any waiting workers first 

   int errorno = -21001;

   map < int, bool > still_waiting;

   for ( map < int, bool >::iterator it = waiting_workers.begin();
         it != waiting_workers.end();
         it++ )
   {
      if ( queued_requests.size() )
      {
         message_sent[ 0 ] = 1.0;
         for ( unsigned int i = 0; i < nsa_var_ref.size(); i++ )
         {
            message_sent[ i + 2 ] = queued_requests.front().v[ i ];
         }
         queued_requests.pop_front();

         if ( MPI_SUCCESS != MPI_Send( &( message_sent[ 0 ] ),
                                       message_sent.size(),
                                       MPI_DOUBLE, 
                                       it->first,
                                       0, 
                                       MPI_COMM_WORLD ) )
         {
            cout << QString( "%1: MPI send failed in nsa_ga_process_queue() feeding waiting workers\n" ).arg( myrank ) << flush;
            MPI_Abort( MPI_COMM_WORLD, errorno );
            exit( errorno );
         }
         busy_workers[ it->first ] = true;
      } else {
         still_waiting[ it->first ] = true;
      }
   }

   waiting_workers = still_waiting;

   while ( busy_workers.size() || queued_requests.size() || registered_workers.size() != npes - 1 )
   {
      if ( MPI_SUCCESS != MPI_Recv( &( message_received[ 0 ] ),
                                    message_received.size(), 
                                    MPI_DOUBLE, 
                                    MPI_ANY_SOURCE, 
                                    MPI_ANY_TAG, 
                                    MPI_COMM_WORLD, 
                                    &mpi_status ) )
      {
         cout << QString( "%1: MPI Received failed in nsa_ga_process_queue()\n" ).arg( myrank ) << flush;
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }         

      registered_workers[ mpi_status.MPI_SOURCE ] = true;
      
      if ( message_received[ 0 ] == 1.0 )
      {
         // have results
         individual.fitness = message_received[ 1 ];
         // QString vmsg;
         for ( unsigned int i = 0; i < nsa_var_ref.size(); i++ )
         {
            individual.v[ i ] = message_received[ i + 2 ];
            // vmsg += QString( " %1" ).arg( individual.v[ i ] );
         }
         // cout << QString( "%1: master received result (%2): %3\n" )
         // .arg( myrank )
         // .arg( individual.fitness )
         // .arg( vmsg ) << flush;
         received_results.push_back( individual );
      }
                 
      if ( queued_requests.size() )
      {
         message_sent[ 0 ] = 1.0;
         for ( unsigned int i = 0; i < nsa_var_ref.size(); i++ )
         {
            message_sent[ i + 2 ] = queued_requests.front().v[ i ];
         }
         queued_requests.pop_front();

         if ( MPI_SUCCESS != MPI_Send( &( message_sent[ 0 ] ),
                                       message_sent.size(),
                                       MPI_DOUBLE, 
                                       mpi_status.MPI_SOURCE,
                                       0, 
                                       MPI_COMM_WORLD ) )
         {
            cout << QString( "%1: MPI send failed in nsa_ga_process_queue() in normal work assignment\n" ).arg( myrank ) << flush;
            MPI_Abort( MPI_COMM_WORLD, errorno  - 1 );
            exit( errorno - 1 );
         }
            
         busy_workers[ mpi_status.MPI_SOURCE ] = true;
      } else {
         busy_workers.erase( mpi_status.MPI_SOURCE );
         waiting_workers[ mpi_status.MPI_SOURCE ] = true;
      }
   }
   return true;
}

bool US_Saxs_Util::nsa_ga_master_test( double & nrmsd )
{
   cout << QString("%1: enter nsa ga initial barrier\n" ).arg( myrank ) << flush;
   if ( MPI_SUCCESS != MPI_Barrier( MPI_COMM_WORLD ) )
   {
      MPI_Abort( MPI_COMM_WORLD, -10000 - myrank );
      exit( -1 );
   }         

   nsa_ga_individual individual;
   individual.v.resize( nsa_var_ref.size() );

   queued_requests.clear();
   received_results.clear();

   cout << QString( "%1: queuing requests\n" ).arg( myrank ) << flush;

   for ( unsigned int i = 0; i < 3; i++ )
   {
      // queue up random requests
      for ( unsigned int j = 0; j < nsa_var_ref.size(); j++ )
      {
         individual.v[ j ] = nsa_var_min[ j ] + ( nsa_var_max[ j ] - nsa_var_min[ j ] ) * drand48();
      }

      queued_requests.push_back( individual );
   }

   cout << QString( "%1: queued %2 requests\n" ).arg( myrank ).arg( queued_requests.size() ) << flush;

   if ( !nsa_ga_process_queue() )
   {
      return false;
   }

   cout << QString( "%1: after processing %2 requests results size %3\n" )
      .arg( myrank )
      .arg( queued_requests.size() ) 
      .arg( received_results.size() ) 
        << flush;
   received_results.clear();

   for ( unsigned int i = 0; i < 4; i++ )
   {
      // queue up random requests
      for ( unsigned int j = 0; j < nsa_var_ref.size(); j++ )
      {
         individual.v[ j ] = nsa_var_min[ j ] + ( nsa_var_max[ j ] - nsa_var_min[ j ] ) * drand48();
      }

      queued_requests.push_back( individual );
   }

   cout << QString( "%1: queued %2 requests\n" ).arg( myrank ).arg( queued_requests.size() ) << flush;

   if ( !nsa_ga_process_queue() )
   {
      return false;
   }

   cout << QString( "%1: after processing %2 requests results size %3\n" )
      .arg( myrank )
      .arg( queued_requests.size() ) 
      .arg( received_results.size() ) 
        << flush;

   if ( !nsa_ga_close_workers() )
   {
      return false;
   }

   errormsg = "only testing\n";
   return false;
}

bool US_Saxs_Util::nsa_ga_master( double & nrmsd )
{
   // cout << QString("%1: enter nsa ga initial barrier\n" ).arg( myrank ) << flush;
   if ( MPI_SUCCESS != MPI_Barrier( MPI_COMM_WORLD ) )
   {
      MPI_Abort( MPI_COMM_WORLD, -10000 - myrank );
      exit( -1 );
   }         

   nsa_ga_individual individual;
   individual.v.resize( nsa_var_ref.size() );

   queued_requests.clear();
   received_results.clear();

   list < nsa_ga_individual > nsa_pop;

   // init population
   for ( unsigned int i = 0; i < control_parameters[ "nsapopulation" ].toUInt(); i++ )
   {
      for ( unsigned int j = 0; j < nsa_var_ref.size(); j++ )
      {
         individual.v[ j ] = nsa_var_min[ j ] + ( nsa_var_max[ j ] - nsa_var_min[ j ] ) * drand48();
      }
      queued_requests.push_back( individual );
   }

   // sort by fitness
   double       last_best_fitness;
   unsigned int gens_with_no_improvement = 0;

   for ( unsigned int g = 0; g < control_parameters[ "nsagenerations" ].toUInt(); g++ )
   {
      cout << QString( "%1: queued %2 requests\n" ).arg( myrank ).arg( queued_requests.size() ) << flush;
      if ( !nsa_ga_process_queue() )
      {
         return false;
      }
      cout << QString( "%1: after processing %2 requests results size %3\n" )
         .arg( myrank )
         .arg( queued_requests.size() ) 
         .arg( received_results.size() ) 
           << flush;

      nsa_pop = received_results;
      received_results.clear();

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
               cout << QString( "%1: early termination\n" ).arg( myrank ) << flush;
               break;
            }
         }
      }
            
      cout << QString( "nsa: gen %1 best individual fitness %1\n" )
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

      nsa_pop.clear();

      for ( unsigned int i = 0; i < control_parameters[ "nsapopulation" ].toUInt(); i++ )
      {
         if ( control_parameters.count( "nsaelitism" ) &&
              i < control_parameters[ "nsaelitism" ].toUInt() )
         {
            // cout << "elitism\n";
            queued_requests.push_back( last_pop[ i ] );
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
            queued_requests.push_back( individual );
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
            queued_requests.push_back( individual );
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
            queued_requests.push_back( individual );
            random_count++;
         } else {
            has_been_duplicated[ pos ] = true;
            individual = last_pop[ nsa_pop_selection( last_pop.size() ) ];
            queued_requests.push_back( individual );
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

   if ( queued_requests.size() )
   {
      cout << QString( "%1: queued %2 requests\n" ).arg( myrank ).arg( queued_requests.size() ) << flush;
      if ( !nsa_ga_process_queue() )
      {
         return false;
      }
      cout << QString( "%1: after processing %2 requests results size %3\n" )
         .arg( myrank )
         .arg( queued_requests.size() ) 
         .arg( received_results.size() ) 
           << flush;
      
      nsa_pop = received_results;
      received_results.clear();
   }
   
   nsa_pop.sort();
   nsa_pop.unique();

   if ( !nsa_ga_close_workers() )
   {
      return false;
   }

   for ( unsigned int i = 0; i < nsa_var_ref.size(); i++ )
   {
      *( nsa_var_ref[ i ] ) = nsa_pop.front().v[ i ];
   }

   nrmsd = nsa_fitness();

   return true;
}
