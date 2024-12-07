//! \file us_xpn_data.h
#ifndef US_XPN_DATA_H
#define US_XPN_DATA_H

#include <QtCore>
#include <QtSql>

#include "us_extern.h"
#include "us_dataIO.h"
#include "us_simparms.h"

#ifndef SP_SPEEDPROFILE
#define SP_SPEEDPROFILE US_SimulationParameters::SpeedProfile
#endif

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()  //!< debug-level-conditioned qDebug()
#endif

//! Class for holding Beckman XPN (Optima) Data with functions for
//! importing from raw files or loading from vectors of rawData.
class US_UTIL_EXTERN US_XpnData : public QObject
{
   Q_OBJECT

   public:
      US_XpnData( );

 
      //! ExperimentRun table values
      class tbExpRun
      {
         public:
            int       runId;         //!< Run ID
            int       expId;         //!< Experiment ID
            QString   rotorSN;       //!< Rotor serial number
            QString   datapath;      //!< Data path
            QDateTime expstart;      //!< Experiment start
            QString   instrSN;       //!< Instrument serial number
            QString   scimo1sn;      //!< Science module 1 serial number
            QString   scimo2sn;      //!< Science module 2 serial number
            QString   scimo3sn;      //!< Science module 3 serial number
            int       runstat;       //!< Run status
            QString   expdef;        //!< Experiment definition (json)
            QString   expname;       //!< Experiment name
            QString   resname;       //!< Researcher name
            bool      abscnf;        //!< Flag if Absorbance scans present
            bool      flscnf;        //!< Flag if Flourescence scans present
            bool      inscnf;        //!< Flag if Interference scans present
            bool      wlscnf;        //!< Flag if Wavelength scans present
      };

      //! Absorbance Scan Data table values
      class tbAsData
      {
         public:
            int       dataId;        //!< Data table entry ID
            int       runId;         //!< Run ID
            int       exptime;       //!< Time in seconds from exp. start
            int       stageNum;      //!< Stage number
            int       scanSeqN;      //!< Scan sequence number
            int       modPos;        //!< Module position
            int       cellPos;       //!< Cell position
            int       replic;        //!< Replicate
            int       wavelen;       //!< Wavelength
            double    tempera;       //!< Temperature
            double    speed;         //!< Speed in revs per minute
            double    omgSqT;        //!< OmegaSquaredT
            QVector< double > rads;  //!< Radius values
            QVector< double > vals;  //!< Reading (intensity) values
            int       count;         //!< Number of readings
            QDateTime expstart;      //!< Experiment start
            QString   samplName;     //!< Sample name
            QString   scanTypeF;     //!< Scan type flag
            QString   radPath;       //!< Radial path (' ','A','B')
      };

      //! Fluorescence Scan Data table values
      class tbFsData
      {
         public:
            int       dataId;        //!< Data table entry ID
            int       runId;         //!< Run ID
            int       exptime;       //!< Time in seconds from exp. start
            int       stageNum;      //!< Stage number
            int       scanSeqN;      //!< Scan sequence number
            int       modPos;        //!< Module position
            int       cellPos;       //!< Cell position
            int       replic;        //!< Replicate
            int       wavelen;       //!< Wavelength
            double    tempera;       //!< Temperature
            double    speed;         //!< Speed in revs per minute
            double    omgSqT;        //!< OmegaSquaredT
            QVector< double > rads;  //!< Radius values
            QVector< double > vals;  //!< Reading (intensity) values
            int       count;         //!< Number of readings
            QDateTime expstart;      //!< Experiment start
            QString   samplName;     //!< Sample name
            QString   scanTypeF;     //!< Scan type flag
            QString   radPath;       //!< Radial path (' ','A','B')
      };

