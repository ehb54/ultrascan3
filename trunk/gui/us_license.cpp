//! \file gui/us_license.cpp
#include "us_license.h"
#include "us_license_t.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_help.h"
#include "us_http_post.h"
#include "us_defines.h"

US_License::US_License( QWidget* parent, Qt::WindowFlags flags ) 
  : US_Widgets( parent, flags )
{
  setWindowTitle( "UltraScan License Configuration" );
  setPalette( US_GuiSettings::frameColor() );
  
  const int width = 5 * pushbutton + 4 * spacing;

  int xpos = spacing;
  int ypos = spacing;

  // Banner
  QLabel* banner = us_banner( 
     tr( "Please enter all fields exactly as shown in the issued license,\n"
         "or import a license from an E-mail text file:" ) );

  banner->setGeometry( 
      QRect( xpos, ypos, width, spacing + 2 * rowHeight ) );

  // Row 1 - Name
  ypos += rowHeight * 3 + spacing;

  QLabel* firstname = us_label( tr( "Name (first, last):" ), 0, QFont::Bold );
  firstname->setGeometry( xpos, ypos, buttonw, rowHeight );

  xpos += buttonw + spacing;

  le_firstname = us_lineedit( "" );
  le_firstname->setGeometry( xpos, ypos, half_buttonw, rowHeight );

  connect( le_firstname, SIGNAL( textChanged     ( const QString& ) ), 
                         SLOT  ( update_firstname( const QString& ) ) );

  xpos += half_buttonw + spacing;

  QLabel* comma = new QLabel( ",", this );
  comma->setGeometry( xpos, ypos, 10, rowHeight );

  xpos += spacing + 10;

  le_lastname = us_lineedit( "" );
  le_lastname->setGeometry( xpos, ypos, half_buttonw, rowHeight );
  
  connect( le_lastname, SIGNAL( textChanged    ( const QString& ) ), 
                        SLOT  ( update_lastname( const QString& ) ) );

  // Row 2 - Email
  xpos  = spacing;
  ypos += rowHeight + spacing;

  QLabel* email = us_label( tr( "E-mail Address:" ), 0, QFont::Bold );
  email->setGeometry( xpos, ypos, buttonw, rowHeight );

  xpos += buttonw + spacing;

  le_email = us_lineedit( "" );
  le_email->setGeometry( xpos, ypos, full_buttonw, rowHeight );
  connect( le_email, SIGNAL( textChanged ( const QString& ) ), 
                     SLOT  ( update_email( const QString& ) ) );

  // Row 3 - Institution
  xpos = spacing;
  ypos += rowHeight + spacing;

  QLabel* institution = us_label( tr("Institution:" ), 0, QFont::Bold );
  institution->setGeometry( xpos, ypos, buttonw, rowHeight );

  xpos += spacing + buttonw;

  le_institution = us_lineedit( "" );
  le_institution->setGeometry( xpos, ypos, full_buttonw, rowHeight );
  connect( le_institution, SIGNAL( textChanged       ( const QString& ) ), 
                           SLOT  ( update_institution( const QString& ) ) );

  // Row 4 - Address
  xpos = spacing;
  ypos += rowHeight + spacing;

  QLabel* address = us_label( tr( "Address:" ), 0, QFont::Bold );
  address->setGeometry( xpos, ypos, buttonw, rowHeight );

  xpos += buttonw + spacing;

  le_address =  us_lineedit( "" );
  le_address->setGeometry( xpos, ypos, full_buttonw, rowHeight );
  connect( le_address, SIGNAL( textChanged   ( const QString& ) ), 
                       SLOT  ( update_address( const QString& ) ) );

  // Row 5 - City/State/Zip
  xpos = spacing;
  ypos += rowHeight + spacing;

  QLabel* city = us_label( tr( "City:" ), 0, QFont::Bold );
  city->setGeometry( xpos, ypos, buttonw, rowHeight );

  xpos += spacing + buttonw;

  le_city = us_lineedit( "" );
  le_city->setGeometry( xpos, ypos, buttonw, rowHeight );
  connect( le_city, SIGNAL( textChanged( const QString& ) ), 
                    SLOT  ( update_city( const QString& ) ) );

  xpos += buttonw + spacing;

  QLabel* lb_state = us_label( tr( "State:" ), 0, QFont::Bold );
  lb_state->setGeometry( xpos, ypos, smallColumn, rowHeight );

  xpos += smallColumn + spacing;

  cbb_state = new QComboBox( this );
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
  connect( cbb_state, SIGNAL( currentIndexChanged( const QString& ) ), 
                      SLOT  ( update_state       ( const QString& ) ) );

  xpos += mediumColumn + spacing + 10; // Adjust

  QLabel* zip = us_label( tr( "Zip:" ), 0, QFont::Bold );
  zip->setGeometry( xpos, ypos, smallColumn, rowHeight );

  xpos += smallColumn + spacing;

  le_zip = us_lineedit( "" );
  le_zip->setGeometry( xpos, ypos, mediumColumn, rowHeight );
  connect( le_zip, SIGNAL( textChanged( const QString& ) ), 
                   SLOT  ( update_zip ( const QString& ) ) );

  // Row 6 - Phone/License Type/Version
  xpos  = spacing;
  ypos += rowHeight + spacing;
  
  QLabel* phone = us_label( tr( "Phone Number:" ), 0, QFont::Bold );
  phone->setGeometry( xpos, ypos, buttonw, rowHeight );

  xpos += spacing + buttonw;

  le_phone = us_lineedit( "" );;
  le_phone->setGeometry( xpos, ypos, buttonw, rowHeight );
  connect( le_phone, SIGNAL( textChanged ( const QString& ) ), 
                     SLOT  ( update_phone( const QString& ) ) );

  xpos += buttonw + spacing;

  QLabel* lb_licensetype = us_label( tr( "License:" ), 0, QFont::Bold );
  lb_licensetype->setGeometry( xpos, ypos, smallColumn, rowHeight );

  xpos += smallColumn + spacing;

  cbb_licensetype = new QComboBox( this );
  types << "academic" << "commercial" << "trial";
  cbb_licensetype->addItems( types );
  licensetype = "academic";  // Initialize
  cbb_licensetype->setGeometry( xpos, ypos, mediumColumn, rowHeight );
  
  connect( cbb_licensetype, SIGNAL( currentIndexChanged( const QString& ) ), 
                            SLOT  ( update_licensetype ( const QString& ) ) );
  
  xpos += mediumColumn + spacing + 10; // Adjust

  lbl_version = us_label( tr( "Version:" ), 0, QFont::Bold );
  lbl_version->setGeometry( xpos, ypos, smallColumn, rowHeight );

  xpos += smallColumn + spacing;

  xpos  = spacing;
  ypos += 2 * rowHeight + spacing;

  // Row 7 - Platform/Expiration/License Status
  lbl_platform = us_label( "", 0, QFont::Bold );
  lbl_platform->setGeometry( xpos, ypos, buttonw, rowHeight );

  xpos += buttonw + spacing;
  lbl_expiration = us_label( "", 0, QFont::Bold );
  lbl_expiration->setGeometry( xpos, ypos, buttonw, rowHeight );

  xpos += buttonw + spacing;

  lbl_valid = us_label( "", 0, QFont::Bold );
  lbl_valid->setGeometry( xpos, ypos, buttonw, rowHeight );

  xpos += buttonw + spacing;
  
  pb_update = us_pushbutton( tr( "Update / Renew" ) );
  pb_update->setGeometry( xpos, ypos, pushbutton, rowHeight );
  connect( pb_update, SIGNAL( clicked() ), SLOT( update() ) );

  QString error;
  QString validMsg;
  int     status = US_License_t::isValid( error, QStringList() );

  if ( status == US_License_t::Missing )
  {
    pb_update->setEnabled( false );
  }

  // Row 9 - Pushbuttons
  xpos  = spacing;
  ypos += spacing + rowHeight * 2;
  
  pb_request = us_pushbutton( tr( "Request New" ) );
  pb_request->setGeometry( xpos, ypos, pushbutton, rowHeight );
  connect( pb_request, SIGNAL( clicked() ), SLOT( request() ) );

  xpos += pushbutton + spacing;

  pb_import = us_pushbutton( tr( "E-mail Import" ) );
  pb_import->setGeometry( xpos, ypos, pushbutton, rowHeight );
  connect( pb_import, SIGNAL( clicked() ), SLOT( import() ) );

  xpos += pushbutton + spacing;

  pb_save = us_pushbutton( tr( "Save" ) );
  pb_save->setGeometry( xpos, ypos, pushbutton, rowHeight );
  connect( pb_save, SIGNAL( clicked() ), SLOT( save() ) );

  xpos += pushbutton + spacing;

  pb_help = us_pushbutton( tr( "Help" ) );
  pb_help->setGeometry( xpos, ypos, pushbutton, rowHeight );
  connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

  xpos += pushbutton + spacing;

  pb_cancel = us_pushbutton( tr( "Cancel" ) );
  pb_cancel->setGeometry( xpos, ypos, pushbutton, rowHeight );
  connect( pb_cancel, SIGNAL( clicked() ), SLOT( cancel() ) );

  // Finish up

  ypos += 30;    
  setMinimumSize( spacing + width + spacing, ypos );

  updating_email = false;
  load_current();
}

