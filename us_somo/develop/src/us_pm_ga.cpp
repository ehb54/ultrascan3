#include "../include/us_pm.h"

// #define USPMGA_DEBUG

unsigned int US_PM::ga_pop_selection(unsigned int size) {
  // exponential ranking selection
  int pos;
  double beta = size / 8e0;

  pos = (int)(-log(1e0 - drand48()) * beta);
  if (pos >= (int)size) {
    pos = size - 1;
  }
  if (pos < 0) {
    pos = 0;
  }

  return (unsigned int)pos;
}

bool US_PM::ga_fitness(pm_ga_individual& individual) {
  ga_delta_to_fparams(individual.v, ga_fparams);
  join(ga_params, ga_types, ga_fparams);
  individual.fitness = model_fit(ga_params, individual.model, ga_I_result);
  individual.fitness_computed = true;
  //    cout << US_Vector::qs_vector( "ga_fitness: v", individual.v );
  //    cout << US_Vector::qs_vector( "ga_fitness: params", ga_params );
  //    cout << QString( "ga_fitness: fitness %1 beads %2\n" )
  //       .arg( individual.fitness )
  //       .arg( individual.model.size() );

  return true;
}

void US_PM::ga_compute_fitness() {
  clock_t start_t = clock();
  unsigned int fit_calls = 0;
#if defined(USE_MPI)
  int errorno = -31000;

  list<pm_ga_individual>::iterator it_pop = ga_pop.begin();
  while (it_pop != ga_pop.end() && it_pop->fitness_computed) {
    it_pop++;
  }
  if (it_pop == ga_pop.end()) {
    return;
  }

  pm_msg msg;
  msg.type = PM_CALC_FITNESS;

  map<int, list<pm_ga_individual>::iterator> worker_pop;

  // send out initial requests

  for (set<int>::iterator it = pm_workers_registered.begin();
       it != pm_workers_registered.end() && it_pop != ga_pop.end();
       it++, it_pop++) {
    //       cout << QString(
    //                       "ga_compute_fitness initial requests for() loop:\n"
    //                       "workers registered: %1\n"
    //                       "workers waiting   : %2\n"
    //                       "workers busy      : %3\n" )
    //          .arg( pm_workers_registered.size() )
    //          .arg( pm_workers_waiting.size() )
    //          .arg( pm_workers_busy.size() )
    //            << flush;

    while (it_pop != ga_pop.end() && it_pop->fitness_computed) {
      it_pop++;
    }
    if (it_pop == ga_pop.end()) {
      break;
    }

    ga_delta_to_fparams(it_pop->v, ga_fparams);
    join(ga_params, ga_types, ga_fparams);
    msg.vsize = ga_params.size();

    // if ( us_log )
    // {
    //    us_log->log( US_Vector::qs_vector( "ga_compute_fitness: (for) params
    //    to send", ga_params ) );
    // }
    fit_calls++;

    if (MPI_SUCCESS !=
        MPI_Send(&msg, sizeof(msg), MPI_CHAR, *it, PM_MSG, MPI_COMM_WORLD)) {
      if (us_log) {
        us_log->log(QString("%1: MPI PM_MSG Send failed ga_compute_fitness()\n")
                        .arg(myrank));
      }
      MPI_Abort(MPI_COMM_WORLD, errorno - myrank);
      exit(errorno - myrank);
    }

    if (MPI_SUCCESS != MPI_Send(&(ga_params[0]), msg.vsize * sizeof(double),
                                MPI_CHAR, *it, PM_CALC_FITNESS,
                                MPI_COMM_WORLD)) {
      if (us_log) {
        us_log->log(
            QString(
                "%1: MPI PM_CALC_FITNESS Send failed ga_compute_fitness()\n")
                .arg(myrank));
      }
      MPI_Abort(MPI_COMM_WORLD, errorno - myrank);
      exit(errorno - myrank);
    }
    pm_workers_busy.insert(*it);
    pm_workers_waiting.erase(*it);
    worker_pop[*it] = it_pop;
  }

  if (!pm_workers_busy.size()) {
    if (it_pop != ga_pop.end()) {
      if (us_log) {
        us_log->log(QString("%1: error: ga_compute_fitness() no worker busy "
                            "but we have more ga_pop?\n")
                        .arg(myrank));
      }
      MPI_Abort(MPI_COMM_WORLD, errorno - myrank);
      exit(errorno - myrank);
    }
    return;
  }

  // now wait for workers to become free, get results and send out a new job if
  // any

  MPI_Status mpi_status;
  MPI_Status mpi_status2;
  pm_msg msg_receive;

  while (pm_workers_busy.size()) {
    //       cout << QString(
    //                       "ga_compute_fitness while() loop:\n"
    //                       "workers registered: %1\n"
    //                       "workers waiting   : %2\n"
    //                       "workers busy      : %3\n" )
    //          .arg( pm_workers_registered.size() )
    //          .arg( pm_workers_waiting.size() )
    //          .arg( pm_workers_busy.size() )
    //            << flush;

    while (it_pop != ga_pop.end() && it_pop->fitness_computed) {
      it_pop++;
    }

    if (MPI_SUCCESS != MPI_Recv(&msg_receive, sizeof(msg_receive), MPI_CHAR,
                                MPI_ANY_SOURCE, PM_FITNESS_RESULT,
                                MPI_COMM_WORLD, &mpi_status)) {
      if (us_log) {
        us_log->log(QString("%1: MPI PM_FITNESS_RESULT Receive failed in "
                            "ga_compute_fitness()\n")
                        .arg(myrank));
      }
      MPI_Abort(MPI_COMM_WORLD, errorno - myrank);
      exit(errorno - myrank);
    }

    vector<int16_t> vmodel(3 * msg_receive.vsize);

    if (MPI_SUCCESS !=
        MPI_Recv(&(vmodel[0]), 3 * msg_receive.vsize * sizeof(int16_t),
                 MPI_CHAR, mpi_status.MPI_SOURCE, PM_FITNESS_RESULT_MODEL,
                 MPI_COMM_WORLD, &mpi_status2)) {
      if (us_log) {
        us_log->log(QString("%1: MPI PM_FITNESS_RESULT Receive failed in "
                            "ga_compute_fitness()\n")
                        .arg(myrank));
      }
      MPI_Abort(MPI_COMM_WORLD, errorno - myrank);
      exit(errorno - myrank);
    }

    (*worker_pop[mpi_status.MPI_SOURCE]).fitness = msg_receive.model_fitness;
    (*worker_pop[mpi_status.MPI_SOURCE]).model.clear();

    pm_point tmp_pmp;

    for (int i = 0; i < (int)msg_receive.vsize; ++i) {
      tmp_pmp.x[0] = vmodel[i * 3];
      tmp_pmp.x[1] = vmodel[i * 3 + 1];
      tmp_pmp.x[2] = vmodel[i * 3 + 2];
      (*worker_pop[mpi_status.MPI_SOURCE]).model.insert(tmp_pmp);
    }

    worker_pop[mpi_status.MPI_SOURCE]->fitness_computed = true;

    if (it_pop == ga_pop.end()) {
      worker_pop.erase(mpi_status.MPI_SOURCE);
      pm_workers_busy.erase(mpi_status.MPI_SOURCE);
      pm_workers_waiting.insert(mpi_status.MPI_SOURCE);
      continue;
    }

    // we still have more jobs

    ga_delta_to_fparams(it_pop->v, ga_fparams);
    join(ga_params, ga_types, ga_fparams);
    msg.vsize = ga_params.size();
    // if ( us_log )
    // {
    //    us_log->log( US_Vector::qs_vector( "ga_compute_fitness: (while) params
    //    to send", ga_params ) );
    // }

    fit_calls++;

    if (MPI_SUCCESS != MPI_Send(&msg, sizeof(msg), MPI_CHAR,
                                mpi_status.MPI_SOURCE, PM_MSG,
                                MPI_COMM_WORLD)) {
      if (us_log) {
        us_log->log(QString("%1: MPI PM_MSG Send failed ga_compute_fitness()\n")
                        .arg(myrank));
      }
      MPI_Abort(MPI_COMM_WORLD, errorno - myrank);
      exit(errorno - myrank);
    }

    if (MPI_SUCCESS != MPI_Send(&(ga_params[0]), msg.vsize * sizeof(double),
                                MPI_CHAR, mpi_status.MPI_SOURCE,
                                PM_CALC_FITNESS, MPI_COMM_WORLD)) {
      if (us_log) {
        us_log->log(
            QString(
                "%1: MPI PM_CALC_FITNESS Send failed ga_compute_fitness()\n")
                .arg(myrank));
      }
      MPI_Abort(MPI_COMM_WORLD, errorno - myrank);
      exit(errorno - myrank);
    }

    worker_pop[mpi_status.MPI_SOURCE] = it_pop;
    it_pop++;
  }
#else
  for (list<pm_ga_individual>::iterator it = ga_pop.begin(); it != ga_pop.end();
       it++) {
    if (!it->fitness_computed) {
      fit_calls++;
      ga_fitness(*it);
    }
  }
#endif
  clock_t end_t = clock();
  //    cout << QString( "ga fitness of %1 individuals computed in %2s or %2s
  //    per individual\n" )
  //       .arg( ga_pop.size() )
  //       .arg( (double)(end_t - start_t )/ CLOCKS_PER_SEC, 0, 'f', 8 )
  //       .arg( (double)( (end_t - start_t )/ CLOCKS_PER_SEC ) / ga_pop.size(),
  //       0, 'f', 8 );
  //    ;
  pm_ga_fitness_calls += fit_calls;
  pm_ga_fitness_secs += (double)((end_t - start_t) / CLOCKS_PER_SEC);
}

