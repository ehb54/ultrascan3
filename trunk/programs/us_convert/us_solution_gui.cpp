//! \file us_solutioninfo.cpp

#include <QtGui>
#include <uuid/uuid.h>

#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_investigator.h"
#include "us_expinfo.h"
#include "us_buffer_gui.h"
#include "us_analyte_gui.h"
#include "us_solution_gui.h"
#include "us_convertio.h"

US_SolutionGui::US_SolutionGui( QList< TripleInfo >& dataIn, int& current, int& ID ) :
   US_WidgetsDialog( 0, 0 ), triples( dataIn ), currentTriple( current ), invID( ID )
{
   setWindowTitle( tr( "Enter Solution Data" ) );
   setPalette( US_GuiSettings::frameColor() );

   // Very light gray, for read-only line edits
   QPalette gray = US_GuiSettings::editColor();
   gray.setColor( QPalette::Base, QColor( 0xe0, 0xe0, 0xe0 ) );

   QGridLayout* settings = new QGridLayout;

   int row = 0;

   QGridLayout* ccw = new QGridLayout();

   QGridLayout* db_layout = us_radiobutton( tr( "Use Database" ), rb_db );
   // connect( rb_db, SIGNAL( clicked() ),  SLOT( check_db() ) );
   settings->addLayout( db_layout, row, 0 );

   QGridLayout* disk_layout = us_radiobutton( tr( "Use Local Disk" ), rb_disk );
   //disk ? rb_disk->setChecked( true ) : rb_db->setChecked( true );
   rb_disk->setChecked( true );  // for now
   settings->addLayout( disk_layout, row++, 1 );

   // Cell / Channel / Wavelength
   QLabel* lb_triple = us_label( tr( "Cell / Channel / Wavelength:" ) );
   settings->addWidget( lb_triple, row, 0 );
   cb_triple = new US_SelectBox( this );
   setTripleList();
   connect( cb_triple, SIGNAL( activated    ( int ) ),
                       SLOT  ( change_triple( int ) ) );
   settings->addWidget( cb_triple, row++, 1 );
   cb_triple->setEnabled( true );
   cb_triple       ->load();

   // Description
   QLabel* lb_description = us_label( tr( "C/c/w Description:" ), -1 );
   settings->addWidget( lb_description, row++, 0, 1, 2 );

   le_description = us_lineedit( "", 1 );
   settings->addWidget( le_description, row++, 0, 1, 2 );

   QLabel* lb_ccwinfo = us_label( tr( "Enter Associated c/c/w Info:" ) );
   ccw->addWidget( lb_ccwinfo, row++, 1, 1, 3 );

   pb_buffer = us_pushbutton( tr( "Buffer" ), true );
   connect( pb_buffer, SIGNAL( clicked() ), SLOT( selectBuffer() ) );
   ccw->addWidget( pb_buffer, row, 1 );

   le_bufferInfo = us_lineedit( "", 1 );
   le_bufferInfo ->setPalette ( gray );
   le_bufferInfo ->setReadOnly( true );
   ccw->addWidget( le_bufferInfo, row++, 2, 1, 2 );

   pb_analyte = us_pushbutton( tr( "Analyte" ), true );
   connect( pb_analyte, SIGNAL( clicked() ), SLOT( selectAnalyte() ) );
   ccw->addWidget( pb_analyte, row, 1 );

   le_analyteInfo = us_lineedit( "", 1 );
   le_analyteInfo ->setPalette ( gray );
   le_analyteInfo ->setReadOnly( true );
   ccw->addWidget( le_analyteInfo, row++, 2, 1, 2 );

   QLabel* lb_solutionDesc = us_label( tr( "Description of Solution:" ) );
   ccw->addWidget( lb_solutionDesc, row++, 1, 1, 3 );

   le_solutionDesc = us_lineedit( "", 1 );
   ccw->addWidget( le_solutionDesc, row++, 1, 1, 3 );

   QLabel* lb_notes = us_label( tr( "Solution notes:" ) );
   ccw->addWidget( lb_notes, row++, 1, 1, 3 );

   te_notes = us_textedit();
   ccw->addWidget( te_notes, row, 1, 4, 3 );
   te_notes->setMaximumHeight( 120 );
   te_notes->setReadOnly( false );
   row += 4;

   settings->addLayout( ccw, row++, 0, 1, 2 );

   pb_applyAll = us_pushbutton( tr( "Apply to All" ), false );
   connect( pb_applyAll, SIGNAL( clicked() ), SLOT( tripleApplyAll() ) );
   settings->addWidget( pb_applyAll, row, 0 );

   pb_load = us_pushbutton( tr( "Load Solution" ), true );
   connect( pb_load, SIGNAL( clicked() ), SLOT( load() ) );
   settings->addWidget( pb_load, row++, 1 );

   pb_save = us_pushbutton( tr( "Save Solution" ), false );
   connect( pb_save, SIGNAL( clicked() ), SLOT( save() ) );
   settings->addWidget( pb_save, row, 0 );

   pb_del = us_pushbutton( tr( "Delete Solution" ), false );
   connect( pb_del, SIGNAL( clicked() ), SLOT( delete_solution() ) );
   settings->addWidget( pb_del, row++, 1 );

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

   // Now let's assemble the page
   
   QVBoxLayout* main     = new QVBoxLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   main->addLayout( settings );
   main->addLayout( buttons );
   
   // Beginning solution save status
   for ( int i = 0; i < triples.size(); i++ )
      saveStatus << NOT_SAVED;

   reset();
}

