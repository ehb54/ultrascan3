//! \file us_globeq_data.h
//! \brief Contains the definition of data structures used for global equilibrium analysis in UltraScan.
#ifndef US_GLOBEQDATA_H
#define US_GLOBEQDATA_H

#include <QtCore>
#include "us_extern.h"

//! \struct ScanEdit
//! \brief A structure to hold information about the editing status of a scan.
typedef struct ScanEdit_s
{
    int    dsindex;    //!< Data set index of this scan
    int    speedx;     //!< Speed index within data set
    int    scannbr;    //!< Scan number within data set
    double rad_lo;     //!< Low radius value after edit
    double rad_hi;     //!< High radius value after edit
    bool   edited;     //!< Flag: has the scan been edited?
} ScanEdit;

//! \struct EqRunFit
//! \brief A structure to hold run fitting parameters and results for equilibrium analysis.
typedef struct EqRunFit_s
{
    QVector< double >   mw_vals;       //!< Molecular Weight values
    QVector< int >      mw_ndxs;       //!< Molecular Weight indexes
    QVector< double >   mw_rngs;       //!< Molecular Weight ranges
    QVector< bool >     mw_fits;       //!< Molecular Weight fitted flags
    QVector< bool >     mw_bnds;       //!< Molecular Weight bound flags
    QVector< double >   vbar_vals;     //!< Vbar-20 values
    QVector< int >      vbar_ndxs;     //!< Vbar indexes
    QVector< double >   vbar_rngs;     //!< Vbar ranges
    QVector< bool >     vbar_fits;     //!< Vbar fitted flags
    QVector< bool >     vbar_bnds;     //!< Vbar bound flags
    QVector< double >   viri_vals;     //!< Virial coefficient values
    QVector< int >      viri_ndxs;     //!< Virial coefficient indexes
    QVector< double >   viri_rngs;     //!< Virial coefficient ranges
    QVector< bool >     viri_fits;     //!< Virial coefficient fitted flags
    QVector< bool >     viri_bnds;     //!< Virial coefficient bound flags
    double              eq_vals[ 4 ];  //!< Equilibrium constant values
    int                 eq_ndxs[ 4 ];  //!< Equilibrium constant indexes
    double              eq_rngs[ 4 ];  //!< Equilibrium constant ranges
    bool                eq_fits[ 4 ];  //!< Equilibrium constant fitted flags
    bool                eq_bnds[ 4 ];  //!< Equilibrium constant bound flags
    double              stoichs[ 4 ];  //!< Stoichiometries
    int                 dbdisk;        //!< DB/Disk flag
    int                 nbr_comps;     //!< Number of components
    int                 nbr_assocs;    //!< Number of association constants
    int                 nbr_runs;      //!< Number of runs
    double              runs_percent;  //!< Runs percent
    double              runs_expect;   //!< Runs expected
    double              runs_vari;     //!< Runs variance
    double              bottom_pos;    //!< Bottom CP position for calc_bottom
    double              rcoeffs[ 2 ];  //!< Rotor coefficients for calc_bottom
    QString             projname;      //!< Project name
    QString             modlname;      //!< Model description
} EqRunFit;

//! \struct EqScanFit
//! \brief A structure to hold scan fitting parameters and results for equilibrium analysis.
typedef struct EqScanFit_s
{
    bool                scanFit;       //!< Scan-fitted flag
    bool                autoExcl;      //!< Auto-excluded flag
    bool                limsModd;      //!< Limits-modified flag
    int                 points;        //!< Number of points
    int                 nbr_posr;      //!< Number of positive residuals
    int                 nbr_negr;      //!< Number of negative residuals
    int                 runs;          //!< Number of runs
    int                 start_ndx;     //!< Start index
    int                 stop_ndx;      //!< Stop index
    QVector< double >   xvs;           //!< X values
    QVector< double >   yvs;           //!< Y values
    QVector< double >   amp_vals;      //!< Amplitude values
    QVector< int >      amp_ndxs;      //!< Amplitude indexes
    QVector< double >   amp_rngs;      //!< Amplitude ranges
    QVector< bool >     amp_fits;      //!< Amplitude fitted flags
    QVector< bool >     amp_bnds;      //!< Amplitude bound flags
    QVector< double >   extincts;      //!< Extinction coefficients
    QVector< double >   integral;      //!< Integrals
    double              density;       //!< Density
    double              viscosity;     //!< Viscosity
    double              tempera;       //!< Temperature
    double              pathlen;       //!< Path length
    double              meniscus;      //!< Meniscus position
    double              bottom;        //!< Bottom position
    double              baseline;      //!< Baseline value
    int                 baseln_ndx;    //!< Baseline index
    double              baseln_rng;    //!< Baseline range
    bool                baseln_fit;    //!< Baseline fitted flag
    bool                baseln_bnd;    //!< Baseline bound flag
    int                 rpm;           //!< Speed in revolutions per minute
    int                 cell;          //!< Cell number
    int                 channel;       //!< Channel index
    int                 wavelen;       //!< Wavelength
    int                 rotor;         //!< Rotor index
    int                 centerp;       //!< Centerpiece index
    QString             runID;         //!< Run identifier string
    QString             descript;      //!< Scan description
} EqScanFit;

//! \struct FitCtrlPar
//! \brief A structure to hold control parameters for fitting in equilibrium analysis.
typedef struct FitCtrlPar_s
{
    int                 nlsmeth;       //!< NLS method index
    int                 modelx;        //!< Model type index
    int                 mxiters;       //!< Maximum number of iterations
    int                 ntpts;         //!< Number of total points
    int                 ndsets;        //!< Number of data sets (scans)
    int                 nfpars;        //!< Number of fit parameters
    int                 mxsteps;       //!< Maximum number of fit steps
    int                 k_iter;        //!< Current iteration count
    int                 k_step;        //!< Current fit step count
    int                 nfuncev;       //!< Number of function evaluations
    int                 ndecomps;      //!< Number of decompositions
    int                 status;        //!< Fit iterations status flag
    double              fittoler;      //!< Fit tolerance
    double              lambda;        //!< Current lambda value
    double              lam_start;     //!< Lambda start value
    double              lam_step;      //!< Lambda step size
    double              variance;      //!< Iteration variance
    double              std_dev;       //!< Iteration standard deviation
    double              improve;       //!< Iteration improvement
    bool                lincnstr;      //!< Linear constraints flag
    bool                autocnvg;      //!< Autoconverge flag
    bool                aborted;       //!< Return aborted flag
    bool                converged;     //!< Return converged flag
    bool                completed;     //!< Return iterations completed flag
    QString             emsgdiag;      //!< Error message for dialog
    QString             statmsg;       //!< Status message
    QString             infomsg;       //!< Information message
    int*                setpts;        //!< Set points array
    int*                setlpts;       //!< Set log points array
    double*             y_raw;         //!< Y raw values array
    double*             y_guess;       //!< Y guesses array
    double*             y_delta;       //!< Y deltas array
    double*             BB;            //!< B array
    double*             guess;         //!< Guesses array
    double*             tguess;        //!< Test guesses array
    double**            jacobian;      //!< Jacobian matrix array
    double**            info;          //!< Information matrix array
    double**            LLtr;          //!< LL transpose matrix array
    double**            dcr2;          //!< dcr2 matrix array
    double**            dlncr2;        //!< dlncr2 matrix array
    double**            lncr2;         //!< lncr2 matrix array
} FitCtrlPar;

#endif
