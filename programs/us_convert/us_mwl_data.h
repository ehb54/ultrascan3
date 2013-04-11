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

      //! Import data from a specified directory
      bool    import_data   ( QString&, QLineEdit* );
      //! Return reading values for given triple, scan
      int     rvalues       ( int&, int&, QVector< double >& );
      //! Return lambda values
      int     lambdas       ( QVector< double >& );
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
      QVector< QVector< double > >   ri_readings; // Raw input readings

      QVector< double >              ri_wavelns;  // Raw input wavelengths

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
      int       nlambda;               // Number of wavelengths
      int       ntriple;               // Number of triples
      int       npoint;                // Number radius points per scan
      int       npointt;               // Number points per triple
      int       curccx;                // Current cell/chan index
      int       dbg_level;             // Debug level

      QMap< QString, int >  counts;    // Map of counts ('File','Scan',...)

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
