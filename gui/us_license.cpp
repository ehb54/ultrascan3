//! \file gui/us_license.cpp
#include "us_license.h"
#include "us_license_t.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_help.h"
#include "us_defines.h"
#include "us_http_post.h"

US_License::US_License( QWidget* parent, Qt::WindowFlags flags ) 
  : US_Widgets( true, parent, flags )
{
  setWindowTitle( "UltraScan III Registration" );
  setPalette( US_GuiSettings::frameColor() );
  
  // FIXME:  This layout is positioned with absolute values.  The appearance
  // will be poor if the user's settings are set with a large font size
  // (probably anything over 12pt).


  const int width = 5 * pushbutton + 4 * spacing;

  int xpos = spacing;
  int ypos = spacing;

  // Banner
  QLabel* banner = us_banner( 
     tr( "Please enter all fields" ) );

  banner->setGeometry( 
      QRect( xpos, ypos, width, spacing + rowHeight ) );

  // Row 1 - Name
  ypos += rowHeight + 2 * spacing;

  QLabel* firstname = us_label( tr( "Name (first, last):" ), 0, QFont::Bold );
  firstname->setGeometry( xpos, ypos, buttonw, rowHeight );

  xpos += buttonw + spacing;

  le_firstname = us_lineedit( "" );
  le_firstname->setGeometry( xpos, ypos, half_buttonw, rowHeight );

  connect( le_firstname, &QLineEdit::textChanged, this, &US_License::update_firstname );

  xpos += half_buttonw + spacing;

  QLabel* comma = new QLabel( ",", this );
  comma->setGeometry( xpos, ypos, 10, rowHeight );

  xpos += spacing + 10;

  le_lastname = us_lineedit( "" );
  le_lastname->setGeometry( xpos, ypos, half_buttonw, rowHeight );
  
  connect( le_lastname, &QLineEdit::textChanged, this, &US_License::update_lastname );

  // Row 2 - Email
  xpos  = spacing;
  ypos += rowHeight + spacing;

  QLabel* email = us_label( tr( "E-mail Address:" ), 0, QFont::Bold );
  email->setGeometry( xpos, ypos, buttonw, rowHeight );

  xpos += buttonw + spacing;

  le_email = us_lineedit( "" );
  le_email->setGeometry( xpos, ypos, full_buttonw, rowHeight );
  connect( le_email, &QLineEdit::textChanged, this, &US_License::update_email );

  // Row 3 - Institution
  xpos = spacing;
  ypos += rowHeight + spacing;

  QLabel* institution = us_label( tr("Institution:" ), 0, QFont::Bold );
  institution->setGeometry( xpos, ypos, buttonw, rowHeight );

  xpos += spacing + buttonw;

  le_institution = us_lineedit( "" );
  le_institution->setGeometry( xpos, ypos, full_buttonw, rowHeight );
  connect( le_institution, &QLineEdit::textChanged, this, &US_License::update_institution );

  // Row 5 - City/State/Zip
  xpos = spacing;
  ypos += rowHeight + spacing;

  QLabel* city = us_label( tr( "City:" ), 0, QFont::Bold );
  city->setGeometry( xpos, ypos, buttonw, rowHeight );

  xpos += spacing + buttonw;

  le_city = us_lineedit( "" );
  le_city->setGeometry( xpos, ypos, buttonw, rowHeight );
  connect( le_city, &QLineEdit::textChanged, this, &US_License::update_city );

  xpos += buttonw + spacing;

  QLabel* lb_state = us_label( tr( "State:" ), 0, QFont::Bold );
  lb_state->setGeometry( xpos, ypos, smallColumn, rowHeight );

  xpos += smallColumn + spacing;

  cbb_state = us_comboBox();
  states << "NON-US" << "AL" << "AR" << "AZ" << "CA" << "CO" << "CT"
          << "DC"    << "DE" << "FL" << "GA" << "HI" << "IA" << "ID"
          << "IL"    << "IN" << "KS" << "KY" << "LA" << "MA" << "MD" 
          << "ME"    << "MI" << "MN" << "MO" << "MS" << "MT" << "NC" 
          << "ND"    << "NE" << "NH" << "NJ" << "NM" << "NV" << "NY" 
          << "OH"    << "OK" << "OR" << "PA" << "PR" << "RI" << "SC" 
          << "SD"    << "TN" << "TX" << "UT" << "VA" << "VI" << "VT" 
          << "WA"    << "WI" << "WV" << "WY" ;

  cbb_state->addItems( states );

  state = "NON-US"; // Initialize
  cbb_state->setGeometry( xpos, ypos, mediumColumn, rowHeight );
  connect( cbb_state, &QComboBox::currentTextChanged, this, &US_License::update_state );

  xpos += mediumColumn + spacing + 10; // Adjust

  //xpos  = spacing;
 // ypos += rowHeight + spacing;
  
 // xpos += spacing + buttonw;

 // xpos += buttonw + spacing;

  QLabel* lb_licensetype = us_label( tr( "License:" ), 0, QFont::Bold );
  lb_licensetype->setGeometry( xpos, ypos, smallColumn, rowHeight );

  xpos += smallColumn + spacing;

  cbb_licensetype = us_comboBox();
  types << "academic" << "commercial";
  cbb_licensetype->addItems( types );
  licensetype = "academic";  // Initialize
  cbb_licensetype->setGeometry( xpos, ypos, mediumColumn, rowHeight );
  
  connect( cbb_licensetype, &QComboBox::currentTextChanged, this, &US_License::update_licensetype );
  
  xpos  = spacing;
  ypos += rowHeight + spacing;

  QLabel* status = us_banner( tr( "License Status" ) );
  status->setGeometry( QRect( xpos, ypos, width, spacing + rowHeight ) );


  // Very light gray, for read-only line edits
  QPalette gray = US_GuiSettings::readonlyColor();

  // Row 7 - Platform
  xpos  = spacing;
  ypos += rowHeight + 2 * spacing;

  lbl_platform = us_label( "Platform / OS:", 0, QFont::Bold );
  lbl_platform->setGeometry( xpos, ypos, buttonw, rowHeight );

  xpos += buttonw + spacing;

  le_platform = us_lineedit( "", 0 );
  le_platform->setPalette ( gray );
  le_platform->setReadOnly( true );
  le_platform->setGeometry( xpos, ypos, full_buttonw, rowHeight );

  // Row 8 - Expiration
  xpos  = spacing;
  ypos += rowHeight + spacing;
  
  lbl_expiration = us_label( "Expiration:", 0, QFont::Bold );
  lbl_expiration->setGeometry( xpos, ypos, buttonw, rowHeight );

  xpos += buttonw + spacing;

  le_expiration = us_lineedit( "", 0 );
  le_expiration->setPalette ( gray );
  le_expiration->setReadOnly( true );
  le_expiration->setGeometry( xpos, ypos, full_buttonw, rowHeight );

  // Row 9 - Version
  xpos  = spacing;
  ypos += rowHeight + spacing;

  lbl_valid = us_label( "UltraScan III Version:", 0, QFont::Bold );
  lbl_valid->setGeometry( xpos, ypos, buttonw, rowHeight );

  xpos += buttonw + spacing;

  le_registration = us_lineedit( "", 0 );
  le_registration->setPalette ( gray );
  le_registration->setReadOnly( true );
  le_registration->setGeometry( xpos, ypos, full_buttonw, rowHeight );

  // Row 10 - Pushbuttons
  xpos  = spacing + pushbutton + spacing;
  ypos += spacing + rowHeight;
  
  pb_update = us_pushbutton( tr( "Register" ) );
  pb_update->setGeometry( xpos, ypos, pushbutton, rowHeight );
  connect( pb_update, &QPushButton::clicked, this, &US_License::update );

  QString error;
  QString validMsg;

  switch ( US_License_t::isValid( error, QStringList() ) )
  {
     case US_License_t::Pending:
        pb_update->setText( "Finish Registration" );
        break;

     case US_License_t::OK:
     case US_License_t::Expired:
        pb_update->setText( "Update / Renew" );
        break;
  }

  xpos += pushbutton + spacing;

  pb_help = us_pushbutton( tr( "Help" ) );
  pb_help->setGeometry( xpos, ypos, pushbutton, rowHeight );
  connect( pb_help, &QPushButton::clicked, this, &US_License::help );

  xpos += pushbutton + spacing;

  pb_cancel = us_pushbutton( tr( "Close" ) );
  pb_cancel->setGeometry( xpos, ypos, pushbutton, rowHeight );
  connect( pb_cancel, &QPushButton::clicked, this, &QWidget::close );

  // Finish up
  ypos += 30;    
  setMinimumSize( spacing + width + spacing, ypos );

  updating_email = false;
  load_current();
}

