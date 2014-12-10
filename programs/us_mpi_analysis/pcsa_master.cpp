#include "us_mpi_analysis.h"
#include "us_math2.h"
#include "us_util.h"
#include "us_tar.h"
#include "us_astfem_rsa.h"
#include "us_simparms.h"
#include "us_constants.h"

#define DbgTime() qDebug() << "TM:" << (startTime.msecsTo(QDateTime::currentDateTime())/1000.0)
void US_MPI_Analysis::pcsa_master( void )
{
   current_dataset     = 0;
   datasets_to_process = 1;  // Process one dataset at a time for now

DbgLv(1) << "pcsa_mast: IN";
   init_pcsa_solutes();
DbgLv(1) << "pcsa_mast: init complete";
   fill_queue();
DbgLv(1) << "pcsa_mast: fill_queue complete";

   work_rss.resize( gcores_count );

   max_iterations      = parameters[ "gfit_iterations" ].toInt();
   int kcurve          = 0;

   while ( true )
   {
      int worker;
      meniscus_value   = data_sets[ current_dataset ]->run_data.meniscus;
//if ( max_depth > 1 )
// DbgLv(1) << " master loop-TOP:  jq-empty?" << job_queue.isEmpty() << "   areReady?" << worker_status.contains(READY)
//    << "  areWorking?" << worker_status.contains(WORKING);

//DbgTime() << "test submit";
      // Give the jobs to the workers
      while ( ! job_queue.isEmpty()  &&  worker_status.contains( READY ) )
      {
         worker           = ready_worker();

         Sa_Job job              = job_queue.takeFirst();
         job.mpi_job.depth       = kcurve++;
DbgLv(1) << "pcsa_mast:   submit_pcsa kc" << kcurve;
//DbgTime() << "submit_pcsa  worker" << worker << "worknext" << worknext << my_workers;

         submit_pcsa( job, worker );

         worker_depth [ worker ] = job.mpi_job.depth;
         worker_status[ worker ] = WORKING;
//DbgTime() << "submit_pcsa  rtn: depth" << job.mpi_job.depth;
      }

      // All done with the pass if no jobs are ready or running
      if ( job_queue.isEmpty()  &&  ! worker_status.contains( WORKING ) ) 
      {
         kcurve           = 0;
         qSort( mrecs );
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

         else
            progress     += "; RMSD: "
                            + QString::number( mrecs[ 0 ].rmsd );

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
            qDebug() << "Iteration:" << iterations
               << " Variance:" << mrecs[ 0 ].variance
               << "RMSD:" << mrecs[ 0 ].rmsd;

            iterate_pcsa();
         }

         if ( ! job_queue.isEmpty() ) continue;

         iterations     = 1;
         max_iterations = 1;
DbgLv(1) << " master loop-BOT: dssize" << data_sets.size() << "ds_to_p"
 << datasets_to_process << "curr_ds" << current_dataset;
US_DataIO::EditedData* edat=&data_sets[current_dataset]->run_data;
int ks=edat->scanCount() - 10;
int kr=edat->pointCount() - 10;
int ss=ks/2;
int rr=kr/2;
DbgLv(1) << " master loop-BOT: ds" << current_dataset+1 << "data l m h"
 << edat->value(10,10) << edat->value(ss,rr) << edat->value(ks,kr);
         // Clean up mrecs of any empty-calculated-solutes records
         clean_mrecs( mrecs );

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
            wksim_vals.solutes   = calculated_solutes[ 0 ]; 

            calc_residuals( 0, data_sets.size(), wksim_vals );

            qDebug() << "Base-Sim RMSD" << sqrt( simulation_values.variance )
                     << "  Exp-Sim RMSD" << sqrt( wksim_vals.variance )
                     << "  of MC_Iteration" << mc_iteration;
            max_iterations              = 1;
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
//DbgTime() << "wait_for_worker";
      int        size[ 4 ];
      MPI_Status status;

      MPI_Recv( &size, 
                4, 
                MPI_INT,
                MPI_ANY_SOURCE,
                MPI_ANY_TAG,
                my_communicator,
                &status);
//DbgTime() << " worker_recv  worker" << status.MPI_SOURCE
// << "tag" << status.MPI_TAG << MPI_Job::READY;

//if ( max_depth > 0 )
// DbgLv(1) << " master loop-BOTTOM:   status TAG" << status.MPI_TAG << MPI_Job::READY << MPI_Job::RESULTS
//    << "  source" << status.MPI_SOURCE;
      switch( status.MPI_TAG )
      {
         case MPI_Job::READY:   // Ready for work
            worker                  = status.MPI_SOURCE;
            worker_status[ worker ] = READY;
            break;

         case MPI_Job::RESULTS: // Return solute data
            worker                  = status.MPI_SOURCE;
            process_pcsa_results( worker, size );
            work_rss[ worker ]      = size[ 3 ];
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
DbgLv(1) << "  init_pcsa_sols: IN";
   calculated_solutes.clear();
   orig_solutes      .clear();
   simulation_values.noisflag    = parameters[ "tinoise_option" ].toInt() > 0 ?
                                   1 : 0;
   simulation_values.noisflag   += parameters[ "rinoise_option" ].toInt() > 0 ?
                                   2 : 0;
   simulation_values.dbg_level   = dbg_level;
   simulation_values.dbg_timing  = dbg_timing;
DbgLv(0) << "DEBUG_LEVEL" << simulation_values.dbg_level;

   double s_min      = parameters[ "s_min"           ].toDouble();
   double s_max      = parameters[ "s_max"           ].toDouble();
   double ff0_min    = parameters[ "ff0_min"         ].toDouble();
   double ff0_max    = parameters[ "ff0_max"         ].toDouble();
   int    tikreg     = parameters[ "tikreg_option"   ].toInt();
   double tr_alpha   = parameters[ "tikreg_alpha"    ].toDouble();
   tr_alpha          = ( tikreg == 1 ) ? tr_alpha : 0.0;
   QString s_ctyp    = parameters[ "curve_type" ];
   int    ctype      = US_ModelRecord::ctype_flag( s_ctyp );
   int    nkpts      = parameters[ "vars_count"      ].toInt();
   int    nkpto      = sq( nkpts );
   int    nlpts      = parameters[ "curves_points"   ].toInt();
DbgLv(1) << "  init_pcsa_sols: nkpts" << nkpts << "nkpto" << nkpto;
   double vbar20     = data_sets[ current_dataset ]->vbar20;
DbgLv(1) << "  init_pcsa_sols: currds" << current_dataset << "vbar" << vbar20;
//   int    gf_iters   = parameters[ "gfit_iterations" ].toInt();
//   double thr_dr_rat = parameters[ "thr_deltr_ratio" ].toDouble();
   double pararry[ 15 ];
   double *parlims   = (double*)pararry;
   parlims[ 0 ]      = -1.0;
   parlims[ 4 ]      = vbar20;
   mrecs.clear();
DbgLv(1) << "  init_pcsa_sols: s_ctyp" << s_ctyp << "ctype" << ctype;

   if ( ctype == CTYPE_SL )
   {
      mrecs.reserve( nkpto );
DbgLv(1) << "  init_pcsa_sols: call compute_slines  nkpts" << nkpts;
      US_ModelRecord::compute_slines( s_min, s_max, ff0_min, ff0_max,
            nkpts, nlpts, parlims, mrecs );
DbgLv(1) << "  init_pcsa_sols:  compute_slines: mrecs sz" << mrecs.size();
   }

   else if ( ctype == CTYPE_IS  ||  ctype == CTYPE_DS )
   {
      mrecs.reserve( nkpto );
DbgLv(1) << "  init_pcsa_sols: call compute_sigmoids  nkpts" << nkpts;
      US_ModelRecord::compute_sigmoids( ctype, s_min, s_max, ff0_min, ff0_max,
            nkpts, nlpts, parlims, mrecs );
DbgLv(1) << "  init_pcsa_sols:  compute_sigmoids: mrecs sz" << mrecs.size();
   }

   else if ( ctype == CTYPE_HL )
   {
      nkpto             = nkpts;
      mrecs.reserve( nkpto );
      US_ModelRecord::compute_hlines( s_min, s_max, ff0_min, ff0_max,
            nkpts, nlpts, parlims, mrecs );
   }

   else if ( ctype == CTYPE_ALL )
   {
      nkpto            *= 3;
      QVector< US_ModelRecord > mrecs2;
      int ctype1        = CTYPE_IS;
      int ctype2        = CTYPE_DS;
      double *parlims1  = parlims + 5;
      double *parlims2  = parlims + 10;
      parlims1[ 0 ]     = parlims[ 0 ];
      parlims1[ 4 ]     = parlims[ 4 ];
      parlims2[ 0 ]     = parlims[ 0 ];
      parlims2[ 4 ]     = parlims[ 4 ];
      mrecs.reserve( nkpto );

      US_ModelRecord::compute_slines( s_min, s_max, ff0_min, ff0_max,
            nkpts, nlpts, parlims,  mrecs );
      US_ModelRecord::compute_sigmoids( ctype1, s_min, s_max, ff0_min, ff0_max,
            nkpts, nlpts, parlims1, mrecs );
      US_ModelRecord::compute_sigmoids( ctype2, s_min, s_max, ff0_min, ff0_max,
            nkpts, nlpts, parlims2, mrecs );
   }

   for ( int ii = 0; ii < mrecs.size(); ii++ )
   {
      orig_solutes << mrecs[ ii ].isolutes;
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

// Process the results from a just-completed worker task
void US_MPI_Analysis::process_pcsa_results( const int worker, const int* size )
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

DbgLv(1) << "Mast:  process_results:      worker" << worker
 << " solsize" << size[0];
   Result result;
   result.depth   = worker_depth[ worker ];
   result.worker  = worker;
   result.solutes = simulation_values.solutes;

   // Process the result solutes
   process_pcsa_solutes( result );
}
 
// Process the calculated solute vector from a job result
void US_MPI_Analysis::process_pcsa_solutes( Result& result )
{
   int jcurve     = result.depth;
   jcurve         = qMax( 0, qMin( ( mrecs.size() - 1 ), jcurve ) );
DbgLv(1) << "Mast:    process_solutes:      worker" << result.worker
 << "jcurve" << jcurve << " solsize" << result.solutes.size();

   // Update the model record entry with calculated solutes and RMSD
   US_ModelRecord*
       cMr        = &mrecs[ jcurve ];
   cMr->csolutes  = result.solutes;
   cMr->variance  = simulation_values.variance;
   cMr->rmsd      = cMr->variance > 0.0 ? sqrt( cMr->variance ) : 0.0;
   int noiflg     = simulation_values.noisflag;

   // Add in any noises
   if ( noiflg > 0 )
   {
      if ( ( noiflg & 1 ) > 0 )
         mrecs[ jcurve ].ti_noise = simulation_values.ti_noise;

      if ( ( noiflg & 2 ) > 0 )
         mrecs[ jcurve ].ri_noise = simulation_values.ri_noise;
   }
}

// Write the model records file
void US_MPI_Analysis::write_mrecs()
{
   US_Model * modelP     = &data_sets[ current_dataset ]->model;
   US_DataIO::EditedData*
              edata      = &data_sets[ current_dataset ]->run_data;
   mrecs[ 0 ].modelGUID  = modelP->modelGUID;
DbgLv(1) << "pcsa:wrmr: currds" << current_dataset;
   mrecs[ 0 ].editGUID   = edata->editGUID;
   mrecs[ 0 ].mrecGUID   = US_Util::new_guid();
DbgLv(1) << "pcsa:wrmr:  editGUID=" << mrecs[0].editGUID
 << "modelGUID=" << mrecs[0].modelGUID;
   QString s_desc        = QString( modelP->description )
                           .replace( ".model", ".mrecs" );
   QString s_ctyp        = parameters[ "curve_type" ];
   int    ctype          = US_ModelRecord::ctype_flag( s_ctyp );
   double s_min          = parameters[ "s_min"         ].toDouble();
   double s_max          = parameters[ "s_max"         ].toDouble();
   double ff0_min        = parameters[ "ff0_min"       ].toDouble();
   double ff0_max        = parameters[ "ff0_max"       ].toDouble();
//   int    tikreg         = parameters[ "tikreg_option" ].toInt();
//   double tr_alpha       = parameters[ "tikreg_alpha"  ].toDouble();
//   tr_alpha              = ( tikreg == 1 ) ? tr_alpha : 0.0;
   if ( ctype != CTYPE_ALL )
      mrecs[ 0 ].ctype      = ctype;
   mrecs[ 0 ].smin       = s_min;
   mrecs[ 0 ].smax       = s_max;
   mrecs[ 0 ].kmin       = ff0_min;
   mrecs[ 0 ].kmax       = ff0_max;
   QString fnameo        = s_desc + ".xml";
   QFile fileo( fnameo );

   if ( fileo.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      QXmlStreamWriter xmlo( &fileo );

DbgLv(1) << "pcsa:wrmr: call wr_mr: s_desc" << s_desc;
      US_ModelRecord::write_modelrecs( xmlo, mrecs, s_desc, ctype,
                                       s_min, s_max, ff0_min, ff0_max );
DbgLv(1) << "pcsa:wrmr:  rtn wr_mr";

      fileo.close();
   }

   // Add the file name of the mrecs file to the output list
   QFile filea( "analysis_files.txt" );

   if ( ! filea.open( QIODevice::WriteOnly | QIODevice::Text
                                           | QIODevice::Append ) )
   {
      abort( "Could not open 'analysis_files.txt' for writing" );
      return;
   }

   QTextStream tsout( &filea );
   QString tripleID = edata->cell + edata->channel + edata->wavelength;
   int run     = 1;
   int mc_iter = ( mgroup_count < 2  ||  is_composite_job )
                 ? ( mc_iteration + 1 ) : mc_iteration;

   if ( mc_iterations > 0 )
      run        = mc_iter;

   QString runstring = "Run: " + QString::number( run ) + " " + tripleID;

   tsout << fnameo
         << ";meniscus_value=" << meniscus_value
         << ";MC_iteration="   << mc_iter
         << ";variance="       << simulation_values.variance
         << ";run="            << runstring
         << "\n";
   filea.close();
DbgLv(1) << "pcsa:wrmr: wr-to-atxt fnameo=" << fnameo;
}

// Engineer gfit iterations for PCSA
void US_MPI_Analysis::iterate_pcsa()
{
   static double rmsd_last = 0.0;
   double rmsd_curr  = mrecs[ 0 ].rmsd;
   double thr_dr_rat = parameters[ "thr_deltr_ratio" ].toDouble();

DbgLv(1) << "iter_p: iter" << iterations << "of" << max_iterations;
   // If we have reached the maximum iterations, we are done
   if ( iterations >= max_iterations )  return;

   if ( iterations > 1 )
   {  // If after first iteration, check RMSD difference
      double dr_rat     = qAbs( ( rmsd_curr - rmsd_last ) / rmsd_last );
DbgLv(1) << "iter_p: rmsd_c rmsd_l" << rmsd_curr << rmsd_last
 << "dr_rat thr_dr_rat" << dr_rat << thr_dr_rat;

      // If difference in iteration RMSD less than threshold, we are done
      if ( dr_rat < thr_dr_rat )        return;
   }

   // Set up to create a narrower set of model record curves
   double s_min      = parameters[ "s_min"           ].toDouble();
   double s_max      = parameters[ "s_max"           ].toDouble();
   double ff0_min    = parameters[ "ff0_min"         ].toDouble();
   double ff0_max    = parameters[ "ff0_max"         ].toDouble();
   QString s_ctyp    = parameters[ "curve_type" ];
   int    ctype      = US_ModelRecord::ctype_flag( s_ctyp );
   int    nkpts      = parameters[ "vars_count"      ].toInt();
   int    nlpts      = parameters[ "curves_points"   ].toInt();
   double vbar20     = data_sets[ current_dataset ]->vbar20;
   double pararry[ 15 ];
   double *parlims   = (double*)pararry;
   parlims[ 0 ]      = -1.0;
   parlims[ 4 ]      = vbar20;
   int    ncurve     = sq( nkpts );
DbgLv(1) << "iter_p: ncurve" << ncurve;

   // Recompute mrecs to cover the range of the elite (top 10%) records

   if ( ctype != CTYPE_ALL )
   {  // All non-combination records:  re-compute with narrower limits

      US_ModelRecord::recompute_mrecs( ctype, s_min, s_max, ff0_min, ff0_max,
                                       nkpts, nlpts, parlims, mrecs );
DbgLv(1) << "iter_p: parlims"
 << parlims[0] << parlims[1] << parlims[2] << parlims[3] << parlims[4];
   }  // END: non-combo records

   else
   {  // Records that are a mix of SL,IS,DS
      QVector< US_ModelRecord > mrecs_sl;
      QVector< US_ModelRecord > mrecs_is;
      QVector< US_ModelRecord > mrecs_ds;
      double* plims_sl  = parlims;
      double* plims_is  = plims_sl + 5;
      double* plims_ds  = plims_is + 5;

      // Separate entries by curve type
      filter_mrecs( CTYPE_SL, mrecs, mrecs_sl );
      filter_mrecs( CTYPE_IS, mrecs, mrecs_is );
      filter_mrecs( CTYPE_DS, mrecs, mrecs_ds );

      // Recompute each curve type with narrower limits
      ctype             = CTYPE_SL;
      US_ModelRecord::recompute_mrecs( ctype, s_min, s_max, ff0_min, ff0_max,
                                       nkpts, nlpts, plims_sl, mrecs_sl );

      ctype             = CTYPE_IS;
      US_ModelRecord::recompute_mrecs( ctype, s_min, s_max, ff0_min, ff0_max,
                                       nkpts, nlpts, plims_is, mrecs_is );

      ctype             = CTYPE_DS;
      US_ModelRecord::recompute_mrecs( ctype, s_min, s_max, ff0_min, ff0_max,
                                       nkpts, nlpts, plims_ds, mrecs_ds );

      // Re-combine into a single vector
      int kk            = 0;

      for ( int ii = 0; ii < ncurve; ii++, kk++ )
      {
         mrecs[ kk ]       = mrecs_sl[ ii ];
         mrecs[ kk ].taskx = kk;
      }

      for ( int ii = 0; ii < ncurve; ii++, kk++ )
      {
         mrecs[ kk ]       = mrecs_is[ ii ];
         mrecs[ kk ].taskx = kk;
      }

      for ( int ii = 0; ii < ncurve; ii++, kk++ )
      {
         mrecs[ kk ]       = mrecs_ds[ ii ];
         mrecs[ kk ].taskx = kk;
      }

      ctype             = CTYPE_ALL;
      mrecs[0].v_ctype  = ctype;          // Vector curve type is "All"
      plims_is[ 4 ]     = parlims[ 4 ];   // Set vbar20 for all types
      plims_ds[ 4 ]     = parlims[ 4 ];
DbgLv(1) << "iter_p: parlims"
 << parlims[0] << parlims[1] << parlims[2] << parlims[3] << parlims[4];
DbgLv(1) << "iter_p: parlims2"
 << parlims[5] << parlims[6] << parlims[7] << parlims[8] << parlims[9];
DbgLv(1) << "iter_p: parlims3"
 << parlims[10] << parlims[11] << parlims[12] << parlims[13] << parlims[14];
   }  // END:  records a mix of SL,IS,DS

   // Now reset original solutes and fill queue
   orig_solutes.clear();

   for ( int ii = 0; ii < mrecs.size(); ii++ )
   {
      orig_solutes << mrecs[ ii ].isolutes;
   }

   fill_queue();
//DbgTime() << "iter_pcsa  mrsz" << mrecs.size() << "oosz" << orig_solutes.size()
// << "gccnt" << gcores_count << "jqcnt" << job_queue.size();

   for ( int ii = 1; ii < gcores_count; ii++ )
      worker_status[ ii ] = READY;

   rmsd_last         = rmsd_curr;
   iterations++;
}

// Engineer Tikhonov Regularization for PCSA
void US_MPI_Analysis::tikreg_pcsa()
{
//*TEMP*
qDebug() << "Currently-unimplemented PCSA Tikhonov Regularization is skipped";
printf(  "Currently-unimplemented PCSA Tikhonov Regularization is skipped\n" );
//*TEMP*
}

// Engineer Monte Carlo for PCSA
void US_MPI_Analysis::montecarlo_pcsa()
{
//*TEMP*
qDebug() << "Currently-unimplemented PCSA Monte Carlo iterations is skipped";
printf(  "Currently-unimplemented PCSA Monte Carlo iterations is skipped\n" );
//*TEMP*
}

// Filter model records by a specified curve type
void US_MPI_Analysis::filter_mrecs( const int ctype,
      QVector< US_ModelRecord >& mrecs_a, QVector< US_ModelRecord >& mrecs_t )
{
   int namrec        = mrecs_a.size();
   int ntmrec        = namrec / 3;
   mrecs_t.clear();
   mrecs_t.reserve( ntmrec );

   for ( int ii = 0; ii < namrec; ii++ )
      if ( mrecs_a[ ii ].ctype == ctype )
         mrecs_t << mrecs_a[ ii ];
}

// Clean up model records to handle records with empty calculated solutes
void US_MPI_Analysis::clean_mrecs( QVector< US_ModelRecord >& mrecs )
{
   // First get index to last good
   int nmrec         = mrecs.size();
   int lmrec         = nmrec - 1;
   int goodx         = 0;

   for ( int ii = lmrec; ii > 1; ii-- )
   {
      if ( mrecs[ ii ].csolutes.size() != 0 )
      {
         goodx             = ii;
         break;
      }
   }

   // If last overall is good, return with no change to records
   if ( goodx == lmrec )
      return;

DbgLv(0) << " master:clean_mrecs: goodx lmrec" << goodx << lmrec;
   if ( mrecs[ 0 ].v_ctype != CTYPE_ALL )
   {  // If records of one type, just duplicate solutes from last good
      for ( int ii = goodx + 1; ii < nmrec; ii++ )
      {
         mrecs[ ii ].csolutes = mrecs[ goodx ].csolutes;
         mrecs[ ii ].rmsd     = mrecs[ goodx ].rmsd;
      }
   }

   else
   {  // If mixed records, duplicate solutes from last of each type
      int gx_sl         = 0;
      int gx_is         = 0;
      int gx_ds         = 0;
      int ktype         = 0;

      // Get indexes of last-good for each type
      for ( int ii = goodx; ii > 1; ii-- )
      {
         if ( mrecs[ ii ].ctype == CTYPE_SL )
         {
            if ( gx_sl > 0 )   continue;
            gx_sl             = ii;
            ktype++;
         }
         else if ( mrecs[ ii ].ctype == CTYPE_IS )
         {
            if ( gx_is > 0 )   continue;
            gx_is             = ii;
            ktype++;
         }
         else if ( mrecs[ ii ].ctype == CTYPE_DS )
         {
            if ( gx_ds > 0 )   continue;
            gx_ds             = ii;
            ktype++;
         }

         if ( ktype >= 3 )  break;
      }

      // Then replace each type with last good of that type
      for ( int ii = goodx + 1; ii < nmrec; ii++ )
      {
         if ( mrecs[ ii ].ctype == CTYPE_SL )
         {
            mrecs[ ii ].csolutes = mrecs[ gx_sl ].csolutes;
            mrecs[ ii ].rmsd     = mrecs[ gx_sl ].rmsd;
         }
         else if ( mrecs[ ii ].ctype == CTYPE_IS )
         {
            mrecs[ ii ].csolutes = mrecs[ gx_is ].csolutes;
            mrecs[ ii ].rmsd     = mrecs[ gx_is ].rmsd;
         }
         else if ( mrecs[ ii ].ctype == CTYPE_DS )
         {
            mrecs[ ii ].csolutes = mrecs[ gx_ds ].csolutes;
            mrecs[ ii ].rmsd     = mrecs[ gx_ds ].rmsd;
         }
      }

   }
}

