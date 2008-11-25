//! \file us_admin.cpp
#include "us_admin.h"
#include "us_global.h"
#include "us_license_t.h"
#include "us_help.h"
#include "us_settings.h"
#include "us_gui_settings.h"

US_Admin::US_Admin( QWidget* w, Qt::WindowFlags flags ) : US_Widgets( w, flags )
{
  if ( ! g.isValid() ) 
  {
     // Do something for invalid global memory
     //qDebug( "us_win: invalid global memory" );
  }

  QPoint p = g.global_position();
  move( p );
  g.set_global_position( p + QPoint( 30, 30 ) );

  setWindowTitle( "Change Master Password" );

  int buttonh = 26;
  
  setPalette( US_GuiSettings::frameColor() );
  
  QLabel* header = us_banner( "Change Administrator Password:" );
  header->setMinimumHeight( buttonh * 3 / 2 );
    
  QString oldPass = US_Settings::UltraScanPW();
  QLabel* oldPW;

  if ( ! oldPass.isEmpty() ) 
  {
    oldPW = us_label( "Old Password:" );
    oldPW->setMinimumHeight( buttonh );

    le_oldPasswd = us_lineedit( "" );
    le_oldPasswd->setEchoMode( QLineEdit::Password );
    le_oldPasswd->setMinimumHeight( buttonh );
  }

  QLabel* passwd1 = us_label( "Enter New Password:" );
  passwd1->setMinimumHeight( buttonh );
    
  le_passwd1 = us_lineedit( "" );
  le_passwd1->setEchoMode( QLineEdit::Password );
  le_passwd1->setMinimumHeight( buttonh );
  le_passwd1->setMinimumWidth( 300 );
  
  QLabel* passwd2 = us_label( "Verify New Password:" );
  passwd2->setMinimumHeight(buttonh);
  
  le_passwd2 = us_lineedit( "" );
  le_passwd2->setMinimumHeight( buttonh );
  le_passwd2->setEchoMode( QLineEdit::Password );
  
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
  passwords->addWidget( header, 0, 0, 1, 2 );

  int row = 1;
  if ( ! oldPass.isEmpty() )
  {
    passwords->addWidget( oldPW       , row,   0 );
    passwords->addWidget( le_oldPasswd, row++, 1 );
  }

  passwords->addWidget( passwd1   , row,   0 );
  passwords->addWidget( le_passwd1, row++, 1 );
  passwords->addWidget( passwd2   , row,   0 );
  passwords->addWidget( le_passwd2, row++, 1 );
  
  QHBoxLayout* buttonLine = new QHBoxLayout;
  buttonLine->addWidget( pb_help   );
  buttonLine->addWidget( pb_save   );
  buttonLine->addWidget( pb_cancel );
  
  QVBoxLayout* main = new QVBoxLayout;
  main->addLayout( passwords );
  main->addLayout( buttonLine );

  setLayout( main );
}

US_Admin::~US_Admin()
{
  g.set_global_position( g.global_position() - QPoint( 30, 30 ) );
}

void US_Admin::save( void )
{
  QByteArray oldPW = US_Settings::UltraScanPW();

  if ( ! oldPW.isEmpty() ) 
  {
    QByteArray calcsha1 = 
      QCryptographicHash::hash( le_oldPasswd->text().toAscii(), 
          QCryptographicHash::Sha1 );

    if ( calcsha1 != oldPW )
    {
      qDebug() << "Saved hash:" << oldPW << "; New hash:" << calcsha1;
      QMessageBox::information( this, 
        tr( "Attention:" ), 
        tr( "The old password is incorrect. Please re-input.\n" ) );
      return;
    }
  }

  if ( le_passwd1->text() != le_passwd2->text() )
  {
    le_passwd1->setText("");
    le_passwd2->setText("");

    QMessageBox::information( this, 
        tr( "Attention:" ), 
        tr( "The entered passwords are not same. Please re-input.\n" ) );
    return;
  }

  QString newPW = le_passwd1->text();

  if ( newPW.isEmpty() )
  {
    QMessageBox::information( this, 
        tr( "Attention:" ), 
        tr( "The new password cannot be empty. Please re-input.\n" ) );
    return;
  }

  /// Use settings

  QByteArray sha1string = 
    QCryptographicHash::hash( newPW.toAscii(), QCryptographicHash::Sha1 );

  US_Settings::set_UltraScanPW( sha1string );
  g.setPasswd( newPW );
  close();
}

void US_Admin::help()
{
  US_Help* online_help = new US_Help( this );
  online_help->show_help( "manual/administrator.html" );
}


