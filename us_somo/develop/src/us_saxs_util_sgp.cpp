#include "../include/us_saxs_gp.h"
#include "../include/us_saxs_util.h"
// Added by qt3to4:
#include <QTextStream>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os,
                                            const QString& str) {
  return os << qPrintable(str);
}

bool US_Saxs_Util::sgp_run() {
  if (!sgp_validate()) {
    return false;
  }

  setup_saxs_options();

  control_parameters["sgp_running"] = "yes";
  if (!sgp_init()) {
    control_parameters.erase("sgp_running");
    return false;
  }

  sgp_node sgp;

  double last_best_fitness = population[0]->fitness;
  unsigned int gens_with_no_improvement = 0;

  for (unsigned int g = 1; g <= control_parameters["sgpgenerations"].toUInt();
       g++) {
    vector<sgp_node*> new_population;
    map<unsigned int, bool> has_been_duplicated;

    cout << QString("generation %1\n").arg(g);

    double sum = 0e0;
    double sum2 = 0e0;
    for (unsigned int i = 0; i < population.size(); i++) {
      double this_size = (double)population[i]->size();
      sum += this_size;
      sum2 += this_size * this_size;
    }
    double pop_size = (double)population.size();
    double pop_size_avg = sum / pop_size;
    double pop_size_sd =
        sqrt((pop_size * sum2 - sum * sum) / (pop_size * (pop_size - 1)));

    unsigned int elitism_count = 0;
    unsigned int crossover_count = 0;
    unsigned int mutate_count = 0;
    unsigned int plague_count = 0;
    unsigned int duplicate_count = 0;
    unsigned int random_count = 0;

    for (unsigned int i = 0; i < control_parameters["sgppopulation"].toUInt();
         i++) {
      // cout << QString( "individual %1\n" ).arg( i );
      if (control_parameters.count("sgpelitism") &&
          i < control_parameters["sgpelitism"].toUInt()) {
        // cout << "elitism\n";
        population[i]->fitness_ok = true;
        population[i]->check_normal("elitism");
        sgp_node* node = sgp.copy(population[sgp_pop_selection()]);
        node->fitness = population[i]->fitness;
        ;
        node->fitness_ok = true;
        new_population.push_back(node);
        elitism_count++;
        continue;
      }

      if (control_parameters.count("sgpmutate") &&
          drand48() < control_parameters["sgpmutate"].toDouble()) {
        // cout << "mutate\n";
        sgp_node* node = sgp.copy(population[sgp_pop_selection()]);
        node->mutate((unsigned int)(node->size() * drand48()));
        node->fitness_ok = false;
        node->check_normal("mutate");
        new_population.push_back(node);
        mutate_count++;
        continue;
      }

      if (control_parameters.count("sgpcrossover") &&
          drand48() < control_parameters["sgpcrossover"].toDouble()) {
        // cout << "crossover\n";
        sgp_node* node1 = sgp.copy(population[sgp_pop_selection()]);
        sgp_node* node2 = sgp.copy(population[sgp_pop_selection()]);
        sgp_node* node;
        sgp.crossover(node, node1, node2);
        delete node1;
        delete node2;
        node->fitness_ok = false;
        node->check_normal("crossover");
        new_population.push_back(node);
        crossover_count++;
        continue;
      }

      if (control_parameters.count("sgpplague") &&
          drand48() < control_parameters["sgpplague"].toDouble()) {
        // cout << "plague\n";
        unsigned int pos;
        do {
          pos = sgp_pop_selection();
        } while (control_parameters.count("sgpelitism") &&
                 pos < control_parameters["sgpelitism"].toUInt());

        // cout << QString( "plague selected %1\n" ).arg( pos );
        sgp_node* node1 = population[pos];
        node1->is_dead = true;

        // add 1 to keep population size
        double r1;
        double r2;
        do {
          sgp.random_normal(r1, r2, pop_size_avg, pop_size_sd, pop_size_avg,
                            pop_size_sd);
        } while (r1 < 2.0);
        sgp_node* node = sgp.random((unsigned int)r1);
        node->fitness_ok = false;
        node->check_normal("plague/random");
        new_population.push_back(node);
        plague_count++;
        continue;
      }

      // cout << "duplicate\n";
      unsigned int pos = sgp_pop_selection();
      if (has_been_duplicated.count(pos)) {
        double r1;
        double r2;
        do {
          sgp.random_normal(r1, r2, pop_size_avg, pop_size_sd, pop_size_avg,
                            pop_size_sd);
        } while (r1 < 2.0);
        sgp_node* node = sgp.random((unsigned int)r1);
        node->fitness_ok = false;
        node->check_normal("duplicate/random");
        new_population.push_back(node);
        random_count++;
        continue;
      } else {
        has_been_duplicated[pos] = true;
        sgp_node* node1 = population[sgp_pop_selection()];
        sgp_node* node = sgp.copy(node1);
        node->fitness = node1->fitness;
        node->fitness_ok = true;
        node->check_normal("duplicate");
        new_population.push_back(node);
        duplicate_count++;
      }
    }

    cout << QString(
                "summary counts:\n"
                " elitism   %1\n"
                " mutate    %2\n"
                " crossover %3\n"
                " plague    %4\n"
                " duplicate %5\n"
                " random    %6\n"
                " total     %7\n")
                .arg(elitism_count)
                .arg(mutate_count)
                .arg(crossover_count)
                .arg(plague_count)
                .arg(duplicate_count)
                .arg(random_count)
                .arg(elitism_count + mutate_count + crossover_count +
                     plague_count + duplicate_count + random_count);

    for (unsigned int i = 0; i < population.size(); i++) {
      delete population[i];
    }
    population = new_population;

    sgp_calculate_population_fitness();

    if (control_parameters.count("sgpremoveduplicates")) {
      map<sgp_node*, bool> dup_nodes;

      for (unsigned int i = 1; i < population.size(); i++) {
        if (population[i - 1]->fitness == population[i]->fitness &&
            population[i - 1]->size() == population[i]->size()) {
          point p0 = population[i - 1]->checksum();
          point p1 = population[i]->checksum();
          if (p0.axis[0] == p1.axis[0] && p0.axis[1] == p1.axis[1] &&
              p0.axis[2] == p1.axis[2]) {
            // they are probably might be the same
            dup_nodes[population[i]] = true;
          }
        }
      }

      // now really remove them:
      cout << QString("Duplicates found %1\n").arg(dup_nodes.size());

      new_population.clear();
      for (unsigned int i = 0; i < population.size(); i++) {
        if (!dup_nodes.count(population[i])) {
          new_population.push_back(population[i]);
        } else {
          delete population[i];
        }
      }
      population = new_population;
    }

    // save best member:

    QFile f(QString("sgp_g%1_best.bead_model").arg(g));
    if (f.open(QIODevice::WriteOnly)) {
      QTextStream ts(&f);
      ts << population[0]->qs_bead_model();
      f.close();
      cout << QString("written: %1\n").arg(f.fileName());
    }
    cout << sgp_physical_stats(population[0]);

    control_parameters.erase("sgp_running");
    population[0]->fitness = sgp_fitness(population[0]);
    control_parameters["sgp_running"] = "yes";
    cout << "Usage:" << sgp.usage();

    if (last_best_fitness > population[0]->fitness) {
      last_best_fitness = population[0]->fitness;
      gens_with_no_improvement = 0;
    } else {
      gens_with_no_improvement++;
      cout << QString("generations without improvement %1\n")
                  .arg(gens_with_no_improvement);
    }

    if (control_parameters.count("sgpearlytermination") &&
        gens_with_no_improvement >=
            control_parameters["sgpearlytermination"].toUInt()) {
      cout << "early termination\n";
      break;
    }

  }  // g

  for (unsigned int i = 0; i < population.size(); i++) {
    delete population[i];
  }
  cout << "Final usage:" << sgp.usage();

  control_parameters.erase("sgp_running");
  return false;
}

