#ifndef US_DATABASE_H
#define US_DATABASE_H

#include <qpushbutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include	<qlistbox.h>
#include <qfiledialog.h>
#include <qtextstream.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qlayout.h>
#include "us_db_admin.h"
#include "us_encryption.h"

/*!	
	\brief	Required info for connecting database in Qt-SQL
	\par Example 
	The following example shows the most simple way to use US_LoginData components.
	\verbatim
	#include <qsqldatabase.h>
	struct LoginData login_list;
		
	login_list.driver = "QMYSQL3";
	login_list.dbname = "Ultrascan";
	login_list.username = "David";
	login_list.password = "********";
	login_list.host = "localhost";
	
	QSqlDatabase *defaultDB = QSqlDatabase::addDatabase( login_list.driver );
	if ( defaultDB ) 
	{
		defaultDB->setDatabaseName( login_list.dbname );
		defaultDB->setUserName( login_list.username );
		defaultDB->setPassword( login_list.password );
		defaultDB->setHostName( login_list.host );

		if ( defaultDB->open() )
		{
			// Database successfully opened; we can now issue SQL commands.
		}
	}
	\endverbatim
*/
struct US_LoginData
{
	/*! 	
		The name of Database Driver. \n
		The available drivers are: QMYSQL3(MySQL) and QOCI8(Oracle).	
	*/
	QString driver;
	/*! 
		The user name which you use to login in your DB account
	*/	
	QString username;
	/*!
		The password which you use to login in your DB account
	*/
	QString password;
	/*!
		Which DB name you choose to use
	*/
	QString dbname;
	/*! 
		DB host address
	*/
	QString host;
	/*! 
		DB description
	*/
	QString description;
};

//! An Interface for storing Database Login information.
/*!
	If you want to use database, You need to use this interface to input
	your personal user information when you config UltraScan.
	This interface can be found in "File->Configuration->Database Preferences Change".
	After you save your information, the program will create a binary file to store the 
	information for your advanced using.
*/
class US_EXTERN US_Database : public QFrame
{
	Q_OBJECT
	
	public:
		US_Database(QWidget *p=0, const char *name="us_database");
		~US_Database();
		
		struct US_LoginData login_list;	/*!< A struct LoginData for storing DB login information */

#ifdef WIN32
  #pragma warning( disable: 4251 )
#endif
		vector <struct US_LoginData> dblist;
		
#ifdef WIN32
  #pragma warning( default: 4251 )
#endif
    
    int count, selected_item;
		QListBox *lb_dbname;			/*!< A ListBox shows a database name list. */
		QLabel *lbl_title; 			/*!< A raised Label shows 'Database Configuration'. */
		QLabel *lbl_blank; 			/*!< A raised Label shows 'User Information'. */
		QLabel *lbl_description;	/*!< A Label shows 'Database Description:'. */
		QLabel *lbl_username;		/*!< A Label shows 'User Name:'. */
		QLabel *lbl_password;		/*!< A Label shows 'Password:'. */
		QLabel *lbl_dbname;			/*!< A Label shows 'Database Name:'. */
		QLabel *lbl_host;				/*!< A Label shows 'Host Address'. */
		QLabel *lbl_mysql;			/*!< A Label shows 'MYSQL'. */
		QLabel *lbl_oracle;			/*!< A Label shows 'ORACLE'. */

		QPushButton *pb_add;			/*!< A PushButton shows 'Add New Entry', connect to add(). */
		QPushButton *pb_delete;		/*!< A PushButton shows 'Delete Old Entry', connect to del(). */
		QPushButton *pb_help;		/*!< A PushButton shows 'Help', connect to help(). */
		QPushButton *pb_submit;		/*!< A PushButton shows 'Save and Exit',connect to save(). */
		QPushButton *pb_reset;		/*!< A PushButton shows 'Reset', connect to reset(). */
		QPushButton *pb_close;		/*!< A PushButton shows 'Cancel and Exit', connect to quit(). */
		
		QLineEdit *le_description;	/*!< A LineEdit used by update_description() */
		QLineEdit *le_username;		/*!< A LineEdit used by update_username() */
		QLineEdit *le_password;		/*!< A LineEdit used by update_password() */
		QLineEdit *le_dbname;		/*!< A LineEdit used by update_dbname() */
		QLineEdit *le_host;			/*!< A LineEdit used by update_host() */
	
		QCheckBox *cb_mysql;			/*!< A CheckBox connect to select_mysql() */
		QCheckBox *cb_oracle;		/*!< A CheckBox connect to select_oracle() */

		US_Config *USglobal;			/*!< A US_Config reference. */
		QString data_file,
				  db_name,
				  host_name,
				  filename;				/*!< A String for setting the DB file name and path. */
		bool export_format;			/*!< A Boolean value for controlling CheckBox select. */ 
		
		void read_file();
		void Encode(QString);
		void update_screen();

		
	private slots:
		void setup_GUI();
// listbox:
		void select_db(int);
// pushbuttons:
		void check_add();
		void add(bool);
		void check_del();
		void del(bool);
		void check_save();
		void save(QString);
		void clear();
		void reset();
		void help();
		void quit();
// lineedits:
		void update_description(const QString &);
		void update_username(const QString &);
		void update_password(const QString &);
		void update_dbname(const QString &);
		void update_host(const QString &);
// checkboxes:
		void select_mysql();
		void select_oracle();
};

#endif


