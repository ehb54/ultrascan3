//! \file us.cpp
#include <QtCore>
#include <QTcpSocket>
#include <QFontDatabase>
#include <QtWidgets/QApplication>
#include <QVersionNumber>

#include "us.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_win_data.h"
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
  qputenv("QT_ENABLE_HIGHDPI_SCALING",QByteArray("1"));
  QApplication application( argc, argv );
  application.setApplicationDisplayName( "UltraScan III" );

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
  w.update_user_level();
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
  // We need to handle US_Global::g here because US_Widgets is not a parent
  if ( ! g.isValid() ) 
  {
    // Do something for invalid global memory
    qDebug( "US_Win: invalid global memory" );
    QMessageBox::critical(this
                          ,windowTitle()
                          ,QString(
                                   tr( 
                                      "There is a error connecting to global memory:\n"
                                      "\"%1\"\n"
                                      "This will cause Database connections to fail\n"
                                      "Please ask your system administrator to resolve this."
                                       )
                                   )
                          .arg( g.errorString() )
                          ,QMessageBox::Ok
                          ,QMessageBox::NoButton
                          ,QMessageBox::NoButton
                          );
  }
  
  if ( !US_Settings::status().isEmpty() ) {
     qDebug( "US_Win: invalid settings" );
     QMessageBox::critical(this
                           ,windowTitle()
                           ,QString(
                                    tr(
                                       "There is a error with your settings file:\n"
                                       "\"%1\"\n"
                                       "This can cause various problems and should be corrected.\n"
                                       )
                                    )
                           .arg( US_Settings::status() )
                           ,QMessageBox::Ok
                           ,QMessageBox::NoButton
                           ,QMessageBox::NoButton
                           );
  }

  g.set_global_position( QPoint( 50, 50 ) ); // Ensure initialization
  QString auto_positioning = US_Settings::debug_value("auto_positioning");
  if ( !auto_positioning.isEmpty() && auto_positioning.toLower() == "true" )
  {
    QPoint point = g.global_position();
    setGeometry( QRect( point, point + QPoint( 710, 532 ) ) );
    g.set_global_position( point + QPoint( 30, 30 ) );
  }
  else
  {
    setBaseSize(QSize(710, 532));
    setMinimumWidth(710);
    setMinimumHeight(532);
  }
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
  addMenu(  P_FEMA     , tr( "&Finite Element Model Viewer" ),                          velocity );
  addMenu(  P_2DSA     , tr( "&2-Dimensional Spectrum Analysis (2DSA)" ),               velocity );
  addMenu(  P_FITMEN   , tr( "&Fit Meniscus"                     ),                     velocity );
  addMenu(  P_GAINIT   , tr( "&Initialize Genetic Algorithm (GA)" ),                    velocity );
  addMenu(  P_DMGAINIT , tr( "Initialize Discrete Model &Genetic Algorithm (DMGA)" ),   velocity );
  addMenu(  P_PCSA     , tr( "&Parametrically Constrained Spectrum Analysis (PCSA)" ),  velocity );
  addMenu(  P_GRIDEDIT , tr( "C&ustom Grid Editor (CG)" ),              velocity );
  addMenu(  P_VHWE     , tr( "&Enhanced van Holde - Weischet (vHW)" ),  velocity );
  addMenu(  P_DCDT     , tr( "&Time Derivative (dC/dt)" ),              velocity );
  addMenu(  P_SECOND   , tr( "Second &Moment" ),                        velocity );
  velocity->addSeparator();
  addMenu(  P_RMSD     , tr( "&Query Model RMSDs" ),                velocity );
  addMenu(  P_FEMSTAT  , tr( "FE Model &Statistics" ),              velocity );
  addMenu(  P_PSEUDO3D , tr( "&Combine Pseudo-3D Distributions" ),  velocity );
  addMenu(  P_VHWCOMB  , tr( "Combine Distribution &Plots (vHW)" ), velocity );
  addMenu(  P_DDCOMB   , tr( "Combine Discrete Distrib&utions"   ), velocity );
  addMenu(  P_INTCOMB  , tr( "Combine I&ntegral Distributions"   ), velocity );
  addMenu(  P_GLOMODL  , tr( "Create Glo&bal Model"              ), velocity );
  addMenu(  P_DENSMTCH , tr( "Density Matc&hing"                 ), velocity );

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
  QMenu* spectrum    = new QMenu( tr( "&Spectral Analysis" ),   this );
  addMenu(  P_GETDATA  , tr( "&Data Acquisition"                 ), utilities );
  addMenu(  P_VIEWXPN  , tr( "View Raw &Optima Data"             ), utilities );
  addMenu(  P_LEGDATA  , tr( "&Convert Optima Data (Beckman tar.gz) " ), utilities );
  addMenu(  P_CONVERT  , tr( "&Import Experimental Data"         ), utilities );
  addMenu(  P_EXPORT   , tr( "&Export OpenAUC Data"              ), utilities );
  addMenu(  P_FDSMAN   , tr( "FDS File &Manager"                 ), utilities );
  addMenu(  P_PSEUDO_ABS  , tr( "&Pseudo-Absorbance"             ), utilities );
  addMenu(  P_VIEWTMST , tr( "View &TimeState"                   ), utilities );
  utilities->addSeparator();
  addMenu(  P_ABDE_FIT , tr( "&ABDE Analysis"                    ), utilities );
  addMenu(  P_SPECFIT  , tr( "&Spectrum Fitter"                  ), spectrum );
  addMenu(  P_SPECDEC  , tr( "Spectrum &Decomposition"           ), spectrum );
  utilities->addMenu(spectrum);
  addMenu(  P_RAMP     , tr( "Speed &Ramp Analysis"              ), utilities );
  addMenu(  P_COLORGRAD, tr( "Color &Gradient Generator"         ), utilities );
  addMenu(  P_RPTGEN   , tr( "&Report Generator"                 ), utilities );
  addMenu(  P_ROTORCAL , tr( "Rotor &Calibration"                ), utilities );
  addMenu(  P_LICENSE  , tr( "&License Manager"                  ), utilities );

  // addMenu(  P_GMPRPT   , tr( "&GMP Report Generator and Viewer"  ), utilities );
