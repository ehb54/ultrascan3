//! \file us_rotor_calibration.cpp

#include "us_rotor.h"
#include "us_rotor_calibration.h"
#include "us_rotor_gui.h"
#include "us_abstractrotor_gui.h"
#include "us_settings.h"
#include "us_license_t.h"
#include "us_math2.h"
#include "us_util.h"
#include "us_db2.h"
#include "us_license.h"
#include "us_gui_settings.h"
#include "us_run_details2.h"
#include "us_passwd.h"
#include "us_get_dbexp.h"

//! \brief Main program for US_RotorCalibration. Loads translators and starts
//         the class US_FitMeniscus.

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_RotorCalibration w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

US_RotorCalibration::US_RotorCalibration() : US_Widgets()
{
   unsigned int row = 0;

   rotor    = "Default Rotor";
   newlimit = false;

   setWindowTitle( tr( "Edit Rotor Calibration" ) );
   setPalette( US_GuiSettings::frameColor() );
   check  = QIcon( US_Settings::usHomeDir() + "/etc/check.png" );

   QGridLayout* top = new QGridLayout( this );
   top->setSpacing         ( 2 );
   top->setContentsMargins ( 2, 2, 2, 2 );
   
   QLabel* lb_instructions = us_label( tr("Instructions:"), -1 );
   top->addWidget( lb_instructions, row, 0, 1, 1 );

   le_instructions = us_lineedit( "", -1 );
   le_instructions->setReadOnly( true );
   le_instructions->setText( tr("Please load a calibration data set..."));
   top->addWidget( le_instructions, row++, 1 );

   // Radio buttons
   disk_controls = new US_Disk_DB_Controls( US_Disk_DB_Controls::Default );
   connect( disk_controls, SIGNAL( changed       ( bool ) ),
                           SLOT  ( source_changed( bool ) ) );
   top->addLayout( disk_controls, row++, 0 );

   pb_load = us_pushbutton( tr( "Load Calibration Data" ) );
   connect( pb_load, SIGNAL( clicked() ), SLOT( load() ) );
   top->addWidget( pb_load, row++, 0 );
   
   QLabel* lb_cell = us_label( tr("Current Cell:"), -1 );
   int     height  = lb_instructions->size().height();
   lb_cell->setMaximumHeight( height );
   top->setColumnStretch( row, 0 );

   top->addWidget( lb_cell, row++, 0 );
   
   ct_cell = new QwtCounter(this); // Update range upon load
   //ct_cell = us_counter ( 2, 0, 0 );
   ct_cell->setStep( 1 );
   top->addWidget( ct_cell, row++, 0 );
   
   QLabel* lb_channel = us_label( tr("Current Channel:"), -1 );
   lb_channel->setMaximumHeight( height );
   top->addWidget( lb_channel, row++, 0 );
   
   ct_channel = new QwtCounter (this); // Update range upon load
   //ct_channel = us_counter ( 2, 0, 0 ); // Update range upon load
   ct_channel->setStep( 1 );
   top->addWidget( ct_channel, row++, 0);

   QGridLayout* lo_top = us_radiobutton( tr( "Top of cell" ), rb_top );
   connect( rb_top, SIGNAL( clicked() ), SLOT( update_position() ) );
   top->addLayout( lo_top, row++, 0 );

   QGridLayout* lo_bottom = us_radiobutton( tr( "Bottom of cell" ), rb_bottom );
   connect( rb_bottom, SIGNAL( clicked() ), SLOT( update_position() ) );
   top->addLayout( lo_bottom, row++, 0 );

   QGridLayout* lo_assigned = us_checkbox( tr( "Limits are assigned" ), cb_assigned, false );
   connect (cb_assigned, SIGNAL (clicked()), this, SLOT (update_used()));
   top->addLayout( lo_assigned, row++, 0 );
   
   pb_accept = us_pushbutton( tr( "Next" ) );
   pb_accept->setEnabled( false );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( next() ) );
   top->addWidget( pb_accept, row++, 0 );
   
   pb_calculate = us_pushbutton( tr( "Calculate" ) );
   pb_calculate->setEnabled( false );
   connect( pb_calculate, SIGNAL( clicked() ), SLOT( calculate() ) );
   top->addWidget( pb_calculate, row++, 0 );
   
   //QLabel* lb_spacer = us_banner( tr( "" ) );
   //lb_spacer->setMaximumHeight( height );
   //top->addWidget( lb_spacer, row++, 0 );
   top->addItem( new QSpacerItem( 0, 0 ), row++, 0 );
   
   pb_save = us_pushbutton( tr( "Save Rotor Calibration" ) );
   connect( pb_save, SIGNAL( clicked() ), SLOT( save() ) );
   pb_save->setEnabled( false );
   top->addWidget( pb_save, row++, 0 );
   
   pb_view = us_pushbutton( tr( "View Calibration Report" ) );
   connect( pb_view, SIGNAL( clicked() ), SLOT( view() ) );
   pb_view->setEnabled( false );
   top->addWidget( pb_view, row++, 0 );
   
   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   top->addWidget( pb_help, row++, 0 );

   pb_reset = us_pushbutton( tr( "Reset" ) );
   pb_reset->setEnabled ( false );
   connect( pb_reset, SIGNAL( clicked() ), SLOT( reset() ) );
   top->addWidget( pb_reset, row++, 0 );
   
   QPushButton* pb_close = us_pushbutton( tr( "Close" ) );
   connect( pb_close, SIGNAL( clicked() ), SLOT( close() ) );
   top->addWidget( pb_close, row++, 0 );

   // Plot layout on right side of window
   plot = new US_Plot( data_plot,
         tr( "Intensity Data\n(Channel A in red, Channel B in green)" ),
         tr( "Radius (in cm)" ), tr( "Intensity" ) );

   data_plot->setMinimumSize( 700, 400 );
   data_plot->enableAxis( QwtPlot::xBottom, true );
   data_plot->enableAxis( QwtPlot::yLeft  , true );
   data_plot->setAxisScale( QwtPlot::xBottom, 5.7, 7.3 );
   data_plot->setAxisAutoScale( QwtPlot::yLeft );
   
   connect ( plot, SIGNAL ( zoomedCorners( QwtDoubleRect) ),
             this, SLOT   ( currentRect  ( QwtDoubleRect) ) );
   
   top->addLayout( plot, 1, 1, row - 1, 1 );

   top->setColumnStretch( 0, 0 );
   top->setColumnStretch( 1, 1 );

   pick = new US_PlotPicker( data_plot );

   // Set rubber band to display for Control+Left Mouse Button
   pick->setRubberBand  ( QwtPicker::VLineRubberBand );
   pick->setMousePattern( QwtEventPattern::MouseSelect1, Qt::LeftButton, Qt::ControlModifier );
}

