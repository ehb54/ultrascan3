//! \file us_cfa_data.h
#ifndef US_CFA_DATA_H
#define US_CFA_DATA_H

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

//! Class for holding Centrifigal Fluid Analyzer (CFA) Data with functions for
//! importing from raw files or loading from vectors of rawData.
class US_UTIL_EXTERN US_CfaData : public QObject
{
   Q_OBJECT

   public:
      US_CfaData( );

      //! AbsMeta table values
      class tAbsMeta
      {
         public:
            int      id;            //!< Database ID
            int      scanId;        //!< Scan ID
            int      blankId;       //!< Blank (reference) scan type ID
            int      dataId;        //!< Data (intensity) scan type ID
            int      nolightId;     //!< No-light scan type ID
      };

      //! ScanMeta table values
      class tScanMeta
      {
         public:
            int      id;            //!< Database ID
            int      scanId;        //!< Scan ID
            int      scanType;      //!< Scan Type
            double   startTime;     //!< Scan start time
            double   endTime;       //!< Scan end time
            int      cell;          //!< Cell index
            int      channel;       //!< Channel index
            int      avging;        //!< Averaging flag
            int      firstRec;      //!< First Scans record index
            int      lastRec;       //!< Last Scans record index
            int      wavelen;       //!< Wavelength
            int      scanNbr;       //!< Scan number
            QString  triple;        //!< Triple string (e.g.,"1/A/280")
      };

      //! CFAMeta table values
      class tCFAMeta
      {
         public:
            int      id;            //!< Database ID
            int      scanId;        //!< Scan ID
            int      rpm;           //!< Actual speed in revs per minute
            int      drpm;          //!< Desired speed in RPM
            int      tempera;       //!< Actual temperature
            int      dtempera;      //!< Desired temperature x 10
            double   pressure;      //!< Pressure
            double   time;          //!< Time in seconds
            double   w2t;           //!< Omega-squared-t
            int      accel;         //!< Acceleration flag
            int      decel;         //!< Deceleration flag
      };

      //! DioMeta table values
      class tDioMeta
      {
         public:
            int      id;            //!< Database ID
            int      scanId;        //!< Scan ID
            int      rpm;           //!< Speed in RPM
            int      count;         //!< Count
            int      hiclock;       //!< High-clock flag
            double   period;        //!< Period
            double   jitter;        //!< Jitter
            double   jitstd;        //!< Jitter standard deviation
      };

      //! Angles table values
      class tAngles
      {
         public:
            int      id;            //!< Database ID
            int      cell;          //!< Cell index
            int      channel;       //!< Channel index
            double   offset;        //!< Offset
            double   delay;         //!< Delay
            double   duration;      //!< Duration
      };

      //! Instrument table values
      class tInstru
      {
         public:
            int      id;            //!< Database ID
            int      type;          //!< Instrument type flag
            double   insttime;      //!< Installed time
            double   opertime;      //!< Operating time
            double   servtime;      //!< Last-service time
            double   totaltime;     //!< Total time
            double   magnif;        //!< Magnification scale factor
            int      npixel;        //!< Number of pixels
            int      toppixel;      //!< Top pixel
            int      botpixel;      //!< Bottom pixel
            int      cenpixel;      //!< Center pixel
            int      servinter;     //!< Service interval
            int      srcId;         //!< Source ID
            int      detId;         //!< Detector ID
            int      macId;         //!< Machine ID
            QString  descript;      //!< Description string
      };

      //! Scans table values
      class tScans
      {
         public:
            int      id;            //!< Database ID
            int      scanId;        //!< Scan ID
            double   intensity;     //!< Intensity value
            double   stdd;          //!< Intensity standard deviation
      };

      //! Radius table values
      class tRadius
      {
         public:
            int      id;            //!< Database ID
            double   coef1;         //!< Index-to-radius coefficient 1
            double   coef2;         //!< Index-to-radius coefficient 2
            double   coef3;         //!< Index-to-radius coefficient 3
            double   coef4;         //!< Index-to-radius coefficient 4
      };

      //! Samples table values
      class tSamples
      {
         public:
            int      id;            //!< Database ID
            int      cell;          //!< Cell index
            int      channel;       //!< Channel index
            int      compId;        //!< Component ID
            int      solvId;        //!< Solvent ID
            int      unitsId;       //!< Units ID
            double   concen;        //!< Concentration
            QString  descript;      //!< Description string
            QString  units;         //!< Units string
      };

      //! WavelengthScan table values
      class tWvLnScan
      {
         public:
            int      id;            //!< Database ID
            int      wavelen;       //!< Wavelength
            int      absndx;        //!< AbsIndex
      };

      //! Wavelength table values
      class tWvLens
      {
         public:
            int      id;            //!< Database ID
            int      wavelen;       //!< Wavelength
            double   gain;          //!< Gain
            int      avging;        //!< Averaging flag
      };

      //! \brief Import data from a specified directory
      //! \param cfadir  Raw CFA data directory
      //! \param sctypf  Scan type flag (1,2,3 -> data,ref,no-light)
      //! \returns       Status of import (true->imported OK)
      bool    import_data   ( QString&, const int );

      //! \brief Load cfa internal variables from loaded rawDatas
      //! \param allData Vector of loaded rawDatas
      //! \param ifpaths List of auc file paths
      void    load_auc      ( QVector< US_DataIO::RawData >&,
                              const QStringList );

