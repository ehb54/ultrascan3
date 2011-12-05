#include "us_mpi_analysis.h"
#include "us_math2.h"
#include "us_util.h"
#include "us_astfem_rsa.h"
#include "us_simparms.h"
#include "us_constants.h"

void US_MPI_Analysis::_2dsa_master( void )
{
   init_solutes();
   fill_queue();

   work_rss.resize( proc_count );

   current_dataset     = 0;
   datasets_to_process = 1;  // Process one dataset at a time for now

   while ( true )
   {
      int worker;
//if ( max_depth > 1 )
// DbgLv(1) << " master loop-TOP:  jq-empty?" << job_queue.isEmpty() << "   areReady?" << worker_status.contains(READY)
//    << "  areWorking?" << worker_status.contains(WORKING);

      // Give the jobs to the workers
      while ( ! job_queue.isEmpty()  &&  worker_status.contains( READY ) )
      {
         worker    = worker_status.indexOf( READY, worknext );

         if ( worker < 1 )
         {
            worknext  = 1;
            worker    = worker_status.indexOf( READY, worknext );
         }

         worknext  = worker + 1;
         worknext  = ( worknext >= proc_count ) ? 1 : worknext;

         _2dsa_Job job          = job_queue.takeFirst();
         submit( job, worker );
         worker_depth [ worker ] = job.mpi_job.depth;
         worker_status[ worker ] = WORKING;
      }

      // All done with the pass if no jobs are ready or running
      if ( job_queue.isEmpty()  &&  ! worker_status.contains( WORKING ) ) 
      {
         QString progress = 
            "Iteration: "    + QString::number( iterations ) +
            "; Dataset: "    + QString::number( current_dataset + 1 ) +
            "; Meniscus: "   + 
                 QString::number( meniscus_values[ meniscus_run ], 'f', 3 ) +
                 QString( " (Run %1 of %2)" ).arg( meniscus_run + 1 )
                                             .arg( meniscus_values.size() )  +
            "; MonteCarlo: " + QString::number( mc_iteration + 1 );

         send_udp( progress );

         // Iterative refinement
         if ( max_iterations > 1 )
         {
            qDebug() << "Iteration:" << iterations << " Variance:"
               << simulation_values.variance;

            iterate();
         }

         if ( ! job_queue.isEmpty() ) continue;

         iterations = 1;

         // Manage multiple data sets
         if ( data_sets.size() > 1  &&  datasets_to_process == 1 )
         {
            global_fit();
         }

         if ( ! job_queue.isEmpty() ) continue;
         write_output();

         // Fit meniscus 
         if ( meniscus_run + 1 < meniscus_values.size() )
         {
            set_meniscus(); 
         }

         if ( ! job_queue.isEmpty() ) continue;

         // Monte Carlo
         if ( ++mc_iteration < mc_iterations )
         {
            set_monteCarlo(); 
         }

         if ( ! job_queue.isEmpty() ) continue;

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
                MPI_COMM_WORLD,
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
            QString msg =  "Master 2DSA:  Received invalid status " +
                           QString::number( status.MPI_TAG );
            abort( msg );
            break;
      }

      max_rss();
   }
}


void US_MPI_Analysis::init_solutes( void )
{
// For now assume one data set
   calculated_solutes.clear();
   orig_solutes.clear();
   simulation_values.noisflag    = parameters[ "tinoise_option" ].toInt() > 0 ?
                                   1 : 0;
   simulation_values.noisflag   += parameters[ "rinoise_option" ].toInt() > 0 ?
                                   2 : 0;
   simulation_values.dbg_level   = dbg_level;
   simulation_values.dbg_timing  = dbg_timing;
DbgLv(0) << "DEBUG_LEVEL" << simulation_values.dbg_level;

   double s_min   = parameters[ "s_min"           ].toDouble() * 1.0e-13;
   double s_max   = parameters[ "s_max"           ].toDouble() * 1.0e-13;
   double ff0_min = parameters[ "ff0_min"         ].toDouble();
   double ff0_max = parameters[ "ff0_max"         ].toDouble();

   int grid_repetitions = parameters[ "uniform_grid" ].toInt();
   if ( grid_repetitions < 1 ) grid_repetitions = 1;

   double s_pts    = 60.0;
   double ff0_pts  = 60.0;
   if ( parameters.contains( "s_grid_points"   ) )
      s_pts    = parameters[ "s_grid_points"   ].toDouble();
   else if ( parameters.contains( "s_resolution"    ) )
      s_pts    = parameters[ "s_resolution"    ].toDouble() * grid_repetitions;
   if ( parameters.contains( "ff0_grid_points" ) )
      ff0_pts  = parameters[ "ff0_grid_points" ].toDouble();
   else if ( parameters.contains( "ff0_resolution"  ) )
      ff0_pts  = parameters[ "ff0_resolution"  ].toDouble() * grid_repetitions;

   int    nsstep   = (int)( s_pts );
   int    nkstep   = (int)( ff0_pts );

   US_Solute::init_solutes( s_min,   s_max,   nsstep,
                            ff0_min, ff0_max, nkstep,
                            grid_repetitions, orig_solutes );
}

