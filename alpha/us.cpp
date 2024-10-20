//! \file us.cpp
#include <QtCore>
#if QT_VERSION < 0x050000
#include <QtSingleApplication>
#else
#include <QtWidgets/QApplication>
#endif

#include "us.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_win_data.cpp"
#include "us_defines.h"
#include "us_revision.h"
#include "us_sleep.h"
#include "us_images.h"
#include "us_passwd.h"
#include "us_db2.h"

#if 0
#define EQUI_MENU
#endif

using namespace US_WinData;

/*! \brief Main program for UltraScan

  This program is instantiated using QtSingleApplication.  This class 
  prevents more than one instance of the program being run.

  The environment variable, ULTRASCAN_OPTIONS, can be used to allow
  multiple instances if it contains the string "multiple".

  The main program also sets up internationalization and checks for
  a license.  If a valid license is not found, it launches the
  \ref US_License window.
*/
int main( int argc, char* argv[] )
{
  QString options( getenv( "ULTRASCAN_OPTIONS" ) );
#if QT_VERSION < 0x050000
  QtSingleApplication application( "UltraScan III", argc, argv );
  
  // If environment variable ULTRASCAN_OPTIONS contains the 
  // word 'multiple', then we don't try to limit to one instance
  if ( ! options.contains( "multiple" ) )
  {
    if ( application.sendMessage( "Wake up" ) ) return 0;
  }

  application.initialize();
#else
  QApplication application( argc, argv );
  application.setApplicationDisplayName( "UltraScan III" );
#endif

  // Set up language localization
  QString locale = QLocale::system().name();

  QTranslator translator;
  translator.load( QString( "us_" ) + locale );
  application.installTranslator( &translator );
    
  // See if we need to update the license
  QString ErrorMessage;

  int result = US_License_t::isValid( ErrorMessage );
  if ( result != US_License_t::OK )
  {
    QMessageBox mBox;

    QPushButton* cancel   = mBox.addButton( QMessageBox::Cancel );
    QPushButton* Register = mBox.addButton( qApp->translate( "UltraScan III", "Register"), 
        QMessageBox::ActionRole);
    
    mBox.setDefaultButton( Register );
    mBox.setWindowTitle  ( qApp->translate( "UltraScan III", "UltraScan License Problem" ) );
    mBox.setText         ( ErrorMessage );
    mBox.setIcon         ( QMessageBox::Critical );
    mBox.exec();

    if ( mBox.clickedButton() == cancel )  exit( -1 ); 
    
    US_License* license = new US_License();
    license->show();
#if QT_VERSION < 0x050000
    application.setActivationWindow( license );
#endif
    return application.exec();
  }

  // License is OK.  Start up.
  US_Win w;
  w.show();
#if QT_VERSION < 0x050000
  application.setActivationWindow( &w );
#endif
  return application.exec();
}

//////////////US_Action
US_Action::US_Action( int i, const QString& text, QObject* parent) 
    : QAction( text, parent ), index( i ) 
{
  connect( this, SIGNAL( triggered  ( bool ) ), 
           this, SLOT  ( onTriggered( bool ) ) );
}

