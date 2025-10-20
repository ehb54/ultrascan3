#include "us_mpi_analysis.h"
#include "us_util.h"
#include "us_math2.h"
#include "us_settings.h"

void US_MPI_Analysis::dmga_master( void )
{
   current_dataset     = 0;
   datasets_to_process = data_sets.size();
   max_depth           = 0;
   calculated_solutes.clear();

   // Set noise and debug flags
   simulation_values.noisflag   = 0;
   simulation_values.dbg_level  = dbg_level;
   simulation_values.dbg_timing = dbg_timing;
DbgLv(0) << "DEBUG_LEVEL" << simulation_values.dbg_level;

   if ( data_sets.size() == 1 )
   {
      US_AstfemMath::initSimData( simulation_values.sim_data,
                                  data_sets[ 0 ]->run_data, 0.0 );
      US_AstfemMath::initSimData( simulation_values.residuals,
                                  data_sets[ 0 ]->run_data, 0.0 );
   }
   else
   {
      int ntscan        = data_sets[ 0 ]->run_data.scanCount();
      for ( int ii = 1; ii < data_sets.size(); ii++ )
         ntscan           += data_sets[ ii ]->run_data.scanCount();
      simulation_values.sim_data .scanData.resize( ntscan );
      simulation_values.residuals.scanData.resize( ntscan );
   }

   // Initialize best fitness
   best_dgenes .reserve( gcores_count );
   best_fitness.reserve( gcores_count );

   // Read in the constraints model and build constraints
   QString cmfname  = "../" + parameters[ "DC_model" ];
   wmodel.load( cmfname );                  // Load the constraints model
   constraints.load_constraints( &wmodel ); // Build the constraints object
   constraints.get_work_model  ( &wmodel ); // Get the base work model
DbgLv(1) << "dmga_master: cmfname" << cmfname;
DbgLv(1) << "dmga_master: wmodel #comps" << wmodel.components.size()
 << "#assoc" << wmodel.associations.size();

   // Report on the constraints attribute values and ranges

   DbgLv(0) << parameters[ "DC_model" ] << "Constraints --";

   QString attrtype = tr( "UNKNOWN" );
   US_dmGA_Constraints::AttribType atype;
   QVector< US_dmGA_Constraints::Constraint > cnsv;

   for ( int mcompx = 0; mcompx < wmodel.components.size(); mcompx++ )
   {
      int compno  = mcompx + 1;
      int ncompc  = constraints.comp_constraints( mcompx, &cnsv, NULL );

      DbgLv(0) << "  Component" << compno << ":";
//DbgLv(0) << "mcompx ncompc c0atype" << mcompx << ncompc << cnsv[0].atype
// << "_FF0" << US_dmGA_Constraints::ATYPE_FF0;

      for ( int cx = 0; cx < ncompc; cx++ )
      {
         atype       = cnsv[ cx ].atype;
//DbgLv(0) << " cx" << cx << "atype" << atype;
         bool floats = cnsv[ cx ].floats;
         bool logscl = cnsv[ cx ].logscl;
         double vmin = cnsv[ cx ].low;
         double vmax = cnsv[ cx ].high;
         attrtype    = tr( "UNKNOWN" );
//DbgLv(0) << "  cx" << cx << "fl ls mn mx" << floats << logscl << vmin << vmax;

         if      ( atype == US_dmGA_Constraints::ATYPE_S    )
            attrtype = tr( "Segmentation Coefficient" );
         else if ( atype == US_dmGA_Constraints::ATYPE_FF0  )
            attrtype = tr( "Frictional Ratio" );
         else if ( atype == US_dmGA_Constraints::ATYPE_MW   )
            attrtype = tr( "Molecular Weight" );
         else if ( atype == US_dmGA_Constraints::ATYPE_D    )
            attrtype = tr( "Diffusion Coefficient" );
         else if ( atype == US_dmGA_Constraints::ATYPE_F    )
            attrtype = tr( "Frictional Coefficient" );
         else if ( atype == US_dmGA_Constraints::ATYPE_VBAR )
            attrtype = tr( "Vbar (Specific Density)" );
         else if ( atype == US_dmGA_Constraints::ATYPE_CONC )
            attrtype = tr( "Signal Concentration" );
         else if ( atype == US_dmGA_Constraints::ATYPE_EXT  )
            attrtype = tr( "Extinction" );

//DbgLv(0) << "  cx" << cx << "attrtype" << attrtype;
         if ( floats )
         {
//DbgLv(0) << "      FLOATS";
            if ( logscl )
            {
//DbgLv(0) << "       LOGSCL";
               DbgLv(0) << "    " << attrtype << "floats from "
                        << vmin << "to" << vmax << "(log scale)";
            }
            else
            {
//DbgLv(0) << "       !LOGSCL";
               DbgLv(0) << "    " << attrtype << "floats from "
                        << vmin << "to" << vmax;
            }
         }
         else
         {
//DbgLv(0) << "      !FLOATS";
            DbgLv(0) << "    " << attrtype << "is fixed at "
                     << vmin;
         }
      }
   }

   for ( int assocx = 0; assocx < wmodel.associations.size(); assocx++ )
   {
      int compno  = assocx + 1;
      int nassoc  = constraints.assoc_constraints( assocx, &cnsv, NULL );
      int ncomp   = wmodel.associations[ assocx ].rcomps.size();

      DbgLv(0) << "  Reaction" << compno << ":";

      if ( ncomp == 2 )
      {
         DbgLv(0) << "    Reactant is component"
                  << wmodel.associations[ assocx ].rcomps[ 0 ] + 1
                  << ", Product is component"
                  << wmodel.associations[ assocx ].rcomps[ 1 ] + 1;
      }

      else
      {
         DbgLv(0) << "    Reactants are components"
                  << wmodel.associations[ assocx ].rcomps[ 0 ] + 1
                  << " and" << wmodel.associations[ assocx ].rcomps[ 1 ] + 1
                  << ", Product is component"
                  << wmodel.associations[ assocx ].rcomps[ 2 ] + 1;
      }

      for ( int rx = 0; rx < nassoc; rx++ )
      {
         atype       = cnsv[ rx ].atype;
         bool floats = cnsv[ rx ].floats;
         bool logscl = cnsv[ rx ].logscl;
         double vmin = cnsv[ rx ].low;
         double vmax = cnsv[ rx ].high;
         attrtype    = tr( "UNKNOWN" );

         if      ( atype == US_dmGA_Constraints::ATYPE_KD   )
            attrtype = tr( "K_Dissociation" );
         else if ( atype == US_dmGA_Constraints::ATYPE_KOFF )
            attrtype = tr( "k_Off Rate" );

         if ( floats )
         {
            if ( logscl )
            {
               DbgLv(0) << "    " << attrtype << "floats from "
                        << vmin << "to" << vmax << "(log scale)";
            }
            else
            {
               DbgLv(0) << "    " << attrtype << "floats from "
                        << vmin << "to" << vmax;
            }
         }
         else
         {
            DbgLv(0) << "    " << attrtype << "is fixed at "
                     << vmin;
         }
      }
   }
DbgLv(0) << " wmodel as1 K_d k_off" << wmodel.associations[0].k_d
 << wmodel.associations[0].k_off;

   Fitness empty_fitness;
   empty_fitness.fitness = LARGE;
   // Get base Gene and set up mutation controls
   dgene      = wmodel;
   nfloatc    = constraints.float_constraints( &cns_flt );
   nfvari     = ( 1 << nfloatc ) - 1;
   dgmarker.resize( nfloatc );
   do_astfem  = ( wmodel.components[ 0 ].sigma == 0.0  &&
                  wmodel.components[ 0 ].delta == 0.0  &&
                  wmodel.coSedSolute < 0  &&
                  data_sets[ 0 ]->compress == 0.0 );
   lfvari.fill( 0, nfvari );
//DbgLv(0) << " wmodel DUMP";
//wmodel.debug();

   // Initialize arrays
   for ( int ii = 0; ii < gcores_count; ii++ )
   {
      best_dgenes << dgene;

      empty_fitness.index = ii;
      best_fitness << empty_fitness;
   }

   QDateTime time = QDateTime::currentDateTime();

   // Handle Monte Carlo iterations.  There will always be at least 1.
   while ( true )
   {
      // Compute all generations of an MC iteration

      dmga_master_loop();

      // Get the best-fit gene

      std::sort( best_fitness.begin(), best_fitness.end() );
DbgLv(1) << "GaMast: EOML: fitness sorted  bfx" << best_fitness[0].index
         << "bestdg size" << best_dgenes.size();

      Fitness* bfit  = &best_fitness[ 0 ];
      DbgLv(0) << "Last generation best RMSD" << sqrt( bfit->fitness );

      if ( minimize_opt == 2 )
      {  // If gradient search method for all terminations, minimize now
         double aval;
         in_gsm         = true;
         double fitness = bfit->fitness;
         dgene          = best_dgenes[ bfit->index ];
         int nlim       = 0;

         bfit->fitness  = minimize_dmga( dgene, fitness );

         for ( int ii = 0; ii < nfloatc; ii++ )
         {  // Insure all the new gene attribute values are inside range
            US_dmGA_Constraints::AttribType
                 atype    = cns_flt[ ii ].atype;
            int  mcompx   = cns_flt[ ii ].mcompx;
            double vmin   = cns_flt[ ii ].low;
            double vmax   = cns_flt[ ii ].high;
            fetch_attr_value( aval, dgene, atype, mcompx );

            if ( aval < vmin  ||  aval > vmax )
            {
               aval          = qMax( vmin, qMin( vmax, aval ) );
               store_attr_value( aval, dgene, atype, mcompx );
               nlim++;
            }
         }

         if ( nlim > 0 )
         {  // If adjustments due to limits, recompute fitness
            bfit->fitness  = get_fitness_dmga( dgene );
         }

         DbgLv(0) << "Post-minimization RMSD" << sqrt( bfit->fitness );
         best_dgenes[ bfit->index ] = dgene;
         in_gsm         = false;
      }

      // Compute the variance (fitness) for the final best-fit model

DbgLv(1) << "GaMast: EOML: call calc_residuals";
      calc_residuals_dmga( 0, data_sets.size(), simulation_values, dgene );
DbgLv(1) << "GaMast: EOML: variance" << simulation_values.variance;

      // Output the model

      if ( data_sets.size() == 1 )
      {  // Output the single-data model
DbgLv(1) << "GaMast: EOML: CALL write_output";
         write_output();
      }
      else
      {  // Output the global model
         write_global();
      }

      // Handle any MonteCarlo iteration logic

DbgLv(1) << "GaMast:  mc_iter iters" << mc_iteration << mc_iterations;
      mc_iteration++;
      if ( mc_iterations > 1 )
      {
         qDebug() << "Fit RMSD" << sqrt( simulation_values.variance )
            << " of MC_Iteration" << mc_iteration;
         if ( mc_iteration < mc_iterations )
         {
            // Set scaled_data the first time
            if ( mc_iteration <= mgroup_count )
            {
               scaled_data = simulation_values.sim_data;
            }

            time_mc_iterations();

DbgLv(1) << "GaMast:    set_gaMC call";
            set_dmga_MonteCarlo();
DbgLv(1) << "GaMast:    set_gaMC  return";
         }
         else
            break;
      }
      else
      {
DbgLv(1) << "GaMast: FFrmsd: variance" << simulation_values.variance;
         qDebug() << "Final Fit RMSD" << sqrt( simulation_values.variance );
         break;
      }
   }

   DbgLv(0) << my_rank << ": Master signalling FINISHED to all Demes";

   // Report on the attribute values in the final model

   US_Model* fmodel = &data_sets[ 0 ]->model;
//*DEBUG
double vsum=0.0;
for (int ii=0; ii<simulation_values.sim_data.scanCount(); ii++)
 for (int jj=0; jj<simulation_values.sim_data.pointCount(); jj++ )
  vsum += sq(simulation_values.sim_data.value(ii,jj));
DbgLv(0) << "VSUM=" << vsum;
int hh=simulation_values.sim_data.pointCount()/2;
int ss=simulation_values.sim_data.scanCount();
DbgLv(0) << "SDAT 0-3"
 << simulation_values.sim_data.value(0,hh)
 << simulation_values.sim_data.value(1,hh)
 << simulation_values.sim_data.value(2,hh)
 << simulation_values.sim_data.value(3,hh);
DbgLv(0) << "SDAT *-n"
 << simulation_values.sim_data.value(ss-4,hh)
 << simulation_values.sim_data.value(ss-3,hh)
 << simulation_values.sim_data.value(ss-2,hh)
 << simulation_values.sim_data.value(ss-1,hh);
DbgLv(0) << "MDL comp1 s,k,w,D,f"
 << fmodel->components[0].s    / data_sets[0]->s20w_correction
 << fmodel->components[0].f_f0
 << fmodel->components[0].mw
 << fmodel->components[0].D    / data_sets[0]->D20w_correction
 << fmodel->components[0].f;
DbgLv(0) << "MDL comp2 s,k,w,D,f"
 << fmodel->components[1].s    / data_sets[0]->s20w_correction
 << fmodel->components[1].f_f0
 << fmodel->components[1].mw
 << fmodel->components[1].D    / data_sets[0]->D20w_correction
 << fmodel->components[1].f;
DbgLv(0) << "MDL asoc1 Kd,koff"
 << fmodel->associations[0].k_d
 << fmodel->associations[0].k_off;
DbgLv(0) << "DS dens visc manual temp"
 << data_sets[0]->density
 << data_sets[0]->viscosity
 << data_sets[0]->manual
 << data_sets[0]->temperature;
US_SimulationParameters* simpar = &data_sets[0]->simparams;
DbgLv(0) << "SIMP simpt mType gType rreso menis"
 << simpar->simpoints
 << simpar->meshType
 << simpar->gridType
 << simpar->radial_resolution
 << simpar->meniscus;
DbgLv(0) << "SIMP bott temp rnoise tinoise rinoise"
 << simpar->bottom
 << simpar->temperature
 << simpar->rnoise
 << simpar->tinoise
 << simpar->rinoise;
DbgLv(0) << "SIMP bform bvol bottpos rcoeffs"
 << simpar->band_forming
 << simpar->band_volume
 << simpar->bottom_position
 << simpar->rotorcoeffs[0]
 << simpar->rotorcoeffs[1];
US_SimulationParameters::SpeedProfile* spstep = &simpar->speed_step[0];
DbgLv(0) << "STEP0 durmin dlymin w2tf w2tl timf timl"
 << spstep->duration_minutes
 << spstep->delay_minutes
 << spstep->w2t_first
 << spstep->w2t_last
 << spstep->time_first
 << spstep->time_last;
DbgLv(0) << "STEP0 speed accel accelf"
 << spstep->rotorspeed
 << spstep->acceleration
 << spstep->acceleration_flag; 
//*DEBUG
   DbgLv(0) << fmodel->description;
   DbgLv(0) << " Final Model Attribute Values --";

   for ( int mcompx = 0; mcompx < fmodel->components.size(); mcompx++ )
   {
      int compno  = mcompx + 1;
      int ncompc  = constraints.comp_constraints( mcompx, &cnsv, NULL );

      DbgLv(0) << "  Component" << compno << ":";
//DbgLv(0) << "   mcompx" << mcompx << "ncompc" << ncompc;
//DbgLv(0) << "   at[n-2] at[n-1]" << cnsv[ncompc-2].atype << cnsv[ncompc-1].atype;

      for ( int cx = 0; cx < ncompc; cx++ )
      {
         atype       = cnsv[ cx ].atype;

         if      ( atype == US_dmGA_Constraints::ATYPE_S    )
            attrtype    = tr( "Segmentation Coefficient" );
         else if ( atype == US_dmGA_Constraints::ATYPE_FF0  )
            attrtype    = tr( "Frictional Ratio" );
         else if ( atype == US_dmGA_Constraints::ATYPE_MW   )
            attrtype    = tr( "Molecular Weight" );
         else if ( atype == US_dmGA_Constraints::ATYPE_D    )
            attrtype    = tr( "Diffusion Coefficient" );
         else if ( atype == US_dmGA_Constraints::ATYPE_F    )
            attrtype    = tr( "Frictional Coefficient" );
         else if ( atype == US_dmGA_Constraints::ATYPE_VBAR )
            attrtype    = tr( "Vbar (Specific Density)" );
         else if ( atype == US_dmGA_Constraints::ATYPE_CONC )
            attrtype    = tr( "Signal Concentration" );
         else if ( atype == US_dmGA_Constraints::ATYPE_EXT  )
            attrtype    = tr( "Extinction" );
         else
            attrtype    = tr( "Unknown" );

         double aval = constraints.fetch_attrib( fmodel->components[ mcompx ],
                                                 atype );
         DbgLv(0) << "    " << attrtype << "has a value of"
                  << aval;
      }
   }

   for ( int assocx = 0; assocx < fmodel->associations.size(); assocx++ )
   {
      int compno  = assocx + 1;
      int nassoc  = constraints.assoc_constraints( assocx, &cnsv, NULL );
      int ncomp   = fmodel->associations[ assocx ].rcomps.size();

      DbgLv(0) << "  Reaction" << compno << ":";

      if ( ncomp == 2 )
      {
         DbgLv(0) << "    Reactant is component"
                  << fmodel->associations[ assocx ].rcomps[ 0 ] + 1
                  << ", Product is component"
                  << fmodel->associations[ assocx ].rcomps[ 1 ] + 1;
      }

      else
      {
         DbgLv(0) << "    Reactants are components"
                  << fmodel->associations[ assocx ].rcomps[ 0 ] + 1
                  << " and" << fmodel->associations[ assocx ].rcomps[ 1 ] + 1
                  << ", Product is component"
                  << fmodel->associations[ assocx ].rcomps[ 2 ] + 1;
      }

      for ( int rx = 0; rx < nassoc; rx++ )
      {
         atype       = cnsv[ rx ].atype;

         if      ( atype == US_dmGA_Constraints::ATYPE_KD   )
            attrtype = tr( "K_Dissociation" );
         else if ( atype == US_dmGA_Constraints::ATYPE_KOFF )
            attrtype = tr( "k_Off Rate" );

         double aval = constraints.fetch_attrib( fmodel->associations[ assocx ],
                                                 atype );
         DbgLv(0) << "    " << attrtype << "has a value of"
                  << aval;
      }
   }

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

void US_MPI_Analysis::dmga_master_loop( void )
{
   static const double DIGIT_FIT      = 1.0e+4;
   static const int    min_generation = 10;
   static const int    _KS_BASE_      = 6;
   static const int    _KS_STEP_      = 3;
   QString dbgtext   = parameters[ "debug_text" ];
   QString s_ksbase  = par_key_value( dbgtext, "ksame_base" );
   QString s_ksstep  = par_key_value( dbgtext, "ksame_step" );
   int ks_base       = s_ksbase.isEmpty() ? _KS_BASE_ : s_ksbase.toInt();
   int ks_step       = s_ksstep.isEmpty() ? _KS_STEP_ : s_ksstep.toInt();
   ks_base           = qMax( 2, ks_base );
   ks_step           = qMax( 1, ks_step );
//   static const int    max_same_count = my_workers * 5;
   int    max_same_count       = ( ks_base + ( nfloatc / ks_step ) * ks_step )
                                 * my_workers;
   int    avg_generation       = 0;
   bool   early_termination    = false;
   int    fitness_same_count   = 0;
   double best_overall_fitness = LARGE;
   int    tag;
   int    workers  = my_workers;
DbgLv(1) << "dmga_master start loop:  gcores_count fitsize" << gcores_count
   << best_fitness.size() << "best_overall" << best_overall_fitness;
DbgLv(0) << "dmga_master start loop:  nfloatc max_same_count"
    << nfloatc << max_same_count << "ks_base ks_step" << ks_base << ks_step;

   // Reset best fitness for each worker
   for ( int i = 0; i < gcores_count; i++ )
   {
      best_fitness[ i ].fitness = LARGE;
      best_fitness[ i ].index   = i;
   }

   QList  < DGene > emigres;     // Holds genes passed as emmigrants
   QVector< int   > v_generations( gcores_count, 0 ); 
   int    sum      = 0;
   int    avg      = 0;
   long   rsstotal = 0L;
   double fit_power      = 5;
   double fit_digit      = 1.0e4;
   double fitness_round  = 1.0e5;


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

      max_rss();

      switch ( status.MPI_TAG )
      {
         case GENERATION:
            v_generations[ worker ] = msg.generation;

            sum = 0;
            for ( int i = 1; i <= my_workers; i++ ) 
               sum += v_generations[ i ];

            avg = qRound( (double)sum / (double)my_workers ) + 1;

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
                              + QString::number( current_dataset + 1 )
                              + " of " + QString::number( count_datasets );
                  else
                     progress += "; Datasets: "
                              + QString::number( datasets_to_process );
               }

               progress += "; MonteCarlo: " + QString::number( mc_iter );
               if ( best_overall_fitness != LARGE  &&  best_overall_fitness > 0.0 )
                  progress += "; RMSD: "
                            + QString::number( sqrt( best_overall_fitness ) );

               send_udp( progress );
            }

            // Get the best gene for the current generation from the worker
DbgLv(1) << "  MAST: work" << worker << "Recv#2 nfloatc" << nfloatc;
            MPI_Recv( dgmarker.data(),                 // MPI #2
                      nfloatc,
                      MPI_DOUBLE,  
                      worker,
                      GENE,
                      my_communicator,
                      MPI_STATUS_IGNORE );

DbgLv(1) << "  MAST: work" << worker << " dgm size" << dgmarker.size()
         << "bestdg size" << best_dgenes.size();
            dgene_from_marker( dgmarker, dgene );
            best_dgenes[ worker ]  = dgene;
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
DbgLv(1) << "master: worker/fitness/best gene" << worker <<  msg.fitness << dgene_key( best_dgenes[worker] );

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
//DbgLv(1) << "  best_overall_fitness" << best_overall_fitness
// << "fitness_same_count" << fitness_same_count
// << " early_term?" << early_termination;
if((worker%10)==1)
DbgLv(1) << worker << ": best_overall_fitness" << best_overall_fitness
 << "fitness_same_count" << fitness_same_count
 << " early_term?" << early_termination;

            // Tell the worker to either stop or continue
            tag            = early_termination ? FINISHED : GENERATION; 
DbgLv(1) << "dgmast: Send#3 tag" << tag << "( FINISHED,GENERATION" << FINISHED << GENERATION << " )";

            MPI_Send( &msg,            // MPI #3
                      0,               // Only interested in the tag 
                      MPI_BYTE,  
                      worker,
                      tag,
                      my_communicator );
            break;

         case FINISHED:
DbgLv(1) << "dgmast: Recv FINISH msg.size rsstotal" << msg.size << rsstotal;
            rsstotal += (long)msg.size;
            workers--;
            break;

         case EMMIGRATE:
         {
            // First get a set of genes as a concatenated marker vector.
            int gene_count    = msg.size;
            int doubles_count = gene_count * nfloatc;
            QVector< double > emmigrants( doubles_count ) ;
DbgLv(1) << "dgmast: Recv#4 EMMIG: gene_count doubles_count" << gene_count << doubles_count;

            MPI_Recv( emmigrants.data(),  // MPI #4
                      doubles_count,
                      MPI_DOUBLE,
                      worker,
                      EMMIGRATE,
                      my_communicator,
                      MPI_STATUS_IGNORE );

            // Add the genes to the emmigres list
            int emgx          = emigres.size();
DbgLv(1) << "dgmast: Recv#4 EMMIG: emgx" << emgx;
            marker_to_dgenes( emmigrants, emigres, emgx, gene_count );
DbgLv(1) << "dgmast: Recv#4 EMMIG: emigres size" << emigres.size();

//*DEBUG*
//if(emigres[0][0].s<0.0)
// DbgLv(0) << "MAST: **GENE s" << emigres[0][0].s << " Emigrant";
//*DEBUG*

            max_rss();

            // Don't send any back if the pool is too small
            if ( emigres.size() < gene_count * 5 ) doubles_count = 0;

            // Get immigrants from emmigres
            QVector< double > immigrants;
            dgmarker.resize( nfloatc );

            if ( doubles_count > 0 )
            {
               // Prepare a marker vector from the emmigrant list
               for ( int ii = 0; ii < gene_count; ii++ )
               {
                  dgene       = emigres.takeAt( u_random( emigres.size() ) );

                  marker_from_dgene( dgmarker, dgene );

                  immigrants += dgmarker;
               }
            }
            else
            {
DbgLv(1) << "dgmast: Send#5 IMMIG: count==0 migr data" << immigrants.data();
               immigrants.resize( 1 );
            }
DbgLv(1) << "dgmast: Send#5 IMMIG: immigrants size" << immigrants.size() << "doubles_count" << doubles_count
         << "migr data" << immigrants.data();

            MPI_Send( immigrants.data(),   // MPI #5
                      doubles_count,
                      MPI_DOUBLE,
                      worker,
                      IMMIGRATE,
                      my_communicator );
DbgLv(1) << "dgmast:   Send#5 IMMIG: complete";
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

void US_MPI_Analysis::dmga_global_fit( void ) 
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

// Use DMGA residuals as the standard deviation for varying data in MonteCarlo
void US_MPI_Analysis::set_dmga_gaussians( void ) 
{
   sigmas.clear();
   res_data    = &simulation_values.residuals;
   int ks      = 0;

   for ( int ee = 0; ee < data_sets.size(); ee++ )
   {
      int nscan   = data_sets[ ee ]->run_data.scanCount();
      int npoint  = data_sets[ ee ]->run_data.pointCount();

      // Place the residuals magnitudes into a single vector for convenience
      for ( int ss = 0; ss < nscan; ss++, ks++ )
      {
         for ( int rr = 0; rr < npoint; rr++ )
         {
            sigmas << qAbs( res_data->value( ks, rr ) );
         }
      }
   }
}

// Set up next MonteCarlo iteration for dmGA
void US_MPI_Analysis::set_dmga_MonteCarlo( void ) 
{
DbgLv(1) << "sdMC: mciter" << mc_iteration;
   // This is almost the same as ga set_monteCarlo
   if ( mc_iteration <= mgroup_count )
   {
      max_depth   = 0;  // Make the datasets compatible
      calculated_solutes.clear();

      set_dmga_gaussians();
   }

   mc_data.resize( total_points );
   int index = 0;

   // Get a randomized variation of the concentrations
   // Use a gaussian distribution with the residual as the standard deviation
   for ( int ee = 0; ee < data_sets.size(); ee++ )
   {
      US_DataIO::EditedData* edata = &data_sets[ ee ]->run_data;

      int scan_count    = edata->scanCount();
      int radius_points = edata->pointCount();

      for ( int ss = 0; ss < scan_count; ss++ )
      {
         for ( int rr = 0; rr < radius_points; rr++ )
         {
            double variation = US_Math2::box_muller( 0.0, sigmas[ index ] );
            mc_data[ index ] = scaled_data.value( ss, rr ) + variation;
            index++;
         }
      }
   }
DbgLv(1) << "sdMC: mc_data set index" << index
         << "total_points" << total_points;

   // Broadcast Monte Carlo data to all workers
   MPI_Job job;
   job.command        = MPI_Job::NEWDATA;
   job.length         = total_points;
   job.dataset_offset = 0;
   job.dataset_count  = data_sets.size();

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
   MPI_Barrier( my_communicator );

   MPI_Bcast( mc_data.data(),   // MPI #10
              total_points, 
              MPI_DOUBLE, 
              MPI_Job::MASTER, 
              my_communicator );
}

// Get a marker vector of doubles from a dmga gene (model)
void US_MPI_Analysis::marker_from_dgene( QVector< double >& dgm, DGene& dg )
{
   // Fetch the current value for each floating attribute and store in marker
   for ( int ii = 0; ii < nfloatc; ii++ )
   {
      fetch_attr_value( dgm[ ii ], dg, cns_flt[ ii ].atype,
                        cns_flt[ ii ].mcompx );
   }
}

// Get a dmga gene (model) from a marker vector of doubles
void US_MPI_Analysis::dgene_from_marker( QVector< double >& dgm, DGene& dg )
{
   // Initial gene is the base work model
   dg          = wmodel;

   // Store the current value for each floating attribute into the gene
   for ( int ii = 0; ii < nfloatc; ii++ )
   {
      store_attr_value( dgm[ ii ], dg, cns_flt[ ii ].atype,
                        cns_flt[ ii ].mcompx );
   }
}

// Get a marker vector of doubles from multiple dmga genes (models)
void US_MPI_Analysis::dgenes_to_marker( QVector< double >& dgm,
      QList< DGene >& dgenes, const int stgx, const int ngenes )
{
   int mx       = 0;            // Initial marker index
   int gx       = stgx;         // Initial genes index
   DGene wkgene = wmodel;       // Base gene

   for ( int kg = 0; kg < ngenes; kg++ )
   {  // Fetch each gene and store its float attribute values
      wkgene        = dgenes[ gx++ ];

      for ( int ii = 0; ii < nfloatc; ii++ )
      {  // Fetch each gene float attribute value into the marker vector
         fetch_attr_value( dgm[ mx++ ], wkgene, cns_flt[ ii ].atype,
                           cns_flt[ ii ].mcompx );
      }
   }
}

// Get multiple dmga genes (models) from a marker vector of doubles
void US_MPI_Analysis::marker_to_dgenes( QVector< double >& dgm,
      QList< DGene >& dgenes, const int stgx, const int ngenes )
{
   int mx       = 0;               // Initial marker index
   int gx       = stgx;            // Initial genes index
   int igsize   = dgenes.size();   // Initial genes list size
   DGene wkgene = wmodel;          // Base gene

   for ( int kg = 0; kg < ngenes; kg++ )
   {  // Build and store each gene

      for ( int ii = 0; ii < nfloatc; ii++, gx++ )
      {  // Store each float attribute value into the gene
         store_attr_value( dgm[ mx++ ], wkgene, cns_flt[ ii ].atype,
                           cns_flt[ ii ].mcompx );
      }

      if ( gx >= igsize )
      {
         dgenes << wkgene;         // Append the constructed gene
         igsize++;
      }
      else
         dgenes[ gx ] = wkgene;    // Store the constructed gene
   }
}

// Store a component/association attribute value
bool US_MPI_Analysis::store_attr_value( double& aval, US_Model& model,
      US_dmGA_Constraints::AttribType& atype, int& mcompx )
{
   bool is_ok  = true;
   US_Model::SimulationComponent* sc = NULL;
   US_Model::Association*         as = NULL;

   if ( atype < US_dmGA_Constraints::ATYPE_KD )
      sc  = &model.components  [ mcompx ];
   else
      as  = &model.associations[ mcompx ];

   switch( atype )
   {
      case US_dmGA_Constraints::ATYPE_S:
         sc->s                    = aval;
         break;
      case US_dmGA_Constraints::ATYPE_FF0:
         sc->f_f0                 = aval;
         break;
      case US_dmGA_Constraints::ATYPE_MW:
         sc->mw                   = aval;
         break;
      case US_dmGA_Constraints::ATYPE_D:
         sc->D                    = aval;
         break;
      case US_dmGA_Constraints::ATYPE_F:
         sc->f                    = aval;
         break;
      case US_dmGA_Constraints::ATYPE_VBAR:
         sc->vbar20               = aval;
         break;
      case US_dmGA_Constraints::ATYPE_CONC:
         sc->signal_concentration = aval;
         if ( sc->extinction != 0.0 )
            sc->molar_concentration  = aval / sc->extinction;
         break;
      case US_dmGA_Constraints::ATYPE_EXT:
         sc->extinction           = aval;
         if ( aval != 0.0 )
            sc->molar_concentration  = sc->signal_concentration / aval;
         break;
      case US_dmGA_Constraints::ATYPE_KD:
         as->k_d                  = aval;
         break;
      case US_dmGA_Constraints::ATYPE_KOFF:
         as->k_off                = aval;
         break;
      default:
         is_ok    = false;
         break;
   }

   return is_ok;
}

// Fetch a component/association attribute value
bool US_MPI_Analysis::fetch_attr_value( double& aval, US_Model& model,
      US_dmGA_Constraints::AttribType& atype, int& mcompx )
{
   bool is_ok  = true;
   US_Model::SimulationComponent* sc = NULL;
   US_Model::Association*         as = NULL;

   if ( atype < US_dmGA_Constraints::ATYPE_KD )
      sc  = &model.components  [ mcompx ];
   else
      as  = &model.associations[ mcompx ];

   switch( atype )
   {
      case US_dmGA_Constraints::ATYPE_S:
         aval    = sc->s;
         break;
      case US_dmGA_Constraints::ATYPE_FF0:
         aval    = sc->f_f0;
         break;
      case US_dmGA_Constraints::ATYPE_MW:
         aval    = sc->mw;
         break;
      case US_dmGA_Constraints::ATYPE_D:
         aval    = sc->D;
         break;
      case US_dmGA_Constraints::ATYPE_F:
         aval    = sc->f;
         break;
      case US_dmGA_Constraints::ATYPE_VBAR:
         aval    = sc->vbar20;
         break;
      case US_dmGA_Constraints::ATYPE_CONC:
         aval    = sc->signal_concentration;
         break;
      case US_dmGA_Constraints::ATYPE_EXT:
         aval    = sc->extinction;
         break;
      case US_dmGA_Constraints::ATYPE_KD:
         aval    = as->k_d;
         break;
      case US_dmGA_Constraints::ATYPE_KOFF:
         aval    = as->k_off;
         break;
      default:
         is_ok    = false;
         break;
   }

   return is_ok;
}

// Get an apply-ready model from a dmga gene
void US_MPI_Analysis::model_from_dgene( US_Model& model, DGene& dgene )
{
   model        = dgene;           // Initialize the model

   model.update_coefficients();    // Compute missing coefficients
//*DEBUG*
if(group_rank<1) {
DbgLv(2) << my_rank << "MFG:g-comp1 s,k,w,d,f"
 << dgene.components[0].s
 << dgene.components[0].f_f0
 << dgene.components[0].mw
 << dgene.components[0].D
 << dgene.components[0].f;
DbgLv(2) << my_rank << "MFG:m-comp1 s,k,w,d,f"
 << model.components[0].s
 << model.components[0].f_f0
 << model.components[0].mw
 << model.components[0].D
 << model.components[0].f;
}
//*DEBUG*

   for ( int ii = 0; ii < model.associations.size(); ii++ )
   {  // Modify some coefficients based on associations
      US_Model::Association* as  = &model.associations[ ii ];

      int nrco      = as->rcomps.size();                 // Components involved
      int rpx       = nrco - 1;                          // Last comp index
      int rc1       = as->rcomps[ 0 ];                   // Comp indexes
      int rc2       = nrco == 2 ? rc1 : as->rcomps[ 1 ];
      int rcp       = as->rcomps[ rpx ];
      int st1       = as->stoichs[ 0 ];                  // Stoichiometries
      int st2       = qAbs( as->stoichs[ 1 ] );
      int stp       = qAbs( as->stoichs[ rpx ] );
      double ff0p   = qMax( 1.0, model.components[ rcp ].f_f0 );
      model.components[ rcp ] = dgene.components[ rcp ]; // Raw product comp
if(group_rank<2)
DbgLv(1) << my_rank << "MFG: ii" << ii << "nrco,rc1,rc2,rcp"
 << nrco << rc1 << rc2 << rcp << "st1,st2,stp" << st1 << st2 << stp;

      // Reset concentration for reactant(s) and product
      double cval   = model.components[ rc1 ].signal_concentration;
      model.components[ rc2 ].signal_concentration  = cval;
      //model.components[ rcp ].signal_concentration  = cval;
      model.components[ rcp ].signal_concentration  = 0.0;
if(group_rank<2)
DbgLv(1) << my_rank << "MFG:   orig cval wval vval" << cval
 << model.components[rcp].mw << model.components[rcp].vbar20;

      // Reset molecular weight and vbar for product
      double wval   = model.components[ rc1 ].mw         * (double)st1;
      double vsum   = model.components[ rc1 ].vbar20     * wval;
      double esum   = model.components[ rc1 ].extinction * (double)st1;
      double wsum   = wval;

      if ( nrco > 2 )
      {
         wval          = model.components[ rc2 ].mw         * (double)st2;
         vsum         += model.components[ rc2 ].vbar20     * wval;
         esum         += model.components[ rc2 ].extinction * (double)st2;
         wsum         += wval;
      }

      model.components[ rcp ].vbar20     = vsum / wsum;
      model.components[ rcp ].mw         = wsum;
      model.components[ rcp ].f_f0       = ff0p;
      model.components[ rcp ].s          = 0.0;
      model.components[ rcp ].D          = 0.0;
      model.components[ rcp ].f          = 0.0;
      model.components[ rcp ].extinction = esum;

      // Recompute coefficients with specified vbar,mw,f/f0
      model.calc_coefficients( model.components[ rcp ] );
if(group_rank<2)
DbgLv(1) << my_rank << "MFG:     rcp" << rcp << "c.s c.k c.mw c.vb"
 << model.components[rcp].s << model.components[rcp].f_f0
 << model.components[rcp].mw << model.components[rcp].vbar20;
   }
}

