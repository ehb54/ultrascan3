//! \file us_db.cpp
#include "us_db.h"
#include "us_settings.h"
#include "us_crypto.h"

US_DB::US_DB()
{
   opened   = false;
   sqlQuery = NULL;
   db = QSqlDatabase::addDatabase( "QMYSQL", "UltraScan" );
}

US_DB::~US_DB()
{
   if ( sqlQuery )
   {
      sqlQuery->clear();
      delete sqlQuery;
   }
}

bool US_DB::test_db_connection( 
        const QString& host, const QString& dbname, 
        const QString& user, const QString& password, 
        QString& error )
{
  QSqlDatabase db = QSqlDatabase::addDatabase( "QMYSQL" );
  db.setUserName    ( user     );
  db.setDatabaseName( dbname   );
  db.setHostName    ( host     );
  db.setPassword    ( password );

  bool status = db.open();
  error       = db.lastError().text();
  db.close();

  return status;
}

bool US_DB::test_secure_connection( 
        const QString& host,  const QString& dbname, 
        const QString& user,  const QString& password, 
        const QString& email, const QString& inv_pw, 
        QString& error )
{
  error = "";

  QSqlDatabase db = QSqlDatabase::addDatabase( "QMYSQL" );
  db.setConnectOptions( "CLIENT_SSL=1" );

  db.setUserName    ( user     );
  db.setDatabaseName( dbname   );
  db.setHostName    ( host     );
  db.setPassword    ( password );

  bool OK = db.open();

  if ( ! OK )
  {
    error = db.lastError().text();
    db.close();
    return false;
  }

  QString query = "select check_user( '" 
                + email + "', '"
                + inv_pw + "' )";

  QSqlQuery q( query, db );

  q.next();

  OK = ( q.value( 0 ).toInt() == 1 ) ? true : false;

  if ( ! OK ) error = "Investigator email or password is incorrect";
  
  db.close();
  return OK;
}

bool US_DB::open( const QString& masterPW, QString& error )
{
  if ( opened ) return true;

  QStringList defaultDB = US_Settings::defaultDB();
  if ( defaultDB.size() < 6 )
  {
      error = "DB not configured";
      return false;
  }

  db.setUserName    ( defaultDB.at( 1 ) );
  db.setDatabaseName( defaultDB.at( 2 ) );
  db.setHostName    ( defaultDB.at( 3 ) );

  QString passwd = US_Crypto::decrypt( defaultDB.at( 4 ), masterPW, defaultDB.at( 5 ) );

  db.setPassword    ( passwd );

  opened = db.open();
  error  = db.lastError().text();

  return opened;
}

void US_DB::query( const QString& q )
{
   if ( sqlQuery )
   {
      sqlQuery->clear();
      delete sqlQuery;
   }

   sqlQuery = new QSqlQuery( q, db );
   error    = sqlQuery->lastError();
   sqlQuery->setForwardOnly( true );
}

QVariant US_DB::value( int index )
{
   if ( sqlQuery ) return sqlQuery->value( index );
   
   return QVariant::Invalid;
}

bool US_DB::isOpen ( void )
{
   return opened;
}

bool US_DB::next( void )
{ 
   return ( sqlQuery )? sqlQuery->next() : false ; 
}

int US_DB::numRows( void )
{ 
   return ( sqlQuery )? sqlQuery->size() : -1; 
}
