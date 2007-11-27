#ifndef US_DB_RST_EQUILPROJECT_H
#define US_DB_RST_EQUILPROJECT_H

#include "us_db.h"
#include "us_equilprojectreport.h"
#include "us_db_runrequest.h"
#include <qwidget.h>
#include <qprogressdialog.h>

class US_EXTERN US_DB_RST_EquilProject : public US_DB
{
	Q_OBJECT
	
	public:
	
		US_DB_RST_EquilProject(QWidget *p=0, const char *name=0);
		~US_DB_RST_EquilProject();
		
		QPushButton	*pb_investigator;			/*!< A PushButton connect to sel_investigator(). */
		QPushButton	*pb_runrequest;			/*!< A PushButton connect to sel_runrequest(). */
		QPushButton *pb_hd;						/*!< A PushButton connect to load_HD(). */
		QPushButton *pb_load_db;				/*!< A PushButton connect to load_DB(). */
		QPushButton *pb_retrieve_db;			/*!< A PushButton connect to retrieve_DB(). */
		QPushButton *pb_save_db;				/*!< A PushButton connect to save_db(). */
		QPushButton *pb_display;				/*!< A PushButton connect to display(). */
		QPushButton *pb_help;					/*!< A PushButton connect to help(). */
		QPushButton *pb_close;					/*!< A PushButton connect to quit(). */
		QPushButton *pb_del_db;					/*!< A PushButton connect to del_db(). */
		QPushButton *pb_reset;					/*!< A PushButton connect to clear(). */
		QListBox *lb_result;						/*!< A ListBox shows a result filename list. */
		QLabel *lbl_blank;						/*!< A label shows 'Equilibrium Project Result:'. */
		QLabel *lbl_instr;						/*!< A label shows a instruction. */
		QLabel *lbl_item;					 		/*!< A label shows selected item. */
		QLabel *lbl_investigator;				/*!< A Label shows selected investigator Information. */
		QLabel *lbl_runrequest;					/*!< A Label shows selected runrequest Information. */
		QLineEdit *le_item;

		QString fileName, projectName, htmlDir, baseName, dirName, indexFile;
		QString *item_projectName;
		int investigatorID;
		int runrequestID;
		int projectID;
		int *item_projectID;
		int DataID;
		QProgressDialog *pd;						/*!< A Progress Bar for submitting data to DB. */
		QProcess *compress_proc;
		QProcess *uncompress_proc;
		int c_step, uc_step;
		bool from_HD, from_DB, query_flag;
		
		public slots:
		void setup_GUI();
		bool insertCompressData();
		void endCompressProcess();
		void endUncompressProcess();
		void sel_investigator();
		void update_investigator_lbl(QString, int);
		void sel_runrequest();
		void update_runrequest_lbl(int );
		void load_HD();
		void load_DB();
		void select_result(int);
		void retrieve_db();
		void display();
		void save_db();
		void check_permission();
		void delete_db(bool);
		void cleanCompressFile();
		void clear();
		void reset();
		void help();
		void quit();
		void closeEvent( QCloseEvent *e);

		
		
};

#endif
