#include "us_mpi_analysis.h"
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
if(my_rank>=0)
abort( "DMGA_WORKER not yet implemented" );

   // Initialize grid values
   QStringList keys = parameters.keys();

   s_grid = ( keys.contains( "s_grid" ) )
      ? parameters[ "s_grid" ].toInt() : 100;
   k_grid = ( keys.contains( "k_grid" ) )
      ? parameters[ "k_grid" ].toInt() : 100;

   static const double extn_s  = (double)( s_grid - 1 );
   static const double extn_k  = (double)( k_grid - 1 );

   for ( int b = 0; b < buckets.size(); b++ )
   {
      double x_min   = buckets[ b ].x_min;
      double x_max   = buckets[ b ].x_max;
      double y_min   = buckets[ b ].y_min;
      double y_max   = buckets[ b ].y_max;
      
      buckets[ b ].ds = ( x_max - x_min ) / extn_s;
      buckets[ b ].dk = ( y_max - y_min ) / extn_k;
   }

   MPI_GA_MSG msg;
   MPI_Status status;
   MPI_Job    job;
   bool       finished = false;
   int        grp_nbr  = ( my_rank / gcores_count );
   int        deme_nbr = my_rank - grp_nbr * gcores_count;

   while ( ! finished )
   {
      dmga_worker_loop();

      msg.size = max_rss();
      DbgLv(0) << "Deme" << grp_nbr << deme_nbr
         << ":   Generations finished, second" << ELAPSEDSEC;

      MPI_Send( &msg,           // This iteration is finished
                sizeof( msg ),  // to MPI #1
                MPI_BYTE,
                MPI_Job::MASTER,
                FINISHED,
                my_communicator );

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
   genes.clear();
   population = parameters[ "population" ].toInt();

   for ( int i = 0; i < population; i++ ) genes << new_gene();

//   // Let calc_residuals get the meniscus directly from the edited data for
//   // each data set
//   meniscus_value = -1.0;

   fitness.reserve( population );

   Fitness empty_fitness;
   empty_fitness.fitness = LARGE;

   for ( int i = 0; i < population; i++ )
   {
      empty_fitness.index = i;
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
      for ( int i = 0; i < population; i++ )
      {
         fitness[ i ].index   = i;
         fitness[ i ].fitness = get_fitness( genes[ i ] );
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

         fitness[ 0 ].fitness = minimize( genes[ fitness[ 0 ].index ], 
                                          fitness[ 0 ].fitness );
DbgLv(DL) << "Deme" << grp_nbr << deme_nbr
         << ":   last generation minimize fitness=" << fitness[0].fitness;
DbTimMsg("Worker after gsm rank/generation/elapsed");
      }

      max_rss();

      // Ensure gene is on grid
      align_gene( genes[ fitness[ 0 ].index ] );
DbTimMsg("Worker after align_gene");

      // Send best gene to master
DbgLv(1) << "Best gene to master: gen" << generation << "worker" << deme_nbr;
dump_fitness( fitness );
      msg.generation = generation;
      msg.size       = genes[ fitness[ 0 ].index ].size();
      msg.fitness    = fitness[ 0 ].fitness;

      MPI_Send( &msg,                                // to MPI #1
                sizeof( msg ),
                MPI_BYTE,
                MPI_Job::MASTER,
                GENERATION,
                my_communicator );

      MPI_Send( genes[ fitness[ 0 ].index ].data(),  // to MPI #2
                solute_doubles * buckets.size(),
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

            for ( int ii = 0; ii < buckets.size(); ii++ )
            {
               double sdif = qAbs( genes[ g0 ][ ii ].s -  genes[ g1 ][ ii ].s );
               double kdif = qAbs( genes[ g0 ][ ii ].k -  genes[ g1 ][ ii ].k );

               if ( sdif > NEAR_MATCH  ||  kdif > NEAR_MATCH )
               {
DbgLv(1) << "gw:" << my_rank << ":  Dup NOT cleaned: f0 f1 fit0 fit1"
 << f0 << f1 << fitness[f0].fitness << fitness[f1].fitness << "ii g0 g1 g0s g1s"
 << ii << g0 << g1 << genes[g0][ii].s << genes[f1][ii].s;
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
      
      QList< Gene > old_genes = genes;

      // Create new generation from old
      // First copy elite genes
      for ( int g = 0; g < elitism; g++ )
         genes[ g ] = old_genes[ fitness[ g ].index ];

      int immigrants = migrate_genes();

DbTimMsg("Worker before elitism loop");

      for ( int g = elitism + immigrants; g < population; g++ )
      {
         // Select a random gene from old population using exponential 
         //  distribution
         int  gene_index  = e_random();
         Gene gene        = old_genes[ gene_index ];
         int  probability = u_random( p_plague );

         if      ( probability < p_mutate    ) mutate_gene( gene );
         else if ( probability < p_crossover ) cross_gene ( gene, old_genes );
         else                                  gene = new_gene();

         genes[ g ] = gene;
      }
DbTimMsg("Worker after elitism loop");

      max_rss();

   }  // End of generation loop
DbTimMsg("  +++Worker after generation loop");
}

