//! \file us_settings.cpp
#include "us_settings.h"
#include "us_defines.h"

QSettings::Format US_SettingsStore::format( void )
{
   static const QSettings::Format resolved = []
   {
      const QString root = qEnvironmentVariable( "US3_SETTINGS_ROOT" );

      if ( root.isEmpty() )
         return QSettings::NativeFormat;

      // IniFormat is a plain file on every platform, so setPath does apply to
      // it. This is what makes the override work where setPath alone cannot.
      QSettings::setPath( QSettings::IniFormat, QSettings::UserScope, root );
      return QSettings::IniFormat;
   }();

   return resolved;
}

US_SettingsStore::US_SettingsStore( void )
   : QSettings( format(), QSettings::UserScope, US3, "UltraScan" )
{
}

// Programs
QString US_Settings::browser( void )
{
  US_SettingsStore settings;
  QString   value = settings.value( "browser", "" ).toString();

#if defined( Q_OS_MAC ) || defined( Q_OS_WIN )
  // Self-heal a stale "/usr/bin/firefox" default that earlier versions
  // could persist on save even when the user never set a browser. That
  // path is a real, possibly intentional setting on Linux, so leave it
  // alone there.
  if ( value == "/usr/bin/firefox" )
    value = "";
#endif

  return value;
}

void US_Settings::set_browser( const QString& browser )
{
  US_SettingsStore settings;
  settings.setValue( "browser", browser );
}

// Directories

// Work base directory (where archive,results,reports,etc hang)
QString US_Settings::workBaseDir( void )
{
   US_SettingsStore settings;
   return settings.value( "workBaseDir",
                          QDir::homePath() + "/ultrascan" ).toString();
}

void US_Settings::set_workBaseDir( const QString& dir )
{
  US_SettingsStore settings;
  if ( dir ==  QDir::homePath() + "/ultrascan" )
    settings.remove( "workBaseDir" );
  else
    settings.setValue( "workBaseDir", dir );
}

// importDir
QString US_Settings::importDir( void )
{
  US_SettingsStore settings;
  return settings.value( "importDir", workBaseDir() + "/imports" ).toString();
}

void US_Settings::set_importDir( const QString& dir )
{
  US_SettingsStore settings;
  if ( dir == workBaseDir() + "/imports" )
    settings.remove( "importDir" );
  else
    settings.setValue( "importDir", dir );
}

// tmpDir
QString US_Settings::tmpDir( void )
{
  US_SettingsStore settings;
  return settings.value( "tmpDir", workBaseDir() + "/tmp" ).toString();
}

void US_Settings::set_tmpDir( const QString& dir )
{
  US_SettingsStore settings;
  if ( dir == workBaseDir() + "/tmp" )
    settings.remove( "tmpDir" );
  else
    settings.setValue( "tmpDir", dir );
}

// dataDir
QString US_Settings::dataDir( void )
{
   return ( workBaseDir() + "/data" );
}

// archiveDir
QString US_Settings::archiveDir( void )
{
   return ( workBaseDir() + "/archive" );
}

// resultDir
QString US_Settings::resultDir( void )
{
   return ( workBaseDir() + "/results" );
}

// reportDir
QString US_Settings::reportDir( void )
{
   return ( workBaseDir() + "/reports" );
}

// etcDir
QString US_Settings::etcDir( void )
{
   return ( workBaseDir() + "/etc" );
}

// Base to application directory
QString US_Settings::appBaseDir( void )
{
   QString base = qApp->applicationDirPath().remove( QRegularExpression( "/bin$" ) );

   if ( base.contains( ".app/Contents" ) )
   {  // For Mac, move up path to where ./bin exists
      int ii  = base.lastIndexOf( "/bin/" );

      if ( ii > 0 )
         base    = base.left( ii );
   }

   return base;
}


// License
QStringList US_Settings::license( void )
{
  US_SettingsStore settings;
  return settings.value( "license", QStringList() ).toStringList();
}

void US_Settings::set_license( const QStringList& license )
{
  US_SettingsStore settings;
  settings.setValue( "license", license );
}

// Master Password
QByteArray US_Settings::UltraScanPW( void )
{
  US_SettingsStore settings;
  return settings.value( "UltraScanPW" ).toByteArray(); // No default
}

void US_Settings::set_UltraScanPW( const QByteArray& hash )
{
  US_SettingsStore settings;
  settings.setValue( "UltraScanPW", hash );
}

