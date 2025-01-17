//! \file us_protocol_dev_gui.h
//! \brief This file contains the declaration of the classes for the US Protocol Development GUI.
#ifndef US_PROTODEV_H
#define US_PROTODEV_H

#include <QApplication>
#include <unistd.h>
#include <fstream>
#include <QtSql>

#include "../us_xpn_viewer/us_xpn_viewer_gui.h"
#include "../us_experiment/us_experiment_gui_optima.h"
#include "../us_convert/us_experiment.h"
#include "../us_convert/us_experiment_gui.h"
#include "../us_convert/us_convert_gui.h"
#include "../us_convert/us_convertio.h"
#include "../us_convert/us_get_run.h"
#include "../us_convert/us_intensity.h"
#include "../us_convert/us_selectbox.h"
#include "../us_convert/us_select_triples.h"

#include "../us_edit/us_edit.h"
#include "../us_edit/us_edit_scan.h"
#include "../us_edit/us_exclude_profile.h"
#include "../us_edit/us_get_edit.h"
#include "../us_edit/us_ri_noise.h"
#include "../us_edit/us_select_lambdas.h"

#include "../us_autoflow_analysis/us_autoflow_analysis.h"
#include "../us_reporter_gmp/us_reporter_gmp.h"
#include "../us_esigner_gmp/us_esigner_gmp.h"

#include "us_protocol_util.h"
#include "us_project_gui.h"
#include "us_editor.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_investigator.h"
#include "us_util.h"
#include "us_rotor.h"
#include "us_solution.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_hardware.h"
#include "us_select_runs.h"
#include "us_plot.h"
#include "us_select_item.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_local_server.h"
#include "us_report_gmp.h"
#include "us_report_gui.h"

class US_ProtocolDevMain;

/**
 * \class VerticalTabStyle
 * \brief Class representing a custom style for vertical tabs.
 */
class VerticalTabStyle : public QProxyStyle {
    public:
        /**
         * \brief Calculates the size of the contents for the given type.
         * \param type The type of contents.
         * \param option The style options.
         * \param size The size of the widget.
         * \param widget The widget for which the size is being calculated.
         * \return The size of the contents.
         */
        QSize sizeFromContents(ContentsType type, const QStyleOption* option,
                               const QSize& size, const QWidget* widget) const {
            QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
            if (type == QStyle::CT_TabBarTab) {
                s.transpose();
            }
            return s;
        }

        /**
         * \brief Draws the control element for the given options.
         * \param element The control element to be drawn.
         * \param option The style options.
         * \param painter The painter used for drawing.
         * \param widget The widget for which the control element is being drawn.
         */
        void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const {
            if (element == CE_TabBarTabLabel) {
                if (const QStyleOptionTab* tab = qstyleoption_cast<const QStyleOptionTab*>(option)) {
                    QStyleOptionTab opt(*tab);
                    opt.shape = QTabBar::RoundedNorth;
                    QProxyStyle::drawControl(element, &opt, painter, widget);
                    return;
                }
            }
            QProxyStyle::drawControl(element, option, painter, widget);
        }
};

//! \class US_InitDialogueGui
//! \brief Class representing the initial panel in the protocol development GUI.
class US_InitDialogueGui : public US_WidgetsDialog
{
   Q_OBJECT

    public:
        /**
         * \brief Constructor for the initial dialogue GUI.
         * \param parent The parent widget.
         */
        US_InitDialogueGui( QWidget* parent );

        /**
         * \brief Destructor for the initial dialogue GUI.
         */
        ~US_InitDialogueGui() {};

        QList< QStringList > autoflowdata; //!< Autoflow data
        QList< QStringList > autoflowdataHistory; //!< Autoflow data history
        QStringList occupied_instruments; //!< List of occupied instruments
        US_SelectItem* pdiag_autoflow; //!< Autoflow selection dialog
        US_SelectItem* pdiag_autoflowHistory; //!< Autoflow history selection dialog

        /**
         * \brief Initializes the records dialogue.
         */
        void initRecordsDialogue( void );

        /**
         * \brief Initializes the autoflow panel.
         */
        void initAutoflowPanel( void );

        QLabel* movie_label; //!< Label for movie

