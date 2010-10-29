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
   step = 0; // step=0: left counterbalance
             // step=1: right counterbalance
             // step=2: left cells
             // step=3: right cells

   setWindowTitle( tr( "Edit Rotor Calibration" ) );
   setPalette( US_GuiSettings::frameColor() );
   check  = QIcon( US_Settings::usHomeDir() + "/etc/check.png" );
   

   QGridLayout* top = new QGridLayout( this );
   top->setSpacing         ( 2 );
   top->setContentsMargins ( 2, 2, 2, 2 );
   
   lbl_instructions = us_label( "Instructions:", -1 );
   top->addWidget( lbl_instructions, row, 0 );

   le_instructions = us_lineedit( "", 1 );
   le_instructions->setReadOnly( true );
   le_instructions->setFont( QFont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize()-1 ) );
   le_instructions->setText("Please load a calibration data set...");
   top->addWidget( le_instructions, row, 1, 1, 2 );

   row++;
   
   QPushButton* pb_load = us_pushbutton( tr( "Load Calibration Data" ) );
   connect( pb_load, SIGNAL( clicked() ), SLOT( load() ) );
   top->addWidget( pb_load, row, 0 );
   
   row++;
   
   pb_reset = us_pushbutton( tr( "Reset" ) );
   pb_reset->setEnabled (false);
   connect( pb_reset, SIGNAL( clicked() ), SLOT( reset() ) );
   top->addWidget( pb_reset, row, 0 );
   
   row++;
   
   QGridLayout* lo_counterbalance = us_radiobutton( tr( "Show only Counterbalance" ), rb_counterbalance );
   top->addLayout( lo_counterbalance, row, 0 );
   connect( rb_counterbalance, SIGNAL( clicked() ), SLOT( plot_all() ) );
   connect( rb_counterbalance, SIGNAL( clicked() ), SLOT( showCounterbalance() ) );

   row++;
   
   QGridLayout* lo_cells = us_radiobutton( tr( "Show only Cells" ), rb_cells );
   top->addLayout( lo_cells, row, 0 );
   connect( rb_cells, SIGNAL( clicked() ), SLOT( plot_all() ) );
   connect( rb_cells, SIGNAL( clicked() ), SLOT( showCells() ) );

   row++;
   
   QGridLayout* lo_all = us_radiobutton( tr( "Show all" ), rb_all );
   top->addLayout( lo_all, row, 0 );
   connect( rb_all, SIGNAL( clicked() ), SLOT( plot_all() ) );
   connect( rb_all, SIGNAL( clicked() ), SLOT( showAll() ) );

   row++;
   
   pb_leftCounterbalance = us_pushbutton( tr( "Left side, Counterbalance" ) );
   connect( pb_leftCounterbalance, SIGNAL( clicked() ), SLOT( leftCounterbalance() ) );
   top->addWidget( pb_leftCounterbalance, row, 0 );

   row++;
   
   pb_rightCounterbalance = us_pushbutton( tr( "Right side, Counterbalance" ) );
   connect( pb_rightCounterbalance, SIGNAL( clicked() ), SLOT( rightCounterbalance() ) );
   top->addWidget( pb_rightCounterbalance, row, 0 );

   row++;
   
   pb_leftCells = us_pushbutton( tr( "Left side, Cells" ) );
   connect( pb_leftCells, SIGNAL( clicked() ), SLOT( leftCells() ) );
   top->addWidget( pb_leftCells, row, 0 );

   row++;
   
   pb_rightCells = us_pushbutton( tr( "Right side, Cells" ) );
   connect( pb_rightCells, SIGNAL( clicked() ), SLOT( rightCells() ) );
   top->addWidget( pb_rightCells, row, 0 );

   row++;

   QPushButton* pb_accept = us_pushbutton( tr( "Accept" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( accept() ) );
   top->addWidget( pb_accept, row, 0 );
   
   row++;
   
   QLabel* lbl_spacer = us_banner( tr( "" ) );
   top->addWidget( lbl_spacer, row, 0, 1, 1 );
   
   row++;
   
   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   top->addWidget( pb_help, row, 0 );

   row++;
   
   QPushButton* pb_close = us_pushbutton( tr( "Close" ) );
   connect( pb_close, SIGNAL( clicked() ), SLOT( close() ) );
   top->addWidget( pb_close, row, 0 );
   
   // Plot layout on right side of window
   plot = new US_Plot( data_plot,
         tr( "Intensity Data (Channel A in red, Channel B in green)" ),
         tr( "Radius (in cm)" ), tr( "Intensity" ) );
   data_plot->setMinimumSize( 700, 400 );
   data_plot->enableAxis( QwtPlot::xBottom, true );
   data_plot->enableAxis( QwtPlot::yLeft  , true );
   data_plot->setAxisScale( QwtPlot::xBottom, 5.7, 7.3 );
   data_plot->setAxisAutoScale( QwtPlot::yLeft );
   top->addLayout(plot, 1, 1, row, 1);
   connect (plot, SIGNAL (zoomedCorners (QwtDoubleRect)), this, SLOT (currentRect(QwtDoubleRect)) );

   pick = new US_PlotPicker( data_plot );
   // Set rubber band to display for Control+Left Mouse Button
   pick->setRubberBand  ( QwtPicker::VLineRubberBand );
   pick->setMousePattern( QwtEventPattern::MouseSelect1, Qt::LeftButton, Qt::ControlModifier );

   for (unsigned int i=0; i<row-1; i++)
   {
      top->setRowStretch(i, 0);
   }
   top->setRowStretch( row-2, 1 );
   top->setColumnStretch( 0, 0 );
   top->setColumnStretch( 1, 1 );
}

