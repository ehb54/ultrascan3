//! \file us_buoyancy.cpp

#include <QApplication>
#include <QDomDocument>

#include "us_buoyancy.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_investigator.h"
#include "us_run_details2.h"
#include "us_math2.h"
#include "us_util.h"
#include "us_load_auc.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_constants.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

//! \brief Main program for US_Buoyancy. Loads translators and starts
//         the class US_FitMeniscus.

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.

   US_Buoyancy w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

// Constructor
US_Buoyancy::US_Buoyancy() : US_Widgets()
{
   total_speeds = 0;
   v_line       = NULL;
   dbg_level    = US_Settings::us_debug();

   setWindowTitle( tr( "Buoyancy Equilibrium Data Analysis" ) );
   setPalette( US_GuiSettings::frameColor() );

   QVBoxLayout* top = new QVBoxLayout( this );
   top->setSpacing         ( 2 );
   top->setContentsMargins ( 2, 2, 2, 2 );

   // Put the Run Info across the entire window
   QHBoxLayout* runInfo = new QHBoxLayout();
   QLabel* lb_info = us_label( tr( "Run Info:" ), -1 );
   runInfo->addWidget( lb_info );

   le_info = us_lineedit( "", 1, true );
   runInfo->addWidget( le_info );

   top->addLayout( runInfo );

   QHBoxLayout* main = new QHBoxLayout();
   QVBoxLayout* left = new QVBoxLayout;

   // Start of Grid Layout
   QGridLayout* specs = new QGridLayout;
   int s_row = 0;

   // Row 1
   // Investigator

   QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
   connect( pb_investigator, SIGNAL( clicked() ), SLOT( sel_investigator() ) );
   specs->addWidget( pb_investigator, s_row, 0 );

   if ( US_Settings::us_inv_level() < 1 )
      pb_investigator->setEnabled( false );

   int id = US_Settings::us_inv_ID();
   QString number  = ( id > 0 ) ?
      QString::number( US_Settings::us_inv_ID() ) + ": "
      : "";
   le_investigator = us_lineedit( number + US_Settings::us_inv_name(), 1, true );
   specs->addWidget( le_investigator, s_row++, 1, 1, 3 );

   // Row 1A
   disk_controls = new US_Disk_DB_Controls;
   specs->addLayout( disk_controls, s_row++, 0, 1, 4 );

   // Row 2
   QPushButton* pb_load = us_pushbutton( tr( "Load Data" ) );
   connect( pb_load, SIGNAL( clicked() ), SLOT( load() ) );
   specs->addWidget( pb_load, s_row, 0, 1, 2 );

   pb_details = us_pushbutton( tr( "Run Details" ), false );
   connect( pb_details, SIGNAL( clicked() ), SLOT( details() ) );
   specs->addWidget( pb_details, s_row++, 2, 1, 2 );

   // Row 3
   QLabel* lb_triple = us_label( tr( "Cell / Channel / Wavelength" ), -1 );
   specs->addWidget( lb_triple, s_row, 0, 1, 2 );

   cb_triple = us_comboBox();
   connect( cb_triple, SIGNAL( currentIndexChanged( int ) ),
                       SLOT  ( new_triple         ( int ) ) );
   specs->addWidget( cb_triple, s_row++, 2, 1, 2 );

   lbl_rpms   = us_label( tr( "Speed Step (RPM) of triple" ), -1 );
   cb_rpms   = us_comboBox();
   specs->addWidget( lbl_rpms,   s_row,   0, 1, 2 );
   specs->addWidget( cb_rpms,   s_row++, 2, 1, 2 );

   // Scans
   QLabel* lbl_scan = us_label( tr( "Scan Focus:" ), -1 );
   lbl_scan->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
   specs->addWidget( lbl_scan, s_row, 0, 1, 2 );

   ct_selectScan = us_counter ( 3, 0.0, 0.0 ); // Update range upon load
   ct_selectScan->setStep( 1 );
   specs->addWidget( ct_selectScan, s_row++, 2, 1, 2 );

   // Button rows
   QBoxLayout* buttons = new QHBoxLayout;

   pb_write = us_pushbutton( tr( "Save Datapoint" ), false );
   connect( pb_write, SIGNAL( clicked() ), SLOT( write() ) );
   specs->addWidget( pb_write, s_row++, 2, 1, 2 );

   QPushButton* pb_reset = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset, SIGNAL( clicked() ), SLOT( reset() ) );
   buttons->addWidget( pb_reset );

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   buttons->addWidget( pb_help );

   QPushButton* pb_accept = us_pushbutton( tr( "Close" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( close() ) );
   buttons->addWidget( pb_accept );

   // Plot layout on right side of window
   plot = new US_Plot( data_plot,
         tr( "Absorbance Data" ),
         tr( "Radius (in cm)" ), tr( "Absorbance" ) );

   data_plot->setMinimumSize( 600, 400 );

   data_plot->enableAxis( QwtPlot::xBottom, true );
   data_plot->enableAxis( QwtPlot::yLeft  , true );

   pick = new US_PlotPicker( data_plot );
   // Set rubber band to display for Control+Left Mouse Button
   pick->setRubberBand  ( QwtPicker::VLineRubberBand );
   pick->setMousePattern( QwtEventPattern::MouseSelect1,
                          Qt::LeftButton, Qt::ControlModifier );

   left->addLayout( specs );
   left->addStretch();
   left->addLayout( buttons );

   main->addLayout( left );
   main->addLayout( plot );
   main->setStretchFactor( plot, 3 );
   top ->addLayout( main );

   reset();
}

