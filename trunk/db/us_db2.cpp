//! \file us_db2.cpp
#include "us_db2.h"
#include "us_settings.h"
#include "us_crypto.h"

US_DB2::US_DB2()
{
   QString certDir    = qApp->applicationDirPath().replace( "/bin", "/etc" );
   certFile = certDir + QString( "/ca-cert.pem" );

   connected  = false;
   result     = NULL;
   db         = mysql_init( NULL );
}

US_DB2::~US_DB2()
{
  mysql_free_result( result );
  mysql_close(db);

  result      = NULL;
  connected   = false;
}

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
                 0, NULL, 0 );
 
   if ( ! status )
      error = QString( "Test connection open error " ) + mysql_error( conn );

   mysql_close( conn );

   return status;
}

bool US_DB2::test_secure_connection( 
        const QString& host,  const QString& dbname, 
        const QString& user,  const QString& password, 
        const QString& email, const QString& pw, 
        QString& error )
{
   error = "";

   MYSQL* conn = mysql_init( NULL );
   if ( ! conn )
   {
      error = QString( "Test secure connection allocation error" );
      return false;
   }

   // Set connection to use ssl encryption
   mysql_ssl_set( conn,
                  NULL,
                  NULL,
                  certFile.toAscii(),
                  NULL,
                  "AES128-SHA");

   bool OK = mysql_real_connect( 
             conn,
             host    .toAscii(), 
             user    .toAscii(), 
             password.toAscii(), 
             dbname  .toAscii(), 
             0, NULL, 0 );
 
   if ( ! OK )
   {
      error = QString( "Test secure connection open error " ) + mysql_error( conn );
      mysql_close( conn );
      return false;
   }

   // Let's see if the user can log in
   OK = false;
   QString q = "SELECT check_user('" + email + "', '" + pw + "')";
   if ( mysql_query( conn, q.toAscii() ) == 0 )
   {
      MYSQL_RES* res = mysql_store_result( conn );
      if ( res )
      {
         MYSQL_ROW r = mysql_fetch_row( res );
         if ( r )
            OK = ( atoi( r[0] ) == 0 ) ? true : false;
      }
   }
   if ( ! OK )
      error = "Investigator email or password is incorrect";
   
   mysql_close( conn );

   return OK;
}

bool US_DB2::connect( const QString& masterPW, QString& error )
{
  if ( connected ) return true;

  QStringList defaultDB = US_Settings::defaultDB();
  if ( defaultDB.size() < 6 )
  {
      error = "DB not configured";
      return false;
  }

   QString user     = defaultDB.at( 1 );
   QString dbname   = defaultDB.at( 2 );
   QString host     = defaultDB.at( 3 );

   QString password = US_Crypto::decrypt( defaultDB.at( 4 ), masterPW, defaultDB.at( 5 ) );

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

   error = "";
   if ( !connected )
      error = QString( "Connect open error: " ) + mysql_error( db );

  return connected;
}

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

   error = "";
   if ( !connected )
      error = QString( "Connect open error: " ) + mysql_error( db );

  return connected;
}

void US_DB2::rawQuery( const QString& q )
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

   if ( mysql_query( db, q.toAscii() ) != 0 )
      error = QString( "MySQL error: " ) + mysql_error( db );

   else
      result = mysql_store_result( db );
}

int US_DB2::statusQuery( const QString& q )
{
   int value = 0;

   this->rawQuery( q );
   if ( result )
   {
      row       = mysql_fetch_row( result );
      value     = atoi( row[ 0 ] );
      mysql_free_result( result );
      result = NULL;
   }

   return value;
}

void US_DB2::query( const QString& q )
{
   this->rawQuery( q );
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
               ; // We are here as the result of an INSERT, UPDATE or DELETE

            else
               ; // Error retrieving result set
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

QVariant US_DB2::value( unsigned index )
{
   if ( row && ( index < mysql_field_count( db ) ) )
      return row[ index ];

   return QVariant::Invalid;
}

bool US_DB2::isConnected ( void )
{
   return connected;
}

int US_DB2::numRows( void )
{ 
   return ( result )? ( (int) mysql_num_rows( result ) ) : -1;
}

int US_DB2::lastInsertID( void )
{
   return this->statusQuery( "SELECT last_insertID()" );
}

