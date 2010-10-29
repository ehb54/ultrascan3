//! \file us_dcdt.cpp

#include <QApplication>
#include <QtSvg>

#include "us_dcdt.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"

//! \brief Main program for us_dcdt. Loads translators and starts
//         the class US_Dcdt.

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_Dcdt w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

US_Dcdt::US_Dcdt() : US_AnalysisBase2()
{
   setWindowTitle( tr( "Time Derivative - dC/dt Analysis" ) );

   te_results = NULL;
   dcdt       = NULL;
   sValues    = NULL;
   avgS       = NULL;
   avgDcdt    = NULL;
   arraySizes = NULL;
   arrayStart = NULL;

   sMax       = 10.0;

   check      = QIcon( US_Settings::usHomeDir() + "/etc/check.png" );

   QLabel*       lb_aux    = us_banner( tr( "dC/dt Auxiliary Controls" ) );
   QLabel*       lb_sValue = us_label( tr( "S-value Cutoff:" ) );
   ct_sValue               = us_counter( 3, 0, 20, 20 );
   ct_sValue->setStep( 0.1 );
   connect( ct_sValue, SIGNAL( valueChanged ( double ) ), 
                       SLOT  ( sMaxChanged  ( double ) ) );
   
   QLabel*       lb_graph  = us_label( tr( "Graph Selection" ) );
   lb_graph->setAlignment ( Qt::AlignCenter );

   QGridLayout*  rb_layout1 = us_radiobutton( tr( "x:radius" ) , rb_radius, true );
   
   QRadioButton* rb_sed;
   QGridLayout*  rb_layout2 = us_radiobutton( tr( "x:S" ) , rb_sed, false );

   QRadioButton* rb_avg;
   QGridLayout*  rb_layout3 = us_radiobutton( tr( "Average S" ) , rb_avg, false );

   QButtonGroup* group = new QButtonGroup( this );
   group->addButton( rb_radius, 0 );
   group->addButton( rb_sed   , 1 );
   group->addButton( rb_avg   , 2 );
   graphType = 0;
   connect( group, SIGNAL( buttonClicked( int ) ), SLOT( set_graph( int ) ) );

   QBoxLayout* rb_layout0 = new QHBoxLayout();
   rb_layout0->addLayout( rb_layout1 );
   rb_layout0->addLayout( rb_layout2 );
   rb_layout0->addLayout( rb_layout3 );

   pb_baseline = us_pushbutton( tr( "Subtract Baseline" ) );
   pb_baseline->setIcon( QIcon() );
   connect( pb_baseline, SIGNAL( clicked() ), SLOT( subtract_bl() ) );

   int row = 7;

   controlsLayout->addWidget( lb_aux,     row++, 0, 1, 4 );

   controlsLayout->addWidget( lb_sValue,  row,   0, 1, 2 );
   controlsLayout->addWidget( ct_sValue,  row++, 2, 1, 2 );
   
   controlsLayout->addWidget( lb_graph,   row++, 0, 1, 4 );
   controlsLayout->addLayout( rb_layout0, row++, 0, 1, 4 );
   controlsLayout->addWidget( pb_baseline, row++, 0, 1, 4 );

   connect( pb_help,  SIGNAL( clicked() ), SLOT( help() ) );
   connect( pb_view,  SIGNAL( clicked() ), SLOT( view() ) );
   connect( pb_save,  SIGNAL( clicked() ), SLOT( save() ) );
}

void US_Dcdt::subtract_bl( void )
{
   if ( ! dataLoaded ) return;

   if ( pb_baseline->icon().isNull() )
      pb_baseline->setIcon( check );
   else
      pb_baseline->setIcon( QIcon() );

   data_plot();
}

void US_Dcdt::reset( void )
{
   if ( ! dataLoaded ) return;
   US_AnalysisBase2::reset();
   ct_sValue->setValue( sMax );
   rb_radius->click();
   pb_baseline->setIcon( QIcon() );
}

void US_Dcdt::sMaxChanged( double /* value */ )
{
   data_plot();
}

void US_Dcdt::set_graph( int button )
{
   if ( graphType == button ) return;

   graphType = button;
   data_plot();
}

