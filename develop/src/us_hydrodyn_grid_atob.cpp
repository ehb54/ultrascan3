#include "../include/us_hydrodyn_grid_atob.h"

#include <stdlib.h>
#ifdef OSX
#  include <sys/malloc.h>
#endif

#undef DEBUG

/*Declaration of new structure types*/

/* Declaration of prototypes...*/
static QProgressBar *progress;
static QTextEdit *editor;
static US_Hydrodyn *us_hydrodyn;

#define NR_END 1
#define FREE_ARG char*
#define VW_average_radius 1.5   /* Van der Walls average radius equal to 1.5 A */
#define EXPANSION_COEF 0.2   /*how much bigger LMAX will be with respect to the protein max size! (0.2=20%) */

// #define DEBUG
// #define DEBUG_ATOB

static float
particle_max_size(PDB * pdb)
{

   PDB *p;
   float max_size = 0.0;
   float temp = 0.0;

   // int countr = 0;

   PDB *t;
   for (p = pdb; p; p = p->next)
      for (t = p->next; t; t = t->next)
      {
         temp = ((p->x) - (t->x)) * ((p->x) - (t->x));
         temp += ((p->y) - (t->y)) * ((p->y) - (t->y));
         temp += ((p->z) - (t->z)) * ((p->z) - (t->z));
         if (temp > max_size)
            max_size = temp;
         //       printf("%f %f %f - %f %f %f\n", 
         //   p->x, t->x,
         //   p->y, t->y,
         //   p->z, t->z);

         // printf("temp %d %f\n", countr++, temp); fflush(stdout);
      }

   for (p = pdb; p; p = p->next)
   {
      temp = (p->x) * (p->x) + (p->y) * (p->y) + (p->z) * (p->z);
      if (temp > max_size)
      {
            max_size = temp;
      }
   }

   return ((float) sqrt(max_size));

}

static void
nrerror(char error_text[])
   /* Numerical Recipes standard error handler */
{
   fprintf(stderr, "Numerical Recipes run-time error...\n");
   fprintf(stderr, "%s\n", error_text);
   //   fprintf(stderr, "...now exiting to system...\n");
   //   exit(1);
}

static float ***
f3tensor(long nrl, long nrh, long ncl, long nch, long ndl, long ndh)
   /* allocate a float 3tensor with range t[nrl..nrh][ncl..nch][ndl..ndh] */
{
   long i, j, nrow = nrh - nrl + 1, ncol = nch - ncl + 1, ndep = ndh - ndl + 1;
   float ***t;

   /* allocate pointers to pointers to rows */
   t = (float ***) malloc((size_t) ((nrow + NR_END) * sizeof(float **)));
   if (!t)
   {
      nrerror("allocation failure 1 in f3tensor()");
      return (float ***) 0;
   }
   t += NR_END;
   t -= nrl;

   /* allocate pointers to rows and set pointers to them */
   t[nrl] = (float **) malloc((size_t) ((nrow * ncol + NR_END) * sizeof(float *)));
   if (!t[nrl])
   {
      nrerror("allocation failure 2 in f3tensor()");
      return (float ***) 0;
   }
   t[nrl] += NR_END;
   t[nrl] -= ncl;

   /* allocate rows and set pointers to them */
   t[nrl][ncl] = (float *) malloc((size_t) ((nrow * ncol * ndep + NR_END) * sizeof(float)));
   if (!t[nrl][ncl])
   {
      nrerror("allocation failure 3 in f3tensor()");
      return (float ***) 0;
   }      
   t[nrl][ncl] += NR_END;
   t[nrl][ncl] -= ndl;

   for (j = ncl + 1; j <= nch; j++)
      t[nrl][j] = t[nrl][j - 1] + ndep;
   for (i = nrl + 1; i <= nrh; i++)
   {
      t[i] = t[i - 1] + ncol;
      t[i][ncl] = t[i - 1][ncl] + ncol * ndep;
      for (j = ncl + 1; j <= nch; j++)
         t[i][j] = t[i][j - 1] + ndep;
   }

   /* return pointer to array of pointers to rows */
   return t;
}

static void
free_f3tensor(float ***t, long nrl, long nrh, long ncl, long nch, long ndl, long ndh)
   /* free a float f3tensor allocated by f3tensor() */
{
   free((FREE_ARG) (t[nrl][ncl] + ndl - NR_END));
   free((FREE_ARG) (t[nrl] + ncl - NR_END));
   free((FREE_ARG) (t + nrl - NR_END));
   nrh++;
   nch++;
   ndh++;
}