void US_License::cancel( void )
{
  close();
}

void US_License::help( void )
{
  online_help.show_help( "manual/license.html" );
}

QString US_License::titleCase( const QString& phrase )
{
  QString title = phrase; 
  bool    cap = true;

  for ( int i = 0; i < title.length(); i++ )
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
  QStringList license = US_Settings::license();

  if ( license.length() == 14 )
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
    validation  = license [ 12 ];
    expiration  = license [ 13 ];
  }

  update_screen();
}

void US_License::update_screen( void )
{
  le_firstname  ->setText( firstname   );
  le_lastname   ->setText( lastname    );
  le_institution->setText( institution );
  le_address    ->setText( address     );
  le_city       ->setText( city        );
  le_zip        ->setText( zip         );
  le_phone      ->setText( phone       );
  le_email      ->setText( email       );

  lbl_expiration->setText( "Expiration: " + expiration );
  lbl_version   ->setText( "Version: " + version );
  lbl_platform  ->setText( "Platform: " + platform + " / " + os );

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
      validMsg = "Valid License";
      break;

    case US_License_t::Expired:
      validMsg = "Expired License";
      break;

    case US_License_t::Invalid:
    case US_License_t::BadPlatform:
    case US_License_t::BadOS:
      validMsg = "Invalid License";
      break;

    case US_License_t::Missing:
      validMsg = "Missing License";
      break;

    default:
      validMsg = "Unknown";
  }

  lbl_valid->setText( validMsg );
}

