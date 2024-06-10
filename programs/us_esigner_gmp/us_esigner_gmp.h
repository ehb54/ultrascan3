//! \file us_esigner_gmp.h
//! \brief Contains the declaration of the US_eSignaturesGMP class and its members.

#ifndef US_ESIGNER_GMP_H
#define US_ESIGNER_GMP_H

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

//! \class US_eSignaturesGMP
//! \brief A class for managing electronic signatures for GMP compliance in UltraScan.
class US_eSignaturesGMP : public US_Widgets
{
    Q_OBJECT

    public:
        //! \brief Default constructor for the US_eSignaturesGMP class.
        US_eSignaturesGMP();

        //! \brief Constructor for the US_eSignaturesGMP class with a single string parameter.
        //! \param gmpRunID The GMP run ID.
        US_eSignaturesGMP(QString gmpRunID);

        //! \brief Constructor for the US_eSignaturesGMP class with a string list parameter.
        //! \param gmpRunIDs The list of GMP run IDs.
        US_eSignaturesGMP(QStringList gmpRunIDs);

        //! \brief Constructor for the US_eSignaturesGMP class with a map parameter.
        //! \param gmpDetails The map of GMP run details.
        US_eSignaturesGMP(QMap<QString, QString>& gmpDetails);

        bool auto_mode; //!< Flag for automatic mode.
        bool auto_separate_status; //!< Flag for automatic separate status.
        bool assign_revs_sep; //!< Flag for assigning reviewers separately.
        bool reassign_revs_sep; //!< Flag for reassigning reviewers separately.

        //! \class US_InvestigatorData
        //! \brief A class to hold investigator data.
        class US_InvestigatorData
        {
            public:
                int invID; //!< Unique ID in the DB for the entry.
                int ulev; //!< User level.
                QString gmpReviewerRole; //!< Reviewer or Approver role.
                QString invGuid; //!< Investigator's global ID.
                QString lastName; //!< Last name.
                QString firstName; //!< First name.
                QString address; //!< Investigator's address.
                QString city; //!< Investigator's city.
                QString state; //!< Investigator's state.
                QString zip; //!< Investigator's zip code.
                QString phone; //!< Investigator's phone number.
                QString email; //!< Investigator's email address.
                QString organization; //!< Investigator's organization.
        };

    private:
        QMap<QString, QString> gmp_run_details; //!< Map of GMP run details.
        QMap<QString, QString> eSign_details; //!< Map of eSign details.
        QMap<QString, QString> eSign_details_auto; //!< Map of eSign details for auto mode.
        QMap<QString, QString> it_details; //!< Map of IT details.
        QStringList reassign; //!< List of reassigned items.
        QList<US_InvestigatorData> investigators; //!< List of investigators.
        QList<US_InvestigatorData> g_reviewers; //!< List of global reviewers.
        QList<US_InvestigatorData> g_apprs; //!< List of global approvers.
        US_InvestigatorData info; //!< Investigator information.
        US_InvestigatorData info_grev; //!< Global reviewer information.
        US_InvestigatorData info_gappr; //!< Global approver information.

        // Widgets and layout members
        QListWidget* lw_inv_list; //!< List widget for investigators.
        QTextEdit* te_inv_smry; //!< Text edit for investigator summary.
        QLabel* lb_inv_search; //!< Label for investigator search.
        QLineEdit* le_inv_search; //!< Line edit for investigator search.

        QListWidget* lw_grev_list; //!< List widget for global reviewers.
        QTextEdit* te_grev_smry; //!< Text edit for global reviewer summary.
        QLabel* lb_grev_search; //!< Label for global reviewer search.
        QLineEdit* le_grev_search; //!< Line edit for global reviewer search.

        QListWidget* lw_gappr_list; //!< List widget for global approvers.
        QTextEdit* te_gappr_smry; //!< Text edit for global approver summary.
        QLabel* lb_gappr_search; //!< Label for global approver search.
        QLineEdit* le_gappr_search; //!< Line edit for global approver search.

