#include "us_admin.h"
#include "us_global.h"
#include "us_license_t.h"
#include "us_help.h"
#include "us_settings.h"
#include "us_gui_settings.h"

int main( int argc, char* argv[] )
{
  QApplication application( argc, argv );
  //bool license = US_License_t::isValid();
  US_Admin w;
  w.show();
  return application.exec();
}

US_Admin::US_Admin( QWidget* w, Qt::WindowFlags flags ) : US_Widgets( w, flags )
{
  setWindowTitle( "Administrator Password" );

  int buttonh = 26;
  
  setPalette( US_GuiSettings::frameColor() );
  
  lbl_header = us_banner( " Change Administrator Password:" );
  lbl_header->setMinimumHeight( buttonh * 3 / 2 );
    
  lbl_psswd1 = us_label( " Enter New Password:" );
  lbl_psswd1->setMinimumHeight( buttonh );
    
  le_psswd1 = us_lineedit( "" );
  le_psswd1->setEchoMode( QLineEdit::Password );
  le_psswd1->setMinimumHeight( buttonh );
  connect (le_psswd1, SIGNAL( textChanged(   const QString& ) ), 
                      SLOT  ( update_psswd1( const QString& ) ) );
  
  lbl_psswd2 = us_label( " Verify New Password:" );
  lbl_psswd2->setMinimumHeight(buttonh);
  
  le_psswd2 =  us_lineedit( "" );
  le_psswd2->setMinimumHeight( buttonh );
  le_psswd2->setEchoMode( QLineEdit::Password );
  connect (le_psswd2, SIGNAL( textChanged(   const QString& ) ), 
                      SLOT  ( update_psswd2( const QString& ) ) );
  
  
  pb_help = us_pushbutton( "Help" );
  pb_help->setMinimumHeight( buttonh );
  connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
  
  pb_save = us_pushbutton( "Save" );
  pb_save->setMinimumHeight( buttonh ); 
  connect( pb_save, SIGNAL( clicked() ), SLOT( save() ) );
          
  pb_cancel = us_pushbutton( "Cancel" );
  pb_cancel->setMinimumHeight( buttonh );
  connect( pb_cancel, SIGNAL( clicked() ), SLOT( close() ) );

  // Layout
  QGridLayout* passwords = new QGridLayout;
  passwords->addWidget( lbl_header, 0, 0, 1, 2 );
  passwords->addWidget( lbl_psswd1, 1, 0       );
  passwords->addWidget( le_psswd1 , 1, 1       );
  passwords->addWidget( lbl_psswd2, 2, 0       );
  passwords->addWidget( le_psswd2 , 2, 1       );
  
  QHBoxLayout* buttonLine = new QHBoxLayout;
  buttonLine->addWidget( pb_help   );
  buttonLine->addWidget( pb_save   );
  buttonLine->addWidget( pb_cancel );
  
  QVBoxLayout* main = new QVBoxLayout;
  main->addLayout( passwords );
  main->addLayout( buttonLine );

  setLayout( main );
}

void US_Admin::update_psswd1( const QString& newText )
{
  password1 = newText;
}

void US_Admin::update_psswd2( const QString& newText )
{
  password2 = newText;
}

void US_Admin::save( void )
{
  if ( QString::compare( password1, password2 ) != 0 )
  {
    le_psswd1->setText("");
    le_psswd2->setText("");

    QMessageBox::information( this, 
        tr( "Attention:" ), 
        tr( "The entered passwords are not same, please type again.\n" ) );
    return;
  }

  if ( password1 == "" )
  {
    QMessageBox::information( this, 
        tr( "Attention:" ), 
        tr( "The  password cannot be empty, please type again.\n" ) );
    return;
  }
    

  /// Use settings

  QByteArray sha1string = 
    QCryptographicHash::hash( password1.toAscii(), QCryptographicHash::Sha1 );

  US_Settings::set_UltraScanPW( sha1string );

  close();
}

void US_Admin::help()
{
  US_Help* online_help = new US_Help( this );
  online_help->show_help( "manual/administrator.html" );
}

