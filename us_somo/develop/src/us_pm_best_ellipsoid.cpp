#include "../include/us_pm.h"

bool US_PM::best_ellipsoid(vector<double>& /* params */,
                           vector<double>& /* low_fparams */,
                           vector<double>& /* high_fparams */,
                           set<pm_point>& /* model */
) {
  return false;
}

bool US_PM::best_ellipsoid(set<pm_point>& /* model */) { return false; }
