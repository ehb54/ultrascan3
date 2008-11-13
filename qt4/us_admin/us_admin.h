#ifndef US_ADMIN_H
#define US_ADMIN_H

#include <QtGui>

#include "us_widgets.h"
#include "us_global.h"

class US_Admin : public US_Widgets
{
  Q_OBJECT
  
  public:
    US_Admin( QWidget* = 0, Qt::WindowFlags = 0 );
    ~US_Admin() {};
    
  private:
    QLabel*      lbl_header;    
    QLabel*      lbl_psswd1; 
    QLabel*      lbl_psswd2;
    QLineEdit*   le_psswd1;
    QLineEdit*   le_psswd2;
    QPushButton* pb_help;
    QPushButton* pb_save;
    QPushButton* pb_cancel;
    QString      password1;    
    QString      password2;    
    
    US_Global*   g;

  private slots:
    void update_psswd1( const QString& );
    void update_psswd2( const QString& );
    void closeEvent   ( QCloseEvent* );
    void help();
    void save();
    void quit();
};

#endif


