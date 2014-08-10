#include "us_mpi_analysis.h"
#include "us_util.h"
#include "us_math2.h"
#include "us_settings.h"

void US_MPI_Analysis::ga_master( void )
{
   current_dataset     = 0;
   datasets_to_process = data_sets.size();
   max_depth           = 0;
   calculated_solutes.clear();

   // Set noise and debug flags
   simulation_values.noisflag   = parameters[ "tinoise_option" ].toInt() > 0 ?
                                  1 : 0;
   simulation_values.noisflag  += parameters[ "rinoise_option" ].toInt() > 0 ?
                                  2 : 0;
   simulation_values.dbg_level  = dbg_level;
   simulation_values.dbg_timing = dbg_timing;
DbgLv(0) << "DEBUG_LEVEL" << simulation_values.dbg_level;

   // Initialize best fitness
   best_genes  .reserve( gcores_count );
   best_fitness.reserve( gcores_count );

   Fitness empty_fitness;
   empty_fitness.fitness = LARGE;

   Gene working_gene( buckets.count(), US_Solute() );

   // Initialize arrays
   for ( int i = 0; i < gcores_count; i++ )
   {
      best_genes << working_gene;

      empty_fitness.index = i;
      best_fitness << empty_fitness;
   }

   QDateTime time = QDateTime::currentDateTime();

   // Handle Monte Carlo iterations.  There will always be at least 1.
   while ( true )
   {
      ga_master_loop();

      qSort( best_fitness );
      simulation_values.solutes = best_genes[ best_fitness[ 0 ].index ];
      int nisols      = simulation_values.solutes.size();
DbgLv(1) << "GaMast: sols size" << nisols << "buck size" << buckets.size()
 << "dset size" << data_sets.size();
DbgLv(1) << "GaMast:  sol0.s .k .v .d" << simulation_values.solutes[0].s
 << simulation_values.solutes[0].k << simulation_values.solutes[0].v
 << simulation_values.solutes[0].d;

      solutes_from_gene( simulation_values.solutes, nisols );
DbgLv(1) << "GaMast:   osol0.s .k .v .d" << simulation_values.solutes[0].s
 << simulation_values.solutes[0].k << simulation_values.solutes[0].v
 << simulation_values.solutes[0].d;

      calc_residuals( 0, data_sets.size(), simulation_values );
DbgLv(1) << "GaMast:    calc_resids return - calcsize vari"
 << simulation_values.solutes.size() << simulation_values.variance;
DbgLv(1) << "GaMast:    csol0.s .k .v .d" << simulation_values.solutes[0].s
 << simulation_values.solutes[0].k << simulation_values.solutes[0].v
 << simulation_values.solutes[0].d;

      qSort( simulation_values.solutes );

      // Convert given solute points to s,k for model output
      double vbar20  = data_sets[ 0 ]->vbar20;
      QList< int > attrxs;
      attrxs << attr_x << attr_y << attr_z;
      bool   have_s  = ( attrxs.indexOf( ATTR_S ) >= 0 );
      bool   have_k  = ( attrxs.indexOf( ATTR_K ) >= 0 );
      bool   have_w  = ( attrxs.indexOf( ATTR_W ) >= 0 );
      bool   have_d  = ( attrxs.indexOf( ATTR_D ) >= 0 );
      bool   have_f  = ( attrxs.indexOf( ATTR_F ) >= 0 );
      bool   vary_v  = ( attr_z != ATTR_V );

      for ( int gg = 0; gg < simulation_values.solutes.size(); gg++ )
      {
         US_Solute* solu   = &simulation_values.solutes[ gg ];
         US_Model::SimulationComponent mcomp;
         mcomp.s        = have_s ? solu->s : 0.0;
         mcomp.f_f0     = have_k ? solu->k : 0.0;
         mcomp.mw       = have_w ? solu->d : 0.0;
         mcomp.vbar20   = vary_v ? solu->v : vbar20;
         mcomp.D        = have_d ? solu->d : 0.0;
         mcomp.f        = have_f ? solu->d : 0.0;

         US_Model::calc_coefficients( mcomp );

         solu->s        = mcomp.s;
         solu->k        = mcomp.f_f0;
         solu->v        = mcomp.vbar20;
      }
         
      calculated_solutes.clear();
      calculated_solutes << simulation_values.solutes;

      if ( data_sets.size() == 1 )
      {
         write_output();
      }
      else
      {
         write_global();
      }

DbgLv(1) << "GaMast:  mc_iter iters" << mc_iteration << mc_iterations;
      mc_iteration++;
      if ( mc_iterations > 1 )
      {
         qDebug() << "Fit RMSD" << sqrt( simulation_values.variance )
            << " of MC_Iteration" << mc_iteration;
         if ( mc_iteration < mc_iterations )
         {
            // Set scaled_data the first time
            if ( mc_iteration == 1 ) 
            {
               scaled_data = simulation_values.sim_data;
            }

            time_mc_iterations();

DbgLv(1) << "GaMast:    set_gaMC call";
            set_gaMonteCarlo();
DbgLv(1) << "GaMast:    set_gaMC  return";
         }
         else
            break;
      }
      else
      {
         qDebug() << "Final Fit RMSD" << sqrt( simulation_values.variance );
         break;
      }
   }

   DbgLv(0) << my_rank << ": Master signalling FINISHED to all Demes";

   MPI_Job job;

   // Send finish to workers ( in the tag )
   for ( int worker = 1; worker <= my_workers; worker++ )
   {
      MPI_Send( &job,              // MPI #0
                sizeof( job ),
                MPI_BYTE,
                worker,
                FINISHED,
                my_communicator );
   }            
}

