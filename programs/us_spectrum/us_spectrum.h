//! \file us_spectrum.h
#ifndef US_SPECTRUM_H
#define US_SPECTRUM_H

#include "us_widgets.h"
#include "us_plot.h"
#include "us_math2.h"
#include <math.h>
#include <QFileInfo>

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
        

    private:

        /**
         * @class US_Spectrum::DataProfile
         * @brief The DataProfile class provides a data structure for the basis and target spectra.
         */
        class DataProfile
        {
        public:
            QFileInfo finfo;                //!< Filename
            QString header;                 //!< Column header
            QVector<double> lambda;         //!< Wavelength values
            QVector<double> od;             //!< Extinction coefficients
            QVector<double> xvec;           //!< Trimmed lambda vector
            QVector<double> yvec;           //!< Trimmed Extinction coefficients vector
            QwtPlotCurve* curve = nullptr;  //!< Attached QwtPlotCurve
            float nnls_factor  = -1;        //!< NNLS factor
            float nnls_percent = -1;        //!< NNLS percentage
            bool highlight = false;

            /**
             * @brief Clear data related for fitting and plotting
             */
            void clear( bool );
        };
        
        QString current_path;         //!< Current path
        QList<DataProfile> all_basis;
        DataProfile target;
        DataProfile solution;
        DataProfile residual;




        QVector <double> residuals;    //!< residual vector

        QwtPlot* data_plot;           //!< Data plot
        QwtPlot* error_plot;      //!< Residuals plot
        WavelengthProfile w_target;   //!< Target wavelength profile
        QVector<WavelengthProfile> v_basis; //!< Vector of basis wavelength profiles
        QwtPlotCurve* solution_curve; //!< Solution curve
        WavelengthProfile w_solution; //!< Solution wavelength profile

        QTableWidget* tw_basis;        //!< Basis list widget

        QLineEdit* le_tgt_fname;
        QLineEdit* le_tgt_header;
        QLineEdit* le_tgt_minL;
        QLineEdit* le_tgt_maxL;
        QLineEdit* le_fit_minL;
        QLineEdit* le_fit_maxL;

        QLineEdit* le_angle;          //!< Angle line edit
        QLineEdit* le_rmsd;           //!< RMSD line edit
        
        QComboBox* cb_basis_1;      //!< Angle one combo box
        QComboBox* cb_basis_2;      //!< Angle two combo box
        
        void fill_table();
        void fill_combo();
        void plot();

    private slots:

        void highlight( QTableWidgetItem* );
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
         * @brief Slot to fit the data.
         */
        void fit();

        /**
         * @brief Delete the current basis curve.
         * @return True if successful, false otherwise
         */
        bool deleteBasisCurve(void);

        /**
         * @brief Slot to reset the basis data.
         */
        void reset();

        /**
         * @brief Slot to find angles.
         */
        void find_angle();

        /**
         * @brief Slot to save the results.
         */
        void save();

        /**
         * @brief Slot to overlap the plots.
         */
        void overlap();
};

#endif
