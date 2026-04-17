//! \file us_dataIO.h
#ifndef US_DATAIO_H
#define US_DATAIO_H

#include "us_extern.h"

//! \brief Data structures and methods to read/write experimental data
//!
/*! The US_DataIO class provides data structures and static methods to
    read and write experimental data.
   
    For memory efficiency, there is only a single vector of radius
    values that applies to all the scans. So for instance, if you have
    an instance of RawData or EditedData like this:
   
    EditedData* data;
   
    you could refer to an individual scan reading value like this:
   
    data->scanData[ scanIndex ].rvalues[ readingIndex ]

*/
class US_UTIL_EXTERN US_DataIO
{
   public:

      //!  A constant value that identifies the format of the auc raw data.
      //!  Internal programs use this value to determine if the format of the
      //!  file is known.
      static const uint format_version = 5;

      //! \brief Beckman Raw data scan
      //!
      /*! This is the structure of a Beckman raw data file.  The file
          types are:
           - I intensity
           - P interference
           - R absorbance
           - W multi-wavelength
           - F fluorescence
        
           For multi wavelength data, the wavelength and radius data are in
           opposite positions.
       */
      class BeckmanRawScan
      {
         public:
         QString description; //!<First line of data file
         char    type;        //!< I P R W F  (1st Element of 2nd line)
         char    channel;     //!< Channel ('A', 'B', etc) of scan data
         int     cell;        //!< Cell (hole) of rotor for this data
         double  temperature; //!< Temperature while acquiring data (C)
         double  rpm;         //!< RPM of rotor for this file
         double  seconds;     //!< Time elapsed since start of run
         double  omega2t;     //!< Calculated integration of w2t since start of run
         double  rpoint;      //!< Radius (or Wavelength) point
         int     count;       //!< Actual number of readings for this scan
         bool    nz_stddev;   //!< Flag if any of stddevs is non-zero
         QVector< double > xvalues; //!< Wavelength or radius information
         QVector< double > rvalues; //!< The scan readings values (rad or wvln)
         QVector< double > stddevs; //!< The scan readings standard deviations
      };

      //! \brief Single scan parameter and readings
      //!
      //! The scanned data in the UltraScan III format and the specified
      //! plateau after editing.  The Beckman data is interpolated so there
      //! are no missing entries.  
      class Scan
      {
         public:
         double temperature;     //!< Temperature while acquiring data (C)
         double rpm;             //!< RPM of rotor for this scan
         double seconds;         //!< Time elapsed since start of run
         double omega2t;         //!< Calculated integration of w2t since start of run
         double wavelength;      //!< Wavelength setting of optical system
         double plateau;         //!< Reading value
         double delta_r;         //!< Radial distance between Readings
         bool   nz_stddev;       //!< Flag if any of stddevs is non-zero
         QVector< double > rvalues; //!< The readings values (radius/wavelength)
         QVector< double > stddevs; //!< The scan readings standard deviations
         QByteArray interpolated; //!< Bit array, 1 ea. Reading: 0=actual; 1=interpolated
      };

      //! All data associated with a cell / channel / wavelength (CCW)
      class US_UTIL_EXTERN RawData
      {
         public:
         char    type[ 2 ];         //!< Data type: "RA"|"IP"|"RI"|"FI"|"WA"|"WI"
         char    rawGUID[ 16 ];     //!< A generated globally unique identifier
         int     cell;              //!< Cell (hole) of rotor for this data
         char    channel;           //!< Channel ('A', 'B', etc) of scan data
         QString description;       //!< Descriptive data taken from \ref BeckmanRawScan

         QVector< double > xvalues;  //!< Wavelength or radius information
         QVector< Scan >   scanData; //!<  The collections of scans for the CCW

         int    pointCount  ( void );        //!< Number of readings points
         int    scanCount   ( void );        //!< Number of scans

