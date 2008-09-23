#ifndef US_WIDGETS_H
#define US_WIDGETS_H

#include <qlabel.h>
#include <qpushbutton.h>
#include <qtextedit.h>
#include <qcheckbox.h>
#include <qpopupmenu.h>
#include <qmenubar.h>

#include "us_util.h"

class US_EXTERN US_Widgets : public QFrame
{
	public:	
		US_Widgets ( QWidget* p = 0, const char* name = 0 );
		~US_Widgets( void ) {};

		US_Config* USglobal;
		
		QLabel*      label     ( const QString&, int = 0,  int = QFont::Normal );
		QLabel*      textlabel ( const QString&, int = -1, int = QFont::Normal );
		QLabel*      banner    ( const QString&, int = 0,  int = QFont::Bold );
		QPushButton* pushbutton( const QString&, bool = true ); 
		QTextEdit*   textedit  ( void );
 		QCheckBox*   checkbox  ( const QString&, bool = false );
 		QPopupMenu*  popupmenu ( int = -1 );
 		QMenuBar*    menubar   ( int = -1, int = QFont::Bold );
};
#endif

