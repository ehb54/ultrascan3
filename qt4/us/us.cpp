#include <QApplication>
#include <QtSingleApplication>

#include "us.h"
#include "us_global.h"
#include "us_license_t.h"
#include "us_help.h"
#include "us_gui_settings.h"
#include "us_win_data.cpp"

using namespace us_win_data;

int main( int argc, char* argv[] )
{
  QString options( getenv( "ULTRASCAN_OPTIONS" ) );
  if ( options.contains( "multiple" ) )
  {
    QApplication application( argc, argv );
    //bool license = US_License_t::isValid();
    us_win w;
    w.show();
    return application.exec();
  }
 
  QtSingleApplication application( "UltraScan", argc, argv );
  if ( application.sendMessage( "Wake up" ) ) return 0;
  application.initialize();

  bool license = US_License_t::isValid();
  qDebug() << "License valid? " << license;

  us_win w;
  w.show();
  application.setActivationWindow( &w );

  return application.exec();
}

//////////////us_action
us_action::us_action( int i, const QString& text, QObject* parent) 
    : QAction( text, parent ), index( i ) 
{
  connect( this, SIGNAL( triggered  ( bool ) ), 
           this, SLOT  ( onTriggered( bool ) ) );
}

void us_action::onTriggered( bool ) 
{
  emit indexTriggered( index );
}
/////////////us_win
us_win::us_win( QWidget* parent, Qt::WindowFlags flags )
  : QMainWindow( parent, flags )
{
  g = new US_Global();
  if ( ! g->isValid() ) 
  {
     // Do something for invalid global memory
     qDebug( "us_win: invalid global memory" );
  }

  QPoint p = g->global_position();
  setGeometry( QRect( p, p + QPoint( 710, 532 ) ) );
  //qDebug() << "Global position = " << p << endl;
  g->set_global_position( p + QPoint( 30, 30 ) );

  //p = g->global_position();
  //qDebug() << "Global position = " << p << endl;

  setWindowTitle( "UltraScan Analysis" );

  ////////////
  QMenu* file = new QMenu( "&File", this );

  addMenu(  P_CONFIG, "&Configuration", file );
  addMenu(  P_ADMIN , "&Administrator", file );
  addMenu(  4       , "E&xit"         , file );

  QMenu* type1 = new QMenu( tr( "&VeloityData" ), file );
  addMenu( 21, tr( "&Absorbance Data"     ), type1 );
  addMenu( 22, tr( "&Interference Data"   ), type1 );
  addMenu( 23, tr( "&Fluorescense Data"   ), type1 );
  addMenu( 24, tr( "&Edit Cell ID's Data" ), type1 );

  QMenu* edit = new QMenu( "&Edit", this );
  edit->addMenu( type1 );
  addMenu( 12, "&Equilibrium Data"    , edit );
  addMenu( 13, "Edit &Wavelength Data", edit );
  
  QMenu* help = new QMenu( "&Help", this );
  addMenu( HELP_HOME   , tr("UltraScan &Home"    ), help );
  addMenu( HELP        , tr("UltraScan &Manual"  ), help );
  addMenu( HELP_REG    , tr("&Register Software" ), help );
  addMenu( HELP_UPGRADE, tr("&Upgrade UltraScan" ), help );
  addMenu( HELP_LICENSE, tr("UltraScan &License" ), help );
  addMenu( HELP_ABOUT  , tr("&About"             ), help );
  addMenu( HELP_CREDITS, tr("&Credits"           ), help );
  

  QFont bold = QFont( "Helvetica", 9, QFont::Bold );
  menuBar()->setFont( bold );
  menuBar()->addMenu( file );
  menuBar()->addMenu( edit );
  menuBar()->addMenu( help );

  splash();
  statusBar()->showMessage( tr( "Ready" ) );
}
  
void us_win::addMenu( int index, const QString& label, QMenu* menu )
{
  us_action* action = new us_action( index, label, menu );

  QString family    = "Helvetica";
  int     pointsize = 9;
  int     weight    = QFont::Normal;
  QFont   font      = QFont( family, pointsize, weight );
  action->setFont( font );

  connect( action, SIGNAL( indexTriggered  ( int ) ), 
           this,   SLOT  ( onIndexTriggered( int ) ) );

  menu->addAction( action );
}

void us_win::onIndexTriggered( int index )
{
  qDebug() << index ;
  if ( index == 4 ) close();
  if ( index >= P_CONFIG && index < P_END    ) launch( index );
  if ( index >= HELP     && index < HELP_END ) help  ( index );
}