bool US_PM::ga(pm_ga_individual& best_individual) {
  if (us_log) {
    us_log->log(ga_info());
  }
  pm_ga_individual individual;

  individual.v.resize(ga_fparams_size);

  // init population
  if (ga_p.full_grid) {
    for (unsigned int i = 0; i < ga_p.population; i++) {
      unsigned int r = i;
      for (unsigned int j = 0; j < ga_fparams_size; j++) {
        individual.v[j] = r % ga_points[j];
        r = r / ga_points[j];
      }
      individual.fitness_computed = false;
      ga_pop.push_back(individual);
    }
  } else {
    unsigned int ga_seed_params_size = (unsigned int)ga_seed_params.size();
    if (us_log) {
      us_log->log(QString("ga_seeded: %1").arg(ga_seed_params_size));
    }
    for (unsigned int i = 0; i < ga_seed_params_size; i++) {
      ga_params_to_individual(ga_seed_params[i], individual);
      individual.fitness_computed = false;
      ga_pop.push_back(individual);
    }
    for (unsigned int i = ga_seed_params_size; i < ga_p.population; i++) {
      for (unsigned int j = 0; j < ga_fparams_size; j++) {
        individual.v[j] = int(((double)ga_points[j] * drand48()));
      }
      individual.fitness_computed = false;
      ga_pop.push_back(individual);
    }
  }

  // sort by fitness
  double last_best_fitness = 1e99;
  unsigned int gens_with_no_improvement = 0;

  if (!ga_p.full_grid) {
    vector<double> random_normal_sd(ga_points.size());

    for (unsigned int g = 0; g < ga_p.generations; ++g) {
      ga_compute_fitness();
      ga_pop.sort();
      ga_pop.unique();

      if (ga_p.sa_mutate) {
        for (int pos = 0; pos < (int)ga_points.size(); ++pos) {
          random_normal_sd[pos] =
              ((double)ga_points[pos]) / (6e0 * log(2e0 + 2e0 * g) / log(2e0));
          if (ga_points[pos] > 1 && random_normal_sd[pos] < 1e0) {
            random_normal_sd[pos] = 1e0;
          }
        }
        if (us_log) {
          us_log->log(US_Vector::qs_vector("ga points", ga_points));
          us_log->log(
              US_Vector::qs_vector("random_normal_sd", random_normal_sd));
        }
      }

      map<unsigned int, bool> has_been_duplicated;

      if (!g) {
        last_best_fitness = ga_pop.front().fitness;
      } else {
        if (last_best_fitness > ga_pop.front().fitness) {
          last_best_fitness = ga_pop.front().fitness;
          gens_with_no_improvement = 0;
        } else {
          gens_with_no_improvement++;
          if (gens_with_no_improvement >= ga_p.early_termination) {
            if (us_log) {
              us_log->log("early termination\n");
            }
            break;
          }
        }
      }

      if (us_log) {
        us_log->log(QString("ga_: gen %1 best individual fitness %2 beads %3\n")
                        .arg(g)
                        .arg(ga_pop.front().fitness)
                        .arg(ga_pop.front().model.size()));
      }

      {
        ga_delta_to_fparams(ga_pop.front().v, ga_fparams);
        join(ga_params, ga_types, ga_fparams);
        if (us_log) {
          us_log->log(US_Vector::qs_vector("ga_fitness: v", ga_pop.front().v));
          us_log->log(US_Vector::qs_vector("ga_fitness: params", ga_params));
        }
      }

      unsigned int elitism_count = 0;
      unsigned int crossover_count = 0;
      unsigned int mutate_count = 0;
      unsigned int sa_mutate_count = 0;
      unsigned int duplicate_count = 0;
      unsigned int random_count = 0;

      vector<pm_ga_individual> last_pop;
      for (list<pm_ga_individual>::iterator it = ga_pop.begin();
           it != ga_pop.end(); it++) {
        last_pop.push_back(*it);
      }

      if (us_log) {
        us_log->log(QString("start: ga_pop.size() %1\n").arg(last_pop.size()));
      }

      ga_pop.clear();

      for (unsigned int i = 0; i < ga_p.population; i++) {
        if (i < ga_p.elitism) {
          // cout << "elitism\n";
          // ga_fitness( last_pop[ i ] );
          ga_pop.push_back(last_pop[i]);
          elitism_count++;
          continue;
        }

        if (drand48() < ga_p.mutate) {
          // cout << "mutate\n";
          individual = last_pop[ga_pop_selection(last_pop.size())];
          unsigned int pos = (unsigned int)(drand48() * ga_fparams_size);
          individual.v[pos] = int(((double)ga_points[pos] * drand48()));
          // ga_fitness( individual );
          individual.fitness_computed = false;
          ga_pop.push_back(individual);
          mutate_count++;
          continue;
        }

        if (drand48() < ga_p.sa_mutate) {
          // cout << "sa_mutate\n";
          double x;
          double y;

          unsigned int pos = (unsigned int)(drand48() * ga_fparams_size);
          if (ga_points[pos] > 1) {
            individual = last_pop[ga_pop_selection(last_pop.size())];
            random_normal((double)individual.v[pos], random_normal_sd[pos],
                          (double)individual.v[pos], random_normal_sd[pos], x,
                          y);
            int xi = (int)x;
            int yi = (int)y;

            if (!xi) {
              xi = yi;
            }
            if (xi < 0) {
              xi = 0;
            }
            if (xi >= (int)ga_points[pos]) {
              xi = ga_points[pos] - 1;
            }
            individual.v[pos] = xi;
            // ga_fitness( individual );
            individual.fitness_computed = false;
            ga_pop.push_back(individual);
            sa_mutate_count++;
            continue;
          }
        }

        if (drand48() < ga_p.crossover) {
          // cout << "crossover\n";
          individual = last_pop[ga_pop_selection(last_pop.size())];
          pm_ga_individual individual2 =
              last_pop[ga_pop_selection(last_pop.size())];
          unsigned int pos = (unsigned int)(drand48() * ga_fparams_size);
          for (unsigned int j = pos; j < ga_fparams_size; j++) {
            individual.v[j] = individual2.v[j];
          }
          // ga_fitness( individual );
          individual.fitness_computed = false;
          ga_pop.push_back(individual);
          crossover_count++;
          continue;
        }

        unsigned int pos = ga_pop_selection(last_pop.size());
        if (has_been_duplicated.count(pos)) {
          for (unsigned int j = 0; j < ga_fparams_size; j++) {
            individual.v[j] = int(((double)ga_points[j] * drand48()));
          }
          // ga_fitness( individual );
          individual.fitness_computed = false;
          ga_pop.push_back(individual);
          random_count++;
        } else {
          has_been_duplicated[pos] = true;
          individual = last_pop[ga_pop_selection(last_pop.size())];
          // ga_fitness( individual );
          individual.fitness_computed = false;
          ga_pop.push_back(individual);
          duplicate_count++;
        }
      }

      if (us_log) {
        us_log->log(QString("summary counts:\n"
                            " elitism   %1\n"
                            " mutate    %2\n"
                            " sa_mutate %3\n"
                            " crossover %4\n"
                            " duplicate %5\n"
                            " random    %6\n"
                            " total     %7\n")
                        .arg(elitism_count)
                        .arg(mutate_count)
                        .arg(sa_mutate_count)
                        .arg(crossover_count)
                        .arg(duplicate_count)
                        .arg(random_count)
                        .arg(elitism_count + mutate_count + sa_mutate_count +
                             crossover_count + duplicate_count + random_count));
      }
    }
  }
  ga_compute_fitness();
  ga_pop.sort();
  best_individual = ga_pop.front();

  if (us_log) {
    us_log->log(QString("ga_: gen final best individual fitness %1 beads %2\n")
                    .arg(ga_pop.front().fitness)
                    .arg(ga_pop.front().model.size()));
  }

  {
    ga_delta_to_fparams(ga_pop.front().v, ga_fparams);
    join(ga_params, ga_types, ga_fparams);
    if (us_log) {
      us_log->log(US_Vector::qs_vector("ga_fitness: v", ga_pop.front().v));
      us_log->log(US_Vector::qs_vector("ga_fitness: params", ga_params));
    }
  }

  return true;
}

