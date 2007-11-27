#ifndef US_RTVREQUEST_H
#define US_RTVREQUEST_H

#include <qpushbutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qfiledialog.h>
#include <qtextstream.h>
#include <qfiledialog.h>
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <qsqlcursor.h>
#include <qlistbox.h>
#include <qtextedit.h>

#include "us_db_rtv_investigator.h"

//! A Struct for storing required info for project request in DB.
/*! 
	Each element is according to an entry field in Database Table tblRequest.
*/

struct US_Request
{
	int RequestID;				/*!< Table ID of request info. */
	int InvID;					/*!< Table ID of investigator. */
	QString Goal;			
	QString Molecule;
	QString Purity;
	QString Expense;
	QString Buffer;
	QString Salt;
	QString AUC;
	QString Note;
	QString Description;			

};

class US_EXTERN US_RequestStatus : public US_DB
{
	Q_OBJECT
	
	public:
		US_RequestStatus(int temp_RID, QWidget *p = 0, const char *name = "us_requeststatus");
		~US_RequestStatus();
		
		QLabel *lbl_title;
		QLabel *lbl_s1;
		QLabel *lbl_s2;
		QLabel *lbl_s3;
		QLabel *lbl_s4;
		QLabel *lbl_s5;
		QLabel *lbl_s6;
		QLabel *lbl_s7;
		QCheckBox *cb1;
		QCheckBox *cb2;
		QCheckBox *cb3;
		QCheckBox *cb4;
		QCheckBox *cb5;
		QCheckBox *cb6;
		QCheckBox *cb7;
		QPushButton *pb_update;
		QPushButton *pb_cancel;
		QString Status;
		int RID;
		bool selected;
		
	protected slots:
		void select_cb1();
		void select_cb2();
		void select_cb3();
		void select_cb4();
		void select_cb5();
		void select_cb6();
		void select_cb7();
		void update_status();
		void quit();
	
	signals:
		void statusChanged(QString Status);
};

class US_EXTERN US_DB_RtvRequest : public US_DB_RtvInvestigator
{
	Q_OBJECT
	
	public:
		US_DB_RtvRequest(QWidget *p=0, const char *name="us_rtvrequest");
		~US_DB_RtvRequest();
		
		struct US_Request req_info;
		int *item_RequestID;
		QPushButton *pb_edit;
		QPushButton *pb_status;
	protected slots:
		void checkRequest();	
	   void check_request(int);
	   void select_request(int);
		void check_status();
		void edit_notes();
		void show_info();	
		void get_GUI();
	   void setup_GUI();
	};

#endif