      //! Interference Scan Data table values
      class tbIsData
      {
         public:
            int       dataId;        //!< Data table entry ID
            int       runId;         //!< Run ID
            int       exptime;       //!< Time in seconds from exp. start
            int       stageNum;      //!< Stage number
            int       scanSeqN;      //!< Scan sequence number
            int       modPos;        //!< Module position
            int       cellPos;       //!< Cell position
            int       replic;        //!< Replicate
            int       wavelen;       //!< Wavelength
            double    tempera;       //!< Temperature
            double    speed;         //!< Speed in revs per minute
            double    omgSqT;        //!< OmegaSquaredT
            double    startPos;      //!< Start position
            double    resolu;        //!< Resolution
            QVector< double > rads;  //!< Radius (position) values
            QVector< double > vals;  //!< Reading (interference) values
            int       count;         //!< Number of readings
            QDateTime expstart;      //!< Experiment start
            QString   samplName;     //!< Sample name
            QString   scanTypeF;     //!< Scan type flag
      };

      //! Wavelength Scan Data table values
      class tbWsData
      {
         public:
            int       dataId;        //!< Data table entry ID
            int       runId;         //!< Run ID
            int       exptime;       //!< Time in seconds from exp. start
            int       stageNum;      //!< Stage number
            int       scanSeqN;      //!< Scan sequence number
            int       modPos;        //!< Module position
            int       cellPos;       //!< Cell position
            int       replic;        //!< Replicate
            double    tempera;       //!< Temperature
            double    speed;         //!< Speed in revs per minute
            double    omgSqT;        //!< OmegaSquaredT
            double    scanPos;       //!< Scan position
            QVector< double > wvls;  //!< Wavelength values
            QVector< double > vals;  //!< Reading (intensity) values
            int       count;         //!< Number of readings
            QDateTime expstart;      //!< Experiment start
            QString   samplName;     //!< Sample name
            QString   scanTypeF;     //!< Scan type flag
            QString   radPath;       //!< Radial path (' ','A','B')
      };

      //! Table values shared by all Scan Data types
      class tbCsData
      {
         public:
            int       dataId;        //!< Data table entry ID
            int       runId;         //!< Run ID
            int       exptime;       //!< Time in seconds from exp. start
            int       stageNum;      //!< Stage number
            int       scanSeqN;      //!< Scan sequence number
            int       modPos;        //!< Module position
            int       cellPos;       //!< Cell position
            int       replic;        //!< Replicate
            int       wavelen;       //!< Wavelength
            double    tempera;       //!< Temperature
            double    speed;         //!< Speed in revs per minute
            double    omgSqT;        //!< OmegaSquaredT
            QVector< double >* rads; //!< Pointer to Radius values
            QVector< double >* vals; //!< Pointer to Reading values
            int       count;         //!< Number of readings
            QDateTime expstart;      //!< Experiment start
            QString   samplName;     //!< Sample name
            QString   scanTypeF;     //!< Scan type flag
            QString   radPath;       //!< Radial path (' ','A','B')
      };

      //! System Status Data table values
      class tbSyData
      {
         public:
            int       dataId;        //!< Data table entry ID
            int       runId;         //!< Run ID
            int       exptime;       //!< Time in seconds from exp. start
            int       stageNum;      //!< Stage number
            double    tempera;       //!< Temperature
            double    speed;         //!< Speed in revs per minute
            double    omgSqT;        //!< OmegaSquaredT
            QDateTime expstart;      //!< Experiment start
      };

      //! Centrifuge Run Profile table values
      class tbCrProf
      {
         public:
            int       frunId;        //!< Fuge run profile ID
            int       sbrake;        //!< Step Braking
            int       holdte;        //!< Hold temp after final
            int       holdsp;        //!< Hold speed after final
            int       sette;         //!< Set temp before start
            int       sstintv;       //!< System status interval
            double    brrate;        //!< Braking rate
            double    tempera;       //!< Temperature
            double    stgrpm;        //!< Stage RPM
            double    stgstart;      //!< Stage start
            double    stgdurat;      //!< Stage duration
            double    stgaccel;      //!< Stage acceleration rate
            QString   stages;        //!< Stages
            QString   paramids;      //!< Stage cell parameter Ids
      };


