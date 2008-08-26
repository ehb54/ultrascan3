#ifndef US_DB_EQUIL_H
#define US_DB_EQUIL_H

#include "us_db_widgets.h"
#include "us_equilreport.h"
#include "us_db_runrequest.h"

#include <qwidget.h>
#include <qprogressdialog.h>

class US_EXTERN US_DB_Equil : public US_DB_Widgets
{
	Q_OBJECT

	public:

		US_DB_Equil( QWidget* p = 0, const char* name = 0 );
		~US_DB_Equil() {};

	private: 

		// Widgets

		QPushButton* pb_runrequest;
		QPushButton* pb_load;
		QPushButton* pb_load_db;
		QPushButton* pb_retrieve_db;
		QPushButton* pb_save_db;
		QPushButton* pb_del_db;
		QPushButton* pb_investigator;
		QPushButton* pb_help;
		QPushButton* pb_close;
		QPushButton* pb_reset;
		QPushButton* pb_display;

		QListBox*    lb_result;

		QLabel*      lbl_instr;
		QLabel*      lbl_expdata;
		QLabel*      lbl_expdt;
		QLabel*      lbl_runrequest;
		QLabel*      lbl_invst;		
		QLabel*      lbl_date;
		QLabel*      lbl_dt;
		QLabel*      lbl_description;
		QLabel*      le_dscrpt;

		// Class variables 

		struct US_ExpResult exp_rst;

		int                 DataID;
		int*                item_ExpdataID;
		int                 runrequestID;
                        
		bool                from_HD;
		bool                from_DB;
		bool                query_flag;
		bool                retrieve_flag;
		bool                del_flag;

		QString             run_id;
		QString*            item_Description;

		// Internal methods
		void setup_GUI              ( void );
		void endUncompressProcess();

		void cleanCompressFile      ( void );
		void closeEvent             ( QCloseEvent* );
		bool create_tar             ( const QString&, const QString&, QStringList& );

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
};

#endif
