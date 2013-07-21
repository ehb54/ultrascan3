#include "us_settings.h"
#include "us_defines.h"

// Programs
QString US_Settings::browser( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "browser",  "/usr/bin/firefox" ).toString();
}

void US_Settings::set_browser( const QString& browser )
{
  QSettings settings( US3, "UltraScan" );
  settings.setValue( "browser", browser );
}

// Directories
// UltraScan Home
QString US_Settings::usHomeDir( void )
{
  QSettings settings( US3, "UltraScan" );
  //QString home = qApp->applicationDirPath().remove( QRegExp( "/bin$" ) );
  QString home = appBaseDir();
  return settings.value( "usHomeDir",  home ).toString();
}

void US_Settings::set_usHomeDir( const QString& dir )
{
  QSettings settings( US3, "UltraScan" );
  if ( dir == appBaseDir() )
    settings.remove( "usHomeDir" );
  else
    settings.setValue( "usHomeDir", dir );
}

// dataDir
QString US_Settings::dataDir( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "dataDir", QDir::homePath() + "/ultrascan/data" ).toString();
}

void US_Settings::set_dataDir( const QString& dir )
{
  QSettings settings( US3, "UltraScan" );
  if ( dir ==  QDir::homePath() + "/ultrascan/data" )
    settings.remove( "dataDir" );
  else
    settings.setValue( "dataDir", dir );
}

// resultDir
QString US_Settings::resultDir( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "resultDir", QDir::homePath() + "/ultrascan/results" ).toString();
}

void US_Settings::set_resultDir( const QString& dir )
{
  QSettings settings( US3, "UltraScan" );
  if ( dir ==  QDir::homePath() + "/ultrascan/results" )
    settings.remove( "resultDir" );
  else
    settings.setValue( "resultDir", dir );
}

// reportDir
QString US_Settings::reportDir( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "reportDir", QDir::homePath() + "/ultrascan/reports" ).toString();
}

void US_Settings::set_reportDir( const QString& dir )
{
  QSettings settings( US3, "UltraScan" );
  if ( dir ==  QDir::homePath() + "/ultrascan/reports" )
    settings.remove( "reportDir" );
  else
    settings.setValue( "reportDir", dir );
}

// archiveDir
QString US_Settings::archiveDir( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "archiveDir", QDir::homePath() + "/ultrascan/archive" ).toString();
}

void US_Settings::set_archiveDir( const QString& dir )
{
  QSettings settings( US3, "UltraScan" );
  if ( dir ==  QDir::homePath() + "/ultrascan/archive" )
    settings.remove( "archiveDir" );
  else
    settings.setValue( "archiveDir", dir );
}

// Help
QString US_Settings::helpDir( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "helpDir", QDir::homePath() + "/ultrascan/doc" ).toString();
}

void US_Settings::set_helpDir( const QString& dir )
{
  QSettings settings( US3, "UltraScan" );
  if ( dir ==  QDir::homePath() + "/ultrascan/doc" )
    settings.remove( "helpDir" );
  else
    settings.setValue( "helpDir", dir );
}

// tmp
QString US_Settings::tmpDir( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "tmpDir", QDir::homePath() + "/ultrascan/tmp" ).toString();
}

void US_Settings::set_tmpDir( const QString& dir )
{
  QSettings settings( US3, "UltraScan" );
  if ( dir ==  QDir::homePath() + "/ultrascan/tmp" )
    settings.remove( "tmpDir" );
  else
    settings.setValue( "tmpDir", dir );
}

// Base to application directory
QString US_Settings::appBaseDir( void )
{
   QString base = qApp->applicationDirPath().remove( QRegExp( "/bin$" ) );

   if ( base.contains( ".app/Contents" ) )
   {
      int ii  = base.lastIndexOf( "/bin/" );

      if ( ii > 0 )
         base    = base.left( ii );
   }

   return base;
}

// Base to data directory
QString US_Settings::baseDataDir( void )
{
   return dataDir().remove( QRegExp( "/data$" ) );
}


// License
QStringList US_Settings::license( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "license", QStringList() ).toStringList();
}

void US_Settings::set_license( const QStringList& license )
{
  QSettings settings( US3, "UltraScan" );
  settings.setValue( "license", license );
}

// Master Password
QByteArray US_Settings::UltraScanPW( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "UltraScanPW" ).toByteArray(); // No default
}

void US_Settings::set_UltraScanPW( const QByteArray& hash )
{
  QSettings settings( US3, "UltraScan" );
  settings.setValue( "UltraScanPW", hash );
}

// Temperature Tolerance
double US_Settings::tempTolerance( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "tempTolerance", 0.5 ).toDouble(); // No default
}

void US_Settings::set_tempTolerance( double tempTolerance )
{
  QSettings settings( US3, "UltraScan" );
  if ( tempTolerance == 0.5 )
    settings.remove( "tempTolerance" );
  else
    settings.setValue( "tempTolerance", tempTolerance );
}