void US_License::help( void )
{
  online_help.show_help( "manual/register.html" );
}

QString US_License::titleCase( const QString& phrase )
{
  QString title = phrase; 
  bool    cap = true;

  for ( int i = 0; i < title.size(); i++ )
  {
    QChar c = title[ i ];
    
    if ( c == QChar( ' ' ) ) cap = true;
    
    if ( cap )
    {
      title[ i ] = c.toUpper();
      cap = false;
    }
    else
      title[ i ] = c.toLower();
  }

  return title;
}

void US_License::load_current( void )
{
  lastname    = "";
  firstname   = "";
  institution = "";
  address     = "n/a";
  city        = "";
  state       = "NON-US";
  zip         = "n/a";
  phone       = "n/a";
  email       = "";
  version     = US_Version;  // us_defines.h
  validation  = "";
  expiration  = "";

  os          = titleCase( OS );
  platform    = titleCase( PLATFORM );
   
  QStringList license = US_Settings::license();

  if ( license.size() >= 12 )
  {
    lastname    = license [ 0 ];
    firstname   = license [ 1 ];
    institution = license [ 2 ];
    address     = license [ 3 ];
    city        = license [ 4 ];
    state       = license [ 5 ];
    zip         = license [ 6 ];
    phone       = license [ 7 ];
    email       = license [ 8 ];
    platform    = titleCase( license [ 9 ]  );
    os          = titleCase( license [ 10 ] );
    version     = license [ 11 ];
  }

  if ( license.size() == 14 )
  {
    validation  = license [ 12 ];
    expiration  = license [ 13 ];
  }
  // Force unused fields to "n/a"
  address     = "n/a";
  zip         = "n/a";
  phone       = "n/a";

  update_screen();
}

