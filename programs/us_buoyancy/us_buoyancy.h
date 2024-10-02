//! \file us_buoyancy.h
//! \brief Contains the declaration of the US_Buoyancy class and its members.
#ifndef US_BUOYANCY_H
#define US_BUOYANCY_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_dataIO.h"
#include "us_simparms.h"
#include "us_editor.h"
#include "us_minimize.h"
#include "us_extinctfitter_gui.h"
#include "qwt_plot_marker.h"

//! \struct DataPoint
//! \brief Structure to hold data points for buoyancy analysis.
struct DataPoint
{
    QString name; //!< Name of the data point.
    QString description; //!< Description of the data point.
    QString dataset; //!< Dataset associated with the data point.
    QString triple; //!< Triple associated with the data point.
    double peakPosition; //!< Position of the peak.
    double peakDensity; //!< Density of the peak.
    double peakVbar; //!< Vbar of the peak.
    double peakGaussArea; //!< Gaussian area of the peak.
    double percentTotal; //!< Percentage of the total.
    double percentTotal_uncorrected; //!< Uncorrected percentage of the total.
    double temperature; //!< Temperature.
    double bufferDensity; //!< Buffer density.
    double centerpiece; //!< Centerpiece.
    double meniscus; //!< Meniscus position.
    double bottom; //!< Bottom position.
    double speed; //!< Speed.
    double gradientMW; //!< Gradient molecular weight.
    double gradientC0; //!< Gradient C0.
    double gradientVbar; //!< Gradient Vbar.
    double stretch; //!< Stretch.
    double sigma; //!< Sigma.
};

//! \struct cellInfo
//! \brief Structure to hold cell information.
struct cellInfo
{
    QString cellName; //!< Name of the cell.
    QString channelName; //!< Name of the channel.
    int centerpieceID; //!< Centerpiece ID.
};

//! \class US_Buoyancy
//! \brief A class for performing buoyancy analysis.
class US_Buoyancy : public US_Widgets
{
    Q_OBJECT

    public:
        //! \brief Constructor for the US_Buoyancy class.
        US_Buoyancy();

        //! \brief Constructor for the US_Buoyancy class with auto mode.
        //! \param auto_mode The auto mode string.
        US_Buoyancy(QString auto_mode);

        bool us_buoyancy_auto_mode; //!< Flag for auto mode.

        signals:
                //! \brief Signal emitted when the widget is closed.
                void widgetClosed();

    protected:
        //! \brief Override of the close event to emit widgetClosed signal.
        //! \param event The close event.
        void closeEvent(QCloseEvent* event) override;

    private:
        QVector<WavelengthScan> v_wavelength; //!< Vector of wavelength scans.
        US_ExtinctFitter* fitter; //!< Pointer to the extinct fitter.
        double* fitparameters; //!< Fit parameters.

        QMap<QString, QVector<double>> xfit_data; //!< X fit data.
        QMap<QString, QVector<double>> yfit_data; //!< Y fit data.
        QMap<QString, QVector<double>> triple_name_to_Cdata; //!< C data for each triple.

        QMap<QString, QMap<double, QMap<int, QVector<double>>>> xfit_data_all_orders; //!< X fit data for all orders.
        QMap<QString, QMap<double, QMap<int, QVector<double>>>> yfit_data_all_orders; //!< Y fit data for all orders.
        QMap<QString, QMap<double, QMap<int, double>>> variance_triple_order_map; //!< Variance map for each triple and order.

        int current_order; //!< Current order.
        double current_sigma; //!< Current sigma value.

        US_DataIO::RawData data; //!< Raw data.
        QList<US_DataIO::SpeedData> sData; //!< Speed data.
        QVector<US_DataIO::RawData> allData; //!< All raw data.
        QVector<double> meniscus; //!< Meniscus positions.
        QVector<DataPoint> dpoint; //!< Vector of data points.