void US_Dcdt::data_plot( void )
{
   US_AnalysisBase2::data_plot();

   int                    index  = lw_triples->currentRow();
   US_DataIO2::EditedData* d      = &dataList[ index ];

   int     scanCount   = d->scanData.size();
   int     skipped     = 0;
   double  boundaryPct = ct_boundaryPercent->value() / 100.0;
   double  positionPct = ct_boundaryPos    ->value() / 100.0;
   double  baseline    = calc_baseline();

   for ( int i = 0; i < scanCount; i++ )
   {
      if ( excludedScans.contains( i ) ) continue;
      
      double range  = d->scanData[ i ].plateau - baseline;
      double test_y = baseline + range * positionPct;
      
      if ( d->scanData[ i ].readings[ 0 ].value > test_y ) skipped++;
   }

   le_skipped->setText( QString::number( skipped ) );

   if ( scanCount - skipped - excludedScans.count() < 4 )
   {
      QMessageBox::warning( this,
            tr( "Attention" ),
            tr( "You must have at least four non-skipped scans "
                "for this analysis." ) );
      return;
   }

   // Delete old arrays if they exist to handle the case of changed triple
   if ( dcdt != NULL )
   {
      for ( int i = 0; i < scanCount; i++ )
      {
         delete [] dcdt   [ i ];
         delete [] sValues[ i ];
      }

      delete [] dcdt;
      delete [] sValues;
      delete [] avgDcdt;
      delete [] avgS;
      delete [] arraySizes;
      delete [] arrayStart;
   }
   
   int points = d->scanData[ 0 ].readings.size();

   // Create the new arrays
   dcdt       = new double* [ scanCount ];
   sValues    = new double* [ scanCount ];
   avgDcdt    = new double  [ points ];
   avgS       = new double  [ points ];
   arraySizes = new int     [ scanCount ];
   arrayStart = new int     [ scanCount ];

   for ( int i = 0; i < scanCount; i++ )
   {
      dcdt      [ i ] = new double [ points ];
      sValues   [ i ] = new double [ points ];
      arraySizes[ i ] = 0;
      arrayStart[ i ] = 0;
   }

   // Calculate dcdt and sValues
   int previous = skipped;
   int count    = 0;

   for ( int i = skipped + 1; i < scanCount; i++ )
   {
      if ( excludedScans.contains( i ) ) continue;

      US_DataIO2::Scan* thisScan = &d->scanData[ i ];
      US_DataIO2::Scan* prevScan = &d->scanData[ previous ];

      double range       = thisScan->plateau - baseline;
      double lower_limit = baseline    + range * positionPct;
      double upper_limit = lower_limit + range * boundaryPct;
            
      double dt          = thisScan->seconds - prevScan->seconds;
     
      double adjust      = ( pb_baseline->icon().isNull() ) ? 0.0 : baseline; 

      double plateau     = thisScan->plateau - adjust;
      double prevPlateau = prevScan->plateau - adjust;

      double meniscus    = d->meniscus;
      double omega       = thisScan->rpm * M_PI / 30.0;

      int    size        = 0;
      bool   started     = false;

      for ( int j = 0; j < points; j++ )
      {
         double currentV  = thisScan->readings[ j ].value - adjust;
         double previousV = prevScan->readings[ j ].value - adjust;

         if ( currentV < lower_limit ) continue;
         if ( currentV > upper_limit ) break;

         if ( ! started )
         {
            started = true;
            arrayStart[ count ] = j;
         }

         double dC = currentV / plateau - previousV / prevPlateau;

         // We are really plotting g*(s) so -dC/dt is saved for the plots
         dcdt[ count ][ size ] = -dC / dt;
      
         //double radius = d->x[ j ].radius;
         double radius = d->radius( j );

         sValues[ count ][ size ] = 
            solution.s20w_correction * 1.0e13 * log( radius / meniscus ) /
            ( sq( omega ) * ( prevScan->seconds + dt / 2.0 ) );

         size++;
      }

      arraySizes[ count ] = size;

      count++;
      previous = i;
   }

   int    last  = arraySizes[ 0 ] - 1;
   double s_max = sValues[ 0 ][ last ];

   ct_sValue->setMaxValue( ceil( s_max ) );

   if ( s_max < sMax )
   {
      if ( ct_sValue->value() > s_max )
      {
         ct_sValue->disconnect();
         ct_sValue->setValue( ceil( s_max ) );

         connect( ct_sValue, SIGNAL( valueChanged ( double ) ), 
                             SLOT  ( sMaxChanged  ( double ) ) );
      }

      sMax = s_max;
   }

   double s_min = 9.9e10;

   for ( int i = 0; i < count; i++ )
   {
      for ( int j = 0; j < arraySizes[ i ]; j++ )
      {
         s_min = min( s_min, sValues[ i ][ j ] );
      }
   }

   // Figure the total points to plot
   double increment   = ( s_max - s_min ) / arrayLength;

   // Assign new equally spaced s-values for the x-axis
   for ( int i = 0; i < arrayLength; i++ ) 
      avgS[ i ] = s_min + i * increment;

   // For each new s value, find the corresponding g*(s) through
   // linear interpolation and add up, then average by the number of
   // total scans included.
   
   for ( int j = 0; j < arrayLength; j++ ) 
   {
      avgDcdt[ j ] = 0.0;

      for ( int i = 0; i < count; i++ )
      {
         int k = 1;

         // Find index where svalue for the scan first exceeds 
         // the point on the x-axis
         while ( sValues[ i ][ k ] < avgS[ j ] ) 
         {
            k++;
            if ( k == arraySizes[ i ] ) goto next; // Skip rest of scans
         }

         // Interpolate and apply y = mx + b
         double m = ( dcdt   [ i ][ k ] - dcdt   [ i ][ k - 1 ] ) /
                    ( sValues[ i ][ k ] - sValues[ i ][ k - 1 ] );
         
         double b = dcdt[ i ][ k ] - m * sValues[ i ][ k ];
         
         avgDcdt[ j ] += m * avgS[ j ] + b;
      }

next: avgDcdt[ j ] /= ( count - 1 ); 
   }

   // Draw plot
   data_plot1->clear();
   us_grid( data_plot1 );

   data_plot1->setTitle( tr( "Run " ) + d->runID + tr( ": Cell " ) + d->cell
         + " (" + d->wavelength + tr( " nm) - Time Derivative (dC/dt)" ) );

   data_plot1->setAxisTitle( QwtPlot::yLeft  , tr( "g<sup>*</sup>(s)" ) );

   double*       x = new double[ points ];
   double*       y = new double[ points ];
   QwtPlotCurve* curve;
   QwtText       title = QwtText( tr( "<b>Sedimentation Coefficient x "
                                      "10<sup>13</sup> sec</b>" ) );
         
   data_plot1->setAxisTitle( QwtPlot::xBottom, title );
         
   // Draw the desired plot
   switch ( graphType )
   {
      case 0:  // Radius Plot
         data_plot1->setAxisTitle( QwtPlot::xBottom, tr( "Radius (cm)" ) );


         for ( int i = 0; i < count; i++ )
         {
            int size = 0;
            int end  = arraySizes[ i ] + arrayStart[ i ];

            for ( int j = arrayStart[ i ]; j < end; j++ ) 
               x[ size++ ] = d->x[ j ].radius;

            curve = us_curve( data_plot1, 
                  tr( "Scan " ) + QString::number( i + skipped + 1 ) );

            curve->setData( x, dcdt[ i ], arraySizes[ i ] );
         }

         data_plot1->setAxisAutoScale( QwtPlot::xBottom );

         break;

      case 1:  // S-value plot
         for ( int i = 0; i < count; i++ )
         {
            curve = us_curve( data_plot1, 
                  tr( "Scan " ) + QString::number( i + skipped + 1 ) );

            curve->setData( sValues[ i ], dcdt[ i ], arraySizes[ i ] );
         }
         
         data_plot1->setAxisScale( QwtPlot::xBottom, 0.0, ct_sValue->value() );

         break;

      case 2:  // Average dC/dt or g*(s) plot
         data_plot1->setAxisTitle( QwtPlot::yLeft, 
                                   tr( "Average g<sup>*</sup>(s)" ) );

         curve = us_curve( data_plot1, tr( "Average g*(s)" ) );
         curve->setData( avgS, avgDcdt, arrayLength );

         data_plot1->setAxisScale( QwtPlot::xBottom, 0.0, ct_sValue->value() );
         break;
   }

   data_plot1->replot();

   delete [] x;
   delete [] y;
}

