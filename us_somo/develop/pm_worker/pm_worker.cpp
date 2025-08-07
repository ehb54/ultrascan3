#include "pm_worker.h"

void PM_WORKER::set_grid_size(double grid_conversion_factor, bool quiet) {
  // be careful with this routine!
  // have to clear because any rtp data is now invalid
  clear();
  this->grid_conversion_factor = grid_conversion_factor;
  one_over_grid_conversion_factor = 1e0 / grid_conversion_factor;
  cube_size =
      grid_conversion_factor * grid_conversion_factor * grid_conversion_factor;

  // radius gives a sphere with equal size to the cube:
  bead_radius = pow(cube_size / M_PI, 1e0 / 3e0);
  bead_radius_over_2gcf = (bead_radius * 5e-1) / grid_conversion_factor;

  if (!quiet) {
    cout << "US_PM:cube size   " << cube_size << endl
         << "US_PM:bead radius " << bead_radius << endl;
  }

  double conv_F = cube_size / org_cube_size;
  for (int i = 0; i < (int)F.size(); ++i) {
    F[i] = org_F[i] * conv_F;
  }
  max_dimension = org_max_dimension / grid_conversion_factor;
}

void PM_WORKER::reset_grid_size(bool quiet) {
  set_grid_size(org_conversion_factor, quiet);
}

PM_WORKER::PM_WORKER(double grid_conversion_factor, int max_dimension,
                     unsigned int max_harmonics, vector<double> F,
                     vector<double> q, vector<double> I, vector<double> e,
                     unsigned int max_mem_in_MB, int debug_level, bool quiet) {
  this->max_dimension = abs(max_dimension);
  this->max_harmonics = max_harmonics;
  this->F = F;
  this->q = q;
  this->I = I;
  this->e = e;
  this->max_mem_in_MB = max_mem_in_MB;
  this->debug_level = debug_level;

  this->max_dimension =
      this->max_dimension < USPM_MAX_VAL ? this->max_dimension : USPM_MAX_VAL;
  if (this->max_dimension != max_dimension) {
    cerr << "Warning: maximum dimension requested " << max_dimension
         << "exceedes maximum allowed " << this->max_dimension
         << "reset to maximum allowed." << endl;
  }

  org_F = F;
  org_conversion_factor = grid_conversion_factor;
  org_cube_size =
      grid_conversion_factor * grid_conversion_factor * grid_conversion_factor;
  org_max_dimension = max_dimension;
  set_grid_size(grid_conversion_factor, quiet);

  max_dimension_d = (double)this->max_dimension;

  q_points = (unsigned int)q.size();

  Z0 = complex<float>(0.0f, 0.0f);
  i_ = complex<float>(0.0f, 1.0f);

  i_l.resize(max_harmonics + 1);
  for (unsigned int l = 0; l <= max_harmonics; l++) {
    i_l[l] = pow(i_, l);
  }

  Y_points = max_harmonics + 1 + (max_harmonics) * (max_harmonics + 1);
  J_points = (1 + max_harmonics) * q_points;
  i_k.clear();
  for (unsigned int l = 0; l <= max_harmonics; ++l) {
    for (int m = -(int)l; m <= (int)l; m++) {
      i_k.push_back(i_l[l]);
    }
  }

  for (unsigned int k = 0; k < Y_points; k++) {
    A0.push_back(Z0);
  }
  Av0.resize(q_points);
  I0.resize(q_points);
  for (unsigned int i = 0; i < q_points; ++i) {
    Av0[i] = A0;
    I0[i] = 0e0;
  }
  no_harmonics = max_harmonics + 1;

  q_Y_points = q_points * Y_points;

  for (unsigned int i = 0; i < q_Y_points; ++i) {
    A1v0.push_back(Z0);
  }
  if (!quiet) {
    cout << "q_Y_points " << q_Y_points << endl
         << "A1v0 size  " << A1v0.size() << endl;
  }

  ccY.resize(Y_points);
  ccJ.resize(J_points);
  ccA1v = A1v0;

  use_errors = e.size() == q.size();

  if (use_errors) {
    bool any_non_zero = false;
    for (unsigned int i = 0; i < e.size(); i++) {
      if (e[i]) {
        any_non_zero = true;
        break;
      }
    }
    for (unsigned int i = 0; i < e.size(); i++) {
      if (!e[i]) {
        use_errors = false;
        break;
      }
    }
    if (!quiet && !use_errors && any_non_zero) {
      cout << "Notice: SD's provided but some were zero, so SD fitting is "
              "turned off"
           << endl;
    }
  }

  if (use_errors) {
    oneoversd2.resize(q_points);
    for (unsigned int i = 0; i < q_points; ++i) {
      oneoversd2[i] = 1e0 / (e[i] * e[i]);
    }
  } else {
    oneoversd2.clear();
  }

  // memory computations

  bytes_per_pm_data = sizeof(pm_data) + J_points * sizeof(double) +
                      Y_points * sizeof(float) * 2;

  bytes_per_pmc_data = sizeof(pmc_data) + q_Y_points * sizeof(float) * 2;

  unsigned int base_mem = 20;

  max_beads_CYJ =
      (1024 * 1024 * (max_mem_in_MB - base_mem)) / bytes_per_pm_data;
  max_beads_CA =
      (1024 * 1024 * (max_mem_in_MB - base_mem)) / bytes_per_pmc_data;

  if (!quiet) {
    cout << "bytes per pm_data " << bytes_per_pm_data << endl;
    cout << "bytes per pmc data " << bytes_per_pmc_data << endl;

    cout << "Memory max " << max_mem_in_MB << endl;
    cout << "Memory available " << (max_mem_in_MB - base_mem) << endl;
    cout << "max beads CYJ " << max_beads_CYJ << endl;
    cout << "max beads CA " << max_beads_CA << endl;
  }

  use_CYJ = false;

  init_objects();

  shs = (SHS_USE *)0;
  shs = new SHS_USE(max_harmonics);
}

