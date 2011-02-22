#ifndef US_GLOBEQDATA_H
#define US_GLOBEQDATA_H

#include <QtCore>
#include "us_extern.h"

// Global Equilibrium Data structure type definitions

typedef struct ScanEdit_s
{
   int    dsindex;    // Data set index of this scan
   int    speedx;     // Speed index within data set
   int    scannbr;    // Scan number within data set
   double rad_lo;     // Low radius value after edit
   double rad_hi;     // High radius value after edit
   bool   edited;     // Flag:  has scan been edited?
} ScanEdit;

typedef struct EqRunFit_s
{
   QVector< double >   mw_vals;       // Molecular Weight values
   QVector< int >      mw_ndxs;       // MW indexes
   QVector< double >   mw_rngs;       // MW ranges
   QVector< bool >     mw_fits;       // MW fitted flags
   QVector< bool >     mw_bnds;       // MW bound flags
   QVector< double >   vbar_vals;     // Vbar-20 values
   QVector< int >      vbar_ndxs;     // Vbar indexes
   QVector< double >   vbar_rngs;     // Vbar ranges
   QVector< bool >     vbar_fits;     // Vbar fitted flags
   QVector< bool >     vbar_bnds;     // Vbar bound flags
   QVector< double >   viri_vals;     // Virial coefficient values
   QVector< int >      viri_ndxs;     // Virial indexes
   QVector< double >   viri_rngs;     // Virial ranges
   QVector< bool >     viri_fits;     // Virial fitted flags
   QVector< bool >     viri_bnds;     // Virial bound flags
   double              eq_vals[ 4 ];  // Equilibrium constant values 
   int                 eq_ndxs[ 4 ];  // Equil indexes 
   double              eq_rngs[ 4 ];  // Equil ranges 
   bool                eq_fits[ 4 ];  // Equil fitted flags
   bool                eq_bnds[ 4 ];  // Equil bound flags
   double              stoichs[ 4 ];  // Stoichiometries
   int                 nbr_comps;     // Number of components
   int                 nbr_assocs;    // Number of association constants
   int                 nbr_runs;      // Number of runs
   double              runs_percent;  // Runs percent
   double              runs_expect;   // Runs expected
   double              runs_vari;     // Runs variance
   double              bottom_pos;    // Bottom CP position for calc_bottom
   double              rcoeffs[ 2 ];  // Rotor coefficients for calc_bottom
   QString             projname;      // Project name
   QString             modlname;      // Model description
} EqRunFit;

typedef struct EqScanFit_s
{
   bool                scanFit;       // Scan-fitted flag
   bool                autoExcl;      // Auto-excluded flag
   bool                limsModd;      // Limits-modified flag
   int                 points;        // Number of points
   int                 nbr_posr;      // Number of positive residuals
   int                 nbr_negr;      // Number of negative residuals
   int                 runs;          // Number of runs
   int                 start_ndx;     // Start index
   int                 stop_ndx;      // Stop index
   QVector< double >   xvs;           // X values
   QVector< double >   yvs;           // Y values
   QVector< double >   amp_vals;      // Amplitude values
   QVector< int >      amp_ndxs;      // Amplitude indexes
   QVector< double >   amp_rngs;      // Amplitude ranges
   QVector< bool >     amp_fits;      // Amplitude fitted flags
   QVector< bool >     amp_bnds;      // Amplitude bound flags
   QVector< double >   extincts;      // Extinctions
   QVector< double >   integral;      // Integrals
   double              density;       // Density
   double              viscosity;     // Viscosity
   double              tempera;       // Temperature
   double              pathlen;       // Path length
   double              meniscus;      // Meniscus
   double              bottom;        // Bottom
   double              baseline;      // Baseline
   int                 baseln_ndx;    // Baseline index
   double              baseln_rng;    // Baseline range
   bool                baseln_fit;    // Baseline fitted flag
   bool                baseln_bnd;    // Baseline bound flag
   int                 rpm;           // Speed in revolutions per minute
   int                 cell;          // Cell
   int                 channel;       // Channel index
   int                 wavelen;       // Wavelength
   int                 rotor;         // Rotor index
   int                 centerp;       // Centerpiece index
   QString             runID;         // Run identifier string
   QString             descript;      // Scan description
} EqScanFit;

typedef struct FitCtrlPar_s
{
   int                 nlsmeth;       // NLS method index
   int                 modelx;        // model type index
   int                 mxiters;       // Maximum iterations
   int                 ntpts;         // Number of total points
   int                 ndsets;        // Number of data sets (scans)
   int                 nfpars;        // Number of fit parameters
   int                 mxsteps;       // Maximum fit steps
   int                 k_iter;        // Current Iteration count
   int                 k_step;        // Current fit step count
   int                 nfuncev;       // Function evaluations
   int                 ndecomps;      // Decompositions
   int                 status;        // Fit iterations status flag
   double              fittoler;      // Fit Tolerance
   double              lambda;        // Current Lambda
   double              lam_start;     // Lambda start
   double              lam_step;      // Lambda step size
   double              variance;      // Iteration variance
   double              std_dev;       // Iteration standard deviation
   double              improve;       // Iteration improvement
   bool                lincnstr;      // Linear constraints flag
   bool                autocnvg;      // Autoconverge flag
   bool                aborted;       // Return aborted flag
   bool                converged;     // Return converged flag
   bool                completed;     // Return iterations completed flag
   QString             emsgdiag;      // Error message for dialog
   QString             statmsg;       // Status message
   QString             infomsg;       // Information message
   int*                setpts;        // Set points array
   int*                setlpts;       // Set log points array
   double*             y_raw;         // Y raw values array
   double*             y_guess;       // Y guesses array
   double*             y_delta;       // Y deltas array
   double*             BB;            // B array
   double*             guess;         // Guesses array
   double*             tguess;        // Test Guesses array
   double**            jacobian;      // Jacobian matrix array
   double**            info;          // Information matrix array
   double**            LLtr;          // LL transpose matrix array
   double**            dcr2;          // dcr2 matrix array
   double**            dlncr2;        // dlncr2 matrix array
   double**            lncr2;         // lncr2 matrix array
} FitCtrlPar;

#endif
