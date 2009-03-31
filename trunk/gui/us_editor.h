#ifndef US_EDITOR_H
#define US_EDITOR_H

#include <QtGui>
#include "us_extern.h"

class US_EXTERN US_Editor : public QMainWindow
{
	Q_OBJECT

   public:
	
      US_Editor( int, bool = false, QWidget* = 0, Qt::WindowFlags = 0 );

	   QTextEdit* e;
   
   private:

	   QFont      currentFont;
      QString    filename;
	
   private slots:
	   void saveAs     ( void );
	   void print      ( void );
	   void clear      ( void ) { e->clear(); };
      void update_font( void );
};
#endif
