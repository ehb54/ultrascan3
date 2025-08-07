#include "us_lamm_astfvm.h"
#include "us_math2.h"
#include "us_mpi_analysis.h"

QDateTime elapsedd = QDateTime::currentDateTime();
#define ELAPSEDNOW (elapsedd.msecsTo(QDateTime::currentDateTime()))
#define ELAPSEDSEC (elapsedd.secsTo(QDateTime::currentDateTime()))
#define DbTimMsg(a) DbTiming << my_rank << generation << ELAPSEDNOW << a;
#define DRTiming DbTiming << my_rank

#if 1
#define UPDATE_FIT
#endif

// Minimize prints debug level
// #define DL 0
#define DL 1

void US_MPI_Analysis::dmga_worker(void) {
  static const double BASE_SIG = 0.2;
  static const int GR_INCR = 10;
  current_dataset = 0;
  datasets_to_process = data_sets.size();
  QStringList keys = parameters.keys();
  p_grid = (keys.contains("p_grid")) ? parameters["p_grid"].toInt() : 100;
  QString dbgtext = parameters["debug_text"];
  QString s_bsig = par_key_value(dbgtext, "base_sig");
  QString s_grinc = par_key_value(dbgtext, "g_redo_inc");
  base_sig = s_bsig.isEmpty() ? BASE_SIG : s_bsig.toDouble();
  g_redo_inc = s_grinc.isEmpty() ? GR_INCR : s_grinc.toInt();
  if (group_rank == 1)
    DbgLv(0) << my_rank << "dmga_worker: base_sig" << base_sig << "g_redo_inc"
             << g_redo_inc;

  buckets.clear();

  simulation_values.noisflag = 0;
  simulation_values.dbg_level = dbg_level;
  simulation_values.dbg_timing = dbg_timing;

  if (data_sets.size() == 1) {
    US_AstfemMath::initSimData(simulation_values.sim_data,
                               data_sets[0]->run_data, 0.0);
    US_AstfemMath::initSimData(simulation_values.residuals,
                               data_sets[0]->run_data, 0.0);
  } else {
    int ntscan = data_sets[0]->run_data.scanCount();
    for (int ii = 1; ii < data_sets.size(); ii++)
      ntscan += data_sets[ii]->run_data.scanCount();
    simulation_values.sim_data.scanData.resize(ntscan);
    simulation_values.residuals.scanData.resize(ntscan);
  }
  // if(group_rank>=0)
  // abort( "DMGA_WORKER not yet implemented" );

  // Read in the constraints model and build constraints
  QString cmfname = "../" + parameters["DC_model"];
  wmodel.load(cmfname);                   // Load the constraints model
  constraints.load_constraints(&wmodel);  // Build the constraints object
  constraints.get_work_model(&wmodel);    // Get the base work model
  DbgLv(1) << my_rank << "dmga_worker: cmfname" << cmfname;
  DbgLv(1) << my_rank << "dmga_worker: wmodel #comps"
           << wmodel.components.size();

  MPI_GA_MSG msg;
  MPI_Status status;
  MPI_Job job;
  bool finished = false;
  int grp_nbr = (my_rank / gcores_count);
  int deme_nbr = my_rank - grp_nbr * gcores_count;
  // Get the number of floating attributes
  nfloatc = constraints.float_constraints(&cns_flt);
  // Compute the number of possible float combinations
  nfvari = (1 << nfloatc) - 1;
  lfvari.fill(0, nfvari);
  // Set up a working marker vector for a single gene.
  // Note: the "marker" is a simple vector of doubles, consisting of the
  //  attribute values that are unique to each gene. That is, only the
  //  float attributes differ from gene to gene and the specific values
  //  of these floats form a unique signature for each gene. Each gene
  //  itself can be formed by initializing with the base model (wmodel)
  //  and then setting the floating attribute values from the marker vector.
  dgmarker.resize(nfloatc);
  do_astfem =
      (wmodel.components[0].sigma == 0.0 && wmodel.components[0].delta == 0.0 &&
       wmodel.coSedSolute < 0 && data_sets[0]->compress == 0.0);
  DbgLv(1) << my_rank << "dmga_worker: nfloatc nfvari do_astfem" << nfloatc
           << nfvari << do_astfem;
  // DbgLv(1) << my_rank << "dmga_worker: wmodel DUMP";
  // wmodel.debug();

  while (!finished) {
    dmga_worker_loop();

    msg.size = (int)max_rss();
    DbgLv(0) << "Deme" << grp_nbr << deme_nbr
             << ":   Generations finished, second" << ELAPSEDSEC;

    MPI_Send(&msg,         // This iteration is finished
             sizeof(msg),  // to MPI #1
             MPI_BYTE, MPI_Job::MASTER, FINISHED, my_communicator);
    DbgLv(1) << my_rank << "dmgw: FIN sent";
    if (group_rank < 2) {
      DbgLv(1) << my_rank << "lfvari  n" << nfvari << lfvari.size();
      for (int ii = 0; ii < nfvari; ii++)
        DbgLv(1) << my_rank << "  smask" << ii + 1 << "count" << lfvari[ii];
    }

    MPI_Recv(&job,         // Find out what to do next
             sizeof(job),  // from MPI #0, MPI #7, MPI #9
             MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, my_communicator, &status);

    int dataset = job.dataset_offset;
    int count = job.dataset_count;
    int length = job.length;
    DbgLv(1) << my_rank << "dmgw: job offs count len" << dataset << count
             << length << "tag" << status.MPI_TAG;
    int index = 0;

    switch (status.MPI_TAG) {
      case FINISHED:
        finished = true;
        DbgLv(0) << "    Deme" << grp_nbr << deme_nbr << ":" << fitness_hits
                 << "fitness hits of" << fitness_count
                 << "fitness checks   maxrss" << maxrss;
        break;

      case UPDATE:
        // Update data for global fit or Monte Carlo
        // Global fit comes before MC (if necessary), one dataset at a time
        // Monte Carlo always comes as a sequence of all datasets

        mc_data.resize(length);

        MPI_Barrier(my_communicator);

        // This is a receive
        MPI_Bcast(mc_data.data(),  // from MPI #8, #10
                  length, MPI_DOUBLE, MPI_Job::MASTER, my_communicator);

        for (int e = dataset; e < dataset + count; e++) {
          US_DataIO::EditedData* data = &data_sets[e]->run_data;

          int scan_count = data->scanCount();
          int radius_points = data->pointCount();

          for (int s = 0; s < scan_count; s++) {
            US_DataIO::Scan* scan = &data->scanData[s];

            for (int r = 0; r < radius_points; r++) {
              scan->rvalues[r] = mc_data[index++];
            }
          }
        }

        if (count == data_sets.size())  // Next iteration will be global
        {
          current_dataset = 0;
          datasets_to_process = data_sets.size();
        } else  // Next dataset is a part of global fit
        {
          current_dataset = dataset;
          // datasets_to_process will stay at 1
        }

        break;

      case GENERATION:
        break;

      default:
        abort("Unknown message at end of GA worker loop");
        break;

    }  // end switch
  }  // end while
}

