#include "us_mpi_analysis.h"
#include "us_lamm_astfvm.h"
#include "us_math2.h"

QDateTime elapsedd = QDateTime::currentDateTime();
#define ELAPSEDNOW (elapsedd.msecsTo(QDateTime::currentDateTime()))
#define ELAPSEDSEC (elapsedd.secsTo(QDateTime::currentDateTime()))
#define DbTimMsg(a) DbTiming << my_rank << generation << ELAPSEDNOW << a;
#define DRTiming    DbTiming << my_rank

#if 1
#define UPDATE_FIT
#endif

// Minimize prints debug level
//#define DL 0
#define DL 1

void US_MPI_Analysis::dmga_worker( void )
{
   current_dataset     = 0;
   datasets_to_process = data_sets.size();
   QStringList keys    = parameters.keys();
   s_grid              = ( keys.contains( "s_grid" ) )
                         ? parameters[ "s_grid" ].toInt() : 100;
   buckets.clear();

   simulation_values.noisflag   = 0;
   simulation_values.dbg_level  = dbg_level;
   simulation_values.dbg_timing = dbg_timing;

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
//if(my_rank>=0)
//abort( "DMGA_WORKER not yet implemented" );

   // Read in the constraints model and build constraints
   QString cmfname  = "../" + parameters[ "DC_model" ];
   wmodel.load( cmfname );                  // Load the constraints model
   constraints.load_constraints( &wmodel ); // Build the constraints object
   constraints.get_work_model  ( &wmodel ); // Get the base work model
DbgLv(1) << my_rank << "dmga_worker: cmfname" << cmfname;
DbgLv(1) << my_rank << "dmga_worker: wmodel #comps" << wmodel.components.size();

   MPI_GA_MSG msg;
   MPI_Status status;
   MPI_Job    job;
   bool       finished = false;
   int        grp_nbr  = ( my_rank / gcores_count );
   int        deme_nbr = my_rank - grp_nbr * gcores_count;
   // Get the number of floating attributes
   nfloatc             = constraints.float_constraints( &cns_flt );
   // Compute the number of possible float combinations
   nfvari              = ( 1 << nfloatc ) - 1;
   // Set up a working marker vector for a single gene.
   // Note: the "marker" is a simple vector of doubles, consisting of the
   //  attribute values that are unique to each gene. That is, only the
   //  float attributes differ from gene to gene and the specific values
   //  of these floats form a unique signature for each gene. Each gene
   //  itself can be formed by initializing with the base model (wmodel)
   //  and then setting the floating attribute values from the marker vector.
   dgmarker.resize( nfloatc );
   dgmsize             = nfloatc * sizeof( double );
   do_astfem           = ( wmodel.components[ 0 ].sigma == 0.0  &&
                           wmodel.components[ 0 ].delta == 0.0  &&
                           wmodel.coSedSolute < 0  &&
                           data_sets[ 0 ]->compress == 0.0 );
DbgLv(1) << my_rank << "dmga_worker: nfloatc nfvari dgmsize do_astfem"
 << nfloatc << nfvari << dgmsize << do_astfem;

   while ( ! finished )
   {
      dmga_worker_loop();

      msg.size = (int)max_rss();
      DbgLv(0) << "Deme" << grp_nbr << deme_nbr
         << ":   Generations finished, second" << ELAPSEDSEC;

      MPI_Send( &msg,           // This iteration is finished
                sizeof( msg ),  // to MPI #1
                MPI_BYTE,
                MPI_Job::MASTER,
                FINISHED,
                my_communicator );
DbgLv(1) << my_rank << "dmgw: FIN sent";

      MPI_Recv( &job,          // Find out what to do next
                sizeof( job ), // from MPI #0, MPI #7, MPI #9
                MPI_BYTE,
                MPI_ANY_SOURCE,
                MPI_ANY_TAG,
                my_communicator,
                &status );

      int dataset = job.dataset_offset;
      int count   = job.dataset_count;
      int length  = job.length;
DbgLv(1) << my_rank << "dmgw: job offs count len" << dataset << count << length
         << "tag" << status.MPI_TAG;
      int index   = 0;

      switch ( status.MPI_TAG )
      {
         case FINISHED: 
            finished = true;
            DbgLv(0) << "    Deme" << grp_nbr << deme_nbr << ":"
               << fitness_hits << "fitness hits" << "  maxrss" << maxrss;
            break;

         case UPDATE:   
            // Update data for global fit or Monte Carlo
            // Global fit comes before MC (if necessary), one dataset at a time
            // Monte Carlo always comes as a sequence of all datasets

            mc_data.resize( length );

            MPI_Barrier( my_communicator );

            // This is a receive
            MPI_Bcast( mc_data.data(),  // from MPI #8, #10
                       length,
                       MPI_DOUBLE,
                       MPI_Job::MASTER,
                       my_communicator );

            for ( int e = dataset; e < dataset + count; e++ )
            {
               US_DataIO::EditedData* data = &data_sets[ e ]->run_data;

               int scan_count    = data->scanCount();
               int radius_points = data->pointCount();

               for ( int s = 0; s < scan_count; s++ )
               {
                  US_DataIO::Scan* scan = &data->scanData[ s ];

                  for ( int r = 0; r < radius_points; r++ )
                  {
                     scan->rvalues[ r ] = mc_data[ index++ ];
                  }
               }
            }

            if ( count == data_sets.size() ) // Next iteration will be global
            {
               current_dataset     = 0;
               datasets_to_process = data_sets.size();
            }
            else // Next dataset is a part of global fit
            {
               current_dataset = dataset;
               // datasets_to_process will stay at 1
            }

            break;

         case GENERATION:   
            break;

         default:
            abort( "Unknown message at end of GA worker loop" );
            break;

      } // end switch
   }  // end while
}

