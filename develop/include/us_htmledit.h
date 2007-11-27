#ifndef US_HTMLEDIT_H
#define US_HTMLEDIT_H

#include <qlabel.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qmultilineedit.h>
#include <qframe.h>
#include <qtextedit.h>
#include <qmessagebox.h>
#include "us.h"
#include "us_util.h"


class US_htmlEdit : public QDialog
{
	Q_OBJECT
	
	public:
		US_htmlEdit(QString, int, QWidget *p=0, const char *name = 0);
		~US_htmlEdit();
		
		US_Config *USglobal;
		QLabel *lbl_info;
		int model;
		QTextEdit *mle_text;
		QPushButton *pb_cancel;
		QPushButton *pb_save;
		QPushButton *pb_help;
		QPushButton *pb_quit;
		QString fileName;
		int buttonh, buttonw, border, spacing;
		bool modified;

	public slots:
	
		void cancel();
		void save();
		void help();
		void quit();
		
	protected slots:
		void closeEvent(QCloseEvent *);
		void resizeEvent(QResizeEvent *e);
};

#endif

