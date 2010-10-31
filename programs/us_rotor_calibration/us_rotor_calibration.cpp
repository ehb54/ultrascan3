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

   leftCB = false;
   rightCB = false;
   leftCL = false;
   rightCL = false;
   newlimit = false;

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
   connect( rb_counterbalance, SIGNAL( clicked() ), SLOT( plotAll() ) );
   connect( rb_counterbalance, SIGNAL( clicked() ), SLOT( showCounterbalance() ) );

   row++;
   
   QGridLayout* lo_cells = us_radiobutton( tr( "Show only Cells" ), rb_cells );
   top->addLayout( lo_cells, row, 0 );
   connect( rb_cells, SIGNAL( clicked() ), SLOT( plotAll() ) );
   connect( rb_cells, SIGNAL( clicked() ), SLOT( showCells() ) );

   row++;
   
   QGridLayout* lo_all = us_radiobutton( tr( "Show all" ), rb_all );
   top->addLayout( lo_all, row, 0 );
   connect( rb_all, SIGNAL( clicked() ), SLOT( plotAll() ) );
   connect( rb_all, SIGNAL( clicked() ), SLOT( showAll() ) );

   row++;
   
   pb_leftCounterbalance = us_pushbutton( tr( "Left side, Counterbalance" ) );
   pb_leftCounterbalance->setEnabled(false);
   connect( pb_leftCounterbalance, SIGNAL( clicked() ), SLOT( leftCounterbalance() ) );
   top->addWidget( pb_leftCounterbalance, row, 0 );

   row++;
   
   pb_rightCounterbalance = us_pushbutton( tr( "Right side, Counterbalance" ) );
   pb_rightCounterbalance->setEnabled(false);
   connect( pb_rightCounterbalance, SIGNAL( clicked() ), SLOT( rightCounterbalance() ) );
   top->addWidget( pb_rightCounterbalance, row, 0 );

   row++;
   
   pb_leftCells = us_pushbutton( tr( "Left side, Cells" ) );
   pb_leftCells->setEnabled(false);
   connect( pb_leftCells, SIGNAL( clicked() ), SLOT( leftCells() ) );
   top->addWidget( pb_leftCells, row, 0 );

   row++;
   
   pb_rightCells = us_pushbutton( tr( "Right side, Cells" ) );
   pb_rightCells->setEnabled(false);
   connect( pb_rightCells, SIGNAL( clicked() ), SLOT( rightCells() ) );
   top->addWidget( pb_rightCells, row, 0 );

   row++;

   pb_accept = us_pushbutton( tr( "Accept" ) );
   pb_accept->setEnabled(false);
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
   leftCB = false;
   rightCB = false;
   leftCL = false;
   rightCL = false;
   avg.clear();
   data.scanData.clear();
   allData.clear();
   data_plot->detachItems( QwtPlotItem::Rtti_PlotCurve );
   data_plot->clear();
   data_plot->replot();
   pb_reset->setEnabled (false);
   pb_accept->setEnabled (false);
   le_instructions->setText("Please load a calibration data set...");
   QPalette p = US_GuiSettings::pushbColor();
   pb_rightCells->setIcon(QIcon());
   pb_rightCells->setPalette(p);
   pb_leftCells->setIcon(QIcon());
   pb_leftCells->setPalette(p);
   pb_rightCounterbalance->setIcon(QIcon());
   pb_rightCounterbalance->setPalette(p);
   pb_leftCounterbalance->setIcon(QIcon());
   pb_leftCounterbalance->setPalette(p);
   pb_reset->setEnabled (false);
   pb_leftCounterbalance->setEnabled(false);
   pb_rightCounterbalance->setEnabled(false);
   pb_leftCells->setEnabled(false);
   pb_rightCells->setEnabled(false);
   rb_counterbalance->setChecked(true);
   plot->btnZoom->setChecked(false);   
}

// load the experimental calibration dataset and store all data in allData,
// an array of all scans, cells, channels and wavelengths.
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

   plotAll();
   data = allData[ 0 ];
   
   //plot_current( 0 );
   pb_reset->setEnabled (true);
   pb_leftCounterbalance->setEnabled(true);
   pb_rightCounterbalance->setEnabled(true);
   pb_leftCells->setEnabled(true);
   pb_rightCells->setEnabled(true);
   leftCounterbalance();
}

