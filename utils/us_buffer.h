//! \file us_buffer.h
#ifndef US_BUFFER_H
#define US_BUFFER_H

#include "us_extern.h"
#include "ius_db2.h"

//! The internal structure of a buffer component

class US_UTIL_EXTERN US_BufferComponent
{
   public:
      QString componentID;       //!< The BufferComponentID field in the DB.
      QString name;              //!< The buffer component's description. 
      QString unit;              //!< The buffer component value's unit.
      double      dens_coeff[6]{ 0.0 }; //!< The density coefficients.
      double      visc_coeff[6]{ 0.0 }; //!< The viscosity coefficients.
      QString range;             //!< The buffer component's range. 
      bool    grad_form{false};         //!< Flag if gradient forming.

      //! Get all component data from the DB. Components whose rows cannot be read are skipped.
      //! \param masterPW The user's master password.
      //! \param componentList A reference to where the data should be placed.
      //! \param error If not null, receives the reason when the read fails.
      //! \return False when the DB cannot be opened, the component list cannot be queried, or
      //!         none of the listed components can be read.
      static bool getAllFromDB( const QString&, QMap<QString, US_BufferComponent>&, QString* error = nullptr );

      //! Get all component data from the etc/buffer_comp.xml file
      //! \param componentList A reference to where the data should be placed.
      static void getAllFromHD ( QMap< QString, US_BufferComponent >& );

      //! Write all component data from the etc/buffer_comp.xml file
      //! \param componentList A reference to where the data is found.
      static void putAllToHD ( const QMap< QString, US_BufferComponent >& );

      //! Get the info for an individual component from the DB.
      //! \param db A \ref IUS_DB2 structure to an opened connection to the DB.
      //! \return False, leaving the component unchanged, when the query fails,
      //!         finds no row or the row lacks six numeric coefficients per field.
      bool        getInfoFromDB( IUS_DB2* = 0 );

      //! \brief Write a new buffer to the DB.
      //! \param db An open database connection
      //! \return The buffercomponentID of the new buffer component
      int saveToDB( IUS_DB2* = 0);

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
      //! \return A boolean success or failure. The file's extinction, refraction
      //!         and fluorescence values replace the ones the buffer held. On
      //!         failure (an unreadable file, malformed or truncated XML, or no
      //!         buffer element) the buffer is unchanged.
      bool readFromDisk( const QString& );

      //! \brief Write a new buffer to the DB.
      //! \param db An open database connection
      //! \param private_buffer An indication to mark the buffer
      //!        public "0" or private "1";
      //! \param error Optional diagnostic output, cleared on entry. Preserves
      //!        transaction errors even if rollback changes the DB error state.
      //! \return The bufferID of the new buffer, or a negative error: -9 failed
      //!         lookup, -1 failed insert, -2 failed update, -3 failed component
      //!         delete, -4 failed component write, -5 failed extinction-profile
      //!         write, -6 failed disk copy (DB already committed), -7 failed begin,
      //!         -8 failed commit (rolled back, nothing kept), -10 unconfirmed rollback
      //!         (the error text says whether anything may have been kept).
      //! Joins an existing transaction, otherwise owns a new one. When joining,
      //! the owner MUST roll back on failure; success returns a provisional ID,
      //! leaves spectrum flags pending, and does not write a disk copy.
      //! Owned DB writes use one transaction; DB failures restore the previous bufferID
      //! and leave spectrum flags pending. Requires transactional tables and
      //! procedures without implicit commits (see IUS_DB2::beginTransaction).
      //! An unconfirmed rollback (-10) requires reconciliation before retrying.
      int            saveToDB( IUS_DB2* = 0, const QString = "1", QString* error = nullptr );

      //! \brief Read a buffer from the DB
      //! \param db  An open database connection
      //! \param bufID  ID number in string format of the buffer to be read.
      //! \return A boolean success or failure. On failure (a failed query, a
      //!         missing buffer row or an unreadable component) the buffer is
      //!         unchanged. A buffer with no stored profile loads with empty
      //!         extinction values.
      bool readFromDB( IUS_DB2*, const QString& );

      //! \brief Get spectrum data from the DB for a type
      //! \param db An open database connection
      //! \param type The type of data to retrieve.  One of:  "Extinction", 
      //!             "Refraction", or "Fluorescence"
      void getSpectrum( IUS_DB2*, const QString& );

      //! \brief Put spectrum data to the DB for a type
      //! \param db An open database connection
      //! \param type The type of data to put to the DB.  One of:  "Extinction",
      //!             "Refraction", or "Fluorescence"
      void putSpectrum( IUS_DB2*, const QString& ) const;

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
