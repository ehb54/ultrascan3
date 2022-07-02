#include <mpi.h>

#include "../include/us_saxs_util.h"
//Added by qt3to4:
#include <QTextStream>

extern int npes;
extern int myrank;

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

// #define USUNGM_DEBUG

#include <qdatetime.h>

extern bool       use_warning_time;
extern QDateTime  start_time;
extern QDateTime  warning_time;
extern bool       timed_out;

bool US_Saxs_Util::nsa_run()
{
#if defined( USUNGM_DEBUG )
   debug_mpi( QString( "%1: run_mpi at beginning\n" 
                       " : busy_workers       %2\n"
                       " : queued_requests    %3\n"
                       " : registered_workers %4\n" )
              .arg( myrank )
              .arg( busy_workers.size() )
              .arg( queued_requests.size() )
              .arg( registered_workers.size() )
              );
#endif
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

   unsigned int startloop = 1;
   unsigned int endloop   = control_parameters[ "nsaspheres" ].toUInt();
   {
      QRegExp rx( "^(\\d+)\\s+(\\d+)$" );
      QString qs = control_parameters[ "nsaspheres" ];
      qs.replace( QRegExp( "(:|,|-)" ), " " );
      if ( rx.indexIn( qs ) != -1 )
      {
         startloop = rx.cap( 1 ).toUInt();
         endloop   = rx.cap( 2 ).toUInt();
      }
   }
   if ( !myrank )
   {
      cout << QString( "nsa run: %1 to %2\n" ).arg( startloop ).arg( endloop );
   }

   for ( unsigned int i = startloop; i <= endloop; i++ )
   {
      US_Timer ust;
      ust.init_timer ( "nsa ga time" );
      ust.start_timer( "nsa ga time" );

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
            if ( use_warning_time )
            {
               if ( !myrank )
               {
                  if ( QDateTime::currentDateTime() >= warning_time )
                  {
                     // make timeout message
                     cout << "timeout main loop\n";
                     timed_out = true;
                  }
               }
               if ( MPI_SUCCESS != MPI_Bcast( (void *)&timed_out, 1, MPI_INT, 0, MPI_COMM_WORLD ) )
               {
                  MPI_Abort( MPI_COMM_WORLD, -177 );
                  exit( -177 );
               }         
               if ( timed_out )
               {
                  cout << QString( "%1: message timed out is true\n" ).arg( myrank );
                  return true;
               }
            }
            
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
            ( control_parameters.count( "experimentgrid" ) ?
              ( QFileInfo( control_parameters[ "experimentgrid" ] ).baseName() + "_" ) : "" )
            +
            QString( "%1sa-%2%3%4%5" )
            .arg( i )
            .arg( control_parameters.count( "nsaga" ) ?
                  "" : (control_parameters[ "nsagsm" ] + "-" ) )
            .arg( control_parameters.count( "nsaess" ) ?
                  "ess-" : "" )
            .arg( control_parameters.count( "nsaexcl" ) ?
                  "excl-" : "" )
            .arg( control_parameters[ "nsaiterations" ] );

         QString original_outname = outname;
         
         QString outfilename = QString( "%1.bead_model" ).arg( outname );
         unsigned int ext = 0;
         while ( QFile::exists( outfilename ) )
         {
            outname = QString( "%1-%2" ).arg( original_outname ).arg( ++ext );
            outfilename = QString( "%1.bead_model" ).arg( outname );
         }

         QFile f( outfilename );
            
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
               ust.end_timer( "nsa ga time" );
               out_params[ "stats nsa ga time" ] = ust.list_time( "nsa ga time" ).replace( "\n", "" );
               out_params[ "stats np" ] = QString( "%1" ).arg( npes );
               out_params[ "stats host" ] = QString( "%1" ).arg( getenv( "HOSTNAME" ) );
               ts << Qt::endl << "__json:" << US_Json::compose( out_params ) << Qt::endl;
            }

            f.close();
            cout << QString( "written: %1\n" ).arg( f.fileName() );
            output_files << f.fileName();
         }
         control_parameters[ "outputfile" ] = outname;
         nsa_fitness();
      }
      control_parameters[ "outputfile" ] = save_outputfile;
   }

#if defined( USUNGM_DEBUG )
   debug_mpi( QString( "%1: run_mpi at end\n" 
                       " : busy_workers       %2\n"
                       " : queued_requests    %3\n"
                       " : registered_workers %4\n" )
              .arg( myrank )
              .arg( busy_workers.size() )
              .arg( queued_requests.size() )
              .arg( registered_workers.size() )
              );
#endif
   return true;
}

