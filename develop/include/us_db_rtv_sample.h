#ifndef US_RTVSAMPLE_H
#define US_RTVSAMPLE_H

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

#include "us_db_rtv_investigator.h"
#include "us_db_tbl_buffer.h"
#include "us_db_tbl_vbar.h"
#include "us_db_tbl_nucleotide.h"
#include "us_db_rtv_image.h"


class US_EXTERN US_DB_Sample : public US_DB
{
	Q_OBJECT
	
	public:
		US_DB_Sample(int temp_sampleID, QWidget *p = 0, const char *name = "us_db_sample");
		~US_DB_Sample();
		
		QLabel *lbl_sample;
		QLabel *lbl_description;
		QLabel *lbl_STemp;
		QLabel *lbl_RTemp;
		QLabel *lbl_vbar;
		QLabel *lbl_note;
						
	// for display value
		QLineEdit *le_sample;
		QLineEdit *le_buff;
		QLineEdit *le_STemp;
		QLineEdit *le_RTemp;
		QLineEdit *le_vbar;
		QLineEdit *le_pep;
		QLineEdit *le_dna;
		QLineEdit *le_image;
		QLineEdit *le_note;
	
	//show detail
		QPushButton *pb_buff;
		QPushButton *pb_pep;
		QPushButton *pb_dna;
		QPushButton *pb_image;
		
		QPushButton *pb_help;
		QPushButton *pb_close;
	
		int BufferID, PeptideID, DNAID, ImageID, InvID;
		
	protected slots:
		void load(int);
		void view_buff();
		void view_pep();
		void view_DNA();
		void view_image();
		void help();
		void quit();
	

};

class US_EXTERN US_DB_RtvSample : public US_DB_RtvInvestigator
{
	Q_OBJECT
	
	public:
		US_DB_RtvSample(QWidget *p=0, const char *name="us_rtvsample");
		~US_DB_RtvSample();
		
		int *item_SampleID;
		int SampleID;
	protected slots:
		void checkSample();	
	   void check_sample(int);
	   void select_sample(int); 
		void show_info();	
	};

#endif