static PDB *
AtoB(PDB * pdb, long npoints, float dx,
     // long *natoms,
     PHYSPROP * prop,
     PHYSPROP * nprop, 
     long *natoms3, 
     int centre_or_cog, 
     char input_file[], 
     int set_bead_radius, 
     char atnam[], 
     char resnam[])
{
#if defined(DEBUG)
   printf("npoints %ld\n", npoints);
   puts("in atob 0");
   fflush(stdout);
#endif
   /*
     pdb= PDB * pointer of structure
     npoints = LMAX/dx, LMAX=1.2*DMAX
     dx= cube's side
     prop= PHYSPROP structure with radii and masses of beads read from AtomicVW.dat
     nprop= pointer where it puts new pdb PHYSPROP to be used if writing file in output format. has to be allocated in calling routine!
     natoms3= number of atoms in new AtoB model. has to be allocated in calling routine!
     centre_of_cog= if zero places beads in cubes centres, if 1 if calculates the cog of the beads in the cube and places bead therein
     set_bead_radius= if zero, does nothing, if not it sets the bead radius to half the cube's side
     input_file used to generate log file
     atnam ? string for atom name used
     atnam ? string for residue name used
   */

   PDB *p, *npdb, *temp, *prev, *current;
   PHYSPROP *pp = prop, *nthis, *nprev, *ntemp;
   FILE *fptr;
   // BEAD_CONN ***w,*wthis,*wprev;

#if defined(DEBUG)
   puts("in atob 1");
   fflush(stdout);
#endif
   int i, j, k, x_c, y_c, z_c, n, nn;
   long natoms2 = 0;
   char log_filename[50];
   float ***ro = (float ***) 0;
   float ***x = (float ***) 0;
   float ***y = (float ***) 0;
   float ***z = (float ***) 0;
   float ***mass = (float ***) 0;

   if ( !( ro = (float ***) f3tensor(0, npoints, 0, npoints, 0, npoints) ) ||
        !( mass = (float ***) f3tensor(0, npoints, 0, npoints, 0, npoints) ) ||
        !( x = (float ***) f3tensor(0, npoints, 0, npoints, 0, npoints) ) ||
        !( y = (float ***) f3tensor(0, npoints, 0, npoints, 0, npoints) ) ||
        !( z = (float ***) f3tensor(0, npoints, 0, npoints, 0, npoints) ) )
   {
      if ( ro )
         free_f3tensor(ro, 0, npoints, 0, npoints, 0, npoints);
      if ( x )
         free_f3tensor(x, 0, npoints, 0, npoints, 0, npoints);
      if ( y )
         free_f3tensor(y, 0, npoints, 0, npoints, 0, npoints);
      if ( z )
         free_f3tensor(z, 0, npoints, 0, npoints, 0, npoints);
      if ( mass )
         free_f3tensor(mass, 0, npoints, 0, npoints, 0, npoints);
      return (PDB *)0;
   }
#if defined(DEBUG)
   puts("in atob 2");
   fflush(stdout);
#endif

   //  wthis=w;wprev=w;

   //calculates radii and mass vs i,j,k
#if defined(DEBUG)
   puts("in atob 3");
   fflush(stdout);
#endif

   for (i = 0; i < npoints; i++)   /* clears up r0 and mass */
      for (j = 0; j < npoints; j++)
         for (k = 0; k < npoints; k++)
         {
            ro[i][j][k] = 0.0;
            mass[i][j][k] = 0.0;
            x[i][j][k] = 0.0;
            y[i][j][k] = 0.0;
            z[i][j][k] = 0.0;
            //    w[i][j][k].i=0;
         }

#if defined(DEBUG)
   puts("in atob 3b");
   fflush(stdout);
#endif

   strcpy(log_filename, input_file);
   strcat(log_filename, ".log");
   if ((fptr = fopen(log_filename, "w")) == NULL)
   {
      fprintf(stderr, "\n No se puede abrir archivo para escribir!!");
      exit(1);
   }

#if defined(DEBUG)
   puts("in atob 3c");
   fflush(stdout);
#endif
   for (p = pdb, pp = prop, n = 0; p; p = p->next, pp = pp->next, n++)   /* over the atoms  */
   {
#if defined(DEBUG)
      printf("n = %d p = %lx dx = %f\n", (int) n, (long unsigned int) p, dx);
      fflush(stdout);
      printf("n = %d p->x = %f\n", (int) n, p->x);
      fflush(stdout);
#endif
      x_c = (int) ((p->x) / dx) + (int) (npoints / 2);
      y_c = (int) ((p->y) / dx) + (int) (npoints / 2);
      z_c = (int) ((p->z) / dx) + (int) (npoints / 2);
#if defined(DEBUG)
      puts("in atob 3c_1");
      fflush(stdout);
      printf("_c %d %d %d\n", x_c, y_c, z_c);
      fflush(stdout);
#endif

      ro[x_c][y_c][z_c] += pow(pp->rVW, 3);   /* = ctte*electron_density*gaussian */
#if defined(DEBUG)
      puts("in atob 3c_2");
      fflush(stdout);
#endif
      mass[x_c][y_c][z_c] += (float) pp->mass;
      if (centre_or_cog == 1)   //calculates centre of mass
      {
#if defined(DEBUG)
         puts("in atob 3c_3_cog");
         fflush(stdout);
#endif
         x[x_c][y_c][z_c] += (float) (p->x) * (pp->mass);
         y[x_c][y_c][z_c] += (float) (p->y) * (pp->mass);
         z[x_c][y_c][z_c] += (float) (p->z) * (pp->mass);
      }
#if defined(DEBUG)
      puts("in atob 3c_4");
      fflush(stdout);

      printf("2nd n = %d\n", (int) n);
      fflush(stdout);
#endif
      fprintf(fptr, "[%d,%d,%d] Bead %d \n", x_c, y_c, z_c, n);

   }
   //wprev->next=NULL;
   fclose(fptr);

#if defined(DEBUG)
   puts("in atob 4");
   fflush(stdout);
#endif

   for (i = 0; i < npoints; i++)   /* Now the radius of the beads are in r0[][][] */
      for (j = 0; j < npoints; j++)
         for (k = 0; k < npoints; k++)
            ro[i][j][k] = pow((double)ro[i][j][k], 0.333333);

#if defined(DEBUG)
   puts("in atob 4 b");
   fflush(stdout);
#endif

   // writes down results in PDB and PHYSPROP linked lists

   npdb = (PDB *) malloc(sizeof(PDB));   //allocates new pdb structure file
   current = npdb;
   prev = npdb;
   nthis = nprop;
   nprev = nprop;
#if defined(DEBUG)
   puts("in atob 4 c");
   fflush(stdout);
   printf("npoints %d\n", (int)npoints); fflush(stdout);
#endif

   for (i = 0; i < npoints; i++)   /* clears up r0 */
   {
      qApp->processEvents();
      if (us_hydrodyn->stopFlag)
      {
         free_f3tensor(mass, 0, npoints, 0, npoints, 0, npoints);
         free_f3tensor(ro, 0, npoints, 0, npoints, 0, npoints);
         free_f3tensor(x, 0, npoints, 0, npoints, 0, npoints);
         free_f3tensor(y, 0, npoints, 0, npoints, 0, npoints);
         free_f3tensor(z, 0, npoints, 0, npoints, 0, npoints);
         return npdb;
      }
      for (j = 0; j < npoints; j++)
         for (k = 0; k < npoints; k++)
         {
            if (ro[i][j][k] > 0.000001)
            {
               natoms2++;
               prev = current;
               nprev = nthis;
               if (centre_or_cog == 1)   //calculates centre of mass for locating the beads
               {
                  current->x = (float) (x[i][j][k] / mass[i][j][k]);
                  current->y = (float) (y[i][j][k] / mass[i][j][k]);
                  current->z = (float) (z[i][j][k] / mass[i][j][k]);

               }
               else   //puts centres of cells
               {
                  current->x = dx * i + (float) (dx * npoints / 2);
                  current->y = dx * j + (float) (dx * npoints / 2);
                  current->z = dx * k + (float) (dx * npoints / 2);
               }

               current->atnum = natoms2;
               current->bval = 1;
               current->resnum = (int) (natoms2);
               strcpy(current->insert, " ");
               strcpy(current->junk, "ATOM");
               strcpy(current->resnam, resnam);
               strcpy(current->atnam, atnam);
               strcpy(current->chain, "A");
               current->occ = 1.0;
               //printf("#%d: x=%f , y=%f, z=%f\n",i,current->x,current->y,current->z);
               //fprintf(stderr,"\n >>>>> set bead radius = %d !!! >>> \n",set_bead_radius);
               //assigns radius, mass and colour
               if (centre_or_cog == 1 || set_bead_radius == 0)
                  nthis->rVW = (float) ro[i][j][k];   /*Van der Walls radii */
               else
               {
                  if (ro[i][j][k] <= (float) (dx / 2))   //if coord centre of cell, it puts a maximum radius
                     nthis->rVW = (float) ro[i][j][k];   //equal to half the cell's diameter
                  else
                  {
                     nthis->rVW = (float) (dx / 2);
                     fprintf(stderr, "\n >>>>> changing radius of bead %d!!! >>> \n", (int) natoms2);
                  }
               }

               nthis->mass = (float) mass[i][j][k];   /* atomic mass */
               nthis->f = (float) 6;   /* assigns electron density. */
               temp = (PDB *) malloc(sizeof(PDB));
               ntemp = (PHYSPROP *) malloc(sizeof(PHYSPROP));

               current->next = temp;
               current = temp;
               nthis->next = ntemp;

               nthis = ntemp;
            }
         }
   }
   prev->next = NULL;
   nprev->next = NULL;

#if defined(DEBUG)
   puts("in atob 5");
   fflush(stdout);
   printf("\nNumber of atoms in AtoB model at dx=%.2f A: %d\n", dx, (int) natoms2);
#endif
   /*    for(i=0,p=npdb,pp=nprop;p;p=p->next,pp=pp->next,i++)
         {
         //fprintf(fptr,"%f\t%f\t%f\n",p->x,p->y,p->z); //writes output file
         //fprintf(rmc,"%f\t%.0f\t%.0f\n",pp->rVW,pp->mass,pp->f);// writes RMC file

         //printf("%d:%f\t%.0f\t%.0f\n",i,pp->rVW,pp->mass,pp->f);
         }
   */

   *natoms3 = natoms2;

   //writes down log file with bead connections!

   strcpy(log_filename, input_file);
   strcat(log_filename, ".rmcoresp");
   if ((fptr = fopen(log_filename, "w")) == NULL)
   {
      fprintf(stderr, "\n No se puede abrir archivo para escribir!!");
      exit(1);
   }
   printf(">>Writing down correspondance file: %s\n", log_filename);

   for (n = 1, i = 0; i < npoints; i++)
      for (j = 0; j < npoints; j++)
         for (k = 0; k < npoints; k++)
         {
            if (mass[i][j][k] > 0)   //gets only if there is a bead
            {
               fprintf(fptr, "\n\nBead %d: ", n);

               for (p = pdb, nn = 0; p; p = p->next, nn++)   // goes over all atoms
                  if (((int) ((p->x) / dx) + (int) (npoints / 2)) == i)
                     if (((int) ((p->y) / dx) + (int) (npoints / 2)) == j)
                        if (((int) ((p->z) / dx) + (int) (npoints / 2)) == k)
                        {
                           fprintf(fptr, "%d-", nn);
                        }
               n++;
            }
         }

   fclose(fptr);

   /*Freeing electron density memory! */
   free_f3tensor(mass, 0, npoints, 0, npoints, 0, npoints);
   free_f3tensor(ro, 0, npoints, 0, npoints, 0, npoints);
   free_f3tensor(x, 0, npoints, 0, npoints, 0, npoints);
   free_f3tensor(y, 0, npoints, 0, npoints, 0, npoints);
   free_f3tensor(z, 0, npoints, 0, npoints, 0, npoints);
   printf("\n");
#if defined(DEBUG)
   puts("in atob _end");
   fflush(stdout);
#endif
   return (npdb);
}

