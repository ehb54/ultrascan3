
#if defined( USE_MPI )
#  include <mpi.h>
#endif

#define  SMAX 20

#include <stdio.h>
#include <math.h>
#ifdef OSX
#include <sys/malloc.h>
#endif
#include <string.h>

#include <stdlib.h>

#include "../include/us_hydrodyn_pat.h"
#ifndef PI
# define  PI 3.141592654
#endif

// #define DEBUG_WW
#if defined(DEBUG_WW)
static int log_cnt = 0;
static double cks;
static FILE *logfx;
static void dww(char *s) {
   fprintf(logfx, "dww %s: %.12e\n", s, cks);
}
#endif

// #define OLD_WAY_CHECK
#if defined(OLD_WAY)
 static FILE *mol;
 static FILE *rmc;
#endif
#if defined(OLD_WAY_CHECK)
 static FILE *omol;
 static FILE *ormc;
 static void inp_inter();
#endif

static struct dati1_pat *dt = 0;   // [NMAX]
static struct dati1_pat *dtn = 0;   // [NMAX]

#if defined(OLD_WAY)
 static char ragcol[SMAX];
#endif
static int nat, flag, cc, FL, autovett;

static double a[3][3], a1[3][3], a2[3][3], a3[3][3];
static double v1[3], v2[3], v3[3], xx[3];
static double b, c, d;
static double vv[3][4];
static double dl1, dl2, dl3;
static double raggio;

#if defined(OLD_WAY)
 static void initarray();
#endif
static double approx(double a);
static void calc_CM();
static void calc_inertia_tensor();
static void secondo(double b, double c);
static void terzo(double b, double c, double d);
static void sisli(double w[3][3]);
static void sol(double c[2][3]);
static void place1(double a1[3]);
static void place2(double a1[3], double a2[3]);

static int nmax;

static void
pat_free_alloced()
{
   if (dt)
   {
      free(dt);
   }
   if (dtn)
   {
      free(dtn);
   }
}

static int
pat_alloc()
{
   dt = (struct dati1_pat *) malloc(2 * nmax * sizeof(struct dati1_pat));
   if (!dt)
   {
      pat_free_alloced();
      fprintf(stderr, "memory allocation error\n");
      return US_HYDRODYN_PAT_ERR_MEMORY_ALLOC;
   }
   memset(dt, 0, 2 * nmax * sizeof(struct dati1_pat));

   dtn = (struct dati1_pat *) malloc(nmax * sizeof(struct dati1_pat));
   if (!dtn)
   {
      pat_free_alloced();
      fprintf(stderr, "memory allocation error\n");
      return US_HYDRODYN_PAT_ERR_MEMORY_ALLOC;
   }
   memset(dtn, 0, nmax * sizeof(struct dati1_pat));
   
   return 0;
}

int 
us_hydrodyn_pat_main(int                 use_nmax, 
                     int                 in_nat, 
                     struct dati1_supc  *in_dt, 
                     int                *out_nat,
                     struct dati1_pat   *out_dt)
{
   nmax = use_nmax;
   if (int retval = pat_alloc())
   {
      return retval;
   }

#if defined(DEBUG_WW)
   cks = 0e0;
   {
      char s[30];
      sprintf(s, "pat_log-%d", log_cnt++);
      logfx = us_fopen(s, "w");
   }
#endif

   int vect; // not used: , kbh, num;
   int i, j, k, contatore;
   double max, vvj;

#if defined(DEBUG_WW)
   cks += (double)nmax;
   dww("start");
#endif

#if defined(OLD_WAY)
   initarray();
#endif
   nat = in_nat;
   raggio = 0.0;
   for (i = 0; i < nat; i++)
   {
#if defined(MIMIC_FILE)
      dt[i].x = QString("").sprintf("%f",in_dt[i].x).toDouble();
      dt[i].y = QString("").sprintf("%f",in_dt[i].y).toDouble();
      dt[i].z = QString("").sprintf("%f",in_dt[i].z).toDouble();
      dt[i].r = QString("").sprintf("%f",in_dt[i].r).toDouble();
      dt[i].m = QString("").sprintf("%f",in_dt[i].m).toDouble();
      dt[i].col = QString("").sprintf("%d",in_dt[i].col).toInt();
#else
      dt[i].x = in_dt[i].x;
      dt[i].y = in_dt[i].y;
      dt[i].z = in_dt[i].z;
      dt[i].r = in_dt[i].r;
      dt[i].m = in_dt[i].m;
      dt[i].col = in_dt[i].col;
#endif
      if ( i < 3 ) 
      {
         // printf("reading in coords in_dt bead %d @ %f %f %f %f %f %d\n", i, in_dt[i].x,in_dt[i].y,in_dt[i].z,in_dt[i].r, in_dt[i].m, in_dt[i].col);
         // printf("reading in coords dt bead %d @ %f %f %f %f %f %d\n", i, dt[i].x,dt[i].y,dt[i].z,dt[i].r, dt[i].m, dt[i].col);
      }
   }
#if defined(OLD_WAY_CHECK)
   inp_inter();
   {
      
      int i;
      
      FILE *interinp = us_fopen("p_in_ifraxon", "wb");
      FILE *interinp1 = us_fopen("p_in_ifraxon1", "wb");

      fprintf(interinp, "%d\n", nat);
      fprintf(interinp, "%f\n", 0.0);
      fprintf(interinp, "%s\n", "p_in_ifraxon1");
      
      for (i = 0; i < nat; i++)
      {
         fprintf(interinp, "%f\t", in_dt[i].x);
         fprintf(interinp, "%f\t", in_dt[i].y);
         fprintf(interinp, "%f\n", in_dt[i].z);
         fprintf(interinp1, "%f\t", in_dt[i].r);
         fprintf(interinp1, "%f\t", in_dt[i].m);
         fprintf(interinp1, "%d\n", in_dt[i].col);
      }
      
      fclose(interinp);
      fclose(interinp1);
   }
#endif