unsigned int US_Saxs_Util::sgp_pop_selection() {
  // exponential ranking selection
  int pos;
  double beta = population.size() / 8e0;

  do {
    pos = (int)(-log(1e0 - drand48()) * beta);
    if (pos >= (int)population.size()) {
      pos = population.size() - 1;
    }
    if (pos < 0) {
      pos = 0;
    }
  } while (population[pos]->is_dead);
  // cout << QString( "selection pos %1\n" ).arg( pos );
  return (unsigned int)pos;
}

bool US_Saxs_Util::sgp_init_sgp() {
  if (control_parameters.count("sgprandomseed")) {
    srand48((long int)control_parameters["sgprandomseed"].toInt());
  } else {
    long int li = (long int)QTime::currentTime().msec();
    cout << QString("to reproduce use random seed %1\n").arg(li);
    srand48(li);
  }

  QStringList param;
  QStringList sgp_param;
  param << "sgpdistancequantum"
        << "sgpdistancemin"
        << "sgpdistancemax"
        << "sgpradiusmin"
        << "sgpradiusmax"
        << "sgpbranchmax";

  sgp_param << "distancequantum"
            << "distancemin"
            << "distancemax"
            << "radiusmin"
            << "radiusmax"
            << "branchmax";

  for (unsigned int i = 0; i < (unsigned int)param.size(); i++) {
    sgp_params[sgp_param[i]] = control_parameters[param[i]].toDouble();
  }

  if (!sgp_node::validate_params().isEmpty()) {
    errormsg = sgp_node::validate_params();
    return false;
  }
  return true;
}

bool US_Saxs_Util::sgp_init() {
  errormsg = "";
  noticemsg = "";

  if (!sgp_init_sgp()) {
    return false;
  }

  population.clear();

  for (unsigned int i = 0; i < control_parameters["sgppopulation"].toUInt();
       i++) {
    population.push_back(sgp.random(10 + i));
    if (i && !(i % 100)) {
      cout << QString("creating population %1 of %2\n")
                  .arg(i)
                  .arg(control_parameters["sgppopulation"]);
    }
  }

  cout << QString("population of %1 created\n").arg(population.size());
  sgp_calculate_population_fitness();

  return true;
}