bool US_Saxs_Util::nsa_ga_worker()
{
#if defined( USUNGM_DEBUG )
   debug_mpi( QString( "%1: nsa_ga_worker starting\n" ).arg( myrank ) );
#endif
   int errorno = -23000;

#if defined( USUNGM_DEBUG )
   debug_mpi( QString("%1: start nsa_worker: BARRIER enter nsa ga initial barrier\n" ).arg( myrank ) );
#endif

   if ( MPI_SUCCESS != MPI_Barrier( MPI_COMM_WORLD ) )
   {
      MPI_Abort( MPI_COMM_WORLD, errorno - myrank );
      exit( errorno - myrank );
   }

#if defined( USUNGM_DEBUG )
   debug_mpi( QString("%1: exit nsa ga initial barrier\n" ).arg( myrank ) );
#endif
   
   MPI_Status mpi_status;

   vector < double > message_received( nsa_var_ref.size() + 2 );
   vector < double > message_sent    ( nsa_var_ref.size() + 2 );

   nsa_ga_individual individual;
   individual.v.resize( nsa_var_ref.size() );

   message_sent[ 0 ] = 0.0;

   errorno = -24000;

#if defined( USUNGM_DEBUG )
   debug_mpi( QString( "%1: SEND MPI send in nsa_ga_worker() initial registry\n" ).arg( myrank ) );
   debug_mpi( QString( "%1: message_received.size %2 messsage_send.size %3\n" )
              .arg( myrank ).arg( message_received.size() ).arg( message_sent.size() ) );
#endif

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
#if defined( USUNGM_DEBUG )
      debug_mpi( QString( "%1: RECV MPI  in nsa_ga_worker() receive msg\n" ).arg( myrank ) );
#endif
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
#if defined( USUNGM_DEBUG )
      debug_mpi( QString( "%1: MPI  in nsa_ga_worker() msg received\n" ).arg( myrank ) );
#endif
      
      if ( message_received[ 0 ] == 1.0 )
      {
#if defined( USUNGM_DEBUG )
         debug_mpi( QString( "%1: MPI  in nsa_ga_worker() msg received is 1.0\n" ).arg( myrank ) );
#endif
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

#if defined( USUNGM_DEBUG )
         debug_mpi( QString( "%1: MPI  in nsa_ga_worker() send results\n" ).arg( myrank ) );
#endif
         message_sent[ 0 ] = 1.0;
         message_sent[ 1 ] = individual.fitness;
#if defined( USUNGM_DEBUG )
         QString vmsg;
#endif
         for ( unsigned int i = 0; i < nsa_var_ref.size(); i++ )
         {
            message_sent[ i + 2 ] = individual.v[ i ];
#if defined( USUNGM_DEBUG )
            vmsg += QString( " %1" ).arg( individual.v[ i ] );
#endif
         }

#if defined( USUNGM_DEBUG )
         debug_mpi( QString( "%1: SEND worker sending result (%2): %3\n" )
          .arg( myrank )
          .arg( individual.fitness )
                    .arg( vmsg ) );
#endif

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

#if defined( USUNGM_DEBUG )
         debug_mpi( QString( "%1: worker sent result (%2): %3\n" )
          .arg( myrank )
          .arg( individual.fitness )
                    .arg( vmsg ) );
#endif
      }
   } while ( message_received[ 0 ] != 0.0 );

#if defined( USUNGM_DEBUG )
   debug_mpi( QString( "%1: nsa_ga_worker closing\n" ).arg( myrank ) );
#endif
   return true;
}

