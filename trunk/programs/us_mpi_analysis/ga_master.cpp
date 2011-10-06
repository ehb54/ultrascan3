#include "us_mpi_analysis.h"
#include "us_util.h"
#include "us_math2.h"
#include "us_settings.h"

void US_MPI_Analysis::ga_master( void )
{
   startTime       = QDateTime::currentDateTime();
   current_dataset = 0;
DbgLv(0) << "master start GA" << startTime;
   // // Tell calc_residuals to use the edited data meniscus value
   //meniscus_value  = -1.0; 

   // Set noise and debug flags
   simulation_values.noisflag   = parameters[ "tinoise_option" ].toInt() > 0 ?
                                  1 : 0;
   simulation_values.noisflag  += parameters[ "rinoise_option" ].toInt() > 0 ?
                                  2 : 0;
   simulation_values.dbg_level  = dbg_level;
   simulation_values.dbg_timing = dbg_timing;
DbgLv(0) << "DEBUG_LEVEL" << simulation_values.dbg_level;

   // Initialize best fitness
   best_genes  .reserve( proc_count );
   best_fitness.reserve( proc_count );

   Fitness empty_fitness;
   empty_fitness.fitness = LARGE;

   Gene working_gene( buckets.count(), US_Solute() );

   // Initialize arrays
   for ( int i = 0; i < proc_count; i++ )
   {
      best_genes << working_gene;

      empty_fitness.index = i;
      best_fitness << empty_fitness;
   }

   QDateTime time = QDateTime::currentDateTime();

   // Handle global fit if needed
   if ( data_sets.size() > 1 )
   {
      for ( int i = 0; i < data_sets.size(); i++ )
      {
         ga_master_loop();
         qSort( best_fitness );
         simulation_values.solutes = best_genes[ best_fitness[ 0 ].index ];

         for ( int g = 0; g < buckets.size(); g++ )
            simulation_values.solutes[ g ].s *= 1.0e-13;

         calc_residuals( current_dataset, 1, simulation_values );

         ga_global_fit();  // Normalize data and update workers
      }
   }

   // Handle Monte Carlo iterations.  There will always be at least 1.
   while ( true )
   {
      ga_master_loop();

      qSort( best_fitness );
      simulation_values.solutes = best_genes[ best_fitness[ 0 ].index ];

     for ( int g = 0; g < buckets.size(); g++ )
         simulation_values.solutes[ g ].s *= 1.0e-13;

DbgLv(1) << "GaMast: sols size" << simulation_values.solutes.size()
 << "buck size" << buckets.size();
DbgLv(1) << "GaMast:   dset size" << data_sets.size();
DbgLv(1) << "GaMast:   sol0.s" << simulation_values.solutes[0].s;
      calc_residuals( 0, data_sets.size(), simulation_values );
DbgLv(1) << "GaMast:    calc_resids return";

      qSort( simulation_values.solutes );

      write_model( simulation_values, US_Model::GA );

DbgLv(1) << "GaMast:  mc_iter iters" << mc_iteration << mc_iterations;
      if ( ++mc_iteration < mc_iterations )
      {
         // Set scaled_data the first time
         if ( mc_iteration == 1 ) 
         {
            scaled_data = simulation_values.sim_data;
         }

DbgLv(1) << "GaMast:    set_gaMC call";
         set_gaMonteCarlo();
DbgLv(1) << "GaMast:    set_gaMC  return";
      }
      else
         break;
   }

   MPI_Job job;

   // Send finish to workers ( in the tag )
   for ( int worker = 1; worker < proc_count; worker++ )
   {
      MPI_Send( &job,              // MPI #0
                sizeof( job ),
                MPI_BYTE,
                worker,
                FINISHED,
                MPI_COMM_WORLD );
   }            
}

