//! \file us_rasmol_control.cpp
#include <QApplication>

#include "us_rasmol_control.h"
#include "us_gui_settings.h"
#include "us_settings.h"
#include "us_license_t.h"
#include "us_license.h"

//! \brief Main program for US_RasmolControl. Loads translators and starts
//         the class US_RasmolControl.

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

#include "main1.inc"

   US_RasmolControl w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

US_RasmolControl::US_RasmolControl() : US_Widgets()
{
   setWindowTitle( tr( "Interface with RasMol Instances" ) );
   setPalette( US_GuiSettings::frameColor() );
   dbg_level = US_Settings::us_debug();
   winmsgs   = new US_WindowMessage( );

   // Main layout
   QGridLayout* mainLayout   = new QGridLayout( this );
   mainLayout->setSpacing         ( 2 );
   mainLayout->setContentsMargins ( 2, 2, 2, 2 );

   // Lay out the controls
   QLabel* lb_intname = us_label( tr( "Interp Name:" ) );
   cb_intname         = us_comboBox();
   cb_intname->addItem( "rasmol-pdb" );
   cb_intname->addItem( "rasmol-bead" );
   cb_intname->addItem( "(other)" );

   pb_sendcmd         = us_pushbutton( tr( "Send Command:" ) );
   le_sendcmd         = us_lineedit( "reset", -1, false );
   QLabel* lb_commcmd = us_label( tr( "Send Common Command:" ) );
   cb_commcmd         = us_comboBox();
   cb_commcmd->addItem( "reset" );
   cb_commcmd->addItem( "background black" );
   cb_commcmd->addItem( "wireframe on" );
   cb_commcmd->addItem( "wireframe off" );
   cb_commcmd->addItem( "backbone on" );
   cb_commcmd->addItem( "backbone off" );
   cb_commcmd->addItem( "spacefill on" );
   cb_commcmd->addItem( "spacefill off" );
   cb_commcmd->addItem( "ribbons on" );
   cb_commcmd->addItem( "ribbons off" );
   cb_commcmd->addItem( "strands on" );
   cb_commcmd->addItem( "strands off" );
   cb_commcmd->addItem( "cartoons on" );
   cb_commcmd->addItem( "cartoons off" );
   cb_commcmd->addItem( "set shadow on" );
   cb_commcmd->addItem( "set shadow off" );

   QLabel* lb_results = us_label( tr( "Command Results:" ) );
   te_status          = us_textedit();
   us_setReadOnly( te_status, true );

   pb_listints        = us_pushbutton( tr( "List Interps" ) );
   pb_close           = us_pushbutton( tr( "Close" ) );
   
   connect( cb_commcmd,  SIGNAL( activated     ( const QString& ) ),
            this,        SLOT(   choose_command( const QString& ) ) );
   connect( pb_sendcmd,  SIGNAL( clicked()      ),
            this,        SLOT  ( send_command() ) );
   connect( pb_listints, SIGNAL( clicked()      ),
            this,        SLOT  ( list_interps() ) );
   connect( pb_close,    SIGNAL( clicked()      ),
            this,        SLOT  ( close()        ) );

   // Do detailed layout of the controls
   int row = 0;
   mainLayout->addWidget( lb_intname,   row,    0, 1,  2 );
   mainLayout->addWidget( cb_intname,   row++,  2, 1,  2 );
   mainLayout->addWidget( lb_commcmd,   row,    0, 1,  2 );
   mainLayout->addWidget( cb_commcmd,   row++,  2, 1,  2 );
   mainLayout->addWidget( pb_sendcmd,   row,    0, 1,  1 );
   mainLayout->addWidget( le_sendcmd,   row++,  1, 1,  3 );
   mainLayout->addWidget( lb_results,   row++,  0, 1,  4 );
   mainLayout->addWidget( te_status,    row++,  0, 1,  4 );
   mainLayout->addWidget( pb_listints,  row,    0, 1,  2 );
   mainLayout->addWidget( pb_close,     row++,  2, 1,  2 );

   resize( 400, 300 );

   list_interps();               // Get the initial list of live interpreters
}

// Execute a Send command
void US_RasmolControl::send_command( void )
{
   QString     scmnd  = le_sendcmd->text();
   QString     iname  = cb_intname->currentText();
DbgLv(1) << "SCMD: command" << scmnd << "iname" << iname;

#if 1
   int errcd = winmsgs->sendMessage( iname, scmnd );

   te_status->setPlainText( errcd == 0 ?
      tr( "Successful Send to \"%1\"\n  of command \"%2\"." )
      .arg( iname ).arg( scmnd ) :
      winmsgs->lastSendResult() );
#endif
#if 0
   QString resp = winmsgs->sendQuery( iname, scmnd );
   te_status->setPlainText( resp );
#endif
}

// Execute a chosen Send command
void US_RasmolControl::choose_command( const QString& scmnd )
{
DbgLv(1) << "CHCMD: command" << scmnd;
   le_sendcmd->setText( scmnd );
   send_command();
}

// Get and list interp names
void US_RasmolControl::list_interps( void )
{
   winmsgs   ->findLiveInterps();
   cb_intname->clear();

   QList< ulong > w_ids;
   QStringList    w_inames;

   int nids   = winmsgs->interpIDs  ( w_ids );
   int nnames = winmsgs->interpNames( w_inames );

   if ( nids != nnames )
   {
      qDebug() << "*ERROR* Number of IDs does not match number of Names"
         << nids << nnames;
      return;
   }

   else if ( nids == 0 )
   {
      te_status->setPlainText( QString( "(none)" ) );
      return;
   }

   QString namtext;

   for ( int ii = 0; ii < nids; ii++ )
   {
      if ( ii != 0 )
         namtext       += "\n";

      QString iname  = w_inames[ ii ];
      QString ientry = QString::asprintf( "%lx : ", w_ids[ ii ] ) + iname;

      cb_intname->addItem( iname );        // Add name to combo box list
      namtext       += ientry;             // Add line to status text
   }

   QStringList zombies;
   int nzomb  = winmsgs->zombieList( zombies );

   if ( nzomb > 0 )
   {  // If zombies exist, list them in the status box
      namtext       += tr( "\n\nZombie Interpreters:" );

      for ( int ii = 0; ii < nzomb; ii++ )
      {
         namtext       += "\n" + zombies[ ii ];
      }
   }

   // Populate command result list
   te_status->setPlainText( namtext );     // Pure results in status box

}