        QMessageBox * msg_norec; //!< Message box for no record
        QMessageBox * msg_norec_del; //!< Message box for no record deletion

        bool initDialogueOpen; //!< Flag for dialogue open
        bool initMsgNorecOpen; //!< Flag for no record message open
        bool initMsgNorecDelOpen; //!< Flag for no record deletion message open

    private:
        US_ProtocolDevMain* mainw; //!< Parent to all panels
        int offset; //!< Offset

        int autoflow_records; //!< Autoflow records count
        QMap< QString, QString > channels_report; //!< Channels report

        /**
         * \brief Initializes the records.
         */
        void initRecords( void );

        /**
         * \brief Gets the autoflow records.
         * \return The number of autoflow records.
         */
        int get_autoflow_records( void );

        /**
         * \brief Reads the autoflow record.
         * \param index The index of the record.
         * \param status The status of the record.
         * \return The autoflow record as a QMap.
         */
        QMap< QString, QString > read_autoflow_record( int index, QString status );

        /**
         * \brief Reads the failed autoflow record.
         * \param status The status of the record.
         * \return The failed autoflow record as a QMap.
         */
        QMap< QString, QString > read_autoflow_failed_record( QString status );

        /**
         * \brief Lists all autoflow records.
         * \param records The list of records.
         * \param db The database connection.
         * \param status The status of the records.
         * \return The number of records listed.
         */
        static int list_all_autoflow_records( QList< QStringList >&, US_DB2*, QString );

        /**
         * \brief Reads the Optima machines.
         * \param db The database connection.
         */
        void read_optima_machines( US_DB2* db = 0 );
        QList< QMap<QString, QString> > instruments; //!< List of instruments

        /**
         * \brief Loads the autoflow history dialog.
         */
        void load_autoflowHistory_dialog( void );

        /**
         * \brief Cleans up the run tables.
         * \param run_table The run table to clean up.
         */
        void do_run_tables_cleanup( QMap< QString, QString > run_table );

        /**
         * \brief Cleans up the run data.
         * \param run_data The run data to clean up.
         */
        void do_run_data_cleanup( QMap< QString, QString > run_data );
  
        void do_create_autoflowStatus_for_failedRun( QMap< QString, QString > );
  
        /**
         * \brief Reads the basic parameters of the profile automatically.
         * \param xml_reader The XML stream reader.
         * \return True if successful, false otherwise.
         */
        bool readAProfileBasicParms_auto ( QXmlStreamReader& xml_reader );

    protected:
        /**
         * \brief Handles the resize event.
         * \param event The resize event.
         */
        void resizeEvent(QResizeEvent *event) override;

    private slots:
                /**
                 * \brief Updates the autoflow data.
                 */
                void update_autoflow_data( void );

        signals:
                /**
                 * \brief Signal to define a new experiment.
                 * \param protocol_details The protocol details.
                 */
                void define_new_experiment_init ( QMap < QString, QString > & protocol_details );

            /**
             * \brief Signal to switch to live update.
             * \param protocol_details The protocol details.
             */
            void switch_to_live_update_init(  QMap < QString, QString > & protocol_details );

            /**
             * \brief Signal to switch to post-processing.
             * \param protocol_details The protocol details.
             */
            void switch_to_post_processing_init(  QMap < QString, QString > & protocol_details );

            /**
             * \brief Signal to switch to editing.
             * \param protocol_details The protocol details.
             */
            void switch_to_editing_init(  QMap < QString, QString > & protocol_details );

            /**
             * \brief Signal to switch to analysis.
             * \param protocol_details The protocol details.
             */
            void switch_to_analysis_init(  QMap < QString, QString > & protocol_details );

            /**
             * \brief Signal to switch to report.
             * \param protocol_details The protocol details.
             */
            void switch_to_report_init(  QMap < QString, QString > & protocol_details );

            /**
             * \brief Signal to initialize autoflow.
             */
            void to_initAutoflow( void );
};

//! \class US_ExperGui
//! \brief Class representing the experiment panel in the protocol development GUI.
class US_ExperGui : public US_WidgetsDialog
{
    Q_OBJECT

    public:
        /**
         * \brief Constructor for the experiment GUI.
         * \param parent The parent widget.
         */
        US_ExperGui( QWidget* parent );

