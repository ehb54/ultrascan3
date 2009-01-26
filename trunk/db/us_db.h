//! \file us_db.h
#ifndef US_DB_H
#define US_DB_H

#include <QtCore>
#include <QtSql>

#include "us_extern.h"

/*! \brief This class provides connectivity and convenience functions
           for database access.
*/

class US_EXTERN US_DB
{
  public:

    //! A null constructor
    US_DB () {};
    //! A null destructor
    ~US_DB() {};

    // Programs

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
    static bool test_db_connection( const QString&, const QString&, 
        const QString&, const QString&, QString& );

    /*! \brief A function to open a database using the currently defined database
               stored by \ref US_Config and \ref US_Settings.

        \param masterPW Master password to decrypt DB password
        \param error    A reference to a string for error responses as defined
                        in the class QSqlError.
    */
    bool open ( const QString&, QString& );

    //! \brief Close the database 
    void close( void );
    //QString query( const QString& );
    // Others
    
  private:
    QSqlDatabase db;
};
#endif
