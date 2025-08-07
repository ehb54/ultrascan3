#include "../include/us_saxs_util.h"
// Added by qt3to4:
#include <QTextStream>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os,
                                            const QString& str) {
  return os << qPrintable(str);
}

#if defined(USE_MPI)
extern int myrank;
#endif

bool US_Saxs_Util::nsa_sga_validate() {
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
    qsl_required << "targetedensity";
    qsl_required << "nsagenerations";
    qsl_required << "nsapopulation";
    qsl_required << "nsasgaincrement";

    for (unsigned int i = 0; i < (unsigned int)qsl_required.size(); i++) {
      if (!control_parameters.count(qsl_required[i])) {
        missing_required += " " + qsl_required[i];
      }
    }
  }

  if (!missing_required.isEmpty()) {
    errormsg = QString("Error: nsa requires prior definition of:%1")
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

    checks << "nsamutate";
    vals << "0.1";
    checks << "nsacrossover";
    vals << "0.3";

    validate_control_parameters_set_one(checks, vals);
  }

  if (!sgp_exp_q.size() || sgp_exp_q.size() != sgp_exp_I.size()) {
    errormsg =
        "Error: empty or inconsistant grid found experimental data, note:csv "
        "experiment grids not currently supported for sgp\n";
    return false;
  }

#if defined(USE_MPI)
  if (!nsa_mpi || !myrank) {
#endif
    cout << QString("setup experiment grid sizes %1 %2 %3\n")
                .arg(sgp_exp_q.size())
                .arg(sgp_exp_I.size())
                .arg(sgp_exp_e.size());
#if defined(USE_MPI)
  }
#endif

  if (sgp_exp_e.size() && sgp_exp_e.size() == sgp_exp_q.size() &&
      is_nonzero_vector(sgp_exp_q)) {
    sgp_use_e = true;
#if defined(USE_MPI)
    if (!nsa_mpi || !myrank) {
#endif
      cout << "Notice: nsa using sd's for fitting\n";
#if defined(USE_MPI)
    }
#endif
  } else {
    sgp_use_e = false;
#if defined(USE_MPI)
    if (!nsa_mpi || !myrank) {
#endif
      cout << "Notice: nsa NOT using sd's for fitting\n";
#if defined(USE_MPI)
    }
#endif
  }

  return true;
}

nsa_sga_individual nsa_sga_individual::random(
    unsigned int size /* in char's */) {
  nsa_sga_individual sga;

  for (unsigned int i = 0; i < size; i++) {
    char val = 0;
    for (unsigned int j = 0; j < 8; j++) {
      val |= drand48() > .5;
      val = val << 1;
    }
    sga.v.push_back(val);
  }

  return sga;
}

vector<sgp_sphere> nsa_sga_individual::sgp_spheres() {
  vector<sgp_sphere> result;

  // should do better than this, should grow from center?

  unsigned int dim = (unsigned int)pow((double)v.size() * 8e0, 1e0 / 3e0);

  // treat v as a bit string

  for (unsigned int i = 0; i < v.size(); i++) {
    unsigned int base = i * 8;  // assuming 8 bit characters

    for (unsigned int j = 0; j < 8; j++) {
      if ((v[i] >> j) & 1) {
        unsigned int pos = base + j;
        // cout << QString( "sgp_spheres: bit on i %1 j %2 pos %3 dim %4\n"
        // ).arg( i ).arg( j ).arg( pos ).arg( dim );

        sgp_sphere tmp_sphere;
        tmp_sphere.radius = 0.5;
        tmp_sphere.coordinate.axis[0] = pos % dim;
        tmp_sphere.coordinate.axis[1] = (pos / dim) % dim;
        tmp_sphere.coordinate.axis[2] = pos / (dim * dim);

        result.push_back(tmp_sphere);
      }
    }
  }

  return result;
}