void US_MPI_Analysis::dmga_worker_loop(void) {
  // Initialize genes
  dgenes.clear();
  population = parameters["population"].toInt();

  // Build the first generation of genes
  for (int ii = 0; ii < population; ii++) {
    dgenes << new_dmga_gene();
  }

  fitness.reserve(population);

  Fitness empty_fitness;
  empty_fitness.fitness = LARGE;

  for (int ii = 0; ii < population; ii++) {
    empty_fitness.index = ii;
    fitness << empty_fitness;
  }

  int generations = parameters["generations"].toInt();
  int crossover = parameters["crossover"].toInt();
  int mutation = parameters["mutation"].toInt();
  int plague = parameters["plague"].toInt();
  int elitism = parameters["elitism"].toInt();

  int p_mutate = mutation;
  int p_crossover = p_mutate + crossover;
  int p_plague = p_crossover + plague;
  int grp_nbr = my_rank / gcores_count;
  int deme_nbr = my_rank - grp_nbr * gcores_count;

  fitness_map.clear();
  fitness_count = 0;
  fitness_hits = 0;

  max_rss();

  QDateTime start = QDateTime::currentDateTime();
  MPI_GA_MSG msg;

  for (generation = 0; generation < generations; generation++) {
    max_rss();

    DbTimMsg("Worker start rank/generation/elapsed-secs");
    // Calculate fitness
    for (int ii = 0; ii < population; ii++) {
      fitness[ii].index = ii;
      fitness[ii].fitness = get_fitness_dmga(dgenes[ii]);
      DbgLv(1) << my_rank << "dg:getf:  ii" << ii << "  fitness"
               << fitness[ii].fitness;
    }

    // Sort fitness
    qSort(fitness);
    DbTimMsg("Worker after get_fitness loop + sort");

    // Refine with gradient search method (gsm) on last generation
    if (generation == generations - 1) {
      DbgLv(DL) << "Deme" << grp_nbr << deme_nbr
                << ": At last generation minimize.";
      DbTimMsg("Worker before gsm rank/generation/elapsed");
      in_gsm = true;

      fitness[0].fitness =
          minimize_dmga(dgenes[fitness[0].index], fitness[0].fitness);
      DbgLv(DL) << "Deme" << grp_nbr << deme_nbr
                << ":   last generation minimize fitness="
                << fitness[0].fitness;
      DbTimMsg("Worker after gsm rank/generation/elapsed");
      in_gsm = false;
    }

    max_rss();

    // Send best gene to master
    if ((deme_nbr % 10) == 1) {
      DbgLv(1) << "Best gene to master: gen" << generation << "worker"
               << deme_nbr << "fitness" << fitness[0].fitness;
      dump_fitness(fitness);
    }
    msg.generation = generation;
    dgene = dgenes[fitness[0].index];
    marker_from_dgene(dgmarker, dgene);
    msg.size = nfloatc;
    msg.fitness = fitness[0].fitness;

    MPI_Send(&msg,  // to MPI #1
             sizeof(msg), MPI_BYTE, MPI_Job::MASTER, GENERATION,
             my_communicator);

    MPI_Send(dgmarker.data(),  // to MPI #2
             nfloatc, MPI_DOUBLE, MPI_Job::MASTER, GENE, my_communicator);

    DbTimMsg("Worker after send fitness,genes");
    // Receive instructions from master (continue or finish)
    MPI_Status status;

    MPI_Recv(&msg,  // from MPI #3
             sizeof(msg), MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG,
             my_communicator, &status);
    DbTimMsg("Worker after receive instructions");

    max_rss();

    if (status.MPI_TAG == FINISHED) {
      DbgLv(0) << "Deme" << grp_nbr << deme_nbr
               << ": Finish signalled at deme generation" << generation + 1;
      break;
    }

    // See if we are really done
    if (generation == generations - 1) {
      DbgLv(0) << "Deme" << grp_nbr << deme_nbr << ": At last generation";
      continue;
    }

    // Mark duplicate genes
    int f0 = 0;  // An index into the fitness array
    int f1 = 1;  // A second index
    // The value of 1.0e-8 for close fitness is arbitrary. Parameterize?
    const double NEAR_MATCH = 1.0e-8;
    const double EPSF_SCALE = 1.0e-3;
    double fitpwr = (double)qRound(log10(fitness[0].fitness));
    double epsilon_f = pow(10.0, fitpwr) * EPSF_SCALE;
    DbgLv(1) << "gw:" << my_rank
             << ": Dup best-gene clean: fitness0 fitpwr epsilon_f"
             << fitness[0].fitness << fitpwr << epsilon_f;

    while (f1 < population) {
      double fitdiff = qAbs(fitness[f0].fitness - fitness[f1].fitness);

      if (fitdiff < epsilon_f) {
        bool match = true;
        int g0 = fitness[f0].index;
        int g1 = fitness[f1].index;
        DGene gene0 = dgenes[g0];
        DGene gene1 = dgenes[g1];

        for (int ii = 0; ii < nfloatc; ii++) {
          double val0;
          double val1;
          US_dmGA_Constraints::AttribType atype = cns_flt[ii].atype;
          int mcompx = cns_flt[ii].mcompx;

          fetch_attr_value(val0, gene0, atype, mcompx);
          fetch_attr_value(val1, gene1, atype, mcompx);

          double difv = qAbs((val0 - val1) / val0);

          if (difv > NEAR_MATCH) {
            DbgLv(1) << "gw:" << my_rank
                     << ":  Dup NOT cleaned: f0 f1 fit0 fit1" << f0 << f1
                     << fitness[f0].fitness << fitness[f1].fitness
                     << "ii g0 g1 g0v g1v" << ii << g0 << g1 << val0 << val1;
            match = false;
            f0 = f1;
            break;
          }
        }

        if (match) {
          DbgLv(1) << "gw:" << my_rank << ":  Dup cleaned: f0 f1 fit0 fit1"
                   << f0 << f1 << fitness[f0].fitness << fitness[f1].fitness;
          fitness[f1].fitness = LARGE;  // Invalidate gene/sim_values
        }
      } else
        f0 = f1;

      f1++;
    }

    // Re-sort
    qSort(fitness);
    DbgLv(1) << "gw:" << my_rank << ": fitness sorted";

    QList<DGene> old_genes = dgenes;
    int gener_save = generation;

    if (generation > 0 &&
        (generation % g_redo_inc) ==
            0) {  // Set up to re-do initial genes after minimization
      double fit0 = fitness[0].fitness;
      fitness[0].fitness =
          minimize_dmga(dgenes[fitness[0].index], fitness[0].fitness);
      double fit1 = fitness[0].fitness;
      dgene = dgenes[fitness[0].index];
      old_genes[0] = dgene;
      double aval;
      int nlim = 0;

      for (int ii = 0; ii < nfloatc;
           ii++) {  // Insure all the new gene attribute values are inside range
        US_dmGA_Constraints::AttribType atype = cns_flt[ii].atype;
        int mcompx = cns_flt[ii].mcompx;
        double vmin = cns_flt[ii].low;
        double vmax = cns_flt[ii].high;
        fetch_attr_value(aval, dgene, atype, mcompx);

        if (aval < vmin || aval > vmax) {
          if ((group_rank % 10) == 1)
            DbgLv(0) << "gw:" << my_rank << ": re-do : ii" << ii
                     << "aval vmin vmax" << aval << vmin << vmax;
          aval = qMax(vmin, qMin(vmax, aval));
          store_attr_value(aval, dgene, atype, mcompx);
          nlim++;
        }
      }

      if (nlim > 0) {  // If adjustments due to limits, recompute fitness
        fit1 = get_fitness_dmga(dgene);
        if ((group_rank % 10) == 1)
          DbgLv(0) << "gw:" << my_rank << ": re-do :     fit1a fit1b"
                   << fitness[0].fitness << fit1;
        fitness[0].fitness = fit1;
      }

      p_mutate = p_plague;

      for (int gg = 0; gg < population; gg++) old_genes[gg] = dgene;
      if ((group_rank % 10) == 1)
        DbgLv(0) << "gw:" << my_rank << ": re-do genes at gen" << generation
                 << "gen fitness" << fit0 << "post-min fitness" << fit1;

      generation = 0;
    }

    // Create new generation from old
    // First copy elite genes
    for (int gg = 0; gg < elitism; gg++)
      dgenes[gg] = old_genes[fitness[gg].index];
    DbgLv(1) << "gw:" << my_rank << ": elites copied";

    int immigr_count = migrate_dgenes();
    DbgLv(1) << "gw:" << my_rank << ": immigr_count" << immigr_count
             << "dgenes,old sizes" << dgenes.size() << old_genes.size()
             << population;

    DbTimMsg("Worker before elitism loop");

    for (int gg = elitism + immigr_count; gg < population; gg++) {
      // Select a random gene from old population using exponential
      //  distribution
      int gene_index = e_random();
      int probability = u_random(p_plague);
      DbgLv(1) << "gw:" << my_rank << ":  gg" << gg << "gene_index"
               << gene_index << "probability" << probability;
      dgene = old_genes[gene_index];

      // Do a self-mutate on some genes
      if (probability < p_mutate) mutate_dgene(dgene);

      // Do cross-mutation on some other genes
      else if (probability < p_crossover)
        cross_dgene(dgene, old_genes);

      // Purge the remainder (replace with new gene)
      else
        dgene = new_dmga_gene();

      dgenes[gg] = dgene;
    }
    DbTimMsg("Worker after elitism loop");

    generation = gener_save;
    p_mutate = mutation;
    max_rss();

  }  // End of generation loop
  DbTimMsg("  +++Worker after generation loop");
}