        QPushButton* pb_set_global_rev; //!< Button to set global reviewer.
        QPushButton* pb_set_global_appr; //!< Button to set global approver.
        QPushButton* pb_unset_global_rev; //!< Button to unset global reviewer.
        QPushButton* pb_unset_global_appr; //!< Button to unset global approver.

        // Widgets for setting operators/reviewers for selected GMP Run
        QPushButton* pb_selRun_operRev_set; //!< Button to select run and set operators/reviewers.
        QPushButton* pb_set_operRev; //!< Button to set operators/reviewers.

        QPushButton* pb_add_oper; //!< Button to add operator.
        QPushButton* pb_remove_oper; //!< Button to remove operator.
        QPushButton* pb_add_rev; //!< Button to add reviewer.
        QPushButton* pb_remove_rev; //!< Button to remove reviewer.
        QPushButton* pb_add_appr; //!< Button to add approver.
        QPushButton* pb_remove_appr; //!< Button to remove approver.

        QLineEdit* le_run_name; //!< Line edit for run name.
        QLineEdit* le_optima_name; //!< Line edit for Optima name.

        QTextEdit* te_operator_names; //!< Text edit for operator names.
        QTextEdit* te_reviewer_names; //!< Text edit for reviewer names.
        QTextEdit* te_appr_names; //!< Text edit for approver names.
        QTextEdit* te_opers_to_assign; //!< Text edit for operators to assign.
        QTextEdit* te_revs_to_assign; //!< Text edit for reviewers to assign.
        QTextEdit* te_apprs_to_assign; //!< Text edit for approvers to assign.

        QComboBox* cb_choose_operator; //!< Combo box to choose operator.
        QComboBox* cb_choose_rev; //!< Combo box to choose reviewer.
        QComboBox* cb_choose_appr; //!< Combo box to choose approver.

        QList<QStringList> autoflowdata; //!< List of autoflow data.
        US_SelectItem* pdiag_autoflow; //!< Dialog for selecting autoflow.

        bool isEsignRecord; //!< Flag to indicate if it is an eSign record.

        // Widgets for downloading, reviewing, and e-signing GMP reports
        QPushButton* pb_loadreport_db; //!< Button to load report from DB.
        QPushButton* pb_view_report_db; //!< Button to view report from DB.
        QPushButton* pb_esign_report; //!< Button to e-sign the report.
        QPushButton* pb_view_eSigns; //!< Button to view eSignatures.
        QPushButton* pb_help; //!< Button to show help.
        QPushButton* pb_cancel; //!< Button to cancel the operation.

        QLineEdit* le_loaded_run_db; //!< Line edit for loaded run from DB.
        QTextEdit* te_fpath_info; //!< Text edit for file path information.
        QLineEdit* le_eSign_status; //!< Line edit for eSign status.

        US_SelectItem* pdiag_autoflow_db; //!< Dialog for selecting autoflow from DB.
        QList<QStringList> gmpReportsDBdata; //!< List of GMP reports from DB.
        QString folderRunName; //!< Folder run name.
        QString filePath_db; //!< File path from DB.
        QString filePath_db_html; //!< HTML file path from DB.
        QString filePath_eSign; //!< File path for eSign.
        QString gmpRunID_eSign; //!< GMP run ID for eSign.
        QString eSignID_global; //!< Global eSign ID.

        QVBoxLayout* topLayout_auto; //!< Top layout for auto mode.
        QVBoxLayout* mainLayout_auto; //!< Main layout for auto mode.
        QGridLayout* eSignersGrid_auto; //!< Grid layout for eSigners in auto mode.
        QGridLayout* eSignActionsGrid_auto; //!< Grid layout for eSign actions in auto mode.
        QVBoxLayout* leftLayout; //!< Left layout.
        QVBoxLayout* rghtLayout; //!< Right layout.
        QWidget* mainWidget_auto; //!< Main widget for auto mode.
        QHBoxLayout* lowerButtons; //!< Layout for lower buttons.
        QVBoxLayout* spacerLayout; //!< Layout for spacers.