#if 0    // temporarily disable Create Experiment until truly ready
  addMenu(  P_CEXPERI  , tr( "Create E&xperiment"                ), utilities );
#endif

  addMenu(  P_VIEWMWL ,  tr( "&View Multiwavelength Data"        ), multiwave );
  addMenu(  P_VIEWMSS ,  tr( "View MWL-Spectra"               ), multiwave );
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

  QMenu* gmp = new QMenu( tr( "&GMP" ),  this );
  addMenu(  P_GMPACQ,   tr( "&Data Acquisition Routine" ), gmp );
  addMenu(  P_PROTOCOL, tr( "&Protocol Development" ),     gmp );
  addMenu(  P_GMPRPT,   tr( "&GMP Report Generator and Viewer"  ), gmp );
  addMenu(  P_ESIGN,    tr( "&e-Signature Assignment" ),   gmp );
  addMenu(  P_AUDIT,    tr( "&Audit Trail" ),              gmp );

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
  menuBar()->addMenu( gmp         );
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

   // get notices if available
   if ( US_Settings::default_data_location() != 2 ) {
      // notices only if location is database
      connect( &notices_get_url, SIGNAL( downloaded() ), this, SLOT( notices_ready() ), Qt::UniqueConnection );
      notices_get_url.get("https://ultrascan.aucsolutions.com/notices.json");
   }
}

US_Win::~US_Win()
{
   QString auto_positioning = US_Settings::debug_value("auto_positioning");
   if ( !auto_positioning.isEmpty() && auto_positioning.toLower() == "true" )
   {
      QPoint point = g.global_position();
      g.set_global_position( point - QPoint( 30, 30 ) );
   }
}