void US_License::update_screen( void )
{
  le_firstname   ->setText( firstname   );
  le_lastname    ->setText( lastname    );
  le_institution ->setText( institution );
  le_city        ->setText( city        );
  le_email       ->setText( email       );

  le_platform    ->setText( platform + " / " + os );
  le_expiration  ->setText( expiration );
  le_registration->setText( version );

  // License type
  for ( int i = 0; i < types.count(); i++ )
  {
    if ( licensetype == types.at( i ) )
    {
      cbb_licensetype->setCurrentIndex( i );
      break;
    }
  }

  // State
  for ( int i = 0; i < states.count(); i++ )
  {
    if ( state == states.value( i ) )
    {
      cbb_state->setCurrentIndex( i );
      break;
     }
   } 

  // Update status
  QString error;
  QString validMsg;
  int     status = US_License_t::isValid( error, QStringList() );
  
  switch ( status )
  {
    case US_License_t::OK:
      validMsg = tr( "Valid Registration" );
      break;

    case US_License_t::Expired:
      validMsg = tr( "Expired Registration" );
      break;

    case US_License_t::Invalid:
    case US_License_t::BadPlatform:
    case US_License_t::BadOS:
      validMsg = tr( "Invalid Registration" );
      break;

    case US_License_t::Missing:
      validMsg = tr( "Missing Registration" );
      break;

    case US_License_t::Pending:
      validMsg = tr( "Pending Registration" );
      break;

    default:
      validMsg = tr( "Unknown" );
  }

//lbl_valid->setText( validMsg );
  validMsg = version + "       [ " + validMsg + " ]";
  le_registration->setText( validMsg );
}

void US_License::update( void )
{
   // The registration scripts refuse a request with any of these empty
   QStringList missing;

   if ( firstname.isEmpty() )
   {
      missing << tr( "First name" );
   }
   if ( lastname.isEmpty() )
   {
      missing << tr( "Last name" );
   }
   if ( email.isEmpty() )
   {
      missing << tr( "E-mail Address" );
   }
   if ( institution.isEmpty() )
   {
      missing << tr( "Institution" );
   }
   if ( city.isEmpty() )
   {
      missing << tr( "City" );
   }
   if ( state.isEmpty() )
   {
      missing << tr( "State" );
   }

   if ( !missing.isEmpty() )
   {
      QMessageBox::information( this,
                                tr( "Request Error" ),
                                tr( "Your request is incomplete.\n"
                                    "Please fill in: %1." )
                                   .arg( missing.join( ", " ) ) );

      return;
   }

  // Nothing is sent or saved unless the registration server can be reached.
  // This comes before the e-mail-change question, so nobody is asked to
  // confirm a change that cannot be sent.
  if ( !server_reachable() )
  {
     return;
  }

  // If the email address has changed, just do a normal request.
  QStringList license = US_Settings::license();

  if ( license.size() > 8 )
  {
    // qDebug() << "license len > 8";
    if ( le_email->text() != license[ 8 ] )
    {
      int result = QMessageBox::question( this,
          tr( "License Change" ),
          tr( "Your email address is changing.\n"
              "You must validate your new address using new registration procedures.\n"
              "Continue?" ),
          QMessageBox::Yes, QMessageBox::No );

      if ( result == QMessageBox::No ) 
      {
         return;
      }
      else
      {
         request();
         return;
      }
    }
  }
  else
  {
    request();
    return;
  }

//qDebug() << "requesting update";
  // Send request
  QString url     = "https://ultrascan.aucsolutions.com/update-license-new.php";
  US_HttpPost* transmit = new US_HttpPost( url, form_data() );
  connect( transmit, &US_HttpPost::US_Http_post_response, this, &US_License::update_response );
}