      //! \brief Connect for XPN data with remote host DB
      //! \param xpnhost Host name of XPN database server
      //! \param xpnport Port value of XPN database server
      //! \param adbname Name of database to examine
      //! \param adbuser Database user
      //! \param adbpasw Database password
      //! \returns       Status of connect (true->connected OK)
      bool    connect_data( const QString, const int = 5432,
                            const QString = QString( "AUC_DATA_DB" ),
                            const QString = QString( "aucuser" ),
                            const QString = QString( "aucuser" ) );

      //! \brief Connect for XPN data with remote host DB
      //! \param adbname Name of database to examine
      //! \param xpnhost Host name of XPN database server
      //! \param xpnport Port value of XPN database server
      //! \returns       Status of connect (true->connected OK)
      bool    connect_data( const QString, const QString, const int = 5432 );

      //! \brief Scan the DB for Runs information
      //! \param runInfo Reference for returned run info strings
      //! \returns       Number of runs (runInfo size)
      int     scan_runs( QStringList& );
      int     scan_runs_auto( QStringList& , QString&  );
      
      int     get_runid( QString );

      int     checkExpStatus( QString );
      int     checkExpStatus_auto( QString, bool& );
      
      //! \brief Filter Runs information to exclude zero-data ones
      //! \param runInfo Reference for input/updated run info strings
      //! \returns       Number of runs (runInfo size)
      int     filter_runs( QStringList& );

      //! \brief Scan the DB for [AIFW]ScanData table information
      //! \param runId    Run ID to match
      //! \param scantype Scan type ('A','F','I','W','S','C') to match
      //! \returns        Number of rows of match ScanData found
      int     scan_xpndata( const int, const QChar );
      int     scan_xpndata_auto( const int, const QChar, bool&, QElapsedTimer );

      //! \brief Update the DB [AIFW]ScanData table information
      //! \param runId    Run ID to match
      //! \param scantype Scan type ('A','F','I','W') to match
      //! \returns        Number of rows of match ScanData found or
      //! \                negative count if no new rows added.
      int     update_xpndata( const int, const QChar );

      //! \brief Update current System Status record (isyrec)
      //! \param runId    Run ID to match
      //! \returns        Current f rows of match ScanData found or
      //! \                negative count if no new rows added.
      //int     update_isysrec( const int, tbSyData& );
      int     update_isysrec( const int );

      // set explicitly etimoff variable to 0
      void    setEtimOffZero ( void );
      
      //! \brief Import ScanData from the postgres database
      //! \param runId    Run ID to match
      //! \param scanMask Scan mask (AFIW, 1 to 15) of tables
      //! \returns        Status of import (true->imported OK)
      bool    import_data   ( const int, const int );
      bool    import_data_auto   ( const int, const int, bool& );

      //! \brief Reimport ScanData from the postgres database
      //! \param runId    Run ID to match
      //! \param scanMask Scan mask (AFIW, 1 to 15) of tables
      //! \returns        Status of reimport (false->reimport not needed)
      bool    reimport_data ( const int, const int );
      bool    reimport_data_auto ( const int, const int, bool& );

      //! \brief Load XPN internal variables from loaded rawDatas
      //! \param allData Vector of loaded rawDatas
      //! \param ifpaths List of auc file paths
      void    load_auc      ( QVector< US_DataIO::RawData >&,
                              const QStringList );

      //! \brief Return lambda values for raw original
      //! \param wls     Output lambdas vector for the imported data
      //! \returns       Number of lambdas in the imported XPN data
      int     lambdas_raw   ( QVector< int >& );

      //! \brief Match lambda in original list
      //! \param lambda  Lambda value to find in the original import list
      //! \returns       Index of the lambda match in the lambdas list
      int     indexOfLambda ( int );

      //! \brief Return list of cells/channels
      //! \param celchns Output cell/channels string list
      //! \returns       Number of cell/channels in the list
      int     cellchannels  ( QStringList& );

      //! \brief Return list of cell/channel/wavelength strings
      //! \param trips   Output cell/channel/wavelengths string list
      //! \returns       Number of triples in the list
      int     data_triples  ( QStringList& );

