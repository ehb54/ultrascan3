#ifndef US_NUCLEOTIDE_H
#define US_NUCLEOTIDE_H

#include <qlabel.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qframe.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qwt_counter.h>
#include "us.h"
#include "us_util.h"


class US_EXTERN US_Nucleotide : public QFrame
{
	Q_OBJECT
	
	public:
		US_Nucleotide(QWidget *p=0, const char *name = 0);
		~US_Nucleotide();

		QLabel *lbl_banner1;
		QLabel *lbl_banner2;
		US_Config *USglobal;
		QCheckBox *cb_doubleStranded;
		QCheckBox *cb_3prime_oh;
		QCheckBox *cb_3prime_po4;
		QCheckBox *cb_5prime_oh;
		QCheckBox *cb_5prime_po4;
		QCheckBox *cb_DNA;
		QCheckBox *cb_RNA;
		QCheckBox *cb_complement;
		QLabel *lbl_doubleStranded;
		QLabel *lbl_complement;
		QLabel *lbl_3prime_oh;
		QLabel *lbl_3prime_po4;
		QLabel *lbl_5prime_oh;
		QLabel *lbl_5prime_po4;
		QLabel *lbl_sodium;
		QLabel *lbl_potassium;
		QLabel *lbl_lithium;
		QLabel *lbl_calcium;
		QLabel *lbl_magnesium;
		QLabel *lbl_sequence;
		QLineEdit *le_sequence;
		QLabel *lbl_mw;
		QLineEdit *le_mw;
		QLabel *lbl_DNA;
		QLabel *lbl_RNA;
		QwtCounter *cnt_sodium;
		QwtCounter *cnt_lithium;
		QwtCounter *cnt_potassium;
		QwtCounter *cnt_calcium;
		QwtCounter *cnt_magnesium;
		QPushButton *pb_load;
		QPushButton *pb_download;
		QPushButton *pb_update;
		QPushButton *pb_save;
		QPushButton *pb_help;
		QPushButton *pb_quit;
		bool doubleStranded, _3prime_oh, _5prime_oh, isDNA, complement;
		float sodium, calcium, lithium, potassium, magnesium, MW;
		unsigned int A, C, G, T, U, total;
		QString filename, title;
		
	public slots:
	
		virtual void load();
		virtual void download();
		virtual void update();
		virtual void update_doubleStranded();
		virtual void update_complement();
		virtual void update_3prime_oh();
		virtual void update_3prime_po4();
		virtual void update_5prime_oh();
		virtual void update_5prime_po4();
		virtual void update_DNA();
		virtual void update_RNA();
		virtual void update_sodium(double);
		virtual void update_potassium(double);
		virtual void update_lithium(double);
		virtual void update_calcium(double);
		virtual void update_magnesium(double);
		virtual void save();
		virtual void help();
		virtual void quit();
		
	protected slots:
		void closeEvent(QCloseEvent *);
};

#endif