void US_MPI_Analysis::dmga_worker_loop( void )
{
   // Initialize genes
   dgenes.clear();
   population = parameters[ "population" ].toInt();

   // Build the first generation of genes
   for ( int ii = 0; ii < population; ii++ )
      dgenes << new_dmga_gene();

   fitness.reserve( population );

   Fitness empty_fitness;
   empty_fitness.fitness = LARGE;

   for ( int ii = 0; ii < population; ii++ )
   {
      empty_fitness.index = ii;
      fitness << empty_fitness;
   }

   int generations = parameters[ "generations" ].toInt();
   int crossover   = parameters[ "crossover"   ].toInt();
   int mutation    = parameters[ "mutation"    ].toInt();
   int plague      = parameters[ "plague"      ].toInt();
   int elitism     = parameters[ "elitism"     ].toInt();

   int p_mutate    = mutation;
   int p_crossover = p_mutate + crossover;
   int p_plague    = p_crossover + plague;
   int grp_nbr     = ( my_rank / gcores_count );
   int deme_nbr    = my_rank - grp_nbr * gcores_count;

   fitness_map.clear();
   fitness_hits = 0;

   max_rss();

   QDateTime  start = QDateTime::currentDateTime();
   MPI_GA_MSG msg;

   for ( generation = 0; generation < generations; generation++ )
   {
      max_rss();

DbTimMsg("Worker start rank/generation/elapsed-secs");
      // Calculate fitness
      for ( int ii = 0; ii < population; ii++ )
      {
         fitness[ ii ].index   = ii;
         fitness[ ii ].fitness = get_fitness_dg( dgenes[ ii ] );
      }

      // Sort fitness
      qSort( fitness );
DbTimMsg("Worker after get_fitness loop + sort");

      // Refine with gradient search method (gsm) on last generation
      if ( generation == generations - 1 )
      {
DbgLv(DL) << "Deme" << grp_nbr << deme_nbr
         << ": At last generation minimize.";
DbTimMsg("Worker before gsm rank/generation/elapsed");

         fitness[ 0 ].fitness = minimize_dg( dgenes[ fitness[ 0 ].index ], 
                                             fitness[ 0 ].fitness );
DbgLv(DL) << "Deme" << grp_nbr << deme_nbr
         << ":   last generation minimize fitness=" << fitness[0].fitness;
DbTimMsg("Worker after gsm rank/generation/elapsed");
      }

      max_rss();

      // Send best gene to master
DbgLv(1) << "Best gene to master: gen" << generation << "worker" << deme_nbr;
dump_fitness( fitness );
      msg.generation = generation;
      dgene          = dgenes[ fitness[ 0 ].index ];
      marker_from_dgene( dgmarker, dgene );
      msg.size       = dgmsize;
      msg.fitness    = fitness[ 0 ].fitness;

      MPI_Send( &msg,                                // to MPI #1
                sizeof( msg ),
                MPI_BYTE,
                MPI_Job::MASTER,
                GENERATION,
                my_communicator );

      MPI_Send( dgmarker.data(),                     // to MPI #2
                dgmsize,
                MPI_DOUBLE,
                MPI_Job::MASTER,
                GENE,
                my_communicator );

DbTimMsg("Worker after send fitness,genes");
      // Receive instructions from master (continue or finish)
      MPI_Status status;

      MPI_Recv( &msg,                                // from MPI #3
                sizeof( msg ),
                MPI_BYTE,
                MPI_ANY_SOURCE,
                MPI_ANY_TAG,
                my_communicator,
                &status );
DbTimMsg("Worker after receive instructions");

      max_rss();

      if ( status.MPI_TAG == FINISHED )
      {
         DbgLv(0) << "Deme" << grp_nbr << deme_nbr
            << ": Finish signalled at deme generation" << generation + 1;
         break;
      }

      // See if we are really done
      if ( generation == generations - 1 )
      {
         DbgLv(0) << "Deme" << grp_nbr << deme_nbr
            << ": At last generation";
         continue;
      }

      // Mark duplicate genes 
      int f0 = 0;  // An index into the fitness array
      int f1 = 1;  // A second index
      // The value of 1.0e-8 for close fitness is arbitrary. Parameterize?
      const double NEAR_MATCH = 1.0e-8;
      const double EPSF_SCALE = 1.0e-3;
      double fitpwr      = (double)qRound( log10( fitness[ 0 ].fitness ) );
      double epsilon_f   = pow( 10.0, fitpwr ) * EPSF_SCALE;
DbgLv(1) << "gw:" << my_rank << ": Dup best-gene clean: fitness0 fitpwr epsilon_f"
 << fitness[0].fitness << fitpwr << epsilon_f;

      while ( f1 < population )
      {
         double fitdiff = qAbs( fitness[ f0 ].fitness - fitness[ f1 ].fitness );

         if ( fitdiff < epsilon_f )
         {
            bool match   = true;
            int  g0      = fitness[ f0 ].index;
            int  g1      = fitness[ f1 ].index;
            DGene gene0  = dgenes[ g0 ];
            DGene gene1  = dgenes[ g1 ];

            for ( int ii = 0; ii < nfloatc; ii++ )
            {
               double val0;
               double val1;
               US_dmGA_Constraints::AttribType
                     atype  = cns_flt[ ii ].atype;
               int   mcompx = cns_flt[ ii ].mcompx;

               fetch_attr_value( val0, gene0, atype, mcompx );
               fetch_attr_value( val1, gene1, atype, mcompx );

               double difv  = qAbs( ( val0 - val1 ) / val0 );

               if ( difv > NEAR_MATCH )
               {
DbgLv(1) << "gw:" << my_rank << ":  Dup NOT cleaned: f0 f1 fit0 fit1"
 << f0 << f1 << fitness[f0].fitness << fitness[f1].fitness << "ii g0 g1 g0v g1v"
 << ii << g0 << g1 << val0 << val1;
                  match        = false;
                  f0           = f1;
                  break;
               }
            }

            if ( match )
            {
DbgLv(1) << "gw:" << my_rank << ":  Dup cleaned: f0 f1 fit0 fit1"
 << f0 << f1 << fitness[f0].fitness << fitness[f1].fitness;
               fitness[ f1 ].fitness = LARGE;  // Invalidate gene/sim_values 
            }
         }
         else
            f0           = f1;

         f1++;
      }

      // Re-sort
      qSort( fitness );
DbgLv(1) << "gw:" << my_rank << ": fitness sorted";
      
      QList< DGene > old_genes = dgenes;

      // Create new generation from old
      // First copy elite genes
      for ( int gg = 0; gg < elitism; gg++ )
         dgenes[ gg ] = old_genes[ fitness[ gg ].index ];
DbgLv(1) << "gw:" << my_rank << ": elites copied";

      int immigr_count = migrate_dgenes();
DbgLv(1) << "gw:" << my_rank << ": immigr_count" << immigr_count
         << "dgenes,old sizes" << dgenes.size() << old_genes.size() << population;

DbTimMsg("Worker before elitism loop");

      for ( int gg = elitism + immigr_count; gg < population; gg++ )
      {
         // Select a random gene from old population using exponential 
         //  distribution
         int  gene_index  = e_random();
         int  probability = u_random( p_plague );
DbgLv(1) << "gw:" << my_rank << ":  gg" << gg << "gene_index" << gene_index
         << "probability" << probability;
         dgene            = old_genes[ gene_index ];

         // Do a self-mutate on some genes
         if      ( probability < p_mutate    ) mutate_dgene( dgene );

         // Do cross-mutation on some other genes
         else if ( probability < p_crossover ) cross_dgene ( dgene, old_genes );

         // Purge the remainder (replace with new gene)
         else                                  dgene = new_dmga_gene();

         dgenes[ gg ] = dgene;
      }
DbTimMsg("Worker after elitism loop");

      max_rss();

   }  // End of generation loop
DbTimMsg("  +++Worker after generation loop");
}

