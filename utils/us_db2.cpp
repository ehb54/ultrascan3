//! \file us_db2.cpp
#include "us_db2.h"
#ifndef NO_DB
#include "errmsg.h"
#include "mysqld_error.h"
#endif
#include "us_settings.h"
#include "us_crypto.h"
#include "us_gzip.h"
#include "us_util.h"

#define CIPHER \
/* TLS 1.2 (Prioritize Galois/Counter Mode) */ \
"ECDHE-ECDSA-AES256-GCM-SHA384:" \
"ECDHE-RSA-AES256-GCM-SHA384:" \
"ECDHE-ECDSA-AES128-GCM-SHA256:" \
"ECDHE-RSA-AES128-GCM-SHA256:" \
/* TLS 1.2 (CCM) */ \
"AES256-CCM:" \
"AES128-CCM:" \
/* TLS 1.2 (CBC + Forward Secrecy) */ \
"ECDHE-ECDSA-AES256-SHA384:" \
"ECDHE-RSA-AES256-SHA384:" \
"ECDHE-ECDSA-AES128-SHA256:" \
"ECDHE-RSA-AES128-SHA256:" \
/* LEGACY FALLBACKS (For very old servers/SHA1) */ \
"ECDHE-ECDSA-AES256-SHA:" \
"ECDHE-RSA-AES256-SHA:" \
"ECDHE-ECDSA-AES128-SHA:" \
"ECDHE-RSA-AES128-SHA:" \
"AES256-GCM-SHA384:" \
"AES128-GCM-SHA256:" \
"AES256-SHA256:" \
"AES128-SHA256:" \
"AES256-SHA:" \
"AES128-SHA"

US_DB2::US_DB2()
{
#ifndef NO_DB
   QString certPath = US_Settings::appBaseDir() + QString( "/etc/mysql/" );
   keyFile    = certPath + QString( "server-key.pem" );
   certFile   = certPath + QString( "server-cert.pem" );
   caFile     = certPath + QString( "ca-cert.pem" );

   connected  = false;
   result     = NULL;
   db         = mysql_init( NULL );
#endif
}

#ifdef NO_DB
US_DB2::US_DB2( const QString& ){}
#else
US_DB2::US_DB2( const QString& masterPW )
{
   QString certPath = US_Settings::appBaseDir() + QString( "/etc/mysql/" );
   keyFile    = certPath + QString( "server-key.pem" );
   certFile   = certPath + QString( "server-cert.pem" );
   caFile     = certPath + QString( "ca-cert.pem" );

   connected  = false;
   result     = NULL;
   db         = mysql_init( NULL );

   QString err;
   if ( ! connect( masterPW, err ) )
   {
      db_errno = NOT_CONNECTED;
      error = "US_DB2 error: could not connect\n" + err;
      return;
   }

   db_errno  = OK;
   error     = "";
   connected = true;
}
#endif

US_DB2::~US_DB2()
{
#ifndef NO_DB
   // Closing would end the transaction anyway; roll it back explicitly so a
   // dangling one is undone by us, not by however the server treats the close.
   if ( inTransaction && connected )
   {
      drainResults();
      mysql_rollback( db );
   }

  mysql_free_result( result );
  mysql_close(db);

  result      = NULL;
  connected   = false;
#endif
}

QString US_DB2::lastError() {
    return error;
}

int US_DB2::lastErrno() {
    return db_errno;
}

#ifdef NO_DB
bool US_DB2::test_db_connection( const QString&, const QString&,
      const QString&, const QString&, QString& ) { return false; }
#else
bool US_DB2::test_db_connection( 
        const QString& host, const QString& dbname, 
        const QString& user, const QString& password, 
        QString& error )
{
   error = "";

   MYSQL* conn = mysql_init( NULL );
   if ( ! conn )
   {
      error = QString( "Test connection allocation error" );
      return false;
   }

   QString uhost  = host.section( ":", 0, 0 ).simplified();
   int     uport  = host.section( ":", 1, 1 ).simplified().toInt();

   bool status = mysql_real_connect(
                 conn,
                 uhost   .toLatin1().constData(),
                 user    .toLatin1().constData(),
                 password.toLatin1().constData(),
                 dbname  .toLatin1().constData(),
                 uport, NULL, CLIENT_MULTI_STATEMENTS );
 
   if ( ! status )
      error = QString( "Test connection open error\n" ) + mysql_error( conn );

   mysql_close( conn );

   return status;
}
#endif