bool US_PM::ga_state_ok() {
  if (ga_low_fparams.size() != ga_high_fparams.size()) {
    error_msg = "ga_state_ok: not ok, fparams not equal";
    return false;
  }

  for (int i = 0; i < (int)ga_low_fparams.size(); i++) {
    if (ga_low_fparams[i] > ga_high_fparams[i]) {
      // US_Vector::printvector2( QString( "low fparams > high fparams @ %1"
      // ).arg( i ), ga_low_fparams, ga_high_fparams );

      error_msg = "ga_state_ok: not ok, low fparams > high fparams";
      return false;
    }
  }
  return true;
}

bool US_PM::ga_compute_delta(unsigned int points_max) {
  if (!ga_state_ok()) {
    return false;
  }
  if (points_max < 2) {
    error_msg = "ga_compute_delta: too few points";
    return false;
  }
  ga_delta.resize(ga_low_fparams.size());
  ga_points.resize(ga_low_fparams.size());

  if (us_log) {
    us_log->log(QString("ga_compute_delta points max %1").arg(points_max));
  }

  unsigned int total_pts = 1;

  pm_ga_last_max_best_delta_min = 0e0;

  // if ( us_log )
  // {
  //    us_log->log( US_Vector::qs_vector2( "ga_compute_delta low high",
  //    ga_low_fparams, ga_high_fparams ) );
  // }

  for (int i = 0; i < (int)ga_low_fparams.size(); i++) {
    if (ga_low_fparams[i] == ga_high_fparams[i]) {
      ga_delta[i] = 0e0;
      ga_points[i] = 1;
    } else {
      ga_delta[i] =
          (ga_high_fparams[i] - ga_low_fparams[i]) / (double)(points_max - 1);
      if (us_log) {
        us_log->log(QString("ga_compute_delta: ga_delta[ %1 ] = %2\n")
                        .arg(i)
                        .arg(ga_delta[i]));
      }
      if (ga_delta[i] < best_delta_min) {
        if (us_log) {
          us_log->log(
              QString("ga_compute_delta: ga_delta[ %1 ] < best_delta_min %2")
                  .arg(i)
                  .arg(best_delta_min));
        }
        ga_delta[i] = best_delta_min;
        ga_points[i] =
            (unsigned int)(0.5 + (ga_high_fparams[i] - ga_low_fparams[i]) /
                                     best_delta_min);
        if (ga_points[i] == 0) {
          ga_delta[i] = 0e0;
          ga_points[i] = 1;
        }
      } else {
        ga_points[i] = points_max;
      }
      if (pm_ga_last_max_best_delta_min < ga_delta[i]) {
        pm_ga_last_max_best_delta_min = ga_delta[i];
      }
    }
    if (ga_points[i] > 0 && total_pts > INT_MAX / ga_points[i]) {
      // overflow, set to MAX_INT
      us_log->log("overflow detected\n");
      total_pts = INT_MAX;
    } else {
      if (ga_points[i] > 0) {
        total_pts *= ga_points[i];
      }
    }

    // if ( us_log )
    // {
    //    us_log->log( QString( "ga_points[ %1 ] = %2 total pts %3\n" )
    //                 .arg( i ).arg( ga_points[ i ] ).arg( total_pts ) );
    // }
  }

  if (us_log) {
    us_log->log(QString("ga_compute_delta: total pts %1").arg(total_pts));
    us_log->log(QString("ga_compute_delta: last_max_best_delta_min %1 ")
                    .arg(pm_ga_last_max_best_delta_min));
    us_log->log(US_Vector::qs_vector("ga_compute_delta: deltas", ga_delta));
    us_log->log(US_Vector::qs_vector("ga_compute_delta: points", ga_points));
  }

  if (total_pts <=
      ga_p.population *
          (ga_p.generations > 2 ? ga_p.generations / 2 : ga_p.generations)) {
    if (us_log) {
      us_log->log(
          "ga_compute_delta: points <= population * generations / 2, switching "
          "to full grid search\n");
    }
    ga_p.population = total_pts;
    ga_p.generations = 1;
    ga_p.mutate = 0e0;
    ga_p.sa_mutate = 0e0;
    ga_p.crossover = 0e0;
    ga_p.elitism = 0;
    ga_p.early_termination = 1;
    ga_p.full_grid = true;
  } else {
    ga_p.full_grid = false;
  }

  ga_delta_ok = true;
  return true;
}