void US_RotorCalibration::reset()
{
   avg.clear();
   data.scanData.clear();
   allData.clear();

   data_plot      ->detachItems( QwtPlotItem::Rtti_PlotCurve );
   data_plot      ->clear();
   data_plot      ->replot();
   plot->btnZoom  ->setChecked( false );

   pb_load        ->setEnabled( true  );
   pb_reset       ->setEnabled( false );
   pb_accept      ->setEnabled( false );
   pb_calculate   ->setEnabled( false );
   pb_save        ->setEnabled( false );
   pb_view        ->setEnabled( false );

   cb_assigned    ->setChecked( false );

   QPalette p     = US_GuiSettings::pushbColor();

//   disconnect(ct_cell);
//   disconnect(ct_channel);
//   ct_cell        ->setRange(0, 0, 1);
//   ct_channel     ->setRange(0, 0, 1);
//   connect (ct_channel, SIGNAL(valueChanged (double)), this, SLOT(update_channel(double)));
//   connect (ct_cell,    SIGNAL(valueChanged (double)), this, SLOT(update_cell(double)));

   rotor          = "Default Rotor";
   le_instructions->setText("Please load a calibration data set...");
}

// Process situation where the disk/db selection has changed
void US_RotorCalibration::source_changed( bool db )
{
   QStringList DB = US_Settings::defaultDB();

   if ( db && ( DB.size() < 5 ) )
   {
      QMessageBox::warning( this,
         tr( "Attention" ),
         tr( "There is no default database set." ) );
   }

   load();
   reset();
}

// Changes the default data location
void US_RotorCalibration::update_disk_db( bool db )
{
   ( db ) ? disk_controls->set_db() : disk_controls->set_disk();
}

// Function to load calibration data
void US_RotorCalibration::load( void )
{
   if ( disk_controls->db() )
      loadDB();

   else
      loadDisk();

}

// load the experimental calibration dataset and store all data in allData,
// an array of all scans, cells, channels and wavelengths.
void US_RotorCalibration::loadDisk( void )
{  
   // Ask for data directory
   workingDir = QFileDialog::getExistingDirectory( this, 
         tr("US3 Raw Data Directory"), 
         US_Settings::resultDir(), 
         QFileDialog::DontResolveSymlinks );

   // Restore area beneath dialog
   qApp->processEvents();

   if ( workingDir.isEmpty() ) return;

   workingDir.replace( "\\", "/" );  // WIN32 issue
   if ( workingDir.right( 1 ) != "/" ) workingDir += "/"; // Ensure trailing /

   reset();

   QStringList components =  workingDir.split( "/", QString::SkipEmptyParts );  
   
   runID = components.last();

   QStringList nameFilters = QStringList( "*.auc" );

   QDir d( workingDir );

   files =  d.entryList( nameFilters, 
         QDir::Files | QDir::Readable, QDir::Name );

   if ( files.size() == 0 )
   {
      QMessageBox::warning( this,
            tr( "No Files Found" ),
            tr( "There were no files of the form *.auc\n"  
                "found in the specified directory." ) );
      return;
   }

   // Look for cell / channel / wavelength combinations
   maxcell=0;
   for ( int i = 0; i < files.size(); i++ )
   {
      QStringList part = files[ i ].split( "." );

      QString t = part[ 2 ] + " / " + part[ 3 ] + " / " + part[ 4 ];
      if (maxcell < part[2].toInt())
      {
         maxcell = part[2].toInt();
      }
      if ( ! triples.contains( t ) ) triples << t;
   }
   ct_cell->setRange(1, maxcell, 1);
   ct_channel->setRange(1, 2, 1);

   // Read all data
   if ( workingDir.right( 1 ) != "/" ) workingDir += "/"; // Ensure trailing /

   QString file;
   foreach ( file, files )
   {
      QString filename = workingDir + file;
      
      int result = US_DataIO2::readRawData( filename, data );
      if ( result != US_DataIO2::OK )
      {
         QMessageBox::warning( this,
            tr( "UltraScan Error" ),
            tr( "Could not read data file.\n" ) 
            + US_DataIO2::errorString( result ) + "\n" + filename );
         return;
      }

      allData << data;
      data.scanData.clear();
   }

   if ( allData.isEmpty() )
   {
      QMessageBox::warning( this,
         tr( "UltraScan Error" ),
         tr( "Could not read any data file." ) );
      return;
   }

   dataType = QString( QChar( data.type[ 0 ] ) ) 
            + QString( QChar( data.type[ 1 ] ) );

   if ( dataType != "RI" )
   {
      le_instructions->setText( tr("Attention - ") + runID + tr(" is not intensity data!"));
      return;
   }
   Limit tmp_limit;
   limit.clear();
   for (int i=0; i<allData.size(); i++) // all the triples
   {
      tmp_limit.used[0] = false;
      tmp_limit.used[1] = false;
      limit.push_back(tmp_limit);
   }

   current_triple = -1;
   top_of_cell = false;
   pb_reset->setEnabled ( true );
   pb_accept->setEnabled( true );
   next();
}