#ifdef NO_DB
bool US_DB2::test_secure_connection( 
      const QString&, const QString&,
      const QString&, const QString&, 
      const QString&,  const QString&, 
      QString& ) { return false; }
#else
bool US_DB2::test_secure_connection( 
        const QString& host,  const QString& dbname, 
        const QString& user,  const QString& password, 
        const QString& email, const QString& pw, 
        QString&       err )
{
   error = "";

   if ( ! db )
   {
      error = QString( "Test secure connection allocation error" );
      err   = error;
      return false;
   }

   const bool config_status = configure_ssl( err );
   if ( ! config_status ) {
      err = error;
      return false;
   }

   QString uhost  = host.section( ":", 0, 0 ).simplified();
   int     uport  = host.section( ":", 1, 1 ).simplified().toInt();

   bool status = mysql_real_connect(
                 db,
                 uhost   .toLatin1().constData(),
                 user    .toLatin1().constData(),
                 password.toLatin1().constData(),
                 dbname  .toLatin1().constData(),
                 uport, NULL, CLIENT_MULTI_STATEMENTS );
 
   if ( ! status )
   {
      error = QString( "Test secure connection open error\n" ) +
              mysql_error( db );
      err   = mysql_error( db );
      return false;
   }

   // Let's see if the user can log in
   status    = false;
   QString q = "CALL validate_user( '', '" + email + "', '" + pw + "' )";
   
   this->query( q );

   if ( db_errno != OK )
   {
      error = mysql_error( db );
      err   = error;
      return false;
   }
   
   next();  // Setup for calling routine to just use value()

   return true;
}
#endif

#ifdef NO_DB
bool US_DB2::connect( const QString&, QString& ){ return false; }
#else
bool US_DB2::connect( const QString& masterPW, QString& err )
{
   if ( connected ) return true;

   QStringList defaultDB = US_Settings::defaultDB();
   if ( defaultDB.size() < 6 )
   {
       db_errno = NOT_CONNECTED;
       error = "US_DB2 error: DB not configured";
       err = error;
       return false;
   }

   QString user     = defaultDB.at( 1 );
   QString dbname   = defaultDB.at( 2 );
   QString host     = defaultDB.at( 3 );
   QString cipher   = defaultDB.at( 4 );
   QString iv       = defaultDB.at( 5 );  // Initialization vector

   QString password = US_Crypto::decrypt( cipher, masterPW, iv );
   const bool config_status = configure_ssl( err );
   if ( ! config_status ) {
      db_errno = NOT_CONNECTED;
      error = QString( "Connect open error:\n" ) + err;
      err = error;
      return false;
   }
   try
   {
      // The CLIENT_MULTI_STATEMENTS flag allows for multiple queries and
      //   multiple result sets from a single stored procedure. It is required
      //   for any stored procedure that returns result sets.
      QString uhost  = host.section( ":", 0, 0 ).simplified();
      int     uport  = host.section( ":", 1, 1 ).simplified().toInt();
//qDebug() << "DB: connect uport" << uport << "uhost" << uhost;

      connected = mysql_real_connect(
                  db,
                  uhost   .toLatin1().constData(),
                  user    .toLatin1().constData(),
                  password.toLatin1().constData(),
                  dbname  .toLatin1().constData(),
                  uport, NULL, CLIENT_MULTI_STATEMENTS );
 
   }

   catch ( std::exception &e )
   {
      db_errno = NOT_CONNECTED;
      error = e.what();
      err = "US_DB2: uncaught exception " + error;
      return false;
   }

   db_errno = OK;
   error = "";
   inTransaction        = false;
   rolledBackByDeadlock = false;

   if ( ! connected )
   {
      db_errno = NOT_CONNECTED;
      error = QString( "Connect open error: " ) + mysql_error( db );
      err = error;
      return false;
   }

   email  = defaultDB.at( 6 );  // Save for later

   // DB Internal PW
   cipher = defaultDB.at( 7 );
   iv     = defaultDB.at( 8 );
   userPW = US_Crypto::decrypt( cipher, masterPW, iv );
   guid   = defaultDB.at( 9 );

   QString q = "CALL validate_user( '" + guid + "', '', '" + userPW + "' )";
   
   this->query( q );

   if ( db_errno != OK )
   {
      err = error;
      return false;
   }
   
   next();  // Setup for calling routine to just use value()

   // See if email was changed in the database
   if ( email != this->value( 1 ).toString() )
   {
      QList< QStringList > dbinfo = US_Settings::databases();

      for ( int i = 0; i < dbinfo.size(); i++ )
      {
         QStringList info = dbinfo.at( i );
         
         if ( info.at( 9 ) == guid )  // Found it
         {
            email = this->value( 1 ).toString();
            info.replace( 6, email );
            US_Settings::set_defaultDB( info );  // Update the current DB 
            dbinfo.replace( i, info );
            break;
         }
      }

      US_Settings::set_databases( dbinfo );  // Update the full DB list
   }

   return connected;
}
#endif