// Function to refresh the display with values from the triple structure,
//  and to enable/disable features
void US_SolutionGui::reset( void )
{
   US_SolutionGui::TripleInfo triple = triples[ currentTriple ];

   cb_triple       -> setLogicalIndex( currentTriple );
   le_description  -> setText( triple.description );

   le_bufferInfo   -> setText( triple.bufferDesc   );
   le_analyteInfo  -> setText( triple.analyteDesc  );
   le_solutionDesc -> setText( triple.solutionDesc );
   te_notes        -> setText( triple.notes        );

   pb_buffer       -> setEnabled( true );
   pb_analyte      -> setEnabled( true );
   pb_load         -> setEnabled( true );

   // Let's calculate if we're eligible to copy this triple info to all
   // or to save it
   pb_applyAll     -> setEnabled( false );
   pb_save         -> setEnabled( false );
   if ( triple.analyteID   > 0 &&
        triple.bufferID    > 0 )
   {
      pb_applyAll  -> setEnabled( true );
      pb_save      -> setEnabled( true );
   }

   // We can always delete something, even if it's just what's in the dialog
   pb_del          -> setEnabled( false );
   if ( triple.analyteID   > 0 ||
        triple.bufferID    > 0 )
   {
      pb_del       -> setEnabled( true );
   }
}

// Function to accept the current set of solutions and return
void US_SolutionGui::accept( void )
{
   saveFormData();

   emit updateSolutionGuiSelection( triples, currentTriple );

   close();
}

// Function to cancel the current dialog and return
void US_SolutionGui::cancel( void )
{
   emit cancelSolutionGuiSelection();

   close();
}

// Function to create the list of triples for the combo box
void US_SolutionGui::setTripleList( void )
{
   QList<listInfo> options;
   foreach ( TripleInfo triple, triples )
   {
      if ( triple.excluded ) continue;

      struct listInfo option;
      option.ID      = QString::number( triple.tripleID );
      option.text    = triple.tripleDesc;
      options << option;
   }

   cb_triple->clear();
   if ( options.size() > 0 )
   {
      cb_triple->addOptions( options );
      cb_triple->setLogicalIndex( currentTriple );
   }
}

// Function to change the current triple
void US_SolutionGui::change_triple( int ndx )
{
   saveFormData();

   cb_triple->setCurrentIndex( ndx );

   // First time through here the combo box might not be displayed yet
   currentTriple = ( cb_triple->getLogicalID() == -1 )
                   ? currentTriple
                   : cb_triple->getLogicalID();

   reset();
}

// Create a dialog to request a buffer selection
void US_SolutionGui::selectBuffer( void )
{
   US_BufferGui* buffer_dialog = new US_BufferGui( invID, true );         // Ask for a signal

   connect( buffer_dialog, SIGNAL( valueBufferID ( const QString& ) ),
            this,          SLOT  ( assignBuffer  ( const QString& ) ) );

   buffer_dialog->exec();
}

// Get information about selected buffer
void US_SolutionGui::assignBuffer( const QString& bufferID )
{
   triples[ currentTriple ].bufferID = bufferID.toInt();

   // Now get the corresponding description 
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      db_error( db.lastError() );
      return;
   }

   QStringList q( "get_buffer_info" );
   q << bufferID;
   db.query( q );

   if ( db.next() )
   {
      triples[ currentTriple ].bufferGUID = db.value( 0 ).toString();
      triples[ currentTriple ].bufferDesc = db.value( 1 ).toString();
   }

   reset();
}

// Create dialog to request an analyte selection
void US_SolutionGui::selectAnalyte( void )
{
   US_AnalyteGui* analyte_dialog = new US_AnalyteGui( invID, true ); 

   connect( analyte_dialog, SIGNAL( valueChanged  ( US_Analyte ) ),
            this,           SLOT  ( assignAnalyte ( US_Analyte ) ) );

   analyte_dialog->exec();
}

