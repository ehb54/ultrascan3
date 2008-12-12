//! \file us_license.cpp
#include "us_license.h"
#include "us_license_t.h"
#include "us_settings.h"
#include "us_help.h"

US_License::US_License( QWidget* parent, Qt::WindowFlags flags ) 
  : US_Widgets( parent, flags )
{
  setWindowTitle( "UltraScan License Configuration" );

#define spacing        5
#define commaWidth    10
#define buttonw      180
#define span         600
#define full_buttonw 610
#define half_buttonw 295
#define pushbutton   156
#define rowHeight     26

  int width = 5 * pushbutton + 4 * spacing;

  int xpos = spacing;
  int ypos = spacing;

  // Banner
  QLabel* banner = us_banner( 
     tr( "Please enter all fields exactly as shown in the issued license,\n"
         "or import a license from an E-mail text file:" ) );

  banner->setPalette( palette() ); // Back to default
  banner->setGeometry( 
      QRect( xpos, ypos, width, spacing + 2 * rowHeight ) );

  // Row 1 - Name
  ypos += rowHeight * 3 + spacing;

  QLabel* firstname = us_label( tr( "Name (first, last):" ) );
  firstname->setFrameStyle( QFrame::NoFrame );
  firstname->setGeometry( xpos, ypos, buttonw, rowHeight );

  xpos += buttonw + spacing;

  le_firstname = us_lineedit( "" );
  le_firstname->setGeometry( xpos, ypos, half_buttonw, rowHeight );

  connect( le_firstname, SIGNAL( textChanged     ( const QString& ) ), 
                         SLOT  ( update_firstname( const QString& ) ) );

  xpos += half_buttonw + spacing;

  QLabel* comma = us_label( "," );
  comma->setFrameStyle( QFrame::NoFrame );
  comma->setMargin( 0 );
  comma->setGeometry( xpos, ypos, 10, rowHeight );

  xpos += spacing + 10;

  le_lastname = us_lineedit( "" );
  le_lastname->setGeometry( xpos, ypos, half_buttonw, rowHeight );
  
  connect( le_lastname, SIGNAL( textChanged    ( const QString& ) ), 
                        SLOT  ( update_lastname( const QString& ) ) );

  // Row 2 - Email
  xpos  = spacing;
  ypos += rowHeight + spacing;

  QLabel* email = us_label( tr( "E-mail Address:" ) );
  email->setFrameStyle( QFrame::NoFrame );
  email->setGeometry( xpos, ypos, buttonw, rowHeight );

  xpos += buttonw + spacing;

  le_email = us_lineedit( "" );
  le_email->setGeometry( xpos, ypos, full_buttonw, rowHeight );
  connect( le_email, SIGNAL( textChanged ( const QString& ) ), 
                     SLOT  ( update_email( const QString& ) ) );

  // Row 3 - Institution
  xpos = spacing;
  ypos += rowHeight + spacing;

  QLabel* institution = us_label( tr("Institution:" ) );
  institution->setFrameStyle( QFrame::NoFrame );
  institution->setGeometry( xpos, ypos, buttonw, rowHeight );

  xpos += spacing + buttonw;

  le_institution = us_lineedit( "" );
  le_institution->setGeometry( xpos, ypos, full_buttonw, rowHeight );
  connect( le_institution, SIGNAL( textChanged       ( const QString& ) ), 
                           SLOT  ( update_institution( const QString& ) ) );

  // Row 4 - Address
  xpos = spacing;
  ypos += rowHeight + spacing;

  QLabel* address = us_label( tr( "Address:" ) );
  address->setFrameStyle( QFrame::NoFrame );
  address->setGeometry( xpos, ypos, buttonw, rowHeight );

  xpos += buttonw + spacing;

  le_address =  us_lineedit( "" );
  le_address->setGeometry( xpos, ypos, full_buttonw, rowHeight );
  connect( le_address, SIGNAL( textChanged   ( const QString& ) ), 
                       SLOT  ( update_address( const QString& ) ) );

  // Row 5 - City/State/Zip
  xpos = spacing;
  ypos += rowHeight + spacing;

  QLabel* city = us_label( tr( "City:" ) );
  city->setFrameStyle( QFrame::NoFrame );
  city->setGeometry( xpos, ypos, buttonw, rowHeight );

  xpos += spacing + buttonw;

  le_city = us_lineedit( "" );
  le_city->setGeometry( xpos, ypos, buttonw, rowHeight );
  connect( le_city, SIGNAL( textChanged( const QString& ) ), 
                    SLOT  ( update_city( const QString& ) ) );

  xpos += buttonw + spacing;
#define smallColumn 90

  QLabel* lb_state = us_label( tr( "State:" ) );
  lb_state->setFrameStyle( QFrame::NoFrame );
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

#define mediumColumn 110
  state = "NON-US"; // Initialize
  cbb_state->setGeometry( xpos, ypos, mediumColumn, rowHeight );
  connect( cbb_state, SIGNAL( currentIndexChanged( const QString& ) ), 
                      SLOT  ( update_state       ( const QString& ) ) );

  xpos += mediumColumn + spacing + 10; // Adjust

  QLabel* zip = us_label( tr( "Zip:" ) );
  zip->setFrameStyle( QFrame::NoFrame );
  zip->setGeometry( xpos, ypos, smallColumn, rowHeight );

  xpos += smallColumn + spacing;

  le_zip = new QLineEdit( this );
  le_zip->setGeometry( xpos, ypos, mediumColumn, rowHeight );
  connect( le_zip, SIGNAL( textChanged( const QString& ) ), 
                   SLOT  ( update_zip ( const QString& ) ) );

  // Row 6 - Phone/License Type/Version
  xpos  = spacing;
  ypos += rowHeight + spacing;
  
  QLabel* phone = us_label( tr( "Phone Number:" ) );
  phone->setFrameStyle( QFrame::NoFrame );
  phone->setGeometry( xpos, ypos, buttonw, rowHeight );

  xpos += spacing + buttonw;

  le_phone = us_lineedit( "" );;
  le_phone->setGeometry( xpos, ypos, buttonw, rowHeight );
  connect( le_phone, SIGNAL( textChanged ( const QString& ) ), 
                     SLOT  ( update_phone( const QString& ) ) );

  xpos += buttonw + spacing;

  QLabel* lb_licensetype = us_label( tr( "License:" ) );
  lb_licensetype->setFrameStyle( QFrame::NoFrame );
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

  QLabel* lb_version = us_label( tr( "Version:" ) );
  lb_version->setFrameStyle( QFrame::NoFrame );
  lb_version->setGeometry( xpos, ypos, smallColumn, rowHeight );

  xpos += smallColumn + spacing;

  cbb_version = new QComboBox( this );
  versions <<  "6.0" << "6.2" << "7.0" << "7.1" << "7.2" << "7.3"
           <<  "7.4" << "8.0" << "8.1" << "9.0" << "9.2" << "9.3"
           <<  "9.4" << "9.5" << "9.6" << "9.7" << "9.8" << "9.9"
           << "10.0"; 
  cbb_version->addItems( versions );
  version = "10.0";  // Initialize
  cbb_version->setCurrentIndex( versions.length() - 1 );
  cbb_version->setGeometry( xpos, ypos, mediumColumn, rowHeight );
  
  connect( cbb_version, SIGNAL( currentIndexChanged( const QString& ) ), 
                        SLOT  ( update_version     ( const QString& ) ) );

  xpos  = spacing;
  ypos += 2 * rowHeight + spacing;

  // Row 7 - Platform
  QLabel* lb_platform = us_label( tr( "Platform:" ) );
  lb_platform->setFrameStyle( QFrame::NoFrame );
  lb_platform->setGeometry( xpos, ypos, buttonw, rowHeight );

  xpos += buttonw + spacing;

  rb_intel = new QRadioButton( tr( "Intel" ), this );
  rb_intel->setGeometry( xpos, ypos, buttonw, rowHeight );
  rb_intel->setChecked( true );
  platform = "Intel";  // Initialize
  connect( rb_intel, SIGNAL( clicked() ), SLOT( update_intel_rb() ) );

  xpos += spacing * 4 + buttonw;

  lb_os = new QListWidget( this );
  
  QStringList labels;
  labels << "Linux"   << "Windows" << "Mac OS-X" << "Irix 6.5"
         << "Solaris" << "FreeBSD" << "NetBSD"   << "OpenBSD";
  
  lb_os->addItems( labels );
  lb_os-> setCurrentRow( 0 );
  os = "linux";
  lb_os->setGeometry( xpos, ypos, half_buttonw, rowHeight * 5 + spacing * 4 );

  connect( lb_os, SIGNAL( currentRowChanged( int ) ), 
                  SLOT  ( update_os        ( int ) ) );

  ypos += rowHeight + spacing;
  xpos = buttonw + spacing * 2;

  rb_sparc = new QRadioButton( tr( "Sparc" ), this );
  rb_sparc->setGeometry( xpos, ypos, buttonw, rowHeight );
  connect( rb_sparc, SIGNAL( clicked() ), SLOT( update_sparc_rb() ) );

  ypos += rowHeight + spacing;

  rb_mac = new QRadioButton( tr( "Macintosh" ), this );
  rb_mac->setGeometry( xpos, ypos, buttonw, rowHeight );
  connect( rb_mac, SIGNAL( clicked() ), SLOT( update_mac_rb() ) );

  ypos += rowHeight + spacing;

  rb_opteron = new QRadioButton( tr( "Opteron" ), this );
  rb_opteron->setGeometry( xpos, ypos, buttonw, rowHeight );
  connect( rb_opteron, SIGNAL( clicked() ), SLOT( update_opteron_rb() ) );

  ypos += rowHeight + spacing;

  rb_sgi = new QRadioButton( tr( "Silicon Graphics" ), this );
  rb_sgi->setGeometry( xpos, ypos, buttonw, rowHeight );
  connect( rb_sgi, SIGNAL( clicked() ), SLOT( update_sgi_rb() ) );

  // Row 8 - License Code
  ypos += spacing + 2 * rowHeight;
  xpos  = spacing;
  
  QLabel* code = us_label( tr( "License Code:" ) );
  code->setFrameStyle( QFrame::NoFrame );
  code->setGeometry( xpos, ypos, buttonw, rowHeight );

  xpos += buttonw + spacing;

#define codeWidth 60

  le_code1 = us_lineedit( "" );
  le_code1->setGeometry( xpos, ypos, codeWidth, rowHeight );
  connect( le_code1, SIGNAL( textChanged ( const QString& ) ), 
                     SLOT  ( update_code1( const QString& ) ) );

  xpos += codeWidth + spacing;

  code = us_label( "-", 1 );
  code->setFrameStyle( QFrame::NoFrame );
  code->setMargin( 3 );
  code->setGeometry( xpos, ypos, commaWidth + 5, rowHeight );

  xpos += commaWidth + spacing;

  le_code2 = us_lineedit( "" );
  le_code2->setGeometry( xpos, ypos, codeWidth, rowHeight );
  connect( le_code2, SIGNAL( textChanged ( const QString& ) ), 
                     SLOT  ( update_code2( const QString& ) ) );

  xpos += codeWidth + spacing;

  code = us_label( "-", 1 );
  code->setFrameStyle( QFrame::NoFrame );
  code->setMargin( 3 );
  code->setGeometry( xpos, ypos, commaWidth + 5, rowHeight );

  xpos += commaWidth + spacing;

  le_code3 = us_lineedit( "" );
  le_code3->setGeometry( xpos, ypos, codeWidth, rowHeight );
  connect( le_code3, SIGNAL( textChanged ( const QString& ) ), 
                     SLOT  ( update_code3( const QString& ) ) );

  xpos += codeWidth + spacing;

  code = us_label( "-", 1 );
  code->setFrameStyle( QFrame::NoFrame );
  code->setMargin( 3 );
  code->setGeometry( xpos, ypos, commaWidth + 5, rowHeight );

  xpos += commaWidth + spacing;

  le_code4 = us_lineedit( "" );
  le_code4->setGeometry( xpos, ypos, codeWidth, rowHeight );
  connect( le_code4, SIGNAL( textChanged ( const QString& ) ), 
                     SLOT  ( update_code4( const QString& ) ) );

  xpos += codeWidth + spacing;

  code = us_label( "-", 1 );
  code->setFrameStyle( QFrame::NoFrame );
  code->setMargin( 3 );
  code->setGeometry( xpos, ypos, commaWidth + 5, rowHeight );

  xpos += commaWidth + spacing;

  le_code5 = us_lineedit( "" );
  le_code5->setGeometry( xpos, ypos, codeWidth, rowHeight );
  connect( le_code5, SIGNAL( textChanged ( const QString& ) ), 
                     SLOT  ( update_code5( const QString& ) ) );

  ypos += rowHeight + spacing;
  xpos  = spacing;

  // Line 9 - Expiration Date
  QLabel* expiration = us_label( tr( "Expiration Date:" ) );
  expiration->setFrameStyle( QFrame::NoFrame );
  expiration->setGeometry( xpos, ypos, buttonw, rowHeight );

  xpos += buttonw + spacing;

  le_expiration = new QLineEdit(this);
  le_expiration->setGeometry( xpos, ypos, buttonw, rowHeight );
  connect( le_expiration, SIGNAL( textChanged      ( const QString& ) ), 
                          SLOT  ( update_expiration( const QString& ) ) );

  // Line 10 - Pushbuttons
  xpos  = spacing;
  ypos += spacing + rowHeight * 2;
  
  pb_request = us_pushbutton( tr( "Request New" ) );
  pb_request->setGeometry( xpos, ypos, pushbutton, rowHeight );
  pb_request->setPalette( palette() ); // Back to default
  connect( pb_request, SIGNAL( clicked() ), SLOT( request() ) );

  xpos += pushbutton + spacing;

  pb_import = us_pushbutton( tr( "E-mail Import" ) );
  pb_import->setGeometry( xpos, ypos, pushbutton, rowHeight );
  pb_import->setPalette( palette() ); // Back to default
  connect( pb_import, SIGNAL( clicked() ), SLOT( import() ) );

  xpos += pushbutton + spacing;

  pb_save = us_pushbutton( tr( "Save" ) );
  pb_save->setGeometry( xpos, ypos, pushbutton, rowHeight );
  pb_save->setPalette( palette() ); // Back to default
  connect( pb_save, SIGNAL( clicked() ), SLOT( save() ) );

  xpos += pushbutton + spacing;

  pb_help = us_pushbutton( tr( "Help" ) );
  pb_help->setGeometry( xpos, ypos, pushbutton, rowHeight );
  pb_help->setPalette( palette() ); // Back to default
  connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

  xpos += pushbutton + spacing;

  pb_cancel = us_pushbutton( tr( "Cancel" ) );
  pb_cancel->setGeometry( xpos, ypos, pushbutton, rowHeight );
  pb_cancel->setPalette( palette() ); // Back to default
  connect( pb_cancel, SIGNAL( clicked() ), SLOT( cancel() ) );

  // Finish up
//  QPoint p = g.global_position();
//  g.set_global_position( p + QPoint( 30, 30 ) );

  ypos += 30;    
//  setGeometry( QRect( p, p + QPoint( spacing + width + spacing, ypos ) ) );
  setMinimumSize( spacing + width + spacing, ypos );

  // Initialize for later
  osTypes << "linux"   << "win32"  << "osx"     << "irix" << "solaris"
          << "freebsd" << "netbsd" << "openbsd";
}

