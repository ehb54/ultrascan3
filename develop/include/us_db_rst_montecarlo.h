#ifndef US_DB_RST_MONTECARLO_H
#define US_DB_RST_mONTECARLO_H

#include "us_db.h"
#include "us_montecarloreport.h"
#include "us_db_runrequest.h"
#include <qprogressdialog.h>
#include <qwidget.h>
#include <qprocess.h>

class US_EXTERN US_DB_RST_Montecarlo : public US_DB
{
	Q_OBJECT
	
	public:
	
		US_DB_RST_Montecarlo(QWidget *p=0, const char *name=0);
		~US_DB_RST_Montecarlo();
		
		QPushButton	*pb_investigator;			/*!< A PushButton connect to sel_investigator(). */
		QPushButton	*pb_runrequest;			/*!< A PushButton connect to sel_runrequest(). */
		QPushButton *pb_hd;						/*!< A PushButton connect to load_HD(). */
		QPushButton *pb_db;				   	/*!< A PushButton connect to load_DB(). */
		QPushButton *pb_retrieve;			 	/*!< A PushButton connect to retrieve(). */
		QPushButton *pb_save;					/*!< A PushButton connect to save_db(). */
		QPushButton *pb_display;				/*!< A PushButton connect to display(). */
		QPushButton *pb_help;					/*!< A PushButton connect to help(). */
		QPushButton *pb_close;					/*!< A PushButton connect to quit(). */
		QPushButton *pb_delete;					/*!< A PushButton connect to del_db(). */
		QPushButton *pb_reset;					/*!< A PushButton connect to clear(). */
		QListBox *lb_result;						/*!< A ListBox shows a result filename list. */
		QLabel *lbl_blank;						/*!< A label shows 'Equilibrium Project Result:'. */
		QLabel *lbl_instr;						/*!< A label shows a instruction. */
		QLabel *lbl_item;					 		/*!< A label shows selected item. */
		QLabel *lbl_investigator;				/*!< A Label shows selected investigator Information. */
		QLabel *lbl_runrequest;					/*!< A Label shows selected runrequest Information. */
		QProgressDialog *pd;
		QProcess *compress_proc;

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

		vector <QString> parameter_name;

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

		QString fileName, parameterFile, projectName, htmlDir, baseName, dirName, indexFile;
		QString *item_projectName;
		int investigatorID;
		int runrequestID;
		int parameters;
		int montecarloID, projectID;
		int *item_projectID;
		int modelID[15];
		
		int c_step; // keeps track of current compression process
		bool from_HD, from_DB, query_flag, c_flag; //c_flag keeps track of problems during compression
		
		public slots:
		
		void setup_GUI();
		void sel_investigator();
		void update_investigator_lbl(QString, int);
		void sel_runrequest();
		void update_runrequest_lbl(int );
		void load_HD();
		void load_DB();
		void select_result(int);
		void display();
		void save_db();
		void check_permission();
		void delete_db(bool);
		void clear();
		void reset();
		void help();
		void quit();
		void closeEvent( QCloseEvent *e);
		bool insertCompressData();
		void endCompressProcess();
		void showOutput();
		void retrieve();
		void clearTmpDir();

};

#endif
