//! \file us_hardware.h
#ifndef US_HARDWARE_H
#define US_HARDWARE_H

#include <QtCore>
#include "us_extern.h"
#include "us_util.h"
#include "us_db2.h"

//! \brief A set of static functions to handle hardware data
class US_EXTERN US_Hardware
{
   public:

   //! General rotor characteristics
   class RotorInfo
   {
      public:
      int     serial_number;    //!< Rotor's serial number (series starts at zero)
      QString type;             //!< Name of rotor
      double  coefficient[ 5 ]; //!< Stretching coefficients for 5th order
                                //!<  polynomial  (radius vs speed in rpm)
   };

   //! General centerpiece characteristics
   class CenterpieceInfo
   {
      public:
      //! The centerpiece's serial number (series starts zero)
      int     serial_number;     
      QString material;             //!< epon, aluminum, titanium
      int     channels;             //!< number of channels divided by 2
      
      //! Bottom position of each channel (max 4) 
      //! If sector==3: position for synth. boundary
      double  bottom_position[ 4 ]; 
      
      //! 0 sector shaped, 1 for rectangular, 2 for circular, 
      //! 3 for synthetic boundary cells, 4 for band-forming centerpiece
      int     sector; 
      
      //! Pathlength of centerpiece in cm, default: 1.2 cm
      double  pathlength;   
      
      //! Angle of sector, if sector shaped, default: 2.5 degrees
      double  angle;    
      
      //! Width of channel, if rectangular, or radius if sector==2, 0 otherwise.
      double  width;
   };

   //! \brief Read rotor information from a local XML file to a QMap
   //! \param rotor_map Reference of QMap into which to place data
   static bool readRotorMap( QMap< QString, QString >& );
   
   //! \brief Read rotor information from the database to a QMap
   //! \param db        Pointer to opened database connection
   //! \param rotor_map Reference of QMap into which to place data
   static bool readRotorMap( US_DB2*, QMap< QString, QString >& );
   
   //! \brief Get type, coefficient values for a specified rotor
   //! \param serial    Rotor serial for which to get values
   //! \param rotor_map QMap of serial,value mappings
   //! \param type      Reference into which to return rotor type string
   //! \param rotcoeffs Array of 5 doubles to fill with rotor coefficients
   static bool rotorValues ( QString, QMap< QString, QString >,
                             QString&, double* );
   
   //! \brief Read rotor information into a structure
   //! \param rotor_list Reference of structure data to place data
   static bool readRotorInfo      ( QVector< RotorInfo >& );
   
   //! \brief Read centerpiect information into a structure
   //! \param cp_list Reference of structure to place data 
   static bool readCenterpieceInfo( QVector< CenterpieceInfo >& );
};

#endif
