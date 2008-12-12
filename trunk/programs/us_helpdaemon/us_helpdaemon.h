//! \file us_helpdaemon.cpp
#include <QtCore>

class us_helpdaemon : public QObject
{
  Q_OBJECT

  public:
    us_helpdaemon( const QString&, QObject* = 0 );
    ~us_helpdaemon() {};

  public slots:
    void show ( const QString& );
    void close( int, QProcess::ExitStatus );

  private:
    QProcess    daemon;
    QStringList args; 
    QTextStream ts;
};
