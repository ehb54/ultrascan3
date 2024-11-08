//! \file us_audittrail_gmp.h
//! \brief Contains the declaration of the US_auditTrailGMP class and its members.
#ifndef US_AUDITTRAIL_GMP_H
#define US_AUDITTRAIL_GMP_H

#include <QPrinter>

#include "us_widgets.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_run_protocol.h"
#include "us_protocol_util.h"
#include "us_solution.h"
#include "us_help.h"
#include "us_extern.h"
#include "us_select_item.h"

//! \class US_auditTrailGMP
//! \brief A class for handling the GMP audit trail functionality.
class US_auditTrailGMP : public US_Widgets
{
    Q_OBJECT

    public:
        //! \brief Constructor for the US_auditTrailGMP class.
        US_auditTrailGMP();

    private:
        QVBoxLayout* topLayout_auto; //!< Top layout for the auto section.
        QPushButton* pb_loadreport_db; //!< Button to load report from database.
        QPushButton* pb_viewAPDF; //!< Button to view a PDF.

        QVBoxLayout* mainLayout_auto; //!< Main layout for the auto section.
        QGridLayout* loadedRunGrid; //!< Grid layout for loaded runs.
        QGridLayout* eSignersGrid; //!< Grid layout for eSigners.
        QGridLayout* userInteractions; //!< Grid layout for user interactions.
        QGridLayout* lowerButtons; //!< Grid layout for lower buttons.

        QVBoxLayout* leftLayout; //!< Left layout.
        QVBoxLayout* rghtLayout; //!< Right layout.
        QWidget* mainWidget_auto; //!< Main widget for the auto section.

        QVector<QGridLayout*> gridLayoutVector; //!< Vector of grid layouts.

        QTreeWidget* eSignTree; //!< Tree widget for eSignatures.
        QTreeWidget* uInteractionsTree; //!< Tree widget for user interactions.

        US_SelectItem* pdiag_autoflow_db; //!< Dialog for selecting autoflow from database.
        QString autoflowID_passed; //!< Passed autoflow ID.
        QString gmpRunName_passed; //!< Passed GMP run name.
        int autoflowStatusID; //!< Status ID of the autoflow.
        QList<QStringList> gmpReportsDBdata; //!< Data of GMP reports from database.
        bool isEsignRecord; //!< Flag for eSign record.
        QString gmpRunID_eSign; //!< GMP run ID for eSign.
        QString AProfileGUID; //!< Analysis profile GUID.
        QMap<QString, QString> eSign_details_auto; //!< Details of eSign for auto section.
        bool p_2dsa_run_fitmen; //!< Flag for 2DSA run fitmen.
        bool p_2dsa_auto_fitmen; //!< Flag for 2DSA auto fitmen.
        QString dataSource;

        int RowHeight; //!< Height of the row.
        QString html_assembled; //!< Assembled HTML content.
        QString html_assembled_esigs; //!< Assembled HTML content for eSignatures.
        QString filePath_pdf; //!< File path for the PDF.

    public slots:

    private slots:
        //! \brief Load the GMP report.
        void loadGMPReport(void);

        //! \brief Print a PDF.
        void printAPDF(void);

        //! \brief View a PDF.
        void viewAPDF(void);

        //! \brief Create a directory.
        //! \param path The path of the directory.
        //! \param dirname The name of the directory.
        //! \return True if the directory was created successfully, false otherwise.
        bool mkdir(const QString& path, const QString& dirname);

        //! \brief List all GMP reports from the database.
        //! \param data The list to store the GMP report data.
        //! \param db The database connection.
        //! \return The number of reports listed.
        int list_all_gmp_reports_db(QList<QStringList>& data, US_DB2* db);

        //! \brief Read the autoflow GMP report eSign record.
        //! \param id The ID of the eSign record.
        //! \return A map containing the details of the eSign record.
        QMap<QString, QString> read_autoflowGMPReportEsign_record(QString id);

        //! \brief Initialize the auto panel.
        //! \param details The details map for initialization.
        void initPanel_auto(QMap<QString, QString>& details);

        //! \brief Reset the panel.
        void reset_panel(void);

        //! \brief Create a group box for eSign.
        //! \param title The title of the group box.
        //! \return The created group box.
        QGroupBox* createGroup_eSign(QString title);

        //! \brief Create group boxes for stages.
        //! \param title The title of the group boxes.
        //! \param subtitle The subtitle of the group boxes.
        //! \return A vector of the created group boxes.
        QVector<QGroupBox*> createGroup_stages(QString title, QString subtitle);

        //! \brief Display the reviewers for auto section.
        //! \param index The index of the reviewer.
        //! \param details The details of the reviewers.
        //! \param role The role of the reviewer.
        //! \param layout The layout to add the reviewers to.
        void display_reviewers_auto(int& index, QMap<QString, QString> details, QString role, QGridLayout* layout);