bool US_PM::ga_refine_limits(unsigned int top_count, bool needs_sort_unique,
                             unsigned int extend_deltas) {
  if (needs_sort_unique) {
    ga_pop.sort();
    ga_pop.unique();
  }

  if ((unsigned int)ga_pop.size() < top_count) {
    error_msg = "ga_refine_limits: insufficient population";
    return false;
  }

  vector<int> mins(ga_fparams_size);
  vector<int> maxs(ga_fparams_size);

  unsigned int i = 0;
  for (list<pm_ga_individual>::iterator it = ga_pop.begin();
       it != ga_pop.end() && i < top_count; ++it, ++i) {
    if (i) {
      for (unsigned int j = 0; j < ga_fparams_size; ++j) {
        if (mins[j] > it->v[j]) {
          mins[j] = it->v[j];
        }
        if (maxs[j] < it->v[j]) {
          maxs[j] = it->v[j];
        }
      }
    } else {
      mins = it->v;
      maxs = it->v;
    }
  }

  for (unsigned int i = 0; i < ga_fparams_size; ++i) {
    mins[i] -= extend_deltas;
    maxs[i] += extend_deltas;
  }

  if (us_log) {
    us_log->log(US_Vector::qs_vector2("ga_refine_limits: limits before",
                                      ga_low_fparams, ga_high_fparams));
  }

  ga_delta_to_fparams(mins, ga_low_fparams);
  ga_delta_to_fparams(maxs, ga_high_fparams);

  if (us_log) {
    us_log->log(US_Vector::qs_vector2("ga_refine_limits: limits after",
                                      ga_low_fparams, ga_high_fparams));
  }

  clip_limits(ga_low_fparams, ga_min_low_fparams, ga_max_high_fparams);
  clip_limits(ga_high_fparams, ga_min_low_fparams, ga_max_high_fparams);

  if (us_log) {
    us_log->log(US_Vector::qs_vector2("ga_refine_limits: limits after clipping",
                                      ga_low_fparams, ga_high_fparams));
  }

  ga_pop.clear();  // or recompute deltas for those that fall within (?)

  return ga_compute_delta(ga_points_max);
}