void US_RotorCalibration::reset()
{
   data.scanData .clear();
   allData.clear();
   data_plot->detachItems( QwtPlotItem::Rtti_PlotCurve );
   data_plot->clear();
   data_plot->replot();
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
//      return;
   }

   plot_all();
   data = allData[ 0 ];
   
   //plot_current( 0 );
   pb_reset->setEnabled (true);
   leftCounterbalance();
}

void US_RotorCalibration::plot_all( void )
{
   
   data_plot->detachItems( QwtPlotItem::Rtti_PlotCurve );
   data_plot->clear();
   QwtPlotCurve* c;
   QPen channelAPen( Qt::red );
   QPen channelBPen( Qt::green );
   for (int i=0; i<allData.size(); i++) // all the triples
   {
      for (int j=0; j<allData[i].scanData.size(); j++) //all scans in each triple
      {
         if (rb_counterbalance->isChecked() && allData[i].cell < 4)
         {
            break;
         }
         else if (rb_cells->isChecked() && allData[i].cell == 4)
         {
            break;
         }
         QString title="";
         char guid[ 37 ];
         uuid_unparse( (const uchar*)allData[ i ].rawGUID, guid );
         char type[ 3 ];
         type[ 2 ] = '\0';
         memcpy( type, allData[ i ].type, 2 );
         QTextStream ts(&title);
         ts << tr("Cell ") << allData[i].cell
         << tr( ", Channel ") << allData[i].channel
         << tr(", Speed ") << allData[i].scanData[j].rpm
         << tr(", Scan ") << j
         << tr(", GUID ") << guid
         << tr(", Type ") << type;
           
         c = us_curve( data_plot, title );
         c->setPaintAttribute( QwtPlotCurve::ClipPolygons, true );
         int size = allData[i].scanData[j].readings.size();
         double *x = new double [size];
         double *y = new double [size];
         for (int k=0; k<size; k++)
         {
            x[k] = allData[i].x[k].radius;
            y[k] = allData[i].scanData[j].readings[k].value;
         }
         title = "";
         ts << allData[i].channel;
         if (title == "A")
         {
            c->setPen(channelAPen);
         }
         else
         {
            c->setPen(channelBPen);
         }
         c->setData( x, y, allData[i].scanData[j].readings.size() );
         delete x;
         delete y;
      }
   }
   data_plot->replot();
}

void US_RotorCalibration::currentRect (QwtDoubleRect rect)
{
   limits[step] = rect;
}