      //! \brief Build RawData vector
      //! \param allData Output vector of rawDatas built from XPN data
      //! \returns       Number of triples in the data (size of allData)
      int     build_rawData ( QVector< US_DataIO::RawData >& );

      //! \brief Rebuild RawData vector
      //! \param allData Input/Updated vector of rawDatas built from XPN data
      //! \returns       Number of triples in the data (size of allData)
      int     rebuild_rawData ( QVector< US_DataIO::RawData >& );

      //! \brief Export to openAUC
      //! \param allData Input vector of rawDatas built from XPN data
      //! \returns       Number of files written
      int     export_auc      ( QVector< US_DataIO::RawData >& );
      int     export_auc_auto ( QVector< US_DataIO::RawData >&, bool& );
      
      //! \brief A count of specified type
      //! \param key     Key string for which to map a value ("file",...)
      //! \returns       Number of values for the array with given key
      int     countOf       ( QString );

      QString   countOf_sysdata  ( QString );

      //! \brief Return the cell/channel description string
      //! \param celchn  Cell/channel to examine (e.g.,"1 / A")
      //! \returns       Description string for the specified channel
      QString cc_description( QString );

      //! \brief Set the run ID and type string ("RI"|"FI"|"IP"|"WI")
      //! \param arunid   Run ID value to set
      //! \param aruntype Run type value to set
      void    set_run_values( const QString, const QString );

      //! \brief Return runID and runType string for the data
      //! \param arunid   Returned runID value for the data
      //! \param aruntype Returned runType value for the data ("RI")
      void    run_values    ( QString&, QString& );

      //! \brief Set the common-values Scan Data record by runType
      //! \param datx   Scan data array index for data fetch
      //! \param arType Run Type ("RI"|"FI"|"IP"|"WI")
      void    set_scan_data ( const int, const QString="" );

      //! \brief Clear all the data structures
      void    clear         ( void  );

      //! \brief Close the database connection
      void    close         ( void  );

      //! \brief Return string text of run data details
      //! \returns      Details text for use in text dialog
      QString runDetails    ( void );

      //! \brief Return the last error string
      //! \returns      DB Connection last error string
      QString lastError     ( void );

      //! \brief Dump information to STDERR on tables,fields
      void    dump_tables   ( void  );

   signals:
      //! \brief Emit a signal that includes status text
      void status_text  ( QString );

   private:
      QString   dbname;              //!< XPN database name
      QString   dbhost;              //!< XPN db server host name
      QString   dbuser;              //!< XPN db server user name
      QString   dbpasw;              //!< XPN db server user password
      int       dbport;              //!< XPN server port number (def.=5432)

      QVector< int >       wavelns;  //!< Raw input wavelengths
      QVector< int >       stgnbrs;  //!< Stage Numbers, selected type
      QVector< int >       scnnbrs;  //!< Scan Numbers, selected type
      QVector< double >    r_radii;  //!< Raw radius values
      QVector< double >    a_radii;  //!< AUC data radius values
      QVector< QString >   datrecs;  //!< trnode.stage.scan recs per type

      QVector< tbExpRun >  tExprun;  //!< ExperimentRun table values
      QVector< tbAsData >  tAsdata;  //!< Absorbance   data table values
      QVector< tbFsData >  tFsdata;  //!< Fluorescence data table values
      QVector< tbIsData >  tIsdata;  //!< Interference data table values
      QVector< tbWsData >  tWsdata;  //!< Wavelength   data table values
      QVector< tbSyData >  tSydata;  //!< System status data table values
      QVector< tbCrProf >  tCrprof;  //!< Centrifuge run prof. table values

      QVector< double >   r_rpms;    //!< Raw RPMs from scans
      QVector< double >   s_rpms;    //!< Set RPMs from speed steps
      QVector< double >   a_rpms;    //!< Average RPMs from speed steps

      tbCsData            csdrec;    //!< Common values scan data record
      tbSyData            isyrec;    //!< Instantaneous System status data

      QStringList         fpaths;    //!< Input file paths
      QStringList         fnames;    //!< Input file names
      QStringList         cells;     //!< Input Cell strings
      QStringList         cellchans; //!< Input Cell/Channel strings