// Create a new discrete GA Gene
DGene US_MPI_Analysis::new_dmga_gene(void) {
  DGene gene_new = wmodel;               // Initialize gene to base model
  double extn_p = (double)(p_grid - 1);  // Par grid extent

  // Loop thru float attributes, getting random values for each
  for (int ii = 0; ii < nfloatc; ii++) {
    // Get the current attribute characteristics
    US_dmGA_Constraints::AttribType atype = cns_flt[ii].atype;
    int mcompx = cns_flt[ii].mcompx;
    bool logscl = cns_flt[ii].logscl;
    double vmin = cns_flt[ii].low;
    double vmax = cns_flt[ii].high;
    DbgLv(1) << my_rank << "dg:newg:  ii" << ii << "vmin vmax" << vmin << vmax;

    if (logscl) {  // Convert low,high to log scale
      vmin = log(vmin);
      vmax = log(vmax);
    }

    // Get a value somewhere in the range
    double delta = (double)u_random(p_grid);
    double aval = vmin + ((vmax - vmin) * delta / extn_p);
    aval = qMax(vmin, qMin(vmax, aval));
    // If need be, convert value back from log
    aval = logscl ? exp(aval) : aval;
    DbgLv(1) << my_rank << "dg:newg:   ii" << ii << "p_grid" << p_grid
             << "delta" << delta << "aval" << aval;

    // Update the new attribute value
    store_attr_value(aval, gene_new, atype, mcompx);
  }

  return gene_new;  // Then return the new gene
}

