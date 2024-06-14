//! \file us_eqfit_control.h
//! \brief Contains the declaration of the US_EqFitControl class and its members.
#ifndef US_EQFITCTRL_H
#define US_EQFITCTRL_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_globeq_data.h"
#include "us_eqmath.h"
#include "us_eqreporter.h"
#include "us_fit_worker.h"
#include "us_dataIO.h"
#include "us_plot.h"
#include "us_help.h"

//! \class US_EqFitControl
//! \brief A class for controlling equilibrium fitting in UltraScan.
class US_EqFitControl : public US_WidgetsDialog
{
    Q_OBJECT

    public:
        //! \brief Constructor for the US_EqFitControl class.
        //! \param scan_fits Vector of scan fits.
        //! \param run_fit Reference to the run fit.
        //! \param edata Pointer to the edited data.
        //! \param emath Pointer to the equil-math object.
        //! \param ereporter Pointer to the equil-reporter object.
        //! \param model_index Selected model index.
        //! \param model_titles List of model titles.
        //! \param fitting_widget_created Flag indicating if the fitting widget is created.
        //! \param selected_scan Current selected scan.
        US_EqFitControl( QVector< EqScanFit >&, EqRunFit&,
                         US_DataIO::EditedData*, US_EqMath*, US_EqReporter*,
                         int, QStringList, bool&, int& );

        //! \brief Slot to handle new scan.
        //! \param scan_index The new scan index.
        void new_scan( int scan_index );

        //! \brief Slot to handle new components.
        void new_components( void );

        //! \brief Slot to set float flag.
        //! \param is_float Boolean indicating if the flag is float.
        void set_float( bool is_float );

        signals:
                //! \brief Signal to update scan.
                //! \param scan_index The scan index to update.
                void update_scan( int scan_index );

    private:
        QVector< EqScanFit >&   scanfits;  //!< Vector of scan fits.
        EqRunFit&               runfit;    //!< Run fit parameters structure.
        US_DataIO::EditedData*  edata;     //!< Pointer to edited data.
        US_EqMath*              emath;     //!< Pointer to equil-math object.
        US_EqReporter*          ereporter; //!< Pointer to equil-reporter object.
        int                     modelx;    //!< Selected model index.
        QStringList             models;    //!< List of model titles.
        bool&                   fWidget;   //!< Flag indicating if fitting widget is created.
        int&                    selscan;   //!< Current selected scan.

        QLineEdit*         le_iternbr;    //!< Line edit for iteration number.
        QLineEdit*         le_varianc;    //!< Line edit for variance.
        QLineEdit*         le_stddev;     //!< Line edit for standard deviation.
        QLineEdit*         le_improve;    //!< Line edit for improvement.
        QLineEdit*         le_funceva;    //!< Line edit for function evaluations.
        QLineEdit*         le_decompo;    //!< Line edit for decomposition.
        QLineEdit*         le_clambda;    //!< Line edit for lambda.
        QLineEdit*         le_nbrpars;    //!< Line edit for number of parameters.
        QLineEdit*         le_nbrsets;    //!< Line edit for number of sets.
        QLineEdit*         le_nbrdpts;    //!< Line edit for number of data points.
        QLineEdit*         le_lamstrt;    //!< Line edit for lambda start.
        QLineEdit*         le_lamsize;    //!< Line edit for lambda size.
        QLineEdit*         le_mxiters;    //!< Line edit for maximum iterations.
        QLineEdit*         le_fittolr;    //!< Line edit for fitting tolerance.
        QLineEdit*         le_status;     //!< Line edit for status.
        QLineEdit*         le_inform;     //!< Line edit for information.

        QRadioButton*      rb_pltalld;    //!< Radio button for plot all data.
        QRadioButton*      rb_pltgrp5;    //!< Radio button for plot group of 5.
        QRadioButton*      rb_pltsscn;    //!< Radio button for plot single scan.
        QRadioButton*      rb_lincnsn;    //!< Radio button for linear constraint no.
        QRadioButton*      rb_lincnsy;    //!< Radio button for linear constraint yes.
        QRadioButton*      rb_autocnn;    //!< Radio button for auto constraint no.
        QRadioButton*      rb_autocny;    //!< Radio button for auto constraint yes.

