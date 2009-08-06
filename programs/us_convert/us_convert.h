#ifndef US_CONVERT_H
#define US_CONVERT_H

#include <QtGui>

#include "us_widgets.h"
#include "us_help.h"
#include "us_dataIO.h"

class US_Convert : public US_Widgets
{
	Q_OBJECT

	public:
		US_Convert();

	private:

      US_Help       showHelp;
    
      QString       runType;

      QLineEdit*    le_dir;

      QComboBox*    cb_cell;
      QComboBox*    cb_channel;
      QComboBox*    cb_wavelength;

      QPushButton*  pb_write;
      QPushButton*  pb_writeAll;

      QTextEdit*    te_data;

      QList< beckmanRaw > legacyData;      

      int  write           ( const QString& );
      void setInterpolated ( unsigned char*, int );

	private slots:
      void load     ( void );
      void reset    ( void );

      void write    ( void );



      void writeAll ( void );
		void help     ( void )
      { showHelp.show_help( "manual/us_convert.html" ); };
};
#endif
