//! \file us_data_process.h
#ifndef US_DATA_PROCESS_H
#define US_DATA_PROCESS_H

#include "us_extern.h"
#include "us_data_model.h"
#include "us_sync_exper.h"
#include "us_dataIO.h"
#include "us_db2.h"

//! \class US_DataProcess
//! \brief Class to manage data processing operations, including database interactions and file handling.
class US_DataProcess : public QObject
{
    Q_OBJECT

    public:
        //! \brief Constructor for US_DataProcess
        //! \param model Pointer to US_DataModel object
        //! \param parent Optional parent widget
        US_DataProcess( US_DataModel* model, QWidget* parent = 0 );

        //! \brief Upload a record to the database
        //! \param recordID ID of the record to be uploaded
        //! \return Status code of the operation
        int record_upload( int recordID );

        //! \brief Download a record from the database
        //! \param recordID ID of the record to be downloaded
        //! \return Status code of the operation
        int record_download( int recordID );

        //! \brief Remove a record from the database
        //! \param recordID ID of the record to be removed
        //! \return Status code of the operation
        int record_remove_db( int recordID );

        //! \brief Remove a record from local storage
        //! \param recordID ID of the record to be removed
        //! \return Status code of the operation
        int record_remove_local( int recordID );

        //! \brief Get the last error message
        //! \return Last error message
        QString lastError( void );

        //! \brief Handle partial errors
        //! \param errCode Error code
        //! \param errCount Error count
        void partialError( int errCode, int errCount );

        //! \brief Append an error message to the log
        //! \param message Error message to append
        void appendError( QString message );

        //! \brief Check if the raw ancestor record is okay
        //! \param recordID ID of the record to check
        //! \return True if the raw ancestor record is okay, false otherwise
        bool raw_ancestor_ok( int recordID );

    private:
        QString                 errMsg;     //!< Message from the last error
        QWidget*                parentw;    //!< Parent widget
        US_DB2*                 db;         //!< Pointer to database connection
        US_DataModel*           da_model;   //!< Data model object
        US_DataModel::DataDesc  cdesc;      //!< Current record description
        US_SyncExperiment*      syncExper;  //!< Experiment synchronizer
        int                     dbg_level;  //!< Debug level

    private slots:
        //! \brief Get the filename of a model record
        //! \param desc Pointer to DataDesc object
        //! \return Filename of the model record
        QString get_model_filename( US_DataModel::DataDesc* desc );

        //! \brief Get the filename of a model record
        //! \param guid GUID of the model record
        //! \return Filename of the model record
        QString get_model_filename( QString guid );

        //! \brief Get the filename of a noise record
        //! \param desc Pointer to DataDesc object
        //! \return Filename of the noise record
        QString get_noise_filename( US_DataModel::DataDesc* desc );

        //! \brief Get the filename of a noise record
        //! \param guid GUID of the noise record
        //! \return Filename of the noise record
        QString get_noise_filename( QString guid );
};

#endif // US_DATA_PROCESS_H