QString nsa_sga_individual::qs_bead_model() {
  vector<sgp_sphere> spheres = sgp_spheres();

  QString qs;

  qs += QString("%1 %2\n").arg(spheres.size()).arg(.7);

  for (unsigned int i = 0; i < spheres.size(); i++) {
    qs +=
        QString("%1 %2 %3 %4  20 8 Unk 10\n")
            .arg(spheres[i].coordinate.axis[0] * sgp_params["distancequantum"])
            .arg(spheres[i].coordinate.axis[1] * sgp_params["distancequantum"])
            .arg(spheres[i].coordinate.axis[2] * sgp_params["distancequantum"])
            .arg(spheres[i].radius * sgp_params["distancequantum"]);
  }
  return qs;
}

vector<PDB_atom> nsa_sga_individual::bead_model() {
  vector<sgp_sphere> spheres = sgp_spheres();

  vector<PDB_atom> result;

  for (unsigned int i = 0; i < spheres.size(); i++) {
    PDB_atom tmp_atom;
    tmp_atom.coordinate.axis[0] =
        spheres[i].coordinate.axis[0] * sgp_params["distancequantum"];
    tmp_atom.coordinate.axis[1] =
        spheres[i].coordinate.axis[1] * sgp_params["distancequantum"];
    tmp_atom.coordinate.axis[2] =
        spheres[i].coordinate.axis[2] * sgp_params["distancequantum"];
    tmp_atom.bead_coordinate.axis[0] =
        spheres[i].coordinate.axis[0] * sgp_params["distancequantum"];
    tmp_atom.bead_coordinate.axis[1] =
        spheres[i].coordinate.axis[1] * sgp_params["distancequantum"];
    tmp_atom.bead_coordinate.axis[2] =
        spheres[i].coordinate.axis[2] * sgp_params["distancequantum"];
    tmp_atom.bead_computed_radius =
        spheres[i].radius * sgp_params["distancequantum"];
    tmp_atom.bead_actual_radius =
        spheres[i].radius * sgp_params["distancequantum"];
    tmp_atom.radius = spheres[i].radius * sgp_params["distancequantum"];
    tmp_atom.bead_mw = 20;
    tmp_atom.mw = 20;
    tmp_atom.bead_ref_volume = 0;
    tmp_atom.bead_color = 1;
    tmp_atom.serial = i + 1;
    tmp_atom.exposed_code = 1;
    tmp_atom.all_beads.clear();
    tmp_atom.name = "SGP";
    tmp_atom.resName = "SGP";
    tmp_atom.iCode = "";
    tmp_atom.chainID = "";
    tmp_atom.chain = 1;
    tmp_atom.active = 1;
    tmp_atom.normalized_ot_is_valid = false;

    result.push_back(tmp_atom);
  }

  return result;
}

double US_Saxs_Util::nsa_sga_fitness(nsa_sga_individual individual) {
  // take node & run current bead model iq on its bead model and compute chi2
  // (if errors present) or rmsd & return
  bead_models.resize(1);
  bead_models[0] = individual.bead_model();

  if (bead_models[0].size() > 1) {
    run_iqq_bead_model();
  } else {
    // cout << "single sphere fit\n";
    double delta_rho = control_parameters["targetedensity"].toDouble() -
                       our_saxs_options.water_e_density;
    if (fabs(delta_rho) < 1e-5) {
      delta_rho = 0e0;
    }

    if (!iqq_sphere("tmp", bead_models[0][0].bead_computed_radius, delta_rho,
                    our_saxs_options.start_q, our_saxs_options.end_q,
                    our_saxs_options.delta_q)) {
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
      ts << nsa_qs_bead_model();
      f.close();
    }
  }

  // cout << QString( "last fitness %1\n").arg( chi2 );
  return chi2;
}

