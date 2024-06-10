//! \file us_globalequil.h
//! \brief Contains the declaration of the US_GlobalEquil class and its members.
#ifndef US_GLOBEQUIL_H
#define US_GLOBEQUIL_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_editor.h"
#include "us_db2.h"
#include "us_globeq_data.h"
#include "us_eqmodel_control.h"
#include "us_eqfit_control.h"
#include "us_eqreporter.h"
#include "us_eqmath.h"
#include "us_eqhistogram.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

//! \class US_GlobalEquil
//! \brief A class for performing global equilibrium analysis in UltraScan.
class US_GlobalEquil : public US_Widgets
{
    Q_OBJECT

    public:
        //! \brief Constructor for the US_GlobalEquil class.
        US_GlobalEquil();

    private:
        QVector< ScanEdit >               scedits;        //!< Vector of scan edits.
        QVector< EqScanFit >              scanfits;       //!< Vector of scan fits.
        EqRunFit                          runfit;         //!< Run fit parameters structure.

        QVector< US_DataIO::EditedData >  dataList;       //!< Vector of edited data.
        QVector< US_DataIO::RawData >     rawList;        //!< Vector of raw data.
        QList< int >                      excludedScans;  //!< List of excluded scans.
        QStringList                       triples;        //!< List of triples.
        QStringList                       models;         //!< List of models.

        US_DataIO::EditedData*   edata;      //!< Pointer to the edited data.
        US_DataIO::SpeedData*    spdata;     //!< Pointer to the speed data.

        US_Disk_DB_Controls*     dkdb_cntrls; //!< Disk/DB controls.

        US_EqModelControl*       emodctrl;   //!< Pointer to the model control widget.
        US_EqFitControl*         efitctrl;   //!< Pointer to the fit control widget.
        US_EqReporter*           ereporter;  //!< Pointer to the reporter object.
        US_EqMath*               emath;      //!< Pointer to the math object.
        US_EqHistogram*          ehisto;     //!< Pointer to the histogram object.

        QList< double >          speed_steps; //!< List of speed steps.
        QList< double >          aud_params;  //!< List of audit parameters.
        QList< double >          ds_vbar20s;  //!< List of vbar values at 20°C.
        QList< double >          ds_densits;  //!< List of densities.
        QList< double >          ds_viscos;   //!< List of viscosities.
        QStringList              ds_solIDs;   //!< List of solution IDs.

        US_Help        showHelp;     //!< Help object.
        US_Plot*       eplot;        //!< Pointer to the plot layout.
        QwtPlot*       equil_plot;   //!< Pointer to the equilibrium plot.

        QwtCounter*    ct_scselect;  //!< Counter for scan selection.

        QCheckBox*     ck_edlast;    //!< Checkbox for editing last.

        QTableWidget*  tw_equiscns;  //!< Table widget for equilibrium scans.

        QLineEdit*     le_prjname;   //!< Line edit for project name.
        QLineEdit*     le_currmodl;  //!< Line edit for current model.
        QLineEdit*     le_mxfringe;  //!< Line edit for maximum fringe.
        QTextEdit*     te_status;    //!< Text edit for status.

        QPushButton*   pb_details;    //!< Push button for details.
        QPushButton*   pb_view;       //!< Push button for view report.
        QPushButton*   pb_unload;     //!< Push button for unload.
        QPushButton*   pb_scdiags;    //!< Push button for scan diagnostics.
        QPushButton*   pb_ckscfit;    //!< Push button for check scan fit.
        QPushButton*   pb_conchist;   //!< Push button for concentration histogram.
        QPushButton*   pb_resetsl;    //!< Push button for reset scan limits.
        QPushButton*   pb_selModel;   //!< Push button for select model.
        QPushButton*   pb_modlCtrl;   //!< Push button for model control.
        QPushButton*   pb_fitcntrl;   //!< Push button for fit control.
        QPushButton*   pb_loadFit;    //!< Push button for load fit.
        QPushButton*   pb_monCarlo;   //!< Push button for Monte Carlo.
        QPushButton*   pb_floatPar;   //!< Push button for float parameters.
        QPushButton*   pb_initPars;   //!< Push button for initialize parameters.

        QIcon          blue_arrow;    //!< Blue arrow icon.
        QIcon          green_arrow;   //!< Green arrow icon.
        QIcon          red_arrow;     //!< Red arrow icon.

        QVector< double > rvec; //!< Vector of radii.
        QVector< double > vvec; //!< Vector of values.

        QString        workingDir;  //!< Working directory.
        QString        modelname;   //!< Model name.

        int            dbg_level;   //!< Debug level.
        int            dbdisk;      //!< Disk/DB selection.
        int            iconw;       //!< Icon width.
        int            vecknt;      //!< Vector count.
        int            ntscns;      //!< Number of scans.
        int            modelx;      //!< Model index.
        int            sscanx;      //!< Selected scan index.
        int            sscann;      //!< Selected scan number.

