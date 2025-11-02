#include "us3i_settings.h"
#include <QRegularExpression>
#include "us3i_defines.h"

// Programs
QString US3i_Settings::browser( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "browser",  "/usr/bin/firefox" ).toString();
}

void US3i_Settings::set_browser( const QString& browser )
{
  QSettings settings( US3, "UltraScan" );
  settings.setValue( "browser", browser );
}

// Directories

// Work base directory (where archive,results,reports,etc hang)
QString US3i_Settings::workBaseDir( void )
{
   QSettings settings( US3, "UltraScan" );
   return settings.value( "workBaseDir",
                          QDir::homePath() + "/ultrascan" ).toString();
}

void US3i_Settings::set_workBaseDir( const QString& dir )
{
  QSettings settings( US3, "UltraScan" );
  if ( dir ==  QDir::homePath() + "/ultrascan" )
    settings.remove( "workBaseDir" );
  else
    settings.setValue( "workBaseDir", dir );
}

// importDir
QString US3i_Settings::importDir( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "importDir", workBaseDir() + "/imports" ).toString();
}

void US3i_Settings::set_importDir( const QString& dir )
{
  QSettings settings( US3, "UltraScan" );
  if ( dir == workBaseDir() + "/imports" )
    settings.remove( "importDir" );
  else
    settings.setValue( "importDir", dir );
}

// tmpDir
QString US3i_Settings::tmpDir( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "tmpDir", workBaseDir() + "/tmp" ).toString();
}

void US3i_Settings::set_tmpDir( const QString& dir )
{
  QSettings settings( US3, "UltraScan" );
  if ( dir == workBaseDir() + "/tmp" )
    settings.remove( "tmpDir" );
  else
    settings.setValue( "tmpDir", dir );
}

// dataDir
QString US3i_Settings::dataDir( void )
{
   return ( workBaseDir() + "/data" );
}

// archiveDir
QString US3i_Settings::archiveDir( void )
{
   return ( workBaseDir() + "/archive" );
}

// resultDir
QString US3i_Settings::resultDir( void )
{
   return ( workBaseDir() + "/results" );
}

// reportDir
QString US3i_Settings::reportDir( void )
{
   return ( workBaseDir() + "/reports" );
}

// etcDir
QString US3i_Settings::etcDir( void )
{
   return ( workBaseDir() + "/etc" );
}

// Base to application directory
QString US3i_Settings::appBaseDir( void )
{
   QString base = qApp->applicationDirPath().remove( QRegularExpression( QStringLiteral( "/bin$" ) ) );

   if ( base.contains( ".app/Contents" ) )
   {  // For Mac, move up path to where ./bin exists
      int ii  = base.lastIndexOf( "/bin/" );

      if ( ii > 0 )
         base    = base.left( ii );
   }

   return base;
}


// License
QStringList US3i_Settings::license( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "license", QStringList() ).toStringList();
}

void US3i_Settings::set_license( const QStringList& license )
{
  QSettings settings( US3, "UltraScan" );
  settings.setValue( "license", license );
}

// Master Password
QByteArray US3i_Settings::UltraScanPW( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "UltraScanPW" ).toByteArray(); // No default
}

void US3i_Settings::set_UltraScanPW( const QByteArray& hash )
{
  QSettings settings( US3, "UltraScan" );
  settings.setValue( "UltraScanPW", hash );
}

// Temperature Tolerance
double US3i_Settings::tempTolerance( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "tempTolerance", 0.5 ).toDouble(); // No default
}

void US3i_Settings::set_tempTolerance( double tempTolerance )
{
  QSettings settings( US3, "UltraScan" );
  if ( tempTolerance == 0.5 )
    settings.remove( "tempTolerance" );
  else
    settings.setValue( "tempTolerance", tempTolerance );
}

// Beckman Bug
bool US3i_Settings::beckmanBug( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "beckmanBug", false ).toBool(); // No default
}

void US3i_Settings::set_beckmanBug( bool setBug )
{
  QSettings settings( US3, "UltraScan" );
  if ( ! setBug )
    settings.remove( "beckmanBug" );
  else
    settings.setValue( "beckmanBug", true );
}

// Default data location  1 = DB, 2 = Disk
int US3i_Settings::default_data_location( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "dataLocation", 2 ).toInt(); 
}

void US3i_Settings::set_default_data_location( int location )
{
  QSettings settings( US3, "UltraScan" );
  if ( location == 2 )
    settings.remove( "dataLocation" );
  else
    settings.setValue( "dataLocation", location );
}

// us_debug
#ifndef NO_DB
int US3i_Settings::us_debug( void )
{
   QSettings settings( US3, "UltraScan" );
   return settings.value( "us_debug", 0 ).toInt();
}

void US3i_Settings::set_us_debug( int level )
{
   QSettings settings( US3, "UltraScan" );
   if ( level == 0 )
      settings.remove( "us_debug" );
   else
      settings.setValue( "us_debug", level );
}
#else
static int us3i_settings_debug = 0;
int US3i_Settings::us_debug( void )
{
   return us3i_settings_debug;
}

void US3i_Settings::set_us_debug( int level )
{
   us3i_settings_debug = level;
}
#endif

// debug text
#ifndef NO_DB
void US3i_Settings::set_debug_text( QStringList debuglist )
{
   QSettings settings( US3, "UltraScan" );
   if ( debuglist.count() == 0 )
      settings.remove( "debug_text" );
   else
      settings.setValue( "debug_text", debuglist );
}