void us_win::terminated( int /* code*/, QProcess::ExitStatus /* status */ )
{
  for ( int i = 0; i < P_END - P_CONFIG; i++ )
  {
    QProcess* proc = p[ i ].process;
    if ( proc ) 
    {
      if ( proc->state() == QProcess::NotRunning )
      {
        delete proc;
        p[ i ].process = NULL;
        return;
      }
    }
  }
}

void us_win::launch( int index )
{
  index -= P_CONFIG;

  if ( p[ index ].process ) 
  {
    QMessageBox::information( this,
      tr( "Error" ),
      tr( QString( "The " + p[ index ].name + " is already running" ).toAscii() ) );
  
    return;
  }

  statusBar()->showMessage( 
      tr( QString( "Loading " + p[ index ].runningMsg + "..." ).toAscii() ) );

  QProcess* process = new QProcess( this );
  process->closeReadChannel( QProcess::StandardOutput );
  process->closeReadChannel( QProcess::StandardError );
  connect ( process, SIGNAL( finished  ( int, QProcess::ExitStatus ) ),
            this   , SLOT  ( terminated( int, QProcess::ExitStatus ) ) );

  process->start( p[ index ].name );

  if ( ! process->waitForStarted( 10000 ) ) // 10 second timeout
  {
    QMessageBox::information( this,
      tr( "Error" ),
      tr( QString( "There was a problem creating a subprocess\n"
          "for " + p[ index ].name ).toAscii() ) );
  }
  else
    p[ index ].process = process;

  statusBar()->showMessage( 
      tr( QString( "Loaded " + p[ index ].runningMsg + "..." ).toAscii() ) );
}


void us_win::closeEvent( QCloseEvent* event )
{
  QPoint p = g->global_position();
  g->set_global_position( p - QPoint( 30, 30 ) );
  delete g;
  event->accept();
}

void us_win::splash( void )
{
  int y =           menuBar  ()->size().rheight();
  int h = 532 - y - statusBar()->size().rheight();
  int w = 710;

  bigframe = new QLabel( this );
  bigframe->setFrameStyle        ( QFrame::Box | QFrame::Raised);
  bigframe->setPalette           ( US_GuiSettings::frameColor() );
  bigframe->setGeometry          ( 0, y, w, h );
  bigframe->setAutoFillBackground( true );

  splash_shadow = new QLabel( this );
  splash_shadow->setGeometry( (unsigned int)( ( w / 2 ) - 210 ) , 130, 460, 276 );
  splash_shadow->setPalette( QPalette( Qt::black, Qt::cyan ) );
  splash_shadow->setAutoFillBackground ( true );

  logo( w );

  QTimer::singleShot( 6000, this, SLOT( closeSplash() ) );
}

void us_win::logo( int width )
{
#if defined(WIN32)
    #define OS_TITLE "Windows"
#elif defined(LINUX)
    #define OS_TITLE "Linux"
#elif defined(OSX)
    #define OS_TITLE "OS-X"
#elif defined(FREEBSD)
    #define OS_TITLE "FreeBSD"
#elif defined(OPENBSD)
    #define OS_TITLE "OpenBSD"
#elif defined(NETBSD)
    #define OS_TITLE "NetBSD"
#elif defined(IRIX)
    #define OS_TITLE "Irix"
#elif defined(SOLARIS)
    #define OS_TITLE "Solaris"
#else
    #define OS_TITLE "Unknown"
#endif

  QString dir = qApp->applicationDirPath() + "/../etc/";  // For now -- later UltraScan sytem dir
  qDebug() << dir;
  QPixmap rawpix( dir + "flash-combined-no-text.png" );

  int ph = rawpix.height();
  int pw = rawpix.width();

  QPixmap  pixmap( pw, ph );
  QPainter painter( &pixmap );

  painter.drawPixmap( 0, 0, rawpix );
  painter.setPen    ( QPen( Qt::white, 3 ) );

#define REVISION "Revision: 999"
#define US_Version QString("10.0")

  QString version = "UltraScan " + US_Version + " ( " REVISION
  " ) for " OS_TITLE;  // REVISON is #define "Revision: xxx"

  QFont font( "Arial" );
  font.setWeight( QFont::DemiBold );
  font.setPixelSize( 16 );
  painter.setFont( font );
  QFontMetrics metrics( font );

  int sWidth = metrics.boundingRect( version ).width();
  int x      = ( pw - sWidth ) / 2;

  painter.drawLine( 0, 111, pw, 111);
  painter.drawText( x, 139, version );
  painter.drawLine( 0, 153, pw, 153);

  QString s = "Author: Borries Demeler";
  sWidth    = metrics.boundingRect( s ).width();
  painter.drawText( ( pw - sWidth ) / 2, 177, s );

  s      = "The University of Texas";
  sWidth = metrics.boundingRect( s ).width();
  painter.drawText( ( pw - sWidth ) / 2, 207, s );

  s      = "Health Science Center at San Antonio";
  sWidth = metrics.boundingRect( s ).width();
  painter.drawText( ( pw - sWidth ) / 2, 227, s );

  s      = "Department of Biochemistry";
  sWidth = metrics.boundingRect( s ).width();
  painter.drawText( ( pw - sWidth ) / 2, 247, s );
  
  smallframe = new QLabel(this);
  smallframe->setPixmap(pixmap);
  smallframe->setGeometry( (unsigned int)( (width / 2) - 230 ), 110, 460, 276);
}

