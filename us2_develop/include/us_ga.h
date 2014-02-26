#ifndef US_GA_H
#define US_GA_H

#include "../include/us_fe_nnls_t.h"

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_BR 4 /* the maximum number of arguments per function element, equiv. the max branching of the tree */

typedef float value;

// #define VALUE_MAX MAXINT
#define VALUE_MAX MAXFLOAT
// #define VALUE_MAX MAXDOUBLE
// #define VALUE_MAX MAXLONG
// #define VALUE_MAX MAXSHORT


typedef struct {
  int id;
  int argc;
  int init_weight;
  int weight_sum;
  int control; // if non-zero, this is an evaluation control structure
  int rooted;  // if non-zero, only allow at root
  value (*func)(value *, void *); // the function with the node as the second argument
  void (*init)(void *);  // initialization function, note void * is really a node, but we haven't defined one yet,
                         // can alloc data for each node at node->data
  void (*node_mutate)(void *);  // node mutation function, note void * is really a node, but we haven't defined one yet,
                                // the idea is to mutate the nodes' data space
  unsigned int (*hash)(void *);  // hash the data space
  void (*fprint_params)(FILE *, void *);  // display function for data params of element node
  const char *name;
} element;

#define CTL_NONE                0
#define CTL_WHILETERMINALS      1
#define CTL_WHILETERMINALSWRITE 2

typedef struct _node {
  element *e;
  //  struct _node *parent; do we need this?
  struct _node *children[MAX_BR];
  int argv[MAX_BR];
  int pos;
  void *data;  // data for initialization function, freed upon destruct
  int data_size; // size of data inited, for memcpy in case of node copy
} node;

typedef struct _population {
  struct _population *next;
  struct _population *prev;
  node *root;
  int points;
  int active_points;
  node **nodes;
  value fitness;
  char fitness_valid;
  char no_gsm_improvement;
  void *data;
} population;


extern char *cachedir;
value evaluate(node *, value *);
extern value pass_fit_args[];
extern int pop_data_size;

void list_tree(node *);
extern int solute_rounding;
#define DEFAULT_SOLUTE_ROUNDING 3
#define SOLUTE_DATA_SIZE ((7 * sizeof(double)) + 2 * sizeof(char))
#define SOLUTE_DATA_ACTIVE_OFS (SOLUTE_DATA_SIZE - 2)
#define SOLUTE_DATA_PROX_FLAG (SOLUTE_DATA_SIZE - 1)
#define SOLUTE_DATA_CONCENTRATION 6

extern double min_f_ratio;
extern double max_f_ratio;
extern double use_vbar;
extern double baseline_adder;
extern double time_correction;

void ga_setup(struct ga_data GA_Params, int myrank, US_fe_nnls_t *us_fe_nnls_t);
extern vector <Simulation_values> ga_last_sve;
extern vector <vector <Simulation_values> > ga_best_sve;
extern vector <float> experiment_weights;

extern int debug_level;
extern int this_rank;
extern US_fe_nnls_t *our_us_fe_nnls_t;
extern unsigned long total_fitness_time;
#define SOLUTE_CONCENTRATION_THRESHOLD .000001
extern double regularization_factor;
extern int regularize_on_RMSD;
extern int ga_mw;
extern int ga_sc;
extern double s_rounding;

// rounding 

// NOTICE: these need to be dealt with or simply replace the mechanism
// to round to x significant digits regardless of magnitude

#define ROUNDING_VBAR             1
#define ROUNDING_MW               100
#define ROUNDING_S                1e-13
#define ROUNDING_D                1e-7
#define ROUNDING_SIGMA            1
#define ROUNDING_DELTA            1
#define ROUNDING_CONCENTRATION    1
#define ROUNDING_F_F0             1
#define ROUNDING_KEQ              1
#define ROUNDING_KOFF             1e-6

// define dynamic rounding to use grid point rounding in GA_SC
#define DYNAMIC_ROUNDING

// the number of points for dynamic rounding, this could be put under 
// advanced user control either as a single value for all ranges or range specific...
#define DR_POINTS 200         

#endif
