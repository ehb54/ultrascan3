#include "../include/us_hydrodyn_mals_saxs_fit.h"

#if defined( USE_MPI )
    extern int myrank;
#endif

// #define UHSHFG_DEBUG_F
// #define UHSHFG_DEBUG_EMGGMG
// #define USUNG_DEBUG

vector < double > US_Hydrodyn_Mals_Saxs_Fit::gsm_t;
vector < double > US_Hydrodyn_Mals_Saxs_Fit::gsm_y;
vector < double > US_Hydrodyn_Mals_Saxs_Fit::gsm_yp;

/* gsm stuff */
our_matrix *US_Hydrodyn_Mals_Saxs_Fit::new_our_matrix(int rows, int cols) {
   our_matrix *m;
   if((m = (our_matrix *)malloc(sizeof(our_matrix))) == NULL) {
      fputs("new_our_matrix malloc error\n", stderr);
      exit(-1);
   }
   m->rows = rows;
   m->cols = cols;
   if((m->d = (double *)malloc(sizeof(double) * rows * cols)) == NULL) {
      fputs("new_our_matrix malloc error\n", stderr);
      exit(-1);
   }
   return(m);
}

void US_Hydrodyn_Mals_Saxs_Fit::free_our_matrix(our_matrix *m) {
   free(m->d);
   free(m);
}

void US_Hydrodyn_Mals_Saxs_Fit::set_our_matrix(our_matrix *m, double s) {
   int i, limit;
   limit = m->rows * m->cols;
   /* could do this with ceil(log2(len)) memcpy()s */
   for(i = 0; i < limit; i++) {
      m->d[i] = s;
   }
}

void US_Hydrodyn_Mals_Saxs_Fit::identity_our_matrix(our_matrix *m) {
   int i;
   set_our_matrix(m, 0e0);

   for(i = 0; i < m->rows; i++) {
      m->d[i * (m->cols + 1)] = 1e0;
   }
}

void US_Hydrodyn_Mals_Saxs_Fit::print_our_matrix(our_matrix *m) {
   int i, j;

   for(i = 0; i < m->rows; i++) {
      // printf("%d: %d:", this_rank, i);
      for(j = 0; j <= m->cols; j++) {
         printf(" %.6g", m->d[(i * m->cols) + j]);
      }
      puts("");
   }
}

our_vector *US_Hydrodyn_Mals_Saxs_Fit::new_our_vector(int len) {
   our_vector *v;
   if((v = (our_vector *)malloc(sizeof(our_vector))) == NULL) {
      fputs("new_our_vector malloc error\n", stderr);
      exit(-1);
   }
   v->len = len;
   if((v->d = (double *)malloc(sizeof(double) * len)) == NULL) {
      fputs("new_our_vector malloc error\n", stderr);
      exit(-1);
   }
   return(v);
}

void US_Hydrodyn_Mals_Saxs_Fit::free_our_vector(our_vector *v) {
   free(v->d);
   free(v);
}

void US_Hydrodyn_Mals_Saxs_Fit::print_our_vector(our_vector *v) {
   int i;

   for(i = 0; i < v->len; i++) {
      printf("%.8g ", v->d[i]);
   }
   puts("");
}

double US_Hydrodyn_Mals_Saxs_Fit::l2norm_our_vector(our_vector *v1, our_vector *v2) {
   double norm = 0e0;
   int i;
#if !defined(SUPRESS_ERROR_CHECKING)
   if(v1->len != v2->len) {
      fputs("l2norm_our_vector incompatable", stderr);
      exit(-1);
   }
#endif
   for(i = 0; i < v1->len; i++) {
      norm += pow(v1->d[i] - v2->d[i], 2e0);
   }
   norm = sqrt(norm);
   return(norm);
}

void US_Hydrodyn_Mals_Saxs_Fit::copy_our_vector(our_vector *v1, our_vector *v2) {
#if !defined(SUPRESS_ERROR_CHECKING)
   if(v1->len != v2->len) {
      fputs("copy_our_vector incompatable", stderr);
      exit(-1);
   }
#endif
   memcpy(v1->d, v2->d, sizeof(double) * v1->len);
}

void US_Hydrodyn_Mals_Saxs_Fit::set_our_vector(our_vector *v1, double s) {
   int i;
   /* could do this with ceil(log2(len)) memcpy()s */
   for(i = 0; i < v1->len; i++) {
      v1->d[i] = s;
   }
}

void US_Hydrodyn_Mals_Saxs_Fit::add_our_vector_vv(our_vector *v1, our_vector *v2) {
   int i;
#if !defined(SUPRESS_ERROR_CHECKING)
   if(v1->len != v2->len) {
      fputs("add_our_vector_vv incompatable", stderr);
      exit(-1);
   }
#endif
   for(i = 0; i < v1->len; i++) {
      v1->d[i] += v2->d[i];
   }
}

void US_Hydrodyn_Mals_Saxs_Fit::add_our_vector_vs(our_vector *v1, double s) {
   int i;
   for(i = 0; i < v1->len; i++) {
      v1->d[i] += s;
   }
}

void US_Hydrodyn_Mals_Saxs_Fit::mult_our_vector_vv(our_vector *v1, our_vector *v2) {
   int i;
#if !defined(SUPRESS_ERROR_CHECKING)
   if(v1->len != v2->len) {
      fputs("mult_our_vector_vv incompatable", stderr);
      exit(-1);
   }
#endif
   for(i = 0; i < v1->len; i++) {
      v1->d[i] *= v2->d[i];
   }
}

void US_Hydrodyn_Mals_Saxs_Fit::mult_our_vector_vs(our_vector *v1, double s) {
   int i;
   for(i = 0; i < v1->len; i++) {
      v1->d[i] *= s;
   }
}

void US_Hydrodyn_Mals_Saxs_Fit::add_our_vector_vvv(our_vector *vd, our_vector *v1, our_vector *v2) {
   int i;
#if !defined(SUPRESS_ERROR_CHECKING)
   if(v1->len != v2->len || vd->len != v1->len) {
      fputs("add_our_vector_vvv incompatable", stderr);
      exit(-1);
   }
#endif
   for(i = 0; i < v1->len; i++) {
      vd->d[i] = v1->d[i] + v2->d[i];
   }
}

void US_Hydrodyn_Mals_Saxs_Fit::add_our_vector_vvs(our_vector *vd, our_vector *v1, double s) {
   int i;
   for(i = 0; i < v1->len; i++) {
      vd->d[i] = v1->d[i] + s;
   }
}

void US_Hydrodyn_Mals_Saxs_Fit::sub_our_vector_vvv(our_vector *vd, our_vector *v1, our_vector *v2) {
   int i;
#if !defined(SUPRESS_ERROR_CHECKING)
   if(v1->len != v2->len || vd->len != v1->len) {
      fputs("add_our_vector_vvv incompatable", stderr);
      exit(-1);
   }
#endif
   for(i = 0; i < v1->len; i++) {
      vd->d[i] = v1->d[i] - v2->d[i];
   }
}

void US_Hydrodyn_Mals_Saxs_Fit::sub_our_vector_vvs(our_vector *vd, our_vector *v1, double s) {
   int i;
   for(i = 0; i < v1->len; i++) {
      vd->d[i] = v1->d[i] - s;
   }
}

void US_Hydrodyn_Mals_Saxs_Fit::mult_our_vector_vvv(our_vector *vd, our_vector *v1, our_vector *v2) {
   int i;
#if !defined(SUPRESS_ERROR_CHECKING)
   if(v1->len != v2->len || vd->len != v1->len) {
      fputs("mult_our_vector_vvv incompatable", stderr);
      exit(-1);
   }
#endif
   for(i = 0; i < v1->len; i++) {
      vd->d[i] = v1->d[i] * v2->d[i];
   }
}

