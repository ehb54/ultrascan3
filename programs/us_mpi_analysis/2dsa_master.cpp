//! \file us_mpi_analysis_2dsa_master.cpp
#include "us_astfem_math.h"
#include "us_astfem_rsa.h"
#include "us_constants.h"
#include "us_math2.h"
#include "us_mpi_analysis.h"
#include "us_simparms.h"
#include "us_tar.h"
#include "us_util.h"

// Drop duplicate solutes from a sorted merge-job solute vector
static void unique_solutes( QVector<US_Solute>& solutes )
{
   solutes.erase( std::unique( solutes.begin(), solutes.end() ), solutes.end() );
}

// SplitMix64 finalizer
static quint64 mix64( quint64 zz )
{
   zz += 0x9e3779b97f4a7c15ULL;
   zz = ( zz ^ ( zz >> 30 ) ) * 0xbf58476d1ce4e5b9ULL;
   zz = ( zz ^ ( zz >> 27 ) ) * 0x94d049bb133111ebULL;
   return zz ^ ( zz >> 31 );
}

// Monte Carlo seed for one dataset and iteration of a fixed-seed job, so
//  that nearby seeds, datasets and iterations give unrelated streams
static uint mc_seed( uint seed, int dataset, int iteration )
{
   quint64 zz    = mix64( mix64( mix64( seed ) ^ (quint32) dataset ) ^ (quint32) iteration );
   uint    mixed = (uint) ( zz ^ ( zz >> 32 ) );
   return ( mixed != 0 ) ? mixed : 1; // Zero would ask for a time-based seed
}

// 2DSA master.  The same loop serves a single master and the group masters
//  of a parallel-masters job, where a supervisor hands out Monte Carlo
//  iterations (pm_mc) or the datasets of a composite job (pm_comp).
void US_MPI_Analysis::_2dsa_master( void )
{
   bool pm_mc   = ( mgroup_count > 1 && !is_composite_job );
   bool pm_comp = ( mgroup_count > 1 && is_composite_job );
   int  super   = 0; // Supervisor world rank
   int  pm_iter = 1; // pm_mc: Monte Carlo iteration of this group

   init_solutes();
   work_rss.resize( gcores_count );

   // A composite job fits its datasets one at a time
   datasets_to_process = is_composite_job ? 1 : count_datasets;
   dset_calc_solutes.clear();

   max_iters_all = max_iterations;

   if ( fit_mb_select == 0 )
   {
      menibott_count      = 1;
      meniscus_points     = 1;
      bottom_points       = 1;
   }
   else if ( fit_menbot )
   {
      menibott_count      = meniscus_points * bottom_points;
   }
   else if ( fit_meni )
   {
      menibott_count      = meniscus_points;
      bottom_points       = 1;
   }
   else if ( fit_bott )
   {
      menibott_count      = bottom_points;
      meniscus_points     = 1;
   }

   int first_dataset = 0;

   if ( pm_mc )
   {
      pm_iter = pm_next_unit( super ); // First MC iteration

      if ( pm_iter < 0 )
      { // More groups than Monte Carlo iterations:  nothing to do but
         //  report the group's memory use
         worker_status.fill( INIT, gcores_count );
         wait_workers_ready();
         int rss = pm_group_rss();
         MPI_Send( &rss, 1, MPI_INT, super, DONELAST, MPI_COMM_WORLD );
         shutdown_all();
         return;
      }
   }

   else if ( pm_comp )
   {
      first_dataset = pm_next_unit( super ); // First dataset
   }

   start_2dsa_dataset( first_dataset, false );

   if ( pm_mc && pm_iter > 1 )
   { // Only group 0 fits the unmodified data:  take the Monte Carlo sigmas
      //  and base simulation from it and start on this group's iteration
      pm_recv_mc_base();
      max_iterations = max_iters_all;
      mc_iteration   = pm_iter - 1;
      set_monteCarlo();
   }

   while ( true )
   {
      run_2dsa_jobs();

      // All done with the pass: no jobs are ready or running
      send_udp( progress_2dsa() );

      // Iterative refinement
      if ( max_iterations > 1 )
      {
         if ( data_sets.size() > 1 && iterations == 1 )
         {
            if ( datasets_to_process == 1 )
            {
               qDebug() << "   == Refinement Iterations for Dataset" << current_dataset + 1 << "==";
            }
            else
            {
               qDebug() << "   == Refinement Iterations for Datasets 1 to" << datasets_to_process << "==";
            }
         }
         qDebug() << "Iteration:" << iterations << " Variance:" << simulation_values.variance
                  << "RMSD:" << sqrt( simulation_values.variance );

         iterate();
      }

      if ( !job_queue.isEmpty() )
      {
         continue;
      }

      iterations = 1;
      DbgLv( 1 ) << " master loop-BOT: dssize" << data_sets.size() << "ds_to_p" << datasets_to_process << "curr_ds"
                 << current_dataset;

      if ( is_global_fit )
      {
         write_global();
      }
      else
      {
         write_output();
      }

      // Fit meniscus
      if ( ( menibott_ndx + 1 ) < menibott_count )
      {
         set_meniscus();
      }

      if ( !job_queue.isEmpty() )
      {
         continue;
      }

      // Monte Carlo
      if ( mc_iterations > 1 )
      { // Recompute final fit to get simulation and residual
         wksim_vals         = simulation_values;
         wksim_vals.solutes = calculated_solutes[max_depth];

         calc_residuals( current_dataset, datasets_to_process, wksim_vals );

         qDebug() << "Base-Sim RMSD" << sqrt( simulation_values.variance ) << "  Exp-Sim RMSD" << sqrt( wksim_vals.variance )
                  << "  of MC_Iteration" << mc_iteration + 1;
         max_iterations    = max_iters_all;
         simulation_values = wksim_vals;

         if ( pm_mc )
         { // Report a written iteration and get the next one
            if ( mc_iteration == 0 )
            { // Group 0 fit the unmodified data:  share its sigmas and
               //  base simulation with the groups waiting for them
               set_gaussians();
               pm_send_mc_base();
            }

            mc_iteration = pm_iter;
            if ( !pm_iteration_done( super, pm_iter ) )
            {
               break; // Group is done
            }

            mc_iteration = pm_iter - 1;
            set_monteCarlo();
            continue;
         }

         mc_iteration++;

         if ( mc_iteration < mc_iterations )
         {
            time_mc_iterations();
         }

         // Check again: a reduction for time may have ended the iterations
         if ( mc_iteration < mc_iterations )
         {
            set_monteCarlo();
         }
      }

      if ( !job_queue.isEmpty() )
      {
         continue;
      }

      if ( is_composite_job )
      { // Composite job:  update outputs in TAR and bump dataset count
         QString tripleID = QString( data_sets[current_dataset]->model.description ).section( ".", -3, -3 );
         dset_calc_solutes << calculated_solutes[max_depth];

         int next_dataset = current_dataset + 1;

         if ( pm_comp )
         {
            next_dataset = pm_dataset_done( super );
         }

         else
         {
            update_outputs();
         }

         if ( simulation_values.noisflag == 0 )
         {
            DbgLv( 0 ) << my_rank << ": Dataset" << current_dataset + 1 << "(" << tripleID << ")" << " :  model was output.";
         }
         else
         {
            DbgLv( 0 ) << my_rank << ": Dataset" << current_dataset + 1 << "(" << tripleID << ")"
                       << " :  model/noise(s) were output.";
         }

         DbgLv( 1 ) << " master loop-BOT:    nds kds" << next_dataset << count_datasets;
         if ( next_dataset < count_datasets )
         {
            wait_workers_ready();
            iterations = 1;
            start_2dsa_dataset( next_dataset, true );
            continue;
         }
      }

      break;
   }

   // Consume outstanding READY messages so none remains at MPI_Finalize
   wait_workers_ready();

   shutdown_all(); // All done
}