void US_Win::update_user_level( void ) {
   //Sync User-level with that set in DB //////////////////////////////////////////
   //If DB set ?
   QList<QStringList> DB_list = US_Settings::databases();
   QStringList defaultDB      = US_Settings::defaultDB();
   if ( !DB_list.empty() && !defaultDB.isEmpty() )
   {
      US_Passwd   pw;
      US_DB2      db( pw.getPasswd() );
      int last_error = db.lastErrno();

      if ( last_error == IUS_DB2::OK ) {
         QStringList q( "get_user_info" );
         db.query( q );
         db.next();

         int ID        = db.value( 0 ).toInt();
         QString fname = db.value( 1 ).toString();
         QString lname = db.value( 2 ).toString();
         int     level = db.value( 5 ).toInt();


         US_Settings::set_us_inv_name ( lname + ", " + fname );
         US_Settings::set_us_inv_ID   ( ID );
         US_Settings::set_us_inv_level( level );
      }
      else
      {
         QString error_msg = db.lastError();
         QMessageBox msgBox;
         msgBox.setWindowTitle ("ERROR: User Level Synchronization");
         msgBox.setText("Error making the DB connection! User-level cannot be synchronized.");
         msgBox.setInformativeText( error_msg );
         msgBox.setIcon  ( QMessageBox::Critical );
         msgBox.exec();
      }

   }
   //END OF user-level sync//////////////////////////////////////
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

   // notice_check now handled by SLOT notices_ready()
   // // At each launch, check for notices if last check was over 24 hours ago
   // if ( ln_time.secsTo( QDateTime::currentDateTime() ) > trig_secs ) {
   //    notice_check();

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
  QStringList args;
  process->start( pname, args );
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

  for (const auto d : procs)
  {
    names       += d->name + "\n";
  }

  QString isAre  = ( procs.size() > 1 ) ? "es are" : " is";
  QString itThem = ( procs.size() > 1 ) ? "them"   : "it";
  
  QMessageBox box;
  box.setWindowTitle( tr( "Attention" ) );
  box.setText( QString( tr( "The following process%1 still active:\n\n%2"
                            "\nYou can leave %3 open or close %3.\n"
                            "If a program is unresponsive, you can terminate it." )
                             .arg( isAre ).arg( names ).arg( itThem ) ) );

  QString leaveText = tr( "&Leave open" );
  QString closeText = tr( "&Close" );
  QString killText  = tr( "&Terminate" );

  QPushButton* close = box.addButton( closeText, QMessageBox::YesRole );
  QPushButton* kill  = box.addButton( killText , QMessageBox::NoRole );
  QPushButton* leave = box.addButton( leaveText, QMessageBox::NoRole );

  close->setToolTip( tr( "Close the process normally" ) );
  kill ->setToolTip( tr( "When process appears to be stuck" ) );
  leave->setToolTip( tr( "Leave the process running" ) );

  box.setDefaultButton( close );

  box.exec();

  if ( box.clickedButton() == leave ) return;

  for (const auto proc: procs )
  {
    procData* d       = proc;
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
  const int y =           menuBar  ()->size().rheight();
  const int height = 532 - y - statusBar()->size().rheight();
  const int w = 710;

  bigframe = new QLabel( this );
  bigframe->setFrameStyle        ( QFrame::Box | QFrame::Raised);
  bigframe->setPalette           ( US_GuiSettings::frameColor() );
  bigframe->setGeometry          ( 0, y, w, height );
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
    // Splash image (fixed size asset, like before)
    QPixmap rawpix = US_Images::getImage( US_Images::US3_SPLASH );
    const int ph   = rawpix.height();   // expected ~276
    const int pw   = rawpix.width();    // expected ~460
    Q_UNUSED(ph);

    QPixmap  pixmap( pw, ph );
    QPainter painter( &pixmap );
    painter.setRenderHint( QPainter::Antialiasing,     true );
    painter.setRenderHint( QPainter::TextAntialiasing, true );

    painter.drawPixmap( 0, 0, rawpix );

    // Colors (subtle, modern)
    const QColor versionColor      ( 245, 248, 255 );
    const QColor metaColor         ( 220, 228, 240 );
    const QColor authorsTitleColor ( 240, 245, 255 );
    const QColor authorsColor      ( 215, 225, 235 );
    const QColor dividerColor      ( 255, 255, 255, 80 );

    const QString uiFontFamily = "Sans Serif";

    // --- Compact vertical layout ---
    const int yVersion       = 120;   // Moved down for more space from silver
    const int versionToRevDate = 16;
    const int revDateToDivider = 14;
    const int dividerToTitle = 20;
    const int titleToFirst   = 16;
    const int nameStep       = 14;

    const int yRevDate      = yVersion + versionToRevDate;
    const int yDivider      = yRevDate + revDateToDivider;
    const int yAuthorsTitle = yDivider + dividerToTitle;
    const int firstNameBase = yAuthorsTitle + titleToFirst;

    // --- Version line (with build and Δ if local changes) ---
    QString version = QString("Version %1 (%2%3) for %4")
        .arg(US_Version)
        .arg(BUILDNUM)
        .arg(LOCAL_CHANGES)
        .arg(OS_TITLE);

    QFont versionFont( uiFontFamily );
    versionFont.setWeight       ( QFont::DemiBold );
    versionFont.setPixelSize    ( 18 );
    versionFont.setLetterSpacing( QFont::AbsoluteSpacing, 0.4 );

    painter.setFont( versionFont );
    painter.setPen ( versionColor );

    QFontMetrics vMetrics( versionFont );
    int vWidth = vMetrics.horizontalAdvance( version );
    painter.drawText( ( pw - vWidth ) / 2, yVersion, version );

    // --- Revision line (rev + date) ---
    QString revDateLine = QString("Source Identifier: %1 (%2)").arg(GIT_REVISION).arg(REVISION_DATE);

    QFont revDateFont( uiFontFamily );
    revDateFont.setWeight   ( QFont::Normal );
    revDateFont.setPixelSize( 12 );

    painter.setFont( revDateFont );
    painter.setPen ( metaColor );

    QFontMetrics rdMetrics( revDateFont );
    int rdWidth = rdMetrics.horizontalAdvance( revDateLine );
    painter.drawText( ( pw - rdWidth ) / 2, yRevDate, revDateLine );

    // --- Divider ---
    painter.setPen( QPen( dividerColor, 1 ) );
    const int dividerInset = 40;
    painter.drawLine( dividerInset, yDivider, pw - dividerInset, yDivider );

    // --- Authors title ---
    QFont authorsTitleFont( uiFontFamily );
    authorsTitleFont.setWeight   ( QFont::Bold );
    authorsTitleFont.setPixelSize( 13 );

    painter.setFont( authorsTitleFont );
    painter.setPen ( authorsTitleColor );

    const QString authorsTitle = "Authors";
    QFontMetrics atMetrics( authorsTitleFont );
    int atWidth = atMetrics.horizontalAdvance( authorsTitle );
    painter.drawText( ( pw - atWidth ) / 2, yAuthorsTitle, authorsTitle );

    // --- Author names ---
    QFont authorsFont( uiFontFamily );
    authorsFont.setWeight   ( QFont::Normal );
    authorsFont.setPixelSize( 11 );

    painter.setFont( authorsFont );
    painter.setPen ( authorsColor );

    QFontMetrics aMetrics( authorsFont );
    QStringList names = {
            "Borries Demeler",
            "Emre Brookes",
            "Alexey Savelyev",
            "Gary Gorbet"
    };

    for ( int i = 0; i < names.size(); ++i )
    {
        const QString& name = names[i];
        int nWidth = aMetrics.horizontalAdvance( name );
        int y      = firstNameBase + i * nameStep;
        painter.drawText( ( pw - nWidth ) / 2, y, name );
    }

    // --- Display ---
    const int splashX = static_cast<int>( ( width / 2 ) - 230 );
    const int splashY = 110;
    const int splashW = 460;
    const int splashH = 276;

    smallframe = new QLabel( this );
    smallframe->setPixmap( pixmap );
    smallframe->setGeometry( splashX, splashY, splashW, splashH );

    // --- Clickable "Additional contributors" link ---
    QLabel* contribLabel = new QLabel( smallframe );
    contribLabel->setText(
            "<a style='color:#FFFFFF; text-decoration:underline;' "
            "href=\"https://www.ultrascan.aucsolutions.com/contributors.php\">"
            "Additional contributors</a>"
    );
    contribLabel->setTextFormat( Qt::RichText );
    contribLabel->setTextInteractionFlags( Qt::TextBrowserInteraction );
    contribLabel->setOpenExternalLinks( true );
    contribLabel->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );

    QFont contribFont( uiFontFamily );
    contribFont.setPixelSize( 10 );
    contribLabel->setFont( contribFont );

    // Make spacing: last-author → link == link → bottom
    const int linkH = 16;  // height of the clickable label box
    // Baseline of last author:
    const int lastNameY = firstNameBase + ( names.size() - 1 ) * nameStep;
    // Approximate bottom of last author text:
    const int textBottom = lastNameY + aMetrics.descent();

    // Solve: textBottom + d + linkH + d = splashH  → d = (splashH - textBottom - linkH) / 2
    int remaining = splashH - textBottom - linkH;
    if ( remaining < 0 ) remaining = 0;   // safety
    int d = remaining / 2;

    int yLink = textBottom + d;
    contribLabel->setGeometry( 0, yLink, splashW, linkH );
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
            "Emre Brookes, Weiming Cao, Bruce Dubbs, Gary Gorbet,\n" 
            "Kip Jay, Jeremy Mann, Suresh Marru, Shabhaz Memon, Marlon Pierce,\n"
            "Alexey Savelyev, Raminder Singh and Dan Zollars.\n\n"
            "Project Director: Borries Demeler\n\n"
            "This development was funded by NIH Grants RR022200 and GM120600\n"
            "and NSF grants ACI-1339649 and TG-MCB070039.\n\n"

            "We thank the following individuals for their contributions\n"
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
            " * Todd Stone\n"
            " * Giovanni Tassara\n"
            " * James Trbovich\n"
            " * Oleg Tsodikov\n"
            " * Johannes Walter\n"
            " * Nikola Wenta\n"
            " * Josh Wilson" ) );

      statusBar()->showMessage( tr( "Ready" ) );
      break;

    case HELP_ABOUT:
      QMessageBox::information( this,
        tr( "About UltraScan..." ),
        tr( "UltraScan III version %1\n"
            "( build %2 )\n"
            "Copyright 1989 - 2025\n"
            "Borries Demeler\n\n"
            "For more information, please visit:\n"
            "https://www.ultrascan.aucsolutions.com/\n\n"
            "The author can be reached at:\n"
            "The University of Montana\n"
            "Department of Chemistry and Biochemistry\n"
            "32 Campus Drive\n"
            "Missoula, Montana  59812\n"
            "Phone:  (406) 285-1935\n"
            "E-mail: borries.demeler@umontana.edu" ).arg( US_Version ).arg( BUILDNUM ) );

      statusBar()->showMessage( tr( "Ready" ) );
      break;

    case HELP_NOTICES:
      ln_time    = ln_time.addYears( -1 );
      notices_get_url.get("https://ultrascan.aucsolutions.com/notices.json");
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