void US_RotorCalibration::loadDB( void )
{
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      QMessageBox::information( this,
             tr( "Error" ),
             tr( "Database connectivity error" ) );
   
      return;
   }

   // Present a dialog to ask user which experiment to load
   QString expID;
   US_GetDBExp dialog( expID );
   if ( dialog.exec() == QDialog::Rejected )
      return;

   if ( expID == QString( "" ) )
      return;

   // Restore area beneath dialog
   qApp->processEvents();

   // Get the rawDataID's that correspond to this experiment
   QStringList q( "get_rawDataIDs" );
   q  << expID;
   db.query( q );

   QStringList rawDataIDs;
   QStringList filenames;

   while ( db.next() )
   {
      rawDataIDs << db.value( 0 ).toString();
      filenames  << db.value( 2 ).toString();
   }

   if ( rawDataIDs.size() < 1 )
   {
      QMessageBox::information( this,
             tr( "Error" ),
             tr( "There were no auc files found in the database." ) );
      return;
   }

   // Set the runID
   QStringList parts =  filenames[ 0 ].split( "." );  
   runID = parts[ 0 ];

   // Look for cell / channel / wavelength combinations
   maxcell=0;
   for ( int i = 0; i < filenames.size(); i++ )
   {
      QStringList part = filenames[ i ].split( "." );

      QString t = part[ 2 ] + " / " + part[ 3 ] + " / " + part[ 4 ];
      if (maxcell < part[2].toInt())
      {
         maxcell = part[2].toInt();
      }
      if ( ! triples.contains( t ) ) triples << t;
   }
   ct_cell->setRange(1, maxcell, 1);
   ct_channel->setRange(1, 2, 1);

   // Load the data
   QString  tempdir  = US_Settings::tmpDir() + "/";
   for ( int i = 0; i < rawDataIDs.size(); i++ )
   {
      QString filename = tempdir + filenames[ i ];
      db.readBlobFromDB( filename, QString( "download_aucData" ), rawDataIDs[ i ].toInt() );

      int result = US_DataIO2::readRawData( filename, data );
      if ( result != US_DataIO2::OK )
      {
         QMessageBox::warning( this,
            tr( "Error" ),
            tr( "Could not read data file.\n" ) 
            + US_DataIO2::errorString( result ) + "\n" + filename );
         return;
      }

      dataType = QString( QChar( data.type[ 0 ] ) ) 
               + QString( QChar( data.type[ 1 ] ) );
      
      if ( dataType != "RI" )
      {
         QMessageBox::information( this,
                tr( "Error" ),
                runID + tr( " is not intensity data!" ) );
         return;
      }

      allData << data;
      data.scanData.clear();

      QFile( filename ).remove();
   }

   Limit tmp_limit;
   limit.clear();
   for (int i=0; i<allData.size(); i++) // all the triples
   {
      tmp_limit.used[0] = false;
      tmp_limit.used[1] = false;
      limit.push_back(tmp_limit);
   }

   current_triple = -1;
   top_of_cell = false;
   pb_reset->setEnabled ( true );
   pb_accept->setEnabled( true );
   next();
}

