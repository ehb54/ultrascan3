// #define US_DEBUG_MPI
#include "../include/us_fe_nnls_t.h"
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <math.h>
#include <values.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include <list>
#include <vector>
#include <algorithm>
#include <mpi.h>

#include "../include/us_ga.h"
#include "../include/us_ga_stacks.h"
#include "../include/us_ga_round.h"
#include "../include/us_ga_random_normal.h"
#include "../include/us_ga_s_estimate.h"
#include "../include/us_ga_gsm.h"
#include "../include/us_ga_interacting.h"

int fitness_mfem_calls = 0;

using namespace std;

int debug_level;
int this_rank;

#define USE_GSM
/* user configuration section */

#define TOLERANCE 1e-6   /* fitness below this is success */
#define MATCH_TOLERANCE 1e-6   /* fitness below this is success */
#define ROUNDING_THRESHOLD 1e-6 /* ignore fitness dropping messages */

/* end user configuration */

vector <float> experiment_weights;

int pct_crossover;
int pct_mutation;
int pct_replication;
int pct_subtree_dup;
int pct_plague;
double pct_node_mutation;    /* this is inline with the other operations, ie. not exclusive like the above */
double pct_point_mutation;   /* this is inline with the other operations, ie. not exclusive like the above */
int point_crossover;        // this flag puts on a 1 or 2 pt crossover instead of the normal crossover


int cut_mutation;
int cut_replication;
int cut_subtree_dup;
int cut_plague;

int pop_size;
int max_generations;

int max_arg;            /* the number of external arguments */
int new_tree_depth;      /* the initial trees will have this as a max depth */
int mutate_tree_depth;    /* when mutating, this will be the max depth of the new subtree */
double beta;            /* this is the expected value of the selection exponential distribution */
double beta_divisor;

int ramped;
int list_best;
double rand_seed;        /* -1 means use timer */
int elitism;
double tolerance;
double bloat_cutoff;      /* zero means no bloat control, otherwise when max pop size > bloat control, terminate */
int fitness_reinit;      /* if non-zero, recalls fitness_init every n-th ngeneration */
int remove_duplicates;    /* remove duplicates in pop to maximize diversity, replace with new random trees */
int force_rooted;        /* if == -1, use anything, ow. use element # */
long fitness_evals;
long fitness_skipped;
int skip_fitness;        /* turn on fancy fitness skipping */
int use_alt_tree_hash;

int first_match_gen;
int best_match_gen;
unsigned int first_match_B;
unsigned int best_match_B;

unsigned long replicate_count;
unsigned long subtree_duplicate_count;
unsigned long mutate_count;
unsigned long crossover_count;
unsigned long beta_overflow;
unsigned long selection_count;
struct timeval global_start_tv;
struct timeval global_end_tv;
unsigned long total_time;

double fitness_param_1;   /* these are made available to the fitness function, defaults 0e0 */
double fitness_param_2;
double fitness_param_3;
double fitness_param_4;
double fitness_param_5;
double fitness_param_6;
double fitness_param_7;

double fitness_out_1;   // five more values for local output
double fitness_out_2;
double fitness_out_3;
double fitness_out_4;
double fitness_out_5;
double fitness_out_6;
double fitness_out_7;
double fitness_out_8;
double fitness_out_9;
double fitness_out_10;
double fitness_out_11;
double fitness_out_12;
double fitness_out_13;
double fitness_out_14;
double fitness_out_15;

double solute_early_l2_termination; // stop when this is met
long solute_max_mfem_calls;       // stop when mfem calls exceeds this

char *cachedir;
unsigned int initial_diskcache_size; // this is number of entries, multiply by vector size to get disk space
int genetic_use_mem_cache;

int pop_data_size;

// gsm control

int elitist_gsm;   // if true, only operates on one elitist member
int elitist_gsm_generation_start;   // no gsm before this generation

double inverse_hessian_prob;
double conjugate_gradient_prob;
double steepest_descent_prob;

int inverse_hessian_iter;
int conjugate_gradient_iter;
int steepest_descent_iter;

double inverse_hessian_h;
double conjugate_gradient_h;
double steepest_descent_h;

char *base_directory;

double concentration_inactivate_prob;
double generation_inactivate_prob;
int node_mutate_reactivates;
int inactive_control;
int inactivation_deletes;

#define MAX_MIGRATE_IN_FILES 256

double migrate_prob;
char *migrate_out_filename;
char *migrate_out_filename_new;
char *migrate_out_mv_new_to_current;
char *migrate_in_filename[MAX_MIGRATE_IN_FILES];
int migrate_in_file_count;

int end_save_best;            // save best # to output
char *end_save_best_filename;
int save_every_fitness_after_generation; // number of generations before saving starts
int save_every_fitness_best; // # to save best every generation
char *save_every_fitness_filename;
FILE *save_every_fitness_file;

double clip_bottom; // this clips the bottom of the cell @ this position
double cell_top;
double cell_bottom;

int use_random_normal;
double random_normal_sd_1;
double random_normal_sd_2;
int gen_mult_sd;
double gen_mult_sd_factor;

double min_f_ratio;
double max_f_ratio;
double use_vbar;

int solute_rounding;

double s_proximity_limit;
int proximity_inactivates;
double min_s, max_s;
int s_partition, f_partition;
double s_partition_increment, f_partition_increment;
int partition_steps;
int partition_wrap;
int partition_merge;
int partition_zoom_steps;
double partition_scaling_factor_initial;
double partition_scaling_factor;
int partition_local_random_steps;
double partition_local_random_radius;
double partition_coalesce_threshold;
double partition_coalesce_concentration_threshold;
double baseline_adder;
double time_correction;
int coalesce_in_local_random;
int coalesce_in_zoom;
double regularization_factor;
int regularize_on_RMSD;

int centered_points;
QString analysis_type;
double s_rounding;
int ga_mw = 0;
int ga_sc = 0;

typedef struct _Solute_center
{
   double s;
   double k;
   double r;
}
Solute_center;

vector<Solute_center> solute_centers;

int prune_depth;

vector <Simulation_values> ga_last_sve;
vector <vector <Simulation_values> > ga_best_sve;


// function definition section

value f_solute(value *, void *v)
{
   // maybe push values onto stack
   // then, fitness evaluation can pop off stack & run the fem stuff
   node *n = (node *)v;
   if(((char *)n->data)[SOLUTE_DATA_ACTIVE_OFS])
   {
      double *d = (double *)n->data;
      if(!ga_sc && s_proximity_limit && sp[RESULT_STACK])
      {
         int i;
         for(i = 0; i < sp[RESULT_STACK]; i+=2)
         {
            if(fabs(d[0] - stack[RESULT_STACK][i]) < s_proximity_limit)
            {
               if(debug_level > 4)
               {
                  printf("s proximity limit exclusion %g %g %g\n", d[0], stack[RESULT_STACK][i], fabs(d[0] - stack[RESULT_STACK][i]));
               }
               if(proximity_inactivates)
               {
                  ((char *)n->data)[SOLUTE_DATA_PROX_FLAG] = 0;
                  ((char *)n->data)[SOLUTE_DATA_ACTIVE_OFS] = 0;
               }
               else
               {
                  ((char *)n->data)[SOLUTE_DATA_PROX_FLAG] = 1;
               }
               return(0);
            }
         }
      }
      push_stack(RESULT_STACK, d[0]); // sedimentation
      push_stack(RESULT_STACK, d[1]); // k (f_ratio)
      ((char *)n->data)[SOLUTE_DATA_PROX_FLAG] = 0;
   }
   return(0);
}

double *s_coeff_ranges;
int next_solute_number;

void init_solute()
{
   next_solute_number = 0;
   // todo setup estimates
   printf("%d: s estimates %d\n", this_rank, s_estimate_solutes);
   fflush(stdout);
}

void f_init_solute(void *v)
{
   // for data usage, set alloc to n->data, & set n->data_size to sizeof alloced space
   node *n = (node *)v;
   double *d;
   //  char *c;
   if((n->data = (void *)malloc(SOLUTE_DATA_SIZE)) == NULL)
   {
      fputs("f_init_solute malloc failure\r\n", stderr);
      exit(-1);
   }
   n->data_size = SOLUTE_DATA_SIZE;
   d = (double *)n->data;
   ((char *)n->data)[SOLUTE_DATA_ACTIVE_OFS] = 1;
   int i = next_solute_number;
   next_solute_number = (next_solute_number + 1) % s_estimate_solutes;
   d[2] = s_estimates[i][0];
   d[3] = s_estimates[i][1];
   d[4] = ff0_estimates[i][0];
   d[5] = ff0_estimates[i][1];
   // find a uniform (for now) value of s
   if (ga_sc) 
   {
      s_rounding = d[4];
#if defined(DYNAMIC_ROUNDING)
      double spacing = (d[3] - d[2]) / (d[4] - 1);
      d[0] = d[2] + floor(.5 + drand48() * (d[3] - d[2]) / spacing) * spacing;
#else
      d[0] = roundn((drand48() * (d[3] - d[2])) + d[2], s_rounding, solute_rounding);
#endif
      d[1] = d[4]; // ga_sc fixes d[1]
   } else {
      d[0] = roundn((drand48() * (d[3] - d[2])) + d[2], s_rounding, solute_rounding);
      d[1] = roundn((drand48() * (d[5] - d[4])) + d[4], 1, solute_rounding);
   }
   if(debug_level > 1 &&
      (d[0] < d[2] || d[0] > d[3] ||
       (!ga_sc && (d[1] < d[4] || d[1] > d[5]))))
   {
      printf("%d: f_init_solute unexpected range error!\n", this_rank);
   }
   if(d[0] < d[2])
   {
      d[0] = d[2];
   }
   if(d[0] > d[3])
   {
      d[0] = d[3];
   }
   if(d[1] < d[4])
   {
      d[1] = d[4];
   }
   if(d[1] > d[5])
   {
      d[1] = d[5];
   }
   if(d[0] < 1e-14 && !ga_sc)
   {
      printf("%d: init solute d[0] %.4g d[2] %.4g d[3] %.4g\n", this_rank, d[0], d[2], d[3]);
      fflush(stdout);
   }
   //  printf("correct init solute %d %g %g\n", i, d[0], d[1]);
   d[SOLUTE_DATA_CONCENTRATION] = 0e0; // last solute concentration
   return;
}

#define SOLUTE_MUTATE_MULT  fitness_param_1
int node_mutate_count;
int node_mutate_count_s;
int node_mutate_count_D;
int this_generation;

void f_node_mutate_solute(void *v)
{
   // mutate n->data  requires -n command line option
   if(100.0 * drand48() < pct_node_mutation)
   {
      node *n = (node *)v;
      double *d = (double *)(n->data);
      double sel = drand48(); // so we have a 60% chance of hitting the s or the D and a 20% chance of hitting both
      // for ga_sc, 100% chance of hitting just the s value, which is the only value mutated
      node_mutate_count++;
      if(use_random_normal || ga_sc)
      {
         double_pair v;
         random_normal_sd_1 = (d[3] - d[2])/(6.0 * log(2.0 + 2.0 * this_generation) / log(2.0));
         if ( ga_sc )
         {
            random_normal_sd_2 = random_normal_sd_1;
         } else {
            random_normal_sd_2 = (d[5] - d[4])/(6.0 * log(2.0 + 2.0 * this_generation) / log(2.0));
         }
         v = random_normal(d[0], random_normal_sd_1, d[1], random_normal_sd_2);
         //     printf("%d: v.x %.4g %.4g [%.4g,%.4g,%.4g] v.y %.4g %.4g [%.4g,%.4g,%.4g]\n",
         //    this_rank,
         //    v.x, random_normal_sd_1,
         //    d[0], d[2], d[3],
         //    v.y, random_normal_sd_2,
         //    d[1], d[4], d[5]); fflush(stdout);

         if(sel > .4 && !ga_sc)
         {
            node_mutate_count_D++;
            d[1] = v.y;
            if(d[1] < d[4])
            {
               d[1] = d[4];
            }
            else
            {
               if(d[1] > d[5])
               {
                  d[1] = d[5];
               }
            }
            if (ga_sc) 
            {
               s_rounding = d[4];
            }
            d[1] = roundn(d[1], 1, solute_rounding);
         }
         if(sel < .6 || ga_sc)
         {
            node_mutate_count_s++;
            d[0] = v.x;
#if defined(DYNAMIC_ROUNDING)
            if ( ga_sc ) 
            {
               double spacing = (d[3] - d[2]) / (d[4] - 1);
               d[0] = d[2] + floor(.5 + (d[0] - d[2]) / spacing) * spacing;
            }
#endif
            if(d[0] < d[2])
            {
               d[0] = d[2];
            }
            else
            {
               if(d[0] > d[3])
               {
                  d[0] = d[3];
               }
            }
#if defined(DYNAMIC_ROUNDING)
            if ( !ga_sc )
            {
#endif
               d[0] = roundn(d[0], s_rounding, solute_rounding);
#if defined(DYNAMIC_ROUNDING)
            }
#endif
         }
      }
      else
      {
         if(sel > .4)
         {
            node_mutate_count_D++;
            if(drand48() > .5)
            {
               d[1] += (drand48() * (d[5] - d[4]) + d[4]) * SOLUTE_MUTATE_MULT;
               if(d[1] > d[5])
               {
                  d[1] = d[4];
               }
            }
            else
            {
               d[1] -= (drand48() * (d[5] - d[4]) + d[4]) * SOLUTE_MUTATE_MULT;
               if(d[1] < d[4])
               {
                  d[1] = d[4];
               }
            }
            d[1] = roundn(d[1], 1, solute_rounding);
         }
         if(sel < .6)
         {
            node_mutate_count_s++;
            if(drand48() > .5)
            {
               d[0] += (drand48() * (d[3] - d[2]) + d[2]) * SOLUTE_MUTATE_MULT;
               if(d[0] > d[3])
               {
                  d[0] = d[3];
               }
            }
            else
            {
               d[0] -= (drand48() * (d[3] - d[2]) + d[2]) * SOLUTE_MUTATE_MULT;
               if(d[0] < d[2])
               {
                  d[0] = d[2];
               }
            }
            d[0] = roundn(d[0], s_rounding, solute_rounding);
         }
      }
      if(d[0] < 1e-14 && !ga_sc)
      {
         printf("%d: mutate solute d[0] %.4g d[2] %.4g d[3] %.4g\n", this_rank, d[0], d[2], d[3]);
         fflush(stdout);
      }
      if(d[0] < d[2])
      {
         d[0] = d[2];
      }
      if(d[0] > d[3])
      {
         d[0] = d[3];
      }
      if(d[1] < d[4])
      {
         d[1] = d[4];
      }
      if(d[1] > d[5])
      {
         d[1] = d[5];
      }
      if(node_mutate_reactivates)
      {
         ((char *)n->data)[SOLUTE_DATA_ACTIVE_OFS] = 1;
      }
      return;
   }
}

unsigned int f_hash_solute(void *v)
{
   // return a hash value
   node *n = (node *)v;
   unsigned int hash = 0;
   char *x = (char *)(n->data);
   int i;
   if(x[SOLUTE_DATA_ACTIVE_OFS])
   {
      for(i = 0; i < (n->data_size - 1); i++)
      {
         hash += x[i];
      }
   }
   return hash;
}

void f_fprint_params_solute(FILE *f, void *v)
{
   // output some data from n->data
   node *n = (node *)v;
   double *d = (double *)(n->data);
   fprintf(f, "[%g %g]%s(%.5g)",
           d[0], d[1],
           ((char *)n->data)[SOLUTE_DATA_ACTIVE_OFS] ? "" : "i",
           ((double *)n->data)[SOLUTE_DATA_CONCENTRATION]);
}

value f_write(value *args, void *)
{
   push_stack(0, args[0]);
   return 0;
}

value f_whileterminals(value *, void *)
{
   // dummy stub for control entry, handled in evaluate
   return 0;
}

value f_whileterminalswrite(value *, void *)
{
   // dummy stub for control entry, handled in evaluate
   return 0;
}


value f_ifgtz(value *args, void *)
{
   if(args[0] > 0)
   {
      return(args[1]);
   }
   else
   {
      return(args[2]);
   }
}

value f_ifgt(value *args, void *)
{
   if(args[0] > args[1])
   {
      return(args[2]);
   }
   else
   {
      return(args[3]);
   }
}

value f_const_1(value *, void *)
{
   return(1);
}

value f_const_0(value *, void *)
{
   return(0);
}

value f_const_max_arg(value *, void *)
{
   return(max_arg);
}

value f_plus(value *args, void *)
{
   return(args[0] + args[1]);
}

value f_minus(value *args, void *)
{
   return(args[0] - args[1]);
}

value f_times(value *args, void *)
{
   return(args[0] * args[1]);
}

value f_divide(value *args, void *)
{
   if(args[0] == args[1])
      return 1;
   return(args[1] ? args[0] / args[1] : 0);
}

int f_int_modulo(int *args, void *)
{
   return(args[1] ? args[0] % args[1] : 0);
}

value f_exp(value *args, void *)
{
   value i;
   value r = args[0];
   if(args[1] == 0)
      return(1);
   if(args[1] == 1)
      return(r);
   if(args[1] < 0)
      return(0);

   if(args[1] > 20)
      return(VALUE_MAX);
   if(args[1] > 1)
   {
      for(i = 1; i < args[1]; i++)
      {
         r *= args[0];
      }
      return r;
   }
   return 0;
   //  return(args[0] > 0 ? (int)pow(args[0],args[1]) : 0);
}

value f_factorial(value *args, void *)
{
   if(args[0] == 0 || args[0] == 1)
      return(1);
   if(args[0] < 0)
      return(0);
   if(args[0] > 12)
      return(VALUE_MAX);
   {
      value j, r = 1;
      for(j = 2; j<= args[0]; j++)
      {
         r *= j;
      }
      return(r);
   }
}

value f_end_of_tree(value *, void *)
{
   return(0);
}

void f_init_null(void *)
{
   // for data usage, set alloc to n->data, & set n->data_size to sizeof alloced space
   //  node *n = (node *)v;
}

void f_node_mutate_null(void *)
{
   // mutate n->data  requires -n command line option
   //  node *n = (node *)v;
}

unsigned int f_hash_null(void *v)
{
   // return a hash value
   node *n = (node *)v;
   return n->e->id;
}

void f_fprint_params_null(FILE *, void *)
{
   // output some data from n->data
   //  node *n = (node *)v;
}


element elements[] = {
   /* id, argc, init_weight, 0, control, rooted, f, f_init, f_node_mutate, f_hash, f_fprint_params,  name */
   { 0, 1, 3, 0, CTL_NONE, 0, f_solute, f_init_solute, f_node_mutate_solute, f_hash_solute, f_fprint_params_solute, "SOLUTE" },
   { 0, 1, 0, 0, CTL_WHILETERMINALS, 1, f_whileterminals, f_init_null, f_node_mutate_null, f_hash_null, f_fprint_params_null, "WHILETERMINALS" },
   { 0, 2, 0, 0, CTL_WHILETERMINALSWRITE, 1, f_whileterminalswrite, f_init_null, f_node_mutate_null, f_hash_null, f_fprint_params_null, "WHILETERMINALSWRITE" },
   { 0, 2, 0, 0, CTL_NONE, 0, f_plus, f_init_null, f_node_mutate_null, f_hash_null, f_fprint_params_null, "+" },
   { 0, 2, 0, 0, CTL_NONE, 0, f_minus, f_init_null, f_node_mutate_null, f_hash_null, f_fprint_params_null, "-" },
   { 0, 2, 0, 0, CTL_NONE, 0, f_times, f_init_null, f_node_mutate_null, f_hash_null, f_fprint_params_null, "*" },
   { 0, 2, 0, 0, CTL_NONE, 0, f_divide,f_init_null, f_node_mutate_null, f_hash_null, f_fprint_params_null,  "/" },
   { 0, 2, 0, 0, CTL_NONE, 0, f_exp, f_init_null, f_node_mutate_null, f_hash_null, f_fprint_params_null, "^" },
   { 0, 0, 0, 0, CTL_NONE, 0, f_const_1, f_init_null, f_node_mutate_null, f_hash_null, f_fprint_params_null, "1" },
   { 0, 0, 0, 0, CTL_NONE, 0, f_const_0, f_init_null, f_node_mutate_null, f_hash_null, f_fprint_params_null, "0" },
   { 0, 0, 0, 0, CTL_NONE, 0, f_const_max_arg, f_init_null, f_node_mutate_null, f_hash_null, f_fprint_params_null, "MAX_ARG" },
   { 0, 0, 0, 0, CTL_NONE, 0, f_factorial, f_init_null, f_node_mutate_null, f_hash_null, f_fprint_params_null, "!" },
   { 0, 3, 0, 0, CTL_NONE, 0, f_ifgtz, f_init_null, f_node_mutate_null, f_hash_null, f_fprint_params_null, "IFGTZ" },
   { 0, 4, 0, 0, CTL_NONE, 0, f_ifgt, f_init_null, f_node_mutate_null, f_hash_null, f_fprint_params_null, "IFGT" },
   { 0, 1, 0, 0, CTL_NONE, 0, f_write, f_init_null, f_node_mutate_null, f_hash_null, f_fprint_params_null, "WRITE" },
   { 0, 0, 0, 0, CTL_NONE, 0, f_end_of_tree, f_init_null, f_node_mutate_null, f_hash_null, f_fprint_params_null, "eot" }
};

int max_e;

int max_element_weight;
int max_element_weight_no_eot;

void setup_elements()
{
   int i;

   max_e = sizeof(elements) / sizeof(element);
   for(i = 0; i < max_e; i++)
   {
      elements[i].id = i;
   }

   max_element_weight = 0;
   max_element_weight_no_eot = 0;
   for(i = 0; i < max_e; i++)
   {
      max_element_weight += elements[i].init_weight;
      if(i < max_e - 1)
         max_element_weight_no_eot += elements[i].init_weight;
      elements[i].weight_sum = max_element_weight;
   }
}

node *new_node()
{
   node *n;
   int i;
   if((n = (node *)malloc(sizeof(node))) == NULL)
   {
      fputs("new_node malloc failure\r\n", stderr);
      exit(-1);
   }
   for(i = 0; i < MAX_BR; i++)
   {
      n->children[i] = (node *)0;   // no children
      n->argv[i] = 0;            // no argument assignments
   }
   n->data = 0;
   n->data_size = 0;
   return(n);
}

void free_node(node *n)
{
   if(n)
   {
      if(n->data)
      {
         //  puts("free n->data");
         free(n->data);
      }
      //   puts("free n");
      free(n);
   }
}

void free_tree(node *n)
{
   //  printf("free_tree %lx\n", n);
   if(n)
   {
      int i;
      for(i = 0; i < MAX_BR; i++)
      {
         free_tree(n->children[i]);
      }
      free_node(n);
   }
   //  printf("ret free_tree %lx\n", n);
}

int set_tree_pos(node *n, node *nodes[], int offset)
{
   int i;
   n->pos = offset; /* offset to offset+argc-1 are this nodes addresses */
   for(i=0; i < n->e->argc; i++)
   {
      nodes[offset++] = n;
   }
   for(i = 0; i < n->e->argc; i++)
   {
      if(n->children[i])
      {
         offset = set_tree_pos(n->children[i], nodes, offset);
      }
   }
   return offset;
}

population *new_population(node *n, int points)
{
   population *p;
   //  int i;
   if((p = (population *)malloc(sizeof(population))) == NULL)
   {
      fputs("new_population malloc failure\r\n", stderr);
      exit(-1);
   }
   p->next = (population *)0;
   p->prev = (population *)0;
   p->root = n;
   p->points = points;
   p->active_points = points;
   p->no_gsm_improvement = 0;
   p->data = (void *)0;
   if(pop_data_size)
   {
      if((p->data = (void *)malloc(pop_data_size)) == NULL)
      {
         fputs("new_population data malloc failure\r\n", stderr);
         exit(-1);
      }
   }
   if((p->nodes = (node **)malloc(sizeof(node) * points)) == NULL)
   {
      fputs("new_population nodes malloc failure\r\n", stderr);
      exit(-1);
   }
   set_tree_pos(n, p->nodes, 0);
   return(p);
}

population *first_population(population *p)
{
   while(p->prev)
   {
      p = p->prev;
   }
   return(p);
}

