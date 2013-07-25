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
                my_communicator ); // let master know we are ready

      // Blocking -- Wait for instructions
      MPI_Recv( &job, // get masters' response
                sizeof( job ),
                MPI_BYTE,
                MPI_Job::MASTER,
                MPI_Job::TAG0,
                my_communicator,
                &status );        // status not used

      meniscus_value     = job.meniscus_value;
      int offset         = job.dataset_offset;
      int dataset_count  = job.dataset_count;

      data_sets[ offset ]->run_data.meniscus  = meniscus_value;
      data_sets[ offset ]->simparams.meniscus = meniscus_value;

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
                         my_communicator,
                         &status );

               max_rss();
//*DEBUG*
if(dbg_level>0 && my_rank==1) {
 int nn = simulation_values.solutes.size() - 1;
 int mm = nn/2;
 DbgLv(1) << "w:" << my_rank << ": offs dscnt" << offset << dataset_count
  << "vbar s20wc bott"
  << data_sets[offset]->vbar20
  << data_sets[offset]->s20w_correction
  << data_sets[offset]->centerpiece_bottom;
 DbgLv(1) << "w:" << my_rank << ": sol0 solm soln"
  << simulation_values.solutes[0].s << simulation_values.solutes[0].k
  << simulation_values.solutes[mm].s << simulation_values.solutes[mm].k
  << simulation_values.solutes[nn].s << simulation_values.solutes[nn].k;
}
//*DEBUG*

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
                         my_communicator );

               // Send back to master all of simulation_values
               MPI_Send( simulation_values.solutes.data(),
                         simulation_values.solutes.size() * solute_doubles,
                         MPI_DOUBLE,
                         MPI_Job::MASTER,
                         MPI_Job::TAG0,
                         my_communicator );

               MPI_Send( &simulation_values.variance,
                         1,
                         MPI_DOUBLE,
                         MPI_Job::MASTER,
                         MPI_Job::TAG0,
                         my_communicator );

               MPI_Send( simulation_values.variances.data(),
                         dataset_count,
                         MPI_DOUBLE,
                         MPI_Job::MASTER,
                         MPI_Job::TAG0,
                         my_communicator );

               MPI_Send( simulation_values.ti_noise.data(),
                         simulation_values.ti_noise.size(),
                         MPI_DOUBLE,
                         MPI_Job::MASTER,
                         MPI_Job::TAG0,
                         my_communicator );

               MPI_Send( simulation_values.ri_noise.data(),
                         simulation_values.ri_noise.size(),
                         MPI_DOUBLE,
                         MPI_Job::MASTER,
                         MPI_Job::TAG0,
                         my_communicator );
            }

            break;

         case MPI_Job::NEWDATA:  // Reset data for Monte Carlo or global fit
            { 
               mc_data.resize( job.length );

               MPI_Barrier( my_communicator );

if(my_rank==1 || my_rank==11)
DbgLv(1) << "newD:" << my_rank << " scld/newdat rcv : offs dsknt"
 << offset << dataset_count;
               // This is a receive
               MPI_Bcast( mc_data.data(),
                          job.length,
                          MPI_DOUBLE,
                          MPI_Job::MASTER,
                          my_communicator );

               int index = 0;

               for ( int ee = offset; ee < offset + dataset_count; ee++ )
               {
                  US_DataIO::EditedData* data = &data_sets[ ee ]->run_data;

                  int scan_count    = data->scanCount();
                  int radius_points = data->pointCount();

int indxh=((scan_count/2)*radius_points)+(radius_points/2);
                  for ( int ss = 0; ss < scan_count; ss++ )
                  {
                     for ( int rr = 0; rr < radius_points; rr++, index++ )
                     {
                        data->setValue( ss, rr, mc_data[ index ] );
if( (my_rank==1||my_rank==11)
 && (index<5 || index>(job.length-6) || (index>(indxh-4)&&index<(indxh+3))) )
DbgLv(1) << "newD:" << my_rank << ":index" << index << "edat" << data->value(ss,rr);
                     }
                  }
               }
if(my_rank==1 || my_rank==11)
DbgLv(1) << "newD:" << my_rank << "  length index" << job.length << index;
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

//*DEBUG*
int dbglvsv=simu_values.dbg_level;
//simu_values.dbg_level=(dbglvsv>1||my_rank==1)?dbglvsv:0;
simu_values.dbg_level=(dbglvsv>1)?dbglvsv:0;
int nsoli=simu_values.solutes.size();
//*DEBUG*

   solvesim.calc_residuals( offset, dataset_count, simu_values );

//*DEBUG*
simu_values.dbg_level=dbglvsv;
if ( dbg_level > 0 && ( group_rank == 1 || group_rank == 11 ) ) {
 int nsolo=simu_values.solutes.size();
 US_DataIO::EditedData* data = &data_sets[offset]->run_data;
 US_SolveSim::DataSet*  dset = data_sets[offset];
 US_DataIO::RawData*    sdat = &simu_values.sim_data;
 int nsc=data->scanCount();
 int nrp=data->pointCount();
 double d0 = data->scanData[0].rvalues[0];
 double d1 = data->scanData[0].rvalues[1];
 double dh = data->scanData[nsc/2].rvalues[nrp/2];
 double dm = data->scanData[nsc-1].rvalues[nrp-2];
 double dn = data->scanData[nsc-1].rvalues[nrp-1];
 DbgLv(1) << "w:" << my_rank << ":d(01hmn)" << d0 << d1 << dh << dm << dn;
 double s0 = sdat->value(0,0);
 double s1 = sdat->value(0,1);
 double sh = sdat->value(nsc/2,nrp/2);
 double sm = sdat->value(nsc-1,nrp-2);
 double sn = sdat->value(nsc-1,nrp-1);
 DbgLv(1) << "w:" << my_rank << ": s(01hmn)" << s0 << s1 << sh << sm << sn;
 DbgLv(1) << "w:" << my_rank << ":  nsoli nsolo" << nsoli << nsolo;
 DbgLv(1) << "w:" << my_rank << ":  simpt men bott temp coef1"
  << dset->simparams.simpoints
  << dset->simparams.meniscus
  << dset->simparams.bottom
  << dset->simparams.temperature
  << dset->simparams.rotorcoeffs[ 0 ];
 DbgLv(1) << "w:" << my_rank << ":  vbar soltype manual visc dens"
  << dset->vbar20 << dset->solute_type << dset->manual
  << dset->viscosity << dset->density;
}
//*DEBUG*
 
}

