#include "us_settings.h"
#include "us_defines.h"
#include <QCoreApplication>
#include <QRegularExpression>
#include <QSettings>
#include <QDir>

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

// Work base directory (where archive,results,reports,etc hang)
QString US_Settings::workBaseDir( void )
{
   QSettings settings( US3, "UltraScan" );
   return settings.value( "workBaseDir",
                          QDir::homePath() + "/ultrascan" ).toString();
}

void US_Settings::set_workBaseDir( const QString& dir )
{
  QSettings settings( US3, "UltraScan" );
  if ( dir ==  QDir::homePath() + "/ultrascan" )
    settings.remove( "workBaseDir" );
  else
    settings.setValue( "workBaseDir", dir );
}

// importDir
QString US_Settings::importDir( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "importDir", workBaseDir() + "/imports" ).toString();
}

void US_Settings::set_importDir( const QString& dir )
{
  QSettings settings( US3, "UltraScan" );
  if ( dir == workBaseDir() + "/imports" )
    settings.remove( "importDir" );
  else
    settings.setValue( "importDir", dir );
}

// tmpDir
QString US_Settings::tmpDir( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "tmpDir", workBaseDir() + "/tmp" ).toString();
}

void US_Settings::set_tmpDir( const QString& dir )
{
  QSettings settings( US3, "UltraScan" );
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
#ifndef NO_DB
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
   QSettings settings( US3, "UltraScan" );
   if ( debuglist.count() == 0 )
      settings.remove( "debug_text" );
   else
      settings.setValue( "debug_text", debuglist );
}

QStringList US_Settings::debug_text( void )
{
   QSettings settings( US3, "UltraScan" );
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

// Noise Dialog:  0 -> Auto, 1 -> Dialog
int US_Settings::noise_dialog( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "noise_dialog", 0 ).toInt();
}

void US_Settings::set_noise_dialog( int diagflag )
{
  QSettings settings( US3, "UltraScan" );
  if ( diagflag == 0 )
    settings.remove( "noise_dialog" );
  else
    settings.setValue( "noise_dialog", diagflag );
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

// Optima Database Host Entries
QList<QStringList> US_Settings::xpn_db_hosts( void )
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

void US_Settings::set_xpn_db_hosts( const QList<QStringList>& xhlist )
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

QStringList US_Settings::defaultXpnHost( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "defXpnHost", QStringList() ).toStringList();
}

void US_Settings::set_def_xpn_host( const QStringList& defXpnHost )
{
  QSettings settings( US3, "UltraScan" );
  if ( defXpnHost.isEmpty() )
    settings.remove( "defXpnHost" );
  else
    settings.setValue( "defXpnHost", defXpnHost );
}


/***************** DA status related **************************/
void US_Settings::set_DA_status( const QString& status )
{
  QSettings settings( US3, "UltraScan" );

  QStringList status_list = status.split(":");
  
  if ( status_list[0] == "COM" ) 
    settings.setValue( "daComOpened", status_list[1] );
  if ( status_list[0] == "ACAD" ) 
    settings.setValue( "daAcadOpened", status_list[1] );
}

bool US_Settings::get_DA_status( const QString& da_type )
{
  QSettings settings( US3, "UltraScan" );

  int status; 
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
  QSettings settings( US3, "UltraScan" );
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
     
     