// Create a new discrete GA Gene
DGene US_MPI_Analysis::new_dmga_gene( void )
{
   DGene geneout = wmodel;      // Initialize gene to base model

   mutate_dgene( geneout );     // Perform a random mutation

   return geneout;              // Then return the new gene
}

// Mutate a discrete GA Gene
void US_MPI_Analysis::mutate_dgene( DGene& dgene )
{
   // Get a random mask that selects which float attributes to vary
   int smask     = u_random( nfvari ) + 1;
   double extn_s = (double)( s_grid - 1 );
   double sigma  = extn_s / ( 6.0 * ( log2( generation + 2 ) ) );
DbgLv(1) << my_rank << "dg:mutg: smask s_grid sigma" << smask << s_grid << sigma;

   // Loop thru float attributes, varying the selected ones
   for ( int ii = 0; ii < nfloatc; ii++ )
   {
      // Skip if this float attribute is not selected for this mutation
      if ( ( ( 1 << ii ) & smask ) == 0 )
         continue;

      // Get the current value of the attribute
      double vv;
      US_dmGA_Constraints::AttribType
           atype    = cns_flt[ ii ].atype;
      int  mcompx   = cns_flt[ ii ].mcompx;
      bool logscl   = cns_flt[ ii ].logscl;
      fetch_attr_value( vv, dgene, atype, mcompx );

      // Get float range and, if need be, convert to log scale
      double vl     = cns_flt[ ii ].low;
      double vh     = cns_flt[ ii ].high;
DbgLv(1) << my_rank << "dg:mutg:  ii" << ii
         << "vl vh" << vl << vh << "vv" << vv;

      if ( logscl )
      {
         vv            = log( vv );
         vl            = log( vl );
         vh            = log( vh );
      }

      // Mutate the value
      double xx     = US_Math2::box_muller( 0.0, sigma );
      double delta  = qRound( xx );
      vv           += ( delta * ( vh - vl ) / extn_s );
      vv            = qMax( vv, vl );
      vv            = qMin( vv, vh );
      vv            = logscl ? exp( vv ) : vv;
DbgLv(1) << my_rank << "dg:mutg:  ii" << ii << "  xx" << xx
         << "delta" << delta << "vv" << vv;

      // Update the mutated value
      store_attr_value( vv, dgene, atype, mcompx );
   }
//DbgLv(1) << "dg:mutgene dgene comps" << dgene.components.size();
}

