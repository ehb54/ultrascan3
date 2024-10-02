//! \file us_manage_data.h
#ifndef US_MANAGE_DATA_H
#define US_MANAGE_DATA_H

#include "us_extern.h"
#include "us_data_model.h"
#include "us_data_process.h"
#include "us_data_tree.h"
#include "us_sync_exper.h"
#include "us_widgets.h"
#include "us_db2.h"
#include "us_model.h"
#include "us_buffer.h"
#include "us_analyte.h"
#include "us_help.h"
#include "us_dataIO.h"
#include "us_settings.h"

//! \class US_ManageData
//! \brief Class to manage data including display, processing, and database interactions.
class US_ManageData : public US_Widgets
{
    Q_OBJECT

    public:
        //! \brief Constructor for US_ManageData
        US_ManageData();

        //! \enum State
        //! \brief Enumeration for data record states
        enum State { NOSTAT=0,  REC_DB=1,  REC_LO=2, PAR_DB=4, PAR_LO=8,
            HV_DET=16, IS_CON=32, ALL_OK=64 };

        //! \class DataDesc
        //! \brief Class for describing data records
        class DataDesc
        {
            public:
                int       recordID;          //!< Record DB Identifier
                int       recType;           //!< Record type (1-4)=Raw/Edit/Model/Noise
                int       parentID;          //!< Parent's DB Identifier
                int       recState;          //!< Record state flag
                QString   subType;           //!< Sub-type (e.g., TI, RI for noises)
                QString   dataGUID;          //!< This record data Global Identifier
                QString   parentGUID;        //!< Parent's GUID
                QString   filename;          //!< File name if on local disk
                QString   contents;          //!< md5sum() and length of data
                QString   label;             //!< Record identifying label
                QString   description;       //!< Record description string
                QString   lastmodDate;       //!< Last modification date/time
        };

        US_DB2*       db;               //!< Database connection

    private:
        QTreeWidget*        tw_recs;    //!< Tree widget
        QTreeWidgetItem*    tw_item;    //!< Current tree widget item
        DataDesc            cdesc;      //!< Current record description
        US_DataModel*       da_model;   //!< Underlying data handler
        US_DataProcess*     da_process; //!< Data processing handler
        US_DataTree*        da_tree;    //!< Data tree display handler
        US_SyncExperiment*  syncExper;  //!< Experiment synchronizer
        QProgressBar*       progress;   //!< Progress bar
        US_Help             showHelp;   //!< Help display
        US_Buffer           buffer;     //!< Buffer object
        US_Analyte          analyte;    //!< Analyte object
        QLabel*             lb_status;  //!< Status label
        QTextEdit*          te_status;  //!< Status text edit
        QLineEdit*          le_invtor;  //!< Investigator line edit
        QPushButton*        pb_invtor;  //!< Investigator button
        QPushButton*        pb_reset;   //!< Reset button
        QPushButton*        pb_scanda;  //!< Scan data button
        QPushButton*        pb_hsedit;  //!< Hide/show edits button
        QPushButton*        pb_hsmodl;  //!< Hide/show models button
        QPushButton*        pb_hsnois;  //!< Hide/show noises button
        QPushButton*        pb_helpdt;  //!< Data help button
        QPushButton*        pb_help;    //!< Help button
        QPushButton*        pb_close;   //!< Close button
        QComboBox*          cb_runid;   //!< Run ID combo box
        QComboBox*          cb_triple;  //!< Triple combo box
        QComboBox*          cb_source;  //!< Source combo box

        int                 personID;   //!< Person ID
        int                 ntrows;     //!< Number of tree rows
        int                 ntcols;     //!< Number of tree columns
        int                 ncrecs;     //!< Number of current records
        int                 ncraws;     //!< Number of current raw records
        int                 ncedts;     //!< Number of current edit records
        int                 ncmods;     //!< Number of current model records
        int                 ncnois;     //!< Number of current noise records
        int                 ndrecs;     //!< Number of database records
        int                 ndraws;     //!< Number of database raw records
        int                 ndedts;     //!< Number of database edit records
        int                 ndmods;     //!< Number of database model records
        int                 ndnois;     //!< Number of database noise records
        int                 nlrecs;     //!< Number of local records
        int                 nlraws;     //!< Number of local raw records
        int                 nledts;     //!< Number of local edit records
        int                 nlmods;     //!< Number of local model records
        int                 nlnois;     //!< Number of local noise records
        int                 kdmy;       //!< Dummy variable
        int                 dbg_level;  //!< Debug level
        bool                rbtn_click; //!< Radio button click state
        QString             run_name;   //!< Run name
        QString             investig;   //!< Investigator name
        QStringList         editIDs;    //!< List of edit IDs
        QStringList         triples;    //!< List of triples
        QStringList         runIDs;     //!< List of run IDs
        QList< QStringList > runid_data; //!< List of run ID data

    private slots:
        //! \brief Toggle the display of edits
        void toggle_edits( void );

        //! \brief Toggle the display of models
        void toggle_models( void );

        //! \brief Toggle the display of noises
        void toggle_noises( void );

        //! \brief Show help information for the data tree
        void dtree_help( void );

        //! \brief Scan data
        void scan_data( void );

        //! \brief Reset the interface
        void reset( void );

        //! \brief Select an investigator
        void sel_investigator( void );

        //! \brief Assign an investigator
        //! \param index Index of the investigator
        void assign_investigator( int index );

        //! \brief Handle item click in the tree
        //! \param item Pointer to the tree widget item
        void clickedItem( QTreeWidgetItem* item );

        //! \brief Get action text
        //! \param action Action to perform
        //! \param text Action text
        //! \return Action text
        QString action_text( QString action, QString text );

        //! \brief Handle the result of an action
        //! \param result Result code
        //! \param text Result text
        void action_result( int result, QString text );

        //! \brief Reset hide/show buttons
        //! \param edit Show edits button state
        //! \param model Show models button state
        //! \param noise Show noises button state
        //! \param browse Browse button state
        void reset_hsbuttons( bool edit, bool model, bool noise, bool browse );

        //! \brief Report the data status
        void reportDataStatus( void );

        //! \brief Handle selected run ID change
        //! \param runID Selected run ID
        void selected_runID( QString runID );

        //! \brief Handle selected run ID change
        //! \param index Index of the selected run ID
        void selected_runID( int index );

        //! \brief Show help information
        void help( void )
        { showHelp.show_help( "manage_data.html" ); };
};

#endif // US_MANAGE_DATA_H
