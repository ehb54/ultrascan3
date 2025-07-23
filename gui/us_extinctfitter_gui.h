//! \file us_extinctfitter_gui.h
#ifndef US_EXTINCTFITTER_GUI_H
#define US_EXTINCTFITTER_GUI_H

#include <QApplication>
#include <QtGui>
#include "us_editor.h"
#include "us_minimize.h"

//! \struct Reading
//! \brief Structure to hold wavelength and optical density readings.
struct Reading
{
    float lambda; /*!< Wavelength */
    float od;     /*!< Optical density */
};

//! \struct WavelengthScan
//! \brief Structure to hold data for each wavelength scan file.
struct WavelengthScan
{
    QVector<Reading> v_readings; /*!< Vector of readings */
    QString fileName;            /*!< Name of the file */
    QString filePath;            /*!< Path to the file */
    QString description;         /*!< Description of the scan */
    int pos;                     /*!< Number of positive readings */
    int neg;                     /*!< Number of negative readings */
    int runs;                    /*!< Number of runs */
};

//! \class US_ExtinctFitter
//! \brief A class to perform extinction fitting using the US_Minimize framework.
class US_GUI_EXTERN US_ExtinctFitter : public US_Minimize
{
    Q_OBJECT

    public:

    /*!
     * \brief Constructor for US_ExtinctFitter.
     * \param wls_v Vector of WavelengthScan structures.
     * \param params Pointer to parameters array.
     * \param order Reference to the order of fitting.
     * \param maxOrder Reference to the maximum order of fitting.
     * \param projectName Reference to the project name.
     * \param fitDone Pointer to the fit completion flag.
     */
    US_ExtinctFitter(QVector<struct WavelengthScan> *wls_v, double *&params, unsigned int &order, unsigned int &maxOrder, QString &projectName, bool *fitDone);

    /*!
     * \brief Alternate constructor for US_ExtinctFitter.
     * \param wls_v Vector of WavelengthScan structures.
     * \param params Pointer to parameters array.
     * \param order Reference to the order of fitting.
     * \param maxOrder Reference to the maximum order of fitting.
     * \param projectName Reference to the project name.
     * \param fitDone Pointer to the fit completion flag.
     * \param silentMode Boolean indicating silent mode.
     */
    US_ExtinctFitter(QVector<struct WavelengthScan> *wls_v, double *&params, unsigned int &order, unsigned int &maxOrder, QString &projectName, bool *fitDone, bool silentMode);

    private:
        QVector<struct WavelengthScan> *wls_v; /*!< Vector of WavelengthScan structures. */
        QString projectName; /*!< Project name. */
        US_Editor *e; /*!< Editor widget for reports. */
        QString htmlDir; /*!< Directory for HTML reports. */
        unsigned int order; /*!< Order of fitting. */

    private slots:

        /*!
         * \brief Slot to clean up resources.
         */
        void cleanup();

        /*!
         * \brief Slot to calculate the Jacobian matrix.
         * \return Status of the Jacobian calculation.
         */
        int calc_jacobian();

    public slots:

        /*!
         * \brief Initialize the fitting process.
         * \return True if initialization is successful, otherwise false.
         */
        bool fit_init();

        /*!
         * \brief Calculate the model based on parameters.
         * \param params Pointer to the parameters array.
         * \return Status of the model calculation.
         */
        int calc_model(double *params);

        /*!
         * \brief View the fitting report.
         */
        void view_report();

        /*!
         * \brief Write the fitting report.
         */
        void write_report();

        /*!
         * \brief Plot overlay graphs.
         */
        void plot_overlays();

        /*!
         * \brief Plot residual graphs.
         */
        void plot_residuals();

        /*!
         * \brief Update the range of the scan.
         * \param scan The new scan range.
         */
        void updateRange(double scan);

        /*!
         * \brief End the fitting process.
         */
        void endFit();

        /*!
         * \brief Save the fitting results.
         */
        void saveFit();

        /*!
         * \brief Start the fitting process.
         */
        void startFit();

    signals:

        /*!
         * \brief Signal emitted when data is saved.
         * \param filePath The path to the saved file.
         * \param status The status of the save operation.
         */
        void dataSaved(const QString &filePath, const int status);

        /*!
         * \brief Signal emitted to get the fitted y-values.
         * \param yfit Vector to store the fitted y-values.
         * \param ydata Vector to store the original y-values.
         */
        void get_yfit(QVector<QVector<double>> &yfit, QVector<QVector<double>> &ydata);

        /*!
         * \brief Signal emitted to get the variance of the fit.
         * \param variance The variance of the fit.
         */
        void get_variance(double variance);
};

#endif