        /**
         * \brief Destructor for the experiment GUI.
         */
        ~US_ExperGui() {};

    private:
        US_ProtocolDevMain* mainw; //!< Parent to all panels

        QLabel* lb_exp_banner; //!< Label for experiment banner
        QPushButton* pb_openexp; //!< Button to open experiment
        QLineEdit* opening_msg; //!< Line edit for opening message
        US_ExperimentMain* sdiag; //!< Experiment main dialog
        int offset; //!< Offset

    protected:
        /**
         * \brief Handles the resize event.
         * \param event The resize event.
         */
        void resizeEvent(QResizeEvent *event) override;

    private slots:
        /**
         * \brief Manages the experiment.
         */
        void manageExperiment ( void );

        /**
         * \brief Sets the button when experiment is closed.
         */
        void us_exp_is_closed_set_button( void );

        /**
         * \brief Switches to live update.
         * \param protocol_details The protocol details.
         */
        void to_live_update( QMap < QString, QString > & protocol_details );

        /**
         * \brief Clears the experiment.
         */
        void exp_cleared( void );

        /**
         * \brief Passes the used instruments.
         * \param used_instruments The used instruments.
         */
        void pass_used_instruments( QMap < QString, QString > & used_instruments );

        /**
         * \brief Closes the experiment setup message.
         */
        void expsetup_msg_closed( void );

        signals:
            /**
             * \brief Signal to switch to live update.
             * \param protocol_details The protocol details.
             */
            void switch_to_live_update( QMap < QString, QString > & protocol_details );

            /**
             * \brief Signal to set auto mode.
             */
            void set_auto_mode( void );

            /**
             * \brief Signal to reset the experiment.
             * \param protocolName The protocol name.
             */
            void reset_experiment( QString & protocolName);

            /**
             * \brief Signal to switch to autoflow records.
             */
            void to_autoflow_records( void );

            /**
             * \brief Signal to define used instruments.
             * \param used_instruments The used instruments.
             */
            void define_used_instruments( QMap < QString, QString > & used_instruments );
    };

//! \class US_ObservGui
//! \brief Class representing the observation panel in the protocol development GUI.
class US_ObservGui : public US_WidgetsDialog
{
    Q_OBJECT

    public:
        /**
         * \brief Constructor for the observation GUI.
         * \param parent The parent widget.
         */
        US_ObservGui( QWidget* parent );

        /**
         * \brief Destructor for the observation GUI.
         */
        ~US_ObservGui() {};

        US_XpnDataViewer* sdiag; //!< Xpn data viewer dialog

    private:
        US_ProtocolDevMain* mainw; //!< Parent to all panels
        int offset; //!< Offset

    protected:
        /**
         * \brief Handles the resize event.
         * \param event The resize event.
         */
        void resizeEvent(QResizeEvent *event) override;

    private slots:
        /**
         * \brief Processes the protocol details.
         * \param protocol_details The protocol details.
         */
        void process_protocol_details( QMap < QString, QString > & protocol_details );

        /**
         * \brief Switches to post-processing.
         * \param protocol_details The protocol details.
         */
        void to_post_processing( QMap < QString, QString > & protocol_details );

        /**
         * \brief Closes the program.
         */
        void to_close_program( void );

        /**
         * \brief Resets live update.
         */
        void reset_live_update( void );

        /**
         * \brief Passes the processes stopped signal.
         */
        void processes_stopped_passed( void );

        /**
         * \brief Initializes the autoflow in Xpn viewer.
         */
        void to_initAutoflow_xpnviewer ( void );

        signals:
                /**
                 * \brief Signal to switch to Xpn viewer.
                 * \param protocol_details The protocol details.
                 */
                void to_xpn_viewer( QMap < QString, QString > & protocol_details );

        /**
         * \brief Signal to switch to post-processing.
         * \param protocol_details The protocol details.
         */
        void switch_to_post_processing( QMap < QString, QString > & protocol_details );

        /**
         * \brief Signal to close everything.
         */
        void close_everything( void );

        /**
         * \brief Signal to reset live update.
         */
        void reset_live_update_passed( void );