bool US_Saxs_Util::sgp_validate() {
  errormsg = "";
  noticemsg = "";

  // need to be able to process iq:
  if (!validate_control_parameters(true)) {
    return false;
  }

  // first check required parameters

  QString missing_required;

  QStringList qsl_required;

  {
    qsl_required << "sgpgenerations";
    qsl_required << "sgppopulation";
    qsl_required << "targetedensity";

    for (unsigned int i = 0; i < (unsigned int)qsl_required.size(); i++) {
      if (!control_parameters.count(qsl_required[i])) {
        missing_required += " " + qsl_required[i];
      }
    }
  }

  if (!missing_required.isEmpty()) {
    errormsg = QString("Error: GP requires prior definition of:%1")
                   .arg(missing_required);
    return false;
  }

  // if certain parameters are not set, set them to defaults
  QStringList checks;
  QStringList vals;

  {
    checks << "sgpdistancequantum";
    vals << "2.5";
    checks << "sgpdistancemin";
    vals << "2.0";
    checks << "sgpdistancemax";
    vals << "10.0";
    checks << "sgpradiusmin";
    vals << "1.0";
    checks << "sgpradiusmax";
    vals << "5.0";
    checks << "sgpbranchmax";
    vals << "4.0";

    checks << "sgpmutate";
    vals << "0.3";
    checks << "sgpcrossover";
    vals << "0.4";
    checks << "sgpplague";
    vals << "0.05";

    validate_control_parameters_set_one(checks, vals);
  }

  if (!sgp_exp_q.size() || sgp_exp_q.size() != sgp_exp_I.size()) {
    errormsg =
        "Error: empty or inconsistant grid found experimental data, note:csv "
        "experiment grids not currently supported for sgp\n";
    return false;
  }

  cout << QString("setup experiment grid sizes %1 %2 %3\n")
              .arg(sgp_exp_q.size())
              .arg(sgp_exp_I.size())
              .arg(sgp_exp_e.size());

  if (sgp_exp_e.size() && sgp_exp_e.size() == sgp_exp_q.size() &&
      is_nonzero_vector(sgp_exp_q)) {
    sgp_use_e = true;
    cout << "Notice: sgp using sd's for fitting\n";
  } else {
    sgp_use_e = false;
    cout << "Notice: sgp NOT using sd's for fitting\n";
  }

  return true;
}

double US_Saxs_Util::sgp_fitness(sgp_node* node) {
  // take node & run current bead model iq on its bead model and compute chi2
  // (if errors present) or rmsd & return

  vector<PDB_atom> bm = node->bead_model();
  bead_models.clear();
  bead_models.push_back(bm);

  // compute iq:

  if (bm.size() > 1) {
    run_iqq_bead_model();
  } else {
    // cout << "single sphere fit\n";
    double delta_rho = control_parameters["targetedensity"].toDouble() -
                       our_saxs_options.water_e_density;
    if (fabs(delta_rho) < 1e-5) {
      delta_rho = 0e0;
    }

    if (!iqq_sphere("tmp", bm[0].radius, delta_rho, our_saxs_options.start_q,
                    our_saxs_options.end_q, our_saxs_options.delta_q)) {
      cout << errormsg;
      return false;
    }
    sgp_last_I = wave["tmp"].r;
    write_output("0", wave["tmp"].q, wave["tmp"].r);
  }

  // values stored in last_q, last_I, experiment in sgp_exp_q,I,e
  double k;
  double chi2;
  if (sgp_use_e) {
    scaling_fit(sgp_last_I, sgp_exp_I, sgp_exp_e, k, chi2);
  } else {
    scaling_fit(sgp_last_I, sgp_exp_I, k, chi2);
  }

  if (us_isnan(chi2)) {
    QFile f("nan_fitness_model.bead_model");
    if (f.open(QIODevice::WriteOnly)) {
      QTextStream ts(&f);
      ts << node->qs_bead_model();
      f.close();
    }
  }

  return chi2;
}

void US_Saxs_Util::sgp_calculate_population_fitness() {
  for (unsigned int i = 0; i < population.size(); i++) {
    if (!population[i]->fitness_ok) {
      population[i]->fitness = sgp_fitness(population[i]);
    }
    cout << QString("sgp_fitness for %1 %2\n")
                .arg(i)
                .arg(population[i]->fitness);
  }
  sgp_sort_population();
}

void US_Saxs_Util::sgp_sort_population() {
  list<sortable_sgp_node> ssns;

  for (unsigned int i = 0; i < population.size(); i++) {
    sortable_sgp_node ssn;
    ssn.node = population[i];
    ssns.push_back(ssn);
  }

  ssns.sort();

  vector<sgp_node*> sorted_population;

  for (list<sortable_sgp_node>::iterator it = ssns.begin(); it != ssns.end();
       it++) {
    sorted_population.push_back((*it).node);
  }

  population = sorted_population;

  cout << "after sort:\n";
  for (unsigned int i = 0; i < population.size(); i++) {
    cout << QString("sgp_fitness for %1 %2\n")
                .arg(i)
                .arg(population[i]->fitness);
  }
}