// Reset the fit state and queue the initial subgrids for a dataset.
//  workers_ready: no worker has a READY message outstanding (after
//  wait_workers_ready); at the start, workers still send their first READY.
void US_MPI_Analysis::start_2dsa_dataset( int dataset, bool workers_ready )
{
   current_dataset = dataset;
   menibott_ndx    = 0;
   meniscus_run    = 0;
   bottom_run      = 0;
   mc_iteration    = 0;
   sigmas.clear(); // Monte Carlo sigmas come from this dataset's base fit

   // Monte Carlo refines the fit to the unmodified data at least 5 times
   max_iterations = ( mc_iterations > 1 && max_iters_all < 2 ) ? 5 : max_iters_all;

   if ( menibott_count > 1 )
   { // Set the range of fit-meniscus/bottom points for this data set
      set_menibott_values( current_dataset );
      DbgLv( 0 ) << " start_2dsa_dataset:  menpt" << meniscus_points << "mv0 mvn" << meniscus_values[0]
                 << meniscus_values[meniscus_points - 1] << "botpt" << bottom_points << "bv0 bvn" << bottom_values[0]
                 << bottom_values[bottom_points - 1];
   }

   fill_queue();

   if ( workers_ready )
   { // Every worker is waiting for a job
      for ( int ii = 1; ii <= my_workers; ii++ )
      {
         worker_status[ii] = READY;
      }
   }

   for ( int ii = 0; ii < calculated_solutes.size(); ii++ )
   {
      calculated_solutes[ii].clear();
   }

   begin_fit_context();
}

// Progress message at the end of a 2DSA pass
QString US_MPI_Analysis::progress_2dsa( void )
{
   US_DataIO::EditedData* edata    = &data_sets[current_dataset]->run_data;
   QString                tripleID = edata->cell + edata->channel + edata->wavelength;
   QString                progress = "Iteration: " + QString::number( iterations );

   if ( datasets_to_process > 1 )
   {
      progress += "; Datasets: " + QString::number( datasets_to_process );
   }
   else
   {
      progress +=
         "; Dataset: " + QString::number( current_dataset + 1 ) + " (" + tripleID + ") of " + QString::number( count_datasets );
   }

   if ( mc_iterations > 1 )
   {
      progress += "; MonteCarlo: " + QString::number( mc_iteration + 1 );
   }

   else if ( fit_menbot )
   {
      progress += "; Meniscus: " + QString::number( meniscus_value, 'f', 4 ) +
                  "; Bottom: " + QString::number( bottom_value, 'f', 4 ) +
                  QString::asprintf( "  ( m%2d b%2d )", ( meniscus_run + 1 ), ( bottom_run + 1 ) );
   }
   else if ( fit_meni )
   {
      progress += "; Meniscus: " + QString::number( meniscus_value, 'f', 4 ) +
                  tr( " (%1 of %2)" ).arg( meniscus_run + 1 ).arg( meniscus_points );
   }
   else if ( fit_bott )
   {
      progress +=
         "; Bottom: " + QString::number( bottom_value, 'f', 4 ) + tr( " (%1 of %2)" ).arg( bottom_run + 1 ).arg( bottom_points );
   }
   else
   {
      progress += "; RMSD: " + QString::number( sqrt( simulation_values.variance ) );
   }

   return progress;
}

// Parallel masters: get the next Monte Carlo iteration or dataset index
//  from the supervisor.  Returns -1 if the supervisor has none left.
int US_MPI_Analysis::pm_next_unit( int super )
{
   int        iter;
   MPI_Status status;

   MPI_Recv( &iter, 1, MPI_INT, super, MPI_ANY_TAG, MPI_COMM_WORLD, &status );

   if ( status.MPI_TAG == STOPITER )
   {
      return -1;
   }

   if ( status.MPI_TAG == STARTLAST )
   { // This is the last unit for the group
      if ( is_composite_job )
      {
         count_datasets = iter + 1;
      }
      else
      {
         mc_iterations = iter;
      }
   }

   else if ( status.MPI_TAG != STARTITER )
   {
      DbgLv( 0 ) << "Unexpected tag in PMG 2DSA Master" << status.MPI_TAG;
   }

   return iter;
}

// Parallel Monte Carlo: report the written iteration (mc_iteration) to the
//  supervisor and get the next one.  Returns false if the group is done.
bool US_MPI_Analysis::pm_iteration_done( int super, int& pm_iter )
{
   // A reduction for time must be in the message, since the supervisor
   //  answers an iteration-done message with the next iteration
   time_mc_iterations();

   // Tell the supervisor that an iteration is done.  The supervisor keeps
   //  the group's memory use from the message that ends the group, which
   //  may also be a DONEITER answered with STOPITER.
   int iter = pm_group_rss();
   int tag  = ( mc_iteration < mc_iterations ) ? DONEITER : DONELAST;

   MPI_Send( &iter, 1, MPI_INT, super, tag, MPI_COMM_WORLD );

   if ( mc_iteration >= mc_iterations )
   {
      return false;
   }

   pm_iter = pm_next_unit( super );
   return ( pm_iter > 0 );
}

// Parallel Monte Carlo: send the sigmas and base simulation of group 0's fit
//  to the unmodified data to the other group masters.  The supervisor starts
//  group ii on iteration ii + 1, so groups 1 to mc_iterations - 1 wait for
//  them and the rest got STOPITER.
void US_MPI_Analysis::pm_send_mc_base( void )
{
   QVector<double> base = sigmas + mc_base_data;
   int             ngrp = qMin( mgroup_count, mc_iterations );

   for ( int gg = 1; gg < ngrp; gg++ )
   {
      MPI_Send( base.data(), base.size(), MPI_DOUBLE, gg * gcores_count, MCBASE, MPI_COMM_WORLD );
   }
}