void US_MPI_Analysis::ga_master_loop( void )
{
   int    avg_generation       = -1;
   bool   early_termination    = false;
   int    fitness_same_count   = 0;
   double best_overall_fitness = 1.0e99;
   int    tag;
   int    workers              = proc_count - 1;

DbgLv(1) << "master start master loop:  proccount fitsize" << proc_count
   << best_fitness.size();
   // Reset best fitness for each worker
   for ( int i = 0; i < proc_count; i++ )
   {
      best_fitness[ i ].fitness = LARGE;
      best_fitness[ i ].index   = i;
   }

   QList  < Gene > emigres;      // Holds genes passed as emmigrants
   QVector< int  > generations( proc_count, 0 ); 
   int             sum = 0;
   int             avg = 0;
   long            rsstotal = 0L;


   while ( workers > 0 )
   {
      MPI_GA_MSG msg;
      MPI_Status status;
      int        worker;

      MPI_Recv( &msg,          // Get a message   MPI #1
                sizeof( msg ),
                MPI_BYTE,
                MPI_ANY_SOURCE,
                MPI_ANY_TAG,
                MPI_COMM_WORLD,
                &status );

      worker = status.MPI_SOURCE;

QString g;
QString s;

      max_rss();

      switch ( status.MPI_TAG )
      {
         case GENERATION:
            generations[ worker ] = msg.generation;

            sum = 0;
            for ( int i = 1; i < proc_count; i++ ) 
               //sum += generations[ worker ];
               sum += generations[ i ];

            avg = qRound( sum / ( proc_count - 1 ) );

            if ( avg > avg_generation )
            {
               avg_generation = avg;
            
               QString progress =
                  "Avg Generation: "  + QString::number( avg_generation ) +
                  "; MonteCarlo: " + QString::number( mc_iteration );

               send_udp( progress );
            }

            // Get the best gene for the current generation from the worker
            MPI_Recv( best_genes[ worker ].data(),     // MPI #2
                      buckets.size() * solute_doubles,
                      MPI_DOUBLE,  
                      worker,
                      GENE,
                      MPI_COMM_WORLD,
                      MPI_STATUS_IGNORE );

            if ( msg.fitness < best_fitness[ worker ].fitness )
               best_fitness[ worker ].fitness = msg.fitness;

g = "";
for ( int i = 0; i < buckets.size(); i++ )
    g += s.sprintf( "(%.3f,%.3f)", best_genes[ worker ][ i ].s, best_genes[ worker ][ i ].k);

DbgLv(1) << "master: worker/fitness/best gene" << worker <<  msg.fitness << g;

            max_rss();

            static const double fitness_threshold = 1.0e-8;
            static const int    max_same_count    = ( proc_count - 1 ) * 5;
            static const int    min_generation    = 10;

            best_overall_fitness = qMin( best_overall_fitness, msg.fitness );

            if ( ( msg.fitness - best_overall_fitness ) < fitness_threshold )
               fitness_same_count++;
            else
               fitness_same_count = 0;

            if ( ! early_termination )
            {
               if ( fitness_same_count > max_same_count  &&
                    avg_generation     > min_generation )
               {
                  DbgLv(0) << "Fitness has not improved in the last" << fitness_same_count
                     << "deme results - Early Termination.";
                  early_termination = true;
               }
            }
DbgLv(1) << "  best_overall_fitness" << best_overall_fitness << "fitness_same_count"
 << fitness_same_count << " early_term?" << early_termination;
            
            tag = early_termination ? FINISHED : GENERATION; 

            // Tell the worker to either continue or stop
            MPI_Send( &msg,            // MPI #3
                      0,               // Only interested in the tag 
                      MPI_BYTE,  
                      worker,
                      tag,
                      MPI_COMM_WORLD );
            break;

         case FINISHED:
            rsstotal += (long)msg.size;
            workers--;
            break;

         case EMMIGRATE:
         {
            // First get a set of genes as a concatenated vector.  
            int               gene_count    = msg.size;
            int               doubles_count = gene_count * buckets.size() * 
                                              solute_doubles;
            QVector< double > emmigrants( doubles_count ) ;

            MPI_Recv( emmigrants.data(),  // MPI #4
                      doubles_count,
                      MPI_DOUBLE,
                      worker,
                      EMMIGRATE,
                      MPI_COMM_WORLD,
                      MPI_STATUS_IGNORE );

            // Add the genes to the emmigres list
            int solute = 0;

            for ( int i = 0; i < gene_count; i++ )
            {
               Gene gene;

               for ( int b = 0; b < buckets.size(); b++ )
               {
                  double s = emmigrants[ solute++ ];
                  double k = emmigrants[ solute++ ];
                  gene << US_Solute( s, k );
                  solute++; // Concentration         
               }

               emigres << gene;
            }

            max_rss();

            // Don't send any back if the pool is too small
            if ( emigres.size() < gene_count * 5 ) doubles_count = 0;

            // Get immigrents from emmigres
            QVector< US_Solute > immigrants;

            if ( doubles_count > 0 )
            {
               // Prepare a vector of concatenated genes form the emmigrant list
               for ( int i = 0; i < gene_count; i++ )
                  immigrants += emigres.takeAt( u_random( emigres.size() ) );
            }

            MPI_Send( immigrants.data(),   // MPI #5
                      doubles_count,
                      MPI_DOUBLE,
                      worker,
                      IMMIGRATE,
                      MPI_COMM_WORLD );
            break;
         }
      }

      max_rss();
   }

DbgLv(1) << "Master maxrss" << maxrss << " worker total rss" << rsstotal;
   maxrss += rsstotal;

   if ( early_termination )
   {
      DbgLv(0) << "Early termination at average generation" << avg;
   }
}

