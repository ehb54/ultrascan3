//! \file us_dataIO.h
#ifndef US_DATAIO_H
#define UD_DATAIO_H

#include <QtCore>
#include <vector>

using namespace std;

struct reading
{
   union
   {
      float radius;
      float wavelength; // Will be wavelength for W data
   } d;
   float value;
   float stdDev;  // Doesn't exist for P data
};

/*!  This is the structure of a Beckman raw data file.  The file
 *   types are:
 *   I intensity
 *   P interference
 *   R absorbance
 *   W multi-wavelength
 *   F fluorescence
 *
 *   For multi wavelength data, the wavelength and radius data are in
 *   opposite positions.
 */

struct beckmanRaw
{
   QString description;
   char    type;   // I P R W F
   int     cell;
   float   temperature;
   float   rpm;
   int     seconds;
   float   omega2t;
   union
   {
      float wavelength;
      float radius;      // Will be radius for W data
   } t;
   int     count;

   vector< struct reading > readings;
};

class US_DataIO
{
   public:

      enum { ABSORBANCE, INTENSITY, INTERFERENCE, FLUORESCENCE, WAVELENGTH };
      static bool readLegacyFile( const QString&, struct beckmanRaw& );
};

#endif
