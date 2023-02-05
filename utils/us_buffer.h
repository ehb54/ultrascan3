//! \file us_buffer.h
#ifndef US_BUFFER_H
#define US_BUFFER_H

#include <QtCore>
#include "us_extern.h"
#include "us_db2.h"

//! The internal structure of a buffer component

class US_UTIL_EXTERN US_BufferComponent
{
   public:
      QString componentID;       //!< The BufferComponentID field in the DB.
      QString name;              //!< The buffer component's description. 
      QString unit;              //!< The buffer component value's unit.
      double  dens_coeff[ 6 ];   //!< The density coefficients. 
      double  visc_coeff[ 6 ];   //!< The viscosity coefficients. 
      QString range;             //!< The buffer component's range. 
      bool    grad_form;         //!< Flag if gradient forming.

      //! Get all component data from the DB
      //! \param masterPW The user's master password.
      //! \param componentList A reference to where the data should be placed.
      static void getAllFromDB ( const QString&, QMap< QString, US_BufferComponent >& );

      //! Get all component data from the etc/buffer_comp.xml file
      //! \param componentList A reference to where the data should be placed.
      static void getAllFromHD ( QMap< QString, US_BufferComponent >& );

      //! Write all component data from the etc/buffer_comp.xml file
      //! \param componentList A reference to where the data is found.
      static void putAllToHD ( const QMap< QString, US_BufferComponent >& );

      //! Get the info for an individual component from the DB.
      //! \param db A \ref US_DB2 structure to an opened connection to the DB.
      void getInfoFromDB( US_DB2* = 0 );
   
      //! \brief Write a new buffer to the DB.
      //! \param db An open database connection
      //! \return The buffercomponentID of the new buffer component
      int saveToDB( US_DB2* = 0);

   private:
      static void component( QXmlStreamReader&, QMap< QString, US_BufferComponent >& );
};

//! The internal structure of a buffer.

class US_UTIL_EXTERN US_Buffer 
{
   public:
      //int     personID;    //!< Investigator's ID of for this  buffer.
      QString person;      //!< Convenience value of investigator's name.
      QString bufferID;    //!< The buffer's DB ID, or -1 if from harddrive.
      QString GUID;        //!< The buffer's Global Identifier
      QString description; //!< The buffer's description. 
      double  compressibility; //!< The buffer's compressibility
      double  pH;          //!< Acidity or basicity of the buffer
      double  density;     //!< The density of the buffer. 
      double  viscosity;   //!< The viscosity of the buffer. 
      bool    manual;      //!< Manual explicit unadjusted density & viscosity

      bool    replace_spectrum;
      bool    new_or_changed_spectrum;
      //static int profileID;

      //! An associative array of extinction coefficients indexed by wavelength.
      QMap< double, double > extinction;  
      //! An associative array of refraction coefficients indexed by wavelength.
      QMap< double, double > refraction;
      //! An associative array of fluorescence coefficients indexed by wavelength.
      QMap< double, double > fluorescence;
      
      //! The list of ingredients
      QList< US_BufferComponent > component;  //!< A list of components that 
                                      //!< make up the buffer.
      QList< double > concentration;  //!< Concentrations for each component.
      QStringList     componentIDs;   //!< An aux list for disk input.

      // Constructor of a null buffer
      US_Buffer();

      //! \brief Write the buffer to a disk file
      //! \param filename  The full path, including filename.
      //! \return A boolean success or failure
      bool writeToDisk( const QString& ) const;

      //! \brief Find a buffer file on the disk
      //! \param path  The full path of the directory to search
      //! \param guid  The GUID of the desired buffer 
      //! \param newFile A reference to a boolean variable.  Sets
      //!                false if the buffer exists, true otherwise.
      //! \return The file name of the buffer.  If newFile is true
      //!         the name is the next in the B???????.xml numeric sequence
      //!         but does not yet exist.
      static QString get_filename( const QString& path, const QString& guid, 
                                   bool& newFile );

      //! \brief Read a buffer from a disk file
      //! \param filename  The full path, including filename.
      //! \return A boolean success or failure
      bool readFromDisk( const QString& );

      //! \brief Write a new buffer to the DB.  
      //! \param db An open database connection
      //! \param private_buffer An indication to mark the buffer 
      //!        public "0" or private "1";
      //! \return The bufferID of the new buffer
      int saveToDB( US_DB2* = 0, const QString = "1" );

      //! \brief Read a buffer from the DB
      //! \param db  An open database connection
      //! \param bufID  ID number in string format of the buffer to be read.
      //! \return A boolean success or failure
      bool readFromDB( US_DB2*, const QString& );

      //! \brief Get spectrum data from the DB for a type
      //! \param db An open database connection
      //! \param type The type of data to retrieve.  One of:  "Extinction", 
      //!             "Refraction", or "Fluorescence"
      void getSpectrum( US_DB2*, const QString& );

      //! \brief Put spectrum data to the DB for a type
      //! \param db An open database connection
      //! \param type The type of data to put to the DB.  One of:  "Extinction",
      //!             "Refraction", or "Fluorescence"
      void putSpectrum( US_DB2*, const QString& ) const;

      //! \brief Compute composite density,viscosity coefficients
      //! \param d_coeff Output array of composite density coefficients
      //! \param v_coeff Output array of composite viscosity coefficients
      void compositeCoeffs( double*, double* );

      //! \brief A debug function to write buffer contents to stderr
      void dumpBuffer( void ) const;

   private:
      void readBuffer  ( QXmlStreamReader& );
      void readSpectrum( QXmlStreamReader& );
};
#endif
