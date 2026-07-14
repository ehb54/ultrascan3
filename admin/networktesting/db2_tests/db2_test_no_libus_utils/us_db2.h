//! \file us_db2.h
#ifndef US_DB2_H
#define US_DB2_H

#include <QtCore>

// The supercomputer does not use the DB
#include "mysql.h"


// #include "us_extern.h"

#define US_UTIL_EXTERN

/*! \brief This class provides connectivity and convenience functions
           for database access in the US3 system. 
*/

class US_UTIL_EXTERN US_DB2
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
#undef ERROR
    enum dbStatus
    {
      OK             = 0,       //!< Query ok; no error
      DBERROR        = -1,      //!< Unspecified error

      NOT_CONNECTED  = 1,       //!< Database system is not connected
     
      DUP_EMAIL      = 101,     //!< Duplicate email addresses exist
      NO_ACCT        = 102,     //!< No account with that email address was found
      INACTIVE       = 103,     //!< The account has not been activated
      BADPASS        = 104,     //!< Incorrect password
      EMPTY          = 105,     //!< A required parameter was empty
      BAD_CHECKSUM   = 106,     //!< A transmission error; bad MD5 checksum
     
      NOTPERMITTED   = 201,     //!< User does not have permission 
      BADOPERATOR    = 202,     //!< Operator does not have permission to use that instrument
      BADLABLOCATION = 203,     //!< Instrument does not belong in that lab
      BADGUID        = 204,     //!< The specified GUID is not in the correct format
     
      NOROWS         = 301,     //!< No rows returned
     
      INSERTNULL     = 401,     //!< Attempt to insert NULL value in a NOT NULL field
      INSERTDUP      = 402,     //!< Attempt to insert a duplicate value in a primary
                                //!<    or unique key field
      DUPFIELD       = 403,     //!< Attempt to insert duplicate value where one should not exist
      CONSTRAINT_FAILED = 404,  //!< A database constraint failed

      NO_BUFFER      = 501,     //!< No buffer with that ID was found
      NO_COMPONENT   = 502,     //!< No buffer component with that ID was found
      NO_ROTOR       = 503,     //!< No rotor or abstract rotor with that ID exists
      NO_ANALYTE     = 504,     //!< No analyte with the specified ID exists
      NO_LAB         = 505,     //!< No lab with the specified ID exists
      NO_PERSON      = 506,     //!< No person with the specified ID exists
      NO_MODEL       = 507,     //!< No model with the specified ID exists 
      NO_EXPERIMENT  = 508,     //!< No experiment with the specified ID exists
      NO_RAWDATA     = 509,     //!< No raw data with the specified ID exists
      NO_EDITDATA    = 510,     //!< No edit profile with the specified ID exists
      NO_SOLUTION    = 511,     //!< No solution with the specified ID exists
      CALIB_IN_USE   = 512,     //!< The specified rotor calibration profile is in use,
                                //!< and cannot be deleted
      ROTOR_IN_USE   = 513,     //!< The specified rotor is in use, and cannot be deleted
      NO_NOISE       = 514,     //!< No noise file with the specified ID exists
      NO_PROJECT     = 515,     //!< No project with the specified ID exists
      BUFFR_IN_USE   = 516,     //!< The buffer to be deleted is in use
      ANALY_IN_USE   = 517,     //!< The analyte to be deleted is in use
      SOLUT_IN_USE   = 518,     //!< The solution to be deleted is in use
      NO_CALIB       = 519,     //!< The specified calibration profile cannot be found
      NO_REPORT      = 520,     //!< No global report structure with that ID exists
      NO_REPORT_DETAIL = 521,   //!< No report detail with the specified ID exists
      NO_REPORT_DOCUMENT = 522, //!< No report document with the specified ID exists

      INSTRUMENT_IN_USE = 553,
      PROTOCOL_IN_USE = 554,
      NO_AUTOFLOW_RECORD = 555,
      UNKNOWN_ERR    = 999      //!< No project with the specified ID exists
    };

    //! Some user levels used by the US3 database procedures
    enum dbUserlevels
    {
      USER           = 0,       //!< Regular user
      PRIV           = 1,       //!< Privileged user
      ANALYST        = 2,       //!< Data Analyst
      SUPER          = 3,       //!< Super User
      ADMIN          = 4        //!< Admin
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
        \param err      A reference to a string for error responses.
    */
    bool test_secure_connection( const QString&, const QString&, 
                                 const QString&, const QString&, 
                                 const QString&, const QString&, 
                                 QString& );

    /*! \brief Connects to a database using the currently defined database
        stored by US_Config and \ref US_Settings. This constructor 
        uses an SSL connection.

        \param masterPW Master password to decrypt DB password
        \param err      A reference to a string for error responses.
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

    /*! \brief Implements a query on the active database connection, 
        cleaning up any unused results from a previous query. FunctionQuery()
        returns the integer status of the query from the US3 database,
        but no other result data. Useful for COUNT style queries. Uses
        a SELECT command.

        \param arguments A list that contains the function name and any 
                         additional arguments needed.
    */
    int           functionQuery( const QStringList& );

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

    /*! \brief Loads raw binary data from a file and writes it to the
               database. This makes writing a record with binary information
               in it a two-step process---first write the record with the
               rest of the information using statusQuery(), get the table ID
               for that record using lastInsertID(), and finally write the
               corresponding binary information into the same record 
               using writeBlobToDB(). WriteBlobToDB() will return 
               dbStatus.ERROR if the file cannot be opened, or pass through
               any error codes returned by the db.

        \param filename The complete and absolute pathname of the file with
               the binary data. WriteBlobToDB() will try to open the file
               and read the raw binary data.

        \param procedure The name of the MySQL stored procedure that will
               accept the data and write it to the database. The procedure
               must implement a parameter list as follows:<br><br>
               CALL procedure( yourGUID, yourPassword, tableID, blobData, 
               checksum ); where checksum is an MD5 checksum<br><br>

        \param tableID The integer primary-key index of the record that the
               raw binary data should be written to.
    */
    int           writeBlobToDB ( const QString& , const QString& , const int );

    /*! \brief Reads raw binary data from the database and writes it to the
               specified file. This makes reading a record with binary information
               in it a two-step process---first get the rest of the record 
               with query(), and then use the table ID for that record 
               to read the corresponding binary information to a file 
               using readBlobFromDB(). ReadBlobFromDB() will return 
               dbStatus.ERROR if the file cannot be opened, or pass through
               any error codes returned by the db.

        \param filename The complete and absolute pathname of the file to
               write the binary data to. ReadBlobFromDB() will try to open 
               the file and write the raw binary data to it.

        \param procedure The name of the MySQL stored procedure that will
               read the data from the database. The procedure
               must implement a parameter list as follows:<br><br>
               CALL procedure( yourGUID, yourPassword, tableID); and should
               return two fields: the blob data and an MD5 checksum 

        \param tableID The integer primary-key index of the record that 
               contains the raw binary data.
    */
    int           readBlobFromDB( const QString&, const QString&, const int );


    /*! \brief Reads AUC data from the database and writes it to the
               specified file.  If the downloaded data is compressed, it
               is decompressed.

        \param filename The complete and absolute pathname of the destination
               file name.  

        \param tableID The integer primary-key index of the record that 
               contains the raw AUC data.

        \return dbStatus.ERROR if the file cannot be opened, or 
                any error codes returned by the db.
    */
    int           readAucFromDB( const QString&, int );


    /*! \brief Loads AUC data from a file, compresses it, and writes it to the
               database. 
               
        \param filename The complete and absolute pathname of the file with
               the binary data. 

        \param tableID The integer primary-key index of the record where the
               raw AUC data will be pplaced.

        \return dbStatus.ERROR if the file cannot be opened, or 
                any error codes returned by the db.
    */
    int           writeAucToDB( const QString&, int );

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
    int           lastErrno   ( void ) { return db_errno;  };

    /*! \brief Returns the ID of the row resulting from the most recent INSERT or
        UPDATE statement. 
    */
    int           lastInsertID( void );
    
    /*! \brief Returns the most recent debug statement. This is only used in the
               development of the MySQL stored routines. To use it, one would
               issue a statement like this inside a stored routine:

               SET \@DEBUG = 'A Debugging message';

               lastDebug() returns only the string that is currently assigned to
               \@DEBUG
    */
    QString       lastDebug( void );

    /*! \brief Inserts escape codes into a string, preparing the string to be inserted
               into a mysql database

        \param to A reference to a data structure where the escaped string will go

        \param from A reference to the source string

        \param length The length of the source data
    */
    unsigned long mysqlEscapeString( QByteArray& , QByteArray& , unsigned long );

  private:
    bool       connected;

    MYSQL*     db;
    MYSQL_RES* result;
    MYSQL_ROW  row;

    QString    email;
    QString    userPW;
    QString    guid;

    QString    certFile;
    QString    keyFile;
    QString    caFile;
    QString    error;
    int        db_errno;

    QString    buildQuery      ( const QStringList& );
    QString    buildQuerySelect( const QStringList& );
};
#endif