// Get information about selected analyte
void US_SolutionGui::assignAnalyte( US_Analyte data )
{
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      db_error( db.lastError() );
      return;
   }

   // Get analyteID
   QStringList q( "get_analyteID" );
   q << data.analyteGUID;

   db.query( q );
   if ( db.lastErrno() != US_DB2::OK ) return;

   db.next();
   QString analyteID = db.value( 0 ).toString();

   triples[ currentTriple ].analyteID = analyteID.toInt();

   // Now get the corresponding description 
   q.clear();
   q << "get_analyte_info";
   q << analyteID;
   db.query( q );

   if ( db.next() )
   {
      triples[ currentTriple ].analyteGUID = db.value( 0 ).toString();
      triples[ currentTriple ].analyteDesc = db.value( 4 ).toString();
   }

   reset();
}

// Function to copy the current triple's data to all triples
void US_SolutionGui::tripleApplyAll( void )
{
   saveFormData();

   US_SolutionGui::TripleInfo triple = triples[ currentTriple ];

   // Copy selected fields only
   for ( int i = 0; i < triples.size(); i++ )
   {
      if ( triples[ i ].excluded ) continue;

      triples[ i ].centerpiece  = triple.centerpiece;
      triples[ i ].bufferID     = triple.bufferID;
      triples[ i ].bufferGUID   = triple.bufferGUID;
      triples[ i ].bufferDesc   = triple.bufferDesc;
      triples[ i ].analyteID    = triple.analyteID;
      triples[ i ].analyteGUID  = triple.analyteGUID;
      triples[ i ].analyteDesc  = triple.analyteDesc;
      memcpy( triples[ i ].solutionGUID, triple.solutionGUID, 16 );
      triples[ i ].solutionDesc = triple.solutionDesc;
      triples[ i ].storageTemp  = triple.storageTemp;
      triples[ i ].notes        = triple.notes;
   }

   QMessageBox::information( this, tr( "C/c/w Apply to All" ),
         tr( "The current c/c/w information has been copied to all\n" ) );
}

// Function to save the current form data into the current triple
void US_SolutionGui::saveFormData( void )
{
   // Save edit fields before changing triple
   triples[ currentTriple ].solutionDesc = le_solutionDesc ->text();
   triples[ currentTriple ].notes        = te_notes        ->toPlainText();

}

// Function to load a solution from disk or db
void US_SolutionGui::load( void )
{
}

// Function to save solution information to disk or db
void US_SolutionGui::save( void )
{
}

// Function to delete a solution from disk, db, or in the current form
void US_SolutionGui::delete_solution( void )
{
}

// Function to display an error returned from the database
void US_SolutionGui::db_error( const QString& error )
{
   QMessageBox::warning( this, tr( "Database Problem" ),
         tr( "Database returned the following error: \n" ) + error );
}

// Initializations
US_SolutionGui::TripleInfo::TripleInfo()
{
   clear();
}

void US_SolutionGui::TripleInfo::clear( void )
{
   tripleID     = 0;
   tripleDesc   = QString( "" );
   description  = QString( "" );
   excluded     = false;
   centerpiece  = 0;
   bufferID     = 0;
   bufferGUID   = QString( "" );
   bufferDesc   = QString( "" );
   analyteID    = 0;
   analyteGUID  = QString( "" );
   analyteDesc  = QString( "" );
   memset( tripleGUID, 0, 16 );
   tripleFilename = QString( "" );
   memset( solutionGUID, 0, 16 );
   solutionDesc  = QString( "" );
   storageTemp  = 0;
   notes        = QString( "" );
}

void US_SolutionGui::TripleInfo::show( void )
{
   char uuidc[ 37 ];
   uuid_unparse( (unsigned char*)tripleGUID, uuidc );
   
   char uuidc2[ 37 ];
   uuid_unparse( (unsigned char*)solutionGUID, uuidc2 );
   qDebug() << "tripleID     = " << tripleID     << '\n'
            << "tripleDesc   = " << tripleDesc   << '\n'
            << "description  = " << description  << '\n'
            << "centerpiece  = " << centerpiece  << '\n'
            << "bufferID     = " << bufferID     << '\n'
            << "bufferGUID   = " << bufferGUID   << '\n'
            << "bufferDesc   = " << bufferDesc   << '\n'
            << "analyteID    = " << analyteID    << '\n'
            << "analyteGUID  = " << analyteGUID  << '\n'
            << "analyteDesc  = " << analyteDesc  << '\n'
            << "tripleGUID   = " << QString( uuidc )  << '\n'
            << "tripleFilename = " << tripleFilename  << '\n'
            << "solutionGUID = " << QString( uuidc2 ) << '\n'
            << "solutionDesc = " << solutionDesc << '\n'
            << "storageTemp  = " << storageTemp  << '\n'
            << "notes        = " << notes        << '\n';
   if ( excluded ) qDebug() << "excluded";
}