// plot all selected limit regions. Routine will automatically
// filter out desired cells only, and show only upper or lower regions
void US_RotorCalibration::plotAll( void )
{
   data_plot->detachItems( QwtPlotItem::Rtti_PlotCurve );
   data_plot->clear();
   data_plot->setTitle(tr( "Intensity Data (Channel A in red, Channel B in green)" ));
   data_plot->setAxisTitle( QwtPlot::xBottom, tr( "Radius (in cm)" ) );
   data_plot->setAxisTitle( QwtPlot::yLeft, tr( "Intensity" ) );
   QwtPlotCurve* c;
   QPen channelAPen( Qt::red );
   QPen channelBPen( Qt::green );
   for (int j=0; j<allData[current_triple].scanData.size(); j++) //all scans in each triple
   {
      QString title="";
      QString guid = US_Util::uuid_unparse( (uchar*)allData[current_triple].rawGUID );
      char type[ 3 ];
      type[ 2 ] = '\0';
      memcpy( type, allData[current_triple].type, 2 );
      QTextStream ts(&title);
      ts << tr("Cell ") << allData[current_triple].cell
      << tr( ", Channel ") << allData[current_triple].channel
      << tr(", Speed ") << allData[current_triple].scanData[j].rpm
      << tr(", Scan ") << j
      << tr(", GUID ") << guid
      << tr(", Type ") << type;
      //qDebug() << title;
      c = us_curve( data_plot, title );
      c->setPaintAttribute( QwtPlotCurve::ClipPolygons, true );
      int size = allData[current_triple].scanData[j].readings.size();
      double *x = new double [size];
      double *y = new double [size];
      for (int k=0; k<size; k++)
      {
         x[k] = allData[current_triple].x[k].radius;
         y[k] = allData[current_triple].scanData[j].readings[k].value;
      }
      ct_channel->disconnect();
      if ((QString) allData[current_triple].channel == "A")
      {
         c->setPen(channelAPen);
         ct_channel->setValue(1);
      }
      else
      {
         c->setPen(channelBPen);
         ct_channel->setValue(2);
      }
      connect (ct_channel, SIGNAL(valueChanged (double)), this, SLOT(update_channel(double)));
      c->setData( x, y, allData[current_triple].scanData[j].readings.size() );
      delete x;
      delete y;
   }
   if (top_of_cell)
   {
      data_plot->setAxisScale( QwtPlot::xBottom, 5.7, 6.1 );
      cb_assigned->setChecked(limit[current_triple].used[0]);
      rb_top->setChecked( true );
   }
   else
   {
      data_plot->setAxisScale( QwtPlot::xBottom, 6.9, 7.3 );
      cb_assigned->setChecked(limit[current_triple].used[1]);
      rb_bottom->setChecked( true );
   }
   data_plot->replot();
}

// check the limits of the zoomed region and store them in limits[],
// for each limit of the cells and counterbalance. If a limit has been defined,
// set the flag for this limit set to true. (disabled) Only assign rectangles that result
// from zoom actions when newlimit is true. Set newlimit to false immediately
// afterwards to prevent automatic zoom actions to override a zoom limit
void US_RotorCalibration::currentRect (QwtDoubleRect rect)
{
   if (newlimit)
   {
      if (top_of_cell)
      {
         limit[current_triple].rect[0] = rect;
         limit[current_triple].used[0] = true;
         cb_assigned->setChecked( true );
      }
      else
      {
         limit[current_triple].rect[1] = rect;
         limit[current_triple].used[1] = true;
         cb_assigned->setChecked( true );
      }
      pb_calculate->setEnabled( true );
   }
   newlimit = false;
}


// Once a limit region has been zoomed the user will accept the limits and
// the routine will automatically cycle to the next limit region. If all limit
// regions have been assigned, the routine will automatically calculate the
// stretch coefficients.
// This function will set up the desired limit region for zooming to capture
// the corresponding limit[] entry
void US_RotorCalibration::next()
{
   if (!top_of_cell)
   {
      current_triple ++;
      if (current_triple == allData.size())
      {
         current_triple --; // make sure we don't go out of bound
         le_instructions->setText("All vertical regions have been reviewed, calculating rotor calibration...");
         pb_accept->setEnabled( false );
         calculate();
         return;
      }
      current_cell = allData[current_triple].cell;
      current_channel = (QString) allData[current_triple].channel;
      ct_cell->disconnect();
      ct_channel->disconnect();
      ct_cell->setValue(allData[current_triple].cell);
      if ((QString) allData[current_triple].channel == "A")
      {
         ct_channel->setValue(1.0);
      }
      else
      {
         ct_channel->setValue(2.0);
      }
      connect (ct_cell, SIGNAL(valueChanged (double)), this, SLOT(update_cell(double)));
      connect (ct_channel, SIGNAL(valueChanged (double)), this, SLOT(update_channel(double)));
   }
   top_of_cell = !top_of_cell;
   pb_accept->setEnabled( true );
   le_instructions->setText( tr( "Please zoom the left vertical region and click "
            "\"Accept\" when done zooming..." ) );
   update_plot();
}

