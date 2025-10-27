//! \file us_xpnhost.h
#ifndef US_XPNHOST_H
#define US_XPNHOST_H

#include <QtCore>

#include "us_widgets.h"

/*! \brief A class to allow the user to set, modify or delete
    parameters for database access.
*/
class US_XpnHost : public US_Widgets
{
  Q_OBJECT
  
  public:
    
    //!  Construct the window to manage database parameters.
    US_XpnHost( QWidget* w = nullptr, Qt::WindowFlags flags = Qt::WindowFlags() );
    
  private:
    QList<QStringList> dblist;
    QString            uuid;
    QMap< QString, QString > conn_stat;
    
    QPushButton* pb_add;   
    QPushButton* pb_delete;
    QPushButton* pb_save;
    QPushButton* pb_reset; 
    QPushButton* pb_testConnect;
    
    QLineEdit*   le_description;
    QLineEdit*   le_dbname; 
    QLineEdit*   le_host;  
    QLineEdit*   le_port;  
    QLineEdit*   le_name;  
    QLineEdit*   le_user;  
    QLineEdit*   le_pasw;  

    QComboBox*   cb_os1;
    QComboBox*   cb_os2;
    QComboBox*   cb_os3;

    QListWidget* lw_entries;

    void update_lw( const QString& = 0 );
    
  private slots:
    void help        ( void );
    void select_db   ( QListWidgetItem*, const bool = true );
    void check_add   ( void );
    void reset       ( void );
    void save_default( void );
    void deleteDB    ( void );
    bool test_connect( void );
};
#endif