void US_RotorCalibration::accept ()
{
   QPalette p;
   switch(step)
   {
      case 0:
      {
         pb_leftCounterbalance->setIcon(check);
         p.setColor( QPalette::Button, Qt::green );
         pb_leftCounterbalance->setPalette(p);
         break;
      }
      case 1:
      {
         pb_rightCounterbalance->setIcon(check);
         p.setColor( QPalette::Button, Qt::green );
         pb_rightCounterbalance->setPalette(p);
         break;
      }
      case 2:
      {
         pb_leftCells->setIcon(check);
         p.setColor( QPalette::Button, Qt::green );
         pb_leftCells->setPalette(p);
         break;
      }
      case 3:
      {
         pb_rightCells->setIcon(check);
         p.setColor( QPalette::Button, Qt::green );
         pb_rightCells->setPalette(p);
         break;
      }
   }
   if (step < 3)
   {
      step++;
      next();
   }
   else
   {
      calculate();
   }
}

void US_RotorCalibration::leftCounterbalance ()
{
   QPalette p = US_GuiSettings::pushbColor();
   pb_leftCounterbalance->setIcon(QIcon());
   pb_leftCounterbalance->setPalette(p);
   step = 0;
   next();
}
   
void US_RotorCalibration::rightCounterbalance ()
{
   QPalette p = US_GuiSettings::pushbColor();
   pb_rightCounterbalance->setIcon(QIcon());
   pb_rightCounterbalance->setPalette(p);
   step = 1;
   next();
}
   
void US_RotorCalibration::leftCells ()
{
   QPalette p = US_GuiSettings::pushbColor();
   pb_leftCells->setIcon(QIcon());
   pb_leftCells->setPalette(p);
   step = 2;
   next();
}
   
void US_RotorCalibration::rightCells ()
{
   QPalette p = US_GuiSettings::pushbColor();
   pb_rightCells->setIcon(QIcon());
   pb_rightCells->setPalette(p);
   step = 3;
   next();
}
   
void US_RotorCalibration::next()
{
   plot->btnZoom->setChecked(false);
   switch(step)
   {
      case 0:
      {
         rb_counterbalance->setChecked(true);
         data_plot->setAxisScale( QwtPlot::xBottom, 5.7, 6.1 );
         le_instructions->setText("Please zoom the left vertical region of the counterbalance, and click \"Accept\" when done zooming...");
         break;
      }
      case 1:
      {
         rb_counterbalance->setChecked(true);
         data_plot->setAxisScale( QwtPlot::xBottom, 6.9, 7.3 );
         le_instructions->setText("Please zoom the right vertical region of the counterbalance, and click \"Accept\" when done zooming...");
         break;
      }
      case 2:
      {
         rb_cells->setChecked(true);
         data_plot->setAxisScale( QwtPlot::xBottom, 5.7, 6.1 );
         le_instructions->setText("Please zoom the left vertical region of the cells, and click \"Accept\" when done zooming...");
         break;
      }
      case 3:
      {
         rb_cells->setChecked(true);
         data_plot->setAxisScale( QwtPlot::xBottom, 6.9, 7.3 );
         le_instructions->setText("Please zoom the right vertical region of the cells, and click \"Accept\" when done zooming...");
         break;
      }
   }
   plot_all();
   plot->btnZoom->setChecked(true);
}

void US_RotorCalibration::showCounterbalance ()
{
   plot->btnZoom->setChecked(false);
   data_plot->setAxisScale( QwtPlot::xBottom, 5.7, 7.3 );
   plot_all();
}

void US_RotorCalibration::showCells ()
{
   plot->btnZoom->setChecked(false);
   data_plot->setAxisScale( QwtPlot::xBottom, 5.7, 7.3 );
   plot_all();
}

void US_RotorCalibration::showAll()
{
   plot->btnZoom->setChecked(false);
   data_plot->setAxisScale( QwtPlot::xBottom, 5.7, 7.3 );
   plot_all();
}

void US_RotorCalibration::calculate()
{
}
