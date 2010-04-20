//! \file us_buffer.h
#ifndef US_BUFFER_H
#define US_BUFFER_H

#include <QtCore>
#include "us_extern.h"
#include "us_db2.h"

//! The internal structure of a buffer component

class US_EXTERN US_BufferComponent
{
   public:
      QString componentID;       //!< The BufferComponentID field in the DB.
      QString name;              //!< The buffer component's description. 
      QString unit;              //!< The buffer component value's unit.
      double  dens_coeff[ 6 ];   //!< The density coefficients. 
      double  visc_coeff[ 6 ];   //!< The viscosity coefficients. 
      QString range;             //!< The buffer component's range. 

      //! Get all component data from the DB
      //! \param masterPW The user's master password.
      //! \param componentList A reference to where the data should be placed.
      static void getAllFromDB ( const QString&, QList< US_BufferComponent >& );

      //! Get all component data from the etc/buffer_comp.xml file
      //! \param componentList A reference to where the data should be placed.
      static void getAllFromHD ( QList< US_BufferComponent >& );

      //! Write all component data from the etc/buffer_comp.xml file
      //! \param componentList A reference to where the data is found.
      static void putAllToHD ( const QList< US_BufferComponent >& );

   private:
      //! Get the info for an individual component from the DB.
      //! \param db A \ref US_DB2 structure to an opened connection to the DB.
      void getInfoFromDB( US_DB2& );

      static void component( QXmlStreamReader&, QList< US_BufferComponent >& );
};

//! The internal structure of a buffer.

class US_EXTERN US_Buffer 
{
   public:
      int     personID;    //!< Investigator's ID of for this  buffer.
      QString person;      //!< Convenience value of investigator's name.
      QString bufferID;    //!< The buffer's DB ID, or -1 if from harddrive.
      QString description; //!< The buffer's description. 
      QString spectrum;    //!< Gaussian data for absorption data
      double  pH;          //!< Acidity or basicity of the buffer
      double  density;     //!< The density of the buffer. 
      double  viscosity;   //!< The viscosity of the buffer. 
      
      //! An associative array of extinction coefficients indexed by wavelength.
      QMap< double, double > extinction;  
      //! An associative array of refraction coefficients indexed by wavelength.
      QMap< double, double > refraction;
      //! An associative array of fluorescense coefficients indexed by wavelength.
      QMap< double, double > fluorescence;
      
      //! The list of ingredients
      QList< US_BufferComponent > component;  //!< A list of components that 
                                      //!< make up the buffer.
      QList< double > concentration;  //!< Concentrations for each component.
      
      //! Get the info for a buffer from the DB.  The class's bufferID 
      //! must be set.
      //! \param masterPW The user's master password.
      void getInfoFromDB( const QString& );

      //! \brief Write the buffer to a disk file
      //! \param filename  The full path, includingfilename.
      //! \return A boolean success or failure
      bool writeToDisk( const QString& ) const;

      //! \brief Read a buffer from a disk file
      //! \param filename  The full path, includingfilename.
      //! \return A boolean success or failure
      bool readFromDisk( const QString& );

      //! \brief Get spectrum data from the DB for a type
      //! \param db The open database connection
      //! \param type The type of data to retrieve.  One of:  "Extinction", 
      //!             "Refraction", or "Fluorescence"
      void getSpectrum( US_DB2&, const QString& );

      //! \brief Put spectrum data to the DB for a type
      //! \param db The open database connection
      //! \param type The type of data to put to the DB.  One of:  "Extinction", 
      //!             "Refraction", or "Fluorescence"
      void putSpectrum( US_DB2&, const QString& );

   private:
      void readBuffer  ( QXmlStreamReader& );
      void readSpectrum( QXmlStreamReader& );
};
#endif