population *last_population(population *p)
{
   while(p->next)
   {
      p = p->next;
   }
   return(p);
}

void delete_population(population *p)
{ /* including the tree at root */
   if(p->prev)
   {
      (p->prev)->next = p->next;
   }
   if(p->next)
   {
      (p->next)->prev = p->prev;
   }
   //  printf("del pop free tree %lx\n", p->root);
   free_tree(p->root);
   //  puts("del pop root deleted");
   free(p->nodes);
   //  puts("del pop nodes");
   if(p->data)
   {
      free(p->data);
   }
   free(p);
   //  puts("del pop deleted");
}

population *insert_population(population *p, population *p_new)
{
   if(p)
   {
      p_new->next = p->next;
      p_new->prev = p;
      p->next = p_new;
   }
   return(p_new);
}

int count_tree_points(node *n)
{
   int i;
   int r = n->e->argc;
   for(i = 0; i < n->e->argc; i++)
   {
      if(n->children[i])
      {
         r += count_tree_points(n->children[i]);
      }
   }
   return r;
}

int count_tree_active_points(node *n)
{
   int i;
   int r = (int)((char *)n->data)[SOLUTE_DATA_ACTIVE_OFS];
   for(i = 0; i < n->e->argc; i++)
   {
      if(n->children[i])
      {
         r += count_tree_active_points(n->children[i]);
      }
   }
   return r;
}

void prune_tree(node *n, int prune_at)
{
   int i;
   if(!prune_at)
   {
      fprintf(stderr, "prune_tree called with zero argument\n");
      exit(-1);
   }
   prune_at--;
   if(prune_at)
   {
      for(i = 0; i < n->e->argc; i++)
      {
         if(n->children[i])
         {
            prune_tree(n->children[i], prune_at);
         }
      }
   }
   else
   {
      for(i = 0; i < n->e->argc; i++)
      {
         if(n->children[i])
         {
            free_tree(n->children[i]);
            n->children[i] = (node *)0;
            n->argv[i] = (int)floor(drand48() * max_arg);
         }
      }
   }
}

void replace_population_node(population *p, node *n)
{
   free_tree(p->root);
   free(p->nodes);
   p->root = n;
   p->points = count_tree_points(p->root);
   p->active_points = count_tree_active_points(p->root);
   if((p->nodes = (node **)malloc(sizeof(node) * p->points)) == NULL)
   {
      fputs("replace_population nodes malloc failure\r\n", stderr);
      exit(-1);
   }
   set_tree_pos(n, p->nodes, 0);
}

node *delete_inactive_next_active_node(node *n, population *p)
{
   // deletes inactives and returns next active or zero of none left.
   node *m;
   //  printf("delete_inactive_next <points %d> solute[%g %g] %d\n", p->points, ((double *)n->data)[0], ((double *)n->data)[1],
   //    ((char *)n->data)[SOLUTE_DATA_ACTIVE_OFS]);
   // delete any inactive
   while(!((char *)n->data)[SOLUTE_DATA_ACTIVE_OFS])
   {
      //   puts("delete_inactive_next found inactive");
      m = n;
      n = n->children[0];
      free_node(m); // clear inactive node
      p->points--;
      if(!n)
      {
         // no children left
         // puts("no children left");
         return 0;
      }
   }
   // ok, we have an active node
   //  puts("ok, we've got an active one");
   if(n->children[0])
   {
      // now point to next inactive or zero
      if(!(n->children[0] = delete_inactive_next_active_node(n->children[0], p)))
      {
         n->argv[0] = 0;
      }
   }
   // return the active node
   return n;
}

void delete_inactive_nodes(population *p)
{
   node *n;
   // need to find first active
   while(!((char *)p->root->data)[SOLUTE_DATA_ACTIVE_OFS])
   {
      if(!p->root->children[0])
      {
         puts("no active child!");
         list_tree(p->root);
         puts("");
         fprintf(stderr, "no active child!\n");
         exit(-1);
      }
      //   printf("inactive root %d\n", ((char *)p->root->data)[SOLUTE_DATA_ACTIVE_OFS]);
      n = p->root;
      p->root = p->root->children[0];
      free_node(n);
      p->points--;
   }
   //  puts("ok, root is now an active one");
   if(p->root->children[0])
   {
      //   puts("with children");
      if(!(p->root->children[0] = delete_inactive_next_active_node(p->root->children[0], p)))
      {
         p->root->argv[0] = 0;
      }
   }
   if(p->points < 1 || p->points != p->active_points)
   {
      fprintf(stderr, "delete inactive nodes error : p->points %d p->active_points %d\n",
              p->points,
              p->active_points
              );
      exit(-1);
   }
   set_tree_pos(p->root, p->nodes, 0);
}

population *insert_population_node(population *p, node *n)
{
   return(insert_population(p, new_population(n, count_tree_points(n))));
}

void list_tree(node *n)
{
   int i;
   printf("(%s", n->e->name);
   n->e->fprint_params(stdout, (void *)n);
   printf(" ");
   for(i = 0; i < n->e->argc; i++)
   {
      if(n->children[i])
      {
         list_tree(n->children[i]);
      }
      else
      {
         printf("@%d ", n->argv[i]);
      }
   }
   printf(") ");
}

void serialize_out_tree(FILE *f, node *n)
{
   int i, tmp;
   fputc('_', f);
   tmp = MAX_BR;
   fwrite(&tmp, sizeof(tmp), 1, f);
   fwrite(&n->e->id, sizeof(n->e->id), 1, f);
   fwrite(&n->e->argc, sizeof(n->e->argc), 1, f);
   for(i = 0; i < n->e->argc; i++)
   {
      fputc(n->children[i] ? 1 : 0, f);
      if(n->children[i])
      {
         serialize_out_tree(f, n->children[i]);
      }
      fwrite(&n->argv[i], sizeof(n->argv[i]), 1, f);
   }
   fwrite(&n->pos, sizeof(n->pos), 1, f);
   fwrite(&n->data_size, sizeof(n->data_size), 1, f);
   fwrite(n->data, n->data_size, 1, f);
}

node *serialize_in_tree(FILE *f)
{
   // returns newly allocated node
   int i, tmp;
   node *n;
   tmp = fgetc(f);
   if(tmp == '.')
   {
      return (node *)0;
   }
   if(tmp != '_')
   {
      fprintf(stderr, "identifier mismatch in serialize_in_tree\n");
      exit(-1);
   }
   fread(&tmp, sizeof(tmp), 1, f);
   if(tmp != MAX_BR)
   {
      fprintf(stderr, "MAX_BR mismatch in serialize_in_tree\n");
      exit(-1);
   }
   n = new_node();
   fread(&tmp, sizeof(n->e->id), 1, f);
   //  printf("element %d\n", tmp);
   if(tmp > max_e || tmp < 0)
   {
      fprintf(stderr, "element greater than max or negative in serialize_in_tree\n");
      exit(-1);
   }
   n->e = &elements[tmp];
   if(n->e->id != tmp)
   {
      fprintf(stderr, "unexpected id mismatch in serialize_in_tree\n");
      exit(-1);
   }
   fread(&tmp, sizeof(n->e->argc), 1, f);
   if(n->e->argc != tmp)
   {
      fprintf(stderr, "unexpected argc mismatch in serialize_in_tree\n");
      exit(-1);
   }
   for(i = 0; i < n->e->argc; i++)
   {
      if(fgetc(f))
      {
         if(!(n->children[i] = serialize_in_tree(f)))
         {
            fprintf(stderr, "premature eof in serialize_in_tree\n");
            free_node(n);
            return((node *)0);
         }
      }
      else
      {
         n->children[i] = 0;
      }
      fread(&n->argv[i], sizeof(n->argv[i]), 1, f);
   }
   fread(&n->pos, sizeof(n->pos), 1, f);
   fread(&n->data_size, sizeof(n->data_size), 1, f);
   //  printf("data size %d\n", n->data_size);
   if(n->data_size)
   {
      if((n->data = (void *)malloc(n->data_size)) == NULL)
      {
         fprintf(stderr, "serialize in tree malloc of size %d failed\n", n->data_size);
         exit(-1);
      }
      fread(n->data, n->data_size, 1, f);
   }
   else
   {
      n->data = (void *)0;
   }
   return n;
}

void list_serial_file(char *name)
{
   FILE *in;
   node *n;
   int i = 0;
   if(NULL == (in = fopen(name, "r")))
   {
      fprintf(stderr, "serial file <%s> file open error\n", name);
      exit(-1);
   }
   while((n = serialize_in_tree(in)))
   {
      printf("record %d:", i);
      list_tree(n);
      puts("");
      free_tree(n);
   }
   fclose(in);
}

FILE *serial_out;
FILE *serial_in;

int init_serial_input(char *name)
{
   if(NULL == (serial_in = fopen(name, "r")))
   {
      fprintf(stderr, "serial in file <%s> file open error\n", name);
      return(1);
   }
   return(0);
}

node *get_serial_input()
{
   return serialize_in_tree(serial_in);
}

void close_serial_input()
{
   fclose(serial_in);
   serial_in = (FILE *)0;
}

void init_serial_output(char *name)
{
   if(NULL == (serial_out = fopen(name, "w")))
   {
      fprintf(stderr, "serial out file <%s> file create error\n", name);
      exit(-1);
   }
}

void add_serial_output(node *n)
{
   serialize_out_tree(serial_out, n);
}

void close_serial_output()
{
   fputc('.', serial_out);
   fclose(serial_out);
   serial_out = (FILE *)0;
}

int cmp_tree(node *n1, node *n2)
{
   int i;
   if(n1->e != n2->e)
      return 0;

   for(i = 0; i < n1->e->argc; i++)
   {
      if((n1->children[i] && !n2->children[i]) ||
         (!n1->children[i] && n2->children[i]) ||
         n1->data_size != n2->data_size || 
         (n1->data_size && memcmp(n1->data, n2->data, n1->data_size)))
         return 0;
      if(n1->children[i])
      {
         if(!cmp_tree(n1->children[i], n2->children[i]))
            return 0;
      }
      else
      {
         if(n1->argv[i] != n2->argv[i])
            return 0;
      }
   }
   return 1;
}

unsigned int tree_hash(node *n)
{
   int i;
   unsigned int hash = n->e->id;
   if(use_alt_tree_hash)
   {
      hash = n->e->hash((void *)n);
   }
   for(i = 0; i < n->e->argc; i++)
   {
      if(n->children[i])
      {
         hash += ((tree_hash(n->children[i]) * (i+1)) << 1);
      }
      else
      {
         hash += (i + 1) * (n->argv[i] + 1);
      }
   }
   return hash;
}

value evaluate(node *n, value *args)
{
   value l[n->e->argc];
   int i;
   switch(n->e->control)
   {
   case CTL_NONE :
      {
         for(i = 0; i < n->e->argc; i++)
         {
            l[i] = n->children[i] ? evaluate(n->children[i], args) : args[n->argv[i]];
         }
         return(n->e->func(l, (void *)n));
         break;
      }
   case CTL_WHILETERMINALS :
      {
         int j, k;
         int length;
         //     int length = (int)args[0];
         //     if(length < 1) {
         //   length = 1;
         //     }
         length = max_arg;
         while(nonempty_stack(ARGUMENT_STACK))
         {
            for(j = 0; j < length; j++)
            {
               k = j % max_arg;
               args[k] = pop_stack(ARGUMENT_STACK);
            }
            if(n->children[0])
            {
               evaluate(n->children[0], args);
            }
         }
         return 0;
         break;
      }
   case CTL_WHILETERMINALSWRITE :
      {
         int j, k;
         //     value ret;
         int length = (int)args[0];
         if(length < 1)
         {
            length = 1;
         }
         //     length = max_arg;
         while(nonempty_stack(ARGUMENT_STACK))
         {
            for(j = 0; j < length; j++)
            {
               k = j % max_arg;
               args[k] = pop_stack(ARGUMENT_STACK);
            }
            push_stack(0, n->children[1] ? evaluate(n->children[1], args) : args[n->argv[1]]);
         }
         return 0;
         break;
      }
   default:
      {
         fprintf(stderr, "evaluate unknown control structure\r\n");
         exit(-1);
         break;
      }
   }
   return 0;
}

int pick_a_random_function(int weight)
{
   double pos = drand48() * weight;
   int r;
   for(r = 0; r < max_e; r++)
   {
      if(elements[r].weight_sum > pos && !elements[r].rooted)
      {
         return(r);
      }
   }
   fputs("pick_a_random_function fall-through error\r\n", stderr);
   exit(-1);
}

int pick_a_random_function_rooted(int weight)
{
   double pos = drand48() * weight;
   int r;
   for(r = 0; r < max_e; r++)
   {
      if(elements[r].weight_sum > pos)
      {
         return(r);
      }
   }
   fputs("pick_a_random_function fall-through error\r\n", stderr);
   exit(-1);
}

int this_global_arg;

node *new_random_subtree(int i, int max_depth)
{
   int j;
   node *n = new_node();
   n->e = &elements[i];
   elements[i].init(n);
   for(i = 0; i < n->e->argc; i++)
   {
      j = pick_a_random_function(max_element_weight);
      if(j < max_e - 1 && max_depth > 1)
      {
         n->children[i] = new_random_subtree(j, max_depth - 1);
      }
      else
      {
         n->argv[i] = this_global_arg;
         this_global_arg = (this_global_arg + 1) % max_arg;
      }
   }
   return(n);
}

node *new_random_tree(int max_depth)
{
   int i = pick_a_random_function(max_element_weight_no_eot);
   if(i == max_e - 1)
   {
      fputs("new_random_tree unexpected eot error\r\n", stderr);
      exit(-1);
   }
   return(new_random_subtree(i, max_depth));
}

node *new_random_tree_rooted(int max_depth)
{
   int i = pick_a_random_function_rooted(max_element_weight_no_eot);
   if(force_rooted != -1)
   {
      i = force_rooted;
   }
   if(i >= max_e - 1)
   {
      fputs("new_random_tree_rooted unexpected eot error\r\n", stderr);

      exit(-1);
   }
   return(new_random_subtree(i, max_depth));
}

#define FITNESS_NO_OF_CONSTANTS 1

void init()
{
   gettimeofday(&global_start_tv, NULL);
   signal(SIGFPE, SIG_IGN);
   if(rand_seed <= 0 )
   {
      struct timeval tv;
      gettimeofday(&tv, NULL);
      srand48(tv.tv_usec + this_rank);
   }
   else
   {
      srand48((long int)rand_seed + this_rank);
   }
   stacks_init();

   //  init_diskcache(initial_diskcache_size, genetic_use_mem_cache);

   pop_data_size = FITNESS_NO_OF_CONSTANTS * sizeof(double);

   setup_elements();
   this_global_arg = 0;
   replicate_count = 0;
   subtree_duplicate_count = 0;
   mutate_count = 0;
   crossover_count = 0;
   beta_overflow = 0;
   selection_count = 0;

   fitness_out_1 =
      fitness_out_2 =
      fitness_out_3 =
      fitness_out_4 =
      fitness_out_5 =
      fitness_out_6 =
      fitness_out_7 =
      fitness_out_8 =
      fitness_out_9 =
      fitness_out_10 =
      fitness_out_11 =
      fitness_out_12 =
      fitness_out_13 =
      fitness_out_14 =
      fitness_out_15 = 0e0;

   if(save_every_fitness_filename &&
      NULL == (save_every_fitness_file = fopen(save_every_fitness_filename, "w")))
   {
      fprintf(stderr, "save every evaluate file open error, can't open \"%s\" for writing\n", save_every_fitness_filename);
      exit(-1);
   }

   init_solute();
}

int fit_cmp[10][11] = {
   { 1, 2 },
   { 1, 2, 2 },
   { 2, 5, 6, 4 },
   { 2, 6, 9, 8, 4 },
   { 4, 14, 25, 28, 20, 8 },
   { 4, 16, 33, 44, 40, 24, 8 },
   { 8, 36, 84, 129, 140, 108, 56, 16 },
   { 8, 40, 104, 180, 225, 208, 140, 64, 16 },
   { 16, 88, 252, 484, 681, 726, 588, 352, 144, 32 },
   { 16, 96, 300, 632, 985, 1182, 1106, 800, 432, 160, 32 } };

int fit_args[] = { 0, 0, 1, 2 };
value pass_fit_args[] = { 0, 0, 1, 2 };


double test_fitness(node *n)
{

   // goal function is x^2+2x+1 (or (x+1)^2)

   //  int i;
   double fit = 0e0;
   value ev;
   value ev2;

   signal(SIGFPE, SIG_IGN);
   for(fit_args[0] = 0; fit_args[0] < 12; fit_args[0]++)
   {
      pass_fit_args[0] = fit_args[0];
      ev = evaluate(n, pass_fit_args) - ((fit_args[0] + 1) * (fit_args[0]+1));
      ev2 = ev * ev;
      if(ev2 < tolerance)
      {
         ev2 = 0e0;
      }
      fit += ev2;
   }
   // printf("--%.4g\n", fit);
   if(fit< 0e0)
   {
      fprintf(stderr, "fit < 0, ugh!\r\n");
      exit(-1);
   }
   return(sqrt(fit));
}

double test_fitness_cubic(node *n)
{

   // goal function is x^3+2x+1

   //  int i;
   double fit = 0e0;
   value ev;
   value ev2;

   signal(SIGFPE, SIG_IGN);
   for(fit_args[0] = 0; fit_args[0] < 12; fit_args[0]++)
   {
      pass_fit_args[0] = fit_args[0];
      ev = evaluate(n, pass_fit_args) - (2 * fit_args[0] * fit_args[0] * fit_args[0] + fit_args[0] * fit_args[0] + 1);
      ev2 = ev * ev;
      if(ev2 < tolerance)
      {
         ev2 = 0e0;
      }
      fit += ev2;
   }
   // printf("--%.4g\n", fit);
   if(fit< 0e0)
   {
      fprintf(stderr, "fit < 0, ugh!\r\n");
      exit(-1);
   }
   return(sqrt(fit));
}

double test_fitness_quartic(node *n)
{

   // goal function is x^4+x^3+1

   //  int i;
   double fit = 0e0;
   value ev;
   value ev2;

   signal(SIGFPE, SIG_IGN);
   for(fit_args[0] = 0; fit_args[0] < 12; fit_args[0]++)
   {
      pass_fit_args[0] = fit_args[0];
      ev = evaluate(n, pass_fit_args) - (fit_args[0] * fit_args[0] * fit_args[0] * fit_args[0] +
                                         fit_args[0] * fit_args[0] * fit_args[0] + 1);

      ev2 = ev * ev;
      if(ev2 < tolerance)
      {
         ev2 = 0e0;
      }
      fit += ev2;
   }
   // printf("--%.4g\n", fit);
   if(fit< 0e0)
   {
      fprintf(stderr, "fit < 0, ugh!\r\n");
      exit(-1);
   }
   return(sqrt(fit));
}

double mmp_fitness(node *n)
{
   //  int i;
   double fit = 0e0;
   value ev;
   value ev2;

   signal(SIGFPE, SIG_IGN);
   for(fit_args[0] = 2; fit_args[0] < 8; fit_args[0]++)
   {
      for(fit_args[1] = 0; fit_args[1] < fit_args[0]; fit_args[1]++)
      {
         pass_fit_args[0] = fit_args[0];
         pass_fit_args[1] = fit_args[1];
         pass_fit_args[2] = fit_args[2];
         pass_fit_args[3] = fit_args[3];
         ev = evaluate(n, pass_fit_args) - fit_cmp[fit_args[0]-2][fit_args[1]];
         ev2 = ev * ev;
         if(ev2 < tolerance)
         {
            ev2 = 0e0;
         }
         fit += ev2;
      }
   }
   //  printf("--%.4g\n", fit);
   if(fit< 0e0)
   {
      fprintf(stderr, "fit < 0, ugh!\r\n");
      exit(-1);
   }
   return(sqrt(fit));
}


void fitness_init_null()
{
   return;
}

#define MAX_DIM   12
#define TEST_CASES 25
#define FINAL_TEST_CASES 1000

int use_dim;

double hyperplanes[TEST_CASES][MAX_DIM];
double points[TEST_CASES][MAX_DIM];
double dots[TEST_CASES];
int signs[TEST_CASES];

void fitness_init_pointhyperplane()
{
   int i, j;
   double dot;
   use_dim = max_arg / 2;
   if(use_dim > MAX_DIM)
   {
      use_dim = MAX_DIM;
   }
   max_arg = use_dim * 2;

   //  printf("using dimension %d max args %d\n", use_dim, max_arg);
   //  puts("test cases:");
   for(i = 0; i < TEST_CASES; i++)
   {
      dot = 0e0;
      for(j = 0; j < use_dim; j++)
      {
         hyperplanes[i][j] = 2e0 * drand48() - 1e0;
         points[i][j] = 2e0 * drand48() - 1e0;
         dot += hyperplanes[i][j] * points[i][j];
      }
      dots[i] = dot;
      if(dots[i] >= 0)
      {
         signs[i] = 1;
      }
      else
      {
         signs[i] = 0;
      }
      /*   printf("\t%d hyperplane (", i);
           for(j = 0; j < use_dim; j++) {
           printf("%.4g ", hyperplanes[i][j]);
           }
           printf(") point (");
           for(j = 0; j < use_dim; j++) {
           printf("%.4g ", points[i][j]);
           }
           printf(") dot %.4g\n", dot);
      */
   }
}

double fitness_pointhyperplane_dot(node *n)
{
   int i, j, k;
   double fit = 0e0;
   value ev;
   value ev2;

   signal(SIGFPE, SIG_IGN);
   for(i = 0; i < TEST_CASES; i++)
   {
      for(j = 0, k = 0; j < use_dim; j++, k++)
      {
         pass_fit_args[k++] = hyperplanes[i][j];
         pass_fit_args[k] = points[i][j];
      }
      ev = evaluate(n, pass_fit_args) - dots[i];
      ev2 = ev * ev;
      if(ev2 < tolerance)
      {
         ev2 = 0e0;
      }
      fit += ev2;
   }
   return(sqrt(fit));
}

double fitness_pointhyperplane_sign(node *n)
{
   int i, j, k;
   double fit = 0e0;
   value ev;

   signal(SIGFPE, SIG_IGN);
   for(i = 0; i < TEST_CASES; i++)
   {
      for(j = 0, k = 0; j < use_dim; j++, k++)
      {
         pass_fit_args[k++] = hyperplanes[i][j];
         pass_fit_args[k] = points[i][j];
      }
      if(evaluate(n, pass_fit_args) >= -tolerance)
      {
         if(signs[i])
         {
            ev = 0; // good match
         }
         else
         {
            ev = 1;
         }
      }
      else
      {
         if(signs[i])
         {
            ev = 1;
         }
         else
         {
            ev = 0;
         }
      }
      fit += ev;
   }
   return(fit);
}


double fitness_final_check_pointhyperplane_dot(node *n)
{
   int i, j, k;
   double fit = 0e0;
   value ev;
   value ev2;
   double test_hyperplane[use_dim];
   double test_point[use_dim];
   double test_dot;
   int test_sign;

   signal(SIGFPE, SIG_IGN);
   for(i = 0; i < FINAL_TEST_CASES; i++)
   {
      test_dot = 0e0;
      for(j = 0; j < use_dim; j++)
      {
         test_hyperplane[j] = 2e0 * drand48() - 1e0;
         test_point[j] = 2e0 * drand48() - 1e0;
         test_dot += test_hyperplane[j] * test_point[j];
      }
      if(test_dot >= 0)
      {
         test_sign = 1;
      }
      else
      {
         test_sign = 0;
      }
      for(j = 0, k = 0; j < use_dim; j++, k++)
      {
         pass_fit_args[k++] = test_hyperplane[j];
         pass_fit_args[k] = test_point[j];
      }
      ev = evaluate(n, pass_fit_args) - test_dot;
      ev2 = ev * ev;
      if(ev2 < tolerance)
      {
         ev2 = 0e0;
      }
      fit += ev2;
   }
   return(sqrt(fit));
}

