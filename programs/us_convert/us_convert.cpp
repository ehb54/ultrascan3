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

   // Create a group box to include a progress bar in the data label section
   QGroupBox* group_box    = new QGroupBox();
   QGridLayout* data_label = new QGridLayout( this );
   data_label->setSpacing         ( 0 );
   data_label->setContentsMargins ( 0, 0, 0, 0 );

   QLabel* lb_data = us_label( tr( "Data:" ) );
   data_label->addWidget( lb_data, 0, 0 );
   progress = us_progressBar( 0, 100, 0 );
   progress -> reset();
   progress -> setVisible( false );
   data_label->addWidget( progress, 1, 0 );

   group_box->setLayout(data_label);
   main->addWidget( group_box, row, 0 );

   te_data = us_textedit();
   te_data->setReadOnly( true );
   main->addWidget( te_data, row++, 1 );

   // Write pushbuttons

   QBoxLayout* writeButtons = new QHBoxLayout;
   pb_write = us_pushbutton( tr( "Write Current Data" ), false );
   connect( pb_write, SIGNAL( clicked() ), SLOT( write() ) );
   writeButtons->addWidget( pb_write );

   pb_writeAll = us_pushbutton( tr( "Write All Data" ), false );
   connect( pb_writeAll, SIGNAL( clicked() ), SLOT( writeAll() ) );
   writeButtons->addWidget( pb_writeAll );
   main->addLayout( writeButtons, row++, 0, 1, 2 );

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
   te_data->setText( "" );

   pb_write   ->setEnabled( false );
   pb_writeAll->setEnabled( false );

   // Clear any data structures
   legacyData.clear();
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
   d.makeAbsolute();
   QStringList files = d.entryList( QDir::Files );
   QStringList fileList;

   // Read into local structures
   
   for ( int i = 0; i < files.size(); i++ )
   {
      // Look for a proper filename match:
      // Optional channel + 4 to 6 digits + dot + file type + cell number

      QRegExp rx( "^[A-J]?\\d{4,6}\\.(?:RA|RI|IP|FI|WA|WI)\\d" );
      QString f = files[ i ].toUpper();
      
      if ( rx.indexIn( f ) < 0 ) continue;

      fileList << files[ i ];
   }

   runType = files[ 0 ].right( 3 ).left( 2 ).toUpper(); // 1st 2 chars of extention

   QStringList channels;

   // Parse the filtered file list to determine cells and channels
   for ( int i = 0; i < fileList.size(); i++ )
   {
      QChar c = fileList[ i ].at( 0 );
      if ( c.isLetter() && ! channels.contains( c ) ) channels << c;
   }

   if ( channels.isEmpty() ) channels << "A";

   // This is not, strictly speaking, correct. The combo boxes need to be
   // updated according to cell, then channel, then wavelength.  That is, a run
   // may have multiple cells for one cell, but only one channel for another
   // cell.

   // This should be a rarity and is ignored for now.

   // Populate the combo boxes
   cb_channel->insertItems( 0, channels );

   // Now read the data.

   for ( int i = 0; i < fileList.size(); i++ )
   {
      beckmanRaw data;
      US_DataIO::readLegacyFile( dir + "/" + fileList[ i ], data );

      // Add channel
      QChar c = fileList[ i ].at( 0 );  // Get 1st character

      data.channel = ( c.isDigit() ) ? 'A' : c.toAscii();

      legacyData << data;
   }

   // We can't trust the filename for cell number
   // Get wavelengths and cell numbers
   
   QStringList cells;
   QStringList wavelengths;

   for ( int i = 0; i < legacyData.size(); i++ )
   {
      QString wl = QString::number( legacyData[ i ].t.wavelength, 'f', 1 );
      if ( ! wavelengths.contains( wl ) ) wavelengths << wl;

      QString s = QString::number( legacyData[ i ].cell );
      if ( ! cells.contains( s ) ) cells << s;
   }

   cb_cell->insertItems( 0, cells );

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

   // Initialize textedit box and progress bar
   te_data    ->clear();
   progress   ->setRange( 0, legacyData.size()-1 );
   progress   ->setValue( 0 );
   progress   ->setVisible( true );

   for ( uint i = 0; i < legacyData.size(); i++ )
   {
      beckmanRaw d = legacyData[ i ];

/*
      qDebug() << d.description;
      qDebug() << d.type         << " "
               << d.cell         << " "
               << d.temperature  << " "
               << d.rpm          << " "
               << d.seconds      << " "
               << d.omega2t      << " "
               << d.t.wavelength << " "
               << d.count;
*/

      te_data  ->append( d.description );

      QString type(d.type);
      QString line2 =                    type                   + " "
                    + QString::number( d.cell )                 + " "
                    + QString::number( d.temperature, 'f', 1 )  + " "
                    + QString::number( d.rpm )                  + " "
                    + QString::number( d.seconds )              + " "
                    + QString::number( d.omega2t, 'E', 4 )      + " "
                    + QString::number( d.t.wavelength )         + " "
                    + QString::number( d.count );
      te_data   ->append( line2 );

      for ( uint j = 0; j < d.readings.size(); j++ )
      {
         reading r = d.readings[ j ];

         QString line = QString::number(r.d.radius, 'f', 4 )    + " "
                      + QString::number(r.value, 'E', 5 )       + " "
                      + QString::number(r.stdDev, 'E', 5 );
         te_data  ->append( line );
      }
      te_data  ->append( "" );      // a blank line between files
      progress ->setValue( i );     // update progress
      qApp     ->processEvents();
   }

   progress   ->setVisible( false );
   pb_write   ->setEnabled( true );
   pb_writeAll->setEnabled( true );
}