//////////////////
void US_MPI_Analysis::fill_queue( void )
{
   worker_status.resize( proc_count );
   worker_depth .resize( proc_count );

   worker_status.fill( INIT );
   worker_depth .fill( 0 );
   max_depth           = 0;
   worknext            = 1;
   max_experiment_size = min_experiment_size;

   // Put all jobs in the queue
   job_queue.clear();

   for ( int i = 0; i < orig_solutes.size(); i++ )
   {
      max_experiment_size = max( max_experiment_size, orig_solutes[ i ].size() );
      _2dsa_Job job;
      job.solutes         = orig_solutes[ i ];
      job_queue << job;
   }
}

//////////////////
void US_MPI_Analysis::global_fit( void )
{
   // To do a global fit across multiple data sets:
   // 1. Each individual data set must be run
   // 2. Sum the total concentration of all returned solutes
   // 3. Divide all experiment concentrations by the total concentration
   // 4. Send the concentration data to the workers
   // 5. Do an additional run against the combined datasets for the baseline
   // Any additional Monte Carlo runs will use the adjusted data for all
   // data sets.
   
   double concentration = 0.0;

   // The first dataset is done automatically.
   for ( int solute = 0; solute < simulation_values.solutes.size(); solute++ )
   {
      concentration += simulation_values.solutes[ solute ].c;
   }

   // Point to current dataset
   US_DataIO2::EditedData* data = &data_sets[ current_dataset ]->run_data;

   int scan_count    = data->scanData.size();
   int radius_points = data->x.size();
   int index         = 0;

   QVector< double > scaled_data( scan_count * radius_points );

   // Scale the data
   for ( int s = 0; s < scan_count; s++ )
   {
      for ( int r = 0; r < radius_points; r++ )
      {
         scaled_data[ index++ ] = data->value( s, r ) / concentration;
      }
   }

   // Send the scaled data to the workers
   MPI_Job job;
   job.command        = MPI_Job::NEWDATA;
   job.length         = scaled_data.size();
   job.dataset_offset = current_dataset;
   job.dataset_count  = 1;

   // Tell each worker that new data coming
   // Can't use a broadcast because the worker is expecting a Send
   for ( int worker = 1; worker < proc_count; worker++ )
   {
      MPI_Send( &job, 
          sizeof( MPI_Job ), 
          MPI_BYTE,
          worker,   
          MPI_Job::MASTER,
          MPI_COMM_WORLD );
   }

   // Get everybody synced up
   MPI_Barrier( MPI_COMM_WORLD );

   MPI_Bcast( scaled_data.data(), 
              scaled_data.size(), 
              MPI_DOUBLE, 
              MPI_Job::MASTER, 
              MPI_COMM_WORLD );

   // Go to the next dataset
   current_dataset++;
   
   // If all datasets have been scaled, do all datasets from now on
   if ( current_dataset >= data_sets.size() )
   {
      datasets_to_process = data_sets.size();
      current_dataset     = 0;
   }

   job_queue.clear();

   for ( int i = 0; i < orig_solutes.size(); i++ )
   {
      _2dsa_Job job;
      job.solutes                = orig_solutes[ i ];
      job.mpi_job.dataset_offset = current_dataset;
      job.mpi_job.dataset_count  = datasets_to_process;

      job_queue << job;
   }

   worker_depth.fill( 0 );
   max_depth = 0;
   for ( int ii = 0; ii < calculated_solutes.size(); ii++ )
      calculated_solutes[ ii ].clear();
}