#ifdef NO_DB
bool US_DB2::connect( const QString&, const QString&, const QString&,
                      const QString&, QString& ){ return false; }
#else
bool US_DB2::connect( 
        const QString& host,  const QString& dbname, 
        const QString& user,  const QString& password, 
        QString& err )
{
  if ( connected ) return true;

  const bool config_status = configure_ssl( err );
  if ( ! config_status ) {
     db_errno = NOT_CONNECTED;
     error = QString( "Connect open error:\n" ) + err;
     err = error;
     return false;
  }
   try
   {
      // The CLIENT_MULTI_STATEMENTS flag allows for multiple queries and
      //   multiple result sets from a single stored procedure. It is required
      //   for any stored procedure that returns result sets.
      QString uhost  = host.section( ":", 0, 0 ).simplified();
      int     uport  = host.section( ":", 1, 1 ).simplified().toInt();

      connected = mysql_real_connect(
                  db,
                  uhost   .toLatin1().constData(),
                  user    .toLatin1().constData(),
                  password.toLatin1().constData(),
                  dbname  .toLatin1().constData(),
                  uport, NULL, CLIENT_MULTI_STATEMENTS );
   }

   catch ( std::exception &e )
   {
      db_errno = NOT_CONNECTED;
      error = e.what();
      err = "US_DB2: uncaught exception " + error;
      return false;
   }

   db_errno = OK;
   error = "";
   inTransaction        = false;
   rolledBackByDeadlock = false;
   if ( ! connected )
   {
      db_errno = NOT_CONNECTED;
      error = QString( "Connect open error: " ) + mysql_error( db );
      err = error;
      return false;
   }

  return connected;
}
#endif

#ifdef NO_DB
bool US_DB2::beginTransaction( QString& transaction_error )
{
   transaction_error = QObject::tr( notConnectedMessage ).arg( "beginTransaction" );
   return false;
}
bool US_DB2::commitTransaction( QString& transaction_error )
{
   transaction_error = QObject::tr( notConnectedMessage ).arg( "commitTransaction" );
   return false;
}
bool US_DB2::rollbackTransaction( QString& transaction_error )
{
   transaction_error = QObject::tr( notConnectedMessage ).arg( "rollbackTransaction" );
   return false;
}
#else
bool US_DB2::beginTransaction( QString& transaction_error )
{
   if ( !connected )
   {
      transaction_error = QObject::tr( notConnectedMessage ).arg( "beginTransaction" );
      return false;
   }

   if ( inTransaction )
   {
      transaction_error = QObject::tr( transactionOpenMessage ).arg( "beginTransaction" );
      return false;
   }

   // Whatever ran before this call may have left results pending.
   drainResults();

   // START TRANSACTION rather than turning autocommit off: the server restores
   // autocommit itself at the COMMIT or ROLLBACK, so there is no second step
   // that can fail after the transaction has already ended.
   if ( mysql_query( db, "START TRANSACTION" ) != 0 )
   {
      transaction_error = mysql_error( db );
      return false;
   }

   inTransaction        = true;
   rolledBackByDeadlock = false;
   transaction_error.clear();
   return true;
}