// Parallel Monte Carlo: get the sigmas and base simulation from group 0
void US_MPI_Analysis::pm_recv_mc_base( void )
{
   int        count;
   MPI_Status status;

   MPI_Probe( 1, MCBASE, MPI_COMM_WORLD, &status ); // Group 0 master rank
   MPI_Get_count( &status, MPI_DOUBLE, &count );

   QVector<double> base( count );
   MPI_Recv( base.data(), count, MPI_DOUBLE, 1, MCBASE, MPI_COMM_WORLD, &status );

   sigmas       = base.mid( 0, count / 2 );
   mc_base_data = base.mid( count / 2 );
}

// Parallel masters: memory use of this group, the master's and its
//  workers' maximum RSS
int US_MPI_Analysis::pm_group_rss( void )
{
   long rss = max_rss();

   for ( int jj = 1; jj <= my_workers; jj++ )
   {
      rss += work_rss[jj];
   }

   return (int) rss;
}

// Parallel composite: report the finished dataset to the supervisor and
//  get the next one.  Returns count_datasets if the group is done.
int US_MPI_Analysis::pm_dataset_done( int super )
{
   // A reduction for time must be in the message, since the supervisor
   //  answers a dataset-done message with the next dataset
   time_datasets_left();

   int ittest = current_dataset + mgroup_count;

   // Tell the supervisor that a dataset is done
   int iter = pm_group_rss();
   int tag  = ( ittest < count_datasets ) ? DONEITER : DONELAST;

   MPI_Send( &iter, 1, MPI_INT, super, tag, MPI_COMM_WORLD );

   if ( ittest >= count_datasets )
   {
      return count_datasets;
   }

   if ( my_group == 0 )
   { // If group 0 master, create an intermediate archive
      update_outputs();
      DbgLv( 0 ) << my_rank << ": Dataset" << current_dataset + 1 << " : Intermediate archive was created.";
   }

   return pm_next_unit( super );
}

// Give queued jobs to ready workers and process worker messages until the
//  job queue is empty and no worker is busy
void US_MPI_Analysis::run_2dsa_jobs( void )
{
   while ( true )
   {
      // Give the jobs to the workers
      while ( !job_queue.isEmpty() && worker_status.contains( READY ) )
      {
         int    worker = ready_worker();

         Sa_Job job = job_queue.takeFirst();
         submit( job, worker );
         worker_depth[worker]  = job.mpi_job.depth;
         worker_seq[worker]    = job.seq;
         worker_status[worker] = WORKING;
      }

      // All done with the pass if no jobs are ready or running
      if ( job_queue.isEmpty() && !worker_status.contains( WORKING ) )
      {
         return;
      }

      // Wait for worker to send a message
      int        sizes[ 4 ];
      MPI_Status status;

      MPI_Recv( sizes,
                4,
                MPI_INT,
                MPI_ANY_SOURCE,
                MPI_ANY_TAG,
                my_communicator,
                &status);

      int worker = status.MPI_SOURCE;

      if ( max_depth > 0 )
      {
         DbgLv( 1 ) << " master loop-BOTTOM:   status TAG" << status.MPI_TAG << MPI_Job::READY << MPI_Job::RESULTS << "  source"
                    << status.MPI_SOURCE;
      }
      switch( status.MPI_TAG )
      {
         case MPI_Job::READY:   // Ready for work
            worker_status[ worker ] = READY;
            work_rss[worker]        = sizes[3];
            break;

         case MPI_Job::RESULTS: // Return solute data
            process_results( worker, sizes );
            work_rss[ worker ] = sizes[ 3 ];
            break;

         default:  // Should never happen
            QString msg =  "Master 2DSA:  Received invalid status " +
                           QString::number( status.MPI_TAG );
            abort( msg );
            break;
      }

      max_rss();
   }
}

// Wait for the READY message of every worker not yet marked ready.  Each
//  worker sends READY after each result, so after a pass at most one is
//  outstanding per worker.
void US_MPI_Analysis::wait_workers_ready( void )
{
   for ( int ii = 1; ii <= my_workers; ii++ )
   {
      if ( worker_status[ii] != READY )
      {
         int        sizes[4];
         MPI_Status status;

         MPI_Recv( sizes, 4, MPI_INT, ii, MPI_Job::READY, my_communicator, &status );

         worker_status[ii] = READY;
         work_rss[ii]      = sizes[3];
      }
   }
}

// Generate the initial set of solutes
void US_MPI_Analysis::init_solutes( void )
{
   calculated_solutes.clear();
   orig_solutes.clear();
   simulation_values.noisflag    = ( parameters[ "tinoise_option" ].toInt() > 0 ?
                                     1 : 0 )
                                 + ( parameters[ "rinoise_option" ].toInt() > 0 ?
                                     2 : 0 );
   simulation_values.dbg_level   = dbg_level;
   simulation_values.dbg_timing  = dbg_timing;
DbgLv(0) << "DEBUG_LEVEL" << simulation_values.dbg_level;

   // Test to see if there is a custom grid model
   QString model_filename = data_sets[ 0 ]->model_file;
   double  cnstff0        = 0.0;

   if ( model_filename.isEmpty() )
   {  // If no model file given, calculate initial solutes in a fixed grid

      double s_min   = parameters[ "s_min"           ].toDouble() * 1.0e-13;
      double s_max   = parameters[ "s_max"           ].toDouble() * 1.0e-13;
      double ff0_min = parameters[ "ff0_min"         ].toDouble();
      double ff0_max = parameters[ "ff0_max"         ].toDouble();

      int grid_reps  = qMax( parameters[ "uniform_grid" ].toInt(), 1 );
      double s_pts   = 60.0;
      double ff0_pts = 60.0;

      if ( parameters.contains( "s_grid_points"   ) )
         s_pts   = parameters[ "s_grid_points"   ].toDouble();

      else if ( parameters.contains( "s_resolution"    ) )
         s_pts   = parameters[ "s_resolution"    ].toDouble() * grid_reps;

      if ( parameters.contains( "ff0_grid_points" ) )
         ff0_pts = parameters[ "ff0_grid_points" ].toDouble();

      else if ( parameters.contains( "ff0_resolution"  ) )
         ff0_pts = parameters[ "ff0_resolution"  ].toDouble() * grid_reps;

      int    nsstep   = (int)( s_pts );
      int    nkstep   = (int)( ff0_pts );

DbgLv(0) << "InSol: nss nks" << s_pts << ff0_pts << nsstep << nkstep << "grid_reps" << grid_reps;
      US_Solute::init_solutes( s_min,   s_max,   nsstep,
                               ff0_min, ff0_max, nkstep,
                               grid_reps, cnstff0, orig_solutes );
DbgLv(0) << "InSol:  s range" << s_min*1.e+13 << s_max*1.e+13 << "k range"
 << ff0_min << ff0_max;
int j0=orig_solutes.count()-1;
int j1=orig_solutes[0].count()-1;
int j2=orig_solutes[j0].count()-1;
DbgLv(0) << "orig_solutes:"
 << orig_solutes[ 0][ 0].s*1.e+13 << orig_solutes[ 0][ 0].k << "  "
 << orig_solutes[ 0][j1].s*1.e+13 << orig_solutes[ 0][j1].k << "  "
 << orig_solutes[j0][ 0].s*1.e+13 << orig_solutes[j0][ 0].k << "  "
 << orig_solutes[j0][j2].s*1.e+13 << orig_solutes[j0][j2].k;
   }

   else
   {  // If a model file was given, use it to set the initial solutes
      US_Model model;
      QString fn = "../" + model_filename;
      model.load( fn );
      int    nsubgrid = model.subGrids;
      int    ncomps   = model.components.size();
DbgLv(0) << "InSol: fn" << fn;
DbgLv(0) << "InSol:  nsubgrid ncomps" << nsubgrid << ncomps;

      if ( nsubgrid < 1 )
         abort( "Custom grid model file has no subgrids", -1 );

      if ( ( ncomps / nsubgrid ) > 150 )
      {  // Subgrids too large:  adjust subgrid count and size
         nsubgrid       = ( ncomps / 100 + 1 ) | 1;
         model.subGrids = nsubgrid;
DbgLv(0) << "InSol:  nsubgrid sbsize" << nsubgrid << ( ncomps / nsubgrid );
      }

      QVector< US_Solute > solvec;

      for ( int ii = 0; ii < nsubgrid; ii++ )
      {
         solvec.clear();

         for ( int jj = ii; jj < ncomps; jj += nsubgrid )
         {
            double ffval    = model.components[ jj ].f_f0;
            double vbval    = model.components[ jj ].vbar20;

            // Save each solute contained in the custom grid model
            US_Solute soli( model.components[ jj ].s,
                            ffval,
                            0.0,
                            vbval,
                            model.components[ jj ].D );
            solvec << soli;
         }

         orig_solutes << solvec;
      }
   }
}

