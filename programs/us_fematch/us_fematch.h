//! \file us_fematch.h
//! \brief Contains the declaration of the US_FeMatch class and its members.
#ifndef US_FEMATCH_H
#define US_FEMATCH_H

#include <QApplication>
#include <QtSvg>

#include "us_resids_bitmap.h"
#include "us_plot_control_fem.h"
#include "us_advanced_fem.h"
#include "us_dmga_mc_stats.h"
#include "us_noise_loader.h"
#include "us_resplot_fem.h"
#include "us_dataIO.h"
#include "us_db2.h"
#include "us_astfem_rsa.h"
#include "us_model.h"
#include "us_noise.h"
#include "us_model_loader.h"
#include "us_editor.h"
#include "us_math2.h"
#include "us_run_details2.h"
#include "us_buffer_gui.h"
#include "us_analyte.h"
#include "us_solution.h"
#include "qwt_plot_marker.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

#ifndef SP_SPEEDPROFILE
#define SP_SPEEDPROFILE US_SimulationParameters::SpeedProfile
#endif

//! \class US_FeMatch
//! \brief A class for managing finite element matches in UltraScan.
class US_FeMatch : public US_Widgets
{
    Q_OBJECT

    public:
        //! \brief Constructor for the US_FeMatch class.
        US_FeMatch();

        //! \brief Get the edited data.
        //! \return Pointer to the edited data.
        US_DataIO::EditedData* fem_editdata();

        //! \brief Get the simulated data.
        //! \return Pointer to the simulated data.
        US_DataIO::RawData* fem_simdata();

        //! \brief Get the exclusion list.
        //! \return Pointer to the list of excluded scans.
        QList<int>* fem_excllist();

        //! \brief Get the model.
        //! \return Pointer to the model.
        US_Model* fem_model();

        //! \brief Get the time-invariant noise.
        //! \return Pointer to the time-invariant noise.
        US_Noise* fem_ti_noise();

        //! \brief Get the radial-invariant noise.
        //! \return Pointer to the radial-invariant noise.
        US_Noise* fem_ri_noise();

        //! \brief Get the residuals bitmap.
        //! \return QPointer to the residuals bitmap.
        QPointer<US_ResidsBitmap> fem_resbmap();

        //! \brief Auto load and simulate data.
        //! \param rawData The raw data.
        //! \param editedData The edited data.
        //! \param model The model.
        //! \param int_param An integer parameter.
        //! \param point_param A point parameter.
        void auto_load_simulate(US_DataIO::RawData rawData, US_DataIO::EditedData editedData, US_Model model, int int_param, QPoint point_param);

        signals:
                //! \brief Signal for ASTFEM comparison.
                //! \param int_param An integer parameter.
                void astfem_cmp(int int_param);

    public slots:
                //! \brief Slot for thread progress.
                //! \param progress Current progress value.
                //! \param total Total progress value.
                void thread_progress(int progress, int total);

        //! \brief Slot for thread completion.
        //! \param result Result of the thread operation.
        void thread_complete(int result);

        //! \brief Slot to start simulation.
        void simulate();

        //! \brief Slot for residual plot completion.
        void resplot_done();

    private:
        QLabel* lb_tolerance; //!< Label for tolerance.
        QLabel* lb_division; //!< Label for division.
        QLabel* lb_rmsd; //!< Label for RMSD.

        QLineEdit* le_id; //!< Line edit for ID.
        QLineEdit* le_temp; //!< Line edit for temperature.
        QLineEdit* le_solution; //!< Line edit for solution.
        QLineEdit* le_density; //!< Line edit for density.
        QLineEdit* le_viscosity; //!< Line edit for viscosity.
        QLineEdit* le_vbar; //!< Line edit for vbar.
        QLineEdit* le_compress; //!< Line edit for compressibility.
        QLineEdit* le_rmsd; //!< Line edit for RMSD.
        QLineEdit* le_variance; //!< Line edit for variance.

        QTextEdit* te_desc; //!< Text edit for description.

        QCheckBox* ck_edit; //!< Checkbox for edit mode.

        QRadioButton* rb_curmod; //!< Radio button for current model.
        QRadioButton* rb_mean; //!< Radio button for mean model.
        QRadioButton* rb_median; //!< Radio button for median model.
        QRadioButton* rb_mode; //!< Radio button for mode model.

        QPushButton* pb_nextm; //!< Button for next model.

        QwtCounter* ct_model; //!< Counter for model.
        QwtCounter* ct_from; //!< Counter for from value.
        QwtCounter* ct_to; //!< Counter for to value.