   contatore = 0;
   max = dt[0].m;

   // printf("pat: nmax %d nat %d\n", nmax, nat);

   for (i = 1; i < nat; i++)
      if (dt[i].m > max)
         max = dt[i].m;

#if defined(DEBUG_WW)
   cks += (double)max;
   dww("pat 2");
#endif

   FL = 1;

   if (max <= 10)
      FL = 10;
   else if (max <= 100)
      FL = 100;
   else if (max <= 1000)
      FL = 1000;
   else if (max <= 10000)
      FL = 10000;
   else if (max <= 100000)
      FL = 100000;
   else if (max <= 1000000)
      FL = 1000000;

#if defined(DEBUG_WW)
   cks += (double)FL;
   dww("pat 2");
#endif

   calc_CM();
#if defined(DEBUG_WW)
   for (i = 0; i < nat; i++) {
      cks += (double)dt[i].x;
      cks += (double)dt[i].y;
      cks += (double)dt[i].z;
      //      cks += (double)dt[i].r;
      //      cks += (double)dt[i].m;
      //      cks += (double)dt[i].col;
   }
   dww("after calc CM");
#endif
   calc_inertia_tensor();
#if defined(DEBUG_WW)
   for (i = 0; i < 3; i++) {
      for (j = 0; j < 3; j++) {
         cks += (double)a[i][j];
      }
   }
   dww("after calc inertia tensor");
#endif

 RET:

   contatore++;

   /* COMPUTATION OF THE EIGENVALUES dl1, dl2, dl3 OF THE MATRIX A */

   cc = 0;

   if ((a[0][1] == 0.0) && (a[1][2] == 0.0) && (a[0][2] == 0.0))
   {
      dl1 = a[0][0];
      dl2 = a[1][1];
      dl3 = a[2][2];
   }

   else if ((a[0][2] == 0.0) && (a[1][2] == 0.0))
   {
      dl1 = a[2][2];
      b = (-a[0][0] - a[1][1]);
      c = a[0][0] * a[1][1] - a[0][1] * a[1][0];
      secondo(b, c);
   }

   else if ((a[0][2] == 0.0) && (a[0][1] == 0.0))
   {
      dl1 = a[0][0];
      b = (-a[2][2] - a[1][1]);
      c = a[2][2] * a[1][1] - a[2][1] * a[1][2];
      secondo(b, c);
   }

   else
   {
      b = (-(a[0][0] + a[1][1] + a[2][2]));
      c = a[0][0] * a[1][1] + a[0][0] * a[2][2] + a[1][1] * a[2][2] - a[0][2] * a[2][0] - a[1][2] * a[2][1] -
         a[0][1] * a[1][0];
      d = (double)(-a[0][0] * a[1][1] * a[2][2] - 2.0 * a[0][1] * a[0][2] * a[1][2] + a[0][2] * a[0][2] * a[1][1] +
                  a[1][2] * a[1][2] * a[0][0] + a[0][1] * a[0][1] * a[2][2]);
      terzo(b, c, d);
   }

   /* FOR EACH EIGENVALUE A MATRIX b IS COMPUTED HAVING AS DIAGONAL: */
   /* b[i][i]=a[i][i]-dli                       */

   for (i = 0; i < 3; i++)
      for (j = 0; j < 3; j++)
         a1[i][j] = a[i][j];

   a1[0][0] = a[0][0] - dl1;
   a1[1][1] = a[1][1] - dl1;
   a1[2][2] = a[2][2] - dl1;

   if (cc == 0)
   {
      for (i = 0; i < 3; i++)
         for (j = 0; j < 3; j++)
         {
            a2[i][j] = a[i][j];
            a3[i][j] = a[i][j];
         }

      a2[0][0] = a[0][0] - dl2;
      a3[0][0] = a[0][0] - dl3;
      a2[1][1] = a[1][1] - dl2;
      a3[1][1] = a[1][1] - dl3;
      a2[2][2] = a[2][2] - dl2;
      a3[2][2] = a[2][2] - dl3;
   }