bool US_Saxs_Util::nsa_ga_close_workers()
{
#if defined( USUNGM_DEBUG )
   debug_mpi( QString( "%1: nsa ga close workers\n" ).arg( myrank ) );
#endif
   int errorno = -22000;
   vector < double > message_sent    ( nsa_var_ref.size() + 2 );
   message_sent[ 0 ] = 0.0;

   for ( map < int, bool >::iterator it = registered_workers.begin();
         it != registered_workers.end();
         it++ )
   {
#if defined( USUNGM_DEBUG )
      debug_mpi( QString( "%1: SEND sending close request to worker %2\n" ).arg( myrank ).arg( it->first ) );
#endif

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
#if defined( USUNGM_DEBUG )
   debug_mpi( QString( "%1: finished closing workers\n" ).arg( myrank ) );
#endif
   waiting_workers.clear( );
#if defined( USUNGM_DEBUG )
   debug_mpi( QString( "%1: after finished closing workers\n" 
                       " : busy_workers       %2\n"
                       " : queued_requests    %3\n"
                       " : waiting_workers    %4\n"
                       " : registered_workers %5\n" )
              .arg( myrank )
              .arg( busy_workers.size() )
              .arg( queued_requests.size() )
              .arg( waiting_workers.size() )
              .arg( registered_workers.size() )
              );
#endif
   return true;
}

bool US_Saxs_Util::nsa_ga_process_queue()
{
   // talk to workers
   // send jobs to fill up queue
   // retrieve results
#if defined( USUNGM_DEBUG )
   debug_mpi( QString( "%1: nsa ga process queue\n" ).arg( myrank ) );
#endif
#if defined( USUNGM_DEBUG )
   debug_mpi( QString( "%1: beginning of process queue\n" 
                       " : busy_workers       %2\n"
                       " : queued_requests    %3\n"
                       " : waiting workers    %4\n"
                       " : registered_workers %5\n" )
              .arg( myrank )
              .arg( busy_workers.size() )
              .arg( queued_requests.size() )
              .arg( waiting_workers.size() )
              .arg( registered_workers.size() )
              );
#endif

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

#if defined( USUNGM_DEBUG )
   debug_mpi( QString( "%1: nsa ga begin process queue %2\n" ).arg( myrank ).arg( waiting_workers.size() ) );
#endif
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

#if defined( USUNGM_DEBUG )
         debug_mpi( QString( "%1: SEND nsa ga begin process queue sending work assignment to worker %1 \n" ).arg( myrank ).arg( it->first ) );
#endif
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
#if defined( USUNGM_DEBUG )
         debug_mpi( QString( "%1: nsa ga begin process queue  worker %2 busy worker \n" ).arg( myrank ).arg( it->first ) );
#endif
         busy_workers[ it->first ] = true;
      } else {
#if defined( USUNGM_DEBUG )
         cout << QString( "%1: nsa ga begin process queue  worker %2 still_waiting \n" ).arg( myrank ).arg( it->first ) << flush;
#endif
         still_waiting[ it->first ] = true;
      }
   }

   waiting_workers = still_waiting;

#if defined( USUNGM_DEBUG )
   debug_mpi( QString( "%1: nsa_ga_process_queue begin process queue: all requests queued, now wait \n"
                       " : busy_workers       %2\n"
                       " : queued_requests    %3\n"
                       " : registered_workers %4\n" )
              .arg( myrank )
              .arg( busy_workers.size() )
              .arg( queued_requests.size() )
              .arg( registered_workers.size() )
              );
#endif

   while ( busy_workers.size() || queued_requests.size() || registered_workers.size() != npes - 1 )
   {
#if defined( USUNGM_DEBUG )
      debug_mpi( QString( "%1: RECV nsa_ga_process_queue: waiting for message \n"
                          " : busy_workers       %2\n"
                          " : queued_requests    %3\n"
                          " : registered_workers %4\n" )
                 .arg( myrank )
                 .arg( busy_workers.size() )
                 .arg( queued_requests.size() )
                 .arg( registered_workers.size() )
                 );
#endif
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

#if defined( USUNGM_DEBUG )
      debug_mpi( QString( "%1: nsa_ga_process_queue: received message \n"
                          " : busy_workers       %2\n"
                          " : queued_requests    %3\n"
                          " : registered_workers %4\n" )
                 .arg( myrank )
                 .arg( busy_workers.size() )
                 .arg( queued_requests.size() )
                 .arg( registered_workers.size() )
                 );
#endif

      registered_workers[ mpi_status.MPI_SOURCE ] = true;
      
#if defined( USUNGM_DEBUG )
      debug_mpi( QString(
                         "%1: nsa_ga_process_queue: received message %2 from %3\n"
                         )
                 .arg( myrank )
                 .arg( message_received[ 0 ] )
                 .arg( mpi_status.MPI_SOURCE )
                 );
#endif

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
#if defined( USUNGM_DEBUG )
         debug_mpi( QString(
                            "%1: nsa_ga_process_queue: more queued requests \n"
                            )
                    .arg( myrank )
                    );
#endif
         message_sent[ 0 ] = 1.0;
         for ( unsigned int i = 0; i < nsa_var_ref.size(); i++ )
         {
            message_sent[ i + 2 ] = queued_requests.front().v[ i ];
         }
         queued_requests.pop_front();

#if defined( USUNGM_DEBUG )
         debug_mpi(  QString( "%1: SEND MPI send in nsa_ga_process_queue() normal work assignment %2\n" )
                     .arg( myrank ).arg( mpi_status.MPI_SOURCE ) );
#endif
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
         if ( message_received[ 0 ] == 1.0 )
         {
            busy_workers.erase( mpi_status.MPI_SOURCE );
            waiting_workers[ mpi_status.MPI_SOURCE ] = true;
         }
      }
   }
#if defined( USUNGM_DEBUG )
   debug_mpi( QString( "%1: nsa ga done with work requests\n" )
              .arg( myrank )
              );
#endif

   return true;
}

