#ifndef US_DB_EQUIL_H
#define US_DB_EQUIL_H

#include "us_db.h"
#include "us_equilreport.h"
#include "us_db_runrequest.h"
#include <qwidget.h>
#include <qprogressdialog.h>

class US_EXTERN US_DB_Equil : public US_DB
{
	Q_OBJECT
	
	public:
	
		US_DB_Equil(QWidget *p=0, const char *name=0);
		~US_DB_Equil();
		
		struct US_ExpResult exp_rst;		/*!< A struct US_ExpRsesult for storing experimental data result. */
		int run_type, source_type;						/*! < 0 means from Hard Drive, 1 means from Database. */
		bool from_HD, from_DB, query_flag, retrieve_flag, del_flag;
		QPushButton	*pb_runrequest;			/*!< A PushButton connect to sel_runrequest(). */
		QPushButton *pb_load;					/*!< A PushButton connect to load(). */
		QPushButton *pb_load_db;				/*!< A PushButton connect to read_db(). */
		QPushButton *pb_retrieve_db;			/*!< A PushButton connect to retrieve_db(). */
		QPushButton *pb_save_db;				/*!< A PushButton connect to save_db(). */
		QPushButton *pb_del_db;					/*!< A PushButton connect to del_db(). */
		QPushButton *pb_investigator;			/*!< A PushButton connect to sel_investigator(). */
		QPushButton *pb_help;					/*!< A PushButton connect to help(). */
		QPushButton *pb_close;					/*!< A PushButton connect to quit(). */
		QPushButton *pb_reset;					/*!< A PushButton connect to clear(). */
		QPushButton *pb_display;				/*!< A PushButton connect to display(). */
		QListBox *lb_result;						/*!< A ListBox shows a result filename list. */
		QLabel *lbl_instr;						/*!< A label shows instruction. */
		QLabel *lbl_expdata;						/*!< A label shows 'Experimental Data ID'. */
		QLabel *lbl_expdt;						/*!< A label shows loaded data's expdata ID. */
		QLabel *lbl_runrequest;					/*!< A Label shows selected runrequest Information. */
		QLabel *lbl_invst;						/*!< A label shows loaded data's investigator ID. */
		QLabel *lbl_date;							/*!< A label shows 'Date'. */
		QLabel *lbl_dt;							/*!< A label shows loaded data's date. */
		QLabel *lbl_description;				/*!< A label shows loaded 'Description:'. */
		QLabel *le_dscrpt;						/*!< A Label shows loaded data's description. */
			
		QProgressDialog *pd;						/*!< A Progress Bar for submitting data to DB. */
		QProcess *compress_proc;
		QProcess *uncompress_proc;
		int c_step, uc_step;
		int border, spacing, xpos, ypos, X, Y, Xpos, Ypos, buttonw, buttonh;
		Data_Control_W *data_control;
		QString run_id, fileName, baseName, htmlDir, printerFile;
		int DataID, *item_ExpdataID;
		QString *item_Description, *display_Str;
		int runrequestID;

		public slots:
		
		void setup_GUI();
		bool insertCompressData();
		void endCompressProcess();
		void endUncompressProcess();
		void cleanCompressFile();
		void sel_runrequest();
		void update_runrequest_lbl(int);
		void sel_investigator();
		void update_investigator_lbl(QString, int );
		void load_HD();
		void save_db();
		void query_db();
		void retrieve_db();
		void select_result(int);
		void check_permission();
		void delete_db(bool);
		void reset();
		void display();
		void help();
		void quit();
		void closeEvent( QCloseEvent *e);
		
};

#endif
