#include "../include/us_hydrodyn_grid_atob.h"

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

#include <stdlib.h>
#ifdef OSX
#  include <sys/malloc.h>
//Added by qt3to4:
#include <QTextStream>
#endif

#undef DEBUG
// #define DEBUG_ATOB

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
#if defined(OLD_WAY)
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
#endif

static vector < float >
particle_max_sizes(PDB * pdb)
{

   PDB *p;
   float max_x = 0.0;
   float max_y = 0.0;
   float max_z = 0.0;
   float min_x = 0.0;
   float min_y = 0.0;
   float min_z = 0.0;

   // int countr = 0;

   for (p = pdb; p; p = p->next)
   {
      if ( max_x < p->x )
      {
         max_x = p->x;
      }
      if ( max_y < p->y )
      {
         max_y = p->y;
      }
      if ( max_z < p->z )
      {
         max_z = p->z;
      }

      if ( min_x > p->x )
      {
         min_x = p->x;
      }
      if ( min_y > p->y )
      {
         min_y = p->y;
      }
      if ( min_z > p->z )
      {
         min_z = p->z;
      }
   }      
   vector < float > results;
   results.push_back(min_x);
   results.push_back(max_x);
   results.push_back(min_y);
   results.push_back(max_y);
   results.push_back(min_z);
   results.push_back(max_z);
   return results;
}