double fitness_final_check_pointhyperplane_sign(node *n)
{
   int i, j, k;
   double fit = 0e0;
   value ev;
   double test_hyperplane[use_dim];
   double test_point[use_dim];
   double test_dot;
   int test_sign;

   signal(SIGFPE, SIG_IGN);
   for(i = 0; i < FINAL_TEST_CASES; i++)
   {
      test_dot = 0e0;
      for(j = 0; j < use_dim; j++)
      {
         test_hyperplane[j] = 2e0 * drand48() - 1e0;
         test_point[j] = 2e0 * drand48() - 1e0;
         test_dot += test_hyperplane[j] * test_point[j];
      }
      if(test_dot >= 0)
      {
         test_sign = 1;
      }
      else
      {
         test_sign = 0;
      }
      for(j = 0, k = 0; j < use_dim; j++, k++)
      {
         pass_fit_args[k++] = test_hyperplane[j];
         pass_fit_args[k] = test_point[j];
      }
      if(evaluate(n, pass_fit_args) >= -tolerance)
      {
         if(test_sign)
         {
            ev = 0; // good match
         }
         else
         {
            ev = 1;
         }
      }
      else
      {
         if(test_sign)
         {
            ev = 1;
         }
         else
         {
            ev = 0;
         }
      }
      fit += ev;
   }
   return(fit);
}
double dots_md[TEST_CASES][MAX_DIM];
int signs_md[TEST_CASES][MAX_DIM];

void fitness_init_pointhyperplane_multi_dim()
{
   int i, j, d;
   double dot;
   use_dim = max_arg / 2;
   if(use_dim > MAX_DIM)
   {
      use_dim = MAX_DIM;
   }
   max_arg = use_dim * 2;

   //  puts("test cases:");
   for(i = 0; i < TEST_CASES; i++)
   {
      dot = 0e0;
      for(j = 0; j < use_dim; j++)
      {
         dots_md[i][j] = 0e0;
         hyperplanes[i][j] = 2e0 * drand48() - 1e0;
         points[i][j] = 2e0 * drand48() - 1e0;
         dot = hyperplanes[i][j] * points[i][j];
         for(d = 0; d <= j; d++)
         {
            dots_md[i][d] += dot;
         }
      }
      for(d = 0; d < use_dim; d++)
      {
         if(dots_md[i][d] >= 0)
         {
            signs_md[i][d] = 1;
         }
         else
         {
            signs_md[i][d] = 0;
         }
      }
   }
   /*  for(d = 0; d < use_dim; d++) {
       for(i = 0; i < TEST_CASES; i++) {
       printf("\t%d %d-d hyperplane (", i, d+1);
       for(j = 0; j <= d; j++) {
       printf("%6.4g ", hyperplanes[i][j]);
       }
       printf(")\t point (");
       for(j = 0; j <= d; j++) {
       printf("%6.4g ", points[i][j]);
       }
       printf(")\t dot %6.4g sign %d\n", dots_md[i][d], signs_md[i][d]);
       }
       puts("");
       } */
}

double fitness_pointhyperplane_multi_dim_dot(node *n)
{
   int i, j, k, d;
   double fit = 0e0;
   value ev;
   value ev2;

   signal(SIGFPE, SIG_IGN);

   for(i = 0; i < TEST_CASES; i++)
   {
      for(d = 0; d < use_dim; d++)
      {
         for(j = 0, k = 0; j < use_dim; j++, k++)
         {
            pass_fit_args[k++] = hyperplanes[i][j % (d+1)];  // loop around the points
            pass_fit_args[k] = points[i][j % (d+1)];
         }
         ev = evaluate(n, pass_fit_args) - dots_md[i][d];
         ev2 = ev * ev;
         if(ev2 < tolerance)
         {
            ev2 = 0e0;
         }
         fit += ev2;
      }
   }
   return(sqrt(fit));
}

double fitness_pointhyperplane_multi_dim_sign(node *n)
{
   int i, j, k, d;
   double fit = 0e0;
   value ev;

   signal(SIGFPE, SIG_IGN);

   for(i = 0; i < TEST_CASES; i++)
   {
      for(d = 0; d < use_dim; d++)
      {
         for(j = 0, k = 0; j < use_dim; j++, k++)
         {
            pass_fit_args[k++] = hyperplanes[i][j % (d+1)];  // loop around the points
            pass_fit_args[k] = points[i][j % (d+1)];
         }
         if(evaluate(n, pass_fit_args) >= -tolerance)
         {
            if(signs_md[i][d])
            {
               ev = 0; // good match
            }
            else
            {
               ev = 1;
            }
         }
         else
         {
            if(signs_md[i][d])
            {
               ev = 1;
            }
            else
            {
               ev = 0;
            }
         }
         fit += ev;
      }
   }
   return(fit);
}

#define MAX_H 5

int number_of_h[TEST_CASES];

double hyperplanes_mh[TEST_CASES][MAX_H][MAX_DIM];
double points_mh[TEST_CASES][MAX_H][MAX_DIM];

double dots_mh[TEST_CASES][MAX_H];
int signs_mh[TEST_CASES][MAX_H];

void fitness_init_pointhyperplane_stack_multi_h()
{
   int i, j, h, use_h;
   double dot;
   use_dim = max_arg / 2;
   if(use_dim > MAX_DIM)
   {
      use_dim = MAX_DIM;
   }
   max_arg = use_dim * 2;

   //  puts("test cases:");
   for(i = 0; i < TEST_CASES; i++)
   {
      use_h = (int)floor(1+drand48()*(MAX_H - 1));
      number_of_h[i] = use_h;
      for(h = 0; h < use_h; h++)
      {
         dot = 0e0;
         for(j = 0; j < use_dim; j++)
         {
            dots_mh[i][h] = 0e0;
            hyperplanes_mh[i][h][j] = 2e0 * drand48() - 1e0;
            points_mh[i][h][j] = 2e0 * drand48() - 1e0;
            dot += hyperplanes_mh[i][h][j] * points_mh[i][h][j];
         }
         dots_mh[i][h] = dot;
         if(dots_mh[i][h] >= 0)
         {
            signs_mh[i][h] = 1;
         }
         else
         {
            signs_mh[i][h] = 0;
         }
      }
   }
   /*  for(i = 0; i < TEST_CASES; i++) {
       printf("number of h %d\n", number_of_h[i]);
       for(h = 0; h < number_of_h[i]; h++) {
       printf("\t%d %d-d hyperplane # %d (", i, use_dim, h);
       for(j = 0; j < use_dim; j++) {
       printf("%6.4g ", hyperplanes_mh[i][h][j]);
       }
       printf(")\t point (");
       for(j = 0; j < use_dim; j++) {
       printf("%6.4g ", points_mh[i][h][j]);
       }
       printf(")\t dot %6.4g sign %d\n", dots_mh[i][h], signs_mh[i][h]);
       }
       puts("");
       } 
   */
}

double fitness_pointhyperplane_stack_multi_h_sign(node *n)
{
   int i, j, k, h;
   double fit = 0e0;
   value ret;
   value ev;

   signal(SIGFPE, SIG_IGN);

   for(i = 0; i < TEST_CASES; i++)
   {
      stacks_init();
      for(h = 0; h < number_of_h[i]; h++)
      {
         for(j = 0, k = 0; j < use_dim; j++, k++)
         {
            push_stack(1, hyperplanes_mh[i][h][j]);
            push_stack(1, points_mh[i][h][j]);
         }
      }
      evaluate(n, pass_fit_args);
      for(h = 0; h < number_of_h[i]; h++)
      {
         if(nonempty_stack(RESULT_STACK))
         {
            ret = pop_stack(RESULT_STACK);
            if(ret >= -tolerance)
            {
               if(signs_mh[i][h])
               {
                  ev = 0; // good match
               }
               else
               {
                  ev = 1;
               }
            }
            else
            {
               if(signs_mh[i][h])
               {
                  ev = 1;
               }
               else
               {
                  ev = 0;
               }
            }
         }
         else
         {
            ev = use_dim * 2; // penalize missing args
         }
         fit += ev;
      }
      fit += use_dim * 2 * sp[RESULT_STACK];  // penalize excess args
   }
   return(fit);
}

double fitness_final_check_pointhyperplane_stack_multi_h_sign(node *n)
{
   int i, j, k, h, use_h;
   double fit = 0e0;
   value ret;
   value ev;
   double test_hyperplanes[MAX_H][use_dim];
   double test_points[MAX_H][use_dim];
   double test_dots[MAX_H];
   int test_signs[MAX_H];

   //  printf("regular fitness check = %g\n", fitness_pointhyperplane_stack_multi_h_sign(n));
   //  fitness_init_pointhyperplane_stack_multi_h();
   //  printf("regular fitness after new init check = %g\n", fitness_pointhyperplane_stack_multi_h_sign(n));

   signal(SIGFPE, SIG_IGN);
   for(i = 0; i < FINAL_TEST_CASES; i++)
   {
      stacks_init();
      use_h = (int)floor(1+drand48()*(MAX_H - 1));
      //   printf("use_h %d\n", use_h);
      for(h = 0; h < use_h; h++)
      {
         test_dots[h] = 0e0;
         for(j = 0; j < use_dim; j++)
         {
            test_hyperplanes[h][j] = 2e0 * drand48() - 1e0;
            test_points[h][j] = 2e0 * drand48() - 1e0;
            test_dots[h] += test_hyperplanes[h][j] * test_points[h][j];
         }
         if(test_dots[h] >= 0)
         {
            test_signs[h] = 1;
         }
         else
         {
            test_signs[h] = 0;
         }
      }
      for(h = 0; h < use_h; h++)
      {
         for(j = 0, k = 0; j < use_dim; j++, k++)
         {
            push_stack(1, test_hyperplanes[h][j]);
            push_stack(1, test_points[h][j]);
         }
      }
      //   puts("stack 1 before evaluate:\n");
      //   print_stack(ARGUMENT_STACK);
      evaluate(n, pass_fit_args);
      //   puts("stack 0 after evaluate:\n");
      //   print_stack(RESULT_STACK);
      //   for(h = 0; h < use_h; h++) {
      //     printf("test_signs[%d] %d\n", h, test_signs[h]);
      //   }
      for(h = 0; h < use_h; h++)
      {
         if(nonempty_stack(RESULT_STACK))
         {
            ret = pop_stack(RESULT_STACK);
            if(ret >= -tolerance)
            {
               if(test_signs[h])
               {
                  ev = 0; // good match
               }
               else
               {
                  ev = 1;
               }
            }
            else
            {
               if(test_signs[h])
               {
                  ev = 1;
               }
               else
               {
                  ev = 0;
               }
            }
         }
         else
         {
            ev = use_dim * 2;
         }
         fit += ev;
      }
      fit += use_dim * 2 * sp[RESULT_STACK];
   }
   //  printf("final fitness check = %g\n", fit);
   return(fit);
}

unsigned long total_fitness_time;

double l2_comps[MAX_STACK_DEPTH];
int l2_comps_size;

#define MAX_INPUT_LIN_LENGTH 4096
#define MAX_INPUT_LIN_OPTS 128

void fitness_init_solute()
{
   max_arg = 1;
   if(fitness_param_1 <= 0e0)
   {
      fitness_param_1 = .1;
      printf("solute mutate pct set to %g, use -X to set otherwise\n", fitness_param_1);
   }
   if(fitness_param_2 <= 10e0)
   {
      fitness_param_2 = 101;
      //   printf("scan points set to %g, use -Y to set otherwise\n", fitness_param_2);
   }
   if(fitness_param_3 <= 0.4e0)
   {
      fitness_param_3 = 1.5e0;
      //   printf("concentration clipping set to %g, use -Z to set otherwise\n", fitness_param_3);
   }
   //  mfem_fitness_init((int)fitness_param_2, fitness_param_3, clip_bottom, cell_top, cell_bottom);
   // todo new fitness stuff
   /*
     total_fitness_time = 0l;

     {
     FILE *load_file;
     char line[MAX_INPUT_LIN_LENGTH];
     char *p;
     int i;
     
     if(load_file = fopen("l2_configuration", "r")) {
     double s;

     while(fgets(line, sizeof(line), load_file) != NULL) {
      
     p = strchr(line, ' ');
     if(!p) {
     fprintf(stderr, "l2_configuration file format error\n");
     exit(-1);
     }
     p++;
     push_stack(TMP_STACK, s = atof(p));
     push_stack(TMP_STACK, k_calc(s, atof(line), use_vbar));
     }
     } else {
     puts("using default concentrations\n");
     push_stack(TMP_STACK, k_calc(4.2376e-13, 7.3426e-7, use_vbar));
     push_stack(TMP_STACK, 4.2376e-13);
     push_stack(TMP_STACK, k_calc(5.9432e-13, 1.6219e-7, use_vbar));
     push_stack(TMP_STACK, 5.9432e-13);
     push_stack(TMP_STACK, k_calc(9.0226e-13, 1.8725e-7, use_vbar)); 
     push_stack(TMP_STACK, 9.0226e-13);
     }
     puts("tmp stack");
     print_stack(TMP_STACK);

     i = 0;
     l2_comps_size = sp[TMP_STACK];
     while(sp[TMP_STACK]) {
     l2_comps[i++] = pop_stack(TMP_STACK);
     l2_comps[i++] = pop_stack(TMP_STACK);
     }
     }
   */
}

struct timeval start_tv;
struct timeval end_tv;

US_fe_nnls_t *our_us_fe_nnls_t;

double fitness_solute(node *n)
{
   double result;
   int nonzeros = 0;
   int i;
   unsigned int u;
   node *m;
   gettimeofday(&start_tv, NULL);
   stacks_init();
   evaluate(n, pass_fit_args);
   Solute solute;
   //  printf("%d: fitness_solute rsp %d\n", this_rank, sp[RESULT_STACK]); fflush(stdout);
   vector<Solute> solute_vector;
   for(i = sp[RESULT_STACK] - 2; i >= 0; i-=2)
   {
      solute.s = stack[RESULT_STACK][i];
      solute.k = stack[RESULT_STACK][i + 1];
      if(ga_mw)
      {
         solute.s =
            pow(pow((solute.s * our_us_fe_nnls_t->experiment[0].vbar20)/(AVOGADRO * M_PI), 2e0)/6e0,1e0/3e0) *
            (1e0 -  DENS_20W *
             our_us_fe_nnls_t->experiment[0].vbar20) /
            (3e0 * solute.k * our_us_fe_nnls_t->experiment[0].vbar20 * VISC_20W);
      }
      solute_vector.push_back(solute);
   }
   if(debug_level > 1)
   {
      for(u = 0; u < solute_vector.size(); u++)
      {
         printf("%d: solute_vector[%d] = %g %g\n", this_rank, u, solute_vector[u].s, solute_vector[u].k);
         fflush(stdout);
      }
   }
   unsigned int e;
   Simulation_values sv;
   vector<Simulation_values> sve;
   for(e = 0; e < our_us_fe_nnls_t->experiment.size(); e++)
   {
      sve.push_back(sv);
      vector <struct mfem_data> use_experiment;
      use_experiment.push_back(our_us_fe_nnls_t->experiment[e]);
      //  printf("%d: call calc_residuals %lx\n", this_rank, &our_us_fe_nnls_t->experiment); fflush(stdout);
      //   Simulation_values sv = our_us_fe_nnls_t->calc_residuals(our_us_fe_nnls_t->experiment, solute_vector, 0e0, 1);
      if(ga_sc) {
         if (solute_vector.size() != s_estimate_solutes) 
         {
            printf("%d: !! solute_vector.size() (%u) != s_estimate_solutes (%u)\n", 
                   this_rank, (unsigned int)solute_vector.size(), s_estimate_solutes); fflush(stdout);
            Simulation_values sv;
            vector<double> no_noise;
            vector<double> variances;
            sv.solutes = solute_vector;
            sv.variance = 1e99;
            variances.push_back(sv.variance);
            sv.ti_noise = no_noise;
            sv.ri_noise = no_noise;
            sve[e] = sv;
         } else {
            sve[e] = us_ga_interacting_calc(use_experiment, solute_vector, 0e0);
         }
      } else {
         sve[e] = our_us_fe_nnls_t->calc_residuals(use_experiment, solute_vector, 0e0, 1, e);
      }
      if(debug_level > 1)
      {
         printf("%d: exp %d variance %g\n", this_rank, e, sve[e].variance);
         fflush(stdout);
      }
      //  printf("%d: back from calc_residuals\n", this_rank); fflush(stdout);
   } // for e
   vector<Solute> tot_solutes = sve[0].solutes;
   result = sve[0].variance;
   if(our_us_fe_nnls_t->experiment.size() > 1)
   {
      for(e = 1; e < our_us_fe_nnls_t->experiment.size(); e++)
      {
         result += sve[e].variance;
         unsigned int f;
         for(f = 0; f < tot_solutes.size(); f++)
         {
            tot_solutes[f].c += sve[e].solutes[f].c;
         }
      }
   }
   ga_last_sve = sve;
   m = n;
   if(debug_level > 1)
   {
      printf("%d: results %g\n", this_rank, result);
      fflush(stdout);
   }
   //  printf("sv.variance %g\n", sv.variance);
   //  printf("sv.variance %g\n", sv.variance);
   if (!ga_sc) 
   {
      if (tot_solutes.size())
      {
         //   result = sv.variance;
         for(u = 0; u < tot_solutes.size(); u++)
         {
            //     printf("results tot_solutes[%d] = %g %g %g\n", u, tot_solutes[u].s, tot_solutes[u].k, tot_solutes[u].c);
            if(tot_solutes[u].c > SOLUTE_CONCENTRATION_THRESHOLD)
            {
               nonzeros++;
            }
            while(m && (!((char *)m->data)[SOLUTE_DATA_ACTIVE_OFS] ||
                        ((char *)m->data)[SOLUTE_DATA_PROX_FLAG]))
            {
               if(((char *)m->data)[SOLUTE_DATA_PROX_FLAG])
               {
                  ((double *)(m->data))[SOLUTE_DATA_CONCENTRATION] = 0e0;
               }
               m = m->children[0];
            }
            if(!m)
            {
               fprintf(stderr, "fitness_solute  unexpected termination\n");
               exit(-1);
            }
            ((double *)(m->data))[SOLUTE_DATA_CONCENTRATION] = tot_solutes[u].c;
            if(concentration_inactivate_prob &&
               ((double *)(m->data))[SOLUTE_DATA_CONCENTRATION] <= SOLUTE_CONCENTRATION_THRESHOLD &&
               drand48() < concentration_inactivate_prob)
            {
               ((double *)(m->data))[SOLUTE_DATA_CONCENTRATION] = 0e0;
               ((char *)(m->data))[SOLUTE_DATA_ACTIVE_OFS] = 0;
               //     puts("solute inactivated!");
            }
            m = m->children[0];
         }
      }
      else
      {
         result = 1e100;
      }
   } else {
      result = sqrt(result);
   }
   //  puts("fitness_solute done");
   gettimeofday(&end_tv, NULL);
   total_fitness_time += 1000000l * (end_tv.tv_sec - start_tv.tv_sec) +  end_tv.tv_usec - start_tv.tv_usec;
   if(!ga_sc && regularization_factor && nonzeros)
   {
      if(regularize_on_RMSD)
      {
         result += result * regularization_factor * nonzeros;
      }
      else
      {
         result += pow(sqrt(result) * regularization_factor * nonzeros, 2.0);
      }
   }
   //  printf("result regularized %g\n", result);
   //  fflush(stdout);
   //   printf("%d: result %g\n", this_rank, result); fflush(stdout);
   return result;
}

double solute_distance_l2(node *n)
{
   /*  double cmps[] = {
       9.0226e-13,
       1.8725e-7 ,
       5.9432e-13,
       1.6219e-7,
       4.2376e-13,
       7.3426e-7,
       };
   */

   double factor[] = {1e13, 1e7};
   double result = 0e0, l1 = 0e0;
   int i;
   stacks_init();
   evaluate(n, pass_fit_args);
   if(sp[RESULT_STACK] != l2_comps_size)
   {
      fitness_param_6 = 1e100;
      return(1e50);
   }
   /*  printf("cmp values sp[RESULT_STACK] %d:\n", sp[RESULT_STACK]);
       for(i = 0; i < sp[RESULT_STACK]; i++) {
       printf(" %g %g\n", stack[RESULT_STACK][i], l2_comps[i]);
       }
   */
   for(i = 0; i < sp[RESULT_STACK]; i++)
   {
      result += (factor[i % 2] * (stack[RESULT_STACK][i] - l2_comps[i])) * (factor[i % 2] * (stack[RESULT_STACK][i] - l2_comps[i]));
      if(fabs(factor[i % 2] * (stack[RESULT_STACK][i] - l2_comps[i])) > l1)
      {
         l1 = fabs(factor[i % 2] * (stack[RESULT_STACK][i] - l2_comps[i]));
      }
   }
   fitness_param_6 = l1;
   return(sqrt(result));
}

double solute_distance_from_target(node *n)
{
   double factor[] = {1e13, 1e7};

   double result = 0e0, l1 = 0e0;
   int i;
   stacks_init();
   evaluate(n, pass_fit_args);
   printf("final values sp[RESULT_STACK] %d:\n", sp[RESULT_STACK]);
   for(i = 0; i < sp[RESULT_STACK]; i++)
   {
      printf(" %g %g\n", stack[RESULT_STACK][i], l2_comps[i]);
   }
   puts("");
   if(sp[RESULT_STACK] == l2_comps_size)
   {
      for(i = 0; i < sp[RESULT_STACK]; i++)
      {
         result += (factor[i % 2] * (stack[RESULT_STACK][i] - l2_comps[i])) * (factor[i % 2] * (stack[RESULT_STACK][i] - l2_comps[i]));
         if(fabs(factor[i % 2] * (stack[RESULT_STACK][i] - l2_comps[i])) > l1)
         {
            l1 = fabs(factor[i % 2] * (stack[RESULT_STACK][i] - l2_comps[i]));
         }
      }
   }
   else
   {
      result = l1 = 1e100;
   }
   printf("final value l2 %g\n", sqrt(result));
   printf("final value l1 %g\n", l1);
   fitness_param_6 = l1;

   return(sqrt(result));
}

double fitness_final_check_solute(node *n)
{
   double result;
   //  int i;
   stacks_init();
   evaluate(n, pass_fit_args);
   // todo new fitness stuff
   /*
     if(use_constants[FITNESS_CONSTANT_1]) {
     push_stack(CONSTANTS_STACK, FITNESS_CONSTANT_1);
     }
     if(use_constants[FITNESS_CONSTANT_NEG_1]) {
     push_stack(CONSTANTS_STACK, FITNESS_CONSTANT_NEG_1);
     }
     result = mfem_fitness(1);
     printf("best fitness %g\n", result);
     puts("constant concentrations stack:");
     print_stack(CONSTANTS_STACK);
     printf("concentrations of best fit:");
     for(i = 0; i < sp[RESULT_STACK]; i++) {
     printf(" %g", stack[RESULT_STACK][i]);
     }
     puts("");
     printf("used composite grid %ld schuck grid %ld\n", used_composite_grid, used_schuck_grid );
     printf("freads %u fwrites %u fopens %u\n", freads, fwrites, fopens);
     printf("diskcache hits %u misses %u total %u\n", diskcache_hits, diskcache_misses, diskcache_hits + diskcache_misses);
     printf("total fitness time usecs %lu\n", total_fitness_time);
     fitness_param_5 = solute_distance_from_target(n);
     fitness_out_1 = fitness_calls;
     fitness_out_2 = fitness_mfem_calls;
     fitness_out_3 = inverse_hessian_prob;
     fitness_out_4 = inverse_hessian_iter;
     fitness_out_5 = inverse_hessian_h;
     fitness_out_6 = conjugate_gradient_prob;
     fitness_out_7 = conjugate_gradient_iter;
     fitness_out_8 = conjugate_gradient_h;
     fitness_out_9 = steepest_descent_prob;
     fitness_out_10 = steepest_descent_iter;
     fitness_out_11 = steepest_descent_h;
     fitness_out_12 = elitist_gsm_generation_start;
   */
   result = 1e5;
   return result;
}