        QCheckBox*         ck_monfitg;    //!< Checkbox for monitor fitting.

        QComboBox*         cb_nlsalgo;    //!< Combo box for NLS algorithm.

        QPushButton*       pb_strtfit;    //!< Button to start fit.
        QPushButton*       pb_pause;      //!< Button to pause fit.
        QPushButton*       pb_resume;     //!< Button to resume fit.
        QPushButton*       pb_savefit;    //!< Button to save fit.
        QPushButton*       pb_viewrep;    //!< Button to view report.
        QPushButton*       pb_resids;     //!< Button to plot residuals.
        QPushButton*       pb_ovrlays;    //!< Button to plot overlays.
        QPushButton*       pb_help;       //!< Button to show help.
        QPushButton*       pb_close;      //!< Button to close dialog.
        QPushButton*       pb_lnvsr2;     //!< Button to plot Ln vs R^2.
        QPushButton*       pb_mwvsr2;     //!< Button to plot MW vs R^2.
        QPushButton*       pb_mwvscv;     //!< Button to plot MW vs CV.
        QPushButton*       pb_cmments;    //!< Button for comments.

        QProgressBar*      progress;      //!< Progress bar.

        QwtCounter*        ct_plotscn;    //!< Counter for plot scan.

        US_Plot*           dplot;         //!< Plot object.
        QwtPlot*           data_plot;     //!< Data plot object.
        US_FitWorker*      fitwork;       //!< Fit worker object.
        FitCtrlPar         fitpars;       //!< Fit control parameters.

        US_Help            showHelp;      //!< Help object.

        //bool               send_signal;

        int                ntpts;           //!< Total data points.
        int                ndsets;          //!< Total data sets (scans).
        int                nfpars;          //!< Total fit parameters.
        int                nlsmeth;         //!< NLS method flag.
        int                mxiters;         //!< Maximum fit iterations.
        int                mxspts;          //!< Maximum points in any scan.
        int                ipscnn;          //!< Start plot scan number.
        int                lpscnn;          //!< Last plot scan number.
        int                npscns;          //!< Number of plot scans.
        int                plotgrpf;        //!< Plot group flag: -1, 0, 5.
        int                plottype;        //!< Plot type 0-4: resids, ...

        QVector< int >     v_dscnx;         //!< Vector of scans' data indices.
        int*               dscnx;           //!< Array of scans' data indices.

        double             fittoler;        //!< Fit tolerance.

    private slots:
        //! \brief Slot to start fitting.
        void start_fit( void );

        //! \brief Slot to pause fitting.
        void pause_fit( void );

        //! \brief Slot to resume fitting.
        void resume_fit( void );

        //! \brief Slot to save fitting results.
        void save_fit( void );

        //! \brief Slot to view report.
        void view_report( void );

        //! \brief Slot to plot residuals.
        void plot_residuals( void );

        //! \brief Slot to plot overlays.
        void plot_overlays( void );

        //! \brief Slot to plot two parameters.
        void plot_two( void );

        //! \brief Slot to plot three parameters.
        void plot_three( void );

        //! \brief Slot to plot four parameters.
        void plot_four( void );

        //! \brief Slot to handle closing event.
        void closed( void );

        //! \brief Slot to update progress.
        //! \param progress_value The progress value.
        void new_progress( int progress_value );

        //! \brief Slot to handle fitting completion.
        void fit_completed( void );

        //! \brief Slot to prepare data.
        void prepare_data( void );

        //! \brief Slot to handle new plot scan.
        void new_pscan( void );

        //! \brief Slot to show help.
        void help( void )
        { showHelp.show_help( "global_equil-fitctrl.html" ); };
};

#endif