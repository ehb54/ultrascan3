//
// C++ Implementation: us_enter_dna
//
// Description: 
//
//
// Author: Borries Demeler <demeler@biochem.uthscsa.edu>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef US_ENTER_DNA_DB_H
#define US_ENTER_DNA_DB_H

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
#include "us_db.h"
#include "us_db_tbl_investigator.h"



//! An Interface for Entering Nucleotide Sequence data with Database.
/*!
	This interface can be found in "Database->Commit Data to DB->Nucleotide Sequence".
	This interface can be used to enter DNA or RNA, and to provide vbar and extinction
	coefficients at 260 and 280 nm. Sequences can be saved both to the database or to the harddrive.
	When backing up data to the database, the program will save the sequence data to database table: tblDNA.
*/

class US_EXTERN US_Enter_DNA_DB : public US_DB
{
	Q_OBJECT
	public:
		US_Enter_DNA_DB(int InvID, QWidget *parent=0, const char *name=0);
		~US_Enter_DNA_DB();
		
		QLabel *lbl_bar;
		QLabel *lbl_description;
		QLabel *lbl_sequence;
		QLabel *lbl_vbar;
		QLabel *lbl_e260;
		QLabel *lbl_e280;
		QLineEdit *le_description;
		QLineEdit *le_vbar;
		QLineEdit *le_e260;
		QLineEdit *le_e280;
		QTextEdit *le_sequence;
		QPushButton *pb_save_HD;
		QPushButton *pb_save_DB;
		QPushButton *pb_help;
		QPushButton *pb_close;
	
		int InvID;
		QString filename, description, sequence;
		float vbar, e280, e260;
	
	protected slots:
		void update_description(const QString &);
		void update_sequence();
		void update_vbar(const QString &);
		void update_e280(const QString &);
		void update_e260(const QString &);
		void save_HD();
		void save_DB();
		void help();
		void quit();
};

#endif
