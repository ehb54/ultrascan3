#ifndef US_HYDRODYN_GRID_ATOB_H
#define US_HYDRODYN_GRID_ATOB_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>

typedef float REAL;

typedef struct pdb_entry
{
    REAL x, y, z, occ, bval;
    struct pdb_entry *next;
    int atnum;
    int resnum;
    char junk[8];
    char atnam[8];
    char resnam[8];
    char insert[8];
    char chain[8];
} PDB;

typedef struct physical_properties
{
    REAL f, rVW,		/*electron density */
      bH2O, bD2O;
    REAL mass;
    struct physical_properties *next;

} PHYSPROP;

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
     char resnam[]);
     /*
        ro is initiallized by : ro=f3tensor(0,npoints,0,npoints,0,npoints); 
        input_file used to generate log file
        set_bead_radius= if zero, does nothing, if not it sets the bead radius to half the cube's side
        centre_of_cog= if zero places beads in cubes centres, if 1 if calculates the cog of the beads in the cube and places bead therein
        pdb= PDB * pointer of structure
        dx= cube's side
        npoints = LMAX/dx, LMAX=1.2*DMAX
        // natoms= n atoms in pdb (excluded since not used)
        prop= PHYSPROP structure with radii and masses of beads read from AtomicVW.dat
        nprop= pointer where it puts new pdb PHYSPROP to be used if writing file in output format. has to be allocated in calling routine!
        natoms3= number of atoms in new AtoB model. has to be allocated in calling routine!

      */

#endif
