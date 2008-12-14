#ifndef US_HELP_H
#define US_HELP_H

#include <QtGui>

//! \brief Launch help programs

/*! \class US_Help

  Provides functions to launch a web browser for external links or
  Qt Assistant for local help files.
*/

class US_Help : public QWidget
{
  Q_OBJECT

  public:

    /*! \brief Allocate QProcess structures and connect browser process to
               methods to capture any output.

        \param parent A pointer to the parent widget.  This normally can be 
                      left as NULL.
    */  
    US_Help( QWidget* = 0 );

    /*! \brief A null destructor.
    */
      ~US_Help() {};

    /*! \brief  Show a web page in a browseer.
        \param  location URL of page to be shown
    */   
    void show_URL      ( const QString& );

    /*! \brief  Show a local help page with Qt Assistant.

        This method works through the external program us_helpdaemon.  
        The daemon stays in memory as long as assistant is open and
        has a mechanism to not create multiple instances.  The daemon 
        takes the input page reference and passes it to assistant.

        \param  helpFile The page to be shown
    */   
    void show_help     ( const QString& );

  private:
    QProcess* proc;
    QProcess* assistant;
    QString   URL;
    int       stderrSize;
    int       trials;

    void openBrowser();

  private slots:
    void captureStdout( void );
    void captureStderr( void );
    void endProcess( int, QProcess::ExitStatus );
};
#endif

