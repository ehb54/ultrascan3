#include "us_mpi_analysis.h"
#include "us_math2.h"
#include "us_util.h"
#include "us_tar.h"
#include "us_astfem_rsa.h"
#include "us_simparms.h"
#include "us_constants.h"

void US_MPI_Analysis::pcsa_master( void )
{
   init_solutes();
   fill_queue();

   work_rss.resize( gcores_count );

   current_dataset     = 0;
   datasets_to_process = 1;  // Process one dataset at a time for now
   dset_calc_solutes.clear();

   int max_iters_all   = max_iterations;

   if ( mc_iterations > 1 )
      max_iterations   = max_iters_all > 1 ? max_iters_all : 5;

   while ( true )
   {
      int worker;
      meniscus_value   = data_sets[ current_dataset ]->run_data.meniscus;
//if ( max_depth > 1 )
// DbgLv(1) << " master loop-TOP:  jq-empty?" << job_queue.isEmpty() << "   areReady?" << worker_status.contains(READY)
//    << "  areWorking?" << worker_status.contains(WORKING);

      // Give the jobs to the workers
      while ( ! job_queue.isEmpty()  &&  worker_status.contains( READY ) )
      {
         worker    = ready_worker();

         Sa_Job job              = job_queue.takeFirst();
         submit_pcsa( job, worker );
         worker_depth [ worker ] = job.mpi_job.depth;
         worker_status[ worker ] = WORKING;
      }

      // All done with the pass if no jobs are ready or running
      if ( job_queue.isEmpty()  &&  ! worker_status.contains( WORKING ) ) 
      {
         US_DataIO::EditedData* edata = &data_sets[ current_dataset ]->run_data;
         QString tripleID = edata->cell + edata->channel + edata->wavelength;
         QString progress = 
            "Iteration: "    + QString::number( iterations );

         if ( datasets_to_process > 1 )
            progress     += "; Datasets: "
                            + QString::number( datasets_to_process );
         else
            progress     += "; Dataset: "
                            + QString::number( current_dataset + 1 )
                            + " (" + tripleID + ")";

         if ( mc_iterations > 1 )
            progress     += "; MonteCarlo: "
                            + QString::number( mc_iteration + 1 );

         else if ( meniscus_values.size() > 1 )
            progress     += "; Meniscus: "
               + QString::number( meniscus_value, 'f', 3 )
               + tr( " (%1 of %2)" ).arg( meniscus_run + 1 )
                                    .arg( meniscus_values.size() );
         else
            progress     += "; RMSD: "
               + QString::number( sqrt( simulation_values.variance ) );

         send_udp( progress );

         // Iterative refinement
         if ( max_iterations > 1 )
         {
            if ( data_sets.size() > 1  &&  iterations == 1 )
            {
               if ( datasets_to_process == 1 )
               {
                  qDebug() << "   == Refinement Iterations for Dataset"
                     << current_dataset + 1 << "==";
               }
               else
               {
                  qDebug() << "   == Refinement Iterations for Datasets 1 to"
                     << datasets_to_process << "==";
               }
            }
            qDebug() << "Iteration:" << iterations << " Variance:"
               << simulation_values.variance << "RMSD:"
               << sqrt( simulation_values.variance );

            iterate();
         }

         if ( ! job_queue.isEmpty() ) continue;

         iterations = 1;
DbgLv(1) << " master loop-BOT: dssize" << data_sets.size() << "ds_to_p"
 << datasets_to_process << "curr_ds" << current_dataset;
US_DataIO::EditedData* edat=&data_sets[current_dataset]->run_data;
int ks=edat->scanCount() - 10;
int kr=edat->pointCount() - 10;
int ss=ks/2;
int rr=kr/2;
DbgLv(1) << " master loop-BOT: ds" << current_dataset+1 << "data l m h"
 << edat->value(10,10) << edat->value(ss,rr) << edat->value(ks,kr);

         // Manage multiple data sets in global fit
         if ( is_global_fit  &&  datasets_to_process == 1 )
         {
            global_fit();
         }
DbgLv(1) << " master loop-BOT: GF job_queue empty" << job_queue.isEmpty();

         if ( ! job_queue.isEmpty() ) continue;

         if ( is_global_fit )
            write_global();

         else
            write_output();

         if ( ! job_queue.isEmpty() ) continue;

         // Monte Carlo
         if ( mc_iterations > 1 )
         {  // Recompute final fit to get simulation and residual
            mc_iteration++;
            wksim_vals           = simulation_values;
            wksim_vals.solutes   = calculated_solutes[ max_depth ]; 

            calc_residuals( 0, data_sets.size(), wksim_vals );

            qDebug() << "Base-Sim RMSD" << sqrt( simulation_values.variance )
                     << "  Exp-Sim RMSD" << sqrt( wksim_vals.variance )
                     << "  of MC_Iteration" << mc_iteration;
            max_iterations              = max_iters_all;
            simulation_values           = wksim_vals;

            if ( mc_iteration < mc_iterations )
            {
               time_mc_iterations();

               set_monteCarlo();
            }
         }

         if ( ! job_queue.isEmpty() ) continue;

         if ( is_composite_job )
         {  // Composite job:  update outputs in TAR and bump dataset count
            QString tripleID = QString( data_sets[ current_dataset ]->model
                               .description ).section( ".", -3, -3 );
            current_dataset++;
            dset_calc_solutes << calculated_solutes[ max_depth ];

            update_outputs();

            if ( simulation_values.noisflag == 0 )
            {
               DbgLv(0) << my_rank << ": Dataset" << current_dataset
                        << "(" << tripleID << ")"
                        << " :  model was output.";
            }
            else
            {
               DbgLv(0) << my_rank << ": Dataset" << current_dataset
                        << "(" << tripleID << ")"
                        << " :  model/noise(s) were output.";
            }

            if ( current_dataset < count_datasets )
            {
               iterations      = 1;
               mc_iteration    = 0;

               for ( int ii = 1; ii < gcores_count; ii++ )
                  worker_status[ ii ] = READY;

               fill_queue();

               for ( int ii = 0; ii < calculated_solutes.size(); ii++ )
                  calculated_solutes[ ii ].clear();

               continue;
            }
         }

         shutdown_all();  // All done
         break;           // Break out of main loop.
      }

      // Wait for worker to send a message
      int        size[ 4 ];
      MPI_Status status;

      MPI_Recv( &size, 
                4, 
                MPI_INT,
                MPI_ANY_SOURCE,
                MPI_ANY_TAG,
                my_communicator,
                &status);

//if ( max_depth > 0 )
// DbgLv(1) << " master loop-BOTTOM:   status TAG" << status.MPI_TAG << MPI_Job::READY << MPI_Job::RESULTS
//    << "  source" << status.MPI_SOURCE;
      switch( status.MPI_TAG )
      {
         case MPI_Job::READY:   // Ready for work
            worker = status.MPI_SOURCE;
            worker_status[ worker ] = READY;
            break;

         case MPI_Job::RESULTS: // Return solute data
            process_results( status.MPI_SOURCE, size );
            worker = status.MPI_SOURCE;
            work_rss[ worker ] = size[ 3 ];
            break;

         default:  // Should never happen
            QString msg =  "Master PCSA:  Received invalid status " +
                           QString::number( status.MPI_TAG );
            abort( msg );
            break;
      }

      max_rss();
   }
}

