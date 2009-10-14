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

US_Dcdt::US_Dcdt() : US_AnalysisBase()
{
   setWindowTitle( tr( "Time Derivative - dC/dt Analysis" ) );

   te_results = NULL;
   dcdt       = NULL;
   sValues    = NULL;
   avgS       = NULL;
   avgDcdt    = NULL;

   sMax       = 20.0;

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

   int row = 7;

   controlsLayout->addWidget( lb_aux,     row++, 0, 1, 4 );

   controlsLayout->addWidget( lb_sValue,  row,   0, 1, 2 );
   controlsLayout->addWidget( ct_sValue,  row++, 2, 1, 2 );
   
   controlsLayout->addWidget( lb_graph,   row++, 0, 1, 4 );
   controlsLayout->addLayout( rb_layout0, row++, 0, 1, 4 );


   connect( pb_help,  SIGNAL( clicked() ), SLOT( help() ) );
   connect( pb_view,  SIGNAL( clicked() ), SLOT( view() ) );
   connect( pb_save,  SIGNAL( clicked() ), SLOT( save() ) );
}

void US_Dcdt::reset( void )
{
   US_AnalysisBase::reset();
   ct_sValue->setValue( sMax );
   rb_radius->click();
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
   US_AnalysisBase::data_plot();

   int                    index  = lw_triples->currentRow();
   US_DataIO::editedData* d      = &dataList[ index ];

   int     scanCount   = d->scanData.size();
   int     exclude     = 0;
   double  positionPct = ct_boundaryPos->value() / 100.0;
   double  baseline    = calc_baseline();

   for ( int i = 0; i < scanCount; i++ )
   {
      if ( excludedScans.contains( i ) ) continue;
      
      double range  = d->scanData[ i ].plateau - baseline;
      double test_y = baseline + range * positionPct;
      
      if ( d->scanData[ i ].readings[ 0 ].value > test_y ) exclude++;
   }

   le_skipped->setText( QString::number( exclude ) );

   if ( scanCount - exclude - excludedScans.count() < 4 )
   {
      QMessageBox::warning( this,
            tr( "Attention" ),
            tr( "You must have at least four non-excluded scans "
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
   }
   
   int points = d->scanData[ 0 ].readings.size();

   // Create the new arrays
   dcdt    = new double* [ scanCount ];
   sValues = new double* [ scanCount ];
   avgDcdt = new double  [ points ];
   avgS    = new double  [ points ];

   for ( int i = 0; i < scanCount; i++ )
   {
      dcdt   [ i ] = new double [ points ];
      sValues[ i ] = new double [ points ];
   }

   // Calculate dcdt and sValues

   int previous = exclude;
   int count    = 0;

   for ( int i = exclude + 1; i < scanCount; i++ )
   {
      if ( excludedScans.contains( i ) ) continue;

      double dt = d->scanData[ i ].seconds - d->scanData[ previous ].seconds;
      
      double plateau     = d->scanData[ i        ].plateau;
      double prevPlateau = d->scanData[ previous ].plateau;

      double meniscus    = d->meniscus;
      double omega       = d->scanData[ i        ].rpm * M_PI / 30.0;

      for ( int j = 0; j < points; j++ )
      {
         double dC = d->scanData[ i        ].readings[ j ].value / plateau -
                     d->scanData[ previous ].readings[ j ].value / prevPlateau;

         // We are really plotting g*(s) so -dC/dt is saved for the plots
         dcdt[ count ][ j ] = -dC / dt;
      
         double radius = d->scanData[ i ].readings[ j ].d.radius;

         sValues[ count ][ j ] = 
            solution.correction * 1.0e13 * log( radius / meniscus ) /
            ( sq( omega ) * ( d->scanData[ previous ].seconds + dt / 2.0 ) );
      }

      count++;
      previous = i;
   }

   // Calculate average dcdt for plotting or writing

   // Find the min and max g*(s) of the first scan
   double gs_max = 0.0;
   double gs_min = 9.9e10;
   
   for ( int i = 0; i < points; i++ )
   {
      gs_max = max( gs_max, dcdt[ 0 ][ i ] );
      gs_min = min( gs_min, dcdt[ 0 ][ i ] );
   }

   // If the value of the y-value is larger than 1% of the total range,
   // then we want to pick the x-value (s-value) at that point and
   // consider it the maximum s-value.  This procedure will only be
   // necessary for the first scan, since the cutoff criterion is 1 % of
   // total.

   double range = ( gs_max - gs_min ) / 100.0;
   double s_max = 0.0;

   for ( int i = points - 1; i > -1; i-- )
   { 
      if ( dcdt[ 0 ][ i ] < range )
      {
         s_max = sValues[ 0 ][ i ];
         break;
      }
   }

   if ( s_max < sMax )
   {
      ct_sValue->setMaxValue( s_max );
      sMax = s_max;
   }

   // Assign new equally spaced s-values for the x-axis
   for ( int i = 0; i < points; i++ ) 
      avgS[ i ] = i * s_max / ( points - 1 );

   // For each new s value, find the corresponding g*(s) through
   // linear interpolation and add up, then average by the number of
   // total scans included.

   for ( int j = 0; j < points; j++ ) 
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
            if ( k == points ) goto next; // Skip rest of scans
         }

         // Interpolate and apply y = mx + b
         double m = ( dcdt   [ i ][ k ] - dcdt   [ i ][ k - 1 ] ) /
                    ( sValues[ i ][ k ] - sValues[ i ][ k - 1 ] );
         
         double b = dcdt[ i ][ k ] - m * sValues[ i ][ k ];
         
         avgDcdt[ j ] += m * avgS[ j ] + b;
      }
      next:
      avgDcdt[ j ] /= count; 
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

         for ( int i = 0; i < points; i++ ) 
            x[ i ] = d->scanData[ 0 ].readings[ i ].d.radius;

         for ( int i = 0; i < count; i++ )
         {
            curve = us_curve( data_plot1, 
                  tr( "Scan " ) + QString::number( i + exclude + 1 ) );

            curve->setData( x, dcdt[ i ], points );
         }

         data_plot1->setAxisAutoScale( QwtPlot::xBottom );

         break;

      case 1:  // S-value plot
         for ( int i = 0; i < count; i++ )
         {
            curve = us_curve( data_plot1, 
                  tr( "Scan " ) + QString::number( i + exclude + 1 ) );

            curve->setData( sValues[ i ], dcdt[ i ], points );
         }
         
         data_plot1->setAxisScale( QwtPlot::xBottom, 0.0, ct_sValue->value() );

         break;

      case 2:  // Average dC/dt plot
         data_plot1->setAxisTitle( QwtPlot::yLeft, 
                                   tr( "Average g<sup>*</sup>(s)" ) );

         curve = us_curve( data_plot1, tr( "Average g*(s)" ) );
         curve->setData( avgS, avgDcdt, points );

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
   US_DataIO::editedData* d      = &dataList[ index ];

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
   US_DataIO::editedData* d      = &dataList[ index ];
   QString                dir    = US_Settings::reportDir();

   if ( ! mkdir( dir, d->runID ) ) return;

   // Note: d->runID is both directory and first segment of file name
   QString filebase = dir + "/" + d->runID + "/" + d->runID + "." + d->cell + 
       + "." + d->channel + "." + d->wavelength;
   
   QString plot1File = ".dcdt_analysis.svg";
   QString plot2File = ".dcdt_analysis_avg.svg";
   QString plot3File = ".dcdt_velocity.svg";
   QString text1File = ".dcdt_scandata.txt";
   QString text2File = ".dcdt_svalues.txt";
   QString text3File = ".dcdt_avg_g_s.txt";
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

   int points    = d->scanData[ 0 ].readings.size();
   int scanCount = d->scanData.size();

   // Write dcdt analysis data
   QFile dcdt1_data( filebase + text1File );
   
   if ( ! dcdt1_data.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
   {
      QMessageBox::warning( this,
            tr( "IO Error" ),
            tr( "Could not open\n" ) + filebase + text1File + "\n" +
                tr( "\nfor writing" ) );
      return;
   }

   QTextStream ts_data1( &dcdt1_data );

   for ( int i = 0; i < points; i++ )
   {
      ts_data1 <<  d->scanData[ 0 ].readings[ i ].d.radius << "\t";
      
      for ( int j = 0; j < scanCount - 1; j++ )
         ts_data1 << dcdt[ j ][ i ] << "\t";

      ts_data1 << dcdt[ scanCount - 1 ][ i ] << endl;
   }

   dcdt1_data.close();

   // Write dcdt analysis s-value data
   QFile dcdt2_data( filebase + text2File );
   
   if ( ! dcdt2_data.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
   {
      QMessageBox::warning( this,
            tr( "IO Error" ),
            tr( "Could not open\n" ) + filebase + text2File + "\n" +
                tr( "\nfor writing" ) );
      return;
   }

   QTextStream ts_data2( &dcdt2_data );

   for ( int i = 0; i < points; i++ )
   {
      for ( int j = 0; j < scanCount - 1; j++ )
         ts_data2 << sValues[ j ][ i ] << "\t" 
                  << dcdt   [ j ][ i ] << "\t";

      ts_data2 << sValues[ scanCount - 1 ][ i ] << "\t" 
               << dcdt   [ scanCount - 1 ][ i ] << endl;
   }

   dcdt2_data.close();

   // Write dcdt analysis average data
   QFile dcdt3_data( filebase + text3File );
   
   if ( ! dcdt3_data.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
   {
      QMessageBox::warning( this,
            tr( "IO Error" ),
            tr( "Could not open\n" ) + filebase + text3File + "\n" +
                tr( "\nfor writing" ) );
      return;
   }

   QTextStream ts_data3( &dcdt3_data );

   for ( int i = 0; i < points; i++ )
      ts_data3 << avgS   [ i ] << "\t" << avgDcdt[ i ] << endl;

   dcdt3_data.close();

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
         "<h3><a href='" + filebase + text1File + "'>" 
         "ASCII File of Time Derivative Plot Data (Scans)</a></h3>\n";

   report_ts << 
         "<h3><a href='" + filebase + text2File + "'>" 
         "ASCII File of Time Derivative Plot Data (S-values)</a></h3>\n";

   report_ts << 
         "<h3><a href='" + filebase + text3File + "'>" 
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
         + filebase + text1File + "\n" 
         + filebase + text2File + "\n" 
         + filebase + text3File );
}