         //! Find the nearest lower bound index for a given x-value.
         //! Returns the index of the largest xvalue that is <= the search value.
         //! Always returns a valid index (never -1). For values below range returns 0,
         //! for values above range returns the last index.
         //! \param value The x-value to search for
         //! \return Valid array index (0 to pointCount()-1)
         //! \warning Requires non-empty xvalues array (no bounds checking performed)
         //!
         //! Examples for array [1.0, 2.0, 3.0]:
         //! xindex(1.5) = 0 (between elements → lower bound)
         //! xindex(4.0) = 2 (above range → last index)
         //! xindex(0.5) = 0 (below range → first index)
         int    xindex      ( double );      //!< Get index of X (radius) value
         double radius      ( int );         //!< Get radius value at index
         double scWavelength( int );         //!< Get wavelength value at index
         double value       ( int, int );    //!< Get reading for scan,radius
         double reading     ( int, int );    //!< Get reading for scan,radius
         bool   setValue    ( int, int, double ); //!< Set reading value
         double std_dev     ( int, int );    //!< Get std.dev. for scan,radius
         double average_temperature() const; //!< Calculate average temperature
         double temperature_spread () const; //!< Calculate temperature spread
      };

      //! Additional data for each triplet, if equilibrium data
      class SpeedData // For equilibrium sets
      {
         public:
            int        first_scan;  //!< First scan index (1,...) of this triplet/speed set
            int        scan_count;  //!< Number of scans in this triplet/speed set
            double     speed;       //!< Speed (rpm) of this set
            double     meniscus;    //!< Meniscus radius
            double     dataLeft;    //!< Minimum radius of valid data, this set
            double     dataRight;   //!< Maximum radius of valid data, this set
      };

      //! \brief Changes made to a scan value
      //!
      //! Holds changes made to a scan value.  Created by the data editor and
      //! applied when Reading data for analysis.
      class EditedPoint
      {
         public:
         int    scan;    //!< The index of the scan
         int    radius;  //!< The radius index of the value to be changed
         double value;   //!< The new value
      };

      //! \brief Entries for edits of a scan.
      //!
      //!< A class that holds all entries associated with the edits of
      //! a scan.
      class EditValues
      {
         public:
         //! A constructor that provides initial values for scan edits
         EditValues()
         {
            excludes    .clear();
            lambdas     .clear();
            editedPoints.clear();
            invert       = 1.0;
            removeSpikes = false;
            floatingData = false;
            noiseOrder   = 0;
            ODlimit      = 1.5;
            bottom       = 0.0;

	    bl_corr_slope = 0.0;
	    bl_corr_yintercept = 0.0; 

            //! Air Gap is only for interference data
            airGapLeft   = 0.0;
            airGapRight  = 9.0;
         };

         QString       expType;     //!< Experiment type
         QList< SpeedData > speedData;   //!< Speed step data list
         QString       runID;       //!< Specified runID of raw data
         QString       cell;        //!< Cell (hole) of rotor for this data
         QString       channel;     //!< Channel ('A', 'B', etc) of scan data
         QString       wavelength;  //!< Wavelength setting of optical system
         QString       editGUID;    //!< A globally unique ID for edit
         QString       dataGUID;    //!< A globally unique ID for data
         double        meniscus;    //!< Designated radius of meniscus
         double        bottom;      //!< Designated radius of bottom
         double        airGapLeft;  //!< Start of air gap for interference data
         double        airGapRight; //!< End of air gap for interfence data
         double        gapTolerance;//!< Tolerance for interference adjustments
         double        rangeLeft;   //!< Minimum radius for valid data
         double        rangeRight;  //!< Maximum radius for valid data
         double        plateau;     //!< A radius value
         double        baseline;    //!< Designated baseline value of data
         double        ODlimit;     //!< OD limit for RMSD calculations
         QList< int >  excludes;    //!< List of scans excluded from raw data
         QList< int >  lambdas;     //!< A list of lambdas from the raw data
         QList< EditedPoint > editedPoints; //!< List of points spec. changed
         int           noiseOrder;   //!< Order of polynomial for noise removal
         double        invert;       //!< Flag to invert sign of data: 1. or -1.
         bool          removeSpikes; //!< Flag to run spike removal algorithm
         bool          floatingData; //!< Flag analyte density < buffer density

	 double             bl_corr_slope;
         double             bl_corr_yintercept; 
      };