// Mutate a discrete GA Gene
void US_MPI_Analysis::mutate_dgene(DGene& dgene) {
  const double extn_p = (double)(p_grid - 1);
  const double sigfact = pow(10.0, (mutate_sigma * 0.1));
  // Get a random mask that selects which float attributes to vary
  int smask = u_random(nfvari) + 1;
  // Compute the sigma based on parameter grid and generation
  double genterm = log2((double)generation * sigfact + 2.0);
  double sigma = base_sig * extn_p / genterm;
  // Bump the count of hits for each unique smask (floats combination)
  lfvari[smask - 1]++;
  DbgLv(1) << my_rank << "dg:mutg: smask p_grid sigma" << smask << p_grid
           << sigma;
  // if(my_rank==11) DbgLv(0) << my_rank << "dg:mutg: smask p_grid sigma" <<
  // smask << p_grid << sigma;

  // Loop thru float attributes, varying the selected ones
  for (int ii = 0; ii < nfloatc; ii++) {
    // Skip if this float attribute is not selected for this mutation
    if (((1 << ii) & smask) == 0) continue;

    // Get the current value of the attribute
    double vv;
    US_dmGA_Constraints::AttribType atype = cns_flt[ii].atype;
    int mcompx = cns_flt[ii].mcompx;
    bool logscl = cns_flt[ii].logscl;
    fetch_attr_value(vv, dgene, atype, mcompx);

    // Get float range and, if need be, convert to log scale
    double vl = cns_flt[ii].low;
    double vh = cns_flt[ii].high;
    DbgLv(1) << my_rank << "dg:mutg:  ii" << ii << "vl vh" << vl << vh << "vv"
             << vv;

    if (logscl) {
      vv = log(vv);
      vl = log(vl);
      vh = log(vh);
    }

    // Mutate the value
    double xx = US_Math2::box_muller(0.0, sigma);
    double delta = qRound(xx);
    vv += (delta * (vh - vl) / extn_p);
    vv = qMax(vv, vl);
    vv = qMin(vv, vh);
    vv = logscl ? exp(vv) : vv;
    DbgLv(1) << my_rank << "dg:mutg:  ii" << ii << "  xx" << xx << "delta"
             << delta << "vv" << vv;

    // Update the mutated value
    store_attr_value(vv, dgene, atype, mcompx);
  }
  // DbgLv(1) << "dg:mutgene dgene comps" << dgene.components.size();
}

// Cross-mutate a pair of discrete GA Genes
void US_MPI_Analysis::cross_dgene(DGene& dgene, QList<DGene> dgenes) {
  // Get the crossing gene according to an exponential distribution
  int gndx = e_random();
  DGene dgene2 = dgenes[fitness[gndx].index];

  // Get a random mask that selects float attributes from the second gene
  int smask = u_random(nfvari) + 1;
  DbgLv(1) << "dg:crsg: gndx" << gndx << "smask" << smask;

  for (int ii = 0; ii < nfloatc; ii++) {
    // Skip if float attribute is not selected for cross mutation
    if (((1 << ii) & smask) == 0) continue;

    // Get the current value of the attribute from the second gene
    double vv;
    US_dmGA_Constraints::AttribType atype = cns_flt[ii].atype;
    int mcompx = cns_flt[ii].mcompx;
    fetch_attr_value(vv, dgene2, atype, mcompx);

    // Store this value in the first gene
    store_attr_value(vv, dgene, atype, mcompx);
  }
}