        QString autoflowID_passed; //!< Passed autoflow ID.
        QString autoflowGMPReport_id_selected; //!< Selected autoflow GMP report ID.
        QMap<QString, QString> operators_info; //!< Map of operators' information.
        QMap<QString, QString> reviewers_info; //!< Map of reviewers' information.
        QMap<QString, QString> approvers_info; //!< Map of approvers' information.

    public slots:

    private slots:
        //! \brief Initialize the auto panel.
        //! \param details The details map for initialization.
        void initPanel_auto(QMap<QString, QString>& details);

        //! \brief Display reviewers for auto mode.
        //! \param index The index of the reviewer.
        //! \param details The details of the reviewers.
        //! \param role The role of the reviewer.
        void display_reviewers_auto(int& index, QMap<QString, QString> details, QString role);

        //! \brief Initialize investigators.
        void init_invs(void);

        //! \brief Initialize global reviewers.
        void init_grevs(void);

        //! \brief Initialize global approvers.
        void init_gapprs(void);

        //! \brief Limit investigator names based on input.
        //! \param name The input name to limit by.
        void limit_inv_names(const QString& name);

        //! \brief Limit global reviewer names based on input.
        //! \param name The input name to limit by.
        void limit_grev_names(const QString& name);

        //! \brief Limit global approver names based on input.
        //! \param name The input name to limit by.
        void limit_gappr_names(const QString& name);

        //! \brief Get investigator data from a list item.
        //! \param item The list widget item.
        void get_inv_data(QListWidgetItem* item);

        //! \brief Get global reviewer data from a list item.
        //! \param item The list widget item.
        void get_grev_data(QListWidgetItem* item);

        //! \brief Get global approver data from a list item.
        //! \param item The list widget item.
        void get_gappr_data(QListWidgetItem* item);

        //! \brief Get summary for investigator or global reviewer.
        //! \param data The investigator data.
        //! \param role The role of the investigator.
        //! \return The summary as a string.
        QString get_inv_or_grev_smry(US_InvestigatorData data, QString role);

        //! \brief Set the global reviewer.
        void set_greviewer(void);

        //! \brief Set the global approver.
        void set_gappr(void);

        //! \brief Unset the global reviewer.
        void unset_greviewer(void);

        //! \brief Unset the global approver.
        void unset_gappr(void);

        //! \brief Select a GMP run.
        void selectGMPRun(void);

        //! \brief Select a GMP run for separate assignment.
        void selectGMPRun_sa(void);

        //! \brief Reset the panel for setting reviewer/operator.
        void reset_set_revOper_panel(void);

        //! \brief Set the GUI for the reviewer/operator panel.
        void set_revOper_panel_gui(void);

        //! \brief Set the GUI for the reviewer/operator panel for separate assignment.
        void set_revOper_panel_gui_sa(void);

        //! \brief List all autoflow records.
        //! \param records The list of autoflow records.
        //! \param query The query string.
        //! \return The number of records found.
        int list_all_autoflow_records(QList<QStringList>& records, QString query);

        //! \brief Read an autoflow record.
        //! \param index The index of the record.
        //! \param autoflowID The ID of the autoflow.
        //! \return The details of the autoflow record.
        QMap<QString, QString> read_autoflow_record(int index, QString autoflowID);

        //! \brief Read operators from a file.
        //! \param filepath The path to the file.
        //! \return The list of operators.
        QStringList read_operators(QString filepath);

        //! \brief Read an autoflow GMP report eSign record.
        //! \param reportID The ID of the report.
        //! \return The details of the eSign record.
        QMap<QString, QString> read_autoflowGMPReportEsign_record(QString reportID);

        //! \brief Get assigned operators and reviewers from a JSON document.
        //! \param doc The JSON document.
        //! \return The list of assigned operators and reviewers as a string.
        QString get_assigned_oper_revs(QJsonDocument doc);

