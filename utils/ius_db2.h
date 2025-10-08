//! \file ius_db2.h
#ifndef IUS_DB2_H
#define IUS_DB2_H

#include <QtCore>

/**
 * @brief Interface for US_DB2 database operations
 * Based on actual usage in tests and US_Project class
 */
class IUS_DB2
{
public:
    // Default constructor
    IUS_DB2() = default;

    // Constructor with password parameter
    IUS_DB2(const QString& pw) { Q_UNUSED(pw); }

    virtual ~IUS_DB2() = default;

    // Database status codes (needed for return values)
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
        INSERTDUP      = 402,     //!< Attempt to insert a duplicate value in a primary or unique key field
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
        CALIB_IN_USE   = 512,     //!< The specified rotor calibration profile is in use
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
        UNKNOWN_ERR    = 999      //!< Unknown error
    };

    // Methods actually used in your tests and US_Project

    // Connection methods (used in tests)
    virtual bool connect(const QString& masterPW, QString& err) = 0;
    virtual bool connect(const QString& host, const QString& dbname,
                         const QString& user, const QString& password,
                         QString& error) = 0;

    // Query methods (used in US_Project and tests)
    virtual void query(const QString& sqlQuery) = 0;
    virtual void query(const QStringList& arguments) = 0;
    virtual int statusQuery(const QString& sqlQuery) = 0;
    virtual int statusQuery(const QStringList& arguments) = 0;

    // Result methods (used in US_Project)
    virtual bool next() = 0;
    virtual QVariant value(unsigned index) = 0;
    virtual bool isConnected() = 0;

    // Error methods (used in tests and error handling)
    virtual QString lastError() = 0;
    virtual int lastErrno() = 0;
    virtual int lastInsertID() = 0;

    // Connection test methods (used in tests)
    virtual bool test_db_connection(const QString& host, const QString& dbname,
                                    const QString& user, const QString& password,
                                    QString& error) = 0;

    virtual bool test_secure_connection(const QString& host, const QString& dbname,
                                        const QString& user, const QString& password,
                                        const QString& email, const QString& pw,
                                        QString& err) = 0;

    // Blob operations (used in tests)
    virtual int writeBlobToDB(const QString& filename, const QString& procedure, const int tableID) = 0;
    virtual int readBlobFromDB(const QString& filename, const QString& procedure, const int tableID) = 0;
    virtual int writeAucToDB(const QString& filename, int tableID) = 0;
    virtual int readAucFromDB(const QString& filename, int tableID) = 0;

    // Utility methods (used in tests)
    virtual void rawQuery(const QString& sqlQuery) = 0;
    virtual int functionQuery(const QStringList& arguments) = 0;
    virtual int numRows() = 0;
    virtual QString lastDebug() = 0;
    virtual unsigned long mysqlEscapeString(QByteArray& to, QByteArray& from, unsigned long length) = 0;
};

#endif // IUS_DB2_H