//! \file us_ccwinfo.cpp

#include "us_ccwinfo.h"
#include "us_buffer.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_db.h"
#include "us_investigator.h"

US_CCWInfo::CCWInfo::CCWInfo()
{
   CCWInfo::clear();
}

void US_CCWInfo::CCWInfo::clear( void )
{
   centerpiece  = 0;
   bufferID     = 0;
   analyteID    = 0;
}

US_CCWInfo::US_CCWInfo( int invID ) : US_WidgetsDialog( 0, 0 )
{
   save_investigator = invID;
   reset();

   setWindowTitle( tr( "Cell/Channel/Wavelength Information" ) );
   setPalette( US_GuiSettings::frameColor() );

   // Everything will be in the main layout
   QGridLayout* main = new QGridLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   int row = 0;

   // Database choices
   QStringList DB = US_Settings::defaultDB();
   QLabel* lb_DB = us_banner( tr( "Database: " ) + DB.at( 0 ) );
   main->addWidget( lb_DB, row++, 0, 1, 2 );

   QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
   connect( pb_investigator, SIGNAL( clicked() ), SLOT( sel_investigator() ) );
   pb_investigator->setEnabled( true );
   main->addWidget( pb_investigator, row, 0 );

   le_investigator = us_lineedit( "", 1 );
   le_investigator->setReadOnly( true );
   main->addWidget( le_investigator, row++, 1);
   if ( save_investigator != 0 )
      le_investigator->setText( QString::number( save_investigator)  );

   // Set up centerpiece drop-down
   QLabel* lb_centerpiece = us_label( tr( "Centerpiece:" ) );
   main->addWidget( lb_centerpiece, row, 0 );

   centerpieceInfo();
   cb_centerpiece = us_comboBox();
   cb_centerpiece -> addItems( centerpieceTypes );

   main->addWidget( cb_centerpiece, row++, 1 );

   pb_buffer = us_pushbutton( tr( "Buffer" ), false );
   connect( pb_buffer, SIGNAL( clicked() ), SLOT( sel_buffer() ) );
   pb_buffer->setEnabled( true );
   main->addWidget( pb_buffer, row, 0 );

   pb_analyte = us_pushbutton( tr( "Analyte" ), false );
   connect( pb_analyte, SIGNAL( clicked() ), SLOT( sel_analyte() ) );
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

bool US_CCWInfo::centerpieceInfo( void )
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

void US_CCWInfo::reset( void )
{
   save_bufferID    = 0;
   save_analyteID   = 0;
}

void US_CCWInfo::accept( void )
{
   US_CCWInfo::CCWInfo d;

   // First get the invID
   QString invInfo = le_investigator->text();
   if ( invInfo.isEmpty() )
   {
      QMessageBox::information( this,
                tr( "Error" ),
                tr( "You must choose an investigator before accepting" ) );
      return;
   }

   if ( save_investigator != 0 )
      d.investigator = save_investigator;

   else
   {
      QStringList components = invInfo.split( ")", QString::SkipEmptyParts );
      components = components[0].split( "(", QString::SkipEmptyParts );
      d.investigator = components.last().toInt();
   }

   d.centerpiece  = cb_centerpiece   ->currentIndex();

   d.bufferID     = save_bufferID;
   d.analyteID    = save_analyteID;

   emit update_ccwinfo_selection( d );
   close();
}

void US_CCWInfo::cancel( void )
{
   reset();

   emit cancel_ccwinfo_selection();
   close();
}

void US_CCWInfo::sel_investigator( void )
{
   US_Investigator* inv_dialog = new US_Investigator( true );
   connect( inv_dialog, 
      SIGNAL( investigator_accepted( int, const QString&, const QString& ) ),
      SLOT  ( assign_investigator  ( int, const QString&, const QString& ) ) );
   inv_dialog->exec();
}

void US_CCWInfo::assign_investigator( int invID,
      const QString& lname, const QString& fname)
{
   le_investigator->setText( "InvID (" + QString::number( invID ) + "): " +
         lname + ", " + fname );

   save_investigator = invID;
}

void US_CCWInfo::sel_buffer( void )
{
   US_Buffer_DB* buffer_dialog = new US_Buffer_DB;

   // This will change depending on what signal we actually get back
   connect( buffer_dialog, SIGNAL( IDChanged    ( int ) ),
            this,          SLOT(   assign_buffer( int ) ) );

   assign_buffer( 3 );  // for now, since we aren't getting anything back

   buffer_dialog->exec();
}

void US_CCWInfo::assign_buffer( int bufferID )
{
   save_bufferID = bufferID;

}

void US_CCWInfo::sel_analyte( void )
{
qDebug() << "In sel_analyte";

   assign_analyte( 2 ); // for now, since this isn't implemented yet
}

void US_CCWInfo::assign_analyte( int analyteID )
{
qDebug() << "In assign_analyte";

   save_analyteID = analyteID;

}


