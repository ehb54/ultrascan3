#include "us_mpi_analysis.h"
#include "us_constants.h"
#include "us_math2.h"
#include "us_astfem_rsa.h"
#include "us_simparms.h"

void US_MPI_Analysis::_2dsa_worker( void )
{
   // Start at offset 0 and a daatset size of 1
   _2dsa_worker_loop( 0, 1 );

   // When we get here, all datasets and parameters have been read into memory.
   // First go through each dataset individually and scale the concentrations.
   // Then run monte carlo iterations on the combined data.

/*
   DataSet* current_data;
   int      dataset_size = data_sets.size();

// Note: this won't work for dataset_size > 1
// Need to coordinate this with master
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
         QVector< US_DataIO2::Scan >* scans = 
            &data_sets[ i ]->run_data.scanData;
         
         for ( int j = 0; j < scans->size(); j++ ) //scan
         {
            // Get a pointer to the jth scan in the ith data set
            US_DataIO2::Scan* scan = scans->data() + j;

            for ( int k = 0; k < scan->readings.size(); k++ ) // concentration
               scan->readings[ k ].value /= concentrations[ i ];
         }
      }
   }
*/
}

void US_MPI_Analysis::_2dsa_worker_loop( int offset, int dataset_count )
{
   bool repeat_loop = true;
   MPI_Job     job;
   MPI_Status  status;

   while ( repeat_loop )
   {
      int x[ 3 ];

      MPI_Send( x, // Basically don't care
                3,
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

      meniscus_value = job.meniscus_value;
      offset         = job.dataset_offset;
      dataset_count  = job.dataset_count;

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

         case MPI_Job::NEWDATA:  // Reset data for Monte Carlo calcs
            { 
               mc_data.resize( job.length );

               // This is a receive
               MPI_Bcast( mc_data.data(),
                          job.length,
                          MPI_DOUBLE,
                          MPI_Job::MASTER,
                          MPI_COMM_WORLD2 );

               int index = 0;

               for ( int e = job.dataset_offset; e < job.dataset_count; e++ )
               {
                  US_DataIO2::EditedData* data = &data_sets[ e ]->run_data;

                  int scan_count    = data->scanData.size();
                  int radius_points = data->x.size();

                  for ( int s = 0; s < scan_count; s++ )
                  {
                     US_DataIO2::Scan* scan = &data->scanData[ s ];

                     for ( int r = 0; r < radius_points; r++ )
                     {
                        scan->readings[ r ].value = mc_data[ index++ ];
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
                                      Simulation& simulation_values )
{
   int ti_noise_size = 0;
   int ri_noise_size = 0;
   int total_points  = 0;

   for ( int e = offset; e < offset + dataset_count; e++ )
   {
      int scan_count    = data_sets[ e ]->run_data.scanData.size();
      int radius_points = data_sets[ e ]->run_data.x.size();

      total_points  += scan_count * radius_points;
      ti_noise_size += radius_points;
      ri_noise_size += scan_count;
   }

   QVector< double > ti_noise( ti_noise_size, 0.0 );
   QVector< double > ri_noise( ri_noise_size, 0.0 );

   int solute_count = simulation_values.solutes.size();

   // Contains the simulated data
   QVector< double > nnls_a( total_points * solute_count );

   // Contains the experimental data
   QVector< double > nnls_b( total_points );

   // The solution vector, pre-allocated for nnls
   QVector< double > nnls_x( solute_count );

   int count = 0;

   for ( int e = offset; e < offset + dataset_count; e++ )
   {
      int scan_count    = data_sets[ e ]->run_data.scanData.size();
      int radius_points = data_sets[ e ]->run_data.x.size();

      // Populate the b vector for the NNLS routine with the model
      // function:
      for ( int s = 0; s < scan_count; s++ )
      {
         for ( int r = 0; r < radius_points; r++ )
         {
            nnls_b[ count++ ] =
               data_sets[ e ]->run_data.scanData[ s ].readings[ r ].value;
         }
      }
   }

   count = 0;
   QList< US_DataIO2::RawData > simulations;

   for ( int i = 0; i < solute_count; i++ )
   {
      for ( int e = offset; e < offset + dataset_count; e++ )
      {
         DataSet* data = data_sets[ e ];

         // Create simulation parameters from experimental data
         US_SimulationParameters params;
         params.initFromData( NULL, data->run_data );
         params.simpoints   = data->simpoints;
         params.meshType    = data->radial_grid;
         params.gridType    = data->time_grid;
         
         params.band_volume = data->band_volume;
         if ( params.band_volume > 0.0 ) params.band_forming = true;

         // Calculation of centerpiece bottom - just use 1st rpm for now
         double rpm = params.speed_step[ 0 ].rotorspeed;
         params.bottom = calc_bottom( e, rpm );

         params.meniscus = meniscus_value;
         //params.debug();

         double vbar20 = data->vbar20;
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
         double sum           = 0.0;
         int    scan_count    = data->run_data.scanData.size();
         int    radius_points = data->run_data.x.size();

         for ( int i = 0; i < scan_count; i++ )
            sum += data->run_data.scanData[ i ].temperature;

         double temperature = sum / scan_count;

         // Determine corrections for experimental space
         US_Math2::SolutionData solution;
         solution.density   = data->density;
         solution.viscosity = data->viscosity;
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
         US_AstfemMath::initSimData( sim_data, data->run_data, 0.0 );

         // Run the simulation
         astfem_rsa.calculate( sim_data );

         simulations << sim_data;

         // Populate the A matrix for the NNLS routine with the model function:
         for ( int s = 0; s < scan_count; s++ )
         {
            for ( int r = 0; r < radius_points; r++ )
            {
               nnls_a[ count++ ] = sim_data.value( s, r );
            }
         }
      } // for each dataset
   } // for each solute

   if ( parameters[ "tinoise_option" ].toInt() > 0 )  // Time invariant noise
   {
      // Compute a_tilde, the average experiment signal at each time
      QVector< double > a_tilde( ri_noise_size, 0.0 );

      if ( parameters[ "rinoise_option" ].toInt() > 0 )
      {
         compute_a_tilde( offset, dataset_count, a_tilde );
      }

      // Compute a_bar, the average experiment signal at each radius
      QVector< double > a_bar( ti_noise_size, 0.0 );
      compute_a_bar( offset, dataset_count, a_bar, a_tilde );

      // Compute L_tildes, the average signal at each radius
      QVector< double > L_tildes( ri_noise_size * solute_count, 0.0 );

      if ( parameters[ "rinoise_option" ].toInt() > 0 ) 
      {
         compute_L_tildes( offset, dataset_count, ri_noise_size, total_points,
                           solute_count, L_tildes, nnls_a );
      }

      QVector< double > L_bars( ti_noise_size * solute_count, 0.0 );
      compute_L_bars( offset, dataset_count, solute_count, ri_noise_size, 
                      ri_noise_size, total_points, L_bars, nnls_a, L_tildes );

      // Setup small_a, small_b for the alternate nnls
      QVector< double > small_a( sq( solute_count ), 0.0 );
      QVector< double > small_b( solute_count,       0.0 );

      ti_small_a_and_b(  
            offset, dataset_count, solute_count, total_points, ti_noise_size,
            small_a, small_b, a_bar, L_bars, nnls_a );

      US_Math2::nnls( small_a.data(), solute_count, solute_count, solute_count,
                      small_b.data(),
                      nnls_x.data() );

      // This is Sum( concentration * Lamm ) for the models after NNLS
      QVector< double > L( total_points, 0.0 );
      compute_L( offset, dataset_count, total_points, solute_count,
                 L, nnls_a, nnls_x );

      // Now L contains the best fit sum of L equations
      // Compute L_tilde, the average model signal at each radius
      QVector< double > L_tilde( ri_noise_size, 0.0 );

      if ( parameters[ "rinoise_option" ].toInt() > 0 ) 
      {
         compute_L_tilde( offset, dataset_count, L_tilde, L );
      }

      // Compute L_bar, the average model signal at each radius
      QVector< double > L_bar( ti_noise_size, 0.0 );
      compute_L_bar( offset, dataset_count, L_bar, L, L_tilde );

      // Compute ti_noise
      for ( int i = 0; i < ti_noise_size; i++ )
      {
         ti_noise[ i ] = a_bar[ i ] - L_bar[ i ];
      }

      if ( parameters[ "rinoise_option" ].toInt() > 0 ) 
      {
         // Compute ri_noise, Is this correct?????????
         for ( int i = 0; i < ri_noise_size; i++ )
         {
            ri_noise[ i ] = a_tilde[ i ] - L_tilde[ i ];
         }
      }
   }  // End tinoise and optional rinoise calculation

   else if ( parameters[ "rinoise_option" ].toInt() > 0 )  // ri noise only
   {
      // Compute a_tilde, the average experiment signal at each time
      QVector< double > a_tilde( ri_noise_size, 0.0 );
      compute_a_tilde( offset, dataset_count, a_tilde );

      // Compute L_tildes, the average signal at each radius
      QVector< double > L_tildes( ri_noise_size * solute_count, 0.0 );

      compute_L_tildes( offset, dataset_count, ri_noise_size, total_points,
                        solute_count, L_tildes, nnls_a );

      // Setup small_a, small_b for the nnls
      QVector< double > small_a( sq( solute_count ), 0.0 );
      QVector< double > small_b( solute_count,       0.0 );

      ri_small_a_and_b(  
            offset, dataset_count, solute_count, total_points, ri_noise_size, 
            small_a, small_b, a_tilde, L_tildes, nnls_a );

      US_Math2::nnls( small_a.data(), solute_count, solute_count, solute_count,
                      small_b.data(),
                      nnls_x.data() );
      //
      // This is sum( concentration * Lamm ) for the models after NNLS
      QVector< double > L( total_points, 0.0 );
      
      compute_L( offset, dataset_count, total_points, solute_count,
                 L, nnls_a, nnls_x );

      // Now L contains the best fit sum of L equations
      // Compute L_tilde, the average model signal at each radius
      QVector< double > L_tilde( ri_noise_size, 0.0 );

      compute_L_tilde( offset, dataset_count, L_tilde, L );

      // Compute ri_noise, Is this correct???
      for ( int i = 0; i < ri_noise_size; i++ )
         ri_noise[ i ] = a_tilde[ i ] - L_tilde[ i ];
   }

   else // no ti or ri noise
   {
      US_Math2::nnls( nnls_a.data(), total_points, total_points, solute_count,
                      nnls_b.data(),
                      nnls_x.data() );

      // Note: ti_noise and ri_noise are already zero

   } // End of core calculations

   // Initialize structures.  Note that the elements of both residuals and
   // solution are not really needed at the same time so the solution data
   // structure could be used to store the residuals to save memory.

   residuals.scanData.clear();
   solution .scanData.clear();

   US_DataIO2::Scan scan;

   for ( int e = offset; e < offset + dataset_count; e++ )
   {
      int radius_points = data_sets[ e ]->run_data.x.size();
      int scan_count    = data_sets[ e ]->run_data.scanData.size();
      //int index         = e - offset;
      
      scan.readings.fill( US_DataIO2::Reading(), radius_points );

      solution .scanData.fill( scan, scan_count );
      residuals.scanData.fill( scan, scan_count );
   }

   // Scale readings and store in residuals structure
   for ( int solute = 0; solute < solute_count; solute++ )
   {
      if ( nnls_x[ solute ] != 0.0 )
      {
         for ( int e = offset; e < offset + dataset_count; e++ )
         {
            int radius_points = data_sets[ e ]->run_data.x.size();
            int scan_count    = data_sets[ e ]->run_data.scanData.size();
            //int index         = e - offset;

            // Scale the data
            for ( int s = 0; s < scan_count; s++ )
            {
               for ( int r = 0; r < radius_points; r++ )
               {
                  solution.scanData[ s ].readings[ r ].value +=
                     nnls_x     [ solute ] *
                     simulations[ solute ].value( s, r );
               }
            }
         }

      } // if ( nnls_x[ i ] != 0 )
   } // for each solute

   double variance = 0.0;
   double rmsds[ dataset_count ];

   int ti_noise_offset = 0;
   int ri_noise_offset = 0;

   // Calculate residuals and rmsd values
   for ( int e = offset; e < offset + dataset_count; e++ )
   {
      US_DataIO2::EditedData* data =  &data_sets[ e ]->run_data;
      int radius_points = data->x.size();
      int scan_count    = data->scanData.size();
      int index         = e - offset;

      rmsds[ index ] = 0.0;

      for ( int s = 0; s < scan_count; s++ )
      {
         for ( int r = 0; r < radius_points; r++)
         {
            // TODO: multi experiment
            residuals.scanData[ s ].readings[ r ].value =
              data_sets[ e ]->run_data.value( s, r )   // actual data
            - solution.value( s, r )                   // simulation data
            - ti_noise[ r + ti_noise_offset ]          // time invariant noise
            - ri_noise[ s + ri_noise_offset ];         // radial invariant noise

            double r2 = sq( residuals.value( s, r ) );

            variance       += r2;
            rmsds[ index ] += r2;
         }
      }

      ti_noise_offset += radius_points;
      ri_noise_offset += scan_count;
   }

   variance /= total_points;

   QVector< double > variances( dataset_count );

   for ( int e = offset; e < offset + dataset_count; e++ )
   {
      int radius_points = data_sets[ e ]->run_data.x.size();
      int scan_count    = data_sets[ e ]->run_data.scanData.size();
      int index         = e - offset;

      variances[ index ] = rmsds[ index ] / ( radius_points * scan_count );
   }

   QVector< Solute > solutes = simulation_values.solutes;
   simulation_values.solutes.clear();

   // Store solutes for return
   for ( int i = 0; i < solute_count; i++ )
   {
      if ( nnls_x[ i ] > 0.0 )
      {
         solutes[ i ].c = nnls_x [ i ];
         simulation_values.solutes << solutes[ i ];
      }
   }
   
   simulation_values.variance  = variance;
   simulation_values.variances = variances;
   simulation_values.ti_noise  = ti_noise;
   simulation_values.ri_noise  = ri_noise;
}

double US_MPI_Analysis::calc_bottom( int index, double rpm )
{
   DataSet* d = data_sets[ index ];

   double stretch = d->rotor_stretch[ 0 ] * rpm +
                    d->rotor_stretch[ 1 ] * sq( rpm );

   return d->centerpiece_bottom + stretch;
}

// Compute a_tilde, the average experiment signal at each time
void US_MPI_Analysis:: compute_a_tilde( int                offset, 
                                        int                dataset_count, 
                                        QVector< double >& a_tilde )
{
   int s_offset = 0;

   for ( int e = offset; e < offset + dataset_count; e++ )
   {
      int radius_points = data_sets[ e ]->run_data.x.size();
      int scan_count    = data_sets[ e ]->run_data.scanData.size();

      US_DataIO2::EditedData* data = &data_sets[ e ]->run_data;

      for ( int s = 0; s < scan_count; s++ )
      {
         int s_index = s + s_offset;

         for ( int r = 0; r < radius_points; r++ )
         {
           a_tilde[ s_index ] += data->value( s, r );
         }

         a_tilde[ s_index ] /= radius_points;
      }

      s_offset += scan_count;
   }
}

// Compute L_tildes, the average signal at each radius
void US_MPI_Analysis::compute_L_tildes( int                      offset,  
                                        int                      dataset_count,
                                        int                      ri_noise_size,
                                        int                      total_points,
                                        int                      solute_count,
                                        QVector< double >&       L_tildes,
                                        const QVector< double >& nnls_a )
{
   int exp_offset = 0;
   int s_offset   = 0;

   for ( int e = offset; e < offset + dataset_count; e++ )
   {
      int radius_points = data_sets[ e ]->run_data.x.size();
      int scan_count    = data_sets[ e ]->run_data.scanData.size();;

      for ( int solute = 0; solute < solute_count; solute++ )
      {
         int solute_index = solute * total_points + exp_offset;

         for ( int s = 0; s < scan_count; s++ )
         {
            int index      = solute * ri_noise_size + s + s_offset;
            int scan_index = solute_index + s * radius_points;

            for ( int r = 0; r < radius_points; r++ )
            {
               L_tildes[ index ] += nnls_a[ scan_index + r ];
            }

            L_tildes[ index ] /= radius_points;
         }
      }

      s_offset   += scan_count;
      exp_offset += scan_count * radius_points;
   }
}

// Compute L_tilde, the average model signal at each radius
void US_MPI_Analysis::compute_L_tilde( int                      offset,
                                       int                      dataset_count,
                                       QVector< double >&       L_tilde,
                                       const QVector< double >& L )
{
   int exp_offset = 0;
   int s_offset   = 0;

   for ( int e = offset; e < offset + dataset_count; e++ )
   {
      int radius_points = data_sets[ e ]->run_data.x.size();
      int scan_count    = data_sets[ e ]->run_data.scanData.size();

      for ( int s = 0; s < scan_count; s++ )
      {
         int s_index  = s + s_offset;
         int L_offset = exp_offset + s * radius_points;

         for ( int r = 0; r < radius_points; r++ )
         {
            L_tilde[ s_index ] += L[ L_offset  + r ];
         }

         L_tilde[ s_index ] /= radius_points;
      }

      s_offset   += scan_count;
      exp_offset += scan_count * radius_points;
   }
}

void US_MPI_Analysis::compute_L( int                      offset,
                                 int                      dataset_count,
                                 int                      total_points,
                                 int                      solute_count,
                                 QVector< double >&       L,
                                 const QVector< double >& nnls_a,
                                 const QVector< double >& nnls_x )
{
   for ( int solute = 0; solute < solute_count; solute++ )
   {
      if ( nnls_x[ solute ] > 0 )
      {
         int count      = 0;
         int exp_offset = 0;

         for ( int e = offset; e < offset + dataset_count; e++ )
         {
            int radius_points = data_sets[ e ]->run_data.x.size();
            int scan_count    = data_sets[ e ]->run_data.scanData.size();

            for ( int s = 0; s < scan_count; s++ )
            {
               for ( int r = 0; r < radius_points; r++ )
               {
                  L[ count++ ] +=
                     nnls_x[ solute ] *
                     nnls_a[ solute * total_points +
                             exp_offset +
                             s * radius_points +
                             r
                           ];
               }
            }

            exp_offset += radius_points * scan_count;
         }
      }
   }
}

void US_MPI_Analysis::ri_small_a_and_b( int                      offset,
                                        int                      dataset_count,
                                        int                      solute_count,
                                        int                      total_points,
                                        int                      ri_noise_size,
                                        QVector< double >&       small_a,
                                        QVector< double >&       small_b,
                                        const QVector< double >& a_tilde,
                                        const QVector< double >& L_tildes,
                                        const QVector< double >& nnls_a )
{
   int exp_offset = 0;
   int s_offset   = 0;

   for ( int e = offset; e < offset + dataset_count; e++ )
   {
      int radius_points = data_sets[ e ]->run_data.x.size();
      int scan_count    = data_sets[ e ]->run_data.scanData.size();;

      US_DataIO2::EditedData* run_data = &data_sets[ e ]->run_data;

      for ( int solute = 0; solute < solute_count; solute++ )
      {
         for ( int r = 0; r < radius_points; r++ )
         {
            for ( int s = 0; s < scan_count; s++ )
            {
               int s_index = s + s_offset;

               small_b[ solute ] +=
                  ( run_data->value( s, r ) - a_tilde[ s_index ] )
                  *
                  ( nnls_a[ solute * total_points + 
                            exp_offset +
                            s * radius_points + 
                            r
                          ] 
                    -
                    L_tildes[ solute * ri_noise_size + s_index ]
                  );

               for ( int k = 0; k < solute_count; k++ )
               {
                  small_a[ k * solute_count + solute ] +=
                     ( nnls_a[ k * total_points + 
                               exp_offset + 
                               s * radius_points + 
                               r
                             ]
                       - 
                       L_tildes[ k * ri_noise_size + s_index  ]
                     ) 
                     *
                     ( nnls_a[ solute * total_points + 
                               exp_offset + 
                               s * radius_points + 
                               r
                             ]
                       -  
                       L_tildes[ solute * ri_noise_size + s_index ]
                     );
               }
            }
         }
      }

      exp_offset += radius_points * scan_count;
      s_offset   += scan_count;
   }
}

void US_MPI_Analysis::ti_small_a_and_b( int                      offset,
                                        int                      dataset_count,
                                        int                      solute_count,
                                        int                      total_points,
                                        int                      ti_noise_size,
                                        QVector< double >&       small_a,
                                        QVector< double >&       small_b,
                                        const QVector< double >& a_bar,
                                        const QVector< double >& L_bars,
                                        const QVector< double >& nnls_a )
{
   int exp_offset = 0;
   int r_offset   = 0;

   for ( int e = offset; e < offset + dataset_count; e++ )
   {
      int radius_points = data_sets[ e ]->run_data.x.size();
      int scan_count    = data_sets[ e ]->run_data.scanData.size();

      US_DataIO2::EditedData* run_data = &data_sets[ e ]->run_data;

      for ( int solute = 0; solute < solute_count; solute++ )
      {
         for ( int r = 0; r < radius_points; r++ )
         {
            int r_index = r + r_offset;

            for ( int s = 0; s < scan_count; s++ )
            {
               small_b[ solute ] +=
                  ( run_data->value( s, r ) - a_bar[ r_index ] ) 
                  *
                  ( nnls_a[ solute * total_points +
                            exp_offset +
                            s * radius_points +
                            r
                          ]
                    -
                    L_bars[ solute * ti_noise_size + r_index ]
                  );

               for ( int k = 0; k < solute_count; k++ )
               {
                  small_a[ k * solute_count + solute ] +=
                     ( nnls_a[ k * total_points +
                               exp_offset +
                               s * radius_points +
                               r
                             ]
                       -
                       L_bars[ k * ti_noise_size + r_index ]
                     )
                     *
                     ( nnls_a[ solute * total_points +
                               exp_offset +
                               s * radius_points +
                               r
                             ]
                       -
                       L_bars[ solute * ti_noise_size + r_index ]
                     );
               }
            }
         }
      }

      exp_offset += radius_points * scan_count;
      r_offset   += radius_points;
   }
}

void US_MPI_Analysis::compute_L_bar( int                      offset,
                                     int                      dataset_count,
                                     QVector< double >&       L_bar,
                                     const QVector< double >& L,
                                     const QVector< double >& L_tilde )
{
   int exp_offset = 0;
   int s_offset   = 0;
   int r_offset   = 0;

   for ( int e = offset; e < offset + dataset_count; e++ )
   {
      int radius_points = data_sets[ e ]->run_data.x.size();
      int scan_count    = data_sets[ e ]->run_data.scanData.size();

      for ( int r = 0; r < radius_points; r++)
      {
         int r_index = r + r_offset;

         // Note  L_tilde is always zero when rinoise has not been requested
         for ( int s = 0; s < scan_count; s++ )
         {
            int L_index = exp_offset + s * radius_points + r;
            int s_index = s + s_offset;

            L_bar[ r_index ] += L[ L_index ] - L_tilde[ s_index ];
         }

         L_bar[ r_index ] /= scan_count;
      }

      s_offset   += scan_count;
      r_offset   += radius_points;
      exp_offset += scan_count * radius_points;
   }
}

// Calculate the average measured concentration at each radius point
void US_MPI_Analysis::compute_a_bar( int                      offset,
                                     int                      dataset_count,
                                     QVector< double >&       a_bar,
                                     const QVector< double >& a_tilde )
{
   int s_offset = 0;
   int r_offset = 0;

   for ( int e = offset; e < offset + dataset_count; e++ )
   {
      int radius_points = data_sets[ e ]->run_data.x.size();
      int scan_count    = data_sets[ e ]->run_data.scanData.size();
      
      US_DataIO2::EditedData* data = &data_sets[ e ]->run_data;

      for ( int r = 0; r < radius_points; r++ )
      {
         int r_index = r + r_offset;

         // Note: a_tilde is always zero when rinoise has not been requested
         for ( int s = 0; s < scan_count; s++ )
         {
            a_bar[ r_index ] += data->value( s, r ) - a_tilde[ s + s_offset ];
         }

         a_bar[ r_index ] /= scan_count;
      }

      r_offset += radius_points;
      s_offset += scan_count;
   }
}

// Calculate the average simulated concentration at each radius point
void US_MPI_Analysis::compute_L_bars( int                      offset,
                                      int                      dataset_count,
                                      int                      solute_count,
                                      int                      ri_noise_size,
                                      int                      ti_noise_size,
                                      int                      total_points,
                                      QVector< double >&       L_bars,
                                      const QVector< double >& nnls_a,
                                      const QVector< double >& L_tildes )
{
   int exp_offset = 0;
   int s_offset   = 0;
   int r_offset   = 0;

   for ( int e = offset; e < offset + dataset_count; e++ )
   {
      int radius_points = data_sets[ e ]->run_data.x.size();
      int scan_count    = data_sets[ e ]->run_data.scanData.size();

      for ( int solute = 0; solute < solute_count; solute++ )
      {
         int solute_offset = solute * total_points + exp_offset;

         for ( int r = 0; r < radius_points; r++ )
         {
            int r_index = solute * ti_noise_size + r_offset + r;

            for ( int s = 0; s < scan_count; s++ )
            {
               // Note: L_tildes is always zero when rinoise has not been 
               // requested
               
               int nnls_index = solute_offset + s * radius_points + r;
               int s_index    = solute * ri_noise_size + s_offset + s;

               L_bars[ r_index ] +=
                  nnls_a[ nnls_index ] - L_tildes[ s_index ];
            }

            L_bars[ r_index ] /= scan_count;
         }
      }

      exp_offset += sq( scan_count );
      r_offset   += radius_points;
      s_offset   += scan_count;
   }
}