bool US_DB2::commitTransaction( QString& transaction_error )
{
   if ( !connected )
   {
      transaction_error = QObject::tr( notConnectedMessage ).arg( "commitTransaction" );
      return false;
   }

   if ( !inTransaction )
   {
      transaction_error = QObject::tr( noTransactionMessage ).arg( "commitTransaction" );
      return false;
   }

   // The last query of the transaction left its results on the connection, and
   // a CALL left a status set behind them.  mysql_commit() talks to the server
   // directly rather than through rawQuery(), so without this it is answered
   // with "Commands out of sync" and the whole transaction is lost at the last
   // step -- with the rollback that follows failing for the same reason.
   drainResults();

   // A COMMIT after the transaction has ended succeeds as a no-op, so check
   // first or lost writes would be reported as committed.  The client does
   // not always see what ended it: a procedure can catch a deadlock itself,
   // or roll back on its own.
   if ( transactionEndedEarly() )
   {
      transaction_error = transactionEndedText();
      return false;
   }

   if ( mysql_commit( db ) != 0 )
   {
      // The server may have committed before the connection dropped; only a
      // lost connection leaves that open, so only then is the outcome unknown.
      const unsigned int code = mysql_errno( db );
      if ( code == CR_SERVER_LOST || code == CR_SERVER_GONE_ERROR )
      {
         transaction_error = QObject::tr( commitUnknownMessage ).arg( mysql_error( db ) );
      }
      else
      {
         transaction_error = mysql_error( db );
      }

      // Still open as far as the caller is concerned: it rolls back next.
      return false;
   }

   inTransaction = false;
   transaction_error.clear();
   return true;
}

bool US_DB2::rollbackTransaction( QString& transaction_error )
{
   if ( !connected )
   {
      inTransaction     = false;
      transaction_error = QObject::tr( notConnectedMessage ).arg( "rollbackTransaction" );
      return false;
   }

   if ( !inTransaction )
   {
      transaction_error = QObject::tr( noTransactionMessage ).arg( "rollbackTransaction" );
      return false;
   }

   // Same reason as commitTransaction(), and it matters more here: a rollback
   // that cannot run is a partial load left in place.
   drainResults();

   inTransaction = false;

   // If the server no longer has the transaction open, something ended it
   // before this call and the rollback below undoes nothing, so a quiet
   // success here would be a lie.
   const bool ended_early = transactionEndedEarly();

   if ( mysql_rollback( db ) != 0 )
   {
      transaction_error = mysql_error( db );
      return false;
   }

   if ( ended_early )
   {
      transaction_error = transactionEndedText();
      return false;
   }

   transaction_error.clear();
   return true;
}
#endif

#ifndef NO_DB
bool US_DB2::transactionEndedEarly()
{
   // The status comes from the last statement that succeeded; an error packet
   // does not carry one, so it may predate a rollback by the server.  One
   // empty statement brings it up to date.  A connection that cannot run even
   // that has lost the transaction with it.
   if ( mysql_query( db, "DO 0" ) != 0 )
   {
      return true;
   }

   // The C API has no accessor for the status flags; MYSQL documents the field.
   return ( db->server_status & SERVER_STATUS_IN_TRANS ) == 0;
}

void US_DB2::noteStatementError()
{
   // InnoDB always answers a deadlock by rolling back the whole transaction,
   // so that error alone says what ended it.  A lock wait timeout does so only
   // when innodb_rollback_on_timeout is set, and is not recorded.  Remember
   // the error while it is still on the connection; the next statement
   // replaces it.
   if ( inTransaction && mysql_errno( db ) == ER_LOCK_DEADLOCK )
   {
      rolledBackByDeadlock = true;
   }
}

QString US_DB2::transactionEndedText() const
{
   return QObject::tr( rolledBackByDeadlock ? serverRollbackMessage : transactionEndedMessage );
}
#endif

#ifdef NO_DB
void US_DB2::drainResults()
{
}
#else
void US_DB2::drainResults()
{
   if ( result )
      mysql_free_result( result );

   while ( mysql_next_result( db ) == 0 )
   {
      result = mysql_store_result( db );
      mysql_free_result( result );
   }

   // A procedure that failed part way reports it on the result set after the
   // last good one, which ends the loop above.
   noteStatementError();

   result = NULL;
}
#endif

#ifdef NO_DB
void US_DB2::rawQuery( const QString& )
{
}
#else
void US_DB2::rawQuery( const QString& sqlQuery )
{
   // Make sure that we clear out any unused
   //   result sets
   drainResults();

   if ( mysql_query( db, sqlQuery.toLatin1() ) != 0 )
   {
      error = QString( "MySQL error: " ) + mysql_error( db );
      noteStatementError();
   }

   else
   {
      result = mysql_store_result( db );

      // A statement without a result set gives NULL too; only an error code
      // says that reading the result failed.
      if ( result == NULL && mysql_errno( db ) != 0 )
      {
         error = QString( "MySQL error: " ) + mysql_error( db );
         noteStatementError();
      }
   }
}
#endif