void solute_test_perfect_fitness_run()
{
   FILE *load_file;
   char line[MAX_INPUT_LIN_LENGTH];
   char *p;
   double s, k;
   int i;

   puts("test perfect fitness run");
   stacks_init();

   if(NULL == (load_file = fopen("save_concentration", "r")))
   {
      fprintf(stderr, "save_concentration file open error\n");
      exit(-1);
   }

   while(fgets(line, sizeof(line), load_file) != NULL)
   {
      if(*line != '#')
      {
         s = atof(line);
         p = strchr(line, ' ');
         if(!p)
         {
            fprintf(stderr, "save_concentration file format error\n");
            exit(-1);
         }
         p++;
         k = atof(p);
         // p = strchr(p, ' ');
         //     if(!p) {
         //   fprintf(stderr, "save_concentration file format error\n");
         //   exit(-1);
         //     }
         //     p++;
         //     conc = atof(p);

         //     printf("adding s %g d %g conc %g\n", s, d, conc);
         //     k =  k_calc(s, d, use_vbar);
         // todo fix this!
         printf("adding s %g k %g\n", s, k);

         push_stack(RESULT_STACK, s);
         push_stack(RESULT_STACK, k);
         //     push_stack(CONCENTRATION_STACK, conc);
      }
   }
   puts("mfem_fitness");
   printf("target fitness params:");
   for(i = 0; i < sp[RESULT_STACK]; i++)
   {
      printf(" %g", stack[RESULT_STACK][i]);
   }
   puts("");
   // todo new fitness
   /*
     if(use_constants[FITNESS_CONSTANT_1]) {
     push_stack(CONSTANTS_STACK, FITNESS_CONSTANT_1);
     }
     if(use_constants[FITNESS_CONSTANT_NEG_1]) {
     push_stack(CONSTANTS_STACK, FITNESS_CONSTANT_NEG_1);
     }
     printf("run %d fitness = %g\n", i, mfem_fitness(1));
     puts("constant concentrations stack:");
     print_stack(CONSTANTS_STACK);
     printf("concentrations of best fit sp=%d:", sp[RESULT_STACK]);
     for(i = 0; i < sp[RESULT_STACK]; i++) {
     printf(" %g", stack[RESULT_STACK][i]);
     }
     puts("");
     printf("used composite grid %ld schuck grid %ld\n", used_composite_grid, used_schuck_grid );
     printf("freads %u fwrites %u fopens %u\n", freads, fwrites, fopens);
     printf("total fitness time usecs %lu\n", total_fitness_time);
   */
   //  exit(0);
}

void fitness_test()
{
   //  int i = 0;
   //  int j = 0;
   // todo new fitness stuff
   /*
     if(fitness_param_4 == 1e0) {
     for(j = 0; j < 3; j++) {
     solute_test_perfect_fitness_run();
     }
     }
     if(fitness_param_4 == 2e0) {
     int i;
     double s, k;
     for(i = 0; i < 3; i++) {
     printf("solute %d\n", i);
     for(s = floorn(s_estimates[i][0], 1e-13, 2); s <= s_estimates[i][1]; s += .1e-13) {
     for(k = min_f_ratio; k <= max_f_ratio ; k += .1) {
     printf("%g|%g|", s, k);
     fflush(stdout);
     stacks_init();
     if(i == 0) {
     push_stack(RESULT_STACK, s);
     push_stack(RESULT_STACK, k);
     } else {
     push_stack(RESULT_STACK, 4.2376e-13);
     push_stack(RESULT_STACK, k_calc(4.2376e-13, 7.3426e-7, use_vbar));
     }
     if(i == 1) {
     push_stack(RESULT_STACK, s);
     push_stack(RESULT_STACK, k);
     } else {
     push_stack(RESULT_STACK, 5.9432e-13);
     push_stack(RESULT_STACK, k_calc(5.9432e-13, 1.6219e-7, use_vbar));
     }
     if(i == 2) {
     push_stack(RESULT_STACK, s);
     push_stack(RESULT_STACK, k);
     } else {
     push_stack(RESULT_STACK, 9.0226e-13);
     push_stack(RESULT_STACK, k_calc(9.0226e-13, 1.8725e-7, use_vbar)); 
     }
     if(use_constants[FITNESS_CONSTANT_1]) {
     push_stack(CONSTANTS_STACK, FITNESS_CONSTANT_1);
     }
     if(use_constants[FITNESS_CONSTANT_NEG_1]) {
     push_stack(CONSTANTS_STACK, FITNESS_CONSTANT_NEG_1);
     }
     printf("%g\n", mfem_fitness(1));
     puts("constant concentrations stack:");
     print_stack(CONSTANTS_STACK);
     }
     }
     }
     }
     if(fitness_param_4 == 3e0) {
     puts("save concentration run");
     stacks_init();
     FILE *load_file;
     char line[MAX_INPUT_LIN_LENGTH];
     char *p;
     double s, d, conc;

     if(NULL == (load_file = fopen("save_concentration", "r"))) {
     fprintf(stderr, "save_concentration file open error\n");
     exit(-1);
     }

     while(fgets(line, sizeof(line), load_file) != NULL) {
     if(*line != '#') {
     s = atof(line);
     p = strchr(line, ' ');
     if(!p) {
     fprintf(stderr, "save_concentration file format error\n");
     exit(-1);
     }
     p++;
     d = atof(p);
     p = strchr(p, ' ');
     if(!p) {
     fprintf(stderr, "save_concentration file format error\n");
     exit(-1);
     }
     p++;
     conc = atof(p);

     printf("adding s %g d %g conc %g\n", s, d, conc);

     push_stack(RESULT_STACK, s);
     push_stack(RESULT_STACK, d);
     push_stack(CONCENTRATION_STACK, conc);
     }
     }
     mfem_save_concentration();
     }

     if(fitness_param_4 == 4e0) {
     int i;
     double s, k;
     printf("solute %d\n", i);
     for(s = floorn(min_s, 1e-13, 2); s <= max_s; s += .05e-13) {
     for(k = min_f_ratio; k <= max_f_ratio ; k += .05) {
     printf("%g|%g|", s, D_calc(s, use_vbar, k));
     fflush(stdout);
     stacks_init();
     push_stack(RESULT_STACK, s);
     push_stack(RESULT_STACK, k);
     if(use_constants[FITNESS_CONSTANT_1]) {
     push_stack(CONSTANTS_STACK, FITNESS_CONSTANT_1);
     }
     if(use_constants[FITNESS_CONSTANT_NEG_1]) {
     push_stack(CONSTANTS_STACK, FITNESS_CONSTANT_NEG_1);
     }
     printf("%g", mfem_fitness(1));
     //   puts("constant concentrations stack:");
     for(i = 0; i < sp[RESULT_STACK]; i++) {
     printf("|%g", stack[RESULT_STACK][i]);
     }
     puts("");
     }
     }
     }
   */
}

#define fitness(x) fitness_solute(x)
#define fitness_init() fitness_init_solute()
#define fitness_final_check(x) fitness_final_check_solute(x)

int partition(double *, population **, int, int);

void quicksort(double *A, population *pop[], int p, int r)
{
   int q;
   if(p < r)
   {
      q = partition(A, pop, p,r);
      quicksort(A, pop, p, q-1);
      quicksort(A, pop, q+1, r);
   }
}

int partition(double *A, population *pop[], int p, int r)
{

   double tmp;
   double x = A[r];

   population *tmp_pop;
   population *x_pop = pop[r];

   int i = p-1;
   int j;
   for(j = p; j < r; j++)
   {
      if(A[j] <= x)
      {
         i++;

         tmp = A[i];
         A[i] = A[j];
         A[j] = tmp;

         tmp_pop = pop[i];
         pop[i] = pop[j];
         pop[j] = tmp_pop;
      }
   }
   i++;
   A[r] = A[i];
   A[i] = x;

   pop[r] = pop[i];
   pop[i] = x_pop;
   return i;
}

int partition_hash(double *, unsigned int *, population **, int, int);

void quicksort_hash(double *A, unsigned int *B, population *pop[], int p, int r)
{
   int q;
   if(p < r)
   {
      q = partition_hash(A, B, pop, p,r);
      quicksort_hash(A, B, pop, p, q-1);
      quicksort_hash(A, B, pop, q+1, r);
   }
}

int partition_hash(double *A, unsigned int *B, population *pop[], int p, int r)
{
   double tmp;
   double x = A[r];

   int tmp_B;
   unsigned int x_B = B[r];

   population *tmp_pop;
   population *x_pop = pop[r];

   int i = p-1;
   int j;
   for(j = p; j < r; j++)
   {
      if(A[j] < x || (A[j] == x && B[j] <= x_B))
      {
         i++;

         tmp = A[i];
         A[i] = A[j];
         A[j] = tmp;

         tmp_B = B[i];
         B[i] = B[j];
         B[j] = tmp_B;

         tmp_pop = pop[i];
         pop[i] = pop[j];
         pop[j] = tmp_pop;
      }
   }
   i++;
   A[r] = A[i];
   A[i] = x;

   B[r] = B[i];
   B[i] = x_B;

   pop[r] = pop[i];
   pop[i] = x_pop;
   return i;
}

node *dup_tree(node *n)
{
   int i;
   node *dup = new_node();
   if(!n)
   {
      fprintf(stderr, "dup_tree called with null node\r\n");
      exit(-1);
   }
   dup->e = n->e;
   if(n->data)
   {
      if((dup->data = (void *)malloc(n->data_size)) == NULL)
      {
         fprintf(stderr, "dup_tree malloc of size %d failed\n", n->data_size);
         exit(-1);
      }
      dup->data_size = n->data_size;
      memcpy(dup->data, n->data, dup->data_size);
   }
   for(i = 0; i < n->e->argc; i++)
   {
      if(n->children[i])
      {
         dup->children[i] = dup_tree(n->children[i]);
      }
      else
      {
         dup->argv[i] = n->argv[i];
      }
   }
   return dup;
}

void pstats(FILE *fp)
{
   fprintf(fp, "%d: crossover %lu mutate %lu replicate %lu subtree_duplicate %lu beta_overflow %lu\n",
           this_rank,
           crossover_count,
           mutate_count,
           replicate_count,
           subtree_duplicate_count,
           beta_overflow);
}

node *replicate(population *mom)
{
   node *spawn = dup_tree(mom->root);
   replicate_count++;
   return spawn;
}

node *subtree_duplicate(population *mom)
{
   /* ok, it's just replication for now */
   node *spawn = dup_tree(mom->root);
   subtree_duplicate_count++;
   return spawn;
}

node *mutate(population *mom)
{

   mutate_count++;
   if(mom->points)
   {
      node *spawn = dup_tree(mom->root);
      population *tmp_pop = insert_population_node((population *)0, spawn);
      int spawn_pos = (int)(drand48() * tmp_pop->points);
      node *spawn_remove_node = tmp_pop->nodes[spawn_pos];

      int spawn_arg = spawn_pos - spawn_remove_node->pos;

      if(spawn_remove_node->children[spawn_arg])
         free_tree(spawn_remove_node->children[spawn_arg]);

      spawn_remove_node->children[spawn_arg] = new_random_tree(mutate_tree_depth);

      free(tmp_pop->nodes);
      free(tmp_pop);
      return spawn;
   }
   else
   {
      return(new_random_tree(mutate_tree_depth));
   }
   return 0;
}

node *crossover(population *mom, population *dad)
{

   int counter = 0;
   //  printf("crossover mom <points %d>:\n", mom->points);
   //  list_tree(mom->root);
   //  printf("\ncrossover dad <points %d>:\n", dad->points);
   //  list_tree(dad->root);
   //  puts("");

   crossover_count++;

   if(mom->points)
   { // mom has non-zero size, ie isn't a root terminal
      node *spawn = dup_tree(mom->root);
      population *tmp_pop = insert_population_node((population *)0, spawn);
      int spawn_pos = (int)(drand48() * tmp_pop->points);
      int dad_pos;
      node *spawn_remove_node;
      int spawn_arg;
      //   printf("mom(spawn) pos %d\n", spawn_pos);
      do
      {
         dad_pos = (int)(drand48() * (1 + dad->points)) - 1; // - 1 to allow root selection
         counter++;
         if(counter > 100000)
         {
            fprintf(stderr, "stuck trying to find non-root element in crossover\r\n");
            exit(-1);
         }
      }
      while(dad_pos < 0 && dad->root->e->rooted);
      //   printf("dad pos %d\n", dad_pos);
      spawn_remove_node = tmp_pop->nodes[spawn_pos]; // one from the duped mom
      spawn_arg = spawn_pos - spawn_remove_node->pos;  // which argument to moms duped node will be replaced ->pos is
      // the base position.
      if(spawn_remove_node->children[spawn_arg])         // is the selected position a non-terminal
         free_tree(spawn_remove_node->children[spawn_arg]);

      if(dad_pos >= 0)
      {                           // does dad have anything besides a root?
         node *dad_insert_node = dad->nodes[dad_pos];
         int dad_arg = dad_pos - dad_insert_node->pos;

         if(dad_insert_node->children[dad_arg])
         {
            node * tmptree = dup_tree(dad_insert_node->children[dad_arg]);
            spawn_remove_node->children[spawn_arg] = tmptree;
         }
         else
         {
            spawn_remove_node->children[spawn_arg] = (node *)0;
            spawn_remove_node->argv[spawn_arg] = dad_insert_node->argv[dad_arg];
         }
      }
      else
      {
         spawn_remove_node->children[spawn_arg] = dup_tree(dad->root);
      }
      free(tmp_pop->nodes);
      free(tmp_pop);
      return spawn;
   }
   else
   {
      int dad_pos;
      do
      {
         dad_pos = (int)(drand48() * (1 + dad->points)) - 1; // - 1 to allow root selection
         counter++;
         if(counter > 100000)
         {
            fprintf(stderr, "stuck trying to find non-root element in crossover\r\n");
            exit(-1);
         }
      }
      while(dad_pos < 0 && dad->root->e->rooted);
      if(dad_pos >= 0)
      {
         node *dad_insert_node = dad->nodes[dad_pos];
         int dad_arg = dad_pos - dad_insert_node->pos;

         if(dad_insert_node->children[dad_arg])
         {  // is there a tree here?
            return(dup_tree(dad_insert_node->children[dad_arg]));
         }
         else
         {
            return(dup_tree(dad_insert_node));
         }
      }
      return(dup_tree(dad->root));
   }
   return 0;
}


node *crossover_1pt(population *mom, population *dad)
{
   // assume GA style population... ie. completely single terminal
   // get a random point one less than the length of the mom
   // remove the subtree
   // go to the same pos on dad & make a copy

   //  int counter = 0;

   crossover_count++;
   if(mom->points < 2 ||
      mom->points != dad->points)
   {
      fprintf(stderr, "%d: crossover 1pt mismatch or length < 2\n", this_rank);
      MPI_Abort(MPI_COMM_WORLD, -24);
   }

   {
      node *spawn = dup_tree(mom->root);
      population *tmp_pop = insert_population_node((population *)0, spawn);
      int spawn_pos = (int)(drand48() * (tmp_pop->points - 1));
      node *spawn_remove_node;
      int spawn_arg;

      spawn_remove_node = tmp_pop->nodes[spawn_pos]; // one from the duped mom
      spawn_arg = spawn_pos - spawn_remove_node->pos;  // which argument to moms duped node will be replaced ->pos is
      // the base position.
      if(spawn_remove_node->children[spawn_arg])         // is the selected position a non-terminal
         free_tree(spawn_remove_node->children[spawn_arg]);

      {
         node *dad_insert_node = dad->nodes[spawn_pos];
         int dad_arg = spawn_pos - dad_insert_node->pos;

         if(dad_insert_node->children[dad_arg])
         {
            node * tmptree = dup_tree(dad_insert_node->children[dad_arg]);
            spawn_remove_node->children[spawn_arg] = tmptree;
         }
         else
         {
            spawn_remove_node->children[spawn_arg] = (node *)0;
            spawn_remove_node->argv[spawn_arg] = dad_insert_node->argv[dad_arg];
         }
      }
      free(tmp_pop->nodes);
      free(tmp_pop);
      return spawn;
   }
}

node *crossover_2pt(population *mom, population *dad)
{
   // assume GA style population... ie. completely single terminal
   // get a random point one less than the length of the mom
   // remove the subtree
   // go to the same pos on dad & make a copy

   //  int counter = 0;
   fprintf(stderr, "crossover 2pt not implemented\n");
   exit(-1);

   crossover_count++;
   if(mom->points < 2 ||
      mom->points != dad->points)
   {
      fprintf(stderr, "crossover 1pt mismatch or length < 2\n");
      exit(-1);
   }

   {
      node *spawn = dup_tree(mom->root);
      population *tmp_pop = insert_population_node((population *)0, spawn);
      int spawn_pos = (int)(drand48() * (tmp_pop->points - 1));
      node *spawn_remove_node;
      int spawn_arg;

      spawn_remove_node = tmp_pop->nodes[spawn_pos]; // one from the duped mom
      spawn_arg = spawn_pos - spawn_remove_node->pos;  // which argument to moms duped node will be replaced ->pos is
      // the base position.
      if(spawn_remove_node->children[spawn_arg])         // is the selected position a non-terminal
         free_tree(spawn_remove_node->children[spawn_arg]);

      {
         node *dad_insert_node = dad->nodes[spawn_pos];
         int dad_arg = spawn_pos - dad_insert_node->pos;

         if(dad_insert_node->children[dad_arg])
         {
            node * tmptree = dup_tree(dad_insert_node->children[dad_arg]);
            spawn_remove_node->children[spawn_arg] = tmptree;
         }
         else
         {
            spawn_remove_node->children[spawn_arg] = (node *)0;
            spawn_remove_node->argv[spawn_arg] = dad_insert_node->argv[dad_arg];
         }
      }
      free(tmp_pop->nodes);
      free(tmp_pop);
      return spawn;
   }
}

void node_mutate(node *n)
{
   int i;
   n->e->node_mutate((void *)n);
   for(i = 0; i < n->e->argc; i++)
   {
      if(n->children[i])
      {
         node_mutate(n->children[i]);
      }
   }
}

void point_node_mutate(node *n, int points)
{
   int i;
   int pos = (int)(drand48() * points);
   double save_pct_node_mutation = pct_node_mutation;
   if(debug_level > 2)
   {
      printf("pos to mutate %d\n", pos);
   }
   for(i = 0; i < pos; i++)
   {
      if(!(n = n->children[0]))
      {
         fprintf(stderr, "point node mutate overflow!\n");
         exit(-1);
      }
   }
   pct_node_mutation = 100;
   n->e->node_mutate((void *)n);
   pct_node_mutation = save_pct_node_mutation;
   return;
}

int node_inactivate(population *p)
{
   int i;
   node *n = p->root;
   if(p->active_points > 1)
   {
      i = (int)floor(p->points * drand48()); // pick a s range
      if(debug_level > 2)
      {
         printf("%d: node had %d points will inactivate # %d\n", this_rank, p->points, i);
      }
      n = p->root;
      while(i > 0)
      {
         if(!(n = n->children[0]))
         {
            fprintf(stderr, "node_inactivate error, no child!\n");
            exit(-1);
         }
         i--;
      }
      if(((char *)n->data)[SOLUTE_DATA_ACTIVE_OFS])
      {
         ((char *)n->data)[SOLUTE_DATA_ACTIVE_OFS] = 0;
         ((double *)n->data)[SOLUTE_DATA_CONCENTRATION] = 0e0;
         p->active_points--;
         return 1;
      }
      else
      {
         if(debug_level > 2)
         {
            printf("inactive already inactive\n");
         }
      }
   }
   else
   {
      if(debug_level > 2)
      {
         printf("node had %d points only %d active, will skip\n", p->points, p->active_points);
      }
   }
   return 0;
}

int pop_selection()
{
   /* exponential ranking selection */
   int pos;
   selection_count++;
   pos = (int)( - log(1e0 - drand48()) * beta);
   if(pos >= pop_size)
   {
      pos = pop_size - 1;
      beta_overflow++;
   };
   if(pos < 0)
   {
      fprintf(stderr, "ransom pop_selection less than zero, ugh!\n");
      exit(-1);
   }
   return pos;
}

void print_stats()
{
   printf(
          "%d: %d max generations\n"

          "%d: %d population size\n"

          "%d: %d crossover percent\n"
          "%d: %.4g point mutation percent\n"
          "%d: %d subtree dup percent\n"
          "%d: %d replication percent\n"

          "%d: %d number of terminals (external variables)\n"

          "%d: %d new tree depth\n"
          "%d: %d mutate tree depth\n"
          "%d: %.4g migration probability\n"
          "%d: %.4g regularization\n"
          ,
          this_rank, max_generations,
          this_rank, pop_size,
          this_rank, pct_crossover,
          this_rank, pct_point_mutation,
          this_rank, pct_subtree_dup,
          this_rank, pct_replication,
          this_rank, max_arg,
          this_rank, new_tree_depth,
          this_rank, mutate_tree_depth,
          this_rank, migrate_prob,
          this_rank, regularization_factor
          );
   fflush(stdout);
}

void save_every_this_pop(population *p, int g)
{
   if(p->active_points > 0)
   {
      int j;
      int active_points = 0;
      node *m = p->root;
      while(m)
      {
         while(m && (!((char *)m->data)[SOLUTE_DATA_ACTIVE_OFS] ||
                     ((double *)m->data)[SOLUTE_DATA_CONCENTRATION] <= SOLUTE_CONCENTRATION_THRESHOLD)
               )
         {
            m = m->children[0];
         }
         if(m)
         {
            active_points++;
            m = m->children[0];
         }
      }
      if(active_points)
      {
         m = p->root;
         fprintf(save_every_fitness_file, "%d|%g|%d", g, p->fitness , active_points);
         for(j = 0; j < FITNESS_NO_OF_CONSTANTS; j++)
         {
            fprintf(save_every_fitness_file, "|%g", ((double *)p->data)[j]);
         }
         while(m)
         {
            while(m && (!((char *)m->data)[SOLUTE_DATA_ACTIVE_OFS] ||
                        ((double *)m->data)[SOLUTE_DATA_CONCENTRATION] <= SOLUTE_CONCENTRATION_THRESHOLD)
                  )
            {
               m = m->children[0];
            }
            if(m)
            {
               fprintf(save_every_fitness_file, "|%g|%g|%g" ,
                       ((double *)m->data)[0],
                       ((double *)m->data)[1],
                       ((double *)m->data)[SOLUTE_DATA_CONCENTRATION]);
               m = m->children[0];
            }
         }
         fprintf(save_every_fitness_file, "\n");
      }
   }
}

// MPI deme migration utilities

Simulation_values node_to_sv(node *n)
{
   stacks_init();
   evaluate(n, pass_fit_args);
   Solute solute;
   vector<Solute> solute_vector;
   int i;
   for(i = sp[RESULT_STACK] - 2; i >= 0; i-=2)
   {
      solute.s = stack[RESULT_STACK][i];
      solute.k = stack[RESULT_STACK][i + 1];
      //   if(ga_mw) {
      //   solute.s =
      //      pow(pow((solute.s * our_us_fe_nnls_t->experiment[0].vbar20)/(AVOGADRO * M_PI), 2e0)/6e0,1e0/3e0) *
      //      (1e0 - our_us_fe_nnls_t->experiment[0].density *
      //       our_us_fe_nnls_t->experiment[0].vbar20) /
      //      (3e0 * solute.k * our_us_fe_nnls_t->experiment[0].vbar20 *
      //       our_us_fe_nnls_t->experiment[0].viscosity * .01);
      //   }
      solute_vector.push_back(solute);
   }
   Simulation_values sv;
   vector<Simulation_values> sve;
   unsigned int e;
   for(e = 0; e < our_us_fe_nnls_t->experiment.size(); e++)
   {
      sve.push_back(sv);
      vector <struct mfem_data> use_experiment;
      use_experiment.push_back(our_us_fe_nnls_t->experiment[e]);
      if(ga_sc) {
         if (solute_vector.size() != s_estimate_solutes) 
         {
            printf("%d: !! solute_vector.size() (%u) != s_estimate_solutes (%u)\n", 
                   this_rank, (unsigned int)solute_vector.size(), s_estimate_solutes); fflush(stdout);
            Simulation_values sv;
            vector<double> no_noise;
            vector<double> variances;
            sv.solutes = solute_vector;
            sv.variance = 1e99;
            variances.push_back(sv.variance);
            sv.ti_noise = no_noise;
            sv.ri_noise = no_noise;
            sve[e] = sv;
         } else {
            sve[e] = us_ga_interacting_calc(use_experiment, solute_vector, 0e0);
         }
      } else {
         sve[e] = our_us_fe_nnls_t->calc_residuals(use_experiment, solute_vector, 0e0, 1, e);
      }
   } // for e
   if(our_us_fe_nnls_t->experiment.size() > 1)
   {
      for(e = 1; e < our_us_fe_nnls_t->experiment.size(); e++)
      {
         unsigned int f;
         sve[0].variance += sve[e].variance;
         for(f = 0; f < sve[0].solutes.size(); f++)
         {
            sve[0].solutes[f].c += sve[e].solutes[f].c;
         }
      }
   }
   return(sve[0]);
}

