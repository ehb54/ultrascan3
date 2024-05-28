//! \file us_eqmath.h
//! \brief Contains the declaration of the US_EqMath class and its members.
#ifndef US_EQMATH_H
#define US_EQMATH_H

#include <QtCore>

#include "us_extern.h"
#include "us_dataIO.h"
#include "us_matrix.h"
#include "us_math2.h"
#include "us_globeq_data.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

//! \class US_EqMath
//! \brief A class for performing equilibrium mathematical calculations in UltraScan.
class US_EqMath : public QObject
{
    Q_OBJECT

    public:
        //! \brief Constructor for the US_EqMath class.
        //! \param data_list Reference to the vector of edited data.
        //! \param scan_edits Reference to the vector of scan edits.
        //! \param scan_fits Reference to the vector of scan fits.
        //! \param run_fit Reference to the run fit parameters structure.
        US_EqMath(QVector< US_DataIO::EditedData >& data_list,
                  QVector< ScanEdit >& scan_edits, QVector< EqScanFit >& scan_fits, EqRunFit& run_fit);

        //! \brief Initialize parameters for the fit.
        //! \param modelx Model type index.
        //! \param isFloat Boolean indicating if the fit is float.
        //! \param guess_vals List of guess values.
        //! \param upper_bounds List of upper bounds.
        void init_params(int modelx, bool isFloat, QList< double >& guess_vals, QList< double >& upper_bounds);

        //! \brief Initialize the fit.
        //! \param modelx Model type index.
        //! \param nlsmeth NLS method index.
        //! \param fit_params Reference to the fit control parameters.
        void init_fit(int modelx, int nlsmeth, FitCtrlPar& fit_params);

        //! \brief Calculate the Jacobian matrix.
        //! \return The number of Jacobian elements.
        int calc_jacobian(void);

        //! \brief Calculate the test parameter.
        //! \param parameter The test parameter value.
        //! \return The calculated test parameter.
        double calc_testParameter(double parameter);

        //! \brief Perform a line search.
        //! \return The line search result.
        double linesearch(void);

        //! \brief Calculate the B matrix.
        void calc_B(void);

        //! \brief Calculate the residuals.
        //! \return The residuals value.
        double calc_residuals(void);

        //! \brief Calculate the model.
        //! \param guess Pointer to the guess parameters array.
        //! \return The number of model elements.
        int calc_model(double* guess);

        //! \brief Generate the least squares order 2.
        //! \param jacobi Matrix of Jacobian values.
        //! \param npoints Number of points.
        //! \param info Matrix of information values.
        //! \param LLtr Matrix of LL transpose values.
        void genLeastSquaresOrd2(double** jacobi, int npoints, double* info, double** LLtr);

        //! \brief Perform forward mapping of guesses.
        //! \param guess Pointer to the guess parameters array.
        void guess_mapForward(double* guess);

        //! \brief Perform backward mapping of parameters.
        //! \param parameters Pointer to the parameters array.
        void parameter_mapBackward(double* parameters);

        //! \brief Calculate the runs.
        void calc_runs(void);

        //! \brief Calculate the integral.
        void calc_integral(void);

    private:
        QVector< US_DataIO::EditedData >&   dataList;  //!< Reference to the vector of edited data.
        QVector< ScanEdit >&                scedits;   //!< Reference to the vector of scan edits.
        QVector< EqScanFit >&               scanfits;  //!< Reference to the vector of scan fits.
        EqRunFit&                           runfit;    //!< Reference to the run fit parameters structure.

        QVector< int >      v_setpts;   //!< Vector of set points.
        QVector< int >      v_setlpts;  //!< Vector of set log points.

        QVector< double >   v_yraw;     //!< Vector of raw Y values.
        QVector< double >   v_yguess;   //!< Vector of guess Y values.
        QVector< double >   v_ydelta;   //!< Vector of delta Y values.
        QVector< double >   v_BB;       //!< Vector of B values.
        QVector< double >   v_guess;    //!< Vector of guess values.
        QVector< double >   v_tguess;   //!< Vector of test guess values.
        QVector< double >   v_jacobi;   //!< Vector of Jacobian matrix values.
        QVector< double >   v_info;     //!< Vector of information matrix values.
        QVector< double >   v_LLtrns;   //!< Vector of LL transpose matrix values.
        QVector< double >   v_dcr2;     //!< Vector of dcr2 matrix values.
        QVector< double >   v_dlncr2;   //!< Vector of dlncr2 matrix values.
        QVector< double >   v_lncr2;    //!< Vector of lncr2 matrix values.

        QVector< double* >  m_jacobi;   //!< Matrix of Jacobian values.
        QVector< double* >  m_info;     //!< Matrix of information values.
        QVector< double* >  m_LLtrns;   //!< Matrix of LL transpose values.
        QVector< double* >  m_dcr2;     //!< Matrix of dcr2 values.
        QVector< double* >  m_dlncr2;   //!< Matrix of dlncr2 values.
        QVector< double* >  m_lncr2;    //!< Matrix of lncr2 values.

        double*             d_jacobi;   //!< Data array of Jacobian values.
        double*             d_info;     //!< Data array of information values.
        double*             d_LLtrns;   //!< Data array of LL transpose values.
        double*             d_dcr2;     //!< Data array of dcr2 values.
        double*             d_dlncr2;   //!< Data array of dlncr2 values.
        double*             d_lncr2;    //!< Data array of lncr2 values.

        int*                setpts;     //!< Array of set points.
        int*                setlpts;    //!< Array of set log points.
        double*             y_raw;      //!< Array of raw Y values.
        double*             y_guess;    //!< Array of guess Y values.
        double*             y_delta;    //!< Array of delta Y values.
        double*             BB;         //!< Array of B values.
        double*             guess;      //!< Array of guess values.
        double*             tguess;     //!< Array of test guess values.
        double**            jacobian;   //!< Array of Jacobian matrix values.
        double**            info;       //!< Array of information matrix values.
        double**            LLtr;       //!< Array of LL transpose matrix values.
        double**            dcr2;       //!< Array of dcr2 matrix values.
        double**            dlncr2;     //!< Array of dlncr2 matrix values.
        double**            lncr2;      //!< Array of lncr2 matrix values.

        int      dbg_level;    //!< Debug level.
        int      modelx;       //!< Model type index.
        int      nlsmeth;      //!< NLS method index.
        int      ntpts;        //!< Number of total points.
        int      ndsets;       //!< Number of data sets.
        int      nfpars;       //!< Number of fit parameters.
        int      ffitx;        //!< First fit scan index.
        int      nspts;        //!< Number of set points.
        int      nslpts;       //!< Number of set log points.

    private slots:
        //! \brief Perform Cholesky decomposition of order 2.
        //! \param matrix The matrix to decompose.
        //! \return True if successful, false otherwise.
        bool Cholesky_DecompOrd2(double** matrix);

        //! \brief Solve the system using Cholesky decomposition of order 2.
        //! \param matrix The matrix to solve.
        //! \param vector The vector to solve.
        //! \return True if successful, false otherwise.
        bool Cholesky_SolveSysOrd2(double** matrix, double* vector);

        //! \brief Check if a value is NaN.
        //! \param value The value to check.
        //! \return True if the value is NaN, false otherwise.
        bool isNan(double value);

        //! \brief Calculate the bottom value.
        //! \param value The input value.
        //! \return The calculated bottom value.
        double calc_bottom(double value);
};

#endif