// Cross-mutate a pair of discrete GA Genes
void US_MPI_Analysis::cross_dgene( DGene& dgene, QList< DGene > dgenes )
{
   // Get the crossing gene according to an exponential distribution
   int gndx      = e_random();
   DGene dgene2  = dgenes[ fitness[ gndx ].index ];

   // Get a random mask that selects float attributes from the second gene
   int smask     = u_random( nfvari ) + 1;
DbgLv(1) << "dg:crsg: gndx" << gndx << "smask" << smask;

   for ( int ii = 0; ii < nfloatc; ii++ )
   {
      // Skip if float attribute is not selected for cross mutation
      if ( ( ( 1 << ii ) & smask ) == 0 )
         continue;

      // Get the current value of the attribute from the second gene
      double vv;
      US_dmGA_Constraints::AttribType
          atype     = cns_flt[ ii ].atype;
      int mcompx    = cns_flt[ ii ].mcompx;
      fetch_attr_value( vv, dgene2, atype, mcompx );

      // Store this value in the first gene
      store_attr_value( vv, dgene,  atype, mcompx );
   }
}

// Migrate a list of discrete GA Genes
int US_MPI_Analysis::migrate_dgenes( void )
{
   static const int migrate_pcent = parameters[ "migration" ].toInt();
   static const int elitism_count = parameters[ "elitism"   ].toInt();

   QList < DGene >   emmigres;
   int migrate_count   = ( migrate_pcent * population + 50 ) / 100;
   int doubles_count   = migrate_count * nfloatc;
   QVector< double > emmigrants( doubles_count );
DbgLv(1) << my_rank << "dg:migrdg: migrate_count doubles_count" << migrate_count << doubles_count;

   // Send genes to master

   for ( int ii = 0; ii < migrate_count; ii++ )
   {  // Build the current list of emmigrating genes
      int gene_index     = e_random();
      emmigres << dgenes[ fitness[ gene_index ].index ];
   }

   // Represent migrating genes as marker vector
   dgenes_to_marker( emmigrants, emmigres, 0, migrate_count );
DbgLv(1) << my_rank << "dg:migrdg: emmigres size" << emmigres.size() << "emmigrants size" << emmigrants.size();

   // MPI send msg type
   MPI_GA_MSG msg;
   msg.size            = migrate_count;

   MPI_Send( &msg,                 // to MPI #1
             sizeof( msg ),
             MPI_BYTE,
             MPI_Job::MASTER,
             EMMIGRATE,
             my_communicator );

   // MPI send emmigrants
   MPI_Send( emmigrants.data(),    // to MPI #1
             doubles_count,
             MPI_DOUBLE,
             MPI_Job::MASTER,
             EMMIGRATE,
             my_communicator );

   // Get genes from master as marker vector
   QVector< double > immigres( doubles_count );
   MPI_Status        status;

   MPI_Recv( immigres.data(),      // to MPI #1
             doubles_count,
             MPI_DOUBLE,
             MPI_Job::MASTER,
             IMMIGRATE,
             my_communicator,
             &status );

   int doubles_sent;
   MPI_Get_count( &status, MPI_DOUBLE, &doubles_sent );
   int mgenes_count    = doubles_sent / nfloatc;
DbgLv(1) << my_rank << "dg:migrdg: immigres size" << immigres.size() << "doubles_sent" << doubles_sent;

   // The number returned equals the number sent or zero
   if ( mgenes_count > 0 )
   {
      marker_to_dgenes( immigres, dgenes, elitism_count, mgenes_count );
   }

   return mgenes_count;
}

