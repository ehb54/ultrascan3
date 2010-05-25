//! \file us_db2.h
#ifndef US_DB2_H
#define US_DB2_H

#include <QtCore>
#include <mysql/mysql.h>

#include "us_extern.h"

/*! \brief This class provides connectivity and convenience functions
           for database access in the US3 system. 
*/

class US_EXTERN US_DB2
{
  public:

    /*! \brief Generic constructor for the US_DB2 class. Initialization is
        performed but database connection is not attempted.
    */
    US_DB2();

    /*! \brief Constructor for the US_DB2 class. Initialization is performed
        and connection is attempted. Connects to a database using the currently 
        defined database stored by US_Config and \ref US_Settings. This 
        constructor uses an SSL connection.

        \param masterPW Master password to decrypt DB password
    */
    US_DB2( const QString& );

    /*! \brief Generic destructor for the US_DB2 class. Database connection 
        is terminated and any resources in use are closed and deleted.
    */
    ~US_DB2();

    //! Some status codes returned by the US3 database
    enum dbStatus
    {
      OK             = 0,       //!< Query ok; no error
      ERROR          = -1,      //!< Unspecified error

      NOT_CONNECTED  = 1,       //!< Database system is not connected
     
      DUP_EMAIL      = 101,     //!< Duplicate email addresses exist
      NO_ACCT        = 102,     //!< No account with that email address was found
      INACTIVE       = 103,     //!< The account has not been activated
      BADPASS        = 104,     //!< Incorrect password
      EMPTY          = 105,     //!< A required parameter was empty
     
      NOTPERMITTED   = 201,     //!< User does not have permission 
      BADOPERATOR    = 202,     //!< Operator does not have permission to use that instrument
      BADLABLOCATION = 203,     //!< Instrument does not belong in that lab
      BADGUID        = 204,     //!< The specified GUID is not in the correct format
     
      NOROWS         = 301,     //!< No rows returned
     
      INSERTNULL     = 401,     //!< Attempt to insert NULL value in a NOT NULL field
      INSERTDUP      = 402,     //!< Attempt to insert a duplicate value in a primary
                                //!<    or unique key field
      DUPFIELD       = 403,     //!< Attempt to insert duplicate value where one should not exist

      NO_BUFFER      = 501,     //!< No buffer with that ID was found
      NO_COMPONENT   = 502,     //!< No buffer component with that ID was found
      NO_ROTOR       = 503,     //!< No rotor or abstract rotor with that ID exists
      NO_ANALYTE     = 504,     //!< No analyte with the specified ID exists
      NO_LAB         = 505,     //!< No lab with the specified ID exists
      NO_PERSON      = 506,     //!< No person with the specified ID exists
      NO_MODEL       = 507      //!< No model with the specified ID exists 
    };

    //! Some user levels used by the US3 database procedures
    enum dbUserlevels
    {
      USER           = 0,       //!< Regular user
      PRIV           = 1,       //!< Privileged user
      ANALYST        = 2,       //!< Data Analyst
      SUPER          = 3,       //!< Super User
      ADMIN          = 4,       //!< Admin
    };

    // Methods

    /*! \brief Provides a quick test of database connectivity
        to ensure that the parameters in the database setup are correct.
        The database is merely opened and then immediately closed.

        \param host     The name of the host of the database server.
        \param dbname   The name of the database to access.
        \param user     The user name that can access the database.
        \param password The unencrypted password for the database/user.
        \param error    A reference to a string for error responses.
    */
    bool test_db_connection( const QString&, const QString&, 
                             const QString&, const QString&, 
                             QString& );

    /*! \brief Provides a quick test of database connectivity
        over an SSL connection. The database is merely opened and 
        then immediately closed.

        \param host     The name of the host of the database server.
        \param dbname   The name of the database to access.
        \param user     The user name that can access the database.
        \param password The unencrypted password for the database/user.
        \param email    The email address of the investigator
        \param pw       The investigator's password
        \param error    A reference to a string for error responses.
    */
    bool test_secure_connection( const QString&, const QString&, 
                                 const QString&, const QString&, 
                                 const QString&, const QString&, 
                                 QString& );

    /*! \brief Connects to a database using the currently defined database
        stored by US_Config and \ref US_Settings. This constructor 
        uses an SSL connection.

        \param masterPW Master password to decrypt DB password
        \param error    A reference to a string for error responses.
    */
    bool          connect     ( const QString&, QString& );

