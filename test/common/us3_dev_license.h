//! \file us3_dev_license.h
#ifndef US3_DEV_LICENSE_H
#define US3_DEV_LICENSE_H

#include <QDate>
#include <QSettings>
#include <QString>
#include <QStringList>

#include "us_defines.h"

// Writes a locally-valid UltraScan license registration under homeDir,
// matching the validation algorithm in US_License_t::isValid() (see
// utils/us_license_t.cpp:84-90). A headless-CLI gui test that starts a real
// us3 program with HOME pointed at homeDir uses this to skip that program's
// interactive registration dialog, which otherwise blocks indefinitely.
//
// Writes the ini file directly at its native UserScope path instead of
// going through QSettings(format, scope, org, app): US3_GUI_TEST_MAIN
// redirects this process's own IniFormat/UserScope settings into a sandbox
// (see us3_gui_test_main.h), which would otherwise hijack this write away
// from the child process's HOME.
inline void registerDevLicenseFor( const QString& homeDir )
{
   const QString expiration = QDate::currentDate().addDays( 365 ).toString( Qt::TextDate );
   const QString email      = QStringLiteral( "dev@localhost" );

   // Mirrors US_License_t::isValid()'s own calculation exactly, so it
   // accepts what it is given here.
   const QString licenseString = expiration + email;
   QString       validation;
   for ( const QChar& c : licenseString )
   {
      validation += QString::asprintf( "%X", c.unicode() );
   }
   validation.truncate( 24 );

   const QStringList license = {
      QStringLiteral( "Dev" ),
      QStringLiteral( "Developer" ),
      QStringLiteral( "Dev Build" ),
      QStringLiteral( "n/a" ),
      QStringLiteral( "n/a" ),
      QStringLiteral( "NON-US" ),
      QStringLiteral( "n/a" ),
      QStringLiteral( "n/a" ),
      email,
      QStringLiteral( PLATFORM ),
      QStringLiteral( OS ),
      QStringLiteral( "4.1.0-dev" ),
      validation,
      expiration,
   };

   QSettings settings( homeDir + "/.config/UltraScan3/UltraScan.conf", QSettings::IniFormat );
   settings.setValue( "license", license );
   settings.sync();
}

#endif // US3_DEV_LICENSE_H
