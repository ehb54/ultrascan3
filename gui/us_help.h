//! \file us_help.h
#ifndef US_HELP_H
#define US_HELP_H

#include <QtCore>
#if QT_VERSION > 0x050000
#include <QtWidgets>
#else
#include <QtGui>
#define setSingleStep(a) setStep(a)
#define setMinorPen(a) setMinPen(a)
#define setMajorPen(a) setMajPen(a)
#endif
#include "us_extern.h"

//! \brief Launch help programs

/*! \class US_Help

  Provides functions to launch a web browser for external links or
  Qt Assistant for local help files.
*/

class US_GUI_EXTERN US_Help : public QObject
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

    /*! \brief Show URL page in the configured browser for US3
        \param location URL to open in the configured browser
    */
    void show_URL(  const QString& );

    /*! \brief Show HTML file in the configured browser for US3
        \param location HTML file to open in the configured browser
    */
    void show_html_file(  const QString& );

    /*! \brief Register the QCH file path
    */
    void register_init( void );

  private:
    // Only needed because we don't want it to be deleted when 
    // show_help() returns.
    QProcess* assistant;

    void openBrowser( const QString& );
};
#endif
