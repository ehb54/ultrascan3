//! \file us_data_model.h
#ifndef US_DATA_MODEL_H
#define US_DATA_MODEL_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_db2.h"
#include "us_model.h"
#include "us_noise.h"
#include "us_buffer.h"
#include "us_analyte.h"
#include "us_help.h"
#include "us_dataIO.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()  //!< debug-level-conditioned qDebug()
#endif

//! \class US_DataModel
//! \brief Class to manage data model operations, including database interactions and data processing.
class US_DataModel : public QObject
{
    Q_OBJECT

    public:
        //! \brief Constructor for US_DataModel
        //! \param parent Optional parent widget
        US_DataModel( QWidget* parent = 0 );

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
                QString   filemodDate;       //!< Last modification date/time (file)
                QString   lastmodDate;       //!< Last modification date/time (DB/file)
        };

        //! \brief Set the database connection
        //! \param db Database connection pointer
        void setDatabase( US_DB2* db );

        //! \brief Set the progress bar and status label
        //! \param progress Progress bar
        //! \param status Status label
        void setProgress( QProgressBar* progress, QLabel* status );

        //! \brief Set sibling objects for synchronization
        //! \param proc Data processor object
        //! \param tree Data tree handler object
        void setSiblings( QObject* proc, QObject* tree );

        //! \brief Get the list of run IDs
        //! \param runIDs List of run IDs
        //! \param count Count of run IDs
        void getRunIDs( QStringList& runIDs, int& count );

        //! \brief Get the list of triples
        //! \param triples List of triples
        //! \param runID Run ID
        void getTriples( QStringList& triples, QString runID );

        //! \brief Set filters for data selection
        //! \param run Filter by run
        //! \param triple Filter by triple
        //! \param source Filter by source
        void setFilters( QString run, QString triple, QString source );

        //! \brief Get the database connection
        //! \return Database connection pointer
        US_DB2* dbase( void );

        //! \brief Get investigator text
        //! \return Investigator text
        QString invtext( void );

        //! \brief Get the progress bar
        //! \return Progress bar pointer
        QProgressBar* progrBar( void );

        //! \brief Get the status label
        //! \return Status label pointer
        QLabel* statlab( void );

        //! \brief Get the data processor object
        //! \return Data processor object pointer
        QObject* procobj( void );

        //! \brief Get the data tree handler object
        //! \return Data tree handler object pointer
        QObject* treeobj( void );

        //! \brief Browse data
        void browse_data( void );

        //! \brief Scan data
        void scan_data( void );

        //! \brief Load dummy data
        void dummy_data( void );

        //! \brief Set the investigator
        //! \param investigator Investigator name
        void set_investigator( QString investigator );

        //! \brief Get investigator text
        //! \return Investigator text
        QString investigator_text( void );

        //! \brief Get data description for a specific row
        //! \param row Row index
        //! \return Data description for the specified row
        DataDesc row_datadesc( int row );

        //! \brief Get the current data description
        //! \return Current data description
        DataDesc current_datadesc( void );

        //! \brief Change data description for a specific row
        //! \param desc Data description
        //! \param row Row index
        void change_datadesc( DataDesc desc, int row );

        //! \brief Set the current data description row
        //! \param row Row index
        void setCurrent( int row );

        //! \brief Get the record count
        //! \return Record count
        int recCount( void );

        //! \brief Get the database record count
        //! \return Database record count
        int recCountDB( void );

        //! \brief Get the local record count
        //! \return Local record count
        int recCountLoc( void );

    private:
        US_DB2*       db;               //!< Pointer to opened DB connection
        QProgressBar* progress;         //!< Progress bar on main window
        QLabel*       lb_status;        //!< Status label on main window
        QWidget*      parentw;          //!< Parent widget (main window)

        QTreeWidget*        tw_recs;    //!< Tree widget
        QTreeWidgetItem*    tw_item;    //!< Current tree widget item

        DataDesc            cdesc;      //!< Current record description
        QVector< DataDesc > ddescs;     //!< DB descriptions
        QVector< DataDesc > ldescs;     //!< Local-disk descriptions
        QVector< DataDesc > adescs;     //!< All (merged) descriptions
        QVector< int >      chgrows;    //!< Changed rows

        QObject*            ob_process; //!< Data processor
        QObject*            ob_tree;    //!< Data tree handler
        QObject*            ob_exper;   //!< Experiment synchronizer

        US_Buffer     buffer;           //!< Buffer object
        US_Analyte    analyte;          //!< Analyte object

        int           personID;         //!< Person ID
        int           dbg_level;        //!< Debug level
        int           maxdlen;          //!< Maximum data length

        QString       invID;            //!< Investigator ID
        QString       run_name;         //!< Run name
        QString       cell;             //!< Cell
        QString       filt_run;         //!< Filter run
        QString       filt_triple;      //!< Filter triple
        QString       filt_source;      //!< Filter source

        QPoint        cur_pos;          //!< Current position

    private slots:
        //! \brief Slot to scan database
        void scan_dbase( void );

        //! \brief Slot to scan local data
        void scan_local( void );

        //! \brief Slot to merge database and local data
        void merge_dblocal( void );

        //! \brief Slot to exclude trees
        void exclude_trees( void );

        //! \brief Slot to review database
        void review_dbase( void );

        //! \brief Sort descriptions
        //! \param descs Vector of descriptions
        void sort_descs( QVector< DataDesc >& descs );

        //! \brief Review descriptions
        //! \param list List of strings
        //! \param descs Vector of descriptions
        //! \return true if successful, false otherwise
        bool review_descs( QStringList& list, QVector< DataDesc >& descs );

        //! \brief Get the index of a substring in a list
        //! \param str String to search for
        //! \param pos Position to start searching
        //! \param list List to search in
        //! \return Index of the substring
        int index_substring( QString str, int pos, QStringList& list );

        //! \brief Filter a list by substring
        //! \param str Substring to filter by
        //! \param pos Position to start filtering
        //! \param list List to filter
        //! \return Filtered list
        QStringList filter_substring( QString str, int pos, QStringList& list );

        //! \brief List orphans
        //! \param list1 First list
        //! \param list2 Second list
        //! \return List of orphans
        QStringList list_orphans( QStringList& list1, QStringList& list2 );

        //! \brief Get the record state flag
        //! \param desc Data description
        //! \param flag State flag
        //! \return Record state flag
        int record_state_flag( DataDesc desc, int flag );

        //! \brief Get the sort string for a data description
        //! \param desc Data description
        //! \param flag State flag
        //! \return Sort string
        QString sort_string( DataDesc desc, int flag );

        //! \brief Get the model type
        //! \param attr1 Attribute 1
        //! \param attr2 Attribute 2
        //! \param attr3 Attribute 3
        //! \param attr4 Attribute 4
        //! \return Model type string
        QString model_type( int attr1, int attr2, int attr3, bool attr4 );

        //! \brief Get the model type
        //! \param model US_Model object
        //! \return Model type string
        QString model_type( US_Model model );

        //! \brief Get the model type
        //! \param type Type string
        //! \return Model type string
        QString model_type( QString type );

        //! \brief Get the experiment GUID from AUC data
        //! \param auc AUC data
        //! \return Experiment GUID
        QString expGUIDauc( QString auc );
};

#endif // US_DATA_MODEL_H