   sisli(a1);

   for (i = 0; i < 3; i++)
      v1[i] = xx[i];

   if (cc == 0)
   {
      sisli(a2);

      for (i = 0; i < 3; i++)
         v2[i] = xx[i];

      sisli(a3);

      for (i = 0; i < 3; i++)
         v3[i] = xx[i];
   }
   else
   {
      dl2 = dl3 = 999999;
      for (i = 0; i < 3; i++)
      {
         v2[i] = 0.0;
         v3[i] = 0.0;
      }
   }

   vv[0][0] = dl1;
   vv[1][0] = dl2;
   vv[2][0] = dl3;

   for (j = 0; j < 3; j++)
   {
      vv[0][j + 1] = v1[j];
      vv[1][j + 1] = v2[j];
      vv[2][j + 1] = v3[j];
   }

   /*
     for(i=0;i<3;i++) 
     printf("%s%f\t%f\t%f\t%f\n","autov + vett ",vv[i][0],vv[i][1],vv[i][2],vv[i][3]);
   */

#if defined(DEBUG_WW)
   for (i = 0; i < 3; i++) {
      cks += (double)a1[i][i];
      cks += (double)a2[i][i];
      cks += (double)a2[i][i];
      cks += (double)vv[i][0];
      cks += (double)vv[i][1];
      cks += (double)vv[i][2];
   }
   dww("pat 3");
#endif

   for (i = 0; i < 2; i++)
      for (k = i + 1; k < 3; k++)
      {
         if (vv[i][0] <= vv[k][0]);
         else
         {
            for (j = 0; j < 4; j++)
            {
               vvj = vv[i][j];
               vv[i][j] = vv[k][j];
               vv[k][j] = vvj;
            }
         }
      }

   /*
     for(i=0;i<3;i++) 
     printf("%s%f\t%f\t%f\t%f\n","autov + vett ",vv[i][0],vv[i][1],vv[i][2],vv[i][3]);
   */

   /* WE BUILD THE NEW COORDINATES OF THE CENTERS */

   vect = 0;
   autovett = 0;

   if ((vv[0][1] != 0.0) || (vv[0][2] != 0.0) || (vv[0][3] != 0.0))
   {
      autovett = 1;
      vect = 1;
      for (i = 0; i < 3; i++)
         v1[i] = vv[0][i + 1];

      if ((vv[1][1] != 0.0) || (vv[1][2] != 0.0) || (vv[1][3] != 0.0))
      {
         vect = 2;
         for (i = 0; i < 3; i++)
            v2[i] = vv[1][i + 1];
      }
   }

   else if ((vv[1][1] != 0.0) || (vv[1][2] != 0.0) || (vv[1][3] != 0.0))
   {
      autovett = 2;
      vect = 1;
      for (i = 0; i < 3; i++)
         v1[i] = vv[1][i + 1];
   }
   else
   {
      autovett = 3;
      vect = 1;
      for (i = 0; i < 3; i++)
         v1[i] = vv[2][i + 1];
   }

   /*
     printf("%s%d\t","vect ",vect);
     printf("%s%d\n","autovett ",autovett);
   */

   if (vect == 1)
      place1(v1);

   if (vect == 2)
      place2(v1, v2);

   for (i = 0; i < nat; i++)
   {
      dt[i].x = dtn[i].x;
      dt[i].y = dtn[i].y;
      dt[i].z = dtn[i].z;
   }

   calc_CM();
#if defined(DEBUG_WW)
   for (i = 0; i < nat; i++) {
      cks += (double)dt[i].x;
      cks += (double)dt[i].y;
      cks += (double)dt[i].z;
      //      cks += (double)dt[i].r;
      //      cks += (double)dt[i].m;
      //      cks += (double)dt[i].col;
   }
   dww("after calc CM 2");
#endif
   calc_inertia_tensor();
#if defined(DEBUG_WW)
   for (i = 0; i < 3; i++) {
      for (j = 0; j < 3; j++) {
         cks += (double)a[i][j];
      }
   }
   dww("after calc inertia tensor 2");
#endif