void US_Action::onTriggered( bool ) 
{
  emit indexTriggered( index );
}
/////////////US_Win
US_Win::US_Win( QWidget* parent, Qt::WindowFlags flags )
  : QMainWindow( parent, flags )
{
  // We need to handle US_Global::g here becuse US_Widgets is not a parent
  if ( ! g.isValid() ) 
  {
    // Do something for invalid global memory
    qDebug( "US_Win: invalid global memory" );
  }
  
  g.set_global_position( QPoint( 50, 50 ) ); // Ensure initialization
  QPoint p = g.global_position();
  setGeometry( QRect( p, p + QPoint( 710, 532 ) ) );
  g.set_global_position( p + QPoint( 30, 30 ) );

  setWindowTitle( "UltraScan III" );

  QIcon us3_icon = US_Images::getIcon( US_Images::US3_ICON );
  setWindowIcon( us3_icon );

  procs = QList<procData*>(); // Initialize to an empty list

  ////////////
  QMenu* file = new QMenu( tr( "&File" ), this );

  //addMenu(  P_CONFIG, tr( "&Configuration" ), file );
  //addMenu(  P_ADMIN , tr( "&Administrator" ), file );
  //file->addSeparator();
  addMenu(  P_EXIT, tr( "E&xit"          ), file );

  //QMenu* type1 = new QMenu( tr( "&Velocity Data" ), file );
  //addMenu( 21, tr( "&Absorbance Data"     ), type1 );
  //addMenu( 22, tr( "&Interference Data"   ), type1 );
  //addMenu( 23, tr( "&Fluorescense Data"   ), type1 );
  //addMenu( 24, tr( "&Edit Cell ID's Data" ), type1 );

  ///////////////
  QMenu* edit        = new QMenu( tr( "&Edit" ),       this );
  //addMenu( 12, tr( "&Equilibrium Data" )    , edit );
  //addMenu( 13, tr( "Edit &Wavelength Data" ), edit );
  //addMenu( 14, tr( "View/Edit &Multiwavelength Data" ), edit );
  addMenu(  P_EDIT,   tr( "&Edit Data" )          , edit );
  edit->addSeparator();
  addMenu(  P_CONFIG, tr( "&Preferences" )        , edit );
  
  /////////////
  QMenu* velocity    = new QMenu( tr( "&Velocity" ),    this );
  addMenu(  P_VHWE     , tr( "&Enhanced van Holde - Weischet" ),   velocity );
  addMenu(  P_GRIDEDIT , tr( "C&ustom 2-D Grid Editor" ),          velocity );
  addMenu(  P_2DSA     , tr( "&2-D Spectrum Analysis" ),           velocity );
  addMenu(  P_PCSA     , tr( "&Parametrically Constrained Spectrum Analysis" ),
                                                                   velocity );
  addMenu(  P_GAINIT   , tr( "&Initialize Genetic Algorithm" ),    velocity );
  addMenu(  P_DMGAINIT , tr( "Initialize Discrete Model &Genetic Algorithm" ),
                                                                   velocity );
  addMenu(  P_SECOND   , tr( "Second &Moment" ),                   velocity );
  addMenu(  P_DCDT     , tr( "&Time Derivative" ),                 velocity );
  addMenu(  P_FEMA     , tr( "&FE Model Viewer" ),                 velocity );
  addMenu(  P_FEMSTAT  , tr( "FE Model &Statistics" ),             velocity );
  addMenu(  P_PSEUDO3D , tr( "&Combine Pseudo-3D Distributions" ), velocity );
  addMenu(  P_RAMP     , tr( "Speed &Ramp Analysis" ),             velocity );
  
#ifdef EQUI_MENU
  QMenu* equilibrium = new QMenu( tr( "E&quilibrium" ), this );
  addMenu(  P_EQGLOBFIT, tr( "&Global Fit" ),                 equilibrium );
  //addMenu(  P_EQTIMEEST, tr( "Estimate Equilibrium &Times",   equilibrium );
#endif

//  QMenu* fit         = new QMenu( tr( "&Global Fit" ),  this );
//  addMenu(  P_GLOBFITEQ, tr( "Global &Equilibrium Fit" ),     fit );
  //addMenu(  P_GLOBFITEX, tr( "Global E&xtinction Fit" ),      fit );
  //addMenu(  P_GLOBFITSP, tr( "Global &Spectrum Fit" ),        fit );
  
  QMenu* utilities   = new QMenu( tr( "&Utilities" ),   this );
  QMenu* multiwave   = new QMenu( tr( "&Multiwavelength" ),   this );
  QMenu* spectrum    = new QMenu( tr( "Spectral &Analysis" ),   this );
  addMenu(  P_SPECFIT  , tr( "&Spectrum Fitter"                  ), spectrum);
  addMenu(  P_SPECDEC  , tr( "Spectrum &Decomposition"           ), spectrum);
  addMenu(  P_CONVERT  , tr( "&Import Experimental Data"         ), utilities );
  addMenu(  P_EXPORT   , tr( "&Export OpenAUC Data"              ), utilities );
#if 0    // temporarily disable Create Experiment until truly ready
  addMenu(  P_CEXPERI  , tr( "Create E&xperiment"                ), utilities );
#endif
  addMenu(  P_FDSMAN   , tr( "FDS File &Manager"                 ), utilities );
  addMenu(  P_FITMEN   , tr( "&Fit Meniscus"                     ), utilities );
  utilities->addMenu(spectrum);
  addMenu(  P_COLORGRAD, tr( "Color &Gradient Generator"         ), utilities );
  addMenu(  P_RPTGEN   , tr( "&Report Generator"                 ), utilities );
  addMenu(  P_ROTORCAL , tr( "Rotor &Calibration"                ), utilities );
  addMenu(  P_LICENSE  , tr( "&License Manager"                  ), utilities );
  addMenu(  P_VHWCOMB ,  tr( "Combine Distribution &Plots (vHW)" ), utilities );
  addMenu(  P_DDCOMB   , tr( "Combine &Discrete Distributions"   ), utilities );
  addMenu(  P_GLOMODL ,  tr( "Create Global &Model"              ), utilities );
  addMenu(  P_VIEWCFA ,  tr( "View Raw C&FA Data"                ), utilities );
  addMenu(  P_VIEWXPN ,  tr( "View Raw &Optima Data"             ), utilities );
  addMenu(  P_VIEWTMST,  tr( "View &TimeState"                   ), utilities );

  addMenu(  P_VIEWMWL ,  tr( "&View Multiwavelength Data"        ), multiwave );
  addMenu(  P_VIEWMSS ,  tr( "View MWL &S-Spectra"               ), multiwave );
  addMenu(  P_MWSPECF ,  tr( "MWL Species Fit"                   ), multiwave );
  addMenu(  P_MWFSIMU ,  tr( "Optima MWL Fit Simulation"         ), multiwave );

  QMenu* simulation  = new QMenu( tr( "S&imulation" ),  this );
  addMenu(  P_ASTFEM, tr( "&Finite Element Simulation (ASTFEM)" ), simulation );
  addMenu(  P_EQUILTIMESIM, 
                      tr( "Estimate Equilibrium &Times"         ), simulation );
  addMenu(  P_SASSOC, tr( "&Self-Association Equilibrium"       ), simulation );
  addMenu(  P_MODEL1, tr( "&Model s, D and f from MW for 4 basic shapes" ), 
                                                                   simulation );
  addMenu(  P_MODEL2, tr( "&Predict f and axial ratios for 4 basic shapes" ), 
                                                                   simulation );
  addMenu(  P_SOMO,   tr( "S&OMO Bead Modeling"                 ), simulation );
  addMenu(  P_SOMOCONFIG,   tr( "S&OMO Configuration"           ), simulation );

  QMenu* database    = new QMenu( tr( "&Database" ),    this );
  addMenu(  P_INVESTIGATOR , tr( "Manage &Investigator Data" ), database );
  addMenu(  P_BUFFER       , tr( "Manage &Buffer Data"       ), database );
  addMenu(  P_VBAR         , tr( "Manage &Analytes"          ), database );
  addMenu(  P_MODEL        , tr( "Manage &Models"            ), database );
  addMenu(  P_MANAGEDATA   , tr( "Manage &Data"              ), database );
  addMenu(  P_MANAGESOLN   , tr( "Manage &Solutions"         ), database );
  addMenu(  P_MANAGEPROJ   , tr( "Manage &Projects"          ), database );
  addMenu(  P_MANAGEROTOR  , tr( "Manage &Rotors"            ), database );

  ///////////////
  QMenu* help        = new QMenu( tr( "&Help" ),        this );
  addMenu( HELP_HOME   , tr("UltraScan &Home"    ), help );
  addMenu( HELP        , tr("UltraScan &Manual"  ), help );
  addMenu( HELP_REG    , tr("&Register Software" ), help );
  addMenu( HELP_UPGRADE, tr("&Upgrade UltraScan" ), help );
  addMenu( HELP_LICENSE, tr("UltraScan &License" ), help );
  addMenu( HELP_ABOUT  , tr("&About"             ), help );
  addMenu( HELP_CREDITS, tr("&Credits"           ), help );
  addMenu( HELP_NOTICES, tr("Show &Notices"      ), help );
  
#ifndef Q_OS_MAC
  QFont bfont = QFont( US_GuiSettings::fontFamily(),
                       US_GuiSettings::fontSize() - 1,
                       QFont::Bold );
  menuBar()->setFont( bfont       );
#endif
  menuBar()->addMenu( file        );
  menuBar()->addMenu( edit        );
  menuBar()->addMenu( velocity    );
#ifdef EQUI_MENU
  menuBar()->addMenu( equilibrium );
//  menuBar()->addMenu( fit         );
#endif
  menuBar()->addMenu( utilities   );
  menuBar()->addMenu( multiwave   );
  menuBar()->addMenu( simulation  );
  menuBar()->addMenu( database    );
  menuBar()->addMenu( help        );

#ifndef Q_OS_MAC
  QFont mfont = QFont( US_GuiSettings::fontFamily(),
                       US_GuiSettings::fontSize() - 1,
                       QFont::Normal );
  file       ->setFont( mfont );
  edit       ->setFont( mfont );
  velocity   ->setFont( mfont );
#ifdef EQUI_MENU
  equilibrium->setFont( mfont );
//  fit        ->setFont( mfont );
#endif
  utilities  ->setFont( mfont );
  multiwave  ->setFont( mfont );
  simulation ->setFont( mfont );
  database   ->setFont( mfont );
  help       ->setFont( mfont );
#endif

   splash();
   statusBar()->showMessage( tr( "Ready" ) );

   notice_check();              // Check for any notices pending
}

