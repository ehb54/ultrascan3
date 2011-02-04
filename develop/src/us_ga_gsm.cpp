#include "../include/us_fe_nnls_t.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <string.h>
#include <mpi.h>

#include "../include/us_ga.h"
#include "../include/us_ga_stacks.h"
#include "../include/us_ga_round.h"
#include "../include/us_ga_random_normal.h"
#include "../include/us_ga_s_estimate.h"
#include "../include/us_ga_gsm.h"
#include "../include/us_ga_interacting.h"

using namespace std;

unsigned long gsm_fitness_calls;
// #define DEBUG_GSM

long global_iter;
int show_times;
int evenn;
int queries;
int N;
int N_2;
int fitness_type;
int this_iterations;
long iterations_max;
int go;
double min_acceptable;
int prev_iterations;

#define MAX_REPS 100

typedef struct _our_vector 
{
   int     len;
   double* d;
} our_vector;

typedef struct _our_matrix 
{
   int     rows;
   int     cols;
   double* d;
} our_matrix;

void active_exit( int ret ) 
{
   exit( ret );
}

/* gsm stuff */
our_matrix* new_our_matrix( int rows, int cols ) 
{
   our_matrix* m;

   if ( ( m = (our_matrix*)malloc( sizeof( our_matrix ) ) ) == NULL ) 
   {
      fputs( "new_our_matrix malloc error\n", stderr );
      active_exit( -1 );
   }

   m->rows = rows;
   m->cols = cols;
   
   if ( ( m->d = (double*)malloc( sizeof( double ) * rows * cols ) ) == NULL ) 
   {
      fputs( "new_our_matrix malloc error\n", stderr );
      active_exit( -1 );
   }

   return m;
}

void free_our_matrix( our_matrix* m ) 
{
   free( m->d );
   free( m );
}

void set_our_matrix( our_matrix* m, double s ) 
{
   int limit = m->rows * m->cols;
   /* could do this with ceil(log2(len)) memcpy()s */
   for ( int i = 0; i < limit; i++ ) m->d[ i ] = s;
}

void identity_our_matrix( our_matrix* m ) 
{
   set_our_matrix( m, 0.0 );

   for ( int i = 0; i < m->rows; i++ ) 
      m->d[ i * ( m->cols + 1 ) ] = 1.0;
}


void print_our_matrix(our_matrix *m) {
   int i, j;

   for(i = 0; i < m->rows; i++) {
      printf("%d: %d:", this_rank, i);
      for(j = 0; j <= m->cols; j++) {
         printf(" %.6g", m->d[(i * m->cols) + j]);
      }
      puts("");
   }
}

our_vector* new_our_vector( int len ) 
{
   our_vector* v;

   if ( ( v = (our_vector*)malloc( sizeof( our_vector ) ) ) == NULL ) 
   {
      fputs( "new_our_vector malloc error\n", stderr );
      active_exit( -1 );
   }

   v->len = len;
   
   if ( ( v->d = (double*)malloc( sizeof( double ) * len ) ) == NULL ) 
   {
      fputs( "new_our_vector malloc error\n", stderr );
      active_exit( -1 );
   }

   return v;
}

our_vector* vmin;
our_vector* vmax;
our_vector* vlen;

void free_our_vector( our_vector* v ) 
{
   free( v->d );
   free( v );
}

void print_our_vector(our_vector *v) {
   int i;

   for(i = 0; i < v->len; i++) {
      printf("%.8g ", v->d[i]);
   }
   puts("");
}

double l2norm_our_vector( our_vector* v1, our_vector* v2 ) 
{
   double norm = 0.0;

   for ( int i = 0; i < v1->len; i++ ) 
   {
      norm += sq( v1->d[ i ] - v2->d[ i ] );
   }

   return sqrt( norm );
}

void copy_our_vector( our_vector* v1, our_vector* v2 ) 
{
   memcpy( v1->d, v2->d, sizeof( double ) * v1->len );
}

void set_our_vector( our_vector* v1, double s ) 
{
   /* could do this with ceil(log2(len)) memcpy()s */
   for ( int i = 0; i < v1->len; i++ ) v1->d[i] = s;
}

void add_our_vector_vv( our_vector* v1, our_vector* v2 ) 
{
   for ( int i = 0; i < v1->len; i++ ) v1->d[ i ] += v2->d[ i ];
}

void add_our_vector_vs( our_vector* v1, double s ) 
{
   for ( int i = 0; i < v1->len; i++ ) v1->d[ i ] += s;
}

void mult_our_vector_vv( our_vector* v1, our_vector* v2 ) 
{
   for ( int i = 0; i < v1->len; i++ ) v1->d[ i ] *= v2->d[ i ];
}

void mult_our_vector_vs( our_vector* v1, double s ) 
{
   for ( int i = 0; i < v1->len; i++ ) v1->d[ i ] *= s;
}

void add_our_vector_vvv( our_vector* vd, our_vector* v1, our_vector* v2 ) 
{
   for ( int i = 0; i < v1->len; i++ ) vd->d[i] = v1->d[ i ] + v2->d[ i ];
}

void add_our_vector_vvs(our_vector* vd, our_vector* v1, double s ) 
{
   for ( int i = 0; i < v1->len; i++ ) vd->d[i] = v1->d[ i ] + s;
}

void sub_our_vector_vvv( our_vector* vd, our_vector* v1, our_vector* v2 ) 
{
   for ( int i = 0; i < v1->len; i++ ) vd->d[ i ] = v1->d[ i ] - v2->d[ i ];
}

void sub_our_vector_vvs( our_vector* vd, our_vector* v1, double s ) 
{
   // add_our_vector_vvs( vd, v1, -s );
   for ( int i = 0; i < v1->len; i++) vd->d[ i ] = v1->d[ i ] - s;
}

void mult_our_vector_vvv( our_vector* vd, our_vector* v1, our_vector* v2 ) 
{
   for ( int i = 0; i < v1->len; i++ ) vd->d[ i ] = v1->d[ i ] * v2->d[ i ];
}

