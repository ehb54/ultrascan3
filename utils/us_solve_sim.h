//! \file us_solve_sim.h
#ifndef US_SOLVE_SIM_H
#define US_SOLVE_SIM_H

#include <QtCore>
#include <algorithm>

#include "us_model.h"
#include "us_dataIO.h"
#include "us_noise.h"
#include "us_simparms.h"
#include "us_solution.h"
#include "us_solute.h"
#include "us_zsolute.h"
#include "us_astfem_math.h"
#include "us_astfem_rsa.h"

#define SIMPARAMS US_SimulationParameters

//! \brief Solve a simulation, using an experiment data set,
//! a model, and simulation parameters
//!
//! The central method used herein is calc_residuals().

class US_UTIL_EXTERN US_SolveSim : public QObject
{
 Q_OBJECT

  public:

    //! Class containing information for each data set input to calc_residuals()
    class US_UTIL_EXTERN DataSet
    {
        public:
            QString                 requestID;    //!< Request ID for 2DSA/GA
            QString                 auc_file;     //!< Raw AUC file path
            QString                 edit_file;    //!< Edit file path
            QString                 tmst_file;    //!< TimeState file path
            QString                 model_file;   //!< Model file path
            QList< QString >        noise_files;  //!< Path(s) of noise file(s)
            US_DataIO::EditedData   run_data;     //!< Experiment data
            US_Model                model;        //!< Model input and output
            SIMPARAMS               simparams;    //!< Simulation parameters
            US_Solution             solution_rec; //!< Solution record
            double                  viscosity;    //!< Solution buffer viscosity
            double                  density;      //!< Solution buffer density
            double                  compress;     //!< Sol.buff. compressibility
            double                  temperature;  //!< Average run temperature

            double            vbar20;             //!< VBar at 20 degrees C
            double            vbartb;             //!< VBar at temperature
            double            s20w_correction;    //!< s data correction
            double            D20w_correction;    //!< D data correction
            double            rotor_stretch[ 2 ]; //!< Stretch coefficients
            double            centerpiece_bottom; //!< Base bottom
            double            zcoeffs[ 4 ];       //!< Solute Z coefficients

            int               solute_type;        //!< Solute type (0,1,2)
            bool              manual;             //!< visc.,dens. manual
    };

    //! Class for communicating simulation
    class US_UTIL_EXTERN Simulation
    {
      public:

         Simulation();

         double                variance;   //!< Total variance
         double                xnormsq;    //!< X-norm squared
         double                alpha;      //!< Tikhonov regularization factor
         QVector< double >     variances;  //!< Variances for data sets
         QVector< double >     ti_noise;   //!< Time-invariant noise 
         QVector< double >     ri_noise;   //!< Radially-invariant noise
         QVector< US_Solute >  solutes;    //!< Input/Output solutes
         QVector< US_ZSolute > zsolutes;   //!< Input/Output solutes
         long int              maxrss;     //!< Running max rss memory in KB
         int                   noisflag;   //!< Calculated-noise flag: 0-3
         int                   dbg_level;  //!< Debug level
         bool                  dbg_timing; //!< Debug-timing-prints flag
         US_DataIO::RawData    sim_data;   //!< Simulation data
         US_DataIO::RawData    residuals;  //!< Residuals data (run-sim-noi)
    };

    //! Constructor for the SolveSim class
    //!
    //! \param data_sets      The set of data sets for which to solve
    //! \param thrnrank       Thread number or processor rank (1,...)
    //! \param signal_wanted  Flag whether to emit progress signals
    US_SolveSim        ( QList< DataSet* >&, int, bool = false );