bool US_Saxs_Util::nsa_sga(double& nrmsd) {
  list<nsa_sga_individual> nsa_pop;
  nsa_sga_individual individual;

  if (!control_parameters["nsasgaincrement"].toUInt()) {
    errormsg =
        "Error: nsa_sga requires nsasgaincrement to be greater than zero";
    return false;
  }

  cout << "nsa_sga init population\n";

  // init population
  for (unsigned int i = 0; i < control_parameters["nsapopulation"].toUInt();
       i++) {
    individual = nsa_sga_individual::random(2);
    cout << "nsa_sga init population: one individual:\n";
    cout << individual.qs_bead_model();
    cout << "nsa_sga init population: now fitness:\n";
    individual.fitness = nsa_sga_fitness(individual);
    nsa_pop.push_back(individual);
  }

  // sort by fitness
  double last_best_fitness = 1e99;
  unsigned int gens_with_no_improvement = 0;

  for (unsigned int g = 0; g < control_parameters["nsagenerations"].toUInt();
       g++) {
    nsa_pop.sort();
    nsa_pop.unique();

    // save current best
    nsa_sga_fitness(nsa_pop.front());
    QFile f(QString("sga-g%1-best.bead_model").arg(g));

    if (f.open(QIODevice::WriteOnly)) {
      QTextStream ts(&f);
      ts << nsa_qs_bead_model();
      ts << nsa_physical_stats();
      f.close();
      cout << QString("written: %1\n").arg(f.fileName());
    }

    map<unsigned int, bool> has_been_duplicated;

    if (!g) {
      last_best_fitness = nsa_pop.front().fitness;
    } else {
      if (last_best_fitness > nsa_pop.front().fitness) {
        last_best_fitness = nsa_pop.front().fitness;
        gens_with_no_improvement = 0;
      } else {
        gens_with_no_improvement++;
        if (control_parameters.count("nsaearlytermination") &&
            gens_with_no_improvement >=
                control_parameters["nsaearlytermination"].toUInt()) {
          cout << "early termination\n";
          break;
        }
      }
    }

    cout << QString("nsa: gen %1 best individual fitness %2\n")
                .arg(g)
                .arg(nsa_pop.front().fitness);

    unsigned int elitism_count = 0;
    unsigned int crossover_count = 0;
    unsigned int mutate_count = 0;
    unsigned int duplicate_count = 0;
    unsigned int random_count = 0;

    double avg_pop_beads_size = 0.0;
    vector<nsa_sga_individual> last_pop;
    for (list<nsa_sga_individual>::iterator it = nsa_pop.begin();
         it != nsa_pop.end(); it++) {
      last_pop.push_back(*it);
      avg_pop_beads_size += (double)it->v.size();
    }

    avg_pop_beads_size /= 3.0 * (double)last_pop.size();

    cout << QString("start: nsa_pop.size() %1 average individual size %2\n")
                .arg(last_pop.size())
                .arg(avg_pop_beads_size);

    nsa_pop.clear();

    for (unsigned int i = 0; i < control_parameters["nsapopulation"].toUInt();
         i++) {
      if (control_parameters.count("nsaelitism") &&
          i < control_parameters["nsaelitism"].toUInt()) {
        // cout << "elitism\n";
        nsa_pop.push_back(last_pop[i]);
        elitism_count++;
        continue;
      }

      if (control_parameters.count("nsamutate") &&
          drand48() < control_parameters["nsamutate"].toDouble()) {
        // cout << "mutate\n";
        nsa_sga_individual individual =
            last_pop[nsa_pop_selection(last_pop.size())];
        unsigned int pos = (unsigned int)(individual.v.size() * drand48());
        unsigned int bit = (unsigned int)(8 * drand48());
        // flip bit
        if ((individual.v[pos] >> bit) & 1) {
          // turn off
          individual.v[pos] &= ~(1 << bit);
        } else {
          // turn on
          individual.v[pos] |= 1 << bit;
        }

        individual.fitness = nsa_sga_fitness(individual);
        nsa_pop.push_back(individual);
        mutate_count++;
        continue;
      }

      if (control_parameters.count("nsacrossover") &&
          drand48() < control_parameters["nsacrossover"].toDouble()) {
        // cout << "crossover\n";
        nsa_sga_individual individual =
            last_pop[nsa_pop_selection(last_pop.size())];
        nsa_sga_individual individual2 =
            last_pop[nsa_pop_selection(last_pop.size())];
        unsigned int pos =
            (unsigned int)(drand48() * (double)individual.v.size());
        unsigned int pos2 =
            (unsigned int)(drand48() * (double)individual2.v.size());

        nsa_sga_individual new_individual;
        for (unsigned int j = 0; j < pos; j++) {
          new_individual.v.push_back(individual.v[j]);
        }
        for (unsigned int j = pos2; j < individual2.v.size(); j++) {
          new_individual.v.push_back(individual2.v[j]);
        }

        individual = new_individual;
        individual.fitness = nsa_sga_fitness(individual);
        nsa_pop.push_back(individual);
        crossover_count++;
        continue;
      }

      unsigned int pos = nsa_pop_selection(last_pop.size());
      if (has_been_duplicated.count(pos)) {
        nsa_sga_individual individual =
            nsa_sga_individual::random(last_pop[pos].v.size());
        individual.fitness = nsa_sga_fitness(individual);
        nsa_pop.push_back(individual);
        random_count++;
      } else {
        nsa_pop.push_back(last_pop[pos]);
        duplicate_count++;
      }
    }

    cout << QString(
                "summary counts:\n"
                " elitism   %1\n"
                " mutate    %2\n"
                " crossover %3\n"
                " duplicate %4\n"
                " random    %5\n"
                " total     %6\n")
                .arg(elitism_count)
                .arg(mutate_count)
                .arg(crossover_count)
                .arg(duplicate_count)
                .arg(random_count)
                .arg(elitism_count + mutate_count + crossover_count +
                     duplicate_count + random_count);
  }

  nsa_pop.sort();
  nsa_pop.unique();

  nsa_sga_last_individual = nsa_pop.front();
  nrmsd = nsa_sga_last_individual.fitness;

  return true;
}

