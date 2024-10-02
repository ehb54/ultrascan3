//! \file us_fit_worker.h
//! \brief Contains the declaration of the US_FitWorker class and its members.
#ifndef US_EQFWORKER_H
#define US_EQFWORKER_H

#include <QtCore>

#include "us_extern.h"
#include "us_dataIO.h"
#include "us_matrix.h"
#include "us_math2.h"
#include "us_globeq_data.h"
#include "us_eqmath.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

//! \class US_FitWorker
//! \brief A class for performing equilibrium fitting in a separate thread in UltraScan.
class US_FitWorker : public QThread
{
    Q_OBJECT

    public:
        //! \brief Constructor for the US_FitWorker class.
        //! \param eq_math Pointer to the EqMath object.
        //! \param fit_params Reference to the fit control parameters.
        //! \param parent Pointer to the parent object.
        US_FitWorker(US_EqMath* eq_math, FitCtrlPar& fit_params, QObject* parent);

        //! \brief Destructor for the US_FitWorker class.
        ~US_FitWorker();

        //! \brief Redefine the work parameters.
        void redefine_work(void);

        //! \brief Run the thread.
        void run(void) override;

        //! \brief Set the pause flag to true or false.
        //! \param pause Boolean indicating if the pause flag should be set.
        void flag_paused(bool pause);

        //! \brief Set the abort flag to true.
        void flag_abort(void);

    signals:
        //! \brief Signal emitted to indicate work progress.
        //! \param progress The current progress step.
        void work_progress(int progress);

        //! \brief Signal emitted to indicate work completion.
        void work_complete(void);

    private:
        US_EqMath*  emath;     //!< Pointer to the EqMath object.
        FitCtrlPar& fitpars;   //!< Reference to the fit control parameters.

        int         dbg_level; //!< Debug level.
        int         mxiters;   //!< Maximum iterations.
        int         k_iter;    //!< Iteration count.
        int         nlsmeth;   //!< NLS method index.
        int         modelx;    //!< Model type index.
        int         ntpts;     //!< Number of total x,y data points.
        int         ndsets;    //!< Number of data sets (scans).
        int         nfpars;    //!< Number of fit parameters per point.

        double      tolerance; //!< Fit variance tolerance.
        double      variance;  //!< Variance value (sum diffs. squared).
        double      old_vari;  //!< Previous iteration variance.
        double      lambda;    //!< Current lambda value.

        bool        paused;    //!< Flag for pause/resume.
        bool        abort;     //!< Flag for abort.
        bool        aborted;   //!< Flag indicating if fitting was aborted.
        bool        converged; //!< Flag indicating if fitting converged.
        bool        completed; //!< Flag indicating if fitting completed.

    private slots:
        //! \brief Perform the main work method for fit iterations.
        //! \return The result of the fit iterations.
        int fit_iterations(void);

        //! \brief Perform a fit iteration using the Levenberg-Marquardt method.
        //! \return The result of the fit iteration.
        int fit_iter_LM(void);

        //! \brief Perform a fit iteration using the Modified Gauss-Newton method.
        //! \return The result of the fit iteration.
        int fit_iter_MGN(void);

        //! \brief Perform a fit iteration using the Hybrid Method.
        //! \return The result of the fit iteration.
        int fit_iter_HM(void);

        //! \brief Perform a fit iteration using the Quasi-Newton method.
        //! \return The result of the fit iteration.
        int fit_iter_QN(void);

        //! \brief Perform a fit iteration using the Generalized Linear Least Squares method.
        //! \return The result of the fit iteration.
        int fit_iter_GLLS(void);

        //! \brief Perform a fit iteration using the Non-Negative Least Squares method.
        //! \return The result of the fit iteration.
        int fit_iter_NNLS(void);

        //! \brief Check if the thread is paused and handle accordingly.
        void check_paused(void);

        //! \brief Perform a line search.
        //! \param params Pointer to the parameters array.
        //! \param step The step size.
        //! \return The result of the line search.
        double linesearch(double* params, double step);

        //! \brief Calculate the test parameter.
        //! \param params Pointer to the parameters array.
        //! \param value The test parameter value.
        //! \return The calculated test parameter.
        double calc_testParameter(double* params, double value);

        //! \brief Update the Quasi-Newton parameters.
        //! \param params Pointer to the parameters array.
        //! \param deltas Pointer to the deltas array.
        void updateQN(double* params, double* deltas);

        //! \brief Check if a value is NaN.
        //! \param value The value to check.
        //! \return True if the value is NaN, false otherwise.
        bool isNan(double value);
};

#endif