    //! \brief Non-negative least squares for concentrations with algebraic
    //!        removal of time-invariant (TI) and/or radially-invariant (RI)
    //!        noise, each data set having its own noise vectors
    //!
    //! For given concentrations, the least-squares TI noise (one value per
    //! radius) and RI noise (one value per scan) of a data set are the mean
    //! residuals over its scans and over its radii.  Eliminating them
    //! projects the data and each simulation onto the complement of the
    //! noise subspace:  the data rows of each data set are centered by their
    //! per-scan means (RI), per-radius means (TI), or both.  The
    //! concentrations are the NNLS solution of the projected system; the
    //! noise vectors follow from the removed means.
    //!
    //! \param noisflag  Noise flag:  1 = TI, 2 = RI, 3 = TI and RI
    //! \param nscans    Number of scans of each data set
    //! \param npoints   Number of radial points of each data set
    //! \param nsolutes  Number of solutes (columns of A)
    //! \param narows    Rows in each A column:  the nscans*npoints data rows
    //!                  (scan-major) of each data set in turn, followed by
    //!                  any Tikhonov rows
    //! \param nnls_a    A matrix, column-major (overwritten)
    //! \param nnls_b    B vector with narows values (overwritten)
    //! \param nnls_x    Output concentrations (nsolutes values)
    //! \param tinvec    Output TI noise (npoints values of each data set in
    //!                  turn), if noisflag has 1
    //! \param rinvec    Output RI noise (nscans values of each data set in
    //!                  turn), if noisflag has 2
    //! \returns         Return code of US_Math2::nnls
    static int nnls_noise( int, const QVector< int >&, const QVector< int >&,
                           int, int,
                           QVector< double >&, QVector< double >&,
                           QVector< double >&, QVector< double >&,
                           QVector< double >& );

  public slots:

    //! \brief Static function to check if implied grid size is beyond limits
    //! \param data_sets The set of data sets for which to check
    //! \param s_max     S-value maximum
    //! \param smsg      Returned size error message (if return=true)
    //! \returns         Flag of size problem existing
    static bool checkGridSize( QList< DataSet* >&, double, QString& );

    //! \brief Check if implied grid size is beyond limits
    //! \param s_max     S-value maximum
    //! \param smsg      Returned size error message (if return=true)
    //! \returns         Flag of size problem existing
    bool check_grid_size( double, QString& );

    //! \brief Calculate a simulation and the resulting residuals
    //!
    //! \param offset         Starting data-sets offset
    //! \param dataset_count  Number of data sets for which to solve
    //! \param sim_vals       Simulation parameters object
    //! \param padAB          Optional flag to pad saved A and B
    //! \param ASave          Optional pointer for saving A matrix
    //! \param BSave          Optional pointer for saving B matrix
    //! \param NSave          Optional pointer for saving norm vector
    void calc_residuals( int, int, Simulation&, bool = false,
                         QVector< double >* = 0, QVector< double >*  = 0,
                         QVector< double >* = 0 );

    //! \brief Set a flag so that the worker aborts at the earliest opportunity
    void abort_work    ( void );

  signals:
    //! \brief emit a signal that includes a progress step count
    void work_progress ( int );

  private:

    enum attr_type { ATTR_S, ATTR_K, ATTR_W, ATTR_V, ATTR_D, ATTR_F };

    QList< DataSet* >& data_sets;     // Data sets for which to solve

    int                thrnrank;      // Thread number or processor rank (1,...)
    bool               signal_wanted; // Flag whether to emit progress signals

    int                d_offs;        // Current data offset
    int                noisflag;      // Calc-noise flag (0-3 for no|ti|ri|both)
    int                dbg_level;     // Debug level
    bool               dbg_timing;    // Flag whether to print timings
    bool               abort;         // Flag to abort at next opportunity
    bool               calc_ti;       // Calculate-TI-noise flag
    bool               calc_ri;       // Calculate-RI-noise flag
    bool               banddthr;      // Band-forming data threshold peak enhance
    QDateTime          startCalc;     // Start calc time for elapsed time prints

  private slots:
    // Limit data to thresholds
    bool data_threshold    ( US_DataIO::RawData*,
                             double, double, double, double );

    // Limit data to thresholds  (experiment data version)
    bool data_threshold    ( US_DataIO::EditedData*,
                             double, double, double, double );

    // Set a model component attribute value
    void set_comp_attr     ( US_Model::SimulationComponent&,
                             US_Solute&, int );

    // Output a debug print of time for a labelled event
    void DebugTime         ( QString );

    double angle_vectors( QVector<double>&, QVector<double>&, int );
    double angle_vectors( double*, double*, int );


};
#endif

