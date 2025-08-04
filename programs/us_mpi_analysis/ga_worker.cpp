#include "us_mpi_analysis.h"
#include "us_math2.h"

QDateTime elapsed = QDateTime::currentDateTime();
#define ELAPSEDNOW (elapsed.msecsTo(QDateTime::currentDateTime()))
#define ELAPSEDSEC (elapsed.secsTo(QDateTime::currentDateTime()))
#define DbTimMsg(a) DbTiming << my_rank << generation << ELAPSEDNOW << a;
#define DRTiming    DbTiming << my_rank

#if 1
#define UPDATE_FIT
#endif

// Minimize prints debug level
//#define DL 0
#define DL 1

void US_MPI_Analysis::ga_worker( void )
{
   current_dataset     = 0;
   count_datasets      = data_sets.size();
   datasets_to_process = count_datasets;

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
      ga_worker_loop();

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
               << fitness_hits << "fitness hits of" << fitness_count
               << " fitness checks   maxrss" << maxrss;
            break;

         case UPDATE:   
            // Update data for global fit or Monte Carlo
            // Global fit comes before MC (if necessary), one dataset at a time
            // Monte Carlo always comes as a sequence of all datasets

            mc_data.resize( length );
DbgLv(1) << "Deme" << deme_nbr << "UPD ds cnt len" << dataset << count << length;

            MPI_Barrier( my_communicator );

            // This is a receive
            MPI_Bcast( mc_data.data(),  // from MPI #8, #10
                       length,
                       MPI_DOUBLE,
                       MPI_Job::MASTER,
                       my_communicator );

            if ( is_global_fit  &&  count == 1 )
            {  // For global update to scaled data, extra is new ODLimit
               length--;
               data_sets[ dataset ]->run_data.ODlimit = mc_data[ length ];
            }

            for ( int ee = dataset; ee < dataset + count; ee++ )
            {
               US_DataIO::EditedData* edata = &data_sets[ ee ]->run_data;

               int scan_count    = edata->scanCount();
               int radius_points = edata->pointCount();
DbgLv(1) << "Deme" << deme_nbr << "  ee" << ee << "scnt rcnt" << scan_count << radius_points;
double dsumi=0.0;
double dsumo=0.0;

               for ( int ss = 0; ss < scan_count; ss++ )
               {
                  for ( int rr = 0; rr < radius_points; rr++, index++ )
                  {
dsumi+=edata->value(ss,rr);
                     edata->setValue( ss, rr, mc_data[ index ] );
dsumo+=mc_data[index];
                  }
               }
DbgLv(1) << "Deme" << deme_nbr << "  dsumi" << dsumi << "dsumo" << dsumo;
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

void US_MPI_Analysis::ga_worker_loop( void )
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
   fitness_hits    = 0;
   fitness_count   = 0;

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

void US_MPI_Analysis::align_gene( Gene& gene )
{
   int grid_es = s_grid - 1;
   int grid_ek = k_grid - 1;

   for ( int i = 0; i < gene.size(); i++ )
   {
      double s     = gene[ i ].s;
      double s_min = buckets[ i ].x_min;
      double ds    = s - s_min;

      if ( ds < 0 )
         s = s_min;
      else
      {
         int gridpoint = qRound( ds / buckets[ i ].ds );
         gridpoint     = qMin( grid_es, gridpoint );
         s             = s_min + gridpoint * buckets[ i ].ds;
      }

      double k     = gene[ i ].k;
      double k_min = buckets[ i ].y_min;
      double dk    = k - k_min;

      if ( dk < 0 )
         k = k_min;
      else
      {
         int gridpoint = qRound( dk / buckets[ i ].dk );
         gridpoint     = qMin( grid_ek, gridpoint );
         k             = k_min + gridpoint * buckets[ i ].dk;
      }

      gene[ i ].s = s;
      gene[ i ].k = k;
//*DEBUG*
//if(gene[i].s<0.0) {
//int grp_nbr  = ( my_rank / gcores_count );
//int deme_nbr = my_rank - grp_nbr * gcores_count;
//DbgLv(0) << "Dem :" << grp_nbr << deme_nbr << "gene.s" << gene[i].s << "ALIGN"
// << "ds s_min gridpt" << ds << s_min << qRound( ds / buckets[i].ds );
//}
//*DEBUG*
   }
}

double US_MPI_Analysis::get_fitness( const Gene& gene )
{
   US_SolveSim::Simulation sim = simulation_values;
sim.dbg_level = qMax(0,dbg_level-1);
   sim.solutes = gene;
   qSort( sim.solutes );

   fitness_count++;
   int     nisols = gene.size();
   QString key    = "";
   QString str;

   for ( int cc = 0; cc < nisols; cc++ )
   {  // Concatenate all solute s,k values to form fitness key
      key += str.sprintf( "%.5f%.5f", sim.solutes[ cc ].s,
                                      sim.solutes[ cc ].k );
   }

DbgLv(2) << "get_fitness: nisols" << nisols << "key" << key;
   if ( fitness_map.contains( key ) )
   {  // We already have a match to this key, so use its fitness value
      fitness_hits++;
DbgLv(2) << "get_fitness: HIT!  new hits" << fitness_hits;
      return fitness_map.value( key );
   }

   solutes_from_gene( sim.solutes, nisols );

//DbTimMsg("  ++ call gf calc_residuals");
   calc_residuals( current_dataset, datasets_to_process, sim );
//DbTimMsg("  ++  return calc_residuals");

   double fitness      = sim.variance;
   int    solute_count = 0;
   int    nosols       = sim.solutes.size();

   if ( data_sets.size() == 1 )
   {  // Count solutes whose concentration is at least the threshold
      for ( int cc = 0; cc < nosols; cc++ )
      {
         if ( sim.solutes[ cc ].c >= concentration_threshold ) solute_count++;
      }
DbgLv(2) << "get_fitness: sol_count conc_thresh" << solute_count << concentration_threshold;
   }

   else
   {  // For multiple datasets, use a weighted average fitness
      double concen    = 0.0;
      fitness          = 0.0;

      for ( int ee = 0; ee < datasets_to_process; ee++ )
      {
         fitness         += ( sim.variances[ ee ] / maxods[ ee ] );
      }

      fitness         /= (double)datasets_to_process;

      for ( int cc = 0; cc < nosols; cc++ )
         concen          += sim.solutes[ cc ].c;

      double cthresh   = concentration_threshold * concen;

      for ( int cc = 0; cc < nosols; cc++ )
      {
         if ( sim.solutes[ cc ].c >= cthresh ) solute_count++;
      }
   }

   fitness *= ( 1.0 + sq( regularization * solute_count ) );
   fitness_map.insert( key, fitness ); 
DbgLv(2) << "get_fitness:  out fitness" << fitness;
//*DEBUG*
if(dbg_level>0 && fitness_map.size()==20 )
{
 int n=nosols-1;
 DbgLv(1) << "w:" << my_rank << generation << ": fmapsize fitness nsols"
  << fitness_map.size() << fitness << nisols << nosols
  << "s0 s,k,v" << sim.solutes[0].s << sim.solutes[0].k << sim.solutes[0].v
  << "sn s,k,v" << sim.solutes[n].s << sim.solutes[n].k << sim.solutes[n].v;
}
//*DEBUG*

//*DEBUG*
//if(datasets_to_process>1 && generation<11 )
if(dbg_level>1 && datasets_to_process>1 && generation<11 )
{
 DbgLv(0) << "rank generation" << my_rank << generation << "fitness" << fitness
  << "vari0 vari1" << sim.variances[0] << sim.variances[1];
}
//*DEBUG*
   return fitness;
}

// Compute the fitness of a gene represented as a vector of doubles
double US_MPI_Analysis::get_fitness_v( const US_Vector& vv )
{
   // Convert from US_Vector to Gene
   int  size  = vv.size() / 2;
   int  index = 0;
   Gene gene( size );
   
   for ( int ii = 0; ii < size; ii++ )
   {
      gene[ ii ].s = vv[ index++ ];
      gene[ ii ].k = vv[ index++ ];
   }

//*DEBUG*
if(gene[0].s<0.0) {
int grp_nbr  = ( my_rank / gcores_count );
int deme_nbr = my_rank - grp_nbr * gcores_count;
DbgLv(DL) << "Dem :" << grp_nbr << deme_nbr << "gene.s" << gene[0].s << "FIT_V";
}
//*DEBUG*
//   return get_fitness( gene );
   int dbgsv = dbg_level;
//   dbg_level = 2;
   double fitn = get_fitness( gene );
   dbg_level = dbgsv;
   return fitn;
}

int US_MPI_Analysis::u_random( int modulo )
{
   // Returns a uniform random integer between 0 and modulo - 1
   // Default modulo is 100
   return (int)qFloor( (double)modulo * US_Math2::ranf() );
}

int US_MPI_Analysis::e_random( void )
{
   // Exponential distribution
   double       randx   = US_Math2::ranf();
   const double divisor = 8.0;  // Parameterize?
   static const double
                beta    = population / divisor;
   int          gnsize  = ( buckets.size() > 0 ) ? genes.size() : dgenes.size();

   int gene_index = (int)( -log( 1.0 - randx ) * beta );
       gene_index = qMin( ( gnsize - 1 ), qMax( 0, gene_index ) );
   
   return gene_index;
}

void US_MPI_Analysis::mutate_gene( Gene& gene )
{
   static const int p_mutate_s  = parameters[ "p_mutate_s"  ].toInt();
   static const int p_mutate_k  = parameters[ "p_mutate_k"  ].toInt();
   static const int p_mutate_sk = parameters[ "p_mutate_sk" ].toInt();

   static const int p_k         = p_mutate_s + p_mutate_k;   // e.g., 40
   static const int p_total     = p_k + p_mutate_sk;         // e.g., 60

   int solute = u_random( gene.size() );
   int rand   = u_random( p_total );

   if      ( rand < p_mutate_s )
      mutate_s( gene[ solute ], solute );
   else if ( rand < p_k )      
      mutate_k( gene[ solute ], solute );
   else
   {
      mutate_s( gene[ solute ], solute );
      mutate_k( gene[ solute ], solute );
   }
//*DEBUG*
//if(gene[0].s<0.0) {
//int grp_nbr  = ( my_rank / gcores_count );
//int deme_nbr = my_rank - grp_nbr * gcores_count;
//DbgLv(0) << "Dem :" << grp_nbr << deme_nbr << "gene.s" << gene[0].s << "MUTATE";
//}
//*DEBUG*
}

void US_MPI_Analysis::mutate_s( US_Solute& solute, int b )
{
   // Consider paramaterizing the sigma and x calculations
   double sigma = ( s_grid - 1 ) / ( 6.0 * ( log2( generation + 2 ) ) );
   double x     = US_Math2::box_muller( 0.0, sigma );
   double delta = qRound( x );
//DbgLv(1) << "    MUTATE_S x" << x << "sg sigma delta"
//   << s_grid << sigma << delta;

   // Ensure new value is at a grid point and within bucket
   solute.s += delta * buckets[ b ].ds;
   solute.s  = qMax( solute.s, buckets[ b ].x_min );
   solute.s  = qMin( solute.s, buckets[ b ].x_max );
//*DEBUG*
//if(solute.s<0.0) {
//int grp_nbr  = ( my_rank / gcores_count );
//int deme_nbr = my_rank - grp_nbr * gcores_count;
//DbgLv(0) << "Dem :" << grp_nbr << deme_nbr << "  solute.s" << solute.s << "MUTATE_S";
//}
//*DEBUG*
}

void US_MPI_Analysis::mutate_k( US_Solute& solute, int b )
{
   //static const double mutate_sigma = parameters[ "mutate_sigma" ].toDouble();

   double sigma   = ( k_grid - 1 ) / ( 6.0 * ( log2( generation + 2 ) ) );
   double x       = US_Math2::box_muller( 0.0, sigma );
   double delta   = qRound( x );
//DbgLv(1) << "     MUTATE_K x" << x << "kg sigma delta"
//   << k_grid << sigma << delta;

   // Ensure new value is at a grid point and within bucket
   solute.k += delta * buckets[ b ].dk;
   solute.k  = qMax( solute.k, buckets[ b ].y_min );
   solute.k  = qMin( solute.k, buckets[ b ].y_max );
}

void US_MPI_Analysis::cross_gene( Gene& gene, QList< Gene > old_genes )
{
   // Get the crossing gene according to an exponential distribution
   int  gene_index = e_random();
   Gene cross_from = old_genes[ fitness[ gene_index ].index ]; 

   // Select a random solute.  The number will always be between
   // 1 and size - 1.
   int solute_index = u_random( gene.size() - 1 ) + 1;
   
   // Swap the genes half the time before cross so if we have
   // (abcde, ABCDE) the crossed gene will be either abCDE or ABcde
   if ( u_random() < 50 ) 
   {
      Gene temp  = gene;
      gene       = cross_from;
      cross_from = temp;
   }

   for ( int i = solute_index; i < buckets.size(); i++ )
   {
      gene[ i ] = cross_from[ i ];
   }
//*DEBUG*
//if(gene[0].s<0.0) {
//int grp_nbr  = ( my_rank / gcores_count );
//int deme_nbr = my_rank - grp_nbr * gcores_count;
//DbgLv(0) << "Dem :" << grp_nbr << deme_nbr << "gene.s" << gene[0].s << "CROSS";
//}
//*DEBUG*
}

int US_MPI_Analysis::migrate_genes( void )
{
   static const int migrate_pcent = parameters[ "migration" ].toInt();
   static const int elitism_count = parameters[ "elitism"   ].toInt();

   QVector< US_Solute > emmigres;
   int migrate_count = ( migrate_pcent * population + 50 ) / 100;

   // Send genes to master
   for ( int i = 0; i < migrate_count; i++ )
   {
      int  gene_index = e_random();
      emmigres += genes[ fitness[ gene_index ].index ];
   }
//*DEBUG*
//if(emmigres[0].s<0.0) {
//int grp_nbr  = ( my_rank / gcores_count );
//int deme_nbr = my_rank - grp_nbr * gcores_count;
//DbgLv(0) << "Dem :" << grp_nbr << deme_nbr << "gene.s" << emmigres[0].s << "EMMIGRES";
//}
//*DEBUG*

   // MPI send msg type
   MPI_GA_MSG msg;
   msg.size = migrate_count;
   int bucket_sols  = buckets.size();
   int migrate_sols = migrate_count * bucket_sols;

   MPI_Send( &msg,               // to MPI #1
             sizeof( msg ),
             MPI_BYTE,
             MPI_Job::MASTER,
             EMMIGRATE,
             my_communicator );

   // MPI send emmigrants
   MPI_Send( emmigres.data(),  // to MPI #4
             migrate_sols * solute_doubles,
             MPI_DOUBLE,
             MPI_Job::MASTER,
             EMMIGRATE,
             my_communicator );

   // Get genes from master as concatenated genes
   QVector< US_Solute > immigres( migrate_sols );
   MPI_Status        status;

   MPI_Recv( immigres.data(),  // from MPI #5
             migrate_sols * solute_doubles,
             MPI_DOUBLE,
             MPI_Job::MASTER,
             IMMIGRATE,
             my_communicator,
             &status );

   int solutes_sent;
   MPI_Get_count( &status, MPI_DOUBLE, &solutes_sent );
   solutes_sent    /= solute_doubles;
   int mgenes_count = solutes_sent / bucket_sols;

   // The number returned equals the number sent or zero.
   if ( mgenes_count > 0 )
   {
      int grp_nbr  = ( my_rank / gcores_count );
      int deme_nbr = my_rank - grp_nbr * gcores_count;
DbgLv(1) << "MG:Deme" << deme_nbr << ": solsent mg_count" << solutes_sent
   << mgenes_count << " elit" << elitism_count << "sol_dbls" << solute_doubles;
      for ( int i = 0; i < mgenes_count; i++ )
      {
         Gene gene = immigres.mid( i * bucket_sols, bucket_sols );
         genes[ elitism_count + i ] = gene;
      }
//*DEBUG*
//if(genes[elitism_count][0].s<0.0) {
//int grp_nbr  = ( my_rank / gcores_count );
//int deme_nbr = my_rank - grp_nbr * gcores_count;
//DbgLv(0) << "Dem :" << grp_nbr << deme_nbr << "gene.s" << genes[elitism_count][0].s << "IMMIGRANT";
//}
//*DEBUG*
   }

   return mgenes_count;
}

US_MPI_Analysis::Gene US_MPI_Analysis::new_gene( void )
{
   Gene gene;

   for ( int b = 0; b < buckets.size(); b++ )
   {
      US_Solute solute;

      double x_min   = buckets[ b ].x_min;
      double y_min   = buckets[ b ].y_min;
      
      solute.s = x_min + u_random( s_grid ) * buckets[ b ].ds;
      solute.k = y_min + u_random( k_grid ) * buckets[ b ].dk;

      gene << solute;
   }
//*DEBUG*
//if(gene[0].s<0.0) {
//int grp_nbr  = ( my_rank / gcores_count );
//int deme_nbr = my_rank - grp_nbr * gcores_count;
//DbgLv(0) << "Dem :" << grp_nbr << deme_nbr << "gene.s" << gene[0].s << "NEW";
//}
//*DEBUG*

   return gene;
}

// Find the minimum fitness value close to a gene using
// inverse hessian minimization 
double  US_MPI_Analysis::minimize( Gene& gene, double fitness )
{
#if 0
#define TIMING_MZ
#endif
#ifdef TIMING_MZ
static int  ncalls=0;
static long totms=0L;
static long totT1=0L;
static long totT2=0L;
static long totT3=0L;
static long totT4=0L;
static long totT5=0L;
static long totT6=0L;
long insms;
QDateTime clcSt0=QDateTime::currentDateTime();
#endif
   int       gsize = gene.size();
   int       vsize = gsize * 2;
   US_Vector vv( vsize );  // Input values
   US_Vector uu( vsize );  // Vector of derivatives

   // Create hessian as identity matrix
   QVector< QVector< double > > hessian( vsize );

   for ( int ii = 0; ii < vsize; ii++ ) 
   {
      hessian[ ii ] = QVector< double >( vsize, 0.0 );
      hessian[ ii ][ ii ] = 1.0;
   }

int grp_nbr  = ( my_rank / gcores_count );
int deme_nbr = my_rank - grp_nbr * gcores_count;
QString Phd = "MIN:" + QString::asprintf( "%d:%d",grp_nbr,deme_nbr ) + ":";
DbgLv(DL) << Phd << "vsize" << vsize << "fitness" << fitness
 << "gene0.s" << gene[0].s;
   int index = 0;

   // Convert gene to array of doubles
   for ( int ii = 0; ii < gsize; ii++ )
   {
      vv.assign( index++, gene[ ii ].s );
      vv.assign( index++, gene[ ii ].k );
   }

DbgLv(DL) << Phd << "   (0)call lamm_gsm_df";
   lamm_gsm_df( vv, uu );   // uu is vector of derivatives
DbgLv(DL) << Phd << "    (0) rtn fr lamm_gsm_df u0" << uu[0];
#ifdef TIMING_MZ
QDateTime clcSt1=QDateTime::currentDateTime();
totT1+=clcSt0.msecsTo(clcSt1);
#endif

   static const double epsilon_f      = 1.0e-7;
   static const int    max_iterations = 20;
   int                 iteration      = 0;
   double              epsilon        = epsilon_f * fitness * 4.0;
DbgLv(DL) << Phd << "epsilon" << epsilon << "uL2norm" << uu.L2norm();
   bool                neg_cnstr      = ( vv[ 0 ] < 0.1 );  // Negative constraint?

   while ( uu.L2norm() >= epsilon_f  && iteration < max_iterations )
   {
#ifdef TIMING_MZ
clcSt1=QDateTime::currentDateTime();
#endif
      iteration++;
DbgLv(DL) << Phd << " iter" << iteration << "fitness" << fitness;
      if ( fitness == 0.0 ) break;

      US_Vector v_s1 = vv;
      double g_s1    = fitness;
      double s1      = 0.0;
      double s2      = 0.5;
      double s3      = 1.0;

      // v_s2 = vv - uu * s2
      US_Vector v_s2( vsize );
      vector_scaled_sum( v_s2, uu, -s2, vv );

      if ( neg_cnstr  &&  v_s2[ 0 ] < 0.1 )
      {
DbgLv(DL) << Phd << " NEG-CNSTR:01: v_s2[0]" << v_s2[0];
         v_s2.assign( 0, 0.1 + u_random( 100 ) * 0.001 );
      }

DbgLv(DL) << Phd << "  iter" << iteration << "v_s2[0]" << v_s2[0];
      double g_s2 = get_fitness_v( v_s2 );
DbgLv(DL) << Phd << "   iter" << iteration << "g_s2" << g_s2;

      // Cut down until we have a decrease
      while ( s2 > epsilon  &&  g_s2 > g_s1 )
      {
         s3  = s2;
         s2 *= 0.5;
         // v_s2 = vv - uu * s2
         vector_scaled_sum( v_s2, uu, -s2, vv );
DbgLv(DL) << Phd << "  s2 g_s2 g_s1" << s2 << g_s2 << g_s1;
         if ( neg_cnstr  &&  v_s2[ 0 ] < 0.1 )
         {
DbgLv(DL) << Phd << " NEG-CNSTR:02: v_s2[0]" << v_s2[0];
            v_s2.assign( 0, 0.1 + u_random( 100 ) * 0.001 );
         }

         g_s2 = get_fitness_v( v_s2 );
      }
#ifdef TIMING_MZ
QDateTime clcSt2=QDateTime::currentDateTime();
totT2+=clcSt1.msecsTo(clcSt2);
#endif
DbgLv(DL) << Phd << "  g_s2" << g_s2 << "s2 s3" << s2 << s3;

      // Test for initial decrease
      if ( s2 <= epsilon  ||  ( s3 - s2 ) < epsilon ) break;

      US_Vector v_s3( vsize );

      // v_s3 = vv - uu * s3
      vector_scaled_sum( v_s3, uu, -s3, vv );

      if ( neg_cnstr  &&  v_s3[ 0 ] < 0.1 )
      {
DbgLv(DL) << Phd << " NEG-CNSTR:03: v_s3[0]" << v_s3[0];
         v_s3.assign( 0, 0.1 + u_random( 100 ) * 0.001 );
      }

      double g_s3 = get_fitness_v( v_s3 );
#ifdef TIMING_MZ
QDateTime clcSt3=QDateTime::currentDateTime();
totT3+=clcSt2.msecsTo(clcSt3);
#endif
DbgLv(DL) << Phd << "  g_s3" << g_s3 << "s1 s2 s3" << s1 << s2 << s3;

      int              reps     = 0;
      static const int max_reps = 100;

      while ( ( ( s2 - s1 ) > epsilon )  &&
              ( ( s3 - s2 ) > epsilon )  &&
              ( reps++ < max_reps ) )
      {
         double s1_s2 = 1.0 / ( s1 - s2 );
         double s1_s3 = 1.0 / ( s1 - s3 );
         double s2_s3 = 1.0 / ( s2 - s3 );

         double s1_2 = sq( s1 );
         double s2_2 = sq( s2 );
         double s3_2 = sq( s3 );

         double aa = ( ( g_s1 - g_s3 ) * s1_s3 -
                       ( g_s2 - g_s3 ) * s2_s3
                     ) * s1_s2;

         double bb = ( g_s3 * ( s2_2 - s1_2 ) +
                       g_s2 * ( s1_2 - s3_2 ) +
                       g_s1 * ( s3_2 - s2_2 )
                     ) *
                     s1_s2 * s1_s3 * s2_s3;

         static const double max_a = 1.0e-25;

DbgLv(DL) << Phd << "   reps" << reps << "aa" << aa;
         if ( qAbs( aa ) < max_a )
         {
            index = 0;

            // Restore gene from array of doubles
            for ( int ii = 0; ii < gsize; ii++ )
            {
               gene[ ii ].s = vv[ index++ ];
               gene[ ii ].k = vv[ index++ ];
            }

#ifdef TIMING_MZ
QDateTime clcSt4=QDateTime::currentDateTime();
totT4+=clcSt3.msecsTo(clcSt4);
insms=(long)clcSt0.msecsTo(clcSt4);
totms+=insms;
ncalls++;
DRTiming << "MINIMIZE: msecs" << insms << "totmsecs calls" << totms << ncalls;
DRTiming << "   MMIZE:  t1 t2 t3 t4 t5 t6"
 << totT1 << totT2 << totT3 << totT4 << totT5 << totT6;
#endif
            return fitness;
         }

         double xx        = -bb / ( 2.0 * aa );
         double prev_g_s2 = g_s2;

         if ( xx < s1 )
         {
            if ( xx < ( s1 + s1 - s2 ) ) // Keep it close
            {
               xx = s1 + s1 - s2;             // xx <- s1 + ds
               if ( xx < 0 ) xx = s1 / 2.0;
            }

            if ( xx < 0 )  //  Wrong direction!
            {
               if ( s1 < 0 ) s1 = 0.0;
               xx = 0;
            }

            // OK, take xx, s1, s2 
            v_s3  = v_s2;
            g_s3  = g_s2;                  // 3 <- 2
            s3    = s2;
            v_s2  = v_s1;
            g_s2  = g_s1;
            s2    = s1;                    // 2 <- 1
            s1    = xx;                    // 1 <- xx
 
            // v_s1 = vv - uu * s1
            vector_scaled_sum( v_s1, uu, -s1, vv );
 
            if ( neg_cnstr  &&  v_s1[ 0 ] < 0.1 )
            {
DbgLv(DL) << Phd << " NEG-CNSTR:04: v_s1[0]" << v_s1[0];
               v_s1.assign( 0, 0.1 + u_random( 100 ) * 0.001 );
            }

DbgLv(DL) << Phd << "  x<s1 get_fitness";
            g_s1 = get_fitness_v( v_s1 ); 
         }
         else if ( xx < s2 ) // Take s1, xx, s2
         {
            v_s3  = v_s2;
            g_s3  = g_s2;             // 3 <- 2
            s3    = s2;
            s2    = xx;               // 2 <- xx

            // v_s2 = vv - uu * s2
            vector_scaled_sum( v_s2, uu, -s2, vv );

            if ( neg_cnstr  &&  v_s2[ 0 ] < 0.1 )
            {
DbgLv(DL) << Phd << " NEG-CNSTR:05: v_s2[0]" << v_s2[0];
               v_s2.assign( 0, 0.1 + u_random( 100 ) * 0.001 );
            }

DbgLv(DL) << Phd << "  xx<s2 get_fitness";
            g_s2 = get_fitness_v( v_s2 );
         }
         else if ( xx < s3 )  // Take s2, xx, s3
         {
            v_s1  = v_s2;
            g_s1  = g_s2;
            s1    = s2;              // 2 <- 1
            s2    = xx;              // 2 <- xx

            // v_s2 = vv - uu * s2
            vector_scaled_sum( v_s2, uu, -s2, vv );

            if ( neg_cnstr  &&  v_s2[ 0 ] < 0.1 )
            {
DbgLv(DL) << Phd << " NEG-CNSTR:06: v_s2[0]" << v_s2[0];
               v_s2.assign( 0, 0.1 + u_random( 100 ) * 0.001 );
            }

DbgLv(DL) << Phd << "  xx<s3 get_fitness";
            g_s2 = get_fitness_v( v_s2 );
         }
         else  // xx >= s3
         {
            if ( xx > ( s3 + s3 - s2 ) ) // if xx > s3 + ds/2
            { 
               // v_s4 = vv - uu * xx
               US_Vector v_s4( vsize );
               vector_scaled_sum( v_s4, uu, -xx, vv );

               if ( neg_cnstr  &&  v_s4[ 0 ] < 0.1 )
               {
DbgLv(DL) << Phd << " NEG-CNSTR:07: v_s4[0]" << v_s4[0];
                  v_s4.assign( 0, 0.1 + u_random( 100 ) * 0.001 );
               }

DbgLv(DL) << Phd << "  xx>=s3 get_fitness (A)";
               double g_s4 = get_fitness_v( v_s4 );

               if ( g_s4 > g_s2  &&  g_s4 > g_s3  &&  g_s4 > g_s1 ) 
               {
                  xx = s3 + s3 - s2;   // xx = s3 + ds/2
               }
            }

            // Take s2, s3, xx 
            v_s1  = v_s2;
            g_s1  = g_s2;            // 1 <- 2
            s1    = s2;
            v_s2  = v_s3;
            g_s2  = g_s3;
            s2    = s3;              // 2 <- 3
            s3    = xx;              // 3 <- xx

            // v_s3 = vv - uu * s3
            vector_scaled_sum( v_s3, uu, -s3, vv );

            if ( neg_cnstr  &&  v_s3[ 0 ] < 0.1 )
            {
DbgLv(DL) << Phd << " NEG-CNSTR:01: v_s2[0]" << v_s2[0];
               v_s3.assign( 0, 0.1 + u_random( 100 ) * 0.001 );
            }

DbgLv(DL) << Phd << "  xx>=s3 get_fitness";
            g_s3 = get_fitness_v( v_s3 );
         }

DbgLv(DL) << Phd << "  p_g_s2 g_s2" << prev_g_s2 << g_s2;
         if ( qAbs( prev_g_s2 - g_s2 ) < epsilon ) break;
      }  // end of inner loop

      US_Vector v_p( vsize );

      if ( g_s2 < g_s3  &&  g_s2 < g_s1 )
      {
         v_p     = v_s2;
         fitness = g_s2;
      }
      else if ( g_s1 < g_s3 )
      {
         v_p     = v_s1;
         fitness = g_s1;
      }
      else
      {
         v_p     = v_s3;
         fitness = g_s3;
      }
DbgLv(DL) << Phd << "   g_s? fitness" << fitness;
      
#ifdef TIMING_MZ
QDateTime clcSt4=QDateTime::currentDateTime();
totT4+=clcSt3.msecsTo(clcSt4);
#endif
DbgLv(DL) << Phd << "   call lamm_gsm_df";
      US_Vector v_g( vsize );        // Vector of derivatives
      lamm_gsm_df( v_p, v_g );       // New gradient in v_g (old in uu) 
DbgLv(DL) << Phd << "    retn fr lamm_gsm_df";
#ifdef TIMING_MZ
QDateTime clcSt5=QDateTime::currentDateTime();
totT5+=clcSt4.msecsTo(clcSt5);
#endif

      US_Vector v_dx( vsize );
      // v_dx = v_p - vv
      vector_scaled_sum( v_dx, vv, -1.0, v_p );


      vv = v_p;                      // vv   = v_p

      // dgradient  v_dg = v_g - uu
      US_Vector v_dg( vsize );
      vector_scaled_sum( v_dg, uu, -1.0, v_g );

      US_Vector v_hdg( vsize );

      // v_hdg = hessian * v_dg ( matrix * vector )
      for ( int ii = 0; ii < vsize; ii++ )
      {
         double dotprod = 0.0;

         for ( int jj = 0; jj < vsize; jj++ )
            dotprod += ( hessian[ ii ][ jj ] * v_dg[ jj ] );

         v_hdg.assign( ii, dotprod );
      }

      double fac   = v_dg.dot( v_dx  );
      double fae   = v_dg.dot( v_hdg );
      double sumdg = v_dg.dot( v_dg  );
      double sumxi = v_dx.dot( v_dx  );
DbgLv(DL) << Phd << "  fac sumdg sumxi" << fac << sumdg << sumxi;

      if ( fac > sqrt( epsilon * sumdg * sumxi ) )
      {
         fac        = 1.0 / fac;
         double fad = 1.0 / fae;

         for ( int ii = 0; ii < vsize; ii++ )
         {
            v_dg.assign( ii, fac * v_dx[ ii ] - fad * v_hdg[ ii ] );
         }

         for ( int ii = 0; ii < vsize; ii++ )
         {
            for ( int jj = ii; jj < vsize; jj++ )
            {
               hessian[ ii ][ jj ] +=
                  fac * v_dx [ ii ] * v_dx [ jj ] -
                  fad * v_hdg[ ii ] * v_hdg[ jj ] +
                  fae * v_dg [ ii ] * v_dg [ jj ];

                 // It's a symmetrical matrix
                 hessian[ jj ][ ii ] = hessian[ ii ][ jj ];
            }
         }
      }

      // uu = hessian * v_g ( matrix * vector )
      for ( int ii = 0; ii < vsize; ii++ )
      {
         double dotprod = 0.0;

         for ( int jj = 0; jj < vsize; jj++ )
            dotprod    += ( hessian[ ii ][ jj ] * v_g[ jj ] );

         uu.assign( ii, dotprod );
      }

#ifdef TIMING_MZ
QDateTime clcSt6=QDateTime::currentDateTime();
totT6+=clcSt5.msecsTo(clcSt6);
#endif
   }  // end while ( uu.L2norm() > epsilon )

   index = 0;

   // Restore gene from array of doubles
   for ( int ii = 0; ii < gsize; ii++ )
   {
      gene[ ii ].s = vv[ index++ ];
      gene[ ii ].k = vv[ index++ ];
   }
DbgLv(DL) << Phd << "OUT:fitness" << fitness << "gene0.s" << gene[0].s;

#ifdef TIMING_MZ
insms=(long)clcSt0.msecsTo(QDateTime::currentDateTime());
totms+=insms;
ncalls++;
DRTiming << "MINIMIZE: msecs" << insms << "totmsecs calls" << totms << ncalls;
DRTiming << "   MMIZE:  t1 t2 t3 t4 t5 t6"
 << totT1 << totT2 << totT3 << totT4 << totT5 << totT6;
#endif
   return fitness;
}

// Set vector values to the scaled sums of two vectors
void US_MPI_Analysis::vector_scaled_sum( US_Vector& cc,
      US_Vector& aa, double sa, US_Vector& bb, double sb )
{
   int sizec = qMin( cc.size(), qMin( aa.size(), bb.size() ) );

   if ( sb == 1.0 )
      for ( int ii = 0; ii < sizec; ii++ )
         cc.assign( ii, aa[ ii ] * sa + bb[ ii ] );
   else
      for ( int ii = 0; ii < sizec; ii++ )
         cc.assign( ii, aa[ ii ] * sa + bb[ ii ] * sb );

   return;
}

// Update the fitness value for a set of solutes.
// Initially (index<0), the A and B matrices are populated.
// Thereafter, each call updates a single column of the A matrix
// and recalculates the x vector, with fitness update.
double US_MPI_Analysis::update_fitness( int index, US_Vector& v )
{
   double fitness = 0.0;
   static QVector< double > nnls_a;
   static QVector< double > nnls_b;
          QVector< double > nnls_x;
          QVector< double > nnls_c;
          QVector< double > w_nnls_a;
          QVector< double > w_nnls_b;
   US_SolveSim::DataSet*   dset  = data_sets[ 0 ];
   US_DataIO::EditedData*  edata = &dset->run_data;
   US_DataIO::RawData      simdat;
   int    nscans  = edata->scanCount();
   int    npoints = edata->pointCount();
   int    ntotal  = nscans * npoints;
   int    vsize   = v.size();
   int    nsols   = vsize / 2;
   int    navals  = nsols * ntotal;
   double fixval  = parameters[ "bucket_fixed" ].toDouble();
   fixval         = ( attr_z == ATTR_V ) ? dset->vbar20 : fixval;
//qDebug() << "UF: vsize nsols ntotal navals" << vsize << nsols << ntotal << navals;

   US_Math2::SolutionData sd;
   sd.density     = dset->density;
   sd.viscosity   = dset->viscosity;
   sd.manual      = dset->manual;
   sd.vbar20      = dset->vbar20;
   sd.vbar        = US_Math2::adjust_vbar20( sd.vbar20, dset->temperature );
   US_Math2::data_correction( dset->temperature, sd );

   US_Model::SimulationComponent zcomponent;
   zcomponent.s      = 0.0;
   zcomponent.f_f0   = 0.0;
   zcomponent.mw     = 0.0;
   zcomponent.vbar20 = dset->vbar20;
   zcomponent.D      = 0.0;
   zcomponent.f      = 0.0;

   set_comp_attrib( zcomponent, fixval, attr_z );
//DbgLv(1) << "UF: xyz" << attr_x << attr_y << attr_z << "fixval" << fixval;

   if ( index < 0 )
   {  // Do the initial population of A and B matrices

      nnls_a.resize( navals );    // Prepare the NNLS A,B matrices
      nnls_b.resize( ntotal );

      int kk = 0;

      for ( int vv = 0; vv < vsize; vv += 2 )
      {  // Fit each solute and populate the A matrix with simulations
         double xval   = v[ vv ];
         double yval   = v[ vv + 1 ];
         US_Model model;
         model.components.resize( 1 );
         model.components[ 0 ]      = zcomponent;

         build_component( model.components[ 0 ], sd, xval, yval );

         US_AstfemMath::initSimData( simdat, *edata, 0.0 );
         US_Astfem_RSA astfem_rsa( model, dset->simparams );
         astfem_rsa.set_debug_flag( dbg_level );
         astfem_rsa.calculate( simdat );

         for ( int ss = 0; ss < nscans; ss++ )
            for ( int rr = 0; rr < npoints; rr++ )
               nnls_a[ kk++ ] = simdat.value( ss, rr );
      }

      kk     = 0;

      // Populate the B matrix with experiment data
      for ( int ss = 0; ss < nscans; ss++ )
         for ( int rr = 0; rr < npoints; rr++ )
            nnls_b[ kk++ ] = edata->value( ss, rr );

   }

   else
   {  // Calculate one column of the A matrix and re-do NNLS
      nnls_x.resize( nsols  );
      nnls_c.resize( ntotal );
      US_Model model;
      model.components.resize( 1 );

      int vv = index * 2;
      model.components[ 0 ]      = zcomponent;

      build_component( model.components[ 0 ], sd, v[ vv ], v[ vv + 1 ] );

//qDebug() << "UF: index vv" << index << vv << "s,D"
// << model.components[0].s << model.components[0].D;

      US_AstfemMath::initSimData( simdat, *edata, 0.0 );
      US_Astfem_RSA astfem_rsa( model, dset->simparams );
      astfem_rsa.set_debug_flag( dbg_level );
      astfem_rsa.calculate( simdat );

      int kk = index * ntotal;
      int jj = 0;

      for ( int ss = 0; ss < nscans; ss++ )
      {
         for ( int rr = 0; rr < npoints; rr++ )
         {  // Save the column, then replace it with simulation values
            nnls_c[ jj++ ] = nnls_a[ kk ];
            nnls_a[ kk++ ] = simdat.value( ss, rr );
         }
      }
//qDebug() << "UF:  kk jj" << kk << jj;

      // Put A and B in work vectors since NNLS corrupts them
      w_nnls_a = nnls_a;
      w_nnls_b = nnls_b;

      // Re-do NNLS to get X concentrations using replaced A
      US_Math2::nnls( w_nnls_a.data(), ntotal, ntotal, nsols,
                      w_nnls_b.data(), nnls_x.data() );

      kk     = index * ntotal;
      jj     = 0;

      // Restore the A column with its previous values
      for ( int ss = 0; ss < nscans; ss++ )
         for ( int rr = 0; rr < npoints; rr++ )
            nnls_a[ kk++ ] = nnls_c[ jj++ ];

      model.components.clear();
      kk       = 0;
      int ksol = 0;

      for ( int cc = 0; cc < nsols; cc++ )
      {  // Build a model using solutes where the concentration is positive
         double soluval = nnls_x[ cc ];

         if ( soluval > 0.0 )
         {
            model.components << zcomponent;
            int vv = cc * 2;

            build_component( model.components[ kk ], sd, v[ vv ], v[ vv + 1 ] );

            model.components[ kk++ ].signal_concentration = soluval;

            if ( soluval >= concentration_threshold )  ksol++;
         }
      }

      // Calculate the simulation using a model of all live solutes
      US_AstfemMath::initSimData( simdat, *edata, 0.0 );
      US_Astfem_RSA astfem_rsa2( model, dset->simparams );
      astfem_rsa2.set_debug_flag( dbg_level );
      astfem_rsa2.calculate( simdat );

      // Calculate the fitness (variance) == average of residuals-squared
      fitness   = 0.0;

      for ( int ss = 0; ss < nscans; ss++ )
      {
         for ( int rr = 0; rr < npoints; rr++ )
         {
            double resid = edata->value( ss, rr ) - simdat.value( ss, rr );
            fitness     += sq( resid );
//int ms=nscans/2;int mr=npoints/2;
//if((ss>ms-3)&&(ss<ms+3)&&(rr>mr-3)&&(rr<mr+3))
// qDebug() << "UF: index" << index << "ss rr edat sdat resd"
//  << ss << rr << edata->value(ss,rr) << simdat.value(ss,rr) << resid;
         }
      }

      fitness  /= (double)( ntotal );
      fitness  *= ( 1.0 + sq( regularization * ksol ) );
   }

   return fitness;
}

void US_MPI_Analysis::lamm_gsm_df( const US_Vector& vv, US_Vector& vd )
{
   static const double hh       = 0.01;
   static const double h2_recip = 0.5 / hh;

   // Work with a temporary vector
   US_Vector tt = vv;

#ifdef UPDATE_FIT
   if ( current_dataset == 0  &&  data_sets.size() == 1 )
   {
      update_fitness( -1, tt );

      for ( int ii = 0; ii < tt.size(); ii++ )
      {
         double save = tt[ ii ];
         int    cc   = ii / 2;

         tt.assign( ii, save - hh );
         double y0   = update_fitness( cc, tt );  // Calc fitness value -h

         tt.assign( ii, save + hh );
         double y2   = update_fitness( cc, tt );  // Calc fitness value +h

         vd.assign( ii, ( y2 - y0 ) * h2_recip ); // The derivative
         tt.assign( ii, save );
      }
   }

   else
#endif
   {
      for ( int ii = 0; ii < tt.size(); ii++ )
      {
         double save = tt[ ii ];

         tt.assign( ii, save - hh );
         double y0   = get_fitness_v( tt );       // Calc fitness value -h

         tt.assign( ii, save + hh );
         double y2   = get_fitness_v( tt );       // Calc fitness value +h

         vd.assign( ii, ( y2 - y0 ) * h2_recip ); // The derivative
         tt.assign( ii, save );
      }
   }
int nn=tt.size()-1;
DbgLv(DL) << "GDF: vd0..." << vd[0] << vd[1] << vd[2] << vd[3];
DbgLv(DL) << "GDF: ...vdn" << vd[nn-3] << vd[nn-2] << vd[nn-1] << vd[nn];

}

/////////////   Debug routines
void US_MPI_Analysis::dump_buckets( void )
{
   if ( my_rank != 1 ) return;
   DbgLv(1) << "Buckets:";
   
   for ( int b = 0; b < buckets.size(); b++ )
   {
      DbgLv(1) << buckets[ b ].x_min
               << buckets[ b ].x_max
               << buckets[ b ].y_min
               << buckets[ b ].y_max
               << buckets[ b ].ds
               << buckets[ b ].dk;
   }
}

void US_MPI_Analysis::dump_genes( int gene )
{
   if ( my_rank != 1 ) return;
   
   if ( gene > -1 )
   {
      QString s = "Gene " + QString::number( gene ) + ": ";

      for ( int b = 0; b < genes[ gene ].size(); b++ )
      {
         s += QString( "(%1, %2); " ) 
                       .arg( genes[ gene ][ b ]. s )  
                       .arg( genes[ gene ][ b ]. k );

      }

      DbgLv(1) << s;
   }
   else
   {
      DbgLv(1) << "Genes:";
      
      for ( int g = 0; g < genes.size(); g++ )
      {
         QString s;

         for ( int b = 0; b < genes[ g ].size(); b++ )
         {
            s += QString( "(%1, %2)\n" ) 
                       .arg( genes[ g ][ b ]. s )  
                       .arg( genes[ g ][ b ]. k );
         }
       
         DbgLv(1) << s;
      }
   }
}

void US_MPI_Analysis::dump_fitness( const QList< Fitness >& fitness )
{
   if ( my_rank != 1 ) return;
   
   QString s = "Fitness:\n";

   for ( int f = 0; f < fitness.size(); f++ )
   {

      s += QString::asprintf( "i, index, fitness: %3i, %3i, %.6e\n",
f, fitness[ f ].index, fitness[ f ].fitness );
   }

   DbgLv(1) << s;
}

// Set a coefficient value for a specified model component attribute
void US_MPI_Analysis::set_comp_attrib( US_Model::SimulationComponent& mcomp,
      double covalue, int attribx )
{
   switch ( attribx )
   {  // Set the coefficient value indicated by the attribute index
      default:
      case ATTR_S:      // Set sedimentation coefficient at scale
         mcomp.s      = covalue * 1.e-13;
         break;
      case ATTR_K:      // Set frictional ratio
         mcomp.f_f0   = covalue;
         break;
      case ATTR_W:      // Set molecular weight
         mcomp.mw     = covalue;
         break;
      case ATTR_V:      // Set partial specific volume (vbar)
         mcomp.vbar20 = covalue;
         break;
      case ATTR_D:      // Set diffusion coefficient
         mcomp.D      = covalue;
         break;
      case ATTR_F:      // Set frictional coefficient
         mcomp.f      = covalue;
         break;
   }
}

// Build a model component from sparse attribute values
void US_MPI_Analysis::build_component( US_Model::SimulationComponent& mcomp,
   US_Math2::SolutionData& sd, double xval, double yval )
{
   US_SolveSim::DataSet* dset  = data_sets[ 0 ];

   // Set the X,Y attributes of the model component
   set_comp_attrib( mcomp, xval, attr_x );
   set_comp_attrib( mcomp, yval, attr_y );

   // Compute all the remaining coefficients
   US_Model::calc_coefficients( mcomp );

   // If vbar is not fixed, recompute solution-based corrections
   if ( attr_z != ATTR_V )
   {
      sd.vbar20     = mcomp.vbar20;
      sd.vbar       = US_Math2::adjust_vbar20( sd.vbar20, dset->temperature );

      US_Math2::data_correction( dset->temperature, sd );
   }

   // Do corrections to sedimentation and diffusion coefficients
   mcomp.s      /= sd.s20w_correction;
   mcomp.D      /= sd.D20w_correction;
}

void US_MPI_Analysis::solutes_from_gene( Gene& solutes, int nsols )
{
   double fixval    = ( attr_z == ATTR_V )
                    ? data_sets[ 0 ]->vbar20
                    : parameters[ "bucket_fixed" ].toDouble();

   for ( int cc = 0; cc < nsols; cc++ )
   {
      double sval      = solutes[ cc ].s;
      double kval      = solutes[ cc ].k;
      solutes[ cc ].s  = ( attr_z == ATTR_S ) ? fixval : 0.0;
      solutes[ cc ].k  = ( attr_z == ATTR_K ) ? fixval : 0.0;
      solutes[ cc ].v  = ( attr_z == ATTR_V ) ? fixval : 0.0;
      solutes[ cc ].d  = ( attr_z == ATTR_W  ||  attr_z == ATTR_D  ||
                           attr_z == ATTR_F ) ? fixval : 0.0;

      switch( attr_x )
      {
         case ATTR_S:
            solutes[ cc ].s  = sval * 1.0e-13;
            break;
         case ATTR_K:
            solutes[ cc ].k  = sval;
            break;
         case ATTR_W:
         case ATTR_D:
         case ATTR_F:
            solutes[ cc ].d  = sval;
            break;
         case ATTR_V:
            solutes[ cc ].v  = sval;
         default:
            break;
      }
      switch( attr_y )
      {
         case ATTR_S:
            solutes[ cc ].s  = kval * 1.0e-13;
            break;
         case ATTR_K:
            solutes[ cc ].k  = kval;
            break;
         case ATTR_W:
         case ATTR_D:
         case ATTR_F:
            solutes[ cc ].d  = kval;
            break;
         case ATTR_V:
            solutes[ cc ].v  = kval;
         default:
            break;
      }
   }
}