void US_MPI_Analysis::ga_master_loop( void )
{
//   static const double fit_div        = 1.0e-9;
//   static const double fit_mul        = 1.0e+9;
   static const double DIGIT_FIT      = 1.0e+4;
   static const int    max_same_count = my_workers * 5;
   static const int    min_generation = 10;
   int    avg_generation       = 0;
   bool   early_termination    = false;
   int    fitness_same_count   = 0;
   double best_overall_fitness = LARGE;
   int    tag;
   int    workers              = my_workers;
DbgLv(1) << "ga_master start loop:  gcores_count fitsize" << gcores_count
   << best_fitness.size() << "best_overall" << best_overall_fitness;

   // Reset best fitness for each worker
   for ( int i = 0; i < gcores_count; i++ )
   {
      best_fitness[ i ].fitness = LARGE;
      best_fitness[ i ].index   = i;
   }

   QList  < Gene > emigres;      // Holds genes passed as emmigrants
   QVector< int  > v_generations( gcores_count, 0 ); 
   int             sum = 0;
   int             avg = 0;
   long            rsstotal = 0L;
   double          fit_power      = 5;
   double          fit_digit      = 1.0e4;
   double          fitness_round  = 1.0e5;


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
                my_communicator,
                &status );

      worker = status.MPI_SOURCE;

