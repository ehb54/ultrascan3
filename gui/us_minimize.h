//! \file us_minimize.h
#ifndef US_MINIMIZE_H
#define US_MINIMIZE_H

#include "us_settings.h"
#include "us_widgets.h"
#include "us_plot.h"
#include "us_util.h"
#include "us_math2.h"
#include "us_matrix.h"
#include "us_timer.h"

//! \class US_Minimize
//! \brief A class for performing minimization operations with GUI support.
class US_GUI_EXTERN US_Minimize : public US_Widgets
{
    Q_OBJECT

    public:
        /*!
         * \brief Constructor for US_Minimize.
         * \param temp_fitting_widget Pointer to a fitting widget flag.
         * \param temp_GUI Boolean indicating if GUI is used.
         */
        US_Minimize(bool *temp_fitting_widget, bool temp_GUI);

        /*!
         * \brief Destructor for US_Minimize.
         */
        ~US_Minimize();

        QVector<unsigned int> points_per_dataset; /*!< Points per dataset vector. */
        unsigned int parameters; /*!< Number of parameters. */
        unsigned int iteration; /*!< Iteration count. */
        unsigned int function_evaluations; /*!< Function evaluations count. */
        unsigned int decompositions; /*!< Decompositions count. */
        unsigned long int points; /*!< Total points. */
        unsigned long int runs; /*!< Runs count. */
        unsigned long int datasets; /*!< Datasets count. */
        unsigned long int firstScan; /*!< First scan index. */
        unsigned long int maxIterations; /*!< Maximum iterations. */
        unsigned int column1; /*!< Column 1 index. */
        unsigned int column2; /*!< Column 2 index. */
        unsigned int column3; /*!< Column 3 index. */
        unsigned int span; /*!< Span value. */
        unsigned int totalSteps; /*!< Total steps count. */
        int nlsMethod; /*!< Non-linear solver method. */
        int return_value; /*!< Return value. */
        unsigned int plotGroup; /*!< Plot group index. */
        bool us_auto_mode; /*!< Auto mode flag. */
        bool *fitting_widget; /*!< Fitting widget flag. */
        bool plotResiduals; /*!< Plot residuals flag. */
        bool showGuiFit; /*!< Show GUI fit flag. */
        bool GUI; /*!< GUI flag. */
        bool constrained; /*!< Constrained flag. */
        bool autoconverge; /*!< Auto converge flag. */
        bool suspend_flag; /*!< Suspend flag. */
        bool aborted; /*!< Aborted flag. */
        bool converged; /*!< Converged flag. */
        bool completed; /*!< Completed flag. */
        bool first_plot; /*!< First plot flag. */
        bool init_simulation; /*!< Initialize simulation flag. */
        float *y_raw; /*!< Raw y-values. */
        float lambdaStart; /*!< Lambda start value. */
        float lambdaStep; /*!< Lambda step value. */
        float runs_percent; /*!< Runs percentage. */
        float variance; /*!< Variance value. */
        float tolerance; /*!< Tolerance value. */
        double **LL_transpose; /*!< LL transpose matrix. */
        double *B; /*!< B matrix. */
        double *y_guess; /*!< Y guess values. */
        double **jacobian; /*!< Jacobian matrix. */
        double *guess; /*!< Guess values. */
        double *test_guess; /*!< Test guess values. */
        double **information_matrix; /*!< Information matrix. */
        double *y_delta; /*!< Y delta values. */
        int buttonh; /*!< Button height. */

        US_Settings* settings; /*!< Settings object. */
        US_Matrix* matrix; /*!< Matrix object. */