void mult_our_vector_vvs( our_vector* vd, our_vector* v1, double s ) 
{
   for ( int i = 0; i < v1->len; i++ ) vd->d[ i ] = v1->d[ i ] * s;
}

double dot_our_vector( our_vector* v1, our_vector* v2 ) 
{
   double ret = 0.0;

   for ( intn i = 0; i < v1->len; i++ ) ret += v1->d[ i ] * v2->d[ i ];
   
   return ret;
}


void mult_our_matrix_vmv( our_vector* vd, our_matrix* m, our_vector *vs ) 
{
   set_our_vector( vd, 0.0 );
   for ( int i = 0; i < m->rows; i++ ) 
   {
      for ( intn j = 0; j < m->cols; j++ ) 
      {
         vd->d[ i ] += m->d[ ( i * m->cols ) + j ] * vs->d[ j ];
      }
   }
}

void our_vector_test() {
   our_vector *v1, *v2, *v3;

   v1 = new_our_vector(16);
   v2 = new_our_vector(16);
   v3 = new_our_vector(16);
  
   set_our_vector(v1, 0);
   set_our_vector(v2, 1e0);
   mult_our_vector_vvs(v3, v2, 2.5e-1);
   print_our_vector(v1);
   print_our_vector(v2);
   print_our_vector(v3);
   printf("%d: %.12g %.12g %.12g\n", this_rank, l2norm_our_vector(v1, v2), 
         l2norm_our_vector(v1, v3), l2norm_our_vector(v2,v3));
}

// Need to clip to bucket
void clip_our_vector_scaled( our_vector* v ) 
{
   for ( int i = 0; i < v->len; i += 2 ) 
   {
      int j = i + 1;

      if      ( v->d[ i ] < .1  ) v->d[ i ] = 1;  
      else if ( v->d[ i ] > 200 ) v->d[ i ] = 200; 

      if      ( v->d[ j ] < 1  ) v->d[ j ] = 1; 
      else if ( v->d[ j ] > 50 ) v->d[ j ] = 50; 
   }
}

#define MIN_A 1e-25