void US_License::update( void )
{
  // If the email address has changed, just do a normal request.
  QStringList license = US_Settings::license();

  if ( license.length() > 8 )  // Skip if no license (2nd part of initial setup)
  {
    if ( le_email->text() != license[ 8 ]  && ! updating_email )
    {
      int result = QMessageBox::question( this,
          tr( "License Change" ),
          tr( "Your email address is changing.\n"
              "You must validate your new address using new license procedures\n"
              "Continue?" ),
          QMessageBox::Yes, QMessageBox::No );

      if ( result == QMessageBox::Yes )
        request();
      
      return;
    }
  }

  // Form request
  QString request = "firstname="    + firstname 
                  + "&lastname="    + lastname
                  + "&institution=" + institution
                  + "&address="     + address
                  + "&city="        + city
                  + "&state="       + state
                  + "&zip="         + zip
                  + "&phone="       + phone
                  + "&email="       + email
                  + "&os="          + OS
                  + "&platform="    + PLATFORM
                  + "&version="     + US_Version
                  + "&licensetype=" + licensetype;

  // Send request
  QString      url      = "http://ultrascan.uthscsa.edu/update-license.php";
  US_HttpPost* transmit = new US_HttpPost( url, request );
  connect( transmit, SIGNAL( US_Http_post_response( const QString& ) ),
           this,     SLOT  ( update_response      ( const QString& ) ) );
}

