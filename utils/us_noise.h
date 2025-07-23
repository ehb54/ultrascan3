//! \file us_noise.h
#ifndef US_NOISE_H
#define US_NOISE_H

#include <QtCore>
#include "us_extern.h"
#include "us_db2.h"
#include "us_dataIO.h"

//! \brief Noise Vector object

/*! \class US_Noise
  
    This class sets up a noise vector of time-invariant or radially-invariant
    values. It provides an interface to read and write values in the database
    or in local disk files.
*/
class US_UTIL_EXTERN US_Noise
{
   public:
      //! \brief Create a noise vector object.
      US_Noise();

      //! Types of noise: radially-invariant or time-invariant
      enum NoiseType { RI, TI };

      NoiseType  type;           //!< Type of noise: RI or TI

      QString    description;    //!< String describing the noise set
      QString    noiseGUID;      //!< Global ID of this noise
      QString    modelGUID;      //!< Global ID of parent model object

      double     minradius;      //!< Minimum radius value for TI type
      double     maxradius;      //!< Maximum radius value for TI type

      int        count;          //!< Number of noise values in the vector

      QVector< double > values;  //!< Vector of noise values

      QString    message;        //!< Used internally for communication

      //! Read a noise vector from the disk or database
      //! \param db_access A flag to indicate if the DB (true) or disk (false)
      //!                  should be searched for the noise vector
      //! \param guid      The guid of the noise vector to be loaded
      //! \param db        For DB access, pointer to an open database connection
      //! \returns         The \ref US_DB2 return code for the operation
      int load( bool, const QString&, US_DB2* = 0 );

      //! An overloaded function to read a noise vector from a database
      //! \param id       The DB ID of the desired Noise record
      //! \param db       A pointer to an open database connection
      //! \returns        The \ref US_DB2 return code for the operation
      int load( const QString&, US_DB2* ); 


      //! An overloaded function to read a noise vector from the disk
      //! \param filename The name, including full path, of the noise XML file
      //! \returns        The \ref US_DB2 return code for the operation
      int load( const QString& );  
      
      //! Compare two noise objects for equality
      bool operator== ( const US_Noise& ) const;      
      //! Compare two noise objects for non-equality
      inline bool operator!= ( const US_Noise& n ) const
         { return ! operator==(n); }

      //! Write a noise vector to the disk or database
      //! \param db_access A flag to indicate if the DB (true) or disk (false)
      //!                  should be used to save the noise vector
      //! \param filename  The filename (with path) where the xml file is
      //!                  to be written if disk access is specified
      //! \param db        For DB access, pointer to an open database connection
      //! \returns         The \ref US_DB2 return code for the operation
      int write( bool, const QString&, US_DB2* = 0 );

      //! An overloaded function to write a noise vector to the DB
      //! \param db        A pointer to an open database connection 
      //! \returns         The \ref US_DB2 return code for the operation
      int write( US_DB2* );

      //! An overloaded function to write a noise vector to a file on disk
      //! \param filename  The filename to write
      //! \returns         The \ref US_DB2 return code for the operation
      int write( const QString& );

      //! Apply a noise vector to an EditedData set.
      //! \param editdata  Reference to EditedData set to apply to.
      //! \param remove    Flag to remove (true) or add back (false) noise.
      //! \returns         Noise apply status (< 0 if error):
      //!                    0  iff  noise subtracted/added.
      //!                    1  iff  null noise or noise count==0.
      //!                   -1  iff  noise count does not match readings count
      //!                   -2  iff  noise count does not match scan count
      int apply_to_data( US_DataIO::EditedData&, bool = true );

      //! Static function to apply a noise vector to an EditedData set.
      //! \param editdata  Reference to EditedData set to apply to.
      //! \param noise     Pointer to noise to apply to data (ti or ri).
      //! \param remove    Flag to remove (true) or add back (false) noise.
      //! \returns         Noise apply status (< 0 if error):
      //!                    0  iff  noise subtracted/added.
      //!                    1  iff  null noise or noise count==0.
      //!                   -1  iff  noise count does not match readings count
      //!                   -2  iff  noise count does not match scan count
      static int apply_noise( US_DataIO::EditedData&, US_Noise* = 0,
            bool = true );

      //! Static function to find and, if need be, create noise directory path.
      //! \param path A reference to the full disk path for the directory
      //!             into which to write the noise vector file.
      //! \returns    Success if the path is found or created and failure
      //!             if the path cannot be created
      static bool noise_path( QString& );

      //! Sum a second noise vector into a noise vector
      //! \param noise2     A second noise object to sum into current noise
      //! \param always_sum Flag if summing should proceed even with mismatch
      //! \returns          Flag if summing was performed.
      bool sum_noise( US_Noise, bool = false );

      //! Static function to sum two noise vectors
      //! \param noise1     A first noise object into which to sum a second
      //! \param noise2     A second noise object to sum into the first
      //! \param always_sum Flag if summing should proceed even with mismatch
      //! \returns          Flag if summing was performed.
      static bool sum_noises( US_Noise&, US_Noise, bool = false );
   private:

      int  load_disk       ( const QString& );
      int  load_db         ( const QString&, US_DB2* );
                           
      int  load_stream     ( QXmlStreamReader& );
      void write_stream    ( QXmlStreamWriter& );

      void debug( void );
};
#endif