// Fill the job queue, using the list of initial solutes
void US_MPI_Analysis::fill_queue( void )
{
   worker_status.resize( gcores_count );
   worker_depth .resize( gcores_count );
   worker_seq.resize( gcores_count );

   worker_status.fill( INIT );
   worker_depth .fill( 0 );
   worker_seq.fill( 0 );
   max_depth           = 0;
   worknext            = 1;
   max_experiment_size = min_experiment_size;

   // Put all jobs in the queue
   job_queue.clear();
   cached_results.clear();

   for ( int i = 0; i < orig_solutes.size(); i++ )
   {
      max_experiment_size = qMax( max_experiment_size,
                                  orig_solutes[ i ].size() );
      Sa_Job job;
      job.solutes         = orig_solutes[ i ];
      add_to_queue( job );
   }
}

//////////////////
void US_MPI_Analysis::global_fit( void )
{
   // To do a global fit across multiple data sets:
   //  1. Each individual data set must be run
   //  2. Sum the total concentration of all returned solutes
   //  3. Divide all experiment concentrations by the total concentration
   //  4. Send the concentration-equalized data to the workers
   //  5. Do an additional run against the combined datasets for the baseline
   // Any additional Monte Carlo iterations will use the adjusted data for
   // all data sets.

   double concentration = 0.0;
   US_Model::AnalysisType mdl_type = model_type( analysis_type );

   // The first dataset is done automatically.
   if ( mdl_type != US_Model::PCSA )
   {
      for ( int solute = 0; solute < simulation_values.solutes.size(); solute++ )
      {
         concentration += simulation_values.solutes[ solute ].c;
      }
DbgLv(1) << ":gf: 2DSA: nsols" << simulation_values.solutes.size() << "concen" << concentration;
   }
   else
   {
      for ( int solute = 0; solute < simulation_values.zsolutes.size(); solute++ )
      {
         concentration += simulation_values.zsolutes[ solute ].c;
      }
DbgLv(1) << ":gf: PCSA: nzsols" << simulation_values.zsolutes.size() << "concen" << concentration;
   }

   qDebug() << "   == Dataset" << current_dataset + 1
            << "Total Concentration" << concentration << "==";

   // Point to current dataset
   US_DataIO::EditedData* edata = &data_sets[ current_dataset ]->run_data;

   concentrations[ current_dataset ] = concentration;
   edata->ODlimit   /= concentration;
   int scan_count    = edata->scanCount();
   int radius_points = edata->pointCount();
int ks=scan_count;
int kp=radius_points;
DbgLv(1) << ":gf:  ee" << current_dataset << "BB-dset(m)" << edata->value(ks/2,kp/2);
   int index         = 0;
   QVector< double > scaled_data( scan_count * radius_points  + 1 );
double isum=0.0;
double dsum=0.0;

   // Scale the data
   for ( int ss = 0; ss < scan_count; ss++ )
   {
      for ( int rr = 0; rr < radius_points; rr++ )
      {
double ival=edata->value(ss,rr);
isum+=ival;
         double scaled_value    = edata->value( ss, rr ) / concentration;
         scaled_data[ index++ ] = scaled_value;
         edata->setValue( ss, rr, scaled_value );
dsum+=scaled_value;
      }
   }

   scaled_data[ index ] = edata->ODlimit;
DbgLv(0) << "ScaledData sum" << dsum << "iSum" << isum << "concen" << concentration;

   // Send the scaled version of current data to the workers
   MPI_Job job;
   job.command         = MPI_Job::NEWDATA;
   job.length          = scaled_data.size();
   job.solution        = 1;
   job.meniscus_value  = data_sets[ current_dataset ]->run_data.meniscus;
   job.dataset_offset  = current_dataset;
   job.dataset_count   = 1;

   // Tell each worker that new data is coming.
   // Cannot use a broadcast, since the worker is expecting a Send.
   for ( int worker = 1; worker <= my_workers; worker++ )
   {
      job.solution        = worker;
      job.depth           = worker;
      MPI_Send( &job,
                sizeof( MPI_Job ),
                MPI_BYTE,
                worker,
                MPI_Job::MASTER,
                my_communicator );
   }

   // Get everybody synced up
   MPI_Barrier( my_communicator );

   MPI_Bcast( scaled_data.data(),
              scaled_data.size(),
              MPI_DOUBLE,
              MPI_Job::MASTER,
              my_communicator );

   // Go to the next dataset
   job_queue.clear();
   if ( mdl_type != US_Model::PCSA )
      dset_calc_solutes << calculated_solutes[ max_depth ];
   current_dataset++;

   if ( current_dataset >= count_datasets )
   {  // If all datasets have been scaled, do all datasets from now on
      datasets_to_process = count_datasets;
      current_dataset     = 0;
   }

   else
   {
      for ( int ii = 1; ii < gcores_count; ii++ )
         worker_status[ ii ] = READY;
   }

   if ( mdl_type != US_Model::PCSA )
   {
      fill_queue();

      for ( int ii = 0; ii < calculated_solutes.size(); ii++ )
         calculated_solutes[ ii ].clear();
   }

   else
   {
      fill_pcsa_queue();

      for ( int ii = 0; ii < calculated_zsolutes.size(); ii++ )
         calculated_zsolutes[ ii ].clear();
DbgLv(1) << "ScaledData fill/solclear complete";

      iterations      = 1;
      max_iterations  = parameters[ "gfit_iterations" ].toInt();
   }
}

