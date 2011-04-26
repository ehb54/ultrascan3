//! \file us_dataIO2.h
#ifndef US_DATAIO2_H
#define US_DATAIO2_H

#include <QtCore>
#include "us_extern.h"

//! \brief Data structures and methods to read/write experimental data
//!
/*! The US_DataIO2 class provides data structures and static methods to
    read and write experimental data.
   
    For memory efficiency, there is only a single vector of radius
    values that applies to all the scans. So for instance, if you have
    an instance of RawData or EditedData like this:
   
    EditedData* data;
   
    you could refer to an individual scan reading value like this:
   
    data->scanData[ scanIndex ].readings[ readingIndex ].value

    and the corresponding radius value could be specified like this:

    data->radius( readingIndex )

    In wavelength data, the wavelength would be specified like this:

    data->scanWavelength( readingIndex )
*/
class US_UTIL_EXTERN US_DataIO2
{
   public:

      //!  A constant value that identifies the format of the auc raw data.
      //!  Internal programs use this value to determine if the format of the
      //!  file is known.
      static const uint format_version = 4;

      //! A class to aid in representation of radius/wavelength data
      class XValue
      {
         public:
         union
         {
            double radius;     //!< Distance from the center of rotation (in centimeters)
            double wavelength; //!< Overloaded entry for W style data
         };                    //!< Data union for above values

         XValue( void ) { }
         XValue( double value) { radius = value; } //!< A constructor
      };

      //!  A single Reading of data from the centrifuge
      class RawReading
      {
         public:
         union
         {
            double radius;     //!< Distance from the center of rotation (in centimeters)
            double wavelength; //!< Overloaded entry for W style data
         } d;                  //!< Data union for above values
         double value;   //!< Value of the sensor's reading
         double stdDev;  //!< Standard deviation of the reading.  Doesn't exist for P data.
      };

      //!  A single Reading of centrifuge data in the new format
      class Reading
      {
         public:
         double value;   //!< Value of the sensor's reading
         double stdDev;  //!< Standard deviation of the reading.  Doesn't exist for P data.

         Reading( void )               { } //!< Null constructor
         Reading( double v, double s ) { value = v; stdDev = s;   } //!< Alternate Constructor
         Reading( double v )           { value = v; stdDev = 0.0; } //!< Alternate Constructor
      };

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
         union
         {
            double radius;     //!< Distance from the center of rotation (in centimeters)
            double wavelength; //!< Overloaded entry for W style data
         } t;                  //!< Data union for above values
         int     count;        //!< Actual number of Readings for this scan

