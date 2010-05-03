//! \file us_settings.h
#ifndef US_SETTINGS_H
#define US_SETTINGS_H

#include <QtCore>

#include "us_extern.h"

//! \brief Fetch and set values via QSettings.  All functions are static.
class US_EXTERN US_Settings
{
  public:
    //! \brief Null constructor
    US_Settings () {};
    //! \brief Null destructor
    ~US_Settings() {};

    // Programs
    
    //! \brief Get the browser program
    static QString     browser( void );
    //! \brief Set the browser program.  The path is only necessary if it  
    //! is not found via the PATH envrionment variable.
    static void        set_browser( const QString& );

    // Directories

    //! \brief Get the user's stored home directory
    static QString     usHomeDir( void );
    //! \brief Set the user's stored home directory
    static void        set_usHomeDir( const QString& );

    //! \brief Get the directory for raw data
    static QString     dataDir( void );
    //! \brief Set the directory for raw data
    static void        set_dataDir( const QString& );

    //! \brief Get the directory for result data
    static QString     resultDir( void );
    //! \brief Set the directory for result data
    static void        set_resultDir( const QString& );

    //! \brief Get the directory for reports from the supercomputer
    static QString     reportDir( void );
    //! \brief Set the directory for reports from the supercomputer
    static void        set_reportDir( const QString& );

    //! \brief Get the directory for help files
    //! \note This function is deprecated and not used
    static QString     helpDir( void );
    //! \brief Set the directory for help files
    //! \note This function is deprecated and not used
    static void        set_helpDir( const QString& );

    //! \brief Get the directory where data and reports are archived locally
    static QString     archiveDir( void );
    //! \brief Set the directory where data and reports are archived locally
    static void        set_archiveDir( const QString& );

    //! \brief Get the directory where temporary files can be stored
    static QString     tmpDir( void );
    //! \brief Set the directory where temporary files can be stored
    static void        set_tmpDir( const QString& );

    //! \brief Get the base directory in which ./bin apps reside
    static QString     appBaseDir();

    // License
    
    //! \brief Get the current license
    static QStringList license( void );
    //! \brief Set the current license
    static void        set_license( const QStringList& );

    // Master Password
    
    //! \brief Get the cryptographic hash of the master password
    static QByteArray  UltraScanPW( void );
    //! \brief Set the cryptographic hash of the master password
    static void        set_UltraScanPW( const QByteArray& );

    // Misc
    
    //! \brief Get the temperature tolerance for runs
    static double      tempTolerance( void );
    //! \brief Set the temperature tolerance for runs
    static void        set_tempTolerance( double );

    //! \brief Get the Beckman Bug flag
    static bool        beckmanBug( void );
    //! \brief Set the Beckman Bug flag
    static void        set_beckmanBug( bool );

    //! \brief Get the US_Debug setting
    static int         us_debug( void );
    //! \brief Set the US_Debug setting
    static void        set_us_debug( int );

    //! \brief Get the number of threads to use for internal procesing
    static int         threads( void );
    //! \brief Set the number of threads to use for internal procesing.
    //!        This is normally the number of processors or cores in 
    //!        the local computer.
    static void        set_threads( int );

    // Database info

    //! \brief Get a list of stored database connection descriptions
    static QList<QStringList> databases( void );
    //! \brief Set a list of stored database connection descriptions
    static void        set_databases( const QList<QStringList>& );

    //! \brief Get the current stored database connection description
    static QStringList defaultDB( void );
    //! \brief Set the current stored database connection description
    static void        set_defaultDB( const QStringList& );
};
#endif