void US_Win::notices_ready() {
   qDebug() << "notices_ready()";

   QByteArray notices_bytearray = notices_get_url.downloadedData();
   if ( notices_bytearray.isEmpty() ) {
      qDebug() << "notices_ready(): notices are empty";
      return;
   }

   QJsonParseError parseError;
   QJsonDocument notices_jdoc;
   notices_jdoc = QJsonDocument::fromJson( notices_bytearray, &parseError );
   if ( parseError.error != QJsonParseError::NoError ) {
      qWarning() << "notices_ready(): JSON parse error at " << parseError.offset << ":" << parseError.errorString();
      return;
   }
      
   if ( notices_jdoc.isEmpty() ) {
      qDebug() << "notices_ready(): notices are empty";
      return;
   }

   if ( !notices_jdoc.isArray() ) {
      qWarning() << "notices_ready(): top level JSON is not an array";
      QTextStream( stdout ) << notices_jdoc.toJson() << "\n";
      return;
   }

   QJsonArray notices_jarray = notices_jdoc.array();
   
   QStringList msgs;
   QStringList revs;
   QStringList types;

   bool do_abort     = false;                         // Default: no abort
   int level         = 0;
   int nn_info       = 0;
   int nn_warn       = 0;
   int nn_crit       = 0;

   for ( auto it = notices_jarray.constBegin();
         it != notices_jarray.constEnd();
         ++it
         ) {
      const QJsonObject obj = it->toObject();
      if ( !obj.contains("message" ) || !obj.contains("revision" ) || !obj.contains("type") ) {
         qWarning() << "notices_ready(): JSON array element does not contain message, type & revision keys, skipping";
         continue;
      }
      msgs  << obj["message"].toString();
      revs  << obj["revision"].toString();
      types << obj["type"].toString();

      if ( types.back() == "info" )       nn_info++;
      else if ( types.back() == "warn" )  nn_warn++;
      else if ( types.back() == "crit" )  nn_crit++;
   }

   // Build notice message
   QMap< QString, QString > typeMap;
   typeMap[ "info" ] = tr( "Information" );
   typeMap[ "warn" ] = tr( "Warning"     );
   typeMap[ "crit" ] = tr( "Critical"    );

   level             = ( nn_warn > 0 ) ? 1 : level;
   level             = ( nn_crit > 0 ) ? 2 : level;
   QString msg_note  = tr( "UltraScan III notices posted :<br><br>" );
   bool empty_msg    = true;

   // Strips out any "-" data in version (e.g. 4.1.0-dev becomes 4.1.0)
   QVersionNumber base_ver = QVersionNumber::fromString(US_Version);
   int sys_revision = QString(BUILDNUM).toInt();

   // Extract only major.minor (first 2 segments) from base_ver
   QVector<int> base_segments = base_ver.segments();
   double sys_version = 0.0;
   if (base_segments.size() >= 2) {
       sys_version = base_segments[0] + (base_segments[1] / 10.0);  // e.g., 4 + 0.1 = 4.1
   }

   for ( int ii = 0; ii < (int) msgs.size(); ++ii )
   {
      double msg_version  = QString( "%1" ).arg( revs[ii] ).replace( QRegularExpression( "\\.\\d+$" ), "" ).toDouble();
      double msg_revision = QString( "%1" ).arg( revs[ii] ).replace( QRegularExpression( "^[^\\.]*\\.\\d+\\." ), "" ).toDouble();

      // Skip messages where message version.revision is less than our version.revision
      if ( sys_version > msg_version ||
           ( sys_version == msg_version &&
             sys_revision > msg_revision ) ) {
         continue;
      }

      // Add current message to full text
      msg_note         += typeMap[ types[ ii ] ] + " for release "
                       + revs[ ii ] + ":<br>"
                       + msgs[ ii ] + "<br>";

      empty_msg        = false;
      
      // Critical from later revision than current means an abort
      if ( types[ ii ] == "crit" ) {
         do_abort = true;
      }
   }

   if ( do_abort )
   {  // Append an additional note if an abort is happening
      msg_note         += tr( "<br><br>*** US3 Abort: UPDATE REQUIRED!!! ***<br>" );
      empty_msg        = false;
   }

   if ( !empty_msg ) {
      // Display notices at level of highest level currently set
      QWidget* wthis    = (QWidget*)this;
      if (      level == 0 )
         QMessageBox::information( wthis, tr( "US3 Notices" ), msg_note );
      else if ( level == 1 )
         QMessageBox::warning    ( wthis, tr( "US3 Notices" ), msg_note );
      else if ( level == 2 )
         QMessageBox::critical   ( wthis, tr( "US3 Notices" ), msg_note );
   }

   // Abort if that is indicated
   if ( do_abort ) {
      exit( -77 );
   }
}