// Start a fit context: every change of dataset, meniscus/bottom point or
//  Monte Carlo iteration passes here before its jobs are submitted.  Sets
//  the meniscus and bottom that the jobs carry to the workers.
void US_MPI_Analysis::begin_fit_context( void )
{
   meniscus_value = ( meniscus_points == 1 ) ? data_sets[current_dataset]->run_data.meniscus : meniscus_values[meniscus_run];
   bottom_value   = ( bottom_points == 1 ) ? dataset_bottom( current_dataset ) : bottom_values[bottom_run];

   bool sim_changed =
      ( fit_context.dataset != current_dataset || fit_context.meniscus != meniscus_value || fit_context.bottom != bottom_value );

   fit_context.dataset      = current_dataset;
   fit_context.meniscus     = meniscus_value;
   fit_context.bottom       = bottom_value;
   fit_context.mc_iteration = mc_iteration;

   DbgLv( 1 ) << "FitContext: ds" << current_dataset << "men" << meniscus_value << "bot" << bottom_value << "mc" << mc_iteration
              << "sim_changed" << sim_changed;
}

// Bottom of a dataset: the edited bottom if given, else the centerpiece
//  bottom stretched at the rotor speed
double US_MPI_Analysis::dataset_bottom( int dataset )
{
   US_SolveSim::DataSet* ds = data_sets[dataset];

   if ( ds->run_data.bottom > 0.0 )
   {
      return ds->run_data.bottom;
   }

   return US_AstfemMath::calc_bottom( ds->run_data.scanData[0].rpm, ds->centerpiece_bottom, ds->rotor_stretch );
}

// Set the fit-meniscus and fit-bottom points around a dataset's edited
//  meniscus and bottom, keeping the meniscus range below the data range
void US_MPI_Analysis::set_menibott_values( int dataset )
{
   US_DataIO::EditedData* edata   = &data_sets[dataset]->run_data;
   double                 dat_str = edata->radius( 0 );
   double                 men_dpt = ( meniscus_points > 1 ) ? (double) ( meniscus_points - 1 ) : 1;
   double                 bot_dpt = ( bottom_points > 1 ) ? (double) ( bottom_points - 1 ) : 1;
   double                 men_str = edata->meniscus - ( meniscus_range * 0.5 );
   double                 bot_str = dataset_bottom( dataset ) - ( bottom_range * 0.5 );
   double                 men_inc = meniscus_range / men_dpt;
   double                 bot_inc = bottom_range / bot_dpt;
   double                 men_end = men_str + meniscus_range - men_inc;

   if ( men_end >= dat_str )
   { // Adjust first meniscus so range remains below data range
      men_end = dat_str - ( men_inc * 0.5 );
      men_str = men_end - meniscus_range + men_inc;
   }

   for ( int ii = 0; ii < meniscus_points; ii++ )
   {
      meniscus_values[ii] = men_str + men_inc * ii;
   }

   for ( int ii = 0; ii < bottom_points; ii++ )
   {
      bottom_values[ii] = bot_str + bot_inc * ii;
   }
}

// Reset for a fit-meniscus iteration
void US_MPI_Analysis::set_meniscus( void )
{
   menibott_ndx++;

   if ( fit_menbot )
   {
      meniscus_run   = menibott_ndx / meniscus_points;
      bottom_run     = menibott_ndx % meniscus_points;
   }
   else if ( fit_meni )
   {
      meniscus_run   = menibott_ndx;
   }
   else if ( fit_bott )
   {
      bottom_run     = menibott_ndx;
   }
   else
   {
DbgLv(0) << "FMB:set_meniscus: HUH????: mb m b:"
 << fit_menbot << fit_meni << fit_bott;
   }
DbgLv(0) << "FMB:set_meniscus:  mb_ndx men_run bot_run"
 << menibott_ndx << meniscus_run << bottom_run << "mb m b mbc"
 << fit_menbot << fit_meni << fit_bott << menibott_count;

   // We incremented meniscus_run above.  Just rerun from the beginning.
   for ( int i = 0; i < orig_solutes.size(); i++ )
   {
      Sa_Job job;
      job.solutes = orig_solutes[ i ];

      add_to_queue( job );
   }

   worker_depth.fill( 0 );
   max_depth = 0;
   for ( int ii = 0; ii < calculated_solutes.size(); ii++ )
      calculated_solutes[ ii ].clear();

   begin_fit_context();
}