US_License::~US_License()
{
//  QPoint p = g.global_position();
//  g.set_global_position( p - QPoint( 30, 30 ) );
}

void US_License::cancel( void )
{
  close();
}

void US_License::update_os( int row )
{
  os = osTypes.at( row );                       
}

void US_License::help( void )
{
  online_help.show_help( "manual/license.html" );
}

void US_License::request( void )
{
  online_help.show_URL( "http://www.ultrascan.uthscsa.edu/register.html" );
}

void US_License::save( void )
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

  switch ( US_License_t::isValid( error, license ) )
  {
    case US_License_t::Expired:
      message = tr( "Failure.\n"
          "The data of the license is less than one year ago." );
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
      message = tr( "Success" );
      break;

    default:
      message = tr( "An unknown error occured." );
      break;
  }

  QMessageBox::information( this, tr( "License save results" ), message );
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

    lastname = ts.readLine();
    le_lastname->setText( lastname );
    
    firstname = ts.readLine();
    le_firstname->setText( firstname );

    institution = ts.readLine();
    le_institution->setText( institution );

    address = ts.readLine();
    le_address->setText( address );

    city = ts.readLine();
    le_city->setText( city );

    state = ts.readLine();
    for ( int i = 0; i < states.count(); i++ )
    {
      if ( version == states.value( i ) )
      {
        cbb_state->setCurrentIndex( i );
        return;
      }
    }

    zip = ts.readLine();
    le_zip->setText( zip );
    
    phone = ts.readLine();
    le_phone->setText( phone );

    email = ts.readLine();
    le_email->setText( email );

    // Platform is a radio button
    platform = ts.readLine();
    
    QStringList platforms = QStringList() 
       << "opteron" << "intel" << "sparc" << "mac" << "sgi";

    QList< QRadioButton* > radios;
    radios << rb_opteron << rb_intel << rb_sparc << rb_mac << rb_sgi;

    for ( int i = 0; i < platforms.count(); i++ )
    {
      if ( platform == platforms.at( i ) )
      {
        radios.value( i )->setDown( true );
        break;
      }
    }

    // Operating system intrnally is different form the displayed name
    os = ts.readLine();

    for ( int i = 0; i < osTypes.count(); i++ )
    {
      if ( os == osTypes.at( i ) )
      {
        lb_os->setCurrentRow( i );
        break;
      }
    }

    // Version is a dropdown
    version = ts.readLine();
    for ( int i = 0; i < versions.count(); i++ )
    {
      if ( version == versions.value( i ) )
      {
        cbb_version->setCurrentIndex( i );
        break;
      }
    }

    // License type is a dropdown
    licensetype = ts.readLine();
    for ( int i = 0; i < types.count(); i++ )
    {
      if ( licensetype == types.at( i ) )
      {
        cbb_licensetype->setCurrentIndex( i );
        break;
      }
    }

    QString Code = ts.readLine();
    QStringList codes = Code.split( "-" );
    
    code1 = codes.value( 0 );
    le_code1->setText( code1 );

    code2 = codes.value( 1 );
    le_code2->setText( code2 );
    
    code3 = codes.value( 2 );
    le_code3->setText( code3 );
    
    code4 = codes.value( 3 );
    le_code4->setText( code4 );
    
    code5 = codes.value( 4 );
    le_code5->setText( code5 );

    expiration = ts.readLine();
    le_expiration->setText( expiration );

    texfile.close();
  }
  else  // Open failed
    QMessageBox::information( this, tr( "License Error" ),
        tr( "Could not open license file\n%1" ).arg( filename ) );

}





