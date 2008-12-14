//! \file us_license.h
#ifndef US_LICENSE_H
#define US_LICENSE_H

#include <QtGui>

#include "us_widgets.h"
#include "us_help.h"

//! \brief Manage a user's UltraScan license.

/*! \class US_License

    This class allows a user to set license data either manually or
    via import of a mail message.
*/

//! \note All spacing values are in pixel units.
#define spacing        5 //!< Spacing between widgets.
#define commaWidth    10 //!< Spacing for a simple comma.
#define buttonw      180 //!< Width of each pushbutton.
#define full_buttonw 610 //!< Width of large line edit widgets.
#define half_buttonw 295 //!< Width of small line edit widgets.
#define pushbutton   156 //!< Width of pushbuttons on bottom row.
#define rowHeight     26 //!< Height of each row of widgets.
#define smallColumn   90 //!< Width of a small column
#define mediumColumn 110 //!< Width of a medium column
#define codeWidth     60 //!< Width of license code line edit widgets

class US_License : public US_Widgets
{
  Q_OBJECT
  
  public:
    /*! \brief Set up Gui page for license import.
         
        The constructor creates all needed widgets and provides
        appropriate connections to save the license via \ref US_Settings.
        \param parent A pointer to the parent widget.  This normally can be
                      left as NULL.
        \param flags  An enum value that describes the window properties.  This 
                      normally can be left as 0 (Qt::Widget).
    */
    US_License( QWidget* parent = 0, Qt::WindowFlags flags = 0 );

    //! \brief A null destructor.
    ~US_License();
  
  private:
    QPushButton* pb_save;
    QPushButton* pb_help;
    QPushButton* pb_cancel;
    QPushButton* pb_load;
    QPushButton* pb_request;
    QPushButton* pb_import;

    QString firstname;
    QString lastname;
    QString institution;
    QString address;
    QString city;
    QString state;
    QString zip;
    QString phone;
    QString email;
    QString os;
    QString platform;
    QString version;
    QString licensetype;
    QString code1;
    QString code2;
    QString code3;
    QString code4;
    QString code5;
    QString expiration;

    QLineEdit* le_firstname;
    QLineEdit* le_lastname;
    QLineEdit* le_institution;
    QLineEdit* le_address;
    QLineEdit* le_city;
    QLineEdit* le_zip;
    QLineEdit* le_phone;
    QLineEdit* le_email;
    QLineEdit* le_code1;
    QLineEdit* le_code2;
    QLineEdit* le_code3;
    QLineEdit* le_code4;
    QLineEdit* le_code5;
    QLineEdit* le_expiration;

    QComboBox* cbb_state;
    QComboBox* cbb_os1;
    QComboBox* cbb_version;
    QComboBox* cbb_licensetype;
    
    QListWidget*  lb_os;
    QRadioButton* rb_opteron;
    QRadioButton* rb_intel;
    QRadioButton* rb_sparc;
    QRadioButton* rb_sgi;
    QRadioButton* rb_mac;

    US_Help       online_help;

    QStringList   states;
    QStringList   types;
    QStringList   versions;
    QStringList   osTypes;

  private slots:
    void cancel ( void );
    void help   ( void );
    void request( void );
    void save   ( void );
    void import ( void );

    void update_firstname  ( const QString& s ){ firstname   = s; };
    void update_lastname   ( const QString& s ){ lastname    = s; };
    void update_institution( const QString& s ){ institution = s; };
    void update_address    ( const QString& s ){ address     = s; };
    void update_city       ( const QString& s ){ city        = s; };
    void update_state      ( const QString& s ){ state       = s; };
    void update_zip        ( const QString& s ){ zip         = s; };
    void update_phone      ( const QString& s ){ phone       = s; };
    void update_email      ( const QString& s ){ email       = s; };
    void update_version    ( const QString& s ){ version     = s; };
    void update_licensetype( const QString& s ){ licensetype = s; };
    void update_code1      ( const QString& s ){ code1       = s; };
    void update_code2      ( const QString& s ){ code2       = s; };
    void update_code3      ( const QString& s ){ code3       = s; };
    void update_code4      ( const QString& s ){ code4       = s; };
    void update_code5      ( const QString& s ){ code5       = s; };
    void update_expiration ( const QString& s ){ expiration  = s; };
    void update_os         ( const int );

    void update_opteron_rb ( void ) { platform = "opteron"; };
    void update_intel_rb   ( void ) { platform = "intel";   };
    void update_sparc_rb   ( void ) { platform = "sparc";   };
    void update_mac_rb     ( void ) { platform = "mac";     };
    void update_sgi_rb     ( void ) { platform = "sgi";     };

    void closeEvent        ( QCloseEvent* e ) { e->accept(); };
};

#endif

