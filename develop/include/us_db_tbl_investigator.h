#ifndef US_TBINVESTIGATOR_H
#define US_TBINVESTIGATOR_H

#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qfiledialog.h>
#include <qtextstream.h>
#include <qfiledialog.h>
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <qsqlcursor.h>
#include <qlistbox.h>
#include "us_db.h"

//! A Struct for storing required info for investigator register in DB.
/*!
	Each element is according to an entry field in Database Table <tt>tblInvestigators</tt>.
*/
struct US_InvestigatorData
{
	int InvID;					/*!< An integer number for tblInvestigator entry. */
	QString LastName;			/*!< A string stored the value of LastName.*/
	QString FirstName;		/*!< A string stored the value of FirstName.*/
	QString Address;			/*!< A string stored the value of Address.*/
	QString City;				/*!< A string stored the value of City.*/
	QString State;				/*!< A string stored the value of State.*/
	QString Zip;				/*!< A string stored the value of Zip.*/
	QString Phone;				/*!< A string stored the value of Phone Number.*/
	QString Email;				/*!< A string stored the value of Email.*/
	QString Display;			/*!< A String stored the value of Display */
};

//! An Interface for investigator register information.
/*!
	You need to use this interface to input your personal information when you register in database.
	This interface can be found in "Database->Commit Data to DB->Investigator".
	After you commit your input, the program will save your information to database table: tblInvestigators.
	Also you can use this interface to check your register records.
*/
class US_EXTERN US_DB_TblInvestigator : public US_DB
{
	Q_OBJECT

	public:
		US_DB_TblInvestigator(QWidget *p=0, const char *name="us_tblinvestigator");
		~US_DB_TblInvestigator();

		struct US_InvestigatorData info_list;	/*!< A struct US_InvestigatorData for storing Inverstigator register information. */
//		struct US_LoginData login_list;			/*!< A struct US_LoginData for storing DB login information. */
#ifdef WIN32
		  #pragma warning ( disable: 4251 )
#endif

		vector <QString> nameString; 			/*!< A String Array stores name dispaly.*/
		vector <QString> item_firstname;		/*!< A String Array stores first name.*/
		vector <QString> item_lastname;			/*!< A String Array stores last name.*/
		vector <int> item_InvID;					/*!< A Integer Array stores investigator ID.*/

#ifdef WIN32
		  #pragma warning ( default: 4251 )
#endif

		int newInvID,						/*!< A Integer variable for new investigator ID.*/
			 changed_flag, 				/*!< A flag for update(), 0 no change and 1 changed. */
			 new_register;					/*!< A flag for save(), 0 existed guy and 1 new guy. */
		bool select_flag;					/*!< A flag for query listbox select, default set to <var>false</var> */

		QLabel *lbl_blank;				/*!< A raised Label shows 'Investigator Information'. */
		QLabel *lbl_InvID;				/*!< A Label shows 'INvestigator ID:'. */
		QLabel *lbl_IdNumber;			/*!< A Label shows the number of Investigator ID. */
		QLabel *lbl_LastName;			/*!< A Label shows 'Last Name:'. */
		QLabel *lbl_FirstName;			/*!< A Label shows 'First Name:'. */
		QLabel *lbl_Address;				/*!< A Label shows 'Address:'. */
		QLabel *lbl_City;					/*!< A Label shows 'City:'. */
		QLabel *lbl_State;				/*!< A Label shows 'State:'. */
		QLabel *lbl_Zip;					/*!< A Label shows 'Zip:'. */
		QLabel *lbl_Phone;				/*!< A Label shows 'Phone:'. */
		QLabel *lbl_Email;				/*!< A Label shows 'Email:'. */

		QListBox *lb_name;				/*!< A ListBox shows a name list. */
		QPushButton *pb_chkname;		/*!< A PushButton connect to checkname(). */
		QPushButton *pb_help; 			/*!< A PushButton connect to help(). */
		QPushButton *pb_update;			/*!< A PushButton connect to update(). */
		QPushButton *pb_commit;			/*!< A PushButton connect to save(). */
		QPushButton *pb_reset;			/*!< A PushButton connect to reset(). */
		QPushButton *pb_delete;			/*!< A PushButton connect to del(). */
		QPushButton *pb_exit;			/*!< A PushButton connect to quit(). */

		QLineEdit *le_LastName;			/*!< A LineEdit used by update_lastname() */
		QLineEdit *le_FirstName;		/*!< A LineEdit used by update_firstname() */
		QLineEdit *le_Address;			/*!< A LineEdit used by update_address() */
		QLineEdit *le_City;				/*!< A LineEdit used by update_city() */
		QLineEdit *le_State;				/*!< A LineEdit used by update_state() */
		QLineEdit *le_Zip;				/*!< A LineEdit used by update_zip() */
		QLineEdit *le_Phone;				/*!< A LineEdit used by update_phone() */
		QLineEdit *le_Email;				/*!< A LineEdit used by update_email() */

	private slots:
		void GUI();
// pushbuttons:
		void checkname();
		void update();
		void save();
		void reset();
		void check_permission();
		void del(bool);
		void help();
		void quit();
// lineedits:
		void update_firstname(const QString &);
		void update_lastname(const QString &);
		void update_address(const QString &);
		void update_city(const QString &);
		void update_state(const QString &);
		void update_zip(const QString &);
		void update_phone(const QString &);
		void update_email(const QString &);
// Listbox:
		void select_name(int item);
// other:
		bool check_fields();
		void clear();

	signals:
/*!
	This signal is emitted whenever the name record is selected or closing the interface.\n
	The argument is String <var>Display</var> (InvID(#):+ FirstName + LastName) and integer <var>InvID</var>.
*/
		void valueChanged(QString Display, int InvID);
};

#endif