// Select a new triple
void US_Buoyancy::new_triple( int index )
{
   current_triple = index;
   //reset_triple();

   // Need to reconnect after reset
   //connect( cb_triple, SIGNAL( currentIndexChanged( int ) ),
   //                    SLOT  ( new_triple         ( int ) ) );

   data = allData[ index ];

   // Enable pushbuttons
   pb_details  ->setEnabled( true );
//   pb_meniscus ->setEnabled( true );
//   pb_write    ->setEnabled( all_edits );
/*
   connect( ct_from, SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_from   ( double ) ) );

   connect( ct_to,   SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_to     ( double ) ) );
   if ( expIsEquil )
   {  // Equilibrium
      cb_rpms->clear();
      trip_rpms.clear();

      for ( int ii = 0; ii < data.scanData.size(); ii++ )
      {  // build unique-rpm list for triple
         QString arpm = QString::number( data.scanData[ ii ].rpm );

         if ( ! trip_rpms.contains( arpm ) )
         {
            trip_rpms << arpm;
         }
      }
      cb_rpms->addItems( trip_rpms );

      le_edtrsp->setText( cb_triple->currentText() + " : " + trip_rpms[ 0 ] );

      init_includes();
   }

   else
   {  // non-Equilibrium
      set_pbColors( pb_meniscus );
      plot_current( index );
   }
*/
//   plot_current( index );

   set_meniscus();
}


// Load an AUC data set
void US_Buoyancy::load( void )
{
   bool isLocal = ! disk_controls->db();
   reset();

   US_LoadAUC* dialog =
      new US_LoadAUC( isLocal, allData, triples, workingDir );

   connect( dialog, SIGNAL( changed       ( bool ) ),
            this,     SLOT( update_disk_db( bool ) ) );

   if ( dialog->exec() == QDialog::Rejected )  return;

   runID = workingDir.section( "/", -1, -1 );
   cb_triple->clear();
   delete dialog;
   if ( triples.size() == 0 )
   {
      QMessageBox::warning( this,
            tr( "No Files Found" ),
            tr( "There were no files of the form *.auc\n"
                "found in the specified directory." ) );
      return;
   }

   cb_triple->addItems( triples );
   connect( cb_triple, SIGNAL( currentIndexChanged( int ) ),
                       SLOT  ( new_triple         ( int ) ) );
   current_triple = 0;

   le_info->setText( runID );

   data     = allData[ 0 ];
   dataType = QString( QChar( data.type[ 0 ] ) )
            + QString( QChar( data.type[ 1 ] ) );


   for ( int ii = 0; ii < triples.size(); ii++ )
   {  // Generate file names
      QString triple = QString( triples.at( ii ) ).replace( " / ", "." );
      QString file   = runID + "." + dataType + "." + triple + ".auc";
      files << file;
   }

   QString file = workingDir + "/" + runID + "." + dataType + ".xml";
   expType = "";
   QFile xf( file );

   if ( xf.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
      QXmlStreamReader xml( &xf );

      while( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement()  &&  xml.name() == "experiment" )
         {
            QXmlStreamAttributes xa = xml.attributes();
            expType   = xa.value( "type" ).toString();
            break;
         }
      }

      xf.close();
   }

   if ( expType.isEmpty()  &&  disk_controls->db() )
   {  // no experiment type yet and data read from DB:  try for DB exp type
      US_Passwd pw;
      US_DB2    db( pw.getPasswd() );

      if ( db.lastErrno() != US_DB2::OK )
      {
         QMessageBox::warning( this, tr( "Connection Problem" ),
           tr( "Could not connect to database \n" ) + db.lastError() );
         return;
      }

      QStringList query;
      query << "get_experiment_info_by_runID" << runID
            << QString::number( US_Settings::us_inv_ID() );

      db.query( query );
      db.next();
      expType    = db.value( 8 ).toString();
   }
   else                      // insure Ulll... form, e.g., "Equilibrium"
   {
      expType    = expType.left( 1 ).toUpper() +
                   expType.mid(  1 ).toLower();
   }
   expIsBuoyancy = ( expType.compare( "Buoyancy", Qt::CaseInsensitive ) == 0 );
	if (expIsBuoyancy)
	{
		update_speedData();
      pick     ->disconnect();
      connect( pick, SIGNAL( cMouseUp( const QwtDoublePoint& ) ),
                     SLOT  ( mouse   ( const QwtDoublePoint& ) ) );

      plot_scan();

      connect( cb_rpms,   SIGNAL( currentIndexChanged( int ) ),
                          SLOT  ( new_rpmval         ( int ) ) );
   }
   else
   {  // non-Equilibrium
		     QMessageBox::warning( this, tr( "Wrong Type of Data" ),
			tr( "This analysis program requires data of type \"Buoyancy\".\n"
			    "Please load a different dataset with the correct type.") );
			return;
   }
   // Enable pushbuttons
   pb_details   ->setEnabled( true );

   // Temperature check
   double              dt = 0.0;
   US_DataIO2::RawData triple;

   foreach( triple, allData )
   {
       double temp_spread = triple.temperature_spread();
       dt = ( temp_spread > dt ) ? temp_spread : dt;
   }

   if ( dt > US_Settings::tempTolerance() )
   {
      QMessageBox::warning( this,
            tr( "Temperature Problem" ),
            tr( "The temperature in this run varied over the course\n"
                "of the run to a larger extent than allowed by the\n"
                "current threshold (" )
                + QString::number( US_Settings::tempTolerance(), 'f', 1 )
                + " " + DEGC + tr( ". The accuracy of experimental\n"
                "results may be affected significantly." ) );
   }
}