// Migrate a list of discrete GA Genes
int US_MPI_Analysis::migrate_dgenes(void) {
  static const int migrate_pcent = parameters["migration"].toInt();
  static const int elitism_count = parameters["elitism"].toInt();

  QList<DGene> emmigres;
  int migrate_count = (migrate_pcent * population + 50) / 100;
  int doubles_count = migrate_count * nfloatc;
  QVector<double> emmigrants(doubles_count);
  DbgLv(1) << my_rank << "dg:migrdg: migrate_count doubles_count"
           << migrate_count << doubles_count;

  // Send genes to master

  for (int ii = 0; ii < migrate_count;
       ii++) {  // Build the current list of emmigrating genes
    int gene_index = e_random();
    emmigres << dgenes[fitness[gene_index].index];
  }

  // Represent migrating genes as marker vector
  dgenes_to_marker(emmigrants, emmigres, 0, migrate_count);
  DbgLv(1) << my_rank << "dg:migrdg: emmigres size" << emmigres.size()
           << "emmigrants size" << emmigrants.size();

  // MPI send msg type
  MPI_GA_MSG msg;
  msg.size = migrate_count;

  MPI_Send(&msg,  // to MPI #1
           sizeof(msg), MPI_BYTE, MPI_Job::MASTER, EMMIGRATE, my_communicator);

  // MPI send emmigrants
  MPI_Send(emmigrants.data(),  // to MPI #1
           doubles_count, MPI_DOUBLE, MPI_Job::MASTER, EMMIGRATE,
           my_communicator);

  // Get genes from master as marker vector
  QVector<double> immigres(doubles_count);
  MPI_Status status;

  MPI_Recv(immigres.data(),  // to MPI #1
           doubles_count, MPI_DOUBLE, MPI_Job::MASTER, IMMIGRATE,
           my_communicator, &status);

  int doubles_sent;
  MPI_Get_count(&status, MPI_DOUBLE, &doubles_sent);
  int mgenes_count = doubles_sent / nfloatc;
  DbgLv(1) << my_rank << "dg:migrdg: immigres size" << immigres.size()
           << "doubles_sent" << doubles_sent;

  // The number returned equals the number sent or zero
  if (mgenes_count > 0) {
    marker_to_dgenes(immigres, dgenes, elitism_count, mgenes_count);
  }

  return mgenes_count;
}

// Get fitness from a vector  (used in inverse hessian)
double US_MPI_Analysis::get_fitness_v_dmga(US_Vector& vv, US_Vector& zz) {
  int vsize = vv.size();
  dgmarker.resize(vsize);

  // Convert from US_Vector to Gene

  for (int ii = 0; ii < vsize;
       ii++) {  // De-normalize values and put into marker QVector<double>
    dgmarker[ii] = vv[ii] / zz[ii];
  }

  dgene_from_marker(dgmarker, dgene);

  // Compute and return the fitness

  return get_fitness_dmga(dgene);
}

// Compute a derivatives vector  (an inverse hessian minimization step)
void US_MPI_Analysis::lamm_gsm_df_dmga(US_Vector& vv, US_Vector& vd,
                                       US_Vector& zz) {
  static const double hh = 0.01;
  static const double h2_recip = 0.5 / hh;

  // Work with a temporary vector
  US_Vector tt = vv;

  for (int ii = 0; ii < tt.size(); ii++) {
    double save = tt[ii];

    tt.assign(ii, save - hh);
    double y0 = get_fitness_v_dmga(tt, zz);  // Calc fitness value -h

    tt.assign(ii, save + hh);
    double y2 = get_fitness_v_dmga(tt, zz);  // Calc fitness value +h

    vd.assign(ii, (y2 - y0) * h2_recip);  // The derivative
    tt.assign(ii, save);
  }
}