// Process response
void US_License::update_response( const QString& response )
{
  QStringList data = response.split( "-" );

  int error = data[ 0 ].toInt();

  // 5: the server has no record of this email.  If the saved registration
  // is still pending (12 entries: saved by request(), no key yet), its
  // request never succeeded (refused, or lost to a network error), so send
  // the request again.  Otherwise every later attempt with this address
  // would end here.  The size is tested directly, because isValid() reports
  // a pending record from another platform or OS as BadPlatform or BadOS.
  if ( error == 5 && US_Settings::license().size() == 12 )
  {
     request();
     return;
  }

  // 0 comes with a key and an expiration; a shorter reply is an error
  if ( error == 0 && data.size() >= 3 )
  {
    validation = data[ 1 ];
    expiration = data[ 2 ];
    version    = US_Version;

    if ( save() ) 
    {
      pb_update->setText( "Update / Renew" );
      update_screen();
    }

    // After registration, ensure basic user directories are created
    QDir dir;
    dir.mkpath( US_Settings::dataDir() );
    dir.mkpath( US_Settings::resultDir() );
    dir.mkpath( US_Settings::reportDir() );
    dir.mkpath( US_Settings::tmpDir() );
  }
  else
    // Process possible errors
    QMessageBox::information ( this, 
      tr( "Update Error" ), response );
}

void US_License::request( void )
{
   //qDebug() << "Starting Req";
   // A new request is for this computer (form_data() sends PLATFORM and OS),
   // whatever a saved record from another one says
   platform = titleCase( PLATFORM );
   os       = titleCase( OS );

   // Save partial request
   QStringList license;
   license << lastname << firstname << institution << address << city << state << zip << phone << email << platform.toLower()
           << os.toLower() << version;

   US_Settings::set_license( license );

   // Send request
   //qDebug() << "sending: " << form_data();
   QString      url      = "https://ultrascan.aucsolutions.com/request-license.php";
   US_HttpPost* transmit = new US_HttpPost( url, form_data() );
   connect( transmit, &US_HttpPost::US_Http_post_response, this, &US_License::request_response );
}

void US_License::request_response( const QString& response )
{
  QStringList data = response.split( "-" );
  //qDebug() << data;
  int error = data[ 0 ].toInt();

  //qDebug() << "Request response data: " << error;
  if ( error == 0 )
  {
    pb_update->setText( "Finish Registration" );

    QMessageBox::information ( this, 
      tr( "Request Success" ),
      tr( "You have been sent an email to confirm your request.\n"
          "After you respond to the email, select 'Finish Registration'." ) );
  }
  else
  {
    // Process possible errors
    QMessageBox::information ( this, 
      tr( "Request Error" ),
      response );
     
    return;
  } 
}

// The body of a registration POST (application/x-www-form-urlencoded).
// Each value is percent-encoded, so an '&', '=', '+' or '%' in a field
// reaches the server as typed.  The bytes stay Latin-1, as they were when
// the values went unencoded, so the server stores what it stored before.
QString US_License::form_data( void ) const
{
   const QStringList names  = { "firstname", "lastname", "institution", "address",  "city",    "state",      "zip",
                                "phone",     "email",    "os",          "platform", "version", "licensetype" };
   const QStringList values = { firstname, lastname, institution, address,  city,       state,      zip,
                                phone,     email,    OS,          PLATFORM, US_Version, licensetype };
   QStringList       items;

   Q_ASSERT( names.size() == values.size() );
   for ( int i = 0; i < names.size(); i++ )
   {
      items << names[i] + "=" + QString::fromLatin1( values[i].toLatin1().toPercentEncoding() );
   }

   return items.join( "&" );
}

