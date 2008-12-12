#ifndef US_SETTINGS_H
#define US_SETTINGS_H

#include <QtCore>

class US_Settings
{
  public:
    US_Settings (){};
    ~US_Settings(){};

    // Programs
    static QString     browser( void );
    static void        set_browser( const QString& );

    // Directories
    static QString     usHomeDir( void );
    static void        set_usHomeDir( const QString& );

    static QString     dataDir( void );
    static void        set_dataDir( const QString& );

    static QString     resultDir( void );
    static void        set_resultDir( const QString& );

    static QString     reportDir( void );
    static void        set_reportDir( const QString& );

    static QString     helpDir( void );
    static void        set_helpDir( const QString& );

    static QString     archiveDir( void );
    static void        set_archiveDir( const QString& );

    static QString     tmpDir( void );
    static void        set_tmpDir( const QString& );

    // License
    static QStringList license( void );
    static void        set_license( const QStringList& );

    // Master Password
    static QByteArray  UltraScanPW( void );
    static void        set_UltraScanPW( const QByteArray& );

    // Misc
    static double      tempTolerance( void );
    static void        set_tempTolerance( double );

    static bool        beckmanBug( void );
    static void        set_beckmanBug( bool );

    static int         threads( void );
    static void        set_threads( int );

    static QList<QStringList> databases( void );
    static void        set_databases( const QList<QStringList>& );

    static QStringList defaultDB( void );
    static void        set_defaultDB( const QStringList& );
};
#endif