// Generate the initial set of solutes
void US_MPI_Analysis::init_pcsa_solutes( void )
{
   calculated_solutes.clear();
   orig_solutes      .clear();
   simulation_values.noisflag    = parameters[ "tinoise_option" ].toInt() > 0 ?
                                   1 : 0;
   simulation_values.noisflag   += parameters[ "rinoise_option" ].toInt() > 0 ?
                                   2 : 0;
   simulation_values.dbg_level   = dbg_level;
   simulation_values.dbg_timing  = dbg_timing;
DbgLv(0) << "DEBUG_LEVEL" << simulation_values.dbg_level;

   double s_min      = parameters[ "s_min"           ].toDouble() * 1.0e-13;
   double s_max      = parameters[ "s_max"           ].toDouble() * 1.0e-13;
   double ff0_min    = parameters[ "ff0_min"         ].toDouble();
   double ff0_max    = parameters[ "ff0_max"         ].toDouble();
   int    tikreg     = parameters[ "tikreg_option"   ].toInt();
   double tr_alpha   = parameters[ "tikreg_alpha"    ].toDouble();
   tr_alpha          = ( tikreg == 1 ) ? tr_alpha : 0.0;
   QString s_ctyp    = parameters[ "curve_type" ];
   int    ctype      = cTypeMap[ s_ctyp ];
   int    nkpts      = parameters[ "vars_count"      ].toInt();
   int    nlpts      = parameters[ "curves_points"   ].toInt();
//   int    gf_iters   = parameters[ "gfit_iterations" ].toInt();
//   double thr_dr_rat = parameters[ "thr_deltr_ratio" ].toDouble();
   double pararry[ 5 ];
   double *parlims   = (double*)pararry;
   parlims[ 0 ]      = ff0_min;
   parlims[ 1 ]      = ff0_max;
   parlims[ 2 ]      = ff0_min;
   parlims[ 3 ]      = ff0_max;
   parlims[ 4 ]      = tr_alpha;

   if ( s_ctyp == "SL" )
   {
      US_ModelRecord::compute_slines( s_min, s_max, ff0_min, ff0_max,
            nkpts, nlpts, parlims, mrecs );
   }

   else if ( s_ctyp == "IS"  ||  s_ctyp == "DS" )
   {
      US_ModelRecord::compute_sigmoids( ctype, s_min, s_max, ff0_min, ff0_max,
            nkpts, nlpts, parlims, mrecs );
   }

   else if ( s_ctyp == "HL" )
   {
      US_ModelRecord::compute_slines( s_min, s_max, ff0_min, ff0_max,
            nkpts, nlpts, parlims, mrecs );
   }

   else if ( s_ctyp == "All" )
   {
      QVector< US_ModelRecord > mrecs2;
      int ctype1        = cTypeMap[ "IS" ];
      int ctype2        = cTypeMap[ "DS" ];

      US_ModelRecord::compute_slines( s_min, s_max, ff0_min, ff0_max,
            nkpts, nlpts, parlims, mrecs );
      US_ModelRecord::compute_sigmoids( ctype1, s_min, s_max, ff0_min, ff0_max,
            nkpts, nlpts, parlims, mrecs2 );
      mrecs << mrecs2;
      US_ModelRecord::compute_sigmoids( ctype2, s_min, s_max, ff0_min, ff0_max,
            nkpts, nlpts, parlims, mrecs2 );
      mrecs << mrecs2;
      nkpts             = mrecs.size();
   }

   for ( int ii = 0; ii < nkpts; ii++ )
   {
      orig_solutes[ ii] = mrecs[ ii ].isolutes;
   }
}