bool US_License::save( void )
{
  QString     error;
  QString     message;
  QStringList license;

  license << lastname              << firstname 
          << institution           << address 
          << city                  << state                 
          << zip                   << phone       
          << email                 << platform.toLower() 
          << os.toLower()          << version   
          << validation.toUpper()  << expiration; 

  bool isOK = false;
  switch ( US_License_t::isValid( error, license ) )
  {
    case US_License_t::Expired:
      message = tr( "Failure.\n"
          "The date of the registration is more than one year ago." );
      break;

    case US_License_t::Invalid:
      message = tr( "Failure.\n"
          "The data presented is invalid." ) + error;
      break;

    case US_License_t::BadPlatform:
      message = tr( "Failure.\n"
          "The wrong hardware architecture has been selected." );
      break;

    case US_License_t::BadOS:
      message = tr( "Failure.\n"
          "The wrong operating system has been selected." );
      break;

    case US_License_t::OK:
      US_Settings::set_license( license );
      message        = tr( "Success.\n"
          "The registration has been saved successfully." );
      isOK           = true;
      updating_email = false;
      break;

    default:
      message = tr( "An unknown error occured." );
      break;
  }

  QMessageBox::information( this, tr( "License save results" ), message );
  return isOK;
}

QString US_License::trim( const QString& st )
{
   QString s = st;
   QRegularExpression blanks( "(^\\s+)|(\\s+$)" );

   s.replace( blanks, "" );
   return s;
}

// Can the registration server be reached on port 443 (HTTPS)?  This asks for
// the small static file the main window reads its notices from, the same way
// US_HttpPost sends a registration: through the same proxy, with the same TLS
// settings.  Any HTTP answer means the port is open, a redirect included.
// Otherwise the user is told why, and update() stops before anything is sent
// or saved.  It also stops, without a message, if the dialog was closed
// during the check.
bool US_License::server_reachable( void )
{
   const int         timeout_s = 15;
   const QUrl        url( "https://ultrascan.aucsolutions.com/notices.json" );
   QNetworkRequest   request( url );
   QSslConfiguration conf = request.sslConfiguration();

   conf.setPeerVerifyMode( QSslSocket::VerifyNone ); // as in US_HttpPost
   request.setSslConfiguration( conf );
   // Qt 6 follows redirects by default; the server's own answer is enough
   request.setAttribute( QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy );

   QNetworkAccessManager manager;
   QEventLoop            loop;
   QTimer                timer;
   QNetworkReply*        reply   = manager.head( request );
   const bool            visible = isVisible();

   timer.setSingleShot( true );
   connect( &timer, &QTimer::timeout, reply, &QNetworkReply::abort );
   connect( reply, &QNetworkReply::finished, &loop, &QEventLoop::quit );
   timer.start( timeout_s * 1000 );

   // The loop holds back clicks and key presses and delivers them when it
   // ends.  They then reach a disabled dialog, which drops them, so another
   // click on Register cannot start a second request.
   setEnabled( false );
   QApplication::setOverrideCursor( Qt::WaitCursor );
   if ( !reply->isFinished() )
   {
      loop.exec( QEventLoop::ExcludeUserInputEvents );
   }
   QCoreApplication::processEvents();
   QApplication::restoreOverrideCursor();
   setEnabled( true );

   const bool closed    = visible && !isVisible();
   const bool reachable = !closed && reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).isValid();

   if ( !reachable && !closed )
   {
      // Only the timer aborts the request
      const QString why = reply->error() == QNetworkReply::OperationCanceledError
                             ? tr( "No answer within %1 seconds." ).arg( timeout_s )
                             : reply->errorString();

      QMessageBox::warning( this,
                            tr( "Connection Error" ),
                            tr( "UltraScan cannot reach the registration server\n"
                                "%1 on port %2 (HTTPS):\n"
                                "%3\n\n"
                                "Nothing has been sent or saved.\n\n"
                                "Please check your network connection and try again.  "
                                "If this keeps happening, a firewall or proxy may be "
                                "blocking outgoing HTTPS to %1; your network "
                                "administrator must allow it, at least temporarily, "
                                "to obtain or renew the license." )
                               .arg( url.host(), QString::number( url.port( 443 ) ), why ) );
   }

   delete reply;
   return reachable;
}
