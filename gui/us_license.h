//! \file us_license.h
#ifndef US_LICENSE_H
#define US_LICENSE_H

#include "us_widgets.h"
#include "us_help.h"
#include "us_extern.h"

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

class US_GUI_EXTERN US_License : public US_Widgets
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
    ~US_License() {};
  
  private:
    QPushButton* pb_cancel;
    QPushButton* pb_save;
    QPushButton* pb_help;
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
    QString validation;
    QString expiration;

    QLineEdit* le_firstname;
    QLineEdit* le_lastname;
    QLineEdit* le_institution;
    QLineEdit* le_city;
    QLineEdit* le_email;
    QLineEdit* le_platform;
    QLineEdit* le_expiration;
    QLineEdit* le_registration;

    QLabel*    lbl_version;
    QLabel*    lbl_platform;
    QLabel*    lbl_expiration;
    QLabel*    lbl_valid;

    QComboBox* cbb_state;
    QComboBox* cbb_os1;
    QComboBox* cbb_version;
    QComboBox* cbb_licensetype;
    
    US_Help       online_help;

    QStringList   states;
    QStringList   types;
    bool          updating_email;
    QPushButton*  pb_update;

    void    update_screen   ( void );
    void    load_current    ( void );
    QString titleCase       ( const QString& );
    void    request         ( void );
    bool    save            ( void );

  private slots:
    void help              ( void );
    void update            ( void );
    void update_response   ( const QString& );
    void request_response  ( const QString& );
    QString trim           ( const QString& );

    void update_firstname  ( const QString& s ){ firstname   = trim( s ); };
    void update_lastname   ( const QString& s ){ lastname    = trim( s ); };
    void update_institution( const QString& s ){ institution = trim( s ); };
    void update_city       ( const QString& s ){ city        = trim( s ); };
    void update_state      ( const QString& s ){ state       = trim( s ); };
    void update_email      ( const QString& s ){ email       = trim( s ); };
    void update_licensetype( const QString& s ){ licensetype = trim( s ); };
};
#endif