// Reset for a Monte Carlo iteration
void US_MPI_Analysis::set_monteCarlo( void )
{
   // Workers answer the new data with READY; consume the READY each worker
   //  sent after its last result first, so no stale READY can later mark a
   //  busy worker as ready
   wait_workers_ready();

   DbgLv( 1 ) << "sMC: max_depth" << max_depth << "calcsols size" << calculated_solutes[max_depth].size() << "simvsols size"
              << simulation_values.solutes.size();

   // Set up new data modified by a gaussian distribution.  The sigmas come
   //  from the fit to the unmodified data, which this process just did
   //  unless it got them from parallel-masters group 0.
   if ( sigmas.isEmpty() )
   {
      set_gaussians();
   }

   US_DataIO::EditedData* edata = &data_sets[ current_dataset ]->run_data;
   int ds_points     = total_points;
   int ds_start      = 0;
   int ds_end        = count_datasets;

   if ( is_composite_job )
   {
      ds_points         = edata->scanCount() * edata->pointCount();
      ds_start          = current_dataset;
      ds_end            = ds_start + datasets_to_process;
   }
   mc_data.resize( ds_points );
   DbgLv( 1 ) << "sMC: totpts" << total_points << "mc_iter" << mc_iteration;
   DbgLv( 1 ) << "sMC:  sig-size" << sigmas.count() << "mcd-size" << mc_data.count() << "base-size" << mc_base_data.count();
   int    sigsiz  = sigmas.count();
   int    mcdsiz  = mc_data.count();
   int    index   = 0;
   double varrmsd = 0.0;
   double varisum = 0.0;
   double varimin = 1.0;
   double varimax = -1.0;
   double datasum = 0.0;

   // With a fixed seed, seed each iteration from the seed, dataset and
   //  iteration index, so its data do not depend on which master group
   //  computes it, and differ between datasets and nearby seeds
   uint   seed = parameters["seed"].toUInt();
   if ( seed != 0 )
   {
      US_Math2::randomize( mc_seed( seed, current_dataset, mc_iteration ) );
   }

   // Get a randomized variation of the concentrations
   // Use a gaussian distribution with the residual as the standard deviation
   for ( int ee = ds_start; ee < ds_end; ee++ )
   {
      edata             = &data_sets[ ee ]->run_data;
      int scan_count    = edata->scanCount();
      int radius_points = edata->pointCount();
      int indxh         = ( ( scan_count / 2 ) * radius_points ) + ( radius_points / 2 );

      for ( int ss = 0; ss < scan_count; ss++ )
      {
         for ( int rr = 0; rr < radius_points; rr++ )
         {
            if ( index >= sigsiz )
            {
               DbgLv( 1 ) << "sMC:  *ERR* index" << index << "sigsiz" << sigsiz << "ee,ss,rr" << ee << ss << rr;
            }
            if ( index >= mcdsiz )
            {
               DbgLv( 1 ) << "sMC:  *ERR* index" << index << "mcdsiz" << mcdsiz << "ee,ss,rr" << ee << ss << rr;
            }
            double variation = US_Math2::box_muller( 0.0, sigmas[index] );
            double mcdata    = mc_base_data[index] + variation;
            varrmsd += sq( variation );
            varisum += variation;
            varimin = qMin( varimin, variation );
            varimax = qMax( varimax, variation );
            datasum += mcdata;

            if ( index < 5 || index > ( total_points - 6 ) || ( index > ( indxh - 4 ) && index < ( indxh + 3 ) ) )
            {
               DbgLv( 1 ) << "sMC:  index" << index << "sdat" << mc_base_data[index] << "sigma" << sigmas[index] << "vari"
                          << variation << "mdat" << mcdata;
            }

            mc_data[index++] = mcdata;
         }
      }
   }
DbgLv(1) << "sMC:   mcdata sum" << datasum;

   varrmsd          = sqrt( varrmsd / (double)( ds_points ) );
   qDebug() << "  Box_Muller Variation RMSD"
            << QString::number( varrmsd, 'f', 7 )
            << "  for MC_Iteration" << mc_iteration + 1;

DbgLv(1) << "sMC:   variation  sum min max" << varisum << varimin << varimax
 << "mcdata sum" << datasum;

   // Broadcast Monte Carlo data to all workers
   MPI_Job newdata;
   newdata.command        = MPI_Job::NEWDATA;
   newdata.length         = ds_points;
   newdata.solution       = mc_iteration + 1;
   newdata.meniscus_value = data_sets[ 0 ]->run_data.meniscus;
   newdata.dataset_offset = ds_start;
   newdata.dataset_count  = ds_end - ds_start;

   // Tell each worker that new data coming
   // Can't use a broadcast because the worker is expecting a Send
DbgLv(1) << "sMC: MPI send   my_workers" << my_workers;
   for ( int worker = 1; worker <= my_workers; worker++ )
   {
      MPI_Send( &newdata,
          sizeof( MPI_Job ),
          MPI_BYTE,
          worker,
          MPI_Job::MASTER,
          my_communicator );
   }

   // Get everybody synced up
DbgLv(1) << "sMC: MPI Barrier";
   MPI_Barrier( my_communicator );

DbgLv(1) << "sMC: MPI Bcast";
   MPI_Bcast( mc_data.data(),
              ds_points,
              MPI_DOUBLE,
              MPI_Job::MASTER,
              my_communicator );

   fill_queue();

   for ( int ii = 0; ii < calculated_solutes.size(); ii++ )
      calculated_solutes[ ii ].clear();

   begin_fit_context();
}

//////////////////
//  Generate the simulated data and calculate the residuals
//  Use the residuals as the standard deviation for varying the
//  data in Monte Carlo iterations
void US_MPI_Analysis::set_gaussians( void )
{
   US_DataIO::EditedData* edata = &data_sets[ current_dataset ]->run_data;
   int ds_start      = 0;
   int ds_end        = count_datasets;
   int ds_count      = count_datasets;

   if ( is_composite_job )
   {
      ds_start          = current_dataset;
      ds_end            = ds_start + datasets_to_process;
      ds_count          = datasets_to_process;
   }
DbgLv(1) << "sGA: calcsols size mxdpth" << calculated_solutes.size() << max_depth;

   simulation_values.solutes = calculated_solutes[ max_depth ];

int mm=simulation_values.solutes.size()-1;
DbgLv(1) << "sGA:   sol0.s solM.s" << simulation_values.solutes[0].s
 << simulation_values.solutes[mm].s << "  M=" << mm;;
DbgLv(1) << "sGA:     solM.k" << simulation_values.solutes[mm].k;
DbgLv(1) << "sGA:     solM.c" << simulation_values.solutes[mm].c;
edata = &data_sets[ds_start]->run_data;
DbgLv(1) << "sGA:    edata scans points" << edata->scanCount() << edata->pointCount();

   calc_residuals( ds_start, ds_count, simulation_values );

   sigmas.clear();
   mc_base_data.clear();
   res_data          = &simulation_values.residuals;
   US_DataIO::RawData* sim_data      = &simulation_values.sim_data;
   int scnx          = 0;
DbgLv(1) << "sGA:  resids scans points" << res_data->scanCount() << res_data->pointCount();

   for ( int ee = ds_start; ee < ds_end; ee++ )
   {
      edata             = &data_sets[ ee ]->run_data;
      int scan_count    = edata->scanCount();
      int radius_points = edata->pointCount();
      DbgLv( 1 ) << "sGA:   ee" << ee << "scans points" << scan_count << radius_points;

      // Place the residuals magnitudes and the simulation into single
      //  vectors, indexed like the Monte Carlo data
      for ( int ss = 0; ss < scan_count; ss++, scnx++ )
      {
         for ( int rr = 0; rr < radius_points; rr++ )
         {
            sigmas << qAbs( res_data->value( scnx, rr ) );
            mc_base_data << sim_data->value( scnx, rr );
         }
      }
   }
int ssz=sigmas.size();
DbgLv(1) << "sGA:   sigmas size" << ssz << "sigmas[mm]" << sigmas[ssz/2];

qDebug() << "  MC_Iteration 1 Simulation RMSD" << QString::number( sqrt( simulation_values.variance ), 'f', 7 );
}

