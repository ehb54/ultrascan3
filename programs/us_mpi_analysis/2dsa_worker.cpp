#include "us_mpi_analysis.h"
#include "us_constants.h"
#include "us_math2.h"
#include "us_astfem_rsa.h"
#include "us_simparms.h"

void US_MPI_Analysis::_2dsa_worker( void )
{
   bool repeat_loop = true;
   MPI_Job     job;
   MPI_Status  status;

   // Use 3 here because the master will be reading 3 with the
   // same instruction when reading ::READY or ::RESULTS.
   int x[ 3 ];

   while ( repeat_loop )
   {
      MPI_Send( x, // Basically don't care
                3,
                MPI_INT,
                MPI_Job::MASTER,
                MPI_Job::READY,
                MPI_COMM_WORLD ); // let master know we are ready

      // Blocking -- Wait for instructions
      MPI_Recv( &job, // get masters' response
                sizeof( job ),
                MPI_BYTE,
                MPI_Job::MASTER,
                MPI_Job::TAG0,
                MPI_COMM_WORLD,
                &status );        // status not used

      meniscus_value     = job.meniscus_value;
      int offset         = job.dataset_offset;
      int dataset_count  = job.dataset_count;

      data_sets[ 0 ]->run_data.meniscus  = meniscus_value;
      data_sets[ 0 ]->simparams.meniscus = meniscus_value;

      switch( job.command )
      {

         case MPI_Job::PROCESS:  // Process solutes
            {
               US_SolveSim::Simulation simulation_values;

               simulation_values.noisflag    =
                  parameters[ "tinoise_option" ].toInt() > 0 ?  1 : 0;
               simulation_values.noisflag   +=
                  parameters[ "rinoise_option" ].toInt() > 0 ?  2 : 0;
               simulation_values.dbg_level   = dbg_level;
               simulation_values.dbg_timing  = dbg_timing;

DbgLv(1) << "w:" << my_rank << ": sols size" << job.length;
               simulation_values.solutes.resize( job.length );

               MPI_Recv( simulation_values.solutes.data(), // Get solutes
                         job.length * solute_doubles,
                         MPI_DOUBLE,
                         MPI_Job::MASTER,
                         MPI_Job::TAG0,
                         MPI_COMM_WORLD,
                         &status );

               max_rss();

               calc_residuals( offset, dataset_count, simulation_values );

               // Tell master we are sending back results
               int size[ 4 ] = { simulation_values.solutes.size(),
                                 simulation_values.ti_noise.size(),
                                 simulation_values.ri_noise.size(),
                                 max_rss() };

DbgLv(1) << "w:" << my_rank << ":   result sols size" << size[0];
               MPI_Send( &size,
                         4,
                         MPI_INT,
                         MPI_Job::MASTER,
                         MPI_Job::RESULTS,
                         MPI_COMM_WORLD );

               // Send back to master all of simulation_values
               MPI_Send( simulation_values.solutes.data(),
                         simulation_values.solutes.size() * solute_doubles,
                         MPI_DOUBLE,
                         MPI_Job::MASTER,
                         MPI_Job::TAG0,
                         MPI_COMM_WORLD );

               MPI_Send( &simulation_values.variance,
                         1,
                         MPI_DOUBLE,
                         MPI_Job::MASTER,
                         MPI_Job::TAG0,
                         MPI_COMM_WORLD );

               MPI_Send( simulation_values.variances.data(),
                         data_sets.size(),
                         MPI_DOUBLE,
                         MPI_Job::MASTER,
                         MPI_Job::TAG0,
                         MPI_COMM_WORLD );

               MPI_Send( simulation_values.ti_noise.data(),
                         simulation_values.ti_noise.size(),
                         MPI_DOUBLE,
                         MPI_Job::MASTER,
                         MPI_Job::TAG0,
                         MPI_COMM_WORLD );

               MPI_Send( simulation_values.ri_noise.data(),
                         simulation_values.ri_noise.size(),
                         MPI_DOUBLE,
                         MPI_Job::MASTER,
                         MPI_Job::TAG0,
                         MPI_COMM_WORLD );
            }

            break;

         case MPI_Job::NEWDATA:  // Reset data for Monte Carlo or global fit
            { 
               mc_data.resize( job.length );

               MPI_Barrier( MPI_COMM_WORLD );

               // This is a receive
               MPI_Bcast( mc_data.data(),
                          job.length,
                          MPI_DOUBLE,
                          MPI_Job::MASTER,
                          MPI_COMM_WORLD );

               int index = 0;

               for ( int e = offset; e < offset + dataset_count; e++ )
               {
                  US_DataIO2::EditedData* data = &data_sets[ e ]->run_data;

                  int scan_count    = data->scanData.size();
                  int radius_points = data->x.size();

//int indxh=((scan_count/2)*radius_points)+(radius_points/2);
                  for ( int s = 0; s < scan_count; s++ )
                  {
                     US_DataIO2::Scan* scan = &data->scanData[ s ];

                     for ( int r = 0; r < radius_points; r++ )
                     {
                        scan->readings[ r ].value = mc_data[ index ];
//if ( index<5 || index>(job.length-6) || (index>(indxh-4)&&index<(indxh+3)) )
//DbgLv(1) << "newD:" << my_rank << ":index" << index << "edat" << data->value(s,r);
                        index++;
                     }
                  }
               }
            }

            break;

         default:
            repeat_loop = false;
            break;
      }  // switch
   }  // repeat_loop
}

void US_MPI_Analysis::calc_residuals( int         offset,
                                      int         dataset_count,
                                      US_SolveSim::Simulation& simu_values )
{
   count_calc_residuals++;

   US_SolveSim solvesim( data_sets, my_rank, false );

int dbglvsv=simu_values.dbg_level;
simu_values.dbg_level=(dbglvsv>1?dbglvsv:0);

   solvesim.calc_residuals( offset, dataset_count, simu_values );

simu_values.dbg_level=dbglvsv;
if ( dbg_level > 0 && ( my_rank == 1 || my_rank == 11 ) ) {
 US_DataIO2::EditedData* data = &data_sets[0]->run_data;
 int nsc=data->scanData.size();
 int nrp=data->x.size();
 double d0 = data->scanData[0].readings[0].value;
 double d1 = data->scanData[0].readings[1].value;
 double dh = data->scanData[nsc/2].readings[nrp/2].value;
 double dm = data->scanData[nsc-1].readings[nrp-2].value;
 double dn = data->scanData[nsc-1].readings[nrp-1].value;
 DbgLv(1) << "w:" << my_rank << ":d(01hmn)" << d0 << d1 << dh << dm << dn;
}
 
}

