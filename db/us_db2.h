//! \file us_db2.h
#ifndef US_DB2_H
#define US_DB2_H

#include <QtCore>
#include <mysql/mysql.h>

#include "us_extern.h"

/*! \brief This class provides connectivity and convenience functions
           for database access.
*/

class US_EXTERN US_DB2
{
  public:

    US_DB2();
    ~US_DB2();

    // Methods

    /*! \brief This function provides a quick test of database connectivity
        to ensure that the parameters in the database setup are correct.
        The database is merely opened and then immediately closed.

        \param host     The name of the host of the database server.
        \param dbname   The name of the database to access.
        \param user     The user name that can access the database.
        \param password The unencrypted password for the database/user.
        \param error    A reference to a string for error responses as defined
                        in the class QSqlError.
    */
    bool test_db_connection( const QString&, const QString&, 
        const QString&, const QString&, QString& );

    /*! \brief This function provides a quick test of database connectivity
        over an SSL connection.
        The database is merely opened and then immediately closed.

        \param host     The name of the host of the database server.
        \param dbname   The name of the database to access.
        \param user     The user name that can access the database.
        \param password The unencrypted password for the database/user.
        \param email    The email address of the investigator
        \param inv_pw   The investigator's password
        \param error    A reference to a string for error responses as defined
                        in the class QSqlError.
    */
    bool test_secure_connection( const QString&, const QString&, 
                                 const QString&, const QString&, 
                                 const QString&, const QString&, 
                                 QString& );

    /*! \brief A function to open a database using the currently defined database
               stored by \ref US_Config and \ref US_Settings.

        \param masterPW Master password to decrypt DB password
        \param error    A reference to a string for error responses as defined
                        in the class QSqlError.
    */
    bool          connect     ( const QString&, QString& );
    bool          connect     ( const QString&, const QString&, 
                                const QString&, const QString&, 
                                QString& );
    void          rawQuery    ( const QString& );
    int           statusQuery ( const QString& );
    void          query       ( const QString& );
    bool          next        ( void );
    QVariant      value       ( unsigned );
                  
    bool          isConnected ( void );
    int           numRows     ( void );

    QString       lastError   ( void ) { return error;  };
    int           lastErrno   ( void ) { return errno;  };
    int           lastInsertID( void );
    
  private:
    bool          connected;
    MYSQL*        db;
    MYSQL_RES*    result;
    MYSQL_ROW     row;
      
    QString       certFile;
    QString       error;
    int           errno;
};
#endif
