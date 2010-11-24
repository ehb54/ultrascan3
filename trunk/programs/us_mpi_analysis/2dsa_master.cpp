#include "us_mpi_analysis.h"
#include "us_math2.h"
#include "us_util.h"
#include "us_astfem_rsa.h"
#include "us_simparms.h"
#include "us_constants.h"

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
void US_MPI_Analysis::_2dsa_master( void )
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

   double s_step   = ( s_max   - s_min   ) / s_res;
   double ff0_step = ( ff0_max - ff0_min ) / ff0_res;

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

   QTime time;  // For debug/timing

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

   while ( true )
   {
      int worker;
top:
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
         // Iterative refinement
         if ( iterations < max_iterations + 1 ) iterate();
         if ( ! job_queue.isEmpty() ) goto top;;
           
         iterations = 0;

         // Fit meniscus 
         if ( ++meniscus_run < meniscus_values.size() )
         {
            write_output();
            set_meniscus();
         }

         if ( ! job_queue.isEmpty() ) goto top;;
         
         // Monte Carlo
         if ( ++mc_iteration < mc_iterations )
         {
            write_output();
            set_monteCarlo();
         }

         if ( ! job_queue.isEmpty() ) goto top;;

         break;   // All done.  Break out of main loop.
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

qDebug() << "Finishing up";
   // Finish up
   shutdown_all();

qDebug() << "Writing last";
   write_output();
}

