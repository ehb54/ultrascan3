#ifndef US_DB_RST_VELOC_H
#define US_DB_RST_VELOC_H


#include "us_dtactrl.h"
#include "us_db.h"
#include "us_db_rst_cell.h"
#include "us_db_runrequest.h"
#include <qprogressdialog.h>
#include <qwidget.h>

class US_EXTERN US_DB_RST_Veloc : public US_DB
{
	Q_OBJECT
	
	public:
	
		US_DB_RST_Veloc(QWidget *p=0, const char *name=0);
		~US_DB_RST_Veloc();
		
		struct US_ExpResult exp_rst;		/*!< A struct US_ExpRsesult for storing experimental data result. */
		QString run_id;
		int source_type;						/*! < 0 means from Hard Drive, 1 means from Database. */
		bool load_flag, query_flag, cell_flag, del_flag, runinf_flag, vhw_flag, mw_flag;
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
		QPushButton *pb_mw;
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
		QLabel *lbl_VHW;							/*!< A label shows 'VHW Combined Distribution'. */
		QLabel *lbl_VHW1;							/*!< A label shows 'VHW Combined Distribution Plot'. */
		QLabel *lbl_vhw1;							/*!< A label shows the vhw plot available or not. */
		QLabel *lbl_VHW2;							/*!< A label shows 'VHW ASCII file fot plot data'. */
		QLabel *lbl_vhw2;							/*!< A label shows the vhw file available or not. */
		QLabel *lbl_VHW3;							/*!< A label shows 'Text Listing for including Data sets'. */
		QLabel *lbl_vhw3;							/*!< A label shows the vhw text list available or not. */
		QLabel *lbl_MW;							/*!< A label shows 'MW Distribution'. */
		QLabel *lbl_MW1;							/*!< A label shows 'MW Distribution Plot'. */
		QLabel *lbl_mw1;							/*!< A label shows the mw plot available or not. */
		QLabel *lbl_MW2;							/*!< A label shows 'MW ASCII file fot plot data'. */
		QLabel *lbl_mw2;							/*!< A label shows the mw file available or not. */
		QLabel *lbl_MW3;							/*!< A label shows 'Text Listing for including Data sets'. */
		QLabel *lbl_mw3;							/*!< A label shows the mw text list available or not. */
		QLabel *lbl_dataset;							/*!< A label shows 'Velocity Result Dataset'. */
		QLabel *lbl_Cell;							/*!< A label shows 'Cell Result Data'. */
		QLabel *lbl_cell;							/*!< A label shows 'Cell :' */
		QComboBox *cmbb_cell;					/*!< A ComboBox for listing Cell item. */
		
		QProcess *compress_proc;
		int c_step;
		int border, spacing, xpos, ypos, X, Y, Xpos, Ypos, buttonw, buttonh;
		US_DB_RST_Cell *ct;
		Data_Control_W *data_control;
		QString fileName, baseName, htmlDir, printerFile, residual_file, overlay_file;
		int *item_ExpdataID;
		QString *item_Description, *display_Str;
		int EDID[8][4], VHWID[8][4], FEID[8][4], CSID[8][4],SMID[8][4], TDID[8][4], RDID[8][4];
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
		void show_vhw();
		void show_mw();
		void show_dataset();
		void save_db();
		int save_db_expdata(int);
		int save_db_vhw(int);
		int save_db_fe(int);
		int save_db_cs(int);
		int save_db_sm(int);
		int save_db_td(int);
		int save_db_rd(int);
		void del_subTable();
		int insert_upTable(QString, QString, int *);
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