      //! The CCW data after edits are applied
      class US_UTIL_EXTERN EditedData
      {
         public:
         QString       expType;      //!< Experiment type:
                                     //!<  velocity|equilibrium|diffusion|other
         QList< SpeedData > speedData;  //!< Speed step list, equilibrium data
         QString       runID;        //!< Specified runID of raw data
         QString       editID;       //!< ID of file containing edits for data
         QString       dataType;     //!< Sensor type
         QString       cell;         //!< Cell (hole) of rotor for this data
         QString       channel;      //!< Channel ('A', 'B', etc) of scan data
         QString       wavelength;   //!< Wavelength setting of optical system
         QString       description;  //!< ASCII description of the data
         QString       editGUID;     //!< A globally unique ID for edit
         QString       dataGUID;     //!< A globally unique ID for data
         double        meniscus;     //!< Designated radius of meniscus
         double        bottom;       //!< Designated radius of bottom
         double        plateau;      //!< Loc. max. value of data, a radius
         double        baseline;     //!< Designated baseline value of data
         double        ODlimit;      //!< OD limit for RMSD calculations
         bool          floatingData; //!< Flag analyte density < buffer density
         QVector< double > xvalues;  //!< Wavelength or radius information
         QVector< Scan >   scanData; //!< The actual data. Interpolated omitted

	     double             bl_corr_slope;
         double             bl_corr_yintercept; 

         int    pointCount  ( void );        //!< Number of readings points
         int    scanCount   ( void );        //!< Number of scans
         int    xindex      ( double );      //!< Get index of X (radius) value
         double radius      ( int );         //!< Get radius value at index
         double scWavelength( int );         //!< Get wavelength value at index
         double value       ( int, int );    //!< Get reading for scan,radius
         double reading     ( int, int );    //!< Get reading for scan,radius
         bool   setValue    ( int, int, double ); //!< Set reading value
         double std_dev     ( int, int );    //!< Get std.dev. for scan,radius
         double average_temperature() const; //!< Calculate average temperature
         double temperature_spread () const; //!< Calculate temperature spread
      };

      //! The CCW data after edits are applied
      class NewEditedData
      {
         public:
         QString       runID;        //!< Specified runID of raw data
         QString       editID;       //!< Identifier of file with edits for data
         QString       dataType;     //!< Sensor type
         QString       cell;         //!< Cell (hole) of rotor for this data
         QString       channel;      //!< Channel ('A', 'B', etc) of scan data
         QString       wavelength;   //!< Wavelength setting of optical system
         QString       description;  //!< ASCII description of the data
         QString       editGUID;     //!< A globally unique ID for edit
         QString       dataGUID;     //!< A globally unique ID for data
         double        meniscus;     //!< Designated radius of meniscus
         double        bottom;       //!< Designated radius of bottom
         double        plateau;      //!< Loc. max. value of data, a radius
         double        baseline;     //!< Designated baseline value of data
         double        ODlimit;      //!< OD limit for RMSD calculations
         bool          floatingData; //!< Flag analyte density < buffer density
         QVector< Scan > scanData;   //!< The actual data. Interpolated omitted
      };

      //! Values that are returned for various errors when managing scan data
      enum ioError { OK, CANTOPEN, BADCRC, NOT_USDATA, BADTYPE, BADXML, 
                     NODATA, NO_GUID_MATCH, BAD_VERSION };

      /*! Read a set of legacy data in raw teckman data set format
          \param file  The filename to be read
          \param data  A reference to the data structure location for read data
      */
      static bool    readLegacyFile( const QString&, BeckmanRawScan& );
      
      /*! Write a set of data to a file in the US3 binary format
          \param file  The filename to be read
          \param data  A reference to the data structure of
                       the data to be written
      */
      static int     writeRawData( const QString&, RawData& );
      
      /*! Read a set of data in the US3 binary format
          \param file  The filename to be read
          \param data  A reference to the data structure for the read data
      */
      static int     readRawData ( const QString&, RawData& );

      /*! Read a set of edit parameters in xml format
          \param filename   The filename to be read
          \param parameters A reference to the data structure for the read data
      */
      static int     readEdits   ( const QString&, EditValues& );

