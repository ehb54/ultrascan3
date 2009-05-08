#ifndef US_WIN_H
#define US_WIN_H

#include "us_extern.h"
#include "us_widgets.h"

#include <qwidget.h>
#include <qprocess.h>
#include <qstring.h>
#include <qlabel.h>
#include <qevent.h>

class US_EXTERN UsWin : public US_Widgets
{
   Q_OBJECT

public:

   UsWin( QWidget* parent = 0, const char* name = 0 );
   ~UsWin() {};

private:
   QLabel*                 splash_b;
   QLabel*                 bigframe;
   QLabel*                 smallframe;

   QLabel*                 stat_bar;

   void make_splash( int );

private slots:
   void display_help( int );
  void data_control( int );
  void launch      ( int );

   void quit        ( void );
   void about       ( void );
   void credits     ( void );
   void close_splash( void );

   void export_V    ( void );
   void export_E    ( void );

   void print_V     ( void );
   void print_E     ( void );
   
   void report_V    ( void );
   void report_E    ( void );
   void report_EP   ( void );
   void report_MC   ( void );

   void closeAttnt ( QProcess*, QString );
   void closeEvent ( QCloseEvent* );
   void resizeEvent( QResizeEvent* );

signals:
  void explain( const QString& );
};

#endif // USWIN_H

