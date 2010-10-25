//! \file us_rotor_calibration.cpp

#include "us_rotor_calibration.h"
#include "us_settings.h"
#include "us_license_t.h"
#include "us_math2.h"
#include "us_util.h"
#include "us_db2.h"
#include "us_license.h"
#include "us_gui_settings.h"
#include "us_run_details2.h"
#include "us_passwd.h"

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
   check  = QIcon( US_Settings::usHomeDir() + "/etc/check.png" );
   unsigned int row=0;

   setWindowTitle( tr( "Edit Rotor Calibration" ) );
   setPalette( US_GuiSettings::frameColor() );

   QGridLayout* top = new QGridLayout( this );
   top->setSpacing         ( 2 );
   top->setContentsMargins ( 2, 2, 2, 2 );
   
   lbl_instructions = us_label( "Instructions:", -1 );
   top->addWidget( lbl_instructions, row, 0 );

   le_instructions = us_lineedit( "", 1 );
   le_instructions->setReadOnly( true );
   le_instructions->setText("Please load a calibration data set...");
   top->addWidget( le_instructions, row, 1, 1, 3 );

   row++;
   
   // Row 2
   QPushButton* pb_load = us_pushbutton( tr( "Load Calibration Data" ) );
   connect( pb_load, SIGNAL( clicked() ), SLOT( load() ) );
   top->addWidget( pb_load, row, 0 );
   
   pb_reset = us_pushbutton( tr( "Reset" ) );
   pb_reset->setEnabled (false);
   connect( pb_reset, SIGNAL( clicked() ), SLOT( reset() ) );
   top->addWidget( pb_reset, row, 1 );
   
   row++;
   
   // Row 3
   QGridLayout* lo_counterbalance = us_radiobutton( tr( "Show Counterbalance" ), rb_counterbalance );
   rb_counterbalance->setChecked( false );
   top->addLayout( lo_counterbalance, row, 0 );

   QGridLayout* lo_cells = us_radiobutton( tr( "Show Cells" ), rb_cells );
   rb_cells->setChecked( true );
   top->addLayout( lo_cells, row, 1 );

   row++;
   
   // Row 4

   QLabel* lbl_spacer = us_banner( tr( "" ) );
   top->addWidget( lbl_spacer, row, 0, 1, 2 );
   
   row++;
   
   // Row 5
   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   top->addWidget( pb_help, row, 0 );

   QPushButton* pb_close = us_pushbutton( tr( "Close" ) );
   connect( pb_close, SIGNAL( clicked() ), SLOT( close() ) );
   top->addWidget( pb_close, row, 1 );
   
   // Plot layout on right side of window
   plot = new US_Plot( data_plot,
         tr( "Absorbance Data" ),
         tr( "Radius (in cm)" ), tr( "Absorbance" ) );
   data_plot->setMinimumSize( 600, 400 );
   data_plot->enableAxis( QwtPlot::xBottom, true );
   data_plot->enableAxis( QwtPlot::yLeft  , true );
   top->addLayout(plot, 1, 2, row, 2);

   pick = new US_PlotPicker( data_plot );
   // Set rubber band to display for Control+Left Mouse Button
   pick->setRubberBand  ( QwtPicker::VLineRubberBand );
   pick->setMousePattern( QwtEventPattern::MouseSelect1, Qt::LeftButton, Qt::ControlModifier );

   for (unsigned int i=0; i<row-1; i++)
   {
      top->setRowStretch(i, 0);
   }
   top->setRowStretch(row-3, 0.1);
}

void US_RotorCalibration::reset()
{
   data.scanData .clear();
   allData.clear();
   pb_reset->setEnabled (false);
   le_instructions->setText("Please load a calibration data set...");
}

void US_RotorCalibration::load( void )
{  
// Ask for data directory
   workingDir = QFileDialog::getExistingDirectory( this, tr("Raw Data Directory"), US_Settings::resultDir(), QFileDialog::DontResolveSymlinks );
   if ( workingDir.isEmpty() ) return;
   reset();
   workingDir.replace( "\\", "/" );  // WIN32 issue

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
   for ( int i = 0; i < files.size(); i++ )
   {
      QStringList part = files[ i ].split( "." );

      QString t = part[ 2 ] + " / " + part[ 3 ] + " / " + part[ 4 ];
      if ( ! triples.contains( t ) ) triples << t;
   }

   le_instructions->setText( runID + "; please review data...");

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
      le_instructions->setText("Attention - " + runID + " is not intensity data!");
//      return;
   }

   plot_all();
   data = allData[ 0 ];
   
   //plot_current( 0 );
   pb_reset->setEnabled (true);
}

void US_RotorCalibration::plot_all( void )
{
   
   data_plot->detachItems( QwtPlotItem::Rtti_PlotCurve );
   data_plot->clear();
   for (int i=0; i<allData.size(); i++) // all the triples
   {
      for (int j=0; j<allData[i].scanData.size(); j++) //all scans in each triple
      {
         //qDebug() << allData[i].description << allData[i].cell << allData[i].channel;
         /*
         QString title="", str;
         title += "Cell " + str.setNum(allData[i].cell);
         title += ", Channel " + QString::QString(allData[i].channel);
         title += ", Speed " + str.setNum(allData[i].scanData[j].rpm);
         title += ", Scan " + str.setNum(j);
         title += ", GUID " + QString::QString(allData[i].rawGUID);
         title += ", type " + QString::QString(allData[i].type);
         */
         QString title="";
         QTextStream ts(&title);
         ts << "Cell " << allData[i].cell
         << ", Channel " << allData[i].channel
         << ", Speed " << allData[i].scanData[j].rpm
         << ", Scan " << j
         << ", GUID " << allData[i].rawGUID
         << ", Type " << allData[i].type;
               
         qDebug() << title;

               //title.sprintf("Cell %i, Channel %c, Speed %f, Scan %d, GUID: %s, type: %s ", allData[i].cell, allData[i].channel, allData[i].scanData[j].rpm, j, allData[i].rawGUID, allData[i].type);
         QwtPlotCurve* c = us_curve( data_plot, title);
         c->setPaintAttribute( QwtPlotCurve::ClipPolygons, true );
         qDebug() << title;
         int size = allData[i].scanData[j].readings.size();
         double *x = new double [size];
         double *y = new double [size];
         for (int k=0; k<size; k++)
         {
            x[k] = allData[i].x[k].radius;
            y[k] = allData[i].scanData[j].readings[k].value;
         }
         c->setData( x, y, allData[i].scanData[j].readings.size() );
         delete x;
         delete y;
      }
   }
   data_plot->replot();
}
