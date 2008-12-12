//! \file us_crypto.h
#ifndef US_CRYPTO_H
#define US_CRYPTO_H

#include <QtCore>

class US_Crypto
{
  public:
    US_Crypto() {};
    ~US_Crypto(){};

    static QStringList encrypt( const QString& );
    static QString     decrypt( const QString&, const QString& );
};

#endif