   if (contatore > 4)
   {
      *out_nat = nat;
      for (i = 0; i < nat; i++)
      {
         // if ( i < 3 ) 
         // {
         // printf("pat bead %d @ %f %f %f\n", i, dtn[i].x,dtn[i].y,dtn[i].z);
         // }
         out_dt[i].x = dtn[i].x;
         out_dt[i].y = dtn[i].y;
         out_dt[i].z = dtn[i].z;
         out_dt[i].r = dtn[i].r;
         out_dt[i].m = dtn[i].m;
         out_dt[i].col = dtn[i].col;
      }

#if defined(OLD_WAY_CHECK)
      omol = us_fopen("ofraxon", "w");

      fprintf(omol, "%d\t", nat);
      fprintf(omol, "%f\t", raggio);
      fprintf(omol, "%s\n", "ofraxon1");

      ormc = us_fopen("ofraxon1", "w");

      for (i = 0; i < nat; i++)
      {
         fprintf(omol, "%f\t", dtn[i].x);
         fprintf(omol, "%f\t", dtn[i].y);
         fprintf(omol, "%f\n", dtn[i].z);
         if ((raggio == -1.0) || (raggio == -3.0))
            fprintf(ormc, "%f\t", dt[i].rg);
         fprintf(ormc, "%f\t", dt[i].r);
         fprintf(ormc, "%d\t", dt[i].m);
         fprintf(ormc, "%d\n", dt[i].col);
      }

      fclose(omol);
      fclose(ormc);
#endif

   }
   else
   {
      if ((fabs(a[0][0] - vv[0][0]) < 1) && (fabs(a[1][1] - vv[1][0]) < 1) && (fabs(a[2][2] - vv[2][0]) < 1))
      {
         *out_nat = nat;
         for (i = 0; i < nat; i++)
         {
            // if ( i < 3 ) 
            // {
            // printf("pat bead %d @ %f %f %f\n", i, dtn[i].x,dtn[i].y,dtn[i].z);
            // }
            out_dt[i].x = dtn[i].x;
            out_dt[i].y = dtn[i].y;
            out_dt[i].z = dtn[i].z;
            out_dt[i].r = dtn[i].r;
            out_dt[i].m = dtn[i].m;
            out_dt[i].col = dtn[i].col;
         }
#if defined(OLD_WAY_CHECK)

         omol = us_fopen("ofraxon", "w");

         fprintf(omol, "%d\t", nat);
         fprintf(omol, "%f\t", raggio);
         fprintf(omol, "%s\n", "ofraxon1");

         ormc = us_fopen("ofraxon1", "w");

         for (i = 0; i < nat; i++)
         {
            fprintf(omol, "%f\t", dtn[i].x);
            fprintf(omol, "%f\t", dtn[i].y);
            fprintf(omol, "%f\n", dtn[i].z);
            if ((raggio == -1.0) || (raggio == -3.0))
               fprintf(ormc, "%f\t", dt[i].rg);
            fprintf(ormc, "%f\t", dt[i].r);
            fprintf(ormc, "%d\t", dt[i].m);
            fprintf(ormc, "%d\n", dt[i].col);
         }

         fclose(omol);
         fclose(ormc);
#endif
      }
      else
         goto RET;

   }

   pat_free_alloced();
#if defined(DEBUG_WW)
   fclose(logfx);
#endif
   return 0;
}

static double
approx(double a)
{

   double app;

   app = a;

   if ((ceil(a) - a <= 0.01) && (ceil(a) - a >= 0.0))
      app = ceil(a);

   if ((a - floor(a) <= 0.01) && (a - floor(a) >= 0.0))
      app = floor(a);

   return (app);
}

#if defined(OLD_WAY_CHECK)

static void
inp_inter()
{

   int i;

   FILE *interinp = us_fopen("p_ifraxon", "wb");
   FILE *interinp1 = us_fopen("p_ifraxon1", "wb");

   fprintf(interinp, "%d\n", nat);
   fprintf(interinp, "%f\n", 0.0);
   fprintf(interinp, "%s\n", "p_ifraxon1");

   for (i = 0; i < nat; i++)
   {
      fprintf(interinp, "%f\t", dt[i].x);
      fprintf(interinp, "%f\t", dt[i].y);
      fprintf(interinp, "%f\n", dt[i].z);
      fprintf(interinp1, "%f\t", dt[i].r);
      fprintf(interinp1, "%d\t", dt[i].m);
      fprintf(interinp1, "%d\n", dt[i].col);
   }

   fclose(interinp);
   fclose(interinp1);
}

#endif

static void
calc_CM()
{

   int i; // not used: nn;
   double b1x, b1y, b1z, mas1;

   /* COMPUTATION OF THE MODELS' CENTER OF MASS */

   b1x = 0.0;
   b1y = 0.0;
   b1z = 0.0;
   mas1 = 0.0;

   for (i = 0; i < nat; i++)
   {
      b1x += (double) dt[i].x * dt[i].m;
      b1y += (double) dt[i].y * dt[i].m;
      b1z += (double) dt[i].z * dt[i].m;
      mas1 += dt[i].m;
   }

   b1x = b1x / mas1;
   b1y = b1y / mas1;
   b1z = b1z / mas1;

   /*
     printf ("\ncoordinates of the models's center of mass\n\n");
     printf("%.2f\t%.2f\t%.2f\n\n",b1x,b1y,b1z);
   */

   for (i = 0; i < nat; i++)
   {
      dt[i].x = dt[i].x - b1x;
      dt[i].y = dt[i].y - b1y;
      dt[i].z = dt[i].z - b1z;
   }

   /*
     printf(" new coordinates with the center of mass in (0 0 0)\n\n");
     for(i=0;i<nat;i++)
     printf("%f\t%f\t%f\n",dt[i].x,dt[i].y,dt[i].z);

   */

}