// Beckman Bug
bool US_Settings::beckmanBug( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "beckmanBug", false ).toBool(); // No default
}

void US_Settings::set_beckmanBug( bool setBug )
{
  QSettings settings( US3, "UltraScan" );
  if ( ! setBug )
    settings.remove( "beckmanBug" );
  else
    settings.setValue( "beckmanBug", true );
}

// Default data location  1 = DB, 2 = Disk
int US_Settings::default_data_location( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "dataLocation", 2 ).toInt(); 
}

void US_Settings::set_default_data_location( int location )
{
  QSettings settings( US3, "UltraScan" );
  if ( location == 2 )
    settings.remove( "dataLocation" );
  else
    settings.setValue( "dataLocation", location );
}

// us_debug
#ifdef NO_DB
static int us_settings_debug = 0;
int US_Settings::us_debug( void )
{
   return us_settings_debug;
}

void US_Settings::set_us_debug( int level )
{
   us_settings_debug = level;
}
#else
int US_Settings::us_debug( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "us_debug", 0 ).toInt();
}

void US_Settings::set_us_debug( int level )
{
  QSettings settings( US3, "UltraScan" );
  if ( level == 0 )
    settings.remove( "us_debug" );
  else
    settings.setValue( "us_debug", level );
}
#endif

// debug text
QStringList US_Settings::debug_text( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "debug_text", "" ).toStringList();
}

bool US_Settings::debug_match( QString match )
{
  return debug_text().contains( match, Qt::CaseInsensitive );
}

void US_Settings::set_debug_text( QStringList debuglist )
{
  QSettings settings( US3, "UltraScan" );
  if ( debuglist.count() == 0 )
    settings.remove( "debug_text" );
  else
    settings.setValue( "debug_text", debuglist );
}

// Investigator
QString US_Settings::us_inv_name( void )
{
   QSettings settings( US3, "UltraScan" );
   return settings.value( "us_investigator_name", "Not Available" ).toString();
}

void US_Settings::set_us_inv_name( const QString& name )
{
  QSettings settings( US3, "UltraScan" );
  if ( name == "" )
    settings.remove( "us_investigator_name" );
  else
    settings.setValue( "us_investigator_name", name );
}

int US_Settings::us_inv_ID( void )
{
   QSettings settings( US3, "UltraScan" );
   return settings.value( "us_investigator_ID", -1 ).toInt();
}

void US_Settings::set_us_inv_ID( int id )
{
  QSettings settings( US3, "UltraScan" );
  if ( id == -1 )
    settings.remove( "us_investigator_ID" );
  else
    settings.setValue( "us_investigator_ID", id );
}

int US_Settings::us_inv_level( void )
{
   QSettings settings( US3, "UltraScan" );
   return settings.value( "us_investigator_level", 0 ).toInt();
}

void US_Settings::set_us_inv_level( int level )
{
  QSettings settings( US3, "UltraScan" );
  if ( level == 0 )
    settings.remove( "us_investigator_level" );
  else
    settings.setValue( "us_investigator_level", level );
}

// advanced level
int US_Settings::advanced_level( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "advanced_level", 0 ).toInt();
}

void US_Settings::set_advanced_level( int level )
{
  QSettings settings( US3, "UltraScan" );
  if ( level == 0 )
    settings.remove( "advanced_level" );
  else
    settings.setValue( "advanced_level", level );
}

// Threads
int US_Settings::threads( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "threads", 1 ).toInt(); // No default
}

void US_Settings::set_threads( int threads )
{
  QSettings settings( US3, "UltraScan" );
  if ( threads == 1 )
    settings.remove( "threads" );
  else
    settings.setValue( "threads", threads );
}

// Database Entries

QList<QStringList> US_Settings::databases( void )
{
  QSettings settings( US3, "UltraScan" );
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
  QSettings settings( US3, "UltraScan" );

  // First remove any existing database entries
  if ( settings.contains( "dbCount" ) )
  {
    int count = settings.value( "dbCount" ).toInt();
    for ( int i = 0; i < count; i++ )
    {
      QString key = QString( "dblist%1" ).arg( i );
      settings.remove( key );
    }
  }

  int dbCount = dblist.size();

  for ( int i = 0; i < dbCount; i++ )
  {
    QString key = QString( "dblist%1" ).arg( i );
    settings.setValue( key, dblist.at( i ) );
  }

  settings.setValue( "dbCount", dbCount );
}

QStringList US_Settings::defaultDB( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "defaultDB", QStringList() ).toStringList();
}

void US_Settings::set_defaultDB( const QStringList& defaultDB )
{
  QSettings settings( US3, "UltraScan" );
  if ( defaultDB.isEmpty() )
    settings.remove( "defaultDB" );
  else
    settings.setValue( "defaultDB", defaultDB );
}
