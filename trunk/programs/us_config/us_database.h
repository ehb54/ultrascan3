//! \file us_database.h
#ifndef US_DATABASE_H
#define US_DATABASE_H

#include <QtCore>

#include "us_widgets.h"

/*! \brief A class to allow the user to set, modify or delete
    parameters for database access.
*/
class US_Database : public US_Widgets
{
  Q_OBJECT
  
  public:
    
    //!  Construct the window to manage database parameters.
    US_Database( QWidget* w = 0, Qt::WindowFlags flags = 0 );
    
  private:
    QList<QStringList> dblist;
    
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
    QLineEdit*   le_investigator_email;  
    QLineEdit*   le_investigator_pw;  

    QListWidget* lw_entries;

    void update_lw( const QString& = 0 );
    
  private slots:
    void help        ( void );
    void select_db   ( QListWidgetItem* );
    void check_add   ( void );
    void reset       ( void );
    void save_default( void );
    void deleteDB    ( void );
    void test_connect( void );
};
#endif