static void
calc_inertia_tensor()
{

   int i, j;

   /* COMPUTATION OF THE INERTIA TENSOR */

   for (i = 0; i < 3; i++)
      for (j = 0; j < 3; j++)
         a[i][j] = 0.0;

   for (i = 0; i < nat; i++)
   {
      a[0][0] += dt[i].m * (dt[i].y * dt[i].y + dt[i].z * dt[i].z) / FL;
      a[1][1] += dt[i].m * (dt[i].x * dt[i].x + dt[i].z * dt[i].z) / FL;
      a[2][2] += dt[i].m * (dt[i].y * dt[i].y + dt[i].x * dt[i].x) / FL;
      a[0][1] += dt[i].m * dt[i].x * dt[i].y / FL;
      a[1][2] += dt[i].m * dt[i].y * dt[i].z / FL;
      a[0][2] += dt[i].m * dt[i].x * dt[i].z / FL;
   }

   a[0][1] = (-a[0][1]);
   a[1][2] = (-a[1][2]);
   a[0][2] = (-a[0][2]);
   a[1][0] = a[0][1];
   a[2][1] = a[1][2];
   a[2][0] = a[0][2];

   for (i = 0; i < 3; i++)
      for (j = 0; j < 3; j++)
         a[i][j] = approx(a[i][j]);

}

#if defined(OLD_WAY)
static void
initarray()
{
   int i;

   //   mol = us_fopen("ifraxon", "r");

   //   fscanf(mol, "%d", &nat);
   //   fscanf(mol, "%f", &raggio);

   if (raggio == 0.0)
   {
      fscanf(mol, "%s", ragcol);
      rmc = us_fopen(ragcol, "r");   /* opening the file containing the unhydrated radii, the masses and the colors of the beads */

      for (i = 0; i < nat; i++)
      {
         fscanf(mol, "%f", &dt[i].x);
         fscanf(mol, "%f", &dt[i].y);
         fscanf(mol, "%f", &dt[i].z);
         fscanf(rmc, "%f", &dt[i].r);
         fscanf(rmc, "%d", &dt[i].m);
         fscanf(rmc, "%d", &dt[i].col);
      }
   }

   else if ((raggio == -1.0) || (raggio == -3.0))
   {
      fscanf(mol, "%s", ragcol);
      rmc = us_fopen(ragcol, "r");   /* opening the file containg both the hydrated and unhydrated radii, the masses and the colors of the beads */

      for (i = 0; i < nat; i++)
      {
         fscanf(mol, "%f", &dt[i].x);
         fscanf(mol, "%f", &dt[i].y);
         fscanf(mol, "%f", &dt[i].z);
         fscanf(rmc, "%f", &dt[i].rg);
         fscanf(rmc, "%f", &dt[i].r);
         fscanf(rmc, "%d", &dt[i].m);
         fscanf(rmc, "%d", &dt[i].col);
      }
   }

   fclose(rmc);
   fclose(mol);

#if defined(DEBUG_WW)
   for (i = 0; i < nat; i++) {
      cks += (double)dt[i].x;
      cks += (double)dt[i].y;
      cks += (double)dt[i].z;
      cks += (double)dt[i].r;
      cks += (double)dt[i].m;
      cks += (double)dt[i].col;
   }
   dww("initarray end");
#endif

}
#endif

