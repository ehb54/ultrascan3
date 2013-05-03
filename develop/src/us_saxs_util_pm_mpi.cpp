#include "../include/us_saxs_util.h"
#include "../include/us_pm.h"

// note: this program uses cout and/or cerr and this should be replaced

bool US_Saxs_Util::run_pm_mpi( QString controlfile )
{

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
                                       0, 
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
      cout << QString( "%1: worker listening\n" ).arg( myrank ) << flush;
      if ( MPI_SUCCESS != MPI_Recv( &msg,
                                    sizeof( pm_msg ),
                                    MPI_CHAR, 
                                    0, 
                                    0, 
                                    MPI_COMM_WORLD, 
                                    &mpi_status ) )
      {
         cout << QString( "%1: MPI Initial Receive failed in pm_mpi_worker()\n" ).arg( myrank ) << flush;
         MPI_Abort( MPI_COMM_WORLD, errorno - myrank );
         exit( errorno - myrank );
      }         
      
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
            cout << QString( "%1: worker PM_NEW_PM\n" ).arg( myrank ) << flush;
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
                            true );
         }
         break;

      case PM_NEW_GRID_SIZE:
         {
            cout << QString( "%1: worker PM_NEW_GRID_SIZE\n" ).arg( myrank ) << flush;
            if ( !pm )
            {
               cout << QString( "%1: MPI PM_CALC_FITNESS Receive called before PM_NEW_PM  pm_mpi_worker()\n" ).arg( myrank ) << flush;
               MPI_Abort( MPI_COMM_WORLD, errorno - myrank );
               exit( errorno - myrank );
            }
            pm->set_grid_size( msg.grid_conversion_factor, true );
         }
         break;

      case PM_CALC_FITNESS:
         {
            cout << QString( "%1: worker PM_CALC_FITNESS\n" ).arg( myrank ) << flush;
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