void US_Convert::write( void )
{ 
   // Specify the filename
   QString     dirname    = le_dir->text();
   QStringList components = dirname.split( "/", QString::SkipEmptyParts );
   QString     runID      = components.last();

   QString     cell       = cb_cell      ->currentText();
   QString     channel    = cb_channel   ->currentText();
   QString     wavelength = cb_wavelength->currentText();

   QString filename   = runID      + "." 
                      + runType    + "." 
                      + cell       + "." 
                      + channel    + "." 
                      + wavelength + ".auc";

   int err = write( filename );
   
   if ( err != US_DataIO::OK )
   {
      // Try to delete the file and tell the user   
   }
   else 
   {
      QMessageBox::information( this,
            tr( "Success" ),
            dirname + "/" + filename + tr( " written." ) );
   }        
}

void US_Convert::writeAll( void )
{  
}

int US_Convert::write( const QString& filename )
{
   // Convert the data into the UltraScan3 data structure
   QStringList parts = filename.split( "." );
   QString     runType    = parts[ 1 ];
   int         cell       = parts[ 2 ].toInt();
   char        channel    = parts[ 3 ].at( 0 ).toAscii();
   double      wavelength = parts[ 4 ].toDouble();
   rawData     newRawData;

   // Get a list of the data that matches the cell / channel / wl

   QList< beckmanRaw* > ccwLegacyData;      // legacy data with this cell/channel/wl

   for ( int i = 0; i < legacyData.size(); i++ )
   {
      if ( legacyData[ i ].cell == cell       &&
           legacyData[ i ].channel == channel &&
           fabs ( legacyData[ i ].t.wavelength - wavelength ) < 5.0 )
         ccwLegacyData << &legacyData[ i ];
   }

   // Sort the list according to time.  Use a simple bubble sort
   for ( int i = 0; i < ccwLegacyData.size(); i++ )
   {
      for ( int j = i + i; j < ccwLegacyData.size(); j++ )
      {
         if ( ccwLegacyData[ j ]->seconds < ccwLegacyData[ i ]->seconds ) ccwLegacyData.swap( i, j );
      }
   }

   if ( ccwLegacyData.isEmpty() ) return US_DataIO::NODATA; 

   strncpy( newRawData.type, runType.toAscii().constData(), 2 );
   // GUID is done by US_DataIO.
   newRawData.cell        = cell;
   newRawData.channel     = channel;
   newRawData.description = ccwLegacyData[ 0 ]->description;
   
   // Get the min and max radius
   double min_radius = 100.0;
   double max_radius = 0.0;

   for ( int i = 0; i < ccwLegacyData.size(); i++ )
   {
      double first = ccwLegacyData[ i ]->readings[ 0 ].d.radius;

      uint   size  = ccwLegacyData[ i ]->readings.size();
      double last  = ccwLegacyData[ i ]->readings[ size - 1 ].d.radius; 

      min_radius = min( min_radius, first );
      max_radius = max( max_radius, last );
   }

   // Convert the scans
   
   // Set the distance between readings to a constant for now
   double delta_r = 0.001;

   for ( int i = 0; i < ccwLegacyData.size(); i++ )
   {
      scan s;
      s.temperature = ccwLegacyData[ i ]->temperature;
      s.rpm         = ccwLegacyData[ i ]->rpm;
      s.seconds     = ccwLegacyData[ i ]->seconds;
      s.omega2t     = ccwLegacyData[ i ]->omega2t;
      s.wavelength  = ccwLegacyData[ i ]->t.wavelength;

      // Readings here and interpolated array
      int radius_count = (int) round( ( max_radius - min_radius ) / delta_r ) + 1;
      int bitmap_size = ( radius_count + 7 ) / 8;
      s.interpolated = (unsigned char*) malloc( bitmap_size );
      bzero( s.interpolated, bitmap_size );

      /*
         There are two indexes needed here.  The new radius as iterated
         from min_radius to max_radius and the pointer to the current 
         scan readings is j.  

         The old scan reading is ccwLegacyData[ i ]->values[ j ]

         If the current new radius is within 0.0003 of the ccwLegacyData[ i ]->values[ j ].d.radius
            copy ccwLegacyData[ i ]->values[ j ].value into the new reading
            copy ccwLegacyData[ i ]->values[ j ].stdDev into the new reading
            increment j

         If the current new radius is less than ccwLegacyData[ i ]->values[ 0 ].d.radius,
         then 
            copy ccwLegacyData[ i ]->values[ 0 ].value into the new reading
            set the std dev to 0.0.
            set the interpolated flag
         
         If the current new radius is greater than ccwLegacyData[ i ]->values[ last() ].d.radius
            copy ccwLegacyData[ i ]->values[ last ].value into the new reading
            set the std dev to 0.0.
            set the interpolated flag

         else
            interplate between ccwLegacyData[ i ]->values[ j ] and ccwLegacyData[ i ]->values[ j -1 ]
            set the std dev to 0.0.
            set the interpolated flag

         Append the new reading and continue.
      */

      double radius = min_radius;
      double r0     = ccwLegacyData[ i ]->readings[ 0 ].d.radius;
      int    rCount = ccwLegacyData[ i ]->readings.size();       
      double rLast  = ccwLegacyData[ i ]->readings[ rCount - 1 ].d.radius;
      
      int    k      = 0;
      
      for ( int j = 0; j < radius_count; j++ )
      {
         reading r;
         double  dr = radius - ccwLegacyData[ i ]->readings[ k ].d.radius;

         r.d.radius = radius;
         
         if ( dr > -3.0e-4   &&  k < rCount ) // A value
         {
            r.value  = ccwLegacyData[ i ]->readings[ k ].value;
            r.stdDev = ccwLegacyData[ i ]->readings[ k ].stdDev;
            k++;
         }
         else if ( radius < r0 ) // Before the first
         {
            r.value  = ccwLegacyData[ i ]->readings[ 0 ].value;
            r.stdDev = 0.0;
            setInterpolated( s.interpolated, j );
         }
         else if ( radius > rLast  ||  k >= rCount ) // After the last
         {
            r.value  = ccwLegacyData[ i ]->readings[ rCount - 1 ].value;
            r.stdDev = 0.0;
            setInterpolated( s.interpolated, j );
         }
         else  // Interpolate the value
         {
            double dv = ccwLegacyData[ i ]->readings[ k     ].value - 
                        ccwLegacyData[ i ]->readings[ k - 1 ].value;
            
            double dR = ccwLegacyData[ i ]->readings[ k     ].d.radius -
                        ccwLegacyData[ i ]->readings[ k - 1 ].d.radius;

            dr = radius - ccwLegacyData[ i ]->readings[ k - 1 ].d.radius;

            r.value  =  ccwLegacyData[ i ]->readings[ k - 1 ].value + dr * dv / dR;
            r.stdDev = 0.0;

            setInterpolated( s.interpolated, j );
         }

         s.values.push_back( r );
         radius += delta_r;
      }

      newRawData.scanData.push_back( s );
   }

   
   // Get the directory and write out the data
   QString dirname = le_dir->text();

   int result = US_DataIO::writeRawData( dirname + "/" + filename, newRawData );
   
   // Delete the bitmaps we allocated

   for ( uint i = 0; i < newRawData.scanData.size(); i++ ) 
      delete newRawData.scanData[ i ].interpolated;
  
   return result;
}

void US_Convert::setInterpolated ( unsigned char* bitmap, int location )
{
   int byte = location / 8;
   int bit  = location % 8;

   bitmap[ byte ] |= 1 << 7 - bit;
}