//////////////////
void US_MPI_Analysis::set_meniscus( void )
{
   meniscus_run++;

   // We incremented meniscus_run above.  Just rerun from the beginning.
   for ( int i = 0; i < orig_solutes.size(); i++ )
   {
      _2dsa_Job job;
      job.solutes = orig_solutes[ i ];

      job_queue << job;
   }

   worker_depth.fill( 0 );
   max_depth = 0;
   for ( int ii = 0; ii < calculated_solutes.size(); ii++ )
      calculated_solutes[ ii ].clear();
}

//////////////////
void US_MPI_Analysis::set_monteCarlo( void )
{
DbgLv(1) << "sMC: max_depth" << max_depth << "calcsols size" << calculated_solutes[max_depth].size()
 << "simvsols size" << simulation_values.solutes.size();

   // Set up new data modified by a gaussian distribution
   if ( mc_iteration == 1 )
   {
      set_gaussians();

//      sim_data1 = simulation_values.sim_data;
      US_AstfemMath::initSimData( sim_data1, data_sets[ 0 ]->run_data, 0.0 );
      int scan_count    = sim_data1.scanData.size();
      int radius_points = sim_data1.x.size();

      for ( int ss = 0; ss < scan_count; ss++ )
         for ( int rr = 0; rr < radius_points; rr++ )
            sim_data1.scanData[ ss ].readings[ rr ].value = 
               simulation_values.sim_data.value( ss, rr );
   }

   int total_points = 0;

   for ( int e = 0; e < data_sets.size(); e++ )
   {
      US_DataIO2::EditedData* data = &data_sets[ e ]->run_data;

      int scan_count    = data->scanData.size();
      int radius_points = data->x.size();

      total_points += scan_count * radius_points;
   }
DbgLv(1) << "sMC: totpts" << total_points << "mc_iter" << mc_iteration;

   mc_data.resize( total_points );
   int index = 0;

   // Get a randomized variation of the concentrations
   // Use a gaussian distribution with the residual as the standard deviation
   for ( int e = 0; e < data_sets.size(); e++ )
   {
      US_DataIO2::EditedData* data = &data_sets[ e ]->run_data;

      int scan_count    = data->scanData.size();
      int radius_points = data->x.size();
      double varrmsd    = 0.0;

double varisum=0.0;
double varimin=1.0;
double varimax=-1.0;
double datasum=0.0;
//int indxh=((scan_count/2)*radius_points)+(radius_points/2);

      for ( int s = 0; s < scan_count; s++ )
      {

         for ( int r = 0; r < radius_points; r++ )
         {
            double variation = US_Math2::box_muller( 0.0, sigmas[ index ] );
            double mcdata    = sim_data1.value( s, r ) + variation;
            varrmsd         += sq( variation );

//if ( index<5 || index>(total_points-6) || (index>(indxh-4)&&index<(indxh+3)) )
//DbgLv(1) << "sMC:  index" << index << "sdat" << sim_data1.value(s,r)
// << "sigma" << sigmas[index] << "vari" << variation << "mdat" << mcdata;
varisum += variation;
varimin  = qMin(varimin,variation);
varimax  = qMax(varimax,variation);
datasum += mcdata;

            mc_data[ index++ ] = mcdata;
         }
      }

      varrmsd = sqrt( varrmsd / (double)( scan_count * radius_points ) );
      DbgLv(0) << "Variation RMSD" << varrmsd << "  for MC_Iteration" << mc_iteration + 1;

DbgLv(1) << "sMC:   variation  sum min max" << varisum << varimin << varimax
 << "mcdata sum" << datasum;
   }

   // Broadcast Monte Carlo data to all workers
   MPI_Job newdata;
   newdata.command        = MPI_Job::NEWDATA;
   newdata.length         = total_points;
   newdata.dataset_offset = 0;
   newdata.dataset_count  = data_sets.size();

   // Tell each worker that new data coming
   // Can't use a broadcast because the worker is expecting a Send
DbgLv(1) << "sMC: MPI send   proc_count" << proc_count;
   for ( int worker = 1; worker < proc_count; worker++ )
   {
      MPI_Send( &newdata, 
          sizeof( MPI_Job ), 
          MPI_BYTE,
          worker,   
          MPI_Job::MASTER,
          MPI_COMM_WORLD );
   }

   // Get everybody synced up
DbgLv(1) << "sMC: MPI Barrier";
   MPI_Barrier( MPI_COMM_WORLD );

DbgLv(1) << "sMC: MPI Bcast";
   MPI_Bcast( mc_data.data(), 
              total_points, 
              MPI_DOUBLE, 
              MPI_Job::MASTER, 
              MPI_COMM_WORLD );

   fill_queue();

   for ( int ii = 0; ii < calculated_solutes.size(); ii++ )
      calculated_solutes[ ii ].clear();
}

