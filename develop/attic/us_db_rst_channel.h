#ifndef US_DB_RST_CELL_H
#define US_DB_RST_CELL_H

#include "us_db.h"
#include <qtabwidget.h>
#include <qlayout.h>

class US_EXTERN US_DB_RST_Channel_Layout : public US_DB
{
	Q_OBJECT
	
	public:
	US_DB_RST_Channel_Layout(int source_type, QWidget *parent, const char *name = 0);
		~US_DB_RST_Channel_Layout();
	int source_type;
	QString display_Str;
	QLabel *lbl_DBID;
	QLabel *lbl_dbid;
	QLabel *lbl_channel;
	QLabel *lbl_source;
	QLabel *lbl_label1;
	QLabel *lbl_label2;
	QLabel *lbl_label3;
	QLabel *lbl_label4;
		
	QCheckBox *cb_lbl1;
	QCheckBox *cb_lbl2;
	QCheckBox *cb_lbl3;
	QCheckBox *cb_lbl4;
	
	QPushButton *pb_display;
	
	public slots:
	void get_dbid(int);
	void get_display(QString);
	void display();
	
	
};

class US_EXTERN US_DB_RST_Channel : public US_DB
{
	Q_OBJECT
	
	public:
	
		US_DB_RST_Channel(QString run_id, QString text, int source_type, QWidget *p=0, const char *name=0);
		~US_DB_RST_Channel();
		
		int run_type, source_type, cell, cellid, wl, scans;
		
		QString run_id, text, htmlDir, baseName, description, expdata_file, display;
		int EDID[4], Pepid[4], Bufid[4];
		QGridLayout *channelGrid;
		QTabWidget *channelTab;
		US_DB_RST_Channel_Layout *channel1, *channel2, *channel3, *channel4;
		
	public slots:	
		void read_HD_expdata(int);
		void read_DB_expdata(int);
};

#endif
