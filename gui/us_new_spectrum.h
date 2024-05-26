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

#include "us_db2.h"
#include "us_eprofile.h"

#include "us_minimize.h"
#include "us_extinction_gui.h"
#include "us_extinctfitter_gui.h"

//! \class US_NewSpectrum
//! \brief A class to provide a GUI for managing new spectrum data.
class US_GUI_EXTERN US_NewSpectrum : public US_Widgets
{
    Q_OBJECT

    public:
        /*!
         * \brief Constructor for US_NewSpectrum with US_Buffer.
         * \param type The type of spectrum.
         * \param exists Flag indicating if the spectrum exists.
         * \param text The text description of the spectrum.
         * \param text_e280 The text description for E280.
         * \param p_buffer Pointer to the buffer.
         */
        US_NewSpectrum(QString type, QString exists, const QString &text, const QString &text_e280, US_Buffer* p_buffer);

        /*!
         * \brief Constructor for US_NewSpectrum with US_Analyte.
         * \param type The type of spectrum.
         * \param exists Flag indicating if the spectrum exists.
         * \param text The text description of the spectrum.
         * \param text_e280 The text description for E280.
         * \param p_analyte Pointer to the analyte.
         */
        US_NewSpectrum(QString type, QString exists, const QString &text, const QString &text_e280, US_Analyte* p_analyte);

        /*!
         * \brief Constructor for US_NewSpectrum with US_Solution.
         * \param type The type of spectrum.
         * \param exists Flag indicating if the spectrum exists.
         * \param text The text description of the spectrum.
         * \param text_e280 The text description for E280.
         * \param p_solution Pointer to the solution.
         */
        US_NewSpectrum(QString type, QString exists, const QString &text, const QString &text_e280, US_Solution* p_solution);

        US_Buffer* buffer; /*!< Buffer object. */
        US_Buffer* tmp_buffer; /*!< Temporary buffer object. */

        US_Analyte* analyte; /*!< Analyte object. */
        US_Analyte* tmp_analyte; /*!< Temporary analyte object. */

        US_Solution* solution; /*!< Solution object. */
        US_Solution* tmp_solution; /*!< Temporary solution object. */

        US_Extinction* w_spec; /*!< Extinction object. */
        QString text; /*!< Text description of the spectrum. */
        QString tmp_text; /*!< Temporary text description of the spectrum. */
        QString type; /*!< Type of the spectrum. */
        QString tmp_type; /*!< Temporary type of the spectrum. */
        QString text_e280; /*!< Text description for E280. */
        QString tmp_text_e280; /*!< Temporary text description for E280. */
        QString exists; /*!< Flag indicating if the spectrum exists. */
        QString tmp_exists; /*!< Temporary flag indicating if the spectrum exists. */

    signals:
        /*!
         * \brief Signal emitted when the spectrum changes.
         */
        void change_spectrum();

        /*!
         * \brief Signal emitted to enable the accept button.
         */
        void accept_enable();

    private:
        QPushButton* pb_view; /*!< View button. */
        QPushButton* pb_delete; /*!< Delete button. */
        QPushButton* pb_cancel; /*!< Cancel button. */
        QPushButton* pb_manual; /*!< Manual button. */
        QPushButton* pb_uploadDisk; /*!< Upload from disk button. */
        QPushButton* pb_uploadFit; /*!< Upload fit button. */

        QMap<double, double> loc_extinct; /*!< Local extinction map. */

    private slots:
        /*!
         * \brief Slot to handle the cancel action.
         */
        void cancel();

        /*!
         * \brief Slot to handle the upload from disk action.
         */
        void uploadDisk();

        /*!
         * \brief Slot to handle the upload fit action.
         */
        void uploadFit();

        /*!
         * \brief Slot to process the results.
         * \param xyz The extinction data.
         */
        void process_results(QMap<double, double> &xyz);

        /*!
         * \brief Slot to add spectrum from disk.
         */
        void add_spectrumDisk();

        /*!
         * \brief Slot to read spectra from a file.
         * \param filename The file name.
         */
        void readingspectra(const QString &filename);

        /*!
         * \brief Slot to edit the spectrum manually.
         */
        void editmanually();

        /*!
         * \brief Slot to delete the spectrum.
         */
        void delete_spectrum();

        /*!
         * \brief Slot to view the spectrum.
         */
        void view_spectrum();
};

#endif