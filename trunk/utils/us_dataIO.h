//! \file us_dataIO.h
#ifndef US_DATAIO_H
#define US_DATAIO_H

#include <QtCore>

class US_DataIO
{
   public:

      static const uint format_version = 2;

      class reading
      {
         public:
         union
         {
            double radius;
            double wavelength; // Will be wavelength for W data
         } d;
         double value;
         double stdDev;  // Doesn't exist for P data
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

      class beckmanRaw
      {
         public:
         QString description;
         char    type;   // I P R W F
         char    channel;
         int     cell;
         double  temperature;
         double  rpm;
         double  seconds;
         double  omega2t;
         union
         {
            double wavelength;
            double radius;      // Will be radius for W data
         } t;
         int     count;

         QList< reading > readings;
      };

      class scan
      {
         public:
         double temperature;
         double rpm;
         double seconds;
         double omega2t;
         double wavelength;
         double plateau;         // Reading value
         double delta_r;
         QList< reading > readings;
         QByteArray interpolated; 
      };

      class rawData
      {
         public:
         char    type[ 2 ];
         char    guid[ 16 ];
         int     cell;
         char    channel;
         QString description;
         
         QList< scan > scanData;
      };

      class editedPoint
      {
         public:
         int    scan;
         double radius;
         double value;
      };

      class editValues
      {
         public:
         editValues()
         {
            excludes    .clear();
            editedPoints.clear();
            invert       = 1.0;
            removeSpikes = false;
            floatingData = false;
            noiseOrder   = 0;

            // Air Gap is only for interference data
            airGapLeft   = 0.0;
            airGapRight  = 9.0;
         };

         QString              runID;
         QString              cell;
         QString              channel;
         QString              wavelength;
         QString              uuid;
         double               meniscus;
         double               airGapLeft;
         double               airGapRight;
         double               rangeLeft;
         double               rangeRight;
         double               plateau;       // Radius value
         double               baseline;
         QList< int >         excludes;
         QList< editedPoint > editedPoints;
         int                  noiseOrder;
         double               invert;
         bool                 removeSpikes;
         bool                 floatingData;
      };

      class editedData
      {
         public:
         QString       runID;
         QString       editID;
         QString       dataType;
         QString       cell;
         QString       channel;
         QString       wavelength;
         QString       description;
         QString       uuid;
         double        meniscus;
         double        plateau;   // Radius value
         double        baseline;
         bool          floatingData;
         QList< scan > scanData;       // The interpolated data array is omitted
      };

      enum ioError { OK, CANTOPEN, BADCRC, NOT_USDATA, BADTYPE, BADXML, 
                     NODATA, NO_UUID_MATCH, BAD_VERSION };

      static bool    readLegacyFile( const QString&, beckmanRaw& );
      static int     writeRawData  ( const QString&, rawData& );
      static int     readRawData   ( const QString&, rawData& );
      static int     readEdits     ( const QString&, editValues& );
      static QString errorString   ( int );
      static int     index         ( const scan&, double );
      static int     loadData      ( const QString&, const QString&, 
                                     QList< editedData >&, QList< rawData >& );

   private:

      class parameters
      {
         public:
         double min_data1;
         double max_data1;
         double min_data2;
         double max_data2;
      };

      static void writeScan( QDataStream&, const scan&, ulong&, 
                             const parameters& );

      static void write( QDataStream&, const char*, int, ulong& );
      static void read ( QDataStream&,       char*, int, ulong& );
      
      static void ident     ( QXmlStreamReader&, editValues& );
      static void run       ( QXmlStreamReader&, editValues& );
      static void params    ( QXmlStreamReader&, editValues& );
      static void operations( QXmlStreamReader&, editValues& );
      static void do_edits  ( QXmlStreamReader&, editValues& );
      static void excludes  ( QXmlStreamReader&, editValues& );

      static void copyRange  ( double, double, const scan&, scan& );
      static bool spike_check( const scan&, int, int, int, double* );
      static QList< double > calc_residuals( int, const QList< scan >& );
};
#endif