bool US_Saxs_Util::nsa_ga_master_test( double & nrmsd )
{
#if defined( USUNGM_DEBUG )
   debug_mpi( QString("%1: BARRIER enter nsa ga initial barrier\n" ).arg( myrank ) );
#endif
   if ( MPI_SUCCESS != MPI_Barrier( MPI_COMM_WORLD ) )
   {
      MPI_Abort( MPI_COMM_WORLD, -10000 - myrank );
      exit( -1 );
   }         

   nsa_ga_individual individual;
   individual.v.resize( nsa_var_ref.size() );

   queued_requests.clear( );
   received_results.clear( );

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
   received_results.clear( );

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
#if defined( USUNGM_DEBUG )
   debug_mpi( QString("%1: nsa_ga_master() start: BARRIER enter nsa ga initial barrier\n" ).arg( myrank ) );
#endif
   if ( MPI_SUCCESS != MPI_Barrier( MPI_COMM_WORLD ) )
   {
      MPI_Abort( MPI_COMM_WORLD, -10000 - myrank );
      exit( -1 );
   }         

   nsa_ga_individual individual;
   individual.v.resize( nsa_var_ref.size() );

   queued_requests.clear( );
   received_results.clear( );

   list < nsa_ga_individual > nsa_pop;

   // init population
   for ( unsigned int i = 0; i < control_parameters[ "nsapopulation" ].toUInt(); i++ )
   {
      if ( nsa_ga_inits.count( (unsigned int)nsa_var_ref.size() ) &&
           nsa_ga_inits[ (unsigned int)nsa_var_ref.size() ].size() > i )
      {
         individual = nsa_ga_inits[ (unsigned int)nsa_var_ref.size() ][ i ];
         bool cropped = false;
         for ( unsigned int j = 0; j < nsa_var_ref.size(); j++ )
         {
            if ( individual.v[ j ]  < nsa_var_min[ j ] )
            {
               cropped = true;
               individual.v[ j ] = nsa_var_min[ j ];
            }
            if ( individual.v[ j ]  < nsa_var_max[ j ] )
            {
               cropped = true;
               individual.v[ j ] = nsa_var_max[ j ];
            }
         }
         if ( cropped )
         {
            cout << QString( "Warning: nsa ga init size %1 entry %2 cropped\n" ).arg( nsa_var_ref.size() ).arg( i + 1 );
         }
      } else {
         for ( unsigned int j = 0; j < nsa_var_ref.size(); j++ )
         {
            individual.v[ j ] = nsa_var_min[ j ] + ( nsa_var_max[ j ] - nsa_var_min[ j ] ) * drand48();
         }
      }
      queued_requests.push_back( individual );
   }

   // sort by fitness
   double       last_best_fitness;
   unsigned int gens_with_no_improvement = 0;

   for ( unsigned int g = 0; g < control_parameters[ "nsagenerations" ].toUInt(); g++ )
   {
      if ( use_warning_time &&
           QDateTime::currentDateTime() >= warning_time )
      {
         // make timeout message
         cout << "timeout\n";
         timed_out = true;
         break;
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

      nsa_pop = received_results;
      received_results.clear( );

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
            
      cout << QString( "nsa: gen %1 best individual fitness %2\n" )
         .arg( g )
         .arg( nsa_pop.size() ? nsa_pop.front().fitness : 9e99 );
#if defined( USUNGM_DEBUG )
      debug_mpi( QString( "%1: nsa: gen %2 best individual fitness %3\n" )
                 .arg( myrank )
                 .arg( g )
                 .arg( nsa_pop.size() ? nsa_pop.front().fitness : 9e99 ) );
#endif

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

      cout << QString( "start: nsa_pop.size() %1\n" ).arg( last_pop.size() ) << flush;
#if defined( USUNGM_DEBUG )
      debug_mpi( QString( "%1: start: nsa_pop.size() %2\n" ).arg( myrank ).arg( last_pop.size() ) );
#endif
      nsa_pop.clear( );

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
#if defined( USUNGM_DEBUG )
      debug_mpi( QString( 
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
                 .arg( elitism_count + mutate_count + crossover_count + duplicate_count + random_count ) );
#endif
   }

   if ( queued_requests.size() )
   {
      cout << QString( "%1: queued %2 requests\n" ).arg( myrank ).arg( queued_requests.size() ) << flush;
#if defined( USUNGM_DEBUG )
      debug_mpi( QString( "%1: queued %2 requests\n" ).arg( myrank ).arg( queued_requests.size() ) );
#endif
      if ( !nsa_ga_process_queue() )
      {
         return false;
      }
      cout << QString( "%1: after processing %2 requests results size %3\n" )
         .arg( myrank )
         .arg( queued_requests.size() ) 
         .arg( received_results.size() ) 
           << flush;
#if defined( USUNGM_DEBUG )
      debug_mpi( QString( "%1: after processing %2 requests results size %3\n" )
                 .arg( myrank )
                 .arg( queued_requests.size() ) 
                 .arg( received_results.size() ) );
#endif
      
      nsa_pop = received_results;
      received_results.clear( );
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
