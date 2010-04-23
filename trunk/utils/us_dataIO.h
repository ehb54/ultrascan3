//! \file us_dataIO.h
#ifndef US_DATAIO_H
#define US_DATAIO_H

#include <QtCore>

/*! The US_DataIO class provides data structures and static methods to
    read and write experimental data.  */


class US_DataIO
{
   public:

      //!  A constant value that identifies the format of the auc raw data.
      //!  Internal programs use this value to determine if the format of the
      //!  file is known.
      static const uint format_version = 2;

      //!  A single reading of data from the centrifuge
      class reading
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

      /*!  This is the structure of a Beckman raw data file.  The file
       *   types are:
       *   - I intensity
       *   - P interference
       *   - R absorbance
       *   - W multi-wavelength
       *   - F fluorescence
       *
       *   For multi wavelength data, the wavelength and radius data are in
       *   opposite positions.
       */

      class beckmanRawScan
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
            double wavelength;  //!< Wavlength setting of optical system
            double radius;      //!< Will be radius for W data
         } t;                   //!< Data union for above values
         int     count;         //!< Actual number of readings for this scan

         QVector< reading > readings;  //!< The scan readings
      };

      //!  The scanned data in the UltraScan III format and the specified
      //!  plateau after editing.  The Beckman data is interpolated so there
      //!  are no missing entries.  
      class scan
      {
         public:
         double temperature;     //!< Temperature while acquiring data (C)
         double rpm;             //!< RPM of rotor for this scan
         double seconds;         //!< Time elapsed since start of run
         double omega2t;         //!< Calculated integration of w2t since start of run
         double wavelength;      //!< Wavlength setting of optical system
         double plateau;         //!< Reading value
         double delta_r;         //!< Radial distance between readings
         QVector< reading > readings; //!< The scan readings
         //! A bit array.  One bit for each reading.  0 is an actual reading and
         //! 1 is an interpolated reading.
         QByteArray interpolated; 
      };

      //!  All data associated with a cell / channel / wavelength (CCW)
      class rawData
      {
         public:
         char    type[ 2 ];         //!< Data type: "RA", "IP", "RI", "FI"
                                    //!< "WA" << "WI";

         char    guid[ 16 ];        //!< A generated globally unique identifier
         int     cell;              //!< Cell (hole) of rotor for this data
         char    channel;           //!< Channel ('A', 'B', etc) of scan data
         QString description;       //!< Descriptive data taken from \ref beckmanRawScan
         
         QVector< scan > scanData;  //!<  The collections of scans for the CCW
      };

      //! Holds changes made to a scan value.  Created by the data editor and
      //! applied when reading data for analysis.
      class editedPoint
      {
         public:
         int    scan;    //!< The index of the scan
         double radius;  //!< The radius of the value to be changed
         double value;   //!< The new value
      };

      //! A class that holds all entries assiciated with the edits of
      //! a scan.
      class editValues
      {
         public:
         //! A constructor that provides initial values for scan edits
         editValues()
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

         QString              runID;       //!< Specified runID of raw data
         QString              cell;        //!< Cell (hole) of rotor for this data
         QString              channel;     //!< Channel ('A', 'B', etc) of scan data
         QString              wavelength;  //!< Wavlength setting of optical system
         QString              uuid;        //!< A globally uniquie ID
         double               meniscus;    //!< Designated radius of meniscus
         double               airGapLeft;  //!< Start of air gap for interference data
         double               airGapRight; //!< End of air gap for interfence data
         double               rangeLeft;   //!< Minimum radius for valid data
         double               rangeRight;  //!< Maximum radius for valid data
         double               plateau;     //!< A radius value
         double               baseline;    //!< Designated baseline value of data
         QList< int >         excludes;    //!< A list of scans excluded from the raw data
         QList< editedPoint > editedPoints; //!< A list of points specifically changed
         int                  noiseOrder;   //!< The order of the polynomial for noise removal
         double               invert;       //!< A setting to invert the sign of the data.
                                            //!< Valid values are 1.0 and -1.0
         bool                 removeSpikes; //!< A setting designating whether the spike
                                            //!< removal algorithm should be run
         bool                 floatingData; //!< A value indicating that the density of
                                            //!< the analyte is less than the density of
                                            //!< the buffer.
      };

      //! The CCW data after edits are applied
      class editedData
      {
         public:
         QString       runID;       //!< Specified runID of raw data
         QString       editID;      //!< The identifier of the file that contained
                                    //!< the edits for the data
         QString       dataType;    //!< Sensor type
         QString       cell;        //!< Cell (hole) of rotor for this data
         QString       channel;     //!< Channel ('A', 'B', etc) of scan data
         QString       wavelength;  //!< Wavlength setting of optical system
         QString       description; //!< ASCII description of the data
         QString       uuid;        //!< A globally uniquie ID
         double        meniscus;    //!< Designated radius of meniscus
         double        plateau;     //!< Location of maximum value of data.  A radius value.
         double        baseline;    //!< Designated baseline value of data
         bool          floatingData; //!< A value indicating that the density of
                                     //!< the analyte is less than the density of
                                     //!< the buffer.
         QVector< scan > scanData;   //!< The actual data.  The interpolated data 
                                     //!< array is omitted
      };

      //! Values that are returned for various errors when managing scan data
      enum ioError { OK, CANTOPEN, BADCRC, NOT_USDATA, BADTYPE, BADXML, 
                     NODATA, NO_UUID_MATCH, BAD_VERSION };

      /*! Read a set of legacy data in raw Beckman data set format
          \param file  The filename to be read
          \param data  A reference to the data structure location for the read data
      */
      static bool    readLegacyFile( const QString&, beckmanRawScan& );
      
      /*! Write a set of data to a file in the US3 binary format
          \param file  The filename to be read
          \param data  A reference to the data structure of
                       th edata to be written
      */
      static int     writeRawData  ( const QString&, rawData& );
      
      /*! Read a set of data in the US3 binary format
          \param file  The filename to be read
          \param data  A reference to the data structure for the read data
      */
      static int     readRawData   ( const QString&, rawData& );

      /*! Read a set of edit parameters in xml format
          \param filename   The filename to be read
          \param parameters A reference to the data structure for the read data
      */
      static int     readEdits     ( const QString&, editValues& );

      /*! A string describing the last read or write error
          \param code  The error code to be described
      */
      static QString errorString   ( int );

      /*! Determine the index of a radius in a scan
          \param s A reference of the scan to be searched
          \param r The radius where the index is needed
      */
      static int     index         ( const scan&, double );


      /*! Load edited data into a data structure.  This is the method most
          analysis programs will call to read data into memory.  It uses
          the functions readRawData and readEdits.
          \param directory    The directory of the auc files
          \param editFilename The the file with the edited parameters
          \param data         The location where the edited data is placed
          \param raw          The location where the raw data is placed
      */

      static int     loadData      ( const QString&, const QString&, 
                                     QVector< editedData >&, QVector< rawData >& );

      /*! This is an overloaded method to read data into memory.  It uses
          the functions readRawData and readEdits.
          \param directory    The directory of the auc files
          \param editFilename The the file with the edited parameters
          \param data         The location where the edited data is placed
      */

      static int     loadData      ( const QString&, const QString&, 
                                     QVector< editedData >& ); 

   private:

      //!  \private A private convenience class
      class parameters
      {
         public:
         double min_data1;
         double max_data1;
         double min_data2;
         double max_data2;
      };

      static void writeScan( QDataStream&, const scan&, quint32&, 
                             const parameters& );

      static void write( QDataStream&, const char*, int, quint32& );
      static void read ( QDataStream&,       char*, int, quint32& );
      
      static void ident     ( QXmlStreamReader&, editValues& );
      static void run       ( QXmlStreamReader&, editValues& );
      static void params    ( QXmlStreamReader&, editValues& );
      static void operations( QXmlStreamReader&, editValues& );
      static void do_edits  ( QXmlStreamReader&, editValues& );
      static void excludes  ( QXmlStreamReader&, editValues& );

      static void copyRange  ( double, double, const scan&, scan& );
      static bool spike_check( const scan&, int, int, int, double* );
      static QList< double > calc_residuals( int, const QVector< scan >& );
};
#endif
