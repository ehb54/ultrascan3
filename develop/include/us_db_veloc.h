#ifndef US_DB_VELOC_H
#define US_DB_VELOC_H

#include "us_db_widgets.h"
#include "us_util.h"
#include "us_dtactrl.h"
#include "us_db.h"
#include "us_velocreport.h"
#include "us_db_runrequest.h"

#include <qwidget.h>
#include <qprogressdialog.h>

class US_EXTERN US_DB_Veloc : public US_DB_Widgets
{
	Q_OBJECT
	
	public:
	
		US_DB_Veloc( QWidget* p = 0, const char* name = 0 );
		~US_DB_Veloc() {};

	private:

		
		QPushButton* pb_runrequest;   /*!< A PushButton connect to sel_runrequest(). */
		QPushButton* pb_load;         /*!< A PushButton connect to load(). */
		QPushButton* pb_load_db;      /*!< A PushButton connect to read_db(). */
		QPushButton* pb_retrieve_db;  /*!< A PushButton connect to retrieve_db(). */
		QPushButton* pb_save_db;      /*!< A PushButton connect to save_db(). */
		QPushButton* pb_del_db;       /*!< A PushButton connect to del_db(). */
		QPushButton* pb_investigator; /*!< A PushButton connect to sel_investigator(). */
		QPushButton* pb_help;         /*!< A PushButton connect to help(). */
		QPushButton* pb_close;        /*!< A PushButton connect to quit(). */
		QPushButton* pb_reset;        /*!< A PushButton connect to clear(). */
		QPushButton* pb_display;      /*!< A PushButton connect to display(). */
		QListBox*    lb_result;       /*!< A ListBox shows a result filename list. */
		
		QLabel*      lbl_instr;       /*!< A label shows instruction. */
		QLabel*      lbl_expdata;     /*!< A label shows 'Experimental Data ID'. */
		QLabel*      lbl_expdt;       /*!< A label shows loaded data's expdata ID. */
		QLabel*      lbl_runrequest;  /*!< A Label shows selected runrequest Information. */
		QLabel*      lbl_invst;       /*!< A label shows loaded data's investigator ID. */
		QLabel*      lbl_date;        /*!< A label shows 'Date'. */
		QLabel*      lbl_dt;          /*!< A label shows loaded data's date. */
		QLabel*      lbl_description; /*!< A label shows loaded 'Description:'. */
		QLabel*      le_dscrpt;       /*!< A Label shows loaded data's description. */

		bool from_HD;
		bool from_DB;
		bool query_flag;
		bool retrieve_flag;
		bool del_flag;

		int  DataID;
		int  runrequestID;
		int* item_ExpdataID;

		struct US_ExpResult exp_rst;  /*!< Storing experimental data results. */
		Data_Control_W*     data_control;
		
		QString  run_id;
		QString* item_Description;
		
		// Internal methods
		void setup_GUI              ( void );
		void cleanCompressedFiles   ( void );
		bool insertCompressedData   ( void );

	private slots:

		void load_HD                ( void );
		void help                   ( void );
		void quit                   ( void );
		void query_db               ( void );
		void save_db                ( void );
		void retrieve_db            ( void );
		void check_permission       ( void );
		void select_result          ( int );
		void reset                  ( void );
		void display                ( void );
		void sel_runrequest         ( void );
		void sel_investigator       ( void );
		void update_runrequest_lbl  ( int );
		void update_investigator_lbl( QString, int );
		void delete_db              ( bool );

		void closeEvent             ( QCloseEvent* );
};

#endif