bool US_PM::ga_delta_to_fparams(vector<int>& delta, vector<double>& fparams) {
  for (unsigned int i = 0; i < ga_fparams_size; ++i) {
    fparams[i] = ga_low_fparams[i] + (double)delta[i] * ga_delta[i];
  }
  return true;
}

bool US_PM::ga_run(vector<int>& types, pm_ga_individual& best_individual,
                   unsigned int points_max, vector<double>& low_fparams,
                   vector<double>& high_fparams) {
  ga_types = types;
  ga_points_max = points_max;
  ga_low_fparams = low_fparams;
  ga_high_fparams = high_fparams;
  ga_fparams_size = low_fparams.size();
  ga_fparams.resize(ga_fparams_size);
  ga_points.resize(ga_fparams_size);

  pm_ga_pegged = false;
  pm_ga_peggedv.resize(ga_fparams_size);
  for (int i = 0; i < (int)ga_fparams_size; ++i) {
    pm_ga_peggedv[i] = false;
  }

  ga_delta_ok = false;

  if (!zero_params(ga_params, types)) {
    if (us_log) {
      us_log->log(error_msg);
    }
    return false;
  }

  if (!set_limits(ga_params, ga_min_low_fparams, ga_max_high_fparams)) {
    if (us_log) {
      us_log->log(error_msg);
    }
    return false;
  }

  clip_limits(ga_params, ga_min_low_fparams, ga_max_high_fparams);

  ga_p_save = ga_p;
  if (!ga_compute_delta(ga_points_max)) {
    return false;
  }

  ga_pop.clear();

  // possibly seed with best individual (from previous grid) matched to delta

  bool ret_stat = ga(best_individual);

  for (int i = 0; i < (int)ga_fparams_size; ++i) {
    pm_ga_peggedv[i] = best_individual.v[i] == 0 ||
                       best_individual.v[i] == (int)ga_points[i] - 1;
    if (pm_ga_peggedv[i]) {
      pm_ga_pegged = true;
    }
  }
  if (us_log) {
    us_log->log(US_Vector::qs_vector("best.v", best_individual.v));
    us_log->log(US_Vector::qs_vector("ga_points", ga_points));
  }

  if (pm_ga_pegged) {
    if (us_log) {
      us_log->log(US_Vector::qs_vector("pegged:", pm_ga_peggedv));
    }
  }
  ga_p = ga_p_save;
  ga_seed_params.clear();

  return ret_stat;
}