//////////////////
//  Generate the simulated data and calculate the residuals
//  Use the residuals as the standard deviation for varying the
//  data in Monte Carlo iterations
void US_MPI_Analysis::set_gaussians( void )
{
DbgLv(1) << "sGA: calcsols size mxdpth" << calculated_solutes.size() << max_depth;

   simulation_values.solutes = calculated_solutes[ max_depth ];

int mm=simulation_values.solutes.size()-1;
DbgLv(1) << "sGA:   sol0.s solM.s" << simulation_values.solutes[0].s
 << simulation_values.solutes[mm].s << "  M=" << mm;;
DbgLv(1) << "sGA:     solM.k" << simulation_values.solutes[mm].k;
DbgLv(1) << "sGA:     solM.c" << simulation_values.solutes[mm].c;
US_DataIO2::EditedData *edata = &data_sets[0]->run_data;
DbgLv(1) << "sGA:    edata scans points" << edata->scanData.size() << edata->x.size();

   calc_residuals( 0, data_sets.size(), simulation_values );

   sigmas.clear();
   res_data = &simulation_values.residuals;
DbgLv(1) << "sGA:  resids scans points" << res_data->scanData.size() << res_data->x.size();

   for ( int e = 0; e < data_sets.size(); e++ )
   {
      US_DataIO2::EditedData* data = &data_sets[ e ]->run_data;

      int scan_count    = data->scanData.size();
      int radius_points = data->x.size();

      // Smooth the data and place into a single vector for convenience
      for ( int s = 0; s < scan_count; s++ )
      {
         QVector< double > v( radius_points );

         for ( int r = 0; r < radius_points; r++ )
         {
            v[ r ] = fabs( res_data->value( s, r ) );
         }

         // Smooth using 5 points to the left and right of each point
         US_Math2::gaussian_smoothing( v, 5 );
         sigmas << v;
      }
   }
}

void US_MPI_Analysis::write_output( void )
{
   US_SolveSim::Simulation sim = simulation_values;

   double     save_meniscus = meniscus_value;
   sim.solutes              = calculated_solutes[ max_depth ]; 
   meniscus_value           = meniscus_values[ meniscus_run ];
   qSort( sim.solutes );
DbgLv(1) << "WrO: mciter mxdepth" << mc_iteration+1 << max_depth << "calcsols size"
 << calculated_solutes[max_depth].size() << "simvsols size" << sim.solutes.size();

   write_model( sim, US_Model::TWODSA );
   meniscus_value = save_meniscus;

   if (  parameters[ "tinoise_option" ].toInt() > 0 )
      write_noise( US_Noise::TI, sim.ti_noise );

   if (  parameters[ "rinoise_option" ].toInt() > 0 )
      write_noise( US_Noise::RI, sim.ri_noise );
}