QString g;
QString s;

      max_rss();

      switch ( status.MPI_TAG )
      {
         case GENERATION:
            v_generations[ worker ] = msg.generation;

            sum = 0;
            for ( int i = 1; i <= my_workers; i++ ) 
               sum += v_generations[ i ];

            avg = qRound( sum / my_workers ) + 1;

            if ( avg > avg_generation )
            {
               avg_generation = avg;
               int mc_iter    = mgroup_count < 2 ? ( mc_iteration + 1 )
                                                 : mc_iteration;

               QString progress =
                  "Avg. Generation: "  + QString::number( avg_generation );

               if ( data_sets.size() > 1 )
               {
                  if ( datasets_to_process == 1 )
                     progress += "; Dataset: "
                              + QString::number( current_dataset + 1 );
                  else
                     progress += "; Datasets: "
                              + QString::number( datasets_to_process );
               }

               progress += "; MonteCarlo: " + QString::number( mc_iter );

               send_udp( progress );
            }

            // Get the best gene for the current generation from the worker
            MPI_Recv( best_genes[ worker ].data(),     // MPI #2
                      buckets.size() * solute_doubles,
                      MPI_DOUBLE,  
                      worker,
                      GENE,
                      my_communicator,
                      MPI_STATUS_IGNORE );

            max_rss();

            // Compute a current-deme best fitness value that is rounded
            //  to 4 significant digits
            fit_power      = (double)qRound( log10( msg.fitness ) );
            fit_digit      = pow( 10.0, -fit_power ) * DIGIT_FIT;
            fitness_round  = (double)qRound64( msg.fitness * fit_digit )
                             / fit_digit;

DbgLv(1) << "  MAST: work" << worker << "fit msg,round,bestw,besto"
 << msg.fitness << fitness_round << best_fitness[worker].fitness
 << best_overall_fitness;
            // Set deme's best fitness
            if ( fitness_round < best_fitness[ worker ].fitness )
               best_fitness[ worker ].fitness = fitness_round;
g = "";
for ( int i = 0; i < buckets.size(); i++ )
  g += s.sprintf( "(%.3f,%.3f)", best_genes[ worker ][ i ].s, best_genes[ worker ][ i ].k);
DbgLv(1) << "master: worker/fitness/best gene" << worker <<  msg.fitness << g;

            if ( ! early_termination )
            {  // Handle normal pre-early-termination updates
               if ( avg_generation == 1  &&  mc_iterations == 1  &&
                   best_overall_fitness == LARGE )
               {  // Report first best-fit RMSD
                  DbgLv(0) << "First Best Fit RMSD" << sqrt( fitness_round );
               }
DbgLv(1) << "  MAST: work" << worker << "fit besto,round" << best_overall_fitness << fitness_round
 << "fit_power fit_digit msgfit" << fit_power << fit_digit << msg.fitness;

               if ( fitness_round < best_overall_fitness )
               {  // Update over-all best fitness value (rounded)
                  best_overall_fitness = fitness_round;
                  fitness_same_count   = 0;
               }
               else
               {  // Bump the count of consecutive same best overall fitness
                  fitness_same_count++;
               }


               if ( fitness_same_count > max_same_count  &&
                    avg_generation     > min_generation )
               {  // Mark early termination at threshold same-fitness count
                  DbgLv(0) << "Fitness has not improved in the last"
                     << fitness_same_count
                     << "deme results - Early Termination.";
                  early_termination = true;
               }

            }
DbgLv(1) << "  best_overall_fitness" << best_overall_fitness
 << "fitness_same_count" << fitness_same_count
 << " early_term?" << early_termination;

            // Tell the worker to either stop or continue
            tag = early_termination ? FINISHED : GENERATION; 

            MPI_Send( &msg,            // MPI #3
                      0,               // Only interested in the tag 
                      MPI_BYTE,  
                      worker,
                      tag,
                      my_communicator );
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
                      my_communicator,
                      MPI_STATUS_IGNORE );

            // Add the genes to the emmigres list
            int solute = 0;
            int solinc = solute_doubles - 2;

            for ( int i = 0; i < gene_count; i++ )
            {
               Gene gene;

               for ( int b = 0; b < buckets.size(); b++ )
               {
                  double s = emmigrants[ solute++ ];
                  double k = emmigrants[ solute++ ];
                  gene << US_Solute( s, k );
                  solute  += solinc; // Concentration, Vbar, DiffCoeff
               }

               emigres << gene;
            }
//*DEBUG*
//if(emigres[0][0].s<0.0)
// DbgLv(0) << "MAST: **GENE s" << emigres[0][0].s << " Emigrant";
//*DEBUG*

            max_rss();

            // Don't send any back if the pool is too small
            if ( emigres.size() < gene_count * 5 ) doubles_count = 0;

            // Get immigrants from emmigres
            QVector< US_Solute > immigrants;

            if ( doubles_count > 0 )
            {
               // Prepare a vector of concatenated genes from the emmigrant list
               for ( int i = 0; i < gene_count; i++ )
                  immigrants += emigres.takeAt( u_random( emigres.size() ) );
            }

            MPI_Send( immigrants.data(),   // MPI #5
                      doubles_count,
                      MPI_DOUBLE,
                      worker,
                      IMMIGRATE,
                      my_communicator );