#ifdef NO_DB
int US_DB2::statusQuery( const QString& ){ return 0; }
#else
int US_DB2::statusQuery( const QString& sqlQuery )
{
   db_errno = DBERROR;

   this->rawQuery( sqlQuery );
   if ( result )
   {
      row       = mysql_fetch_row( result );
      db_errno     = atoi( row[ 0 ] );
      mysql_free_result( result );
      result = NULL;
   }

   if ( db_errno != 0 )
   {
      this->rawQuery( "SELECT last_error()" );
      if ( result )
      {
         row       = mysql_fetch_row( result );
         error     = row[ 0 ];
         mysql_free_result( result );
         result = NULL;
      }
   }

   return db_errno;
}
#endif

int US_DB2::statusQuery( const QStringList& arguments )
{
   return statusQuery( buildQuery( arguments ) );
}

int US_DB2::functionQuery( const QStringList& arguments )
{
   return statusQuery( buildQuerySelect( arguments ) );
}

#ifdef NO_DB
void US_DB2::query( const QString& ) {}
#else
void US_DB2::query( const QString& sqlQuery )
{
   this->rawQuery( sqlQuery );

   // A statement the server refused outright leaves no result set, and
   // rawQuery() reports that only through `error`.  Without this, db_errno
   // still holds whatever the previous query returned, so lastErrno() answers
   // OK for a query that never ran -- a lost connection, a deadlock, a
   // lock-wait timeout.  statusQuery() has always defaulted to DBERROR for the
   // same reason; this brings query() in line with it.
   if ( mysql_errno( db ) != 0 )
   {
      db_errno = DBERROR;
      return;
   }

   if ( result )
   {
      // This is a 2-set result: status, then data
      row    = mysql_fetch_row( result );
      db_errno  = atoi( row[ 0 ] );        // status
      mysql_free_result( result );
      result = NULL;

      const int next = mysql_next_result( db );

      // The procedure failed after its status row, while producing its data.
      if ( next > 0 )
      {
         error    = QString( "MySQL error: " ) + mysql_error( db );
         db_errno = DBERROR;
         noteStatementError();
         return;
      }

      else if ( next == 0 ) // get the result data
      {
         result = mysql_store_result( db );
         if ( ! result )
         {
            if ( mysql_field_count( db ) == 0 )
            {} // We are here as the result of an INSERT, UPDATE or DELETE

            else
            {} // Error retrieving result set
         }
      }
   }

   if ( db_errno != 0 )
   {
      this->rawQuery( "SELECT last_error()" );

      if ( result )
      {
         row       = mysql_fetch_row( result );
         error     = row[ 0 ];
         mysql_free_result( result );
         result = NULL;
      }
   }

   else
      error     = "";
}
#endif

void US_DB2::query( const QStringList& arguments )
{
   query( buildQuery( arguments ) );
}

QString US_DB2::buildQuery( const QStringList& arguments )
{
   QString newquery = "CALL " + arguments[ 0 ]
                    + "('" + guid + "', '" + userPW + "'";

   for ( int i = 1; i < arguments.size(); i++ )
   {
      QString arg = arguments[ i ];
      arg.replace( "'", "\\'" );

      newquery += ", '" + arg + "'";
   }

   newquery += ")";

//qDebug() << "NewQuery:" << newquery;
   return newquery;
}

QString US_DB2::buildQuerySelect( const QStringList& arguments )
{
   QString newquery = "SELECT " + arguments[ 0 ]
                    + "('" + guid + "', '" + userPW + "'";

   for ( int i = 1; i < arguments.size(); i++ )
   {
      QString arg = arguments[ i ];
      arg.replace( "'", "\\'" );

      newquery += ", '" + arg + "'";
   }

   newquery += ")";

//qDebug() << "NewQuerySelect:" << newquery;
   return newquery;
}

#ifdef NO_DB
bool US_DB2::next( void ){ return false; }
#else
bool US_DB2::next( void )
{ 
   row = NULL;
   if ( result )
   {
      if ( ( row = mysql_fetch_row( result ) ) != NULL )
         return true;
   }

   return false;
}
#endif

#ifdef NO_DB
QVariant US_DB2::value( unsigned ){ return QVariant(); }
#else
QVariant US_DB2::value( unsigned index )
{
   if ( row && ( index < mysql_field_count( db ) ) )
      return QVariant( QString( row[ index ] ) );

   return QVariant();
}
#endif

bool US_DB2::isConnected ( void )
{
   return connected;
}

#ifdef NO_DB
int US_DB2::numRows( void ){ return 0; }
#else
int US_DB2::numRows( void )
{ 
   return ( result )? ( (int) mysql_num_rows( result ) ) : -1;
}
#endif

