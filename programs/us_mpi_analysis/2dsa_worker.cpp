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

   // Use 4 here because the master will be reading 4 with the
   // same instruction when reading ::READY or ::RESULTS.
   int x[ 4 ];

   while ( repeat_loop )
   {
      MPI_Send( x, // Basically don't care
                4,
                MPI_INT,
                MPI_Job::MASTER,
                MPI_Job::READY,
                my_communicator ); // let master know we are ready
//if(my_rank==1)
DbgLv(1) << "w:" << my_rank << ": ready sent";

      // Blocking -- Wait for instructions
      MPI_Recv( &job, // get masters' response
                sizeof( job ),
                MPI_BYTE,
                MPI_Job::MASTER,
                MPI_Job::TAG0,
                my_communicator,
                &status );        // status not used
//if(my_rank==1)
DbgLv(1) << "w:" << my_rank << ": job_recvd  length" << job.length
 << "command" << job.command;

      meniscus_value     = job.meniscus_value;
      bottom_value       = job.bottom_value;
      int offset         = job.dataset_offset;
      int bfg_offset     = job.bfg_offset;
      int dataset_count  = job.dataset_count;
      int job_length     = job.length;
      int noisflag       = ( parameters[ "tinoise_option" ].toInt() > 0 ?  1 : 0 )
                         + ( parameters[ "rinoise_option" ].toInt() > 0 ?  2 : 0 );
DbgLv(1) << "w:" << my_rank << ": offs cnt" << offset << dataset_count;

      data_sets[ offset ]->run_data.meniscus  = meniscus_value;
      data_sets[ offset ]->simparams.cp_angle   = bottom_value;
      data_sets[ offset ]->simparams.meniscus = meniscus_value;
      //data_sets[ offset ]->simparams.bottom   = bottom_value;

      switch( job.command )
      {

         case MPI_Job::PROCESS:  // Process solutes
            {
               US_SolveSim::Simulation simulation_values;
               simulation_values.noisflag    = noisflag;
               simulation_values.dbg_level   = dbg_level;
               simulation_values.dbg_timing  = dbg_timing;

//DbgLv(1) << "w:" << my_rank << ": sols size" << job.length;
//if(my_rank==1)
DbgLv(1) << "w:" << my_rank << ": sols size" << job.length;
               simulation_values.solutes.resize( job.length );

               MPI_Recv( simulation_values.solutes.data(), // Get solutes
                         job_length * solute_doubles,
                         MPI_DOUBLE,
                         MPI_Job::MASTER,
                         MPI_Job::TAG0,
                         my_communicator,
                         &status );

               max_rss();
//*DEBUG*
//if(dbg_level>0 && my_rank==1)
//if(my_rank==1)
{
 int nn = simulation_values.solutes.size() - 1;
 int mm = nn/2;
 DbgLv(1) << "w:" << my_rank << ": offs dscnt" << offset << dataset_count
  << "vbar s20wc cpbott dabott"
  << data_sets[offset]->vbar20
  << data_sets[offset]->s20w_correction
  << data_sets[offset]->centerpiece_bottom
  << data_sets[offset]->run_data.bottom;
 DbgLv(1) << "w:" << my_rank << ": sol0 solm soln"
  << simulation_values.solutes[0].s << simulation_values.solutes[0].k
  << simulation_values.solutes[mm].s << simulation_values.solutes[mm].k
  << simulation_values.solutes[nn].s << simulation_values.solutes[nn].k;
}
//*DEBUG*

               calc_residuals( offset, dataset_count, simulation_values, bfg_offset );

               // Tell master we are sending back results
               int size[ 4 ] = { simulation_values.solutes.size(),
                                 simulation_values.ti_noise.size(),
                                 simulation_values.ri_noise.size(),
                                 (int)max_rss() };

DbgLv(1) << "w:" << my_rank << ":   result sols size" << size[0]
 << "max_rss" << size[ 3 ];
//*DEBUG*
if(dbg_level==0 && my_rank==1) {
DbgLv(1) << "w:" << my_rank << ":   result sols size" << size[0]
 << "nsscan" << simulation_values.sim_data.scanCount();
}
//*DEBUG*
               MPI_Send( size,
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
               int  mc_iter    = job.solution;

               //if ( dataset_count > 0  &&  mc_iter < 4 )
               if ( is_global_fit  &&  mc_iter < 3  &&  my_rank < 3 )
               {  // For global fits, check the memory requirements
                  long memused    = max_rss();
                  long memdata    = job_length * sizeof( double );
                  int grid_reps   = qMax( parameters[ "uniform_grid" ].toInt(), 1 );
                  double s_pts    = 60.0;
                  double ff0_pts  = 60.0;
                  if ( parameters.contains( "s_grid_points"   ) )
                     s_pts   = parameters[ "s_grid_points"   ].toDouble();
                  else if ( parameters.contains( "s_resolution"    ) )
                     s_pts   = parameters[ "s_resolution"    ].toDouble() * grid_reps;
                  if ( parameters.contains( "ff0_grid_points" ) )
                     ff0_pts = parameters[ "ff0_grid_points" ].toDouble();
                  else if ( parameters.contains( "ff0_resolution"  ) )
                     ff0_pts = parameters[ "ff0_resolution"  ].toDouble() * grid_reps;
                  int  nsstep     = (int)( s_pts );
                  int  nkstep     = (int)( ff0_pts );
                  //grid_reps       = US_Math2::best_grid_reps( nsstep, nkstep );
                  int  maxsols    = nsstep * nkstep;
                  long memamatr   = memdata * maxsols;
                  long membmatr   = memdata;
                  long memneed    = memdata + memamatr + membmatr;
                  const double mb_bytes = ( 1024. * 1024. );
                  const double gb_bytes = ( mb_bytes * 1024. );
                  double gb_need  = (double)memneed / gb_bytes;
                  gb_need         = qRound( gb_need * 1000.0 ) * 0.001;
                  double gb_used  = (double)memused / mb_bytes;
                  gb_used         = qRound( gb_used * 1000.0 ) * 0.001;
                  long pgavail    = sysconf( _SC_PHYS_PAGES );
                  long pgsize     = sysconf( _SC_PAGE_SIZE );
                  long memavail   = pgavail * pgsize;
                  double gb_avail = (double)memavail / gb_bytes;
                  gb_avail        = qRound( gb_avail * 1000.0 ) * 0.001;
                  long pgcurav    = sysconf( _SC_AVPHYS_PAGES );
                  long memcurav   = pgcurav * pgsize;
                  double gb_curav = (double)memcurav / gb_bytes;
                  gb_curav        = qRound( gb_curav * 1000.0 ) * 0.001;

                  qDebug() << "++ Worker" << my_rank << ": MC iteration"
                     << mc_iter << ": Memory Profile :"
                     << "\n    Maximum memory used to this point" << memused
                     << "\n    Composite data memory needed" << memdata
                     << "\n    Maximum subgrid solute count" << maxsols
                     << "\n    NNLS A matrix memory needed" << memamatr
                     << "\n    NNLS B matrix memory needed" << membmatr
                     << "\n    Total memory (GB) used" << gb_used
                     << "\n    Total memory (GB) needed" << gb_need
                     << "\n    Total memory (GB) available" << gb_avail
                     << "\n    Memory (GB) currently available" << gb_curav;
               }

               mc_data.resize( job_length );

               if ( mc_data.size() != job_length )
               {
                  DbgLv(0) << "*ERROR* mc_data.size() job_length"
                     << mc_data.size() << job_length;
               }

               MPI_Barrier( my_communicator );

if(my_rank==1 || my_rank==11)
DbgLv(1) << "newD:" << my_rank << " scld/newdat rcv : offs dsknt"
 << offset << dataset_count << "joblen" << job_length;
double dsum=0.0;
               // This is a receive
               MPI_Bcast( mc_data.data(),
                          job_length,
                          MPI_DOUBLE,
                          MPI_Job::MASTER,
                          my_communicator );


               if ( is_global_fit  &&  dataset_count == 1 )
               {  // For global update to scaled data, extra value is new ODlimit
                  job_length--;
                  data_sets[ offset ]->run_data.ODlimit = mc_data[ job_length ];
if( (my_rank==1||my_rank==11) )
DbgLv(1) << "newD:" << my_rank << ":offset ODlimit" << offset
 << data_sets[ offset ]->run_data.ODlimit;
               }

               int index = 0;

               for ( int ee = offset; ee < offset + dataset_count; ee++ )
               {
                  US_DataIO::EditedData* edata = &data_sets[ ee ]->run_data;

                  int scan_count    = edata->scanCount();
                  int radius_points = edata->pointCount();

int indxh=((scan_count/2)*radius_points)+(radius_points/2);
                  for ( int ss = 0; ss < scan_count; ss++ )
                  {
                     for ( int rr = 0; rr < radius_points; rr++, index++ )
                     {
                        edata->setValue( ss, rr, mc_data[ index ] );
dsum+=edata->value(ss,rr);
if( (my_rank==1||my_rank==11)
 && (index<5 || index>(job_length-6) || (index>(indxh-4)&&index<(indxh+3))) )
DbgLv(1) << "newD:" << my_rank << ":index" << index << "edat" << edata->value(ss,rr)
 << "ee" << ee;
                     }
                  }
               }
if(my_rank==1 || my_rank==11)
DbgLv(1) << "newD:" << my_rank << "  length index" << job_length << index
 << "dsum" << dsum;
            }

            break;

         default:
            repeat_loop = false;
            break;
      }  // switch
   }  // repeat_loop
}

