#include "us_settings.h"

#define BASE "/home/bdubbs/ultrascan"

// Programs
QString US_Settings::browser( void )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  return settings.value( "browser",  "/usr/bin/firefox" ).toString();
}

void US_Settings::set_browser( const QString& browser )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  settings.setValue( "browser", browser );
}

// Directories
// UltraScan Home
QString US_Settings::usHomeDir( void )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  QString home = qApp->applicationDirPath().remove( QRegExp( "/bin$" ) );
  return settings.value( "usHomeDir",  home ).toString();
}

void US_Settings::set_usHomeDir( const QString& dir )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  if ( dir == qApp->applicationDirPath () )
    settings.remove( "usHomeDir" );
  else
    settings.setValue( "usHomeDir", dir );
}

// dataDir
QString US_Settings::dataDir( void )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  return settings.value( "dataDir", QDir::homePath() + "/ultrascan/data" ).toString();
}

void US_Settings::set_dataDir( const QString& dir )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  if ( dir ==  QDir::homePath() + "/ultrascan/data" )
    settings.remove( "dataDir" );
  else
    settings.setValue( "dataDir", dir );
}

// resultDir
QString US_Settings::resultDir( void )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  return settings.value( "resultDir", QDir::homePath() + "/ultrascan/results" ).toString();
}

void US_Settings::set_resultDir( const QString& dir )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  if ( dir ==  QDir::homePath() + "/ultrascan/results" )
    settings.remove( "resultDir" );
  else
    settings.setValue( "resultDir", dir );
}

// reportDir
QString US_Settings::reportDir( void )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  return settings.value( "reportDir", QDir::homePath() + "/ultrascan/reports" ).toString();
}

void US_Settings::set_reportDir( const QString& dir )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  if ( dir ==  QDir::homePath() + "/ultrascan/reports" )
    settings.remove( "reportDir" );
  else
    settings.setValue( "reportDir", dir );
}

// archiveDir
QString US_Settings::archiveDir( void )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  return settings.value( "archiveDir", QDir::homePath() + "/ultrascan/archive" ).toString();
}

void US_Settings::set_archiveDir( const QString& dir )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  if ( dir ==  QDir::homePath() + "/ultrascan/archive" )
    settings.remove( "archiveDir" );
  else
    settings.setValue( "archiveDir", dir );
}

// Help
QString US_Settings::helpDir( void )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  return settings.value( "helpDir", QDir::homePath() + "/ultrascan/doc" ).toString();
}

void US_Settings::set_helpDir( const QString& dir )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  if ( dir ==  QDir::homePath() + "/ultrascan/doc" )
    settings.remove( "helpDir" );
  else
    settings.setValue( "helpDir", dir );
}

// tmp
QString US_Settings::tmpDir( void )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  return settings.value( "tmpDir", QDir::homePath() + "/ultrascan/tmp" ).toString();
}

void US_Settings::set_tmpDir( const QString& dir )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  if ( dir ==  QDir::homePath() + "/ultrascan/tmp" )
    settings.remove( "tmpDir" );
  else
    settings.setValue( "tmpDir", dir );
}

// License
QStringList US_Settings::license( void )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  return settings.value( "license", "" ).toStringList();
}

void US_Settings::set_license( const QStringList& license )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  settings.setValue( "license", license );
}

// Master Password
QByteArray US_Settings::UltraScanPW( void )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  return settings.value( "UltraScanPW" ).toByteArray(); // No default
}

void US_Settings::set_UltraScanPW( const QByteArray& hash )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  settings.setValue( "UltraScanPW", hash );
}

// Temperature Tolerance
double US_Settings::tempTolerance( void )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  return settings.value( "tempTolerance", 0.5 ).toDouble(); // No default
}

void US_Settings::set_tempTolerance( double tempTolerance )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  if ( tempTolerance == 0.5 )
    settings.remove( "tempTolerance" );
  else
    settings.setValue( "tempTolerance", tempTolerance );
}

// Beckman Bug
bool US_Settings::beckmanBug( void )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  return settings.value( "beckmanBug", false ).toBool(); // No default
}

void US_Settings::set_beckmanBug( bool setBug )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  if ( ! setBug )
    settings.remove( "beckmanBug" );
  else
    settings.setValue( "beckmanBug", true );
}

// Threads
int US_Settings::threads( void )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  return settings.value( "threads", 1 ).toInt(); // No default
}

void US_Settings::set_threads( int threads )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  if ( threads == 1 )
    settings.remove( "threads" );
  else
    settings.setValue( "threads", threads );
}

// Database Entries

QList<QStringList> US_Settings::databases( void )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  QList<QStringList> dblist; 
  int dbCount = settings.value( "dbCount", 0 ).toInt();

  for ( int i = 0; i < dbCount; i++ )
  {
    QString key = QString( "dblist%1" ).arg( i );
    dblist << settings.value( key ).toStringList();
  }

  return dblist;
}

void US_Settings::set_databases( const QList<QStringList>& dblist )
{
  QSettings settings( "UTHSCSA", "UltraScan" );

  int dbCount = dblist.length();

  for ( int i = 0; i < dbCount; i++ )
  {
    QString key = QString( "dblist%1" ).arg( i );
    settings.setValue( key, dblist.at( i ) );
  }

  settings.setValue( "dbCount", dbCount );
}

QStringList US_Settings::defaultDB( void )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  return settings.value( "defaultDB", "" ).toStringList();
}

void US_Settings::set_defaultDB( const QStringList& defaultDB )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  settings.setValue( "defaultDB", defaultDB );
}