        /**
         * \brief Signal to indicate processes stopped.
         */
        void processes_stopped( void );

        /**
         * \brief Signal to stop no data.
         */
        void stop_nodata( void );
};

//! \class US_PostProdGui
//! \brief Class representing the post-production panel in the protocol development GUI.
class US_PostProdGui : public US_WidgetsDialog
{
    Q_OBJECT

    public:
        /**
         * \brief Constructor for the post-production GUI.
         * \param parent The parent widget.
         */
        US_PostProdGui( QWidget* parent );

        /**
         * \brief Destructor for the post-production GUI.
         */
        ~US_PostProdGui() {};

    private:
        US_ProtocolDevMain* mainw; //!< Parent to all panels
        US_ConvertGui* sdiag; //!< Convert GUI dialog
        int offset; //!< Offset

    protected:
        /**
         * \brief Handles the resize event.
         * \param event The resize event.
         */
        void resizeEvent(QResizeEvent *event) override;

    private slots:
        /**
         * \brief Imports data using the convert GUI.
         * \param protocol_details The protocol details.
         */
        void import_data_us_convert( QMap < QString, QString > & protocol_details );

        /**
         * \brief Switches to editing.
         * \param protocol_details The protocol details.
         */
        void to_editing( QMap < QString, QString > & protocol_details );

        /**
         * \brief Switches to initialization autoflow.
         */
        void to_initAutoflow( void );

        /**
         * \brief Resets the LIMS import.
         */
        void reset_lims_import( void );

        /**
         * \brief Resizes the main window.
         */
        void resize_main( void );

        signals:
            /**
             * \brief Signal to switch to post-production.
             * \param protocol_details The protocol details.
             */
            void to_post_prod( QMap < QString, QString > & protocol_details );

            /**
             * \brief Signal to switch to editing.
             * \param protocol_details The protocol details.
             */
            void switch_to_editing( QMap < QString, QString > & protocol_details );

            /**
             * \brief Signal to switch to initialization autoflow.
             */
            void switch_to_initAutoflow( void );

            /**
             * \brief Signal to reset the LIMS import.
             */
            void reset_lims_import_passed( void );
};

//! \class US_EditingGui
//! \brief Class representing the editing panel in the protocol development GUI.
class US_EditingGui : public US_WidgetsDialog
{
    Q_OBJECT

    public:
        /**
         * \brief Constructor for the editing GUI.
         * \param parent The parent widget.
         */
        US_EditingGui( QWidget* parent );

        /**
         * \brief Destructor for the editing GUI.
         */
        ~US_EditingGui() {};

    private:
        US_ProtocolDevMain* mainw; //!< Parent to all panels
        US_Edit* sdiag; //!< Edit dialog
        int offset; //!< Offset

    protected:
        /**
         * \brief Handles the resize event.
         * \param event The resize event.
         */
        void resizeEvent(QResizeEvent *event) override;

    private slots:
        /**
         * \brief Performs editing.
         * \param protocol_details The protocol details.
         */
        void do_editing( QMap < QString, QString > & protocol_details );

        /**
         * \brief Resets data editing.
         */
        void reset_data_editing( void );

        /**
         * \brief Switches to analysis.
         * \param protocol_details The protocol details.
         */
        void to_analysis( QMap < QString, QString > & protocol_details );

        /**
         * \brief Resizes the main window.
         */
        void resize_main( void );

        /**
         * \brief Switches to initialization autoflow.
         */
        void to_initAutoflow( void );

        signals:
            /**
             * \brief Signal to start editing.
             * \param protocol_details The protocol details.
             */
            void start_editing( QMap < QString, QString > & protocol_details );

            /**
             * \brief Signal to reset data editing.
             */
            void reset_data_editing_passed ( void );

            /**
             * \brief Signal to switch to analysis.
             * \param protocol_details The protocol details.
             */
            void switch_to_analysis( QMap < QString, QString > & protocol_details );

            /**
             * \brief Signal to switch to initialization autoflow.
             */
            void switch_to_initAutoflow( void );
};

//! \class US_AnalysisGui
//! \brief Class representing the analysis panel in the protocol development GUI.
class US_AnalysisGui : public US_WidgetsDialog
{
    Q_OBJECT