/////////////////////
void US_MPI_Analysis::submit_pcsa( Sa_Job& job, int worker )
{
   job.mpi_job.command        = MPI_Job::PROCESS;
   job.mpi_job.length         = job.solutes.size(); 
   job.mpi_job.meniscus_value = alpha;
   job.mpi_job.solution       = mc_iteration;
   job.mpi_job.dataset_offset = current_dataset;
   job.mpi_job.dataset_count  = datasets_to_process;
int dd=job.mpi_job.depth;
if (dd==0) { DbgLv(1) << "Mast: submit: worker" << worker << "  sols"
 << job.mpi_job.length << "mciter cds" << mc_iteration << current_dataset << " depth" << dd; }
else { DbgLv(1) << "Mast: submit:     worker" << worker << "  sols"
 << job.mpi_job.length << "mciter cds" << mc_iteration << current_dataset << " depth" << dd; }
DbgLv(1) << "Mast: submit: len sol offs cnt" 
 << job.mpi_job.length
 << job.mpi_job.solution
 << job.mpi_job.dataset_offset
 << job.mpi_job.dataset_count;

   // Tell worker that solutes are coming
   MPI_Send( &job.mpi_job, 
       sizeof( MPI_Job ), 
       MPI_BYTE,
       worker,      // Send to system that needs work
       MPI_Job::MASTER,
       my_communicator );
DbgLv(1) << "Mast: submit: send #1";

   // Send solutes
   MPI_Send( job.solutes.data(), 
       job.mpi_job.length * solute_doubles, 
       MPI_DOUBLE,   // Pass solute vector as hw independent values
       worker,       // to worker
       MPI_Job::MASTER,
       my_communicator );
DbgLv(1) << "Mast: submit: send #2";
}