static void
place2(double a1[3], double a2[3])
{

   int i;
   double tetar = 0.0;
   double b1x, b1y, b1z, b1nx, b1ny, b1nz;
   double b2x, b2y, b2z, b2nx, b2ny, b2nz;

   b1x = a1[0];
   b1y = a1[1];
   b1z = a1[2];
   b2x = a2[0];
   b2y = a2[1];
   b2z = a2[2];

   /* ORIENTATION OF THE MODEL ON THE XZ PLANE */

   /* computation of the angle between the line      */
   /* connecting the origin O with a1 and the X axis */

   if (fabs(b1y) > 0.0001)   /* if y is > 0 ... */
   {
      if (fabs(b1x) < 0.0001)   /* if x is = 0 ... */
      {
         if (b1y > 0.0)
            tetar = (double)(PI / 2.0);
         else
            tetar = (double)(-PI / 2.0);
      }

      else         /* if x and y are both > 0 ...  */
      {
         if ((b1y > 0.0) && (b1x > 0.0))
            tetar = (double)atan(b1y / b1x);
         else if ((b1y < 0.0) && (b1x > 0.0))
            tetar = (double)(-atan(fabs(b1y / b1x)));
         else if ((b1y < 0.0) && (b1x < 0.0))
            tetar = (double)(-(PI - atan(b1y / b1x)));
         else if ((b1y > 0.0) && (b1x < 0.0))
            tetar = (double)(PI - atan(fabs(b1y / b1x)));
      }
   }

   else
   {
      if (b1x < 0.0)
         tetar = (double)PI;
      else
         tetar = 0.0;
   }

   for (i = 0; i < nat; i++)
   {
      dtn[i].x = dt[i].x * cos(tetar) + dt[i].y * sin(tetar);
      dtn[i].y = (-dt[i].x * sin(tetar)) + dt[i].y * cos(tetar);
      dtn[i].z = dt[i].z;
   }

   b1nx = b1x * cos(tetar) + b1y * sin(tetar);
   b1ny = (-b1x * sin(tetar)) + b1y * cos(tetar);
   b1nz = b1z;

   b2nx = b2x * cos(tetar) + b2y * sin(tetar);
   b2ny = (-b2x * sin(tetar)) + b2y * cos(tetar);
   b2nz = b2z;

   /* the model is aligned so to have the line paasing through */
   /* the origin O and a1 in the XZ plane                      */

   if (fabs(b1nz) > 0.0001)   /* if y is > 0 ... */
   {
      if (fabs(b1nx) < 0.0001)   /* if x is = 0 ... */
      {
         if (b1nz > 0.0)
            tetar = (double)(PI / 2.0);
         else
            tetar = (double)(-PI / 2.0);
      }

      else         /* if x and y are both > 0 ...  */
      {
         if ((b1nz > 0.0) && (b1nx > 0.0))
            tetar = (double)atan(b1nz / b1nx);
         else if ((b1nz < 0.0) && (b1nx > 0.0))
            tetar = (double)(-atan(fabs(b1nz / b1nx)));
         else if ((b1nz < 0.0) && (b1nx < 0.0))
            tetar = (double)(-(PI - atan(b1nz / b1nx)));
         else if ((b1nz > 0.0) && (b1nx < 0.0))
            tetar = (double)(PI - atan(fabs(b1nz / b1nx)));
      }
   }

   else
   {
      if (b1nx < 0.0)
         tetar = (double)PI;
      else
         tetar = 0.0;
   }

   for (i = 0; i < nat; i++)
   {
      dt[i].x = dtn[i].x * cos(tetar) + dtn[i].z * sin(tetar);
      dt[i].z = (-dtn[i].x * sin(tetar)) + dtn[i].z * cos(tetar);
      dt[i].y = dtn[i].y;
   }

   b1x = b1nx * cos(tetar) + b1nz * sin(tetar);
   b1z = (-b1nx * sin(tetar)) + b1nz * cos(tetar);
   b1y = b1ny;

   b2x = b2nx * cos(tetar) + b2nz * sin(tetar);
   b2z = (-b2nx * sin(tetar)) + b2nz * cos(tetar);
   b2y = b2ny;

   /* we have now the eigenvector a1 aligned on the X axis */
   /* we must still bring the O,a2 axis on the XY plane    */

   /* PART TWO */

   /* ROTATION OF THE MODELS AROUND THE O,a1 AXIS */
   /* TO BRING THE O,a2 AXIS ON THE XY PLANE      */

   /* computation of the angle between the projection */
   /* of the O,a2 line on the ZY plane and the Y axis */

   if (fabs(b2z) > 0.0001)   /* if z is != 0 ... */
   {
      if (fabs(b2y) < 0.0001)   /* if y is = 0 ... */
      {
         if (b2z > 0.0)
            tetar = (double)(-PI / 2.0);
         else
            tetar = (double)(PI / 2.0);
      }

      else         /* if z and y are both > 0 ...  */
      {
         if ((b2z > 0.0) && (b2y > 0.0))
            tetar = (-atan(b2z / b2y));
         else if ((b2z < 0.0) && (b2y > 0.0))
            tetar = atan(fabs(b2z / b2y));
         else if ((b2z < 0.0) && (b2y < 0.0))
            tetar = (double)(PI - atan(b2z / b2y));
         else if ((b2z > 0.0) && (b2y < 0.0))
            tetar = (double)(-(PI - atan(fabs(b2z / b2y))));
      }
   }

   else
   {
      if (b2y < 0.0)
         tetar = (double)PI;
      else
         tetar = 0.0;
   }

   for (i = 0; i < nat; i++)
   {
      dtn[i].z = dt[i].z * cos(tetar) + dt[i].y * sin(tetar);
      dtn[i].y = (-dt[i].z * sin(tetar)) + dt[i].y * cos(tetar);
      dtn[i].x = dt[i].x;
   }

}

/****************************************************************/