bool US_Saxs_Util::nsa_sga_run() {
  QString save_outputfile = control_parameters["outputfile"];
  if (!nsa_sga_validate()) {
    return false;
  }
  setup_saxs_options();
  if (!sgp_init_sgp()) {
    return false;
  }

  double nrmsd;

  control_parameters["sgp_running"] = "yes";
  if (!nsa_sga(nrmsd)) {
    control_parameters.erase("sgp_running");
    return false;
  }

  control_parameters.erase("sgp_running");

  QString outname = QString("sga");

  QFile f(QString("%1.bead_model").arg(outname));

  if (f.open(QIODevice::WriteOnly)) {
    QTextStream ts(&f);
    ts << nsa_qs_bead_model();
    ts << nsa_physical_stats();
    ts << QString(
              "\n"
              "nsa parameters:\n"
              " population       %1\n"
              " generations      %2\n"
              " increment        %3\n"
              " scaling          %4\n")
              .arg(control_parameters["nsapopulation"])
              .arg(control_parameters["nsagenerations"])
              .arg(control_parameters["nsasgaincrement"])
              .arg(nsa_use_scaling_fit ? "Yes" : "No");

    ts << QString(" distance quantum %1\n").arg(sgp_params["distancequantum"]);

    ts << QString(
              " target curve     %1\n"
              " target curve sd  %2\n"
              " fitness          %3\n")
              .arg(control_parameters["experimentgrid"])
              .arg(sgp_use_e ? "present" : "not present or not useable")
              .arg(nsa_sga_last_individual.fitness);

    f.close();
    cout << QString("written: %1\n").arg(f.fileName());
    output_files << f.fileName();
  }
  control_parameters["outputfile"] = outname;
  nsa_sga_fitness(nsa_sga_last_individual);
  control_parameters["outputfile"] = save_outputfile;

  return true;
}
