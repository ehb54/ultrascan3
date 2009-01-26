//! \file us_db.cpp
#include "us_db.h"
#include "us_settings.h"
#include "us_crypto.h"

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

bool US_DB::open( const QString& masterPW, QString& error )
{
  QStringList defaultDB = US_Settings::defaultDB();

  db.setUserName    ( defaultDB.at( 1 ) );
  db.setDatabaseName( defaultDB.at( 2 ) );
  db.setHostName    ( defaultDB.at( 3 ) );

  QString passwd = US_Crypto::decrypt( defaultDB.at( 4 ), masterPW, defaultDB.at( 5 ) );

  db.setPassword    ( passwd );

  bool status = db.open();
  error       = db.lastError().text();

  return status;
}

void US_DB::close( void )
{
  db.close();
}
