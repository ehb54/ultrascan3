#include "../include/us_saxs_util.h"
#include "../include/us_pm.h"

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}
#define USE_AFFINITY
#if defined( USE_AFFINITY )
#include <sched.h>
#endif

ostream & operator << ( ostream& out, const pm_msg& c )
{
   QString type_msg = QString( "Unknown: %1" ).arg( c.type );
   switch( c.type )
   {
   case PM_SHUTDOWN:
      type_msg = "PM_SHUTDOWN";
      break;
   case PM_MSG:
      type_msg = "PM_MSG";
      break;
   case PM_NEW_PM:
      type_msg = "PM_NEW_PM";
      break;
   case PM_NEW_GRID_SIZE:
      type_msg = "PM_NEW_GRID_SIZE";
      break;
   case PM_CALC_FITNESS:
      type_msg = "PM_CALC_FITNESS";
      break;
   case PM_REGISTER:
      type_msg = "PM_REGISTER";
      break;
   case PM_FITNESS_RESULT:
      type_msg = "PM_FITNESS_RESULT";
      break;
   case PM_FITNESS_RESULT_MODEL:
      type_msg = "PM_FITNESS_RESULT_MODEL";
      break;
   default:
      break;
   }

   out << QString( 
                  "pm_msg:\n"
                  "type         : %1\n"
                  "flags        : %2\n"
                  "vsize        : %3\n"
                  "gcf or fit   : %4\n"
                  "max_dimension: %5\n"
                  "max_harmonics: %6\n"
                  "max_mem_in_MB: %7\n" 
                  )
      .arg( type_msg )
      .arg( c.flags )
      .arg( c.vsize )
      .arg( c.grid_conversion_factor )
      .arg( c.max_dimension )
      .arg( c.max_harmonics )
      .arg( c.max_mem_in_MB )
      ;
   return out;
}                   