long min_gsm_5_1(our_vector *i, double f(our_vector *), 
                 void df(our_vector *, our_vector *), double epsilon, long max_iter) 
{
   /* try to find a local minimum via a gradient search method */
   struct timeval tv1, tv2;
   our_vector *u, *zero;
   //  double t;
   double s1, s2, s3;
   double g_s1, g_s2, g_s3, g_s4;
   our_vector *v_s1, *v_s2, *v_s3, *v_tmp, *v_s4;
   double a, b, x;
   double s1_s2, s1_s3, s2_s3;
   double s1_2, s2_2, s3_2;
   double prev_s2, prev_g_s2;
   double fitness;
   long iter = 0l;
   int reps, last_reps = 0;
   //  int j;

   //  printf("conjugate gradient initial position: ");
   //  print_our_vector(i); 

   zero = new_our_vector(i->len);
   set_our_vector(zero, 0e0);

   u = new_our_vector(i->len);
   df(u, i);
   /*  mult_our_vector_vs(u, 1e0 / l2norm_our_vector(u, zero)); */

   v_s1 = new_our_vector(i->len);
   v_s2 = new_our_vector(i->len);
   v_s3 = new_our_vector(i->len);
   v_s4 = new_our_vector(i->len);

   //  printf("norm %g\n", l2norm_our_vector(u, zero));

   while(l2norm_our_vector(u, zero) >= epsilon && iter++ < max_iter) {

      printf("%d: global %ld, iter %ld fitness %.12g |grad|=%.12g last reps %d\n", i
            this_rank, global_iter++, 
             iter, fitness = f(i), l2norm_our_vector(u, zero), last_reps); fflush(stdout);
      printf("%d: ", this_rank);
      print_our_vector(i);
      fflush(stdout);
      
      if(!fitness) 
      {
         free_our_vector(v_s1);
         free_our_vector(v_s2);
         free_our_vector(v_s3);
         free_our_vector(v_s4);
         free_our_vector(zero);
         free_our_vector(u);
         return(0);
      }
      /* find minimum of f(i - t u) */
      /* alg 5_2 */

      s1 = 0e0;
      copy_our_vector(v_s1, i);
      g_s1 = fitness;

      s3 = 1e0;
      
      s2 = 5e-1;
      mult_our_vector_vvs(v_s2, u, -s2);
      add_our_vector_vv(v_s2, i);
      g_s2 = f(v_s2);

      /* cut down interval until we have a decrease */
      printf("%d: decrease\n", this_rank); 
      fflush(stdout);
      gettimeofday(&tv1, NULL);

      while(g_s2 > g_s1 && s2 - s1 > epsilon) {
         s3 = s2;
         s2 *= 5e-1;
         mult_our_vector_vvs(v_s2, u, -s2);
         add_our_vector_vv(v_s2, i);
         g_s2 = f(v_s2);
      }

      if(s2 - s1 <= epsilon || s3 - s2 <= epsilon) {
         /* ugh, no decrease */
         printf("%d: no initial decrease, terminating pos = ", this_rank);
         /*      print_our_vector(i); */
         free_our_vector(v_s1);
         free_our_vector(v_s2);
         free_our_vector(v_s3);
         free_our_vector(v_s4);
         free_our_vector(zero);
         free_our_vector(u);
         return(max_iter - iter);
      }

      mult_our_vector_vvs(v_s3, u,-s3);
      add_our_vector_vv(v_s3, i);
      g_s3 = f(v_s3);


    
      reps = 0;
      gettimeofday(&tv2, NULL);
      if(show_times)
         printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
      printf("%d: start line search\n", this_rank); 
      fflush(stdout);
      gettimeofday(&tv1, NULL);
      while(s2 - s1 > epsilon && s3 - s2 > epsilon && reps++ < MAX_REPS) {
      

         s1_s2 = 1e0 / (s1 - s2);
         s1_s3 = 1e0 / (s1 - s3);
         s2_s3 = 1e0 / (s2 - s3);

         s1_2 = s1 * s1;
         s2_2 = s2 * s2;
         s3_2 = s3 * s3;

         a = ((g_s1 - g_s3) * s1_s3 - (g_s2 - g_s3) * s2_s3) * s1_s2;

         b = (g_s3 * (s2_2 - s1_2) + g_s2 * (s1_2 - s3_2) + g_s1 * (s3_2 - s2_2)) *
            s1_s2 * s1_s3 * s2_s3;


         if(fabs(a) < MIN_A) {
            /* maybe we should switch to a bisection method? */
            printf("%d: a limit reached\n", this_rank);
            printf("%d: done iter %ld, i = ", this_rank, iter); 
            print_our_vector(i); fflush(stdout);
            free_our_vector(v_s1);
            free_our_vector(v_s2);
            free_our_vector(v_s3);
            free_our_vector(v_s4);
            free_our_vector(zero);
            free_our_vector(u);
            return(max_iter - iter);
         }

         x = -b / (2e0 * a);

         prev_g_s2 = g_s2;
         prev_s2 = s2;


         if(x < s1) 
         {
            if(x < (s1 + s1 - s2)) { /* keep it close */
               x = s1 + s1 - s2;
               if(x < 0) {
                  x = s1 / 2;
               }
            }
            if(x < 0) { /* ugh we're in the wrong direction! */
               printf("%d: unexpected minimum pos %.12g\n", this_rank, x);
               active_exit(-1);
            } 
            /* ok, take x, s1, s2 */
            v_tmp = v_s3;
            v_s3 = v_s2;
            g_s3 = g_s2;
            s3 = s2;
            v_s2 = v_s1;
            g_s2 = g_s1;
            s2 = s1;
            v_s1 = v_tmp;
   
            s1 = x;
            mult_our_vector_vvs(v_s1, u, -s1);
            add_our_vector_vv(v_s1, i);
            g_s1 = f(v_s1);
         } else {
            if(x < s2) 
            {
               /* ok, take s1, x, s2 */
               v_tmp = v_s3;
               v_s3 = v_s2;
               g_s3 = g_s2;
               s3 = s2;
               v_s2 = v_tmp;
   
               s2 = x;
               mult_our_vector_vvs(v_s2, u, -s2);
               add_our_vector_vv(v_s2, i);
               g_s2 = f(v_s2);
            } else {
               if(x < s3) 
               {
                  /* ok, take s2, x, s3 */
                  v_tmp = v_s1;
                  v_s1 = v_s2;
                  g_s1 = g_s2;
                  s1 = s2;
                  v_s2 = v_tmp;
   
                  s2 = x;
                  mult_our_vector_vvs(v_s2, u, -s2);
                  add_our_vector_vv(v_s2, i);
                  g_s2 = f(v_s2);
               } else {
                  /* ugh x >= s3.. well why not? */
                  if(x > (s3 + s3 - s2)) { /* keep it close */
                     mult_our_vector_vvs(v_s4, u, -x);
                     add_our_vector_vv(v_s4, i);
                     g_s4 = f(v_s4);
                     if(g_s4 > g_s2 &&
                        g_s4 > g_s3 &&
                        g_s4 > g_s1) {
                        x = (s3 + s3 - s2);
                     }
                  }
                  /* take s2, s3, x */
       
                  v_tmp = v_s1;
                  v_s1 = v_s2;
                  g_s1 = g_s2;
                  s1 = s2;
                  v_s2 = v_s3;
                  g_s2 = g_s3;
                  s2 = s3;
                  v_s3 = v_tmp;
   
                  s3 = x;
                  mult_our_vector_vvs(v_s3, u, -s3);
                  add_our_vector_vv(v_s3, i);
                  g_s3 = f(v_s3);
               }
            }
         }
      
         if(fabs(prev_g_s2 - g_s2) < epsilon) {
            printf("%d: fabs(g-prev) < epsilon\n", this_rank); fflush(stdout);
            break;
         }
      }
      last_reps = reps;

      if(g_s2 < g_s3 && g_s2 < g_s1) {
         copy_our_vector(i, v_s2);
         g_s4 = g_s2;
      } else {
         if(g_s1 < g_s3) {
            copy_our_vector(i, v_s1);
            g_s4 = g_s1;
         } else {
            copy_our_vector(i, v_s3);
            g_s4 = g_s3;
         }
      }

      if(evenn) {
         switch(queries) {
         case 8 : i->d[(6 * N_2) - 1] = 0;
         case 7 :
         case 6 : i->d[(4 * N_2) - 1] = 0;
         case 5 :
         case 4 : i->d[(2 * N_2) - 1] = 0; 
         default : break;
         }
      }
      gettimeofday(&tv2, NULL);
      if(show_times)
         printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
      printf("%d: df start\n", this_rank); 
      fflush(stdout);
      df(u, i);

   }

   printf("%d: done iter %ld, i = ", this_rank, iter);
   print_our_vector(i);
   free_our_vector(v_s1);
   free_our_vector(v_s2);
   free_our_vector(v_s3);
   free_our_vector(v_s4);
   free_our_vector(zero);
   free_our_vector(u);
   return(0);
}

