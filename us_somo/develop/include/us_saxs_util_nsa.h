#ifndef US_SAXS_UTIL_NSA_H
#define US_SAXS_UTIL_NSA_H

class nsa_ga_individual {
 public:
  vector<double> v;
  double fitness;
  bool operator<(const nsa_ga_individual& objIn) const {
    return fitness < objIn.fitness;
  }

  bool operator==(const nsa_ga_individual& objIn) const { return v == objIn.v; }
  bool operator!=(const nsa_ga_individual& objIn) const { return v != objIn.v; }
};

class nsa_sga_individual {
 public:
  // bit string
  // bits determine coordinate on or off

  vector<char> v;

  bool fitness_ok;
  double fitness;

  void unique();

  static nsa_sga_individual random(unsigned int size /* in bytes */);

  vector<sgp_sphere> sgp_spheres();
  QString qs_bead_model();
  vector<PDB_atom> bead_model();

  bool operator<(const nsa_sga_individual& objIn) const {
    return fitness < objIn.fitness;
  }

  bool operator==(const nsa_sga_individual& objIn) const {
    return v == objIn.v;
  }
  bool operator!=(const nsa_sga_individual& objIn) const {
    return v != objIn.v;
  }
};

#endif
