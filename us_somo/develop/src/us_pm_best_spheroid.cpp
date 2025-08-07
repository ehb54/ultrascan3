#include "../include/us_pm.h"

bool US_PM::best_spheroid(vector<double>& params, vector<double>& low_fparams,
                          vector<double>& high_fparams, set<pm_point>& model) {
  // start with min radius, delta to find optimal length, quit when fitness get

  params.resize(3);
  params[0] = 2e0;

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

bool US_PM::best_spheroid(set<pm_point>& model) {
  // start with min radius, delta to find optimal length, quit when fitness get

  vector<double> params(3);
  params[0] = 2e0;

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