// Find the minimum fitness value close to a discrete GA gene using
//  inverse hessian minimization
double US_MPI_Analysis::minimize_dg( DGene& dgene, double fitness )
{
   double fitnout  = fitness;
DbgLv(1) << "dg:minimize dgene comps" << dgene.components.size() << fitness;
   return fitnout;
}

// Get the fitness value for a discrete GA Gene
double US_MPI_Analysis::get_fitness_dg( DGene& dgene )
{
   QString fkey    = dgene_key( dgene );   // Get an identifying key string

   if ( fitness_map.contains( fkey ) )
   {  // We already have a match to this key, so use its fitness value
      fitness_hits++;
      return fitness_map.value( fkey );
   }

   US_SolveSim::Simulation sim = simulation_values;

   // Compute the simulation and residuals for this model
   calc_residuals_dmga( current_dataset, datasets_to_process, sim, dgene );

   double fitness  = sim.variance;         // Get the computed fitness
   fitness_map.insert( fkey, fitness );    // Add it the fitness map

//DbgLv(1) << "dg:get_fit fitness" << fitness;
   return fitness;
}

// Compose an identifying key string for a discrete GA Gene
QString US_MPI_Analysis::dgene_key( DGene& dgene )
{
   // Get the marker for this gene
   marker_from_dgene( dgmarker, dgene );

   // Compose a key string that is a concatenation of marker value strings
   QString str;
   QString fkey = str.sprintf( "%.5e", dgmarker[ 0 ] );

   for ( int ii = 1; ii < nfloatc; ii++ )
      fkey += str.sprintf( " %.5e", dgmarker[ ii ] );

DbgLv(1) << my_rank << "dg:dgkey" << fkey << "mv0 mvn" << dgmarker[0] << dgmarker[nfloatc-1];
   return fkey;
}