#ifdef NO_DB
int US_DB2::writeBlobToDB( const QString& , const QString& , const int ) { return 0; }
#else
int US_DB2::writeBlobToDB( const QString& filename, 
    const QString& procedure, const int tableID )
{
   // First let's read the file
   QFile fin( filename );

   if ( ! fin.open( QIODevice::ReadOnly ) )
   {
      error = QString( "writeBlob: cannot open file " ) + filename;
      db_errno = DBERROR;
      return DBERROR;
   }

   QByteArray blobData = fin.readAll();
   fin.close();

   if ( blobData.size() < 1 )
   {
      error = QString( "writeBlob: no data in file " ) + filename;
      db_errno = DBERROR;
      return DBERROR;
   }

   if ( tableID == 0 )
   {
      error = QString( "writeBlob: don't know which record data belongs to in " ) + filename;
      db_errno = DBERROR;
      return DBERROR;
   }

   // Create an escaped version of the data
   QByteArray blobData_escaped;
   ulong escaped_length = mysqlEscapeString( blobData_escaped, blobData, blobData.size() );

   // Calculate a checksum
   QByteArray checksum = 
        QCryptographicHash::hash( blobData, QCryptographicHash::Md5 ).toHex();

   // Now let's start building the query
   QString queryPart1 = "CALL " + procedure +
                        "('"    + guid      + 
                        "', '"  + userPW    + 
                        "', "   + QString::number( tableID )   +
                        ", '"   ;
   QByteArray sqlQuery( escaped_length 
                      + queryPart1.size()
                      + checksum.size() + 7, '\0' );
   strcpy( sqlQuery.data(), queryPart1.toLatin1().constData() );
   char* queryPtr = sqlQuery.data() + queryPart1.size();
   memcpy( queryPtr, blobData_escaped.data(), escaped_length );
   queryPtr += escaped_length;
   strcpy( queryPtr, "', '" );
   queryPtr += 4;
   memcpy( queryPtr, checksum.data(), checksum.size() );
   queryPtr += checksum.size();
   strcpy( queryPtr, "')\0" );

   /*
   QByteArray sqlQuery2( escaped_length 
                       + queryPart1.size()
                       + checksum.size() + 7, '\0' );
   sqlQuery2 = queryPart1.toLatin1()
             + blobData_escaped + "', '"
             + checksum + "')\0";

   ulong checkSize = escaped_length + queryPart1.size() + checksum.size() + 7;
   qDebug() << ( ( sqlQuery == sqlQuery2 ) ? "check equal" : "check not equal" );
   qDebug() << "check2 " << memcmp( sqlQuery.data(), sqlQuery2.data(), checkSize );
*/

   // We can't use rawQuery() since it takes a QString
   drainResults();

   if ( mysql_query( db, sqlQuery.data() ) != 0 )
   {
     error = QString( "MySQL error: " ) + mysql_error( db ); // ALEXEY: writing huge blob, MySql server has gone away...
     noteStatementError();

     db_errno = DBERROR;
     return DBERROR;
   }

   result   = mysql_store_result( db );
   row      = mysql_fetch_row( result );
   db_errno = atoi( row[ 0 ] );

//qDebug() << "Debug " << lastDebug();
   mysql_free_result( result );
   result = NULL;

   if ( db_errno == BAD_CHECKSUM )
   {
      error = QString( "writeBlob: data transmission error (MD5 checksum)" ) ;

      return BAD_CHECKSUM;
   }

   else if ( db_errno != OK )
   {
      error = QString( "MySQL error: " ) + lastError();

      return db_errno;
   }

   return db_errno;
}
#endif