        //! \brief Read the autoflow record.
        //! \param id The ID of the autoflow record.
        //! \return A map containing the details of the autoflow record.
        QMap<QString, QString> read_autoflow_record(int id);

        //! \brief Check the eSign status for GMP report in auto section.
        //! \param id The ID of the eSign record.
        //! \param details The details map for eSign.
        //! \return The QLineEdit with the eSign status.
        QLineEdit* check_eSign_status_for_gmpReport_auto(QString id, QMap<QString, QString> details);

        //! \brief Read the autoflow status record.
        //! \param status The status of the autoflow.
        //! \param runID The run ID.
        //! \param protocolName The protocol name.
        //! \param investigatorName The investigator name.
        //! \param expType The experiment type.
        //! \param date The date.
        //! \param filename The filename.
        //! \param projectName The project name.
        //! \param solutionName The solution name.
        //! \param bufferName The buffer name.
        //! \param analyteName The analyte name.
        //! \param gradientName The gradient name.
        //! \param hardwareName The hardware name.
        //! \param aprofileName The analysis profile name.
        //! \param aprofileGUID The analysis profile GUID.
        //! \param dataSource The data source.
        void read_autoflowStatus_record(QString& status, QString& runID, QString& protocolName, QString& investigatorName,
                                        QString& expType, QString& date, QString& filename, QString& projectName,
                                        QString& solutionName, QString& bufferName, QString& analyteName,
                                        QString& gradientName, QString& hardwareName, QString& aprofileName,
                                        QString& aprofileGUID, QString& dataSource);

        //! \brief Parse the autoflow status JSON.
        //! \param json The JSON string.
        //! \param key The key for the JSON.
        //! \return A map of the parsed autoflow status.
        QMap<QString, QMap<QString, QString>> parse_autoflowStatus_json(const QString json, const QString key);

        //! \brief Parse the autoflow status analysis JSON.
        //! \param json The JSON string.
        //! \return A map of the parsed analysis status.
        QMap<QString, QString> parse_autoflowStatus_analysis_json(QString json);

        //! \brief Read the report lists from the analysis profile.
        //! \param reportList The list of reports.
        //! \param methodList The list of methods.
        void read_reportLists_from_aprofile(QStringList& reportList, QStringList& methodList);

        //! \brief Read the report lists from XML.
        //! \param xml The XML stream reader.
        //! \param reports The map to store the report details.
        //! \param methods The map to store the method details.
        //! \return True if the report lists were read successfully, false otherwise.
        bool readReportLists(QXmlStreamReader& xml, QMap<QString, QString>& reports, QMap<QString, QString>& methods);

        //! \brief Read the 2DSA settings from XML.
        //! \param xml The XML stream reader.
        //! \return True if the 2DSA settings were read successfully, false otherwise.
        bool read_2dsa_settings(QXmlStreamReader& xml);

        //! \brief Build the list of dropped triples.
        //! \param db The database connection.
        //! \param details The details map.
        //! \return The list of dropped triples.
        QStringList buildDroppedTriplesList(US_DB2* db, QMap<QString, QString> details);

        //! \brief Assemble the initial GMP data.
        //! \param statusMap The map of the status.
        //! \param runID The run ID.
        void assemble_GMP_init(QMap<QString, QMap<QString, QString>> statusMap, QString runID);

        //! \brief Assemble the live update for GMP.
        //! \param statusMap The map of the status.
        //! \param runID The run ID.
        //! \param status The status of the GMP.
        void assemble_GMP_live_update(QMap<QString, QMap<QString, QString>> statusMap, QString runID, QString status);

        //! \brief Assemble the GMP import data.
        //! \param statusMap The map of the status.
        //! \param runID The run ID.
        //! \param status The status of the GMP.
        void assemble_GMP_import(QMap<QString, QMap<QString, QString>> statusMap, QString runID, QString status);

        //! \brief Assemble the GMP editing data.
        //! \param statusMap The map of the status.
        //! \param runID The run ID.
        //! \param status The status of the GMP.
        void assemble_GMP_editing(QMap<QString, QMap<QString, QString>> statusMap, QString runID, QString status);

        //! \brief Assemble the GMP analysis fitmen data.
        //! \param details The details map.
        void assemble_GMP_analysis_fitmen(QMap<QString, QString> details);

        //! \brief Assemble the GMP analysis cancelled data.
        //! \param details The details map.
        //! \param status The status of the GMP.
        void assemble_GMP_analysis_cancelled(QMap<QString, QString> details, QString status);

        //! \brief Assemble the GMP eSign reassign data.
        //! \param statusMap The map of the status.
        void assemble_GMP_esign_reassign(QMap<QString, QMap<QString, QString>> statusMap);

        //! \brief Assemble the eSignatures.
        //! \param details The details map.
        void assemble_esigs(QMap<QString, QString> details);

        //! \brief Initialize the HTML content.
        void initHTML(void);

        //! \brief Close the HTML content.
        void closeHTML(void);

    signals:
};

#endif // US_AUDITTRAIL_GMP_H
