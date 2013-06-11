//! \file us_rasmol_control.cpp
#include <QApplication>

#include "us_rasmol_control.h"
#include "us_gui_settings.h"
#include "us_settings.h"

#ifdef Q_WS_X11
extern "C" {
#include "us_x11_utils.h"
}
#endif

//! \brief Main program for US_RasmolControl. Loads translators and starts
//         the class US_RasmolControl.

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   US_RasmolControl w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

US_RasmolControl::US_RasmolControl() : US_Widgets()
{
   setWindowTitle( tr( "Interface with RasMol Instances" ) );
   setPalette( US_GuiSettings::frameColor() );
   dbg_level = US_Settings::us_debug();

   // Main layout
   QGridLayout* mainLayout   = new QGridLayout( this );
   mainLayout->setSpacing         ( 2 );
   mainLayout->setContentsMargins ( 2, 2, 2, 2 );

   // Lay out the controls
   pb_listints        = us_pushbutton( tr( "List Interps" ) );
   pb_close           = us_pushbutton( tr( "Close" ) );
   QLabel* lb_results = us_label( tr( "Command Results:" ) );
   
   te_status          = us_textedit();
   us_setReadOnly( te_status, true );

   QLabel* lb_intname = us_label( tr( "Interp Name:" ) );
   cb_intname         = us_comboBox();
   cb_intname->addItem( "rasmol-pdb" );
   cb_intname->addItem( "rasmol-bead" );
   cb_intname->addItem( "(other)" );

   pb_sendcmd         = us_pushbutton( tr( "Exec Send Command" ) );
   le_sendcmd         = us_lineedit( "set background yellow", -1, false );

   pb_genlcmd         = us_pushbutton( tr( "Exec General Command" ) );
   le_genlcmd         = us_lineedit( "winfo interps", -1, false );

   connect( pb_listints, SIGNAL( clicked() ), SLOT( list_interps()    ) );
   connect( pb_close,    SIGNAL( clicked() ), SLOT( close()           ) );
   connect( pb_sendcmd,  SIGNAL( clicked() ), SLOT( send_command()    ) );
   connect( pb_genlcmd,  SIGNAL( clicked() ), SLOT( general_command() ) );

   // Do detailed layout of the controls
   int row = 0;
   mainLayout->addWidget( pb_listints,  row,    0, 1,  2 );
   mainLayout->addWidget( pb_close,     row++,  2, 1,  2 );
   mainLayout->addWidget( lb_results,   row++,  0, 1,  4 );
   mainLayout->addWidget( te_status,    row++,  0, 1,  4 );
   mainLayout->addWidget( lb_intname,   row,    0, 1,  2 );
   mainLayout->addWidget( cb_intname,   row++,  2, 1,  2 );
   mainLayout->addWidget( pb_sendcmd,   row,    0, 1,  2 );
   mainLayout->addWidget( le_sendcmd,   row++,  2, 1,  2 );
   mainLayout->addWidget( pb_genlcmd,   row,    0, 1,  2 );
   mainLayout->addWidget( le_genlcmd,   row++,  2, 1,  2 );

   resize( 400, 200 );
}

// Execute a Send command
void US_RasmolControl::send_command( void )
{
   QString     scmnd  = le_sendcmd->text();
   QString     iname  = cb_intname->currentText();
DbgLv(1) << "SCMD: command" << scmnd << "iname" << iname;

   SendCommand( iname.toAscii().data(), scmnd.toAscii().data() );

   return;
}

// Execute a General command
void US_RasmolControl::general_command( void )
{
   QString     gcmnd  = le_genlcmd->text();
DbgLv(1) << "GCMD: command" << gcmnd;

   if ( gcmnd.contains( "winfo interps" ) )
   {
DbgLv(1) << "GCMD: call list_interps";
      list_interps();
   }

   return;
}


// Get and list interp names
void US_RasmolControl::list_interps( void )
{
   char nameints[ 512 ];
   strcpy( nameints, "" );

   int nchar = ShowInterpNames( nameints, 511 );
DbgLv(1) << "LSTI: nchar=" << nchar << "names:" << QString(nameints);

   if ( nchar == 0 )
   {
      strcpy( nameints, "(none)" );
      nchar    = (int)strlen( nameints ) + 1;
   }

   fill_interps( nchar, nameints );

   return;
}

// Fill command result and interp name list from Interp Names return
void US_RasmolControl::fill_interps( int nchar, char* inresult )
{
   QStringList intress;
   QStringList intnams;
   QString     intres;
   QString     intnam;
   QString     namtext;

   char* ptr1  = inresult;
   int   kchar = nchar;

   while ( kchar > 0 )
   {  // Get individual result strings and individual interp names
      int   jchar = (int)strlen( ptr1 );   // Get string length
      if ( jchar == 0 )  break;            // If NULL, we're done

      intres    = QString( ptr1 );         // Result string
      ptr1     += ( ++jchar );             // Bump to next
      kchar    -= jchar;                   // Decrement total count
      intress << intres;                   // Add result to list
                                           // Get name part of string
      intnam    = intres.section( " ", 1, -1 ).simplified();
      intnams << intnam;                   // Add to names list

      namtext  += intres;                  // Add result to status string
      namtext  += "\n";
   }

   // Populate command result list and names combo box
   te_status->setPlainText( namtext );     // Pure results in status box

   cb_intname->clear();
   cb_intname->addItems( intnams );        // Names in interps combo box
}

