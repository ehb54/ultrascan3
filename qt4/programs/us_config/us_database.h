//! \file us_database.h
#ifndef US_DATABASE_H
#define US_DATABASE_H

#include <QtCore>

#include "us_widgets.h"
#include "us_global.h"

class US_Database : public US_Widgets
{
  Q_OBJECT
  
  public:
    US_Database( QWidget* w = 0, Qt::WindowFlags flags = 0 );
    ~US_Database();
    
  private:
    QList<QStringList> dblist;
    
    US_Global    g;

    QPushButton* pb_add;   
    QPushButton* pb_delete;
    QPushButton* pb_save;
    QPushButton* pb_reset; 
    QPushButton* pb_testConnect;
    
    QLineEdit*   le_description;
    QLineEdit*   le_username; 
    QLineEdit*   le_password;
    QLineEdit*   le_dbname; 
    QLineEdit*   le_host;  

    QListWidget* lw_entries;

    void update_lw( const QString& = 0 );
    
  private slots:
    void help        ( void );
    void select_db   ( void );
    void check_add   ( void );
    void reset       ( void );
    void save_default( void );
    //void delete      ( void );
    //void test_connect( void );
};
#endif
