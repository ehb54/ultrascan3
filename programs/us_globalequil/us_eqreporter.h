//! \file us_eqreporter.h
//! \brief Contains the declaration of the US_EqReporter class and its members.
#ifndef US_EQREPORTER_H
#define US_EQREPORTER_H

#include <QtCore>
#if QT_VERSION > 0x050000
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include "us_extern.h"
#include "us_dataIO.h"
#include "us_globeq_data.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

//! \class US_EqReporter
//! \brief A class for generating reports on equilibrium fitting in UltraScan.
class US_EqReporter : public QObject
{
    Q_OBJECT

    public:
        //! \brief Constructor for the US_EqReporter class.
        //! \param data_list Reference to the vector of edited data.
        //! \param scan_edits Reference to the vector of scan edits.
        //! \param scan_fits Reference to the vector of scan fits.
        //! \param run_fit Reference to the run fit parameters structure.
        //! \param parent Pointer to the parent widget.
        US_EqReporter(QVector< US_DataIO::EditedData >& data_list,
                      QVector< ScanEdit >& scan_edits, QVector< EqScanFit >& scan_fits, EqRunFit& run_fit,
                      QWidget* parent = 0);

        //! \brief Perform scan diagnostics.
        void scan_diagnostics();

        //! \brief Check the fit of a scan.
        //! \param scan_index The index of the scan to check.
        //! \return True if the fit is good, false otherwise.
        bool check_scan_fit(int scan_index);

        //! \brief Generate a fit report.
        //! \param fit_params Reference to the fit control parameters.
        //! \param include_scan_info Boolean to include scan info in the report.
        //! \param include_params Boolean to include parameters in the report.
        //! \param report String to hold the generated report.
        //! \return The fit report as a string.
        QString fit_report(FitCtrlPar& fit_params, bool include_scan_info, bool include_params, QString& report);

    private:
        QVector< US_DataIO::EditedData >&   dataList;  //!< Reference to the vector of edited data.
        QVector< ScanEdit >&                scedits;   //!< Reference to the vector of scan edits.
        QVector< EqScanFit >&               scanfits;  //!< Reference to the vector of scan fits.
        EqRunFit&                           runfit;    //!< Reference to the run fit parameters structure.
        QWidget*                            wparent;   //!< Pointer to the parent widget.

        int      dbg_level;  //!< Debug level.

        QString  asters;     //!< Asterisk string for formatting.

    private slots:
        //! \brief Get the index of a radius in edited data.
        //! \param edited_data Pointer to the edited data.
        //! \param radius The radius value.
        //! \return The index of the radius.
        int index_radius(US_DataIO::EditedData* edited_data, double radius);

        //! \brief Center a string in a line.
        //! \param text The text to center.
        //! \param width The width of the line.
        //! \param is_header Boolean indicating if the text is a header.
        //! \param fill_char The character to fill the line with.
        //! \return The centered text as a string.
        QString centerInLine(const QString& text, int width, bool is_header, const QChar fill_char);

        //! \brief Get the header information for a scan.
        //! \param scan_number The scan number.
        //! \param num_scans The total number of scans.
        //! \return The scan header information as a string.
        QString scanInfoHeader(int scan_number, int num_scans);

        //! \brief Get the maximum line width.
        //! \param fm Reference to the font metrics.
        //! \param text The text to measure.
        //! \return The maximum line width.
        int maxLineWidth(QFontMetrics& fm, const QString& text);
};

#endif
