//! \file us_ccwinfo.cpp

#include "us_convert.h"
#include "us_tripleinfo.h"
#include "us_buffer.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_db.h"
#include "us_investigator.h"

US_TripleInfo::US_TripleInfo( void ) : US_WidgetsDialog( 0, 0 )
{
   reset();

   setWindowTitle( tr( "Cell/Channel/Wavelength Information" ) );
   setPalette( US_GuiSettings::frameColor() );

   // Everything will be in the main layout
   QGridLayout* main = new QGridLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   int row = 0;

   // Set up centerpiece drop-down
   QLabel* lb_centerpiece = us_label( tr( "Centerpiece:" ) );
   main->addWidget( lb_centerpiece, row, 0 );

   centerpieceInfo();
   cb_centerpiece = us_comboBox();
   cb_centerpiece -> addItems( centerpieceTypes );

   main->addWidget( cb_centerpiece, row++, 1 );

   pb_buffer = us_pushbutton( tr( "Buffer" ), false );
   connect( pb_buffer, SIGNAL( clicked() ), SLOT( selectBuffer() ) );
   pb_buffer->setEnabled( true );
   main->addWidget( pb_buffer, row, 0 );

   pb_analyte = us_pushbutton( tr( "Analyte" ), false );
   connect( pb_analyte, SIGNAL( clicked() ), SLOT( selectAnalyte() ) );
   pb_analyte->setEnabled( true );
   main->addWidget( pb_analyte, row++, 1 );

   // Some pushbuttons
   QHBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   buttons->addWidget( pb_help );

   QPushButton* pb_accept = us_pushbutton( tr( "Accept" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( accept() ) );
   buttons->addWidget( pb_accept );

   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   connect( pb_cancel, SIGNAL( clicked() ), SLOT( cancel() ) );
   buttons->addWidget( pb_cancel );

   main->addLayout( buttons, row++, 0, 1, 2 );
}

bool US_TripleInfo::centerpieceInfo( void )
{
   QString home = qApp->applicationDirPath().remove( QRegExp( "/bin$" ) );
   QFile f( home + "/etc/centerpiece.dat" );
   if ( ! f.open( QIODevice::ReadOnly | QIODevice::Text ) ) return false;
   QTextStream ts( &f );

   centerpieceTypes.clear();
   while ( ! ts.atEnd() )
   {
      // Read a line at a time
      QString line      = ts.readLine();

      // Make sure we skip the comments
      if ( line[ 0 ] != '#' ) 
      {
         QStringList parts = line.split(" ", QString::SkipEmptyParts );
         QString material = parts[ 1 ].toAscii();
         QString channels = QString::number( parts[ 2 ].toInt() * 2 );

         int pl_index = parts[ 2 ].toInt() + 4;           // bottom position for each channel
         QString pathlength = parts[ pl_index ].toAscii();

         QString c = material   + ", "
                   + channels   + " channels, "
                   + pathlength + "cm";

         centerpieceTypes << c;

      }
   }

   f.close();

   return true;
}

void US_TripleInfo::reset( void )
{
   save_bufferID    = 0;
   save_analyteID   = 0;
}

void US_TripleInfo::accept( void )
{
   US_Convert::TripleInfo d;

   d.centerpiece  = cb_centerpiece   ->currentIndex();

   d.bufferID     = save_bufferID;
   d.analyteID    = save_analyteID;

   emit updateTripleInfoSelection( d );
   close();
}

void US_TripleInfo::cancel( void )
{
   reset();

   emit cancelTripleInfoSelection();
   close();
}

void US_TripleInfo::selectBuffer( void )
{
   US_Buffer_DB* buffer_dialog = new US_Buffer_DB;

   // This will change depending on what signal we actually get back
   connect( buffer_dialog, SIGNAL( IDChanged    ( int ) ),
            this,          SLOT(   assignBuffer( int ) ) );

   assignBuffer( 3 );  // for now, since we aren't getting anything back

   buffer_dialog->exec();
}

void US_TripleInfo::assignBuffer( int bufferID )
{
   save_bufferID = bufferID;

}

void US_TripleInfo::selectAnalyte( void )
{
qDebug() << "In selectAnalyte";

   assignAnalyte( 2 ); // for now, since this isn't implemented yet
}

void US_TripleInfo::assignAnalyte( int analyteID )
{
qDebug() << "In assignAnalyte";

   save_analyteID = analyteID;

}


