#ifndef US_VBAR_DB_H
#define US_VBAR_DB_H

#include <qlistbox.h>
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <qsqlcursor.h>
#include <qfiledlg.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qstring.h>

#include "us_editor.h"
#include "us_vbar.h"
#include "us_db.h"
#include "us_db_tbl_investigator.h"


class US_EXTERN US_Enter_Vbar_DB : public US_DB
{
	Q_OBJECT
	public:
		US_Enter_Vbar_DB(int InvID, QWidget *parent=0, const char *name=0);
		~US_Enter_Vbar_DB();
		
	QLabel *lbl_bar;
	QLabel *lbl_description;
	QLabel *lbl_sequence;
	QLabel *lbl_vbar;
	QLabel *lbl_e280;
	QLineEdit *le_description;
	QLineEdit *le_vbar;
	QLineEdit *le_e280;
	QTextEdit *le_sequence;
	QPushButton *pb_save_HD;
	QPushButton *pb_save_DB;
	QPushButton *pb_help;
	QPushButton *pb_close;
	
	int InvID;
	QString filename, description, sequence;
	float vbar, e280;
	
	protected slots:
		void update_description(const QString &);
		void update_sequence();
		void update_vbar(const QString &);
		void update_e280(const QString &);
		void save_HD();
		void save_DB();
		void help();
		void quit();
		
};

//! An Interface for Peptide data with Database.
/*!
	This interface can be found in "Database->Commit Data to DB->Peptide".
	You can use this interface to pickup your peptide values.
	When you Save Peptide to HD, the program will save your peptide data to hard drive.
	When you Backup Peptide to DB, the program will save your peptide data to database table: tblPeptide.
	Also you can use this interface to load your peptide data records from hard drive or DB.
*/
class US_EXTERN US_Vbar_DB : public US_DB 
{
	Q_OBJECT
	public:
		US_Vbar_DB(float temp, float *temp_vbar, float *temp_vbar20, bool temp_GUI, bool from_cell, int temp_InvID,
				  QWidget *parent=0, const char *name="us_vbar_db");
		~US_Vbar_DB();
	
		struct US_VbarData vbar_info;			/*!< A struct US_VbarData for storing Peptide Data. */
		struct peptideDetails Vbar;			/*!< A struct peptideDetails for exporting Peptide Data. */
//		US_Vbar *us_vbar;							/*!< Instantiation of non-GUI VBAR class. */
		struct peptide pep;
		int *item_PepID;							/*!< A Integer Array store Peptide ID.*/
		int mySize_x, mySize_y;					/*!< Integer values for x, y size of frame's geometry*/
		QString *item_Description, 			/*!< A String Array store Peptide description.*/
				  *display_Str;					/*!< A String Array store peptide name dispaly.*/
		int Item;									/*!< A Integer variable for ListBox item select. */
		bool GUI,									/*!< A flag for deciding to display the interface or not. */
			  select_flag, 						/*!< A flag for query listbox select, initialize to <var>false</var>. */
			  cell_flag, 							/*!< A flag for checking this wigdet whether is called from US_Cell table or not. */
			  from_HD,								/*!< A flag for loading Peptide data whether is from Hard Drive or not. */
			  sequence_loaded;					/*!< A flag for checking *.pep file is read or not. */
		