    public:
        /**
         * \brief Constructor for the analysis GUI.
         * \param parent The parent widget.
         */
        US_AnalysisGui( QWidget* parent );

        /**
         * \brief Destructor for the analysis GUI.
         */
        ~US_AnalysisGui() {};

        US_Analysis_auto* sdiag; //!< Analysis auto dialog

    private:
        US_ProtocolDevMain* mainw; //!< Parent to all panels
        int offset; //!< Offset

    protected:
        /**
         * \brief Handles the resize event.
         * \param event The resize event.
         */
        void resizeEvent(QResizeEvent *event) override;

    private slots:
        /**
         * \brief Performs analysis.
         * \param protocol_details The protocol details.
         */
        void do_analysis( QMap < QString, QString > & protocol_details );

        /**
         * \brief Passes the processes stopped signal.
         */
        void processes_stopped_passed( void );

        /**
         * \brief Closes the analysis setup message.
         */
        void analysissetup_msg_closed( void );

        /**
         * \brief Switches to initialization autoflow.
         */
        void to_initAutoflow( void );

        /**
         * \brief Switches to report.
         * \param protocol_details The protocol details.
         */
        void to_report( QMap < QString, QString > & protocol_details );

        signals:
            /**
             * \brief Signal to start analysis.
             * \param protocol_details The protocol details.
             */
            void start_analysis( QMap < QString, QString > & protocol_details );

            /**
             * \brief Signal to indicate processes stopped.
             */
            void processes_stopped( void );

            /**
             * \brief Signal to switch to initialization autoflow.
             */
            void switch_to_initAutoflow( void );

            /**
             * \brief Signal to switch to report.
             * \param protocol_details The protocol details.
             */
            void switch_to_report( QMap < QString, QString > & protocol_details );
};

//! \class US_ReportStageGui
//! \brief Class representing the report panel in the protocol development GUI.
class US_ReportStageGui : public US_WidgetsDialog
{
    Q_OBJECT

    public:
        /**
         * \brief Constructor for the report stage GUI.
         * \param parent The parent widget.
         */
        US_ReportStageGui( QWidget* parent );

        /**
         * \brief Destructor for the report stage GUI.
         */
        ~US_ReportStageGui() {};

        US_ReporterGMP* sdiag; //!< Reporter GMP dialog

    private:
        US_ProtocolDevMain* mainw; //!< Parent to all panels
        int offset; //!< Offset

    protected:
        /**
         * \brief Handles the resize event.
         * \param event The resize event.
         */
        void resizeEvent(QResizeEvent *event) override;

    private slots:
                /**
                 * \brief Performs reporting.
                 * \param protocol_details The protocol details.
                 */
                void do_report( QMap < QString, QString > & protocol_details );

        /**
         * \brief Resets reporting.
         */
        void reset_reporting( void );

        signals:
                /**
                 * \brief Signal to start reporting.
                 * \param protocol_details The protocol details.
                 */
                void start_report( QMap < QString, QString > & protocol_details );

        /**
         * \brief Signal to reset reporting.
         */
        void reset_reporting_passed ( void );
};

//! \class US_ProtocolDevMain
//! \brief Class representing the main window for protocol development.
class US_ProtocolDevMain : public US_Widgets
{
    Q_OBJECT

    public:
        /**
         * \brief Constructor for the protocol development main window.
         */
        US_ProtocolDevMain();

        QLabel* gen_banner; //!< General banner label
        QTextEdit* welcome; //!< Welcome text edit
        QTabWidget* tabWidget; //!< Tab widget holding the panels
        QTextEdit* logWidget; //!< Log widget
        QTextEdit* test_footer; //!< Test footer
        QString icon_path; //!< Icon path

        QMessageBox * msg_expsetup; //!< Experiment setup message box
        QMessageBox * msg_liveupdate_finishing; //!< Live update finishing message box
        QMessageBox * msg_analysis_update_finishing; //!< Analysis update finishing message box
        QMessageBox * msg_analysissetup; //!< Analysis setup message box

        QDialog * diag_expsetup; //!< Experiment setup dialog

        void call_AutoflowDialogue();
        void close_initDialogue();

