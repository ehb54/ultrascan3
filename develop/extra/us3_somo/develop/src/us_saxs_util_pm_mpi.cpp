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
     cout << QString( "%1: MPI send failed in nsa_ga_worker() initial registry\n" ).arg( myrank ) << flush;
     MPI_Abort( MPI_COMM_WORLD, errorno - myrank );
     exit( errorno - myrank );
     }
   */

   do 
   {
      // cout << QString( "%1: worker listening\n" ).arg( myrank ) << flush;
      if ( MPI_SUCCESS != MPI_Recv( &msg,
                                    sizeof( pm_msg ),
                                    MPI_CHAR, 
                                    0, 
                                    PM_MSG, 
                                    MPI_COMM_WORLD, 
                                    &mpi_status ) )
      {
         cout << QString( "%1: MPI Initial Receive failed in pm_mpi_worker()\n" ).arg( myrank ) << flush;
         MPI_Abort( MPI_COMM_WORLD, errorno - myrank );
         exit( errorno - myrank );
      }         
      
      // cout << msg;

      switch( msg.type )
      {
      case PM_SHUTDOWN :
         {
            cout << QString( "%1: worker PM_SHUTDOWN\n" ).arg( myrank ) << flush;
            MPI_Finalize();
            exit(0);
         }
         break;

      case PM_NEW_PM :
         {
            // cout << QString( "%1: worker PM_NEW_PM\n" ).arg( myrank ) << flush;
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
               cout << QString( "%1: MPI PM_NEW_PM Receive failed in pm_mpi_worker()\n" ).arg( myrank ) << flush;
               MPI_Abort( MPI_COMM_WORLD, errorno - myrank );
               exit( errorno - myrank );
            }         

            if ( use_errors )
            {
               for ( int i = 0; i < msg.vsize; i++ )
               {
                  F[ i ] = d[ i ];
                  q[ i ] = d[ msg.vsize + i ];
                  I[ i ] = d[ 2 * msg.vsize + i ];
                  e[ i ] = d[ 3 * msg.vsize + i ];
               }
            } else {
               for ( int i = 0; i < msg.vsize; i++ )
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
            // cout << QString( "%1: worker PM_NEW_GRID_SIZE\n" ).arg( myrank ) << flush;
            if ( !pm )
            {
               cout << QString( "%1: MPI PM_NEW_GRID_SIZE Receive called before PM_NEW_PM  pm_mpi_worker()\n" ).arg( myrank ) << flush;
               MPI_Abort( MPI_COMM_WORLD, errorno - myrank );
               exit( errorno - myrank );
            }
            pm->set_grid_size( msg.grid_conversion_factor, true );
         }
         break;

      case PM_CALC_FITNESS:
         {
            // cout << QString( "%1: worker PM_CALC_FITNESS\n" ).arg( myrank ) << flush;
            if ( !pm )
            {
               cout << QString( "%1: MPI PM_CALC_FITNESS Receive called before PM_NEW_PM  pm_mpi_worker()\n" ).arg( myrank ) << flush;
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
               cout << QString( "%1: MPI PM_CALC_FITNESS Receive failed in pm_mpi_worker()\n" ).arg( myrank ) << flush;
               MPI_Abort( MPI_COMM_WORLD, errorno - myrank );
               exit( errorno - myrank );
            }         


            // US_Vector::printvector( QString( "%1: PM_CALC_FITNESS params:" ).arg( myrank ), params );
            // cout << flush;

            if ( !pm->create_model( params, model ) ||
                 !pm->compute_I( model, I_result ) )
            {
               cout << QString( "%1: MPI PM_CALC_FITNESS error %2 in pm_mpi_worker()\n" ).arg( myrank ).arg( pm->error_msg ) << flush;
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
               cout << QString( "%1: MPI PM_CALC_FITNESS Send 1 failed in pm_mpi_worker()\n" ).arg( myrank ) << flush;
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
               cout << QString( "%1: MPI PM_CALC_FITNESS Send 2 failed in pm_mpi_worker()\n" ).arg( myrank ) << flush;
               MPI_Abort( MPI_COMM_WORLD, errorno - myrank );
               exit( errorno - myrank );
            }         
         }
         break;

      default:
         {
            cout << QString( "%1: MPI Receive unknown message type %2 in pm_mpi_worker()\n" ).arg( myrank ).arg( msg.type ) << flush;
            MPI_Abort( MPI_COMM_WORLD, errorno - myrank );
            exit( errorno - myrank );
         }         
      }
   } while( true );
}