vector<Solute> node_to_solute_vector(node *n)
{
   stacks_init();
   evaluate(n, pass_fit_args);
   Solute solute;
   vector<Solute> solute_vector;
   int i;
   for(i = sp[RESULT_STACK] - 2; i >= 0; i-=2)
   {
      solute.s = stack[RESULT_STACK][i];
      solute.k = stack[RESULT_STACK][i + 1];
      //   if(ga_mw) {
      //   solute.s =
      //      pow(pow((solute.s * our_us_fe_nnls_t->experiment[0].vbar20)/(AVOGADRO * M_PI), 2e0)/6e0,1e0/3e0) *
      //      (1e0 - our_us_fe_nnls_t->experiment[0].density *
      //       our_us_fe_nnls_t->experiment[0].vbar20) /
      //      (3e0 * solute.k * our_us_fe_nnls_t->experiment[0].vbar20 *
      //       our_us_fe_nnls_t->experiment[0].viscosity * .01);
      //   }
      solute_vector.push_back(solute);
   }
   return(solute_vector);
}

void list_node_as_solute_vector(node *n)
{
   vector<Solute> solute_vector = node_to_solute_vector(n);
   for(unsigned int i = 0; i < solute_vector.size(); i++)
   {
      printf("%d: best solute %d %.4g %.4g\n",
             this_rank, i, solute_vector[i].s, solute_vector[i].k);
   }
   fflush(stdout);
}

// two migrate ins' for a ring topo
#define MIGRATE_IN_BUFS   2
#define MIGRATE_OUT      2
#define MIGRATE_BUFS     3

char *migrate_buffer[MIGRATE_BUFS];
unsigned int migrate_size[MIGRATE_BUFS];
unsigned int migrate_end_pos[MIGRATE_BUFS];
unsigned int migrate_read_pos[MIGRATE_BUFS];

void alloc_migrate_buffers()
{
   int use_migration_size = (int)rint(pop_size * migrate_prob * 8);
   if(use_migration_size < 32)
   {
      use_migration_size = 32;
   }

   migrate_size[0] =
      use_migration_size *
      s_estimate_solutes * // # of nodes in tree
      ( // size of each node
       1 + // '_' header
       sizeof(int) + // MAX_BR
       sizeof(int) + // n->e->id
       sizeof(int) + // n->e->argc
       sizeof(int) + // n->pos
       sizeof(int) + // n->data_size
       SOLUTE_DATA_SIZE
       );

   int i;
   if(debug > 0)
   {
      printf("%d: migrate buffer size = %d\n", this_rank, migrate_size[0]);
      fflush(stdout);
   }
   for(i = 1; i < MIGRATE_BUFS; i++)
   {
      migrate_size[i] = migrate_size[0];
   }
   for(i = 0; i < MIGRATE_BUFS; i++)
   {
      if((migrate_buffer[i] = (char *)malloc(migrate_size[i])) == NULL)
      {
         fputs("migrate buffers malloc failure\r\n", stderr);
         MPI_Abort(MPI_COMM_WORLD, -1);
         exit(-1);
      }
      migrate_read_pos[i] =
         migrate_end_pos[i] = 0;
   }
}

void bufrewind(int buf)
{
   migrate_read_pos[buf] = 0;
}

void bufinit(int buf)
{
   migrate_read_pos[buf] = 0;
   migrate_end_pos[buf] = 0;
}

void bufwrite(void *src, int size, int buf)
{
   if(size + migrate_end_pos[buf] > migrate_size[buf])
   {
      fputs("migrate buffers write overflow\r\n", stderr);
      MPI_Abort(MPI_COMM_WORLD, -2);
      exit(-1);
   }
   memcpy(&migrate_buffer[buf][migrate_end_pos[buf]], src, size);
   migrate_end_pos[buf] += size;
}

int bufread(void *dest, int size, int buf)
{
   int avail_size = migrate_end_pos[buf] - migrate_read_pos[buf];
   if(size > avail_size)
   {
      size = avail_size;
   }
   memcpy(dest, &migrate_buffer[buf][migrate_read_pos[buf]], size);
   migrate_read_pos[buf] += size;
   return(size);
}

char bufgetc(int buf)
{
   char c;
   if(bufread(&c, 1, buf))
   {
      return c;
   }
   return 0;
}

void bufputc(char c, int buf)
{
   bufwrite(&c, 1, buf);
}

void buffer_serialize_out_tree(int buf, node *n)
{
   int i, tmp;
   char c;
   bufputc('_', buf);
   tmp = MAX_BR;
   bufwrite(&tmp, sizeof(tmp), buf);
   bufwrite(&n->e->id, sizeof(n->e->id), buf);
   bufwrite(&n->e->argc, sizeof(n->e->argc), buf);
   for(i = 0; i < n->e->argc; i++)
   {
      c = n->children[i] ? 1 : 0;
      bufwrite(&c, 1, buf);
      if(n->children[i])
      {
         buffer_serialize_out_tree(buf, n->children[i]);
      }
      bufwrite(&n->argv[i], sizeof(n->argv[i]), buf);
   }
   bufwrite(&n->pos, sizeof(n->pos), buf);
   bufwrite(&n->data_size, sizeof(n->data_size), buf);
   bufwrite(n->data, n->data_size, buf);
}

node *buffer_serialize_in_tree(int buf)
{
   // returns newly allocated node
   int i, tmp;
   node *n;
   tmp = bufgetc(buf);
   if(tmp == '.')
   {
      migrate_read_pos[buf]--; // to protect for multiple reads on end
      return (node *)0;
   }
   if(tmp != '_')
   {
      fprintf(stderr, "identifier mismatch in serialize_in_tree\n");
      exit(-1);
   }
   bufread(&tmp, sizeof(tmp), buf);
   if(tmp != MAX_BR)
   {
      fprintf(stderr, "MAX_BR mismatch in serialize_in_tree\n");
      exit(-1);
   }
   n = new_node();
   bufread(&tmp, sizeof(n->e->id), buf);
   //  printf("element %d\n", tmp);
   if(tmp > max_e || tmp < 0)
   {
      fprintf(stderr, "element greater than max or negative in serialize_in_tree\n");
      exit(-1);
   }
   n->e = &elements[tmp];
   if(n->e->id != tmp)
   {
      fprintf(stderr, "unexpected id mismatch in serialize_in_tree\n");
      exit(-1);
   }
   bufread(&tmp, sizeof(n->e->argc), buf);
   if(n->e->argc != tmp)
   {
      fprintf(stderr, "unexpected argc mismatch in serialize_in_tree\n");
      exit(-1);
   }
   for(i = 0; i < n->e->argc; i++)
   {
      if(bufgetc(buf))
      {
         if(!(n->children[i] = buffer_serialize_in_tree(buf)))
         {
            fprintf(stderr, "premature eof in serialize_in_tree\n");
            free_node(n);
            return((node *)0);
         }
      }
      else
      {
         n->children[i] = 0;
      }
      bufread(&n->argv[i], sizeof(n->argv[i]), buf);
   }
   bufread(&n->pos, sizeof(n->pos), buf);
   bufread(&n->data_size, sizeof(n->data_size), buf);
   //  printf("data size %d\n", n->data_size);
   if(n->data_size)
   {
      if((n->data = (void *)malloc(n->data_size)) == NULL)
      {
         fprintf(stderr, "serialize in tree malloc of size %d failed\n", n->data_size);
         exit(-1);
      }
      bufread(n->data, n->data_size, buf);
   }
   else
   {
      n->data = (void *)0;
   }
   return n;
}

void buffer_list_serial(int buf)
{
   node *n;
   int i = 0;
   bufrewind(buf);
   while((n = buffer_serialize_in_tree(buf)))
   {
      printf("record %d:", i++);
      list_tree(n);
      puts("");
      free_tree(n);
   }
}

void buffer_close_serial_output(int buf)
{
   bufputc('.', buf);
}

void buffer_close_serial_input(int buf)
{
   bufrewind(buf);
}

