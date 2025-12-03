//! \file us_xpnhost.h
#ifndef US_XPNHOSTDB_H
#define US_XPNHOSTDB_H

#include <QtCore>

#include "us_widgets.h"
#include "us_newxpnhost_db.h"
#include "us_db2.h"

/*! \brief A class to allow the user to set, modify or delete
    parameters for database access.
*/
class US_XpnHostDB : public US_Widgets
{
  Q_OBJECT
  
  public:
    
    //!  Construct the window to manage database parameters.
    US_XpnHostDB( QWidget* w = nullptr, Qt::WindowFlags flags = Qt::WindowFlags() );

    class Instrument
    {
    public:
      int       ID;        //!< The database ID of the instrument
      QString   name;      //!< The name of the instrument
      QString   serial;    //!< The serial number of the instrument
      int       radialCalID;
      QString   optimaHost;
      int       optimaPort;
      QString   optimaDBname;
      QString   optimaDBusername;
      QString   optimaDBpassw;
      int       selected;
      QString   os1;
      QString   os2;
      QString   os3;

      QString   radcalwvl;
      QString   chromoab;

      int       msgPort;
      
      //Instrument();
      
      void reset (void); 
    };

    Instrument currentInstrument;
    int        instID_toedit;
    
 private:
    //US_NewXpnHostDB* new_xpnhost_db;

    QList<QStringList> dblist;
    QString            uuid;
    QMap< QString, QString > conn_stat;
    
    QPushButton* pb_add;   
    QPushButton* pb_delete;
    QPushButton* pb_edit;
    QPushButton* pb_reset; 
    QPushButton* pb_testConnect;

    QLabel* host;
    QLabel* port;
    QLabel* msgPort;
    QLabel* name;
    QLabel* user;
    QLabel* pasw;
    QLabel* bn_chromoab;
    QLabel* lb_radcalwvl;
    QLabel* lb_chromofile;
    
    QLineEdit*   le_description;
    QLineEdit*   le_serial;
    QLineEdit*   le_dbname; 
    QLineEdit*   le_host;  
    QLineEdit*   le_port;
    QLineEdit*   le_msgPort;
    QLineEdit*   le_name;  
    QLineEdit*   le_user;  
    QLineEdit*   le_pasw;

    QLineEdit*   le_radcalwvl;
    QLineEdit*   le_chromofile;

    QLineEdit*   le_os1;  
    QLineEdit*   le_os2;  
    QLineEdit*   le_os3;  

    QComboBox*   cb_os1;
    QComboBox*   cb_os2;
    QComboBox*   cb_os3;

    QListWidget* lw_entries;

    void update_lw( void );
    void update_inv( void );
    bool use_db;
    
    QList< Instrument > instruments; 
    
  private slots:
    void help        ( void ); 
    void select_db   ( QListWidgetItem*, const bool = true );
    void add_new     ( void ); 
    /* void check_add   ( void ); */
    void reset       ( void ); 
    void close_program ( void ); 
    void editDB      ( void ); 
    void deleteDB    ( void ); 
    bool test_connect( void ); 
    void readInstruments ( IUS_DB2* = 0 );
    void newHost( QMap <QString, QString> & newInstrument );
    void editHost( QMap <QString, QString> & newInstrument );
    void fillGui( QMap <QString, QString> & );
    void optima_selected( void );
    void optima_selected( QListWidgetItem* );
    void editnew_cancelled( void );
    bool check_user_level( void );
};
#endif
