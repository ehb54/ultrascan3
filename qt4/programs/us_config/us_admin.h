//! \file us_admin.h
#ifndef US_ADMIN_H
#define US_ADMIN_H

#include <QtGui>

#include "us_widgets.h"

class US_Admin : public US_Widgets
{
  Q_OBJECT
  
  public:
    US_Admin( QWidget* = 0, Qt::WindowFlags = 0 );
    ~US_Admin() {};
    
  private:
    QLineEdit*   le_oldPasswd;
    QLineEdit*   le_passwd1;
    QLineEdit*   le_passwd2;

    QPushButton* pb_help;
    QPushButton* pb_save;
    QPushButton* pb_cancel;
    
    QString      password1;    
    QString      password2;    
    
  private slots:
    void help();
    void save();
};

#endif


