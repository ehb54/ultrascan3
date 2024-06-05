//! \file us_vhw_enhcd.h
#ifndef US_VHW_ENHCD_H
#define US_VHW_ENHCD_H

#include "us_dataIO.h"
#include "us_analysis_base2.h"
#include "us_distrib_plot.h"
#include "us_editor.h"
#include "us_math2.h"
#include "us_model.h"
#include "us_run_details2.h"
#include "us_buffer_gui.h"
#include "qwt_plot_marker.h"

#define PA_POINTS 20     //!< Plateau average points to left and right

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug() //!< Debug-level-conditioned qDebug()
#endif

/**
 * @struct GrpInfo
 * @brief Structure to hold group information for vHW Enhanced analysis.
 */
typedef struct groupinfo_s
{
    double        x1;          //!< X coordinate of top mouse pick
    double        y1;          //!< Y coordinate of top mouse pick
    double        x2;          //!< X coordinate of bottom mouse pick
    double        y2;          //!< Y coordinate of bottom mouse pick
    double        sed;         //!< Average intercept sedimentation coefficient of group
    double        percent;     //!< Percent fraction of all divisions
    int           ndivs;       //!< Number of division lines included in pick
    QList<int>    idivs;       //!< List of divisions (0 to n-1) included
} GrpInfo;

/**
 * @class US_vHW_Enhanced
 * @brief The US_vHW_Enhanced class provides functionality for enhanced vHW analysis.
 */
class US_vHW_Enhanced : public US_AnalysisBase2
{
    Q_OBJECT

    public:
        /**
         * @brief Constructor for US_vHW_Enhanced.
         */
        US_vHW_Enhanced();

    private:

        enum { NONE, START, END } groupstep; //!< Enum for group selection steps

        QLabel*       lb_tolerance;          //!< Label for tolerance
        QLabel*       lb_division;           //!< Label for division

        QLineEdit*    le_temper;             //!< Line edit for temperature
        QLineEdit*    le_model;              //!< Line edit for model

        QCheckBox*    ck_modelpl;            //!< Checkbox for model plot
        QCheckBox*    ck_vhw_enh;            //!< Checkbox for vHW enhanced
        QCheckBox*    ck_manrepl;            //!< Checkbox for manual replacement
        QCheckBox*    ck_use_fed;            //!< Checkbox for use fed

        US_Help       showHelp;              //!< Help display object

        QwtCounter*   ct_tolerance;          //!< Counter for tolerance
        QwtCounter*   ct_division;           //!< Counter for division

        QwtPlotCurve*  curve;                //!< Plot curve
        QwtPlotCurve*  dcurve;               //!< Division plot curve
        US_PlotPicker* gpick;                //!< Plot picker

        QPushButton*  pb_dstrpl;             //!< Distribution plot button
        QPushButton*  pb_rsetda;             //!< Reset data button
        QPushButton*  pb_selegr;             //!< Select group button
        QPushButton*  pb_replot;             //!< Replot button

        double        boundPct;              //!< Bound percentage
        double        positPct;              //!< Position percentage
        double        baseline;              //!< Baseline value
        double        correc;                //!< Correction value
        double        C0;                    //!< Initial concentration
        double        c0term;                //!< C0 term
        double        Swavg;                 //!< Average sedimentation coefficient
        double        omega;                 //!< Angular velocity
        double        range;                 //!< Range value
        double        span;                  //!< Span value
        double        basecut;               //!< Base cut value
        double        platcut;               //!< Plateau cut value
        double        bdtoler;               //!< Back-diffusion tolerance
        double        invert;                //!< Invert value
        double        divfac;                //!< Division factor
        double        cpij;                  //!< CPij value
        double        sumcpij;               //!< Sum of CPij values
        double        sedc;                  //!< Sedimentation coefficient
        double        sdiff;                 //!< Sedimentation difference
        double        bdiff_coef;            //!< Back-diffusion coefficient
        double        bdiff_sedc;            //!< Back-diffusion sedimentation coefficient
        double        total_conc;            //!< Total concentration

        int           row;                   //!< Row index
        int           run_id;                //!< Run ID
        int           divsCount;             //!< Divisions count
        int           scanCount;             //!< Scan count
        int           valueCount;            //!< Value count
        int           dbg_level;             //!< Debug level
        int           lscnCount;             //!< Last scan count
        int           last_plot;             //!< Last plot's triple row index

        bool          haveZone;              //!< Flag for having a zone
        bool          groupSel;              //!< Flag for group selection
        bool          forcePlot;             //!< Flag for forcing plot
        bool          skipPlot;              //!< Flag for skipping plot
        bool          mo_plats;              //!< Flag for model plateaus
        bool          vhw_enh;               //!< Flag for vHW enhancement

        QString       run_name;              //!< Run name
        QString       cell;                  //!< Cell name
        QString       wavelength;            //!< Wavelength
        QString       workingDir;            //!< Working directory
        QString       runID;                 //!< Run ID
        QString       editID;                //!< Edit ID