      //! \brief Return lambda values for raw original
      //! \param wls     Output lambdas vector for the imported data
      //! \returns       Number of lambdas in the imported CFA data
      int     lambdas_raw   ( QVector< int >& );

      //! \brief Match lambda in original list
      //! \param lambda  Lambda value to find in the original import list
      //! \returns       Index of the lambda match in the lambdas list
      int     indexOfLambda ( int );

      //! \brief Return list of cells/channels
      //! \param celchns Output cell/channels string list
      //! \returns       Number of cell/channels in the list
      int     cellchannels  ( QStringList& );

      //! \brief Build RawData vector
      //! \param allData Output vector of rawDatas built from CFA data
      //! \returns       Number of triples in the data (size of allData)
      int     build_rawData ( QVector< US_DataIO::RawData >& );

      //! \brief Export to openAUC
      //! \param allData Input vector of rawDatas built from CFA data
      //! \returns       Number of files written
      int     export_auc    ( QVector< US_DataIO::RawData >& );

      //! \brief A count of specified type
      //! \param key     Key string for which to map a value ("file",...)
      //! \returns       Number of values for the array with given key
      int     countOf       ( QString );

      //! \brief Return the cell/channel description string
      //! \param celchn  Cell/channel to examine (e.g.,"1 / A")
      //! \returns       Description string for the specified channel
      QString cc_description( QString );

      //! \brief Return runID and runType string for the data
      //! \param arunid   Returned runID value for the data
      //! \param aruntype Returned runType value for the data ("RI")
      void    run_values    ( QString&, QString& );

      //! \brief Clear all the data structures
      void    clear         ( void  );

      //! \brief Return string text of run data details
      //! \returns      Details text for use in text dialog
      QString runDetails    ( void );

   signals:
      //! \brief Emit a signal that includes status text
      void status_text  ( QString );

   private:
      QVector< int >        wavelns; //!< Raw input wavelengths
      QVector< int >        scanids; //!< Scan IDs, selected type
      QVector< int >        scnnbrs; //!< Scan Numbers, selected type
      QVector< double >     r_radii; //!< Raw radius values
      QVector< double >     a_radii; //!< AUC data radius values

      QVector< tAbsMeta  >  tametas; //!< AbsMeta table values
      QVector< tScanMeta >  tsmetas; //!< ScanMeta table values
      QVector< tWvLens   >  twvlens; //!< Wavelengths table values
      QVector< tAngles   >  tangles; //!< Angles table values
      QVector< tCFAMeta  >  tcmetas; //!< CFAMeta table values
      QVector< tInstru   >  tinstrs; //!< Instrument table values
      QVector< tScans    >  tscans;  //!< Scans table values
      QVector< tDioMeta  >  tdmetas; //!< DIOMeta table values
      QVector< tRadius   >  tradius; //!< Radius table values
      QVector< tSamples  >  tsampls; //!< Samples table values
      QVector< tWvLnScan >  twvlnss; //!< WavelengthScan table values

      QVector< double >   r_rpms;    //!< Raw RPMs from scans
      QVector< double >   s_rpms;    //!< Set RPMs from speed steps
      QVector< double >   a_rpms;    //!< Average RPMs from speed steps

      QStringList         fpaths;    //!< Input file paths
      QStringList         fnames;    //!< Input file names
      QStringList         cells;     //!< Input Cell strings
      QStringList         cellchans; //!< Input Cell/Channel strings

      QStringList         ccdescs;   //!< Cell/Chann description strings
      QStringList         triples;   //!< Triple strings  ("1 / A / 280")
      QStringList         trnodes;   //!< Triple file nodes ("1.A.280")

      QSqlDatabase        dbcfa;     //!< SQLITE CFA database

      QMap< QString, int >  counts;  //!< Map of type counts for tables
      QMap< QString, int >  totals;  //!< Map of total rows for tables

      int       sctype;              //!< Scan type captured (1<==>Data)
      int       nfile;               //!< Number of input files
      int       nscan;               //!< Number of scans per triple
      int       ncell;               //!< Number of cells
      int       nchan;               //!< Number of channels
      int       ncelchn;             //!< Number of Cell/Channels
      int       nlambda;             //!< Number of wavelengths
      int       ntriple;             //!< Number of triples
      int       npoint;              //!< Number radius points per scan
      int       npointt;             //!< Number points per triple
      int       nscnn;               //!< Number of scan numbers
      int       curccx;              //!< Current cell/chan index
      int       slambda;             //!< Starting lambda
      int       elambda;             //!< Ending lambda
      int       dbg_level;           //!< Debug level

      bool      is_absorb;           //!< Flag if import is absorbance
      bool      is_mwl;              //!< Flag if multi-wavelength
      bool      is_raw;              //!< Flag if loaded from raw CFA

      QString   cur_dir;             //!< Selected import data directory
      QString   dbfile;              //!< Full path .sqlite DB file
      QString   runID;               //!< Run ID
      QString   runType;             //!< Run Type (usually "RI")

   private slots:

      // Scan CFA .sqlite file and return its values
      void   scan_cfafile  ( const QString, const int );
      // Create key-indexed mappings of useful values
      void   mapCounts     ( void );
      // Return a scan number index within the tScanMeta vector
      int    scan_nbr_index( const QString, const int );
      // Return a scan ID index within the tCFAMeta vector
      int    scan_id_index( const int );
      // Get data readings from a range of Scans table records
      int    get_readings  ( QVector< double >&, const int, const int );

};
#endif