void US_MPI_Analysis::iterate( void )
{
   // Just return if the number of iterations exceeds the max
   // or if the last two iterations converged and are essentially identical
   if ( ++iterations > max_iterations ) return;

   double diff  = qAbs( simulation_values.variance - previous_values.variance );
   bool   ssame = false;

   if ( iterations > 2 )
   {
      if ( diff < min_variance_improvement )  return;

      int    nsols = previous_values.solutes.size();

      if ( nsols == simulation_values.solutes.size() )
      {
         ssame   = true;

         for ( int jj = 0; jj < nsols; jj++ )
         {
            if ( previous_values.solutes[ jj ] != simulation_values.solutes[ jj ] )
            {  // Mismatch:  may need to iterate
               ssame    = false;
               break;
            }
         }
      }
   }

   if ( ssame )  return;  // Solutes same as previous:  no more iterations

   // Save the most recent variance for the next time
   previous_values.variance = simulation_values.variance;
   previous_values.solutes  = simulation_values.solutes;

   // Set up for another round at depth 0
   _2dsa_Job job;
   job.mpi_job.dataset_offset = current_dataset;
   job.mpi_job.dataset_count  = datasets_to_process;

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

      job_queue << job;
   }

   worker_depth.fill( 0 );
   max_depth = 0;
   for ( int ii = 0; ii < calculated_solutes.size(); ii++ )
      calculated_solutes[ ii ].clear();

   return;
}

/////////////////////
// Shutdown the workers
// Actually this doesn't necessarily shut them down, it breaks 
// out of the processing loop.
void US_MPI_Analysis::shutdown_all( void )
{
   MPI_Job job;
   job.command = MPI_Job::SHUTDOWN;
DbgLv(1) << "2dsa master shutdown : master maxrss" << maxrss;
 
   for ( int i = 1; i < proc_count; i++ )
   {
      MPI_Send( &job, 
         sizeof( job ), 
         MPI_BYTE,
         i,               // Send to each worker
         MPI_Job::MASTER,
         MPI_COMM_WORLD );

      maxrss += work_rss[ i ];
DbgLv(1) << "2dsa master shutdown : worker" << i << " upd. maxrss" << maxrss
 << "  wkrss" << work_rss[ i ];
   }
}

/////////////////////
void US_MPI_Analysis::submit( _2dsa_Job& job, int worker )
{
   job.mpi_job.command        = MPI_Job::PROCESS;
   job.mpi_job.length         = job.solutes.size(); 
   job.mpi_job.meniscus_value = meniscus_values[ meniscus_run ];
   job.mpi_job.solution       = mc_iteration;
   job.mpi_job.dataset_offset = current_dataset;
   job.mpi_job.dataset_count  = datasets_to_process;
int dd=job.mpi_job.depth;
if (dd==0) { DbgLv(1) << "Mast: submit: worker" << worker << "  sols"
 << job.mpi_job.length << "mciter" << mc_iteration << " depth" << dd; }
else { DbgLv(1) << "Mast: submit:     worker" << worker << "  sols"
 << job.mpi_job.length << "mciter" << mc_iteration << " depth" << dd; }

   // Tell worker that solutes are coming
   MPI_Send( &job.mpi_job, 
       sizeof( MPI_Job ), 
       MPI_BYTE,
       worker,      // Send to system that needs work
       MPI_Job::MASTER,
       MPI_COMM_WORLD );

   // Send solutes
   MPI_Send( job.solutes.data(), 
       job.mpi_job.length * solute_doubles, 
       MPI_DOUBLE,   // Pass solute vector as hw independent values
       worker,       // to worker
       MPI_Job::MASTER,
       MPI_COMM_WORLD );
}

