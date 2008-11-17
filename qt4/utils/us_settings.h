#ifndef US_SETTINGS_H
#define US_SETTINGS_H

#include <QtCore>

class US_Settings
{
  public:
    US_Settings (){};
    ~US_Settings(){};

    static QString     browser( void );
    static void        set_browser( const QString& );

    static QString     helpDir( void );
    static void        set_helpDir( const QString& );

    static QStringList license( void );
    static void        set_license( const QString& );

    static QByteArray  UltraScanPW( void );
    static void        set_UltraScanPW( const QByteArray& );
};

#endif
