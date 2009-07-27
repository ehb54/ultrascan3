//! \file us_convert.cpp

#include <QApplication>

#include "us_convert.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"

//! \brief Main program for us_convert. Loads translators and starts
//         the class US_Convert.

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_Convert w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

US_Convert::US_Convert() : US_Widgets()
{
   setWindowTitle( tr( "Convert Legacy Raw Data" ) );
   setPalette( US_GuiSettings::frameColor() );

   QGridLayout* main = new QGridLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   int row = 0;

   // Row 1
   QPushButton* pb_load = us_pushbutton( tr( "Load Legacy Data" ) );
   connect( pb_load, SIGNAL( clicked() ), SLOT( load() ) );
   main->addWidget( pb_load, row++, 0, 1, 2 );

   QLabel* lb_dir = us_label( tr( "Directory:" ) );
   main->addWidget( lb_dir, row, 0 );

   le_dir = us_lineedit( "", 1 );
   le_dir->setReadOnly( true );
   main->addWidget( le_dir, row++, 1 );


   // Cell / Channel / Wavelength

   QLabel* lb_cell = us_label( tr( "Cell:" ) );
   main->addWidget( lb_cell, row, 0 );

   cb_cell = us_comboBox();
   cb_cell->setInsertPolicy( QComboBox::InsertAlphabetically );
   main->addWidget( cb_cell, row++, 1 );

   QLabel* lb_channel = us_label( tr( "Channel:" ) );
   main->addWidget( lb_channel, row, 0 );

   cb_channel = us_comboBox();
   cb_channel->setInsertPolicy( QComboBox::InsertAlphabetically );
   main->addWidget( cb_channel, row++, 1 );

   QLabel* lb_wavelength = us_label( tr( "Wavelength:" ) );
   main->addWidget( lb_wavelength, row, 0 );

   cb_wavelength = us_comboBox();
   cb_wavelength->setInsertPolicy( QComboBox::InsertAlphabetically );
   main->addWidget( cb_wavelength, row++, 1 );

   // Write pushbuttons

   QBoxLayout* write = new QHBoxLayout;
   pb_write = us_pushbutton( tr( "Write Current Data" ), false );
   //connect( pb_load, SIGNAL( clicked() ), SLOT( write() ) );
   write->addWidget( pb_write );

   pb_writeAll = us_pushbutton( tr( "Write All Data" ), false );
   //connect( pb_load, SIGNAL( clicked() ), SLOT( writeAll() ) );
   write->addWidget( pb_writeAll );
   main->addLayout( write, row++, 0, 1, 2 );

   // Standard pushbuttons

   QBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_reset = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset, SIGNAL( clicked() ), SLOT( reset() ) );
   buttons->addWidget( pb_reset );

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   buttons->addWidget( pb_help );

   QPushButton* pb_accept = us_pushbutton( tr( "Close" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( close() ) );
   buttons->addWidget( pb_accept );

   main->addLayout( buttons, row++, 0, 1, 2 );
}

void US_Convert::reset( void )
{
   cb_cell      ->clear();
   cb_channel   ->clear();
   cb_wavelength->clear();

   le_dir->setText( "" );

   pb_write   ->setEnabled( false );
   pb_writeAll->setEnabled( false );

   // Clear any data structures
   raw_scans.clear();
}

void US_Convert::load( void )
{  
   // Ask for data directory
   QString dir = QFileDialog::getExistingDirectory( this, 
         tr( "Raw Data Directory" ),
         US_Settings::dataDir(),
         QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );

   if ( dir.isEmpty() ) return; 

   reset();
   le_dir->setText( dir );

   // Get legacy file names
   QDir        d( dir, "*", QDir::Name, QDir::Files | QDir::Readable );
   QStringList files = d.entryList();
   QStringList good;

   // Read into local structures
   
   for ( int i = 0; i < files.size(); i++ )
   {
      // Look for a proper filename match:
      // Optional channel + 4 to 6 digits + dot + file type + cell number

      QRegExp rx( "^[A-J]?\\d{4,6}\\.(?:RA|RI|IP|FI|WA|WI)\\d" );
      QString f = files[ i ].toUpper();
      
      if ( rx.indexIn( f ) < 0 ) continue;

      good << files[ i ];
   }

   QStringList channels;
   QStringList cells;

   // Parse the good filenames to determine cells and channels
   for ( int i = 0; i < good.size(); i++ )
   {
      QChar c = good[ i ].at( 0 );
      if ( c.isLetter() && ! channels.contains( c ) ) channels << c;
   
      QString cell = good[ i ].right( 1 );  // Get the last character
      if ( ! cells.contains( cell ) ) cells << cell;
   }

   if ( channels.isEmpty() ) channels << "A";


// This is not, strictly speaking, correct. The combo boxes need to be updated
// according to cell, then channel, then wavelength.  That is, a run may
// have multiple cells for one cell, but only one channel for another cell.

// This should be a rarity and is ignored for now.

   // Populate the combo boxes
   cb_cell   ->insertItems( 0, cells );
   cb_channel->insertItems( 0, channels );

   // Now read the data.

   for ( int i = 0; i < good.size(); i++ )
   {
      beckmanRaw data;
      US_DataIO::readLegacyFile( dir + good[ i ], data );
      raw_scans << data;
   }

   // Get wavelengths
   
   QStringList wavelengths;

   for ( int i = 0; i < raw_scans.size(); i++ )
   {
      QString wl = QString::number( raw_scans[ i ].t.wavelength, 'f', 1 );
      if ( ! wavelengths.contains( wl ) ) wavelengths << wl;
   }

   //cb_wavelength->insertItems( 0, wavelengths );

   // Merge wavelengths

   wavelengths.sort();

   QList< QList< double > > modes;
   QList< double >          mode;
   
   for ( int i = 0; i < wavelengths.size(); i++ )
   {
      double wl = wavelengths[ i ].toDouble();
      
      if ( ! mode.empty()  &&  mode.last() - wl > 5.0 )
      {
         modes << mode;
         mode.clear();
      }

      mode << wl;   
   }

   if ( mode.size() > 0 ) modes << mode;

   // Now we have a list of modes.  
   // Average each list and round to the closest integer.
   
   QList< int > wl_average;

   for ( int i = 0; i < modes.size(); i++ )
   {
      double sum = 0.0;

      for ( int j = 0; j < modes[ i ].size(); j++ ) sum += modes[ i ][ j ]; 

      wl_average << (int) round( sum / modes[ i ].size() );

      cb_wavelength->addItem( QString::number( wl_average.last() ) );
   }

   pb_write   ->setEnabled( true );
   pb_writeAll->setEnabled( true );
}

void US_Convert::write( void )
{  
}

void US_Convert::writeAll( void )
{  
}
