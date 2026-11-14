//! \file us_helpdaemon.h
#ifndef US_HELPDAEMON_H
#define US_HELPDAEMON_H

#include <QtCore>

//! \brief Launch help programs

/*! \class US_HelpDaemon

  Provides functions to launch a web browser for external links or
  Qt Assistant for local help files.
*/

class US_HelpDaemon : public QObject
{
  Q_OBJECT

  public:
    /*! \brief Launch Qt Assistant and set the page to show.
     
        \param page  The page to have assistant show.
        \param o     A pointer to the parent widget.  This normally can be
        left as NULL.
    */
    US_HelpDaemon( const QString&, QObject* = 0 );

  public slots:
    //! \brief Sends the web page to be shown to Qt's Assistant.
    //! \param helpPage A QString of the page to show.
    void show ( const QString& );

    //! \brief A slot to catch Assistant exiting.  A normal exit ends the
    //!        daemon; an error exit is reported to the user first.
    void close( int, QProcess::ExitStatus );

  private:
     //! \brief Report a fatal help problem to the user and exit.
     [[noreturn]] static void fail( const QString& );
     static void              debug( const QString& );
     QProcess                 daemon;
};

#endif
