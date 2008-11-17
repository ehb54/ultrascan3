#include "us_settings.h"

#define BASE "/home/bdubbs/ultrascan"

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

QString US_Settings::helpDir( void )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  return settings.value( "helpDir",  BASE + QString( "/doc" ) ).toString();
}

void US_Settings::set_helpDir( const QString& dir )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  settings.setValue( "helpDir", dir );
}

QStringList US_Settings::license( void )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  return settings.value( "license", "" ).toStringList();
}

void US_Settings::set_license( const QString& license )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  settings.setValue( "license", license );
}

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