// Process response
void US_License::update_response( const QString& response )
{
  QStringList data = response.split( "-" );

  int error = data[ 0 ].toInt();
  if ( error == 0 )
  {
    code1      = data[ 1 ];
    code2      = data[ 2 ];
    code3      = data[ 3 ];
    code4      = data[ 4 ];
    code5      = data[ 5 ];
    expiration = data[ 6 ];
    version    = US_Version;
qDebug() << "Debug success: " << response;
    if ( save() ) update_screen();
  }
  else
    // Process possible errors
    QMessageBox::information ( this, 
      tr( "Update Error" ),
      response );
}

void US_License::request( void )
{
  // Form request
  QString request = "firstname="    + firstname 
                  + "&lastname="    + lastname
                  + "&institution=" + institution
                  + "&address="     + address
                  + "&city="        + city
                  + "&state="       + state
                  + "&zip="         + zip
                  + "&phone="       + phone
                  + "&email="       + email
                  + "&os="          + OS
                  + "&platform="    + PLATFORM
                  + "&version="     + US_Version
                  + "&licensetype=" + licensetype;

  // Send request
  QString      url      = "http://ultrascan.uthscsa.edu/request-license.php";
  US_HttpPost* transmit = new US_HttpPost( url, request );
  connect( transmit, SIGNAL( US_Http_post_response( const QString& ) ),
           this,     SLOT  ( request_response     ( const QString& ) ) );
}

void US_License::request_response( const QString& response )
{
  QStringList data = response.split( "-" );
  qDebug() << response;
  int error = data[ 0 ].toInt();
  if ( error == 0 )
  {
    pb_update->setEnabled( true );

    QMessageBox::information ( this, 
      tr( "Request Success" ),
      tr( "You have been sent an email to confirm your request.\n"
          "After you respond to the email, select 'Update / Renew'." ) );

  }
  else
  {
    // Process possible errors
    QMessageBox::information ( this, 
      tr( "Request Error" ),
      tr( "The response was " ) +  response );

    pb_update->setEnabled( true );
  } 

  updating_email = true; // Needed in the case of changing esisting email
}

bool US_License::save( void )
{
  QString     error;
  QString     message;
  QStringList license;

  QString validation = code1 + "-" + code2 + "-" + code3 + "-" + 
                       code4 + "-" + code5;

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
          "The date of the license is more than one year ago." );
      break;

    case US_License_t::Invalid:
      message = tr( "Failure.\n"
          "The data presented is invalid." );
      break;

    case US_License_t::BadPlatform:
      message = tr( "Failure.\n"
          "The wrong hardware architecture has been selected." );
      break;

    case US_License_t::BadOS:
      message = tr( "failure,\n"
          "The wrong operating system has been selected." );
      break;

    case US_License_t::OK:
      US_Settings::set_license( license );
      message        = tr( "Success" );
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

void US_License::import( void )
{
  QString line;

  QString filename = QFileDialog::getOpenFileName( this,
      tr( "Please select an e-mail file containing an UltraScan license..." ) );
  
  QFile texfile( filename );

  if ( texfile.open( QIODevice::ReadOnly ) )
  {
    QTextStream   ts( &texfile );
    QString cut = "_____________________CUT HERE_____________________";
    
    do
    {
      line = ts.readLine();
    } 
    while ( line != cut && ! ts.atEnd() );

    if ( ts.atEnd() )
    {
      QMessageBox::information( this,
          tr( "License Error" ), 
          tr( "This file does not seem to be an E-mail text file.\n"
              "It is missing the\n\n"
              "\"%1\"\n\n"
              "text string.\n"
              "Please load an e-mail license message file or add the string "
              "exactly as shown to the\n"
              "top of the license file so this program can recognize it." )
              .arg( cut ) );
      return;
    }

    firstname   = ts.readLine();
    lastname    = ts.readLine();
    institution = ts.readLine();
    address     = ts.readLine();
    city        = ts.readLine();
    state       = ts.readLine();
    zip         = ts.readLine();
    phone       = ts.readLine();
    email       = ts.readLine();
    platform    = titleCase( ts.readLine() );
    os          = titleCase( ts.readLine() );
    version     = ts.readLine();
    licensetype = ts.readLine();
    validation  = ts.readLine();
    expiration  = ts.readLine();
    texfile.close();

    update_screen();
  }
  else  // Open failed
    QMessageBox::information( this, tr( "License Error" ),
        tr( "Could not open license file\n%1" ).arg( filename ) );
}