        QMap<QString, QVector<double>> triple_name_to_peaks_map; //!< Peaks map for each triple.
        QMap<QString, double> triple_name_to_total_area; //!< Total area for each triple.
        QMap<QString, double> triple_name_to_total_area_uncorrected; //!< Uncorrected total area for each triple.
        QMap<QString, double> triple_name_to_rmsd; //!< RMSD for each triple.
        QMap<QString, double*> triple_name_to_fit_parameters; //!< Fit parameters for each triple.
        QMap<QString, QVector<QwtPlotCurve*>> triple_name_to_peak_curves_map; //!< Peak curves map for each triple.
        QMap<QString, QVector<QwtPlotCurve*>> triple_name_to_peak_gauss_envelopes_map; //!< Gaussian envelope curves map for each triple.
        QMap<QString, QMap<QString, QStringList>> triple_name_to_peak_to_parms_map; //!< Peak parameters map for each triple.
        QMap<QString, bool> triple_report_saved_map; //!< Report saved map for each triple.
        QMap<QString, bool> triple_fitted_map; //!< Fitted map for each triple.
        QMap<QString, bool> triple_peaks_defined_map; //!< Peaks defined map for each triple.
        QMap<QString, bool> do_fit_for_triple; //!< Fit flag for each triple.

        QMap<QString, double> alpha_centerpiece; //!< Alpha centerpiece for each triple.
        QMap<QString, double> data_left_to_triple_name_map; //!< Data left map for each triple.
        QMap<QString, double> data_right_to_triple_name_map; //!< Data right map for each triple.
        QMap<QString, double> meniscus_to_triple_name_map; //!< Meniscus map for each triple.
        QMap<QString, double> buffDensity_to_triple_name_map; //!< Buffer density map for each triple.
        QMap<QString, double> sigma_to_triple_name_map; //!< Sigma map for each triple.
        QMap<QString, double> gradMW_to_triple_name_map; //!< Gradient MW map for each triple.
        QMap<QString, double> gradVbar_to_triple_name_map; //!< Gradient Vbar map for each triple.
        QMap<QString, double> gradC0_to_triple_name_map; //!< Gradient C0 map for each triple.

        QMap<QString, int> gauss_order_minVariance; //!< Minimum variance order for each triple.
        QMap<QString, double> sigma_val_minVariance; //!< Minimum variance sigma value for each triple.

        QRadioButton* rb_meniscus; //!< Radio button for meniscus.
        QRadioButton* rb_datapoint; //!< Radio button for data point.
        DataPoint tmp_dpoint; //!< Temporary data point.

        int current_triple; //!< Current triple index.
        int current_scan; //!< Current scan index.
        int total_speeds; //!< Total speeds.
        int dbg_level; //!< Debug level.

        bool expIsBuoyancy; //!< Flag for buoyancy experiment.

        double bottom; //!< Bottom position.
        double bottom_calc; //!< Calculated bottom position.
        double current_rpm; //!< Current RPM.
        double current_stretch; //!< Current stretch.
        double current_point; //!< Current point (x coordinate picked by the user).

        QList<int> sd_offs; //!< Speed data offsets for each triple.
        QList<int> sd_knts; //!< Speed data counts for each triple.

        US_Help showHelp; //!< Help object.
        US_Editor* te; //!< Text editor.

        QVector<US_SimulationParameters> simparams; //!< Simulation parameters.

        QString workingDir; //!< Working directory.
        QString runID; //!< Run ID.
        QString editID; //!< Edit ID.
        QString dataType; //!< Data type.
        QString expType; //!< Experiment type.
        QStringList files; //!< List of files.
        QStringList triples; //!< List of triples.
        QStringList trip_rpms; //!< List of RPMs for each triple.

        QPushButton* pb_details; //!< Button for details.
        QPushButton* pb_load; //!< Button to load data.

        QwtPlot* data_plot; //!< Data plot.
        QwtPlotCurve* v_line; //!< Vertical line on the plot.
        QwtPlotCurve* minimum_curve; //!< Minimum curve on the plot.
        QwtPlotGrid* grid; //!< Plot grid.
        QwtPlotMarker* marker; //!< Plot marker.
        US_PlotPicker* pick; //!< Plot picker.
        US_Plot* plot; //!< US_Plot object.