//*DEBUG*
//if(immigrants[0].s<0.0)
// DbgLv(0) << "MAST: **GENE s" << immigrants[0].s << " Immigrant-to-send";
//*DEBUG*
            break;
         }
      }

      max_rss();
   }

DbgLv(1) << "Master maxrss" << maxrss << " worker total rss" << rsstotal
 << "rank" << my_rank;
   maxrss += rsstotal;

   if ( early_termination )
   {  // Report when we have reached early termination of generations
      int mc_iter  = mgroup_count < 2 ? ( mc_iteration + 1 ) : mc_iteration;
      DbgLv(0) << "Early termination at average generation" << avg
         << ", MC" << mc_iter;
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
   US_DataIO::EditedData* data = &data_sets[ current_dataset ]->run_data;

   int scan_count    = data->scanCount();
   int radius_points = data->pointCount();
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

   for ( int worker = 1; worker <= my_workers; worker++ )
   {
      MPI_Send( &job,                   // MPI #7
          sizeof( MPI_Job ), 
          MPI_BYTE,
          worker,   
          UPDATE,
          my_communicator );
   }

   // Get everybody synced up
   MPI_Barrier( my_communicator );

   MPI_Bcast( scaled_data.data(),      // MPI #8
              scaled_data.size(), 
              MPI_DOUBLE, 
              MPI_Job::MASTER, 
              my_communicator );

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
      max_depth   = 0;  // Make the datasets compatible
      calculated_solutes.clear();
      calculated_solutes << best_genes[ best_fitness[ 0 ].index ];
      int ncsols  = calculated_solutes[ 0 ].size();
DbgLv(1) << "sgMC: bfgenes stored" << ncsols;

      solutes_from_gene( calculated_solutes[ 0 ], ncsols );

DbgLv(1) << "sgMC:  sol0 s" << calculated_solutes[0][0].s;
      set_gaussians();
DbgLv(1) << "sgMC: gaussians set";
   }

   mc_data.resize( total_points );
   int index = 0;

   // Get a randomized variation of the concentrations
   // Use a gaussian distribution with the residual as the standard deviation
   for ( int e = 0; e < data_sets.size(); e++ )
   {
      US_DataIO::EditedData* data = &data_sets[ e ]->run_data;

      int scan_count    = data->scanCount();
      int radius_points = data->pointCount();

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
DbgLv(1) << "sgMC: MPI send my_workers" << my_workers;

   // Tell each worker that new data coming
   // Can't use a broadcast because the worker is expecting a Send
   for ( int worker = 1; worker <= my_workers; worker++ )
   {
      MPI_Send( &job,         // MPI #9
          sizeof( job ), 
          MPI_BYTE,
          worker,   
          UPDATE,
          my_communicator );
   }

   // Get everybody synced up
DbgLv(1) << "sgMC: MPI Barrier";
   MPI_Barrier( my_communicator );

DbgLv(1) << "sgMC: MPI Bcast";
   MPI_Bcast( mc_data.data(),   // MPI #10
              total_points, 
              MPI_DOUBLE, 
              MPI_Job::MASTER, 
              my_communicator );
}