void us_win::closeSplash( void )
{
   delete smallframe;
   delete splash_shadow;
   bigframe->show();
}

void us_win::help( int index )
{
  int i = index - HELP;

  statusBar()->showMessage( tr( h[i].loadMsg.toAscii() ) );
  switch ( index )
  {
    case HELP_CREDITS:
      QMessageBox::information( this,
        tr( "UltraScan Credits" ),
        tr( QString( "UltraScan II version " + US_Version + "\n"
          "Copyright 1998 - 2008\n"
          "Borries Demeler and the University of Texas System\n\n"
          " - Credits -\n\n"
          "The development of this software has been supported by grants\n"
          "from the National Science Foundation (grants #9724273 and\n"
          "#9974819), the Robert J. Kleberg Jr. and Helen C. Kleberg\n"
          "Foundation, the Howard Hughes Medical Institute Research\n"
          "Resources Program Award to the University of Texas Health\n"
          "Science Center at San Antonio (# 76200-550802), grant #119933\n"
          "from the San Antonio Life Science Institute, and the NIH\n"
          "Center for Research Resources with grant 5R01RR022200.\n\n"
          "The following individuals have made significant contributions\n"
          "to the UltraScan Software:\n\n"
          "   * Emre Brookes (parallel distributed code, supercomputing\n"
          "     implementations, GA, 2DSA, SOMO, simulation, optimization)\n"
          "   * Weiming Cao (ASTFEM, ASTFEM-RA, Simulator)\n"
          "   * Bruce Dubbs (Win32 Port, Qt4 Port, USLIMS, \n"
          "     GNU code integration)\n"
          "   * Jeremy Mann (Porting, TIGRE/Globus Integration)\n"
          "   * Yu Ning (Database Functionality)\n"
          "   * Marcelo Nollman (SOMO)\n"
          "   * Zach Ozer (Equilibrium Fitter)\n"
          "   * Nithin Rai (SOMO)\n"
          "   * Mattia Rocco (SOMO)\n"
          "   * Bruno Spotorno (SOMO)\n"
          "   * Giovanni Tassara (SOMO)\n"
          "   * Oleg Tsodikov (SurfRacer in SOMO)\n"
          "   * Josh Wilson (initial USLIMS)\n"
          "   * Dan Zollars (USLIMS and Database)\n\n"
          "and many users who contributed bug fixes and feature suggestions." ).toAscii() ) );

      statusBar()->showMessage( tr( "Ready" ) );
      break;

    case HELP_ABOUT:
      QMessageBox::information( this,
        tr("About UltraScan..."),
        tr( QString( "UltraScan III version " + US_Version + "\n"
          REVISION "\n"
          "Copyright 1989 - 2008\n"
          "Borries Demeler and the University of Texas System\n\n"
          "For more information, please visit:\n"
          "http://www.ultrascan.uthscsa.edu/\n\n"
          "The author can be reached at:\n"
          "The University of Texas Health Science Center\n"
          "Department of Biochemistry\n"
          "7703 Floyd Curl Drive\n"
          "San Antonio, Texas 78229-3900\n"
          "voice: (210) 567-6592\n"
          "Fax:   (210) 567-6595\n"
          "E-mail: demeler@biochem.uthscsa.edu" ).toAscii() ) );

      statusBar()->showMessage( tr( "Ready" ) );
      break;

    default:
      
      if (  h[i].type == URL )
        showhelp.show_URL( h[i].url );
      else
        showhelp.show_help( h[i].url );

      statusBar()->showMessage( tr( "Loaded " + h[i].loadMsg.toAscii() ) );
      break;
  }
}

