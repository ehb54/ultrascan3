#ifndef US_NOISE_H
#define US_NOISE_H

#include <QtCore>
#include "us_extern.h"
#include "us_db2.h"

class US_EXTERN US_Noise
{
   public:
      US_Noise();

      enum NoiseType { RI, TI };

      NoiseType  type;

      QString    description;
      QString    guid;
      QString    editguid;   

      double     minradius;
      double     maxradius;

      int        count;

      QVector< double > values;

      QString    message;  // Used internally for communication

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
      //! \param filename The name, including full path, of the analyte file
      //! \returns        The \ref US_DB2 return code for the operation
      int load( const QString& );  
      
      bool operator== ( const US_Noise& ) const;      
      inline bool operator!= ( const US_Noise& n ) const
         { return ! operator==(n); }

      //! Write a noise vector to the disk or database
      //! \param db_access A flag to indicate if the DB (true) or disk (false)
      //!                  should be used to save the noise vector
      //! \param filename  The filename (with path) where the xml file
      //!                  be written if disk access is specified
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

      //! \param path A reference to the full disk path for the directory
      //!             into which to write the noise vector file.
      //! \returns    Success if the path is found or created and failure
      //!             if the path cannot be created
      static bool noise_path( QString& );

   private:

      int  load_disk       ( const QString& );
      int  load_db         ( const QString&, US_DB2* );
                           
      void write_temp      ( QTemporaryFile& );

      void debug( void );
};
#endif