        QwtPlotCurve* curve; //!< Plot curve.
        QwtPlotCurve* dcurve; //!< Differential plot curve.
        US_PlotPicker* gpick; //!< Plot picker.

        US_Disk_DB_Controls* dkdb_cntrls; //!< Disk/DB controls.

        QPushButton* pb_load; //!< Button to load data.
        QPushButton* pb_details; //!< Button to view details.
        QPushButton* pb_solution; //!< Button to view solution.
        QPushButton* pb_view; //!< Button to view data.
        QPushButton* pb_save; //!< Button to save data.
        QPushButton* pb_distrib; //!< Button to view distribution.
        QPushButton* pb_advanced; //!< Button for advanced options.
        QPushButton* pb_adv_dmga; //!< Button for advanced DMGA.
        QPushButton* pb_plot3d; //!< Button to plot 3D data.
        QPushButton* pb_plotres; //!< Button to plot residuals.
        QPushButton* pb_reset; //!< Button to reset data.
        QPushButton* pb_help; //!< Button for help.
        QPushButton* pb_close; //!< Button to close the dialog.
        QPushButton* pb_rmsd; //!< Button to calculate RMSD.
        QPushButton* pb_exclude; //!< Button to exclude data.
        QPushButton* pb_reset_exclude; //!< Button to reset exclusions.
        QPushButton* pb_loadmodel; //!< Button to load model.
        QPushButton* pb_simumodel; //!< Button to simulate model.

        QListWidget* lw_triples; //!< List widget for triples.

        int scanCount; //!< Scan count.
        int dbg_level; //!< Debug level.
        int nthread; //!< Number of threads.
        int thrdone; //!< Number of threads completed.
        int mc_iters; //!< Number of MC iterations.

        bool dataLoaded; //!< Flag indicating if data is loaded.
        bool haveSim; //!< Flag indicating if simulation data is available.
        bool dataLatest; //!< Flag indicating if data is the latest.
        bool buffLoaded; //!< Flag indicating if buffer is loaded.
        bool cnstvb; //!< Flag for constant vbar.
        bool cnstff; //!< Flag for constant ff.
        bool exp_steps; //!< Flag for experimental steps.
        bool dat_steps; //!< Flag for data steps.
        bool is_dmga_mc; //!< Flag for DMGA Monte Carlo.

        QString run_name; //!< Run name.
        QString cell; //!< Cell identifier.
        QString wavelength; //!< Wavelength value.
        QString workingDir; //!< Working directory.
        QString runID; //!< Run ID.
        QString editID; //!< Edit ID.
        QString dfilter; //!< Data filter.
        QString mfilter; //!< Model filter.

        QStringList files; //!< List of files.

        US_DataIO::EditedData* edata; //!< Pointer to edited data.
        US_DataIO::Scan* dscan; //!< Pointer to scan data.
        US_DataIO::RawData* rdata; //!< Pointer to raw data.
        US_DataIO::RawData* sdata; //!< Pointer to simulated data.

        US_DataIO::RawData wsdata; //!< Workspace raw data.

        US_Model model; //!< Current model.
        US_Model model_loaded; //!< Loaded model.
        US_Model model_used; //!< Used model.
        US_Noise ri_noise; //!< Radial-invariant noise.
        US_Noise ti_noise; //!< Time-invariant noise.
        US_Solution solution_rec; //!< Solution record.
        QList<US_Model> models; //!< List of models.

        QVector<SP_SPEEDPROFILE> speed_steps; //!< Vector of speed steps.

        QPointer<US_ResidsBitmap> rbmapd; //!< Pointer to residuals bitmap.
        QPointer<US_PlotControlFem> eplotcd; //!< Pointer to plot control.
        QPointer<US_ResidPlotFem> resplotd; //!< Pointer to residual plot.
        QPointer<US_AdvancedFem> advdiag; //!< Pointer to advanced dialog.

        QPoint bmd_pos; //!< Bitmap position.
        QPoint epd_pos; //!< Plot control position.
        QPoint rpd_pos; //!< Residual plot position.

        QDateTime start_time; //!< Start time.

        QMap<QString, QString> adv_vals; //!< Advanced values map.

        QVector<int> kcomps; //!< Vector of component indices.

        QList<US_DataIO::RawData> tsimdats; //!< List of simulated data.
        QList<US_Model> tmodels; //!< List of models.
        US_SimulationParameters simparams; //!< Simulation parameters.
        QVector<US_Model> imodels; //!< Vector of intermediate models.

