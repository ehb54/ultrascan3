#ifndef US_CLIPDATA_H
#define US_CLIPDATA_H

#include <QtGui>

#include "us_widgets_dialog.h"
#include "us_help.h"

class US_ClipData : public US_WidgetsDialog
{
	Q_OBJECT
	
	public:
		US_ClipData( double&, double&, double, float, 
            QWidget* = 0, Qt::WindowFlags = 0 );

   private:
      double&     conc;
      double&     rad;

      QwtCounter* ct_conc;
		QwtCounter* ct_rad;

      US_Help     showhelp;

	public slots:
      void ok  ( void );
		void help( void )
        { showhelp.show_help( "manual/equil2.html" ); };	
};

#endif

