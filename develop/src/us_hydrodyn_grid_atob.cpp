#include "../include/us_hydrodyn_grid_atob.h"

/*Declaration of new structure types*/

typedef struct kummy_entry
{
    REAL x, y, z, occ, bval;	/* in the case of nucleic acids the CofG of the bases will be at xyz! */
    REAL dx, dy, dz, px, py, pz;	/*accordingly the deoxyribose and OPO group CofG will be here! */
    int natoms;			/*number of atoms in the related atomic linked list */
    struct pdb_entry *xnext;
    struct kummy_entry *knext;
    int atnum;
    int resnum;
    char junk[8];
    char atnam[8];
    char resnam[8];
    char insert[8];
    char chain[8];
} KUMMY;

/* Declaration of prototypes...*/

#define NR_END 1
#define FREE_ARG char*
static void
nrerror(char error_text[])
/* Numerical Recipes standard error handler */
{
    fprintf(stderr, "Numerical Recipes run-time error...\n");
    fprintf(stderr, "%s\n", error_text);
    fprintf(stderr, "...now exiting to system...\n");
    exit(1);
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
	nrerror("allocation failure 1 in f3tensor()");
    t += NR_END;
    t -= nrl;

    /* allocate pointers to rows and set pointers to them */
    t[nrl] = (float **) malloc((size_t) ((nrow * ncol + NR_END) * sizeof(float *)));
    if (!t[nrl])
	nrerror("allocation failure 2 in f3tensor()");
    t[nrl] += NR_END;
    t[nrl] -= ncl;

    /* allocate rows and set pointers to them */
    t[nrl][ncl] = (float *) malloc((size_t) ((nrow * ncol * ndep + NR_END) * sizeof(float)));
    if (!t[nrl][ncl])
	nrerror("allocation failure 3 in f3tensor()");
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

PDB *
AtoB(PDB * pdb,
     long npoints,
     float dx,
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
     /*
        ro is initiallized by : ro=f3tensor(0,
	points,
	,npoints,0,npoints); 
        input_file used to generate log file
        set_bead_radius= if zero, does nothing, if not it sets the bead radius to half the cube's side
        centre_of_cog= if zero places beads in cubes centres, if 1 if calculates the cog of the beads in the cube and places bead therein
        pdb= PDB * pointer of structure
        dx= cube's side
        npoints = LMAX/dx, LMAX=1.2*DMAX
        natoms= n atoms in pdb
        prop= PHYSPROP structure with radii and masses of beads read from AtomicVW.dat
        nprop= pointer where it puts new pdb PHYSPROP to be used if writing file in output format. has to be allocated in calling routine!
        natoms3= number of atoms in new AtoB model. has to be allocated in calling routine!

      */

    PDB *p, *npdb, *temp, *prev, *current;
    PHYSPROP *pp = prop, *nthis, *nprev, *ntemp;
    FILE *fptr;
    // BEAD_CONN ***w,*wthis,*wprev;

    int i, j, k, x_c, y_c, z_c, n, nn;
    long natoms2 = 0;
    char log_filename[50];
    float ***ro, ***x, ***y, ***z;
    float ***mass;

    // w=(BEAD_CONN *)malloc(sizeof(BEAD_CONN));
    // w=(BEAD_CONN ***)malloc(sizeof(BEAD_CONN)*npoints*npoints*npoints);
    ro = (float ***) f3tensor(0, npoints, 0, npoints, 0, npoints);
    mass = (float ***) f3tensor(0, npoints, 0, npoints, 0, npoints);
    x = (float ***) f3tensor(0, npoints, 0, npoints, 0, npoints);
    y = (float ***) f3tensor(0, npoints, 0, npoints, 0, npoints);
    z = (float ***) f3tensor(0, npoints, 0, npoints, 0, npoints);

//  wthis=w;wprev=w;

//calculates radii and mass vs i,j,k

    for (i = 0; i < npoints; i++)	/* clears up r0 and mass */
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

    strcpy(log_filename, input_file);
    strcat(log_filename, ".log");
    if ((fptr = fopen(log_filename, "w")) == NULL)
    {
	fprintf(stderr, "\n No se puede abrir archivo para escribir!!");
	exit(1);
    }

    for (p = pdb, pp = prop, n = 0; p; p = p->next, pp = pp->next, n++)	/* over the atoms  */
    {
	x_c = (int) ((p->x) / dx) + (int) (npoints / 2);
	y_c = (int) ((p->y) / dx) + (int) (npoints / 2);
	z_c = (int) ((p->z) / dx) + (int) (npoints / 2);

	ro[x_c][y_c][z_c] += pow(pp->rVW, 3);	/* = ctte*electron_density*gaussian */
	mass[x_c][y_c][z_c] += (int) pp->mass;
	if (centre_or_cog == 1)	//calculates centre of mass
	{
	    x[x_c][y_c][z_c] += (float) (p->x) * (pp->mass);
	    y[x_c][y_c][z_c] += (float) (p->y) * (pp->mass);
	    z[x_c][y_c][z_c] += (float) (p->z) * (pp->mass);
	}

	fprintf(fptr, "[%d,%d,%d] Bead %d \n", x_c, y_c, z_c, n);

    }
    //wprev->next=NULL;
    fclose(fptr);

    for (i = 0; i < npoints; i++)	/* Now the radius of the beads are in r0[][][] */
	for (j = 0; j < npoints; j++)
	    for (k = 0; k < npoints; k++)
		ro[i][j][k] = pow(ro[i][j][k], 0.333333);

// writes down results in PDB and PHYSPROP linked lists

    npdb = (PDB *) malloc(sizeof(PDB));	//allocates new pdb structure file
    current = npdb;
    prev = npdb;
    nthis = nprop;
    nprev = nprop;

    for (i = 0; i < npoints; i++)	/* clears up r0 */
	for (j = 0; j < npoints; j++)
	    for (k = 0; k < npoints; k++)
	    {
		if (ro[i][j][k] > 0.000001)
		{
		    natoms2++;
		    prev = current;
		    nprev = nthis;
		    if (centre_or_cog == 1)	//calculates centre of mass for locating the beads
		    {
			current->x = (float) (x[i][j][k] / mass[i][j][k]);
			current->y = (float) (y[i][j][k] / mass[i][j][k]);
			current->z = (float) (z[i][j][k] / mass[i][j][k]);

		    }
		    else	//puts centres of cells
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
			nthis->rVW = (float) ro[i][j][k];	/*Van der Walls radii */
		    else
		    {
			if (ro[i][j][k] <= (float) (dx / 2))	//if coord centre of cell, it puts a maximum radius
			    nthis->rVW = (float) ro[i][j][k];	//equal to half the cell's diameter
			else
			{
			    nthis->rVW = (float) (dx / 2);
			    fprintf(stderr, "\n >>>>> changing radius of bead %d!!! >>> \n", (int) natoms2);
			}
		    }

		    nthis->mass = (float) mass[i][j][k];	/* atomic mass */
		    nthis->f = (float) 6;	/* assigns electron density. */
		    temp = (PDB *) malloc(sizeof(PDB));
		    ntemp = (PHYSPROP *) malloc(sizeof(PHYSPROP));

		    current->next = temp;
		    current = temp;
		    nthis->next = ntemp;
		    nthis = ntemp;

		}

	    }
    prev->next = NULL;
    nprev->next = NULL;

    printf("\nNumber of atoms in AtoB model at dx=%.2f A: %d\n", dx, (int) natoms2);

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
		if (mass[i][j][k] > 0)	//gets only if there is a bead
		{
		    fprintf(fptr, "\n\nBead %d: ", n);

		    for (p = pdb, nn = 0; p; p = p->next, nn++)	// goes over all atoms
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

    return (npdb);
}