PM_WORKER::~PM_WORKER() {
  if (shs) {
    delete shs;
  }
}

void PM_WORKER::clear() {
  pcdata.clear();
  pdata.clear();
}

bool PM_WORKER::create_model(vector<double> params, set<pm_point> &model,
                             bool only_last_model) {
  model.clear();

  vector<double> params_left;

  int model_pos = 0;

  while (params.size()) {
    if (only_last_model) {
      model.clear();
    }
    if (!create_1_model(model_pos, params, params_left, model)) {
      return false;
    }
    params = params_left;
    ++model_pos;
  }
  return true;
}

set<pm_point> PM_WORKER::recenter(set<pm_point> &model) {
  int cx = 0;
  int cy = 0;
  int cz = 0;

  for (set<pm_point>::iterator it = model.begin(); it != model.end(); it++) {
    cx += it->x[0];
    cy += it->x[1];
    cz += it->x[2];
  }

  cx /= model.size();
  cy /= model.size();
  cz /= model.size();

  set<pm_point> result;

  pm_point pmp;

  for (set<pm_point>::iterator it = model.begin(); it != model.end(); it++) {
    pmp.x[0] = it->x[0] - (int16_t)cx;
    pmp.x[1] = it->x[1] - (int16_t)cy;
    pmp.x[2] = it->x[2] - (int16_t)cz;
    result.insert(pmp);
  }
  return result;
}

bool PM_WORKER::rotation_matrix(double l, double m, double n, double theta,
                                vector<vector<double> > &rm) {
  rm.clear();
  rm.resize(3);
  rm[0].resize(3);
  rm[1].resize(3);
  rm[2].resize(3);

  double c = cos(theta);
  double s = sin(theta);
  double omc = 1e0 - c;

  rm[0][0] = l * l * omc + c;
  rm[0][1] = m * l * omc - n * s;
  rm[0][2] = n * l * omc + m * s;

  rm[1][0] = l * m * omc + n * s;
  rm[1][1] = m * m * omc + c;
  rm[1][2] = n * m * omc - l * s;

  rm[2][0] = l * n * omc - m * s;
  rm[2][1] = m * n * omc + l * s;
  rm[2][2] = n * n * omc + c;
  return true;
}

bool PM_WORKER::apply_rotation_matrix(vector<vector<double> > &rm, int x, int y,
                                      int z, double &newx, double &newy,
                                      double &newz) {
  newx = (double)x * rm[0][0] + (double)y * rm[0][1] + (double)z * rm[0][2];
  newy = (double)x * rm[1][0] + (double)y * rm[1][1] + (double)z * rm[1][2];
  newz = (double)x * rm[2][0] + (double)y * rm[2][1] + (double)z * rm[2][2];

  return true;
}

bool PM_WORKER::split(vector<double> &params, vector<int> &types,
                      vector<double> &fparams) {
  types.clear();
  fparams.clear();

  for (int i = 0; i < (int)params.size();) {
    types.push_back(params[i++]);
    if ((int)params.size() <= i) {
      error_msg = "split: no params for type";
      return false;
    }
    if ((int)object_parameter_types.size() < types.back()) {
      error_msg = "split: unknown object type";
      return false;
    }
    int pos =
        (int)types.size() - 1 < (int)object_parameter_types[types.back()].size()
            ? types.size() - 1
            : object_parameter_types[types.back()].size() - 1;
    if (params.size() < object_parameter_types[types.back()][pos].size()) {
      error_msg = "split: error insufficient params for type";
      return false;
    }
    for (int k = 0; k < (int)object_parameter_types[types.back()][pos].size();
         ++k) {
      fparams.push_back(params[i++]);
    }
  }
  return true;
}

bool PM_WORKER::join(vector<double> &params, vector<int> &types,
                     vector<double> &fparams) {
  params.clear();

  int fpos = 0;
  for (int i = 0; i < (int)types.size(); ++i) {
    params.push_back(types[i]);
    if ((int)object_parameter_types.size() < types[i]) {
      error_msg = "split: unknown object type";
      return false;
    }
    int pos = i < (int)object_parameter_types[types[i]].size()
                  ? i
                  : object_parameter_types[types[i]].size() - 1;
    if ((int)fparams.size() <
        fpos + (int)object_parameter_types[types[i]][pos].size()) {
      error_msg = "join: error insufficient params for type";
      return false;
    }
    for (int k = 0; k < (int)object_parameter_types[types[i]][pos].size();
         ++k, ++fpos) {
      params.push_back(fparams[fpos]);
    }
  }
  return true;
}