static void
place1(double a1[3])
{

   int i;
   double tetar = 0.0;
   double b1x, b1y, b1z, b1nx, /* b1ny, */ b1nz;

   b1x = a1[0];
   b1y = a1[1];
   b1z = a1[2];

   /* ORIENTATION ON THE XZ PLANE */

   /* computation of the angle between the line   */
   /* joining the origin O with a1 and the X axis */

   if (fabs(b1y) > 0.0001)   /* if y is > 0 ... */
   {
      if (fabs(b1x) < 0.0001)   /* if x is = 0 ... */
      {
         if (b1y > 0.0)
            tetar = (double)(PI / 2.0);
         else
            tetar = (double)(-PI / 2.0);
      }

      else         /* if both x and y are > 0 ...  */
      {
         if ((b1y > 0.0) && (b1x > 0.0))
            tetar = atan(b1y / b1x);
         else if ((b1y < 0.0) && (b1x > 0.0))
            tetar = (-atan(fabs(b1y / b1x)));
         else if ((b1y < 0.0) && (b1x < 0.0))
            tetar = (double)(-(PI - atan(b1y / b1x)));
         else if ((b1y > 0.0) && (b1x < 0.0))
            tetar = (double)PI - atan(fabs(b1y / b1x));
      }
   }

   else
   {
      if (b1x < 0.0)
         tetar = (double)PI;
      else
         tetar = 0.0;
   }

   for (i = 0; i < nat; i++)
   {
      dtn[i].x = dt[i].x * cos(tetar) + dt[i].y * sin(tetar);
      dtn[i].y = (-dt[i].x * sin(tetar)) + dt[i].y * cos(tetar);
      dtn[i].z = dt[i].z;
   }

   b1nx = b1x * cos(tetar) + b1y * sin(tetar);
   // b1ny = (-b1x * sin(tetar)) + b1y * cos(tetar);
   b1nz = b1z;

   /* the model is now aligned so that the line passing */
   /* through the origin O and a1 lies in the XZ plane  */

   if (fabs(b1nz) > 0.0001)   /* if y is > 0 ... */
   {
      if (fabs(b1nx) < 0.0001)   /* if x is = 0 ... */
      {
         if (b1nz > 0.0)
            tetar = (double)(PI / 2.0);
         else
            tetar = (double)(-PI / 2.0);
      }

      else         /* if x and y are both > 0 ...  */
      {
         if ((b1nz > 0.0) && (b1nx > 0.0))
            tetar = atan(b1nz / b1nx);
         else if ((b1nz < 0.0) && (b1nx > 0.0))
            tetar = (-atan(fabs(b1nz / b1nx)));
         else if ((b1nz < 0.0) && (b1nx < 0.0))
            tetar = (-((double)PI - atan(b1nz / b1nx)));
         else if ((b1nz > 0.0) && (b1nx < 0.0))
            tetar = (double)PI - atan(fabs(b1nz / b1nx));
      }
   }

   else
   {
      if (b1nx < 0.0)
         tetar = (double)PI;
      else
         tetar = 0.0;
   }

   for (i = 0; i < nat; i++)
   {
      dt[i].x = dtn[i].x * cos(tetar) + dtn[i].z * sin(tetar);
      dt[i].z = (-dtn[i].x * sin(tetar)) + dtn[i].z * cos(tetar);
      dt[i].y = dtn[i].y;
   }

   /* here the eigenvector is aligned on the X axis */

   if (autovett == 1)
   {

      for (i = 0; i < nat; i++)
      {
         dtn[i].x = dt[i].x;
         dtn[i].y = dt[i].y;
         dtn[i].z = dt[i].z;
      }
      /* here the eigenvector a1 is aligned on the X axis */
   }
   else
   {
      if (autovett == 2)
      {
         for (i = 0; i < nat; i++)
         {
            dtn[i].y = dt[i].x;
            dtn[i].x = (-dt[i].y);
            dtn[i].z = dt[i].z;
         }
         /* here the eigenvector a1 is aligned on the Y axis */
      }
      else
      {
         for (i = 0; i < nat; i++)
         {
            dtn[i].z = dt[i].x;
            dtn[i].x = (-dt[i].z);
            dtn[i].y = dt[i].y;
         }
      }
      /* here the eigenvector a1 is aligned on the Z axis */
   }

}

/*********************************************************************/

static void
secondo(double b, double c)
{
   double delta, pfraz, pin;

   delta = b * b - 4.0f * c;

   pfraz = fabs(delta);
   pin = floor(pfraz);
   pfraz = pfraz - pin;

   if (((pin == 0) && (pfraz < 0.00001) && (delta <= 0.0)) || ((pin == 0) && (pfraz < 0.2) && (delta > 0.0)))
   {
      dl2 = (-b / 2.0f);
      dl3 = dl2;
   }
   else if (delta > 0.0)
   {
      dl2 = (-b + sqrt(b * b - 4.0f * c)) / 2.0f;
      dl3 = (-b - sqrt(b * b - 4.0f * c)) / 2.0f;
   }
   else
      cc = 1;         /* TWO COMPLEX CONJUGATED EIGENVALUES */

}