static void
nrerror(const char error_text[])
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
AtoB(PDB * pdb, 
     long npoints_x, 
     long npoints_y, 
     long npoints_z, 
     float dx,
     // long *natoms,
     PHYSPROP * prop,
     PHYSPROP * nprop, 
     long *natoms3, 
     int centre_or_cog, 
     const char input_file[], 
     int set_bead_radius, 
     const char atnam[], 
     const char resnam[],
     US_Hydrodyn *us_hydrodyn,
     bool create_nmr,
     QString &fn,
     bool &nmr_created )
{
#if defined(DEBUG)
   //   printf("npoints %ld\n", npoints);
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
   float ***si  = (float ***) 0;
   float ***saxs_excl_vol = (float ***) 0;

   if ( !( ro = (float ***) f3tensor(0, npoints_x, 0, npoints_y, 0, npoints_z) ) ||
        !( mass = (float ***) f3tensor(0, npoints_x, 0, npoints_y, 0, npoints_z) ) ||
        !( saxs_excl_vol = (float ***) f3tensor(0, npoints_x, 0, npoints_y, 0, npoints_z) ) ||
        !( si = (float ***) f3tensor(0, npoints_x, 0, npoints_y, 0, npoints_z) ) ||
        !( x = (float ***) f3tensor(0, npoints_x, 0, npoints_y, 0, npoints_z) ) ||
        !( y = (float ***) f3tensor(0, npoints_x, 0, npoints_y, 0, npoints_z) ) ||
        !( z = (float ***) f3tensor(0, npoints_x, 0, npoints_y, 0, npoints_z) ) )
   {
      if ( ro )
         free_f3tensor(ro, 0, npoints_x, 0, npoints_y, 0, npoints_z);
      if ( x )
         free_f3tensor(x, 0, npoints_x, 0, npoints_y, 0, npoints_z);
      if ( y )
         free_f3tensor(y, 0, npoints_x, 0, npoints_y, 0, npoints_z);
      if ( z )
         free_f3tensor(z, 0, npoints_x, 0, npoints_y, 0, npoints_z);
      if ( mass )
         free_f3tensor(mass, 0, npoints_x, 0, npoints_y, 0, npoints_z);
      if ( si )
         free_f3tensor(si, 0, npoints_x, 0, npoints_y, 0, npoints_z);
      if ( saxs_excl_vol )
         free_f3tensor(saxs_excl_vol, 0, npoints_x, 0, npoints_y, 0, npoints_z);
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

   for (i = 0; i < npoints_x; i++)   /* clears up r0 and mass */
      for (j = 0; j < npoints_y; j++)
         for (k = 0; k < npoints_z; k++)
         {
            ro[i][j][k] = 0.0;
            mass[i][j][k] = 0.0;
            si[i][j][k] = 0.0;
            saxs_excl_vol[i][j][k] = 0.0;
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
   if ((fptr = us_fopen(log_filename, "w")) == NULL)
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
      x_c = (int) ((p->x) / dx) + (int) (npoints_x / 2);
      y_c = (int) ((p->y) / dx) + (int) (npoints_y / 2);
      z_c = (int) ((p->z) / dx) + (int) (npoints_z / 2);
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
      si[x_c][y_c][z_c] += (float) pp->si;
      saxs_excl_vol[x_c][y_c][z_c] += (float) pp->saxs_excl_vol;
      if (centre_or_cog == 0)   //calculates centre of mass
      {
#if defined(DEBUG)
         puts("in atob 3c_3_cog");
         fflush(stdout);
#endif
         x[x_c][y_c][z_c] += (float) (p->x) * (pp->mass);
         y[x_c][y_c][z_c] += (float) (p->y) * (pp->mass);
         z[x_c][y_c][z_c] += (float) (p->z) * (pp->mass);
      }
      if (centre_or_cog == 2)   //calculates centre of si
      {
#if defined(DEBUG)
         puts("in atob 3c_3_cog");
         fflush(stdout);
#endif
         x[x_c][y_c][z_c] += (float) (p->x) * (pp->si);
         y[x_c][y_c][z_c] += (float) (p->y) * (pp->si);
         z[x_c][y_c][z_c] += (float) (p->z) * (pp->si);
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

   for (i = 0; i < npoints_x; i++)   /* Now the radius of the beads are in r0[][][] */
      for (j = 0; j < npoints_y; j++)
         for (k = 0; k < npoints_z; k++)
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
   printf("npoints %d %d %d\n", (int)npoints_x, (int)npoints_y, (int)npoints_z ); fflush(stdout);
#endif

   for (i = 0; i < npoints_x; i++)   /* clears up r0 */
   {
      us_hydrodyn->lbl_core_progress->setText(QString("Gridding %1 of %2").arg(i+1).arg(npoints_x));
      qApp->processEvents();
      if (us_hydrodyn->stopFlag)
      {
         free_f3tensor(ro, 0, npoints_x, 0, npoints_y, 0, npoints_z);
         free_f3tensor(x, 0, npoints_x, 0, npoints_y, 0, npoints_z);
         free_f3tensor(y, 0, npoints_x, 0, npoints_y, 0, npoints_z);
         free_f3tensor(z, 0, npoints_x, 0, npoints_y, 0, npoints_z);
         free_f3tensor(mass, 0, npoints_x, 0, npoints_y, 0, npoints_z);
         free_f3tensor(si, 0, npoints_x, 0, npoints_y, 0, npoints_z);
         free_f3tensor(saxs_excl_vol, 0, npoints_x, 0, npoints_y, 0, npoints_z);
         return npdb;
      }
      for (j = 0; j < npoints_y; j++)
      {
         for (k = 0; k < npoints_z; k++)
         {
            if (ro[i][j][k] > 0.000001)
            {
               natoms2++;
               prev = current;
               nprev = nthis;
               switch( centre_or_cog )
               {
               case 0:
                  //calculates centre of mass for locating the beads
                  {
                     current->x = (float) (x[i][j][k] / mass[i][j][k]);
                     current->y = (float) (y[i][j][k] / mass[i][j][k]);
                     current->z = (float) (z[i][j][k] / mass[i][j][k]);
                  }
                  break;
               case 1:
                  //puts centres of cells
                  {
                     current->x = dx * i + (float) (dx * npoints_x / 2);
                     current->y = dx * j + (float) (dx * npoints_y / 2);
                     current->z = dx * k + (float) (dx * npoints_z / 2);
                  }
                  break;
               case 2:
                  //puts centres of si
                  {
                     current->x = (float) (x[i][j][k] / si[i][j][k]);
                     current->y = (float) (y[i][j][k] / si[i][j][k]);
                     current->z = (float) (z[i][j][k] / si[i][j][k]);
                  }
                  break;
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
               if ( centre_or_cog == 0 ||
                    centre_or_cog == 2 ||
                    set_bead_radius == 0 )
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
               nthis->si   = (float) si[i][j][k];   /* si */
               nthis->saxs_excl_vol = (float) saxs_excl_vol[i][j][k];   /* atomic mass */
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
   }
   prev->next = NULL;
   nprev->next = NULL;
   us_hydrodyn->lbl_core_progress->setText("");
   qApp->processEvents();

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
   if ((fptr = us_fopen(log_filename, "w")) == NULL)
   {
      fprintf(stderr, "\n No se puede abrir archivo para escribir!!");
      exit(1);
   }
#if defined( DEBUG_ATOB )
   printf(">>Writing down correspondance file: %s\n", log_filename);
#endif

   for (n = 1, i = 0; i < npoints_x; i++) 
   {
      us_hydrodyn->lbl_core_progress->setText(QString("Gridding %1 of %2").arg(i+1).arg(npoints_x));
      qApp->processEvents();
      for (j = 0; j < npoints_y; j++)
      {
         for (k = 0; k < npoints_z; k++)
         {
            if (mass[i][j][k] > 0)   //gets only if there is a bead
            {
               fprintf(fptr, "\n\nBead %d: ", n);

               for (p = pdb, nn = 0; p; p = p->next, nn++)   // goes over all atoms
                  if (((int) ((p->x) / dx) + (int) (npoints_x / 2)) == i)
                     if (((int) ((p->y) / dx) + (int) (npoints_y / 2)) == j)
                        if (((int) ((p->z) / dx) + (int) (npoints_z / 2)) == k)
                        {
                           fprintf(fptr, "%d-", nn);
                        }
               n++;
            }
         }
      }
   }

   fclose(fptr);

   nmr_created = false;
   if ( create_nmr )
   {
      QFile fn_out( fn );
                  
      if ( !fn_out.open( QIODevice::WriteOnly ) )
      {
         QMessageBox::warning( us_hydrodyn, "US-SOMO: A2B: Create mulitple model output file",
                               QString( "Could not open %1 for writing!" ).arg( fn ) );
      } else {
                  
         QTextStream tso( &fn_out );

         tso << QString( "REMARK bead model correspondence file\n" );

         unsigned int model  = 0;

         for (n = 1, i = 0; i < npoints_x; i++) 
         {
            us_hydrodyn->lbl_core_progress->setText( QString( "Creating NMR style output file %1 of %2" ).arg( i ).arg( npoints_x ) );
            qApp->processEvents();
            for (j = 0; j < npoints_y; j++)
            {
               for (k = 0; k < npoints_z; k++)
               {
                  if (mass[i][j][k] > 0)   //gets only if there is a bead
                  {
                     // fprintf(fptr, "\n\nBead %d: ", n);
                  
                     unsigned int atomno = 0;
                     tso << QString( "MODEL     %1\n" ).arg( ++model );

                     for (p = pdb, nn = 0; p; p = p->next, nn++)
                     {
                        // goes over all atoms
                        if (((int) ((p->x) / dx) + (int) (npoints_x / 2)) == i)
                        {
                           if (((int) ((p->y) / dx) + (int) (npoints_y / 2)) == j) 
                           {
                              if (((int) ((p->z) / dx) + (int) (npoints_z / 2)) == k)
                              {
                                 tso << 
                                    QString("")
                                    .sprintf(     
                                             "ATOM  %5d%5s%4s %1s%4d    %8.3f%8.3f%8.3f%6.2f%6.2f          %2s\n",
                                             ++atomno,
                                             p->atnam,
                                             p->resnam,
                                             p->chain,
                                             p->resnum,
                                             p->x,
                                             p->y,
                                             p->z,
                                             p->occ,
                                             p->bval,
                                             "  "
                                             );
                              }
                           }
                        }
                     }
                     n++;
                     tso << "TER\nENDMDL\n";
                  }
               }
            }
                                                     
         }
         tso << "END\n";
         fn_out.close();
         nmr_created = true;
      }
   }

   us_hydrodyn->lbl_core_progress->setText("");

   /*Freeing electron density memory! */
   free_f3tensor(ro, 0, npoints_x, 0, npoints_y, 0, npoints_z);
   free_f3tensor(x, 0, npoints_x, 0, npoints_y, 0, npoints_z);
   free_f3tensor(y, 0, npoints_x, 0, npoints_y, 0, npoints_z);
   free_f3tensor(z, 0, npoints_x, 0, npoints_y, 0, npoints_z);
   free_f3tensor(mass, 0, npoints_x, 0, npoints_y, 0, npoints_z);
   free_f3tensor(si, 0, npoints_x, 0, npoints_y, 0, npoints_z);
   free_f3tensor(saxs_excl_vol, 0, npoints_x, 0, npoints_y, 0, npoints_z);
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
   double pre_mw = 0e0;
   double post_mw = 0e0;
   int pre_mw_c = 0;
   int post_mw_c = 0;
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
         tmp_pdb.resnum = (*bead_model)[i].resSeq.toInt();

         strcpy(tmp_pdb.junk, "ATOM");
#if defined(DEBUG)
         puts("grid_atob 1 b"); fflush(stdout);
#endif
         strncpy(tmp_pdb.atnam, (*bead_model)[i].name.toLatin1().data(), 7);
         tmp_pdb.atnam[7] = 0;

         strncpy(tmp_pdb.resnam, (*bead_model)[i].resName.toLatin1().data(), 7);
         tmp_pdb.resnam[7] = 0;

         strncpy(tmp_pdb.insert, (*bead_model)[i].iCode.toLatin1().data(), 7);
         tmp_pdb.insert[7] = 0;

         strncpy(tmp_pdb.chain, (*bead_model)[i].chainID.toLatin1().data(), 7);
         tmp_pdb.chain[7] = 0;
         pdb.push_back(tmp_pdb);

         PHYSPROP tmp_prop;

         tmp_prop.f             = (*bead_model)[i].bead_color;
         tmp_prop.rVW           = (*bead_model)[i].bead_computed_radius;
         tmp_prop.mass          = (*bead_model)[i].bead_mw + (*bead_model)[i].bead_ionized_mw_delta;
         tmp_prop.mass          = ((int)(tmp_prop.mass * 100.0 + .5)) / 100.0;
         tmp_prop.saxs_excl_vol = (*bead_model)[i].saxs_excl_vol;
         tmp_prop.si            = (*bead_model)[i].si;
         // cout << QString( "atom %1 si %2\n" ).arg( (*bead_model)[i].name ).arg( (*bead_model)[i].si );;
         if ( us_hydrodyn->advanced_config.debug_1 )
         {
            pre_mw += ((int)((double)tmp_prop.mass * 100e0 + 5e-1)) / 1e2;
            pre_mw_c++;
         }
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
   for (unsigned int i = 0; i < (unsigned int)pdb.size() - 1; i++)
   {
      pdb[i].next = (PDB *) & pdb[i + 1];
      prop[i].next = (PHYSPROP *) & prop[i + 1];
   }

   float LMAX_x = 0.0;
   float LMAX_y = 0.0;
   float LMAX_z = 0.0;
   float DMAX_x;
   float DMAX_y;
   float DMAX_z;
   float epsilon = 0.0;

   vector < float > DMAX = particle_max_sizes((PDB *)&pdb[0]);
   DMAX_x = (float)(DMAX[1] - DMAX[0]);
   DMAX_y = (float)(DMAX[3] - DMAX[2]);
   DMAX_z = (float)(DMAX[5] - DMAX[4]);


   /*This assures that the spacing left between the box frontere and the closest particle atom
     is larger than three times the standard VW radius, that is the electron density will always
     have space left to expand at least 1.5A away from the particle surface */
   if (EXPANSION_COEF < (float) (max_radius * 3 / DMAX_x))
   {
      epsilon = (float) (max_radius * 3 / DMAX_x);
   }
   else
   {
      epsilon = (float)EXPANSION_COEF;
   }
   LMAX_x = DMAX_x * (1 + epsilon);

   if (EXPANSION_COEF < (float) (max_radius * 3 / DMAX_y))
   {
      epsilon = (float) (max_radius * 3 / DMAX_y);
   }
   else
   {
      epsilon = (float)EXPANSION_COEF;
   }
   LMAX_y = DMAX_y * (1 + epsilon);

   if (EXPANSION_COEF < (float) (max_radius * 3 / DMAX_z))
   {
      epsilon = (float) (max_radius * 3 / DMAX_z);
   }
   else
   {
      epsilon = (float)EXPANSION_COEF;
   }
   LMAX_z = DMAX_z * (1 + epsilon);
   /*max box size is slightly (EXP_COEF*100 %) bigger than particle! */


#if defined(DEBUG_ASA)
   printf("0 - LMAX_x %f DMAX_x %f\n", LMAX_x, DMAX_z); fflush(stdout);
   printf("0 - LMAX_y %f DMAX_y %f\n", LMAX_y, DMAX_y); fflush(stdout);
   printf("0 - LMAX_z %f DMAX_z %f\n", LMAX_z, DMAX_x); fflush(stdout);
#endif

   long npoints_x = (long) (LMAX_x / use_grid_options->cube_side);
   long npoints_y = (long) (LMAX_y / use_grid_options->cube_side);
   long npoints_z = (long) (LMAX_z / use_grid_options->cube_side);
   npoints_x++;
   npoints_y++;
   npoints_z++;

#if defined(DEBUG_ASA)
   printf("npoints_x %ld\n", npoints_x);
   printf("npoints_y %ld\n", npoints_y);
   printf("npoints_z %ld\n", npoints_z);
#endif

#if defined(DEBUG)
   puts("grid_atob 3");
   fflush(stdout);
#endif
   npoints_x *= 2;
   npoints_y *= 2;
   npoints_z *= 2;
   
   // nprop.resize((int)(npoints_x * npoints_y * npoints_z));
   if (!(nprop = (PHYSPROP *) malloc((int) (npoints_x * npoints_y * npoints_z)* sizeof(PHYSPROP)) ))
   {
      QColor save_color = us_hydrodyn->editor->textColor();
      us_hydrodyn->editor->setTextColor("red");
      us_hydrodyn->editor->append("ERROR: Memory allocation failure.  Please try a larger grid cube size and/or close all other applications");
      us_hydrodyn->editor->setTextColor(save_color);
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
   printf(
          "npoints_x %d\n"
          "npoints_y %d\n"
          "npoints_z %d\n"
          "cube side %f\n"
          "max radius %f\n"
          ,(int)npoints_x
          ,(int)npoints_y
          ,(int)npoints_z
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

   QString fn = "";

   bool nmr_created = false;

   if ( use_grid_options->create_nmr_bead_pdb )
   {
      fn =
         us_hydrodyn->somo_dir + QDir::separator() + "structures" + QDir::separator() +
         us_hydrodyn->project + 
         QString("_%1").arg( us_hydrodyn->current_model + 1) +
         QString( us_hydrodyn->bead_model_suffix.length() ? 
                  ("-" + us_hydrodyn->bead_model_suffix) : "") +
         "_grid_atoms.pdb";
      
      //       fn = QFileDialog::getSaveFileName( fn, 
      //                                          "PDB (*.pdb *.PDB)",
      //                                          us_hydrodyn,
      //                                          "save the models",
      //                                          "US_SOMO: A2B: Choose a name to save the multiple model output file" );
      
      if ( QFile::exists( fn ) && 
           !((US_Hydrodyn *)us_hydrodyn)->overwrite )
      {
         fn = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( fn, 0, us_hydrodyn );
      }
   }

   PDB *result_pdb = AtoB((PDB *) & pdb[0],
                          npoints_x,
                          npoints_y,
                          npoints_z,
                          (float) use_grid_options->cube_side,
                          (PHYSPROP *) & prop[0],
                          (PHYSPROP *) & nprop[0],
                          &natoms3,
                          use_grid_options->center,
                          "tmp_atob",
                          use_grid_options->tangency ? 1 : 0,
                          "PB",
                          "RES",
                          us_hydrodyn,
                          use_grid_options->create_nmr_bead_pdb,
                          fn,
                          nmr_created
                          );

   us_hydrodyn->sf_factors.saxs_name = "undefined";
   us_hydrodyn->sf_bead_factors.clear( );

   if ( nmr_created )
   {
      QString error_msg;
      if ( !us_hydrodyn->compute_structure_factors( fn, error_msg ) )
      {
         us_hydrodyn->editor_msg( "red", error_msg );
         us_hydrodyn->editor->verticalScrollBar()->setValue(editor->verticalScrollBar()->maximum());
         us_hydrodyn->raise();
      }
   }

   us_hydrodyn->lbl_core_progress->setText("");
   if ( !result_pdb ) 
   {
      free(nprop);
      QColor save_color = us_hydrodyn->editor->textColor();
      us_hydrodyn->editor->setTextColor("red");
      us_hydrodyn->editor->append("ERROR: Memory allocation failure.  Please try a larger grid cube size and/or close all other applications");
      us_hydrodyn->editor->setTextColor(save_color);
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
      tmp_atom.bead_computed_radius    = this_prop->rVW;
      tmp_atom.bead_actual_radius      = this_prop->rVW;
      tmp_atom.radius                  = this_prop->rVW;
      tmp_atom.bead_mw                 = this_prop->mass;
      tmp_atom.bead_ionized_mw_delta   = 0;
      tmp_atom.si                      = this_prop->si;
      // cout << QString( "bead sum %1 si %2\n" ).arg( result_bead_model.size() )
      // .arg( tmp_atom.si );
      tmp_atom.mw                      = ((int)(this_prop->mass * 100 + .5)) / 100.0;
      if ( us_hydrodyn->advanced_config.debug_1 )
      {
         post_mw += ((int)((double)tmp_atom.mw * 100e0 + 5e-1)) / 1e2;
         post_mw_c++;
      }
      tmp_atom.bead_ref_mw               = this_prop->mass;
      tmp_atom.bead_ref_ionized_mw_delta = 0;
      tmp_atom.saxs_excl_vol             = this_prop->saxs_excl_vol;
      tmp_atom.bead_ref_volume           = 0;
      tmp_atom.bead_color                = 1;
      tmp_atom.serial                    = this_pdb->atnum;
      tmp_atom.exposed_code              = 1;
      tmp_atom.all_beads.clear( );
      tmp_atom.name                      = QString(this_pdb->atnam);
      tmp_atom.resName                   = QString(this_pdb->resnam);
      tmp_atom.iCode                     = QString(this_pdb->insert);
      tmp_atom.chainID                   = QString(this_pdb->chain);
      tmp_atom.chain                     = 1;
      tmp_atom.active                    = 1;
      tmp_atom.normalized_ot_is_valid    = false;
      result_bead_model.push_back(tmp_atom);
#if defined(DEBUG_ATOB)
      printf(
             "after a2b: %d %s %s %s %s %g %g %g %g %g\n"
             ,this_pdb->atnum
             ,this_pdb->atnam
             ,this_pdb->resnam
             ,this_pdb->insert
             ,this_pdb->chain
             ,this_pdb->x
             ,this_pdb->y
             ,this_pdb->z
             ,this_prop->rVW
             ,this_prop->mass
             );
#endif

   }
   free(nprop);
#if defined(DEBUG)
   puts("grid_atob 7");
   fflush(stdout);
#endif
   editor->append(QString("Grid contains %1 beads\n").arg(result_bead_model.size()));
   printf("bead model size %d\n", (int)result_bead_model.size()); fflush(stdout);
   if ( us_hydrodyn->advanced_config.debug_1 )
   {
      printf("grid pre mw %.6f (%d), post mw %.6f (%d), diff %.6f\n", pre_mw, pre_mw_c, post_mw, post_mw_c, pre_mw - post_mw);
   }

   if ( use_grid_options->equalize_radii_constant_volume )
   {
      us_hydrodyn->editor_msg( "red", QString( "Equalizing bead model radii requested, but it is currently DISABLED" ) );
   }
      
   // disabled!
   // if ( use_grid_options->equalize_radii_constant_volume )
   // {
   //    double tot_vol = 0e0;
   //    for ( unsigned int i = 0; i < ( unsigned int ) result_bead_model.size(); i++ )
   //    {
   //       tot_vol += ( 4e0 / 3e0 ) * M_PI * result_bead_model[ i ].radius * result_bead_model[ i ].radius * result_bead_model[ i ].radius;
   //    }
   //    us_hydrodyn->editor_msg( "dark blue", QString( "Equalizing bead model, total volume %1\n" ).arg( tot_vol ) );
   //    double pi43           = M_PI * 4e0 / 3e0;
   //    float radius = (float)pow( tot_vol / ( ( double ) result_bead_model.size() * pi43 ), 1e0 / 3e0 );
   //    for ( unsigned int i = 0; i < ( unsigned int ) result_bead_model.size(); i++ )
   //    {
   //       result_bead_model[ i ].radius               = radius;
   //       result_bead_model[ i ].bead_computed_radius = radius;
   //       result_bead_model[ i ].bead_actual_radius   = radius;
   //    }
   // }

   return result_bead_model;
}

bool US_Hydrodyn::compute_structure_factors( QString filename, 
                                             QString &error_msg )
{
   error_msg = "";
   if ( !QFileInfo( filename ).exists() )
   {
      error_msg = QString( "Error: compute_structure_factors(): file %1 does not exist" ).arg( filename );
      return false;
   }
   if ( !QFileInfo( filename ).isReadable() )
   {
      error_msg = QString( "Error: compute_structure_factors(): file %1 is not readable" ).arg( filename );
      return false;
   }

   // setup & do batch run
   QString csv_addendum =
      QString( "_s%1h%2c%3%4" )
      .arg( grid.cube_side )
      .arg( grid.hydrate ? "y" : "n" )
      .arg( grid.center ? ( grid.center == 2 ? "s" : "c" ) : "m" )
      .arg( grid.equalize_radii_constant_volume ? "er" : "" )
      ;

   csv_addendum.replace( ".", "_" );

   // bool created_batch = false;
   batch_info save_batch_info = batch;
   batch.file.clear( );
   batch.file.push_back( filename );
   if ( !batch_widget )
   {
      // created_batch = true;
      batch_window = new US_Hydrodyn_Batch(&batch, &batch_widget, this);
      fixWinButtons( batch_window );
      batch_window->lb_files->item( 0)->setSelected( true );
   } else {
      batch_window->lb_files->clear( );
      batch_window->lb_files->addItem(batch.file[0]);
      batch_window->lb_files->item( 0)->setSelected( true );
   }
   batch.mm_all = true;
   batch.dmd = false;
   batch.somo = false;
   batch.grid = false;
   batch.iqq = true;
   batch.compute_iq_avg = true;
   batch.csv_saxs = true;
   batch.create_native_saxs = false;
   batch.prr = false;
   batch.hydro = false;
   batch_window->cb_mm_first->setChecked( false );
   batch_window->cb_mm_all  ->setChecked( true );
   batch_window->cb_dmd     ->setChecked( false );
   batch_window->cb_somo    ->setChecked( false );
   batch_window->cb_grid    ->setChecked( false );
   batch_window->cb_iqq     ->setChecked( true );
   batch_window->cb_csv_saxs->setChecked( true );
   batch_window->cb_create_native_saxs->setChecked( false );
   batch_window->cb_prr     ->setChecked( false );
   batch_window->cb_hydro   ->setChecked( false );
   batch_window->le_csv_saxs_name->setText( QFileInfo( filename ).baseName() + csv_addendum );
   batch_window->cb_compute_iq_avg->setChecked( true );
   batch_window->cb_compute_iq_only_avg->setChecked( false );
   batch.hydrate = false;
   if ( batch_window->cb_hydrate )
   {
      batch_window->cb_hydrate->setChecked( false );
   }

   // ok, bc no hydrate allowed in batch
   save_state();
   float save_e_density = saxs_options.water_e_density;
   if ( !saxs_options.bead_models_rho0_in_scat_factors )
   {
      saxs_options.water_e_density = 0.0f;
      if ( save_e_density )
      {
         editor_msg( "dark blue", "Temporarily setting water electron denisty to zero for Iq computation" );
      } else {
         editor_msg( "dark blue", "Water electron denisty is already zero for Iq computation" );
      }
   }
   batch_window->show();
   batch_window->start( true );
   restore_state();
   saxs_options.water_e_density = save_e_density;
   if ( !saxs_options.bead_models_rho0_in_scat_factors )
   {
      editor_msg( "dark blue", QString( "Restored water electron denisty to %1" ).arg( save_e_density ) );
   }
   
   if ( batch_window->stopFlag )
   {
      error_msg = "Error: compute_structure_factors(): batch computation terminated";
      return false;
   }

   cout << "batch finished\n";
   // convert csv average to .dat and "compute structure factors"
   QString csvfile = 
      us_hydrodyn->somo_dir + QDir::separator() +
      "saxs" + QDir::separator() + 
      QFileInfo( filename ).completeBaseName() + 
      csv_addendum +
      "_iqq" +
      batch_window->iqq_suffix() +
      ".csv";
   if ( !QFileInfo( csvfile ).exists() )
   {
      error_msg = QString( "Error: compute_structure_factors(): batch computed csv output file %1 does not exist" ).arg( csvfile );
      return false;
   }
   if ( !QFileInfo( csvfile ).isReadable() )
   {
      error_msg = QString( "Error: compute_structure_factors(): batch computed csv output file %1 is not readable" ).arg( csvfile );
      return false;
   }
   QFile f( csvfile );
   if ( !f.open( QIODevice::ReadOnly ) )
   {
      error_msg = QString( "Error: compute_structure_factors(): batch computed csv output file %1: can not open" ).arg( csvfile );
      return false;
   }
   // read csv file (should only contain average
         
   QTextStream ts( &f );

   QString qsq;
   QStringList qslIs;

   qsq = ts.readLine();

   while ( !ts.atEnd() )
   {
      QString qs = ts.readLine();
      if ( qs.contains( ",\"I(q)\"," ) )
      {
         qslIs << qs;
      }
   }

   f.close();

   QStringList qslq = (qsq ).split( "," , Qt::SkipEmptyParts );
   if ( qslq.size() < 3 ||
        qslq[ 0 ] != "\"Name\"" ||
        qslq[ 1 ] != "\"Type; q:\"" 
        )
   {
      error_msg = QString( "Error: compute_structure_factors(): batch computed csv output file %1: format error in q line" ).arg( csvfile );
      return false;
   }

   if ( qslIs.size() < 2 )
   {
      error_msg = QString( "Error: compute_structure_factors(): batch computed csv output file %1: insufficinet Iq lines (only found %2)" ).arg( csvfile ).arg( qslIs.size() );
      return false;
   }

   qslq.pop_front();
   qslq.pop_front();
   qslq.pop_back();
   qslq.pop_back();

   vector < double > q( qslq.size() );

   for ( unsigned int i = 0; i < (unsigned int)qslq.size(); i++ )
   {
      q[ i ] = qslq[ i ].toDouble();
   }

   vector < vector < double > > Is( qslIs.size() );

   // first beads, last average
   for ( unsigned int i = 0; i < ( unsigned int ) qslIs.size(); i++ )
   {
      QStringList qslI = (qslIs[ i ] ).split( "," , Qt::SkipEmptyParts );

      if ( qslI.size() < 3 || qslI[ 1 ] != "\"I(q)\"" )
      {
         error_msg = QString( "Error: compute_structure_factors(): batch computed csv output file %1: format error in Iq line %2" ).arg( csvfile ).arg( i + 1 );
         return false;
      }

      qslI.pop_front();
      qslI.pop_front();
      qslI.pop_back();

      if ( qslq.size() != qslI.size() )
      {
         error_msg = QString( "Error: compute_structure_factors(): batch computed csv output file %1: format error (I & q lines incompatible) Iq line %2" ).arg( csvfile ).arg( i + 1 );
         return false;
      }

      vector < double > I( qslI.size() );

      for ( unsigned int ii = 0; ii < (unsigned int) qslI.size(); ii++ )
      {
         I[ ii ] = qslI[ ii ].toDouble();
      }
      Is[ i ] = I;
   }

   editor_msg( "blue", us_tr( "Computing 4 & 5 term exponentials\nThis can take awhile & the program may seem unresponsive" ) );
   editor->verticalScrollBar()->setValue(editor->verticalScrollBar()->maximum());
   raise();
   qApp->processEvents();

   unsigned last_bead = ( unsigned int ) Is.size() - 1;
   cout << QString( "last bead is %1\n" ).arg( last_bead );

   for ( unsigned int j = 0; j < ( unsigned int ) Is.size(); j++ )
   {
      editor_msg( "blue", QString( us_tr( "Computing 4 & 5 term exponentials for bead %1" ) ).arg( j == last_bead ? "Global average" : QString( "%1" ).arg( j + 1 ) ) );
      editor->verticalScrollBar()->setValue(editor->verticalScrollBar()->maximum());
      qApp->processEvents();
      
      vector < double > coeff4;
      vector < double > coeff5;
      vector < double > coeffv;
      double            norm4;
      double            norm5;
      double            normv;
      double            nnorm4;
      double            nnorm5;
      US_Saxs_Util usu;

      if ( !usu.compute_exponential( q,
                                     Is[ j ], 
                                     coeff4,
                                     coeff5,
                                     coeffv,
                                     norm4,
                                     norm5,
                                     normv,
                                     saxs_options.bead_models_use_var_len_sf ?
                                     saxs_options.bead_models_var_len_sf_max : 5,
                                     saxs_options.bead_models_use_gsm_fitting,
                                     saxs_options.bead_models_use_quick_fitting
                                     ) )
      {
         error_msg = QString( "Error: compute_structure_factors(): %1" ).arg( usu.errormsg );
         return false;
      } 

      nnorm4 = norm4 / q.size();
      nnorm5 = norm5 / q.size();

      QString norm4tag = "unacceptable";
      if ( nnorm4 < 1000e0 )
      {
         norm4tag = "bad";
      }
      if ( nnorm4 < 500e0 )
      {
         norm4tag = "not so good";
      }
      if ( nnorm4 < 250e0 )
      {
         norm4tag = "ok";
      }
      if ( nnorm4 < 10e0 )
      {
         norm4tag = "good";
      }
      if ( nnorm4 < 1e0 )
      {
         norm4tag = "very good";
      }
      if ( nnorm4 < 1e-2 )
      {
         norm4tag = "excellent";
      }
      if ( nnorm4 < 1e-4 )
      {
         norm4tag = "amazingly excellent";
      }

      QString norm5tag = "unacceptable";
      if ( nnorm5 < 1000e0 )
      {
         norm5tag = "bad";
      }
      if ( nnorm5 < 500e0 )
      {
         norm5tag = "not so good";
      }
      if ( nnorm5 < 250e0 )
      {
         norm5tag = "ok";
      }
      if ( nnorm5 < 10e0 )
      {
         norm5tag = "good";
      }
      if ( nnorm5 < 1e0 )
      {
         norm5tag = "very good";
      }
      if ( nnorm5 < 1e-2 )
      {
         norm5tag = "excellent";
      }
      if ( nnorm5 < 1e-4 )
      {
         norm5tag = "amazingly excellent";
      }

      sf_factors.saxs_name = QFileInfo( csvfile ).baseName().toUpper();
      sf_4term_notes = norm4tag;
      sf_5term_notes = norm5tag;

      editor_msg( "dark blue", QString( "structure factors name: %1\n" ).arg( sf_factors.saxs_name ) );

      QString qs4 = sf_factors.saxs_name;
      for ( unsigned int i = 1; i < ( unsigned int ) coeff4.size(); i++ )
      {
         qs4 += QString( " %1" ).arg( coeff4[ i ] );
      }
      qs4 += QString( " %1" ).arg( coeff4[ 0 ] );
      cout << "Terms4: " << qs4 << endl;
      editor_msg( "dark blue", QString( "Exponentials 4 terms norm %1 nnorm %2 (%3): %4" )
                  .arg( norm4 )
                  .arg( nnorm4 )
                  .arg( norm4tag )
                  .arg( qs4 ) );

      QString qs5 = sf_factors.saxs_name;
      for ( unsigned int i = 1; i < ( unsigned int ) coeff5.size(); i++ )
      {
         qs5 += QString( " %1" ).arg( coeff5[ i ] );
      }
      qs5 += QString( " %1" ).arg( coeff5[ 0 ] );
      cout << "Terms5: " << qs5 << endl;
      editor_msg( "dark blue", QString( "Exponentials 5 terms norm %1 nnorm %2 (%3): %4" )
                  .arg( norm5 )
                  .arg( nnorm5 )
                  .arg( norm5tag )
                  .arg( qs5 ) );

      editor->setTextColor( QColor( "black" ) );
      for ( unsigned int i = 0; i < 4; i++ )
      {
         sf_factors.a[ i ] = coeff4[ 1 + i * 2 ];
         sf_factors.b[ i ] = coeff4[ 2 + i * 2 ];
      }
      sf_factors.c = coeff4[ 0 ];

      for ( unsigned int i = 0; i < 5; i++ )
      {
         sf_factors.a5[ i ] = coeff5[ 1 + i * 2 ];
         sf_factors.b5[ i ] = coeff5[ 2 + i * 2 ];
      }
      sf_factors.c5 = coeff5[ 0 ];

      sf_factors.vcoeff = coeffv;

      if ( j != last_bead )
      {
         sf_factors.saxs_name = QString( "%1" ).arg( j + 1 );
         sf_bead_factors.push_back( sf_factors );
         sf_factors.saxs_name = "undefined";
      }

      // structure factors global are the same if only one bead is used
      if ( last_bead == 1 )
      {
         break;
      }
   }

   return true;
}
