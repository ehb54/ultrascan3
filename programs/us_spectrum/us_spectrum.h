//! \file us_spectrum.h
#ifndef US_SPECTRUM_H
#define US_SPECTRUM_H

#include "us_plot.h"
#include "us_widgets.h"
#include <QFileInfo>

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
        QFileInfo finfo;                   //!< Filename
        QString header;                    //!< Column header
        QVector<double> lambda;            //!< Wavelength values
        QVector<double> od;                //!< Extinction coefficients
        QVector<double> xvec;              //!< Trimmed lambda vector
        QVector<double> yvec;              //!< Trimmed Extinction coefficients vector
        float nnls_factor = -1;            //!< NNLS factor
        float nnls_percent = -1;           //!< NNLS percentage
        bool highlight = false;

        /**
         * @brief Clear fit data.
         */
        void clear_fit();
    };

    double wavl_min;                       //!< Minimum wavelength to fit
    double wavl_max;                       //!< Maximum wavelength to fit
    QString current_path;                  //!< Current path
    QList<DataProfile> basis_list;         //!< Basis spectra array
    DataProfile target;                    //!< Target profile
    QVector<double> solution;              //!< Fitted data
    QVector<double> residual;              //!< Residual data
    QVector<QwtPlotCurve *> basis_curves;  //!< Basis plot curves

    QwtPlot *data_plot;                    //!< Data plot
    QwtPlot *error_plot;                   //!< Residuals plot
             
    QTableWidget *tw_basis;                //!< Basis list widget
             
    QLineEdit *le_tgt_fname;               //!< Target filename
    QLineEdit *le_tgt_header;              //!< Target header
    QLineEdit *le_tgt_wavl;                //!< Wavelength range of the target spectrum
    QLineEdit *le_wavl_min;                //!< Input minimum wavelength to fit
    QLineEdit *le_wavl_max;                //!< Input maximum wavelength to fit
    QLineEdit *le_angle;                   //!< Correlation angle value
    QLineEdit *le_rmsd;                    //!< RMSD value
             
    QComboBox *cb_basis_1;                 //!< The first Basis selector for correlation angle
    QComboBox *cb_basis_2;                 //!< The second Basis selector for correlation angle

    QCheckBox *chkb_db_target;             //!< Checkbox to load the target from DB
    QCheckBox *chkb_db_basis;              //!< Checkbox to load the basis from DB    

    /**
     * @brief Plot data.
     */
    void plot();

    /**
     * @brief Clear plot data.
     */
    void clear_plot();

    /**
     * @brief Clear fitted data.
     */
    void clear_fit();

    /**
     * @brief Populate the list of the basis spectra.
     */
    void fill_table();

    /**
     * @brief Populate the combo list for correlation angle of the basis spectra.
     */
    void fill_combo();

    /**
     * @brief Hightlight the checked basis spectra.
     */
    void highlight();

    /**
     * @brief Find overlap region between the target and basis spectra.
     */
    void overlap();

    /**
     * @brief Check if the wavelenght is found the basis
     */
    bool find_lambda(const DataProfile &, const double, int &);

    /**
     * @brief Remove the basis from the list.
     */
    void delete_basis(int);

    /**
     * @brief Returns the indices to sort the given array.
     */
    QVector<int> argsort(const QVector<double> &);

    /**
     * @brief Sort an array by given indices.
     */
    void sort(const QVector<int> &, QVector<double> &);

  private slots:
    /**
     * @brief Slot to check the table item edited.
     */
    void basis_checked(QTableWidgetItem *);
    /**
     * @brief Slot to load basis data.
     */
    void load_basis();

    /**
     * @brief Slot to load target data.
     */
    void load_target();

    /**
     * @brief Minimum wavelength updated.
     */
    void min_wavl_updated();

    /**
     * @brief Maximum wavelength updated.
     */
    void max_wavl_updated();

    /**
     * @brief Slot to fit the data.
     */
    void fit();

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
};

#endif