    private slots:
        //! \brief Slot to load data.
        void load();

        //! \brief Slot to plot data.
        void data_plot();

        //! \brief Slot to save data.
        void save_data();

        //! \brief Slot to view details.
        void details();

        //! \brief Slot to reset data.
        void reset();

        //! \brief Slot for advanced options.
        void advanced();

        //! \brief Slot for advanced DMGA.
        void adv_dmga();

        //! \brief Slot to plot 3D data.
        void plot3d();

        //! \brief Slot to plot residuals.
        void plotres();

        //! \brief Slot to update data.
        //! \param value The value to update.
        void update(int value);

        //! \brief Slot to load model.
        void load_model();

        //! \brief Slot to set distribution type.
        void distrib_type();

        //! \brief Slot to simulate model.
        void simulate_model();

        //! \brief Slot to view report.
        void view_report();

        //! \brief Slot to exclude data.
        void exclude();

        //! \brief Slot to adjust model.
        void adjust_model();

        //! \brief Slot to reset excludes.
        void reset_excludes();

        //! \brief Slot to exclude data from a specific range.
        //! \param value The from value.
        void exclude_from(double value);

        //! \brief Slot to exclude data to a specific range.
        //! \param value The to value.
        void exclude_to(double value);

        //! \brief Slot to set component number.
        //! \param value The component number.
        void comp_number(double value);

        //! \brief Slot to set component values.
        //! \param value The component index.
        void component_values(int value);

        //! \brief Get last edit files.
        //! \param editFiles List of edit files.
        //! \return List of last edit files.
        QStringList last_edit_files(QStringList editFiles);

        //! \brief Get the type distribution.
        //! \return The type distribution.
        int type_distrib();

        //! \brief Slot to plot stick distribution.
        //! \param index The distribution index.
        void distrib_plot_stick(int index);

        //! \brief Slot to plot 2D distribution.
        //! \param index The distribution index.
        void distrib_plot_2d(int index);

        //! \brief Slot to plot residuals distribution.
        void distrib_plot_resids();

        //! \brief Slot to load noise data.
        void load_noise();

        //! \brief Interpolate a value.
        //! \param x_value The x value.
        //! \param x_data The x data array.
        //! \param y_data The y data array.
        //! \param size The size of the data arrays.
        //! \return The interpolated value.
        double interp_sval(double x_value, double* x_data, double* y_data, int size);

        //! \brief Write a report.
        //! \param stream The text stream to write to.
        void write_report(QTextStream& stream);

        //! \brief Write dataset report.
        //! \param report The report string.
        void write_dset_report(QString& report);

        //! \brief Indent a string.
        //! \param level The indentation level.
        //! \return The indented string.
        QString indent(int level) const;

        //! \brief Get the text representation of a model.
        //! \param model The model to convert to text.
        //! \param level The indentation level.
        //! \return The text representation of the model.
        QString text_model(US_Model model, int level);

        //! \brief Calculate the baseline.
        //! \param index The index value.
        //! \return The baseline value.
        double calc_baseline(int index) const;

        //! \brief Calculate residuals.
        void calc_residuals();

        //! \brief Close all dialogs.
        void close_all();

        //! \brief Create a table row.
        //! \param col1 The first column value.
        //! \param col2 The second column value.
        //! \return The table row string.
        QString table_row(const QString& col1, const QString& col2) const;

        //! \brief Create a table row with three columns.
        //! \param col1 The first column value.
        //! \param col2 The second column value.
        //! \param col3 The third column value.
        //! \return The table row string.
        QString table_row(const QString& col1, const QString& col2, const QString& col3) const;

        //! \brief Create a table row with five columns.
        //! \param col1 The first column value.
        //! \param col2 The second column value.
        //! \param col3 The third column value.
        //! \param col4 The fourth column value.
        //! \param col5 The fifth column value.
        //! \return The table row string.
        QString table_row(const QString& col1, const QString& col2, const QString& col3, const QString& col4, const QString& col5) const;

        //! \brief Create a table row with seven columns.
        //! \param col1 The first column value.
        //! \param col2 The second column value.
        //! \param col3 The third column value.
        //! \param col4 The fourth column value.
        //! \param col5 The fifth column value.
        //! \param col6 The sixth column value.
        //! \param col7 The seventh column value.
        //! \return The table row string.
        QString table_row(const QString& col1, const QString& col2, const QString& col3, const QString& col4, const QString& col5, const QString& col6, const QString& col7) const;

