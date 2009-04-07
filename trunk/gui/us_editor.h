#ifndef US_EDITOR_H
#define US_EDITOR_H

#include <QtGui>
#include "us_extern.h"

class US_EXTERN US_Editor : public QMainWindow
{
	Q_OBJECT

   public:
	
      US_Editor( int, bool = false, const QString& = "Data Files (*.dat)", 
            QWidget* = 0, Qt::WindowFlags = 0 );

      enum { LOAD, DEFAULT };

	   QTextEdit* e;
   
   private:

	   QFont      currentFont;
      QString    filename;
      QString    file_extention;
	
      void saveFile   ( void );

   private slots:
	   void load       ( void );
	   void save       ( void );
	   void saveAs     ( void );
	   void print      ( void );
	   void clear      ( void ) { e->clear(); };
      void update_font( void );
};
#endif