      QStringList         ccdescs;   //!< Cell/Chann description strings
      QStringList         triples;   //!< Triple strings  ("1 / A / 280")
      QStringList         trnodes;   //!< Triple file nodes ("1.A.280")

      QSqlDatabase        dbxpn;     //!< PostgresSql XPN database

      QMap< QString, int >  counts;  //!< Map of type counts for tables
      QMap< QString, QString >  counts_sysdata;  //!< Map of type counts for tables
      QMap< QString, int >  totals;  //!< Map of total rows for tables

      int       sctype;              //!< Scan type captured (1<==>Data)
      int       nfile;               //!< Number of input files
      int       nscan;               //!< Number of scans per triple (max.)
      int       ncell;               //!< Number of cells
      int       nchan;               //!< Number of channels
      int       ncelchn;             //!< Number of Cell/Channels
      int       nlambda;             //!< Number of wavelengths
      int       ntriple;             //!< Number of triples
      int       npoint;              //!< Number radius points per scan
      int       npointt;             //!< Number points per triple
      int       nstgn;               //!< Number of stage numbers
      int       nscnn;               //!< Number of scan numbers
      int       curccx;              //!< Current cell/chan index
      int       slambda;             //!< Starting lambda
      int       elambda;             //!< Ending lambda
      int       dbg_level;           //!< Debug level
      int       mnstgn;              //!< Minimum stage number
      int       mxstgn;              //!< Maximum stage number
      int       mnscnn;              //!< Minimum scan number
      int       mxscnn;              //!< Maximum scan number
      int       ntscan;              //!< Total scans for all triples
      int       ntsrow;              //!< Total (A+F+I+W) scan rows
      int       arows;               //!< A scan rows
      int       frows;               //!< F scan rows
      int       irows;               //!< I scan rows
      int       wrows;               //!< W scan rows
      
      int       etimoff;             //!< Experimental time offset
      int       fnzstx;              //!< First non-zero-speed time index
      int       sstintv;             //!< System Status interval
      int       issintv;             //!< Instantaneous System Status interval

      
      double    radinc;              //!< Output AUC data radial increment
      double    cur_rpm;             //!< Current SS speed in RPM;

      
      bool      is_absorb;           //!< Flag if import is absorbance
      bool      is_mwl;              //!< Flag if multi-wavelength
      bool      is_raw;              //!< Flag if loaded from raw XPN

      QString   cur_dir;             //!< Selected import data directory
      QString   dbfile;              //!< Full path .sqlite DB file
      QString   runID;               //!< Run ID
      QString   runType;             //!< Run Type (usually "RI")

   private slots:

      // Create key-indexed mappings of useful values
      void   mapCounts     ( void );

      // Create key-indexed mappings of sysdata
      void   mapCounts_sysdata ( void );
            
      // Return a scan ID index within the tCFAMeta vector
      int    scan_data_index( const QString, const int, const int );
      // Get data readings from a range of Scans table records
      int    get_readings  ( QVector< double >&, const int, const int, const int );
      // Interpolate readings values to a contant-increment radial grid
      void   interp_rvalues( QVector< double >&, QVector< double >&,
                             QVector< double >&, QVector< double >& );
      // Parse a string into a vector of doubles
      int    parse_doubles ( const QString, QVector< double >& );
      // Build internal arrays and variables
      void   build_internals( void );
      // Rebuild internal arrays and variables
      void   rebuild_internals( void );
      // Get column indexes and other db table information
      int    column_indexes( const QString, QStringList&, QList< int >& );
      // Update an entry in the Absorbance data table
      void   update_ATable( QSqlQuery&, QList< int >& );
      // Update an entry in the Fluorescence data table
      void   update_FTable( QSqlQuery&, QList< int >& );
      // Update an entry in the Interference data table
      void   update_ITable( QSqlQuery&, QList< int >& );
      // Update an entry in the Wavelength data table
      void   update_WTable( QSqlQuery&, QList< int >& );

};
#endif
