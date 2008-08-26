#ifndef US_DB_RST_EQUILPROJECT_H
#define US_DB_RST_EQUILPROJECT_H

#include "us_db_widgets.h"
#include "us_equilprojectreport.h"
#include "us_db_runrequest.h"

#include <qwidget.h>
#include <qprogressdialog.h>

class US_EXTERN US_DB_RST_EquilProject : public US_DB_Widgets
{
	Q_OBJECT
	
	public:
	
		US_DB_RST_EquilProject( QWidget* p = 0, const char* name = 0 );
		~US_DB_RST_EquilProject() {};
		
	private:

		// Widgets
		QPushButton* pb_investigator;
		QPushButton* pb_runrequest;
		QPushButton* pb_hd;
		QPushButton* pb_load_db;
		QPushButton* pb_retrieve_db;
		QPushButton* pb_save_db;
		QPushButton* pb_display;
		QPushButton* pb_help;
		QPushButton* pb_close;
		QPushButton* pb_del_db;
		QPushButton* pb_reset;

		QListBox*    lb_result;
		
		QLabel*      lbl_blank;
		QLabel*      lbl_instr;
		QLabel*      lbl_item;
		QLabel*      lbl_investigator;
		QLabel*      lbl_runrequest;
		
		QLineEdit*   le_item;

		// Class Variables
		QString  projectName;
		QString* item_projectName;

		int      investigatorID;
		int      runrequestID;
		int      projectID;
		int*     item_projectID;
		int      DataID;

		bool     from_HD;
		bool     from_DB;
		bool     query_flag;
		
		// Internal functions 
		void setup_GUI        ( void );
		void cleanCompressFile( void );
		void clear            ( void );
		void closeEvent       ( QCloseEvent* );	

		bool insertCompressData();
		void endCompressProcess();
		void endUncompressProcess();

	private slots:

		void load_HD          ( void );
		void load_DB          ( void );
		void retrieve_db      ( void );
		void sel_investigator ( void );
		void sel_runrequest   ( void );
		void select_result    ( int );
		void save_db          ( void );
		void display          ( void );
		void check_permission ( void );
		void reset            ( void );
		void help             ( void );
		void quit             ( void );

		void update_investigator_lbl ( QString, int );
		void update_runrequest_lbl   ( int );
		void delete_db               ( bool );
};

#endif
