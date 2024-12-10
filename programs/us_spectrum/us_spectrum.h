//! \file us_spectrum.h
#ifndef US_SPECTRUM_H
#define US_SPECTRUM_H

#include "us_widgets.h"
#include "us_plot.h"
#include "us_math2.h"
#include <math.h>

/**
 * @struct WavelengthProfile
 * @brief Structure to hold wavelength profile data.
 */
struct WavelengthProfile
{
    QVector<double> extinction;   //!< Extinction coefficients
    QVector<double> wvl;          //!< Wavelength values
    QwtPlotCurve* matchingCurve;  //!< Matching plot curve
    unsigned int lambda_scale;    //!< Lambda scale
    unsigned int lambda_min;      //!< Minimum lambda
    unsigned int lambda_max;      //!< Maximum lambda
    float scale;                  //!< Scale factor
    float amplitude;              //!< Amplitude
    QString filename;             //!< Filename
    QString header;               //!< Column header
    float nnls_factor;            //!< NNLS factor
    float nnls_percentage;        //!< NNLS percentage
};

/**
 * @class US_Spectrum
 * @brief The US_Spectrum class provides a user interface for spectrum analysis.
 */
class US_Spectrum : public US_Widgets
{
    Q_OBJECT

    public:
        /**
         * @brief Constructor for US_Spectrum.
         */
        US_Spectrum();
        int basisIndex;               //!< Index of the basis
        QString current_path;         //!< Current path

    private:
        QVector <double> residuals;    //!< residual vector

        QwtPlot* data_plot;           //!< Data plot
        QwtPlot* residuals_plot;      //!< Residuals plot
        US_Plot* plotLayout1;         //!< Plot layout 1
        US_Plot* plotLayout2;         //!< Plot layout 2
        WavelengthProfile w_target;   //!< Target wavelength profile
        QVector<WavelengthProfile> v_basis; //!< Vector of basis wavelength profiles
        QwtPlotCurve* solution_curve; //!< Solution curve
        QwtPlotPicker* pick;          //!< Plot picker
        WavelengthProfile w_solution; //!< Solution wavelength profile

        QPushButton* pb_load_target;  //!< Load target button
        QPushButton* pb_load_basis;   //!< Load basis button
        QPushButton* pb_load_fit;     //!< Load fit button
        QPushButton* pb_fit;          //!< Fit button
        QPushButton* pb_find_angles;  //!< Find angles button
        QPushButton* pb_help;         //!< Help button
        QPushButton* pb_save;         //!< Save button
        QPushButton* pb_overlap;      //!< Overlap button
        QPushButton* pb_close;        //!< Close button
        QPushButton* pb_reset_basis;  //!< Reset basis button
        QPushButton* pb_delete;       //!< Delete button
        QPushButton* pb_find_angle;   //!< Find angle button
        QPushButton* pb_find_extinction; //!< Find extinction button

        QListWidget* lw_target;       //!< Target list widget
        QListWidget* lw_basis;        //!< Basis list widget
        QLabel* lbl_wavelength;       //!< Wavelength label
        QLabel* lbl_extinction;       //!< Extinction label
        QLineEdit* le_angle;          //!< Angle line edit
        QLineEdit* le_wavelength;     //!< Wavelength line edit
        QLineEdit* le_extinction;     //!< Extinction line edit
        QLineEdit* le_rmsd;           //!< RMSD line edit

        QLabel* lbl_wvlinfo;          //!< Wavelength info label
        QLabel* lbl_correlation;      //!< Correlation label
        QLabel* lbl_fit;              //!< Fit label
        QLabel* lbl_rmsd;             //!< RMSD label
        QLabel* lbl_angle;            //!< Angle label
        QLabel* lbl_load_save;        //!< Load/Save label

        QComboBox* cb_angle_one;      //!< Angle one combo box
        QComboBox* cb_angle_two;      //!< Angle two combo box
        QComboBox* cb_spectrum_type;  //!< Spectrum type combo box

    private slots:
                /**
                 * @brief Slot to load basis data.
                 */
                void load_basis();

        /**
         * @brief Slot to plot basis data.
         */
        void plot_basis();

        /**
         * @brief Slot to load target data.
         */
        void load_target();

        /**
         * @brief Slot to plot target data.
         */
        void plot_target();

        /**
         * @brief Slot to handle new values from plot picker.
         * @param point The point picked
         */
        void new_value(const QwtDoublePoint& point);

        /**
         * @brief Slot to find extinction coefficients.
         */
        void findExtinction();

        /**
         * @brief Slot to fit the data.
         */
        void fit();

        /**
         * @brief Slot to delete the current basis data.
         */
        void deleteCurrent();

        /**
         * @brief Delete the current basis curve.
         * @return True if successful, false otherwise
         */
        bool deleteBasisCurve(void);

        /**
         * @brief Slot to reset the basis data.
         */
        void resetBasis();

        /**
         * @brief Slot to find angles.
         */
        void findAngles();

        /**
         * @brief Slot to save the results.
         */
        void save();

        /**
         * @brief Slot to load saved results.
         */
        void load();

        /**
         * @brief Slot to overlap the plots.
         */
        void overlap();
};

#endif