long min_fr_pr_cgd(our_vector *i, double f(our_vector *), 
      void df(our_vector *, our_vector *), double epsilon, long max_iter) 
{
   /* polak-ribiere version of fletcher-reeves conjugate gradient  */
   struct timeval tv1, tv2;
   our_vector *u, *zero;
   //  double t;
   double s1, s2, s3;
   double g_s1, g_s2, g_s3, g_s4;
   our_vector *v_s1, *v_s2, *v_s3, *v_tmp, *v_s4, *v_h, *v_g;
   double a, b, x;
   double s1_s2, s1_s3, s2_s3;
   double s1_2, s2_2, s3_2;
   double prev_s2, prev_g_s2;
   long iter = 0l;
   int reps, last_reps = 0;
   double gg, ggd;
   double fitness;

   puts("fr.1");
   zero = new_our_vector(i->len);
   set_our_vector(zero, 0e0);

   u = new_our_vector(i->len);
   df(u, i);
   /*  mult_our_vector_vs(u, 1e0 / l2norm_our_vector(u, zero)); */

   v_s1 = new_our_vector(i->len);
   v_s2 = new_our_vector(i->len);
   v_s3 = new_our_vector(i->len);
   v_s4 = new_our_vector(i->len);
   v_h = new_our_vector(i->len);
   v_g = new_our_vector(i->len);

   copy_our_vector(v_g, u);
   copy_our_vector(v_h, u);

   while(l2norm_our_vector(u, zero) >= epsilon && iter++ < max_iter) {
      this_iterations++;
      if(!fitness) {
         free_our_vector(v_s1);
         free_our_vector(v_s2);
         free_our_vector(v_s3);
         free_our_vector(v_s4);
         free_our_vector(v_h);
         free_our_vector(v_g);
         free_our_vector(zero);
         free_our_vector(u);
         return(0);
      }
      /* find minimum of f(i - t u) */
      /* alg 5_2 */

      s1 = 0e0;
      copy_our_vector(v_s1, i);
      g_s1 = fitness;

      s3 = 1e0;
      
      s2 = 5e-1;
      mult_our_vector_vvs(v_s2, u, -s2);
      add_our_vector_vv(v_s2, i);
      print_our_vector(v_s2);
      g_s2 = f(v_s2);

      /* cut down interval until we have a decrease */
      printf("%d: decrease\n", this_rank); 
      fflush(stdout);

      gettimeofday(&tv1, NULL);


      while(g_s2 > g_s1 && s2 - s1 > epsilon) {
         s3 = s2;
         s2 *= 5e-1;
         mult_our_vector_vvs(v_s2, u, -s2);
         add_our_vector_vv(v_s2, i);
         g_s2 = f(v_s2);
      }

      if(s2 - s1 <= epsilon || s3 - s2 <= epsilon) {
         /* ugh, no decrease */
         printf("%d: no initial decrease, terminating pos = ", this_rank);
         free_our_vector(v_s1);
         free_our_vector(v_s2);
         free_our_vector(v_s3);
         free_our_vector(v_s4);
         free_our_vector(v_h);
         free_our_vector(v_g);
         free_our_vector(zero);
         free_our_vector(u);
         return(max_iter - iter);
      }

      mult_our_vector_vvs(v_s3, u,-s3);
      add_our_vector_vv(v_s3, i);
      g_s3 = f(v_s3);


      reps = 0;

      gettimeofday(&tv2, NULL);
      if(show_times)
         printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
      printf("%d: start line search\n", this_rank); 
      fflush(stdout);
      gettimeofday(&tv1, NULL);
    
      while(s2 - s1 > epsilon && s3 - s2 > epsilon && reps++ < MAX_REPS) {
      

         s1_s2 = 1e0 / (s1 - s2);
         s1_s3 = 1e0 / (s1 - s3);
         s2_s3 = 1e0 / (s2 - s3);

         s1_2 = s1 * s1;
         s2_2 = s2 * s2;
         s3_2 = s3 * s3;

         a = ((g_s1 - g_s3) * s1_s3 - (g_s2 - g_s3) * s2_s3) * s1_s2;

         b = (g_s3 * (s2_2 - s1_2) + g_s2 * (s1_2 - s3_2) + g_s1 * (s3_2 - s2_2)) *
            s1_s2 * s1_s3 * s2_s3;


         if(fabs(a) < MIN_A) {
            /* maybe we should switch to a bisection method? */
            printf("%d: a limit reached", this_rank);
            printf("done iter %ld, i = ", iter);
            print_our_vector(i);
            free_our_vector(v_s1);
            free_our_vector(v_s2);
            free_our_vector(v_s3);
            free_our_vector(v_s4);
            free_our_vector(v_h);
            free_our_vector(v_g);
            free_our_vector(zero);
            free_our_vector(u);
            return(max_iter - iter);
         }

         x = -b / (2e0 * a);

         prev_g_s2 = g_s2;
         prev_s2 = s2;


         if(x < s1) {
            if(x < (s1 + s1 - s2)) { /* keep it close */
               x = s1 + s1 - s2;
               if(x < 0) {
                  x = s1 / 2;
               }
            }
            if(x < 0) { /* ugh we're in the wrong direction! */
               printf("%d: unexpected minimum pos %.12g\n", this_rank, x);
               //     active_exit(-1);
               if(s1 < 0) {
                  s1 = 0;
               }
               x = 0;
            } 
            /* ok, take x, s1, s2 */
            v_tmp = v_s3;
            v_s3 = v_s2;
            g_s3 = g_s2;
            s3 = s2;
            v_s2 = v_s1;
            g_s2 = g_s1;
            s2 = s1;
            v_s1 = v_tmp;
   
            s1 = x;
            mult_our_vector_vvs(v_s1, u, -s1);
            add_our_vector_vv(v_s1, i);
            g_s1 = f(v_s1);
         } else {
            if(x < s2) {
               /* ok, take s1, x, s2 */
               v_tmp = v_s3;
               v_s3 = v_s2;
               g_s3 = g_s2;
               s3 = s2;
               v_s2 = v_tmp;
   
               s2 = x;
               mult_our_vector_vvs(v_s2, u, -s2);
               add_our_vector_vv(v_s2, i);
               g_s2 = f(v_s2);
            } else {
               if(x < s3) {
                  /* ok, take s2, x, s3 */
                  v_tmp = v_s1;
                  v_s1 = v_s2;
                  g_s1 = g_s2;
                  s1 = s2;
                  v_s2 = v_tmp;
   
                  s2 = x;
                  mult_our_vector_vvs(v_s2, u, -s2);
                  add_our_vector_vv(v_s2, i);
                  g_s2 = f(v_s2);
               } else {
                  /* ugh x >= s3.. well why not? */
                  if(x > (s3 + s3 - s2)) { /* keep it close */
                     mult_our_vector_vvs(v_s4, u, -x);
                     add_our_vector_vv(v_s4, i);
                     g_s4 = f(v_s4);
                     if(g_s4 > g_s2 &&
                        g_s4 > g_s3 &&
                        g_s4 > g_s1) {
                        x = (s3 + s3 - s2);
                     }
                  }
                  /* take s2, s3, x */
       
                  v_tmp = v_s1;
                  v_s1 = v_s2;
                  g_s1 = g_s2;
                  s1 = s2;
                  v_s2 = v_s3;
                  g_s2 = g_s3;
                  s2 = s3;
                  v_s3 = v_tmp;
   
                  s3 = x;
                  mult_our_vector_vvs(v_s3, u, -s3);
                  add_our_vector_vv(v_s3, i);
                  g_s3 = f(v_s3);
               }
            }
         }
      
      }
      last_reps = reps;
      if(g_s2 < g_s3 && g_s2 < g_s1) {
         copy_our_vector(i, v_s2);
         g_s4 = g_s2;
      } else {
         if(g_s1 < g_s3) {
            copy_our_vector(i, v_s1);
            g_s4 = g_s1;
         } else {
            copy_our_vector(i, v_s3);
            g_s4 = g_s3;
         }
      }


      if(evenn) {
         switch(queries) {
         case 8 : i->d[(6 * N_2) - 1] = 0;
         case 7 :
         case 6 : i->d[(4 * N_2) - 1] = 0;
         case 5 :
         case 4 : i->d[(2 * N_2) - 1] = 0; 
         default : break;
         }
      }

      gettimeofday(&tv2, NULL);
      if(show_times)
         printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
      printf("%d: df start\n", this_rank); 
      fflush(stdout);
      df(u, i);
      puts("conj dir start");
      fflush(stdout);
      gettimeofday(&tv1, NULL);
    
      ggd = dot_our_vector(v_g, v_g);
      if(ggd == 0e0) {
         puts("ggd == 0e0, returning");
         free_our_vector(v_s1);
         free_our_vector(v_s2);
         free_our_vector(v_s3);
         free_our_vector(v_s4);
         free_our_vector(v_h);
         free_our_vector(v_g);
         free_our_vector(zero);
         free_our_vector(u);
         return(0);
      }
      sub_our_vector_vvv(v_s4, u, v_g);
      gg = dot_our_vector(v_s4, u);
      mult_our_vector_vs(v_h, gg / ggd);
      copy_our_vector(v_g, u);
      add_our_vector_vv(v_h, u);
      copy_our_vector(u, v_h);
      gettimeofday(&tv2, NULL);

   }

   printf("%d: done iter %ld, i = ", this_rank, iter);
   print_our_vector(i);
   free_our_vector(v_s1);
   free_our_vector(v_s2);
   free_our_vector(v_s3);
   free_our_vector(v_s4);
   free_our_vector(v_h);
   free_our_vector(v_g);
   free_our_vector(zero);
   free_our_vector(u);
   return(0);
}
  