        QVector<US_DataIO::EditedData> dsimList; //!< Simulation data
        QVector<bool>                  have_sims; //!< Have-simulation flags
        QVector<US_Model>              modlList; //!< Models for triples

        QVector<double>            aseds;      //!< All division sedimentation coefficients
        QVector<double>            dseds;      //!< Division sedimentation intercepts
        QVector<double>            dslos;      //!< Division slope values
        QVector<double>            dsigs;      //!< Division sigma values
        QVector<double>            dcors;      //!< Division correlation values
        QVector<int>               dpnts;      //!< Division fitted line points

        QVector<double>            scPlats;    //!< Scan plateaus for the current triple
        QVector<int>               liveScans;  //!< Vector of live scan indexes
        QVector<QVector<double>>   CPijs;      //!< CPij vectors, divisions in scans
        QVector<QVector<double>>   mconcs;     //!< Mid-division concentrations, divisions in scans
        QVector<double>            bdrads;     //!< Back-diffusion radii
        QVector<double>            bdcons;     //!< Back-diffusion concentrations

        QList<double>              groupxy;    //!< Group select pick coordinates
        QList<GrpInfo>             groupdat;   //!< Selected group information structures

        US_DataIO::EditedData*     edata;      //!< Current triple edited data
        US_DataIO::EditedData*     expda;      //!< Current triple edited data (experimental)
        US_DataIO::EditedData*     simda;      //!< Current triple edited data (simulation)
        US_DataIO::Scan*           dscan;      //!< Current data scan
        US_DataIO::Scan*           expsc;      //!< Current data scan (experimental)
        US_DataIO::Scan*           simsc;      //!< Current data scan (simulation)

        US_Model                   model;      //!< Current loaded model

        int kcalls[20];                        //!< Timing counts
        int kmsecs[20];                        //!< Timing milliseconds

    private slots:
                void load(void);                       //!< Slot to load data
        void data_plot(void);                  //!< Slot to plot data
        void distr_plot(void);                 //!< Slot to plot distribution
        void save_data(void);                  //!< Slot to save data
        void view_report(void);                //!< Slot to view the report
        void sel_groups(void);                 //!< Slot to select groups
        void div_seds(void);                   //!< Slot to divide sedimentation coefficients
        void update_density(double);           //!< Slot to update density
        void update_viscosity(double);         //!< Slot to update viscosity
        void update_vbar(double);              //!< Slot to update vbar
        void update_bdtoler(double);           //!< Slot to update back-diffusion tolerance
        void update_divis(double);             //!< Slot to update division
        int first_gteq(double, QVector<double>&, int, int); //!< Find first value greater than or equal to given value
        int first_gteq(double, QVector<double>&, int);      //!< Find first value greater than or equal to given value
        double sed_coeff(double, double, double* = NULL, int* = NULL); //!< Calculate sedimentation coefficient
        double avg_plateau(void);              //!< Calculate average plateau
        double sedcoeff_intercept(void);       //!< Calculate sedimentation coefficient intercept
        double back_diff_coeff(double);        //!< Calculate back-diffusion coefficient
        void groupClick(const QwtDoublePoint&); //!< Handle group click
        void add_group_info(void);             //!< Add group information
        void write_vhw(void);                  //!< Write vHW data
        void write_dis(void);                  //!< Write distribution data
        void write_model(void);                //!< Write model data
        void write_report(QTextStream&);       //!< Write the report
        QString text_time(double, int);        //!< Convert time to text format
        QString text_time(double);             //!< Convert time to text format
        QStringList last_edit_files(QStringList); //!< Get last edit files
        void new_triple(int);                  //!< Handle new triple selection
        void update(int);                      //!< Update the display
        void copy_data_files(QString, QString, QString); //!< Copy data files
        bool fitted_plateaus(void);            //!< Check for fitted plateaus
        bool model_plateaus(void);             //!< Check for model plateaus
        void plot_refresh(void);               //!< Refresh the plot
        void vhw_calcs_enhanced(double*, double*); //!< Perform enhanced vHW calculations
        void vhw_calcs_standard(double*, double*); //!< Perform standard vHW calculations
        bool have_model(void);                 //!< Check if model is available
        void vert_exclude_lines(void);         //!< Exclude vertical lines
        void exclude_from(double);             //!< Exclude data from a certain point
        void exclude_to(double);               //!< Exclude data to a certain point
        void live_scans(void);                 //!< Handle live scans
        void init_partials(void);              //!< Initialize partials
        void update_mid_concs(void);           //!< Update mid concentrations
        void create_simulation(void);          //!< Create simulation
        void plot_data2(void);                 //!< Plot data (alternative)
        void get_model(void);                  //!< Get the model

        /**
         * @brief Display help.
         */
        void help(void)
        { showHelp.show_help("vhw_enhanced.html"); };
};

#endif
