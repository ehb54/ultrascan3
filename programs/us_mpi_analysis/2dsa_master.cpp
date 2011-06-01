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

   current_dataset     = 0;
   datasets_to_process = 1;  // Process one dataset at a time for now

QDateTime time = QDateTime::currentDateTime();  // For debug/timing

   while ( true )
   {
      int worker;

      // Give the jobs to the workers
      while ( ! job_queue.isEmpty()  &&  worker_status.contains( READY ) )
      {
         worker                 = worker_status.indexOf( READY );
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
            "; Dataset: "    + QString::number( current_dataset ) +
            "; Meniscus: "   + 
                 QString::number( meniscus_values[ meniscus_run ], 'f', 3 ) +
                 QString( " (Run %1 of %2)" ).arg( meniscus_run + 1 )
                                             .arg( meniscus_values.size() )  +
            "; MonteCarlo: " + QString::number( mc_iteration );
         
         send_udp( progress );

         // Iterative refinement
         if ( iterations < max_iterations + 1 )
         {   
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
      int        size[ 3 ];
      MPI_Status status;

      MPI_Recv( &size, 
                3, 
                MPI_INT2,
                MPI_ANY_SOURCE,
                MPI_ANY_TAG,
                MPI_COMM_WORLD2,
                &status);

      switch( status.MPI_TAG )
      {
         case MPI_Job::READY: // Ready for work
            worker = status.MPI_SOURCE;
            worker_status[ worker ] = READY;
            break;

         case MPI_Job::RESULTS: // Return solute data
            process_results( status.MPI_SOURCE, size );
            break;

         default:  // Should never happen
            QString msg =  "Master 2DSA:  Received invalid status " +
                           QString::number( status.MPI_TAG );
            abort( msg );
            break;
      }
   } 
}

//////////////////
QVector< US_MPI_Analysis::Solute > US_MPI_Analysis::create_solutes( 
        double s_min,   double s_max,   double s_step,
        double ff0_min, double ff0_max, double ff0_step )
{
   QVector< Solute > solute_vector;

   for ( double ff0 = ff0_min; ff0 <= ff0_max; ff0 += ff0_step )
      for ( double s = s_min; s <= s_max; s += s_step )
      {
         // Omit s values close to zero.
         if ( s >= -1.0e-14  &&  s <= 1.0e-14 ) continue;

         solute_vector << Solute( s, ff0, 0.0 ); 
      }

   return solute_vector;
}

//////////////////
void US_MPI_Analysis::init_solutes( void )
{
// For now assume one data set
   calculated_solutes.clear();
   orig_solutes.clear();

   double s_min   = parameters[ "s_min"          ].toDouble() * 1.0e-13;
   double s_max   = parameters[ "s_max"          ].toDouble() * 1.0e-13;
   double s_res   = parameters[ "s_resolution"   ].toDouble();
   double ff0_min = parameters[ "ff0_min"        ].toDouble();
   double ff0_max = parameters[ "ff0_max"        ].toDouble();
   double ff0_res = parameters[ "ff0_resolution" ].toDouble();

   int grid_repetitions = parameters[ "uniform_grid" ].toInt();
   if ( grid_repetitions < 1 ) grid_repetitions = 1;

   double s_step   = ( s_max   - s_min   ) / ( s_res   - 1 );
   double ff0_step = ( ff0_max - ff0_min ) / ( ff0_res - 1 );

   // Allow a 5% overscan
   s_max   += 0.05 * s_step;  // Assumes positive s
   ff0_max += 0.05 * ff0_step;

   orig_solutes.reserve( sq( grid_repetitions ) );

   double s_grid   = s_step   / grid_repetitions;
   double ff0_grid = ff0_step / grid_repetitions;

   // Generate solutes for each grid repetition
   for ( int i = 0; i < grid_repetitions; i++ )
   {
      for ( int j = 0; j < grid_repetitions; j++ )
      {
         orig_solutes << create_solutes( 
               s_min   + s_grid   * i,   s_max,   s_step, 
               ff0_min + ff0_grid * j, ff0_max, ff0_step );
   
      }
   }
}

//////////////////
void US_MPI_Analysis::fill_queue( void )
{
   worker_status.resize( node_count );
   worker_depth .resize( node_count );

   worker_status.fill( INIT );
   worker_depth .fill( 0 );
   max_depth           = 0;
   max_experiment_size = 0;

   // Put all jobs in the queue
   job_queue.clear();

   for ( int i = 0; i < orig_solutes.size(); i++ )
   {
      max_experiment_size = max( max_experiment_size, orig_solutes[ i ].size());
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
   for ( int worker = 1; worker < node_count; worker++ )
   {
      MPI_Send( &job, 
          sizeof( MPI_Job ), 
          MPI_BYTE2,
          worker,   
          MPI_Job::MASTER,
          MPI_COMM_WORLD2 );
   }

   // Get everybody synced up
   MPI_Barrier( MPI_COMM_WORLD2 );

   MPI_Bcast( scaled_data.data(), 
              scaled_data.size(), 
              MPI_DOUBLE, 
              MPI_Job::MASTER, 
              MPI_COMM_WORLD2 );

   // Go to the next dataset
   current_dataset++;
   
   // If all datasets have been scaled, do all datasets from now on
   if ( current_dataset >= data_sets.size() )
   {
      datasets_to_process = data_sets.size();
      current_dataset     = 0;
   }

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
}

//////////////////
void US_MPI_Analysis::set_monteCarlo( void )
{
   // Set up new data modified by a gaussian distribution
   if ( mc_iteration == 1 ) set_gaussians();

   int total_points = 0;

   for ( int e = 0; e < data_sets.size(); e++ )
   {
      US_DataIO2::EditedData* data = &data_sets[ e ]->run_data;

      int scan_count    = data->scanData.size();
      int radius_points = data->x.size();

      total_points += scan_count * radius_points;
   }

   mc_data.resize( total_points );

   int index = 0;

   // Get a randomized variation of the concentrations
   // Use a gaussian distribution with the residual as the standard deviation
   for ( int e = 0; e < data_sets.size(); e++ )
   {
      US_DataIO2::EditedData* data = &data_sets[ e ]->run_data;

      int scan_count    = data->scanData.size();
      int radius_points = data->x.size();

      for ( int s = 0; s < scan_count; s++ )
      {
         for ( int r = 0; r < radius_points; r++ )
         {
            double variation = US_Math2::box_muller( 0.0, sigmas[ index ] );
            mc_data[ index ] = solution.value( s, r ) + variation;
            index++;
         }
      }
   }

   // Broadcast Monte Carlo data to all workers
   MPI_Job newdata;
   newdata.command        = MPI_Job::NEWDATA;
   newdata.length         = total_points;
   newdata.dataset_offset = 0;
   newdata.dataset_count  = data_sets.size();

   // Tell each worker that new data coming
   // Can't use a broadcast because the worker is expecting a Send
   for ( int worker = 1; worker < node_count; worker++ )
   {
      MPI_Send( &newdata, 
          sizeof( MPI_Job ), 
          MPI_BYTE2,
          worker,   
          MPI_Job::MASTER,
          MPI_COMM_WORLD2 );
   }

   // Get everybody synced up
   MPI_Barrier( MPI_COMM_WORLD2 );

   MPI_Bcast( mc_data.data(), 
              total_points, 
              MPI_DOUBLE, 
              MPI_Job::MASTER, 
              MPI_COMM_WORLD2 );

   _2dsa_Job job;
   job.mpi_job.dataset_offset = 0;
   job.mpi_job.dataset_count  = data_sets.size();

   // Set up to run the next Monte Carlo iteration
   for ( int i = 0; i < orig_solutes.size(); i++ )
   {
      _2dsa_Job job;
      job.solutes = orig_solutes[ i ];

      job_queue << job;
   }

   worker_depth.fill( 0 );
   max_depth = 0;
}

//////////////////
//  Generate the simulated data and calculate the residuals
//  Use the residuals as the standard deviation for varying the
//  data in Monte Carlo iterations
void US_MPI_Analysis::set_gaussians( void )
{
   simulation_values.solutes = calculated_solutes[ max_depth ];
   meniscus_value            = -1.0;   // Used edited value

   calc_residuals( 0, data_sets.size(), simulation_values );

   sigmas.clear();

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
            v[ r ] = fabs( residuals.value( s, r ) );
         }

         // Smooth using 5 points to the left and right of each point
         US_Math2::gaussian_smoothing( v, 5 );
         sigmas << v;
      }
   }
}
//////////////////
void US_MPI_Analysis::write_output( void )
{
   qSort( simulation_values.solutes );
   
   Simulation sim;
   double     save_meniscus = meniscus_value;
   sim.solutes              = calculated_solutes[ max_depth ]; 
   sim.variance             = simulation_values.variance;
   meniscus_value           = meniscus_values[ meniscus_run ];

   //write_2dsa();
   write_model( sim, US_Model::TWODSA );
   meniscus_value = save_meniscus;

   if (  parameters[ "tinoise_option" ].toInt() > 0 )
      write_noise( US_Noise::TI, simulation_values.ti_noise );

   if (  parameters[ "rinoise_option" ].toInt() > 0 )
      write_noise( US_Noise::RI, simulation_values.ri_noise );
}
//////////////////
void US_MPI_Analysis::iterate( void )
{
   // Just return if the number of iterations exceed the max or of the
   // last two iterations converged and are essentially identical
   if ( ++iterations > max_iterations ) return;

   double diff = fabs( simulation_values.variance - previous_values.variance );
   if ( iterations > 1  &&  diff < min_variance_improvement  ) return;

   // Save the most recent variance for the next time
   previous_values.variance = simulation_values.variance;
   
   // Set up for another round at depth 0
   _2dsa_Job job;
   job.mpi_job.dataset_offset = current_dataset;
   job.mpi_job.dataset_count  = datasets_to_process;

   QVector< Solute > prev_solutes = simulation_values.solutes;
   
   for ( int i = 0; i < orig_solutes.size(); i++ )
   {
      job.solutes = orig_solutes[ i ];

      // Add back all non-zero Solutes to each job
      // Ensure thee are no duplicates
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
 
   for ( int i = 1; i < node_count; i++ )
   {
      MPI_Send( &job, 
         sizeof( job ), 
         MPI_BYTE2,
         i,               // Send to each worker
         MPI_Job::MASTER,
         MPI_COMM_WORLD2 );
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

   // Tell worker that solutes are coming
   MPI_Send( &job.mpi_job, 
       sizeof( MPI_Job ), 
       MPI_BYTE2,
       worker,      // Send to system that needs work
       MPI_Job::MASTER,
       MPI_COMM_WORLD2 );

   // Send solutes
   MPI_Send( job.solutes.data(), 
       job.mpi_job.length * solute_doubles, 
       MPI_DOUBLE2,  // Pass solute vector as hw independent values
       worker,       // to worker
       MPI_Job::MASTER,
       MPI_COMM_WORLD2 );
}

/////////////////////
void US_MPI_Analysis::process_results( int        worker, 
                                       const int* size )
{
   simulation_values.solutes.resize( size[ 0 ] );
   simulation_values.variances.resize( data_sets.size() );
   simulation_values.ti_noise.resize( size[ 1 ] );
   simulation_values.ri_noise.resize( size[ 2 ] );

   max_experiment_size = max( min_experiment_size, max_experiment_size );

   MPI_Status status;

   // Get all simulation_values
   MPI_Recv( simulation_values.solutes.data(),
             size[ 0 ] * solute_doubles,
             MPI_DOUBLE,
             worker,
             MPI_Job::TAG0,
             MPI_COMM_WORLD2,
             &status );

   MPI_Recv( &simulation_values.variance,
             1,
             MPI_DOUBLE,
             worker,
             MPI_Job::TAG0,
             MPI_COMM_WORLD2,
             &status );
   
   MPI_Recv( simulation_values.variances.data(),
             data_sets.size(),
             MPI_DOUBLE,
             worker,
             MPI_Job::TAG0,
             MPI_COMM_WORLD2,
             &status );

   MPI_Recv( simulation_values.ti_noise.data(),
             size[ 1 ],
             MPI_DOUBLE,
             worker,
             MPI_Job::TAG0,
             MPI_COMM_WORLD2,
             &status );

   MPI_Recv( simulation_values.ri_noise.data(),
             size[ 2 ],
             MPI_DOUBLE,
             worker,
             MPI_Job::TAG0,
             MPI_COMM_WORLD2,
             &status );

   worker_status[ worker ] = INIT;
   int depth = worker_depth[ worker ];   
 
   // This loop should only execute, at most, once per result.
   while ( calculated_solutes.size() < depth + 1 )
      calculated_solutes << QVector< Solute >();

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
      job_queue << job;

      max_depth = max( depth + 1, max_depth );
      calculated_solutes[ depth ].clear();
   }

   // Add the current results
   calculated_solutes[ depth ] += simulation_values.solutes;

   // At this point we need to clean up,  For each depth
   // below the current one, if there is nothing in the queue
   // or working and there are calculated solutes left, those
   // need to be submitted.
   
   for ( int d = 0; d < depth; d++ )
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
      for ( int w = 1; w < node_count; w++ )
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
         job_queue << job;

         calculated_solutes[ d ].clear();
      }
   }

   // Is anyone working?
   bool working = false;
   for ( int w = 1; w < node_count; w++ )
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
         job_queue << job;

         calculated_solutes[ depth ].clear();
         max_depth = depth + 1;
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
   // runID.tripleID.analysisID.recordType
   //    analysisID = editID_analysisDate_analysisType_requestID_iterID (underscores)
   //       editID:     
   //       requestID: from lims or 'local' 
   //       analysisType : 2DSA GA others
   //       iterID:       'i01-m62345' for meniscus, mc001 for monte carlo, i01 default 
   //      
   //       recordType: ri_noise, ti_noise, model

   QString tripleID = data->cell + data->channel + data->wavelength;
   QString dates    = "e" + data->editID + "_a" + analysisDate;

   QString iterID;

   if ( mc_iterations > 1 )  // Will not happen
      iterID.sprintf( "mc%04d", mc_iteration + 1 );

   else if (  meniscus_points > 1 )
      iterID.sprintf( "i%02d-m%05d", 
              meniscus_run + 1,
              (int)(meniscus_values[ meniscus_run ] * 10000 ) );
   else
      iterID = "i01";

   QString analysisID = dates + "_2DSA_" + requestID + "_" + iterID;

   noise.description = data->runID + "." + tripleID + "." + analysisID + "." + type_name + "_noise";

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