bool US_PM::ga_run(vector<int>& types, pm_ga_individual& best_individual,
                   unsigned int points) {
  if (us_log) {
    us_log->log(
        "Warning: ga_run() without limits can create unnecessarrily huge "
        "models (i.e. slow!)\n");
  }
  vector<double> low_fparams;
  vector<double> high_fparams;
  zero_params(ga_params, types);
  set_limits(ga_params, low_fparams, high_fparams);
  return ga_run(types, best_individual, points, low_fparams, high_fparams);
}

void US_PM::ga_set_params(unsigned int ga_population,
                          unsigned int ga_generations, double ga_mutate,
                          double ga_sa_mutate, double ga_crossover,
                          unsigned int ga_elitism,
                          unsigned int ga_early_termination) {
  this->ga_p.population = ga_population;
  this->ga_p.generations = ga_generations;
  this->ga_p.mutate = ga_mutate;
  this->ga_p.sa_mutate = ga_sa_mutate;
  this->ga_p.crossover = ga_crossover;
  this->ga_p.elitism = ga_elitism;
  this->ga_p.early_termination = ga_early_termination;
}

void US_PM::ga_set_params(map<QString, QString> control_parameters) {
  ga_set_params();
  if (control_parameters.count("ga_population")) {
    ga_p.population = control_parameters["ga_population"].toUInt();
  }
  if (control_parameters.count("ga_generations")) {
    ga_p.generations = control_parameters["ga_generations"].toUInt();
  }
  if (control_parameters.count("ga_p.mutate")) {
    ga_p.mutate = control_parameters["ga_mutate"].toDouble();
  }
  if (control_parameters.count("ga_p.sa_mutate")) {
    ga_p.sa_mutate = control_parameters["ga_sa_mutate"].toDouble();
  }
  if (control_parameters.count("ga_crossover")) {
    ga_p.crossover = control_parameters["ga_crossover"].toDouble();
  }
  if (control_parameters.count("ga_elitism")) {
    ga_p.elitism = control_parameters["ga_elitism"].toUInt();
  }
  if (control_parameters.count("ga_early_termination")) {
    ga_p.early_termination =
        control_parameters["ga_early_termination"].toUInt();
  }
}

