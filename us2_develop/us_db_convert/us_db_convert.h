#ifndef US_DB_CONVERT_H
#define US_DB_CONVERT_H
#ifdef WIN32
//#define ZLIB_DLL
#define _WINDOWS
#include <zlib.h>
#endif
#include <qpushbutton.h>
#include <qlabel.h>
#include <qdatetime.h>
#include <qcombobox.h>
#include <qlistbox.h>
#include <qlineedit.h>
#include <qfiledialog.h>
#include <qtextstream.h>
#include <qfiledialog.h>
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <qsqlcursor.h>
#include <qprogressdialog.h>
#include <qtimer.h>
#ifdef UNIX
#include "../include/unix/zlib.h"
#endif
#include "../include/us_db.h"
#include "../include/us_db_tbl_investigator.h"
#include "../include/us_db_tbl_cell.h"

//! An Interface for experimental data.
/*!
	This interface can be found in "Database->Commit Data to DB->Experimental Data".
	You can use this interface to store or retrieve experimental data from DB table <tt>tblExpData</tt>.
*/
class US_Convert_DB : public US_DB
{
	Q_OBJECT
	public:
		US_Convert_DB(QWidget *parent=0, const char *name="us_tblexpdata");
		~US_Convert_DB();
		
		struct US_ExpData exp_info;		/*!< A struct US_ExpData for storing experimental data info. */
		bool 	cell_flag, 						/*!< A flag use for cell table connect, initialize to <var>false</var>. */
				disp_changed[8], 
				query_flag, 					/*!< A flag use for query listbox select, initialize to <var>false</var>. */
				from_query,						/*!< A flag use for Cell ID display, initialize to <var>false</var>. */
				all_done;						/*!< A flag use for check all DB saving is done, initialize to <var>false</var>. */
		int *item_ExpdataID;					/*!< An integer array stores <tt>tblExpData</tt> id.*/
		int cell_table_unfinished, 		/*!< An integer variable as a count for cell table.*/
			 Steps;								/*!< An integer variable as a count for Progress Bar.*/
		int bufferid[8][4], 					/*!< A double integer variable for Buffer database table id.*/
			 pepid[8][4][3],					/*!< A double integer variable for Peptide database table id.*/
			 DNAid[8][4][3]; 					/*!< A double integer variable for DNA database table id.*/
		QString 	current_date, 				/*!< A string variable for Date.*/
					data_path, 					/*!< A string variable for data's hard drive location.*/
					compFile;					/*!< A string variable for compress file name and path.*/
		QString  *item_Runid, 		/*!< A string array stores experimental data description.*/
					*display_Str,				/*!< A string array stores experimental data display content.*/
					flo_channel;				/*!< A string that holds the channel from a fluorescent experiment.*/
					
					
		QPushButton *pb_run;					/*!< A PushButton connect to load_run(). */
		QPushButton *pb_help;				/*!< A PushButton connect to help(). */
		QPushButton *pb_close;				/*!< A PushButton connect to quit(). */
		QPushButton *pb_investigator;		/*!< A PushButton connect to sel_investigator(). */
		QPushButton *pb_add;					/*!< A PushButton connect to add_db(). */
		QPushButton *pb_delete;				/*!< A PushButton connect to delete_db(). */
		QPushButton *pb_date;				/*!< A PushButton connect to sel_date(). */
		QPushButton *pb_reset;				/*!< A PushButton connect to reset(). */
		QPushButton *pb_query;				/*!< A PushButton connect to query_db(). */
	
		QLabel *lbl_run;						/*!< A label shows loaded experimental data filename. */
		QLabel *lbl_investigator;			/*!< A label shows selected investigator. */
		QLabel *lbl_date;						/*!< A label shows selected date. */
		QLabel *lbl_experiment;				/*!< A label shows 'Experiment Type:'. */
		QLabel *lbl_exp;						/*!< A label shows loaded data's experiment type. */
		QLabel *lbl_cell;						/*!< A label shows loaded 'Cell ID:'. */
		QLabel *lbl_duration;				/*!< A label shows loaded 'Duration:'. */
		QLabel *lbl_dur;						/*!< A label shows loaded data's duration. */
		QLabel *lbl_description;			/*!< A label shows loaded 'Description:'. */
		QLabel *lbl_optical;					/*!< A label shows loaded 'Optical System:'. */
		QLabel *lbl_opt;						/*!< A label shows loaded data's optical system. */
		QLabel *lbl_temperature;			/*!< A label shows loaded 'Temperature:'. */
		QLabel *lbl_temp;						/*!< A label shows loaded data's temperature. */
		QLabel *lbl_rotor;					/*!< A label shows loaded 'Rotor:'. */
		QLabel *lbl_rt;						/*!< A label shows loaded data's rotor. */
		QLabel *lbl_runName;					/*!< A label shows loaded 'Run Name:'. */
		QLabel *lbl_runid;					/*!< A label shows loaded data's run id. */
		QProgressDialog *pd;					/*!< A Progress Bar for submitting data to DB. */
		QProgressDialog *pd_add;			/*!< A Progress Bar for retrieving data from DB. */
		QComboBox *cmbb_cell;				/*!< A ComboBox for listing Cell item. */
		QListBox *lb_query;					/*!< A ListBox for listing experimental data. */
		QLineEdit *le_description;			/*!< A LineEdit for inputting description. */

		US_DB_TblInvestigator *investigator_dlg;		/*!< A reference of US_DB_Investigator. */
		int create(QString, QString, QString);
		int retrieve(QString, int);
		QProcess *uncompress_proc;
		int uc_step;
		
	private slots:
		void setup_GUI();
		void add_db();
		void convert_db();
		void retrieve_db();
		void sel_query(int);
		void endUncompressProcess();

				void quit();
					
	public slots:
		bool retrieve_all(int, QString);
		
};

#endif

