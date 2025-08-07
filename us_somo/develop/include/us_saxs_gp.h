#ifndef US_SAXS_GP_H
#define US_SAXS_GP_H

#include <list>

#include "us_hydrodyn_pdbdefs.h"

// structs/classes for saxs bead GP

// notable control_parameters:
// "distancequantum" : distance quantum in angstrom
//  all distances discretize to this
//  all sphere sizes discretize to this

#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(disable : 4251)
#endif
#endif

extern map<QString, double> sgp_params;

class sgp_sphere {
 public:
  point coordinate;
  double radius;
};

// the basic tree node:

class sgp_node {
 public:
  sgp_node();
  sgp_node(point normal, unsigned int distance, unsigned int radius);
  ~sgp_node();

  sgp_node *parent;
  list<sgp_node *> children;

  point normal;

  // these are scaled via control_parameters[ "sbgpquantum" ]:

  unsigned int distance;
  unsigned int radius;

  // utility fields not maintained internally:
  // could be kept separately, as they really only belong to the root node of an
  // individual and would save a a bit of wasted memory

  double fitness;
  bool fitness_ok;
  bool is_dead;

  // end utility fields

  QString contents(bool include_children = true);
  QString contents_by_pos();
  unsigned int size();
  unsigned int depth();
  sgp_node *ref(unsigned int pos);
  sgp_node *copy(sgp_node *node);
  bool insert_copy(unsigned int pos, sgp_node *node);

  sgp_node *random(unsigned int size);

  void test();
  sgp_node *root();
  point checksum();

  vector<sgp_sphere> sgp_spheres();  // minimal structure for now
  vector<PDB_atom> bead_model();
  QString qs_bead_model();

  point get_coordinate();

  bool mutate(unsigned int pos = 0);
  bool crossover(sgp_node *&result, sgp_node *node1, sgp_node *node2);

  static void random_normal(double &r1, double &r2, double mean1, double sd1,
                            double mean2, double sd2);

  static QString validate_params();
  static QString default_params();
  static QString list_params();
  static QString usage();

  void check_normal(QString qs = "");

 private:
  point cross(point p1, point p2);
  float dot(point p1, point p2);
  point plus(point p1, point p2);
  point norm(point p1);
  point scale(point p, float m);
};

class sortable_sgp_node {
 public:
  sgp_node *node;
  bool operator<(const sortable_sgp_node &objIn) const {
    return node->fitness < objIn.node->fitness;
  }
};

#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(default : 4251)
#endif
#endif

#endif