QString US_PM::ga_info() {
  QString qs;

  qs += QString("ga_p.population %1\n").arg(ga_p.population);
  qs += QString("ga_p.generations %1\n").arg(ga_p.generations);
  qs += QString("ga_p.mutate %1\n").arg(ga_p.mutate);
  qs += QString("ga_p.sa_mutate %1\n").arg(ga_p.sa_mutate);
  qs += QString("ga_p.crossover %1\n").arg(ga_p.crossover);
  qs += QString("ga_p.elitism %1\n").arg(ga_p.elitism);
  qs += QString("ga_p.early_termination %1\n").arg(ga_p.early_termination);

  qs += US_Vector::qs_vector("ga_points", ga_points);
  qs += US_Vector::qs_vector("ga_types", ga_types);
  qs += US_Vector::qs_vector("ga_delta", ga_delta);
  qs += US_Vector::qs_vector2("ga_(low/high)_fparams", ga_low_fparams,
                              ga_high_fparams);
  qs += US_Vector::qs_vector2("ga_(min_low/max_high)_fparams",
                              ga_min_low_fparams, ga_max_high_fparams);

  return qs;
}

void US_PM::random_normal(double mean1, double sd1, double mean2, double sd2,
                          double& x, double& y) {
  double s;
  do {
    x = 2e0 * drand48() - 1e0;
    y = 2e0 * drand48() - 1e0;
    s = x * x + y * y;
  } while (s > 1e0);
  s = sqrt(-2 * log(s) / s);
  x *= s * sd1;
  y *= s * sd2;
  x += mean1;
  y += mean2;
  return;
}