        bool window_closed; //!< Flag for window closed
        bool data_location_disk; //!< Flag for data location disk
        bool xpn_viewer_closed_soft; //!< Flag for Xpn viewer closed softly

        int curr_panx; //!< Current panel index (0-7)

        QWidget * cornerWidget; //!< Corner widget

    private:
        US_InitDialogueGui* epanInit; //!< US_Init panel
        US_ExperGui* epanExp; //!< US_Exp panel
        US_ObservGui* epanObserv; //!< US_Observ panel
        US_PostProdGui* epanPostProd; //!< US_PostProd panel
        US_EditingGui* epanEditing; //!< US_Editing panel
        US_AnalysisGui* epanAnalysis; //!< US_Analysis panel
        US_ReportStageGui* epanReport; //!< US_Report panel

    private slots:
        /**
         * \brief Checks the data location.
         */
        void checkDataLocation( void );

        /**
         * \brief Initializes the panels.
         * \param index The index of the panel to initialize.
         */
        void initPanels  ( int index );

        /**
         * \brief Switches to live update.
         * \param protocol_details The protocol details.
         */
        void switch_to_live_update( QMap < QString, QString > & protocol_details );

        /**
         * \brief Switches to post-processing.
         * \param protocol_details The protocol details.
         */
        void switch_to_post_processing( QMap < QString, QString > & protocol_details );

        /**
         * \brief Switches to editing.
         * \param protocol_details The protocol details.
         */
        void switch_to_editing( QMap < QString, QString > & protocol_details );

        /**
         * \brief Switches to analysis.
         * \param protocol_details The protocol details.
         */
        void switch_to_analysis( QMap < QString, QString > & protocol_details );

        /**
         * \brief Switches to report.
         * \param protocol_details The protocol details.
         */
        void switch_to_report( QMap < QString, QString > & protocol_details );

        /**
         * \brief Closes all windows.
         */
        void close_all( void );

        /**
         * \brief Handles the close event.
         * \param event The close event.
         */
        void closeEvent ( QCloseEvent* event );

        /**
         * \brief Switches to autoflow records.
         */
        void to_autoflow_records( void );

        /**
         * \brief Defines a new experiment.
         * \param protocol_details The protocol details.
         */
        void define_new_experiment( QMap < QString, QString > & protocol_details );

        /**
         * \brief Indicates that live update has stopped.
         */
        void liveupdate_stopped( void );

        /**
         * \brief Shows the live update finishing message.
         */
        void show_liveupdate_finishing_msg( void );

        /**
         * \brief Indicates that analysis update has stopped.
         */
        void analysis_update_stopped( void );

        /**
         * \brief Shows the analysis update finishing message.
         */
        void show_analysis_update_finishing_msg( void );

        signals:
                /**
                 * \brief Signal to pass to live update.
                 * \param protocol_details The protocol details.
                 */
                void pass_to_live_update( QMap < QString, QString > & protocol_details );

        /**
         * \brief Signal to pass to post-processing.
         * \param protocol_details The protocol details.
         */
        void pass_to_post_processing(  QMap < QString, QString > & protocol_details );

        /**
         * \brief Signal to pass to editing.
         * \param protocol_details The protocol details.
         */
        void pass_to_editing( QMap < QString, QString > & protocol_details );

        /**
         * \brief Signal to pass to analysis.
         * \param protocol_details The protocol details.
         */
        void pass_to_analysis( QMap < QString, QString > & protocol_details );

        /**
         * \brief Signal to pass to report.
         * \param protocol_details The protocol details.
         */
        void pass_to_report( QMap < QString, QString > & protocol_details );

        /**
         * \brief Signal to indicate that the US project is closed.
         */
        void us_comproject_closed( void );

        /**
         * \brief Signal to pass used instruments.
         * \param used_instruments The used instruments.
         */
        void pass_used_instruments( QMap < QString, QString > & used_instruments );

        /**
         * \brief Signal to reset LIMS import.
         */
        void reset_lims_import( void );

        /**
         * \brief Signal to reset data editing.
         */
        void reset_data_editing( void );

        /**
         * \brief Signal to reset live update.
         */
        void reset_live_update( void );

        /**
         * \brief Signal to reset reporting.
         */
        void reset_reporting( void );
};

#endif