// Display run details
void US_Buoyancy::details( void )
{
   US_RunDetails2* dialog
      = new US_RunDetails2( allData, runID, workingDir, triples );
   dialog->exec();
   qApp->processEvents();
   delete dialog;
}

// Select DB investigator
void US_Buoyancy::sel_investigator( void )
{
   int investigator = US_Settings::us_inv_ID();

   US_Investigator* dialog = new US_Investigator( true, investigator );
   dialog->exec();

   investigator = US_Settings::us_inv_ID();

   QString inv_text = QString::number( investigator ) + ": "
                      +  US_Settings::us_inv_name();

   le_investigator->setText( inv_text );
}

// Reset parameters to their defaults
void US_Buoyancy::reset( void )
{
   le_info     ->setText( "" );

   ct_selectScan->disconnect();
   ct_selectScan->setMinValue( 0 );
   ct_selectScan->setMaxValue( 0 );
   ct_selectScan->setValue   ( 0 );

   cb_triple->disconnect();

   data_plot->detachItems( QwtPlotItem::Rtti_PlotCurve );
   data_plot->detachItems( QwtPlotItem::Rtti_PlotMarker );
   v_line = NULL;
   pick     ->disconnect();

   data_plot->setAxisScale( QwtPlot::xBottom, 5.7, 7.3 );
   data_plot->setAxisScale( QwtPlot::yLeft  , 0.0, 1.5 );
   grid = us_grid( data_plot );
   data_plot->replot();

   // Disable pushbuttons
   pb_details     ->setEnabled( false );
   pb_write       ->setEnabled( false );

   // Remove icons

   data.scanData .clear();
   trip_rpms     .clear();
   triples       .clear();
   cb_rpms      ->disconnect();
   cb_rpms      ->clear();
}

// Select DB investigator// Private slot to update disk/db control with dialog changes it
void US_Buoyancy::update_disk_db( bool isDB )
{
   if ( isDB )
      disk_controls->set_db();
   else
      disk_controls->set_disk();
}

// Plot a single scan curve
void US_Buoyancy::plot_scan( void )
{
   int    rsize = data.scanData[ 0 ].readings.size();
   int    ssize = data.scanData.size();
   int    count = 0;
   QVector< double > rvec( rsize );
   QVector< double > vvec( rsize );
   double* r    = rvec.data();
   double* v    = vvec.data();

   data_plot->detachItems( QwtPlotItem::Rtti_PlotCurve );
   v_line = NULL;

   double maxR  = -1.0e99;
   double minR  =  1.0e99;
   double maxV  = -1.0e99;
   double minV  =  1.0e99;
   QString srpm = cb_rpms->currentText();

   // Plot only the currently selected scan(s)
   //
   for ( int ii = 0; ii < ssize; ii++ )
   {
      US_DataIO2::Scan* s = &data.scanData[ ii ];

      QString arpm        = QString::number( s->rpm );

      if ( arpm != srpm )
         continue;

      count = 0;

      for ( int jj = 0; jj < rsize; jj++ )
      {
         r[ count ] = data.x[ jj ].radius;
         v[ count ] = s->readings[ jj ].value;

         maxR = max( maxR, r[ count ] );
         minR = min( minR, r[ count ] );
         maxV = max( maxV, v[ count ] );
         minV = min( minV, v[ count ] );

         count++;
      }

      QString title = tr( "Raw Data at " )
         + QString::number( s->seconds ) + tr( " seconds" )
         + " #" + QString::number( ii );

      QwtPlotCurve* c = us_curve( data_plot, title );
      c->setData( r, v, count );

      // Reset the scan curves within the new limits
      double padR = ( maxR - minR ) / 30.0;
      double padV = ( maxV - minV ) / 30.0;

      data_plot->setAxisScale( QwtPlot::yLeft  , minV - padV, maxV + padV );
      data_plot->setAxisScale( QwtPlot::xBottom, minR - padR, maxR + padR );
   }

   data_plot->replot();
}


