#include "../include/us_pm.h"

bool US_PM::best_cylinder(vector<double>& params, vector<double>& low_fparams,
                          vector<double>& high_fparams, set<pm_point>& model) {
  // start with min radius, delta to find optimal length, quit when fitness get

  params.resize(3);
  params[0] = 1e0;

  double best_fit;
  if (best_vary_two_param(1, 2, params, low_fparams, high_fparams, model,
                          best_fit)) {
    last_best_rmsd_ok = true;
    last_best_rmsd2 = best_fit;
    return true;
  } else {
    last_best_rmsd_ok = false;
    return false;
  }
}

bool US_PM::best_cylinder(set<pm_point>& model) {
  // start with min radius, delta to find optimal length, quit when fitness get

  vector<double> params(3);
  params[0] = 1e0;

  double best_fit;
  if (best_vary_two_param(1, 2, params, model, best_fit)) {
    last_best_rmsd_ok = true;
    last_best_rmsd2 = best_fit;
    return true;
  } else {
    last_best_rmsd_ok = false;
    return false;
  }
}

#if defined(USPM_OLD_WAY)

set<pm_point> this_model;
use_CYJ = false;

us_timers.clear_timers();

us_timers.init_timer("cylinder try with delta fallback");
us_timers.start_timer("cylinder try with delta fallback");

double delta = best_delta_start;
double low_limit = best_delta_size_min;
double high_limit = best_delta_size_max;

double prev_size;
double best_size;

double best_fit = 1e99;
double prev_fit = 1e99;
double this_fit;

map<double, double> fitnesses;
while (delta > best_delta_min) {
  double last_fitness_1_pos = -1e0;
  double last_fitness_2_pos = -1e0;
  double last_fitness_3_pos = -1e0;
  prev_fit = 1e99;

  if (us_log) {
    us_log->log(QString("this limit %1 %2 delta %3\n")
                    .arg(low_limit)
                    .arg(high_limit)
                    .arg(delta));
  }
  for (params[2] = low_limit; params[2] <= high_limit; params[2] += delta) {
    best_vary_one_param(1, params, this_model, this_fit);
    fitnesses[params[2]] = this_fit;

    last_fitness_3_pos = last_fitness_2_pos;
    last_fitness_2_pos = last_fitness_1_pos;
    last_fitness_1_pos = params[2];

    if (this_fit < prev_fit) {
      best_fit = this_fit;
      best_size = params[2];
      model = this_model;
    }
    if (this_fit > prev_fit) {
      break;
    }
    prev_fit = this_fit;
    prev_size = params[2];
  }
  low_limit = last_fitness_3_pos;
  high_limit = last_fitness_1_pos;
  delta /= 10e0;
}
us_timers.end_timer("cylinder try with delta fallback");

if (us_log) {
  us_log->log(us_timers.list_times());
}

return true;
}
#endif