void US_MPI_Analysis::write_model( const US_SolveSim::Simulation& sim, 
                                   US_Model::AnalysisType         type,
                                   bool                           glob_sols )
{
   US_DataIO::EditedData* edata = &data_sets[ current_dataset ]->run_data;

   // Fill in and write out the model file
   US_Model model;

DbgLv(1) << "wrMo: type" << type << "(DMGA)" << US_Model::DMGA;
   if ( type == US_Model::DMGA )
   {  // For discrete GA, get the already constructed model
      model             = data_sets[ 0 ]->model;
DbgLv(1) << "wrMo:  model comps" << model.components.size();
   }

   model.monteCarlo  = mc_iterations > 1;
   model.wavelength  = edata->wavelength.toDouble();
   model.modelGUID   = US_Util::new_guid();
   model.editGUID    = edata->editGUID;
   model.requestGUID = requestGUID;
   model.dataDescrip = edata->description;
   //model.optics      = ???  How to get this?  Is is needed?
   model.analysis    = type;
   QString runID     = edata->runID;

   if ( meniscus_points > 1 ) 
      model.global      = US_Model::MENISCUS;

   else if ( data_sets.size() > 1 )
   {
      model.global      = US_Model::GLOBAL;
      if ( glob_sols )
         runID             = "Global-" + runID;
   }

   else
      model.global      = US_Model::NONE; 

   model.meniscus    = meniscus_value;
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

   QString tripleID = edata->cell + edata->channel + edata->wavelength;
   QString dates    = "e" + edata->editID + "_a" + analysisDate;
DbgLv(1) << "wrMo: tripleID" << tripleID << "dates" << dates;

   QString iterID;
   int mc_iter      = mgroup_count < 2 ? ( mc_iteration + 1 ) : mc_iteration;

   if ( mc_iterations > 1 )
      iterID.sprintf( "mc%04d", mc_iter );
   else if (  meniscus_points > 1 )
      iterID.sprintf( "i%02d-m%05d", 
              meniscus_run + 1,
              (int)(meniscus_value * 10000 ) );
   else
      iterID = "i01";

   QString id        = model.typeText();
   if ( analysis_type.contains( "CG" ) )
      id                = id.replace( "2DSA", "2DSA-CG" );
   QString analyID   = dates + "_" + id + "_" + requestID + "_" + iterID;
   int     stype     = data_sets[ current_dataset ]->solute_type;
   double  vbar20    = data_sets[ current_dataset ]->vbar20;

   model.description = runID + "." + tripleID + "." + analyID + ".model";
DbgLv(1) << "wrMo: model descr" << model.description;

   // Save as class variable for later reference
   modelGUID         = model.modelGUID;

   if ( type != US_Model::DMGA )
   {  // For non-DMGA, construct the model from solutes
      for ( int i = 0; i < sim.solutes.size(); i++ )
      {
         const US_Solute* solute = &sim.solutes[ i ];

         US_Model::SimulationComponent component;
         component.s                    = solute->s;
         component.f_f0                 = solute->k;
         component.signal_concentration = solute->c;
         component.name                 = QString().sprintf( "SC%04d", i + 1 );
         component.vbar20               = (attr_z == ATTR_V) ? vbar20 : solute->v;

         US_Model::calc_coefficients( component );
         model.components << component;
      }
   }
DbgLv(1) << "wrMo: stype" << stype << QString().sprintf("0%o",stype)
 << "attr_z vbar20 mco0.v" << attr_z << vbar20 << model.components[0].vbar20;

   QString fn        = edata->runID + "." + id + "." + model.modelGUID + ".xml";
   int lenfn         = fn.length();

   if ( lenfn > 99 )
   { // Insure a model file name less than 100 characters in length (tar limit)
      int lenri         = edata->runID.length() + 99 - lenfn;
      fn                = edata->runID.left( lenri )
                          + "." + id + "." + model.modelGUID + ".xml";
   }

   model.write( fn );                // Output the model to a file

   data_sets[ current_dataset ]->model = model;    // Save the model in case needed for noise

   // Add the file name of the model file to the output list
   QFile fileo( "analysis_files.txt" );

   if ( ! fileo.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append ) )
   {
      abort( "Could not open 'analysis_files.txt' for writing" );
      return;
   }

   QTextStream tsout( &fileo );

   QString meniscus = QString::number( meniscus_value, 'e', 4 );
   QString variance = QString::number( sim.variance,   'e', 4 );

   int run     = 1;
   mc_iter     = mgroup_count < 2 ? ( mc_iteration + 1 ) : mc_iteration;

   if ( meniscus_run > 0 ) 
       run        = meniscus_run + 1;
   else if ( mc_iterations > 0 )
       run        = mc_iter;

   QString runstring = "Run: " + QString::number( run ) + " " + tripleID;

   tsout << fn << ";meniscus_value=" << meniscus_value
               << ";MC_iteration="   << mc_iter
               << ";variance="       << sim.variance
               << ";run="            << runstring
               << "\n";
   fileo.close();
}

