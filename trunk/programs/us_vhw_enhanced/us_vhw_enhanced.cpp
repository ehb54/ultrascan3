//! \file us_vhw_enhanced.cpp

#include <QApplication>

#include <uuid/uuid.h>

#include "us_vhw_enhanced.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_matrix.h"

// main program
int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_vHW_Enhanced w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

// US_vHW_Enhanced class constructor
US_vHW_Enhanced::US_vHW_Enhanced() : US_AnalysisBase()
{
   // set up the GUI (mostly handled in US_AnalysisBase)

   setWindowTitle( tr( "Enhanced van Holde - Weischet Analysis:" ) );

   pb_dstrpl     = us_pushbutton( tr( "Distribution Plot" ) );
   pb_dstrpl->setEnabled( false );
   connect( pb_dstrpl, SIGNAL( clicked() ),
            this,       SLOT(  distr_plot() ) );

   pb_selegr     = us_pushbutton( tr( "Select Groups" ) );
   pb_selegr->setEnabled( true );
   connect( pb_selegr, SIGNAL( clicked() ),
            this,       SLOT(  sel_groups() ) );

   parameterLayout->addWidget( pb_dstrpl, 2, 0, 1, 2 );
   parameterLayout->addWidget( pb_selegr, 2, 2, 1, 2 );

   QLayoutItem* litem = parameterLayout->itemAtPosition( 1, 0 );
   QWidget*     witem = litem->widget();

   if ( witem )
   {  // change "vbar" to "Vbar" on pushbutton
      QPushButton* bitem = (QPushButton*)witem;
      bitem->setText( "Vbar" );
   }
   else
      qDebug() << "parLay 1,0 item is NOT widget";

   QLabel* lb_analysis     = us_banner( tr( "Analysis Controls"      ) );
   QLabel* lb_scan         = us_banner( tr( "Scan Control"           ) );
   QLabel* lb_smoothing    = us_label ( tr( "Data Smoothing:"        ) );
   QLabel* lb_boundPercent = us_label ( tr( "% of Boundary:"         ) );
   QLabel* lb_boundPos     = us_label ( tr( "Boundary Position (%):" ) );

   QLabel* lb_from         = us_label ( tr( "From:" ) );
   QLabel* lb_to           = us_label ( tr( "to:"   ) );

   lb_tolerance  = us_label( tr( "Back Diffusion Tolerance:" ) );
   lb_tolerance->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_tolerance = us_counter( 3, 0.0, 1000.0, 0.001 );
   ct_tolerance->setStep( 0.001 );
   ct_tolerance->setEnabled( true );
   connect( ct_tolerance, SIGNAL( valueChanged(   double ) ),
            this,          SLOT(  update_bdtoler( double ) ) );

   lb_division   = us_label( tr( "Divisions:" ) );
   lb_division->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_division  = us_counter( 3, 0.0, 1000.0, 50.0 );
   ct_division->setStep( 1 );
   ct_division->setEnabled( true );
   connect( ct_division, SIGNAL( valueChanged(  double ) ),
            this,         SLOT(  update_divis(  double ) ) );

   controlsLayout->addWidget( lb_analysis       , 0, 0, 1, 4 );
   controlsLayout->addWidget( lb_tolerance      , 1, 0, 1, 2 );
   controlsLayout->addWidget( ct_tolerance      , 1, 2, 1, 2 );
   controlsLayout->addWidget( lb_division       , 2, 0, 1, 2 );
   controlsLayout->addWidget( ct_division       , 2, 2, 1, 2 );
   controlsLayout->addWidget( lb_smoothing      , 3, 0, 1, 2 );
   controlsLayout->addWidget( ct_smoothing      , 3, 2, 1, 2 );
   controlsLayout->addWidget( lb_boundPercent   , 4, 0, 1, 2 );
   controlsLayout->addWidget( ct_boundaryPercent, 4, 2, 1, 2 );
   controlsLayout->addWidget( lb_boundPos       , 5, 0, 1, 2 );
   controlsLayout->addWidget( ct_boundaryPos    , 5, 2, 1, 2 );
   controlsLayout->addWidget( lb_scan           , 6, 0, 1, 4 );
   controlsLayout->addWidget( lb_from           , 7, 0 );
   controlsLayout->addWidget( ct_from           , 7, 1 );
   controlsLayout->addWidget( lb_to             , 7, 2 );
   controlsLayout->addWidget( ct_to             , 7, 3 );
   controlsLayout->addWidget( pb_exclude        , 8, 0, 1, 4 );

   connect( pb_help, SIGNAL( clicked() ),
            this,    SLOT(   help() ) );
   dataLoaded = false;

}

