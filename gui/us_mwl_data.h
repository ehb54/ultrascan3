//! \file us_mwl_data.h
#ifndef US_MWL_DATA_H
#define US_MWL_DATA_H

#include <QtCore>
#if QT_VERSION > 0x050000
#include <QtWidgets>
#else
#include <QtGui>
#define setSingleStep(a) setStep(a)
#define setMinorPen(a) setMinPen(a)
#define setMajorPen(a) setMajPen(a)
#define setMajorPen(a) setMajPen(a)
#endif
#include "us_extern.h"
#include "us_dataIO.h"
#include "us_simparms.h"

#ifndef SP_SPEEDPROFILE
#define SP_SPEEDPROFILE US_SimulationParameters::SpeedProfile
#endif

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()  //!< debug-level-conditioned qDebug()
#endif

//! Class for holding Multi-WaveLength Data with functions for
//! importing from raw files or loading from vectors of rawData.
class US_GUI_EXTERN US_MwlData : public QObject
{
   Q_OBJECT

   public:
      US_MwlData( );

      //! Data description, including header values, extracted
      //!  from each raw input MWL file.
      class DataHdr
      {
         public:
            double   temperature;   //!< Scan temperature
            double   omega2t;       //!< Omega-squared-T
            double   radius_start;  //!< Radius start
            double   radius_step;   //!< Radius step increment
            int      icell;         //!< Cell index (0,...)
            int      ichan;         //!< Channel index (0,...)
            int      iscan;         //!< Scan index (0,...)
            int      set_speed;     //!< Rotor speed in RPM set constant
            int      rotor_speed;   //!< Rotor speed in RPM
            int      elaps_time;    //!< Scan elapsed time in seconds
            int      npoint;        //!< Number of radius data points
            int      nlambda;       //!< Number of wavelengths
            QChar    cell;          //!< Cell character (e.g., '1')
            QChar    channel;       //!< Channel character (e.g., 'A')
      };

      //! \brief Import data from a specified directory
      //! \param mwldir  Raw MWL data directory
      //! \param lestat  Status LineEdit pointer
      //! \returns       Status of import (true->imported OK)
      bool    import_data   ( QString&, QLineEdit* );

      //! \brief Load mwl internal variables from loaded rawDatas
      //! \param allData Vector of loaded rawDatas
      void    load_mwl      ( QVector< US_DataIO::RawData >& );

      //! \brief Return reading values for given triple, scan
      //! \param tripx   Triple index
      //! \param scanx   Scan index
      //! \param rvs     Output readings values vector
      //! \returns       Index for next scan
      int     rvalues       ( int&, int&, QVector< double >& );

      //! \brief Return lambda values
      //! \param wls     Output lambdas vector for given channel
      //! \param ccx     Cell/channel index to use (-1 -> current)
      //! \returns       Number of lambdas for the cell/channel
      int     lambdas       ( QVector< int >&, int = -1 );

      //! \brief Return lambda values for raw original
      //! \param wls     Output lambdas vector for the imported data
      //! \returns       Number of lambdas in the imported MWL data
      int     lambdas_raw   ( QVector< int >& );

      //! \brief Update output lambdas by range redefinition
      //! \param start   Start lambda (0 -> first)
      //! \param end     End lambda (0 -> last)
      //! \param ccx     Cell/channel index to use (-1 -> current)
      //! \returns       Number of lambdas in the new lambda range
      int     set_lambdas   ( int = 0, int = 0, int = -1 );

      //! \brief Update output lambdas by new vector specification
      //! \param wls     New wavelengths vector to use for the channel
      //! \param ccx     Cell/channel index to use (-1 -> current)
      //! \returns       Number of lambdas in the new lambda vector
      int     set_lambdas   ( QVector< int >&, int = -1 );

      //! \brief Match lambda in original list
      //! \param lambda  Lambda value to find in the original import list
      //! \returns       Index of the lambda match in the lambdas list
      int     indexOfLambda ( int );

      //! \brief Return list of cells/channels
      //! \param celchns Output cell/channels string list
      //! \returns       Number of cell/channels in the list
      int     cellchannels  ( QStringList& );

      //! \brief Build RawData vector
      //! \param allData Output vector of rawDatas built from MWL data
      //! \returns       Number of triples in the data (size of allData)
      int     build_rawData ( QVector< US_DataIO::RawData >& );

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