#ifdef NO_DB
int US_DB2::readBlobFromDB( const QString& , const QString& , const int ) { return 0; }
#else
int US_DB2::readBlobFromDB( const QString& filename, 
    const QString& procedure, const int tableID )
{
   // First let's build the query
   QString sqlQuery = "CALL " + procedure +
                      "('"    + guid      + 
                      "', '"  + userPW    + 
                      "', "   + QString::number( tableID )   +
                      ")"   ;

   // We can't use standard methods because the
   // binary data doesn't all transfer

   // Make sure that we clear out any unused
   //   result sets
   drainResults();

   if ( mysql_query( db, sqlQuery.toLatin1() ) != OK )
   {
      error = QString( "MySQL error: " ) + mysql_error( db );
      noteStatementError();
      db_errno = DBERROR;
      return DBERROR;
   }

   // First result set is status
   result   = mysql_store_result( db );
   row      = mysql_fetch_row( result );
   db_errno = atoi( row[ 0 ] );

   mysql_free_result( result );
   result = NULL;

   // Now get the result data
   if ( mysql_next_result( db ) == 0 )
   {
      result = mysql_store_result( db );

      // Make sure we get the data and the right number of bytes
      row    = mysql_fetch_row( result );
      ulong* lengths = mysql_fetch_lengths( result );

      // Now convert
      QByteArray aucData( row[ 0 ], lengths[ 0 ] );
      QByteArray checksum = row[ 1 ];
      QByteArray calculated = 
           QCryptographicHash::hash( aucData, QCryptographicHash::Md5 ).toHex();

      mysql_free_result( result );
      result = NULL;

      // Since we got data, let's write it out
      QFile fout( filename );
      if ( checksum != calculated )
      {
         error = QString( "readBlob: data transmission error (MD5 checksum)" ) ;

         db_errno = BAD_CHECKSUM;
      }

      else if ( ! fout.open( QIODevice::WriteOnly ) )
      {
         error = QString( "readBlob: could not write file " ) + filename;

         db_errno = DBERROR;
      }

      else
      {
         fout.write( aucData );
         fout.close();
      }
if(!error.isEmpty()) qDebug() << error;
   }

   return db_errno;
}
#endif

#ifdef NO_DB
int US_DB2::lastInsertID( void ){ return 0; }
#else
int US_DB2::lastInsertID( void )
{
   int ID = -1;

   this->rawQuery( "SELECT last_insertID()" );
   
   if ( result )
   {
      row = mysql_fetch_row( result );
      ID = atoi( row[ 0 ] );
      mysql_free_result( result );
      result = NULL;
   }

   return ID;
}
#endif

#ifdef NO_DB
QString US_DB2::lastDebug( void ){ return ""; }
#else
QString US_DB2::lastDebug( void )
{
   QString debug = "";

   // Let's see if there is a debug message available
   this->rawQuery( "SELECT last_debug()" );
   if ( result )
   {
      row       = mysql_fetch_row( result );
      debug     = row[ 0 ];
      mysql_free_result( result );
      result = NULL;
   }

   return ( debug );
}
#endif

#ifdef NO_DB
unsigned long US_DB2::mysqlEscapeString( QByteArray& , QByteArray& , unsigned long  ) { return 0; }
#else
unsigned long US_DB2::mysqlEscapeString( QByteArray& to, QByteArray& from, unsigned long length )
{
   // Size the QByteArray properly
   // worst case every character has to be escaped plus a trailing \0
   to.resize( length * 2 + 1 );

   const char* fromPtr = from.constData();
   char* toPtr         = to.data();

   // to_length is the length of the QByteArray without the \0, but according to the docs
   // the trailing \0 is still written for the mariadb-/mysql-connector
   ulong to_length = mysql_real_escape_string( db, toPtr, fromPtr, length );

   // Size string appropriately and return new length
   // The trailing '\0' shouldn't be included in the size
   to.resize( to_length );
   return to_length;
}
#endif

#ifdef NO_DB
int US_DB2::writeAucToDB( const QString&, int ) { return 0; }
#else
int US_DB2::writeAucToDB( const QString& filename, int tableID ) 
{
   // First compress th file
   US_Gzip gz;
   int     retCode = gz.gzip( filename );
   
   QString fn = filename;

   if ( retCode == 0 )
   {  
      fn  += ".gz";  // gzip renames the file
      retCode = writeBlobToDB( fn, "upload_aucData", tableID );

      // Now uncompress the file
      gz.gzip( fn );
   }

   return retCode;
}
#endif

#ifdef NO_DB
int US_DB2::readAucFromDB( const QString&, int ) { return 0; }
#else
int US_DB2::readAucFromDB( const QString& filename, int tableID ) 
{
   QString fn = filename + ".gz";

   int retCode = readBlobFromDB( fn, "download_aucData", tableID );

   if ( retCode == OK )
   {
      // Check to see if it is a gzipped file
      char  buf[ 2 ];
      QFile t( fn );
      t.open( QIODevice::ReadOnly );
      t.peek( buf, 2 );
      t.close();

      // Look for gzip magic number
      if ( buf[ 0 ] == '\037'  &&  buf[ 1 ] == '\213' )
      {
         US_Gzip gz;
         retCode = gz.gunzip( fn );
      }
      else // not a gz file, jsut rename it
      {
         QFile::rename( fn, filename ); 
      }
   }

   return retCode;
}
#endif