        double         rpm_start;   //!< Start RPM.
        double         sRadLo;      //!< Low radius.
        double         sRadHi;      //!< High radius.
        double         sRadMv;      //!< Mid radius.
        double         cRadLo;      //!< Center low radius.
        double         cRadHi;      //!< Center high radius.
        double         od_limit;    //!< OD limit.

        bool           dataLoaded;    //!< Flag indicating if data is loaded.
        bool           buffLoaded;    //!< Flag indicating if buffer is loaded.
        bool           modelLoaded;   //!< Flag indicating if model is loaded.
        bool           dataLatest;    //!< Flag indicating if data is the latest.
        bool           mDown;         //!< Mouse down flag.
        bool           mMoved;        //!< Mouse moved flag.
        bool           mLowerH;       //!< Mouse lower half flag.
        bool           model_widget;  //!< Model widget flag.
        bool           fit_widget;    //!< Fit widget flag.
        bool           signal_mc;     //!< Signal Monte Carlo flag.
        bool           floated_pars;  //!< Floated parameters flag.
        bool           show_msgs;     //!< Show messages flag.

    private slots:
        //! \brief Slot to load data.
        void load(void);

        //! \brief Slot to show details.
        void details(void);

        //! \brief Slot to view the report.
        void view_report(void);

        //! \brief Slot to unload data.
        void unload(void);

        //! \brief Slot to perform scan diagnostics.
        void scan_diags(void);

        //! \brief Slot to check the scan fit.
        void check_scan_fit(void);

        //! \brief Slot to show concentration histogram.
        void conc_histogram(void);

        //! \brief Slot to reset scan limits.
        void reset_scan_lims(void);

        //! \brief Slot to load a model.
        void load_model(void);

        //! \brief Slot to handle new project name.
        //! \param project_name The new project name.
        void new_project_name(const QString& project_name);

        //! \brief Slot to select a model.
        void select_model(void);

        //! \brief Slot to control the model.
        void model_control(void);

        //! \brief Slot to control fitting.
        void fitting_control(void);

        //! \brief Slot to load fitting parameters.
        void load_fit(void);

        //! \brief Slot to perform Monte Carlo analysis.
        void monte_carlo(void);

        //! \brief Slot to float parameters.
        void float_params(void);

        //! \brief Slot to initialize parameters.
        void init_params(void);

        //! \brief Slot to close all dialogs.
        void close_all(void);

        //! \brief Slot to select a scan.
        //! \param scan_value The selected scan value.
        void scan_select(double scan_value);

        //! \brief Slot to update disk/DB selection.
        //! \param use_disk Boolean indicating if disk is used.
        void update_disk_db(bool use_disk);

        //! \brief Slot to handle item row change.
        void itemRowChanged(void);

        //! \brief Slot to handle double-click on an item.
        //! \param item The double-clicked table widget item.
        void doubleClickedItem(QTableWidgetItem* item);

        //! \brief Find data in the list.
        //! \param search_string The search string.
        //! \param tolerance The search tolerance.
        //! \param row The found row index.
        //! \param col The found column index.
        //! \return True if data is found, false otherwise.
        bool findData(QString search_string, double tolerance, int& row, int& col);

        //! \brief Slot for mouse down event.
        //! \param point The point where the mouse was pressed.
        void pMouseDown(const QwtDoublePoint& point);

        //! \brief Slot for mouse up event.
        //! \param point The point where the mouse was released.
        void pMouseUp(const QwtDoublePoint& point);

        //! \brief Slot for mouse move event.
        //! \param point The point where the mouse moved.
        void pMouseMoved(const QwtDoublePoint& point);

        //! \brief Plot the edited data.
        void edata_plot(void);

        //! \brief Plot the edited data after modifications.
        void edited_plot(void);

        //! \brief Assign scan fits.
        void assign_scanfit(void);

        //! \brief Setup run fitting parameters.
        void setup_runfit(void);

        //! \brief Add a new scan.
        //! \param scan_index The index of the new scan.
        void new_scan(int scan_index);

        //! \brief Slot to handle OD limit change.
        //! \param limit_string The new OD limit as a string.
        void od_limit_changed(const QString& limit_string);

        //! \brief Slot to update the OD limit.
        //! \param new_limit The new OD limit.
        void update_limit(double new_limit);

        //! \brief Slot to float all parameters.
        void float_all(void);

        //! \brief Slot to fix all parameters.
        void fix_all(void);

        //! \brief Get the index of a radius value.
        //! \param edata The edited data.
        //! \param radius The radius value.
        //! \return The index of the radius value.
        int index_radius(US_DataIO::EditedData* edata, double radius);

        //! \brief Get the index of an OD limit.
        //! \param scan_fit The scan fit.
        //! \param od_limit The OD limit.
        //! \return The index of the OD limit.
        int index_od_limit(EqScanFit& scan_fit, double od_limit);

        //! \brief Show help documentation.
        void help(void)
        {
            showHelp.show_help("global_equil.html");
        };
};
#endif
