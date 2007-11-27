#ifndef US_RUNREQUEST_H
#define US_RUNREQUEST_H

#include <qpushbutton.h>
#include <qlabel.h>
#include <qtextstream.h>
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <qlistbox.h>
#include "us_db.h"

class US_EXTERN US_DB_RunRequest : public US_DB
{
	Q_OBJECT
	
	public:
		US_DB_RunRequest(QWidget *p=0, const char *name="us_runrequest");
		~US_DB_RunRequest();
		
		QString 	Display;							/*!< A string variable for data item dispaly. */ 
		int RunRequestID;							/*!< An integer variable for RunRequest id. */
		int *item_RRID;							/*!< An integer array stores RunRequest id.*/
		QString *display_Str;					/*!< A string array stores dispaly string.*/
		bool sel_data, 							/*!< A flag use for data listbox select, initialize to <var>false</var>. */
			  accept_flag;
		QLabel *lbl_blank;						/*!< A label shows 'Select Run Request' */
		QLabel *lbl_instr;						/*!< A label shows 'Doubleclick on item to select:'. */
		QLabel *lbl_item;							/*!< A label shows selected RunRequest data item. */
		QListBox *lb_data;						/*!< A ListBox shows a run request data list. */
		QPushButton *pb_check;					/*!< A PushButton connect to check(). */
		QPushButton *pb_help;					/*!< A PushButton connect to help(). */
		QPushButton *pb_accept;					/*!< A PushButton connect to accept(). */
		QPushButton *pb_reset;					/*!< A PushButton connect to reset(). */
		QPushButton *pb_close;					/*!< A PushButton connect to quit(). */
				
	private slots:

		void check();
		void reset();
		void accept();
		void help();
		void quit();
		void select_data(int item);
		
	signals:
	
		void issue_RRID(int RunRequestID);
};

#endif


