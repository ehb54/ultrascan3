#ifndef US_MWL_DATA_H
#define US_MWL_DATA_H

#include <QtCore>
#include <QtGui>

#include "us_extern.h"
#include "us_dataIO2.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()  //!< debug-level-conditioned qDebug()
#endif

class US_MwlData : public QObject
{
   Q_OBJECT

   public:
      US_MwlData( );

      //! Data description, including header values, extracted
      //!  from each raw input MWL file.
      class DataHdr
      {
         public:
            double   temperature;   // Scan temperature
            double   omega2t;       // Omega-squared-T
            double   radius_start;  // Radius start
            double   radius_step;   // Radius step increment
            int      icell;         // Cell index (0,...)
            int      ichan;         // Channel index (0,...)
            int      iscan;         // Scan index (0,...)
            int      rotor_speed;   // Rotor speed in RPM
            int      elaps_time;    // Scan elapsed time in seconds
            int      npoint;        // Number of radius data points
            int      nlambda;       // Number of wavelengths
            QChar    cell;          // Cell character (e.g., '1')
            QChar    channel;       // Channel character (e.g., 'A')
      };

      class RunHdr
      {
         public:
            QString  runID;
            bool     speed_mode;
            bool     intensity;
            QMap< QString, QString > descriptions;
      };

      //! Import data from a specified directory
      bool    import_data   ( QString&, QLineEdit* );
      //! Return reading values for given triple, scan
      int     rvalues       ( int&, int&, QVector< double >& );
      //! Return raw input reading values for triple,scan
      int     rvalues_raw   ( int&, int&, QVector< double >& );
      //! Return lambda values
      int     lambdas       ( QVector< double >& );
      //! Return lambda values for raw original
      int     lambdas_raw   ( QVector< double >& );
      //! Update output lambda range
      int     set_lambdas   ( double = 0.0, double = 0.0, double = 0.0 );
      //! Do lambda averaging 
      int     average_lambda( void );
      //! Match lambda in original list
      int     indexOfLambda ( double );
      //! Return list of cells/channels
      int     cellchannels  ( QStringList& );
      //! Build RawData vector
      int     build_rawData ( QVector< US_DataIO2::RawData >& );
      //! A count of specified type
      int     countOf       ( QString );
      //! Return cell/channel description string
      QString cc_description( QString );
      //! Return runID string
      void    run_values    ( QString&, QString& );
      //! Clear all the data structures
      void    clear         ( void  );

   private:
      QVector< QVector< double > >   iraw_reads;  // Raw readings
      QVector< QVector< double > >   avgd_reads;  // Averaged readings

      QVector< double >   iraw_wvlns;  // Raw input wavelengths
      QVector< double >   avgd_wvlns;  // Averaged wavelengths

      QVector< int >      avgd_knts;   // Averaging counts used per triple

      QList< DataHdr >    headers;     // Mwl input file headers

      QStringList         fpaths;      // Input file paths
      QStringList         fnames;      // Input file names
      QStringList         cells;       // Input Cell strings
      QStringList         cellchans;   // Input Cell/Channel strings

      QStringList         ccdescs;     // Cell/Channel description strings
      QStringList         triples;     // Output triple strings  ("1 / A / 280")
      QStringList         trnodes;     // Output triple file nodes ("1.A.280")

      QLineEdit*          le_status;   // Status report LineEdit

      int       nfile;                 // Number of input files
      int       nscan;                 // Number of scans per triple
      int       ncell;                 // Number of cells
      int       nchan;                 // Number of channels
      int       ncelchn;               // Number of Cell/Channels
      int       nlambda;               // Number wavelengths output
      int       nlamb_i;               // Number wavelengths input
      int       ntriple;               // Number triples output
      int       ntrip_i;               // Number triples input
      int       npoint;                // Number radius points per scan
      int       npointt;               // Number points per triple
      int       lavgg;                 // Lambda averaging points
      int       curccx;                // Current cell/chan index
      int       dbg_level;             // Debug level

      QMap< QString, int >     counts; // Map of counts ('File','Scan',...)

      double    dlambda;               // Delta for output lambdas
      double    slambda;               // Starting output lambda
      double    elambda;               // Ending output lambda

      QString   cur_dir;               // Currently selected i/p data directory
      QString   runID;                 // Run ID
      QString   runType;               // Run Type (e.g., "RI")

      bool      speed_mode;            // Speed_mode from run XML
      bool      intensity;             // Intensity flag from run XML

   private slots:

      int    hword   ( char* );
      int    iword   ( char* );
      float  fword   ( char* );
      double dword   ( char* );
      void   read_header ( QDataStream&, DataHdr& );
      void   read_lambdas( QDataStream&, QVector< double >&, int& );
      void   read_rdata  ( QDataStream&, QVector< double >&, int&, int& );
      void   read_runxml ( QDir, QString );
      void   mapCounts   ( void );

};
#endif