// calculates the stretch coefficients based on the averages that
// are gleaned from the limit regions applied over the corresponding raw data.
// If there are multiple scans for each cell at each speed, this routine
// will also average the averages from each scan for corresponding
// speed, cell, channel, top or bottom. Then the routine will calculate
// the differences between the first speed and all other speeds for each
// unique cell, channel, position, and speed entry. These differences will
// again be averaged for all entries that have the same speed. Those averages
// will be plotted and fitted to a 2nd order polynomial (quadratic function)
// to obtain the stretch function. The point (0,0) is added to the measured
// values to assure that the zeroth-order coefficient is close to zero. 
void US_RotorCalibration::calculate()
{

   QString str = "";
   avg.clear();
   Average tmp_avg;

   // For each cell, channel, speed use the appropriate limits to average
   // the points within the limits, producing two points for each scan, one
   // for the top of the channel and one for the bottom of the channel. These
   // points are stored in the avg structure together with the identifying
   // cell, channel and speed information

   for ( int i = 0; i < allData.size(); i++ ) // all the triples
   {
      for ( int j = 0; j < allData[ i ].scanData.size(); j++ ) //all scans in each triple
      {
         if ( ( QString)allData[ i ].channel == "A" )
         {
            tmp_avg.channel = 0;
         }
         else
         {
            tmp_avg.channel = 1;
         }
         
         tmp_avg.cell = allData[ i ].cell;
         tmp_avg.rpm = (int) allData[ i ].scanData[ j ].rpm;

         if ( limit[ i ].used[ 0 ] )
         {
            tmp_avg.top = findAverage( limit[i].rect[ 0 ], allData[i], j );
         }
         else
         {
            tmp_avg.top = 0.0;
         }
         
         if ( limit[ i ].used[ 1 ] )
         {
            tmp_avg.bottom = findAverage( limit[ i ].rect[ 1 ], allData[ i ], j );
         }
         else
         {
            tmp_avg.bottom = 0.0;
         }

         avg.push_back(tmp_avg);
      }
   }

   QVector< int > speeds;
   speeds.clear();
   QVector< int > cells;
   cells.clear();

   // Find out how many unique speeds and cells there are in the experiment:

   for ( int i = 0; i < allData.size(); i++ ) // all the triples
   {
      for ( int j = 0; j < allData[ i ].scanData.size(); j++ ) //all scans in each triple
      {
         if ( ! speeds.contains( (int)allData[ i ].scanData[ j ].rpm ) )
         {
            speeds << (int)allData[ i ].scanData[ j ].rpm;
         }

         if ( ! cells.contains( allData[ i ].cell ) )
         {
            cells << allData[ i ].cell;
         }
      }
   }

   qSort( speeds ); // sort the speeds with the slowest being the first element
   QVector< Average > avg2;
   avg2.clear();

   // in order to average out the top and bottom values for multiple scans
   // performed at the same speed, channel and cell we first find out how
   // many points there are in each set, and save the results in tmp_avg.
   // If there are no points for a corresponding cell, channel, top/bottom
   // and speed, set the average for those to zero. Next, average by all
   // points available and save the results in avg2.

   for ( int i = 0; i < speeds.size(); i++ )
   {
      for ( int j = 0; j < cells.size(); j++ )
      {
         for ( int k = 0; k < 2; k++ )
         {
            tmp_avg.top          = 0;
            tmp_avg.bottom       = 0;
            tmp_avg.top_count    = 0;
            tmp_avg.bottom_count = 0;
            tmp_avg.cell         = j;
            tmp_avg.channel      = k;
            tmp_avg.rpm          = speeds[ i ];

            for ( int L = 0; L < avg.size(); L++ )
            {
               if ( avg[ L ].rpm     == speeds[ i ] && 
                    avg[ L ].cell    == cells[ j ] && 
                    avg[ L ].channel == k )
               {
                  if ( avg[ L ].top != 0 )
                  {
                     tmp_avg.top += avg[ L ].top;
                     tmp_avg.top_count++;
                  }
                  if ( avg[ L ].bottom != 0 )
                  {
                     tmp_avg.bottom += avg[ L ].bottom;
                     tmp_avg.bottom_count++;
                  }
               }
            }

            if ( tmp_avg.top_count == 0 )
            {
               tmp_avg.top = 0;
            }
            else
            {
               tmp_avg.top = tmp_avg.top / tmp_avg.top_count;
            }

            if ( tmp_avg.bottom_count == 0 )
            {
               tmp_avg.bottom = 0;
            }
            else
            {
               tmp_avg.bottom = tmp_avg.bottom / tmp_avg.bottom_count;
            }

            avg2.push_back( tmp_avg );
         }
      }
   }
 
   // Collect all averages for each cell, channel and speed in a 2-dimensional
   // array where each row is another cell, channel or top and bottom position
   // (reading[i]), and each column is another speed  (reading[i][j]). On the
   // second dimension the entries are ordered with increasing speed. Each
   // 'reading' now contains the combined average of a respective cell,
   // channel, top and bottom position, ordered by speed in the second
   // dimension of 'reading'.

   QVector< double > entry_top;
   QVector< double > entry_bottom;
   
   for ( int i = 0; i < reading.size(); i++ )
   {
      reading[ i ].clear();
   }

   reading.clear();
   
   for ( int i = 0; i < maxcell; i++ ) //cells
   {
      for ( int j = 0; j < 2; j++ ) //channels
      {
         entry_top.clear();
         entry_bottom.clear();

         for ( int k = 0; k < speeds.size(); k++ )
         {
            for ( int L = 0; L < avg2.size(); L++ )
            {
               if ( avg2[ L ].rpm     == speeds[ k ] && 
                    avg2[ L ].cell    == i           && 
                    avg2[ L ].channel == j )
               {
                  entry_top    << avg2[ L ].top;
                  entry_bottom << avg2[ L ].bottom;
               }
            }
         }

         reading <<  entry_top;
         reading <<  entry_bottom;
      }
   }
   
   for ( int i = 0; i < reading.size(); i++ )
   {
      str = "";
      QTextStream ts( &str );
      
      for ( int j = 0; j < reading[ i ].size(); j++ )
      {
         if ( (int) reading[ i ][ j ] != 0 )
         ts << reading[ i ][ j ] << "\t";
      }
   }

   // For each speed, generate the differences by subtracting the position of
   // the lowest speed.
   
   for ( int i = 0; i < reading.size(); i++ )
   {
      str = "";
      QTextStream ts( &str );

      for ( int j = 1; j < reading[ i ].size(); j++ )
      {
         if ( (int)reading[ i ][ j ] != 0 )
         ts << reading[ i ][ j ] - reading[ i ][ 0 ]<< "\t";
      }
   }


   // Calculate the averages and standard deviations for all entries for a
   // given speed:

   stretch_factors.clear();
   std_dev.clear();

   for ( int i = 1; i < speeds.size(); i++ )
   {
      entry_top.clear();
      double sum1 = 0.0;
      int    k    = 0;

      for ( int j = 0; j < reading.size(); j++ )
      {
         if ( i < reading[ j ].size() && (int)reading[ j ][ i ] != 0 )
         {
            entry_top << reading[ j ][ i ] - reading[ j ][ 0 ];
            sum1 += reading[ j ][ i ] - reading[ j ][ 0 ];
            k++;
         }
      }

      stretch_factors << sum1  /k; // save the average of all values for this speed.

      double sum2 = 0.0;

      for ( int j = 0; j < k; j++ )
      {
         sum2 += sq( entry_top[ j ] - sum1 / k );
      }
      
      std_dev << sqrt( sum2 / k );
   }
   
   int size = stretch_factors.size();

   x  .resize( size );
   y  .resize( size );
   sd1.resize( size );
   sd2.resize( size );
   
   for ( int i = 0; i < size; i++ )
   {
      x[ i ] = speeds[ i ];
      y[ i ] = stretch_factors[ i ];
   }
   
   if ( ! US_Matrix::lsfit( coef, x.data(), y.data(), size, 3 ) )
   {
      QMessageBox::warning( this,
            tr( "Data Problem" ),
            tr( "The data is inadequate for this fit order" ) );
   }

   // since the calculations were done with the lowest speed (which isn't zero) as a reference,
   // the intercept at rpm=zero should now be negative, which reflects the stretching difference
   // between zero rpm and the lowest speed used here as a reference. To correct for this error,
   // the zeroth-order term needs to be added as an offset to all stretch values and the readings
   // need to be refit, to hopefully give a vanishing zeroth order term.
   
   for ( int i = 0; i < size; i++ )
   {
      y  [ i ] = stretch_factors[ i ] - coef[ 0 ];
      sd1[ i ] = y[ i ] + std_dev[ i ];
      sd2[ i ] = y[ i ] - std_dev[ i ];
   }
   
   plot->btnZoom->setChecked( false );
   data_plot->clear();
   data_plot->replot();
   QwtPlotCurve* c1;
   QwtPlotCurve* c2;
   QwtPlotCurve* c3;
   QwtPlotCurve* c4;

   QwtSymbol sym1;
   QwtSymbol sym2;
   
   sym1.setStyle( QwtSymbol::Ellipse );
   sym1.setBrush( QColor( Qt::cyan ) );
   sym1.setPen  ( QColor( Qt::white ) );
   sym1.setSize( 10 );
   
   sym2.setStyle( QwtSymbol::Cross );
   sym2.setBrush( QColor( Qt::white ) );
   sym2.setPen  ( QColor( Qt::white ) );
   sym2.setSize( 10 );
   
   c1 = us_curve( data_plot, "Rotor Stretch" );
   c1->setData  ( x.data(), y.data(), size );
   c1->setSymbol( sym1 );
   c1->setStyle ( QwtPlotCurve::NoCurve );
   
   c2 = us_curve( data_plot, "+std Dev" );
   c2->setData  ( x.data(), sd1.data(), size );
   c2->setSymbol( sym2 );
   c2->setStyle ( QwtPlotCurve::NoCurve );

   c3 = us_curve( data_plot, "+std Dev" );
   c3->setData  ( x.data(), sd2.data(), size );
   c3->setSymbol( sym2 );
   c3->setStyle ( QwtPlotCurve::NoCurve );
   
   if ( ! US_Matrix::lsfit( coef, x.data(), y.data(), size, 3 ) )
   {
      QMessageBox::warning( this,
            tr( "Data Problem" ),
            tr( "The data is inadequate for this fit order" ) );
   }

   QVector< double > xfit( 501 );
   QVector< double > yfit( 501 );
   
   for ( int i = 0; i < 501; i++ )
   {
      xfit[ i ] = (double) i * 60000.0 / 500.0;
      yfit[ i ] = coef[ 0 ] + coef[ 1 ] * xfit[ i ] + coef[ 2 ] *  sq( xfit[ i ] );
   }
      
   c4 = us_curve( data_plot, "fit" );
   c4->setData  ( xfit.data(), yfit.data(), 501 );
   c4->setStyle ( QwtPlotCurve::Lines );
   c4->setPen   ( QColor( Qt::yellow ) );

   data_plot->setTitle(tr( "Rotor Stretch\n"
               "(Error bars = 1 standard deviation)" ) );

   data_plot->setAxisTitle( QwtPlot::xBottom, tr( "Revolutions per minute" ) );
   data_plot->setAxisTitle( QwtPlot::yLeft,   tr( "Stretch (in cm)" ) );
   data_plot->setAxisAutoScale( QwtPlot::xBottom );
   data_plot->setAxisAutoScale( QwtPlot::yLeft );
   data_plot->replot();

   pb_save->setEnabled( true );
   pb_view->setEnabled( true );

   QDateTime now = QDateTime::currentDateTime();
   fileText = "CALIBRATION REPORT FOR ROTOR: " + rotor + "\nPERFORMED ON: " + now.toString();
   fileText += "\n\nCalibration is based on data from run: " + runID;
   fileText += "\n\nThe following equation was fitted to the measured "
               "stretch values for this rotor:\n\n";
   
   fileText += "Stretch = " + QString("%1").arg(coef[0], 0, 'e', 5 ) + " + "
                            + QString("%1").arg(coef[1], 0, 'e', 5 ) + " rpm + "
                            + QString("%1").arg(coef[2], 0, 'e', 5 ) + " rpm^2\n\n";
   
   fileText += "Below is a listing of the stretching values as a function of speed:\n\n";
   fileText += "Speed: Stretch (cm): Standard Dev.:\n";
   
   fileText += QString( "%1" ).arg( 0, 5, 10 ) + "   "
             + QString( "%1" ).arg( 0.0, 0, 'e', 5 ) + "   "
             + QString( "%1" ).arg( 0.0, 0, 'e', 5 ) + "\n";
   
   for ( int i = 0; i < size; i++ )
   {
      fileText += QString( "%1" ).arg( speeds[ i + 1 ], 5, 10)             + "   "
                + QString( "%1").arg( y[ i ], 0, 'e', 5 ) + "   "
                + QString( "%1").arg( std_dev[ i ], 0, 'e', 5 )         + "\n";
   }
   fileText += "\nBased on these stretching factors, the bottom of each cell and channel at ";
   fileText += "rest is estimated to be as follows:\n\n";
   fileText += "Cell: Channel:     Top:       Bottom:     Length:       Center:\n\n";
   
   double top_avg      = 0.0;
   double bottom_avg   = 0.0;
   double length_avg   = 0.0;
   double center_avg   = 0.0;
   int    top_count    = 0;
   int    bottom_count = 0;
   int    length_count = 0;
   int    center_count = 0;
   
   for ( int j = 0; j < cells.size(); j++ )
   {
      for ( int k = 0; k < 2; k++ )
      {
         double sum1 = 0.0;
         double sum2 = 0.0;
         int     m   = 0;
         int     n   = 0;

         for ( int i = 0; i < speeds.size(); i++ )
         {
            for ( int L = 0; L < avg2.size(); L++ )
            {
               if ( avg2[ L ].rpm     == speeds[ i ] && 
                    avg2[ L ].cell    == j           && 
                    avg2[ L ].channel == k )
               {
                  if ( (int) avg2[ L ].top != 0 )
                  {
                     sum1 += avg2[ L ].top - ( coef[ 1 ] * speeds[ i ] + 
                                               coef[ 2 ] * sq( speeds[ i ] ) );
                     m++;
                  }

                  if ( (int)avg2[ L ].bottom != 0 )
                  {
                     sum2 += avg2[ L ].bottom - ( coef[ 1 ] * speeds[ i ] + 
                                                  coef[ 2 ] * sq( speeds[ i ] ) );
                     n++;
                  }
               }
            }
         }

         fileText += QString( "%1" ).arg( j + 1, 2, 10 ) + "      "
                   + QString( "%1" ).arg( k + 1, 2, 10 ) + "      ";
         
         if ( m > 0 )
         {
            fileText += QString( "%1" ).arg( sum1 / m, 0, 'e', 5 ) + " ";
            
            if ( cells.size() == 4 && j !=3 || cells.size() == 8 && j != 7 )
            {
               top_avg += sum1 / m;
               top_count++;
            }
         }
         else
         {
            fileText += "    N/D     ";
         }

         if ( n > 0 )
         {
            fileText += QString("%1").arg(sum2/n, 0, 'e', 5 ) + " ";
            
            if ( cells.size() == 4 && j !=3 || cells.size() == 8 && j != 7 )
            {
               bottom_avg += sum2 / n;
               bottom_count++;
            }
         }
         else
         {
            fileText += "    N/D     ";
         }

         if ( n > 0 && m > 0 )
         {
            fileText += QString( "%1" ).arg( (sum2 / n) - ( sum1 / m ), 0, 'e', 5 ) + " " +
                        QString( "%1" ).arg( (sum1 / m) + 
                              ( ( sum2 / n) - ( sum1 / m ) ) / 2.0, 0, 'e', 5 ) + "\n";
            
            if ( cells.size() == 4 && j != 3 || cells.size() == 8 && j != 7 )
            {
               length_avg += ( sum2 / n ) - ( sum1 / m );
               length_count++;
               center_avg += ( sum1 / m) + ( ( sum2 / n ) - ( sum1 / m ) ) / 2.0;
               center_count++;
            }
         }
         else
         {
            fileText += "    N/D           N/D     \n";
         }
      }
   }

   fileText += "_______________________________________________________________\n";
   fileText += "Avgs. for CPs:  ";
   fileText += QString( "%1" ).arg( top_avg    / top_count,    0, 'e', 5 ) + " ";
   fileText += QString( "%1" ).arg( bottom_avg / bottom_count, 0, 'e', 5 ) + " ";
   fileText += QString( "%1" ).arg( length_avg / length_count, 0, 'e', 5 ) + " ";
   fileText += QString( "%1" ).arg( center_avg / center_count, 0, 'e', 5 ) + "\n\n";
}