US_Win::~US_Win()
{
    QPoint p = g.global_position();
    g.set_global_position( p - QPoint( 30, 30 ) );
}
  
void US_Win::addMenu( int index, const QString& label, QMenu* menu )
{
  US_Action* action = new US_Action( index, label, menu );

#ifndef Q_OS_MAC
  QFont      font   = QFont( US_GuiSettings::fontFamily(),
                             US_GuiSettings::fontSize() - 1,
                             QFont::Normal );
  action->setFont( font );
#endif

  connect( action, SIGNAL( indexTriggered  ( int ) ), 
           this,   SLOT  ( onIndexTriggered( int ) ) );

  menu->addAction( action );
}

void US_Win::onIndexTriggered( int index )
{
  if ( index == 4 )         close();

//qDebug() << index << P_CONFIG << P_END;
  if ( index >= P_CONFIG && index < P_END    ) launch( index );
  if ( index >= HELP     && index < HELP_END ) help  ( index );
}

void US_Win::terminated( int code, QProcess::ExitStatus status )
{
qDebug() << "PROCESS terminated:  code" << code << "status" << status;
  QList<procData*>::iterator pr;
  
  for ( pr = procs.begin(); pr != procs.end(); pr++ )
  {
    procData* d       = *pr;
    QProcess* process = d->proc;
    int       index   = d->index;

    if ( process->state() == QProcess::NotRunning )
    {
qDebug() << "PROCESS NotRunning index" << index << "Proc name" << d->name;

      if ( status == QProcess::CrashExit )
      { // Process crashed:  output a message and possibly do a pop-up
        QString msg_sb;
        QString msg_mb;
        QString msg_ru  = p[ index ].runningMsg;
        QString estderr = QString( process->readAllStandardError() )
                          .right( 2000 );    // End lines of STDERR
        bool    badallo = estderr.contains( "bad_alloc" );
qDebug() << "PROCESS   status" << status << "e-stderr len" << estderr.length();
//qDebug() << "PROCESS   bad_alloc? " << badallo;

        if ( badallo )
        { // It was a "bad_alloc" crash
          msg_sb = tr( "MEMORY ALLOC crash: " ) + msg_ru;
          msg_mb = tr( "Process MEMORY ALLOCATION Crash:\n" ) + msg_ru;
        }

        else
        { // It was other than bad_alloc
          int kwhat = estderr.lastIndexOf( "what(): " );

          if ( kwhat > 0 )
          { // We have a "what():" hint in stderr
            QString msg_wh = estderr.mid( kwhat + 7, 20 ).simplified();
            msg_sb = "[ " + msg_wh + " ] crash: " + msg_ru;
            msg_mb = tr( "Process Crash [ "  ) + msg_wh + " ]\n" + msg_ru;
          }

          else
          { // Crash type is undetermined
            msg_sb = "[ unknown type ] crash: " + msg_ru;
            msg_mb = tr( "Process Crash [ unknown type ]\n" ) + msg_ru;
          }
        }

        // Report the crash in the status bar and in a message box pop-up
        statusBar()->showMessage( msg_sb );
        QString selines = estderr.endsWith( "\n" )
           ? estderr.section( "\n", -4, -2 )
           : estderr.section( "\n", -3, -1 );
        msg_mb += tr( "\n\nLast several stderr lines --\n\n" ) + selines;
        QMessageBox::warning( this, tr( "Process Crash" ), msg_mb );
      }

      procs.removeOne( d );
      p[ index ].currentRunCount--;
      delete process;  // Deleting the process structure
      delete d;        // Deleting the procData structure

      if ( index == 0 )
        apply_prefs();

      return;
    }
  }
}