// Find the minimum fitness value close to a discrete GA gene using
//  inverse hessian minimization
double US_MPI_Analysis::minimize_dmga(DGene& dgene, double fitness) {
  DbgLv(1) << my_rank << "dg:IHM:minimize dgene comps"
           << dgene.components.size() << fitness;
  int vsize = nfloatc;
  US_Vector vv(vsize);  // Input values
  US_Vector uu(vsize);  // Vector of derivatives
  US_Vector zz(vsize);  // Vector of normalizing factors

  // Create hessian as identity matrix
  QVector<QVector<double> > hessian(vsize);

  for (int ii = 0; ii < vsize; ii++) {
    hessian[ii] = QVector<double>(vsize, 0.0);
    hessian[ii][ii] = 1.0;
  }

  dgmarker.resize(vsize);
  marker_from_dgene(dgmarker, dgene);

  // Convert gene to array of normalized doubles and save normalizing factors
  for (int ii = 0; ii < vsize; ii++) {
    double vval = dgmarker[ii];
    double vpwr = (double)qFloor(log10(vval));
    double vnorm = pow(10.0, -vpwr);
    vv.assign(ii, vval * vnorm);
    zz.assign(ii, vnorm);
    DbgLv(1) << my_rank << "dg:IHM:  ii" << ii << "vval vnorm" << vval << vnorm
             << "vpwr" << vpwr << "vvi" << vv[ii];
  }

  lamm_gsm_df_dmga(vv, uu, zz);  // uu is vector of derivatives

  static const double epsilon_f = 1.0e-7;
  static const int max_iterations = 20;
  int iteration = 0;
  double epsilon = epsilon_f * fitness * 4.0;
  bool neg_cnstr = (vv[0] < 0.1);  // Negative constraint?

  while (uu.L2norm() >= epsilon_f && iteration < max_iterations) {
    iteration++;
    if (fitness == 0.0) break;

    US_Vector v_s1 = vv;
    double g_s1 = fitness;
    double s1 = 0.0;
    double s2 = 0.5;
    double s3 = 1.0;
    DbgLv(1) << my_rank << "dg:IHM:   iteration" << iteration << "fitness"
             << fitness;

    // v_s2 = vv - uu * s2
    US_Vector v_s2(vsize);
    vector_scaled_sum(v_s2, uu, -s2, vv);

    if (neg_cnstr && v_s2[0] < 0.1) {
      v_s2.assign(0, 0.1 + u_random(100) * 0.001);
    }

    double g_s2 = get_fitness_v_dmga(v_s2, zz);
    DbgLv(1) << my_rank << "dg:IHM: g_s2" << g_s2 << "s2" << s2 << "epsilon"
             << epsilon;

    // Cut down until we have a decrease
    while (s2 > epsilon && g_s2 > g_s1) {
      s3 = s2;
      s2 *= 0.5;
      // v_s2 = vv - uu * s2
      vector_scaled_sum(v_s2, uu, -s2, vv);

      if (neg_cnstr && v_s2[0] < 0.1) {
        v_s2.assign(0, 0.1 + u_random(100) * 0.001);
      }

      g_s2 = get_fitness_v_dmga(v_s2, zz);
    }
    DbgLv(1) << my_rank << "dg:IHM:  g_s2" << g_s2;

    // Test for initial decrease
    if (s2 <= epsilon || (s3 - s2) < epsilon) break;

    US_Vector v_s3(vsize);

    // v_s3 = vv - uu * s3
    vector_scaled_sum(v_s3, uu, -s3, vv);

    if (neg_cnstr && v_s3[0] < 0.1) {
      v_s3.assign(0, 0.1 + u_random(100) * 0.001);
    }

    double g_s3 = get_fitness_v_dmga(v_s3, zz);

    int reps = 0;
    static const int max_reps = 100;

    while (((s2 - s1) > epsilon) && ((s3 - s2) > epsilon) &&
           (reps++ < max_reps)) {
      double s1_s2 = 1.0 / (s1 - s2);
      double s1_s3 = 1.0 / (s1 - s3);
      double s2_s3 = 1.0 / (s2 - s3);

      double s1_2 = sq(s1);
      double s2_2 = sq(s2);
      double s3_2 = sq(s3);

      double aa = ((g_s1 - g_s3) * s1_s3 - (g_s2 - g_s3) * s2_s3) * s1_s2;

      double bb =
          (g_s3 * (s2_2 - s1_2) + g_s2 * (s1_2 - s3_2) + g_s1 * (s3_2 - s2_2)) *
          s1_s2 * s1_s3 * s2_s3;

      static const double max_a = 1.0e-25;

      if (qAbs(aa) < max_a) {
        // Restore gene from array of normalized doubles
        for (int ii = 0; ii < vsize; ii++) {
          dgmarker[ii] = vv[ii] / zz[ii];
        }

        dgene_from_marker(dgmarker, dgene);

        return fitness;
      }

      double xx = -bb / (2.0 * aa);
      double prev_g_s2 = g_s2;

      if (xx < s1) {
        if (xx < (s1 + s1 - s2))  // Keep it close
        {
          xx = s1 + s1 - s2;  // xx <- s1 + ds
          if (xx < 0) xx = s1 / 2.0;
        }

        if (xx < 0)  //  Wrong direction!
        {
          if (s1 < 0) s1 = 0.0;
          xx = 0;
        }

        // OK, take xx, s1, s2
        v_s3 = v_s2;
        g_s3 = g_s2;  // 3 <- 2
        s3 = s2;
        v_s2 = v_s1;
        g_s2 = g_s1;
        s2 = s1;  // 2 <- 1
        s1 = xx;  // 1 <- xx

        // v_s1 = vv - uu * s1
        vector_scaled_sum(v_s1, uu, -s1, vv);

        if (neg_cnstr && v_s1[0] < 0.1) {
          v_s1.assign(0, 0.1 + u_random(100) * 0.001);
        }

        g_s1 = get_fitness_v_dmga(v_s1, zz);
      } else if (xx < s2)  // Take s1, xx, s2
      {
        v_s3 = v_s2;
        g_s3 = g_s2;  // 3 <- 2
        s3 = s2;
        s2 = xx;  // 2 <- xx

        // v_s2 = vv - uu * s2
        vector_scaled_sum(v_s2, uu, -s2, vv);

        if (neg_cnstr && v_s2[0] < 0.1) {
          v_s2.assign(0, 0.1 + u_random(100) * 0.001);
        }

        g_s2 = get_fitness_v_dmga(v_s2, zz);
      } else if (xx < s3)  // Take s2, xx, s3
      {
        v_s1 = v_s2;
        g_s1 = g_s2;
        s1 = s2;  // 2 <- 1
        s2 = xx;  // 2 <- xx

        // v_s2 = vv - uu * s2
        vector_scaled_sum(v_s2, uu, -s2, vv);

        if (neg_cnstr && v_s2[0] < 0.1) {
          v_s2.assign(0, 0.1 + u_random(100) * 0.001);
        }

        g_s2 = get_fitness_v_dmga(v_s2, zz);
      } else  // xx >= s3
      {
        if (xx > (s3 + s3 - s2))  // if xx > s3 + ds/2
        {
          // v_s4 = vv - uu * xx
          US_Vector v_s4(vsize);
          vector_scaled_sum(v_s4, uu, -xx, vv);

          if (neg_cnstr && v_s4[0] < 0.1) {
            v_s4.assign(0, 0.1 + u_random(100) * 0.001);
          }

          double g_s4 = get_fitness_v_dmga(v_s4, zz);

          if (g_s4 > g_s2 && g_s4 > g_s3 && g_s4 > g_s1) {
            xx = s3 + s3 - s2;  // xx = s3 + ds/2
          }
        }

        // Take s2, s3, xx
        v_s1 = v_s2;
        g_s1 = g_s2;  // 1 <- 2
        s1 = s2;
        v_s2 = v_s3;
        g_s2 = g_s3;
        s2 = s3;  // 2 <- 3
        s3 = xx;  // 3 <- xx

        // v_s3 = vv - uu * s3
        vector_scaled_sum(v_s3, uu, -s3, vv);

        if (neg_cnstr && v_s3[0] < 0.1) {
          v_s3.assign(0, 0.1 + u_random(100) * 0.001);
        }

        g_s3 = get_fitness_v_dmga(v_s3, zz);
      }

      if (qAbs(prev_g_s2 - g_s2) < epsilon) break;
    }  // end of inner loop

    US_Vector v_p(vsize);

    if (g_s2 < g_s3 && g_s2 < g_s1) {
      v_p = v_s2;
      fitness = g_s2;
    } else if (g_s1 < g_s3) {
      v_p = v_s1;
      fitness = g_s1;
    } else {
      v_p = v_s3;
      fitness = g_s3;
    }

    US_Vector v_g(vsize);            // Vector of derivatives
    lamm_gsm_df_dmga(v_p, v_g, zz);  // New gradient in v_g (old in uu)

    US_Vector v_dx(vsize);
    // v_dx = v_p - vv
    vector_scaled_sum(v_dx, vv, -1.0, v_p);

    vv = v_p;  // vv   = v_p

    // dgradient  v_dg = v_g - uu
    US_Vector v_dg(vsize);
    vector_scaled_sum(v_dg, uu, -1.0, v_g);

    US_Vector v_hdg(vsize);

    // v_hdg = hessian * v_dg ( matrix * vector )
    for (int ii = 0; ii < vsize; ii++) {
      double dotprod = 0.0;

      for (int jj = 0; jj < vsize; jj++)
        dotprod += (hessian[ii][jj] * v_dg[jj]);

      v_hdg.assign(ii, dotprod);
    }

    double fac = v_dg.dot(v_dx);
    double fae = v_dg.dot(v_hdg);
    double sumdg = v_dg.dot(v_dg);
    double sumxi = v_dx.dot(v_dx);

    if (fac > sqrt(epsilon * sumdg * sumxi)) {
      fac = 1.0 / fac;
      double fad = 1.0 / fae;

      for (int ii = 0; ii < vsize; ii++) {
        v_dg.assign(ii, fac * v_dx[ii] - fad * v_hdg[ii]);
      }

      for (int ii = 0; ii < vsize; ii++) {
        for (int jj = ii; jj < vsize; jj++) {
          hessian[ii][jj] += fac * v_dx[ii] * v_dx[jj] -
                             fad * v_hdg[ii] * v_hdg[jj] +
                             fae * v_dg[ii] * v_dg[jj];

          // It's a symmetrical matrix
          hessian[jj][ii] = hessian[ii][jj];
        }
      }
    }

    // uu = hessian * v_g ( matrix * vector )
    for (int ii = 0; ii < vsize; ii++) {
      double dotprod = 0.0;

      for (int jj = 0; jj < vsize; jj++) dotprod += (hessian[ii][jj] * v_g[jj]);

      uu.assign(ii, dotprod);
    }

  }  // end while ( uu.L2norm() > epsilon )

  // Restore gene from array of normalized doubles
  for (int ii = 0; ii < vsize; ii++) {
    dgmarker[ii] = vv[ii] / zz[ii];
    DbgLv(1) << my_rank << "dg:IHM: ii" << ii << "vvi zzi dgmi" << vv[ii]
             << zz[ii] << dgmarker[ii];
  }

  dgene_from_marker(dgmarker, dgene);
  DbgLv(1) << my_rank << "dg:IHM: FITNESS" << fitness;

  return fitness;
}