		QPushButton *pb_enter;					/*!< A PushButton connect to enter_pep(). */
		QPushButton *pb_load_db;				/*!< A PushButton connect to read_db(). */
		QPushButton *pb_save_db;				/*!< A PushButton connect to save_db(). */
		QPushButton *pb_del_db;					/*!< A PushButton connect to delete_db(). */
		QPushButton *pb_reset;					/*!< A PushButton connect to reset(). */
		QPushButton *pb_filename;				/*!< A PushButton connect to read_file(). */
		QPushButton *pb_download;				/*!< A PushButton connect to download(). */
		QPushButton *pb_sequence;				/*!< A PushButton connect to show_sequence(). */
		QPushButton *pb_help;					/*!< A PushButton connect to help(). */
		QPushButton *pb_ok;						/*!< A PushButton connect to ok(). */
		QPushButton *pb_info;					/*!< A PushButton connect to info(). */
		QPushButton	*pb_investigator;			/*!< A PushButton connect to sel_investigator(). */
		QString filename, 						/*!< A string variable for storing raw peptide file's path and name. */
				  res_file, 						/*!< A string variable for storing readable peptide file's path and name. */
				  description;						/*!< A string variable for peptide description. */
		US_Editor *e;								/*!< A reference of US_Editor. */
		QListBox *lb_vbar;						/*!< A ListBox shows a peptide filename list. */
		QLineEdit *le_temperature;				/*!< A LineEdit for temperature input. */
		QLabel *instr_lbl;						/*!< A Label shows 'Doubleclick on buffer data to select:'. */
		QLabel *lbl_investigator;				/*!< A Label shows selected investigator Information. */
		QLabel *lbl_file1;						/*!< A Label shows 'Selected File:'. */
		QLabel *lbl_file2;						/*!< A Label shows selected peptide filename. */
		QLabel *lbl_desc1;						/*!< A Label shows 'Protein Description:'. */
		QLabel *lbl_desc2;						/*!< A Label shows selected peptide description. */
		QLabel *lbl_temp1;						/*!< A Label shows 'Temperature (in ºC):'. */
		QLabel *lbl_vbar1;						/*!< A Label shows 'vbar (temperature):'. */
		QLabel *lbl_vbar2;						/*!< A Label shows selected vbar value. */
		QLabel *lbl_vbar20_1;					/*!< A Label shows 'vbar (20 ºC):'. */
		QLabel *lbl_vbar20_2;					/*!< A Label shows selected vbar20 value. */
		QLabel *lbl_e280_1;						/*!< A Label shows .'E280 (denatured):'. */
		QLabel *lbl_e280_2;						/*!< A Label shows selected E280 value. */
		QLabel *lbl_resid1;						/*!< A Label shows '# of Residues:'. */
		QLabel *lbl_resid2;						/*!< A Label shows selected residues value. */
		QLabel *lbl_mw1;							/*!< A Label shows 'Molecular Weight:'. */
		QLabel *lbl_mw2;							/*!< A Label shows selected molecular weight value. */
		float temperature,						/*!< A float variable for temerature value.*/
				t1,									/*!< A float variable for argument <var>temp</var> value.*/
				t2,									/*!< A float variable for argument <var>*temp_vbar</var> value.*/
				t3;									/*!< A float variable for argument <var>*temp_vbar20</var> value.*/
		int border, 								/*!< An integer variable for distance between widget's elements and border. */
			 spacing, 								/*!< An integer variable for distance between of widget's elements. */
			 xpos, 									/*!< An integer variable for widget's x position. */
			 ypos,									/*!< An integer variable for widget's y position. */
			 buttonw, 								/*!< An integer variable for button width. */
			 buttonh;								/*!< An integer variable for button height. */

		struct peptideDetails export_vbar(int id);
		US_Enter_Vbar_DB *enterPepDlg;

	public slots:
		void retrieve_vbar(int);
		void result_output(QString);
	protected slots:
		
		void setup_GUI();
		void setup_GUI2();
		void update_temp(const QString &str);
		void update_vbar();
	//pushbuttons:
		void read_file();
//		QString get_sequence(QByteArray);
//		void result_output(QString);
		void enter_pep();
		void read_db();
		void save_db();
		void check_permission();
		void delete_db(bool);
		void clear();
		void reset();
		void help();
		void download();
		void ok();
		void info();
		void sel_investigator();
		void update_investigator_lbl(QString, int );
		void show_sequence();
	//ListBoxs:
		void select_vbar(int item);
	//others:
		void closeEvent(QCloseEvent *e);

	signals:
/*!
	This signal is emitted whenever the widget is called by US_Cell_DB.\n
	The argument is integer <var>PepID</var>.
*/
		void idChanged(int PepID);
/*!
	This signal is emitted in function read_file() and select_vbar().\n
	The argument is float variable <var>pep.vbar</var> and <var>pep.vbar20</var>.
*/
		void valueChanged(float vbar, float vbar20);
/*!
	This signal is emitted in function read_file() and select_vbar().\n
	The argument is float variable <var>pep.e280</var>.
*/
		void e280Changed(float e280);
/*!
	This signal is emitted in function read_file() and select_vbar().\n
	The argument is float variable <var>pep.mw</var>.
*/
		void mwChanged(float mw);
};

#endif

