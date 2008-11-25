//! \file us_passwd.h
#ifndef US_PASSWD_H
#define US_PASSWD_H

#include <QtGui>

class US_Passwd : public QWidget
{
  Q_OBJECT

  public:
    US_Passwd( QWidget* parent = 0 ) : QWidget( parent ) {};
    ~US_Passwd() {};

    QString getPasswd( void );
};
#endif

