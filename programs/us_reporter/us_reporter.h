//! \file us_reporter.h
#ifndef US_REPORTER_H
#define US_REPORTER_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_db2.h"
#include "us_help.h"
#include "us_settings.h"
#if QT_VERSION > 0x050000
#include <QtPrintSupport/QPrinter>
#endif

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

/**
 * @class US_Reporter
 * @brief The US_Reporter class provides a user interface for generating and managing reports.
 */
class US_Reporter : public US_Widgets
{
    Q_OBJECT

    public:
        /**
         * @brief Constructor for US_Reporter.
         */
        US_Reporter();

        /**
         * @class DataDesc
         * @brief Class to describe data entries in the report.
         */
        class DataDesc
        {
        public:
            int       linen;             //!< Line number
            int       level;             //!< Tree level (0,...)
            int       checkState;        //!< Check state flag (0,1,2)
            int       children;          //!< Number of children
            QString   label;             //!< Entry label
            QString   type;              //!< Type of entry (Run, Data, ...)
            QString   filename;          //!< File name
            QString   filepath;          //!< Full file path name
            QString   runid;             //!< Run ID
            QString   triple;            //!< Triple (e.g., "2/A/280")
            QString   analysis;          //!< Analysis
            QString   lastmodDate;       //!< File last modification date/time
            QString   description;       //!< Triple data set description
        };

    private:
        QTreeWidget*        tw_recs;    //!< Tree widget

        DataDesc            cdesc;      //!< Current record description
        QVector<DataDesc>   adescs;     //!< All descriptions

        QMap<QString, QString> appmap;  //!< Applications name, label map
        QMap<QString, QString> extmap;  //!< Extensions name, label map
        QMap<QString, QString> rptmap;  //!< Reports name, label map

        QList<int>          se_rptrows; //!< List of selected report rows

        QStringList         sl_runids;  //!< List of existing report runIDs
        QStringList         se_runids;  //!< List of selected runIDs
        QStringList         se_reports; //!< List of selected reports

        US_Help       showHelp;         //!< Help display object

        US_DB2*       db;               //!< Database connection

        QComboBox*    cb_runids;        //!< Combo box for run IDs

        QPushButton*  pb_view;          //!< View button
        QPushButton*  pb_save;          //!< Save button
        QPushButton*  pb_help;          //!< Help button
        QPushButton*  pb_close;         //!< Close button

        int           ntrows;           //!< Number of tree rows
        int           ntcols;           //!< Number of tree columns
        int           nsrpts;           //!< Number of selected reports
        int           nsruns;           //!< Number of selected runs
        int           nstrips;          //!< Number of selected triples
        int           nshtmls;          //!< Number of selected HTML files
        int           nsplots;          //!< Number of selected plots
        int           dbg_level;        //!< Debug level

        bool          rbtn_click;       //!< Radio button click flag
        bool          change_tree;      //!< Change tree flag
        bool          changed;          //!< Changed flag
        bool          load_ok;          //!< Load OK flag
        bool          ld_wait;          //!< Load wait flag

        QString       run_name;         //!< Run name
        QString       investig;         //!< Investigation
        QString       pagedir;          //!< Page directory
        QString       pagepath;         //!< Page path
        QString       ppdfpath;         //!< PDF path
        QString       hsclogo;          //!< HS logo
        QString       becklogo;         //!< Beckman logo
        QString       us3logo;          //!< US3 logo
        QString       archdir;          //!< Archive directory

    private slots:
        /**
         * @brief Slot for clicking on an item in the tree widget.
         * @param item The clicked item
         */
        void clickedItem(QTreeWidgetItem* item);

        /**
         * @brief Slot for changing an item in the tree widget.
         * @param item The changed item
         * @param column The column of the changed item
         */
        void changedItem(QTreeWidgetItem* item, int column);

        /**
         * @brief Slot for showing context menu on row right-click.
         * @param item The clicked item
         */
        void row_context(QTreeWidgetItem* item);

        /**
         * @brief Build the list of run IDs.
         */
        void build_runids(void);

        /**
         * @brief Slot for selecting a new run ID.
         * @param index The index of the selected run ID
         */
        void new_runid(int index);

        /**
         * @brief Build the descriptions for the specified run ID.
         * @param runid The run ID
         * @param level The level of the tree
         */
        void build_descs(QString& runid, int& level);

        /**
         * @brief Build a map for the specified type.
         * @param type The type of entries
         * @param map The map to build
         */
        void build_map(QString type, QMap<QString, QString>& map);

        /**
         * @brief Build the tree structure.
         */
        void build_tree(void);

        /**
         * @brief Count the children of a data description.
         * @param desc The data description
         * @param count The count of children
         * @param level The level in the tree
         */
        void count_children(DataDesc* desc, int& count, int& level);

        /**
         * @brief Update the state of children items.
         * @param desc The data description
         * @param state The state to set
         */
        void state_children(DataDesc* desc, int& state);

        /**
         * @brief Update the state of parent items.
         * @param desc The data description
         * @param state The state to set
         */
        void state_parents(DataDesc* desc, int& state);

        /**
         * @brief Mark checked items.
         */
        void mark_checked(void);

        /**
         * @brief View the selected item.
         */
        void view(void);

        /**
         * @brief Save the selected item.
         */
        void save(void);

        /**
         * @brief Write the report to a file.
         * @return True if successful, false otherwise
         */
        bool write_report(void);

        /**
         * @brief Count the number of reports.
         * @return True if successful, false otherwise
         */
        bool count_reports(void);

        /**
         * @brief View the selected item.
         */
        void item_view(void);

        /**
         * @brief Show the selected item.
         */
        void item_show(void);

        /**
         * @brief Save the selected item.
         */
        void item_save(void);

        /**
         * @brief Load the profile.
         */
        void load_profile(void);

        /**
         * @brief Save the profile.
         */
        void save_profile(void);

        /**
         * @brief Synchronize with the database.
         */
        void sync_db(void);

        /**
         * @brief Copy logos to the specified directory.
         * @param directory The directory to copy logos to
         */
        void copy_logos(QString directory);

        /**
         * @brief Write the report to a PDF file.
         */
        void write_pdf(void);

        /**
         * @brief Pad a line with spaces.
         * @param line The line to pad
         * @return The padded line
         */
        QString pad_line(const QString line);

        /**
         * @brief Show help for the reporter.
         */
        void help(void)
        {
            showHelp.show_help("reporter.html");
        }
};

#endif // US_REPORTER_H