static void
sisli(double w[3][3])
{

   // not used: double det, det1, det2, det3, det4, det5, det6;
   double b[2][3]; // not used: k[3];
   int i, j;

   /* WE CONSIDER THE MINORS OF MAXIMUM ORDER EQUAL TO TWO */
   /* TO FIND THE SOLUTIONS FOR THE SYSTEM                 */

   for (i = 0; i < 2; i++)
      for (j = 0; j < 3; j++)
         b[i][j] = w[i][j];

   /* CALLING "SOL" TO CHECK FOR THE MINOR OF ORDER 2 <> 0 */

   sol(b);

   if (flag == 1)
      goto LAB;

   for (i = 1; i < 3; i++)
      for (j = 0; j < 3; j++)
         b[i - 1][j] = w[i][j];

   sol(b);

   if (flag == 1)
      goto LAB;

   for (j = 0; j < 3; j++)
   {
      b[0][j] = w[0][j];
      b[1][j] = w[2][j];
   }

   sol(b);

   if (flag == 1)
      goto LAB;

   /* THERE IS A SYMMETRY PLANE */

 LAB:
   ;
}

static void
sol(double c[2][3])
{

   double xr, x1[3];
   double c1, c2, c3;

   /* CHECK THAT AT LEAST ONE MINOR OF ORDER TWO IS <> 0 */

   c1 = (c[0][0] * c[1][1] - c[1][0] * c[0][1]);
   c2 = (c[0][1] * c[1][2] - c[1][1] * c[0][2]);
   c3 = (c[0][0] * c[1][2] - c[1][0] * c[0][2]);

   if ((c1 < -0.01) || (c1 > 0.01) || (c2 < -0.01) || (c2 > 0.01) || (c3 < -0.01) || (c3 > 0.01))
   {
      flag = 1;

      x1[0] = c[0][1] * c[1][2] - c[1][1] * c[0][2];
      x1[1] = (-(c[0][0] * c[1][2] - c[1][0] * c[0][2]));
      x1[2] = c[0][0] * c[1][1] - c[1][0] * c[0][1];

      /* THE EIGENVECTOR IS NORMALIZED TO 1 */

      xr = 1.0f / sqrt(x1[0] * x1[0] + x1[1] * x1[1] + x1[2] * x1[2]);
      xx[0] = xr * x1[0];
      xx[1] = xr * x1[1];
      xx[2] = xr * x1[2];
   }

   /* PROBABLE SYMMETRY PLANE */
   else
   {
      flag = 0;

      xx[0] = 0;
      xx[1] = 0;
      xx[2] = 0;
   }

}


static void
terzo(double b, double c, double d)
{

   // not used: double nb, nc, nd;
   double p, q, s, r, pq, epsi, y1, y2, y3, alfa, alf1;
   double unterzo, beta, bet1, rad, pfraz, pin;

   if ((c == 0.0) && (d == 0.0))
   {
      dl1 = 0.0;
      dl3 = 0.0;
      dl2 = (-b);
      goto RET0;
   }

   if (d == 0.0)
   {
      dl1 = 0.0;
      secondo(b, c);
      goto RET0;
   }

   rad = (double)(atan(1.00) / 45.0);
   p = (-(b * b) / 3.0f + c);
   q = (2.0f * b * b * b - 9.0f * c * b + 27.0f * d) / 27.0f;
   s = (q * q) / 4.0f + (p * p * p) / 27.0f;

   pfraz = fabs(s);
   pin = floor(pfraz);
   pfraz = pfraz - pin;

   if ((s > 0.0) && ((pin != 0) || ((pin == 0) && (pfraz > 0.01))))
      cc = 1;         /* TWO COMPLEX CONJUGATED EIGENVALUES */

   else if ((s < 0.0) && ((pin != 0) || ((pin == 0) && (pfraz > 0.01))))
   {
      r = sqrt(-4.0f * p / 3.0f);
      pq = 4.0f * q / (r * r * r);
      epsi = (asin(pq)) / 3.0f;
      y1 = r * sin(epsi);
      y2 = r * sin(60.0f * rad - epsi);
      y3 = (-r * sin(60.0f * rad + epsi));

      dl1 = y1 - b / 3.0f;
      dl2 = y2 - b / 3.0f;
      dl3 = y3 - b / 3.0f;

      goto RET0;
   }
   else
   {
      alfa = (-q / 2.0f + sqrt(q * q / 4.0f + p * p * p / 27.0f));
      unterzo = 1.0f / 3.0f;

      if (alfa < 0.0)
      {
         alfa = (-alfa);
         alf1 = (-pow(alfa, unterzo));
      }

      else
         alf1 = pow(alfa, unterzo);

      beta = (-q / 2.0f - sqrt(q * q / 4.0f + p * p * p / 27.0f));

      if (beta < 0.0)
      {
         beta = (-beta);
         bet1 = (-pow(beta, unterzo));
      }

      else
         bet1 = pow(beta, unterzo);

      dl1 = alf1 + bet1;
      dl2 = (-(alf1 + bet1) / 2.0f);
      dl3 = dl2;

      dl1 = dl1 - b / 3.0f;
      dl2 = dl2 - b / 3.0f;
      dl3 = dl3 - b / 3.0f;

   }

 RET0:
   ;
}

// #define USE_MAIN

#if defined(USE_MAIN)
int
main()
{
   us_hydrodyn_pat_main(250);
}
#endif