void generations(double *A1, unsigned int *B1, population *pn1[],
                 double *A2, unsigned int *B2, population *pn2[],
                 int max_g)
{
   /*
    * First, A1 is the initial pop population, then we will make 
    *  A2 our new generation & reverse our pointers.  Locally this
    *  will be A & A_next, with A_tmp for a swap location
    * similarly for B & pn, but we'll only keep one 'population' list
    * which we'll delete from.
    */
   int return_stat;
   double *A, *A_next, *A_tmp;
   unsigned int *B, *B_next, *B_tmp;
   population **pn, **pn_next, **pn_tmp;
   int i;
   int g = 0;
   population *p;
   population *p_migrate_in = (population *)0;
   population **p_migrate_in_n = (population **)0;
   int p_migrate_in_size = 0;
   node *n_migrate_in;
   int last_pos;
   double tot_individual_size;
   double avg_individual_size;
   int best[elitist_gsm + 1][new_tree_depth];  // ok, there's an extra row for when elitism gsm is off
   int best_index[new_tree_depth];
   char gsm_this_index[pop_size];
   char use_gsm_type[GSM_MAX_SEARCH_TYPE];
   char any_gsm;
   double best_fitness = 1e99;
   MPI_GA_Work_Msg mpi_ga_msg_in, mpi_ga_msg_out;
   MPI_Status mpi_status;

   A = A1;
   B = B1;
   pn = pn1;

   A_next = A2;
   B_next = B2;
   pn_next = pn2;

   int all_last_gen = 0;
   if(max_g)
   {
      for(g = 0; !all_last_gen; g++)
      {
         //   if(this_rank == 1 && g == max_generations) {
         //     printf("1: sleeping 30\n"); fflush(stdout);
         //     sleep(30);
         //     printf("1: done sleeping 30\n"); fflush(stdout);
         //   }

         this_generation = g;

         if(fitness_reinit && !(g % fitness_reinit))
         {
            fitness_init();
            if(skip_fitness)
            {
               for(i = 0; i < pop_size; i++)
               {
                  pn[i]->fitness_valid = 0;
               }
            }
         }

         /* selection: sort by fitness */
         //   printf("%d: generation %d sort 1\n", this_rank, g);

         if(debug_level > 4)
         {
            double save_prob = concentration_inactivate_prob;
            concentration_inactivate_prob = 0;
            puts("debug_level 5a");
            for(i = 0; i < pop_size; i++)
            {
               printf("%d %g <%g> %d %d %u ", i, A[i], fitness(pn[i]->root), pn[i]->points, pn[i]->active_points, B[i]);
               list_tree(pn[i]->root);
               puts("");
            }
            concentration_inactivate_prob = save_prob;
         }

         quicksort_hash(A, B, pn, 0, pop_size - 1);
         if(A[0] > best_fitness + ROUNDING_THRESHOLD)
         {
            fprintf(stderr, "fitness got worse! g %d %g %g %g\n",g, A[0], best_fitness, fabs(A[0] - best_fitness));
         }
         if(A[0] < best_fitness)
         {
            best_fitness = A[0];
         }
         if(debug_level > 4)
         {
            double save_prob = concentration_inactivate_prob;
            concentration_inactivate_prob = 0;
            puts("debug_level 5");
            printf("%d %g <%g> %d %d %u ", 0, A[0], fitness(pn[0]->root), pn[0]->points, pn[0]->active_points, B[0]);
            list_tree(pn[0]->root);
            concentration_inactivate_prob = save_prob;
         }

         if(remove_duplicates)
         {
            //     printf("%d: generation %d mark duplicates\n", this_rank, g);
            last_pos = 0;
            for(i = 1; i < pop_size; i++)
            {
               if(debug_level > 3)
               {
                  printf("checking pop %d:", i);
               }
               if(fabs(A[i] - A[last_pos]) < MATCH_TOLERANCE && B[i] == B[last_pos])
               {
                  if(debug_level > 3)
                  {
                     printf(" A & B match");
                  }
                  if(cmp_tree(pn[i]->root, pn[last_pos]->root))
                  {
                     if(debug_level > 3)
                     {
                        printf(" trees match, replacing\n");
                     }
                     replace_population_node(pn[i], new_random_tree_rooted(new_tree_depth));
                     fitness_evals++;
                     A[i] = fitness(pn[i]->root);
                     /*
                       ((double *)pn[i]->data)[FITNESS_CONSTANT_1] = stack[CONSTANTS_STACK][FITNESS_CONSTANT_1];
                       ((double *)pn[i]->data)[FITNESS_CONSTANT_NEG_1] = stack[CONSTANTS_STACK][FITNESS_CONSTANT_NEG_1];
                     */
                     if(A[i] < tolerance)
                        A[i] = 0e0;
                     pn[i]->fitness = A[i];
                     pn[i]->fitness_valid = 1;
                     pn[i]->active_points = count_tree_active_points(pn[i]->root);
                     B[i] = tree_hash(pn[i]->root) + (pn[i]->active_points << 22);
                     // was - tree_hash(pn[i]->root) * pn[i]->points;
                  }
                  else
                  {
                     if(debug_level > 3)
                     {
                        printf(" trees don't match\n");
                     }
                     last_pos = i;
                  }
               }
               else
               {
                  if(debug_level > 3)
                  {
                     if(A[i] != A[last_pos])
                     {
                        printf(" A doesn't match %g %g", A[i], A[last_pos]);
                     }
                     if(B[i] != B[last_pos])
                     {
                        printf(" B doesn't match %u %u", B[i], B[last_pos]);
                     }
                  }
                  last_pos = i;
               }
               if(debug_level > 3)
               {
                  puts("");
               }
            }
            quicksort_hash(A, B, pn, 0, pop_size - 1);
         }

         if(debug_level > 4)
         {
            double save_prob = concentration_inactivate_prob;
            concentration_inactivate_prob = 0;
            puts("debug_level 5a");
            puts("debug_level 5b");
            printf("%d %g <%g> %d %d %u ", 0, A[0], fitness(pn[0]->root), pn[0]->points, pn[0]->active_points, B[0]);
            list_tree(pn[0]->root);
            concentration_inactivate_prob = save_prob;
         }

         tot_individual_size  = 0e0;
         if(A[0] > best_fitness + ROUNDING_THRESHOLD)
         {
            fprintf(stderr, "fitness got worse after remove duplicates! g %d %g %g %g\n", g, A[0], best_fitness, fabs(A[0] - best_fitness));
         }
         for(i = 0; i < pop_size; i++)
         {
            tot_individual_size += pn[i]->active_points;
         }

         avg_individual_size = tot_individual_size / pop_size;

         printf("%d: generation %d best fit %.4g size %d avg size %.2f\n", this_rank, g, pn[0]->fitness, pn[0]->active_points, avg_individual_size);
         fflush(stdout);

         //   printf("%d: generation %d avg individual size %.2f\n", this_rank, g, avg_individual_size);
         /* print out first few of the generation */
         for(i = 0; i < list_best; i++)
         {
            //     printf("%g %d %d %u ", A[i], pn[i]->points, pn[i]->active_points, B[i]);
            //     list_tree(pn[i]->root);
            //     puats("");
            list_node_as_solute_vector(pn[i]->root);
         }

         //   puts("gen 0");
         if(bloat_cutoff && avg_individual_size > bloat_cutoff)
         {
            printf("bloat_cutoff avg individual size %.2f\n", avg_individual_size);
            printf("%ld\n", (long)(pn[0]->root));
            list_tree(pn[0]->root);
            break;
         }
         //   puts("gen 0.a");

         //   printf("l2 %g\n", solute_distance_l2(pn[0]->root));

         if(solute_early_l2_termination &&
            solute_early_l2_termination > solute_distance_l2(pn[0]->root))
         {
            printf("early termination\n");
            if(first_match_gen == INT_MAX)
            {
               first_match_gen = g;
               first_match_B = B[0];
               best_match_gen = g;
               best_match_B = B[0];
            }
            else
            {
               if(best_match_B > B[0])
               {
                  best_match_gen = g;
                  best_match_B = B[0];
               }
            }
            list_tree(pn[0]->root);
            break;
         }

         if(solute_max_mfem_calls &&
            solute_max_mfem_calls < fitness_mfem_calls)
         {
            printf("solute_max_mfem_calls %d > %ld\n", fitness_mfem_calls, solute_max_mfem_calls);
            list_tree(pn[0]->root);
            break;
         }

         if(A[0] < tolerance)
         {
            printf("perfect fit\n");
            if(first_match_gen == INT_MAX)
            {
               first_match_gen = g;
               first_match_B = B[0];
               best_match_gen = g;
               best_match_B = B[0];
            }
            else
            {
               if(best_match_B > B[0])
               {
                  best_match_gen = g;
                  best_match_B = B[0];
               }
            }
         }

         //   puts("gen 0.b");
         /* set p to end of the population chain */
         p = last_population(pn[0]);
         //   puts("gen 0.c");

         /* generation: survivors reproduce to complete population */

         /*
          * we must 1st determine type of new population member creation
          * i.e. reproduction via
          * crossover, mutation, replication or subtree duplication
          * then select from population based upon fitness, we need
          * to create a uniformly distributed random number and
          * transform it to a non-uniform monotonically decreasing
          * distribution.  
          * and perform the 
          */
         {
            population *mom, *dad;
            //     node *offspring;
            int reproduction;

            //     puts("regen");

            if(gen_mult_sd && g && !(g % gen_mult_sd))
            {
               random_normal_sd_1 *= gen_mult_sd_factor;
               random_normal_sd_2 *= gen_mult_sd_factor;
               if(debug_level)
               {
                  printf("new sd multipliers %g %g\n", random_normal_sd_1, random_normal_sd_2);
               }
            }

            if(save_every_fitness_file && g >= save_every_fitness_after_generation)
            {
               int use_pop_size = save_every_fitness_best ? save_every_fitness_best : pop_size;
               for(i = 0; i < use_pop_size; i++)
               {
                  save_every_this_pop(pn[i], g);
               }
               fflush(save_every_fitness_file);
            }

            bufinit(MIGRATE_OUT);
            if(migrate_prob)
            {
               if(p_migrate_in)
               {
                  delete_population(p_migrate_in);
                  p_migrate_in = (population *)0;
                  free(p_migrate_in_n);
               }
               p_migrate_in_size = 0;
               for(int j = 0; j < MIGRATE_IN_BUFS; j++)
               {
                  if(migrate_end_pos[j])
                  {
                     while((n_migrate_in = buffer_serialize_in_tree(j)))
                     {
                        p_migrate_in = insert_population_node(p_migrate_in, n_migrate_in);
                        if(debug_level > 2)
                        {
                           printf("%d:", p_migrate_in_size);
                           list_tree(p_migrate_in->root);
                           puts("");
                        }
                        p_migrate_in_size++;
                     }
                  }
               }
               if(debug_level > 1)
               {
                  printf("migrate in %d individuals for %d buffs\n", p_migrate_in_size, MIGRATE_IN_BUFS);
               }
               if(p_migrate_in_size)
               {
                  if(NULL == (p_migrate_in_n = (population **)malloc(p_migrate_in_size * sizeof(population *))))
                  {
                     fprintf(stderr, "malloc failure\n");
                     exit(-1);
                  }
                  p_migrate_in = first_population(p_migrate_in);
                  for(i = 0; i < p_migrate_in_size; i++)
                  {
                     if(!p_migrate_in)
                     {
                        fprintf(stderr, "p_migrate_in ran out, ugh!\n");
                        exit(-1);
                     }
                     p_migrate_in_n[i] = p_migrate_in;
                     p_migrate_in = p_migrate_in->next;
                  }
               }
            }

            // set best for each number of active points so that we can gsm each best
            // active points population member
            // and read in the in populations

            any_gsm = 0;
            if(ga_sc)
            {
               any_gsm = 0;
            }
            // todo gsm not active

            if(any_gsm &&
               g >= elitist_gsm_generation_start &&
               (inverse_hessian_prob ||
                conjugate_gradient_prob ||
                steepest_descent_prob)) {
               for(i = 0; i < GSM_MAX_SEARCH_TYPE; i++) {
                  use_gsm_type[i] = 0;
                  if(inverse_hessian_prob &&
                     inverse_hessian_prob > drand48()) {
                     use_gsm_type[INVERSE_HESSIAN]++;
                     any_gsm++;
                  }
                  if(conjugate_gradient_prob &&
                     conjugate_gradient_prob > drand48()) {
                     use_gsm_type[CONJUGATE_GRADIENT]++;
                     any_gsm++;
                  }
                  if(steepest_descent_prob &&
                     steepest_descent_prob > drand48()) {
                     use_gsm_type[STEEPEST_DESCENT]++;
                     any_gsm++;
                  }
               }
               if(any_gsm) {
                  int j, active_points_index;
                  puts("will gsm");
                  for(i = 0; i < new_tree_depth; i++) {
                     best_index[i] = 0;
                     for(j = 0; j < elitist_gsm; j++) {
                        best[j][i] = -1;
                     }
                  }
                  puts("any_gsm 1");
                  for(i = 0; i < pop_size; i++) {
                     if(debug_level > 1) {
                        if(pn[i]->active_points != count_tree_active_points(pn[i]->root)) {
                           printf("active points match error\n");
                           pn[i]->active_points = count_tree_active_points(pn[i]->root);
                        }
                     }
                     gsm_this_index[i] = 0;
                     active_points_index = pn[i]->active_points - 1;
                     if(best[best_index[active_points_index]][active_points_index] == -1) {
                        gsm_this_index[i] = 1;
                        best[best_index[active_points_index]][active_points_index] = i;
                        if(best_index[active_points_index] < elitist_gsm - 1) {
                           best_index[active_points_index]++;
                           printf("best_index[%d] = %d\n", active_points_index, best_index[active_points_index]);
                        }
                     }
                  }

                  puts("any_gsm 2");
                  for(i = 0; i < new_tree_depth; i++) {
                     for(j = 0; j < elitist_gsm; j++) {
                        if(best[j][i] != -1) {
                           printf("best[%d][%d]: %g %d %d %u ", 
                                  j, i, 
                                  A[best[j][i]], 
                                  pn[best[j][i]]->points, 
                                  pn[best[j][i]]->active_points, 
                                  B[best[j][i]]);
                           list_tree(pn[best[j][i]]->root);
                           puts("");
                        } else {
                           //        printf("best[%d][%d]: unset\n", j, i);
                        }
                     }
                  }
               }
            }

            puts("any_gsm 3");

            for(i = 0; i < pop_size; i++)
            {
               if(!(i % 25) || debug_level > 3)
               {
                  printf("%d: generation %d completed %d of %d\n", this_rank, g, i, pop_size );
                  fflush(stdout);
               }
#if defined USE_GSM
               if(any_gsm && gsm_this_index[i])
               {
                  if(pn[i]->no_gsm_improvement)
                  {
                     puts("any_gsm 3a");
                     puts("gsm skipped, no improvement");
                     p = insert_population_node(p, dup_tree(pn[i]->root));
                     p->fitness = pn[i]->fitness;
                     p->fitness_valid = pn[i]->fitness_valid;
                     p->active_points = pn[i]->active_points;
                     p->no_gsm_improvement = pn[i]->no_gsm_improvement;
                     memcpy(p->data, pn[i]->data, pop_data_size);
                  }
                  else
                  {
                     puts("any_gsm 3b");
                     node *n = pn[i]->root, *m;
                     double *d, *e;
                     char any_improvement = 0;
                     p = insert_population_node(p, dup_tree(pn[i]->root));
                     m = p->root;
                     p->fitness_valid = 0;
                     if(use_gsm_type[INVERSE_HESSIAN])
                     {
                        gsm_this_node(INVERSE_HESSIAN, p->root, inverse_hessian_iter, inverse_hessian_h);
                     }
                     if(use_gsm_type[CONJUGATE_GRADIENT])
                     {
                        gsm_this_node(CONJUGATE_GRADIENT, p->root, conjugate_gradient_iter, conjugate_gradient_h);
                     }
                     if(use_gsm_type[STEEPEST_DESCENT])
                     {
                        gsm_this_node(STEEPEST_DESCENT, p->root, steepest_descent_iter, steepest_descent_h);
                     }
                     fitness_evals++;
                     p->fitness = fitness(p->root);
                     /*
                       ((double *)p->data)[FITNESS_CONSTANT_1] = stack[CONSTANTS_STACK][FITNESS_CONSTANT_1];
                       ((double *)p->data)[FITNESS_CONSTANT_NEG_1] = stack[CONSTANTS_STACK][FITNESS_CONSTANT_NEG_1];
                     */
                     if(p->fitness < tolerance)
                     {
                        p->fitness = 0e0;
                     }
                     p->active_points = count_tree_active_points(p->root);
                     p->fitness_valid = 1;
                     if(p->fitness >= pn[i]->fitness)
                     {
                        //      fprintf(stderr, "gsm made fitness worse!, reverting g %d %g %g %g (%g)\n", g, p->fitness, pn[i]->fitness, fabs(p->fitness - pn[i]->fitness), fitness(pn[i]->root));
                        if(debug_level > 1 && p->fitness > pn[i]->fitness)
                        {
                           puts("gsm made fitness worse");
                        }
                     }
                     else
                     {
                        any_improvement = 1;
                        /*      while(!any_improvement && n) {
                                d = (double *)n->data;
                                e = (double *)m->data;
                                if(((char *)n->data)[SOLUTE_DATA_ACTIVE_OFS] !=
                                ((char *)m->data)[SOLUTE_DATA_ACTIVE_OFS]) {
                                fprintf(stderr, "unexpected mismatch!\n");
                                exit(-1);
                                } else {
                                if(((char *)m->data)[SOLUTE_DATA_ACTIVE_OFS]) {
                                if(d[0] != e[0] || d[1] != e[1]) {
                                any_improvement++;
                                }
                                }
                                }
                                n = n->children[0];
                                m = m->children[0];
                                }
                        */
                     }
                     if(!any_improvement)
                     {
                        puts("no gsm improvement!!");
                        p->no_gsm_improvement = 1;
                        p->fitness_valid = 1;
                        p->fitness = pn[i]->fitness;
                        p->active_points = pn[i]->active_points;
                        memcpy(p->data, pn[i]->data, pop_data_size);
                     }
                  }
               }
               else
#endif

               {
                  if(i < elitism)
                  {
                     // find i-th unique member to dup
                     if(!i)
                     {
                        if(debug_level > 3)
                        {
                           printf("elitism dupped node %d\n", i);
                        }
                        p = insert_population_node(p, dup_tree(pn[i]->root));
                        p->fitness = pn[i]->fitness;
                        p->fitness_valid = pn[i]->fitness_valid;
                        p->active_points = pn[i]->active_points;
                        p->no_gsm_improvement = pn[i]->no_gsm_improvement;
                        memcpy(p->data, pn[i]->data, pop_data_size);
                     }
                     else
                     {
                        int j, k = i;
                        double last_A = A[0], last_B = B[0];
                        if(debug_level > 3)
                        {
                           printf("elitism find next different %d\n", i);
                        }
                        for(j = 1; j < pop_size && k > 0; j++)
                        {
                           if(A[j] != last_A || B[j] != last_B)
                           {
                              k--;
                              last_A = A[j];
                              last_B = B[j];
                           }
                        }
                        if(j < pop_size)
                        {
                           j--;
                           if(debug_level > 3)
                           {
                              printf("elitism found next different %d\n", j);
                           }
                           p = insert_population_node(p, dup_tree(pn[j]->root));
                           p->fitness = pn[j]->fitness;
                           p->fitness_valid = pn[j]->fitness_valid;
                           p->active_points = pn[j]->active_points;
                           p->no_gsm_improvement = pn[j]->no_gsm_improvement;
                           memcpy(p->data, pn[j]->data, pop_data_size);
                        }
                        else
                        {
                           // just duplicate the original one
                           if(debug_level > 3)
                           {
                              printf("elitism just duplicate original one %d\n", i);
                           }
                           p = insert_population_node(p, dup_tree(pn[i]->root));
                           p->fitness = pn[i]->fitness;
                           p->fitness_valid = pn[i]->fitness_valid;
                           p->active_points = pn[i]->active_points;
                           p->no_gsm_improvement = pn[i]->no_gsm_improvement;
                           memcpy(p->data, pn[i]->data, pop_data_size);
                        }
                     }
                     //      if(elitist_gsm && i < elitist_gsm && g > elitist_gsm_generation_start) {
                     //        int j;
                     //        if(inverse_hessian_prob &&
                     //       inverse_hessian_prob > drand48()) {
                     //
                     //      gsm_this_node(INVERSE_HESSIAN, p->root, inverse_hessian_iter, inverse_hessian_h);
                     //      p->fitness_valid = 0;
                     //        }
                     //        if(conjugate_gradient_prob &&
                     //       conjugate_gradient_prob > drand48()) {
                     //      gsm_this_node(CONJUGATE_GRADIENT, p->root, conjugate_gradient_iter, conjugate_gradient_h);
                     //      p->fitness_valid = 0;
                     //        }
                     //        if(steepest_descent_prob &&
                     //       steepest_descent_prob > drand48()) {
                     //      gsm_this_node(STEEPEST_DESCENT, p->root, steepest_descent_iter, steepest_descent_h);
                     //      p->fitness_valid = 0;
                     //        }
                     //      }
                  }
                  else
                  {
                     if(debug_level > 3)
                     {
                        printf("reproduce %d\n", i);
                     }
                     /* select mom */
                     mom = pn[pop_selection()];
                     reproduction = (int) (100 * drand48());
                     if(debug_level > 3)
                     {
                        printf("reproduction %d\n", reproduction);
                     }
                     if(migrate_prob && drand48() < migrate_prob)
                     {
                        int j;
                        if(debug_level > 3)
                        {
                           puts("migrate:add_serial_output");
                        }
                        buffer_serialize_out_tree(MIGRATE_OUT, mom->root);
                        if(p_migrate_in_size)
                        {
                           j = (int)floor(drand48() * p_migrate_in_size);
                           p = insert_population_node(p, dup_tree(p_migrate_in_n[j]->root));
                        }
                        else
                        {
                           // just duplicate migrated out element
                           p = insert_population_node(p, dup_tree(mom->root));
                        }
                        p->fitness_valid = 0;
                     }
                     else
                     {
                        if(reproduction < cut_mutation)
                        {
                           /* crossover */
                           if(debug_level > 3)
                           {
                              printf("crossover\n");
                           }
                           do
                           {
                              dad = pn[pop_selection()];
                           }
                           while(dad == mom);
                           switch(point_crossover)
                           {
                           case 0 :
                              {
                                 p = insert_population_node(p, crossover(mom, dad));
                                 if(prune_depth && p->points > prune_depth)
                                 {
                                    //         puts("prune tree");
                                    prune_tree(p->root, prune_depth);
                                    p->points = prune_depth;
                                    //          puts("end prune tree");
                                 }
                                 if(inactive_control)
                                 {
                                    node *n = p->root;
                                    do
                                    {
                                       ((char *)n->data)[SOLUTE_DATA_ACTIVE_OFS] = 1;
                                    }
                                    while((n = n->children[0]));
                                 }
                                 break;
                              }
                           case 1 :
                              {
                                 p = insert_population_node(p, crossover_1pt(mom, dad));
                                 break;
                              }
                           case 2 :
                              {
                                 p = insert_population_node(p, crossover_2pt(mom, dad));
                                 break;
                              }
                           default :
                              {
                                 fprintf(stderr, "unexpected case fallthrough\n");
                                 exit(-1);
                                 break;
                              }
                           }
                           p->fitness_valid = 0;
                        }
                        else
                        {
                           if(reproduction < cut_replication)
                           {
                              /* mutation */
                              if(debug_level > 3)
                              {
                                 printf("mutation\n");
                              }
                              p = insert_population_node(p, mutate(mom));
                              if(prune_depth && p->points > prune_depth)
                              {
                                 prune_tree(p->root, prune_depth);
                                 p->points = prune_depth;
                              }
                              p->fitness_valid = 0;
                           }
                           else
                           {
                              if(reproduction < cut_subtree_dup)
                              {
                                 /* replication */
                                 if(debug_level > 3)
                                 {
                                    printf("replication\n");
                                 }
                                 p = insert_population_node(p, replicate(mom));
                                 p->fitness_valid = mom->fitness_valid;
                                 p->fitness = mom->fitness;
                                 p->active_points = mom->active_points;
                                 p->no_gsm_improvement = mom->no_gsm_improvement;
                                 memcpy(p->data, mom->data, pop_data_size);
                              }
                              else
                              {
                                 if(reproduction < cut_plague)
                                 {
                                    /* subtree duplication */
                                    if(debug_level > 3)
                                    {
                                       printf("subtree duplication\n");
                                    }
                                    p = insert_population_node(p, subtree_duplicate(mom));
                                    p->fitness_valid = 0;
                                 }
                                 else
                                 {
                                    if(debug_level > 3)
                                    {
                                       printf("plagueish - new random tree\n");
                                    }
                                    p = insert_population_node(p, new_random_tree_rooted(new_tree_depth));
                                    p->fitness_valid = 0;
                                 }
                              }
                           }
                        }
                        if(pct_point_mutation &&
                           pct_point_mutation > drand48() * 100e0)
                        {
                           node_mutate_count =
                              node_mutate_count_s =
                              node_mutate_count_D = 0;
                           if(debug_level > 3)
                           {
                              puts("point node mutate");
                           }
                           point_node_mutate(p->root, p->points);
                           if(debug_level > 1)
                           {
                              printf("point_node_mutate_count %d s %d D %d\n", node_mutate_count, node_mutate_count_s, node_mutate_count_D);
                           }
                           if(node_mutate_count)
                           {
                              p->fitness_valid = 0;
                           }
                        }
                        else
                        {
                           if(pct_node_mutation)
                           {
                              node_mutate_count =
                                 node_mutate_count_s =
                                 node_mutate_count_D = 0;
                              if(debug_level > 3)
                              {
                                 puts("node mutate");
                              }
                              node_mutate(p->root);
                              if(debug_level > 1)
                              {
                                 printf("node_mutate_count %d s %d D %d\n", node_mutate_count, node_mutate_count_s, node_mutate_count_D);
                              }
                              if(node_mutate_count)
                              {
                                 p->fitness_valid = 0;
                              }
                           }
                        }
                     }
                     if(generation_inactivate_prob &&
                        drand48() < generation_inactivate_prob)
                     {
                        if(debug_level > 3)
                        {
                           puts("inactivate some node");
                        }
                        if(node_inactivate(p))
                        {
                           p->fitness_valid = 0;
                        }
                     }


                     //      if(!elitist_gsm && g > elitist_gsm_generation_start) {
                     //        if(inverse_hessian_prob &&
                     //       inverse_hessian_prob > drand48()) {
                     //      gsm_this_node(INVERSE_HESSIAN, p->root, inverse_hessian_iter, inverse_hessian_h);
                     //      p->fitness_valid = 0;
                     //        }
                     //        if(conjugate_gradient_prob &&
                     //       conjugate_gradient_prob > drand48()) {
                     //      gsm_this_node(CONJUGATE_GRADIENT, p->root, conjugate_gradient_iter, conjugate_gradient_h);
                     //      p->fitness_valid = 0;
                     //        }
                     //        if(steepest_descent_prob &&
                     //       steepest_descent_prob > drand48()) {
                     //      gsm_this_node(STEEPEST_DESCENT, p->root, steepest_descent_iter, steepest_descent_h);
                     //      p->fitness_valid = 0;
                     //        }
                     //      }
                  }
               }

               pn_next[i] = p;

               if(skip_fitness && p->fitness_valid)
               {
                  if(debug_level > 3)
                  {
                     puts("skip fitness");
                  }
                  fitness_skipped++;
                  /*     A_next[i] = fitness(p->root);
                         if(A_next[i] < tolerance)
                         A_next[i] = 0e0;
                         if(fabs(A_next[i] - p->fitness) > TOLERANCE) {
                         printf("bad fitness match %g %g\n", A_next[i], p->fitness);
                         list_tree(p->root);
                         exit(-1);
                         } else { */
                  A_next[i] = p->fitness;
                  //     }
               }
               else
               {
                  p->no_gsm_improvement = 0;
                  if(debug_level > 3)
                  {
                     puts("process fitness");
                  }
                  fitness_evals++;
                  A_next[i] = fitness(p->root);
                  /*
                    ((double *)p->data)[FITNESS_CONSTANT_1] = stack[CONSTANTS_STACK][FITNESS_CONSTANT_1];
                    ((double *)p->data)[FITNESS_CONSTANT_NEG_1] = stack[CONSTANTS_STACK][FITNESS_CONSTANT_NEG_1];
                  */
                  if(A_next[i] < tolerance)
                     A_next[i] = 0e0;
                  p->fitness = A_next[i];
                  p->fitness_valid = 1;
                  p->active_points = count_tree_active_points(p->root);
               }

               if(inactivation_deletes && p->active_points < p->points)
               {
                  if(debug_level > 2)
                  {
                     puts("delete_inactive_nodes before:");
                     list_tree(p->root);
                     puts("");
                  }
                  delete_inactive_nodes(p);
                  if(debug_level > 2)
                  {
                     puts("delete_inactive_nodes after:");
                     list_tree(p->root);
                     puts("");
                  }
               }

               B_next[i] = tree_hash(p->root) + (p->active_points << 22);
               // was - tree_hash(p->root) * p->points;
               //   puts("next");

            }

            if(debug_level > 1)
            {
               for(i = 0; i < pop_size; i++)
               {
                  if(pn[i]->no_gsm_improvement)
                  {
                     printf("no gsm improvement pop # %d\n", i);
                  }
               }
            }


            //     if(migrate_prob) {
            // MPI stuff
            buffer_close_serial_output(MIGRATE_OUT);
            mpi_ga_msg_out.gen = g;
            mpi_ga_msg_out.size = migrate_end_pos[MIGRATE_OUT];
            mpi_ga_msg_out.fitness = pn[0]->fitness;
            // worker sends generation completion message to master
            //     printf("%d: worker sends generation completion message gen %d size %d\n", this_rank, g, migrate_end_pos[MIGRATE_OUT]); fflush(stdout);
#if defined(US_DEBUG_MPI)

            printf("%d: MPI_Send generation comp msg %d MPI_CHAR\n", this_rank, sizeof(mpi_ga_msg_out));
            fflush(stdout);
            fflush(stdout);
#endif

            return_stat = MPI_Send(&mpi_ga_msg_out,
                                   sizeof(mpi_ga_msg_out),
                                   MPI_CHAR,
                                   0,
                                   0,
                                   MPI_COMM_WORLD);
#if defined(US_DEBUG_MPI)

            printf("%d: MPI returned %d from send to 0\n", this_rank, return_stat);
            fflush(stdout);
#endif

            if(migrate_end_pos[MIGRATE_OUT])
            {
               // send out buffer if we have one
               //   printf("%d: send out buffer size %d\n", this_rank, migrate_end_pos[MIGRATE_OUT]); fflush(stdout);
               //   buffer_list_serial(MIGRATE_OUT); fflush(stdout);
#if defined(US_DEBUG_MPI)
               printf("%d: MPI_Send emigrants %d %d MPI_CHAR\n", this_rank, 1001 + this_rank, migrate_end_pos[MIGRATE_OUT]);
               fflush(stdout);
               fflush(stdout);
#endif

               return_stat = MPI_Send(migrate_buffer[MIGRATE_OUT],
                                      migrate_end_pos[MIGRATE_OUT],
                                      MPI_CHAR,
                                      0,
                                      1001 + this_rank,
                                      MPI_COMM_WORLD);
#if defined(US_DEBUG_MPI)

               printf("%d: MPI returned %d send to 0\n", this_rank, return_stat);
               fflush(stdout);
#endif

            }

            // receive masters' response

            for(int j = 0; j < MIGRATE_IN_BUFS; j++)
            {
               bufinit(j); // reinit migrate in buffer
               //   printf("%d: proc migrate in buffer %d\n", this_rank, j); fflush(stdout);
#if defined(US_DEBUG_MPI)

               printf("%d: MPI_Recv immigrants msg %d %d MPI_CHAR\n", this_rank, (j*2)+1, sizeof(mpi_ga_msg_in));
               fflush(stdout);
               fflush(stdout);
#endif

               return_stat = MPI_Recv(&mpi_ga_msg_in,
                                      sizeof(mpi_ga_msg_in),
                                      MPI_CHAR,
                                      0,
                                      (j*2)+1,
                                      MPI_COMM_WORLD,
                                      &mpi_status);
#if defined(US_DEBUG_MPI)

               printf("%d: MPI returned %d from 0 gen %d size %d\n", this_rank, return_stat, mpi_ga_msg_in.gen, mpi_ga_msg_in.size);
               fflush(stdout);
#endif

               if(j || !mpi_ga_msg_in.gen)
               {
                  printf("%d: proc migrate in buffer %d has size %d\n", this_rank, j, mpi_ga_msg_in.size);
                  fflush(stdout);
                  // receive migration in buffers
                  if(mpi_ga_msg_in.size > 0)
                  {
#if defined(US_DEBUG_MPI)
                     printf("%d: MPI_Recv immigrants buf %d %d MPI_CHAR\n", this_rank, (j*2)+2, mpi_ga_msg_in.size);
                     fflush(stdout);
                     fflush(stdout);
#endif

                     return_stat = MPI_Recv(migrate_buffer[j],
                                            mpi_ga_msg_in.size,
                                            MPI_CHAR,
                                            0,
                                            (j*2)+2,
                                            MPI_COMM_WORLD,
                                            &mpi_status);
#if defined(US_DEBUG_MPI)

                     printf("%d: MPI returned %d from 0\n", this_rank, return_stat);
                     fflush(stdout);
#endif

                     migrate_end_pos[j] = mpi_ga_msg_in.size;
                     //      printf("listing buffer %d\n", j);
                     //      buffer_list_serial(j);
                     //      fflush(stdout);
                  }
                  else
                  {
                     migrate_end_pos[j] = 0;
                  }
               }
               else
               {
                  // close up shop
                  printf("%d: close up shop\n", this_rank);
                  fflush(stdout);
                  quicksort_hash(A, B, pn, 0, pop_size - 1);   /* one last sort */
                  vector<Solute> solutes;
                  int use_gsm = 1;
#if defined(US_DEBUG_MPI)

                  use_gsm = 0;
#endif

                  if(/* !ga_sc &&  */ use_gsm)
                  {
                     printf("%d: start use_gsm %d %g\n", this_rank, inverse_hessian_iter, inverse_hessian_h);
                     fflush(stdout);
                     p = insert_population_node(p, dup_tree(pn[0]->root));
                     p->fitness_valid = 0;
                     gsm_this_node(INVERSE_HESSIAN, p->root, inverse_hessian_iter, inverse_hessian_h);
                     printf("%d: start use_gsm 1\n", this_rank);
                     fflush(stdout);
                     fitness_evals++;
                     p->fitness = fitness(p->root);
                     if(p->fitness < tolerance)
                     {
                        p->fitness = 0e0;
                     }
                     printf("%d: start use_gsm 2\n", this_rank);
                     fflush(stdout);
                     p->active_points = count_tree_active_points(p->root);
                     p->fitness_valid = 1;
                     printf("%d: start use_gsm 3\n", this_rank);
                     fflush(stdout);
                     if(p->fitness >= pn[0]->fitness)
                     {
                        fprintf(stdout, "%d: gsm made fitness worse!, reverting g %d %g %g %g (%g)\n", this_rank, g, p->fitness, pn[0]->fitness, fabs(p->fitness - pn[0]->fitness), fitness(pn[0]->root));
                        fflush(stdout);
                        solutes = node_to_solute_vector(pn[0]->root);
                        mpi_ga_msg_out.fitness = pn[0]->fitness;
                        printf("%d: start use_gsm 4\n", this_rank);
                        fflush(stdout);
                     }
                     else
                     {
                        printf("%d: start use_gsm 5\n", this_rank);
                        fflush(stdout);
                        solutes = node_to_solute_vector(p->root);
                        mpi_ga_msg_out.fitness = p->fitness;
                     }
                  }
                  else
                  {
                     solutes = node_to_solute_vector(pn[0]->root);
                     mpi_ga_msg_out.fitness = pn[0]->fitness;
                  }
                  mpi_ga_msg_out.gen = -1;
                  mpi_ga_msg_out.size = (int) solutes.size();

                  printf("%d: worker sending out final solutes preamble size %u\n", this_rank, (unsigned int)solutes.size());
                  fflush(stdout);
#if defined(US_DEBUG_MPI)

                  printf("%d: MPI_Send generation comp msg (final!) %d MPI_CHAR\n", this_rank, sizeof(mpi_ga_msg_out));
                  fflush(stdout);
                  fflush(stdout);
#endif

                  return_stat = MPI_Send(&mpi_ga_msg_out,
                                         sizeof(mpi_ga_msg_out),
                                         MPI_CHAR,
                                         0,
                                         0,
                                         MPI_COMM_WORLD);
#if defined(US_DEBUG_MPI)

                  printf("%d: MPI returned %d from send to 0\n", this_rank, return_stat);
                  fflush(stdout);
#endif

                  printf("%d: worker sending out final solutes size %u\n", this_rank, (unsigned int)solutes.size() * (unsigned int)sizeof(Solute));
                  fflush(stdout);
#if defined(US_DEBUG_MPI)

                  printf("%d: MPI_Send results %d %d MPI_CHAR\n", this_rank, 1000 + this_rank, solutes.size() * sizeof(Solute));
                  fflush(stdout);
                  fflush(stdout);
#endif

                  for(unsigned int l=0; l<solutes.size(); l++)
                  {
                     printf("%d: final solute %u s %g ff0 %g\n", this_rank, l, solutes[l].s, solutes[l].k);
                     fflush(stdout);
                  }
                  return_stat = MPI_Send(&solutes[0],
                                         solutes.size() * sizeof(Solute),
                                         MPI_CHAR,
                                         0,
                                         1000 + this_rank,
                                         MPI_COMM_WORLD);
#if defined(US_DEBUG_MPI)

                  printf("%d: MPI returned %d from send to %d\n", this_rank, return_stat, 0);
                  fflush(stdout);
#endif

                  // & send results (later)
                  //     printf("%d: worker finishing\n", this_rank); fflush(stdout);
                  if(end_save_best)
                  {
                     QString end_save_filename_qs =
                        our_us_fe_nnls_t->experiment[0].id + "_GA_" +
                        our_us_fe_nnls_t->startDateTime.toString("yyMMddhhmmss")  + QString(".%1.deme").arg(this_rank);
                     printf("%d: saving best in %s\n", this_rank, end_save_filename_qs.ascii());
                     fflush(stdout);
                     FILE *end_save_best_file;
                     node *n;
                     //      double *d;
                     //      int j;
                     int active_points;
                     if(end_save_best > pop_size)
                     {
                        end_save_best = pop_size;
                     }
                     if(NULL == (end_save_best_file = fopen(end_save_filename_qs.ascii(), "w")))
                     {
                        fprintf(stderr, "%d: save file open error, can't open \"%s\" for writing\n", this_rank, end_save_filename_qs.ascii());
                        printf("%d: max rss %ld pages\n", this_rank, maxrss);
                        fflush(stdout);
                        printf("%d: goodbye\n", this_rank);
                        fflush(stdout);
                        MPI_Abort(MPI_COMM_WORLD, -3);
                        exit(-1);
                     }
                     for(i = 0; i < end_save_best; i++)
                     {
                        active_points = 0;
                        n = pn[i]->root;
                        while(n)
                        {
                           while(n && (!((char *)n->data)[SOLUTE_DATA_ACTIVE_OFS] ||
                                       ((double *)n->data)[SOLUTE_DATA_CONCENTRATION] <= SOLUTE_CONCENTRATION_THRESHOLD)
                                 )
                           {
                              n = n->children[0];
                           }
                           if(n)
                           {
                              active_points++;
                              n = n->children[0];
                           }
                        }
                        if(active_points > 0)
                        {
                           n = pn[i]->root;
                           fprintf(end_save_best_file, "%g|%d", A[i] , active_points);
                           //      for(j = 0; j < FITNESS_NO_OF_CONSTANTS; j++) {
                           //        fprintf(end_save_best_file, "|%g", ((double *)pn[i]->data)[j]);
                           //      }
                           while(n)
                           {
                              while(n && (!((char *)n->data)[SOLUTE_DATA_ACTIVE_OFS] ||
                                          ((double *)n->data)[SOLUTE_DATA_CONCENTRATION] <= SOLUTE_CONCENTRATION_THRESHOLD)
                                    )
                              {
                                 n = n->children[0];
                              }
                              if(n)
                              {
                                 fprintf(end_save_best_file, "|%g|%g|%g" ,
                                         ((double *)n->data)[0],
                                         ((double *)n->data)[1],
                                         ((double *)n->data)[SOLUTE_DATA_CONCENTRATION]);
                                 n = n->children[0];
                              }
                           }
                           fprintf(end_save_best_file, "\n");
                        }
                     }
                     fclose(end_save_best_file);
                  }
                  printf("%d: max rss %ld pages\n", this_rank, maxrss);
                  fflush(stdout);
                  printf("%d: return\n", this_rank);
                  fflush(stdout);
                  // monte carlo changes
                  //     MPI_Finalize();
                  //     exit(0);
                  return;
               }
#if defined(US_DEBUG_MPI)
               printf("%d: end of buffer in loop\n", this_rank);
               fflush(stdout);
#endif

            }
            // } (always for MPI, no mpi_prob

            if(debug_level > 3)
            {
               puts("delete_pop");
            }
            /* we're done, so delete old population */
            for(i = 0; i < pop_size; i++)
            {
               //   printf("del pop %d\n", i);
               delete_population(pn[i]);
            }

            /* switch pops */
            //     puts("switch pops");

            A_tmp = A;
            B_tmp = B;
            pn_tmp = pn;

            A = A_next;
            B = B_next;
            pn = pn_next;

            A_next = A_tmp;
            B_next = B_tmp;
            pn_next = pn_tmp;
         }

         pstats(stdout);
      }
      ; /* end of generations for loop */
   }

   quicksort_hash(A, B, pn, 0, pop_size - 1);   /* one last sort */
   if(A[0] > best_fitness + ROUNDING_THRESHOLD)
   {
      fprintf(stderr, "fitness got worse after one last sort!\n");
   }

   if(save_every_fitness_file && g >= save_every_fitness_after_generation)
   {
      int use_pop_size = save_every_fitness_best ? save_every_fitness_best : pop_size;
      for(i = 0; i < use_pop_size; i++)
      {
         save_every_this_pop(pn[i], g);
      }
      fflush(save_every_fitness_file);
   }
   if(save_every_fitness_file)
   {
      fclose(save_every_fitness_file);
   }

   if(A[0] < tolerance)
   {
      printf("perfect fit\n");
      if(first_match_gen == INT_MAX)
      {
         first_match_gen = g;
         first_match_B = B[0];
         best_match_gen = g;
         best_match_B = B[0];
      }
      else
      {
         if(best_match_B > B[0])
         {
            best_match_gen = g;
            best_match_B = B[0];
         }
      }
   }

   gettimeofday(&global_end_tv, NULL);
   total_time = 1000000l * (global_end_tv.tv_sec - global_start_tv.tv_sec) + global_end_tv.tv_usec - global_start_tv.tv_usec;
   print_stats();


   {
      int perfect_fit = 0;
      value fitness_final_check_value;
      if(first_match_gen <= max_g)
      {
         if((fitness_final_check_value = fitness_final_check(pn[0]->root)) < tolerance)
         {
            perfect_fit = 1;
         }
      }
      else
      {
         fitness_final_check_value =  fitness_final_check(pn[0]->root);
      }

      printf("fitness evals %ld skipped %ld\n", fitness_evals, fitness_skipped);

      printf(
             "perfect fit after final check %d\n"
             "first perfect fit found in generation %d <%u>\n"
             "best perfect fit found in generation %d <%u>\n"
             ,
             perfect_fit,
             first_match_gen,
             first_match_B,
             best_match_gen,
             best_match_B);
      printf("%d|%d|%d|%d|%g|%g|%g|%g|%g|%g|%g|%g|%g|%g|%g|%g|%g|%g|%g|%g|%g|%g|%g|%g|%g|%g|%g|%g|%lu|%g|%g|%d|%d|%d|%d|%d|%d|%d|%u|%d|%u|%d|%g|%g|%g|%d|%g|%d|%g|%g|%d|%g|%g|%g|%g|",
             pop_size,
             max_arg,
             fitness_reinit,
             remove_duplicates,
             beta_divisor,
             beta_overflow * 100e0 / (selection_count * 1e0),
             fitness_param_1,
             fitness_param_2,
             fitness_param_3,
             fitness_param_4,
             fitness_param_5,
             fitness_param_6,
             fitness_param_7,
             fitness_out_1,
             fitness_out_2,
             fitness_out_3,
             fitness_out_4,
             fitness_out_5,
             fitness_out_6,
             fitness_out_7,
             fitness_out_8,
             fitness_out_9,
             fitness_out_10,
             fitness_out_11,
             fitness_out_12,
             fitness_out_13,
             fitness_out_14,
             fitness_out_15,
             total_time,
             fitness_final_check_value,
             pct_node_mutation,
             pct_crossover,
             pct_mutation,
             pct_subtree_dup,
             pct_replication,
             elitism,
             ramped,
             first_match_gen,
             first_match_B,
             best_match_gen,
             best_match_B,
             perfect_fit,
             migrate_prob,
             concentration_inactivate_prob,
             generation_inactivate_prob,
             node_mutate_reactivates,
             pct_point_mutation,
             use_random_normal,
             random_normal_sd_1,
             random_normal_sd_2,
             gen_mult_sd,
             gen_mult_sd_factor,
             min_f_ratio,
             max_f_ratio,
             use_vbar
             );
      list_tree(pn[0]->root);
      puts("");
      if(end_save_best)
      {
         FILE *end_save_best_file;
         node *n;
         //     double *d;
         int j;
         int active_points;
         if(end_save_best > pop_size)
         {
            end_save_best = pop_size;
         }
         if(NULL == (end_save_best_file = fopen(end_save_best_filename, "w")))
         {
            fprintf(stderr, "save file open error, can't open \"%s\" for writing\n", end_save_best_filename);
            exit(-1);
         }
         for(i = 0; i < end_save_best; i++)
         {
            active_points = 0;
            n = pn[i]->root;
            while(n)
            {
               while(n && (!((char *)n->data)[SOLUTE_DATA_ACTIVE_OFS] ||
                           ((double *)n->data)[SOLUTE_DATA_CONCENTRATION] <= SOLUTE_CONCENTRATION_THRESHOLD)
                     )
               {
                  n = n->children[0];
               }
               if(n)
               {
                  active_points++;
                  n = n->children[0];
               }
            }
            if(active_points > 0)
            {
               n = pn[i]->root;
               fprintf(end_save_best_file, "%g|%d", A[i] , active_points);
               for(j = 0; j < FITNESS_NO_OF_CONSTANTS; j++)
               {
                  fprintf(end_save_best_file, "|%g", ((double *)pn[i]->data)[j]);
               }
               while(n)
               {
                  while(n && (!((char *)n->data)[SOLUTE_DATA_ACTIVE_OFS] ||
                              ((double *)n->data)[SOLUTE_DATA_CONCENTRATION] <= SOLUTE_CONCENTRATION_THRESHOLD)
                        )
                  {
                     n = n->children[0];
                  }
                  if(n)
                  {
                     fprintf(end_save_best_file, "|%g|%g|%g" ,
                             ((double *)n->data)[0],
                             ((double *)n->data)[1],
                             ((double *)n->data)[SOLUTE_DATA_CONCENTRATION]);
                     n = n->children[0];
                  }
               }
               fprintf(end_save_best_file, "\n");
            }
         }
         fclose(end_save_best_file);
      }
   }
   ga_best_sve.clear();
   for(i = 0; i < list_best; i++)
   {
      fitness(pn[i]->root);
      ga_best_sve.push_back(ga_last_sve);
   }
}

