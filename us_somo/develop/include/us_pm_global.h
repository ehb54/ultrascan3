#ifndef US_PM_GLOBAL_H
#define US_PM_GLOBAL_H

#define US_PM_MAX_PMTYPE 4

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_4PI
#define M_4PI (4e0 * 3.14159265358979323846)
#endif

#ifndef M_ONE_OVER_4PI
#define M_ONE_OVER_4PI (1e0 / (4e0 * 3.14159265358979323846))
#endif

// based upon int16 right now
#define USPM_MAX_VAL 32767
#define USPM_MAX_VAL_D ((double)USPM_MAX_VAL)
#define USPM_MIN_VAL -32767
#define USPM_MIN_VAL_D ((double)USPM_MIN_VAL)

#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(disable : 4251)
#endif
#endif

class pm_point {
 public:
  int16_t x[3];
  bool operator<(const pm_point& objIn) const {
    return x[0] < objIn.x[0] || (x[0] == objIn.x[0] && x[1] < objIn.x[1]) ||
           (x[0] == objIn.x[0] && x[1] == objIn.x[1] && x[2] < objIn.x[2]);
  }

  bool operator==(const pm_point& objIn) const {
    return x[0] == objIn.x[0] && x[1] == objIn.x[1] && x[2] == objIn.x[2];
  }
};

class pm_point_f {
 public:
  float x[3];
  bool operator<(const pm_point_f& objIn) const {
    return x[0] < objIn.x[0] || (x[0] == objIn.x[0] && x[1] < objIn.x[1]) ||
           (x[0] == objIn.x[0] && x[1] == objIn.x[1] && x[2] < objIn.x[2]);
  }
};

struct pm_data {
  float x[3];  // converted to grid
  float rtp[3];
  bool no_J;
  bool no_Y;
  vector<double> J;
  vector<complex<float> > Y;
};

struct pmc_data {
  float x[3];  // converted to grid
  float rtp[3];
  vector<complex<float> > A1v;
};

class pm_ga_individual {
 public:
  vector<int> v;
  set<pm_point> model;
  bool fitness_computed;
  double fitness;
  bool operator<(const pm_ga_individual& objIn) const {
    return fitness < objIn.fitness;
  }

  bool operator==(const pm_ga_individual& objIn) const {
    return v == objIn.v || model == objIn.model;
  }
  bool operator!=(const pm_ga_individual& objIn) const { return v != objIn.v; }
};

struct ga_ctl_param {
  unsigned int population;
  unsigned int generations;
  double mutate;
  double sa_mutate;
  double crossover;
  unsigned int elitism;
  unsigned int early_termination;
  bool full_grid;
};

#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(default : 4251)
#endif
#endif

#define LE_OR_LT <

#endif