QStringList US3i_Settings::debug_text( void )
{
   QSettings settings( US3, "UltraScan" );
   return settings.value( "debug_text", "" ).toStringList();
}

#else
static QStringList us_sett_debug_text;
void US3i_Settings::set_debug_text( QStringList debuglist )
{
   if ( debuglist.count() == 0 )
      us_sett_debug_text.clear();
   else
      us_sett_debug_text  = debuglist;
}

QStringList US3i_Settings::debug_text( void )
{
   return us_sett_debug_text;
}
#endif

bool US3i_Settings::debug_match( QString match )
{
   return debug_text().contains( match, Qt::CaseInsensitive );
}


// Investigator
QString US3i_Settings::us_inv_name( void )
{
   QSettings settings( US3, "UltraScan" );
   return settings.value( "us_investigator_name", "Not Available" ).toString();
}

void US3i_Settings::set_us_inv_name( const QString& name )
{
  QSettings settings( US3, "UltraScan" );
  if ( name == "" )
    settings.remove( "us_investigator_name" );
  else
    settings.setValue( "us_investigator_name", name );
}

int US3i_Settings::us_inv_ID( void )
{
   QSettings settings( US3, "UltraScan" );
   return settings.value( "us_investigator_ID", -1 ).toInt();
}

void US3i_Settings::set_us_inv_ID( int id )
{
  QSettings settings( US3, "UltraScan" );
  if ( id == -1 )
    settings.remove( "us_investigator_ID" );
  else
    settings.setValue( "us_investigator_ID", id );
}

int US3i_Settings::us_inv_level( void )
{
   QSettings settings( US3, "UltraScan" );
   return settings.value( "us_investigator_level", 0 ).toInt();
}

void US3i_Settings::set_us_inv_level( int level )
{
  QSettings settings( US3, "UltraScan" );
  if ( level == 0 )
    settings.remove( "us_investigator_level" );
  else
    settings.setValue( "us_investigator_level", level );
}

// advanced level
int US3i_Settings::advanced_level( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "advanced_level", 0 ).toInt();
}

void US3i_Settings::set_advanced_level( int level )
{
  QSettings settings( US3, "UltraScan" );
  if ( level == 0 )
    settings.remove( "advanced_level" );
  else
    settings.setValue( "advanced_level", level );
}

// Threads
int US3i_Settings::threads( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "threads", 1 ).toInt(); // No default
}

void US3i_Settings::set_threads( int threads )
{
  QSettings settings( US3, "UltraScan" );
  if ( threads == 1 )
    settings.remove( "threads" );
  else
    settings.setValue( "threads", threads );
}

// Noise Dialog:  0 -> Auto, 1 -> Dialog
int US3i_Settings::noise_dialog( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "noise_dialog", 0 ).toInt();
}

void US3i_Settings::set_noise_dialog( int diagflag )
{
  QSettings settings( US3, "UltraScan" );
  if ( diagflag == 0 )
    settings.remove( "noise_dialog" );
  else
    settings.setValue( "noise_dialog", diagflag );
}

// Database Entries

QList<QStringList> US3i_Settings::databases( void )
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

void US3i_Settings::set_databases( const QList<QStringList>& dblist )
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

QStringList US3i_Settings::defaultDB( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "defaultDB", QStringList() ).toStringList();
}

void US3i_Settings::set_defaultDB( const QStringList& defaultDB )
{
  QSettings settings( US3, "UltraScan" );
  if ( defaultDB.isEmpty() )
    settings.remove( "defaultDB" );
  else
    settings.setValue( "defaultDB", defaultDB );
}

// Optima Database Host Entries
QList<QStringList> US3i_Settings::xpn_db_hosts( void )
{
  QSettings settings( US3, "UltraScan" );
  QList<QStringList> xhlist; 
  int xhCount = settings.value( "xhCount", 0 ).toInt();

  for ( int ii = 0; ii < xhCount; ii++ )
  {
    QString key = QString( "xhlist%1" ).arg( ii );
    xhlist << settings.value( key ).toStringList();
  }

  return xhlist;
}

void US3i_Settings::set_xpn_db_hosts( const QList<QStringList>& xhlist )
{
  QSettings settings( US3, "UltraScan" );

  // First remove any existing database entries
  if ( settings.contains( "xhCount" ) )
  {
    int count = settings.value( "xhCount" ).toInt();
    for ( int ii = 0; ii < count; ii++ )
    {
      QString key = QString( "xhlist%1" ).arg( ii );
      settings.remove( key );
    }
  }

  int xhCount = xhlist.size();

  for ( int ii = 0; ii < xhCount; ii++ )
  {
    QString key = QString( "xhlist%1" ).arg( ii );
    settings.setValue( key, xhlist.at( ii ) );
  }

  settings.setValue( "xhCount", xhCount );
}

QStringList US3i_Settings::defaultXpnHost( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "defXpnHost", QStringList() ).toStringList();
}

void US3i_Settings::set_def_xpn_host( const QStringList& defXpnHost )
{
  QSettings settings( US3, "UltraScan" );
  if ( defXpnHost.isEmpty() )
    settings.remove( "defXpnHost" );
  else
    settings.setValue( "defXpnHost", defXpnHost );
}