/* inverse hessian */
///////////////////////////////////////
long min_hessian_bfgs( our_vector* ip, 
                       double      f( our_vector* ), 
                       void        df( our_vector*, our_vector* ), 
                       double      epsilon, 
                       long        max_iter ) 
{
  
   struct timeval tv1, tv2;

   our_matrix* hessian;

   our_vector *u, *zero;
   double s1, s2, s3;
   double g_s1, g_s2, g_s3, g_s4;
   our_vector *v_s1, *v_s2, *v_s3, *v_tmp, *v_s4;
   our_vector *v_g, *v_dg, *v_hdg, *v_p, *v_dx;
   double a, b, x;
   double s1_s2, s1_s3, s2_s3;
   double s1_2, s2_2, s3_2;
   double prev_s2, prev_g_s2;
   long   iter = 0L;
   int    reps, last_reps = 0;
   int    i,j;
   double fitness;
   double fac, fad, fae, sumdg, sumxi;

   zero = new_our_vector( ip->len );
   set_our_vector( zero, 0.0 );

   u = new_our_vector( ip->len );
   df( u, ip );                         //*************

   v_p = new_our_vector( ip->len ); /* the new point */

   hessian = new_our_matrix( ip->len, ip->len );
   identity_our_matrix( hessian );
  
   v_s1 = new_our_vector( ip->len );
   v_s2 = new_our_vector( ip->len );
   v_s3 = new_our_vector( ip->len );
   v_s4 = new_our_vector( ip->len );

   v_g   = new_our_vector( ip->len ); /* new gradient */
   v_dg  = new_our_vector( ip->len ); /* dgradient */
   v_hdg = new_our_vector( ip->len ); /* hdg */

   v_p  = new_our_vector( ip->len ); /* the new point */
   v_dx = new_our_vector( ip->len ); /* p - ip, the direction (xi) */


   while ( l2norm_our_vector( u, zero) >= epsilon && iter++ < max_iter) 
   {
      this_iterations++;   // Printout only
      fitness = f( ip );    //////////////

      /* find minimum of f(ip - t u) */
      /* alg 5_2 */
      if ( fitness == 0.0 ) 
      {
         free_our_vector( v_s1    );
         free_our_vector( v_s2    );
         free_our_vector( v_s3    );
         free_our_vector( v_s4    );
         free_our_vector( v_g     );
         free_our_vector( v_dg    );
         free_our_vector( v_hdg   );
         free_our_vector( v_p     );
         free_our_vector( v_dx    );
         free_our_vector( zero    );
         free_our_vector( u       );
         free_our_matrix( hessian );
         return 0;  // ignored
      }

      copy_our_vector( v_s1, ip );
      g_s1 = fitness;

      s1 = 0.0;
      s2 = 0.5;
      s3 = 1.0;

      mult_our_vector_vvs( v_s2, u, -s2 );
      add_our_vector_vv  ( v_s2, ip );     //  v_s2 = u * -s2 + ip
      g_s2 = f( v_s2 );       ////////////////

      /* cut down interval until we have a decrease */

      while ( g_s2    > g_s1 &&    // while fitness improving
              s2 - s1 > epsilon ) 
      {
         s3 = s2;
         s2 *= 0.5;
         mult_our_vector_vvs( v_s2, u, -s2 );
         add_our_vector_vv  ( v_s2, ip );     // v_s2 = u * -s2 + ip
         g_s2 = f( v_s2 );  ////////////////////
      }

      if ( s2 - s1 <= epsilon || s3 - s2 <= epsilon ) 
      {
         /* ugh, no decrease */
         printf("%d: no initial decrease, terminating pos = ", this_rank);

         free_our_vector( v_s1    );
         free_our_vector( v_s2    );
         free_our_vector( v_s3    );
         free_our_vector( v_s4    );
         free_our_vector( v_g     );
         free_our_vector( v_dg    );
         free_our_vector( v_hdg   );
         free_our_vector( v_p     );
         free_our_vector( v_dx    );
         free_our_vector( zero    );
         free_our_vector( u       );
         free_our_matrix( hessian );
         return max_iter - iter;  // ignored
      }

      mult_our_vector_vvs( v_s3, u, -s3 );
      add_our_vector_vv  ( v_s3, ip );         // v_s3 = u * -s3 + ip
      g_s3 = f( v_s3 );        ///////////////////////////
    
      reps = 0;

      while ( s2 - s1 > epsilon && 
              s3 - s2 > epsilon && 
              reps++ < MAX_REPS ) 
      {
         s1_s2 = 1.0 / ( s1 - s2 );
         s1_s3 = 1.0 / ( s1 - s3 );
         s2_s3 = 1.0 / ( s2 - s3 );

         s1_2 = sq( s1 );
         s2_2 = sq( s2 );
         s3_2 = sq( s3 );

         a = ( ( g_s1 - g_s3 ) * s1_s3 - 
               ( g_s2 - g_s3 ) * s2_s3
             ) * s1_s2;

         b = ( g_s3 * ( s2_2 - s1_2 ) + 
               g_s2 * ( s1_2 - s3_2 ) + 
               g_s1 * ( s3_2 - s2_2 )
             ) *
             s1_s2 * s1_s3 * s2_s3;


         if ( fabs( a ) < MIN_A ) 
         {
            /* maybe we should switch to a bisection method? */
            free_our_vector( v_s1    );
            free_our_vector( v_s2    );
            free_our_vector( v_s3    );
            free_our_vector( v_s4    );
            free_our_vector( v_g     );
            free_our_vector( v_dg    );
            free_our_vector( v_hdg   );
            free_our_vector( v_p     );
            free_our_vector( v_dx    );
            free_our_vector( zero    );
            free_our_vector( u       );
            free_our_matrix( hessian );
            return max_iter - iter;  // ignored
         }

         x = -b / ( 2.0 * a );

         prev_g_s2 = g_s2;
         prev_s2   = s2;

         if ( x < s1 ) 
         {
            if ( x < ( s1 + s1 - s2 )) 
            { /* keep it close */
               x = s1 + s1 - s2;
               if ( x < 0 )  x = s1 / 2.0;
            }

            if ( x < 0 ) 
            { /* ugh we're in the wrong direction! */
               if ( s1 < 0 )  s1 = 0.0; 
               x = 0;
            } 
            /* ok, take x, s1, s2 */
            v_tmp = v_s3;
            v_s3  = v_s2;
            g_s3  = g_s2;
            s3    = s2;
            v_s2  = v_s1;
            g_s2  = g_s1;
            s2    = s1;
            v_s1  = v_tmp;
            s1    = x;

            mult_our_vector_vvs( v_s1, u, -s1 );
            add_our_vector_vv  ( v_s1, ip );

            g_s1 = f(v_s1);        ////////////////
         } 
         else 
         {
            if ( x < s2 ) 
            {
               /* ok, take s1, x, s2 */
               v_tmp = v_s3;
               v_s3  = v_s2;
               g_s3  = g_s2;
               s3    = s2;
               v_s2  = v_tmp;
               s2    = x;

               mult_our_vector_vvs( v_s2, u, -s2 );
               add_our_vector_vv  ( v_s2, ip );

               g_s2 = f( v_s2 );      //////////////
            } 
            else 
            {
               if ( x < s3 ) 
               {
                  /* ok, take s2, x, s3 */
                  v_tmp = v_s1;
                  v_s1  = v_s2;
                  g_s1  = g_s2;
                  s1    = s2;
                  v_s2  = v_tmp;
                  s2    = x;

                  mult_our_vector_vvs( v_s2, u, -s2 );
                  add_our_vector_vv  ( v_s2, ip );         // v_s2 = u * -s2 + ip

                  g_s2 = f( v_s2 );   //////////////
               } 
               else 
               {
                  /* ugh x >= s3.. well why not? */
                  if ( x > ( s3 + s3 - s2 ) ) 
                  { /* keep it close */
                     mult_our_vector_vvs( v_s4, u, -x );
                     add_our_vector_vv  ( v_s4, ip );        // v_s4 = u * -x + ip

                     g_s4 = f( v_s4 );

                     if ( g_s4 > g_s2 &&
                          g_s4 > g_s3 &&
                          g_s4 > g_s1 ) 
                     {
                        x = s3 + s3 - s2;
                     }
                  }
                  /* take s2, s3, x */
       
                  v_tmp = v_s1;
                  v_s1  = v_s2;
                  g_s1  = g_s2;
                  s1    = s2;
                  v_s2  = v_s3;
                  g_s2  = g_s3;
                  s2    = s3;
                  v_s3  = v_tmp;
                  s3    = x;

                  mult_our_vector_vvs( v_s3, u, -s3 );
                  add_our_vector_vv  ( v_s3, ip );         // v_s3 = u * -s3 + ip
                  g_s3 = f( v_s3 );      ////////////////
               }
            }
         }
      
         if ( fabs( prev_g_s2 - g_s2 ) < epsilon ) break; 
      }

      last_reps = reps;
      
      if ( g_s2 < g_s3 && g_s2 < g_s1 ) 
      {
         copy_our_vector( v_p, v_s2 );
         g_s4 = g_s2;
      } 
      else 
      {
         if ( g_s1 < g_s3 ) 
         {
            copy_our_vector( v_p, v_s1 );
            g_s4 = g_s1;
         } 
         else 
         {
            copy_our_vector( v_p, v_s3 );
            g_s4 = g_s3;
         }
      }


      /* Always false
      if ( evenn ) 
      {
         switch ( queries ) 
         {
            case 8 : v_p->d[ ( 6 * N_2 ) - 1 ] = 0;
            case 7 :
            case 6 : v_p->d[ ( 4 * N_2 ) - 1 ] = 0;
            case 5 :
            case 4 : v_p->d[ ( 2 * N_2 ) - 1 ] = 0; 
            default : break;
         }
      }
      */

      df( v_g, v_p );       //*****      /* new gradient in v_g (old in u) */


      sub_our_vector_vvv ( v_dx, v_p, ip ); /* calc dx */    // v_dx = v_p - ip
      copy_our_vector    ( ip, v_p );                        // ip   = v_p 
      sub_our_vector_vvv ( v_dg, v_g, u ); /* dgradient */   // v_dg = vg - u
      mult_our_matrix_vmv( v_hdg, hessian, v_dg );

      fac   = dot_our_vector( v_dg, v_dx  );
      fae   = dot_our_vector( v_dg, v_hdg );
      sumdg = dot_our_vector( v_dg, v_dg  );
      sumxi = dot_our_vector( v_dx, v_dx  );

      if ( fac > sqrt( epsilon * sumdg * sumxi ) ) 
      {
         fac = 1.0 / fac;
         fad = 1.0 / fae;

         for ( i = 0; i < v_dg->len; i++ ) 
         {
            v_dg->d[ i ] = fac * v_dx->d[ i ] - fad * v_hdg->d[ i ];
         }

         for ( i = 0; i < v_dg->len; i++ ) 
         {
            for ( j = i; j < v_dg->len; j++ ) 
            {
               hessian->d[ ( i * hessian->cols ) + j ] +=
                  fac * v_dx ->d[ i ] * v_dx ->d[ j ] - 
                  fad * v_hdg->d[ i ] * v_hdg->d[ j ] +
                  fae * v_dg ->d[ i ] * v_dg-> d[ j ];

               hessian->d[ ( j * hessian->cols ) + i ] =   // symmetrical
               hessian->d[ ( i * hessian->cols ) + j ] ;
            }
         }
      }

      mult_our_matrix_vmv( u, hessian, v_g );
   }  // end while

   free_our_vector( v_s1    );
   free_our_vector( v_s2    );
   free_our_vector( v_s3    );
   free_our_vector( v_s4    );
   free_our_vector( v_g     );
   free_our_vector( v_dg    );
   free_our_vector( v_hdg   );
   free_our_vector( v_p     );
   free_our_vector( v_dx    );
   free_our_vector( zero    );
   free_our_vector( u       );
   free_our_matrix( hessian );
   return 0;   // ignored
}