// Handle a mouse click according to the current pick step
void US_Buoyancy::mouse( const QwtDoublePoint& p )
{
   double maximum = -1.0e99;
	double xpoint;

   xpoint = p.x();
            // Un-zoom
            if ( plot->btnZoom->isChecked() )
               plot->btnZoom->setChecked( false );

            draw_vline( meniscus );

            data_plot->replot();

            // Remove the left line
            if ( v_line != NULL )
            {
               v_line->detach();
               delete v_line;
               v_line = NULL;
            }
            marker = new QwtPlotMarker;
            QBrush brush( Qt::white );
            QPen   pen  ( brush, 2.0 );

            marker->setValue( meniscus, maximum );
            marker->setSymbol( QwtSymbol(
                        QwtSymbol::Cross,
                        brush,
                        pen,
                        QSize ( 8, 8 ) ) );

            marker->attach( data_plot );
         data_plot->replot();

}

// Draw a vertical pick line
void US_Buoyancy::draw_vline( double radius )
{
   double r[ 2 ];

   r[ 0 ] = radius;
   r[ 1 ] = radius;

   QwtScaleDiv* y_axis = data_plot->axisScaleDiv( QwtPlot::yLeft );

   double padding = ( y_axis->upperBound() - y_axis->lowerBound() ) / 30.0;

   double v[ 2 ];
   v [ 0 ] = y_axis->upperBound() - padding;
   v [ 1 ] = y_axis->lowerBound() + padding;

   v_line = us_curve( data_plot, "V-Line" );
   v_line->setData( r, v, 2 );

   QPen pen = QPen( QBrush( Qt::white ), 2.0 );
   v_line->setPen( pen );

   data_plot->replot();
}

void US_Buoyancy::write( void )
{
}

// Select a new speed within a triple
void US_Buoyancy::new_rpmval( int index )
{
   QString srpm = cb_rpms->itemText( index );
	update_speedData();
   set_meniscus();
   plot_scan();
}

void US_Buoyancy::set_meniscus( void )
{
}

void US_Buoyancy::update_speedData( void )
{
	sData.clear();
	US_DataIO2::SpeedData ssDat;
	int ksd    = 0;
	for ( int jd = 0; jd < allData.size(); jd++ )
	{
		data  = allData[ jd ];
		sd_offs << ksd;

		if ( jd > 0 )
			sd_knts << ( ksd - sd_offs[ jd - 1 ] );

		trip_rpms.clear();

		for ( int ii = 0; ii < data.scanData.size(); ii++ )
		{
			double  drpm = data.scanData[ ii ].rpm;
			QString arpm = QString::number( drpm );
			if ( ! trip_rpms.contains( arpm ) )
			{
				trip_rpms << arpm;
				ssDat.first_scan = ii + 1;
				ssDat.scan_count = 1;
				ssDat.speed      = drpm;
				ssDat.meniscus   = 0.0;
				ssDat.dataLeft   = 0.0;
				ssDat.dataRight  = 0.0;
				sData << ssDat;
				ksd++;
			}

			else
			{
				int jj = trip_rpms.indexOf( arpm );
				ssDat  = sData[ jj ];
				ssDat.scan_count++;
				sData[ jj ].scan_count++;
			}
		}

		if ( jd == 0 )
			cb_rpms->addItems( trip_rpms );

		total_speeds += trip_rpms.size();
	}

	sd_knts << ( ksd - sd_offs[ allData.size() - 1 ] );

	if ( allData.size() > 1 )
	{
		data   = allData[ current_triple ];
		ksd    = sd_knts[ current_triple ];
		trip_rpms.clear();
		cb_rpms ->clear();
		for ( int ii = 0; ii < ksd; ii++ )
		{
			QString arpm = QString::number( sData[ ii ].speed );
			trip_rpms << arpm;
		}
		cb_rpms->addItems( trip_rpms );
	}
}
