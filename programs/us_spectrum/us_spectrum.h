//! \file us_spectrum.h
#ifndef US_SPECTRUM_H
#define US_SPECTRUM_H

#include "us_math2.h"
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
        QFileInfo finfo;               //!< Filename
        QString header;                //!< Column header
        QVector<double> lambda;        //!< Wavelength values
        QVector<double> od;            //!< Extinction coefficients
        QVector<double> xvec;          //!< Trimmed lambda vector
        QVector<double> yvec;          //!< Trimmed Extinction coefficients vector
        float nnls_factor = -1;        //!< NNLS factor
        float nnls_percent = -1;       //!< NNLS percentage
        bool highlight = false;

        void clear_fit();              //!< Clear fit data
    };

    QString current_path; //!< Current path
    QList<DataProfile> all_basis;
    DataProfile target;
    QVector<double> solution;
    QVector<double> residual;
    QVector<QwtPlotCurve*> basis_curves;

    QwtPlot *data_plot;  //!< Data plot
    QwtPlot *error_plot; //!< Residuals plot

    QTableWidget *tw_basis; //!< Basis list widget

    QLineEdit *le_tgt_fname;
    QLineEdit *le_tgt_header;
    QLineEdit *le_tgt_minL;
    QLineEdit *le_tgt_maxL;
    QLineEdit *le_fit_minL;
    QLineEdit *le_fit_maxL;

    QLineEdit *le_angle; //!< Angle line edit
    QLineEdit *le_rmsd;  //!< RMSD line edit

    QComboBox *cb_basis_1; //!< Angle one combo box
    QComboBox *cb_basis_2; //!< Angle two combo box

    void plot();
    void clear_plot();
    void clear_fit();
    void fill_table();
    void fill_combo();
    void highlight();
    void overlap();
    void delete_basis(int);
    int find_lambda(DataProfile&, int, double);

    QVector<int> argsort(const QVector<double> &);

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
