#ifndef US_KIRKWOOD_H
#define US_KIRKWOOD_H

#include "us_util.h"
#include <qapplication.h>
#include <qradiobutton.h>

#include <qwt_counter.h>


extern int global_Xpos;
extern int global_Ypos;

class US_EXTERN US_Kirkwood : public QFrame
{
	Q_OBJECT
	
	public:
		US_Kirkwood( QWidget *parent=0, const char *name=0 );
		~US_Kirkwood();
		US_Config *USglobal;

	private:
		bool sample1;
		bool sample2;
		float sample3;
		float sample4;
		QLabel *lbl_banner;
		QLabel *lbl_sample1;
		QLabel *lbl_sample2;
		QLabel *lbl_sample3;
		QLabel *lbl_sample4;
		QwtCounter *ct_sample3;
		QLineEdit *le_sample4;
		QLabel *lbl_blank1;
		QRadioButton *bt_sample1;
		QRadioButton *bt_sample2;
		QPushButton *pb_help;
		QPushButton *pb_close;

	private slots:
		void quit();
		void help();
		void update_sample3(double);
		void update_sample4(const QString &val);
		void update_sample1();
		void update_sample2();

	protected slots:
		void closeEvent(QCloseEvent *e);
};

#endif