double h;        // the h in terms of significant digits = 0.01
double h2_r;     // 1/2h

double lamm_gsm_f( our_vector* v )   // returns fitness of v
{
   stacks_init();
   clip_our_vector_scaled( v );

   for ( int i = 0; i < v->len; i++ ) 
   {
      if ( i % 2 ) 
         push_stack( RESULT_STACK, v->d[ i ] );
      else 
         push_stack( RESULT_STACK, v->d[ i ] * s_rounding );
   }

   Solute solute;
   vector< Solute > solute_vector;
   int nonzeros = 0;
   
   for ( int i = sp[ RESULT_STACK ] - 2; i >= 0; i -= 2 ) 
   {
      solute.s = stack[ RESULT_STACK ][ i ];
      solute.k = stack[ RESULT_STACK ][ i + 1 ];

      solute_vector << solute;
   }

   Simulation_values           sv;
   vector< Simulation > sim;

   for ( int e = 0; e < our_us_fe_nnls_t->experiment.size(); e++ ) 
   {
      sve << sv;
      vector < struct mfem_data > use_experiment;
      use_experiment << our_us_fe_nnls_t->experiment[ e ];

      sve[ e ] = our_us_fe_nnls_t->calc_residuals( use_experiment, solute_vector, 0.0, 1, e );
   } // for e

   vector< Solute > tot_solutes = sve[ 0 ].solutes;
   double           result      = 0;

   for ( int e = 0; e < our_us_fe_nnls_t->experiment.size(); e++ ) 
   {
      result += sve[ e ].variance;
 
      for ( int f = 0; f < tot_solutes.size(); f++ ) 
         tot_solutes[ f ].c += sve[ e ].solutes[ f ].c;
   }

   if ( tot_solutes.size() > 0 ) 
   {
      for ( int u = 0; u < tot_solutes.size(); u++ ) 
         if ( tot_solutes[ u ].c > SOLUTE_CONCENTRATION_THRESHOLD ) nonzeros++;
   } 
   else 
      result = 1.0e100;

   return result * ( 1.0 + sq( regularization_factor * nonzeros ) );
}

