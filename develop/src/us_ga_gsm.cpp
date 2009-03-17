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

typedef struct _our_vector {
   int len;
   double *d;
} our_vector;

typedef struct _our_matrix {
   int rows, cols;
   double *d;
} our_matrix;

void active_exit(int ret) {
   exit(ret);
}

/* gsm stuff */
our_matrix *new_our_matrix(int rows, int cols) {
   our_matrix *m;
   if((m = (our_matrix *)malloc(sizeof(our_matrix))) == NULL) {
      fputs("new_our_matrix malloc error\n", stderr);
      active_exit(-1);
   }
   m->rows = rows;
   m->cols = cols;
   if((m->d = (double *)malloc(sizeof(double) * rows * cols)) == NULL) {
      fputs("new_our_matrix malloc error\n", stderr);
      active_exit(-1);
   }
   return(m);
}

void free_our_matrix(our_matrix *m) {
   free(m->d);
   free(m);
}

void set_our_matrix(our_matrix *m, double s) {
   int i, limit;
   limit = m->rows * m->cols;
   /* could do this with ceil(log2(len)) memcpy()s */
   for(i = 0; i < limit; i++) {
      m->d[i] = s;
   }
}

void identity_our_matrix(our_matrix *m) {
   int i;
   set_our_matrix(m, 0e0);

   for(i = 0; i < m->rows; i++) {
      m->d[i * (m->cols + 1)] = 1e0;
   }
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

our_vector *new_our_vector(int len) {
   our_vector *v;
   if((v = (our_vector *)malloc(sizeof(our_vector))) == NULL) {
      fputs("new_our_vector malloc error\n", stderr);
      active_exit(-1);
   }
   v->len = len;
   if((v->d = (double *)malloc(sizeof(double) * len)) == NULL) {
      fputs("new_our_vector malloc error\n", stderr);
      active_exit(-1);
   }
   return(v);
}

our_vector *vmin, *vmax, *vlen;

void free_our_vector(our_vector *v) {
   free(v->d);
   free(v);
}

void print_our_vector(our_vector *v) {
   int i;

   for(i = 0; i < v->len; i++) {
      printf("%.8g ", v->d[i]);
   }
   puts("");
}

double l2norm_our_vector(our_vector *v1, our_vector *v2) {
   double norm = 0e0;
   int i;
#if !defined(SUPRESS_ERROR_CHECKING)
   if(v1->len != v2->len) {
      fputs("l2norm_our_vector incompatable", stderr);
      active_exit(-1);
   }
#endif
   for(i = 0; i < v1->len; i++) {
      norm += pow(v1->d[i] - v2->d[i], 2e0);
   }
   norm = sqrt(norm);
   return(norm);
}

void copy_our_vector(our_vector *v1, our_vector *v2) {
#if !defined(SUPRESS_ERROR_CHECKING)
   if(v1->len != v2->len) {
      fputs("copy_our_vector incompatable", stderr);
      active_exit(-1);
   }
#endif
   memcpy(v1->d, v2->d, sizeof(double) * v1->len);
}

void set_our_vector(our_vector *v1, double s) {
   int i;
   /* could do this with ceil(log2(len)) memcpy()s */
   for(i = 0; i < v1->len; i++) {
      v1->d[i] = s;
   }
}

void add_our_vector_vv(our_vector *v1, our_vector *v2) {
   int i;
#if !defined(SUPRESS_ERROR_CHECKING)
   if(v1->len != v2->len) {
      fputs("add_our_vector_vv incompatable", stderr);
      active_exit(-1);
   }
#endif
   for(i = 0; i < v1->len; i++) {
      v1->d[i] += v2->d[i];
   }
}

void add_our_vector_vs(our_vector *v1, double s) {
   int i;
   for(i = 0; i < v1->len; i++) {
      v1->d[i] += s;
   }
}

void mult_our_vector_vv(our_vector *v1, our_vector *v2) {
   int i;
#if !defined(SUPRESS_ERROR_CHECKING)
   if(v1->len != v2->len) {
      fputs("mult_our_vector_vv incompatable", stderr);
      active_exit(-1);
   }
#endif
   for(i = 0; i < v1->len; i++) {
      v1->d[i] *= v2->d[i];
   }
}

void mult_our_vector_vs(our_vector *v1, double s) {
   int i;
   for(i = 0; i < v1->len; i++) {
      v1->d[i] *= s;
   }
}

void add_our_vector_vvv(our_vector *vd, our_vector *v1, our_vector *v2) {
   int i;
#if !defined(SUPRESS_ERROR_CHECKING)
   if(v1->len != v2->len || vd->len != v1->len) {
      fputs("add_our_vector_vvv incompatable", stderr);
      active_exit(-1);
   }
#endif
   for(i = 0; i < v1->len; i++) {
      vd->d[i] = v1->d[i] + v2->d[i];
   }
}

void add_our_vector_vvs(our_vector *vd, our_vector *v1, double s) {
   int i;
   for(i = 0; i < v1->len; i++) {
      vd->d[i] = v1->d[i] + s;
   }
}

void sub_our_vector_vvv(our_vector *vd, our_vector *v1, our_vector *v2) {
   int i;
#if !defined(SUPRESS_ERROR_CHECKING)
   if(v1->len != v2->len || vd->len != v1->len) {
      fputs("add_our_vector_vvv incompatable", stderr);
      active_exit(-1);
   }
#endif
   for(i = 0; i < v1->len; i++) {
      vd->d[i] = v1->d[i] - v2->d[i];
   }
}

void sub_our_vector_vvs(our_vector *vd, our_vector *v1, double s) {
   int i;
   for(i = 0; i < v1->len; i++) {
      vd->d[i] = v1->d[i] - s;
   }
}

void mult_our_vector_vvv(our_vector *vd, our_vector *v1, our_vector *v2) {
   int i;
#if !defined(SUPRESS_ERROR_CHECKING)
   if(v1->len != v2->len || vd->len != v1->len) {
      fputs("mult_our_vector_vvv incompatable", stderr);
      active_exit(-1);
   }
#endif
   for(i = 0; i < v1->len; i++) {
      vd->d[i] = v1->d[i] * v2->d[i];
   }
}

void mult_our_vector_vvs(our_vector *vd, our_vector *v1, double s) {
   int i;
   for(i = 0; i < v1->len; i++) {
      vd->d[i] = v1->d[i] * s;
   }
}

double dot_our_vector(our_vector *v1, our_vector *v2) {
   int i;
   double ret = 0e0;
   for(i = 0; i < v1->len; i++) {
      ret += v1->d[i] * v2->d[i];
   }
   return(ret);
}


void mult_our_matrix_vmv(our_vector *vd, our_matrix *m, our_vector *vs) {
   int i, j;

#if !defined(SUPRESS_ERROR_CHECKING)
   if(vs->len != m->cols || vd->len != m->rows) {
      fputs("mult_our_matrix_vmv incompatable", stderr);
      active_exit(-1);
   }
#endif

   set_our_vector(vd, 0e0);
   for(i = 0; i < m->rows; i++) {
      for(j = 0; j < m->cols; j++) {
	 vd->d[i] += m->d[(i * m->cols) + j] * vs->d[j];
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
   printf("%d: %.12g %.12g %.12g\n", this_rank, l2norm_our_vector(v1, v2), l2norm_our_vector(v1, v3), l2norm_our_vector(v2,v3));
}

void clip_our_vector_scaled(our_vector *v) {
   // clip to s_estimate range
   int i, j, solute;
   for(i = 0; i < v->len; i+=2) {
      solute = s_estimate_indices[i / 2];
      j = i+1;
      if (ga_sc) 
      {
	 if (v->d[i] < 0) 
	 {
	    v->d[i] = 0;
	 }
	 if (v->d[i] > 1) 
	 {
	    v->d[i] = 1;
	 }
      } else {
	 if(ga_mw) {
	    if(v->d[i] < .01) {
	       v->d[i] = .01;
	    } 
	 } else {
	    if(v->d[i] < .1) {
	       v->d[i] = 1;
	    } else {
	       if(v->d[i] > 200) {
		  v->d[i] = 200;
	       }
	    }
	 }
	 if(v->d[j] < 1) {
	    v->d[j] = 1;
	 } else {
	    if(v->d[j] > 50) {
	       v->d[j] = 50;
	    }
	 }
      }
   }
}

#define MIN_A 1e-25


long min_gsm_5_1(our_vector *i, double f(our_vector *), void df(our_vector *, our_vector *), double epsilon, long max_iter) {
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

#if defined(DEBUG_GSM)
   printf("initial gradient ||=%.12g : ", l2norm_our_vector(u, zero)); 
   print_our_vector(u); 
#endif
  
   v_s1 = new_our_vector(i->len);
   v_s2 = new_our_vector(i->len);
   v_s3 = new_our_vector(i->len);
   v_s4 = new_our_vector(i->len);

   //  printf("norm %g\n", l2norm_our_vector(u, zero));

   while(l2norm_our_vector(u, zero) >= epsilon && iter++ < max_iter) {
#if defined(DEBUG_GSM)
      printf("begin\t{%.12g,%.12g,%.12g} = {%.12g,%.12g,%.12g}\n", s1, s2, s3, g_s1, g_s2, g_s3);
#endif
      printf("%d: global %ld, iter %ld fitness %.12g |grad|=%.12g last reps %d\n", this_rank, global_iter++, 
	     iter, fitness = f(i), l2norm_our_vector(u, zero), last_reps); fflush(stdout);
      printf("%d: ", this_rank);
      print_our_vector(i);
      fflush(stdout);
      if(!fitness) {
	 free_our_vector(v_s1);
	 free_our_vector(v_s2);
	 free_our_vector(v_s3);
	 free_our_vector(v_s4);
	 free_our_vector(zero);
	 free_our_vector(u);
	 return(0);
      }
      /*    printf("i : ");
	    print_our_vector(i); */
      /* find minimum of f(i - t u) */
      /* alg 5_2 */

      s1 = 0e0;
      copy_our_vector(v_s1, i);
      g_s1 = fitness;

      s3 = 1e0;
      
      s2 = 5e-1;
      mult_our_vector_vvs(v_s2, u, -s2);
      /*    printf("v_s2 after mult_our_vector_vvs : ");
	    print_our_vector(v_s2); */
      /*    printf("i : "); */
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


#if defined(DEBUG_GSM)
      printf("pre\t{%.12g,%.12g,%.12g} = {%.12g,%.12g,%.12g}\n", s1, s2, s3, g_s1, g_s2, g_s3); 
#endif

    
      reps = 0;
      gettimeofday(&tv2, NULL);
      if(show_times)
	 printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
      printf("%d: start line search\n", this_rank); 
      fflush(stdout);
      gettimeofday(&tv1, NULL);
      while(s2 - s1 > epsilon && s3 - s2 > epsilon && reps++ < MAX_REPS) {
      
#if defined(DEBUG_GSM)
	 printf("start\t{%.12g,%.12g,%.12g} = {%.12g,%.12g,%.12g}\n", s1, s2, s3, g_s1, g_s2, g_s3);
#endif

	 s1_s2 = 1e0 / (s1 - s2);
	 s1_s3 = 1e0 / (s1 - s3);
	 s2_s3 = 1e0 / (s2 - s3);

	 s1_2 = s1 * s1;
	 s2_2 = s2 * s2;
	 s3_2 = s3 * s3;

	 a = ((g_s1 - g_s3) * s1_s3 - (g_s2 - g_s3) * s2_s3) * s1_s2;

	 /* printf("new a = %.12g\n",a); */
	 b = (g_s3 * (s2_2 - s1_2) + g_s2 * (s1_2 - s3_2) + g_s1 * (s3_2 - s2_2)) *
	    s1_s2 * s1_s3 * s2_s3;

	 /* printf("new b = %.12g\n",b); */

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

#if defined(DEBUG_GSM)
	 printf("new x = %.12g\n", x);
#endif

	 if(x < s1) {
#if defined(DEBUG_GSM)
	    printf("p1 ");
#endif
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
	    if(x < s2) {
#if defined(DEBUG_GSM)
	       printf("p2 ");
#endif
	       /* ok, take s1, x, s2 */
	       v_tmp = v_s3;
	       v_s3 = v_s2;
	       g_s3 = g_s2;
	       s3 = s2;
	       v_s2 = v_tmp;
	
	       s2 = x;
	       /*	  printf("x = %.12g\n", x); */
	       mult_our_vector_vvs(v_s2, u, -s2);
	       add_our_vector_vv(v_s2, i);
	       g_s2 = f(v_s2);
	    } else {
	       if(x < s3) {
#if defined(DEBUG_GSM)
		  printf("p3 ");
#endif
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
#if defined(DEBUG_GSM)
		  printf("p4 ");
#endif
	    
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
	 /*      puts(""); */
      }
      last_reps = reps;
      /*    printf("v_s2 ");
	    print_our_vector(v_s2); */

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
      /*    mult_our_vector_vs(u, 1e0 / l2norm_our_vector(u, zero)); */
#if defined(DEBUG_GSM)
      printf("end\t{%.12g,%.12g,%.12g} = {%.12g,%.12g,%.12g}\n", s1, s2, s3, g_s1, g_s2, g_s3);
#endif

      if(!(global_iter % 5)) {
	 //      int r, s, j;
      
	 printf("%d: query4i_intermediate|alg4|%s|%d|%.12g|%d|%d|%d|%d|%d|%.12g|%.12g|%.12g|%.12g|%ld|%d|%d|%d|%.12g|%ld|%ld", this_rank,
		(f(i) <= 0e0) ? "success" : "fail",
		N, 
		f(i),
		queries,
		fitness_type,
		0,
		this_iterations + prev_iterations, 
		go, 
		0e0,
		0e0,
		0e0,
		0e0,
		iterations_max,
		0,
		0,
		0,
		min_acceptable,
		0L, 0L
		); 
	 puts(""); 
      }
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

long min_fr_pr_cgd(our_vector *i, double f(our_vector *), void df(our_vector *, our_vector *), double epsilon, long max_iter) {
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

   /*  printf("initial position: ");
       print_our_vector(i); */

   puts("fr.1");
   zero = new_our_vector(i->len);
   set_our_vector(zero, 0e0);

   u = new_our_vector(i->len);
   df(u, i);
   /*  mult_our_vector_vs(u, 1e0 / l2norm_our_vector(u, zero)); */

   /*  printf("initial gradient ||=%.12g : ", l2norm_our_vector(u, zero));
       print_our_vector(u); */
  
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
      printf("%d: global %ld, iter %ld fitness %.12g |grad|=%.12g last reps %d\n", this_rank, global_iter++, 
	     iter, fitness = f(i), l2norm_our_vector(u, zero), last_reps); fflush(stdout);
      printf("%d: ", this_rank);
      print_our_vector(i);
#if defined(DEBUG_GSM)
      print_our_vector(u);
#endif
      fflush(stdout);
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
      /*    printf("i : ");
	    print_our_vector(i); */
      /* find minimum of f(i - t u) */
      /* alg 5_2 */

      s1 = 0e0;
      copy_our_vector(v_s1, i);
      g_s1 = fitness;

      s3 = 1e0;
      
      s2 = 5e-1;
      mult_our_vector_vvs(v_s2, u, -s2);
      /*    printf("v_s2 after mult_our_vector_vvs : ");
	    print_our_vector(v_s2); */
      /*    printf("i : "); */
      add_our_vector_vv(v_s2, i);
      print_our_vector(v_s2);
      g_s2 = f(v_s2);

      /* cut down interval until we have a decrease */
      printf("%d: decrease\n", this_rank); 
      fflush(stdout);

      gettimeofday(&tv1, NULL);

#if defined(DEBUG_GSM)
      printf("s values\t{%.12g,%.12g,%.12g} = {%.12g,%.12g,%.12g}\n", s1, s2, s3, g_s1, g_s2, g_s3);
#endif

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
	 free_our_vector(v_h);
	 free_our_vector(v_g);
	 free_our_vector(zero);
	 free_our_vector(u);
	 return(max_iter - iter);
      }

      mult_our_vector_vvs(v_s3, u,-s3);
      add_our_vector_vv(v_s3, i);
      g_s3 = f(v_s3);


#if defined(DEBUG_GSM)
      printf("pre\t{%.12g,%.12g,%.12g} = {%.12g,%.12g,%.12g}\n", s1, s2, s3, g_s1, g_s2, g_s3); 
#endif

      reps = 0;

      gettimeofday(&tv2, NULL);
      if(show_times)
	 printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
      printf("%d: start line search\n", this_rank); 
      fflush(stdout);
      gettimeofday(&tv1, NULL);
    
      while(s2 - s1 > epsilon && s3 - s2 > epsilon && reps++ < MAX_REPS) {
      
#if defined(DEBUG_GSM)
	 printf("start\t{%.12g,%.12g,%.12g} = {%.12g,%.12g,%.12g}\n", s1, s2, s3, g_s1, g_s2, g_s3);
#endif

	 s1_s2 = 1e0 / (s1 - s2);
	 s1_s3 = 1e0 / (s1 - s3);
	 s2_s3 = 1e0 / (s2 - s3);

	 s1_2 = s1 * s1;
	 s2_2 = s2 * s2;
	 s3_2 = s3 * s3;

	 a = ((g_s1 - g_s3) * s1_s3 - (g_s2 - g_s3) * s2_s3) * s1_s2;

	 /* printf("new a = %.12g\n",a); */
	 b = (g_s3 * (s2_2 - s1_2) + g_s2 * (s1_2 - s3_2) + g_s1 * (s3_2 - s2_2)) *
	    s1_s2 * s1_s3 * s2_s3;

	 /* printf("new b = %.12g\n",b); */

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

#if defined(DEBUG_GSM)
	 printf("new x = %.12g\n", x);
#endif

	 if(x < s1) {
#if defined(DEBUG_GSM)
	    printf("p1 ");
#endif
	    if(x < (s1 + s1 - s2)) { /* keep it close */
	       x = s1 + s1 - s2;
	       if(x < 0) {
		  x = s1 / 2;
	       }
	    }
	    if(x < 0) { /* ugh we're in the wrong direction! */
	       printf("%d: unexpected minimum pos %.12g\n", this_rank, x);
	       //	  active_exit(-1);
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
#if defined(DEBUG_GSM)
	       printf("p2 ");
#endif
	       /* ok, take s1, x, s2 */
	       v_tmp = v_s3;
	       v_s3 = v_s2;
	       g_s3 = g_s2;
	       s3 = s2;
	       v_s2 = v_tmp;
	
	       s2 = x;
	       /*	  printf("x = %.12g\n", x); */
	       mult_our_vector_vvs(v_s2, u, -s2);
	       add_our_vector_vv(v_s2, i);
	       g_s2 = f(v_s2);
	    } else {
	       if(x < s3) {
#if defined(DEBUG_GSM)
		  printf("p3 ");
#endif
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
#if defined(DEBUG_GSM)
		  printf("p4 ");
#endif
	    
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
	 /*      puts(""); */
      }
      last_reps = reps;
      /*    printf("v_s2 ");
	    print_our_vector(v_s2); */
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
      /*    printf("ggd = %.12g\n", ggd);*/
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
      /*    printf("gg = %.12g\n", gg);*/
      mult_our_vector_vs(v_h, gg / ggd);
      copy_our_vector(v_g, u);
      add_our_vector_vv(v_h, u);
      copy_our_vector(u, v_h);
      gettimeofday(&tv2, NULL);
      if(show_times)
	 printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
      fflush(stdout);

      if(!(global_iter % 5)) {
	 //      int r, s, j;
      
	 printf("%d: query4i_intermediate|alg3|%s|%d|%.12g|%d|%d|%d|%d|%d|%.12g|%.12g|%.12g|%.12g|%ld|%d|%d|%d|%.12g|%ld|%ld", this_rank,
		(f(i) <= 0e0) ? "success" : "fail",
		N, 
		f(i),
		queries,
		fitness_type,
		0,
		this_iterations + prev_iterations, 
		go, 
		0e0,
		0e0,
		0e0,
		0e0,
		iterations_max,
		0,
		0,
		0,
		min_acceptable,
		0L, 0L
		); 
		
	 puts(""); 
      }
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


long min_hessian_bfgs(our_vector *ip, double f(our_vector *), void df(our_vector *, our_vector *), double epsilon, long max_iter) {
  
   struct timeval tv1, tv2;

   our_matrix *hessian;

   our_vector *u, *zero;
   //  double t;
   double s1, s2, s3;
   double g_s1, g_s2, g_s3, g_s4;
   our_vector *v_s1, *v_s2, *v_s3, *v_tmp, *v_s4;
   our_vector *v_g, *v_dg, *v_hdg, *v_p, *v_dx;
   double a, b, x;
   double s1_s2, s1_s3, s2_s3;
   double s1_2, s2_2, s3_2;
   double prev_s2, prev_g_s2;
   long iter = 0l;
   int reps, last_reps = 0;
   int i,j;
   double fitness;
   double fac, fad, fae, sumdg, sumxi;

   /*  printf("initial position: ");
       print_our_vector(i); */

   zero = new_our_vector(ip->len);
   set_our_vector(zero, 0e0);

   u = new_our_vector(ip->len);
   df(u, ip);

   v_p = new_our_vector(ip->len); /* the new point */

   /*  mult_our_vector_vs(u, 1e0 / l2norm_our_vector(u, zero)); */

   /*  printf("initial gradient ||=%.12g : ", l2norm_our_vector(u, zero)); */
   /*  print_our_vector(u); */

   hessian = new_our_matrix(ip->len, ip->len);
   identity_our_matrix(hessian);
  
   v_s1 = new_our_vector(ip->len);
   v_s2 = new_our_vector(ip->len);
   v_s3 = new_our_vector(ip->len);
   v_s4 = new_our_vector(ip->len);

   v_g = new_our_vector(ip->len); /* new gradient */
   v_dg = new_our_vector(ip->len); /* dgradient */
   v_hdg = new_our_vector(ip->len); /* hdg */

   v_p = new_our_vector(ip->len); /* the new point */
   v_dx = new_our_vector(ip->len); /* p - ip, the direction (xi) */


   while(l2norm_our_vector(u, zero) >= epsilon && iter++ < max_iter) {
      this_iterations++;
#if defined(DEBUG_GSM)
      printf("begin\t{%.12g,%.12g,%.12g} = {%.12g,%.12g,%.12g}\n", s1, s2, s3, g_s1, g_s2, g_s3);
#endif
      printf("%d: global %ld, iter %ld fitness %.12g |grad|=%.12g last reps %d\n", this_rank, global_iter++, 
	     iter, fitness = f(ip), l2norm_our_vector(u, zero), last_reps); fflush(stdout);
      printf("%d: ", this_rank);
      print_our_vector(ip);
      /*    print_our_vector(u); */
      fflush(stdout);
      /*    printf("ip : ");
	    print_our_vector(ip); */
      /* find minimum of f(ip - t u) */
      /* alg 5_2 */
      if(!fitness) {
	 free_our_vector(v_s1);
	 free_our_vector(v_s2);
	 free_our_vector(v_s3);
	 free_our_vector(v_s4);
	 free_our_vector(v_g);
	 free_our_vector(v_dg);
	 free_our_vector(v_hdg);
	 free_our_vector(v_p);
	 free_our_vector(v_dx);
	 free_our_vector(zero);
	 free_our_vector(u);
	 free_our_matrix(hessian);
	 return(0);
      }

      s1 = 0e0;
      copy_our_vector(v_s1, ip);
      g_s1 = fitness;

      s3 = 1e0;
      
      s2 = 5e-1;
      mult_our_vector_vvs(v_s2, u, -s2);
      /*    printf("v_s2 after mult_our_vector_vvs : ");
	    print_our_vector(v_s2); */
      /*    printf("i : "); */
      add_our_vector_vv(v_s2, ip);
      g_s2 = f(v_s2);

      /* cut down interval until we have a decrease */
      printf("%d: decrease\n", this_rank); 
      fflush(stdout);
      gettimeofday(&tv1, NULL);

      while(g_s2 > g_s1 && s2 - s1 > epsilon) {
	 s3 = s2;
	 s2 *= 5e-1;
	 mult_our_vector_vvs(v_s2, u, -s2);
	 add_our_vector_vv(v_s2, ip);
	 g_s2 = f(v_s2);
      }

      if(s2 - s1 <= epsilon || s3 - s2 <= epsilon) {
	 /* ugh, no decrease */
	 printf("%d: no initial decrease, terminating pos = ", this_rank);
	 /*      print_our_vector(ip); */
	 free_our_vector(v_s1);
	 free_our_vector(v_s2);
	 free_our_vector(v_s3);
	 free_our_vector(v_s4);
	 free_our_vector(v_g);
	 free_our_vector(v_dg);
	 free_our_vector(v_hdg);
	 free_our_vector(v_p);
	 free_our_vector(v_dx);
	 free_our_vector(zero);
	 free_our_vector(u);
	 free_our_matrix(hessian);
	 return(max_iter - iter);
      }

      mult_our_vector_vvs(v_s3, u,-s3);
      add_our_vector_vv(v_s3, ip);
      g_s3 = f(v_s3);


#if defined(DEBUG_GSM)
      printf("pre\t{%.12g,%.12g,%.12g} = {%.12g,%.12g,%.12g}\n", s1, s2, s3, g_s1, g_s2, g_s3); 
#endif

    
      reps = 0;

      gettimeofday(&tv2, NULL);
      if(show_times)
	 printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
      printf("%d: start line search\n", this_rank); 
      fflush(stdout);
      gettimeofday(&tv1, NULL);

      while(s2 - s1 > epsilon && s3 - s2 > epsilon && reps++ < MAX_REPS) {
      
#if defined(DEBUG_GSM)
	 printf("start\t{%.12g,%.12g,%.12g} = {%.12g,%.12g,%.12g}\n", s1, s2, s3, g_s1, g_s2, g_s3);
#endif

	 s1_s2 = 1e0 / (s1 - s2);
	 s1_s3 = 1e0 / (s1 - s3);
	 s2_s3 = 1e0 / (s2 - s3);

	 s1_2 = s1 * s1;
	 s2_2 = s2 * s2;
	 s3_2 = s3 * s3;

	 a = ((g_s1 - g_s3) * s1_s3 - (g_s2 - g_s3) * s2_s3) * s1_s2;

	 /* printf("new a = %.12g\n",a); */
	 b = (g_s3 * (s2_2 - s1_2) + g_s2 * (s1_2 - s3_2) + g_s1 * (s3_2 - s2_2)) *
	    s1_s2 * s1_s3 * s2_s3;

	 /* printf("new b = %.12g\n",b); */

	 if(fabs(a) < MIN_A) {
	    /* maybe we should switch to a bisection method? */
	    printf("%d: a limit reached", this_rank);
	    printf("done iter %ld, i = ", iter);
	    print_our_vector(ip);
	    free_our_vector(v_s1);
	    free_our_vector(v_s2);
	    free_our_vector(v_s3);
	    free_our_vector(v_s4);
	    free_our_vector(v_g);
	    free_our_vector(v_dg);
	    free_our_vector(v_hdg);
	    free_our_vector(v_p);
	    free_our_vector(v_dx);
	    free_our_vector(zero);
	    free_our_vector(u);
	    free_our_matrix(hessian);
	    return(max_iter - iter);
	 }

	 x = -b / (2e0 * a);

	 prev_g_s2 = g_s2;
	 prev_s2 = s2;

#if defined(DEBUG_GSM)
	 printf("new x = %.12g\n", x);
#endif

	 if(x < s1) {
#if defined(DEBUG_GSM)
	    printf("p1 ");
#endif
	    if(x < (s1 + s1 - s2)) { /* keep it close */
	       x = s1 + s1 - s2;
	       if(x < 0) {
		  x = s1 / 2;
	       }
	    }
	    if(x < 0) { /* ugh we're in the wrong direction! */
	       printf("%d: unexpected minimum pos %.12g\n", this_rank, x);
	       if(s1 < 0) {
		  s1 = 0;
	       }
	       x = 0;
	       //	  active_exit(-1);
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
	    add_our_vector_vv(v_s1, ip);
	    g_s1 = f(v_s1);
	 } else {
	    if(x < s2) {
#if defined(DEBUG_GSM)
	       printf("p2 ");
#endif
	       /* ok, take s1, x, s2 */
	       v_tmp = v_s3;
	       v_s3 = v_s2;
	       g_s3 = g_s2;
	       s3 = s2;
	       v_s2 = v_tmp;
	
	       s2 = x;
	       /*	  printf("x = %.12g\n", x); */
	       mult_our_vector_vvs(v_s2, u, -s2);
	       add_our_vector_vv(v_s2, ip);
	       g_s2 = f(v_s2);
	    } else {
	       if(x < s3) {
#if defined(DEBUG_GSM)
		  printf("p3 ");
#endif
		  /* ok, take s2, x, s3 */
		  v_tmp = v_s1;
		  v_s1 = v_s2;
		  g_s1 = g_s2;
		  s1 = s2;
		  v_s2 = v_tmp;
	
		  s2 = x;
		  mult_our_vector_vvs(v_s2, u, -s2);
		  add_our_vector_vv(v_s2, ip);
		  g_s2 = f(v_s2);
	       } else {
		  /* ugh x >= s3.. well why not? */
		  if(x > (s3 + s3 - s2)) { /* keep it close */
		     mult_our_vector_vvs(v_s4, u, -x);
		     add_our_vector_vv(v_s4, ip);
		     g_s4 = f(v_s4);
		     if(g_s4 > g_s2 &&
			g_s4 > g_s3 &&
			g_s4 > g_s1) {
			x = (s3 + s3 - s2);
		     }
		  }
		  /* take s2, s3, x */
#if defined(DEBUG_GSM)
		  printf("p4 ");
#endif
	    
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
		  add_our_vector_vv(v_s3, ip);
		  g_s3 = f(v_s3);
	       }
	    }
	 }
      
	 if(fabs(prev_g_s2 - g_s2) < epsilon) {
	    printf("%d: fabs(g-prev) < epsilon\n", this_rank); fflush(stdout);
	    break;
	 }
	 /*      puts(""); */
      }
      last_reps = reps;
      if(g_s2 < g_s3 && g_s2 < g_s1) {
	 copy_our_vector(v_p, v_s2);
	 g_s4 = g_s2;
      } else {
	 if(g_s1 < g_s3) {
	    copy_our_vector(v_p, v_s1);
	    g_s4 = g_s1;
	 } else {
	    copy_our_vector(v_p, v_s3);
	    g_s4 = g_s3;
	 }
      }


      if(evenn) {
	 switch(queries) {
	 case 8 : v_p->d[(6 * N_2) - 1] = 0;
	 case 7 :
	 case 6 : v_p->d[(4 * N_2) - 1] = 0;
	 case 5 :
	 case 4 : v_p->d[(2 * N_2) - 1] = 0; 
	 default : break;
	 }
      }

      /*    printf("v_dx:"); print_our_vector(v_dx); */

      gettimeofday(&tv2, NULL);
      if(show_times)
	 printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
      printf("%d: df start\n", this_rank); 
      fflush(stdout);
      df(v_g, v_p);                 /* new gradient in v_g (old in u) */
      printf("%d: hessian start\n", this_rank);
      gettimeofday(&tv1, NULL);

      sub_our_vector_vvv(v_dx, v_p, ip); /* calc dx */
      copy_our_vector(ip, v_p);

      sub_our_vector_vvv(v_dg, v_g, u); /* dgradient */
      /*    printf("v_dg:"); print_our_vector(v_dg); */

      mult_our_matrix_vmv(v_hdg, hessian, v_dg);
      /*    printf("v_hdg:"); print_our_vector(v_hdg); */

      fac = dot_our_vector(v_dg, v_dx);
      fae = dot_our_vector(v_dg, v_hdg);
      sumdg = dot_our_vector(v_dg, v_dg);
      sumxi = dot_our_vector(v_dx, v_dx);

      /*    printf("fac %.12g fae %.12g sumdg %.12g sumxi %.12g\n",
	    fac, fae, sumdg, sumxi); */

      if(fac > sqrt(epsilon * sumdg * sumxi)) {
	 fac = 1e0/fac;
	 fad = 1e0/fae;

	 for(i = 0; i < v_dg->len; i++) {
	    v_dg->d[i] = fac * v_dx->d[i] - fad * v_hdg->d[i];
	 }
	 for(i = 0; i < v_dg->len; i++) {
	    for(j = i; j < v_dg->len; j++) {
	       hessian->d[(i * hessian->cols) + j] +=
		  fac * v_dx->d[i] * v_dx->d[j] - 
		  fad * v_hdg->d[i] * v_hdg->d[j] +
		  fae * v_dg->d[i] * v_dg->d[j];
	       hessian->d[(j * hessian->cols) + i] =
		  hessian->d[(i * hessian->cols) + j];
	    }
	 }
      }
      mult_our_matrix_vmv(u, hessian, v_g);
      gettimeofday(&tv2, NULL);
      if(show_times)
	 printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
      fflush(stdout);

      /*    mult_our_vector_vs(u, -1e0); */
      /*
	print_our_vector(u);
	print_our_matrix(hessian);
	print_our_vector(v_g);
	printf("|u|=%.12g\n", l2norm_our_vector(u, zero)); 
      */

#if defined(DEBUG_GSM)
      printf("end\t{%.12g,%.12g,%.12g} = {%.12g,%.12g,%.12g}\n", s1, s2, s3, g_s1, g_s2, g_s3);
#endif

      if(!(global_iter % 5)) {
	 //      int r, s, j;
      
	 printf("%d: query4i_intermediate|alg4|%s|%d|%.12g|%d|%d|%.12g|%d|%d|%.12g|%.12g|%.12g|%.12g|%ld|%d|%d|%d|%.12g|%ld|%ld",
		this_rank,
		(f(ip) <= 0e0) ? "success" : "fail",
		N, 
		f(ip),
		queries,
		fitness_type,
		0e0,
		this_iterations + prev_iterations, 
		go, 
		0e0,
		0e0,
		0e0,
		0e0,
		iterations_max,
		0,
		0,
		0,
		min_acceptable,
		0L, 0L
		); 
      
	 puts(""); fflush(stdout);
      }
   }



   printf("%d done iter %ld, i = ", this_rank, iter);
   print_our_vector(ip);
   free_our_vector(v_s1);
   free_our_vector(v_s2);
   free_our_vector(v_s3);
   free_our_vector(v_s4);
   free_our_vector(v_g);
   free_our_vector(v_dg);
   free_our_vector(v_hdg);
   free_our_vector(v_p);
   free_our_vector(v_dx);
   free_our_vector(zero);
   free_our_vector(u);
   free_our_matrix(hessian);
   return(0);
}

double h;        // the h in terms of significant digits
double h2_r;     // 1/2h

double lamm_gsm_f(our_vector *v) {
   int i;
   double result;
   stacks_init();
   clip_our_vector_scaled(v);
   for(i = 0; i < v->len; i++) {
      if (ga_sc) 
      {
	 if(i % 2) {
	    push_stack(RESULT_STACK, ff0_estimates[(i - 1)/2][0]);
	 } else {
	    //	    printf("lamm_gsm_f push value vdi %d %g %g %g %g\n", i, v->d[i], vlen->d[i], vmin->d[i], v->d[i] *  vlen->d[i] + vmin->d[i]);
	    push_stack(RESULT_STACK, v->d[i] * vlen->d[i] + vmin->d[i]);
	 }
      } else {
	 if(i % 2) {
	    push_stack(RESULT_STACK, v->d[i] * 1e0);
	 } else {
	    if(ga_mw) {
	       push_stack(RESULT_STACK, v->d[i]);
	    } else {
	       push_stack(RESULT_STACK, v->d[i] * s_rounding);
	    }
	 }
      }
   }
   //  print_stack(RESULT_STACK);
   //  printf("gsm_f @ ");
   //  for(i = 0; i < v->len; i++) {
   //    printf(" %g", v->d[i]);
   //  }
   Solute solute;
   //  printf("%d: fitness_solute rsp %d\n", this_rank, sp[RESULT_STACK]); fflush(stdout);
   vector<Solute> solute_vector;
   int nonzeros = 0;
   unsigned int u;
   for(i = sp[RESULT_STACK] - 2; i >= 0; i-=2) {
      solute.s = stack[RESULT_STACK][i];
      solute.k = stack[RESULT_STACK][i + 1];
      if(ga_mw) {
	 solute.s = 
	    pow(pow((solute.s * our_us_fe_nnls_t->experiment[0].vbar20)/(AVOGADRO * M_PI), 2e0)/6e0,1e0/3e0) * 
	    (1e0 - our_us_fe_nnls_t->experiment[0].density * 
	     our_us_fe_nnls_t->experiment[0].vbar20) / 
	    (3e0 * solute.k * our_us_fe_nnls_t->experiment[0].vbar20 * 
	     our_us_fe_nnls_t->experiment[0].viscosity * .01);
      }
      solute_vector.push_back(solute);
   }
   if (ga_sc) 
   {
      // reverse
      vector<Solute> reverse;
      for (u = 0; u < solute_vector.size(); u++) 
      {
	 //	  printf("pushing back u=%u\n", solute_vector.size() - u - 1); fflush(stdout);
	 reverse.push_back(solute_vector[solute_vector.size() - u - 1]);
      }
      solute_vector = reverse;
   }
	  
   if(debug_level > 1) {
      for(u = 0; u < solute_vector.size(); u++) {
	 printf("%d: gsm solute_vector[%u] = %g %g\n", this_rank, u, solute_vector[u].s, solute_vector[u].k); fflush(stdout);
      }
   }
   unsigned int e;
   Simulation_values sv;
   vector<Simulation_values> sve;
   for(e = 0; e < our_us_fe_nnls_t->experiment.size(); e++) {
      sve.push_back(sv);
      vector <struct mfem_data> use_experiment;
      use_experiment.push_back(our_us_fe_nnls_t->experiment[e]);
      //    printf("%d: call calc_residuals\n", this_rank); fflush(stdout);
      //    Simulation_values sv = our_us_fe_nnls_t->calc_residuals(our_us_fe_nnls_t->experiment, solute_vector, 0e0, 1, e);
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
      if(debug_level > 1) {
	 printf("%d: exp %d variance %g\n", this_rank, e, sve[e].variance); fflush(stdout);
      }
      //  printf("%d: back from calc_residuals\n", this_rank); fflush(stdout);
   } // for e
   vector<Solute> tot_solutes = sve[0].solutes;
   result = sve[0].variance;
   if(our_us_fe_nnls_t->experiment.size() > 1) {
      for(e = 1; e < our_us_fe_nnls_t->experiment.size(); e++) {
	 result += sve[e].variance;
	 unsigned int f;
	 for(f = 0; f < tot_solutes.size(); f++) {
	    tot_solutes[f].c += sve[e].solutes[f].c;
	 }
      }
   }
   if(debug_level > 0) {
      printf("%d: gsm results %g\n", this_rank, result); fflush(stdout);
   }
   if (!ga_sc) 
   {
      if(tot_solutes.size()) {
	 for(u = 0; u < tot_solutes.size(); u++) {
	    //      printf("results tot_solutes[%d] = %g %g %g\n", u, tot_solutes[u].s, tot_solutes[u].k, tot_solutes[u].c);
	    if(tot_solutes[u].c > SOLUTE_CONCENTRATION_THRESHOLD) {
	       nonzeros++;
	    }
	 }
      } else {
	 result = 1e100;
      }
   }
   //  printf("%d: gsm fitness_solute done\n", this_rank); fflush(stdout);
   if(!ga_sc && regularization_factor && nonzeros) {
      if(regularize_on_RMSD) {
	 result += result * regularization_factor * nonzeros;
      } else {
	 result += pow(sqrt(result) * regularization_factor * nonzeros, 2.0);
      }
   }
   return result;
}

void lamm_gsm_df(our_vector *vd, our_vector *v) {
   int i;
   double y0 = 1e0, y2 = 1e0;
   double sav_ve;

#if !defined(SUPRESS_ERROR_CHECKING)
   if(vd->len != v->len) {
      fputs("lamm_gsm_df incompatable", stderr);
      active_exit(-1);
   }
#endif

   for(i = 0; i < v->len; i++) {
      // compute partials for each our_vector element
      sav_ve = v->d[i];
      v->d[i] -= h;
      //    printf("backside %i:", i);
      //    print_our_vector(v);
      y0 = lamm_gsm_f(v);
      //    printf("backside fitness = %g\n", y0);
      v->d[i] = sav_ve + h;
      //    printf("forside %i:", i);
      //    print_our_vector(v);
      y2 = lamm_gsm_f(v);
      //    printf("foreside fitness = %g\n", y2);
      vd->d[i] = (y2 - y0) * h2_r;
      //    printf("partial_gradient @ %d = %g\n", i, vd->d[i]);
      v->d[i] = sav_ve;
      //    printf("end:", i);
      //    print_our_vector(v);
      if (ga_sc) 
      {
	 vd->d[i+1] = 0;
	 i++;
      }
   }
   //  printf("after gradient search:");
   //  print_our_vector(vd);
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
	 d[0] = floorn(v->d[i], ff0_estimates[i / 2][0], solute_rounding);
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

void gsm_this_node(int gsm_method, node *n, int iter, double use_h) {
   int i, j;
   our_vector *v;       // this is the our_vector holding the evaluation
   our_vector *vd;       // this is the our_vector holding the evaluation

   global_iter = 0;

   if (ga_sc) 
   {
      use_h /= 10;
   }
   h = use_h;
   h2_r =  (1e0 / (2e0 * h));

   gsm_fitness_calls = 0;
   stacks_init();
   evaluate(n, pass_fit_args);

   j = 0;
   node *m = n;
   printf("%d: gsm this node\n", this_rank); fflush(stdout);

   v = new_our_vector(sp[RESULT_STACK]);
   vd = new_our_vector(sp[RESULT_STACK]);
   if (ga_sc)
   {
      vmin = new_our_vector(sp[RESULT_STACK]);
      vmax = new_our_vector(sp[RESULT_STACK]);
      vlen = new_our_vector(sp[RESULT_STACK]);
   }

   printf("%d: gsm this node 0\n", this_rank); fflush(stdout);
   for(i = 0; i < v->len; i+= 2, j++) {
      while(!((char *)m->data)[SOLUTE_DATA_ACTIVE_OFS]) {
	 printf("%d: skipping inactive i = %d, j = %d\n", this_rank, i, j); fflush(stdout);
	 j++;
	 m = m->children[0];
	 if(!m) {
	    fprintf(stderr,"out of indices in gsm this node!\n");
	    exit(-1);
	 }
      }
      printf("%d: adding active i = %d < %d, j = %d\n", this_rank, i, v->len, j); fflush(stdout);
      s_estimate_indices[i / 2] = j;
   }
  
   printf("%d: >>>>>> gsm_this_node initial tree:", this_rank);
   list_tree(n);
   puts(""); fflush(stdout);

   for(i = 0; i < v->len; i += 2) {
      v->d[i] = stack[RESULT_STACK][v->len - i - 2];
      if(!ga_mw && !ga_sc) {
	 v->d[i] /= s_rounding;
      }
      v->d[i+1] = stack[RESULT_STACK][v->len - 1 - i];
      v->d[i+1] *= 1e0;
      if(ga_sc) {
	 // ga_sc we need to rescale 
	 double scale;
	 scale = fabs(v->d[i] - 2*v->d[i]);
	 vmin->d[i] = v->d[i] - scale;
	 if (vmin->d[i] < s_estimates[i / 2][0]) 
	 {
	    vmin->d[i] = s_estimates[i / 2][0];
	 }
	 vmax->d[i] = v->d[i] + scale;
	 if (vmax->d[i] > s_estimates[i / 2][1]) 
	 {
	    vmax->d[i] = s_estimates[i / 2][1];
	 }
	 vlen->d[i] = vmax->d[i] - vmin->d[i];
	 printf("scale pos %d scale %g vmin %g vmax %g vlen %g\n",
		i, scale, vmin->d[i], vmax->d[i], vlen->d[i]); fflush(stdout);
	 v->d[i] = (v->d[i] - vmin->d[i]) / vlen->d[i]; // so we have from zero to 1
	 v->d[i+1] = 0;
      }
   }
   /*  {
       our_vector *t;
       int i;
       t = new_our_vector(sp[RESULT_STACK]);
       for(i = 0; i < t->len; i++) {
       t->d[i] = 100 * drand48();
       } 
       printf("%d: trial our_vector:\t", this_rank);
       print_our_vector(t);
       clip_our_vector_scaled(t);
       printf("%d: trial after clipping:\t", this_rank);
       print_our_vector(t);
       free_our_vector(t);
       }
   */
   printf("%d: v:\t", this_rank);
   print_our_vector(v);
   clip_our_vector_scaled(v);
   printf("%d: v after clipping:\t", this_rank);
   print_our_vector(v);

   printf("%d: stack before lamm_gsm_f\n", this_rank);
   print_stack(RESULT_STACK);
   printf("%d: running lamm_gsm_df iter %d\n", this_rank, iter); fflush(stdout);
   //  lamm_gsm_df(vd, v);
   switch(gsm_method) {
   case CONJUGATE_GRADIENT :  min_fr_pr_cgd(v, lamm_gsm_f, lamm_gsm_df, EPS_DEFAULT, iter); break;
   case STEEPEST_DESCENT :      min_gsm_5_1(v, lamm_gsm_f, lamm_gsm_df, EPS_DEFAULT, iter); break;
   case INVERSE_HESSIAN :  min_hessian_bfgs(v, lamm_gsm_f, lamm_gsm_df, EPS_DEFAULT, iter); break;
   default : fprintf(stderr, "gsm_this_node unknown gsm method\n"); exit(-1); break;
   }
   clip_our_vector_scaled(v);
   for(i = 0; i < v->len; i++) {
      if(i % 2) {
	 v->d[i] *= 1e0;
	 if(ga_sc) {
	    v->d[i] = ff0_estimates[(i - 1) / 2][0];
	 }
      } else {
	 if (ga_sc) 
	 {
	    v->d[i] = v->d[i] * vlen->d[i] + vmin->d[i];
	 } else {
	    if (!ga_mw) 
	    {
	       v->d[i] *= s_rounding;
	    }
	 }
      }
   }
   printf("%d: after scaling back: \n", this_rank);
   print_our_vector(v);
   node_set_to_our_vector(n, v);
   printf("%d: >>>>>> gsm_this_node final tree:", this_rank);
   list_tree(n);
   puts(""); fflush(stdout);
   free_our_vector(vd);
   free_our_vector(v);
   if (ga_sc) 
   {
      free_our_vector(vmin);
      free_our_vector(vmax);
      free_our_vector(vlen);
   }
   printf("fitness calls %lu\n", gsm_fitness_calls); fflush(stdout);
   //  if (ga_sc) 
   //  {
   //      printf("aborting\n"); fflush(stdout);
   //      MPI_Abort(MPI_COMM_WORLD, -9990);
   //      exit(-1);
   //  }
}