        QLabel* lbl_rpms; //!< Label for RPMs.
        QLabel* lbl_stretch; //!< Label for stretch.
        QLabel* lbl_dens_0; //!< Label for density.
        QLabel* lbl_bottom; //!< Label for bottom.
        QLabel* lbl_bottom_calc; //!< Label for calculated bottom.
        QLabel* lbl_vbar; //!< Label for vbar.
        QLabel* lbl_MW; //!< Label for molecular weight.
        QLabel* lbl_meniscus; //!< Label for meniscus.
        QLabel* lbl_temperature; //!< Label for temperature.
        QLabel* lbl_peakVbar; //!< Label for peak vbar.
        QLabel* lbl_peakPosition; //!< Label for peak position.
        QLabel* lbl_peakDensity; //!< Label for peak density.
        QLabel* lbl_peakName; //!< Label for peak name.
        QLabel* lbl_buffer_density; //!< Label for buffer density.

        QLineEdit* le_info; //!< Line edit for info.
        QLineEdit* le_stretch; //!< Line edit for stretch.
        QLineEdit* le_investigator; //!< Line edit for investigator.
        QLineEdit* le_dens_0; //!< Line edit for density.
        QLineEdit* le_bottom; //!< Line edit for bottom.
        QLineEdit* le_bottom_calc; //!< Line edit for calculated bottom.
        QLineEdit* le_vbar; //!< Line edit for vbar.
        QLineEdit* le_MW; //!< Line edit for molecular weight.
        QLineEdit* le_meniscus; //!< Line edit for meniscus.
        QLineEdit* le_sigma; //!< Line edit for sigma.
        QLineEdit* le_temperature; //!< Line edit for temperature.
        QLineEdit* le_peakVbar; //!< Line edit for peak vbar.
        QLineEdit* le_peakPosition; //!< Line edit for peak position.
        QLineEdit* le_peakDensity; //!< Line edit for peak density.
        QLineEdit* le_peakName; //!< Line edit for peak name.
        QLineEdit* le_peakGaussArea; //!< Line edit for peak Gaussian area.
        QLineEdit* le_buffer_density; //!< Line edit for buffer density.

        US_Disk_DB_Controls* disk_controls; //!< Disk/DB controls.

        QComboBox* cb_triple; //!< Combo box for triples.
        QComboBox* cb_rpms; //!< Combo box for RPMs.
        QComboBox* cb_peaks; //!< Combo box for peaks.

        QPushButton* pb_write; //!< Button to write data.
        QPushButton* pb_save; //!< Button to save data.
        QPushButton* pb_delete_peak; //!< Button to delete peak.
        QPushButton* pb_add_peak; //!< Button to add peak.
        QPushButton* pb_fit_current_triple; //!< Button to fit current triple.
        QCheckBox* ck_auto_fit; //!< Checkbox for auto fit.

        QProgressBar* pgb_progress; //!< Progress bar.
        QPushButton* pb_view_reports; //!< Button to view reports.

        QwtCounter* ct_selectScan; //!< Counter to select scan.

    private slots:
        //! \brief Calculate stretch.
        //! \return The calculated stretch.
        double calc_stretch();

        //! \brief Draw vertical line on the plot.
        //! \param x The x-coordinate of the line.
        void draw_vline(double x);

        //! \brief Draw vertical line on the plot automatically.
        //! \param x The x-coordinate of the line.
        //! \param triple The triple associated with the line.
        void draw_vline_auto(double x, QString triple);

        //! \brief Draw Gaussian envelope on the plot.
        //! \param params The parameters for the Gaussian envelope.
        void draw_gauss_envelope(QMap<QString, QStringList> params);

        //! \brief Handle mouse events.
        //! \param point The point clicked by the mouse.
        void mouse(const QwtDoublePoint& point);

        //! \brief Handle mouse events for peaks.
        //! \param point The point clicked by the mouse.
        void mouse_peak(const QwtDoublePoint& point);

        //! \brief Select investigator.
        void sel_investigator();

        //! \brief Update disk/DB selection.
        //! \param disk The disk selection.
        void update_disk_db(bool disk);

        //! \brief Load data.
        void load();

        //! \brief Show details.
        void details();

        //! \brief Change the current triple.
        //! \param index The index of the new triple.
        void new_triple(int index);

        //! \brief Change the current peak.
        //! \param index The index of the new peak.
        void new_peak(int index);

        //! \brief Plot the selected scan.
        //! \param scan The scan to plot.
        void plot_scan(double scan);

        //! \brief Update for sigma.
        void update_for_sigma();

