//! \file us_extinction_gui.h
#ifndef US_EXTINCTION_GUI_H
#define US_EXTINCTION_GUI_H

//#include <QApplication>
//#include <QtGui>

#include "us_widgets.h"
#include "us_plot.h"
#include "us_minimize.h"
#include "us_extinctfitter_gui.h"
#include "us_analyte_gui.h"
#include "us_csv_loader.h"

//! \class CustomListWidgetItem
//! \brief Custom list widget item with comparison operator.
class CustomListWidgetItem : public QListWidgetItem {
    public:
        //! \brief Constructor for CustomListWidgetItem.
        //! \param parent Pointer to parent QListWidget.
        CustomListWidgetItem(QListWidget* parent = nullptr)
                : QListWidgetItem(parent) {}

        //! \brief Comparison operator.
        //! \param other Reference to another QListWidgetItem.
        //! \return True if this item is less than the other item.
        bool operator<(const QListWidgetItem& other) const;
};

//! \class US_Extinction
//! \brief Class to handle extinction data and GUI.
class US_GUI_EXTERN US_Extinction : public US_Widgets
{
    Q_OBJECT

    public:
        //! \brief Default constructor for US_Extinction.
        US_Extinction();

        //! \brief Constructor for US_Extinction with parameters.
        //! \param temp_mode Temporary mode.
        //! \param text Description text.
        //! \param text_e280 E280 text.
        //! \param parent Pointer to parent widget.
        US_Extinction(QString temp_mode, const QString &text, const QString &text_e280, QWidget *parent);

        QVector<QString> filenames; //!< Vector of filenames.
        US_ExtinctFitter* fitter; //!< Pointer to US_ExtinctFitter object.
        US_Disk_DB_Controls* disk_controls; //!< Pointer to disk controls.
        US_Analyte currentAnalyte; //!< Current analyte.
        QVector<double> lambda; //!< Vector of wavelength data.
        QVector<double> extinction; //!< Vector of extinction data.
        float xmin, xmax; //!< Minimum and maximum x values.
        unsigned int order, parameters; //!< Order and parameters for fitting.
        double* fitparameters; //!< Pointer to fit parameters.
        bool fitted, fitting_widget; //!< Fitting status flags.
        QString buffer_temp; //!< Buffer temperature.

        QVector<QVector<double>> xfit_data; //!< Vector of x fit data.
        QVector<QVector<double>> yfit_data; //!< Vector of y fit data.

        QString current_path; //!< Current file path.

    private:
        QVector<WavelengthScan> v_wavelength; //!< Vector of wavelength scans.
        QVector<WavelengthScan> v_wavelength_original; //!< Vector of original wavelength scans.
        QVector<QwtPlotCurve*> v_curve; //!< Vector of plot curves.

        QWidget* parent; //!< Pointer to parent widget.

        QString projectName, filename_one, filename_two; //!< Project and file names.
        unsigned int maxrange; //!< Maximum range.
        float odCutoff, lambdaLimitLeft, lambdaLimitRight, lambda_min, lambda_max,
                pathlength, extinction_coefficient, factor, selected_wavelength; //!< Various parameters for calculations.
        QLabel* lbl_gaussians; //!< Label for gaussians.
        QLabel* lbl_peptide; //!< Label for peptide.
        QLabel* lbl_wvinfo; //!< Label for wavelength info.
        QLabel* lbl_associate; //!< Label for associate.
        QLabel* lbl_cutoff; //!< Label for cutoff.
        QLabel* lbl_lambda1; //!< Label for lambda 1.
        QLabel* lbl_lambda2; //!< Label for lambda 2.
        QLabel* lbl_pathlength; //!< Label for pathlength.
        QLabel* lbl_coefficient; //!< Label for coefficient.
        QLabel* lbl_wavelengthref; //!< Label for wavelength reference.
        QListWidget* lw_file_names; //!< List widget for file names.
        QPushButton* pb_addWavelength; //!< Button to add wavelength.
        QPushButton* pb_reset; //!< Button to reset.
        QPushButton* pb_update; //!< Button to update.

