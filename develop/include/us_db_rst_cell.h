#ifndef US_DB_RST_CELL_H
#define US_DB_RST_CELL_H

#include "us_db.h"
#include <qtabwidget.h>
#include <qcombobox.h>
#include <qlistbox.h>
#include <qlayout.h>

class US_EXTERN US_DB_RST_Cell_Layout : public US_DB
{
	Q_OBJECT
	
	public:
	US_DB_RST_Cell_Layout(int source_type, QWidget *parent, const char *name = 0);
		~US_DB_RST_Cell_Layout();
	//US_Config *USglobal;
	int source_type;
	QString display_Str;
	QLabel *lbl_DBID;
	QLabel *lbl_dbid;
	QLabel *lbl_channel;
	QLabel *lbl_source;
	QLabel *lbl_report;
	QLabel *lbl_label1;
	QLabel *lbl_label2;
	QLabel *lbl_label3;
	QLabel *lbl_label4;
	QLabel *lbl_label5;
	QLabel *lbl_label6;
	
	QCheckBox *cb_report;
	QCheckBox *cb_lbl1;
	QCheckBox *cb_lbl2;
	QCheckBox *cb_lbl3;
	QCheckBox *cb_lbl4;
	QCheckBox *cb_lbl5;
	QCheckBox *cb_lbl6;
	
	QPushButton *pb_display;
	QPushButton *pb_update;

	QComboBox *cmbb_channel;
	
	public slots:
	void get_dbid(QString, int);
	void get_channel(int);
	void get_label1(QString);
	void get_label2(QString);
	void get_label3(QString);
	void get_label4(QString);
	void get_label5(QString);
	void get_label6(QString);
	void get_display(QString);
	void display();
	
	
};

class US_EXTERN US_DB_RST_Cell : public US_DB
{
	Q_OBJECT
	
	public:
	
		US_DB_RST_Cell( QString run_id, QString text, int source_type, QWidget *p=0, const char *name=0);
		~US_DB_RST_Cell();
		
		int source_type, cell, cellid, Num_Channel, wl, scans;
		QString run_id, text, htmlDir, baseName, description, display;
		QString expdata_file, vhw_file, cs_file, fe_file, sm_file, td_file, rd_file;
		int EDID[4], VHWID[4], CSID[4], FEID[4], SMID[4], TDID[4], RDID[4], Pepid[4], Bufid[4];
		QGridLayout *cellGrid;
		QTabWidget *cellTab;
		US_DB_RST_Cell_Layout *l_expdata, *l_vhw, *l_cs, *l_fe, *l_sm, *l_td, *l_rd;
		
	public slots:	
		void read_HD_expdata();
		void read_HD_vhw();
		void read_HD_cs();
		void read_HD_fe();
		void read_HD_sm();
		void read_HD_td();
		void read_HD_rd();
		void read_DB_expdata();
		void read_DB_vhw();
		void read_DB_cs();
		void read_DB_fe();
		void read_DB_sm();
		void read_DB_td();
		void read_DB_rd();
	//	QString make_temp_htmlFile(QString);
	//	QString make_db_tempfile(QString);
	//	bool read_blob(QString, QSqlCursor, QString);
		void clear();
									
};

#endif
