#include "us_mpi_analysis.h"
#include "us_constants.h"
#include "us_math2.h"
#include "us_astfem_math.h"
#include "us_astfem_rsa.h"
#include "us_simparms.h"

void US_MPI_Analysis::_2dsa_worker( void )
{
   // When we get here, all datasets and parameters have been read into memory.
   // First go through each dataset individually and scale the concentrations.
   // Then run monte carlo iterations on the combined data.
   
   DataSet* current_data;
   int      dataset_size = data_sets.size();

   for ( int i = 0; i < dataset_size; i++ )
   {
      current_data = data_sets[ i ];  
      _2dsa_worker_loop( i, 1 );
   }

   // If we have more than one dataset, scale all data
   if ( dataset_size > 1 )
   {
      MPI_Barrier( MPI_COMM_WORLD );
      
      double concentrations[ dataset_size ];

      // This is a receive since it is a worker
      MPI_Bcast( concentrations,
                 dataset_size,
                 MPI_DOUBLE2,
                 MPI_Job::MASTER,
                 MPI_COMM_WORLD);

      for ( int i = 0; i < dataset_size; i++ )  // dataset
      {
         QVector< US_DataIO2::Scan >* scans = &data_sets[ i ]->run_data.scanData;
         double concentration = concentrations[ i ];
         
         for ( int j = 0; j < scans->size(); j++ ) //scan
         {
            // Get a pointer to the jth scan in the ith data set
            //US_DataIO2::Scan* scan = &(*scans)[ j ];
            US_DataIO2::Scan* scan = scans->data() + j;

            for ( int k = 0; k < scan->readings.size(); k++ ) // concentration
               scan->readings[ k ].value /= concentration;
         }
      }
   }
return;
   MPI_Datatype ScanType;

   // Receive all concentrations for all scans and all
   // expereiments for each MC iteration after the first.
   for ( int i = 1; i < iterations; i++ )
   {
      for ( int e = 0; e < dataset_size; e++ ) // fore each dataset
      {
         QVector< US_DataIO2::Scan >* scans = &data_sets[ i ]->run_data.scanData;
    
         int scan_length = scans[ 0 ].size();
         MPI_Type_vector( scan_length,                    // count
                          1,                              // blocksize
                          sizeof( US_DataIO2::Reading ),  // stride
                          MPI_DOUBLE2,
                          &ScanType );

         MPI_Type_commit( &ScanType );

         for ( int j = 0; j < scans->size(); j++ ) // for each scan
         {
            // This is a receive since it is a worker
            MPI_Bcast( scans->data() + j,
                       1,
                       ScanType,
                       MPI_Job::MASTER,
                       MPI_COMM_WORLD2 );
         }

         MPI_Type_free( &ScanType );
      }

      MPI_Type_free( &ScanType );
      MPI_Barrier( MPI_COMM_WORLD2 );

      _2dsa_worker_loop( 0, dataset_size );
   }
}

void US_MPI_Analysis::_2dsa_worker_loop( int offset, int dataset_count )
{
   bool repeat_loop = true;
   MPI_Job     job;
   MPI_Status  status;

   while ( repeat_loop )
   {
      int x = 0;

      MPI_Send( &x, // One zero interger -- basically don't care
                1,
                MPI_INT,
                MPI_Job::MASTER,
                MPI_Job::READY,
                MPI_COMM_WORLD2 ); // let master know we are ready

      // Blocking -- Wait for instructions
      MPI_Recv( &job, // get masters' response
                sizeof( job ),
                MPI_BYTE,
                MPI_Job::MASTER,
                MPI_Job::TAG0,
                MPI_COMM_WORLD2,
                &status );        // status not used

      switch( job.command )
      {
         case MPI_Job::PROCESS:  // Process solutes
            {
               Simulation simulation_values;
               simulation_values.solutes.resize( job.length );

               MPI_Recv( simulation_values.solutes.data(), // Get solutes
                         job.length * solute_doubles,
                         MPI_DOUBLE,
                         MPI_Job::MASTER,
                         MPI_Job::TAG0,
                         MPI_COMM_WORLD2,
                         &status );
 
               calc_residuals( offset, dataset_count, simulation_values );

               // Tell master we are sending back results

               int size[ 3 ] = { simulation_values.solutes.size(),
                                 simulation_values.ti_noise.size(),
                                 simulation_values.ri_noise.size() };

               MPI_Send( &size, 
                         3, 
                         MPI_INT,
                         MPI_Job::MASTER,
                         MPI_Job::RESULTS,
                         MPI_COMM_WORLD2 );
               
               // Send back to master all of simulation_values
               MPI_Send( simulation_values.solutes.data(),
                         simulation_values.solutes.size() * solute_doubles, 
                         MPI_DOUBLE,
                         MPI_Job::MASTER,
                         MPI_Job::TAG0,
                         MPI_COMM_WORLD2 );
          
               MPI_Send( &simulation_values.variance,
                         1,
                         MPI_DOUBLE,
                         MPI_Job::MASTER,
                         MPI_Job::TAG0,
                         MPI_COMM_WORLD2 );
          
               MPI_Send( simulation_values.variances.data(),
                         data_sets.size(),
                         MPI_DOUBLE,
                         MPI_Job::MASTER,
                         MPI_Job::TAG0,
                         MPI_COMM_WORLD2 );
          
               MPI_Send( simulation_values.ti_noise.data(),
                         simulation_values.ti_noise.size(),
                         MPI_DOUBLE,
                         MPI_Job::MASTER,
                         MPI_Job::TAG0,
                         MPI_COMM_WORLD2 );
          
               MPI_Send( simulation_values.ri_noise.data(),
                         simulation_values.ri_noise.size(),
                         MPI_DOUBLE,
                         MPI_Job::MASTER,
                         MPI_Job::TAG0,
                         MPI_COMM_WORLD2 );

            }

            break;
    
         case MPI_Job::WAIT:  // Sleep, wait for wakeup
            MPI_Recv( &job,
                      sizeof( job ),
                      MPI_CHAR,
                      MPI_Job::MASTER,
                      MPI_Job::TAG0,
                      MPI_COMM_WORLD2,
                      &status );
            break;
    
         default:
            repeat_loop = false;
            break;
      }  // switch
   }  // repeat_loop
}