void US_Hydrodyn_Mals_Saxs_Fit::mult_our_vector_vvs(our_vector *vd, our_vector *v1, double s) {
   int i;
   for(i = 0; i < v1->len; i++) {
      vd->d[i] = v1->d[i] * s;
   }
}

double US_Hydrodyn_Mals_Saxs_Fit::dot_our_vector(our_vector *v1, our_vector *v2) {
   int i;
   double ret = 0e0;
   for(i = 0; i < v1->len; i++) {
      ret += v1->d[i] * v2->d[i];
   }
   return(ret);
}


void US_Hydrodyn_Mals_Saxs_Fit::mult_our_matrix_vmv(our_vector *vd, our_matrix *m, our_vector *vs) {
   int i, j;

#if !defined(SUPRESS_ERROR_CHECKING)
   if(vs->len != m->cols || vd->len != m->rows) {
      fputs("mult_our_matrix_vmv incompatable", stderr);
      exit(-1);
   }
#endif

   set_our_vector(vd, 0e0);
   for(i = 0; i < m->rows; i++) {
      for(j = 0; j < m->cols; j++) {
         vd->d[i] += m->d[(i * m->cols) + j] * vs->d[j];
      }
   }
}

void US_Hydrodyn_Mals_Saxs_Fit::our_vector_test() {
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
   // printf("%d: %.12g %.12g %.12g\n", this_rank, l2norm_our_vector(v1, v2), l2norm_our_vector(v1, v3), l2norm_our_vector(v2,v3));
}