// load data
void US_vHW_Enhanced::load( void )
{
   dataLoaded = false;
   // query the directory where .auc and .xml file are
   workingDir = QFileDialog::getExistingDirectory( this,
         tr( "Raw Data Directory" ),
         US_Settings::resultDir(),
         QFileDialog::DontResolveSymlinks );

   if ( workingDir.isEmpty() )
      return;

   //reset_data();

   // insure we have a .auc file
   QStringList nameFilters = QStringList( "*.auc" );
   workingDir.replace( "\\", "/" );
   QDir wdir( workingDir );
   files    = wdir.entryList( nameFilters,
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
   lw_triples->clear();
   dataList.clear();
   rawList.clear();
   excludedScans.clear();
   triples.clear();

   // Read all data
   if ( workingDir.right( 1 ) != "/" )
      workingDir += "/"; // Ensure trailing '/'

   files       = wdir.entryList( QStringList() << "*.*.*.*.*.*.xml",
         QDir::Files | QDir::Readable, QDir::Name );

   for ( int ii = 0; ii < files.size(); ii++ )
   {  // load all data in directory; get triples
      QString file     = files[ ii ];
      QStringList part = file.split( "." );
      QString filename = workingDir + file;
 
      // load edit data (xml) and raw data (auc)
      int result = US_DataIO::loadData( workingDir, file, dataList, rawList );

      if ( result != US_DataIO::OK )
      {
         QMessageBox::warning( this,
            tr( "UltraScan Error" ),
            tr( "Could not read edit file.\n" ) 
            + US_DataIO::errorString( result ) + "\n" + filename );
         return;
      }

      QString t = part[ 3 ] + " / " + part[ 4 ] + " / " + part[ 5 ];
      runID     = part[ 0 ];
      editID    = part[ 1 ];

      if ( ! triples.contains( t ) )
      {  // update ListWidget with cell / channel / wavelength triple
         triples << t;
         lw_triples->addItem( t );
      } 
   }

   lw_triples->setCurrentRow( 0 );

   US_DataIO::editedData* d = &dataList[ 0 ];
   le_id->setText( d->runID + " / " + d->editID );  // set ID text

   double tempera = 0.0;
   int    scnknt  = d->scanData.size();
   savedValues.clear();

   for ( int ii = 0; ii < scnknt; ii++ )
   {
      US_DataIO::scan* s = &d->scanData[ ii ];

      // sum temperature from each scan to determine average
      tempera += s->temperature;

      // save the data
      int npts = s->readings.size();
      QVector< double > v;
      v.resize( npts );

      for ( int jj = 0; jj < npts; jj++ )
      {
         v[ jj ] = s->readings[ jj ].value;
      }

      savedValues << v;
   }

   // display average temperature and data description
   tempera  /= (double)scnknt;
   QString t = QString::number( tempera, 'f', 1 ) + " " + QChar( 176 ) + "C";
   le_temp->setText( t );                            // set avg temp text
   te_desc->setText( d->description );               // set description text

   // Enable pushbuttons
   pb_details   ->setEnabled( true );
   pb_save      ->setEnabled( true );
   pb_view      ->setEnabled( true );
   pb_exclude   ->setEnabled( true );

   data_plot1->setCanvasBackground( Qt::black );
   data_plot2->setCanvasBackground( Qt::black );
   data_plot1->setMinimumSize( 600, 500 );
   data_plot2->setMinimumSize( 600, 300 );

   update( 0 );

   //data_plot();                                      // plot data

   pb_details->disconnect( );                        // reset details connect
   connect( pb_details, SIGNAL( clicked() ),
            this,       SLOT(  details() ) );
   dataLoaded = true;
}

// details
void US_vHW_Enhanced::details( void )
{
   US_RunDetails* dialog
      = new US_RunDetails( rawList, runID, workingDir, triples );
   dialog->exec();
   qApp->processEvents();
   delete dialog;
}

// distribution plot
void US_vHW_Enhanced::distr_plot(  void )
{
}

// data plot
void US_vHW_Enhanced::data_plot( void )
{
   // let AnalysisBase do the lower plot
   US_AnalysisBase::data_plot();

   // handle upper (vHW Extrapolation) plot, here
   int                    row = lw_triples->currentRow();
   US_DataIO::editedData* d   = &dataList[ row ];

   int     scanCount   = d->scanData.size();
   int     exclude     = 0;
   double  boundaryPct = ct_boundaryPercent->value() / 100.0;
   double  positionPct = ct_boundaryPos    ->value() / 100.0;
   double  baseline    = calc_baseline();
   double  meniscus    = d->meniscus;
   double  xmax        = 0.0;
   double  ymax        = 0.0;
   ndivis              = qRound( ct_division->value() );

   for ( int jj = 0; jj < scanCount; jj++ )
   {  // count the scans excluded due to position percent
      if ( excludedScans.contains( jj ) ) continue;
      
      double plateau = calc_plateau( &d->scanData[ jj ] );
      double range   = plateau - baseline;
      double test_y  = baseline + range * positionPct;
      
      if ( d->scanData[ jj ].readings[ 0 ].value > test_y ) exclude++;
   }

   le_skipped->setText( QString::number( exclude ) );

   // Draw plot
   data_plot1->clear();
   us_grid( data_plot1 );

   data_plot1->setTitle( tr( "Run " ) + d->runID + tr( ": Cell " ) + d->cell
             + " (" + d->wavelength + tr( " nm) - vHW Extrapolation Plot" ) );

   data_plot1->setAxisTitle( QwtPlot::xBottom, tr( "(Time)^-0.5" ) );
   data_plot1->setAxisTitle( QwtPlot::yLeft  , 
         tr( "Corrected Sed. Coeff. (1e-13 s)" ) );

   int     ndpts     = ndivis + 1;
   int     ntpts     = scanCount * ndpts;
   double* ptx       = new double[ scanCount ];
   double* pty       = new double[ ntpts ];
   int     kk        = 0;

   // Calculate the corrected sedimentation coefficients
   for ( int ii = 0; ii < scanCount; ii++ )
   {
      US_DataIO::scan* s = &d->scanData[ ii ];

      double timev   = 1.0 / sqrt( s->seconds - time_correction );

      if ( excludedScans.contains( ii ) )
         timev          = -1.0;       // mark excluded scan

      ptx[ ii ]      = timev;         // save corrected time and accum max
      xmax           = ( xmax > timev ) ? xmax : timev;

      double plateau = calc_plateau( s );
      double range   = plateau - baseline;
      double span    = range * boundaryPct;
      double spinc   = span / double( ndivis + 1 );
      double spanv0  = baseline + range * positionPct;
      double omega   = s->rpm * M_PI / 30.0;
      double omegsq  = omega * omega;
      double correc  = solution.correction * 1.0e13;

      if ( timev > 0.0 )              // get time and omega-squared term
         timev          = ( s->seconds - time_correction ) * omegsq;

      // The span is the boundary portion that is going to be analyzed (in
      // percent)

      int    points  = s->readings.size();
      double absorbv = spanv0;        // initial absorbance for span

      for ( int jj = 0; jj < ndpts; jj++ )
      {  // walk through division points; get index to place in readings
         int j2      = first_gteq( absorbv, s->readings, points );
         double rv0  = -1.0;          // mark radius excluded

         if ( j2 >= 0  &&  timev >= 0.0 )
         {  // likely need to interpolate radius from two input values
            int j1      = j2 - 1;

            if ( j2 > 0 )
            {  // interpolate radius value
               double av1  = s->readings[ j1 ].value;
               double av2  = s->readings[ j2 ].value;
               double rv1  = s->readings[ j1 ].d.radius;
               double rv2  = s->readings[ j2 ].d.radius;
               double dav  = av2 - av1;
               double drv  = rv2 - rv1;
               if ( dav != 0.0 )
                  rv0         = rv1 + ( ( absorbv - av1 ) * drv / dav );
               else
                  rv0         = rv1;
            }

            else
            {
               rv0         = -1.0;
            }
         }

         if ( rv0 > 0.0 )
         {  // use radius and other terms to get corrected sed. coeff. value
            rv0         = correc * log( rv0 / meniscus ) / timev;
if(rv0>6.5) {
qDebug() << " rv0=" << rv0 << "   scan " << ii << "   div " << jj;
}
         }
         pty[ kk++ ] = rv0;           // save sed. coeff. value
         ymax        = ( ymax > rv0 ) ? ymax : rv0;
         absorbv    += spinc;         // bump absorbance to next division point
      }
   }

   int nxy    = ( scanCount > ndpts ) ? scanCount : ndpts;
   int count  = 0;
   double* x  = new double[ nxy ];
   double* y  = new double[ nxy ];
   
   QwtPlotCurve* curve;
   QwtSymbol     sym;
   sym.setStyle( QwtSymbol::Ellipse );
   sym.setPen  ( QPen( Qt::blue ) );
   sym.setBrush( QBrush( Qt::white ) );
   sym.setSize ( 8 );
   
   kk         = 0;                    // index to sed. coeff. values

   // Set points for each division of each scan
   for ( int jj = 0; jj < scanCount; jj++ )
   {
      if ( excludedScans.contains( jj ) )
      {
         kk        += ndpts;          // excluded:  bump to next scan
         continue;
      }
      
      count      = 0;
      double xv  = ptx[ jj ];         // reciprocal square root of time value

      for ( int ii = 0; ii < ndpts; ii++ )
      {
         double yv  = pty[ kk++ ];    // sed.coeff. values for divs in scan
         if ( xv >= 0.0  &&  yv >= 0.0 )
         {  // points in a scan
            x[ count ] = xv;
            y[ count ] = yv;
            count++;
         }
      }

      if ( count > 0 )
      {  // plot the points in a scan
         curve = us_curve( data_plot1,
               tr( "Sed Coeff Points, scan %1" ).arg( jj+1 ) );

         curve->setStyle ( QwtPlotCurve::NoCurve );
         curve->setSymbol( sym );
         curve->setData  ( x, y, count );
      }
   }

   // fit lines for each division to all scan points

   for ( int ii = 0; ii < ndpts; ii++ )
   {  // walk thru divisions, fitting line to points from all scans
      count          = 0;
      for ( int jj = 0; jj < scanCount; jj++ )
      {
         if ( excludedScans.contains( jj ) ) continue;

         kk         = jj * ndpts + ii;   // sed. coeff. index

         if ( ptx[ jj ] > 0.0  &&  pty[ kk ] > 0.0 )
         {  // points for scans in a division
            x[ count ] = ptx[ jj ];
            y[ count ] = pty[ kk ];
            count++;
         }
      }

      if ( count > 0 )
      {  // fit a line to the scan points in a division
         double slope;
         double intcept;
         double sigma = 0.0;
         double correl;

         US_Math::linefit( &x, &y, &slope, &intcept, &sigma, &correl, count );

         x[ 0 ] = 0.0;                      // x from 0.0 to max
         x[ 1 ] = xmax + 0.001;
         y[ 0 ] = intcept;                  // y from intercept to y at x-max
         y[ 1 ] = y[ 0 ] + x[ 1 ] * slope;

         curve  = us_curve( data_plot1, tr( "Fitted Line %1" ).arg( ii ) );
         curve->setPen( QPen( Qt::yellow ) );
         curve->setData( x, y, 2 );
      }
   }

   // set scales, then plot the points and lines
   xmax  *= 1.05;
   xmax   = (double)qRound( ( xmax + 0.0015 ) / 0.002 ) * 0.002;
   ymax   = (double)qRound( ( ymax + 0.25 ) / 0.4 ) * 0.4;
   data_plot1->setAxisScale( QwtPlot::yLeft,   0.0, ymax, 0.5 );
   data_plot1->setAxisScale( QwtPlot::xBottom, 0.0, xmax, 0.005 );
   data_plot1->replot();

   delete [] x;                             // clean up
   delete [] y;
   delete [] ptx;
   delete [] pty;
}

// save the enhanced data
void US_vHW_Enhanced::save_data( void )
{ 
   QString filter = tr( "vHW data files (*.vHW.dat);;" )
      + tr( "Any files (*)" );
   QString fsufx = "." + cell + wavelength + ".vHW.dat";
   QString fname = run_name + fsufx;
   fname         = QFileDialog::getSaveFileName( this,
      tr( "Save vHW Data File" ),
      US_Settings::resultDir() + "/" + fname,
      filter,
      0, 0 );

}

void US_vHW_Enhanced::print_data(  void ) {}
void US_vHW_Enhanced::view_report( void ) {}
void US_vHW_Enhanced::show_densi(  void ) {}
void US_vHW_Enhanced::show_visco(  void ) {}

void US_vHW_Enhanced::show_vbar(   void )
{
   //this->get_vbar();
}

// reset the GUI
void US_vHW_Enhanced::reset_data( void )
{
   update( lw_triples->currentRow() );
#if 0
   data_plot1->detachItems( );
   data_plot1->replot();
   epick      = new US_PlotPicker( data_plot1 );
#endif
}

void US_vHW_Enhanced::sel_groups(  void ) {}
void US_vHW_Enhanced::excl_sscan(  void ) {}
void US_vHW_Enhanced::excl_srange( void ) {}

// update density
void US_vHW_Enhanced::update_density( double dval )
{
   densit    = dval;
}

// update viscosity
void US_vHW_Enhanced::update_viscosi( double dval )
{
   viscos    = dval;
}

void US_vHW_Enhanced::update_vbar(    double ) {}

void US_vHW_Enhanced::update_bdtoler( double dval )
{
   bdtoler   = dval;
}

void US_vHW_Enhanced::update_divis(   double dval )
{ 
   ndivis    = qRound( dval );

   data_plot();
}
void US_vHW_Enhanced::update_dsmoo(   double ) {}
void US_vHW_Enhanced::update_pcbound( double ) {}
void US_vHW_Enhanced::update_boundpo( double ) {}
void US_vHW_Enhanced::update_exsscan( double ) {}
void US_vHW_Enhanced::update_exscrng( double ) {}

//  find first absorbance value in array greater than or equal to a given value
int US_vHW_Enhanced::first_gteq( double absorbv,
      QVector< US_DataIO::reading >& readings, int points )
{
   int index = -1;

   for ( int jj = 0; jj < points; jj++ )
   {
      if ( absorbv < readings[ jj ].value )
      {
         index     = ( jj > 0 ) ? jj : -1;
         break;
      }

      else if ( absorbv == readings[ jj ].value )
      {
         index     = ( jj > 0 ) ? jj : 1;
         break;
      }
   }
   return index;
}

//  get average scan plateau value for 11 points around input value
double US_vHW_Enhanced::calc_plateau( US_DataIO::scan* s )
{
   double plato  = s->plateau;
   int    points = s->readings.size();
//qDebug() << "calc_plateau in: " << plato << "   points " << points;
//qDebug() << " rd0 rdn " << s->readings[0].value << s->readings[points-1].value;
   int    j2     = first_gteq( plato, s->readings, points );

   if ( j2 > 0 )
   {
      int    j1     = j2 - 7;
      j2           += 4;
      j1            = ( j1 > 0 )      ? j1 : 0;
      j2            = ( j2 < points ) ? j2 : points;
      plato         = 0.0;
      for ( int jj = j1; jj < j2; jj++ )
      {  // walk through division points; get index to place in readings
         plato     += s->readings[ jj ].value;
      }
      plato        /= (double)( j2 - j1 );
//qDebug() << "     plateau out: " << plato << "    j1 j2 " << j1 << j2;
   }
   return plato;
}