// Temperature Tolerance
double US_Settings::tempTolerance( void )
{
  US_SettingsStore settings;
  return settings.value( "tempTolerance", 0.5 ).toDouble(); // No default
}

void US_Settings::set_tempTolerance( double tempTolerance )
{
  US_SettingsStore settings;
  if ( tempTolerance == 0.5 )
    settings.remove( "tempTolerance" );
  else
    settings.setValue( "tempTolerance", tempTolerance );
}

// Beckman Bug
bool US_Settings::beckmanBug( void )
{
  US_SettingsStore settings;
  return settings.value( "beckmanBug", false ).toBool(); // No default
}

void US_Settings::set_beckmanBug( bool setBug )
{
  US_SettingsStore settings;
  if ( ! setBug )
    settings.remove( "beckmanBug" );
  else
    settings.setValue( "beckmanBug", true );
}

// Default data location  1 = DB, 2 = Disk
int US_Settings::default_data_location( void )
{
  US_SettingsStore settings;
  return settings.value( "dataLocation", 2 ).toInt(); 
}

void US_Settings::set_default_data_location( int location )
{
  US_SettingsStore settings;
  if ( location == 2 )
    settings.remove( "dataLocation" );
  else
    settings.setValue( "dataLocation", location );
}

// us_debug
#ifndef NO_DB
int US_Settings::us_debug( void )
{
   US_SettingsStore settings;
   return settings.value( "us_debug", 0 ).toInt();
}

void US_Settings::set_us_debug( int level )
{
   US_SettingsStore settings;
   if ( level == 0 )
      settings.remove( "us_debug" );
   else
      settings.setValue( "us_debug", level );
}
#else
static int us_settings_debug = 0;
int US_Settings::us_debug( void )
{
   return us_settings_debug;
}

void US_Settings::set_us_debug( int level )
{
   us_settings_debug = level;
}
#endif

// debug text
#ifndef NO_DB
void US_Settings::set_debug_text( QStringList debuglist )
{
   US_SettingsStore settings;
   if ( debuglist.count() == 0 )
      settings.remove( "debug_text" );
   else
      settings.setValue( "debug_text", debuglist );
}

QStringList US_Settings::debug_text( void )
{
   US_SettingsStore settings;
   return settings.value( "debug_text", "" ).toStringList();
}

#else
static QStringList us_sett_debug_text;
void US_Settings::set_debug_text( QStringList debuglist )
{
   if ( debuglist.count() == 0 )
      us_sett_debug_text.clear();
   else
      us_sett_debug_text  = debuglist;
}

QStringList US_Settings::debug_text( void )
{
   return us_sett_debug_text;
}
#endif

bool US_Settings::debug_match( QString match )
{
   return debug_text().contains( match, Qt::CaseInsensitive );
}

QString US_Settings::debug_value( QString match )
{
   QStringList dbgtxt = debug_text();
   QString dbgval( "" );

   for ( int ii = 0; ii < dbgtxt.count(); ii++ )
   {
      if ( dbgtxt[ ii ].startsWith( match, Qt::CaseInsensitive ) )
      {
         dbgval        = QString( dbgtxt[ ii ] ).section( "=", 1, 1 );
         break;
      }
   }

   return dbgval;
}

// Investigator
QString US_Settings::us_inv_name( void )
{
   US_SettingsStore settings;
   return settings.value( "us_investigator_name", "Not Available" ).toString();
}

void US_Settings::set_us_inv_name( const QString& name )
{
  US_SettingsStore settings;
  if ( name == "" )
    settings.remove( "us_investigator_name" );
  else
    settings.setValue( "us_investigator_name", name );
}

int US_Settings::us_inv_ID( void )
{
   US_SettingsStore settings;
   return settings.value( "us_investigator_ID", -1 ).toInt();
}

void US_Settings::set_us_inv_ID( int id )
{
  US_SettingsStore settings;
  if ( id == -1 )
    settings.remove( "us_investigator_ID" );
  else
    settings.setValue( "us_investigator_ID", id );
}

int US_Settings::us_inv_level( void )
{
   US_SettingsStore settings;
   return settings.value( "us_investigator_level", 0 ).toInt();
}

void US_Settings::set_us_inv_level( int level )
{
  US_SettingsStore settings;
  if ( level == 0 )
    settings.remove( "us_investigator_level" );
  else
    settings.setValue( "us_investigator_level", level );
}