bool US_Saxs_Util::run_json_mpi( QString & json )
{
   // note: alternate version for non-mpi in in run_json() in us_saxs_util_pm.cpp

#if defined( USE_AFFINITY )
   {
      cpu_set_t  mask;
      CPU_ZERO( & mask);
      CPU_SET( myrank, &mask);
      if ( sched_setaffinity(0, sizeof( mask ), &mask) < 0 ) 
      {
         perror("sched_setaffinity");
      }
   }
#endif
   
   if ( !us_log )
   {
      us_log = new US_Log( QString( "runlog-%1.txt" ).arg( myrank ) );
      us_log->log( "initial json" );
      us_log->log( json );
   }

   us_log->datetime( "start run_json" );

   map < QString, QString > parameters = US_Json::split( json );
   map < QString, QString > results;

   for ( map < QString, QString >::iterator it = parameters.begin();
         it != parameters.end();
         ++it )
   {
      // us_qdebug( QString( "%1 : %2" ).arg( it->first ).arg( it->second ) );
      if ( it->first.left( 1 ) == "_" )
      {
         results[ it->first ] = it->second;
      }
   }

   if ( !myrank &&
        parameters.count( "_udphostip" ) &&
        parameters.count( "_udpport" ) &&
        parameters.count( "_uuid" ) )
   {
      map < QString, QString > msging;
      msging[ "_uuid" ] = results[ "_uuid" ];
      us_udp_msg = new US_Udp_Msg( parameters[ "_udphostip" ], (quint16) parameters[ "_udpport" ].toUInt() );
      us_udp_msg->set_default_json( msging );
      cout << "udp active\n";
   } else {
      cout << "udp NOT active\n";
   }

   {
      QStringList supported;
      supported
         << "pmrun"
         << "iq";
      
      int count = 0;
      for ( int i = 0; i < (int) supported.size(); ++i )
      {
         if ( parameters.count( supported[ i ] ) )
         {
            count++;
         }
      }

      if ( !count )
      {
         if ( !myrank )
         {
            results[ "errors" ] = "no supported runtype found in input json";
            cout << MPI_JSON_SNIP_START << US_Json::compose( results ).toLatin1().data() << endl << MPI_JSON_SNIP_END << flush;
         }
         return true;
      }
      if ( count > 1 )
      {
         if ( !myrank )
         {
            results[ "errors" ] = "only one run type currently allowed per input json";
            cout << MPI_JSON_SNIP_START << US_Json::compose( results ).toLatin1().data() << endl << MPI_JSON_SNIP_END << flush;
         }
         return true;
      }
   }

   if ( parameters.count( "iq" ) ) {
      if ( !parameters.count( "file" ) ) {
         if ( !myrank )
         {
            results[ "errors" ] = "json runtype iq requires file be defined";
            cout << MPI_JSON_SNIP_START << US_Json::compose( results ).toLatin1().data() << endl << MPI_JSON_SNIP_END << flush;
         }
         return true;
      }         
      QString controlfile     = parameters[ "file" ];
      if ( !run_iq_mpi( controlfile ) )
      {
         if ( !myrank )
         {
            cout << errormsg << endl;
         }
         if ( !myrank ) {
            MPI_Abort( MPI_COMM_WORLD, 0 );
         }
         MPI_Finalize();
         return false;
      }
      if ( !myrank ) {
         // if ( us_udp_msg ) {
         //    us_udp_msg->send_json( { { "_progressmsg", "Done" } } );
         // }
         MPI_Abort( MPI_COMM_WORLD, 0 );
      }
      MPI_Finalize();
      return true;
   }

   if ( myrank )
   {
      if ( us_log )
      {
         us_log->datetime( "json pm worker starting" );
      }
      pm_mpi_worker();
      if ( us_log )
      {
         us_log->datetime( "json pm worker finished" );
      }
      return true;
   }

   if ( us_udp_msg )
   {
      map < QString, QString > msging;
      msging[ "status" ] = "started";
      us_udp_msg->send_json( msging );
   }

   if ( !run_pm( parameters, results ) )
   {
      results[ "errors" ] += " run_pm failed:" + errormsg;
   }
   if ( us_log )
   {
      us_log->log( "final json" );
      us_log->log( US_Json::compose( results ) );
      us_log->datetime( "end run_json" );
   }

   // shutdown

   {
      pm_msg msg;
      msg.type = PM_SHUTDOWN;
      int errorno = -18000;

      for ( int i = 1; i < npes; ++i )
      {
         if ( MPI_SUCCESS != MPI_Send( &msg,
                                       sizeof( pm_msg ),
                                       MPI_CHAR, 
                                       i,
                                       PM_MSG, 
                                       MPI_COMM_WORLD ) )
         {
            us_log->log( QString( "%1: MPI PM_SHUTDOWN failed\n" ).arg( myrank ) );
            delete us_log;
            MPI_Abort( MPI_COMM_WORLD, errorno - myrank );
            exit( errorno - myrank );
         }
      }
   }

   cout << MPI_JSON_SNIP_START << US_Json::compose( results ).toLatin1().data() << endl << MPI_JSON_SNIP_END << flush;
   return true;
}

bool US_Saxs_Util::run_pm_mpi( QString controlfile )
{

#if defined( USE_AFFINITY )
   {
      cpu_set_t  mask;
      CPU_ZERO( & mask);
      CPU_SET( myrank, &mask);
      if ( sched_setaffinity(0, sizeof( mask ), &mask) < 0 ) 
      {
         perror("sched_setaffinity");
      }
   }
#endif

   if ( myrank )
   {
      pm_mpi_worker();
      return true;
   }

   if ( !run_pm( controlfile ) )
   {
      cout << errormsg << endl << flush;
      MPI_Abort( MPI_COMM_WORLD, -7000 );
      exit( -7000 );
   }

   // shutdown

   {
      pm_msg msg;
      msg.type = PM_SHUTDOWN;
      int errorno = -18000;

      for ( int i = 1; i < npes; ++i )
      {
         if ( MPI_SUCCESS != MPI_Send( &msg,
                                       sizeof( pm_msg ),
                                       MPI_CHAR, 
                                       i,
                                       PM_MSG, 
                                       MPI_COMM_WORLD ) )
         {
            cout << QString( "%1: MPI PM_SHUTDOWN failed\n" ).arg( myrank ) << flush;
            MPI_Abort( MPI_COMM_WORLD, errorno - myrank );
            exit( errorno - myrank );
         }
      }
   }

   MPI_Finalize();

   exit( 0 );
   return true;
}