        QLabel *lbl_header; /*!< Header label. */
        QLabel *lbl_iteration; /*!< Iteration label. */
        QLabel *lbl_variance; /*!< Variance label. */
        QLabel *lbl_stddev; /*!< Standard deviation label. */
        QLabel *lbl_improvement; /*!< Improvement label. */
        QLabel *lbl_tolerance; /*!< Tolerance label. */
        QLabel *lbl_evaluations; /*!< Evaluations label. */
        QLabel *lbl_decompositions; /*!< Decompositions label. */
        QLabel *lbl_currentLambda; /*!< Current lambda label. */
        QLabel *lbl_parameters; /*!< Parameters label. */
        QLabel *lbl_datasets; /*!< Datasets label. */
        QLabel *lbl_points; /*!< Points label. */
        QLabel *lbl_constrained; /*!< Constrained label. */
        QLabel *lbl_autoconverge; /*!< Auto converge label. */
        QLabel *lbl_showGuiFit; /*!< Show GUI fit label. */
        QCheckBox *cb_showGuiFit; /*!< Show GUI fit checkbox. */
        QLabel *lbl_progress; /*!< Progress label. */
        QLabel *lbl_nlsMethod; /*!< NLS method label. */
        QLabel *lbl_controls1; /*!< Controls 1 label. */
        QLabel *lbl_controls2; /*!< Controls 2 label. */
        QLabel *lbl_lambdaStart; /*!< Lambda start label. */
        QLabel *lbl_lambdaStep; /*!< Lambda step label. */
        QLabel *lbl_maxIterations; /*!< Max iterations label. */
        QLabel *lbl_global; /*!< Global label. */
        QLineEdit *le_iteration; /*!< Iteration line edit. */
        QLineEdit *le_variance; /*!< Variance line edit. */
        QLineEdit *le_stddev; /*!< Standard deviation line edit. */
        QLineEdit *le_improvement; /*!< Improvement line edit. */
        QLineEdit *le_evaluations; /*!< Evaluations line edit. */
        QLineEdit *le_decompositions; /*!< Decompositions line edit. */
        QLineEdit *le_currentLambda; /*!< Current lambda line edit. */
        QLineEdit *le_parameters; /*!< Parameters line edit. */
        QLineEdit *le_datasets; /*!< Datasets line edit. */
        QLineEdit *le_points; /*!< Points line edit. */
        QLineEdit *le_lambdaStart; /*!< Lambda start line edit. */
        QLineEdit *le_maxIterations; /*!< Max iterations line edit. */
        QLineEdit *le_lambdaStep; /*!< Lambda step line edit. */
        QLineEdit *le_tolerance; /*!< Tolerance line edit. */
        QProgressBar *pgb_progress; /*!< Progress bar. */
        QRadioButton *bt_plotGroup; /*!< Plot group radio button. */
        QRadioButton *bt_plotSingle; /*!< Plot single radio button. */
        QRadioButton *bt_plotAll; /*!< Plot all radio button. */
        QRadioButton *bt_constrained; /*!< Constrained radio button. */
        QRadioButton *bt_unconstrained; /*!< Unconstrained radio button. */
        QRadioButton *bt_autoconverge; /*!< Auto converge radio button. */
        QRadioButton *bt_manualconverge; /*!< Manual converge radio button. */
        QPushButton *pb_fit; /*!< Fit button. */
        QPushButton *pb_pause; /*!< Pause button. */
        QPushButton *pb_resume; /*!< Resume button. */
        QPushButton *pb_close; /*!< Close button. */
        QPushButton *pb_help; /*!< Help button. */
        QPushButton *pb_report; /*!< Report button. */
        QPushButton *pb_overlays; /*!< Overlays button. */
        QPushButton *pb_residuals; /*!< Residuals button. */
        QPushButton *pb_saveFit; /*!< Save fit button. */
        QPushButton *pb_print; /*!< Print button. */
        QPushButton *pb_plottwo; /*!< Plot two button. */
        QPushButton *pb_plotthree; /*!< Plot three button. */
        QPushButton *pb_plotfour; /*!< Plot four button. */
        QPushButton *pb_plotfive; /*!< Plot five button. */

        QComboBox *cbb_nlsMethod; /*!< NLS method combo box. */
        QButtonGroup *bg1; /*!< Button group 1. */
        QButtonGroup *bg2; /*!< Button group 2. */
        QButtonGroup *bg3; /*!< Button group 3. */
        QwtCounter *ct_unlabeled; /*!< Unlabeled counter. */
        QCheckBox *ck_monitor; /*!< Monitor checkbox. */
        QRadioButton *rb_linearN; /*!< Linear N radio button. */
        QRadioButton *rb_linearY; /*!< Linear Y radio button. */
        QRadioButton *rb_autoN; /*!< Auto N radio button. */
        QRadioButton *rb_autoY; /*!< Auto Y radio button. */
        QComboBox *cb_nlsalg; /*!< NLS algorithm combo box. */
        US_Plot *plotLayout; /*!< Plot layout. */
        QwtPlot *data_plot; /*!< Data plot. */