#ifdef NO_DB
bool US_DB2::configure_ssl( QString& err )
{
   error = "";
   err = error;
   return true;
}
#else
bool US_DB2::configure_ssl( QString& err )
{
   int success = -1;
   error = "";
   err = error;
   // Set connection to use ssl encryption
   success = mysql_options( db, MYSQL_OPT_SSL_KEY, keyFile.toLatin1().constData() );
   if ( success != 0 ) {
      error = QString( "Error setting SSL key file: " ) + keyFile;
      err = error;
      return false;
   }
   success = mysql_options( db, MYSQL_OPT_SSL_CERT, certFile.toLatin1().constData() );
   if ( success != 0 ) {
      error = QString( "Error setting SSL cert file: " ) + certFile;
      err = error;
      return false;
   }
   success = mysql_options( db, MYSQL_OPT_SSL_CA, caFile.toLatin1().constData() );
   if ( success != 0 ) {
      error = QString( "Error setting SSL CA file: " ) + caFile;
      err = error;
      return false;
   }
   success = mysql_options( db, MYSQL_OPT_SSL_CAPATH, nullptr );
   if ( success != 0 ) {
      error = QString( "Error setting SSL CA path " );
      err = error;
      return false;
   }
   // Due to legacy certs and ciphers, security policies must be relaxed
   if ( US_Settings::debug_match( "DB_ENFORCE_DEFAULT_SSL_SECURITY" ) ) {
#if !defined(__MINGW64__)
         // set default SSL ciphers
         success = mysql_options( db, MYSQL_OPT_SSL_CIPHER, CIPHER );
         if ( success != 0 ) {
            error = QString( "Error setting SSL cipher: " ) + CIPHER;
            err = error;
            return false;
         }
#endif
      // the user enforces default security
      return true;
   }
#if !defined(__MINGW64__)
   // apply :@SECLEVEL=0 to the cipher
   const QString cipher = QString::asprintf( "%s:@SECLEVEL=0", CIPHER );
   success = mysql_options( db, MYSQL_OPT_SSL_CIPHER, cipher.toLatin1().constData() );
   if ( success != 0 ) {
      error = QString( "Error setting SSL cipher: " ) + cipher;
      err = error;
      return false;
   }
#else
   // MSYS2/64 (MINGW64) Windows builds link a Schannel-backed libmariadb,
   // which fails to match any suite from the OpenSSL-style CIPHER list
   // above (TLS SEC_E_ALGORITHM_MISMATCH, "no cipher match"). Skip setting
   // MYSQL_OPT_SSL_CIPHER here and let the backend negotiate its own
   // default suite set instead. Does not affect MSVC/vcpkg builds
   // (__MINGW64__ is not defined under MSVC). See ultrascan-tickets#940.
#endif
   // force a minimum of TLS 1.2
   const QString tls_versions ("TLSv1.2,TLSv1.3");
#if defined(LIBMARIADB)
   // for libmariadb it is MARIADB_OPT_TLS_VERSION
   success = mysql_options( db, MARIADB_OPT_TLS_VERSION, tls_versions.toLatin1().constData() );
#else
   // for libmysql it is MYSQL_OPT_TLS_VERSION
   success = mysql_options( db, MYSQL_OPT_TLS_VERSION, tls_versions.toLatin1().constData() );
#endif
   if ( success != 0 ) {
      error = QString( "Error setting TLS version: " ) + tls_versions;
      err = error;
      return false;
   }
   // disable verification of ssl server cert (hostname)
#if defined(LIBMARIADB)
   // for libmariadb disable MYSQL_OPT_SSL_VERIFY_SERVER_CERT
   unsigned long verify = 0; // 0 = disable, 1 = enable
   success = mysql_options( db, MYSQL_OPT_SSL_VERIFY_SERVER_CERT, &verify );
#else
   // for libmysql set MYSQL_OPT_SSL_MODE to SSL_MODE_REQUIRED
   unsigned int ssl_mode = SSL_MODE_REQUIRED; // SSL on, but no CA/hostname verification
   success = mysql_options( db, MYSQL_OPT_SSL_MODE, &ssl_mode );
#endif
   if ( success != 0 ) {
      error = QString( "Error disabling SSL verify server cert." );
      err = error;
      return false;
   }
   return true;
}
#endif