void set_command_opt(char *line)
{
   char *token[MAX_INPUT_LIN_OPTS];
   char *p, *p_new;
   int tokens;
   int valid_token;
   int i;

   if(*line == '#')
   {
      return;
   }

   for(i = 0; i < MAX_INPUT_LIN_OPTS; i++)
   {
      token[i] = "";
   }
   tokens = 1;
   p = line;
   while((p_new = strchr(p, ' ')))
   {
      token[tokens - 1] = p;
      *p_new = 0;
      p = p_new + 1;
      tokens++;
   }
   token[tokens - 1] = p;
   while(strlen(p) &&
         (p[strlen(p) - 1] == '\n' ||
          p[strlen(p) - 1] == '\r'))
   {
      p[strlen(p) - 1] = 0;
   }
   if(debug_level > 2)
   {
      printf("config file line:");
      for(i = 0; i < tokens; i++)
      {
         printf(" [%s]", token[i]);
      }
      puts("");
   }
   valid_token = 0;

   if(!strcmp(token[0], "initial_diskcache_size"))
   {
      initial_diskcache_size = atoi(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "point_crossover"))
   {
      point_crossover = atoi(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "skip_fitness"))
   {
      skip_fitness++;
      valid_token = 1;
   }
   if(!strcmp(token[0], "max_arg"))
   {
      max_arg = atoi(token[1]);
      if(max_arg < 1)
      {
         fprintf(stderr,"less than one variable!\n");
         exit(-1);
      }
      valid_token = 1;
   }
   if(!strcmp(token[0],"bloat_cutoff"))
   {
      bloat_cutoff = atoi(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "beta_divisor"))
   {
      beta_divisor = atoi(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "pct_crossover"))
   {
      pct_crossover = atoi(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "cachedir"))
   {
      if(NULL == (cachedir = (char *)malloc(strlen(token[1]) + 1)))
      {
         fprintf(stderr, "malloc failure\n");
         exit(-1);
      }
      strcpy(cachedir, token[1]);
      valid_token = 1;
   }

   if(!strcmp(token[0], "debug"))
   {
      debug_level++;
      valid_token = 1;
   }
   if(!strcmp(token[0], "remove_duplicates"))
   {
      remove_duplicates = 1;
      valid_token = 1;
   }
   if(!strcmp(token[0], "elitism"))
   {
      elitism = atoi(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "force_rooted"))
   {
      force_rooted = atoi(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "fitness_reinit"))
   {
      fitness_reinit = atoi(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "use_alt_tree_hash"))
   {
      use_alt_tree_hash++;
      valid_token = 1;
   }
   if(!strcmp(token[0], "max_generations"))
   {
      max_generations = atoi(token[1]);
      if(max_generations < 0)
      {
         fprintf(stderr,"less than zero generations!\n");
         exit(-1);
      }
      valid_token = 1;
   }
   if(!strcmp(token[0], "list_best"))
   {
      list_best = atoi(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "pct_node_mutation"))
   {
      pct_node_mutation = atof(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "new_tree_depth"))
   {
      new_tree_depth = atoi(token[1]);
      if(new_tree_depth < 1)
      {
         fprintf(stderr,"less than one new tree depth!\n");
         exit(-1);
      }
      valid_token = 1;
   }
   if(!strcmp(token[0], "mutate_tree_depth"))
   {
      mutate_tree_depth = atoi(token[1]);
      if(mutate_tree_depth < 2)
      {
         fprintf(stderr,"less than two mutate tree depth!\n");
         exit(-1);
      }
      valid_token = 1;
   }
   if(!strcmp(token[0], "pct_mutation"))
   {
      pct_mutation = atoi(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "pop_size"))
   {
      pop_size = atoi(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "pct_plague"))
   {
      pct_plague = atoi(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "pct_subtree_dup"))
   {
      pct_subtree_dup = atoi(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "ramped"))
   {
      ramped = 1;
      valid_token = 1;
   }
   if(!strcmp(token[0], "tolerance"))
   {
      tolerance = atof(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "rand_seed"))
   {
      rand_seed = atof(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "fitness_param_1"))
   {
      fitness_param_1 = atof(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "fitness_param_2"))
   {
      fitness_param_2 = atof(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "fitness_param_3"))
   {
      fitness_param_3 = atof(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "fitness_param_4"))
   {
      fitness_param_4 = atof(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "fitness_param_5"))
   {
      fitness_param_5 = atof(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "inverse_hessian"))
   {
      inverse_hessian_prob = atof(token[1]);
      inverse_hessian_iter = atoi(token[2]);
      inverse_hessian_h = atof(token[3]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "conjugate_gradient"))
   {
      conjugate_gradient_prob = atof(token[1]);
      conjugate_gradient_iter = atoi(token[2]);
      conjugate_gradient_h = atof(token[3]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "steepest_descent"))
   {
      steepest_descent_prob = atof(token[1]);
      steepest_descent_iter = atoi(token[2]);
      steepest_descent_h = atof(token[3]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "elitist_gsm"))
   {
      elitist_gsm = atoi(token[1]);
      elitist_gsm_generation_start = atoi(token[2]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "solute_early_l2_termination"))
   {
      solute_early_l2_termination = atof(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "solute_max_mfem_calls"))
   {
      solute_max_mfem_calls = atoi(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "concentration_inactivate_prob"))
   {
      concentration_inactivate_prob = atof(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "generation_inactivate_prob"))
   {
      generation_inactivate_prob = atof(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "use_mem_cache"))
   {
      genetic_use_mem_cache = 1;
      valid_token = 1;
   }
   if(!strcmp(token[0], "migrate_prob"))
   {
      migrate_prob = atof(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "migrate_out"))
   {
      if(NULL == (migrate_out_filename = (char *)malloc(strlen(token[1]) + 1)) ||
         NULL == (migrate_out_filename_new = (char *)malloc(strlen(token[1]) + 1 + strlen(".new"))) ||
         NULL == (migrate_out_mv_new_to_current = (char *)malloc((2 * strlen(token[1])) + 1 + strlen(".new") + strlen("mv ") + strlen(" "))))
      {
         fprintf(stderr, "malloc failure\n");
         exit(-1);
      }
      strcpy(migrate_out_filename, token[1]);
      strcpy(migrate_out_filename_new, token[1]);
      strcat(migrate_out_filename_new, ".new");
      sprintf(migrate_out_mv_new_to_current, "mv %s %s", migrate_out_filename_new, migrate_out_filename);
      valid_token = 1;
   }
   if(!strcmp(token[0], "migrate_in"))
   {
      if(migrate_in_file_count >= MAX_MIGRATE_IN_FILES - 1)
      {
         fprintf(stderr, "maximum number of migrate_in files exceeded [%d]\n", MAX_MIGRATE_IN_FILES);
         exit(-1);
      }
      if(NULL == (migrate_in_filename[migrate_in_file_count] = (char *)malloc(strlen(token[1]) + 1)))
      {
         fprintf(stderr, "malloc failure\n");
         exit(-1);
      }
      strcpy(migrate_in_filename[migrate_in_file_count], token[1]);
      migrate_in_file_count++;
      valid_token = 1;
   }
   if(!strcmp(token[0], "end_save_best"))
   {
      end_save_best = atoi(token[1]);
      if(NULL == (end_save_best_filename = (char *)malloc(strlen(token[2]) + 1)))
      {
         fprintf(stderr, "malloc failure\n");
         exit(-1);
      }
      strcpy(end_save_best_filename, token[2]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "node_mutate_reactivates"))
   {
      node_mutate_reactivates++;
      valid_token = 1;
   }
   if(!strcmp(token[0], "save_every_fitness"))
   {
      save_every_fitness_best = atoi(token[1]);
      save_every_fitness_after_generation = atoi(token[2]);
      if(NULL == (save_every_fitness_filename = (char *)malloc(strlen(token[3]) + 1)))
      {
         fprintf(stderr, "malloc failure\n");
         exit(-1);
      }
      strcpy(save_every_fitness_filename, token[3]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "clip_bottom"))
   {
      clip_bottom = atof(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "cell"))
   {
      cell_top = atof(token[1]);
      cell_bottom = atof(token[2]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "pct_point_mutation"))
   {
      pct_point_mutation = atof(token[1]);
      valid_token = 1;
   }
   /*
     if(!strcmp(token[0], "use_constant")) {
     switch(atoi(token[1])) {
     case 1 : use_constants[FITNESS_CONSTANT_1]++; break;
     case -1 : use_constants[FITNESS_CONSTANT_NEG_1]++; break;
     default : fprintf(stderr, "unknown constant\n"); exit(-1);
     }
     valid_token = 1;
     }
   */
   if(!strcmp(token[0], "use_random_normal"))
   {
      use_random_normal = 1;
      random_normal_sd_1 = atof(token[1]);
      random_normal_sd_2 = atof(token[2]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "gen_mult_sd"))
   {
      gen_mult_sd = atoi(token[1]);
      gen_mult_sd_factor = atof(token[2]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "f_ratio_range"))
   {
      min_f_ratio = atof(token[1]);
      max_f_ratio = atof(token[2]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "use_vbar"))
   {
      use_vbar = atof(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "solute_rounding"))
   {
      solute_rounding = atoi(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "s_proximity_limit"))
   {
      s_proximity_limit = atof(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "s_range"))
   {
      min_s = atof(token[1]);
      max_s = atof(token[2]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "partition_s_f_ratio"))
   {
      s_partition = atoi(token[1]);
      f_partition = atoi(token[2]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "partition_s_f_increments"))
   {
      s_partition_increment = atof(token[1]);
      f_partition_increment = atof(token[2]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "partition_steps"))
   {
      partition_steps = atoi(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "partition_wrap"))
   {
      partition_wrap = 1;
      valid_token = 1;
   }
   if(!strcmp(token[0], "partition_merge"))
   {
      partition_merge = atoi(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "partition_zoom_steps_scaling_factors"))
   {
      partition_zoom_steps = atoi(token[1]);
      partition_scaling_factor_initial = atof(token[2]);
      partition_scaling_factor = atof(token[3]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "partition_local_random_steps_radius"))
   {
      partition_local_random_steps = atoi(token[1]);
      partition_local_random_radius = atof(token[2]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "partition_coalesce_threshold"))
   {
      partition_coalesce_threshold = atof(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "partition_coalesce_concentration_threshold"))
   {
      partition_coalesce_concentration_threshold = atof(token[1]);
      valid_token = 1;
   }
   /*  if(!strcmp(token[0], "coalesce")) {
       puts("init");
       init();
       puts("fitness_init");
       fitness_init();
       coalesce(token[1]);
       exit(0);
       }
       if(!strcmp(token[0], "regularize")) {
       puts("init");
       init();
       puts("fitness_init");
       fitness_init();
       //   solute_test_perfect_fitness_run();
       regularize(token[1]);
       exit(0);
       }
       if(!strcmp(token[0], "regularize_args")) {
       regularization_factor = atof(token[1]);
       puts("init");
       init();
       puts("fitness_init");
       fitness_init();
       //   solute_test_perfect_fitness_run();
       regularize(token[2]);
       exit(0);
       }
       if(!strcmp(token[0], "coalesce_args")) {
       puts("init");
       init();
       puts("fitness_init");
       fitness_init();
       partition_coalesce_concentration_threshold = atof(token[1]);
       partition_coalesce_threshold = atof(token[2]);
       coalesce(token[3]);
       exit(0);
       } */
   if(!strcmp(token[0], "prune_depth"))
   {
      prune_depth = atoi(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "inactivation_deletes"))
   {
      inactivation_deletes++;
      valid_token = 1;
   }
   if(!strcmp(token[0], "proximity_inactivates"))
   {
      proximity_inactivates++;
      valid_token = 1;
   }
   if(!strcmp(token[0], "baseline_adder"))
   {
      baseline_adder = atof(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "time_correction"))
   {
      time_correction = atof(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "coalesce_in_local_random"))
   {
      coalesce_in_local_random++;
      valid_token = 1;
   }
   if(!strcmp(token[0], "coalesce_in_zoom"))
   {
      coalesce_in_zoom++;
      valid_token = 1;
   }
   if(!strcmp(token[0], "regularization_factor"))
   {
      regularization_factor = atof(token[1]);
      valid_token = 1;
   }
   if(!strcmp(token[0], "centered_points"))
   {
      centered_points++;
      min_s = 1e99;
      max_s = -1e99;
      {
         FILE *load_file = fopen(token[1], "r");
         char line[MAX_INPUT_LIN_LENGTH];
         Solute_center solute_center;
         if(!load_file)
         {
            fprintf(stderr, "can't open centered_points file <%s>\n", token[1]);
            exit(-1);
         }
         while(fgets(line, sizeof(line), load_file) != NULL)
         {
            if(*line != '#')
            {
               solute_center.s = atof(line);
               p = strchr(line, ' ');
               if(!p)
               {
                  fprintf(stderr, "centered_points file format error\n");
                  exit(-1);
               }
               p++;
               solute_center.k = atof(p);
               p = strchr(p, ' ');
               if(!p)
               {
                  fprintf(stderr, "save_concentration file format error\n");
                  exit(-1);
               }
               p++;
               solute_center.r = atof(p);

               //     solute_center.r = .1;

               printf("adding solute_center [%g,%g] r %g\n",
                      solute_center.s,
                      solute_center.k, solute_center.r);

               solute_centers.push_back(solute_center);
               if(solute_center.s - (solute_center.r * 1e-13) < min_s)
               {
                  min_s = solute_center.s - (solute_center.r * 1e-13);
                  if(min_s < 1e-15)
                  {
                     min_s =  1e-15;
                  }
               }
               if(solute_center.s + (solute_center.r * 1e-13) > max_s)
               {
                  max_s = solute_center.s + (solute_center.r * 1e-13);
               }
            }
         }
      }

      printf("max centered solutes %d = new_tree_depth\n", new_tree_depth = solute_centers.size());
      printf("min_s %g max_s %g\n", min_s, max_s);
      valid_token = 1;
   }
   if(!valid_token)
   {
      fprintf(stderr, "unrecognized configuration directive [%s]\n", token[0]);
      exit(-1);
   }
}

void load_config_file(char *s)
{
   FILE *load_file;
   char line[MAX_INPUT_LIN_LENGTH];

   if(NULL == (load_file = fopen(s, "r")))
   {
      fprintf(stderr, "config file open error, can't open \"%s\" for reading\n", s);
      exit(-1);
   }

   while(fgets(line, sizeof(line), load_file) != NULL)
   {
      set_command_opt(line);
   }
   fclose(load_file);
}