void lamm_gsm_df( our_vector* vd, our_vector* v ) 
{
   // Compute partials for each our_vector element

   // h is 0.01

   for ( int i = 0; i < v->len; i++ ) // Iterate each s and k
   {
      double save = v->d[ i ];
      v->d[ i ] -= h;

      double y0 = lamm_gsm_f( v );  // Calc fitness value -h
      v->d[ i ] = save + h;
      
      double y2 = lamm_gsm_f( v );  // Calc fitness value +h
      
      vd->d[ i ] = ( y2 - y0 ) * h2_r;  // The derifitive
      v ->d[ i ] = save;  // Restore to original
   }
}

void node_set_to_our_vector(node *n, our_vector *v) {
   int i;
   double *d;
   for(i = 0; i < v->len; i+=2) {
      while(n && !((char *)n->data)[SOLUTE_DATA_ACTIVE_OFS]) {
         n = n->children[0];
      }
      if(!n) {
         fprintf(stderr, "node set to our_vector unexpected termination\n");
         exit(-1);
      }
      d = (double *)(n->data);
      if (ga_sc)
      {
#if defined(DYNAMIC_ROUNDING)
         double spacing = (d[3] - d[2]) / (d[4] - 1);
         d[0] = d[2] + floor(.5 + (v->d[i] - d[2]) / spacing) * spacing;
#else
         d[0] = floorn(v->d[i], ff0_estimates[i / 2][0], solute_rounding);
#endif
         d[1] = ff0_estimates[i / 2][0];
      } else {
         d[0] = floorn(v->d[i], s_rounding, solute_rounding);
         d[1] = floorn(v->d[i+1], 1, solute_rounding);
      }
      n = n->children[0];
   }
}

