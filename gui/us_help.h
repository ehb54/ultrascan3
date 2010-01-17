#ifndef US_HELP_H
#define US_HELP_H

#include <QtCore>
#include "us_extern.h"

//! \brief Launch help programs

/*! \class US_Help

  Provides functions to launch a web browser for external links or
  Qt Assistant for local help files.
*/

// Probably could jsut be QObject
class US_EXTERN US_Help : public QObject
{
  Q_OBJECT

  public:
    /*! \brief Allocate QProcess structures and connect browser process to
               methods to capture any output.

        \param parent A pointer to the parent widget.  This normally can be 
                      left as NULL.
    */  
    US_Help( QObject* = 0 );

    void show_help( const QString& );

  private:
    // Only needed because we don't want it to be deleted when 
    // show_help() returns.
    QProcess* assistant;
};
#endif