//////////////////
void US_MPI_Analysis::set_meniscus( void )
{
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

   int scan_count    = data_sets[ 0 ]->run_data.scanData.size();
   int radius_points = data_sets[ 0 ]->run_data.x.size();
   int index         = 0;

   // Get a randomized variation of the concentrations
   for ( int s = 0; s < scan_count; s++ )
   {
      for ( int r = 0; r < radius_points; r++ )
      {
         double value     = sim_data.value( s, r );
         double variation = US_Math2::box_muller( 0.0, sigmas[ index ] );
         // Add gaussian variation here
         mc_data[ index ] = value + variation;
         index++;
      }
   }

   // Broadcast Monte Carlo data to all workers
   MPI_Job job;
   job.command = MPI_Job::NEWDATA;
   job.length  = mc_data.size();

   // Tell each worker that new data coming
   // Can't use a broadcast because the work is expecting a Send
   for ( int worker = 1; worker < node_count; worker++ )
   {
      MPI_Send( &job, 
          sizeof( MPI_Job ), 
          MPI_BYTE2,
          worker,   
          MPI_Job::MASTER,
          MPI_COMM_WORLD2 );
   }

   MPI_Bcast( mc_data.data(), 
              mc_data.size(), 
              MPI_DOUBLE, 
              MPI_Job::MASTER, 
              MPI_COMM_WORLD2 );

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
   // Count the data points
   static int data_point_count = 0;
   static int dataset_size = data_sets.size();

   for ( int e = 0; e < e < dataset_size; e++ )
   {
      int scan_count    = data_sets[ e ]->run_data.scanData.size();
      int radius_points = data_sets[ e ]->run_data.x.size();

      data_point_count += scan_count * radius_points;
   }

   int    scan_count    = data_sets[ 0 ]->run_data.scanData.size();
   int    radius_points = data_sets[ 0 ]->run_data.x.size();

   mc_data.resize( data_point_count );

   // Create an array standard deviations based on residuals
   US_Model                model;
   US_SimulationParameters params;
   params.initFromData( NULL, data_sets[ 0 ]->run_data );
   
   double rpm = params.speed_step[ 0 ].rotorspeed;
   params.bottom = calc_bottom( 0, rpm );

   double vbar20 = data_sets[ 0 ]->vbar20;

   // Get the temperature for this run
   double sum           = 0.0;

   for ( int i = 0; i < scan_count; i++ )
      sum += data_sets[ 0 ]->run_data.scanData[ i ].temperature;

   double temperature = sum / scan_count;

   // Determine corrections for experimental space
   US_Math2::SolutionData solution;
   solution.density   = data_sets[ 0 ]->density;
   solution.viscosity = data_sets[ 0 ]->viscosity;
   solution.vbar20    = vbar20;
   solution.vbar      = US_Math2::adjust_vbar20( vbar20, temperature );

   US_Math2::data_correction( temperature, solution );

   // Create raw data structures and set cardinality
   US_AstfemMath::initSimData( sim_data, data_sets[ 0 ]->run_data, 0.0 );

   for ( int i = 0; i < simulation_values.solutes.size(); i++ )
   {
      double s20w   = fabs( simulation_values.solutes[ i ].s );
      double f_f0   = simulation_values.solutes[ i ].k;

      double D20w = R * K20 /
         ( AVOGADRO * 18.0 * M_PI *
           pow( f_f0 * VISC_20W / 100.0, 3.0 / 2.0 ) *
           sqrt( s20w * vbar20  /
                 ( 2.0 * ( 1.0 - vbar20 * DENS_20W ) )
               )
         );

      US_Model::SimulationComponent component;

      component.s = s20w / solution.s20w_correction;
      component.D = D20w / solution.D20w_correction;

      model.components << component;
   }

   US_Astfem_RSA astfem_rsa( model, params );

   // Calculate the simulation data
   astfem_rsa.calculate( sim_data );

   // Create residual structure and set cardinality
   US_DataIO2::RawData residuals;
   US_AstfemMath::initSimData( residuals, data_sets[ 0 ]->run_data, 0.0 );

   for ( int s = 0; s < scan_count; s++ )
   {
      for ( int r = 0; r < radius_points; r++ )
      {
         residuals.scanData[ s ].readings[ r ].value = 
            fabs( sim_data.value( s, r ) - 
                  data_sets[ 0 ]->run_data.value( s, r ) );
      }
   }

   sigmas.clear();

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
//////////////////
void US_MPI_Analysis::write_output( void )
{
   qSort( simulation_values.solutes );
   write_2dsa();

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

   QVector< Solute > prev_solutes = simulation_values.solutes;
   
   for ( int i = 0; i < orig_solutes.size(); i++ )
   {
      _2dsa_Job job;
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
      job.solutes         = calculated_solutes[ depth ];
      job.mpi_job.depth   = depth + 1;
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
         job.solutes         = calculated_solutes[ d ];
         job.mpi_job.depth   = d + 1;
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
         job.solutes       = calculated_solutes[ depth ];
         job.mpi_job.depth = depth + 1;
         job_queue << job;

         calculated_solutes[ depth ].clear();
         max_depth = depth + 1;
   }
}

/////////////////////
void US_MPI_Analysis::write_2dsa( void )
{
   US_DataIO2::EditedData* data = &data_sets[ 0 ]->run_data;

   // Fill in and write out the model file
   US_Model model;

   model.monteCarlo  = false;  // For now
   model.wavelength  = data->wavelength.toDouble();
   model.modelGUID   = US_Util::new_guid();
   model.editGUID    = data->editGUID;
   //model.optics      = ???  How to get this?  Is is needed?
   model.analysis    = US_Model::TWODSA;
   model.global      = US_Model::NONE;   // For now.  Will change later.

   model.description = data->runID + ".2DSA e" + data->editID + 
                       " a" + analysisDate + " " +
                       db_name + "-" + requestID;

   // Save as class variable for later reference
   modelGUID = model.modelGUID;

   for ( int i = 0; i < calculated_solutes[ max_depth ].size(); i++ )
   {
      Solute* solute = &calculated_solutes[ max_depth ][ i ];

      US_Model::SimulationComponent component;
      component.s                    = solute->s;
      component.f_f0                 = solute->k;
      component.signal_concentration = solute->c;

      US_Model::calc_coefficients( component );
      model.components << component;
   }

   QString fn = data->runID + ".2dsa." + model.modelGUID + ".xml";
   model.write( fn );

   // Add the file name of the model file to the output list
   QFile f( "analysis_files.txt" );
   if ( ! f.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append ) )
   {
      abort( "Could not open 'analysis_files.txt' for writing" );
      return;
   }

   QTextStream out( &f );

   int current_run = meniscus_run - 1;

   QString meniscus = QString::number( meniscus_values[ current_run ], 'e', 4 );
   QString variance = QString::number( simulation_values.variance,      'e', 4 );

   out << fn << ";meniscus_value=" << meniscus
             << ";MC_iteration="   << mc_iteration 
             << ";variance="       << variance
             << "\n";
   f.close();
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
      noise.minradius   = data_sets[ 0 ]->run_data.radius( 0 );
      int radii         = data_sets[ 0 ]->run_data.x.size();
      noise.maxradius   = data_sets[ 0 ]->run_data.radius( radii - 1 );
   }
   else
   {
      type_name = "ri";
   }

   noise.description = data->runID + ".2DSA e"  + data->editID + " " + 
                       type_name   + "_noise a" + analysisDate + " " + 
                       db_name     + "-"        + requestID;

   noise.type        = type;
   noise.noiseGUID   = US_Util::new_guid();
   noise.modelGUID   = modelGUID;
   noise.values      = noise_data;
   noise.count       = noise_data.size();

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