/////////////////////
void US_MPI_Analysis::process_results( int        worker, 
                                       const int* size )
{
   simulation_values.solutes.resize( size[ 0 ] );
   simulation_values.variances.resize( data_sets.size() );
   simulation_values.ti_noise.resize( size[ 1 ] );
   simulation_values.ri_noise.resize( size[ 2 ] );

   max_experiment_size = qMax( max_experiment_size, size[ 0 ] );

   MPI_Status status;

   // Get all simulation_values
   MPI_Recv( simulation_values.solutes.data(),
             size[ 0 ] * solute_doubles,
             MPI_DOUBLE,
             worker,
             MPI_Job::TAG0,
             MPI_COMM_WORLD,
             &status );

   MPI_Recv( &simulation_values.variance,
             1,
             MPI_DOUBLE,
             worker,
             MPI_Job::TAG0,
             MPI_COMM_WORLD,
             &status );
   
   MPI_Recv( simulation_values.variances.data(),
             data_sets.size(),
             MPI_DOUBLE,
             worker,
             MPI_Job::TAG0,
             MPI_COMM_WORLD,
             &status );

   MPI_Recv( simulation_values.ti_noise.data(),
             size[ 1 ],
             MPI_DOUBLE,
             worker,
             MPI_Job::TAG0,
             MPI_COMM_WORLD,
             &status );

   MPI_Recv( simulation_values.ri_noise.data(),
             size[ 2 ],
             MPI_DOUBLE,
             worker,
             MPI_Job::TAG0,
             MPI_COMM_WORLD,
             &status );

   worker_status[ worker ] = INIT;
   int depth = worker_depth[ worker ];   

if (depth == 0) { DbgLv(1) << "Mast:  process_results: worker" << worker
 << " solsize" << size[0] << "depth" << depth; }
else { DbgLv(1) << "Mast:  process_results:      worker" << worker
 << " solsize" << size[0] << "depth" << depth; }
 
   // This loop should only execute, at most, once per result.
   while ( calculated_solutes.size() < depth + 1 )
      calculated_solutes << QVector< US_Solute >();

   // How big will our vector be?
   int new_length = calculated_solutes[ depth ].size() + 
                    simulation_values.solutes.size();
  
   // Submit it if it's too long
   if ( new_length > max_experiment_size )
   {
      // Put current solutes on queue at depth + 1
      _2dsa_Job job;
      job.solutes                = calculated_solutes[ depth ];
      job.mpi_job.depth          = depth + 1;
      job.mpi_job.dataset_offset = current_dataset;
      job.mpi_job.dataset_count  = datasets_to_process;
      qSort( job.solutes );
      job_queue << job;

DbgLv(1) << "Mast:   queue new DEPTH sols" << job.solutes.size() << " d=" << job.mpi_job.depth;
      max_depth = max( depth + 1, max_depth );
      calculated_solutes[ depth ].clear();
   }

   // Add the current results
   calculated_solutes[ depth ] += simulation_values.solutes;

   // At this point we need to clean up,  For each depth
   // below the current one, if there is nothing in the queue
   // or working and there are calculated solutes left, those
   // need to be submitted.
 
   int dcheck = depth;
   if ( depth == 0  &&  max_depth > 0 )  dcheck = 1;

   for ( int d = 0; d < dcheck; d++ )
   {
      bool queued = false;
      for ( int q = 0; q < job_queue.size(); q++ )
      {
         if ( job_queue[ q ].mpi_job.depth == d )
         {
            queued = true;
            break;
         }
      }

      bool working = false;
      for ( int w = 1; w < proc_count; w++ )
      {
         if ( worker_depth[ w ] == d  &&  worker_status[ w ] == WORKING )
         {
            working = true;
            break;
         }
      }

      int remainder = calculated_solutes[ d ].size();

      if ( ! working && ! queued && remainder > 0 )
      {
         _2dsa_Job job;
         job.solutes                = calculated_solutes[ d ];
         job.mpi_job.depth          = d + 1;
         job.mpi_job.dataset_offset = current_dataset;
         job.mpi_job.dataset_count  = datasets_to_process;
         qSort( job.solutes );
         job_queue << job;
DbgLv(1) << "Mast:   queue REMAINDER" << remainder << " d=" << d+1;

         calculated_solutes[ d ].clear();
      }
   }

   // Is anyone working?
   bool working = false;
   for ( int w = 1; w < proc_count; w++ )
   {
      if ( worker_status[ w ] == WORKING )
      {
         working = true;
         break;
      }
   }

   // Submit one last time with all solutes if necessary
   if ( depth == max_depth     &&
        job_queue.isEmpty()    &&
        ! working              &&
        calculated_solutes[ depth ].size() > simulation_values.solutes.size() )
   {
         _2dsa_Job job;
         job.solutes                = calculated_solutes[ depth ];
         job.mpi_job.depth          = depth + 1;
         job.mpi_job.dataset_offset = current_dataset;
         job.mpi_job.dataset_count  = datasets_to_process;
         qSort( job.solutes );
         job_queue << job;
DbgLv(1) << "Mast:   queue LAST ns=" << job.solutes.size() << "  d=" << depth+1 << max_depth
 << "  nsvs=" << simulation_values.solutes.size();
if(max_depth>10) calculated_solutes[depth+10].clear();  // Force abort if run-away

         calculated_solutes[ depth ].clear();
         max_depth = depth + 1;
         worker    = worker_status.indexOf( READY, worknext );
         if ( worker < 1 )
         {
            worknext  = 1;
            worker    = worker_status.indexOf( READY, worknext );
         }
         worknext  = worker + 1;
         worknext  = ( worknext >= proc_count ) ? 1 : worknext;

         if ( worker > 0 )
         {
            //_2dsa_Job job           = job_queue.takeFirst();
            _2dsa_Job job2          = job_queue.takeFirst();
            submit( job, worker );
            worker_depth [ worker ] = job.mpi_job.depth;
            worker_status[ worker ] = WORKING;
         }

   }
}