void US_Dcdt::view( void )
{
   int                    index  = lw_triples->currentRow();
   US_DataIO2::EditedData* d      = &dataList[ index ];

   // Create US_Editor
   if ( te_results == NULL )
   {
      te_results = new US_Editor( US_Editor::DEFAULT, true, QString(), this );
      te_results->resize( 500, 400 );
      QPoint p = g.global_position();
      te_results->move( p.x() + 30, p.y() + 30 );
      te_results->e->setFont( QFont( US_GuiSettings::fontFamily(),
                                     US_GuiSettings::fontSize() ) );
   }

   // Add results to window
   QString s = "<html><head>\n"
               "<style>td { padding-right: 1em;}</style>\n"
               "</head><body>\n";

   s +=  tr( "<h1>Time - Derivative (dC/dt) Analysis</h1>\n" )   +
         tr( "<h2>Data Report for Run \"" ) + d->runID + tr( "\", Cell " ) + d->cell +
         tr(  ", Wavelength " ) + d->wavelength + "</h2>\n";

   s += run_details();
   s += hydrodynamics();
   s += analysis( "" );
   s += scan_info();
   s += "</body></html>\n";

   te_results->e->setHtml( s );
   te_results->show();
}

void US_Dcdt::save( void )
{
   int                    index  = lw_triples->currentRow();
   US_DataIO2::EditedData* d      = &dataList[ index ];
   QString                dir    = US_Settings::reportDir();

   if ( ! mkdir( dir, d->runID ) ) return;

   // Note: d->runID is both directory and first segment of file name
   QString filebase = dir + "/" + d->runID + "/" + d->runID + "." + d->cell + 
       + "." + d->channel + "." + d->wavelength;
   
   QString plot1File = ".dcdt_analysis.svg";
   QString plot2File = ".dcdt_analysis_avg.svg";
   QString plot3File = ".dcdt_velocity.svg";
   QString textFile  = ".dcdt_avg_g_s.txt";
   QString htmlFile  = ".dcdt_report.html";

   // Write plots
   // Save current plot type
   int saveGraph = graphType;
   // Set current plot type to Scans
   graphType = 0;
   data_plot();
   write_plot( filebase + plot1File, data_plot1 );

   // Set current plot type to svalues
   graphType = 2;
   data_plot();
   write_plot( filebase + plot2File, data_plot1 );
      
   // Restore plot type
   graphType = saveGraph;
   data_plot();
   write_plot( filebase + plot3File, data_plot2 );

   // Write dcdt analysis average data
   QFile dcdt_data( filebase + textFile );
   
   if ( ! dcdt_data.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
   {
      QMessageBox::warning( this,
            tr( "IO Error" ),
            tr( "Could not open\n" ) + filebase + textFile + "\n" +
                tr( "\nfor writing" ) );
      return;
   }

   QTextStream ts_data( &dcdt_data );

   for ( int i = 0; i < arrayLength; i++ )
      ts_data << avgS   [ i ] << "\t" << avgDcdt[ i ] << endl;

   dcdt_data.close();

   // Write report
   QFile report( filebase + htmlFile );

   if ( ! report.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
   {
      QMessageBox::warning( this,
            tr( "IO Error" ),
            tr( "Could not open\n" ) + htmlFile + "\n" +
                tr( "\nfor writing" ) );
      return;
   }

   // Remove directory from string
   filebase = d->runID + "." + d->cell + "." + d->channel + "." + d->wavelength;

   QTextStream report_ts( &report );

   report_ts << 
         "<html><head>\n"
         "<style>td { padding-right: 1em;}</style>\n"
         "</head><body>\n";

   report_ts << run_details();
   report_ts << hydrodynamics();
   report_ts << analysis( "" );
   report_ts << scan_info();
   
   report_ts << 
         tr( "<h1>Time - Derivative (dC/dt) Analysis</h1>\n" )   +
         tr( "<h2>Data Report for Run \"" ) + d->runID + tr( "\", Cell " ) + 
         d->cell + tr(  ", Wavelength " ) + d->wavelength + "</h2>\n";

   report_ts << 
         "<h3><a href='" + filebase + textFile + "'>" 
         "ASCII File of Time Derivative Plot Data (Average g(S))</a></h3>\n";

   report_ts <<
         "<div><h3>Time Derivative Analysis Plot</h3>\n"
         "<object data='" + filebase + plot1File + "' type='image/svg+xml' "
         "width='"  + QString::number( data_plot1->size().width()  * 1.4 ) + "' "
         "height='" + QString::number( data_plot1->size().height() * 1.4 ) + 
         "'></object></div>\n";
   
   report_ts <<      
         "<div><h3>Time Derivative Analysis Plot (Average g(S))</h3>\n"
         "<object data='" + filebase + plot2File + "' type='image/svg+xml' "
         "width='"  + QString::number( data_plot1->size().width()  * 1.4 ) + "' "
         "height='" + QString::number( data_plot1->size().height() * 1.4 ) + 
         "'></object></div>\n";

   report_ts <<      
         "<div><h3>Velocity Plot</h3>\n"
         "<object data='" + filebase + plot3File + "' type='image/svg+xml' "
         "width='"  + QString::number( data_plot2->size().width()  * 1.4 ) + "' "
         "height='" + QString::number( data_plot2->size().height() * 1.4 ) + 
         "'></object></div>\n"

         "</body></html>\n";

   report.close();

   // Tell user
   QMessageBox::warning( this,
         tr( "Success" ),
         tr( "Wrote:\n" ) 
         + filebase + htmlFile  + "\n" 
         + filebase + plot1File + "\n" 
         + filebase + plot2File + "\n" 
         + filebase + plot3File + "\n" 
         + filebase + textFile );
}