void US_Win::launch( int index )
{
   static const int trig_secs=3600;
   index        -= P_CONFIG;
   QString pname = p[ index ].name;

   // At each launch, check for notices if last check was over 24 hours ago
   if ( ln_time.secsTo( QDateTime::currentDateTime() ) > trig_secs )
      notice_check();

  if ( p[ index ].maxRunCount <= p[ index ].currentRunCount && 
       p[ index ].maxRunCount > 0 ) 
  {
    if ( p[ index ].index == P_CONFIG )
    {
       QMessageBox::information( this,
         tr( "Already Running" ),
         tr( "The configuration program is already running.\n"
             "Click on the task bar item named UltraScan " 
             "Configuration to continue." ) );
    }
    else
    {
    QMessageBox::warning( this,
      tr( "Error" ),
      pname + tr( " is already running." ) );
    }
  
    return;
  }

  statusBar()->showMessage( 
      tr( "Loading " ) + p[ index ].runningMsg + "..." );

  QProcess* process = new QProcess( 0 );
  process->closeReadChannel( QProcess::StandardOutput );
  process->closeReadChannel( QProcess::StandardError );
  connect ( process, SIGNAL( finished  ( int, QProcess::ExitStatus ) ),
            this   , SLOT  ( terminated( int, QProcess::ExitStatus ) ) );

#ifndef Q_OS_MAC
  process->start( pname );
#else
   QString procbin = US_Settings::appBaseDir() + "/bin/" + pname;
   QString procapp = procbin + ".app";

   if ( !QFile( procapp ).exists() )
      procapp         = procbin;

   process->start( "open", QStringList(procapp) );
#endif

  if ( ! process->waitForStarted( 10000 ) ) // 10 second timeout
  {
    QMessageBox::information( this,
      tr( "Error" ),
      tr( "There was a problem creating a subprocess\nfor " ) + pname );
  }
  else
  {
    p[ index ].currentRunCount++;
    procData* pr = new procData;
    pr->proc     = process;
    pr->name     = pname;
    pr->index    = index;

    procs << pr;
  }

  statusBar()->showMessage( 
      tr( "Loaded " ) + p[ index ].runningMsg + "..." );
}