void ga_setup(struct ga_data GA_Params, int myrank, US_fe_nnls_t *pass_us_fe_nnls_t)
{
   analysis_type = GA_Params.analysis_type;
   s_rounding = 1e-13;
   if(analysis_type == "GA_MW" ||
      analysis_type == "GA_MW_RA")
   {
      s_rounding = 100;
      ga_mw = 1;
   }
   if(analysis_type == "GA_SC")
   {
      ga_sc = 1;
   }
   our_us_fe_nnls_t = pass_us_fe_nnls_t;
   unsigned int i;
   this_rank = myrank;
   printf("%d: sizeof(double) %d (int) %d (char) %d\n", this_rank, (int)sizeof(double), (int)sizeof(int), (int)sizeof(char));
   printf("%d: ga_setup received %d\n", myrank, this_rank);
   fflush(stdout);
   s_estimate_solutes = GA_Params.initial_solutes;
   if(s_estimate_solutes > MAX_SOLUTES)
   {
      fprintf(stderr, "%d: too many solutes %d max is %d\n", this_rank, s_estimate_solutes, MAX_SOLUTES);
      MPI_Abort(MPI_COMM_WORLD, -21);
      exit(-1);
   }
   if(s_estimate_solutes < 1)
   {
      fprintf(stderr, "%d: at least 1 solutes is needed! %d\n", this_rank, s_estimate_solutes);
      MPI_Abort(MPI_COMM_WORLD, -22);
      exit(-1);
   }
   for(i = 0; i < GA_Params.solute.size(); i++)
   {
      s_estimates[i][0] = GA_Params.solute[i].s_min;
      s_estimates[i][1] = GA_Params.solute[i].s_max;
      ff0_estimates[i][0] = GA_Params.solute[i].ff0_min;
      ff0_estimates[i][1] = GA_Params.solute[i].ff0_max;
   }

   pct_crossover = GA_Params.crossover;
   if(s_estimate_solutes < 2)
   {
      printf("%d: at least 2 solutes is needed for crossover, crossover disabled!\n", this_rank);
      fflush(stdout);
      pct_crossover = 0;
   }

   pct_mutation = 0;
   fitness_param_1 = 0.01;
   if(fitness_param_1 <= 0e0)
   {
      fitness_param_1 = .1;
      if(!myrank)
      {
         printf("solute mutate pct set to %g, use -X to set otherwise\n", fitness_param_1);
      }
   }
   pct_replication = 0;
   pct_subtree_dup = 0;
   pct_plague = GA_Params.plague;
   pct_point_mutation = GA_Params.mutation;
   pct_node_mutation = GA_Params.mutation;

   point_crossover = 1;

   max_generations = GA_Params.generations;
   pop_size = GA_Params.genes;
   if(pop_size < 5)
   {
      pop_size = 10;
   }

   max_arg = 4;
   new_tree_depth = s_estimate_solutes;
   mutate_tree_depth = s_estimate_solutes;

   first_match_gen = INT_MAX;
   best_match_gen = INT_MAX;
   first_match_B = INT_MAX;
   best_match_B = INT_MAX;

   ramped = 0;
   list_best = 1;
   rand_seed = myrank + GA_Params.random_seed;
   elitism = GA_Params.elitism;
   bloat_cutoff = 0;
   debug_level = 0;
   fitness_reinit = 0;
   remove_duplicates = 1;

   tolerance = TOLERANCE;
   beta_divisor = 8e0;

   fitness_param_2 = 0e0;
   fitness_param_3 = 0e0;
   fitness_param_4 = 0e0;
   fitness_param_5 = 0e0;
   fitness_param_6 = 0e0;
   fitness_param_7 = 0e0;

   force_rooted = -1;

   fitness_evals = 0l;
   fitness_skipped = 0l;
   skip_fitness = 1;
   use_alt_tree_hash = 1;

   cachedir = (char *)0;
   initial_diskcache_size = 32;

   inverse_hessian_prob = 0e0;
   conjugate_gradient_prob = 0e0;
   steepest_descent_prob = 0e0;

   elitist_gsm = 0;
   elitist_gsm_generation_start = 0;

   if (ga_sc)
   {
      inverse_hessian_prob = .05;
      conjugate_gradient_prob = .05;
      steepest_descent_prob = .05;
      elitist_gsm = 1;
      elitist_gsm_generation_start = 25;
      inverse_hessian_iter = 10;
      conjugate_gradient_iter = 10;
      steepest_descent_iter = 10;
   } else {
      inverse_hessian_iter = 20;
      conjugate_gradient_iter = 20;
      steepest_descent_iter = 20;
   }

   inverse_hessian_h = .01;
   conjugate_gradient_h = .01;
   steepest_descent_h = .01;


   solute_early_l2_termination = 0e0;
   solute_max_mfem_calls = 0l;

   base_directory = "";

   if (ga_sc)
   {
      concentration_inactivate_prob = 0;
      generation_inactivate_prob = 0;
   } else {
      concentration_inactivate_prob = 1;
      generation_inactivate_prob = .3;
   }
   node_mutate_reactivates = 1;
   inactive_control = 0;

   genetic_use_mem_cache = 0;

   migrate_in_file_count = 0;
   migrate_prob = GA_Params.migration_rate / 100.0;
   if(migrate_prob == 0)
   {
      migrate_prob = 1e-99;
   }
   end_save_best = pop_size;
   save_every_fitness_filename = (char *)0;
   save_every_fitness_file = (FILE *)0;
   save_every_fitness_after_generation = 0;
   save_every_fitness_best = 0;
   clip_bottom = 0e0;
   cell_top = 0e0;
   cell_bottom = 0e0;
   use_random_normal = 1;

   use_vbar = 0e0;
   min_f_ratio = 0e0;
   max_f_ratio = 0e0;

   solute_rounding = 3;
   s_proximity_limit = 0;
   min_s = MIN_S;
   max_s = MAX_S;

   //  memset(use_constants, 0, sizeof(use_constants));

   prune_depth = 0;
   inactivation_deletes = 0;
   proximity_inactivates = 0;
   s_partition = 0;
   f_partition = 0;
   partition_steps = 1;
   partition_wrap = 0;
   partition_merge = 0;
   partition_zoom_steps = 0;
   partition_scaling_factor = 0;
   partition_local_random_steps = 0;
   partition_local_random_radius = .1;
   partition_coalesce_threshold = 0e0;
   partition_coalesce_concentration_threshold = 0e0;
   centered_points = 0;
   baseline_adder = 0e0;
   time_correction = 0e0;
   coalesce_in_local_random = 0;
   coalesce_in_zoom = 0;
   regularization_factor = GA_Params.regularization;
   regularize_on_RMSD = 0;
   //  regularization_factor = 0.05;
   //  printf("regularization %g\n", regularization_factor);
   printf("%d: ga_setup complete\n", myrank);
   fflush(stdout);
   if(!solute_rounding)
   {
      printf("solute_rounding not specified, setting to %d\n", solute_rounding = DEFAULT_SOLUTE_ROUNDING);
   }

   if(concentration_inactivate_prob || generation_inactivate_prob)
   {
      inactive_control = 1;
   }

   if(list_best > pop_size)
   {
      list_best = pop_size;
   }

   pct_replication = 100 - pct_mutation - pct_crossover - pct_subtree_dup - pct_plague;

   if(pct_replication < 0 || pct_replication > 100 ||
      pct_mutation < 0 || pct_mutation > 100 ||
      pct_crossover < 0 || pct_crossover > 100 ||
      pct_subtree_dup < 0 || pct_subtree_dup > 100 ||
      pct_plague < 0 || pct_plague > 100)
   {
      fprintf(stderr,"crossover %d\nmutation %d\nreplication %d\nsubtree dup %d\nplague %d\n",
              pct_crossover,
              pct_mutation,
              pct_replication,
              pct_subtree_dup,
              pct_plague);

      fprintf(stderr,"percentages don't add up to 100\n");
      MPI_Abort(MPI_COMM_WORLD, -5);
      exit(-1);
   }

   cut_mutation = pct_crossover;
   cut_replication = cut_mutation + pct_mutation;
   cut_subtree_dup = cut_replication + pct_replication;
   cut_plague = cut_subtree_dup + pct_subtree_dup;

   if(pop_size < 2)
   {
      fprintf(stderr,"less than two population members!\n");
      MPI_Abort(MPI_COMM_WORLD, -6);
      exit(-1);
   }

   beta = pop_size / beta_divisor;

   print_stats();

   if(myrank)
   {
      int i;
      population *p = (population *)0;
      population *pn[pop_size], *pn2[pop_size];
      double A[pop_size], A2[pop_size];
      unsigned int B[pop_size], B2[pop_size];
      int use_tree_depth = new_tree_depth;
      int half_pop_size = pop_size / 2;
      double half_slope = new_tree_depth / (1e0 * half_pop_size);

      printf("%d: init\n", this_rank);
      fflush(stdout);
      init();
      printf("%d: fitness_init\n", this_rank);
      fflush(stdout);
      fitness_init();
      //   solute_test_perfect_fitness_run();
      if(fitness_param_4 >= 1e0)
      {
         puts("fitness_test");
         fitness_test();
         exit(-1);
      }

      /* initial random population */
      printf("%d: init pop\n", this_rank);
      for(i = 0 ; i < pop_size; i++)
      {
         if(ramped & i < half_pop_size)
         {
            use_tree_depth = (int)floor(1e0 + i * half_slope);
         }
         p = insert_population_node(p, new_random_tree_rooted(use_tree_depth));
         pn[i] = p;
         A[i] = fitness(p->root);
         if(debug_level > 1)
         {
            list_tree(p->root);
            puts("");
         }
         /*
           ((double *)p->data)[FITNESS_CONSTANT_1] = stack[CONSTANTS_STACK][FITNESS_CONSTANT_1];
           ((double *)p->data)[FITNESS_CONSTANT_NEG_1] = stack[CONSTANTS_STACK][FITNESS_CONSTANT_NEG_1];
         */
         fitness_evals++;
         if(A[i] < tolerance)
            A[i] = 0e0;
         p->fitness = A[i];
         p->fitness_valid = 1;
         p->active_points = count_tree_active_points(p->root);
         B[i] = tree_hash(p->root) + (p->active_points << 22);
         // was - tree_hash(p->root) * p->points;
         //     B[i] = tree_hash(p->root) * p->points;
         //     list_tree(p->root);
         //     puts("");
         //     gsm_this_node(STEEPEST_DESCENT, p->root, 5, .01);
         //     gsm_this_node(CONJUGATE_GRADIENT, p->root, 5, .01);
         //     gsm_this_node(INVERSE_HESSIAN, p->root, 5, .01);
         //     exit(0);
      }

      alloc_migrate_buffers();
      // #define SERIALIZATION_TEST
#if defined(SERIALIZATION_TEST)

      puts("serial test");
      printf("pn[0]:");
      list_tree(pn[0]->root);
      puts("");
      printf("pn[3]:");
      list_tree(pn[3]->root);
      puts("");
      puts("init_serial_output");
      bufinit(MIGRATE_OUT);
      puts("add_serial_output");
      buffer_serialize_out_tree(MIGRATE_OUT, pn[0]->root);
      puts("add_serial_output");
      buffer_serialize_out_tree(MIGRATE_OUT, pn[3]->root);
      puts("close_serial_output");
      buffer_close_serial_output(MIGRATE_OUT);
      puts("list_serial_output");
      buffer_list_serial(MIGRATE_OUT);
      bufrewind(MIGRATE_OUT);
      {
         node *n;
         n = buffer_serialize_in_tree(MIGRATE_OUT);
         //     printf("before comparison:");
         //     list_tree(n);
         printf("comparison value %d:\n", cmp_tree(pn[0]->root, n));
         n = buffer_serialize_in_tree(MIGRATE_OUT);
         //     printf("before comparison:");
         //     list_tree(n);
         printf("comparison value %d:\n", cmp_tree(pn[3]->root, n));
         printf("bad comparison value %d:\n", cmp_tree(pn[1]->root, n));
         printf("last read %lu\n", buffer_serialize_in_tree(MIGRATE_OUT));
         buffer_close_serial_input(MIGRATE_OUT);
      }
#endif


      generations(A, B, pn, A2, B2, pn2, max_generations);
   }
}

#ifdef US_GA_USE_MAIN
int main(int argc, char **argv)
{
   int c;

   pct_crossover = 85;
   pct_mutation = 10;
   pct_replication = 3;
   pct_subtree_dup = 2;
   pct_plague = 0;
   pct_node_mutation = 0e0;
   pct_point_mutation = 0e0;

   point_crossover = 0;

   max_generations = 100;
   pop_size = 1000;

   max_arg = 4;
   new_tree_depth = 6;
   mutate_tree_depth = 6;

   first_match_gen = INT_MAX;
   best_match_gen = INT_MAX;
   first_match_B = INT_MAX;
   best_match_B = INT_MAX;

   ramped = 0;
   list_best = 5;
   elitism = 0;
   bloat_cutoff = 0;
   debug_level = 0;
   fitness_reinit = 0;
   remove_duplicates = 0;

   tolerance = TOLERANCE;
   beta_divisor = 8e0;

   fitness_param_1 = 0e0;
   fitness_param_2 = 0e0;
   fitness_param_3 = 0e0;
   fitness_param_4 = 0e0;
   fitness_param_5 = 0e0;
   fitness_param_6 = 0e0;
   fitness_param_7 = 0e0;

   force_rooted = -1;

   fitness_evals = 0l;
   fitness_skipped = 0l;
   skip_fitness = 0;
   use_alt_tree_hash = 0;

   cachedir = (char *)0;
   initial_diskcache_size = 32;

   inverse_hessian_prob = 0e0;
   conjugate_gradient_prob = 0e0;
   steepest_descent_prob = 0e0;

   inverse_hessian_iter = 20;
   conjugate_gradient_iter = 20;
   steepest_descent_iter = 20;

   inverse_hessian_h = .01;
   conjugate_gradient_h = .01;
   steepest_descent_h = .01;

   elitist_gsm = 0;
   elitist_gsm_generation_start = 0;

   solute_early_l2_termination = 0e0;
   solute_max_mfem_calls = 0l;

   base_directory = "";

   concentration_inactivate_prob = 0e0;
   generation_inactivate_prob = 0e0;
   node_mutate_reactivates = 0;
   inactive_control = 0;

   genetic_use_mem_cache = 0;

   migrate_in_file_count = 0;
   migrate_prob = 0;
   end_save_best = 0;
   save_every_fitness_filename = (char *)0;
   save_every_fitness_file = (FILE *)0;
   save_every_fitness_after_generation = 0;
   save_every_fitness_best = 0;
   clip_bottom = 0e0;
   cell_top = 0e0;
   cell_bottom = 0e0;
   use_random_normal = 0;

   use_vbar = 0e0;
   min_f_ratio = 0e0;
   max_f_ratio = 0e0;

   solute_rounding = 0;
   s_proximity_limit = 0;
   min_s = MIN_S;
   max_s = MAX_S;

   //  memset(use_constants, 0, sizeof(use_constants));

   prune_depth = 0;
   inactivation_deletes = 0;
   proximity_inactivates = 0;
   s_partition = 0;
   f_partition = 0;
   partition_steps = 1;
   partition_wrap = 0;
   partition_merge = 0;
   partition_zoom_steps = 0;
   partition_scaling_factor = 0;
   partition_local_random_steps = 0;
   partition_local_random_radius = .1;
   partition_coalesce_threshold = 0e0;
   partition_coalesce_concentration_threshold = 0e0;
   centered_points = 0;
   baseline_adder = 0e0;
   time_correction = 0e0;
   coalesce_in_local_random = 0;
   coalesce_in_zoom = 0;
   regularization_factor = 0;

   while((c = getopt(argc, argv, "h?m:c:s:g:p:N:M:a:rl:R:P:e:t:b:dF:DB:X:Y:Z:E:n:12U:V:THC:A:f:o:G:L:")) != EOF)
   {
      switch(c)
      {
      case 'A' :
         initial_diskcache_size = atoi(optarg);
         break;
      case '1' :
         point_crossover = 1;
         break;
      case 'T' :
         skip_fitness++;
         break;
      case '2' :
         point_crossover = 2;
         break;
      case 'a' :
         max_arg = atoi(optarg);
         if(max_arg < 1)
         {
            fprintf(stderr,"less than one variable!\n");
            exit(-1);
         }
         break;
      case 'b' :
         bloat_cutoff = atoi(optarg);
         break;
      case 'B' :
         beta_divisor = atof(optarg);
         break;
      case 'c' :
         pct_crossover = atoi(optarg);
         break;
      case 'C' :
         cachedir = optarg;
         break;
      case 'd' :
         debug_level++;
         break;
      case 'D' :
         remove_duplicates = 1;
         break;
      case 'e' :
         elitism = atoi(optarg);
         break;
      case 'E' :
         force_rooted = atoi(optarg);
         break;
      case 'f' :
         puts("load config");
         load_config_file(optarg);
         break;
      case 'F' :
         fitness_reinit = atoi(optarg);
         break;
      case 'G' :
         printf("load experiment filenames <%s> not valid\n", optarg);
         //     fitness_load_experiment_filenames(optarg); break;
         exit(-1);
         break;
      case 'H' :
         use_alt_tree_hash++;
         break;
      case 'g' :
         max_generations = atoi(optarg);
         if(max_generations < 0)
         {
            fprintf(stderr,"less than zero generations!\n");
            exit(-1);
         }
         break;
      case 'l' :
         list_best = atoi(optarg);
         break;
      case 'L' :
         if(NULL == (base_directory = (char *)malloc(strlen(optarg) + 1)))
         {
            fprintf(stderr, "malloc failure\n");
            exit(-1);
         }
         strcpy(base_directory, optarg);
         if(chdir(optarg))
         {
            fprintf(stderr, "could not change directory to '%s'\n", optarg);
            exit(-1);
         }
         break;

      case 'n' :
         pct_node_mutation = atof(optarg);
         break;
      case 'N' :
         new_tree_depth = atoi(optarg);
         if(new_tree_depth < 1)
         {
            fprintf(stderr,"less than one new tree depth!\n");
            exit(-1);
         }
         break;
      case 'o' :
         {
            char x[strlen(optarg)+1];
            strcpy(x, optarg);
            set_command_opt(x);
         }
         break;
      case 'M' :
         mutate_tree_depth = atoi(optarg);
         if(mutate_tree_depth < 2)
         {
            fprintf(stderr,"less than two mutate tree depth!\n");
            exit(-1);
         }
         break;
      case 'm' :
         pct_mutation = atoi(optarg);
         break;
      case 'p' :
         pop_size = atoi(optarg);
         break;
      case 'P' :
         pct_plague = atoi(optarg);
         break;
      case 's' :
         pct_subtree_dup = atoi(optarg);
         break;
      case 'r' :
         ramped = 1;
         break;
      case 't' :
         tolerance = atof(optarg);
         break;
      case 'R' :
         rand_seed = atof(optarg);
         break;
      case 'X' :
         fitness_param_1 = atof(optarg);
         break;
      case 'Y' :
         fitness_param_2 = atof(optarg);
         break;
      case 'Z' :
         fitness_param_3 = atof(optarg);
         break;
      case 'U' :
         fitness_param_4 = atof(optarg);
         break;
      case 'V' :
         fitness_param_5 = atof(optarg);
         break;
      case 'h' :
      case '?' :
      default :
         printf(
                "-1       set crossover to 1 point crossover (only GA safe)\n"
                "-2       set crossover to 2 point crossover (only GA safe)\n"
                "-a int    number of terminals (external variables) >0\n"
                "-A int    cache entries (default 32)\n"
                "-b float   max average individual size cutoff/bloat control (default 0 = off)\n"
                "-B float   set exponential ranking selection Beta divisor (Beta = pop_size/divisor, default 8)\n"
                "-c int    crossover percent (default 85)\n"
                "-C string  cachedir (mfemcache)\n"
                "-d       debug/multiple times does more\n"
                "-D       remove duplicates & replace with new random trees to maintain diversity\n"
                "-e int    elitism count (default 0)\n"
                "-E int    rooted element # (default -1 = any function)\n"
                "-f name   configuration file\n"
                "-F int    re init fitness every n-th generation (default 0 = off)\n"
                "-g int    generations >0\n"
                "-G name   mfem datafile\n"
                "-H       use alternate tree hash function\n"
                "-l int    print out best # of pop members (max(5,population_size))\n"
                "-L string  base directory for configuration files\n"
                "-m int    mutation percent (default 10)\n"
                "-M int    mutate tree depth >1\n"
                "-n float   node mutation percent (default 0)\n"
                "-N int    new tree depth >1\n"
                "-o string  option in config file format\n"
                "-p int    population size >1\n"
                "-P int    percent plague (default 0)\n"
                "-r       use ramped half & half initialization\n"
                "-R float   set random seed ( < 0 implies use timer, default)\n"
                "-s int    subtree dup percent\n"
                "-t float   set tolerance (default %g)\n"
                "-T       turn on fitness skipping\n"
                "-U float   set fitness parameter 4 (default 0)\n"
                "-V float   set fitness parameter 5 (default 0)\n"
                "-X float   set fitness parameter 1 (default 0)\n"
                "-Y float   set fitness parameter 2 (default 0)\n"
                "-Z float   set fitness parameter 3 (default 0)\n"
                ,
                TOLERANCE
                );
         exit(0);
         break;
      }
   }

   if(!use_vbar)
   {
      fprintf(stderr, "use_vbar unspecified\n");
      exit(-1);
   }
   if(!solute_rounding)
   {
      printf("solute_rounding not specified, setting to %d\n", solute_rounding = DEFAULT_SOLUTE_ROUNDING);
   }

   if(concentration_inactivate_prob || generation_inactivate_prob)
   {
      inactive_control = 1;
   }

   if(list_best > pop_size)
   {
      list_best = pop_size;
   }

   if(save_every_fitness_best > pop_size)
   {
      save_every_fitness_best = pop_size;
   }

   pct_replication = 100 - pct_mutation - pct_crossover - pct_subtree_dup - pct_plague;

   if(pct_replication < 0 || pct_replication > 100 ||
      pct_mutation < 0 || pct_mutation > 100 ||
      pct_crossover < 0 || pct_crossover > 100 ||
      pct_subtree_dup < 0 || pct_subtree_dup > 100 ||
      pct_plague < 0 || pct_plague > 100)
   {
      fprintf(stderr,"crossover %d\nmutation %d\nreplication %d\nsubtree dup %d\nplague %d\n",
              pct_crossover,
              pct_mutation,
              pct_replication,
              pct_subtree_dup,
              pct_plague);

      fprintf(stderr,"percentages don't add up to 100\n");
      exit(-1);
   }

   cut_mutation = pct_crossover;
   cut_replication = cut_mutation + pct_mutation;
   cut_subtree_dup = cut_replication + pct_replication;
   cut_plague = cut_subtree_dup + pct_subtree_dup;

   if(pop_size < 2)
   {
      fprintf(stderr,"less than two population members!\n");
      exit(-1);
   }

   beta = pop_size / beta_divisor;

   /* sa2d stuff not implemented
      if(s_partition || s_partition_increment) {
      puts("init");
      init();
      puts("fitness_init");
      fitness_init();
      solute_test_perfect_fitness_run();
      partition_run();
      exit(0);
      }
   */

   print_stats();

   {
      int i;
      population *p = (population *)0;
      population *pn[pop_size], *pn2[pop_size];
      double A[pop_size], A2[pop_size];
      unsigned int B[pop_size], B2[pop_size];
      int use_tree_depth = new_tree_depth;
      int half_pop_size = pop_size / 2;
      double half_slope = new_tree_depth / (1e0 * half_pop_size);

      puts("init");
      init();
      puts("fitness_init");
      fitness_init();
      solute_test_perfect_fitness_run();
      if(fitness_param_4 >= 1e0)
      {
         puts("fitness_test");
         fitness_test();
         exit(-1);
      }

      /* initial random population */
      printf("%d: init pop\n", this_rank);
      fflush(stdout);
      for(i = 0 ; i < pop_size; i++)
      {
         if(ramped & i < half_pop_size)
         {
            use_tree_depth = (int)floor(1e0 + i * half_slope);
         }
         p = insert_population_node(p, new_random_tree_rooted(use_tree_depth));
         if(debug_level > 0)
         {
            list_tree(p->root);
            puts("");
         }
         pn[i] = p;
         A[i] = fitness(p->root);
         /*
           ((double *)p->data)[FITNESS_CONSTANT_1] = stack[CONSTANTS_STACK][FITNESS_CONSTANT_1];
           ((double *)p->data)[FITNESS_CONSTANT_NEG_1] = stack[CONSTANTS_STACK][FITNESS_CONSTANT_NEG_1];
         */
         fitness_evals++;
         if(A[i] < tolerance)
            A[i] = 0e0;
         p->fitness = A[i];
         p->fitness_valid = 1;
         p->active_points = count_tree_active_points(p->root);
         B[i] = tree_hash(p->root) + (p->active_points << 22);
         // was - tree_hash(p->root) * p->points;
         //     B[i] = tree_hash(p->root) * p->points;
         //     list_tree(p->root);
         //     puts("");
         //     gsm_this_node(STEEPEST_DESCENT, p->root, 5, .01);
         //     gsm_this_node(CONJUGATE_GRADIENT, p->root, 5, .01);
         //     gsm_this_node(INVERSE_HESSIAN, p->root, 5, .01);
         //     exit(0);
      }
#if defined(SERIALIZATION_TEST)
      puts("serial test");
      printf("pn[0]:");
      list_tree(pn[0]->root);
      puts("");
      printf("pn[3]:");
      list_tree(pn[3]->root);
      puts("");
      puts("init_serial_output");
      init_serial_output("serial_out_test");
      puts("add_serial_output");
      add_serial_output(pn[0]->root);
      puts("add_serial_output");
      add_serial_output(pn[3]->root);
      puts("close_serial_output");
      close_serial_output();
      puts("list_serial_output");
      list_serial_file("serial_out_test");
      init_serial_input("serial_out_test");
      {
         node *n;
         n = get_serial_input();
         printf("comparison value %d:\n", cmp_tree(pn[0]->root, n));
         n = get_serial_input();
         printf("comparison value %d:\n", cmp_tree(pn[3]->root, n));
         printf("bad comparison value %d:\n", cmp_tree(pn[1]->root, n));
         printf("last read %lu\n", get_serial_input());
         close_serial_input();
      }
      exit(0);
#endif


      generations(A, B, pn, A2, B2, pn2, max_generations);
   }
}
#endif