    /*! \brief Connects to a database using authentication parameters
        supplied in the argument list.

        \param host     The name of the host of the database server.
        \param dbname   The name of the database to access.
        \param user     The user name that can access the database.
        \param password The unencrypted password for the database/user.
        \param error    A reference to a string for error responses.
    */
    bool          connect     ( const QString&, const QString&, 
                                const QString&, const QString&, 
                                QString& );

    /*! \brief Implements a simple query on the active database 
        connection, cleaning up any unused results from a previous query.

        \param sqlQuery    A string containing the US3 query to execute.
    */
    void          rawQuery    ( const QString& );

    /*! \brief Implements a query on the active database connection, 
        cleaning up any unused results from a previous query. StatusQuery()
        returns the integer status of the query from the US3 database,
        but no other result data. Useful for INSERT, UPDATE and DELETE 
        style queries.

        \param sqlQuery    A string containing the US3 query to execute.
    */
    int           statusQuery ( const QString& );

    /*! \brief An overloaded method that builds the statusQuery string from
        the passed arguments and the stored DB settings.

        \param arguments A list that contains the function name and any 
                         additional arguments needed.
    */
    int           statusQuery ( const QStringList& );

    /*! \brief Implements a general-purpose query on the active database 
        connection, cleaning up any unused results from a previous query. 
        Query() can be used for INSERT, UPDATE, or DELETE style queries, 
        but statusQuery() is easier to use in those cases. With SELECT
        queries, Query() retrieves the result set and prepares to return
        individual values using the next() and value() methods. Use the 
        numRows() method to find out how many rows there are.

        \param sqlQuery  A string containing the US3 query to execute.
    */
    void          query       ( const QString& );

    //! \brief A \a method to generate and make a CALL to the database
    //!        and automatically add the user's guid and
    //!        password.\n\n
    //!  Queries are normally in the form of:\n 
    //!  CALL function_name( guid, pw, [arg1,] ... )
    //!  \param arguments A list that contains the function name and any 
    //!                   additional arguments needed.
    void          query       ( const QStringList& );
    
    /*! \brief Fetches the next row in the result set, if one exists. 
        Returns TRUE if the operation has been successful, or FALSE 
        if there are no more rows. Use the value() function to 
        retrieve the individual values in the row.
    */
    bool          next        ( void );

    /*! \brief Returns the value of one column in the current row of
        the result set. This function can be called repeatedly to retrieve
        values for all columns in the row. If there are n columns in
        the row, the columns are numbered 0...n-1. Value() returns 
        the QVariant::invalid type in cases where the data type is unknown,
        or if the index is out of bounds. Use next() to advance the row pointer.

        \param index   The column number in the current row. 
    */
    QVariant      value       ( unsigned );
                  
    /*! \brief Returns TRUE if the US3 system is connected to a database,
        or returns FALSE otherwise.
    */
    bool          isConnected ( void );

    /*! \brief Returns the number of rows returned in the current result
        set. 
    */
    int           numRows     ( void );

    /*! \brief Returns a text string containing the most recent error encountered
        by the US3 database system. If a query did not result in an error,
        lastError() might return a text string describing the previous error,
        or it might return an empty string indicating that there was no error. If
        you wish to determine if there has been an error, call lastErrno() first
        and then call lastError() to find out what the error was.
    */
    QString       lastError   ( void ) { return error;  };

    /*! \brief Returns the integer status code of the most recent query performed
        by the US3 database system. If a query did not result in an error, then 
        lastErrno() will return 0. If the status code is not 0, then you can use the
        status code itself to determine what went wrong, or you can call lastError()
        for a text string describing the error. 
    */
    int           lastErrno   ( void ) { return errno;  };

    /*! \brief Returns the ID of the row resulting from the most recent INSERT or
        UPDATE statement. 
    */
    int           lastInsertID( void );
    
  private:
    bool       connected;
    MYSQL*     db;
    MYSQL_RES* result;
    MYSQL_ROW  row;
      
    QString    email;
    QString    userPW;
    QString    guid;

    QString    certFile;
    QString    error;
    int        errno;

    QString    buildQuery( const QStringList& );
};
#endif
