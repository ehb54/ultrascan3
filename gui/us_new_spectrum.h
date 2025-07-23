//! \file us_new_spectrum.h
#ifndef US_NEW_SPECTRUM_H
#define US_NEW_SPECTRUM_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_widgets_dialog.h"
#include "us_buffer.h"
#include "us_analyte.h"
#include "us_solution.h"
#include "us_help.h"

#include "us_minimize.h"
#include "us_extinction_gui.h"
#include "us_extinctfitter_gui.h"

//! \class US_NewSpectrum
//! \brief Class to handle the creation of new spectrum data.
class US_GUI_EXTERN US_NewSpectrum : public US_Widgets
{
   Q_OBJECT

    public:
        //! \brief Constructor for US_NewSpectrum with buffer.
        //! \param type Type of spectrum.
        //! \param text Description text.
        //! \param text_e280 E280 text.
        //! \param buffer Pointer to US_Buffer object.
        US_NewSpectrum(QString type, const QString &text, const QString &text_e280, US_Buffer* buffer);

        //! \brief Constructor for US_NewSpectrum with analyte.
        //! \param type Type of spectrum.
        //! \param text Description text.
        //! \param text_e280 E280 text.
        //! \param analyte Pointer to US_Analyte object.
        US_NewSpectrum(QString type, const QString &text, const QString &text_e280, US_Analyte* analyte);

        //! \brief Constructor for US_NewSpectrum with solution.
        //! \param type Type of spectrum.
        //! \param text Description text.
        //! \param text_e280 E280 text.
        //! \param solution Pointer to US_Solution object.
        US_NewSpectrum(QString type, const QString &text, const QString &text_e280, US_Solution* solution);

        US_Buffer*   buffer;          //!< Pointer to US_Buffer object.
        US_Buffer*   tmp_buffer;      //!< Temporary buffer pointer.

        US_Analyte*   analyte;        //!< Pointer to US_Analyte object.
        US_Analyte*   tmp_analyte;    //!< Temporary analyte pointer.

        US_Solution*   solution;      //!< Pointer to US_Solution object.
        US_Solution*   tmp_solution;  //!< Temporary solution pointer.

        US_Extinction*  w_spec;       //!< Pointer to US_Extinction object.
        QString        text, tmp_text; //!< Description text and temporary text.
        QString        type, tmp_type; //!< Type and temporary type.
        QString        text_e280, tmp_text_e280; //!< E280 text and temporary E280 text.

    signals:
        //! \brief Signal emitted when the protein E280 value changes.
        void change_prot_e280( void );

    private:
        QPushButton*  pb_cancel;        //!< Cancel button.
        QPushButton*  pb_manual;        //!< Manual entry button.
        QPushButton*  pb_uploadDisk;    //!< Upload from disk button.
        QPushButton*  pb_uploadFit;     //!< Upload fit button.

        QMap< double, double > loc_extinct; //!< Local extinction map.

    private slots:
        //! \brief Slot to handle cancel action.
        void cancel ( void );

        //! \brief Slot to handle upload from disk action.
        void uploadDisk ( void );

        //! \brief Slot to handle upload fit action.
        void uploadFit  ( void );

        //! \brief Slot to process results.
        //! \param xyz Reference to QMap containing the results.
        void process_results( QMap < double, double > &xyz );

        //! \brief Slot to add spectrum from disk.
        void add_spectrumDisk   ( void );

        //! \brief Slot to read spectra.
        //! \param filename Name of the file containing the spectra.
        void readingspectra     (const QString& filename);

        //! \brief Slot to enter data manually.
        void entermanually      ( void );
};

//! \class US_ViewSpectrum
//! \brief Class to view spectrum data.
class US_ViewSpectrum : public US_Widgets
{
    Q_OBJECT

    public:
        //! \brief Constructor for US_ViewSpectrum.
        //! \param tmp_extinction Reference to QMap containing extinction data.
        US_ViewSpectrum(QMap<double,double>& tmp_extinciton);

        QMap <double, double> extinction; //!< Extinction data map.
        US_Plot* plotLayout; //!< Layout for the plot.
        QwtPlot* data_plot; //!< Pointer to the QwtPlot object.

        //! \brief Function to plot the extinction data.
        void plot_extinction();

    private slots:
        //! \brief Slot to save data to a CSV file.
        void save_csv();
};

#endif
