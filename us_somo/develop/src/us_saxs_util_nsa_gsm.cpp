#include "../include/us_saxs_util.h"
#include "../include/us_saxs_gp.h"

#if defined( USE_MPI )
    extern int myrank;
#endif

// #define USUNG_DEBUG

long US_Saxs_Util::nsa_min_gsm_5_1( our_vector *i, double epsilon, long max_iter ) 
{
#if defined( USUNG_DEBUG )
# if defined( USE_MPI )
   debug_mpi ( QString( "%1: starting nsa_min_gsm_5_1: our_vector\n" ).arg( myrank ) );
# else
   cout << QString( "starting nsa_min_gsm_5_1\n" ) << fflush;
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
   nsa_gsm_df(u, i);
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
             iter, fitness = nsa_gsm_f(i), l2norm_our_vector(u, zero), last_reps); fflush(stdout);
      printf("%d: ", this_rank);
      print_our_vector(i);
      fflush(stdout);
#endif
#if !defined(PRINT_GSM_INFO)
      global_iter++;
      fitness = nsa_gsm_f( i );
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
      /* find minimum of nsa_gsm_f(i - t u) */
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
      g_s2 = nsa_gsm_f(v_s2);

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
         g_s2 = nsa_gsm_f(v_s2);
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
      g_s3 = nsa_gsm_f(v_s3);


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
                  x = s1 / 2;
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
            g_s1 = nsa_gsm_f(v_s1);
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
               g_s2 = nsa_gsm_f(v_s2);
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
                  g_s2 = nsa_gsm_f(v_s2);
               } else {
                  /* ugh x >= s3.. well why not? */
                  if(x > (s3 + s3 - s2)) { /* keep it close */
                     mult_our_vector_vvs(v_s4, u, -x);
                     add_our_vector_vv(v_s4, i);
                     g_s4 = nsa_gsm_f(v_s4);
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
                  g_s3 = nsa_gsm_f(v_s3);
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

      if(evenn) {
         switch(queries) {
         case 8 : i->d[(6 * N_2) - 1] = 0; [[fallthrough]];
         case 7 : [[fallthrough]];
         case 6 : i->d[(4 * N_2) - 1] = 0; [[fallthrough]];
         case 5 : [[fallthrough]];
         case 4 : i->d[(2 * N_2) - 1] = 0;  [[fallthrough]];
         default : break;
         }
      }
#if defined(SHOW_TIMING)
      gettimeofday(&tv2, NULL);
      if(show_times)
         printf("time = %ld %ld\n", tv2.tv_sec - tv1.tv_sec,  tv2.tv_usec - tv1.tv_usec);
#endif
#if defined(PRINT_GSM_INFO)
      printf("%d: df start\n", this_rank); 
      fflush(stdout);
#endif
      nsa_gsm_df(u, i);
      /*    mult_our_vector_vs(u, 1e0 / l2norm_our_vector(u, zero)); */
#if defined( USUNG_DEBUG )
      printf("end\t{%.12g,%.12g,%.12g} = {%.12g,%.12g,%.12g}\n", s1, s2, s3, g_s1, g_s2, g_s3);
#endif

#if defined(PSV_OUTPUT)
      if(!(global_iter % 5)) {
         //      int r, s, j;
      
         printf("%d: query4i_intermediate|alg4|%s|%d|%.12g|%d|%d|%d|%d|%d|%.12g|%.12g|%.12g|%.12g|%ld|%d|%d|%d|%.12g|%ld|%ld", this_rank,
                (nsa_gsm_f(i) <= 0e0) ? "success" : "fail",
                N, 
                nsa_gsm_f(i),
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

long US_Saxs_Util::nsa_min_fr_pr_cgd(our_vector *i, double epsilon, long max_iter) 
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
   nsa_gsm_df(u, i);
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
             iter, fitness = nsa_gsm_f(i), l2norm_our_vector(u, zero), last_reps); fflush(stdout);
      printf("%d: ", this_rank);
      print_our_vector(i);
#endif
#if defined( USUNG_DEBUG )
      print_our_vector(u);
#endif
#if !defined(PRINT_GSM_INFO)
      global_iter++;
      fitness = nsa_gsm_f( i );
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
      /* find minimum of nsa_gsm_f(i - t u) */
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
      g_s2 = nsa_gsm_f(v_s2);

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
         g_s2 = nsa_gsm_f(v_s2);
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
      g_s3 = nsa_gsm_f(v_s3);


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
                  x = s1 / 2;
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
            g_s1 = nsa_gsm_f(v_s1);
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
               g_s2 = nsa_gsm_f(v_s2);
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
                  g_s2 = nsa_gsm_f(v_s2);
               } else {
                  /* ugh x >= s3.. well why not? */
                  if(x > (s3 + s3 - s2)) { /* keep it close */
                     mult_our_vector_vvs(v_s4, u, -x);
                     add_our_vector_vv(v_s4, i);
                     g_s4 = nsa_gsm_f(v_s4);
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
                  g_s3 = nsa_gsm_f(v_s3);
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


      if(evenn) {
         switch(queries) {
         case 8 : i->d[(6 * N_2) - 1] = 0; [[fallthrough]];
         case 7 : [[fallthrough]];
         case 6 : i->d[(4 * N_2) - 1] = 0; [[fallthrough]];
         case 5 : [[fallthrough]];
         case 4 : i->d[(2 * N_2) - 1] = 0; [[fallthrough]];
         default : break;
         }
      }

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
      nsa_gsm_df(u, i);
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
                (nsa_gsm_f(i) <= 0e0) ? "success" : "fail",
                N, 
                nsa_gsm_f(i),
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

long US_Saxs_Util::nsa_min_hessian_bfgs(our_vector *ip, double epsilon, long max_iter) 
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
   nsa_gsm_df(u, ip);

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
             iter, fitness = nsa_gsm_f(ip), l2norm_our_vector(u, zero), last_reps); fflush(stdout);
      printf("%d: ", this_rank);
      print_our_vector(ip);
      /*    print_our_vector(u); */
      fflush(stdout);
#endif
      /*    printf("ip : ");
            print_our_vector(ip); */
      /* find minimum of nsa_gsm_f(ip - t u) */
      /* alg 5_2 */
#if !defined(PRINT_GSM_INFO)
      global_iter++;
      fitness = nsa_gsm_f( ip );
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
      g_s2 = nsa_gsm_f(v_s2);

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
         g_s2 = nsa_gsm_f(v_s2);
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
      g_s3 = nsa_gsm_f(v_s3);

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
                  x = s1 / 2;
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
            g_s1 = nsa_gsm_f(v_s1);
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
               g_s2 = nsa_gsm_f(v_s2);
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
                  g_s2 = nsa_gsm_f(v_s2);
               } else {
                  /* ugh x >= s3.. well why not? */
                  if(x > (s3 + s3 - s2)) { /* keep it close */
                     mult_our_vector_vvs(v_s4, u, -x);
                     add_our_vector_vv(v_s4, ip);
                     g_s4 = nsa_gsm_f(v_s4);
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
                  g_s3 = nsa_gsm_f(v_s3);
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


      if(evenn) {
         switch(queries) {
         case 8 : v_p->d[(6 * N_2) - 1] = 0; [[fallthrough]];
         case 7 : [[fallthrough]];
         case 6 : v_p->d[(4 * N_2) - 1] = 0; [[fallthrough]];
         case 5 : [[fallthrough]];
         case 4 : v_p->d[(2 * N_2) - 1] = 0; [[fallthrough]];
         default : break;
         }
      }

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
      nsa_gsm_df(v_g, v_p);                 /* new gradient in v_g (old in u) */
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
                (nsa_gsm_f(ip) <= 0e0) ? "success" : "fail",
                N, 
                nsa_gsm_f(ip),
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