void US_Win::closeProcs( void )
{
  QString                    names;
  QList<procData*>::iterator p;
  
  for ( p = procs.begin(); p != procs.end(); p++ )
  {
    procData* d  = *p;
    names       += d->name + "\n";
  }

  QString isAre  = ( procs.size() > 1 ) ? "es are" : " is";
  QString itThem = ( procs.size() > 1 ) ? "them"   : "it";
  
  QMessageBox box;
  box.setWindowTitle( tr( "Attention" ) );
  box.setText( QString( tr( "The following process%1 still running:\n%2"
                            "Do you want to close %3?" )
                             .arg( isAre ).arg( names ).arg( itThem ) ) );

  QString killText  = tr( "&Kill" );
  QString closeText = tr( "&Close Gracefully" );
  QString leaveText = tr( "&Leave Running" );

  QPushButton* kill  = box.addButton( killText , QMessageBox::YesRole );
                       box.addButton( closeText, QMessageBox::YesRole );
  QPushButton* leave = box.addButton( leaveText, QMessageBox::NoRole  );

  box.exec();

  if ( box.clickedButton() == leave ) return;

  for ( p = procs.begin(); p != procs.end(); p++ )
  {
    procData* d       = *p;
    QProcess* process = d->proc;
    
    if ( box.clickedButton() == kill )
      process->kill();
    else
      process->terminate();
  }

  // We need to sleep slightly (one millisecond) so that the system can clean 
  // up and properly release shared memory.
  g.scheduleDelete();
  US_Sleep::msleep( 1 );
}