// plot all selected limit regions. Routine will automatically
// filter out desired cells only, and show only upper or lower regions
void US_RotorCalibration::plotAll( void )
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
         if (rb_counterbalance->isChecked() && allData[i].cell < maxcell)
         {
            break;
         }
         else if (rb_cells->isChecked() && allData[i].cell == maxcell)
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

// check the limits of the zoomed region and store them in limits[],
// for each limit of the cells and counterbalance. If a limit has been defined,
// set the flag for this limit set to true. Only assign rectangles that result
// from zoom actions when newlimit is true. Set newlimit to false immediately
// afterwards to prevent automatic zoom actions to override a zoom limit
void US_RotorCalibration::currentRect (QwtDoubleRect rect)
{
   if (newlimit)
   {
      limits[step] = rect;
      switch (step)
      {
         case 0:
         {
            leftCB = true;
            break;
         }
         case 1:
         {
            rightCB = true;
            break;
         }
         case 2:
         {
            leftCL = true;
            break;
         }
         case 3:
         {
            rightCL = true;
            break;
         }
      }
   }
   newlimit = false;
}

// if all channel limits are defined we can continue to the calculation routine:
void US_RotorCalibration::checkAccept()
{
   if (leftCB == true && rightCB == true && leftCL == true && rightCL == true)
   {
      newlimit = false;
      step = 3;
      calculate();
   }
}

