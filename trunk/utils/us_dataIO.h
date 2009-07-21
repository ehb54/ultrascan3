//! \file us_dataIO.h
#ifndef US_DATAIO_H
#define UD_DATAIO_H

#include <QtCore>
#include <vector>

using namespace std;

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

   vector< reading > readings;
};

class scan
{
   public:
   double temperature;
   double rpm;
   double seconds;
   double omega2t;
   double wavelength;
   vector < reading > values;
   unsigned char* interpolated; 
};

class rawData
{
   public:
   char    type[ 2 ];
   char    guid[ 16 ];
   QString description;
   
   vector< scan > scanData;
};

class US_DataIO
{
   public:

      enum { OK, CANTOPEN, BADCRC };

      static bool readLegacyFile( const QString&, beckmanRaw& );
      static int  writeRawData  ( const QString&, rawData& );
//      static int  readRawData   ( const QString&, rawData& );

   private:

      class parameters
      {
         public:
         double min_data1;
         double max_data1;
         double min_data2;
         double max_data2;
      };

      static void writeScan( QDataStream&, const scan&, 
                             unsigned long&, const parameters& );

      static void write( QDataStream&, char*, int, unsigned long& );
};

#endif
