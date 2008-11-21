//! \file us_license.h
#ifndef US_LICENSE_H
#define US_LICENSE_H

#include <QtGui>

#include "us_widgets.h"
#include "us_global.h"
#include "us_help.h"

class US_License : public US_Widgets
{
	Q_OBJECT
	
	public:
		US_License( QWidget* parent = 0, Qt::WindowFlags flags = 0 );
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

    US_Global     g;
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
};

#endif

