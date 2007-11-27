#ifndef US_RTVDATE_H
#define US_RTVDATE_H

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

//! An Interface for retrieving experimental data from Database through Date.
/*!
	This interface can be found in "Database -> Retrive Experimental Data from DB -> Date".
	You can use this interface to select one experimental date as condition to retrieve 
	experimental data from DB or let DB list all experimental data names for your select.
*/
class US_EXTERN US_DB_RtvDate : public US_DB
{
	Q_OBJECT
	
	public:
		US_DB_RtvDate(QWidget *p=0, const char *name="us_rtvdate");
		~US_DB_RtvDate();
		
		QString Date,								/*!< A string variable for Date. */ 
				  Display;							/*!< A string variable for data item dispaly. */ 
		int ExpdataID; 							/*!< An integer variable for ExpData id. */
		int *item_ExpdataID;						/*!< An integer array stores ExpData id.*/
		QString *display_Str;					/*!< A string array stores dispaly string.*/
		bool sel_data, 							/*!< A flag use for data listbox select, initialize to <var>false</var>. */
			  retrieve_flag;						/*!< A flag use for 'Retrieve' pushbutton, initialize to <var>false</var>. */

		QLabel *lbl_blank;						/*!< A label shows 'Retrieve Experimental Data by Date from DB:' + dbname. */
		QLabel *lbl_date;							/*!< A label shows selected Date, default shows 'not selected'. */
		QLabel *lbl_instr;						/*!< A label shows 'Doubleclick on item to select:'. */
		QLabel *lbl_item;							/*!< A label shows selected experimental data item. */

		QListBox *lb_data;						/*!< A ListBox shows a experimental data name list. */
		QPushButton *pb_date;					/*!< A PushButton connect to select_date(). */
		QPushButton *pb_chkDate;				/*!< A PushButton connect to checkDate(). */
		QPushButton *pb_help;					/*!< A PushButton connect to help(). */
		QPushButton *pb_retrieve;				/*!< A PushButton connect to retrieve(). */
		QPushButton *pb_reset;					/*!< A PushButton connect to reset(). */
		QPushButton *pb_close;					/*!< A PushButton connect to quit(). */

		US_Calendar *calendar_dlg;				/*!< A reference of US_Calendar. */
		US_ExpData_DB *expdata_dlg;			/*!< A reference of US_ExpData_DB. */
		
	private slots:
		void setup_GUI();
// pushbuttons:
		void select_date();
		void checkDate();
		void retrieve();
		void reset();
		void help();
		void quit();
// label:
		void update_date_lbl(QString);	
// Listbox:
		void select_data(int item);
};

#endif


