//! \file us_db2.cpp
#include "us_db2.h"

#ifndef NO_DB
#include "us_settings.h"
#include "us_crypto.h"
#include "us_gzip.h"
#include "us_util.h"
#endif

#define CIPHER "AES256-GCM-SHA384:AES256-SHA:AES256-SHA256:AES256-CCM:AES256-GCM-SHA384:AES128-SHA"

US_DB2::US_DB2()
{
   QString certPath = "/ultrascan3" + QString( "/etc/mysql/" );
   keyFile    = certPath + QString( "server-key.pem" );
   certFile   = certPath + QString( "server-cert.pem" );
   caFile     = certPath + QString( "ca-cert.pem" );

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

bool US_DB2::test_secure_connection( 
        const QString& host,
        const QString& dbname, 
        const QString& user,
        const QString& password, 
        const QString& email,
        const QString& pw, 
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
   QTextStream( stdout ) <<
      QString( "test_secure_connection\n"
               "key file  %1 %2\n"
               "cert file %3 %4\n"
               "caFile    %5 %6\n"
               "host      %8\n"
               "dbname    %9\n"
               "user      %10\n"
               "password  %11\n"
               "email     %12\n"
               "pw        %13\n"
               )
      .arg( keyFile ).arg( QFile::exists( keyFile ) ? "exists" : "does NOT exist" )
      .arg( certFile ).arg( QFile::exists( certFile ) ? "exists" : "does NOT exist" )
      .arg( caFile ).arg( QFile::exists( caFile ) ? "exists" : "does NOT exist" )
      .arg( host )
      .arg( dbname )
      .arg( user )
      .arg( password )
      .arg( email )
      .arg( pw )
      ;

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

   QTextStream( stdout ) << "database connection OK, now test user login\n";

   // Let's see if the user can log in
   status    = false;
   QString q = "CALL validate_user( '', '" + email + "', '" + pw + "' )";
   
   this->query( q );

   if ( db_errno != OK )
   {
      error = mysql_error( db );
      err   = error;
      err = QString( "error '%1'\ndb_errno : %2" ).arg( error ).arg( db_errno );
      return false;
   }
   
   next();  // Setup for calling routine to just use value()

   return true;
}


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

int US_DB2::statusQuery( const QStringList& arguments )
{
   return statusQuery( buildQuery( arguments ) );
}

int US_DB2::functionQuery( const QStringList& arguments )
{
   return statusQuery( buildQuerySelect( arguments ) );
}

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

