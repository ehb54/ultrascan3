//! \file us_license_t.cpp
#include <QtCore>

#include "us_license_t.h"
#include "us_settings.h"
#include "us_defines.h"


int US_License_t::isValid( QString& ErrorMessage, const QStringList& newLicense )
{
  QStringList license;

  if ( newLicense.size() == 0 )
    license = US_Settings::license();
  else 
    license = newLicense;
  
  if ( license.size() < 12 )
  {
    ErrorMessage =  QString( qApp->translate( "UltraScan", 
        "You have not yet registered your copy of UltraScan.\n"
        "Click on 'Register' to register UltraScan\n"
        "for the %1 platform and %2 Operating System." ) )
        .arg( TITLE, OS_TITLE );
    
    return Missing;
  }

  QString lastname   = license.value(  0 );
  QString firstname  = license.value(  1 );
  QString company    = license.value(  2 );
  QString address    = license.value(  3 );
  QString city       = license.value(  4 );
  QString state      = license.value(  5 );
  QString zip        = license.value(  6 );
  QString phone      = license.value(  7 );
  QString email      = license.value(  8 );
  QString platform   = license.value(  9 );
  QString opSys      = license.value( 10 );
  QString version    = license.value( 11 );

  QString validation = "";
  QString expiration = "";

  if ( license.size() > 13 )
  {
    validation = license.value( 12 );
    expiration = license.value( 13 );
  }

  if ( platform != PLATFORM ) 
  {
    ErrorMessage =  QString( qApp->translate( "UltraScan", 
        "You are running UltraScan on a %1 platform,\n"
        "but your registration is issued for the %2 platform\n\n" 
        "You will have to update your registration before\n"
        "proceeding. Click on 'Register' to start the process\n"
        "for the %3 platform." ) )
        .arg( TITLE, platform, TITLE );

    return BadPlatform;
  }

  if ( opSys != OS ) 
  {
    ErrorMessage = QString( qApp->translate( "UltraScan", 
          "You are running UltraScan with a %1 operating system,\n"
          "but your registration is issued for the %2 operating system\n\n"
          "You will have to update your registration  before\n"
          "proceeding. Click on 'Register' to start the process\n"
          "for a %3 operating system." ) )
          .arg( OS_TITLE, opSys, OS_TITLE );

    return BadOS;
  }

  if ( license.size() == 12 )
  {
    ErrorMessage = QString( qApp->translate( "UltraScan", 
             "Your UltraScan resistration is pending.\n"
             "Please respond to the email sent to complete the process." ) );

    return Pending;
  }


  QString code1 = encode( lastname, firstname );
  QString code2 = encode( company , address   );
  QString code3 = encode( city    , zip       );
  QString code4 = encode( phone   , email     );
  QString code5 = encode( platform + opSys + version, expiration );

  QString calculation = code1 + "-" + code2 + "-" + code3 + "-" + code4 + "-" + code5;

  if ( calculation != validation )
  { 
    ErrorMessage = qApp->translate( "UltraScan", 
           "The registration is invalid.\n"
           "You will have to update your registration before\n"
           "proceeding. Click on 'Register' to begin the registration\n" )
            + "\n" + validation + "\n" + calculation;

    return Invalid;
  }

  if ( QDate::currentDate() > QDate::fromString( expiration ) )
  {
    ErrorMessage = qApp->translate( "UltraScan", 
           "The registration is expired.\n"
           "You will have to update your registration before\n"
           "proceeding. Click on 'Update / Renew' to continue.\n" );

    return Expired;
  }

  return OK;
}

QString US_License_t::encode( const QString& str1, const QString& str2 )
{
  int sum1 = 0;
  int sum2 = 0;

  const static QString lower = "abcdefghijklmnopqrstuvwxyz";
  const static QString upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  QRegExp blanks( "(^\\s+)|(\\s+$)" );

  QString STR1 = str1;
  STR1.replace( blanks, "" );
  
  QString STR2 = str2;
  STR2.replace( blanks, "" );

  for ( int i = 0; i < STR1.length(); i++ )
  {
    QChar c = STR1.at( i );

    // Make upper case for ascii only
    int   index = lower.indexOf( c );
    if ( index >= 0 ) 
       c = upper.at( index );

    sum1 += c.unicode();
  }

  for ( int i = 0; i < STR2.length(); i++ )
  {
    QChar c = STR2.at( i );

    // Make upper case for ascii only
    int   index = lower.indexOf( c );
    if ( index >= 0 ) 
       c = upper.at( index );

    sum2 += c.unicode();
  }

  QString SUM = QString::number( sum1 ) + QString::number( sum2 );
                
  int x = int( fabs( sin( SUM.toDouble() ) ) * 65535 );

  QString code;
  code = code.sprintf( "000%X", x ).right( 4 );

  return code;
}