// Calculate residuals for a given discrete GA gene
void US_MPI_Analysis::calc_residuals_dmga( int offset, int dset_count,
      US_SolveSim::Simulation& sim_vals, DGene& dgene )
{
   int lim_offs    = offset + dset_count;
   int ntotal      = 0;
   int ntpoint     = 0;
   int ntscan      = 0;

   for ( int ee = offset; ee < lim_offs; ee++ )
   {  // Count scan,point totals for all data sets
      US_SolveSim::DataSet*  dset   = data_sets[ ee ];
      int npoints     = dset->run_data.xvalues.size();
      int nscans      = dset->run_data.scanData.size();
      ntotal         += ( nscans * npoints );
      ntpoint        += npoints;
      ntscan         += nscans;
   }

   US_DataIO::RawData* sdata = &sim_vals.sim_data;
   US_DataIO::RawData* resid = &sim_vals.residuals;
   int jscan       = 0;

   // Compute simulations for all data sets with the given model
   for ( int ee = offset; ee < lim_offs; ee++ )
   {
      US_SolveSim::DataSet*  dset   = data_sets[ ee ];
      US_DataIO::EditedData* edata  = &dset->run_data;
      US_DataIO::RawData     simdat;
      int nscans      = dset->run_data.scanData.size();

      // Initialize simulation data with experiment's grid
      US_AstfemMath::initSimData( simdat, *edata, 0.0 );

      // Create experimental-space model from DGene for the data set

      US_Math2::SolutionData sd;            // Set up data corrections
      double avtemp   = dset->temperature;
      sd.viscosity    = dset->viscosity;
      sd.density      = dset->density;
      sd.manual       = dset->manual;

      US_Model model  = dgene;              // Get base model
      model.update_coefficients();          // Compute missing coefficients

      if ( ee == offset )
         data_sets[ 0 ]->model = model;     // Save the s20w model

      for ( int cc = 0; cc < model.components.size(); cc++ )
      {  // Loop to perform data corrections to s and D
         sd.vbar20       = model.components[ cc ].vbar20;
         sd.vbar         = US_Math2::adjust_vbar20( sd.vbar20, avtemp );
         US_Math2::data_correction( avtemp, sd );

         model.components[ cc ].s  /= sd.s20w_correction;
         model.components[ cc ].D  /= sd.D20w_correction;
      }

      // Compute the simulation

      if ( do_astfem )
      {  // Calculate simulation for FEM (ideal) case
         US_Astfem_RSA astfem_rsa( model, dset->simparams );
         astfem_rsa.calculate( simdat );
      }

      else
      {  // Calculate simulation for FVM (non-ideal) case
         US_Buffer buffer;
         buffer.viscosity        = dset->viscosity;
         buffer.density          = dset->density;
         buffer.compressibility  = dset->compress;

         US_LammAstfvm astfvm( model, dset->simparams );
         astfvm.set_buffer( buffer );
         astfvm.calculate( simdat );
      }

      // Save simulation for this data set
      if ( ee == offset )
      {  // Initialize simulation data with (first) sim data
         *sdata          = simdat;
         jscan          += nscans;
      }
      else
      {  // Append sim_data for second and subsequent data set
         for ( int ss = 0; ss < nscans; ss++ )
         {
            sdata->scanData[ jscan++ ] = simdat.scanData[ ss ];
         }
      }
   }

   int ks          = 0;
   double variance = 0.0;

   // Compute overall residual and variance
   for ( int ee = 0; ee < lim_offs; ee++ )
   {
      US_SolveSim::DataSet*  dset   = data_sets[ ee ];
      US_DataIO::EditedData* edata  = &dset->run_data;
      US_DataIO::RawData     simdat;
      int npoints     = dset->run_data.pointCount();
      int nscans      = dset->run_data.scanCount();

      for ( int ss = 0; ss < nscans; ss++, ks++ )
      {
         resid->scanData[ ks ] = sdata->scanData[ ks ];

         for ( int rr = 0; rr < npoints; rr++ )
         {
            double resval   = edata->value( ss, rr )
                            - sdata->value( ks, rr );
            variance       += sq( resval );

            resid->setValue( ks, rr, resval );
         }
      }
   }

   sim_vals.variance = variance / (double)ntotal;
}