void US_MPI_Analysis::calc_residuals( int         offset, 
                                      int         dataset_count, 
                                      Simulation& simulation_values )
{
   QVector< double > ti_noise;
   QVector< double > ri_noise;
   
   int ti_noise_size = 0;
   int ri_noise_size = 0;
   int total_points  = 0;
   
   for ( int e = 0; e < data_sets.size(); e++ )
   {
      int scan_count    = data_sets[ e ]->run_data.scanData.size();
      int radius_points = data_sets[ e ]->run_data.x.size();
      
      total_points  += scan_count * radius_points;
      ti_noise_size += radius_points;
      ri_noise_size += scan_count;
   }
   
   int solute_count = simulation_values.solutes.size();

   // Contains the simulated data
   double* nnls_a = new double[ total_points * solute_count ];

   // Contains the experimental data
   double* nnls_b = new double[ total_points ];
   
   // The solution vector, pre-allocated for nnls
   double* nnls_x = new double[ solute_count ];
   
   int count = 0;

   for ( int e = 0; e < data_sets.size(); e++ )
   {
      int scan_count    = data_sets[ e ]->run_data.scanData.size();
      int radius_points = data_sets[ e ]->run_data.x.size();
      
      // Populate the b vector for the NNLS routine with the model
      // function:
      for ( int s = 0; s < scan_count; s++ )
         for ( int r = 0; r < radius_points; r++ )
            nnls_b[ count++ ] = data_sets[ e ]->run_data.scanData[ s ].readings[ r ].value;
   }

   count = 0;
   QList< US_DataIO2::RawData > simulations;

   for ( int i = 0; i < solute_count; i++ )
   {
      for ( int e = 0; e < data_sets.size(); e++ )
      {
         // Create simulation parameters from experimental data
         US_SimulationParameters params;
         params.initFromData( NULL, data_sets[ e ]->run_data );
         
         // Calculation of centerpiece bottom - just use 1st rpm for now
         double rpm = params.speed_step[ 0 ].rotorspeed;
         params.bottom = calc_bottom( e, rpm );
         //params.debug();

         double vbar20 = data_sets[ e ]->vbar20;
         double s20w   = fabs( simulation_values.solutes[ i ].s );
         double f_f0   = simulation_values.solutes[ i ].k;

         double D20w = R * K20 /
            ( AVOGADRO * 18.0 * M_PI * 
              pow( f_f0 * VISC_20W / 100.0, 3.0 / 2.0 ) *
              sqrt( s20w * vbar20  / 
                    ( 2.0 * ( 1.0 - vbar20 * DENS_20W ) ) 
                  ) 
            );

         // Get the temperature for this run
         double sum   = 0.0;
         int    scans = data_sets[ e ]->run_data.scanData.size();

         for ( int i = 0; i < scans; i++ )
            sum += data_sets[ e ]->run_data.scanData[ i ].temperature;

         double temperature = sum / scans;

         // Determine corrections for experimental space
         US_Math2::SolutionData solution;
         solution.density   = data_sets[ e ]->density;
         solution.viscosity = data_sets[ e ]->viscosity;
         solution.vbar20    = vbar20;
         solution.vbar      = US_Math2::adjust_vbar20( vbar20, temperature );

         US_Math2::data_correction( temperature, solution );

         US_Model                      model;
         US_Model::SimulationComponent component;

         component.s = s20w / solution.s20w_correction;
         component.D = D20w / solution.D20w_correction;
         
         model.components << component;
         //model.debug();

         US_Astfem_RSA astfem_rsa( model, params );
                                  
         // Create raw data structures and set cardinality
         US_DataIO2::RawData sim_data;
         US_AstfemMath::initSimData( sim_data, data_sets[ e ]->run_data, 0.0 );

         // Run the simulation
         astfem_rsa.calculate( sim_data );

         simulations << sim_data;

         // Populate the A matrix for the NNLS routine with the model function:

         int scan_count    = data_sets[ e ]->run_data.scanData.size();
         int radius_points = data_sets[ e ]->run_data.x.size();

         for ( int s = 0; s < scan_count; s++ )
            for ( int r = 0; r < radius_points; r++ )
               nnls_a[ count++ ] = sim_data.scanData[ s ].readings[ r ].value;

      } // for each dataset
   } // for each solute

   // no ti or ri noise
   {
      US_Math2::nnls( nnls_a, total_points, total_points, solute_count,
                      nnls_b, 
                      nnls_x );
      
      // zero ti_noise
      ti_noise.resize( ti_noise_size );
      ti_noise.fill  ( 0.0 );
      
      // no ri_noise for now
      ri_noise.resize( ri_noise_size );
      ri_noise.fill  ( 0.0 );

   } // End of core calculations

   // Clear residuals
   QList< US_AstfemMath::MfemData > residuals;
   US_AstfemMath::MfemData          data;
   US_AstfemMath::MfemScan          scan;

   for ( int e = 0; e < data_sets.size(); e++ )
   {
      residuals << data;
  
      for ( int s = 0; s < data_sets[ e ]->run_data.scanData.size(); s++)
      {
         residuals[ e ].scan << scan;
         residuals[ e ].scan[ s ].conc.fill( 0.0, data_sets[ e ]->run_data.x.size() );
      }
   }

   // Scale readings and store in residuals structure
   for ( int solute = 0; solute < solute_count; solute++ )
   {
      if ( nnls_x[ solute ] != 0.0 )
      {
         for ( int e = 0; e < data_sets.size(); e++ )
         {
            int radius_points = data_sets[ e ]->run_data.x.size();
            int scan_count    = data_sets[ e ]->run_data.scanData.size();
         
            // Scale the data
            for ( int s = 0; s < scan_count; s++ )
               for ( int r = 0; r < radius_points; r++ )
               {
                  residuals[ e ].scan[ s ].conc[ r ] =
                     nnls_x[ solute ] *
                     data_sets[ e ]->run_data.scanData[ s ].readings[ r ].value;
               }
         }

      } // if ( nnls_x[ i ] != 0 )
   } // for each solute

   double variance = 0.0;
   double rmsds[ data_sets.size() ];
   
   unsigned int ti_noise_offset = 0;
   unsigned int ri_noise_offset = 0;
   
   // Calculate residuals and rmsd values
   for ( int e = 0; e < data_sets.size(); e++ )
   {
      int radius_points = data_sets[ e ]->run_data.x.size();
      int scan_count    = data_sets[ e ]->run_data.scanData.size();

      rmsds[ e ] = 0.0;
      
      for ( int s = 0; s < scan_count; s++ )
      {
         for ( int r = 0; r < radius_points; r++)
         {
            residuals[ e ].scan[ s ].conc[ r ] =
              data_sets  [ e ]->run_data.scanData[ s ].readings[ r ].value // actuals
            - residuals[ e ].scan[ s ].conc[ r ]                           // scaled data
            - ti_noise[ r + ti_noise_offset ]                              // noise
            - ri_noise[ s + ri_noise_offset ];
            
            double r2 = sq( residuals[ e ].scan[ s ].conc[ r ] );
            
            variance   += r2;
            rmsds[ e ] += r2;
         }
      }

      ti_noise_offset += radius_points;
      ri_noise_offset += scan_count;
   }

   variance /= total_points;
   
   QVector< double > variances( data_sets.size() );
   
   for ( int e = 0; e < data_sets.size(); e++ )
   {
      int radius_points = data_sets[ e ]->run_data.x.size();
      int scan_count    = data_sets[ e ]->run_data.scanData.size();

      variances[ e ] = rmsds[ e ] / ( radius_points * scan_count );
      rmsds    [ e ] = sqrt( rmsds[ e ] );
   }

   QVector< Solute > solutes = simulation_values.solutes;
   simulation_values.solutes.clear();

   // Store solutes for return
   for ( int i = 0; i < solute_count; i++ )
      if ( nnls_x[ i ] > 0 )
      {
         solutes[ i ].c = nnls_x [ i ];
         simulation_values.solutes << solutes[ i ];
      }
/*
   for ( int i = 0; i < simulation_values.solutes.size(); i++ )
      qDebug() <<                  simulation_values.solutes[ i ].s 
               << QString::number( simulation_values.solutes[ i ].k,          'f', 3 )
               << QString::number( simulation_values.solutes[ i ].c,          'e', 3 );
*/



   simulation_values.variance  = variance;
   simulation_values.variances = variances;
   simulation_values.ti_noise  = ti_noise;
   simulation_values.ri_noise  = ri_noise;

   delete[] nnls_a;
   delete[] nnls_b;
   delete[] nnls_x;
   
   return;
}

double US_MPI_Analysis::calc_bottom( int index, double rpm )
{
   DataSet* d = data_sets[ index ];

   double stretch = d->rotor_stretch[ 0 ] * rpm + 
                    d->rotor_stretch[ 1 ] * sq( rpm );
   
   return d->centerpiece_bottom + stretch;
}
