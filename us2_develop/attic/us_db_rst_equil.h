#ifndef US_DB_RST_EQUIL_H
#define US_DB_RST_EQUIL_H

#include "us_dtactrl.h"
#include "us_db.h"
#include "us_db_rst_channel.h"
#include "us_db_runrequest.h"
#include <qwidget.h>

class US_EXTERN US_DB_RST_Equil : public US_DB
{
	Q_OBJECT
	
	public:
	
		US_DB_RST_Equil(QWidget *p=0, const char *name=0);
		~US_DB_RST_Equil();
		
		struct US_ExpResult exp_rst;		/*!< A struct US_ExpRsesult for storing experimental data result. */
		int run_type, 
			 source_type;						/*! < 0 means from Hard Drive, 1 means from Database. */
		bool load_flag, query_flag, cell_flag, del_flag, runinf_flag, vhw_flag;
		QPushButton	*pb_runrequest;			/*!< A PushButton connect to sel_runrequest(). */
		QPushButton *pb_load;					/*!< A PushButton connect to load(). */
		QPushButton *pb_load_db;				/*!< A PushButton connect to read_db(). */
		QPushButton *pb_save_db;				/*!< A PushButton connect to save_db(). */
		QPushButton *pb_del_db;					/*!< A PushButton connect to del_db(). */
		QPushButton *pb_help;					/*!< A PushButton connect to help(). */
		QPushButton *pb_close;					/*!< A PushButton connect to quit(). */
		QPushButton *pb_reset;					/*!< A PushButton connect to clear(). */
		QPushButton *pb_runinfo;
		QPushButton *pb_vhw;
		QPushButton *pb_dataset;
		QListBox *lb_result;						/*!< A ListBox shows a result filename list. */
		QLabel *instr_lbl;						/*!< A label shows a instruction. */
		QLabel *lbl_expdata;						/*!< A label shows 'Experimental Data ID'. */
		QLabel *lbl_expdt;						/*!< A label shows loaded data's expdata ID. */
		QLabel *lbl_runrequest;					/*!< A Label shows selected runrequest Information. */
		QLabel *lbl_investigator;				/*!< A label shows 'Investigator ID'. */
		QLabel *lbl_invst;						/*!< A label shows loaded data's investigator ID. */
		QLabel *lbl_date;							/*!< A label shows 'Date'. */
		QLabel *lbl_dt;							/*!< A label shows loaded data's date. */
		QLabel *lbl_description;				/*!< A label shows loaded 'Description:'. */
		QLineEdit *le_dscrpt;					/*!< A LineEdit shows loaded data's description(read only). */
		QLabel *lbl_runinf;						/*!< A label shows 'Information of this run'. */
		QLabel *lbl_plot1;						/*!< A label shows 'Temp. vs. #Scan'. */
		QLabel *lbl_plt1;							/*!< A label shows the plot1 available or not. */
		QLabel *lbl_plot2;						/*!< A label shows 'Rotorspeed. vs. #Scan'. */
		QLabel *lbl_plt2;							/*!< A label shows the plot2 available or not. */
		QLabel *lbl_plot3;						/*!< A label shows 'time diff. between Scans. vs. #Scan'. */
		QLabel *lbl_plt3;							/*!< A label shows the plot3 available or not. */
		QLabel *lbl_plot4;
		QLabel *lbl_plt4;
		QLabel *lbl_plot5;
		QLabel *lbl_plt5;
		QLabel *lbl_plot6;
		QLabel *lbl_plt6;
		QLabel *lbl_Cell;							/*!< A label shows 'Cell Result Data'. */
		QLabel *lbl_cell;							/*!< A label shows 'Cell :' */
		QComboBox *cmbb_cell;					/*!< A ComboBox for listing Cell item. */

		QProcess *compress_proc;
		int c_step;
		int border, spacing, xpos, ypos, X, Y, Xpos, Ypos, buttonw, buttonh;
		US_DB_RST_Channel *ct;
		Data_Control_W *data_control;
		QString run_id, fileName, baseName, htmlDir, printerFile, residual_file, overlay_file;
		int *item_ExpdataID;
		QString *item_Description, *display_Str;
		int EDID[8][4];
		int runrequestID;

		public slots:
		
		bool insertCompressData();
		void endCompressProcess();
		void sel_runrequest();
		void update_runrequest_lbl(int);
		void load();
		void load_cell();
		void update_cell();
		void sel_cell(int);
		void show_runinfo();
		void show_dataset();
		void save_db();
		int save_db_expdata(int, int);
		void del_subTable();
		void query_db();
		void select_result(int);
		void check_permission();
		void delete_db(bool);
		void reset();
		void help();
		void quit();
		void paintEvent( QPaintEvent * );
		void closeEvent( QCloseEvent *e);
		
};

#endif