vector < PDB_atom > us_hydrodyn_grid_atob(vector < PDB_atom > *bead_model,
                                          grid_options * use_grid_options, 
                                          QProgressBar * use_progress, 
                                          QTextEdit * use_editor,
                                          US_Hydrodyn * use_us_hydrodyn)
{
   // do our stuff
   us_hydrodyn = use_us_hydrodyn;
   us_hydrodyn->errorFlag = false;
#if defined(DEBUG)
   puts("grid_atob 0");
   fflush(stdout);
#endif
   progress = use_progress;
   editor = use_editor;
   vector < PDB > pdb;
   vector < PHYSPROP > prop;
   //   vector < PHYSPROP > nprop;
   PHYSPROP *nprop;

#if defined(DEBUG)
   puts("grid_atob 1");
   fflush(stdout);
#endif

   printf("bead model size %d\n", (int)bead_model->size()); fflush(stdout);
   float max_radius = VW_average_radius;
   for (unsigned int i = 0; i < bead_model->size(); i++)
   {
      if ((*bead_model)[i].active)
      {
#if defined(DEBUG)
         puts("grid_atob 1 a"); fflush(stdout);
#endif
         PDB tmp_pdb;

         tmp_pdb.x = (*bead_model)[i].bead_coordinate.axis[0];
         tmp_pdb.y = (*bead_model)[i].bead_coordinate.axis[1];
         tmp_pdb.z = (*bead_model)[i].bead_coordinate.axis[2];

         tmp_pdb.occ = (*bead_model)[i].occupancy;   // ?emre
         tmp_pdb.bval = 1;

         tmp_pdb.next = (PDB *) 0;

         tmp_pdb.atnum = (*bead_model)[i].serial;
         tmp_pdb.resnum = 1; // (*bead_model)[i].resSeq;

         strcpy(tmp_pdb.junk, "ATOM");
#if defined(DEBUG)
         puts("grid_atob 1 b"); fflush(stdout);
#endif
         strncpy(tmp_pdb.atnam, (*bead_model)[i].name.ascii(), 7);
         tmp_pdb.atnam[7] = 0;

         strncpy(tmp_pdb.resnam, (*bead_model)[i].resName.ascii(), 7);
         tmp_pdb.resnam[7] = 0;

         strncpy(tmp_pdb.insert, (*bead_model)[i].iCode.ascii(), 7);
         tmp_pdb.insert[7] = 0;

         strncpy(tmp_pdb.chain, (*bead_model)[i].chainID.ascii(), 7);
         tmp_pdb.chain[7] = 0;
         pdb.push_back(tmp_pdb);

         PHYSPROP tmp_prop;

         tmp_prop.f = (*bead_model)[i].bead_color;
         tmp_prop.rVW = (*bead_model)[i].bead_computed_radius;
         tmp_prop.mass = (*bead_model)[i].bead_mw;

         tmp_prop.next = (PHYSPROP *) 0;

         prop.push_back(tmp_prop);
#if defined(DEBUG)
         puts("grid_atob 1 g"); fflush(stdout);
#endif
         if (max_radius < tmp_prop.rVW) {
            max_radius = tmp_prop.rVW;
         }

#if defined(DEBUG_ATOB)
         printf(
                "%f %f %f %f %f %d %d %f %f %f\n"
                ,tmp_pdb.x
                ,tmp_pdb.y
                ,tmp_pdb.z
                ,tmp_pdb.occ
                ,tmp_pdb.bval
                ,tmp_pdb.atnum
                ,tmp_pdb.resnum
                ,tmp_prop.f
                ,tmp_prop.rVW
                ,tmp_prop.mass
                );
#endif

      }
   }

#if defined(DEBUG)
   puts("grid_atob 2"); fflush(stdout);
   fflush(stdout);
#endif
   for (unsigned int i = 0; i < pdb.size() - 1; i++)
   {
      pdb[i].next = (PDB *) & pdb[i + 1];
      prop[i].next = (PHYSPROP *) & prop[i + 1];
   }

   float LMAX = 0.0;
   float DMAX;
   float epsilon = 0.0;

   DMAX = particle_max_size((PDB *)&pdb[0]);
   printf("0 - LMAX %f DMAX %f\n", LMAX, DMAX); fflush(stdout);

   /*This assures that the spacing left between the box frontere and the closest particle atom
     is larger than three times the standard VW radius, that is the electron density will always
     have space left to expand at least 1.5A away from the particle surface */
   if (EXPANSION_COEF < (float) (max_radius * 3 / DMAX))
   {
      epsilon = (float) (max_radius * 3 / DMAX);
   }
   else
   {
      epsilon = (float)EXPANSION_COEF;
   }
   /*max box size is slightly (EXP_COEF*100 %) bigger than particle! */

   LMAX = DMAX * (1 + epsilon);
   printf("LMAX %f DMAX %f\n", LMAX, DMAX);
   long npoints = (long) (LMAX / use_grid_options->cube_side);
   printf("npoints %ld\n", npoints);

#if defined(DEBUG)
   puts("grid_atob 3");
   fflush(stdout);
#endif
   npoints *= 2;

   { // test alloc
      PHYSPROP *p;
      p = (PHYSPROP *) malloc((int) pow((double)npoints, 3) * sizeof(PHYSPROP));
      if ( !p )
      {
         fprintf(stderr, "memory allocation failure\n");
         vector < PDB_atom > pdbnull;
         return pdbnull;
      }
      free(p);
#if defined(DEBUG)
      puts("grid_atob 3 - test alloc ok");
      fflush(stdout);
#endif
   }
   
   // nprop.resize((int) pow((double)npoints, 3));
   if (!(nprop = (PHYSPROP *) malloc((int) pow((double)npoints, 3) * sizeof(PHYSPROP)) ))
   {
      QColor save_color = us_hydrodyn->editor->color();
      editor->setColor("red");
      us_hydrodyn->editor->append("ERROR: Memory allocation failure.  Please try a larger grid cube size and/or close all other applications");
      us_hydrodyn->editor->setColor(save_color);
      us_hydrodyn->errorFlag = true;
      vector < PDB_atom > empty_result;
      return empty_result;
   }

#if defined(DEBUG)
   puts("grid_atob 4");
   fflush(stdout);

   // note last ones are already set to zero
   puts("grid_atob 5");
   fflush(stdout);
#endif

   long natoms3;
#if defined(DEBUG_ATOB)
   printf("npoints %d\n"
          "cube side %f\n"
          "max radius %f\n"
          ,(int)npoints
          ,(float) use_grid_options->cube_side
          ,max_radius
          );
#endif

   qApp->processEvents();
   if (us_hydrodyn->stopFlag)
   {
      free(nprop);
      vector < PDB_atom > empty_result;
      return empty_result;
   }

   PDB *result_pdb = AtoB((PDB *) & pdb[0],
                          2 * npoints,
                          (float) use_grid_options->cube_side,
                          (PHYSPROP *) & prop[0],
                          (PHYSPROP *) & nprop[0],
                          &natoms3,
                          use_grid_options->center ? 0 : 1,
                          "tmp_atob",
                          use_grid_options->tangency ? 1 : 0,
                          "PB",
                          "RES");
   if ( !result_pdb ) 
   {
      free(nprop);
      QColor save_color = us_hydrodyn->editor->color();
      editor->setColor("red");
      us_hydrodyn->editor->append("ERROR: Memory allocation failure.  Please try a larger grid cube size and/or close all other applications");
      us_hydrodyn->editor->setColor(save_color);
      us_hydrodyn->errorFlag = true;
      vector < PDB_atom > empty_result;
      return empty_result;
   }

#if defined(DEBUG)
   puts("grid_atob 6");
   fflush(stdout);
#endif
   qApp->processEvents();
   if (us_hydrodyn->stopFlag)
   {
      free(nprop);
      vector < PDB_atom > empty_result;
      return empty_result;
   }

   vector < PDB_atom > result_bead_model;

   PDB *this_pdb = result_pdb;
   PHYSPROP *this_prop = &nprop[0];
   for (; this_pdb; this_pdb = this_pdb->next, this_prop = this_prop->next)
   {
      PDB_atom tmp_atom;
      tmp_atom.bead_coordinate.axis[0] = this_pdb->x;
      tmp_atom.bead_coordinate.axis[1] = this_pdb->y;
      tmp_atom.bead_coordinate.axis[2] = this_pdb->z;
      tmp_atom.bead_computed_radius = this_prop->rVW;
      tmp_atom.bead_actual_radius = this_prop->rVW;
      tmp_atom.radius = this_prop->rVW;
      tmp_atom.bead_mw = this_prop->mass;
      tmp_atom.mw = this_prop->mass;
      tmp_atom.bead_ref_mw = this_prop->mass;
      tmp_atom.bead_ref_volume = 0;
      tmp_atom.bead_color = 1;
      tmp_atom.serial = this_pdb->atnum;
      tmp_atom.exposed_code = 1;
      tmp_atom.all_beads.clear();
      tmp_atom.name = QString(this_pdb->atnam);
      tmp_atom.resName = QString(this_pdb->resnam);
      tmp_atom.iCode = QString(this_pdb->insert);
      tmp_atom.chainID = QString(this_pdb->chain);
      tmp_atom.chain = 1;
      tmp_atom.active = 1;
      tmp_atom.normalized_ot_is_valid = false;
      result_bead_model.push_back(tmp_atom);
   }
   free(nprop);
#if defined(DEBUG)
   puts("grid_atob 7");
   fflush(stdout);
#endif
   editor->append(QString("Grid contains %1 beads\n").arg(result_bead_model.size()));
   printf("bead model size %d\n", (int)result_bead_model.size()); fflush(stdout);

   return result_bead_model;
}
