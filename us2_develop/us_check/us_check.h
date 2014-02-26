#ifndef US_CHECK_H
#define US_CHECK_H

#include <qpushbutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qfiledialog.h>
#include <qcombobox.h>
#include <qfiledialog.h>
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <qsqlcursor.h>
#include <qmessagebox.h>
#include <qlistbox.h>
#include <qtextedit.h>
#include <qmenubar.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qstatusbar.h>
#include <qpaintdevicemetrics.h>

#include <vector>

struct LCinfo
{
	int rid;
	QString firstname;
	QString lastname;
	QString organization;
	QString address;
	QString city;
	QString state;
	QString zip;
	QString phone;
	QString email;
	QString mlist;
	QString lctype;
	QString platform;
	QString os;
	QString key;
	QString approved;
	QString expiration;
	QString version;
	int lid;
};

struct DBinfo
{
	QString username;
	QString password;
	QString host;
};

class Confirm : public QDialog 
{
	Q_OBJECT
	public:
		Confirm(QWidget *parent=0, const char *name=0);
		~Confirm();
		struct DBinfo db_info;
		signals:
		void textChanged(const QString &);
	private slots:
		void update_username(const QString &);
		void update_password(const QString &);
		void update_host(const QString &);
		int db_connect();
		void accept();
	private:
		QLabel *lbl_bar;
		QPushButton *ok;
		QPushButton *cancel;
};

class US_Check : public QFrame
{
	Q_OBJECT
	
	public:
		US_Check (QWidget *p=0, const char *name="us_check");
		~US_Check();
		
		struct LCinfo info_list;
		bool input_flag, sel_key, sel_name, show_flag, export_flag;
		int input_type, RID, Rid, Lid, Ownerid;
		QString input, Key;
		Confirm *confirmDlg;
		QLabel *lbl_blank;
		QLabel *lbl_info;
		QLabel *lbl_type; 
		QLabel *lbl_input;
		QLabel *lbl_instr1;
		QLabel *lbl_instr2;
		QLabel *lbl_license;
		QLabel *lbl_item;
		QLabel *lbl_firstname;
		QLabel *lbl_fname;
		QLabel *lbl_lastname;
		QLabel *lbl_lname;
		QLabel *lbl_organization;
		QLabel *lbl_org;
		QLabel *lbl_address;
		QLabel *lbl_addr;
		QLabel *lbl_city;
		QLabel *lbl_ct;
		QLabel *lbl_state;
		QLabel *lbl_st;
		QLabel *lbl_zipcode;
		QLabel *lbl_zip;
		QLabel *lbl_telephone;
		QLabel *lbl_phone;
		QLabel *lbl_emailaddr;
		QLabel *lbl_email;
		QLabel *lbl_platform;
		QLabel *lbl_os;
		QLabel *lbl_licensetype;
		QLabel *lbl_lctype;
		QLabel *lbl_version;
		QLabel *lbl_vs;
		QLabel *lbl_expiration;
		QLabel *lbl_exp;
		QLabel *lbl_mailinglist;
		QLabel *lbl_mlist;
		QLabel *lbl_approved;
		QLabel *lbl_apprv;
		
				
		QComboBox *cmbb_type;
		QListBox *lb_key;
		QPushButton *pb_check;
		QPushButton *pb_chkLC;	
		QPushButton *pb_chkID;
		QPushButton *pb_reset;
		QPushButton *pb_close;
		QPushButton *pb_delete;
		QPushButton *pb_approve;
		QPushButton *pb_license;
		QPushButton *pb_text;	
		QLineEdit *le_input;
					
	private slots:

// pushbuttons:
		void checklicense();
		void show_all();
		void reset();
		void del();
		void approve();
		void show_info();
		void license_show();
		void text_show();
		void quit();
// lineedits:
		void update_input(const QString &);
		void sel_type(int item);
	
// Listbox:
		void select_name(int item);
		void select_key(int item);
// other:
		void clear();
		
};


class TextShow : public QWidget
{
	Q_OBJECT
	
public:
    TextShow(struct LCinfo info_list, int type, QWidget *parent=0, const char *name=0 );
   ~TextShow();

public slots:
	void load(struct LCinfo, int type);
	void save();
	void print();
   void closeDoc();

protected:
    void resizeEvent( QResizeEvent * );

private:
    QMenuBar	*m;
    QTextEdit	*e;
    QPrinter	printer;
};

#endif