        //! \brief Get assigned operators and reviewers for separate assignment from a JSON document.
        //! \param runID The run ID.
        //! \param doc The JSON document.
        //! \param details The details map.
        //! \return The list of assigned operators and reviewers as a string.
        QString get_assigned_oper_revs_sa(QString runID, QJsonDocument doc, QMap<QString, QString> details);

        //! \brief Assign operators and reviewers.
        void assignOperRevs(void);

        //! \brief Assign operators and reviewers for separate assignment.
        void assignOperRevs_sa(void);

        //! \brief Add an operator to the list.
        void addOpertoList(void);

        //! \brief Remove an operator from the list.
        void removeOperfromList(void);

        //! \brief Add a reviewer to the list.
        void addRevtoList(void);

        //! \brief Remove a reviewer from the list.
        void removeRevfromList(void);

        //! \brief Add an approver to the list.
        void addApprtoList(void);

        //! \brief Remove an approver from the list.
        void removeApprfromList(void);

        //! \brief Check if the eSign process has begun.
        //! \return True if the eSign process has begun, false otherwise.
        bool is_eSignProcessBegan(void);

        //! \brief Disable set/unset buttons for global reviewers/approvers.
        void disableSetUnsetGButtons(void);

        //! \brief Set or unset the operator/reviewer buttons.
        void setUnsetPb_operRev(void);

        //! \brief Set or unset the add/remove reviewer/approver buttons.
        //! \param role The role (Reviewer/Approver).
        void setUnset_AddRemove_RevAppr_bttn(QString role);

        //! \brief Compose an updated admin log JSON.
        //! \param adminID The admin ID.
        //! \param date The date.
        //! \param runID The run ID.
        //! \param operID The operator ID.
        //! \param revID The reviewer ID.
        //! \param apprID The approver ID.
        //! \param status The status.
        //! \param comments The comments.
        //! \param sigID The signature ID.
        //! \return The composed JSON string.
        QString compose_updated_admin_logJson(int adminID, QString date, QString runID, QString operID,
                                              QString revID, QString apprID, QString status, QString comments, QString sigID);

        //! \brief Load a GMP report from the database.
        void loadGMPReportDB_assigned(void);

        //! \brief Load a GMP report from the database for auto mode.
        //! \param runID The run ID.
        void loadGMPReportDB_assigned_auto(QString runID);

        //! \brief Load a GMP report from the database for separate assignment.
        void loadGMPReportDB_assigned_separate(void);

        //! \brief Compose an updated ORA list.
        //! \param runID The run ID.
        //! \param oldORA The old ORA list.
        //! \param newORA The new ORA list.
        //! \param docType The document type.
        //! \param eSignType The eSign type.
        void compose_updated_ora_list(QString& runID, QString& oldORA, QString& newORA, QString docType, QString eSignType);

        //! \brief Parse an old ORA list.
        //! \param ora The ORA list.
        //! \return The parsed ORA list.
        QString parse_old_ora(QString ora);

        //! \brief Parse a new ORA list.
        //! \param ora The ORA list.
        //! \return The parsed ORA list.
        QString parse_new_ora(QString ora);

        //! \brief Compare two ORA lists.
        //! \param oldORA The old ORA list.
        //! \param newORA The new ORA list.
        //! \return The comparison result as a string.
        QString compare_ora_lists(QString oldORA, QString newORA);

        //! \brief List all GMP reports from the database.
        //! \param reports The list of reports.
        //! \param db The database connection.
        //! \return The number of reports found.
        int list_all_gmp_reports_db(QList<QStringList>& reports, US_DB2* db);

        //! \brief Remove files by mask.
        //! \param dir The directory.
        //! \param masks The list of masks.
        void remove_files_by_mask(QString dir, QStringList masks);

        //! \brief View a report from the database.
        void view_report_db(void);

        //! \brief View eSignatures.
        void view_eSignatures(void);

        //! \brief Create a directory.
        //! \param dir The directory path.
        //! \param subdir The subdirectory name.
        //! \return True if the directory was created successfully, false otherwise.
        bool mkdir(const QString& dir, const QString& subdir);

        //! \brief eSign a report.
        void esign_report(void);

