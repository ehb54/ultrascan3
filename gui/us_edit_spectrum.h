//! \file us_solution_gui.h
#ifndef US_EDIT_SPECTRUM_H
#define US_EDIT_SPECTRUM_H

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

//! \class US_EditSpectrum
//! \brief A class to provide a GUI for editing spectra for buffers, analytes, and solutions.
class US_GUI_EXTERN US_EditSpectrum : public US_Widgets
{
Q_OBJECT

public:

/*!
 * \brief Constructor for editing buffer spectrum.
 * \param type The type of spectrum.
 * \param exists Whether the spectrum already exists.
 * \param text The description text.
 * \param text_e280 The E280 text.
 * \param buffer The buffer object.
 */
US_EditSpectrum(QString type, QString exists, const QString &text, const QString &text_e280, US_Buffer* buffer);

/*!
 * \brief Constructor for editing analyte spectrum.
 * \param type The type of spectrum.
 * \param exists Whether the spectrum already exists.
 * \param text The description text.
 * \param text_e280 The E280 text.
 * \param analyte The analyte object.
 */
US_EditSpectrum(QString type, QString exists, const QString &text, const QString &text_e280, US_Analyte* analyte);

/*!
 * \brief Constructor for editing solution spectrum.
 * \param type The type of spectrum.
 * \param exists Whether the spectrum already exists.
 * \param text The description text.
 * \param text_e280 The E280 text.
 * \param solution The solution object.
 */
US_EditSpectrum(QString type, QString exists, const QString &text, const QString &text_e280, US_Solution* solution);

US_Buffer* buffer; /*!< Buffer object. */
US_Buffer* tmp_buffer; /*!< Temporary buffer object. */

US_Analyte* analyte; /*!< Analyte object. */
US_Analyte* tmp_analyte; /*!< Temporary analyte object. */

US_Solution* solution; /*!< Solution object. */
US_Solution* tmp_solution; /*!< Temporary solution object. */

US_Extinction* w_spec; /*!< Extinction object. */
QString text, tmp_text; /*!< Description text. */
QString type, tmp_type; /*!< Spectrum type. */
QString text_e280, tmp_text_e280; /*!< E280 text. */
QString exists, tmp_exists; /*!< Exists flag. */

signals:

    /*!
     * \brief Signal emitted when the spectrum is changed.
     */
    void change_spectrum( void );

    /*!
     * \brief Signal emitted to enable the accept button.
     */
    void accept_enable  ( void );

private:
    QPushButton* pb_view; /*!< View button. */
    QPushButton* pb_delete; /*!< Delete button. */
    QPushButton* pb_cancel; /*!< Cancel button. */
    QPushButton* pb_manual; /*!< Manual edit button. */
    QPushButton* pb_uploadDisk; /*!< Upload from disk button. */
    QPushButton* pb_uploadFit; /*!< Upload fit button. */

    QMap< double, double > loc_extinct; /*!< Local extinction map. */

private slots:

    /*!
     * \brief Slot to handle cancel action.
     */
    void cancel( void );

    /*!
     * \brief Slot to handle upload from disk action.
     */
    void uploadDisk( void );

    /*!
     * \brief Slot to handle upload fit action.
     */
    void uploadFit( void );

    /*!
     * \brief Slot to process results.
     * \param xyz The map of results.
     */
    void process_results( QMap < double, double > &xyz );

    /*!
     * \brief Slot to add spectrum from disk.
     */
    void add_spectrumDisk( void );

    /*!
     * \brief Slot to read spectra from file.
     * \param fileName The file name.
     */
    void readingspectra(const QString& fileName);

    /*!
     * \brief Slot to manually edit the spectrum.
     */
    void editmanually( void );

    /*!
     * \brief Slot to delete the spectrum.
     */
    void delete_spectrum( void );

    /*!
     * \brief Slot to view the spectrum.
     */
    void view_spectrum( void );
};

#endif