void US_Saxs_Util::pm_mpi_worker()
{
   MPI_Status mpi_status;

   US_PM *pm = (US_PM*) 0;
   
   pm_msg  msg;

   msg.type = PM_REGISTER;

   vector < double > F;
   vector < double > q;
   vector < double > I;
   vector < double > e;

   vector < double > I_result;
   vector < double > params;
   set < pm_point >  model;

   int errorno = -10000;

   /* no registry for now, assume all workers available
     if ( MPI_SUCCESS != MPI_Send( &msg,
     sizeof( pm_msg ),
     MPI_CHAR, 
     0,
     0, 
     MPI_COMM_WORLD ) )
     {
     if ( us_log )
     {
     us_log->log( QString( "%1: MPI send failed in nsa_ga_worker() initial registry\n" ).arg( myrank ) );
     }
     MPI_Abort( MPI_COMM_WORLD, errorno - myrank );
     exit( errorno - myrank );
     }
   */

   do 
   {
      // if ( us_log )
      // {
      //    us_log->log( QString( "%1: worker listening\n" ).arg( myrank ) );
      // }
      if ( MPI_SUCCESS != MPI_Recv( &msg,
                                    sizeof( pm_msg ),
                                    MPI_CHAR, 
                                    0, 
                                    PM_MSG, 
                                    MPI_COMM_WORLD, 
                                    &mpi_status ) )
      {
         if ( us_log )
         {
            us_log->log( QString( "%1: MPI Initial Receive failed in pm_mpi_worker()\n" ).arg( myrank ) );
         }
         MPI_Abort( MPI_COMM_WORLD, errorno - myrank );
         exit( errorno - myrank );
      }         
      
      // cout << msg;

      switch( msg.type )
      {
      case PM_SHUTDOWN :
         {
            if ( us_log )
            {
               us_log->log( QString( "%1: worker PM_SHUTDOWN\n" ).arg( myrank ) );
            }
            MPI_Finalize();
            exit(0);
         }
         break;

      case PM_NEW_PM :
         {
            // if ( us_log )
            // {
            //    us_log->log( QString( "%1: worker PM_NEW_PM\n" ).arg( myrank ) );
            // }
            if ( pm )
            {
               delete pm;
               pm = (US_PM*) 0;
            }
            bool use_errors = msg.flags & PM_USE_ERRORS;
            unsigned int tot_vsize = msg.vsize * ( use_errors ? 4 : 3 );
            vector < double > d( tot_vsize );
            F.resize( msg.vsize );
            q.resize( msg.vsize );
            I.resize( msg.vsize );
            e.resize( use_errors ? msg.vsize : 0 );

            if ( MPI_SUCCESS != MPI_Recv( &(d[ 0 ]),
                                          tot_vsize * sizeof( double ),
                                          MPI_CHAR, 
                                          0, 
                                          PM_NEW_PM, 
                                          MPI_COMM_WORLD, 
                                          &mpi_status ) )
            {
               if ( us_log )
               {
                  us_log->log( QString( "%1: MPI PM_NEW_PM Receive failed in pm_mpi_worker()\n" ).arg( myrank ) );
               }
               MPI_Abort( MPI_COMM_WORLD, errorno - myrank );
               exit( errorno - myrank );
            }         

            if ( use_errors )
            {
               for ( int i = 0; i < (int) msg.vsize; i++ )
               {
                  F[ i ] = d[ i ];
                  q[ i ] = d[ msg.vsize + i ];
                  I[ i ] = d[ 2 * msg.vsize + i ];
                  e[ i ] = d[ 3 * msg.vsize + i ];
               }
            } else {
               for ( int i = 0; i < (int) msg.vsize; i++ )
               {
                  F[ i ] = d[ i ];
                  q[ i ] = d[ msg.vsize + i ];
                  I[ i ] = d[ 2 * msg.vsize + i ];
               }
            }

            pm = new US_PM( msg.grid_conversion_factor,
                            msg.max_dimension,
                            msg.max_harmonics,
                            F,
                            q,
                            I,
                            e,
                            msg.max_mem_in_MB,
                            0,
                            true );
         }
         break;

      case PM_NEW_GRID_SIZE:
         {
            // if ( us_log )
            // {
            //    us_log->log( // QString( "%1: worker PM_NEW_GRID_SIZE\n" ).arg( myrank ) );
            // }
            if ( !pm )
            {
               if ( us_log )
               {
                  us_log->log( QString( "%1: MPI PM_NEW_GRID_SIZE Receive called before PM_NEW_PM  pm_mpi_worker()\n" ).arg( myrank ) );
               }
               MPI_Abort( MPI_COMM_WORLD, errorno - myrank );
               exit( errorno - myrank );
            }
            if ( !pm->set_grid_size( msg.grid_conversion_factor, true ) )
            {
               if ( us_log )
               {
                  us_log->log( QString( "%1: MPI PM_NEW_GRID_SIZE failed to set grid size %2  pm_mpi_worker()\n" ).arg( myrank ).arg( msg.grid_conversion_factor ) );
               }
               MPI_Abort( MPI_COMM_WORLD, errorno - myrank );
               exit( errorno - myrank );
            }
         }
         break;

      case PM_CALC_FITNESS:
         {
            // if ( us_log )
            // {
            //    us_log->log( QString( "%1: worker PM_CALC_FITNESS\n" ).arg( myrank ) );
            // }
            if ( !pm )
            {
               if ( us_log )
               {
                  us_log->log( QString( "%1: MPI PM_CALC_FITNESS Receive called before PM_NEW_PM  pm_mpi_worker()\n" ).arg( myrank ) );
               }
               MPI_Abort( MPI_COMM_WORLD, errorno - myrank );
               exit( errorno - myrank );
            }

            params.resize( msg.vsize );
            if ( MPI_SUCCESS != MPI_Recv( &(params[ 0 ]),
                                          msg.vsize * sizeof( double ),
                                          MPI_CHAR, 
                                          0, 
                                          PM_CALC_FITNESS,
                                          MPI_COMM_WORLD, 
                                          &mpi_status ) )
            {
               if ( us_log )
               {
                  us_log->log( QString( "%1: MPI PM_CALC_FITNESS Receive failed in pm_mpi_worker()\n" ).arg( myrank ) );
               }
               MPI_Abort( MPI_COMM_WORLD, errorno - myrank );
               exit( errorno - myrank );
            }         


            // US_Vector::printvector( QString( "%1: PM_CALC_FITNESS params:" ).arg( myrank ), params );
            // cout << flush;

            if ( !pm->create_model( params, model ) ||
                 !pm->compute_I( model, I_result ) )
            {
               if ( us_log )
               {
                  us_log->log( QString( "%1: MPI PM_CALC_FITNESS error %2 in pm_mpi_worker()\n" ).arg( myrank ).arg( pm->error_msg ) );
               }
               MPI_Abort( MPI_COMM_WORLD, errorno - myrank );
               exit( errorno - myrank );
            }

            
            msg.type  = PM_FITNESS_RESULT;
            msg.vsize = model.size();
            msg.model_fitness = pm->fitness2( I_result );

            // cout << QString( "%1: created model with %2 beads fit %3\n" )
            //                .arg( myrank )
            //                .arg( model.size() )
            //                .arg( msg.model_fitness ) << flush;

            vector < int16_t > vmodel;
            for ( set < pm_point >::iterator it = model.begin();
                  it != model.end();
                  it++ )
            {
               vmodel.push_back( it->x[ 0 ] );
               vmodel.push_back( it->x[ 1 ] );
               vmodel.push_back( it->x[ 2 ] );
            }

            if ( MPI_SUCCESS != MPI_Send( &msg,
                                          sizeof( msg ),
                                          MPI_CHAR, 
                                          0, 
                                          PM_FITNESS_RESULT,
                                          MPI_COMM_WORLD ) )
            {
               if ( us_log )
               {
                  us_log->log( QString( "%1: MPI PM_CALC_FITNESS Send 1 failed in pm_mpi_worker()\n" ).arg( myrank ) );
               }
               MPI_Abort( MPI_COMM_WORLD, errorno - myrank );
               exit( errorno - myrank );
            }         
            
            if ( MPI_SUCCESS != MPI_Send( &(vmodel[0]),
                                          3 * msg.vsize * sizeof( int16_t ),
                                          MPI_CHAR, 
                                          0, 
                                          PM_FITNESS_RESULT_MODEL,
                                          MPI_COMM_WORLD ) )
            {
               if ( us_log )
               {
                  us_log->log( QString( "%1: MPI PM_CALC_FITNESS Send 2 failed in pm_mpi_worker()\n" ).arg( myrank ) );
               }
               MPI_Abort( MPI_COMM_WORLD, errorno - myrank );
               exit( errorno - myrank );
            }         
         }
         break;

      default:
         {
            if ( us_log )
            {
               us_log->log( QString( "%1: MPI Receive unknown message type %2 in pm_mpi_worker()\n" ).arg( myrank ).arg( msg.type ) );
            }
            MPI_Abort( MPI_COMM_WORLD, errorno - myrank );
            exit( errorno - myrank );
         }         
      }
   } while( true );
}
