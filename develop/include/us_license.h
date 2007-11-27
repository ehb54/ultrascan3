#ifndef US_LICENSE_H
#define US_LINCESE_H

// QT defs:

#include "us.h"
#include "us_util.h"
#include "us_extern.h"
#include "us_database.h"
#include "us_color.h"
#include "us_font.h"

#include <qdir.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlistbox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qevent.h>
#include <qwidget.h>
#include <qtranslator.h>
#include <qframe.h>
#include <qstring.h>
extern int US_EXTERN global_Xpos;
extern int US_EXTERN global_Ypos;

class US_EXTERN US_License : public QDialog
{
	Q_OBJECT
	
	public:
		US_License(QWidget *parent=0, const char *name=0 );
		~US_License();
	
	private:
		
		QProcess *proc;
		int stderrSize, trials;

		QPushButton *pb_save;
		QPushButton *pb_help;
		QPushButton *pb_cancel;
		QPushButton *pb_load;
		QPushButton *pb_request;
		QPushButton *pb_import;

		QLabel *lbl_blank;
		QLabel *lbl_firstname;
		QLabel *lbl_lastname;
		QLabel *lbl_institution;
		QLabel *lbl_address;
		QLabel *lbl_city;
		QLabel *lbl_state;
		QLabel *lbl_zip;
		QLabel *lbl_phone;
		QLabel *lbl_email;
		QLabel *lbl_os;
		QLabel *lbl_platform;
		QLabel *lbl_version;
		QLabel *lbl_licensetype;
		QLabel *lbl_code;
		QLabel *lbl_expiration;

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


		QLineEdit *le_firstname;
		QLineEdit *le_lastname;
		QLineEdit *le_institution;
		QLineEdit *le_address;
		QLineEdit *le_city;
		QLineEdit *le_zip;
		QLineEdit *le_phone;
		QLineEdit *le_email;
		QLineEdit *le_code1;
		QLineEdit *le_code2;
		QLineEdit *le_code3;
		QLineEdit *le_code4;
		QLineEdit *le_code5;
		QLineEdit *le_expiration;

		QComboBox *cbb_state;
		QComboBox *cbb_os1;
		QComboBox *cbb_version;
		QComboBox *cbb_licensetype;
		
		QListBox *lb_os;
		QRadioButton *rb_opteron;
		QRadioButton *rb_intel;
		QRadioButton *rb_sparc;
		QRadioButton *rb_sgi;
		QRadioButton *rb_mac;
	
	public slots:
		void help();
		void cancel();
		void save();
		void import();
		void request();
		void display();
		void update_firstname(const QString &);
		void update_lastname(const QString &);
		void update_institution(const QString &);
		void update_address(const QString &);
		void update_city(const QString &);
		void update_state(int);
		void update_zip(const QString &);
		void update_phone(const QString &);
		void update_email(const QString &);
		void update_version(int);
		void update_licensetype(int);
		void update_code1(const QString &);
		void update_code2(const QString &);
		void update_code3(const QString &);
		void update_code4(const QString &);
		void update_code5(const QString &);
		void update_expiration(const QString &);
		void update_opteron_rb();
		void update_intel_rb();
		void update_sparc_rb();
		void update_mac_rb();
		void update_sgi_rb();
		void update_os(int);
		void captureStdout();
		void captureStderr();
		void endProcess();

	protected slots:
		void closeEvent(QCloseEvent *);
};

#endif