// Reset for a refinement iteration
void US_MPI_Analysis::iterate( void )
{
   // Just return if the number of iterations exceeds the max
   // or if the last two iterations converged and are essentially identical
   if ( ++iterations > max_iterations )
   {
      qDebug() << "++ Maximum refinement iterations has been reached:" << max_iterations;
      return;
   }

   double diff  = simulation_values.variance - previous_values.variance;
   double adiff = qAbs( diff );
   bool   ssame = false;
   int hiters   = max_iterations / 2;

   if ( iterations > 2 )
   {
      if ( diff > 0.0   &&  iterations > hiters )
      {  // Variance is getting worse
         iterations  -= 2;
         simulation_values = previous_values;
         simulation_values.solutes   = previous_values.solutes;
         simulation_values.variances = previous_values.variances;
         simulation_values.ti_noise  = previous_values.ti_noise;
         simulation_values.ri_noise  = previous_values.ri_noise;
         qDebug() << "++ Refinement iteration degrading" << diff
                  << ". Reverting to iteration" << iterations;
         return;
      }

      if ( adiff < min_variance_improvement )
      {
         qDebug() << "++ Refinement iteration improvement near-zero:" << adiff;
         return;
      }

      int    nsols = previous_values.solutes.size();

      if ( nsols == simulation_values.solutes.size() )
      {
         ssame        = true;

         for ( int jj = 0; jj < nsols; jj++ )
         {
            if ( previous_values.solutes[ jj ] != simulation_values.solutes[ jj ] )
            {  // Mismatch:  may need to iterate
               ssame        = false;
               break;
            }

            else
            {  // Test if concentrations are almost the same
               double pconc = previous_values.solutes[ jj ].c;
               double cconc = simulation_values.solutes[ jj ].c;
               double cdiff = qAbs( ( pconc - cconc ) / pconc );

               if ( cdiff > 1.0e-1 )  // Greater than 10% concentration diff?
               {  // Significant concentration mismatch:  may need to iterate
                  ssame        = false;
                  break;
               }
            }
         }
      }
   }

   if ( ssame )
   { // Solutes same as previous:  no more iterations
      qDebug() << "++ Refinement iteration solutes same as previous:"
               << previous_values.solutes.size()
               << simulation_values.solutes.size();
      return;
   }

   // Save the most recent simulation values for the next time
   previous_values.variance  = simulation_values.variance;
   previous_values.solutes   = simulation_values.solutes;
   previous_values.variances = simulation_values.variances;
   previous_values.ti_noise  = simulation_values.ti_noise;
   previous_values.ri_noise  = simulation_values.ri_noise;

   // Set up for another round at depth 0
   Sa_Job job;
   job.mpi_job.dataset_offset = current_dataset;
   job.mpi_job.dataset_count  = datasets_to_process;
   job.mpi_job.meniscus_value = meniscus_value;
   job.mpi_job.bottom_value   = bottom_value;
   max_experiment_size        = min_experiment_size;

   QVector< US_Solute > prev_solutes = simulation_values.solutes;

   for ( int i = 0; i < orig_solutes.size(); i++ )
   {
      job.solutes = orig_solutes[ i ];

      // Add back all non-zero Solutes to each job
      // Ensure there are no duplicates
      for ( int s = 0; s < prev_solutes.size(); s++ )
      {
         if ( ! job.solutes.contains( prev_solutes[ s ] ) )
         {
            job.solutes << prev_solutes[ s ];
         }
      }

      add_to_queue( job );

      // Bump max solutes per subgrid to new observed max
      max_experiment_size = qMax( max_experiment_size,
                                  job.solutes.size() );
   }

   // Clear depth and calculated-solutes lists
   worker_depth.fill( 0 );
   max_depth = 0;
   for ( int ii = 0; ii < calculated_solutes.size(); ii++ )
      calculated_solutes[ ii ].clear();

   return;
}

