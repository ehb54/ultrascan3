//! \file us_license_t.h
#ifndef US_LICENSE_T_H
#define US_LICENSE_T_H

#include "us_global.h"

class US_License_t
{
  public:
    US_License_t(){};
    ~US_License_t(){};

    enum { OK, Expired, Invalid, Missing, BadPlatform, BadOS };

    static int isValid( QString&, const QStringList& = QStringList() );

  private:

    static QString encode( const QString&, const QString& );
};

#endif