double US_RotorCalibration::findAverage(QwtDoubleRect rect, US_DataIO2::RawData data, int i)
{
   double average = 0.0;
   int j = 0, k = 0;
   while (data.x[j].radius < rect.right() && j < data.x.size()-1)
   {
      // Note: rect.bottom() is actually the upper limit of the bounding rectangle,
      // and rect.top() is the lower limit of the bounding rectangle - weird screen coordinates?
      if (data.x[j].radius > rect.left() &&
          data.scanData[i].readings[j].value < rect.bottom() &&
          data.scanData[i].readings[j].value > rect.top())
      {
         average += data.x[j].radius;
         k++;
      }
      j++;
   }
   if (k == 0)
   {
      return 0.0;
   }
   else
   {
      return (average/(double)k);
   }
}

void US_RotorCalibration::save( void )
{
   // load up calibration data
   US_Rotor::RotorCalibration current;

   // ID would be added later
   // This is a GUID for the calibration profile
   current.GUID        = US_Util::new_guid();
   current.coeff1      = coef[ 1 ];
   current.coeff2      = coef[ 2 ];
   current.report      = fileText;
   current.lastUpdated = QDate::currentDate();
   current.omega2t     = 0.0;     // replace ??

   // Let's verify that the experiment GUID is in the db
   // and matches the current runID
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      QMessageBox::information( this,
             tr( "Error" ),
             tr( "Database connectivity error" ) );
   
      return;
   }

   // Let's see if we can find the run ID
   QStringList q( "get_experiment_info_by_runID" );
   q << runID
     << QString::number( US_Settings::us_inv_ID() );
   db.query( q );

   if ( db.lastErrno() == US_DB2::NOROWS )
   {
      QMessageBox::information( this,
             tr( "Error" ),
             tr( "The current runID cannot be found in the database" ) );
   
      return;
   }

   // Ok, let's get more info
   db.next();
   current.calibrationExperimentID   = db.value( 1 ).toString().toInt();
   current.calibrationExperimentGUID = db.value( 2 ).toString();
   current.rotorID                   = db.value( 6 ).toString().toInt();

   q.clear();
   q  << "get_rotor_info"
      << QString::number( current.rotorID );
   db.query( q );
   db.next();
   current.rotorGUID                 = db.value( 0 ).toString();

   US_RotorGui* rotorDialog = new US_RotorGui( 
                              current,                    // calibration data
                              true,                       // this is a new calibration
                              false,                      // signal wanted
                              US_Disk_DB_Controls::DB );  // disk or db

   rotorDialog->exec();

}

