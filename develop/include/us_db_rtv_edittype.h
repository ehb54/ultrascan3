#ifndef US_RTVEDITTYPE_H
#define US_RTVEDITTYPE_H

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

//! An Interface for retrieving experimental data from Database through Edit Type.
/*!
	This interface can be found in "Database -> Retrive Experimental Data from DB -> Edit Type".
	You can use this interface to select edit type as condition to retrieve 
	experimental data from DB or let DB list all experimental data names for your select.
*/
class US_EXTERN US_DB_RtvEditType : public US_DB
{
	Q_OBJECT
	
	public:
		US_DB_RtvEditType(QWidget *p=0, const char *name="us_rtvedittype");
		~US_DB_RtvEditType();
		
		QString Display;						/*!< A string variable for data item dispaly. */ 
		int ExpdataID;							/*!< An integer variable for ExpData id.  	 */
		int exp_type, 							/*!< An integer variable for Experiment Type. */ 												
			 opt_type, 							/*!< An integer variable for Optical System. */
			 edit_type;							/*!< An integer variable for Edit Type. */
		int *item_ExpdataID;					/*!< An integer array stores ExpData id.*/
		QString *display_Str;				/*!< A string array stores dispaly string.*/
		bool 	sel_experiment, 				/*!< A flag use for 'Choose Experiment Type' combolist, initialize to <var>false</var>. */	
				sel_optical, 					/*!< A flag use for 'Choose Optical System' combolist, initialize to <var>false</var>. */
				sel_data, 						/*!< A flag use for data listbox select, initialize to <var>false</var>. */
				retrieve_flag;					/*!< A flag use for 'Retrieve' pushbutton, initialize to <var>false</var>. */

		QLabel *lbl_blank;					/*!< A label shows 'Retrieve Experimental Data by Edit Type from DB:' + dbname. */
		QLabel *lbl_experiment;				/*!< A label shows 'Choose Experimental Type:'. */
		QLabel *lbl_optical;					/*!< A label shows 'Choose Optical System:'. */
		QLabel *lbl_instr;					/*!< A label shows 'Doubleclick on item to select:'. */
		QLabel *lbl_item;						/*!< A label shows selected experimental data item. */
		QComboBox *cmbb_experiment;		/*!< A ComboBox shows a experimental type name list. */
		QComboBox *cmbb_optical;			/*!< A ComboBox shows a optical system name list. */
		QListBox *lb_data;					/*!< A ListBox shows a experimental data name list. */
		QPushButton *pb_chkET;				/*!< A PushButton connect to checkEditType(). */
		QPushButton *pb_help;				/*!< A PushButton connect to help(). */
		QPushButton *pb_retrieve;			/*!< A PushButton connect to retrieve(). */
		QPushButton *pb_reset;				/*!< A PushButton connect to reset(). */
		QPushButton *pb_close;				/*!< A PushButton connect to quit(). */

		US_ExpData_DB *expdata_dlg;		/*!< A reference of US_ExpData_DB. */

	private slots:
		void setup_GUI();
// pushbuttons:
		void checkEditType();
		void reset();
		void retrieve();
		void help();
		void quit();
// combbox:
		void select_experiment(int);
		void select_optical(int);	
// Listbox:
		void select_data(int item);
};

#endif


