#include "us_license_t.h"
#include "us_settings.h"

//! \file us_license_t.cpp

int US_License_t::isValid( QString& ErrorMessage )
{
  QStringList license = US_Settings::license();
  
  if ( license.value( 0 ) == "" )
  {
    ErrorMessage =  QString( qApp->translate( "UltraScan", 
        "You have not yet installed a valid UltraScan license.\n"
        "Click on 'Register' to obtain an UltraScan License\n"
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
  QString validation = license.value( 12 );
  QString expiration = license.value( 13 );

  if ( platform != PLATFORM ) 
  {
    ErrorMessage =  QString( qApp->translate( "UltraScan", 
        "You are running UltraScan on a %1 platform,\n"
        "but your license is issued for the %2 platform\n\n" 
        "You will have to update your license file before\n"
        "proceeding. Click on 'Register' to obtain an\n"
        "UltraScan License for the %3 platform." ) )
        .arg( TITLE, platform, TITLE );

    return BadPlatform;
  }

  if ( opSys != OS ) 
  {
    ErrorMessage = QString( qApp->translate( "UltraScan", 
          "You are running UltraScan with a %1 operating system,\n"
          "but your license is issued for the %2 operating system\n\n"
          "You will have to update your license file before\n"
          "proceeding. Click on 'Register' to obtain an\n"
          "UltraScan License for a %3 operating system." ) )
          .arg( OS_TITLE, opSys, OS_TITLE );

    return BadOS;
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
           "The license in your home directory is invalid.\n"
           "You will have to update your license file before\n"
           "proceeding. Click on 'Register' to obtain a new\n"
           "UltraScan License" );

    return Invalid;
  }

  if ( QDate::currentDate() > QDate::fromString( expiration ) )
  {
    ErrorMessage = qApp->translate( "UltraScan", 
           "The license in your home directory is expired.\n"
           "You will have to update your license file before\n"
           "proceeding. Click on 'Register' to obtain a new\n"
           "UltraScan License" );

    return Expired;
  }

  return OK;
}

QString US_License_t::encode( const QString& str1, const QString& str2 )
{
  int sum1 = 0;
  int sum2 = 0;

  QString STR1 = str1.toUpper();
  QString STR2 = str2.toUpper();

  for ( int i = 0; i < STR1.length(); i++ )
  {
    QChar c  = STR1.at( i );
    sum1    += c.unicode();
  }

  QString SUM1 = QString::number( sum1 );

  for ( int i = 0; i < STR2.length(); i++ )
  {
    QChar c  = STR2.at( i );
    sum2    += c.unicode();
  }

  QString SUM = QString::number( sum1 ) + QString::number( sum2 );
                
  int x = int( fabs( sin( SUM.toFloat() ) ) * 65535 );

  QString code;

  return code.sprintf( "000%X", x ).right( 4 );
}
