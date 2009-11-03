#ifndef US_PROGRESSBAR_H
#define US_PROGRESSBAR_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_help.h"

class US_EXTERN US_Progressbar : public US_WidgetsDialog
{
  Q_OBJECT

   public:
      US_Progressbar( void );
      US_Progressbar( int, int, int );
      void setValue ( int );
      void setRange ( int, int );
      void setLegend( QString legend );

   signals:
      void cancelConvertOperation( void );

   private:
      QLabel*              lb_progress;
      QProgressBar*        progress;

      US_Help              showHelp;
   
   private slots:
      void reset           ( void );
      void cancel          ( void );
      void help            ( void )
        { showHelp.show_help( "manual/us_convert.html" ); };
};
#endif
