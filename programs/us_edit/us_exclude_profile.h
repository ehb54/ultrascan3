#ifndef US_EXCLUDE_PROFILE_H
#define US_EXCLUDE_PROFILE_H

#include <QtGui>
#include <qwt_counter.h>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_help.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

class US_ExcludeProfile : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      US_ExcludeProfile( QList< int > );

   signals:
      void update_exclude_profile( QList< int > );
      void finish_exclude_profile( QList< int > );
      void cancel_exclude_profile( void );

   private:
      QwtCounter*  ct_start;
      QwtCounter*  ct_stop;
      QwtCounter*  ct_nth;
                  
      QLineEdit*   le_remaining;
      QLineEdit*   le_excluded;

      int          dbg_level;
      bool         finished;
      QList< int > original;
      QList< int > excludes;
      QList< int > current;
      
      US_Help      showHelp;

   private slots:
      void update      ( double=0.0 );
      void update_start( double );
      void update_stop ( double );
      void terminate   ( void );
      void apply       ( void );
      void done        ( void );
      void reset       ( void );
      void help        ( void )
      { showHelp.show_help( "manual/edit_exclude_profile.html" ); };
};
#endif
