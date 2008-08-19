#ifndef US_DB_WIDGETS_H
#define US_DB_WIDGETS_H

#include <qlabel.h>
#include <qpushbutton.h>
#include <qtextedit.h>
#include <qcheckbox.h>
#include <qlistbox.h>
#include <qprogressdialog.h> 

#include "us_db.h"
#include "us_util.h"

class US_EXTERN US_DB_Widgets : public US_DB
{
	public:	
		US_DB_Widgets ( QWidget* p = 0, const char* name = 0 );
		~US_DB_Widgets( void ) {};

		QLabel*          label         ( const QString&, int = 0,  
		                                 int = QFont::Normal );
		
		QLabel*          textlabel     ( const QString&, int = -1,
		                                 int = QFont::Normal );
		
		QLabel*          banner        ( const QString&, int = 0,
		                                 int = QFont::Bold );

		QPushButton*     pushbutton    ( const QString&, bool = true ); 
		QTextEdit*       textedit      ( void );
		QCheckBox*       checkbox      ( const QString&, bool = false );
		QListBox*        listbox       ( const QString&, bool = true );
		QProgressDialog* progressdialog( const QString&, const QString&, int );
};
#endif