        //! \brief Create HTML header.
        //! \param title The title.
        //! \param subtitle The subtitle.
        //! \param edata Pointer to the edited data.
        //! \return The HTML header string.
        QString html_header(QString title, QString subtitle, US_DataIO::EditedData* edata);

        //! \brief Get run details.
        //! \return The run details string.
        QString run_details();

        //! \brief Get hydrodynamics information.
        //! \return The hydrodynamics string.
        QString hydrodynamics() const;

        //! \brief Get scan information.
        //! \return The scan information string.
        QString scan_info() const;

        //! \brief Get distribution information.
        //! \return The distribution information string.
        QString distrib_info();

        //! \brief Create model table.
        //! \param table The table string.
        void model_table(QString table);

        //! \brief Set progress.
        //! \param progress The progress string.
        void set_progress(const QString progress);

        //! \brief Update disk or database.
        //! \param is_disk Flag indicating if it's a disk operation.
        void update_disk_db(bool is_disk);

        //! \brief Write plot to a file.
        //! \param filename The filename.
        //! \param plot Pointer to the plot.
        void write_plot(const QString& filename, const QwtPlot* plot);

        //! \brief Create a directory.
        //! \param path The directory path.
        //! \param subdir The subdirectory name.
        //! \return True if directory was created successfully, false otherwise.
        bool mkdir(const QString& path, const QString& subdir);

        //! \brief Handle new triple selection.
        //! \param index The triple index.
        void new_triple(int index);

        //! \brief Get solution data.
        void get_solution();

        //! \brief Update the solution data.
        //! \param solution The solution data.
        void updateSolution(US_Solution solution);

        //! \brief Update progress.
        //! \param value The progress value.
        void update_progress(int value);

        //! \brief Report files to database.
        //! \param fileList The list of files.
        void reportFilesToDB(QStringList& fileList);

        //! \brief Show results.
        void show_results();

        //! \brief Update the file list.
        //! \param fileList The list of files.
        //! \param filter The filter string.
        void update_filelist(QStringList& fileList, const QString filter);

        //! \brief Handle current model button click.
        //! \param checked Flag indicating if the button is checked.
        void curmod_clicked(bool checked);

        //! \brief Handle model button click.
        //! \param checked Flag indicating if the button is checked.
        void modbtn_clicked(bool checked);

        //! \brief Handle next model button click.
        void next_model();

        //! \brief Update Monte Carlo model.
        void update_mc_model();

        //! \brief Check if intensity profile is present.
        //! \param filename The filename.
        //! \param is_mc Flag indicating if it's a Monte Carlo operation.
        //! \return True if intensity profile is present, false otherwise.
        bool has_intensity_profile(const QString& filename, const bool is_mc);

        //! \brief Show help.
        void help() { showHelp.show_help("fe_match.html"); };

    protected:
        QList<int> excludedScans; //!< List of excluded scans.
        QStringList triples; //!< List of triples.
        QVector<US_DataIO::EditedData> dataList; //!< List of edited data.
        QVector<US_DataIO::RawData> rawList; //!< List of raw data.
        QVector<QVector<double>> resids; //!< Vector of residuals.
        QVector<QList<int>> allExcls; //!< List of all exclusions.

        US_Math2::SolutionData solution; //!< Solution data.

        US_Help showHelp; //!< Help instance.

        double density; //!< Density value.
        double viscosity; //!< Viscosity value.
        double vbar; //!< Vbar value.
        double compress; //!< Compressibility value.

        bool manual; //!< Manual flag.

        // Layouts
        QBoxLayout* mainLayout; //!< Main layout.
        QBoxLayout* leftLayout; //!< Left layout.
        QBoxLayout* rightLayout; //!< Right layout.
        QBoxLayout* buttonLayout; //!< Button layout.
        QBoxLayout* progressLayout; //!< Progress layout.

        QGridLayout* analysisLayout; //!< Analysis layout.
        QGridLayout* runInfoLayout; //!< Run info layout.
        QGridLayout* parameterLayout; //!< Parameter layout.
        QGridLayout* controlsLayout; //!< Controls layout.

        QProgressBar* progress; //!< Progress bar.

        US_Plot* plotLayout1; //!< Plot layout 1.
        US_Plot* plotLayout2; //!< Plot layout 2.

        // Widgets
        QwtPlot* data_plot1; //!< Data plot 1.
        QwtPlot* data_plot2; //!< Data plot 2.

        //! \brief Handle close event.
        //! \param event The close event.
        void closeEvent(QCloseEvent* event) override;
};

#endif // US_FEMATCH_H