void US_Win::closeEvent( QCloseEvent* e )
{
  if ( ! procs.isEmpty() ) closeProcs();
  e->accept();
}


void US_Win::splash( void )
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

  //QTimer::singleShot( 6000, this, SLOT( closeSplash() ) );
}

void US_Win::logo( int width )
{
  // Splash image
  QPixmap rawpix = US_Images::getImage( US_Images::US3_SPLASH );

  int ph = rawpix.height();
  int pw = rawpix.width();

  QPixmap  pixmap( pw, ph );
  QPainter painter( &pixmap );

  painter.drawPixmap( 0, 0, rawpix );
  painter.setPen    ( QPen( Qt::white, 3 ) );

  QString version = "Version " + US_Version + " ( " REVISION
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

void US_Win::closeSplash( void )
{
   delete smallframe;
   delete splash_shadow;
   bigframe->show();
}

void US_Win::help( int index )
{
  int i = index - HELP;

  statusBar()->showMessage( h[i].loadMsg );
  switch ( index )
  {
    case HELP_CREDITS:
      QMessageBox::information( this,
        tr( "UltraScan Credits" ),
        tr( "The UltraScan-III and LIMS-III software were developed by:\n\n"
            "Emre Brookes, Weiming Cao, Bruce Dubbs, Gary Gorbet, " 
            "Jeremy Mann, Suresh Marru, Shabhaz Memon, Marlon Pierce, \n"
	    "Raminder Singh and Dan Zollars.\n\n"
            "Project Director: Borries Demeler\n\n"
            "This development was funded by NIH Grant RR022200 "
	    "and NSF grants ACI-1339649 and TG-MCB070039.\n\n"

            "We thank the following individuals for contributions "
            "to the UltraScan project:\n\n"

            " * Nikolay Dokholyan\n"
            " * Jose Garcia de la Torre\n"
			" * Haram Kim\n"
			" * Minji Kim\n"
            " * Brad Langford\n"
            " * Thomas Laue\n"
			" * Konrad Löhr\n"
            " * Luitgard Nagel-Steger\n"
            " * Zach Ozer\n"
            " * Karel Planken\n"
            " * Mattia Rocco\n"
            " * Virgil Schirf\n"
            " * Bruno Spotorno\n"
            " * Giovanni Tassara\n"
            " * Oleg Tsodikov\n"
            " * Johannes Walter\n"
            " * Nikola Wenta\n"
            " * Josh Wilson" ) );

      statusBar()->showMessage( tr( "Ready" ) );
      break;

    case HELP_ABOUT:
      QMessageBox::information( this,
        tr( "About UltraScan..." ),
        tr( "UltraScan III version %1  (DEVELOPMENT)\n"
            "%2\n"
            "Copyright 1989 - 2024\n"
            "Borries Demeler\n\n"
            "For more information, please visit:\n"
            "http://www.ultrascan.aucsolutions.com/\n\n"
            "The author can be reached at:\n"
            "E-mail: borries.demeler@umontana.edu" ).arg( US_Version ).arg( REVISION ) );

      statusBar()->showMessage( tr( "Ready" ) );
      break;

    case HELP_NOTICES:
      ln_time    = ln_time.addYears( -1 );
      notice_check();
      break;

    default:

      if (  h[i].type == URL )
      {
        showhelp.show_URL( h[i].url );
      }
      else
      {
        showhelp.show_help( h[i].url );
      }

      statusBar()->showMessage( tr( "Loaded " ) + h[i].loadMsg );
      break;
  }
}

// apply font and base frame color preferences
void US_Win::apply_prefs()
{
#ifndef Q_OS_MAC
   // reset the menu bar font
   QFont bfont = QFont( US_GuiSettings::fontFamily(),
                        US_GuiSettings::fontSize() - 1,
                        QFont::Bold );

   menuBar()->setFont( bfont  );

   // reset the font for all the sub-menus
   QFont mfont = QFont( US_GuiSettings::fontFamily(),
                        US_GuiSettings::fontSize() - 1,
                        QFont::Normal );

   QList< QMenu* > mens  = this->findChildren< QMenu* >();  // menubar menus

   for ( int ii = 0; ii < mens.count(); ii++ )
   {  // loop to get and reset actions font for each menu
      QList< QAction* > acts  = mens.at( ii )->findChildren< QAction* >();

      for ( int jj = 0; jj < acts.count(); jj++ )
      {  // reset the font for each submenu action
         acts.at( jj )->setFont( mfont );
      }
   }
#endif

   // reset the frame color
   bigframe->setPalette( US_GuiSettings::frameColor() );

   show();
}

// Check for posted US3 notices
bool US_Win::notice_check()
{
   bool do_abort     = false;                         // Default: no abort
   int  level        = 0;                             // Max level: information
   QDateTime pn_time = ln_time;                       // Previous check time
   ln_time           = QDateTime::currentDateTime();  // Reset last notice time

   if ( US_Settings::default_data_location() == 2 )
      return do_abort;      // If default data location is Disk, do not bother

//do_abort=true;
//level=2;
   // Query notice table in the us3_notice database
   US_Passwd pw;
   US_DB2    db;
   QString   host  ( "uslims3.aucsolutions.edu" );
   QString   dbname( "us3_notice" );
   QString   user  ( "us3_notice" );
   QString   passwd( "us3_notice" );
   QString   errmsg;

   if ( ! db.connect( host, dbname, user, passwd, errmsg  ) )
   {
qDebug() << "US:NOTE: Unable to connect" << errmsg;
      return do_abort;
   }

   QString query( "SELECT type, revision, message, lastUpdated"
                  " FROM us3_notice.notice;" );
   db.rawQuery( query );

   // If no notices in the database, return now with no notice pop-up
   if ( db.lastErrno() != US_DB2::OK  ||  db.numRows() == 0 )
   {
qDebug() << "US:NOTE: No DB notices" << db.lastError()
 << "numRows" << db.numRows();
      return do_abort; 
   }

   // Otherwise accumulate notices and associated type,revision,time
   QStringList  msgs;
   QStringList  types;
   QStringList  revs;
   QList< int > irevs;
   QDateTime   time_d;
   QMap< QString, QString > typeMap;
   typeMap[ "info" ] = tr( "Information" );
   typeMap[ "warn" ] = tr( "Warning"     );
   typeMap[ "crit" ] = tr( "Critical"    );
   QString srev     = US_Version  + "."
                    + QString( REVISION ).section( ":", 1, 1 ).simplified();
qDebug() << "US:NOTE: srev" << srev;
   int    nnotice   = 0;
   int    nn_info   = 0;
   int    nn_warn   = 0;
   int    nn_crit   = 0;
   int    i_rev     = 0;

   while ( db.next() )
   {
      nnotice++;

      QString type     = db.value( 0 ).toString();
      QString mrev     = db.value( 1 ).toString();
      QString msg      = db.value( 2 ).toString();
      QDateTime time_m = db.value( 3 ).toDateTime();
qDebug() << "US:NOTE: mrev(1)" << mrev;
      mrev             = srev.left( 3 ) + mrev.mid( 3 );
qDebug() << "US:NOTE: mrev(2)" << mrev;

      if ( type == "info" )       nn_info++;
      else if ( type == "warn" )  nn_warn++;
      else if ( type == "crit" )  nn_crit++;

      int    m_rev     = QString( mrev ).replace( ".", "" ).toInt();
      i_rev            = qMax( i_rev,  m_rev  );

      if ( nnotice == 1 )
      {
        time_d           = time_m;
      }
      else
      {
        time_d           = time_m.secsTo( time_d ) > 0
                           ? time_d : time_m;
      }

#if QT_VERSION > 0x050000
      if ( type == "warn"  &&  msg.contains( "revision 3.3" ) )
      {
        msg              = msg.replace( "revision 3.3", "revision 3.5" );
      }
      if ( type == "warn"  &&  msg.contains( "revision 3.5" ) )
      {
        msg              = msg.replace( "revision 3.5", "revision 4.0" );
      }
#endif
      types << type;
      revs  << mrev;
      irevs << m_rev;
      msgs  << msg;
   }

   // If current revision is at or beyond max in records, skip pop-up
   int    s_rev     = QString( srev ).replace( ".", "" ).toInt();

qDebug() << "s_rev i_rev" << s_rev << i_rev << "srev" << srev;
   if ( s_rev > i_rev )
      return do_abort;

   // If lastest message time earlier than last notice time, skip pop-up
   int n_dif        = (int)time_d.secsTo( pn_time );
qDebug() << " n_dif" << n_dif << "time_d" << time_d << "pn_time" << pn_time;
   if ( n_dif > 0 )
      return do_abort;

   // Build notice message
   level             = ( nn_warn > 0 ) ? 1 : level;
   level             = ( nn_crit > 0 ) ? 2 : level;
   QString msg_note  = tr( "UltraScan III notices posted  (" ) 
                     + time_d.toString( "yyyy/MM/dd" ) + "):\n\n";

   for ( int ii = 0; ii < nnotice; ii++ )
   {
      // Skip messages for warn/crit same revision or any earlier than current
      if ( ( irevs[ ii ] == s_rev  &&  types[ ii ] != "info" )  ||
           irevs[ ii ] < s_rev )     continue;

      // Add current message to full text
      msg_note         += typeMap[ types[ ii ] ] + " for release "
                       + revs[ ii ] + ":\n"
                       + msgs[ ii ] + "\n";

      // Critical from later revision than current means an abort
      if ( types[ ii ] == "crit" )   do_abort = true;
   }

   if ( do_abort )
   {  // Append an additional note if an abort is happening
      msg_note         += tr( "\n\n*** US3 Abort: UPDATE REQUIRED!!! ***\n" );
   }

   // Display notices at level of highest level currently set
   QWidget* wthis    = (QWidget*)this;
   if (      level == 0 )
      QMessageBox::information( wthis, tr( "US3 Notices" ), msg_note );
   else if ( level == 1 )
      QMessageBox::warning    ( wthis, tr( "US3 Notices" ), msg_note );
   else if ( level == 2 )
      QMessageBox::critical   ( wthis, tr( "US3 Notices" ), msg_note );

   // Abort if that is indicated
   if ( do_abort )
      exit( -77 );

   return do_abort;
}

