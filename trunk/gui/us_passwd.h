//! \file us_passwd.h
#ifndef US_PASSWD_H
#define US_PASSWD_H

#include <QtGui>

#include "us_widgets.h"

class US_Passwd : public US_Widgets
{
  Q_OBJECT

  public:
    US_Passwd( QWidget* parent = 0, Qt::WindowFlags f = 0 ) 
      : US_Widgets( parent, f ) {};
    
    ~US_Passwd() {};

    QString getPasswd( void );
};
#endif