      //! \brief Set the current cell/channel index
      //! \param ccx     Cell/channel index to set as the current one
      //! \returns       The actual current cell/channel index in use
      int     set_celchnx   ( int = 0 );

      //! \brief Return the output data index of a wavelength in a channel
      //! \param waveln  Wavelength in cell/channel to index
      //! \param ccx     Cell/channel index at which to search for wavelength
      //! \returns       The output data index of the triple
      int     data_index    ( int = 0, int = -1 );

      //! \brief Return the output data index of a wavelength in a channel
      //! \param clambda Wavelength (as string) in cell/channel to index
      //! \param ccx     Cell/channel index at which to search for wavelength
      //! \returns       The output data index of the triple
      int     data_index    ( QString, int = -1 );

      //! \brief Return the output data index of a wavelength in a channel
      //! \param clambda Wavelength string for search in cell/channel
      //! \param celchn  Cell/channel string for search of wavelength
      //! \returns       The output data index of the triple
      int     data_index    ( QString, QString );

      //! \brief Update the speed profile for MWL data
      //! \param speedsteps Reference to speed steps profile to update
      //! \returns          The number of speed steps in the profile
      int     update_speedsteps( QVector< SP_SPEEDPROFILE >& );

      //! \brief Return vector of raw speeds for MWL data scans
      //! \param rrpms  Reference for returned raw RPM values for each scan
      //! \returns      The number of rpm values (scans) returned
      int     raw_speeds( QVector< double >& );

   private:
      QVector< QVector< double > > ri_readings; //!< Raw input readings
      QVector< int >               ri_wavelns;  //!< Raw input wavelengths
      QVector< QVector< int > >    ex_wavelns;  //!< Export Wavelengths, ea. cc

      QVector< double >   r_rpms;    //!< Raw RPMs from scans
      QVector< double >   s_rpms;    //!< Set RPMs from speed steps
      QVector< double >   a_rpms;    //!< Average RPMs from speed steps
      QVector< double >   d_rpms;    //!< Standard Deviation RPMs from steps

      QList< DataHdr >    headers;   //!< Mwl input file headers

      QStringList         fpaths;    //!< Input file paths
      QStringList         fnames;    //!< Input file names
      QStringList         cells;     //!< Input Cell strings
      QStringList         cellchans; //!< Input Cell/Channel strings

      QStringList         ccdescs;   //!< Cell/Channel description strings
      QStringList         triples;   //!< Output triple strings  ("1 / A / 280")
      QStringList         trnodes;   //!< Output triple file nodes ("1.A.280")

      QLineEdit*          le_status; //!< Status report LineEdit

      QMap< QString, int >  counts;  //!< Map of counts ('File','Scan',...)

      int       nfile;               //!< Number of input files
      int       nscan;               //!< Number of scans per triple
      int       ncell;               //!< Number of cells
      int       nchan;               //!< Number of channels
      int       ncelchn;             //!< Number of Cell/Channels
      int       nlambda;             //!< Number of wavelengths (output)
      int       nlamb_i;             //!< Number of wavelengths (raw input)
      int       ntriple;             //!< Number of triples (output)
      int       ntrip_i;             //!< Number of triples (raw input)
      int       npoint;              //!< Number radius points per scan
      int       npointt;             //!< Number points per triple
      int       curccx;              //!< Current cell/chan index
      int       slambda;             //!< Starting output lambda
      int       elambda;             //!< Ending output lambda
      int       dbg_level;           //!< Debug level

      bool      is_absorb;           //!< Flag if import is absorbance;

      double    evers;               //!< Experiment version number

      QString   cur_dir;             //!< Currently selected i/p data directory
      QString   runID;               //!< Run ID
      QString   runType;             //!< Run Type (always "RI")

   private slots:

      int    hword   ( char* );
      int    iword   ( char* );
      float  fword   ( char* );
      double dword   ( char* );
      void   read_header ( QDataStream&, DataHdr& );
      void   read_lambdas( QDataStream&, QVector< int >&, int& );
      void   read_rdata  ( QDataStream&, QVector< double >&, int&, int& );
      void   read_runxml ( QDir, QString );
      void   mapCounts   ( void );

};
#endif
