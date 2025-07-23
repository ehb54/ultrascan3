//! \file us_passwd.h
#ifndef US_PASSWD_H
#define US_PASSWD_H

#include "us_widgets.h"
#include "us_extern.h"

//!  A class to get the user's master password.
class US_GUI_EXTERN US_Passwd : public US_Widgets
{
  Q_OBJECT

  public:
    //! A null constructor.
    US_Passwd( QWidget* parent = 0, Qt::WindowFlags f = 0 ) 
      : US_Widgets( true, parent, f ) {};
    
    //! A null destructor. 
    ~US_Passwd() {};

    //!  If it is in global memory, return it.  Otherwise ask the user and
    //!  save it to global memory.
    //!  \retval password The unecrypted master password.
    QString getPasswd( void );
    QMap < QString, QString > getPasswd_auditTrail( QString, QString, QString );
};
#endif