// advanced level
int US_Settings::advanced_level( void )
{
  US_SettingsStore settings;
  return settings.value( "advanced_level", 0 ).toInt();
}

void US_Settings::set_advanced_level( int level )
{
  US_SettingsStore settings;
  if ( level == 0 )
    settings.remove( "advanced_level" );
  else
    settings.setValue( "advanced_level", level );
}

// Threads
int US_Settings::threads( void )
{
  US_SettingsStore settings;
  return settings.value( "threads", 1 ).toInt(); // No default
}

void US_Settings::set_threads( int threads )
{
  US_SettingsStore settings;
  if ( threads == 1 )
    settings.remove( "threads" );
  else
    settings.setValue( "threads", threads );
}

// Noise Dialog:  0 -> Auto, 1 -> Dialog
int US_Settings::noise_dialog( void )
{
  US_SettingsStore settings;
  return settings.value( "noise_dialog", 0 ).toInt();
}

void US_Settings::set_noise_dialog( int diagflag )
{
  US_SettingsStore settings;
  if ( diagflag == 0 )
    settings.remove( "noise_dialog" );
  else
    settings.setValue( "noise_dialog", diagflag );
}

// Database Entries

QList<QStringList> US_Settings::databases( void )
{
  US_SettingsStore settings;
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
  US_SettingsStore settings;

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
  US_SettingsStore settings;
  return settings.value( "defaultDB", QStringList() ).toStringList();
}

void US_Settings::set_defaultDB( const QStringList& defaultDB )
{
  US_SettingsStore settings;
  if ( defaultDB.isEmpty() )
    settings.remove( "defaultDB" );
  else
    settings.setValue( "defaultDB", defaultDB );
}

// Optima Database Host Entries
QList<QStringList> US_Settings::xpn_db_hosts( void )
{
  US_SettingsStore settings;
  QList<QStringList> xhlist; 
  int xhCount = settings.value( "xhCount", 0 ).toInt();

  for ( int ii = 0; ii < xhCount; ii++ )
  {
    QString key = QString( "xhlist%1" ).arg( ii );
    xhlist << settings.value( key ).toStringList();
  }

  return xhlist;
}

void US_Settings::set_xpn_db_hosts( const QList<QStringList>& xhlist )
{
  US_SettingsStore settings;

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

QStringList US_Settings::defaultXpnHost( void )
{
  US_SettingsStore settings;
  return settings.value( "defXpnHost", QStringList() ).toStringList();
}

void US_Settings::set_def_xpn_host( const QStringList& defXpnHost )
{
  US_SettingsStore settings;
  if ( defXpnHost.isEmpty() )
    settings.remove( "defXpnHost" );
  else
    settings.setValue( "defXpnHost", defXpnHost );
}


/***************** DA status related **************************/
void US_Settings::set_DA_status( const QString& status )
{
  US_SettingsStore settings;

  QStringList status_list = status.split(":");
  
  if ( status_list[0] == "COM" ) 
    settings.setValue( "daComOpened", status_list[1] );
  if ( status_list[0] == "ACAD" ) 
    settings.setValue( "daAcadOpened", status_list[1] );
}

bool US_Settings::get_DA_status( const QString& da_type )
{
  US_SettingsStore settings;

  int status = 0;
  if ( da_type == "COM" )
    status = settings.value( "daComOpened", QString() ).toInt();

  if ( da_type == "ACAD" )
    status = settings.value( "daAcadOpened", QString() ).toInt();

  if ( status )
    return true;
  else
    return false;
      
}
/*****************************************************************/

QString US_Settings::status()
{
  US_SettingsStore settings;
  settings.setValue( "status_test", true );
  settings.sync();
  settings.remove( "status_test" );
  
  switch ( settings.status() ) {
  case QSettings::NoError :
     return "";
     break;
  case QSettings::AccessError :
     return QString( "Access error. Check permissions and ownership of %1" ).arg( settings.fileName() );
     break;
  case QSettings::FormatError :
     return QString( "Settings format error. The file %1 is garbled" ).arg( settings.fileName() );
     break;
  default:
     return QString( "Unknown settings error %1. Perhaps remove the file %2 and try again." ).arg( settings.status() ).arg( settings.fileName() );
     break;
  }
}
     
     