        //! \brief Update for molecular weight.
        void update_for_MW();

        //! \brief Update for vbar.
        void update_for_vbar();

        //! \brief Update for density.
        void update_for_dens_0();

        //! \brief Write data.
        void write();

        //! \brief Write data automatically.
        void write_auto();

        //! \brief Save data.
        void save();

        //! \brief Save data automatically.
        //! \param auto_mode The auto mode string.
        void save_auto(QString auto_mode);

        //! \brief Reset the data.
        void reset();

        //! \brief Calculate points.
        void calc_points();

        //! \brief Calculate points automatically.
        //! \param auto_mode The auto mode string.
        void calc_points_auto(QString auto_mode);

        //! \brief Find the closest sigma height.
        //! \param triple The triple associated with the data.
        //! \param value The value to find the closest sigma height.
        //! \return The closest sigma height.
        QMap<QString, double> find_closest_sigma_height(QString triple, double value);

        //! \brief Identify peaks.
        //! \param triple The triple associated with the data.
        //! \param value The value to identify peaks.
        //! \return The identified peaks.
        QVector<double> identify_peaks(QString triple, double value);

        //! \brief Get the index of the data.
        //! \param data The data vector.
        //! \param value The value to find the index.
        //! \return The index of the data.
        int index_of_data(QVector<double> data, double value);

        //! \brief Check if the point is a maximum.
        //! \param data The data vector.
        //! \param index The index of the point.
        //! \param left The left boundary.
        //! \param right The right boundary.
        //! \param triple The triple associated with the data.
        //! \return True if the point is a maximum, otherwise false.
        bool isMaximum_y(QVector<double> data, int index, int left, int right, QString triple);

        //! \brief Get the data configuration from the edit profile.
        //! \param triple The triple associated with the data.
        //! \param edit The edit profile.
        //! \return The data configuration.
        QMap<QString, double> get_data_conf_from_edit_profile(QString triple, QString edit);

        //! \brief Process Y fit data.
        //! \param x The X fit data.
        //! \param y The Y fit data.
        void process_yfit(QVector<QVector<double>>& x, QVector<QVector<double>>& y);

        //! \brief Process variance.
        //! \param variance The variance value.
        void process_variance(double variance);

        //! \brief Compute RMSD.
        //! \param triple The triple associated with the data.
        //! \return The RMSD value.
        double compute_rmsd(QString triple);

        //! \brief Delete a peak.
        void delete_peak();

        //! \brief Add a peak.
        void add_peak();

        //! \brief Calculate Gaussian area.
        //! \param triple The triple associated with the data.
        //! \param mean The mean value.
        //! \param sigma The sigma value.
        //! \param amplitude The amplitude value.
        //! \return The calculated Gaussian area.
        QMap<QString, double> calc_gauss_area(QString triple, double mean, double sigma, double amplitude);

        //! \brief Calculate total area.
        //! \param triple The triple associated with the data.
        //! \return The calculated total area.
        QMap<QString, double> calc_total_area(QString triple);

        //! \brief Enable the fit button.
        //! \param enable True to enable, false to disable.
        void enblFitBtn(bool enable);

        //! \brief Fit the current triple.
        void fit_current_triple();

        //! \brief Print XY data.
        //! \param data The raw data.
        //! \param index The index of the data.
        void print_xy(US_DataIO::RawData data, int index);

        //! \brief Change the current RPM value.
        //! \param index The index of the new RPM value.
        void new_rpmval(int index);

        //! \brief Update fields.
        void update_fields();

        //! \brief Update speed data.
        void update_speedData();

        //! \brief Update density.
        void update_dens_0();

        //! \brief Update buffer density.
        void update_bufferDensity();

        //! \brief Update bottom position.
        void update_bottom();

        //! \brief Update calculated bottom position.
        void update_bottom_calc();

        //! \brief Update vbar.
        void update_vbar();

        //! \brief Update molecular weight.
        void update_MW();

        //! \brief Update peak name.
        void update_peakName();

        //! \brief Update meniscus position.
        void update_meniscus();

        //! \brief Show help.
        void help()
        {
            showHelp.show_help("manual/us_buoyancy.html");
        }
};

#endif // US_BUOYANCY_H