void US_MPI_Analysis::ga_global_fit( void ) 
{
   // This is almost the same as 2dsa global_fit.
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
   job.length         = scaled_data.size();
   job.dataset_offset = current_dataset;
   job.dataset_count  = 1;

   // Tell each worker that new data coming
   // Can't use a broadcast because the worker is expecting a Send

   for ( int worker = 1; worker < proc_count; worker++ )
   {
      MPI_Send( &job,                   // MPI #7
          sizeof( MPI_Job ), 
          MPI_BYTE,
          worker,   
          UPDATE,
          MPI_COMM_WORLD );
   }

   // Get everybody synced up
   MPI_Barrier( MPI_COMM_WORLD );

   MPI_Bcast( scaled_data.data(),      // MPI #8
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
}

void US_MPI_Analysis::set_gaMonteCarlo( void ) 
{
DbgLv(1) << "sgMC: mciter" << mc_iteration;
   // This is almost the same as 2dsa set_monteCarlo
   if ( mc_iteration == 1 )
   {
      //meniscus_values << -1.0;
      max_depth = 0;  // Make the datasets compatible
      calculated_solutes.clear();
      calculated_solutes << best_genes[ best_fitness[ 0 ].index ];
DbgLv(1) << "sgMC: bfgenes stored" << calculated_solutes[0].size();

      for ( int i = 0; i < calculated_solutes[ 0 ].size(); i++ )
         calculated_solutes[ 0 ][ i ].s *= 1.0e-13;

DbgLv(1) << "sgMC:  sol0 s" << calculated_solutes[0][0].s;
      set_gaussians();
DbgLv(1) << "sgMC: gaussians set";
   }

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
            mc_data[ index ] = scaled_data.value( s, r ) + variation;
            index++;
         }
      }
   }
DbgLv(1) << "sgMC: mc_data set index" << index;

   // Broadcast Monte Carlo data to all workers
   MPI_Job job;
   job.command        = MPI_Job::NEWDATA;
   job.length         = total_points;
   job.dataset_offset = 0;
   job.dataset_count  = data_sets.size();
DbgLv(1) << "sgMC: MPI send proc_count" << proc_count;

   // Tell each worker that new data coming
   // Can't use a broadcast because the worker is expecting a Send
   for ( int worker = 1; worker < proc_count; worker++ )
   {
      MPI_Send( &job,         // MPI #9
          sizeof( job ), 
          MPI_BYTE,
          worker,   
          UPDATE,
          MPI_COMM_WORLD );
   }

   // Get everybody synced up
DbgLv(1) << "sgMC: MPI Barrier";
   MPI_Barrier( MPI_COMM_WORLD );

DbgLv(1) << "sgMC: MPI Bcast";
   MPI_Bcast( mc_data.data(),   // MPI #10
              total_points, 
              MPI_DOUBLE, 
              MPI_Job::MASTER, 
              MPI_COMM_WORLD );
}

void US_MPI_Analysis::write_model( const US_SolveSim::Simulation& sim, 
                                   US_Model::AnalysisType         type )
{
   US_DataIO2::EditedData* data = &data_sets[ 0 ]->run_data;

   // Fill in and write out the model file
   US_Model model;

   model.monteCarlo  = mc_iterations > 1;
   model.wavelength  = data->wavelength.toDouble();
   model.modelGUID   = US_Util::new_guid();
   model.editGUID    = data->editGUID;
   model.requestGUID = requestGUID;
   //model.optics      = ???  How to get this?  Is is needed?
   model.analysis    = type;

   if ( meniscus_points > 1 ) 
       model.global = US_Model::MENISCUS;
   else if ( data_sets.size() > 1 )
       model.global = US_Model::GLOBAL;
   else
       model.global = US_Model::NONE; 

   model.meniscus    = meniscus_values[ meniscus_run ];
   model.variance    = sim.variance;

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

   if ( mc_iterations > 1 )
      iterID.sprintf( "mc%04d", mc_iteration + 1 );
   else if (  meniscus_points > 1 )
      iterID.sprintf( "i%02d-m%05d", 
              meniscus_run + 1,
              (int)(meniscus_values[ meniscus_run ] * 10000 ) );
   else
      iterID = "i01";

   QString id = model.typeText();

   QString analysisID = dates + "_" + id + "_" + requestID + "_" + iterID;

   model.description = data->runID + "." + tripleID + "." + analysisID + ".model";

   // Save as class variable for later reference
   modelGUID = model.modelGUID;

   for ( int i = 0; i < sim.solutes.size(); i++ )
   {
      const US_Solute* solute = &sim.solutes[ i ];

      US_Model::SimulationComponent component;
      component.s                    = solute->s;
      component.f_f0                 = solute->k;
      component.signal_concentration = solute->c;
      component.name                 = QString().sprintf( "SC%04d", i + 1 );
      component.vbar20               = data_sets[ 0 ]->vbar20;

      US_Model::calc_coefficients( component );
      model.components << component;
   }

   QString fn = data->runID + "." + id + "." + model.modelGUID + ".xml";
   model.write( fn );

   // Add the file name of the model file to the output list
   QFile f( "analysis_files.txt" );

   if ( ! f.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append ) )
   {
      abort( "Could not open 'analysis_files.txt' for writing" );
      return;
   }

   QTextStream out( &f );

   QString meniscus = QString::number( meniscus_value, 'e', 4 );
   QString variance = QString::number( sim.variance,   'e', 4 );

   int run = 1;

   if ( meniscus_run > 0 ) 
       run = meniscus_run + 1;
   else if ( mc_iterations > 0 )
       run = mc_iteration + 1;

   QString runstring = "Run: " + QString::number( run ) + " " + tripleID;

   out << fn << ";meniscus_value=" << meniscus_value
             << ";MC_iteration="   << mc_iteration
             << ";variance="       << sim.variance
             << ";run="            << runstring
             << "\n";
   f.close();
}
