#include "us_mpi_analysis.h"
#include "us_math2.h"
#include "us_util.h"
#include "us_tar.h"
#include "us_astfem_rsa.h"
#include "us_simparms.h"
#include "us_constants.h"

void US_MPI_Analysis::_2dsa_master( void )
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
      meniscus_value   = meniscus_values.size() == 1 ?
                         data_sets[ current_dataset ]->run_data.meniscus :
                         meniscus_values[ meniscus_run ];
//if ( max_depth > 1 )
// DbgLv(1) << " master loop-TOP:  jq-empty?" << job_queue.isEmpty() << "   areReady?" << worker_status.contains(READY)
//    << "  areWorking?" << worker_status.contains(WORKING);

      // Give the jobs to the workers
      while ( ! job_queue.isEmpty()  &&  worker_status.contains( READY ) )
      {
         worker    = ready_worker();

         Sa_Job job              = job_queue.takeFirst();
         submit( job, worker );
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
                            + " (" + tripleID + ") of "
                            + QString::number( count_datasets );

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

         // Fit meniscus 
         if ( ( meniscus_run + 1 ) < meniscus_values.size() )
         {
            set_meniscus(); 
         }

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

DbgLv(1) << " master loop-BOT:    cds kds" << current_dataset << count_datasets;
            if ( current_dataset < count_datasets )
            {
               meniscus_run    = 0;
               iterations      = 1;
               mc_iteration    = 0;

               if ( meniscus_points > 1 )
               {  // Reset the range of fit-meniscus points for this data set
                  US_DataIO::EditedData* edata
                                  = &data_sets[ current_dataset ]->run_data;
                  double men_str  = edata->meniscus - meniscus_range / 2.0;
                  double men_inc  = meniscus_range / ( meniscus_points - 1.0 );
                  double dat_str  = edata->radius( 0 );
                  double men_end  = men_str + meniscus_range - men_inc;
                  if ( men_end >= dat_str )
                  {  // Adjust first meniscus so range remains below data range
                     men_end         = dat_str - men_inc / 2.0;
                     men_str         = men_end - meniscus_range + men_inc;
                  }
                  for ( int ii = 0; ii < meniscus_points; ii++ )
                     meniscus_values[ ii ] = men_str + men_inc * ii;
DbgLv(1) << " master loop-BOT:     menpt" << meniscus_points << "mv0 mvn"
 << meniscus_values[0] << meniscus_values[meniscus_points-1]
 << "gcores_count" << gcores_count;
               }

//               for ( int ii = 1; ii < gcores_count; ii++ )
//                  worker_status[ ii ] = READY;

               fill_queue();

               for ( int ii = 1; ii < gcores_count; ii++ )
                  worker_status[ ii ] = READY;
DbgLv(1) << " master loop-BOT:      wkst1 wkstn" << worker_status[1]
 << worker_status[gcores_count-1];

               for ( int ii = 0; ii < calculated_solutes.size(); ii++ )
                  calculated_solutes[ ii ].clear();

               continue;
            }
         }

         shutdown_all();  // All done
         break;           // Break out of main loop.
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

      worker = status.MPI_SOURCE;

