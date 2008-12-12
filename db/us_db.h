//! \file us_db.h
#ifndef US_DB_H
#define US_DB_H

#include <QtCore>
#include <QtSql>

class US_DB
{
  public:
    US_DB (){};
    ~US_DB(){};

    QSqlDatabase db;

    // Programs
    static bool test_db_connection( 
        const QString&, const QString&, const QString&,
        const QString&, QString& );

    bool open ( QString& );
    void close( void );
    //QString query( const QString& );
    // Others
};
#endif
