//! \file us_export.h
//! \brief Contains the declaration of the US_ExportLegacy class and its members.
#ifndef US_EXPORT_LEGACY_H
#define US_EXPORT_LEGACY_H

#include "us_widgets.h"
#include "us_plot.h"
#include "us_dataIO.h"
#include "us_db2.h"
#include "us_editor.h"
#include "us_math2.h"
#include "us_help.h"
#include "us_run_details2.h"
#include <qwt_plot.h>
#include <qwt_plot_marker.h>

#ifndef DbgLv
//! \def DbgLv(a)
//! \brief Macro for debug level logging.
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

//! \class US_ExportLegacy
//! \brief A class for exporting legacy data in UltraScan.
class US_ExportLegacy : public US_Widgets
{
    Q_OBJECT

    public:
        //! \brief Constructor for the US_ExportLegacy class.
        US_ExportLegacy();

    private:
        QLineEdit* le_id; //!< Line edit for ID.
        QLineEdit* le_temp; //!< Line edit for temperature.

        QTextEdit* te_desc; //!< Text edit for description.
        QTextEdit* te_stat; //!< Text edit for status.

        QPushButton* pb_load; //!< Button to load data.
        QPushButton* pb_details; //!< Button to show details.
        QPushButton* pb_view; //!< Button to view report.
        QPushButton* pb_save; //!< Button to save data.
        QPushButton* pb_reset; //!< Button to reset the form.
        QPushButton* pb_help; //!< Button to show help.
        QPushButton* pb_close; //!< Button to close the dialog.

        QListWidget* lw_triples; //!< List widget for triples.

        int scanCount; //!< Count of scans.
        int valueCount; //!< Count of values.
        int dbg_level; //!< Debug level.
        int nexport; //!< Export count.

        bool dataLoaded; //!< Flag to indicate if data is loaded.

        QString run_name; //!< Run name.
        QString workingDir; //!< Working directory.
        QString runID; //!< Run ID.
        QString rawDtype; //!< Raw data type.
        QString rawChann; //!< Raw channel.
        QString rawCell; //!< Raw cell.
        QString rawWaveln; //!< Raw wavelength.

        QStringList files; //!< List of files.

        US_DataIO::EditedData* edata; //!< Pointer to edited data.
        US_DataIO::RawData* rdata; //!< Pointer to raw data.
        US_DataIO::Scan* dscan; //!< Pointer to scan data.

        US_Disk_DB_Controls* dkdb_cntrls; //!< Disk/DB controls.

    private slots:
        //! \brief Slot to load data.
        void load(void);

        //! \brief Slot to plot data.
        void data_plot(void);

        //! \brief Slot to export data.
        void export_data(void);

        //! \brief Slot to show details.
        void details(void);

        //! \brief Slot to reset the form.
        void reset(void);

        //! \brief Slot to update the form.
        //! \param value The update value.
        void update(int value);

        //! \brief Export most types of data.
        //! \param files The list of files to export.
        void exp_mosttypes(QStringList& files);

        //! \brief Export intensity data.
        //! \param files The list of files to export.
        void exp_intensity(QStringList& files);

        //! \brief Export interference data.
        //! \param files The list of files to export.
        void exp_interference(QStringList& files);

        //! \brief Write a report to a text stream.
        //! \param ts The text stream to write to.
        void write_report(QTextStream& ts);

        //! \brief Update the disk/DB controls.
        //! \param db True if updating to DB, false for disk.
        void update_disk_db(bool db);

        //! \brief Create a directory.
        //! \param dir The directory path.
        //! \param subdir The subdirectory path.
        //! \return True if the directory was created, false otherwise.
        bool mkdir(QString& dir, QString& subdir);

        //! \brief Slot to handle new triple selection.
        //! \param index The index of the new triple.
        void new_triple(int index);

        //! \brief Get an indentation string.
        //! \param level The indentation level.
        //! \return The indentation string.
        QString indent(int level) const;

        //! \brief Slot to view a report.
        void view_report(void);

        //! \brief Generate a table row HTML string.
        //! \param label The row label.
        //! \param value The row value.
        //! \return The table row as an HTML string.
        QString table_row(const QString& label, const QString& value) const;

        //! \brief Generate an HTML header string.
        //! \param title The title of the header.
        //! \param subtitle The subtitle of the header.
        //! \param data The raw data pointer.
        //! \return The HTML header string.
        QString html_header(QString title, QString subtitle, US_DataIO::RawData* data);

        //! \brief Get data details as a string.
        //! \return The data details string.
        QString data_details(void);

        //! \brief Get the time correction value.
        //! \return The time correction value.
        double time_correction(void);

        //! \brief Get raw data strings.
        //! \param data The raw data pointer.
        //! \param runID The run ID.
        //! \param filename The file name.
        //! \param dataType The data type.
        //! \param comment The comment.
        void rDataStrings(US_DataIO::RawData* data, QString& runID, QString& filename, QString& dataType, QString& comment);

        //! \brief Get RI profile.
        //! \param profile The RI profile vector.
        //! \return The profile index.
        int getRIProfile(QVector<double>& profile);

        //! \brief Convert data to intensity.
        //! \param data The data vector.
        void convertToIntensity(QVector<double>& data);

        //! \brief Parse RI profile from a string.
        //! \param profileString The profile string.
        //! \param profile The RI profile vector.
        void parseRIProfile(QString& profileString, QVector<double>& profile);

        //! \brief Slot to show help documentation.
        void help(void)
        {
            showHelp.show_help("export_legacy.html");
        };

    protected:
        QStringList triples; //!< List of triples.
        QVector<US_DataIO::EditedData> dataList; //!< List of edited data.
        QVector<US_DataIO::RawData> rawList; //!< List of raw data.
        QVector<int> extrips; //!< Vector of extra triples.

        US_Help showHelp; //!< Help object.

        // Layouts
        QBoxLayout* mainLayout; //!< Main layout.
        QBoxLayout* leftLayout; //!< Left layout.
        QBoxLayout* rightLayout; //!< Right layout.
        QBoxLayout* buttonLayout; //!< Button layout.

        QGridLayout* analysisLayout; //!< Analysis layout.
        QGridLayout* runInfoLayout; //!< Run info layout.

        US_Plot* plotLayout2; //!< Plot layout.

        // Widgets
        QwtPlot* data_plot2; //!< Data plot.
};

#endif // US_EXPORT_LEGACY_H
