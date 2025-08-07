#include "../include/us_pm.h"

bool US_PM::best_sphere(vector<double>& params, vector<double>& low_fparams,
                        vector<double>& high_fparams, set<pm_point>& model) {
  // start with 1 sphere, slowly increase size until fitness drops
  params.resize(2);
  params[0] = 0e0;

  use_CYJ = false;

  us_timers.clear_timers();

  double best_fit;
  // us_timers.init_timer( "sphere try with delta fallback calling vary" );
  // us_timers.start_timer( "sphere try with delta fallback calling vary" );
  bool ok = best_vary_one_param(1, params, low_fparams, high_fparams, model,
                                best_fit);
  // us_timers.end_timer( "sphere try with delta fallback calling vary" );
  // cout << us_timers.list_times();
  if (ok) {
    last_best_rmsd_ok = true;
    last_best_rmsd2 = best_fit;
    return true;
  } else {
    last_best_rmsd_ok = false;
    return false;
  }

  return true;
}

bool US_PM::best_sphere(set<pm_point>& model) {
  // start with 1 sphere, slowly increase size until fitness drops
  vector<double> params(2);
  params[0] = 0e0;

  use_CYJ = false;

  us_timers.clear_timers();

  double best_fit;
  // us_timers.init_timer( "sphere try with delta fallback calling vary" );
  // us_timers.start_timer( "sphere try with delta fallback calling vary" );
  bool ok = best_vary_one_param(1, params, model, best_fit);
  // us_timers.end_timer( "sphere try with delta fallback calling vary" );
  // cout << us_timers.list_times();
  if (ok) {
    last_best_rmsd_ok = true;
    last_best_rmsd2 = best_fit;
    return true;
  } else {
    last_best_rmsd_ok = false;
    return false;
  }

  return true;
}

#if defined(USPM_BEST_SPHERE_OLD_WAY)

if (0) {
  us_timers.init_timer("sphere try with CA");
  for (params[1] = 0.1; params[1] <= max_dimension_d; params[1] += delta) {
    create_model(params, this_model);
    compute_I(this_model, I_result);
    this_fit = fitness2(I_result);
    if (us_log) {
      us_log->log(QString("radius %1 beads %2 fitness %3\n")
                      .arg(params[1])
                      .arg(model.size())
                      .arg(this_fit)
                      .toLatin1()
                      .data());
    }
    if (this_fit < prev_fit) {
      best_fit = this_fit;
      model = this_model;
    }
    if (this_fit > prev_fit) {
      break;
    }
    prev_fit = this_fit;
  }
  us_timers.end_timer("sphere try with CA");
}

if (0) {
  us_timers.init_timer("sphere try with delta fallback");
  us_timers.start_timer("sphere try with delta fallback");
  double low_limit = 0.1;
  double high_limit = max_dimension_d;
  double prev_size;
  double best_size;
  delta = 1e0;

  set<pm_point> prev_model;
  vector<vector<complex<float> > > Av;

  map<double, double> fitnesses;
  while (delta > min_delta) {
    double last_fitness_1_pos = -1e0;
    double last_fitness_2_pos = -1e0;
    double last_fitness_3_pos = -1e0;
    prev_fit = 1e99;

    prev_model.clear();
    Av.clear();

    if (us_log) {
      us_log->log(QString("this limit %1 %2 delta %3\n")
                      .arg(low_limit)
                      .arg(high_limit)
                      .arg(delta));
    }
    for (params[1] = low_limit; params[1] <= high_limit; params[1] += delta) {
      bool skip = false;
      create_model(params, this_model);
      if (prev_model.size() != this_model.size()) {
        compute_delta_I(this_model, prev_model, Av, I_result);
        this_fit = fitness2(I_result);
        fitnesses[params[1]] = this_fit;
      } else {
        skip = true;
      }
      if (!skip) {
        if (us_log) {
          us_log->log(QString("radius %1 beads %2 fitness %3\n")
                          .arg(params[1])
                          .arg(model.size())
                          .arg(this_fit, 0, 'g', 8)
                          .toLatin1()
                          .data());
        }

        last_fitness_3_pos = last_fitness_2_pos;
        last_fitness_2_pos = last_fitness_1_pos;
        last_fitness_1_pos = params[1];

        if (this_fit < prev_fit) {
          best_fit = this_fit;
          best_size = params[1];
          model = this_model;
        }
        if (this_fit > prev_fit) {
          break;
        }
        prev_fit = this_fit;
        prev_size = params[1];
      }
      prev_model = this_model;
    }
    low_limit = last_fitness_3_pos;
    high_limit = last_fitness_1_pos;
    delta /= 10e0;
  }
  us_timers.end_timer("sphere try with delta fallback");
}