// Get the fitness value for a discrete GA Gene
double US_MPI_Analysis::get_fitness_dmga(DGene& dgene) {
  QString fkey = dgene_key(dgene);  // Get an identifying key string
  fitness_count++;

  if (fitness_map.contains(fkey)) {  // We already have a match to this key, so
                                     // use its fitness value
    fitness_hits++;
    return fitness_map.value(fkey);
  }

  US_SolveSim::Simulation sim = simulation_values;

  // Compute the simulation and residuals for this model
  calc_residuals_dmga(current_dataset, datasets_to_process, sim, dgene);

  double fitness = sim.variance;      // Get the computed fitness
  fitness_map.insert(fkey, fitness);  // Add it the fitness map

  // DbgLv(1) << "dg:get_fit fitness" << fitness << "count hits"
  //  << fitness_count << fitness_hits;
  return fitness;
}

// Compose an identifying key string for a discrete GA Gene
QString US_MPI_Analysis::dgene_key(DGene& dgene) {
  // Get the marker for this gene
  marker_from_dgene(dgmarker, dgene);

  // Compose a key string that is a concatenation of marker value strings
  QString str;
  QString fkey = str.sprintf("%.5e", dgmarker[0]);

  for (int ii = 1; ii < nfloatc; ii++)
    fkey += str.sprintf(" %.5e", dgmarker[ii]);

  DbgLv(1) << my_rank << "dg:dgkey" << fkey << "mv0 mvn" << dgmarker[0]
           << dgmarker[nfloatc - 1];
  return fkey;
}

