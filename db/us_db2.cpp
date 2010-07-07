//! \file us_db2.cpp
#include "us_db2.h"
#include "us_settings.h"
#include "us_crypto.h"

US_DB2::US_DB2()
{
#ifndef NO_DB
   QString certDir    = qApp->applicationDirPath().replace( "/bin", "/etc" );
   certFile = certDir + QString( "/ca-cert.pem" );

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
   QString certDir    = qApp->applicationDirPath().replace( "/bin", "/etc" );
   certFile = certDir + QString( "/ca-cert.pem" );

   connected  = false;
   result     = NULL;
   db         = mysql_init( NULL );

   QString err;
   if ( ! connect( masterPW, err ) )
   {
      errno = NOT_CONNECTED;
      error = "US_DB2 error: could not connect";
      return;
   }

   errno     = OK;
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

   bool status = mysql_real_connect( 
                 conn,
                 host    .toAscii(), 
                 user    .toAscii(), 
                 password.toAscii(), 
                 dbname  .toAscii(), 
                 0, NULL, CLIENT_MULTI_STATEMENTS );
 
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
        QString& error )
{
   error = "";

   if ( ! db )
   {
      error = QString( "Test secure connection allocation error" );
      return false;
   }

   // Set connection to use ssl encryption
   mysql_ssl_set( db,
                  NULL,
                  NULL,
                  certFile.toAscii(),
                  NULL,
                  "AES128-SHA");

   bool status = mysql_real_connect( 
                 db,
                 host    .toAscii(), 
                 user    .toAscii(), 
                 password.toAscii(), 
                 dbname  .toAscii(), 
                 0, NULL, CLIENT_MULTI_STATEMENTS );
 
   if ( ! status )
   {
      error = QString( "Test secure connection open error\n" ) +
              mysql_error( db );
      return false;
   }

   // Let's see if the user can log in
   status    = false;
   QString q = "CALL validate_user( '', '" + email + "', '" + pw + "' )";
   
   this->query( q );

   if ( errno != OK )
   {
      error = mysql_error( db );
      return false;
   }
   
   next();  // Setup for calling routine to just use value()

   return true;
}
#endif

#ifdef NO_DB
bool US_DB2::connect( const QString&, QString& ){ return false; }
#else
bool US_DB2::connect( const QString& masterPW, QString& error )
{
   if ( connected ) return true;

   QStringList defaultDB = US_Settings::defaultDB();
   if ( defaultDB.size() < 6 )
   {
       errno = NOT_CONNECTED;
       error = "US_DB2 error: DB not configured";
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
                     NULL,
                     NULL,
                     certFile.toAscii(),
                     NULL,
                     "AES128-SHA");

      // The CLIENT_MULTI_STATEMENTS flag allows for multiple queries and multiple
      //   result sets from a single stored procedure. It is required for any 
      //   stored procedure that returns result sets.
      connected = mysql_real_connect( 
                  db,
                  host    .toAscii(), 
                  user    .toAscii(), 
                  password.toAscii(), 
                  dbname  .toAscii(), 
                  0, NULL, CLIENT_MULTI_STATEMENTS );
 
   }

   catch ( std::exception &e )
   {
      error = e.what();
   }

   errno = OK;
   error = "";
   
   if ( ! connected )
   {
      errno = NOT_CONNECTED;
      error = QString( "Connect open error: " ) + mysql_error( db );
   }

   email  = defaultDB.at( 6 );  // Save for later

   // DB Internal PW
   cipher = defaultDB.at( 7 );
   iv     = defaultDB.at( 8 );
   userPW = US_Crypto::decrypt( cipher, masterPW, iv );
   guid   = defaultDB.at( 9 );

   QString q = "CALL validate_user( '" + guid + "', '', '" + userPW + "' )";
   
   this->query( q );

   if ( errno != OK )
   {
      error = mysql_error( db );
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
                     NULL,
                     NULL,
                     certFile.toAscii(),
                     NULL,
                     "AES128-SHA");

      // The CLIENT_MULTI_STATEMENTS flag allows for multiple queries and multiple
      //   result sets from a single stored procedure. It is required for any 
      //   stored procedure that returns result sets.
      connected = mysql_real_connect( 
                  db,
                  host    .toAscii(), 
                  user    .toAscii(), 
                  password.toAscii(), 
                  dbname  .toAscii(), 
                  0, NULL, CLIENT_MULTI_STATEMENTS );
   }

   catch ( std::exception &e )
   {
      error = e.what();
   }

   errno = OK;
   error = "";
   if ( !connected )
   {
      errno = NOT_CONNECTED;
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

   if ( mysql_query( db, sqlQuery.toAscii() ) != 0 )
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
   int value = 0;

   this->rawQuery( sqlQuery );
   if ( result )
   {
      row       = mysql_fetch_row( result );
      value     = atoi( row[ 0 ] );
      mysql_free_result( result );
      result = NULL;
   }

   return value;
}
#endif

int US_DB2::statusQuery( const QStringList& arguments )
{
   return statusQuery( buildQuery( arguments ) );
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
      errno  = atoi( row[ 0 ] );        // status
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

   if ( errno != 0 )
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
int US_DB2::lastInsertID( void ){ return 0; }
#else
int US_DB2::lastInsertID( void )
{
   return this->statusQuery( "SELECT last_insertID()" );
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

