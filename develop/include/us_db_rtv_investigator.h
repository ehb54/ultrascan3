#ifndef US_RTVINVESTIGATOR_H
#define US_RTVINVESTIGATOR_H

#include <qpushbutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qfiledialog.h>
#include <qtextstream.h>
#include <qfiledialog.h>
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <qsqlcursor.h>
#include <qlistbox.h>

#include "us_db_tbl_expdata.h"

//! An Interface for retrieving experimental data from Database through Investigator.
/*!
	This interface can be found in "Database -> Retrive Experimental Data from DB -> Investigator".
	You can use this interface to input investigator's name as condition to retrieve 
	experimental data from DB or let DB list all experimental data names for your select.
*/
class US_EXTERN US_DB_RtvInvestigator : public US_DB
{
	Q_OBJECT
	
	public:
		US_DB_RtvInvestigator(QWidget *p=0, const char *name="us_rtvinvestigator");
		~US_DB_RtvInvestigator();
		
		QString *nameString, 				/*!< A string array stores name dispaly string.*/
				  *display_Str;				/*!< A string array stores data dispaly string.*/
		int *item_InvID, 						/*!< An integer array stores Investigator id.*/
			 *item_ExpdataID;					/*!< An integer array stores ExpData id.*/
		bool 	sel_name, 						/*!< A flag use for 'Check Last Name' pushbutton, initialize to <var>false</var>. */
				check_ID, 						/*!< A flag use for 'Check Experimental Data' pushbutton, initialize to <var>false</var>. */
				sel_data, 						/*!< A flag use for data listbox select, initialize to <var>false</var>. */
				retrieve_flag;					/*!< A flag use for 'Retrieve' pushbutton, initialize to <var>false</var>. */				
		int InvID, 								/*!< An integer variable for Investigator ID. */
			 ExpdataID;							/*!< An integer variable for ExpData ID. */
		QString 	LastName, 					/*!< A string variable for last name. */
					name, 						/*!< A string variable for whole name. */
					Display;						/*!< A string variable for experimental data name. */
		QLabel *lbl_blank;					/*!< A label shows 'Retrieve Experimental Data by Investigator from DB:' + dbname. */
		QLabel *lbl_LastName;				/*!< A label shows 'Input your Last Name:'. */
		QLabel *lbl_instr1;					/*!< A label shows 'Doubleclick on item to select:'. */
		QLabel *lbl_instr2;					/*!< A label shows 'Doubleclick on item to select:'. */
		QLabel *lbl_investigator;			/*!< A label shows selected investigator's name. */
		QLabel *lbl_item;						/*!< A label shows selected experimental data item. */
		QListBox *lb_name;					/*!< A ListBox shows a investigator name list. */
		QListBox *lb_data;					/*!< A ListBox shows a experimental data name list. */
		QPushButton *pb_chkname;			/*!< A PushButton connect to checkname(). */
		QPushButton *pb_chkID;				/*!< A PushButton connect to checkID(). */
		QPushButton *pb_help;				/*!< A PushButton connect to help(). */
		QPushButton *pb_retrieve;			/*!< A PushButton connect to retrieve(). */
		QPushButton *pb_reset;				/*!< A PushButton connect to reset(). */
		QPushButton *pb_close;				/*!< A PushButton connect to quit(). */
		QLineEdit *le_LastName;				/*!< A LineEdit for input investigator's last name */
		US_ExpData_DB *expdata_dlg;		/*!< A reference of US_ExpData_DB. */
		
		QGridLayout *Grid;
				
	protected slots:
		void setup_GUI();
// pushbuttons:
		void checkname();
		void checkID();
		void reset();
		void retrieve();
		void help();
		void quit();
// lineedits:
		void update_lastname(const QString &);
	
// Listbox:
		void select_name(int item);
		void check_data(int item);
		void select_data(int item);
};

#endif