if (0) {
  us_timers.init_timer("sphere try with CA fallback");
  us_timers.start_timer("sphere try with CA fallback");
  double low_limit = 0.1;
  double high_limit = max_dimension_d;
  double min_delta = 1e-3;
  double prev_size;
  double best_size;
  delta = 1e0;

  set<pm_point> prev_model;

  map<double, double> fitnesses;
  while (delta > min_delta) {
    double last_fitness_1_pos = -1e0;
    double last_fitness_2_pos = -1e0;
    double last_fitness_3_pos = -1e0;
    prev_fit = 1e99;

    prev_model.clear();
    if (us_log) {
      us_log->log(QString("this limit %1 %2 delta %3\n")
                      .arg(low_limit)
                      .arg(high_limit)
                      .arg(delta));
    }
    for (params[1] = low_limit; params[1] <= high_limit; params[1] += delta) {
      bool skip = false;
      if (fitnesses.count(params[1])) {
        this_fit = fitnesses[params[1]];
      } else {
        create_model(params, this_model);
        if (prev_model.size() != this_model.size()) {
          compute_I(this_model, I_result);
          this_fit = fitness2(I_result);
          fitnesses[params[1]] = this_fit;
        } else {
          skip = true;
          if (us_log) {
            us_log->log(QString("skipped radius %1 beads %2 fitness %3\n")
                            .arg(params[1])
                            .arg(model.size())
                            .arg(this_fit, 0, 'g', 8)
                            .toLatin1()
                            .data());
          }
        }
      }
      if (!skip) {
        if (us_log) {
          us_log->log(QString("radius %1 beads %2 fitness %3\n")
                          .arg(params[1])
                          .arg(model.size())
                          .arg(this_fit, 0, 'g', 8)
                          .toLatin1()
                          .data());
        }

        last_fitness_3_pos = last_fitness_2_pos;
        last_fitness_2_pos = last_fitness_1_pos;
        last_fitness_1_pos = params[1];

        if (this_fit < prev_fit) {
          best_fit = this_fit;
          best_size = params[1];
          model = this_model;
        }
        if (this_fit > prev_fit) {
          break;
        }
        prev_fit = this_fit;
        prev_size = params[1];
      }
      prev_model = this_model;
    }
    low_limit = last_fitness_3_pos;
    high_limit = last_fitness_1_pos;
    delta /= 10e0;
  }
  us_timers.end_timer("sphere try with CA fallback");
}

if (0) {
  us_timers.init_timer("sphere try with CA bisection");
  // find initial range
  prev_fit = 1e99;

  double prev_size = 1e-1;
  double left_size = 1e-1;
  double left_fit = 1e99;
  double center_size = 1e-1;
  double center_fit = 1e99;
  double right_size = -1e0;
  double right_fit = 1e99;

  for (params[1] = 0.1; params[1] <= max_dimension_d; params[1] += 2e0) {
    this_fit = model_fit(params, model, I_result);
    if (us_log) {
      us_log->log(QString("radius %1 beads %2 fitness %3\n")
                      .arg(params[1])
                      .arg(model.size())
                      .arg(this_fit)
                      .toLatin1()
                      .data());
    }
    if (this_fit < prev_fit) {
      left_size = prev_size;
      left_fit = prev_fit;
    }
    if (this_fit > prev_fit) {
      center_size = prev_size;
      center_fit = prev_fit;
      right_size = params[1];
      right_fit = this_fit;
      break;
    }
    prev_fit = this_fit;
    prev_size = params[1];
  }
  while (right_size - left_size > delta) {
    if (us_log) {
      us_log->log(QString("bisection bracket %1 %2 %3 delta %4\n")
                      .arg(left_size)
                      .arg(center_size)
                      .arg(right_size)
                      .arg(delta));
    }

    // compute parabolic fit

    double center_minus_left_size = center_size - left_size;
    double center_minus_right_size = center_size - right_size;
    double center_minus_left_fit = center_fit - left_fit;
    double center_minus_right_fit = center_fit - right_fit;

    double trial_size =
        center_size - 5e-1 *
                          (center_minus_left_size * center_minus_left_size *
                               center_minus_right_fit -
                           center_minus_right_size * center_minus_right_size *
                               center_minus_left_fit) /
                          (center_minus_left_size * center_minus_right_fit -
                           center_minus_right_size * center_minus_left_fit);

    params[1] = trial_size;
    double trial_fit = model_fit(params, model, I_result);
    if (us_log) {
      us_log->log(QString("radius %1 beads %2 fitness %3\n")
                      .arg(params[1])
                      .arg(model.size())
                      .arg(trial_fit)
                      .toLatin1()
                      .data());
    }

    if (trial_size < center_size) {
      right_fit = center_fit;
      right_size = center_size;
    } else {
      left_fit = center_fit;
      left_size = center_size;
    }
    center_size = trial_size;
    center_fit = trial_fit;
    if (us_log) {
      us_log->log(QString("end bisection bracket %1 %2 %3 delta %4\n")
                      .arg(left_size)
                      .arg(center_size)
                      .arg(right_size)
                      .arg(delta));
    }
  }

  us_timers.end_timer("sphere try with CA bisection");
}

if (0) {
  us_timers.init_timer("sphere try with CA coarse, bisection");
  us_timers.start_timer("sphere try with Delta");
  {
    prev_fit = 1e99;
    best_fit = 1e99;
    set<pm_point> prev_model;
    vector<vector<complex<float> > > Av;

    for (params[1] = 0.1; params[1] <= max_dimension_d; params[1] += delta) {
      create_model(params, this_model);
      compute_delta_I(this_model, prev_model, Av, I_result);
      prev_model = this_model;
      this_fit = fitness2(I_result);
      if (us_log) {
        us_log->log(QString("radius %1 beads %2 fitness %3\n")
                        .arg(params[1])
                        .arg(model.size())
                        .arg(this_fit)
                        .toLatin1()
                        .data());
      }
      if (this_fit < prev_fit) {
        best_fit = this_fit;
        model = this_model;
      }
      if (this_fit > prev_fit) {
        break;
      }
      prev_fit = this_fit;
    }
  }
  us_timers.end_timer("sphere try with Delta");
}

if (us_log) {
  us_log->log(us_timers.list_times());
}

return true;
}
#endif