        //! \brief Compose an updated eSign JSON.
        //! \param adminID The admin ID.
        //! \param date The date.
        //! \param runID The run ID.
        //! \param revIDs The reviewer IDs.
        //! \param apprIDs The approver IDs.
        //! \param status The status.
        //! \param eSignData The eSign data.
        //! \return The composed JSON string.
        QString compose_updated_eSign_Json(int adminID, QString date, QString runID, QJsonArray revIDs, QJsonArray apprIDs,
                                           QString status, QString& eSignData);

        //! \brief Compose an updated eSign JSON for separate assignment.
        //! \param adminID The admin ID.
        //! \param date The date.
        //! \param runID The run ID.
        //! \param eSignData The eSign data.
        //! \param status The status.
        //! \return The composed JSON string.
        QString compose_updated_eSign_Json_sa(int adminID, QString date, QString runID, QString eSignData, QJsonArray status);

        //! \brief Write a PDF of eSignatures.
        //! \param runID The run ID.
        //! \param eSignID The eSign ID.
        //! \param docType The document type.
        //! \param eSignData The eSign data.
        //! \param status The status.
        //! \return The file path of the written PDF.
        QString write_pdf_eSignatures(QString runID, QString eSignID, QString docType, QString eSignData, QString status);

        //! \brief Check the eSign status for a GMP report.
        //! \return The eSign status as a string.
        QString check_eSign_status_for_gmpReport(void);

        //! \brief Check the eSign status for reviewers in separate assignment.
        //! \param runID The run ID.
        //! \param reviewers The map of reviewers.
        //! \return The eSign status as a string.
        QString check_revs_esign_status_sa(QString runID, QMap<QString, QString> reviewers);

        //! \brief Check the eSign status for a GMP report in auto mode.
        //! \param runID The run ID.
        //! \param details The details map.
        //! \return A pointer to the line edit containing the eSign status.
        QLineEdit* check_eSign_status_for_gmpReport_auto(QString runID, QMap<QString, QString> details);

        //! \brief Write and download eSignatures from the database.
        //! \param runID The run ID.
        //! \param eSignID The eSign ID.
        void write_download_eSignatures_DB(QString runID, QString eSignID);

        //! \brief Reset the eSign panel.
        void reset_esign_panel(void);

        //! \brief Paint a page with the given parameters.
        //! \param printer The printer object.
        //! \param pageNumber The page number.
        //! \param pageCount The total number of pages.
        //! \param painter The painter object.
        //! \param doc The text document.
        //! \param textRect The text rectangle.
        //! \param footerHeight The height of the footer.
        //! \param details The map of details.
        void paintPage(QPrinter& printer, int pageNumber, int pageCount,
                       QPainter* painter, QTextDocument* doc,
                       const QRectF& textRect, qreal footerHeight, QMap<QString, QMap<QString, QString>> details);

        //! \brief Print a document with the given parameters.
        //! \param printer The printer object.
        //! \param doc The text document.
        //! \param details The map of details.
        void printDocument(QPrinter& printer, QTextDocument* doc, QMap<QString, QMap<QString, QString>> details);

        //! \brief Convert millimeters to pixels.
        //! \param printer The printer object.
        //! \param mm The millimeters.
        //! \return The corresponding pixels.
        double mmToPixels(QPrinter& printer, int mm);

        //! \brief Convert a JSON string to a QMap.
        //! \param json The JSON string.
        //! \return The converted QMap.
        QMap<QString, QMap<QString, QString>> json_to_qmap(QString json);

        //! \brief Get the role by name.
        //! \param name The name of the role.
        //! \return The role as a string.
        QString get_role_by_name(QString name);

    signals:
        //! \brief Signal emitted to accept reviewers.
        //! \param details The details map.
        void accept_reviewers(QMap<QString, QString>& details);

        //! \brief Signal emitted to cancel reviewers.
        //! \param details The details map.
        void cancel_reviewers(QMap<QString, QString>& details);
};

#endif // US_ESIGNER_GMP_H