         QVector< RawReading > readings;  //!< The scan readings
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
         QVector< Reading > readings; //!< The scan readings
         QByteArray interpolated; //!< Bit array, 1 ea. Reading: 0=actual; 1=interpolated
      };

      //! All data associated with a cell / channel / wavelength (CCW)
      class RawData
      {
         public:
         char    type[ 2 ];         //!< Data type: "RA"|"IP"|"RI"|"FI"|"WA"|"WI"
         char    rawGUID[ 16 ];     //!< A generated globally unique identifier
         int     cell;              //!< Cell (hole) of rotor for this data
         char    channel;           //!< Channel ('A', 'B', etc) of scan data
         QString description;       //!< Descriptive data taken from \ref BeckmanRawScan

         QVector< XValue > x;       //!< Wavelength or radius information
         QVector< Scan > scanData;  //!<  The collections of scans for the CCW
         double radius ( int i )    
            { return x[ i ].radius; } //!< Convenience function returning radius
         double scanWavelength( int i ) 
            { return x[ i ].wavelength; } //!< Convenience function returning wavelength
         double value  ( int i, int j )
            { return scanData[ i ].readings[ j ].value; }
                                       //!< Convenience function returning a reading
         double average_temperature() const
            { double sum = 0.0;
              for ( int i = 0; i < scanData.size(); i++ )
                 sum += scanData[ i ].temperature;
              return sum / (double)scanData.size(); } //!< Average temperature of scans

         double temperature_spread() const
            { double min = 99.0;
              double max = 0.0;
              for ( int i = 0; i < scanData.size(); i++ )
              {
                 double temp = scanData[ i ].temperature;
                 min = ( temp < min ) ? temp : min;
                 max = ( temp > max ) ? temp : max;
              }
                
              return fabs( max - min );  //!< Temperature spread within scan
            }
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
         double radius;  //!< The radius of the value to be changed
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
            editedPoints.clear();
            invert       = 1.0;
            removeSpikes = false;
            floatingData = false;
            noiseOrder   = 0;

            //! Air Gap is only for interference data
            airGapLeft   = 0.0;
            airGapRight  = 9.0;
         };

         QString              expType;     //!< Experiment type
         QList< SpeedData >   speedData;   //!< Speed step data list
         QString              runID;       //!< Specified runID of raw data
         QString              cell;        //!< Cell (hole) of rotor for this data
         QString              channel;     //!< Channel ('A', 'B', etc) of scan data
         QString              wavelength;  //!< Wavelength setting of optical system
         QString              editGUID;    //!< A globally unique ID for edit
         QString              dataGUID;    //!< A globally unique ID for data
         double               meniscus;    //!< Designated radius of meniscus
         double               airGapLeft;  //!< Start of air gap for interference data
         double               airGapRight; //!< End of air gap for interfence data
         double               gapTolerance;//!< Tolerance for interference adjustments
         double               rangeLeft;   //!< Minimum radius for valid data
         double               rangeRight;  //!< Maximum radius for valid data
         double               plateau;     //!< A radius value
         double               baseline;    //!< Designated baseline value of data
         QList< int >         excludes;    //!< A list of scans excluded from the raw data
         QList< EditedPoint > editedPoints; //!< A list of points specifically changed
         int                  noiseOrder;   //!< The order of the polynomial for noise removal
         double               invert;       //!< Setting to invert sign of data: 1.0 or -1.0
         bool                 removeSpikes; //!< Flag if spike removal algorithm should be run
         bool                 floatingData; //!< Flag density of analyte less than density of buffer.
      };

      //! The CCW data after edits are applied
      class EditedData
      {
         public:
         QString       expType;     //!< Experiment type: velocity|equilibrium|diffusion|other
         QList< SpeedData > speedData;  //!< Speed step data list for equilibrium data

         QString       runID;       //!< Specified runID of raw data
         QString       editID;      //!< Identifier of file that contained edits for the data
         QString       dataType;    //!< Sensor type
         QString       cell;        //!< Cell (hole) of rotor for this data
         QString       channel;     //!< Channel ('A', 'B', etc) of scan data
         QString       wavelength;  //!< Wavelength setting of optical system
         QString       description; //!< ASCII description of the data
         QString       editGUID;    //!< A globally unique ID for edit
         QString       dataGUID;    //!< A globally unique ID for data
         double        meniscus;    //!< Designated radius of meniscus
         double        plateau;     //!< Location of maximum value of data.  A radius value.
         double        baseline;    //!< Designated baseline value of data
         bool          floatingData; //!< Flag density of analyte less than density of buffer.
         QVector< XValue > x;        //!< Wavelength or radius information
         QVector< Scan > scanData;   //!< The actual data. Interpolated array is omitted
         double radius ( int i )    
            { return x[ i ].radius; } //!< A convenience function returning a radius value
         double scanWavelength( int i ) 
            { return x[ i ].wavelength; } //!< A convenience function returning a wavelength value 
         double value  ( int i, int j ) 
            { return scanData[ i ].readings[ j ].value; } 
                                      //!< A convenience function returning a reading value
         double average_temperature() const
            { double sum = 0.0;
              for ( int i = 0; i < scanData.size(); i++ )
                 sum += scanData[ i ].temperature;
              return sum / (double)scanData.size(); } //!< Average temperature of scans

         double temperature_spread() const
            { double min = 99.0;
              double max = 0.0;
              for ( int i = 0; i < scanData.size(); i++ )
              {
                 double temp = scanData[ i ].temperature;
                 min = ( temp < min ) ? temp : min;
                 max = ( temp > max ) ? temp : max;
              }
                
              return fabs( max - min );  //!< Temperature spread within scan
            }
      };

      //! The CCW data after edits are applied
      class NewEditedData
      {
         public:
         QString       runID;       //!< Specified runID of raw data
         QString       editID;      //!< Identifier of file that contained edits for the data
         QString       dataType;    //!< Sensor type
         QString       cell;        //!< Cell (hole) of rotor for this data
         QString       channel;     //!< Channel ('A', 'B', etc) of scan data
         QString       wavelength;  //!< Wavelength setting of optical system
         QString       description; //!< ASCII description of the data
         QString       editGUID;    //!< A globally unique ID for edit
         QString       dataGUID;    //!< A globally unique ID for data
         double        meniscus;    //!< Designated radius of meniscus
         double        plateau;     //!< Location of maximum value of data.  A radius value.
         double        baseline;    //!< Designated baseline value of data
         bool          floatingData; //!< Flag density of analyte less than density of buffer.
         QVector< Scan > scanData;   //!< The actual data. Interpolated array is omitted
      };

      //! Values that are returned for various errors when managing scan data
      enum ioError { OK, CANTOPEN, BADCRC, NOT_USDATA, BADTYPE, BADXML, 
                     NODATA, NO_GUID_MATCH, BAD_VERSION };

      /*! Read a set of legacy data in raw teckman data set format
          \param file  The filename to be read
          \param data  A reference to the data structure location for the read data
      */
      static bool    readLegacyFile( const QString&, BeckmanRawScan& );
      
      /*! Write a set of data to a file in the US3 binary format
          \param file  The filename to be read
          \param data  A reference to the data structure of
                       th edata to be written
      */
      static int     writeRawData  ( const QString&, RawData& );
      
      /*! Read a set of data in the US3 binary format
          \param file  The filename to be read
          \param data  A reference to the data structure for the read data
      */
      static int     readRawData   ( const QString&, RawData& );

      /*! Read a set of edit parameters in xml format
          \param filename   The filename to be read
          \param parameters A reference to the data structure for the read data
      */
      static int     readEdits     ( const QString&, EditValues& );

      /*! A string describing the last read or write error
          \param code  The error code to be described
      */
      static QString errorString   ( int );

      /*! Determine the index of a radius in a scan
          \param s A reference of the scan to be searched
          \param x A parallel vector of corresponding radius information
          \param r The radius where the index is needed
      */
      static int     index         ( const Scan&, const QVector< XValue >&, double );


      /*! Load edited data into a data structure.  This is the method most
          analysis programs will call to read data into memory.  It uses
          the functions readRawData and readEdits.
          \param directory    The directory of the auc files
          \param editFilename The the file with the edited parameters
          \param data         The location where the edited data is placed
          \param raw          The location where the raw data is placed
      */

      static int     loadData      ( const QString&, const QString&, 
                                     QVector< EditedData >&, QVector< RawData >& );

      /*! This is an overloaded method to read data into memory.  It uses
          the functions readRawData and readEdits.
          \param directory    The directory of the auc files
          \param editFilename The the file with the edited parameters
          \param data         The location where the edited data is placed
      */

      static int     loadData      ( const QString&, const QString&, 
                                     QVector< EditedData >& ); 

      /*! This is an overloaded method to read data into memory.  It uses
          the functions readRawData and readEdits.
          \param directory    The directory of the auc files
          \param editFilename The the file with the edited parameters
          \param data         The location where the edited data is placed
      */

      static int     loadData      ( const QString&, const QString&, 
                                     EditedData& ); 

      /*! Adjust interference data by aligning air gaps at zero fringes 
          \param data A reference to the RawData structure to be adjusted
          \param ev   The EditValues used for the adjustment.  Only
                      airGapLeft, airGapRight, and includes are needed.
       */

      static void    adjust_interference( RawData&, const EditValues& );
  
      /*! Align integral fringe shifts within the valid data range 
          \param data A reference to the RawData structure to be adjusted
          \param e    The EditValues used for the adjustment.  Only
                      rangeLeft, rangeRight, and gapTolerance are needed.
       */

      static void    calc_integral ( RawData&, const EditValues& );
  
      /*! Check a scan point to see if it is a spike relative to the 
          non-interpolated points around it
          \param s     The scan to check
          \param xval  The vector of radius values associated with
                       the scan
          \param point The inde of the point to check
          \param start The beginning of the valid scan range
          \param end   The end of the valid scan range
          \param value The value of the interpolated point if
                       a spike is detected
          \return      A flag indication if the point is a spike
      */
      
      static bool    spike_check  ( const Scan&, const QVector< XValue >&, 
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

      static void writeScan( QDataStream&, const Scan&, quint32&, 
                             const Parameters& );

      static void write( QDataStream&, const char*, int, quint32& );
      static void read ( QDataStream&,       char*, int, quint32& );
      
      static void ident     ( QXmlStreamReader&, EditValues& );
      static void run       ( QXmlStreamReader&, EditValues& );
      static void params    ( QXmlStreamReader&, EditValues& );
      static void operations( QXmlStreamReader&, EditValues& );
      static void do_edits  ( QXmlStreamReader&, EditValues& );
      static void excludes  ( QXmlStreamReader&, EditValues& );

      static void copyRange    ( double, double, const Scan&, Scan&, 
                                 const QVector< XValue >& );
      static void copyxRange   ( double, double, const Scan&, 
                                 const QVector< XValue >&, QVector< XValue >& );
      static QList< double > calc_residuals( int, const QVector< Scan >& );
};
#endif