// Once a limit region has been zoomed the user will accept() the limits and
// the routine will automatically cycle to the next limit region. If all limit
// regions have been assigned, the routine will automatically calculate the
// stretch coefficients.
void US_RotorCalibration::accept()
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
   checkAccept();
   if (step < 3)
   {
      step++;
      next();
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
   
// This function will set up the desired limit region for zooming to capture
// the corresponding limit[] entry
void US_RotorCalibration::next()
{
   pb_accept->setEnabled(true);
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
   plotAll();
   plot->btnZoom->setChecked(true);
   newlimit = true;
}

void US_RotorCalibration::showCounterbalance ()
{
   plot->btnZoom->setChecked(false);
   data_plot->setAxisScale( QwtPlot::xBottom, 5.7, 7.3 );
   plotAll();
}

void US_RotorCalibration::showCells ()
{
   plot->btnZoom->setChecked(false);
   data_plot->setAxisScale( QwtPlot::xBottom, 5.7, 7.3 );
   plotAll();
}

void US_RotorCalibration::showAll()
{
   plot->btnZoom->setChecked(false);
   data_plot->setAxisScale( QwtPlot::xBottom, 5.7, 7.3 );
   plotAll();
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
   int i, j, k, l;
   avg.clear();
   Average tmp_avg;
   /*
   For each cell, channel, speed use the appropriate limits to average
   the points within the limits, producing two points for each scan, one
   for the top of the channel and one for the bottom of the channel. These
   points are stored in the avg structure together with the identifying
   cell, channel and speed information
   */
   for (i=0; i<allData.size(); i++) // all the triples
   {
      for (j=0; j<allData[i].scanData.size(); j++) //all scans in each triple
      {
         if((QString) allData[i].channel == "A")
         {
            tmp_avg.channel = 0;
         }
         else
         {
            tmp_avg.channel = 1;
         }
         tmp_avg.cell = allData[i].cell;
         tmp_avg.rpm = (int) allData[i].scanData[j].rpm;
         if (allData[i].cell == maxcell)
         {
            tmp_avg.top = findAverage( limits[0], allData[i], j );
            tmp_avg.bottom = findAverage( limits[1], allData[i], j );
         }
         else
         {
            tmp_avg.top = findAverage( limits[2], allData[i], j );
            tmp_avg.bottom = findAverage( limits[3], allData[i], j );
         }
         avg.push_back(tmp_avg);
      }
   }
   /*
   for (i=0; i<4; i++)
   {
      qDebug() << limits[i].left() << limits[i].right() << limits[i].top() << limits[i].bottom();
   }
   for (i=0; i<avg.size(); i++)
   {
      qDebug() << avg[i].cell << avg[i].channel << avg[i].rpm << avg[i].top << avg[i].bottom;
   }
   */
   QVector <int> speeds;
   speeds.clear();
   QVector <int> cells;
   cells.clear();
   /*
   Find out how many unique speeds and cells there are in the experiment:
   */
   for (i=0; i<allData.size(); i++) // all the triples
   {
      for (j=0; j<allData[i].scanData.size(); j++) //all scans in each triple
      {
         if (!speeds.contains((int)allData[i].scanData[j].rpm))
         {
            speeds.push_back(allData[i].scanData[j].rpm);
         }
         if (!cells.contains(allData[i].cell))
         {
            cells.push_back(allData[i].cell);
         }
      }
   }
   qSort(speeds); // sort the speeds with the slowest being the first element
   QVector <Average> avg2;
   avg2.clear();
   /*
   in order to average out the top and bottom values for multiple scans
   performed at the same speed, channel and cell we first find out how
   many points there are in each set, and save the results in tmp_avg.
   If there are no points for a corresponding cell, channel, top/bottom
   and speed, set the average for those to zero. Next, average by all
   points available and save the results in avg2.
   */
   for (i=0; i<speeds.size(); i++)
   {
      for (j=0; j<cells.size(); j++)
      {
         for (k=0; k<2; k++)
         {
            tmp_avg.top = 0;
            tmp_avg.bottom = 0;
            tmp_avg.top_count = 0;
            tmp_avg.bottom_count = 0;
            tmp_avg.cell = j;
            tmp_avg.channel = k;
            tmp_avg.rpm = speeds[i];
            for (l=0; l<avg.size(); l++)
            {
               if (avg[l].rpm == speeds[i] && avg[l].cell == cells[j] && avg[l].channel == k)
               {
                  if (avg[l].top != 0)
                  {
                     tmp_avg.top += avg[l].top;
                     tmp_avg.top_count ++;
                  }
                  if (avg[l].bottom != 0)
                  {
                     tmp_avg.bottom += avg[l].bottom;
                     tmp_avg.bottom_count ++;
                  }
               }
            }
            if (tmp_avg.top_count == 0)
            {
               tmp_avg.top = 0;
            }
            else
            {
               tmp_avg.top = tmp_avg.top/tmp_avg.top_count;
            }
            if (tmp_avg.bottom_count == 0)
            {
               tmp_avg.bottom = 0;
            }
            else
            {
               tmp_avg.bottom = tmp_avg.bottom/tmp_avg.bottom_count;
            }
            avg2.push_back(tmp_avg);
         }
      }
   }
   /*
   for (i=0; i<avg2.size(); i++)
   {
      qDebug() << i+1 << "- Cell:" << avg2[i].cell << "channel:" << avg2[i].channel <<
            "speed:" << avg2[i].rpm << "top:" << avg2[i].top << "bottom:" << avg2[i].bottom;
   }
   collect all averages for each cell, channel and speed in a 2-dimensional array
   where each row in another cell, channel or top and bottom position (reading[i]), and each column is another speed (reading[i][j]). On the second dimension the entries are ordered
   with increasing speed. Each 'reading' now contains the combined average of a respective
   cell, channel, top and bottom position, ordered by speed in the second dimension of 'reading'.
   */
   QVector <double> entry_top, entry_bottom;
   for (i=0; i<reading.size(); i++)
   {
      reading[i].clear();
   }
   reading.clear();
   for (i=0; i<maxcell; i++) //cells
   {
      for (j=0; j<2; j++) //channels
      {
         entry_top.clear();
         entry_bottom.clear();
         for (k=0; k<speeds.size(); k++)
         {
            for (l=0; l<avg2.size(); l++)
            {
               if (avg2[l].rpm == speeds[k] && avg2[l].cell == i && avg2[l].channel == j)
               {
                  entry_top.push_back(avg2[l].top);
                  entry_bottom.push_back(avg2[l].bottom);
               }
            }
         }
         reading.push_back(entry_top);
         reading.push_back(entry_bottom);
      }
   }
   /*
   QString str="";
   for (i=0; i<reading.size(); i++)
   {
      str="";
      QTextStream ts(&str);
      for (j=0; j<reading[i].size(); j++)
      {
         if ((int) reading[i][j] != 0)
         ts << reading[i][j] << "\t";
      }
      qDebug() << str;
   }
   //For each speed, generate the differences by subtracting the position of the lowest speed.
   for (i=0; i<reading.size(); i++)
   {
      str="";
      QTextStream ts(&str);
      for (j=1; j<reading[i].size(); j++)
      {
         if ((int) reading[i][j] != 0)
         ts << reading[i][j] - reading[i][0]<< "\t";
      }
      qDebug() << str;
   }
   */
   /*
   calculate the averages and standard deviations for all entries for a given speed:
   */
   double sum1, sum2;
   stretch_factors.clear();
   std_dev.clear();
   //qDebug() << "Speedsize: " << speeds.size();
   for (i=1; i<speeds.size(); i++)
   {
      entry_top.clear();
      sum1 = 0.0;
      k=0;
      for (j=0; j<reading.size(); j++)
      {
         if(i < reading[j].size() && (int) reading[j][i] != 0)
         {
            entry_top.push_back(reading[j][i] - reading[j][0]);
            sum1 += reading[j][i] - reading[j][0];
            k++;
            //qDebug() << "Speed:" << speeds[i] << "k:" << k << "reading:" << reading[j][i] - reading[j][0];
         }
      }
      stretch_factors.push_back(sum1/k); // save the average of all values for this speed.
      sum2 = 0.0;
      for (j=0; j<k; j++)
      {
         sum2 += pow(entry_top[j] - sum1/k, 2.0);
      }
      std_dev.push_back(pow(sum2/k, 0.5));
   }
   double *x, *y, *sd1, *sd2;
   x = new double [stretch_factors.size()+1];
   y = new double [stretch_factors.size()+1];
   sd1 = new double [stretch_factors.size()+1];
   sd2 = new double [stretch_factors.size()+1];

   x[0] = 0;
   y[0] = 0;
   sd1[0] = 0;
   sd2[0] = 0;
   
   for (i=0; i<stretch_factors.size(); i++)
   {
      x[i+1] = speeds[i+1];
      y[i+1] = stretch_factors[i];
      sd1[i+1] = stretch_factors[i] + std_dev[i];
      sd2[i+1] = stretch_factors[i] - std_dev[i];
   }
   plot->btnZoom->setChecked(false);
//   data_plot->detachItems( QwtPlotItem::Rtti_PlotCurve );
   data_plot->clear();
   data_plot->replot();
   QwtPlotCurve *c1, *c2, *c3, *c4;
   QwtSymbol sym1, sym2;
   
   sym1.setStyle(QwtSymbol::Ellipse);
   sym1.setBrush(QColor(Qt::yellow));
   sym1.setPen(QColor(Qt::white));
   sym1.setSize(10);
   
   sym2.setStyle(QwtSymbol::Cross);
   sym2.setBrush(QColor(Qt::white));
   sym2.setPen(QColor(Qt::white));
   sym2.setSize(10);
   
   c1 = us_curve( data_plot, "Rotor Stretch" );
   c1->setData( x, y, stretch_factors.size()+1 );
   c1->setSymbol(sym1);
   c1->setStyle(QwtPlotCurve::NoCurve);
   
   c2 = us_curve( data_plot, "+std Dev" );
   c2->setData( x, sd1, stretch_factors.size()+1 );
   c2->setSymbol(sym2);
   c2->setStyle(QwtPlotCurve::NoCurve);

   c3 = us_curve( data_plot, "+std Dev" );
   c3->setData( x, sd2, stretch_factors.size()+1 );
   c3->setSymbol(sym2);
   c3->setStyle(QwtPlotCurve::NoCurve);
   double coef[3];
   if ( ! US_Matrix::lsfit( coef, x, y, stretch_factors.size()+1, 3 ) )
   {
      QMessageBox::warning( this,
            tr( "Data Problem" ),
            tr( "The data is inadequate for this fit order" ) );
   }

   coef[0] = 0.0; // force the zeroth order coefficient to be zero
   double *xfit = new double [501];
   double *yfit = new double [501];

   //qDebug() << coef[0] << coef[1] << coef[2];
   for (i=0; i<501; i ++)
   {
      xfit[i] = (double) i*60000/500;
      yfit[i] = coef[0] + coef[1]*xfit[i] + coef[2]*xfit[i]*xfit[i];
   }
      
   c4 = us_curve( data_plot, "fit" );
   c4->setData( xfit, yfit, 501 );
   c4->setStyle(QwtPlotCurve::Lines);
   c4->setPen(QColor(Qt::yellow));

   data_plot->setTitle(tr("Rotor Stretch (Error bars = 1 standard deviation)"));
   data_plot->setAxisTitle( QwtPlot::xBottom, "Revolutions per minute" );
   data_plot->setAxisTitle( QwtPlot::yLeft, "Stretch (in cm)" );
   data_plot->setAxisAutoScale( QwtPlot::xBottom );
   data_plot->setAxisAutoScale( QwtPlot::yLeft );
   data_plot->replot();

   delete x;
   delete y;
   delete xfit;
   delete yfit;
   delete sd1;
   delete sd2;
}

double US_RotorCalibration::findAverage(QwtDoubleRect rect, US_DataIO2::RawData data, int i)
{
   double average = 0.0;
   int j=0, k=0;
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