      /*! A string describing the last read or write error
          \param code  The error code to be described
      */
      static QString errorString ( int );

      /*! Determine the index of a radius in a vector of radii
          \param xvals   A vector of data radius information
          \param xvalue  The radius where the index is needed
      */
      static int     index       ( const QVector< double >&, double );

      /*! Determine the index of a radius in a RawData set
          \param rdata   A pointer to the RawData set to examine
          \param xvalue  The radius where the index is needed
      */
      static int     index       ( US_DataIO::RawData*, double );

      /*! Determine the index of a radius in an EditedData set
          \param edata   A pointer to the EditedData set to examine
          \param xvalue  The radius where the index is needed
      */
      static int     index       ( US_DataIO::EditedData*, double );

      /*! Load edited data into a data structure.  This is the method most
          analysis programs will call to read data into memory.  It uses
          the functions readRawData and readEdits.
          \param directory    The directory of the auc files
          \param editFilename The the file with the edited parameters
          \param data         The location where the edited data is placed
          \param raw          The location where the raw data is placed
      */

      static int     loadData    ( const QString&, const QString&, 
                                   QVector< EditedData >&,
                                   QVector< RawData >& );

      /*! This is an overloaded method to read data into memory.  It uses
          the functions readRawData and readEdits.
          \param directory    The directory of the auc files
          \param editFilename The the file with the edited parameters
          \param data         The location where the edited data is placed
      */

      static int     loadData    ( const QString&, const QString&, 
                                   QVector< EditedData >& ); 

      /*! This is an overloaded method to read data into memory.  It uses
          the functions readRawData and readEdits.
          \param directory    The directory of the auc files
          \param editFilename The the file with the edited parameters
          \param data         The location where the edited data is placed
      */

      static int     loadData    ( const QString&, const QString&, 
                                   EditedData& ); 

      /*! Adjust interference data by aligning air gaps at zero fringes 
          \param data A reference to the RawData structure to be adjusted
          \param ev   The EditValues used for the adjustment.  Only
                      airGapLeft, airGapRight, and includes are needed.
       */

      static void    adjust_interference( RawData&, const EditValues& );
  
      /*! Align integral fringe shifts within the valid data range 
          \param data A reference to the RawData structure to be adjusted
          \param ee   The EditValues used for the adjustment.  Only
                      rangeLeft, rangeRight, and gapTolerance are needed.
       */

      static void    calc_integral ( RawData&, const EditValues& );
  
      /*! Check a scan point to see if it is a spike relative to the 
          non-interpolated points around it
          \param sc       The scan to check
          \param xvalues  The vector of radius values associated with
                          the scan
          \param point    The inde of the point to check
          \param start    The beginning of the valid scan range
          \param end      The end of the valid scan range
          \param value    The value of the interpolated point if
                          a spike is detected
          \return         A flag indication if the point is a spike
      */
      
      static bool    spike_check ( const Scan&, const QVector< double >&, 
                                   int, int, int, double* );

   private:

      //!  \private A private convenience class
      class Parameters
      {
         public:
         double min_data1;
         double max_data1;
         double min_data2;
         double max_data2;
      };

      static void writeScan  ( QDataStream&, const Scan&, quint32&, 
                               const Parameters& );
      static void write      ( QDataStream&, const char*, int, quint32& );
      static void read       ( QDataStream&,       char*, int, quint32& );
      
      static void ident      ( QXmlStreamReader&, EditValues& );
      static void run        ( QXmlStreamReader&, EditValues& );
      static void all_lambdas( QXmlStreamReader&, EditValues& );
      static void excludes   ( QXmlStreamReader&, EditValues& );
      static void params     ( QXmlStreamReader&, EditValues& );
      static void operations ( QXmlStreamReader&, EditValues& );
      static void do_edits   ( QXmlStreamReader&, EditValues& );
      static void copyRange  ( double, double, const Scan&, Scan&, 
                               const QVector< double >& );
      static void copyxRange ( double, double,
                               const QVector< double >&, QVector< double >& );
      static QList< double >
               calc_residuals( int, const QVector< Scan >& );
};
#endif
