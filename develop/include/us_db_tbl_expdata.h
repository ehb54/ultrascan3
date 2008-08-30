#ifndef US_DB_TBL_EXPDATA_H
#define US_DB_TBL_EXPDATA_H

#include <qpushbutton.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qlistbox.h>
#include <qlineedit.h>
#include <qdatetime.h>
#include <qtextstream.h>
#include <qsqlquery.h>
#include <qsqlcursor.h>

#include "us_editdb.h"
#include "us_db_tbl_investigator.h"
#include "us_calendar.h"
#include "us_db_tbl_cell.h"
#include "us_db_widgets.h"

//! An Interface for experimental data.
/*!  This interface can be found in "Database->Commit Data to 
     DB->Experimental Data".	You can use this interface to store or 
     retrieve experimental data from DB table <tt>tblExpData</tt>.
*/

class US_EXTERN US_ExpData_DB : public US_DB_Widgets
{
	Q_OBJECT
	
	public:
	
		US_ExpData_DB( QWidget* parent = 0, const char* name = "us_tblexpdata" );
		~US_ExpData_DB();

		static void cleanDir( const QString& );

	private:

		QPushButton*     pb_run;
		QPushButton*     pb_help;
		QPushButton*     pb_close;
		QPushButton*     pb_investigator;
		QPushButton*     pb_add;
		QPushButton*     pb_delete;
		QPushButton*     pb_date;
		QPushButton*     pb_reset;
		QPushButton*     pb_query;
	
		QLabel*          lbl_run;
		QLabel*          lbl_investigator;
		QLabel*          lbl_date;
		QLabel*          lbl_experiment;
		QLabel*          lbl_exp;
		QLabel*          lbl_cell;
		QLabel*          lbl_duration;
		QLabel*          lbl_dur;
		QLabel*          lbl_description;
		QLabel*          lbl_optical;
		QLabel*          lbl_opt;
		QLabel*          lbl_temperature;
		QLabel*          lbl_temp;
		QLabel*          lbl_rotor;
		QLabel*          lbl_rt;
		QLabel*          lbl_runName;
		QLabel*          lbl_runid;

		QComboBox*       cmbb_cell;
		
		QListBox*        lb_query;
		
		QLineEdit*       le_description;

		// Class variables
		QString  current_date;
		QString  compFile;
		QString* item_Description;
		QString  flo_channel;

		struct US_ExpData exp_info;

		bool cell_flag;
		bool query_flag;
		bool from_query;
		bool all_done;

		int* item_ExpdataID;
		int  cell_table_unfinished;
		int  Steps;   // Set but never used
		int  bufferid[ 8 ][ 4 ];
		int  pepid   [ 8 ][ 4 ][ 3 ];
		int  DNAid   [ 8 ][ 4 ][ 3 ];

		US_Edit_DB*            editdb;
		US_DB_TblInvestigator* investigator_dlg;
		US_Calendar*           calendar_dlg;
		US_Cell_DB*            cell_dlg;

		// Internal support methods 

		void write_dbinfo           ( QString );				
		void setup_GUI              ( void );

	private slots:
		void update_variables       ( void );
		void sel_cell               ( int );
		void update_description     ( const QString& );
		void load_run               ( void );
		void sel_investigator       ( void );
		void update_investigator_lbl( QString, int );
		void sel_date               ( void );
		void update_date_lbl        ( QString );
		void add_db                 ( void );
		void query_db               ( void );
		void sel_query              ( int );
		void check_permission       ( void );
		void delete_db              ( bool );
		void reset                  ( void );
		void help                   ( void );
		void quit                   ( void );

	public slots:

		void update_type            ( int );
		void update_rotor           ( int );
		void update_cell            ( void );
		void update_flo_channel     ( const QString& );
		bool retrieve_all           ( int, QString );
                               
	protected slots:             
                               
		void closeEvent             ( QCloseEvent* );
};

#endif