bool US_PM::ga_params_to_individual(vector<double>& params,
                                    pm_ga_individual& individual) {
  vector<int> types;
  vector<double> fparams;

  if (!split(params, types, fparams)) {
    return false;
  }
  return ga_fparams_to_individual(fparams, individual);
}

bool US_PM::ga_fparams_to_individual(vector<double>& fparams,
                                     pm_ga_individual& individual) {
  if (!ga_delta_ok) {
    error_msg = "Error: ga_fparams_to_individual: ga_compute_delta not ok";
    return false;
  }

  if (ga_fparams_size != fparams.size()) {
    error_msg = QString(
                    "Error: ga_fparams_to_individual: preset ga_fparams_size "
                    "%1 is not equal fparams.size() %2")
                    .arg(ga_fparams_size)
                    .arg(fparams.size());
    return false;
  }

  individual.v.resize(ga_fparams_size);

  for (int i = 0; i < (int)ga_fparams_size; ++i) {
    if (fparams[i] <= ga_low_fparams[i] || !ga_delta[i]) {
      individual.v[i] = 0;
      continue;
    }
    if (fparams[i] >= ga_high_fparams[i]) {
      individual.v[i] = ga_points[i] - 1;
      continue;
    }
    individual.v[i] =
        (int)(5e-1 + (fparams[i] - ga_low_fparams[i]) / ga_delta[i]);
    if (individual.v[i] < 0) {
      individual.v[i] = 0;
      continue;
    }
    if (individual.v[i] > (int)ga_points[i] - 1) {
      individual.v[i] = ga_points[i] - 1;
      continue;
    }
  }

  {
    vector<double> gridded_fparams(ga_fparams_size);
    ga_delta_to_fparams(individual.v, gridded_fparams);
    if (us_log) {
      us_log->log(
          US_Vector::qs_vector2("ga_fparams_to_individual: org vs gridded",
                                fparams, gridded_fparams));
    }
  }

  return true;
}