// Submit a queued job
void US_MPI_Analysis::submit( Sa_Job& job, int worker )
{
   job.mpi_job.command        = MPI_Job::PROCESS;
   job.mpi_job.length         = job.solutes.size();
   job.mpi_job.meniscus_value = meniscus_value;
   job.mpi_job.bottom_value   = bottom_value;
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

// Add a job to the queue, maintaining depth order
void US_MPI_Analysis::add_to_queue( Sa_Job& job )
{
   int jdepth = job.mpi_job.depth;
   job.seq    = job_seq_next++; // The queue is in (depth, seq) order

   for ( int qq = 0; qq < job_queue.size(); qq++ )
   {
      if ( jdepth < job_queue[ qq ].mpi_job.depth )
      { // Insert this job before any with a greater depth
         job_queue.insert( qq, job );
         return;
      }
   }

   // In most circumstances, we just append the job to the end of the queue
   job_queue << job;
   return;
}

// Process the results from a just-completed worker task
void US_MPI_Analysis::process_results( int        worker,
                                       const int* size )
{
   simulation_values.solutes.resize( size[ 0 ] );
   simulation_values.variances.resize( datasets_to_process );
   simulation_values.ti_noise.resize( size[ 1 ] );
   simulation_values.ri_noise.resize( size[ 2 ] );

   MPI_Status status;

   // Get all simulation_values
   MPI_Recv( simulation_values.solutes.data(),
             size[ 0 ] * solute_doubles,
             MPI_DOUBLE,
             worker,
             MPI_Job::TAG0,
             my_communicator,
             &status );

   MPI_Recv( &simulation_values.variance,
             1,
             MPI_DOUBLE,
             worker,
             MPI_Job::TAG0,
             my_communicator,
             &status );

   MPI_Recv( simulation_values.variances.data(),
             datasets_to_process,
             MPI_DOUBLE,
             worker,
             MPI_Job::TAG0,
             my_communicator,
             &status );

   MPI_Recv( simulation_values.ti_noise.data(),
             size[ 1 ],
             MPI_DOUBLE,
             worker,
             MPI_Job::TAG0,
             my_communicator,
             &status );

   MPI_Recv( simulation_values.ri_noise.data(),
             size[ 2 ],
             MPI_DOUBLE,
             worker,
             MPI_Job::TAG0,
             my_communicator,
             &status );

   worker_status[ worker ] = INIT;

   Result result;
   result.depth     = worker_depth[worker];
   result.worker    = worker;
   result.seq       = worker_seq[worker];
   result.solutes   = simulation_values.solutes;
   result.variance  = simulation_values.variance;
   result.variances = simulation_values.variances;
   result.ti_noise  = simulation_values.ti_noise;
   result.ri_noise  = simulation_values.ri_noise;

   DbgLv( 1 ) << "Mast:  process_results: worker" << worker << "depth" << result.depth << "seq" << result.seq << "solutes"
              << result.solutes.size();
   cached_results << result;

   process_ready_results();
}

// Process cached results in queue order.  A result is processed only when
//  no queued or running job comes before it, so the merging of subgrid
//  results does not depend on the order in which workers finish.
void US_MPI_Analysis::process_ready_results( void )
{
   while ( !cached_results.isEmpty() )
   {
      // Find the first result in (depth, seq) order
      int rx = 0;

      for ( int ii = 1; ii < cached_results.size(); ii++ )
      {
         const Result& rr = cached_results[ii];
         const Result& rb = cached_results[rx];

         if ( rr.depth < rb.depth || ( rr.depth == rb.depth && rr.seq < rb.seq ) )
         {
            rx = ii;
         }
      }

      if ( job_pending_before( cached_results[rx].depth, cached_results[rx].seq ) )
      {
         break;
      }

      Result result = cached_results.takeAt( rx );

      simulation_values.solutes   = result.solutes;
      simulation_values.variance  = result.variance;
      simulation_values.variances = result.variances;
      simulation_values.ti_noise  = result.ti_noise;
      simulation_values.ri_noise  = result.ri_noise;
      max_experiment_size         = qMax( max_experiment_size, (int) result.solutes.size() );

      process_solutes( result.depth, result.worker, result.solutes );
   }
}

// Is a queued or running job ahead of (depth, seq)?
bool US_MPI_Analysis::job_pending_before( int depth, int seq )
{
   for ( int qq = 0; qq < job_queue.size(); qq++ )
   {
      int qdepth = job_queue[qq].mpi_job.depth;

      if ( qdepth < depth || ( qdepth == depth && job_queue[qq].seq < seq ) )
      {
         return true;
      }
   }

   for ( int ww = 1; ww <= my_workers; ww++ )
   {
      if ( worker_status[ww] != WORKING )
      {
         continue;
      }

      int wdepth = worker_depth[ww];

      if ( wdepth < depth || ( wdepth == depth && worker_seq[ww] < seq ) )
      {
         return true;
      }
   }

   return false;
}

// Is any job at this depth or below queued, running or not yet processed?
bool US_MPI_Analysis::job_pending_at( int depth )
{
   for ( int qq = 0; qq < job_queue.size(); qq++ )
   {
      if ( job_queue[qq].mpi_job.depth <= depth )
      {
         return true;
      }
   }

   for ( int ww = 1; ww <= my_workers; ww++ )
   {
      if ( worker_status[ww] == WORKING && worker_depth[ww] <= depth )
      {
         return true;
      }
   }

   for ( int ii = 0; ii < cached_results.size(); ii++ )
   {
      if ( cached_results[ii].depth <= depth )
      {
         return true;
      }
   }

   return false;
}

// Process the calculated solute vector from a job result
void US_MPI_Analysis::process_solutes( int& depth, int& worker,
                                       QVector< US_Solute >& result_solutes )
{
DbgLv(1) << "Mast:    process_solutes:      worker" << worker
 << " solsize" << result_solutes.size() << "depth" << depth;
   int next_depth  = depth + 1;
   // This loop should only execute, at most, once per result.
   while ( calculated_solutes.size() < next_depth )
      calculated_solutes << QVector< US_Solute >();

   // How big will our vector be?
   int csol_size  = calculated_solutes[ depth ].size();
   int rsol_size  = result_solutes.size();
   int new_size   = csol_size + rsol_size;

   // Submit with previous solutes if new size would be too big
   if ( new_size > max_experiment_size )
   {
      // Put current solutes on queue at depth + 1
      Sa_Job job;
      job.solutes                = calculated_solutes[ depth ];
      job.mpi_job.depth          = next_depth;
      job.mpi_job.dataset_offset = current_dataset;
      job.mpi_job.dataset_count  = datasets_to_process;
      std::sort( job.solutes.begin(), job.solutes.end() );
      unique_solutes( job.solutes );
      add_to_queue( job );

DbgLv(1) << "Mast:   queue NEW DEPTH sols" << job.solutes.size() << " d="
 << job.mpi_job.depth << " newsz mxesz" << new_size << max_experiment_size;
      max_depth           = qMax( next_depth, max_depth );
      calculated_solutes[ depth ].clear();
   }

   new_size            = rsol_size * 2;

   if ( next_depth > 1  &&  new_size > max_experiment_size )
   { // Adjust max_experiment_size if it is only large enough for one output
      max_experiment_size = ( new_size * 11 + 9 ) / 10;  // 10% above
DbgLv(1) << "Mast:    NEW max_exp_size" << max_experiment_size
 << "from new_size rsol_size" << new_size << rsol_size;
   }

   // Add in the current results
   calculated_solutes[ depth ] += result_solutes;
   csol_size  = calculated_solutes[ depth ].size();

   // At this point we need to clean up,  For each depth
   // below the current one, if there is nothing in the queue or working
   // for that depth or below and there are calculated solutes left, those
   // tasks need to be submitted.

   int dcheck = depth;
   if ( depth == 0  &&  max_depth > 0 )  dcheck = 1;

   for ( int d = 0; d < dcheck; d++ )
   {
      int remainder = calculated_solutes[ d ].size();

      if ( !job_pending_at( d ) && remainder > 0 )
      { // Submit a job with remaining calculated solutes from an earlier depth
         int next_d                 = d + 1;
         Sa_Job job;
         job.solutes                = calculated_solutes[ d ];
         job.mpi_job.depth          = next_d;
         job.mpi_job.dataset_offset = current_dataset;
         job.mpi_job.dataset_count  = datasets_to_process;
         max_depth                  = qMax( next_d, max_depth );
         std::sort( job.solutes.begin(), job.solutes.end() );
         unique_solutes( job.solutes );
         add_to_queue( job );
DbgLv(1) << "Mast:   queue REMAINDER" << remainder << " d=" << d+1;

         calculated_solutes[ d ].clear();
      }
   }

   // Submit one last time with all solutes if necessary.
   // This is the case if
   //  (1) this result is from the maximum submitted-jobs depth;
   //  (2) no jobs are queued or working; and
   //  (3) the current set of calculated solutes comes from more
   //      than one task result.
   if ( depth == max_depth && !job_pending_at( 99 ) && csol_size > rsol_size )
   {
      Sa_Job job;
      job.solutes          = calculated_solutes[ depth ];
      job.mpi_job.depth    = next_depth;
      std::sort( job.solutes.begin(), job.solutes.end() );
      unique_solutes( job.solutes );
      DbgLv( 1 ) << "Mast:   queue LAST ns=" << job.solutes.size() << "  d=" << depth + 1 << max_depth
                 << "  nsvs=" << simulation_values.solutes.size();

      calculated_solutes[ depth ].clear();
      csol_size = 0;
      max_depth = next_depth;
      worker    = ready_worker();

      if ( worker > 0 )
      { // Submit what should be the last job of this iteration
         ljob_solutes            = job.solutes;
         job.seq                 = job_seq_next++;
         submit( job, worker );
         worker_depth [ worker ] = job.mpi_job.depth;
         worker_seq[worker]      = job.seq;
         worker_status[ worker ] = WORKING;
         // Insure calculated solutes is empty for final depth
         if ( calculated_solutes.size() > max_depth )
            calculated_solutes[ max_depth ].clear();
         else
            calculated_solutes << QVector< US_Solute >();
      }
      else
      { // Shouldn't happen, but put job in queue if no worker is yet ready
         DbgLv( 1 ) << "Mast:   WARNING: LAST depth and no worker ready!";
         add_to_queue( job );
      }
   }

   // Force an abort if we are in a run-away situation
   if ( max_depth > 20 )
   {
      abort( "Max Depth is exceeding 20" );
      calculated_solutes[ depth + 20 ].clear();
   }
}