// locate the corresponding triple for a particular cell/channel combination
void US_RotorCalibration::findTriple()
{
   pb_accept->setEnabled( true );
   current_triple = 0;
   while(current_triple < allData.size())
   {
      if (current_cell == allData[current_triple].cell
          && current_channel == (QString) allData[current_triple].channel)
      {
         break;
      }
      current_triple ++;
   }
   if (top_of_cell)
   {
      cb_assigned->setChecked(limit[current_triple].used[0]);
   }
   else
   {
      cb_assigned->setChecked(limit[current_triple].used[1]);
   }
   if (current_triple == allData.size()-1)
   {
      pb_accept->setEnabled( false );
   }
}

void US_RotorCalibration::update_used()
{
   cb_assigned->setChecked( false );
   if (top_of_cell)
   {
      limit[current_triple].used[0] = false;
   }
   else
   {
      limit[current_triple].used[1] = false;
   }
}

void US_RotorCalibration::update_plot()
{
   newlimit = false;
   plot->btnZoom->setChecked( false );
   plotAll();
   plot->btnZoom->setChecked( true );
   newlimit = true;
}

void US_RotorCalibration::update_cell(double val)
{
   current_cell = (int) val;
   findTriple();
   update_plot();
}

void US_RotorCalibration::update_channel(double val)
{
   if ((int) val == 1)
   {
      current_channel = "A";
   }
   else
   {
      current_channel = "B";
   }      
   findTriple();
   update_plot();
}

void US_RotorCalibration::update_position()
{
   top_of_cell = rb_top->isChecked();
   update_plot();
}

void US_RotorCalibration::view()
{
   US_Editor *edit = new US_Editor ( US_Editor::LOAD, true );
   edit->setWindowTitle( tr("Rotor Calibration Report") );
   edit->move( this->pos() + QPoint( 100, 100 ) );
   edit->resize( 600, 500 );
   edit->e->setFont( US_Widgets::fixedFont() );
   edit->e->setText( fileText );
   edit->show();
}

