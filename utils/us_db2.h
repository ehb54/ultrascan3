//! \file us_db2.h
#ifndef US_DB2_H
#define US_DB2_H

#include <QtCore>
#include "ius_db2.h"  // Include the interface

// The supercomputer does not use the DB
#ifndef NO_DB
#include "mysql.h"
#endif
#include "us_extern.h"

/*! \brief This class provides connectivity and convenience functions
           for database access in the US3 system.
*/

class US_UTIL_EXTERN US_DB2 : public IUS_DB2
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
    ~US_DB2() override;

    // Methods from IUS_DB2 interface

    /*! \brief Provides a quick test of database connectivity
        to ensure that the parameters in the database setup are correct.
        The database is merely opened and then immediately closed.

        \param host     The name of the host of the database server.
        \param dbname   The name of the database to access.
        \param user     The user name that can access the database.
        \param password The unencrypted password for the database/user.
        \param error    A reference to a string for error responses.
    */
    bool test_db_connection( const QString& host, const QString& dbname,
                             const QString& user, const QString& password,
                             QString& error ) override;

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
    bool test_secure_connection( const QString& host, const QString& dbname,
                                 const QString& user, const QString& password,
                                 const QString& email, const QString& pw,
                                 QString& err ) override;

    /*! \brief Connects to a database using the currently defined database
        stored by US_Config and \ref US_Settings. This constructor
        uses an SSL connection.

        \param masterPW Master password to decrypt DB password
        \param err      A reference to a string for error responses.
    */
    bool connect( const QString& masterPW, QString& err ) override;

    /*! \brief Connects to a database using authentication parameters
        supplied in the argument list.

        \param host     The name of the host of the database server.
        \param dbname   The name of the database to access.
        \param user     The user name that can access the database.
        \param password The unencrypted password for the database/user.
        \param err    A reference to a string for error responses.
    */
    bool connect( const QString& host, const QString& dbname,
                  const QString& user, const QString& password,
                  QString& err ) override;

    /*! \brief Implements a simple query on the active database
        connection, cleaning up any unused results from a previous query.

        \param sqlQuery    A string containing the US3 query to execute.
    */
    void rawQuery( const QString& sqlQuery ) override;

    /*! \brief Implements a query on the active database connection,
        cleaning up any unused results from a previous query. StatusQuery()
        returns the integer status of the query from the US3 database,
        but no other result data. Useful for INSERT, UPDATE and DELETE
        style queries.

        \param sqlQuery    A string containing the US3 query to execute.
    */
    int statusQuery( const QString& sqlQuery ) override;

    /*! \brief An overloaded method that builds the statusQuery string from
        the passed arguments and the stored DB settings.

        \param arguments A list that contains the function name and any
                         additional arguments needed.
    */
    int statusQuery( const QStringList& arguments ) override;

    /*! \brief Implements a query on the active database connection,
        cleaning up any unused results from a previous query. FunctionQuery()
        returns the integer status of the query from the US3 database,
        but no other result data. Useful for COUNT style queries. Uses
        a SELECT command.

        \param arguments A list that contains the function name and any
                         additional arguments needed.
    */
    int functionQuery( const QStringList& arguments ) override;

    /*! \brief Implements a general-purpose query on the active database
        connection, cleaning up any unused results from a previous query.
        Query() can be used for INSERT, UPDATE, or DELETE style queries,
        but statusQuery() is easier to use in those cases. With SELECT
        queries, Query() retrieves the result set and prepares to return
        individual values using the next() and value() methods. Use the
        numRows() method to find out how many rows there are.

        \param sqlQuery  A string containing the US3 query to execute.
    */
    void query( const QString& sqlQuery ) override;

    //! \brief A \a method to generate and make a CALL to the database
    //!        and automatically add the user's guid and
    //!        password.\n\n
    //!  Queries are normally in the form of:\n
    //!  CALL function_name( guid, pw, [arg1,] ... )
    //!  \param arguments A list that contains the function name and any
    //!                   additional arguments needed.
    void query( const QStringList& arguments ) override;

    /*! \brief Fetches the next row in the result set, if one exists.
        Returns TRUE if the operation has been successful, or FALSE
        if there are no more rows. Use the value() function to
        retrieve the individual values in the row.
    */
    bool next() override;

    /*! \brief Returns the value of one column in the current row of
        the result set. This function can be called repeatedly to retrieve
        values for all columns in the row. If there are n columns in
        the row, the columns are numbered 0...n-1. Value() returns
        the QVariant() type in cases where the data type is unknown,
        or if the index is out of bounds. Use next() to advance the row pointer.

        \param index   The column number in the current row.
    */
    QVariant value( unsigned index ) override;

    /*! \brief Returns TRUE if the US3 system is connected to a database,
        or returns FALSE otherwise.
    */
    bool isConnected() override;

    /*! \brief Returns the number of rows returned in the current result
        set.
    */
    int numRows() override;

    /*! \brief Returns a text string containing the most recent error encountered
        by the US3 database system. If a query did not result in an error,
        lastError() might return a text string describing the previous error,
        or it might return an empty string indicating that there was no error. If
        you wish to determine if there has been an error, call lastErrno() first
        and then call lastError() to find out what the error was.
    */
    QString lastError() override;

    /*! \brief Returns the integer status code of the most recent query performed
        by the US3 database system. If a query did not result in an error, then
        lastErrno() will return 0. If the status code is not 0, then you can use the
        status code itself to determine what went wrong, or you can call lastError()
        for a text string describing the error.
    */
    int lastErrno() override;

    /*! \brief Returns the ID of the row resulting from the most recent INSERT or
        UPDATE statement.
    */
    int lastInsertID() override;

    /*! \brief Returns the most recent debug statement. This is only used in the
               development of the MySQL stored routines. To use it, one would
               issue a statement like this inside a stored routine:

               SET \@DEBUG = 'A Debugging message';

               lastDebug() returns only the string that is currently assigned to
               \@DEBUG
    */
    QString lastDebug() override;

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
    int writeBlobToDB( const QString& filename, const QString& procedure, const int tableID ) override;

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
    int readBlobFromDB( const QString& filename, const QString& procedure, const int tableID ) override;

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
    int readAucFromDB( const QString& filename, int tableID ) override;

    /*! \brief Loads AUC data from a file, compresses it, and writes it to the
               database.

        \param filename The complete and absolute pathname of the file with
               the binary data.

        \param tableID The integer primary-key index of the record where the
               raw AUC data will be pplaced.

        \return dbStatus.ERROR if the file cannot be opened, or
                any error codes returned by the db.
    */
    int writeAucToDB( const QString& filename, int tableID ) override;

    /*! \brief Inserts escape codes into a string, preparing the string to be inserted
               into a mysql database

        \param to A reference to a data structure where the escaped string will go

        \param from A reference to the source string

        \param length The length of the source data
    */
    unsigned long mysqlEscapeString( QByteArray& to, QByteArray& from, unsigned long length ) override;

private:
    bool       connected;
#ifndef NO_DB
    MYSQL*     db;
    MYSQL_RES* result;
    MYSQL_ROW  row;
#endif
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
    bool       configure_ssl   ( QString& err );
};
#endif