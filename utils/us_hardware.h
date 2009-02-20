#ifndef US_HARDWARE_H
#define US_HARDWARE_H

#include <QtCore>

#include "us_extern.h"
#include "us_util.h"

struct rotorInfo
{
   int     serial_number;    //!< Rotor's serial number (series starts at zero)
   QString type;             //!< Name of rotor
   float   coefficient[ 5 ]; //!< Stretching coefficient for 5th order polynomial 
                             //!< (radius vs speed in rpm)
};

struct centerpieceInfo
{
   // The centerpiece's serial number (series starts zero)
   int     serial_number;     
   QString material;             // epon, aluminum, titanium
   uint    channels;             // number of channels divided by 2
   
   // bottom position of each channel, max 4., 
   // if sector==3: position for synth. boundary
   float   bottom_position[ 4 ]; 
   
   // 0 sector shaped, 1 for rectangular, 2 for circular, 
   // 3 for synthetic boundary cells, 4 for band-forming centerpiece
   int     sector; 
   
   // Pathlength of centerpiece in cm, default: 1.2 cm
   float   pathlength;   
   
   // Angle of sector, if sector shaped, default: 2.5 degrees
   float   angle;    
   
   // Width of channel, if rectangular, or radius if sector==2, 0 otherwise.
   float   width;
};

class US_EXTERN US_Hardware
{
   public:
      static bool readRotorInfo      ( QList< struct rotorInfo       >& );
      static bool readCenterpieceInfo( QList< struct centerpieceInfo >& );
};

#endif