// Calculate residuals for a given discrete GA gene
void US_MPI_Analysis::calc_residuals_dmga(int offset, int dset_count,
                                          SIMULATION& sim_vals, DGene& dgene) {
  int lim_offs = offset + dset_count;

  US_DataIO::RawData* sdata = &sim_vals.sim_data;
  US_DataIO::RawData* resid = &sim_vals.residuals;
  int jscan = 0;

  // Compute simulations for all data sets with the given model
  for (int ee = offset; ee < lim_offs; ee++) {
    US_Model model;
    US_Math2::SolutionData sd;
    US_SolveSim::DataSet* dset = data_sets[ee];
    US_DataIO::EditedData* edata = &dset->run_data;
    US_DataIO::RawData simdat;
    int nscans = edata->scanCount();

    // Initialize simulation data with experiment's grid
    US_AstfemMath::initSimData(simdat, *edata, 0.0);

    // Create experimental-space model from DGene for the data set

    sd.viscosity = dset->viscosity;  // Set up data corrections
    sd.density = dset->density;
    sd.manual = dset->manual;

    model_from_dgene(model, dgene);  // Compute apply model from base

    if (ee == offset) data_sets[0]->model = model;  // Save the s20w model

    for (int cc = 0; cc < model.components.size();
         cc++) {  // Loop to perform data corrections to s and D (experimental
                  // space)
      sd.vbar20 = model.components[cc].vbar20;
      sd.vbar = US_Math2::adjust_vbar20(sd.vbar20, dset->temperature);
      US_Math2::data_correction(dset->temperature, sd);

      model.components[cc].s /= sd.s20w_correction;
      model.components[cc].D /= sd.D20w_correction;
    }
    if (group_rank < 2 && dbg_level > 0) {
      DbgLv(1) << my_rank << "CR: SIMPARAMS DUMP";
      dset->simparams.debug();
      DbgLv(1) << my_rank << "CR: dens visc scorr dcorr" << sd.density
               << sd.viscosity << sd.s20w_correction << sd.D20w_correction;
      DbgLv(1) << my_rank << "CR: MODEL DUMP";
      model.debug();
    }

    // Compute the simulation

    if (do_astfem) {  // Calculate simulation for FEM (ideal) case
      US_Astfem_RSA astfem_rsa(model, dset->simparams);
      astfem_rsa.set_debug_flag(my_rank < 2 ? dbg_level : (-1));
      //*DEBUG
      if (group_rank < 1) {
        int dbg_flg = in_gsm ? (-1) : dbg_level;
        // DbgLv(0) << my_rank << "CR: call ASTFEM CALC";
        astfem_rsa.set_debug_flag(dbg_flg);
      }
      //*DEBUG

      astfem_rsa.calculate(simdat);
    }

    else {  // Calculate simulation for FVM (non-ideal) case
      US_Buffer buffer;
      buffer.viscosity = dset->viscosity;
      buffer.density = dset->density;
      buffer.compressibility = dset->compress;

      US_LammAstfvm astfvm(model, dset->simparams);
      astfvm.set_buffer(buffer);
      astfvm.calculate(simdat);
    }

    // Save simulation for this data set
    if (ee == offset) {  // Initialize simulation data with (first) sim data
      *sdata = simdat;
      jscan += nscans;
    } else {  // Append sim_data for second and subsequent data set
      for (int ss = 0; ss < nscans; ss++) {
        sdata->scanData[jscan++] = simdat.scanData[ss];
      }
    }
  }

  int ks = 0;
  double variance = 0.0;

  // Compute overall residual and variance
  for (int ee = 0; ee < lim_offs; ee++) {
    US_SolveSim::DataSet* dset = data_sets[ee];
    US_DataIO::EditedData* edata = &dset->run_data;
    US_DataIO::RawData simdat;
    int npoints = dset->run_data.pointCount();
    int nscans = dset->run_data.scanCount();

    for (int ss = 0; ss < nscans; ss++, ks++) {
      resid->scanData[ks] = sdata->scanData[ks];

      for (int rr = 0; rr < npoints; rr++) {
        double resval = edata->value(ss, rr) - sdata->value(ks, rr);
        variance += sq(resval);

        resid->setValue(ks, rr, resval);
      }
    }
  }

  sim_vals.variance = variance / (double)total_points;
  if (group_rank == 0)
    DbgLv(1) << my_rank << "CR: variance" << sim_vals.variance << variance
             << total_points;
}

// Get the value part of a "key value " substring
QString US_MPI_Analysis::par_key_value(const QString kvtext,
                                       const QString key) {
  QString value("");
  int keyx = kvtext.indexOf(key);
  value = (keyx < 0) ? value : QString(kvtext).mid(keyx).section(" ", 1, 1);
  if (group_rank < 2)
    DbgLv(0) << my_rank << ": p_k_v  key" << key << "value" << value << "kvtext"
             << kvtext;

  return value;
}
