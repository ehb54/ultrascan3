//! \file us_db2.cpp
#include "us_db2.h"
#include "us_settings.h"
#include "us_crypto.h"
#include "us_gzip.h"
#include "us_util.h"

#define CIPHER "AES256-GCM-SHA384:AES256-SHA:AES256-SHA256:AES256-CCM:AES256-GCM-SHA384:AES128-SHA"

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
  mysql_free_result( result );
  mysql_close(db);

  result      = NULL;
  connected   = false;
#endif
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

   // Set connection to use ssl encryption
   mysql_ssl_set( db,
                  keyFile .toLatin1(),
                  certFile.toLatin1(),
                  caFile  .toLatin1(),
                  NULL,
                  CIPHER );

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
   try
   {
      // Set connection to use ssl encryption
      mysql_ssl_set( db,
                     keyFile .toLatin1(),
                     certFile.toLatin1(),
                     caFile  .toLatin1(),
                     NULL,
                     CIPHER );

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
        QString& error )
{
  if ( connected ) return true;

   try
   {
      // Set connection to use ssl encryption
      mysql_ssl_set( db,
                     keyFile .toLatin1(),
                     certFile.toLatin1(),
                     caFile  .toLatin1(),
                     NULL,
                     CIPHER );

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
      error = e.what();
   }

   db_errno = OK;
   error = "";
   if ( ! connected )
   {
      db_errno = NOT_CONNECTED;
      error = QString( "Connect open error: " ) + mysql_error( db );
   }

  return connected;
}
#endif

#ifdef NO_DB
void US_DB2::rawQuery( const QString& ){}
#else
void US_DB2::rawQuery( const QString& sqlQuery )
{
   // Make sure that we clear out any unused
   //   result sets
   if ( result )
      mysql_free_result( result ); 

   while ( mysql_next_result( db ) == 0 )
   {
      result = mysql_store_result( db );
      mysql_free_result( result );
   }
   result = NULL;

   if ( mysql_query( db, sqlQuery.toLatin1() ) != 0 )
      error = QString( "MySQL error: " ) + mysql_error( db );

   else
      result = mysql_store_result( db );
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
   if ( result )
   {
      // This is a 2-set result: status, then data
      row    = mysql_fetch_row( result );
      db_errno  = atoi( row[ 0 ] );        // status
      mysql_free_result( result );
      result = NULL;

      if ( mysql_next_result( db ) == 0 ) // get the result data
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
QVariant US_DB2::value( unsigned ){ return QVariant::Invalid; }
#else
QVariant US_DB2::value( unsigned index )
{
   if ( row && ( index < mysql_field_count( db ) ) )
      return row[ index ];

   return QVariant::Invalid;
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
 
   // We can't use standard methods since they use QStrings
   // Clear out any unused result sets
   if ( result )
      mysql_free_result( result ); 

   while ( mysql_next_result( db ) == 0 )
   {
      result = mysql_store_result( db );
      mysql_free_result( result );
   }
   result = NULL;

   if ( mysql_query( db, sqlQuery.data() ) != 0 )
   {
     error = QString( "MySQL error: " ) + mysql_error( db ); // ALEXEY: writing huge blob, MySql server has gone away...
  
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
   if ( result )
      mysql_free_result( result ); 

   while ( mysql_next_result( db ) == 0 )
   {
      result = mysql_store_result( db );
      mysql_free_result( result );
   }
   result = NULL;

   if ( mysql_query( db, sqlQuery.toLatin1() ) != OK )
   {
      error = QString( "MySQL error: " ) + mysql_error( db );
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
   to.resize( length * 2 + 1 );     // Make room in advance for escaped characters

   const char* fromPtr = from.data();
   char* toPtr         = to.data();

   ulong to_length = mysql_real_escape_string( db, toPtr, fromPtr, length );

   // Add null termination to the string
   toPtr += to_length;
   strcpy( toPtr, "\0" );

   // Size string appropriately and return new length
   to.resize( to_length + 1 );
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