    protected slots:
        /*!
         * \brief Calculate test parameter.
         * \param param1 Parameter 1.
         * \param param2 Parameter 2.
         * \return Calculated test parameter.
         */
        float calc_testParameter(float **param1, float param2);

        /*!
         * \brief Perform line search.
         * \param param1 Parameter 1.
         * \param param2 Parameter 2.
         * \return Line search result.
         */
        float linesearch(float **param1, float param2);

        /*!
         * \brief Calculate B matrix.
         * \return Status of the calculation.
         */
        int calc_B();

        /*!
         * \brief Update QN matrix.
         * \param param1 Parameter 1.
         * \param param2 Parameter 2.
         */
        void updateQN(float **param1, float **param2);

        /*!
         * \brief Handle close event.
         * \param event Close event.
         */
        void closeEvent(QCloseEvent *event);

    public slots:
        /*!
         * \brief Update lambda step.
         * \param text The new lambda step value.
         */
        void update_lambdaStep

                (const QString &text);

        /*!
         * \brief Update lambda start.
         * \param text The new lambda start value.
         */
        void update_lambdaStart(const QString &text);

        /*!
         * \brief Update maximum iterations.
         * \param text The new maximum iterations value.
         */
        void update_maxIterations(const QString &text);

        /*!
         * \brief Update tolerance.
         * \param text The new tolerance value.
         */
        void update_tolerance(const QString &text);

        /*!
         * \brief Update NLS method.
         * \param index The new NLS method index.
         */
        void update_nlsMethod(int index);

        /*!
         * \brief Update fit dialog.
         */
        void update_fitDialog();

        /*!
         * \brief Perform the fitting operation.
         * \return Status of the fitting operation.
         */
        int Fit();

    protected slots:
        /*!
         * \brief Calculate residuals.
         * \return Calculated residuals.
         */
        virtual float calc_residuals();

        /*!
         * \brief Calculate the model.
         * \param param Parameters for the model.
         * \return Status of the model calculation.
         */
        virtual int calc_model(double *param);

        /*!
         * \brief Clean up resources.
         */
        virtual void cleanup();

        /*!
         * \brief Initialize the fitting process.
         * \return True if initialization is successful, otherwise false.
         */
        virtual bool fit_init();

        /*!
         * \brief Plot residuals.
         */
        virtual void plot_residuals();

        /*!
         * \brief Plot overlays.
         */
        virtual void plot_overlays();

        /*!
         * \brief Calculate the Jacobian matrix.
         * \return Status of the Jacobian calculation.
         */
        virtual int calc_jacobian();

        /*!
         * \brief End the fitting process.
         */
        virtual void endFit();

    private slots:
        /*!
         * \brief Set up the GUI.
         */
        void setup_GUI();

        /*!
         * \brief Save the fitting results.
         */
        void save_Fit();

    signals:
        /*!
         * \brief Signal emitted when the fitting process has converged.
         */
        void hasConverged();

        /*!
         * \brief Signal emitted with the current status.
         * \param status The current status.
         */
        void currentStatus(const QString &status);

        /*!
         * \brief Signal emitted when parameters are updated.
         */
        void parametersUpdated();

        /*!
         * \brief Signal emitted when new parameters are available.
         */
        void newParameters();

        /*!
         * \brief Signal emitted when the fitting process is suspended.
         */
        void fitSuspended();

        /*!
         * \brief Signal emitted when the fitting process is aborted.
         */
        void fitAborted();

        /*!
         * \brief Signal emitted when the fitting process is resumed.
         */
        void fitResumed();

        /*!
         * \brief Signal emitted when the fitting process is started.
         */
        void fitStarted();

        /*!
         * \brief Signal emitted when the fitting widget is closed.
         */
        void fittingWidgetClosed();
};

#endif