/////////////////////
void US_MPI_Analysis::write_noise( US_Noise::NoiseType      type, 
                                   const QVector< double >& noise_data )
{
   US_DataIO2::EditedData* data = &data_sets[ 0 ]->run_data;

   QString  type_name;
   US_Noise noise;

   if ( type == US_Noise::TI ) 
   {
      type_name         = "ti";
      int radii         = data->x.size();
      noise.minradius   = data->radius( 0 );
      noise.maxradius   = data->radius( radii - 1 );
   }
   else
   {
      type_name = "ri";
   }

   // demo1_veloc. 1A999. e201101171200_a201101171400_2DSA us3-0000003           .model
   // demo1_veloc. 1A999. e201101171200_a201101171400_2DSA us3-0000003           .ri_noise
   // demo1.veloc. 1A999. e201101171200_a201101171400_2DSA_us3-0000003_i01-m62345.ri_noise
   // demo1_veloc. 1A999. e201101171200_a201101171400_2DSA_us3-0000003_mc001     .model
   // runID.tripleID.analysID.recordType
   //    analysID = editID_analysisDate_analysisType_requestID_iterID (underscores)
   //       editID:     
   //       requestID: from lims or 'local' 
   //       analysisType : 2DSA GA others
   //       iterID:       'i01-m62345' for meniscus, mc001 for monte carlo, i01 default 
   //      
   //       recordType: ri_noise, ti_noise, model

   QString tripleID   = data->cell + data->channel + data->wavelength;
   QString dates      = "e" + data->editID + "_a" + analysisDate;
   QString anType     = "_" + data_sets[ 0 ]->model.typeText() + "_";

   QString iterID;

   if ( mc_iterations > 1 )           // MonteCarlo iteration
      iterID.sprintf( "mc%04d", mc_iteration + 1 );

   else if (  meniscus_points > 1 )   // Fit meniscus
      iterID.sprintf( "i%02d-m%05d", meniscus_run + 1,
              (int)(meniscus_values[ meniscus_run ] * 10000 ) );

   else                               // Non-iterative single
      iterID = "i01";

   QString analysID  = dates + anType + requestID + "_" + iterID;

   noise.description = data->runID + "." + tripleID + "." + analysID
      + "." + type_name + "_noise";

   noise.type        = type;
   noise.noiseGUID   = US_Util::new_guid();
   noise.modelGUID   = modelGUID;
   noise.values      = noise_data;
   noise.count       = noise_data.size();

   // Add in input noise for associated noise type
   // We are not checking for errors here, because that was checked when
   // the input noise was applied.

   US_Noise         input_noise;
   QList< QString > noise_filenames = data_sets[ 0 ]->noise_files;

   for ( int j = 0; j < noise_filenames.size(); j++ )
   {
      QString fn = "../" + noise_filenames[ j ];
      input_noise.load( fn );
      if ( input_noise.type == type ) noise.sum_noise( input_noise );
   }

   QString fn = type_name + ".noise." + noise.noiseGUID + ".xml";
   noise.write( fn );

   // Add the file name of the noise file to the output list
   QFile f( "analysis_files.txt" );
   if ( ! f.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append ) )
   {
      abort( "Could not open 'analysis_files.txt' for writing", -1 );
      return;
   }

   QTextStream out( &f );
   out << fn << "\n";
   f.close();
}