if ( max_depth > 0 )
 DbgLv(1) << " master loop-BOTTOM:   status TAG" << status.MPI_TAG
  << MPI_Job::READY << MPI_Job::RESULTS << "  source" << status.MPI_SOURCE;
      switch( status.MPI_TAG )
      {
         case MPI_Job::READY:   // Ready for work
            worker_status[ worker ] = READY;
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

// Generate the initial set of solutes
void US_MPI_Analysis::init_solutes( void )
{
   calculated_solutes.clear();
   orig_solutes.clear();
   simulation_values.noisflag    = parameters[ "tinoise_option" ].toInt() > 0 ?
                                   1 : 0;
   simulation_values.noisflag   += parameters[ "rinoise_option" ].toInt() > 0 ?
                                   2 : 0;
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

      US_Solute::init_solutes( s_min,   s_max,   nsstep,
                               ff0_min, ff0_max, nkstep,
                               grid_reps, cnstff0, orig_solutes );
   }

   else
   {  // If a model file was given, use it to set the initial solutes
      US_Model model;
      QString fn = "../" + model_filename;
      model.load( fn );
      int    nsubgrid = model.subGrids;
      int    ncomps   = model.components.size();
DbgLv(1) << "InSol: fn" << fn;
DbgLv(1) << "Insol:  nsubgrid ncomps" << nsubgrid << ncomps;

      if ( nsubgrid < 1 )
         abort( "Custom grid model file has no subgrids", -1 );

      if ( ( ncomps / nsubgrid ) > 150 )
      {  // Subgrids too large:  adjust subgrid count and size
         nsubgrid       = ( ncomps / 100 + 1 ) | 1;
         model.subGrids = nsubgrid;
DbgLv(1) << "Insol:  nsubgrid sbsize" << nsubgrid << ( ncomps / nsubgrid );
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

   worker_status.fill( INIT );
   worker_depth .fill( 0 );
   max_depth           = 0;
   worknext            = 1;
   max_experiment_size = min_experiment_size;

   // Put all jobs in the queue
   job_queue.clear();

   for ( int i = 0; i < orig_solutes.size(); i++ )
   {
      max_experiment_size = qMax( max_experiment_size,
                                  orig_solutes[ i ].size() );
      Sa_Job job;
      job.solutes         = orig_solutes[ i ];
      job_queue << job;
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

   // The first dataset is done automatically.
   for ( int solute = 0; solute < simulation_values.solutes.size(); solute++ )
   {
      concentration += simulation_values.solutes[ solute ].c;
   }

   qDebug() << "   == Dataset" << current_dataset + 1
            << "Total Concentration" << concentration << "==";

   // Point to current dataset
   US_DataIO::EditedData* edata = &data_sets[ current_dataset ]->run_data;

   concentrations[ current_dataset ] = concentration;
   edata->ODlimit   /= concentration;
   int scan_count    = edata->scanCount();
   int radius_points = edata->pointCount();
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
DbgLv(1) << "ScaledData sum" << dsum << "iSum" << isum << "concen" << concentration;

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

   fill_queue();

   for ( int ii = 0; ii < calculated_solutes.size(); ii++ )
      calculated_solutes[ ii ].clear();
}

// Reset for a fit-meniscus iteration
void US_MPI_Analysis::set_meniscus( void )
{
   meniscus_run++;

   // We incremented meniscus_run above.  Just rerun from the beginning.
   for ( int i = 0; i < orig_solutes.size(); i++ )
   {
      Sa_Job job;
      job.solutes = orig_solutes[ i ];

      job_queue << job;
   }

   worker_depth.fill( 0 );
   max_depth = 0;
   for ( int ii = 0; ii < calculated_solutes.size(); ii++ )
      calculated_solutes[ ii ].clear();
}

// Reset for a Monte Carlo iteration
void US_MPI_Analysis::set_monteCarlo( void )
{
DbgLv(1) << "sMC: max_depth" << max_depth << "calcsols size" << calculated_solutes[max_depth].size()
 << "simvsols size" << simulation_values.solutes.size();

   // Set up new data modified by a gaussian distribution
   if ( mc_iteration == 1 )
   {
      set_gaussians();

      sim_data1      = simulation_values.sim_data;

      double fitrmsd = sqrt( simulation_values.variance );
      qDebug() << "  MC_Iteration 1 Simulation RMSD"
               << QString::number( fitrmsd, 'f', 7 );
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
DbgLv(1) << "sMC: totpts" << total_points << "mc_iter" << mc_iteration;
   mc_data.resize( ds_points );
   int    index      = 0;
   int    scnx       = 0;
   double varrmsd    = 0.0;
   double varisum    = 0.0;
   double varimin    = 1.0;
   double varimax    = -1.0;
   double datasum    = 0.0;

   // Get a randomized variation of the concentrations
   // Use a gaussian distribution with the residual as the standard deviation
   for ( int ee = ds_start; ee < ds_end; ee++ )
   {
      edata             = &data_sets[ ee ]->run_data;
      int scan_count    = edata->scanCount();
      int radius_points = edata->pointCount();
int indxh=((scan_count/2)*radius_points)+(radius_points/2);

      for ( int ss = 0; ss < scan_count; ss++, scnx++ )
      {
         for ( int rr = 0; rr < radius_points; rr++ )
         {
            double variation = US_Math2::box_muller( 0.0, sigmas[ index ] );
            double mcdata    = sim_data1.value( scnx, rr ) + variation;
            varrmsd         += sq( variation );
            varisum         += variation;
            varimin          = qMin( varimin, variation );
            varimax          = qMax( varimax, variation );
            datasum         += mcdata;

if ( index<5 || index>(total_points-6) || (index>(indxh-4)&&index<(indxh+3)) )
DbgLv(1) << "sMC:  index" << index << "sdat" << sim_data1.value(ss,rr)
 << "sigma" << sigmas[index] << "vari" << variation << "mdat" << mcdata;

            mc_data[ index++ ] = mcdata;
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
   res_data          = &simulation_values.residuals;
   int scnx          = 0;
DbgLv(1) << "sGA:  resids scans points" << res_data->scanCount() << res_data->pointCount();

   for ( int ee = ds_start; ee < ds_end; ee++ )
   {
      edata             = &data_sets[ ee ]->run_data;
      int scan_count    = edata->scanCount();
      int radius_points = edata->pointCount();
DbgLv(1) << "sGA:   ee" << ee << "scans points" << scan_count << radius_points;

      // Place the residuals magnitudes into a single vector for convenience
      for ( int ss = 0; ss < scan_count; ss++, scnx++ )
      {
         for ( int rr = 0; rr < radius_points; rr++ )
         {
            sigmas << qAbs( res_data->value( scnx, rr ) );
         }
      }
   }
int ssz=sigmas.size();
DbgLv(1) << "sGA:   sigmas size" << ssz << "sigmas[mm]" << sigmas[ssz/2];
}

// Write model (and maybe noise) output at the end of an iteration
void US_MPI_Analysis::write_output( void )
{
   US_SolveSim::Simulation sim = simulation_values;

   double save_meniscus = meniscus_value;
   US_Model::AnalysisType mdl_type = model_type( analysis_type );
   int mxdssz   = -1;

   if ( mdl_type == US_Model::TWODSA  ||  mdl_type == US_Model::GA )
   {
      sim.solutes  = calculated_solutes[ max_depth ]; 
      mxdssz       = sim.solutes.size();
   }

   else if ( mdl_type == US_Model::DMGA )
   {  // Handle DMGA need for dummy solutes
      QVector< US_Solute > solvec;
      max_depth    = 0;
      calculated_solutes.clear();
      calculated_solutes << solvec;
      sim.solutes  = solvec;
DbgLv(1) << "MAST: wrout: mdl_type DMGA";
   }

   else if ( mdl_type == US_Model::PCSA )
   {  // PCSA: Order model records and pick best model
      max_depth    = 0;
      qSort( mrecs );
//*DEBUG*
DbgLv(1) << "MAST: wrout: mdl_type PCSA  mrecs size" << mrecs.size();
if(dbg_level>0)
{
 for(int jj=0;jj<mrecs.size();jj++)
 {
  DbgLv(1) << "M:wo: jj" << jj << "typ tx" << mrecs[jj].ctype << mrecs[jj].taskx << "isz csz"
   << mrecs[jj].isolutes.size() << mrecs[jj].csolutes.size() << "rmsd" << mrecs[jj].rmsd
   << "sy ey" << mrecs[jj].str_y << mrecs[jj].end_y
   << "p1 p2" << mrecs[jj].par1 << mrecs[jj].par2;
 }
}
//*DEBUG*
      sim.zsolutes = mrecs[ 0 ].csolutes;
      mxdssz       = sim.zsolutes.size();
      sim.ti_noise = mrecs[ 0 ].ti_noise;
      sim.ri_noise = mrecs[ 0 ].ri_noise;
   }
DbgLv(1) << "WrO: mxdssz" << mxdssz;

   if ( mxdssz == 0 )
   { // Handle the case of a zero-solute final model
      int simssz   = simulation_values.zsolutes.size();
      int dm1ssz   = max_depth > 0 ?
                     calculated_solutes[ max_depth - 1 ].size() : 0;
      DbgLv( 0 ) << "*WARNING* Final solutes size" << mxdssz
         << "max_depth" << max_depth << "Sim and Depth-1 solutes size"
         << simssz << dm1ssz;
      if ( simssz > 0 )
      { // Use the last result solutes if there are some
         sim.zsolutes = simulation_values.zsolutes;
         DbgLv( 0 ) << "   SimValues solutes used";
      }
      else if ( dm1ssz > 0 )
      { // Else use the max-depth-minus-1 solutes if there are some
         //sim.zsolutes = calculated_solutes[ max_depth - 1 ];
         DbgLv( 0 ) << "   CalcValue[mxdepth-1] solutes used";
      }
      else
      { // Else report the bad situation of no solutes available
         DbgLv( 0 ) << "   *ERROR* No solutes will be used";
      }
   }

DbgLv(1) << "WrO: meniscus_run" << meniscus_run << "mvsz" << meniscus_values.size();
   meniscus_value  = meniscus_run < meniscus_values.size() 
                     ? meniscus_values[ meniscus_run ] : save_meniscus;

   if ( mdl_type == US_Model::PCSA )
   {
DbgLv(1) << "WrO: qSort solutes  sssz" << sim.zsolutes.size();
      qSort( sim.zsolutes );
   }

   else if ( mdl_type != US_Model::DMGA )
   {
DbgLv(1) << "WrO: qSort solutes  sssz" << sim.solutes.size();
      qSort( sim.solutes );
   }

DbgLv(1) << "WrO: wr_model  mdl_type" << mdl_type;
   write_model( sim, mdl_type );
   meniscus_value  = save_meniscus;

DbgLv(1) << "WrO: wr_noise";
   if (  parameters[ "tinoise_option" ].toInt() > 0 )
      write_noise( US_Noise::TI, sim.ti_noise );

   if (  parameters[ "rinoise_option" ].toInt() > 0 )
      write_noise( US_Noise::RI, sim.ri_noise );

DbgLv(1) << "WrO: wr_mrecs";
   if ( mdl_type == US_Model::PCSA )
   {  // Output mrecs for PCSA, if we have a final mrecs vector
      int tikreg   = parameters[ "tikreg_option" ].toInt();
      int mc_iters = parameters[ "mc_iterations" ].toInt();

      if ( tikreg == 0  &&  mc_iters < 2 )
         write_mrecs();
   }
}

// Write global model outputs at the end of an iteration
void US_MPI_Analysis::write_global( void )
{
   US_SolveSim::Simulation  sim      = simulation_values;
   US_SolveSim::Simulation* gsim     = &simulation_values;
   US_Model::AnalysisType   mdl_type = model_type( analysis_type );

   int nsolutes = ( mdl_type != US_Model::DMGA ) ? sim.solutes.size() : -1;

   if ( nsolutes == 0 )
   { // Handle the case of a zero-solute final model
      DbgLv( 0 ) << "   *ERROR* No solutes available for global model";
      return;
   }

DbgLv(1) << "WrGlob: mciter mxdepth" << mc_iteration+1 << max_depth
 << "calcsols size" << calculated_solutes[max_depth].size()
 << "simvsols size" << nsolutes;

   if ( mdl_type == US_Model::TWODSA )
   {
      // 2DSA: Recompute the global fit and save A and b matrices for later use
      wksim_vals           = simulation_values;
      US_SolveSim solvesim( data_sets, my_rank, false );
      solvesim.calc_residuals( 0, data_sets.size(), wksim_vals, false,
                               &gl_nnls_a, &gl_nnls_b );
DbgLv(1) << "WrGlob:  glob recompute nsols" << wksim_vals.solutes.size()
 << "globrec A,b sizes" << gl_nnls_a.size() << gl_nnls_b.size();

      for ( int ee = 0; ee < data_sets.size(); ee++ )
      {
         US_DataIO::EditedData* edata = &data_sets[ ee ]->run_data;
         current_dataset      = ee;
         meniscus_value       = edata->meniscus;
         double concentration = concentrations[ ee ];
DbgLv(1) << "WrGlob:   currds" << ee << "concen" << concentration;

         for ( int cc = 0; cc < nsolutes; cc++ )
         {
            sim.solutes[ cc ].c   = gsim->solutes[ cc ].c * concentration;
         }

DbgLv(1) << "WrGlob:    call write_model(1)";
         // Output the model from global solute points
         write_model( sim, mdl_type, true );

         // Grab dataset portion of A and b, then re-fit
         wksim_vals           = sim;
         wksim_vals.solutes.clear();
         int kscans           = edata->scanCount();
         int kpoints          = edata->pointCount();
         int narows           = kscans * kpoints;
         int navals           = narows * nsolutes;
         int ksolutes         = 0;
         QVector< double > nnls_a( navals,   0.0 );
         QVector< double > nnls_b( narows,   0.0 );
         QVector< double > nnls_x( nsolutes, 0.0 );
DbgLv(1) << "WrGlob:    ks kp nar nav" << kscans << kpoints << narows << navals;

         dset_matrices( ee, nsolutes, nnls_a, nnls_b );

DbgLv(1) << "WrGlob:    mats built; calling NNLS";
         US_Math2::nnls( nnls_a.data(), narows, narows, nsolutes,
                         nnls_b.data(), nnls_x.data() );

DbgLv(1) << "WrGlob:     building solutes from nnls_x";
         for ( int cc = 0; cc < nsolutes; cc++ )
         {
            double soluval       = nnls_x[ cc ];
            if ( soluval > 0.0 )
            {
               US_Solute solu       = sim.solutes[ cc ];
               solu.c               = soluval;
               wksim_vals.solutes << solu;
               ksolutes++;
            }
         }
DbgLv(1) << "WrGlob:    currds" << ee << "nsol ksol" << nsolutes << ksolutes;

         // Output the model refitted to individual dataset
         write_model( wksim_vals, mdl_type );
      }
   }

   else if ( mdl_type == US_Model::GA )
   {  // GA:  Compute and output each dataset model
      int ksolutes         = 0;

      for ( int ee = 0; ee < data_sets.size(); ee++ )
      {
         wksim_vals           = simulation_values;
         US_DataIO::EditedData* edata = &data_sets[ ee ]->run_data;
         current_dataset      = ee;
         meniscus_value       = edata->meniscus;
         US_SolveSim solvesim( data_sets, my_rank, false );
         solvesim.calc_residuals( ee, 1, wksim_vals, false );
         ksolutes             = wksim_vals.solutes.size();

         // Output the model fitted to individual dataset
         write_model( wksim_vals, mdl_type );
DbgLv(1) << "WrGlob:    currds" << ee << "nsol ksol" << nsolutes << ksolutes;
      }
   }

   else if ( mdl_type == US_Model::DMGA )
   {  // DMGA:  Compute and output each dataset model
   }

   current_dataset      = 0;
}

// Reset for a refinement iteration
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
   Sa_Job job;
   job.mpi_job.dataset_offset = current_dataset;
   job.mpi_job.dataset_count  = datasets_to_process;
   job.mpi_job.meniscus_value = meniscus_value;
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

      job_queue << job;

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

/////////////////////
// Shutdown the workers
// Actually this doesn't necessarily shut them down, it breaks 
// out of the processing loop.
void US_MPI_Analysis::shutdown_all( void )
{
   MPI_Job job;
   job.command = MPI_Job::SHUTDOWN;
DbgLv(1) << "2dsa master shutdown : master maxrss" << maxrss;
 
   for ( int i = 1; i <= my_workers; i++ )
   {
      MPI_Send( &job, 
         sizeof( job ), 
         MPI_BYTE,
         i,               // Send to each worker
         MPI_Job::MASTER,
         my_communicator );

      maxrss += work_rss[ i ];
DbgLv(1) << "2dsa master shutdown : worker" << i << " upd. maxrss" << maxrss
 << "  wkrss" << work_rss[ i ];
   }
}

/////////////////////
void US_MPI_Analysis::submit( Sa_Job& job, int worker )
{
   job.mpi_job.command        = MPI_Job::PROCESS;
   job.mpi_job.length         = job.solutes.size(); 
   job.mpi_job.meniscus_value = meniscus_value;
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

   max_experiment_size = qMax( max_experiment_size, size[ 0 ] );

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
   int depth       = worker_depth[ worker ];   

if (depth == 0) { DbgLv(1) << "Mast:  process_results: worker" << worker
 << " solsize" << size[0] << "depth" << depth; }
else { DbgLv(1) << "Mast:  process_results:      worker" << worker
 << " solsize" << size[0] << "depth" << depth; }
    Result result;
    result.depth   = depth;
    result.worker  = worker;
    result.solutes = simulation_values.solutes;

    int lwdepth    = low_working_depth();

    // If there are no cached results and the job result's depth
    // is not beyond the low working depth, just process the result solutes
    if ( cached_results.size() == 0  &&  depth <= lwdepth )
    {
       process_solutes( depth, worker, result.solutes );
    }

    // If there are cached results or the job result's depth is below
    // the low working depth, then first cache the current result
    // in its proper depth-ordered place in the cached list
    else
    {
       cache_result( result );
    }

    // Process any previous results that were cached.
    // As long as there are cached depth-ordered results and the low on
    // the list is less than or equal to the low-working depth;
    // each first-on-the-list gets taken off and processed.

    while ( cached_results.size() > 0  &&
            cached_results[ 0 ].depth <= lwdepth )
    {
       result     = cached_results.takeFirst();
       depth      = result.depth;
       worker     = result.worker;

       process_solutes( depth, worker, result.solutes );
    }
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
      qSort( job.solutes );
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
      bool queued = false;
      for ( int q = 0; q < job_queue.size(); q++ )
      {
         if ( job_queue[ q ].mpi_job.depth <= d )
         {
            queued = true;
            break;
         }
      }

      bool working = false;
      for ( int w = 1; w <= my_workers; w++ )
      {
         if ( worker_depth[ w ] <= d  &&  worker_status[ w ] == WORKING )
         {
            working = true;
            break;
         }
      }

      int remainder = calculated_solutes[ d ].size();

      if ( ! working && ! queued && remainder > 0 )
      { // Submit a job with remaining calculated solutes from an earlier depth
         int next_d                 = d + 1;
         Sa_Job job;
         job.solutes                = calculated_solutes[ d ];
         job.mpi_job.depth          = next_d;
         job.mpi_job.dataset_offset = current_dataset;
         job.mpi_job.dataset_count  = datasets_to_process;
         max_depth                  = qMax( next_d, max_depth );
         qSort( job.solutes );
         add_to_queue( job );
DbgLv(1) << "Mast:   queue REMAINDER" << remainder << " d=" << d+1;

         calculated_solutes[ d ].clear();
      }
   }

   // Is anyone working?
   bool working = false;
   for ( int w = 1; w <= my_workers; w++ )
   {
      if ( worker_status[ w ] == WORKING )
      {
         working = true;
         break;
      }
   }

   // Submit one last time with all solutes if necessary.
   // This is the case if
   //  (1) this result is from the maximum submitted-jobs depth;
   //  (2) no jobs are queued or working; and
   //  (3) the current set of calculated solutes comes from more
   //      than one task result.
   if ( depth == max_depth     &&
        job_queue.isEmpty()    &&
        ! working              &&
        csol_size > rsol_size )
   {
      Sa_Job job;
      job.solutes          = calculated_solutes[ depth ];
      job.mpi_job.depth    = next_depth;
      meniscus_value       = ( meniscus_values.size() == 1 )
                           ? data_sets[ current_dataset ]->run_data.meniscus
                           : meniscus_values[ meniscus_run ];
      qSort( job.solutes );
DbgLv(1) << "Mast:   queue LAST ns=" << job.solutes.size() << "  d=" << depth+1
 << max_depth << "  nsvs=" << simulation_values.solutes.size();

      calculated_solutes[ depth ].clear();
      csol_size = 0;
      max_depth = next_depth;
      worker    = ready_worker();

      if ( worker > 0 )
      { // Submit what should be the last job of this iteration
         ljob_solutes            = job.solutes;
         submit( job, worker );
         worker_depth [ worker ] = job.mpi_job.depth;
         worker_status[ worker ] = WORKING;
         // Insure calculated solutes is empty for final depth
         if ( calculated_solutes.size() > max_depth )
            calculated_solutes[ max_depth ].clear();
         else
            calculated_solutes << QVector< US_Solute >();
      }
      else
      { // Shouldn't happen, but put job in queue if no worker is yet ready
DbgLv(1) << "Mast:   WARNING: LAST depth and no worker ready!";
         job_queue << job;
      }
   }

   // Force an abort if we are in a run-away situation
   if ( max_depth > 20 )
   {
      abort( "Max Depth is exceeding 20" );
      calculated_solutes[ depth + 20 ].clear();
   }

}

// Write model output at the end of an iteration
void US_MPI_Analysis::write_model( const US_SolveSim::Simulation& sim, 
                                   US_Model::AnalysisType         type,
                                   bool                           glob_sols )
{
   US_DataIO::EditedData* edata = &data_sets[ current_dataset ]->run_data;

   // Fill in and write out the model file
   US_Model model;
   int subtype       = ( type == US_Model::PCSA ) ? mrecs[ 0 ].ctype : 0;

DbgLv(1) << "wrMo: type" << type << "(DMGA=" << US_Model::DMGA << ") (PCSA="
 << US_Model::PCSA << ") subtype=" << subtype;
   if ( type == US_Model::DMGA )
   {  // For discrete GA, get the already constructed model
      model             = data_sets[ 0 ]->model;
DbgLv(1) << "wrMo:  model comps" << model.components.size();
   }

   int mc_iter       = ( mgroup_count < 2  ||  is_composite_job ) 
                       ? ( mc_iteration + 1 ) : mc_iteration;
   model.monteCarlo  = mc_iterations > 1;
   model.wavelength  = edata->wavelength.toDouble();
   model.modelGUID   = ( ! model.monteCarlo  ||  mc_iter == 1 )
                       ? US_Util::new_guid() : modelGUID;
DbgLv(1) << "wrMo:  mc mciter mGUID" << model.monteCarlo << mc_iter
 << model.modelGUID;
   model.editGUID    = edata->editGUID;
   model.requestGUID = requestGUID;
   model.dataDescrip = edata->description;
   //model.optics      = ???  How to get this?  Is is needed?
   model.analysis    = type;
   QString runID     = edata->runID;

   if ( meniscus_points > 1 ) 
      model.global      = US_Model::MENISCUS;

   else if ( is_global_fit )
   {
      model.global      = US_Model::GLOBAL;
      if ( glob_sols )
         runID             = "Global-" + runID;
      model.modelGUID   = US_Util::new_guid();
      modelGUID         = model.modelGUID;
   }

   else
      model.global      = US_Model::NONE; 
DbgLv(1) << "wrMo:  is_glob glob_sols" << is_global_fit << glob_sols;

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

   QString tripleID  = edata->cell + edata->channel + edata->wavelength;
   QString dates     = "e" + edata->editID + "_a" + analysisDate;
DbgLv(1) << "wrMo: tripleID" << tripleID << "dates" << dates;
   QString iterID;

   if ( mc_iterations > 1 )
      iterID.sprintf( "mc%04d", mc_iter );
   else if (  meniscus_points > 1 )
      iterID.sprintf( "i%02d-m%05d", 
              meniscus_run + 1,
              (int)(meniscus_value * 10000 ) );
   else
      iterID = "i01";

   QString mdlid     = tripleID + "." + iterID;
   QString id        = model.typeText( subtype );
   if ( analysis_type.contains( "CG" ) )
      id                = id.replace( "2DSA", "2DSA-CG" );
   QString analyID   = dates + "_" + id + "_" + requestID + "_" + iterID;
   int     stype     = data_sets[ current_dataset ]->solute_type;
   double  vbar20    = data_sets[ current_dataset ]->vbar20;

   model.description = runID + "." + tripleID + "." + analyID + ".model";
DbgLv(1) << "wrMo: model descr" << model.description;

   // Save as class variable for later reference
   modelGUID         = model.modelGUID;

   if ( type == US_Model::PCSA )
   {  // For PCSA, construct the model from zsolutes
      for ( int ii = 0; ii < sim.zsolutes.size(); ii++ )
      {
         US_ZSolute zsolute = sim.zsolutes[ ii ];

         US_Model::SimulationComponent component;
         US_ZSolute::set_mcomp_values( component, zsolute, stype, true );
         component.name     = QString().sprintf( "SC%04d", ii + 1 );

         US_Model::calc_coefficients( component );
         model.components << component;
      }
   }

   else if ( type != US_Model::DMGA )
   {  // For other non-DMGA, construct the model from solutes
      for ( int ii = 0; ii < sim.solutes.size(); ii++ )
      {
         const US_Solute* solute = &sim.solutes[ ii ];

         US_Model::SimulationComponent component;
         component.s       = solute->s;
         component.f_f0    = solute->k;
         component.name    = QString().sprintf( "SC%04d", ii + 1 );
         component.vbar20  = (attr_z == ATTR_V) ? vbar20 : solute->v;
         component.signal_concentration = solute->c;

         US_Model::calc_coefficients( component );
         model.components << component;
      }
   }
DbgLv(1) << "wrMo: stype" << stype << QString().sprintf("0%o",stype)
 << "attr_z vbar20 mco0.v" << attr_z << vbar20 << model.components[0].vbar20;

   QString fext      = model.monteCarlo ? ".mdl.tmp" : ".model.xml";
   QString fileid    = "." + id + "." + mdlid + fext;
   QString fn        = runID + fileid;
   int lenfn         = fn.length();

   if ( lenfn > 99 )
   { // Insure a model file name less than 100 characters in length (tar limit)
      int lenri         = runID.length() + 99 - lenfn;
      fn                = runID.left( lenri ) + fileid;
   }

   // Output the model to a file
   model.write( fn );

   // Save the model in case needed for noise
   data_sets[ current_dataset ]->model = model;

   // Add the file name of the model file to the output list
   QFile fileo( "analysis_files.txt" );

   if ( ! fileo.open( QIODevice::WriteOnly | QIODevice::Text
                                           | QIODevice::Append ) )
   {
      abort( "Could not open 'analysis_files.txt' for writing" );
      return;
   }

   QTextStream tsout( &fileo );

   QString meniscus = QString::number( meniscus_value, 'e', 4 );
   QString variance = QString::number( sim.variance,   'e', 4 );

   int run     = 1;

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

// Write noise output at the end of an iteration
void US_MPI_Analysis::write_noise( US_Noise::NoiseType      type, 
                                   const QVector< double >& noise_data )
{
   US_DataIO::EditedData* data = &data_sets[ current_dataset ]->run_data;

   QString  type_name;
   US_Noise noise;

   if ( type == US_Noise::TI ) 
   {
      type_name         = "ti";
      int radii         = data->pointCount();
      noise.minradius   = data->radius( 0 );
      noise.maxradius   = data->radius( radii - 1 );
   }
   else
   {
      type_name = "ri";
   }

   // demo1_veloc.1A999.e201101171200_a201101171400_2DSA us3-0000003
   //    _i01
   //    _mc001
   //    _i01-m62345.ri_noise
   //                        .model
   //                        .ri_noise
   // runID.tripleID.analysID.recordType
   //    analysID = editID_analysisDate_analysisType_requestID_iterID
   //       editID:     
   //       requestID: from lims or 'local' 
   //       analysisType : 2DSA GA others
   //       iterID:       'i01-m62345' for meniscus,
   //                     'mc001' for monte carlo, 'i01' default 
   //      
   //       recordType: ri_noise, ti_noise, model

   QString tripleID   = data->cell + data->channel + data->wavelength;
   QString dates      = "e" + data->editID + "_a" + analysisDate;
   QString anType     = "_" + data_sets[ current_dataset ]->model.typeText()
                        + "_";
   if ( analysis_type.contains( "CG" ) )
      anType             = anType.replace( "2DSA", "2DSA-CG" );

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
   QList< QString > noise_filenames = data_sets[ current_dataset ]->noise_files;

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

int US_MPI_Analysis::ready_worker( )
{
   // Find next ready worker by searching status array
   int worker = worker_status.indexOf( READY, worknext );
int w1=worker;
int w1n=worknext;
   worker     = ( worker > 0 ) ? worker :
                worker_status.indexOf( READY, 1 );

   // Set index to start with on next search
   worknext   = ( worker > 0 ) ? ( worker + 1 ) : 1;
   worknext   = ( worknext > my_workers ) ? 1 : worknext;
DbgLv(1) << "ready_worker  w1 w1n worker worknext" << w1 << w1n << worker << worknext;
if(w1<0)
DbgLv(1) << "ready_worker  w1234...wmn"
 << worker_status[1] << worker_status[2] << worker_status[3] << worker_status[4]
 << worker_status[my_workers-1] << worker_status[my_workers];

   return worker;
}

// Find the lowest depth among working jobs
int US_MPI_Analysis::low_working_depth( )
{
   int depth = 99;      // Default to a depth higher than any reasonable one

   for ( int ii = 1; ii <= my_workers; ii++ )
   { // Test all worker statuses and depths
      int wdepth = worker_depth[ ii ];

      if ( worker_status[ ii ] == WORKING  &&
           wdepth < depth )
      { // If working and low depth so far, save depth
         depth      = wdepth;
      }
   }

   return depth;
}

// Cache a job result in a depth-ordered list
void US_MPI_Analysis::cache_result( Result& result )
{
   int rdepth = result.depth;

   for ( int ii = 0; ii < cached_results.size(); ii++ )
   { // Examine all cached results
      int cdepth = cached_results[ ii ].depth;

      if ( rdepth < cdepth )
      { // Insert new result before next highest depth
         cached_results.insert( ii, result );
         return;
      }
   }

   // If no higher depth cached, append new result to the end
   cached_results << result;
   return;
}

// Update the output TAR file after composite job output has been produced
void US_MPI_Analysis::update_outputs( bool is_final )
{
   // Get a list of output files, minus any TAR file
   QDir odir( "." );
   QStringList files = odir.entryList( QStringList( "*" ), QDir::Files );
DbgLv(0) << my_rank << ": UpdOut : final" << is_final
   << "files size" << files.size();

   if ( files.size() == 1  &&  files[ 0 ] == "analysis-results.tar" )
   {  // If the tar file exists alone, do nothing here
DbgLv(0) << my_rank << ": A single output file, the archive, already exists!!!";
      return;
   }

   // Otherwise, remove the tar file from the list of output files
   files.removeOne( "analysis-results.tar" );

   // Sort file list
   files.sort();

   // For Monte Carlo, replace component temporary model files with
   //  concatenated model files
   if ( mc_iterations > 1 )
   {
      // Get a list of model files currently present
      QStringList mfilt;
      mfilt << "*.mdl.tmp" << "*.model.xml";
      QStringList mfiles = odir.entryList( mfilt, QDir::Files );
      QStringList mtrips;
      mfiles.sort();

      // Scan for unique triples
      for ( int ii = 0; ii < mfiles.size(); ii++ )
      {
         QString mftrip     = QString( mfiles[ ii ] ).section( ".", 0, -4 );
         if ( ! mtrips.contains( mftrip ) )
            mtrips << mftrip;
      }

      // Get a list of files in the text file
      QStringList tfiles;
      QFile filet( "analysis_files.txt" );
      if ( filet.open( QIODevice::ReadOnly | QIODevice::Text ) )
      {
         QTextStream tstxt( &filet );
         while ( ! tstxt.atEnd() )
         {
            QString line       = tstxt.readLine();
            QString fname      = line.section( ";", 0, 0 );
            tfiles << fname;
         }
         filet.close();
      }

      // Open text file for appending composite file names
      QFile fileo( "analysis_files.txt" );
      if ( ! fileo.open( QIODevice::WriteOnly | QIODevice::Text
                                              | QIODevice::Append ) )
      {
         abort( "Could not open 'analysis_files.txt' for writing" );
         return;
      }
      QTextStream tsout( &fileo );

      // For each triple, build or update a composite model file
      for ( int ii = 0; ii < mtrips.size(); ii++ )
      {
         // Build a list of model files relating to this triple
         QString mftrip   = mtrips[ ii ];
         QString tripleID = QString( mftrip ).section( ".", -1, -1 );
         QStringList mfilt;
         mfilt << mftrip + ".mc0*";
         QStringList mtfiles = odir.entryList( mfilt, QDir::Files );

         // Skip composite build if not yet enough triple mc iteration models
         if ( mtfiles.count() < mc_iterations )  continue;

         // Build a composite model file and get its name
         QString cmfname  = US_Model::composite_mc_file( mtfiles, true );
DbgLv(0) << my_rank << ": ii" << ii << "mftrip" << mftrip << "cmfname" << cmfname;

         if ( cmfname.isEmpty() )  continue;

         // Remove iteration (non-composite) files from the list
         for ( int jj = 0; jj < mtfiles.size(); jj++ )
         {
            if ( mtfiles[ jj ] != cmfname )
            {
               files.removeOne( mtfiles[ jj ] );
DbgLv(0) << my_rank << ": ii,jj" << ii << jj << "REMOVED from list:" << mtfiles[jj];
            }
         }

         // Add the composite file name to the list if need be
         if ( ! files.contains( cmfname ) )
         {
            files << cmfname;
DbgLv(0) << my_rank << ":     files.size" << files.size() << "after cmfname add";
         }

         // Add composite name to text file if need be
         int f_iters      = QString( cmfname ).section( ".", -3, -3 )
                            .mid( 3 ).toInt();
         if ( ! tfiles.contains( cmfname )  &&
              ( is_final  ||  f_iters == mc_iterations ) )
         {
            US_Model model2;
DbgLv(0) << my_rank << ":      model2.load(" << cmfname << ")";
            model2.load( cmfname );
DbgLv(0) << my_rank << ":       model2.description" << model2.description;
            QString runstring = "Run: " + QString::number( ii + 1 )
                                + " " + tripleID;
            tsout << cmfname 
                  << ";meniscus_value=" << model2.meniscus
                  << ";MC_iteration="   << mc_iterations
                  << ";variance="       << model2.variance
                  << ";run="            << runstring << "\n";

            if ( analysis_type.contains( "PCSA" ) )
            {
               int mrx          = mrecs[ 2 ].taskx == mrecs[ 0 ].taskx ? 2 : 1;
               mrecs[ mrx ].modelGUID = model2.modelGUID;
            }
         }
      }

      fileo.close();
   }

   // Create the archive file containing all outputs
   US_Tar tar;
   tar.create( "analysis-results.tar", files );
for(int jf=0;jf<files.size();jf++)
 DbgLv(0) << my_rank << "   tar file" << jf << ":" << files[jf];

   // If this is the final call, remove all but the archive file
   if ( is_final )
   {  // Remove the files we just put into the tar archive
DbgLv(0) << my_rank << ": All output files except the archive are now removed.";
      QString file;
      foreach( file, files ) odir.remove( file );
   }
}

// Return the model type flag for a given analysis type string
US_Model::AnalysisType US_MPI_Analysis::model_type( const QString a_type )
{
   US_Model::AnalysisType
      m_type      = US_Model::TWODSA;
   if (      a_type.startsWith( "GA" ) )
      m_type      = US_Model::GA;
   else if ( a_type.startsWith( "DMGA" ) )
      m_type      = US_Model::DMGA;
   else if ( a_type.startsWith( "PCSA" ) )
      m_type      = US_Model::PCSA;

   return m_type;
}