double eps;
// #define EPS_DEFAULT 1e-7
#define EPS_DEFAULT 1e-10

void gsm_this_node( int gsm_method, node* n, int iter, double use_h ) 
{
   our_vector* v;        // this is the our_vector holding the evaluation
   our_vector* vd;       // this is the our_vector holding the evaluation

   global_iter = 0;

   if ( ga_sc ) 
   {
      use_h /= 10;
   }

   h    = use_h;
   h2_r =  1.0 / ( 2.0 * h );

   gsm_fitness_calls = 0;
   stacks_init();
   evaluate( n, pass_fit_args );

   int   j = 0;
   node* m = n;
   printf("%d: gsm this node\n", this_rank); fflush(stdout);

   v  = new_our_vector( sp[ RESULT_STACK ] );
   vd = new_our_vector( sp[ RESULT_STACK ] );

   if ( ga_sc )
   {
      vmin = new_our_vector( sp[ RESULT_STACK ] );
      vmax = new_our_vector( sp[ RESULT_STACK ] );
      vlen = new_our_vector( sp[ RESULT_STACK ] );
   }

   for ( int i = 0; i < v->len; i+= 2, j++ ) 
   {
      while ( ! ( (char *)m->data )[ SOLUTE_DATA_ACTIVE_OFS ]) 
      {
         j++;
         m = m->children[ 0 ];
         if ( ! m ) exit( -1 ); 
      }

      s_estimate_indices[ i / 2 ] = j;
   }
  
   printf("%d: >>>>>> gsm_this_node initial tree:", this_rank);
   list_tree( n );

   for( int i = 0; i < v->len; i += 2 ) 
   {
      v->d[ i ] = stack[ RESULT_STACK ][ v->len - i - 2 ];

      if ( ! ga_mw && ! ga_sc ) 
      {
         v->d[ i ] /= s_rounding;
      }
      
      v->d[ i + 1 ] = stack[ RESULT_STACK ][ v->len - 1 - i ];
      v->d[ i + 1 ] *= 1.0;   // Does nothing!!

      if ( ga_sc ) 
      {
         // ga_sc we need to rescale 
         double scale;
         scale        = fabs( v->d[ i ] - 2 * v->d[ i ] );
         vmin->d[ i ] = v->d[ i ] - scale;
         
         if ( vmin->d[ i ] < s_estimates[ i / 2 ][ 0 ] ) 
         {
            vmin->d[ i ] = s_estimates[ i / 2 ][ 0 ];
         }

         vmax->d[ i ] = v->d[ i ] + scale;
         
         if ( vmax->d[ i ] > s_estimates[ i / 2 ][ 1 ] ) 
         {
            vmax->d[ i ] = s_estimates[ i / 2 ][ 1 ];
         }

         vlen->d[ i ] = vmax->d[ i ] - vmin->d[ i ];
         v->d[ i ] = ( v->d[ i ] - vmin->d[ i ]) / vlen->d[ i ]; // so we have from zero to 1
         v->d[ i + 1 ] = 0;
      }
   }
   
   clip_our_vector_scaled( v );

   switch( gsm_method ) 
   {

     case CONJUGATE_GRADIENT: min_fr_pr_cgd   ( v, lamm_gsm_f, lamm_gsm_df, EPS_DEFAULT, iter ); break;
     case STEEPEST_DESCENT  : min_gsm_5_1     ( v, lamm_gsm_f, lamm_gsm_df, EPS_DEFAULT, iter ); break;
     case INVERSE_HESSIAN   : min_hessian_bfgs( v, lamm_gsm_f, lamm_gsm_df, EPS_DEFAULT, iter ); break;
     default                : fprintf( stderr, "gsm_this_node unknown gsm method\n" ); exit(-1); break;
   }

   clip_our_vector_scaled( v );
   
   for ( i = 0; i < v->len; i++ ) 
   {
      if ( i % 2 ) 
      {
         v->d[ i ] *= 1.0; // Does nothing

         if ( ga_sc) 
         {
            v->d[ i ] = ff0_estimates[ ( i - 1 ) / 2 ][ 0 ];
         }
      } 
      else 
      {
         if ( ga_sc ) 
         {
            v->d[ i ] = v->d[ i ] * vlen->d[ i ] + vmin->d[ i ];
         } 
         else 
         {
            if ( ! ga_mw ) 
            {
               v->d[ i ] *= s_rounding;
            }
         }
      }
   }

   node_set_to_our_vector( n, v );
   list_tree( n );
   free_our_vector( vd );
   free_our_vector( v );
   
   if ( ga_sc ) 
   {
      free_our_vector( vmin );
      free_our_vector( vmax );
      free_our_vector( vlen );
   }
}

