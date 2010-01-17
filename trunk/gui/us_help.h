#ifndef US_HELP_H
#define US_HELP_H

#include <QtCore>
#include "us_extern.h"

//! \brief Launch help programs

/*! \class US_Help

  Provides functions to launch a web browser for external links or
  Qt Assistant for local help files.
*/

class US_EXTERN US_Help : public QObject
{
  Q_OBJECT

  public:
    /*! \brief Simple constructor.
        \param parent A pointer to the parent QObject.  This normally can be 
                      left as NULL.
    */  
    US_Help( QObject* = 0 );

    /*! \brief Show help page in Qt assistant window cusomized for US3
        \param helpFile Name of help page to be shown. For example,
               "us_config.html".  Optionally prepended with "manual/"
    */
    void show_help( const QString& );

  private:
    // Only needed because we don't want it to be deleted when 
    // show_help() returns.
    QProcess* assistant;
};
#endif