        QPushButton* pb_perform; //!< Button to perform.
        QPushButton* pb_perform_buffer; //!< Button to perform buffer operation.
        QPushButton* pb_perform_analyte; //!< Button to perform analyte operation.
        QPushButton* pb_perform_solution; //!< Button to perform solution operation.
        QPushButton* pb_accept; //!< Button to accept.

        QPushButton* pb_calculate; //!< Button to calculate.
        QPushButton* pb_save; //!< Button to save.
        QPushButton* pb_view; //!< Button to view.
        QPushButton* pb_help; //!< Button to help.
        QPushButton* pb_close; //!< Button to close.

        QLineEdit* le_associate; //!< Line edit for associate.
        QLineEdit* le_associate_buffer; //!< Line edit for associate buffer.

        QLineEdit* le_odCutoff; //!< Line edit for OD cutoff.
        QLineEdit* le_lambdaLimitLeft; //!< Line edit for left lambda limit.
        QLineEdit* le_lambdaLimitRight; //!< Line edit for right lambda limit.
        QLineEdit* le_pathlength; //!< Line edit for pathlength.
        QLineEdit* le_coefficient; //!< Line edit for coefficient.
        QwtCounter* ct_gaussian; //!< Counter for gaussian.
        QwtCounter* ct_coefficient; //!< Counter for coefficient.
        QwtPlotCurve* changedCurve; //!< Pointer to changed curve.
        US_Plot* plotLayout; //!< Pointer to plot layout.
        QwtPlot* data_plot; //!< Pointer to data plot.
        QWidget* p; //!< Pointer to widget.

    private slots:
        //! \brief Slot to load scan data.
        //! \param data Reference to CSV_Data.
        //! \return True if scan is loaded successfully.
        bool loadScan(US_CSV_Data& data);

        //! \brief Check if a line is a comment.
        //! \param line The line to check.
        //! \return True if the line is a comment.
        bool isComment(const QString& line);

        //! \brief Slot to add a wavelength.
        void add_wavelength(void);

        //! \brief Slot to reset the scan list.
        void reset_scanlist(void);

        //! \brief Slot to update data.
        void update_data(void);

        //! \brief Slot to perform global operation.
        void perform_global(void);

        //! \brief Slot to perform global buffer operation.
        void perform_global_buffer(void);

        //! \brief Slot to perform global analyte operation.
        void perform_global_analyte(void);

        //! \brief Slot to perform global solution operation.
        void perform_global_solution(void);

        //! \brief Slot to accept operation.
        void accept(void);

        //! \brief Slot to calculate E280.
        void calculateE280(void);

        //! \brief Slot to save data.
        void save(void);

        //! \brief Slot to view result.
        void view_result(void);

        //! \brief Slot to show help.
        void help(void);

        //! \brief Slot to plot data.
        void plot(void);

        //! \brief Slot to calculate extinction.
        void calc_extinction(void);

        //! \brief Slot to update order.
        //! \param value New order value.
        void update_order(double value);

        //! \brief Slot to convert list to curve.
        void listToCurve(void);

        //! \brief Slot to delete a curve.
        //! \return True if the curve is deleted.
        bool deleteCurve(void);

        //! \brief Slot to access analyte extinction data.
        //! \param analyte The analyte data.
        void accessAnalyteExtinc(US_Analyte analyte);

        //! \brief Slot to process y fit data.
        //! \param x Reference to x data vector.
        //! \param y Reference to y data vector.
        void process_yfit(QVector<QVector<double>>& x, QVector<QVector<double>>& y);

    signals:
        //! \brief Signal to get results.
        //! \param xyz Reference to results map.
        void get_results(QMap<double, double>& xyz);
};

#endif