long US_Hydrodyn_Mals_Saxs_Fit::min_gsm_5_1( our_vector *i, double epsilon, long max_iter ) 
{
#if defined( USUNG_DEBUG )
# if defined( USE_MPI )
   debug_mpi ( QString( "%1: starting min_gsm_5_1: our_vector\n" ).arg( myrank ) );
# else
   cout << QString( "starting min_gsm_5_1\n" ); //  << fflush;
# endif
#endif

   /* try to find a local minimum via a gradient search method */
#if defined(SHOW_TIMING)
   struct timeval tv1, tv2;
#endif
   our_vector *u, *zero;
   //  double t;
   double s1, s2, s3;
   double g_s1, g_s2, g_s3, g_s4;
   our_vector *v_s1, *v_s2, *v_s3, *v_tmp, *v_s4;
   double a, b, x;
   double s1_s2, s1_s3, s2_s3;
   double s1_2, s2_2, s3_2;
   // double prev_s2;
   double prev_g_s2;
   double fitness;
   long iter = 0l;
   int reps;
#if defined(PRINT_GSM_INFO)
   int last_reps = 0;
#endif
   //  int j;

   //  printf("conjugate gradient initial position: ");
   //  print_our_vector(i); 

   zero = new_our_vector(i->len);
   set_our_vector(zero, 0e0);

   u = new_our_vector(i->len);
   gsm_df(u, i);
   /*  mult_our_vector_vs(u, 1e0 / l2norm_our_vector(u, zero)); */

#if defined( USUNG_DEBUG )
   printf("initial gradient ||=%.12g : ", l2norm_our_vector(u, zero)); 
   print_our_vector(u); 
#endif
  
   v_s1 = new_our_vector(i->len);
   v_s2 = new_our_vector(i->len);
   v_s3 = new_our_vector(i->len);
   v_s4 = new_our_vector(i->len);

   //  printf("norm %g\n", l2norm_our_vector(u, zero));

   while(l2norm_our_vector(u, zero) >= epsilon && iter++ < max_iter) {
#if defined( USUNG_DEBUG )
      printf("begin\t{%.12g,%.12g,%.12g} = {%.12g,%.12g,%.12g}\n", s1, s2, s3, g_s1, g_s2, g_s3);
#endif
#if defined(PRINT_GSM_INFO)
      printf("%d: global %ld, iter %ld fitness %.12g |grad|=%.12g last reps %d\n", this_rank, global_iter++, 
             iter, fitness = gsm_f(i), l2norm_our_vector(u, zero), last_reps); fflush(stdout);
      printf("%d: ", this_rank);
      print_our_vector(i);
      fflush(stdout);
#endif
#if !defined(PRINT_GSM_INFO)
      global_iter++;
      fitness = gsm_f( i );
#endif
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
      /* find minimum of gsm_f(i - t u) */
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
      g_s2 = gsm_f(v_s2);

      /* cut down interval until we have a decrease */
#if defined(PRINT_GSM_INFO)
      printf("%d: decrease\n", this_rank); 
      fflush(stdout);
#endif
#if defined(SHOW_TIMING)
      gettimeofday(&tv1, NULL);
#endif

      while(g_s2 > g_s1 && s2 - s1 > epsilon) {
         s3 = s2;
         s2 *= 5e-1;
         mult_our_vector_vvs(v_s2, u, -s2);
         add_our_vector_vv(v_s2, i);
         g_s2 = gsm_f(v_s2);
      }

      if(s2 - s1 <= epsilon || s3 - s2 <= epsilon) {
         /* ugh, no decrease */
#if defined(PRINT_GSM_INFO)
         printf("%d: no initial decrease, terminating pos = ", this_rank);
#endif
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
      g_s3 = gsm_f(v_s3);


#if defined( USUNG_DEBUG )
      printf("pre\t{%.12g,%.12g,%.12g} = {%.12g,%.12g,%.12g}\n", s1, s2, s3, g_s1, g_s2, g_s3); 
#endif

    
      reps = 0;
#if defined(SHOW_TIMING)
      gettimeofday(&tv2, NULL);
      if(show_times)
         printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
#endif
#if defined(PRINT_GSM_INFO)
      printf("%d: start line search\n", this_rank); 
      fflush(stdout);
#endif
#if defined(SHOW_TIMING)
      gettimeofday(&tv1, NULL);
#endif
      while(s2 - s1 > epsilon && s3 - s2 > epsilon && reps++ < MAX_REPS) {
      
#if defined( USUNG_DEBUG )
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
#if defined(PRINT_GSM_INFO)
            printf("%d: a limit reached\n", this_rank);
            printf("%d: done iter %ld, i = ", this_rank, iter); 
            print_our_vector(i); fflush(stdout);
#endif
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
         // prev_s2 = s2;

#if defined( USUNG_DEBUG )
         printf("new x = %.12g\n", x);
#endif

         if(x < s1) {
#if defined( USUNG_DEBUG )
            printf("p1 ");
#endif
            if(x < (s1 + s1 - s2)) { /* keep it close */
               x = s1 + s1 - s2;
               if(x < 0) {
                  x = s1 * 5e-1;
               }
            }
            if(x < 0) { /* ugh we're in the wrong direction! */
#if defined(PRINT_GSM_INFO)
               printf("%d: unexpected minimum pos %.12g\n", this_rank, x);
#endif
               // exit(-1);
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
            g_s1 = gsm_f(v_s1);
         } else {
            if(x < s2) {
#if defined( USUNG_DEBUG )
               printf("p2 ");
#endif
               /* ok, take s1, x, s2 */
               v_tmp = v_s3;
               v_s3 = v_s2;
               g_s3 = g_s2;
               s3 = s2;
               v_s2 = v_tmp;
   
               s2 = x;
               /*     printf("x = %.12g\n", x); */
               mult_our_vector_vvs(v_s2, u, -s2);
               add_our_vector_vv(v_s2, i);
               g_s2 = gsm_f(v_s2);
            } else {
               if(x < s3) {
#if defined( USUNG_DEBUG )
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
                  g_s2 = gsm_f(v_s2);
               } else {
                  /* ugh x >= s3.. well why not? */
                  if(x > (s3 + s3 - s2)) { /* keep it close */
                     mult_our_vector_vvs(v_s4, u, -x);
                     add_our_vector_vv(v_s4, i);
                     g_s4 = gsm_f(v_s4);
                     if(g_s4 > g_s2 &&
                        g_s4 > g_s3 &&
                        g_s4 > g_s1) {
                        x = (s3 + s3 - s2);
                     }
                  }
                  /* take s2, s3, x */
#if defined( USUNG_DEBUG )
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
                  g_s3 = gsm_f(v_s3);
               }
            }
         }
      
         if(fabs(prev_g_s2 - g_s2) < epsilon) {
#if defined(PRINT_GSM_INFO)
            printf("%d: fabs(g-prev) < epsilon\n", this_rank); fflush(stdout);
#endif
            break;
         }
         /*      puts(""); */
      }
#if defined(PRINT_GSM_INFO)
      last_reps = reps;
#endif
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

      //       if(evenn) {
      //          switch(queries) {
      //          case 8 : i->d[(6 * N_2) - 1] = 0;
      //          case 7 :
      //          case 6 : i->d[(4 * N_2) - 1] = 0;
      //          case 5 :
      //          case 4 : i->d[(2 * N_2) - 1] = 0; 
      //          default : break;
      //          }
      //       }
#if defined(SHOW_TIMING)
      gettimeofday(&tv2, NULL);
      if(show_times)
         printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
#endif
#if defined(PRINT_GSM_INFO)
      printf("%d: df start\n", this_rank); 
      fflush(stdout);
#endif
      gsm_df(u, i);
      /*    mult_our_vector_vs(u, 1e0 / l2norm_our_vector(u, zero)); */
#if defined( USUNG_DEBUG )
      printf("end\t{%.12g,%.12g,%.12g} = {%.12g,%.12g,%.12g}\n", s1, s2, s3, g_s1, g_s2, g_s3);
#endif

#if defined(PSV_OUTPUT)
      if(!(global_iter % 5)) {
         //      int r, s, j;
      
         printf("%d: query4i_intermediate|alg4|%s|%d|%.12g|%d|%d|%d|%d|%d|%.12g|%.12g|%.12g|%.12g|%ld|%d|%d|%d|%.12g|%ld|%ld", this_rank,
                (gsm_f(i) <= 0e0) ? "success" : "fail",
                N, 
                gsm_f(i),
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
#endif
   }

#if defined(PRINT_GSM_INFO)
   printf("%d: done iter %ld, i = ", this_rank, iter);
   print_our_vector(i);
#endif
   free_our_vector(v_s1);
   free_our_vector(v_s2);
   free_our_vector(v_s3);
   free_our_vector(v_s4);
   free_our_vector(zero);
   free_our_vector(u);
   return(0);
}

long US_Hydrodyn_Mals_Saxs_Fit::min_fr_pr_cgd(our_vector *i, double epsilon, long max_iter) 
{
   /* polak-ribiere version of fletcher-reeves conjugate gradient  */

#if defined(SHOW_TIMING)
   struct timeval tv1, tv2;
#endif
   our_vector *u, *zero;
   //  double t;
   double s1, s2, s3;
   double g_s1, g_s2, g_s3, g_s4;
   our_vector *v_s1, *v_s2, *v_s3, *v_tmp, *v_s4, *v_h, *v_g;
   double a, b, x;
   double s1_s2, s1_s3, s2_s3;
   double s1_2, s2_2, s3_2;
   // double prev_s2;
   double prev_g_s2;
   long iter = 0l;
   int reps;
#if defined(PRINT_GSM_INFO)
   int last_reps = 0;
#endif
   double gg, ggd;
   double fitness;

   /*  printf("initial position: ");
       print_our_vector(i); */

   //   puts("fr.1");
   zero = new_our_vector(i->len);
   set_our_vector(zero, 0e0);

   u = new_our_vector(i->len);
   gsm_df(u, i);
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
#if defined(PRINT_GSM_INFO)
      printf("%d: global %ld, iter %ld fitness %.12g |grad|=%.12g last reps %d\n", this_rank, global_iter++, 
             iter, fitness = gsm_f(i), l2norm_our_vector(u, zero), last_reps); fflush(stdout);
      printf("%d: ", this_rank);
      print_our_vector(i);
#endif
#if defined( USUNG_DEBUG )
      print_our_vector(u);
#endif
#if !defined(PRINT_GSM_INFO)
      global_iter++;
      fitness = gsm_f( i );
#endif
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
      /* find minimum of gsm_f(i - t u) */
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
      // print_our_vector(v_s2);
      g_s2 = gsm_f(v_s2);

      /* cut down interval until we have a decrease */
#if defined(PRINT_GSM_INFO)
      printf("%d: decrease\n", this_rank); 
      fflush(stdout);
#endif
#if defined(SHOW_TIMING)
      gettimeofday(&tv1, NULL);
#endif
#if defined( USUNG_DEBUG )
      printf("s values\t{%.12g,%.12g,%.12g} = {%.12g,%.12g,%.12g}\n", s1, s2, s3, g_s1, g_s2, g_s3);
#endif

      while(g_s2 > g_s1 && s2 - s1 > epsilon) {
         s3 = s2;
         s2 *= 5e-1;
         mult_our_vector_vvs(v_s2, u, -s2);
         add_our_vector_vv(v_s2, i);
         g_s2 = gsm_f(v_s2);
      }

      if(s2 - s1 <= epsilon || s3 - s2 <= epsilon) {
         /* ugh, no decrease */
#if defined(PRINT_GSM_INFO)
         printf("%d: no initial decrease, terminating pos = ", this_rank);
#endif
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
      g_s3 = gsm_f(v_s3);


#if defined( USUNG_DEBUG )
      printf("pre\t{%.12g,%.12g,%.12g} = {%.12g,%.12g,%.12g}\n", s1, s2, s3, g_s1, g_s2, g_s3); 
#endif

      reps = 0;

#if defined(SHOW_TIMING)
      gettimeofday(&tv2, NULL);
      if(show_times)
         printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
#endif
#if defined(PRINT_GSM_INFO)
      printf("%d: start line search\n", this_rank); 
      fflush(stdout);
#endif
#if defined(SHOW_TIMING)
      gettimeofday(&tv1, NULL);
#endif
    
      while(s2 - s1 > epsilon && s3 - s2 > epsilon && reps++ < MAX_REPS) {
      
#if defined( USUNG_DEBUG )
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
#if defined(PRINT_GSM_INFO)
            printf("%d: a limit reached", this_rank);
            printf("done iter %ld, i = ", iter);
            print_our_vector(i);
#endif
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
         // prev_s2 = s2;

#if defined( USUNG_DEBUG )
         printf("new x = %.12g\n", x);
#endif

         if(x < s1) {
#if defined( USUNG_DEBUG )
            printf("p1 ");
#endif
            if(x < (s1 + s1 - s2)) { /* keep it close */
               x = s1 + s1 - s2;
               if(x < 0) {
                  x = s1 * 5e-1;
               }
            }
            if(x < 0) { /* ugh we're in the wrong direction! */
#if defined(PRINT_GSM_INFO)
               printf("%d: unexpected minimum pos %.12g\n", this_rank, x);
#endif
               //     exit(-1);
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
            g_s1 = gsm_f(v_s1);
         } else {
            if(x < s2) {
#if defined( USUNG_DEBUG )
               printf("p2 ");
#endif
               /* ok, take s1, x, s2 */
               v_tmp = v_s3;
               v_s3 = v_s2;
               g_s3 = g_s2;
               s3 = s2;
               v_s2 = v_tmp;
   
               s2 = x;
               /*     printf("x = %.12g\n", x); */
               mult_our_vector_vvs(v_s2, u, -s2);
               add_our_vector_vv(v_s2, i);
               g_s2 = gsm_f(v_s2);
            } else {
               if(x < s3) {
#if defined( USUNG_DEBUG )
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
                  g_s2 = gsm_f(v_s2);
               } else {
                  /* ugh x >= s3.. well why not? */
                  if(x > (s3 + s3 - s2)) { /* keep it close */
                     mult_our_vector_vvs(v_s4, u, -x);
                     add_our_vector_vv(v_s4, i);
                     g_s4 = gsm_f(v_s4);
                     if(g_s4 > g_s2 &&
                        g_s4 > g_s3 &&
                        g_s4 > g_s1) {
                        x = (s3 + s3 - s2);
                     }
                  }
                  /* take s2, s3, x */
#if defined( USUNG_DEBUG )
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
                  g_s3 = gsm_f(v_s3);
               }
            }
         }
      
         if(fabs(prev_g_s2 - g_s2) < epsilon) {
#if defined(PRINT_GSM_INFO)
            printf("%d: fabs(g-prev) < epsilon\n", this_rank); fflush(stdout);
#endif
            break;
         }
         /*      puts(""); */
      }
#if defined(PRINT_GSM_INFO)
      last_reps = reps;
#endif
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


      //       if(evenn) {
      //          switch(queries) {
      //          case 8 : i->d[(6 * N_2) - 1] = 0;
      //          case 7 :
      //          case 6 : i->d[(4 * N_2) - 1] = 0;
      //          case 5 :
      //          case 4 : i->d[(2 * N_2) - 1] = 0; 
      //          default : break;
      //          }
      //       }

#if defined(SHOW_TIMING)
      gettimeofday(&tv2, NULL);
      if(show_times)
         printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
#endif
#if defined(PRINT_GSM_INFO)
      printf("%d: df start\n", this_rank); 
      fflush(stdout);
      puts("conj dir start");
      fflush(stdout);
#endif
      gsm_df(u, i);
#if defined(SHOW_TIMING)
      gettimeofday(&tv1, NULL);
#endif
    
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
#if defined(SHOW_TIMING)
      gettimeofday(&tv2, NULL);
      if(show_times)
         printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
      fflush(stdout);
#endif

#if defined(PSV_OUTPUT)
      if(!(global_iter % 5)) {
         //      int r, s, j;
      
         printf("%d: query4i_intermediate|alg3|%s|%d|%.12g|%d|%d|%d|%d|%d|%.12g|%.12g|%.12g|%.12g|%ld|%d|%d|%d|%.12g|%ld|%ld", this_rank,
                (gsm_f(i) <= 0e0) ? "success" : "fail",
                N, 
                gsm_f(i),
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
#endif
   }

#if defined(PRINT_GSM_INFO)
   printf("%d: done iter %ld, i = ", this_rank, iter);
   print_our_vector(i);
#endif
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

long US_Hydrodyn_Mals_Saxs_Fit::min_hessian_bfgs(our_vector *ip, double epsilon, long max_iter) 
{
  
#if defined(SHOW_TIMING)
   struct timeval tv1, tv2;
#endif
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
   // double prev_s2;
   double prev_g_s2;
   long iter = 0l;
   int reps;
#if defined(PRINT_GSM_INFO)
   int last_reps = 0;
#endif
   int i,j;
   double fitness;
   double fac, fad, fae, sumdg, sumxi;

   /*  printf("initial position: ");
       print_our_vector(i); */

   zero = new_our_vector(ip->len);
   set_our_vector(zero, 0e0);

   u = new_our_vector(ip->len);
   gsm_df(u, ip);

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
#if defined( USUNG_DEBUG )
      printf("begin\t{%.12g,%.12g,%.12g} = {%.12g,%.12g,%.12g}\n", s1, s2, s3, g_s1, g_s2, g_s3);
#endif
#if defined(PRINT_GSM_INFO)
      printf("%d: global %ld, iter %ld fitness %.12g |grad|=%.12g last reps %d\n", this_rank, global_iter++, 
             iter, fitness = gsm_f(ip), l2norm_our_vector(u, zero), last_reps); fflush(stdout);
      printf("%d: ", this_rank);
      print_our_vector(ip);
      /*    print_our_vector(u); */
      fflush(stdout);
#endif
      /*    printf("ip : ");
            print_our_vector(ip); */
      /* find minimum of gsm_f(ip - t u) */
      /* alg 5_2 */
#if !defined(PRINT_GSM_INFO)
      global_iter++;
      fitness = gsm_f( ip );
#endif
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
      g_s2 = gsm_f(v_s2);

      /* cut down interval until we have a decrease */
#if defined(PRINT_GSM_INFO)
      printf("%d: decrease\n", this_rank); 
      fflush(stdout);
#endif
#if defined(SHOW_TIMING)
      gettimeofday(&tv1, NULL);
#endif
      while(g_s2 > g_s1 && s2 - s1 > epsilon) {
         s3 = s2;
         s2 *= 5e-1;
         mult_our_vector_vvs(v_s2, u, -s2);
         add_our_vector_vv(v_s2, ip);
         g_s2 = gsm_f(v_s2);
      }

      if(s2 - s1 <= epsilon || s3 - s2 <= epsilon) {
         /* ugh, no decrease */
#if defined(PRINT_GSM_INFO)
         printf("%d: no initial decrease, terminating pos = ", this_rank);
#endif
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
      g_s3 = gsm_f(v_s3);

#if defined( USUNG_DEBUG )
      printf("pre\t{%.12g,%.12g,%.12g} = {%.12g,%.12g,%.12g}\n", s1, s2, s3, g_s1, g_s2, g_s3); 
#endif

      reps = 0;

#if defined(SHOW_TIMING)
      gettimeofday(&tv2, NULL);
      if(show_times)
         printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
#endif
#if defined(PRINT_GSM_INFO)
      printf("%d: start line search\n", this_rank); 
      fflush(stdout);
#endif
#if defined(SHOW_TIMING)
      gettimeofday(&tv1, NULL);
#endif

      while(s2 - s1 > epsilon && s3 - s2 > epsilon && reps++ < MAX_REPS) {
      
#if defined( USUNG_DEBUG )
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
#if defined(PRINT_GSM_INFO)
            printf("%d: a limit reached", this_rank);
            printf("done iter %ld, i = ", iter);
            print_our_vector(ip);
#endif
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
         // prev_s2 = s2;

#if defined( USUNG_DEBUG )
         printf("new x = %.12g\n", x);
#endif

         if(x < s1) {
#if defined( USUNG_DEBUG )
            printf("p1 ");
#endif
            if(x < (s1 + s1 - s2)) { /* keep it close */
               x = s1 + s1 - s2;
               if(x < 0) {
                  x = s1 * 5e-1;
               }
            }
            if(x < 0) { /* ugh we're in the wrong direction! */
#if defined(PRINT_GSM_INFO)
               printf("%d: unexpected minimum pos %.12g\n", this_rank, x);
#endif
               if(s1 < 0) {
                  s1 = 0;
               }
               x = 0;
               //     exit(-1);
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
            g_s1 = gsm_f(v_s1);
         } else {
            if(x < s2) {
#if defined( USUNG_DEBUG )
               printf("p2 ");
#endif
               /* ok, take s1, x, s2 */
               v_tmp = v_s3;
               v_s3 = v_s2;
               g_s3 = g_s2;
               s3 = s2;
               v_s2 = v_tmp;
   
               s2 = x;
               /*     printf("x = %.12g\n", x); */
               mult_our_vector_vvs(v_s2, u, -s2);
               add_our_vector_vv(v_s2, ip);
               g_s2 = gsm_f(v_s2);
            } else {
               if(x < s3) {
#if defined( USUNG_DEBUG )
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
                  g_s2 = gsm_f(v_s2);
               } else {
                  /* ugh x >= s3.. well why not? */
                  if(x > (s3 + s3 - s2)) { /* keep it close */
                     mult_our_vector_vvs(v_s4, u, -x);
                     add_our_vector_vv(v_s4, ip);
                     g_s4 = gsm_f(v_s4);
                     if(g_s4 > g_s2 &&
                        g_s4 > g_s3 &&
                        g_s4 > g_s1) {
                        x = (s3 + s3 - s2);
                     }
                  }
                  /* take s2, s3, x */
#if defined( USUNG_DEBUG )
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
                  g_s3 = gsm_f(v_s3);
               }
            }
         }
      
         if(fabs(prev_g_s2 - g_s2) < epsilon) {
#if defined(PRINT_GSM_INFO)
            printf("%d: fabs(g-prev) < epsilon\n", this_rank); fflush(stdout);
#endif
            break;
         }
         /*      puts(""); */
      }
#if defined(PRINT_GSM_INFO)
      last_reps = reps;
#endif
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


      //       if(evenn) {
      //          switch(queries) {
      //          case 8 : v_p->d[(6 * N_2) - 1] = 0;
      //          case 7 :
      //          case 6 : v_p->d[(4 * N_2) - 1] = 0;
      //          case 5 :
      //          case 4 : v_p->d[(2 * N_2) - 1] = 0; 
      //          default : break;
      //          }
      //       }

      /*    printf("v_dx:"); print_our_vector(v_dx); */

#if defined(SHOW_TIMING)
      gettimeofday(&tv2, NULL);
      if(show_times)
         printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
#endif
#if defined(PRINT_GSM_INFO)
      printf("%d: df start\n", this_rank); 
      fflush(stdout);
#endif
      gsm_df(v_g, v_p);                 /* new gradient in v_g (old in u) */
#if defined(PRINT_GSM_INFO)
      printf("%d: hessian start\n", this_rank);
#endif
#if defined(SHOW_TIMING)
      gettimeofday(&tv1, NULL);
#endif

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
#if defined(SHOW_TIMING)
      gettimeofday(&tv2, NULL);
      if(show_times)
         printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
      fflush(stdout);
#endif
      /*    mult_our_vector_vs(u, -1e0); */
      /*
        print_our_vector(u);
        print_our_matrix(hessian);
        print_our_vector(v_g);
        printf("|u|=%.12g\n", l2norm_our_vector(u, zero)); 
      */

#if defined( USUNG_DEBUG )
      printf("end\t{%.12g,%.12g,%.12g} = {%.12g,%.12g,%.12g}\n", s1, s2, s3, g_s1, g_s2, g_s3);
#endif

#if defined(PSV_OUTPUT)
      if(!(global_iter % 5)) {
         //      int r, s, j;
      
         printf("%d: query4i_intermediate|alg4|%s|%d|%.12g|%d|%d|%.12g|%d|%d|%.12g|%.12g|%.12g|%.12g|%ld|%d|%d|%d|%.12g|%ld|%ld",
                this_rank,
                (gsm_f(ip) <= 0e0) ? "success" : "fail",
                N, 
                gsm_f(ip),
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
#endif
   }

#if defined(PRINT_GSM_INFO)
   printf("%d done iter %ld, i = ", this_rank, iter);
   print_our_vector(ip);
#endif
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


void US_Hydrodyn_Mals_Saxs_Fit::gsm_setup()
{
   gsm_delta    = le_epsilon->text().toDouble(); 
   gsm_delta2_r = 1e0 / ( 2e0 * gsm_delta );

   vector < double > x = mals_saxs_win->f_qs[ mals_saxs_win->wheel_file ];

   double start = mals_saxs_win->le_gauss_fit_start->text().toDouble();
   double end   = mals_saxs_win->le_gauss_fit_end  ->text().toDouble();

   gsm_t.clear( );
   gsm_y.clear( );
   gsm_yp.clear( );

   for ( unsigned int j = 0; j < x.size(); j++ )
   {
      if ( x[ j ] >= start && x[ j ] <= end )
      {
         gsm_t.push_back( x[ j ] );
         gsm_y.push_back( mals_saxs_win->f_Is[ mals_saxs_win->wheel_file ][ j ] );
      }
   }

   gsm_yp.resize( gsm_t.size() );
}

double US_Hydrodyn_Mals_Saxs_Fit::gsm_f_GAUSS( our_vector *v )
{
   for ( unsigned int j = 0; j < gsm_yp.size(); j++ )
   {
      gsm_yp[ j ] = 0e0;
   }

   double height;
   double center;
   double width;

   for ( unsigned int i = 0; i < ( unsigned int ) MSFIT::param_fixed.size(); )
   {
      if ( MSFIT::param_fixed[ i ] )
      {
         height = MSFIT::fixed_params[ MSFIT::param_pos[ i ] ];
      } else {
         height = v->d[ MSFIT::param_pos[ i ] ];
         if ( height < MSFIT::param_min[ MSFIT::param_pos[ i ] ] ||
              height > MSFIT::param_max[ MSFIT::param_pos[ i ] ] )
         {
            return 1e99;
         }
      }

      i++;

      if ( MSFIT::param_fixed[ i ] )
      {
         center = MSFIT::fixed_params[ MSFIT::param_pos[ i ] ];
      } else {
         center = v->d[ MSFIT::param_pos[ i ] ];
         if ( center < MSFIT::param_min[ MSFIT::param_pos[ i ] ] ||
              center > MSFIT::param_max[ MSFIT::param_pos[ i ] ] )
         {
            return 1e99;
         }
      }

      i++;

      if ( MSFIT::param_fixed[ i ] )
      {
         width = MSFIT::fixed_params[ MSFIT::param_pos[ i ] ];
      } else {
         width = v->d[ MSFIT::param_pos[ i ] ];
         if ( width < MSFIT::param_min[ MSFIT::param_pos[ i ] ] ||
              width > MSFIT::param_max[ MSFIT::param_pos[ i ] ] )
         {
            return 1e99;
         }
      }

      i++;

      for ( unsigned int j = 0; j < gsm_yp.size(); j++ )
      {
         double tmp = ( gsm_t[ j ] - center ) / width;
         gsm_yp[ j ] += height * exp( - tmp * tmp * 5e-1 );
      }
   }

   double rmsd = 0e0;

   for ( unsigned int j = 0; j < gsm_y.size(); j++ )
   {
      rmsd += ( gsm_y[ j ] - gsm_yp[ j ] ) * ( gsm_y[ j ] - gsm_yp[ j ] );
   }
#if defined( UHSHFG_DEBUG_F )
   printf( "GAUSS: rmsd %.6g: ", sqrt( rmsd ) );
   print_our_vector( v );
#endif
   return sqrt( rmsd );
}

double US_Hydrodyn_Mals_Saxs_Fit::gsm_f_EMG( our_vector *v )
{
   for ( unsigned int j = 0; j < gsm_yp.size(); j++ )
   {
      gsm_yp[ j ] = 0e0;
   }

   double height;
   double center;
   double width;
   double dist1;

   for ( unsigned int i = 0; i < ( unsigned int ) MSFIT::param_fixed.size(); )
   {
      if ( MSFIT::param_fixed[ i ] )
      {
         height = MSFIT::fixed_params[ MSFIT::param_pos[ i ] ];
      } else {
         height = v->d[ MSFIT::param_pos[ i ] ];
         if ( height < MSFIT::param_min[ MSFIT::param_pos[ i ] ] ||
              height > MSFIT::param_max[ MSFIT::param_pos[ i ] ] )
         {
            return 1e99;
         }
      }

      i++;

      if ( MSFIT::param_fixed[ i ] )
      {
         center = MSFIT::fixed_params[ MSFIT::param_pos[ i ] ];
      } else {
         center = v->d[ MSFIT::param_pos[ i ] ];
         if ( center < MSFIT::param_min[ MSFIT::param_pos[ i ] ] ||
              center > MSFIT::param_max[ MSFIT::param_pos[ i ] ] )
         {
            return 1e99;
         }
      }

      i++;

      if ( MSFIT::param_fixed[ i ] )
      {
         width = MSFIT::fixed_params[ MSFIT::param_pos[ i ] ];
      } else {
         width = v->d[ MSFIT::param_pos[ i ] ];
         if ( width < MSFIT::param_min[ MSFIT::param_pos[ i ] ] ||
              width > MSFIT::param_max[ MSFIT::param_pos[ i ] ] )
         {
            return 1e99;
         }
      }

      i++;

      if ( MSFIT::param_fixed[ i ] )
      {
         if ( MSFIT::comm_backref.count( i ) )
         {
            dist1 = v->d              [ MSFIT::comm_backref[ i ] ];
         } else {
            dist1 = MSFIT::fixed_params[ MSFIT::param_pos[ i ] ];
         }               
      } else {
         dist1 = v->d         [ MSFIT::param_pos[ i ] ];
         if ( dist1 < MSFIT::param_min[ MSFIT::param_pos[ i ] ] ||
              dist1 > MSFIT::param_max[ MSFIT::param_pos[ i ] ] )
         {
            return 1e99;
         }
      }

      i++;

#if defined( UHSHFG_DEBUG_F )
      cout << QString(  "EMG: param %1: %2 %3 %4 %5\n" )
         .arg( i )
         .arg( height )
         .arg( center )
         .arg( width )
         .arg( dist1 )
         ;
#endif

      if ( dist1 )
      {
         double dist1_thresh      = width / ( 5e0 * sqrt(2e0) - 2e0 );
         if ( fabs( dist1 ) < dist1_thresh )
         {
            puts( "EMG averaged with gaussian" );
            double frac_gauss = ( dist1_thresh - fabs( dist1 ) ) / dist1_thresh;
            if ( dist1 < 0 )
            {
               dist1_thresh *= -1e0;
            }

            double area              = height * width * M_SQRT2PI;
            double one_over_a3       = 1e0 / dist1_thresh;
            double emg_coeff         = area * one_over_a3 * 5e-1 * (1e0 - frac_gauss );
            double emg_exp_1         = width * width * one_over_a3 * one_over_a3 * 5e-1;
            double emg_erf_2         = width * M_ONE_OVER_SQRT2 * one_over_a3;
            double sign_a3           = dist1_thresh < 0e0 ? -1e0 : 1e0;
            double one_over_sqrt2_a2 = M_ONE_OVER_SQRT2 / width;
            double gauss_coeff       = frac_gauss * height;

            for ( unsigned int j = 0; j < gsm_yp.size(); j++ )
            {
               double tmp = gsm_t[ j ] - center;
               double tmp2 =  tmp / width;
               
               gsm_yp[ j ] += 
                  emg_coeff * exp( emg_exp_1 - one_over_a3 * tmp ) *
                  ( use_erf( tmp * one_over_sqrt2_a2 - emg_erf_2 ) + sign_a3 ) +
                  gauss_coeff * exp( - tmp2 * tmp2 * 5e-1 );
            }
         } else {
            puts( "EMG pure" );
            double area              = height * width * M_SQRT2PI;
            double one_over_a3       = 1e0 / dist1;
            double emg_coeff         = area * one_over_a3 * 5e-1;
            double emg_exp_1         = width * width * one_over_a3 * one_over_a3 * 5e-1;
            double emg_erf_2         = width * M_ONE_OVER_SQRT2 * one_over_a3;
            double sign_a3           = dist1 < 0e0 ? -1e0 : 1e0;
            double one_over_sqrt2_a2 = M_ONE_OVER_SQRT2 / width;

            for ( unsigned int j = 0; j < gsm_yp.size(); j++ )
            {
               double tmp = gsm_t[ j ] - center;
               gsm_yp[ j ] += 
                  emg_coeff * exp( emg_exp_1 - one_over_a3 * tmp ) *
                  ( use_erf( tmp * one_over_sqrt2_a2 - emg_erf_2 ) + sign_a3 );
            }
         }
      } else {
         puts( "EMG gaussian (dist1 0)" );
         for ( unsigned int j = 0; j < gsm_yp.size(); j++ )
         {
            double tmp = ( gsm_t[ j ] - center ) / width;
            gsm_yp[ j ] += height * exp( - tmp * tmp * 5e-1 );
         }
      }
   }

   double rmsd = 0e0;

   for ( unsigned int j = 0; j < gsm_y.size(); j++ )
   {
      rmsd += ( gsm_y[ j ] - gsm_yp[ j ] ) * ( gsm_y[ j ] - gsm_yp[ j ] );
   }
#if defined( UHSHFG_DEBUG_F )
   printf( "EMG: rmsd %.6g: ", sqrt( rmsd ) );
   print_our_vector( v );
#endif
   return sqrt( rmsd );
}

double US_Hydrodyn_Mals_Saxs_Fit::gsm_f_GMG( our_vector *v )
{
   for ( unsigned int j = 0; j < gsm_yp.size(); j++ )
   {
      gsm_yp[ j ] = 0e0;
   }

   double height;
   double center;
   double width;
   double dist1;

   for ( unsigned int i = 0; i < ( unsigned int ) MSFIT::param_fixed.size(); )
   {
      if ( MSFIT::param_fixed[ i ] )
      {
         height = MSFIT::fixed_params[ MSFIT::param_pos[ i ] ];
      } else {
         height = v->d[ MSFIT::param_pos[ i ] ];
         if ( height < MSFIT::param_min[ MSFIT::param_pos[ i ] ] ||
              height > MSFIT::param_max[ MSFIT::param_pos[ i ] ] )
         {
            return 1e99;
         }
      }

      i++;

      if ( MSFIT::param_fixed[ i ] )
      {
         center = MSFIT::fixed_params[ MSFIT::param_pos[ i ] ];
      } else {
         center = v->d[ MSFIT::param_pos[ i ] ];
         if ( center < MSFIT::param_min[ MSFIT::param_pos[ i ] ] ||
              center > MSFIT::param_max[ MSFIT::param_pos[ i ] ] )
         {
            return 1e99;
         }
      }

      i++;

      if ( MSFIT::param_fixed[ i ] )
      {
         width = MSFIT::fixed_params[ MSFIT::param_pos[ i ] ];
      } else {
         width = v->d[ MSFIT::param_pos[ i ] ];
         if ( width < MSFIT::param_min[ MSFIT::param_pos[ i ] ] ||
              width > MSFIT::param_max[ MSFIT::param_pos[ i ] ] )
         {
            return 1e99;
         }
      }

      i++;

      if ( MSFIT::param_fixed[ i ] )
      {
         if ( MSFIT::comm_backref.count( i ) )
         {
            dist1 = v->d              [ MSFIT::comm_backref[ i ] ];
         } else {
            dist1 = MSFIT::fixed_params[ MSFIT::param_pos[ i ] ];
         }               
      } else {
         dist1 = v->d         [ MSFIT::param_pos[ i ] ];
         if ( dist1 < MSFIT::param_min[ MSFIT::param_pos[ i ] ] ||
              dist1 > MSFIT::param_max[ MSFIT::param_pos[ i ] ] )
         {
            return 1e99;
         }
      }

      i++;

#if defined( UHSHFG_DEBUG_F )
      cout << QString(  "GMG: param %1: %2 %3 %4 %5\n" )
         .arg( i )
         .arg( height )
         .arg( center )
         .arg( width )
         .arg( dist1 )
         ;
#endif

      if ( dist1 )
      {
         // puts( "GMG pure" );
         double area                         = height * width * M_SQRT2PI;
         double one_over_width               = 1e0 / width;
         double one_over_a2sq_plus_a3sq      = 1e0 / ( width * width + dist1 * dist1 );
         double sqrt_one_over_a2sq_plus_a3sq = sqrt( one_over_a2sq_plus_a3sq );
         double gmg_coeff                    = area * M_ONE_OVER_SQRT2PI * sqrt_one_over_a2sq_plus_a3sq;
         double gmg_exp_m1                   = -5e-1 *  one_over_a2sq_plus_a3sq;
         double gmg_erf_m1                   = dist1 * sqrt_one_over_a2sq_plus_a3sq * M_ONE_OVER_SQRT2 * one_over_width;
            
         for ( unsigned int j = 0; j < gsm_yp.size(); j++ )
         {
            double tmp = gsm_t[ j ] - center;
            gsm_yp[ j ] += 
               gmg_coeff * exp( gmg_exp_m1 * tmp * tmp ) *
               ( 1e0 + use_erf( gmg_erf_m1 * tmp ) );
         }
      } else {
         // puts( "GMG gaussian" );
         for ( unsigned int j = 0; j < gsm_yp.size(); j++ )
         {
            double tmp = ( gsm_t[ j ] - center ) / width;
            gsm_yp[ j ] += height * exp( - tmp * tmp * 5e-1 );
         }
      }
   }

   double rmsd = 0e0;

   for ( unsigned int j = 0; j < gsm_y.size(); j++ )
   {
      rmsd += ( gsm_y[ j ] - gsm_yp[ j ] ) * ( gsm_y[ j ] - gsm_yp[ j ] );
   }
#if defined( UHSHFG_DEBUG_F )
   printf( "GMG: rmsd %.6g: ", sqrt( rmsd ) );
   print_our_vector( v );
#endif
   return sqrt( rmsd );
}

double US_Hydrodyn_Mals_Saxs_Fit::gsm_f_EMGGMG( our_vector *v )
{
   for ( unsigned int j = 0; j < gsm_yp.size(); j++ )
   {
      gsm_yp[ j ] = 0e0;
   }

   double height;
   double center;
   double width;
   double dist1;
   double dist2;

   for ( unsigned int i = 0; i < ( unsigned int ) MSFIT::param_fixed.size(); )
   {
      if ( MSFIT::param_fixed[ i ] )
      {
         height = MSFIT::fixed_params[ MSFIT::param_pos[ i ] ];
      } else {
         height = v->d[ MSFIT::param_pos[ i ] ];
         if ( height < MSFIT::param_min[ MSFIT::param_pos[ i ] ] ||
              height > MSFIT::param_max[ MSFIT::param_pos[ i ] ] )
         {
            return 1e99;
         }
      }

      i++;

      if ( MSFIT::param_fixed[ i ] )
      {
         center = MSFIT::fixed_params[ MSFIT::param_pos[ i ] ];
      } else {
         center = v->d[ MSFIT::param_pos[ i ] ];
         if ( center < MSFIT::param_min[ MSFIT::param_pos[ i ] ] ||
              center > MSFIT::param_max[ MSFIT::param_pos[ i ] ] )
         {
            return 1e99;
         }
      }

      i++;

      if ( MSFIT::param_fixed[ i ] )
      {
         if ( MSFIT::comm_backref.count( i ) )
         {
            width = v->d              [ MSFIT::comm_backref[ i ] ] * MSFIT::conc_ratios_map[ i ];
         } else {
            width = MSFIT::fixed_params[ MSFIT::param_pos[ i ] ];
         }
      } else {
         width = v->d[ MSFIT::param_pos[ i ] ];
         if ( width < MSFIT::param_min[ MSFIT::param_pos[ i ] ] ||
              width > MSFIT::param_max[ MSFIT::param_pos[ i ] ] )
         {
            return 1e99;
         }
      }

      i++;

      if ( MSFIT::param_fixed[ i ] )
      {
         if ( MSFIT::comm_backref.count( i ) )
         {
            dist1 = v->d              [ MSFIT::comm_backref[ i ] ];
         } else {
            dist1 = MSFIT::fixed_params[ MSFIT::param_pos[ i ] ];
         }               
      } else {
         dist1 = v->d         [ MSFIT::param_pos[ i ] ];
         if ( dist1 < MSFIT::param_min[ MSFIT::param_pos[ i ] ] ||
              dist1 > MSFIT::param_max[ MSFIT::param_pos[ i ] ] )
         {
            return 1e99;
         }
      }

      i++;

      if ( MSFIT::param_fixed[ i ] )
      {
         if ( MSFIT::comm_backref.count( i ) )
         {
            dist2 = v->d              [ MSFIT::comm_backref[ i ] ];
         } else {
            dist2 = MSFIT::fixed_params[ MSFIT::param_pos[ i ] ];
         }               
      } else {
         dist2 = v->d         [ MSFIT::param_pos[ i ] ];
         if ( dist2 < MSFIT::param_min[ MSFIT::param_pos[ i ] ] ||
              dist2 > MSFIT::param_max[ MSFIT::param_pos[ i ] ] )
         {
            return 1e99;
         }
      }

      i++;

#if defined( UHSHFG_DEBUG_F )
      cout << QString(  "EMGGMG: param %1: %2 %3 %4 %5 %6\n" )
         .arg( i )
         .arg( height )
         .arg( center )
         .arg( width )
         .arg( dist1 )
         .arg( dist2 )
         ;
#endif

      if ( !dist1 && !dist2 )
      {
         // puts( "EMGGMG as Gaussian (dist1, dist2 0)" );
         for ( unsigned int j = 0; j < gsm_yp.size(); j++ )
         {
            double tmp = ( gsm_t[ j ] - center ) / width;
            gsm_yp[ j ] += height * exp( - tmp * tmp * 5e-1 );
         }
      } else {
         if ( !dist1 )
         {
            // puts( "EMGGMG as GMG (dist1, dist2 0)" );
            // GMG
            double area                         = height * width * M_SQRT2PI;
            double one_over_width               = 1e0 / width;
            double one_over_a2sq_plus_a3sq      = 1e0 / ( width * width + dist2 * dist2 );
            double sqrt_one_over_a2sq_plus_a3sq = sqrt( one_over_a2sq_plus_a3sq );
            double gmg_coeff                    = area * M_ONE_OVER_SQRT2PI * sqrt_one_over_a2sq_plus_a3sq;
            double gmg_exp_m1                   = -5e-1 *  one_over_a2sq_plus_a3sq;
            double gmg_erf_m1                   = dist2 * sqrt_one_over_a2sq_plus_a3sq * M_ONE_OVER_SQRT2 * one_over_width;
            for ( unsigned int j = 0; j < gsm_yp.size(); j++ )
            {
               double tmp = gsm_t[ j ] - center;
               gsm_yp[ j ] += 
                  gmg_coeff * exp( gmg_exp_m1 * tmp * tmp ) *
                  ( 1e0 + use_erf( gmg_erf_m1 * tmp ) );
            }
         } else {
            if ( !dist2 )
            {
               // same as EMG here
               double dist1_thresh      = width / ( 5e0 * sqrt(2e0) - 2e0 );
               if ( fabs( dist1 ) < dist1_thresh )
               {
                  // puts( "EMGGMG EMG (dist2 0) averaged with gaussian" );
                  double frac_gauss = ( dist1_thresh - fabs( dist1 ) ) / dist1_thresh;
                  if ( dist1 < 0 )
                  {
                     dist1_thresh *= -1e0;
                  }

                  double area              = height * width * M_SQRT2PI;
                  double one_over_a3       = 1e0 / dist1_thresh;
                  double emg_coeff         = area * one_over_a3 * 5e-1 * (1e0 - frac_gauss );
                  double emg_exp_1         = width * width * one_over_a3 * one_over_a3 * 5e-1;
                  double emg_erf_2         = width * M_ONE_OVER_SQRT2 * one_over_a3;
                  double sign_a3           = dist1_thresh < 0e0 ? -1e0 : 1e0;
                  double one_over_sqrt2_a2 = M_ONE_OVER_SQRT2 / width;
                  double gauss_coeff       = frac_gauss * height;

                  // printf( "EMG t0 %g thresh %g frac gauss %g\n", dist1, dist1_thresh, frac_gauss );

                  for ( unsigned int j = 0; j < gsm_yp.size(); j++ )
                  {
                     double tmp = gsm_t[ j ] - center;
                     double tmp2 =  tmp / width;
               
                     gsm_yp[ j ] += 
                        emg_coeff * exp( emg_exp_1 - one_over_a3 * tmp ) *
                        ( use_erf( tmp * one_over_sqrt2_a2 - emg_erf_2 ) + sign_a3 ) +
                        gauss_coeff * exp( - tmp2 * tmp2 * 5e-1 );
                  }
               } else {
                  // puts( "EMGGMG EMG (dist2 0) pure" );
                  double area              = height * width * M_SQRT2PI;
                  double one_over_a3       = 1e0 / dist1;
                  double emg_coeff         = area * one_over_a3 * 5e-1;
                  double emg_exp_1         = width * width * one_over_a3 * one_over_a3 * 5e-1;
                  double emg_erf_2         = width * M_ONE_OVER_SQRT2 * one_over_a3;
                  double sign_a3           = dist1 < 0e0 ? -1e0 : 1e0;
                  double one_over_sqrt2_a2 = M_ONE_OVER_SQRT2 / width;
                  for ( unsigned int j = 0; j < gsm_yp.size(); j++ )
                  {
                     double tmp               = gsm_t[ j ] - center;
                     gsm_yp[ j ] += 
                        emg_coeff * exp( emg_exp_1 - one_over_a3 * tmp ) *
                        ( use_erf( tmp * one_over_sqrt2_a2 - emg_erf_2 ) + sign_a3 );
                  }
               }
            } else {
               // real EMGGMG
               double area                         = height * width * M_SQRT2PI;
               double one_over_width               = 1e0 / width;
               double one_over_a2sq_plus_a3sq      = 1e0 / ( width * width + dist2 * dist2 );
               double sqrt_one_over_a2sq_plus_a3sq = sqrt( one_over_a2sq_plus_a3sq );
               double gmg_coeff                    = 5e-1 * area * M_ONE_OVER_SQRT2PI * sqrt_one_over_a2sq_plus_a3sq;
               double gmg_exp_m1                   = -5e-1 * one_over_a2sq_plus_a3sq;
               double gmg_erf_m1                   = dist2 * sqrt_one_over_a2sq_plus_a3sq * M_ONE_OVER_SQRT2 * one_over_width;

               double dist1_thresh                 = width / ( 5e0 * sqrt(2e0) - 2e0 );
               if ( fabs( dist1 ) < dist1_thresh )
               {
                  double frac_gauss = ( dist1_thresh - fabs( dist1 ) ) / dist1_thresh;
                  if ( dist1 < 0 )
                  {
                     dist1_thresh *= -1e0;
                  }

                  double one_over_a3       = 1e0 / dist1_thresh;
                  double emg_coeff         = 5e-1 * area * one_over_a3 * 5e-1 * (1e0 - frac_gauss );
                  double emg_exp_1         = width * width * one_over_a3 * one_over_a3 * 5e-1;
                  double emg_erf_2         = width * M_ONE_OVER_SQRT2 * one_over_a3;
                  double sign_a3           = dist1_thresh < 0e0 ? -1e0 : 1e0;
                  double one_over_sqrt2_a2 = M_ONE_OVER_SQRT2 / width;
                  double gauss_coeff       = 5e-1 * frac_gauss * height;

                  for ( unsigned int j = 0; j < gsm_yp.size(); j++ )
                  {
                     double tmp            = gsm_t[ j ] - center;
                     double tmp2           = tmp / width;
               
                     gsm_yp[ j ] +=
                        emg_coeff * exp( emg_exp_1 - one_over_a3 * tmp ) *
                        ( use_erf( tmp * one_over_sqrt2_a2 - emg_erf_2 ) + sign_a3 ) +
                        gauss_coeff * exp( - tmp2 * tmp2 * 5e-1 ) +
                        gmg_coeff * exp( gmg_exp_m1 * tmp * tmp ) *
                        ( 1e0 + use_erf( gmg_erf_m1 * tmp ) );
                  }
               } else {
                  // puts( "EMGGMG GMG + pure EMG" );
                  // EMG
                  double one_over_a3       = 1e0 / dist1;
                  double emg_coeff         = 5e-1 * area * one_over_a3 * 5e-1;
                  double emg_exp_1         = width * width * one_over_a3 * one_over_a3 * 5e-1;
                  double emg_erf_2         = width * M_ONE_OVER_SQRT2 * one_over_a3;
                  double sign_a3           = dist1 < 0e0 ? -1e0 : 1e0;
                  double one_over_sqrt2_a2 = M_ONE_OVER_SQRT2 / width;

                  for ( unsigned int j = 0; j < gsm_yp.size(); j++ )
                  {
                     double tmp            = gsm_t[ j ] - center;
                     gsm_yp[ j ] += 
                        emg_coeff * exp( emg_exp_1 - one_over_a3 * tmp ) *
                        ( use_erf( tmp * one_over_sqrt2_a2 - emg_erf_2 ) + sign_a3 ) +
                        gmg_coeff * exp( gmg_exp_m1 * tmp * tmp ) *
                        ( 1e0 + use_erf( gmg_erf_m1 * tmp ) );
                  }
               }
            }
         }
      }
   }

   double rmsd = 0e0;

   for ( unsigned int j = 0; j < gsm_y.size(); j++ )
   {
      rmsd += ( gsm_y[ j ] - gsm_yp[ j ] ) * ( gsm_y[ j ] - gsm_yp[ j ] );
   }
#if defined( UHSHFG_DEBUG_F )
   printf( "EMGGMG: rmsd %.6g: ", sqrt( rmsd ) );
   print_our_vector( v );
#endif
   return sqrt( rmsd );
}

void US_Hydrodyn_Mals_Saxs_Fit::gsm_df( our_vector *vd, our_vector *v )
{
   double y0;
   double y2;
   double sav_ve;

   // compute partials for each our_vector element

   for ( int i = 0; i < v->len; i++ )
   {
      sav_ve      = v->d[ i ];
      v ->d[ i ] -= gsm_delta;
      y0          = gsm_f( v );
      v ->d[ i ]  = sav_ve + gsm_delta;
      y2          = gsm_f( v );
      vd->d[ i ]  = (y2 - y0) * gsm_delta2_r;
      v ->d[ i ]  = sav_ve;
   }
#if defined( UHSHFG_DEBUG_F )
   printf( "f: " );
   print_our_vector( v );
   printf( "df: " );
   print_our_vector( vd );
#endif
}
