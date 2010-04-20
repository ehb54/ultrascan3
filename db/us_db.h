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

    US_DB();
    ~US_DB();

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
    static bool test_db_connection( const QString&, const QString&, 
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
    static bool test_secure_connection( const QString&, const QString&, 
                                        const QString&, const QString&, 
                                        const QString&, const QString&, 
                                        QString& );

    /*! \brief A function to open a database using the currently defined database
               stored by US_Config and \ref US_Settings.

        \param masterPW Master password to decrypt DB password
        \param error    A reference to a string for error responses as defined
                        in the class QSqlError.
    */
    bool      open   ( const QString&, QString& );
    
    /*! \brief Make an SQL query to the open database.
        \param q  A string with the query text.
    */
    void      query  ( const QString& );
   
    /*! \brief Return a value from the most recent query.
        \param index The index of the component in a retruned row of data
    */
    QVariant  value  ( int );

    //! \brief Return the result of the most recent  attempt to open the 
    //!        DB connection.
    bool      isOpen ( void );

    /*! \brief Iterate to the next row of a returned dataset. */
    bool      next   ( void );

    /*! \brief Return the number of rows resulting from the most recent SELECT 
               statemetn. */
    int       numRows( void );

    //! \brief Retrun a text description of the most recent DB result.
    QString              lastQueryErrorText( void ){ return error.text(); };

    //! \brief Retrun an integer (enum value) of the most recend DB result.
    QSqlError::ErrorType lastQueryErrorType( void ){ return error.type(); };
    
  private:
    bool          opened;
    QSqlDatabase  db;
    QSqlQuery*    sqlQuery;
    QSqlError     error;
};
#endif
