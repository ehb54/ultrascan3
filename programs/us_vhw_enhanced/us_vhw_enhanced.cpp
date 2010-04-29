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

   EDTDAT* d       = &dataList[ 0 ];
   le_id->setText( d->runID + " / " + d->editID );  // set ID text

   double  tempera = 0.0;
   int     scnknt  = d->scanData.size();
   savedValues.clear();

   for ( int ii = 0; ii < scnknt; ii++ )
   {
      SCNDAT* s = &d->scanData[ ii ];

      // sum temperature from each scan to determine average
      tempera  += s->temperature;

      // save the data
      int npts  = s->readings.size();
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

   pb_details->disconnect( );                        // reset details connect
   connect( pb_details, SIGNAL( clicked() ),
            this,       SLOT(   details() ) );
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
   int     row         = lw_triples->currentRow();
   EDTDAT* d           = &dataList[ row ];

   nscns               = d->scanData.size();
   ndivs               = qRound( ct_division->value() );
   divfac              = 1.0 / (double)ndivs;
   exclude             = 0;
   double  boundPct    = ct_boundaryPercent->value() / 100.0;
   double  positPct    = ct_boundaryPos    ->value() / 100.0;
   double  baseline    = calc_baseline();
   double  meniscus    = d->meniscus;
   double  correc      = solution.correction * 1.0e13;
   double  xmax        = 0.0;
   double  ymax        = 0.0;
	double  C0          = 0.0;
	double  Swavg       = 0.0;
   double  omega       = d->scanData[0].rpm * M_PI / 30.0;

   for ( int ii = 0; ii < nscns; ii++ )
   {  // count the scans excluded due to position percent
      if ( excludedScans.contains( ii ) ) continue;
      
      double plateau = avg_plateau( &d->scanData[ ii ] );
      double range   = plateau - baseline;
      double test_y  = baseline + range * positPct;
      
      if ( d->scanData[ ii ].readings[ 0 ].value > test_y ) exclude++;
   }

   le_skipped->setText( QString::number( exclude ) );

   // Do first experimental plateau calcs based on horizontal zones
   int     nrelp       = 0;
   int     nunrp       = 0;
   QList< double > plats;
   QList< int >    isrel;
   QList< int >    isunr;

   for ( int ii = exclude; ii < nscns; ii++ )
   {
      SCNDAT* s          = &d->scanData[ ii ];
qDebug() << "p: scan " << ii+1;
      double  plateau    = zone_plateau( s, baseline );

      if ( plateau > 0.0 )
      {
         plats.append( plateau );
         isrel.append( ii );
         nrelp++;
qDebug() << "p:    *RELIABLE* " << ii+1 << nrelp;
      }
      else
      {
         isunr.append( ii );
         nunrp++;
qDebug() << "p:    -UNreliable- " << ii+1 << nunrp;
      }
   }
qDebug() << "p: nrelp nunrp " << nrelp << nunrp;
qDebug() << "  RELIABLE: 1st " << isrel.at(0)+1 << "  last " << isrel.last()+1;
qDebug() << "  UNreli: 1st " << isunr.at(0)+1 << "  last " << isunr.last()+1;
for (int jj=0;jj<isunr.size();jj++) qDebug() << "    UNr: " << isunr.at(jj)+1;

   int     ntpts     = nscns * ndivs;
   double* ptx       = new double[ nscns ];
   double* pty       = new double[ ntpts ];

   // Find Swavg and C0 by line fit
   // Solve for slope "a" and intercept "b" in
   // set of equations: y = ax + b 
   //   where x is corrected time
   //   and   y is log of plateau concentration
   // log( Cp ) = (-2 * Swavg * omega-sq ) t + log( C0 );
   //   for scans with reliable plateau values

   for ( int jj = 0; jj < nrelp; jj++ )
   {
      int ii     = isrel.at( jj );
      ptx[ jj ]  = d->scanData[ ii ].seconds - time_correction;
      pty[ jj ]  = log( plats.at( jj ) );
   }

   double slope;
   double intcp;
   double sigma;
   double corre;

   US_Math::linefit( &ptx, &pty, &slope, &intcp, &sigma, &corre, nrelp );
	Swavg = slope/(-2.0*omega*omega);
	C0    = exp(intcp);

	qDebug() << "Swavg: " << Swavg << ", C0: " << C0 ;
   // Determine Cp for each of the unreliable scans
   // y = ax + b, using "a" and "b" determined above.
   // Since y = log( Cp ), we get Cp by exponentiating
   // the right-hand term.

   for ( int jj = 0; jj < nunrp; jj++ )
   {
      int ii     = isunr.at( jj );
      double tc  = d->scanData[ ii ].seconds - time_correction;

      d->scanData[ ii ].plateau = exp( tc * slope + intcp );

qDebug() << " jj scan plateau " << jj << ii+1 << d->scanData[ii].plateau;
   }

   QList< double >           scpds;
   QList< double >           tcpds;

   // initialize plateau values for components of scans

   for ( int ii = 0; ii < nscns; ii++ )
   {
      scpds.clear();
      if ( ii < exclude  ||  excludedScans.contains( ii ) )
      {
         cpds << scpds;
         sdifs.append( 0.0 );
         continue;
      }
      SCNDAT* s      = &d->scanData[ ii ];
      int     npts   = s->readings.size();
      double  plat   = s->plateau;
      double  span   = plat - baseline;
      double  bcut   = baseline + span * positPct;
      double  pcut   = bcut     + span * boundPct;
      span           = pcut - bcut;
      double  pconc  = bcut;
      double  cconc  = bcut;
      double  tcpjj  = 0.0;
      double  cinc   = span * divfac;
      //double  omega  = s->rpm * M_PI / 30.0;
      double  oterm  = ( s->seconds - time_correction ) * omega * omega;
      //double eterm  = -2.0 * oterm / 1.0e13;
      double  eterm  = -2.0 * oterm / correc;


      for ( int jj = 0; jj < ndivs; jj++ )
      {
         pconc        = cconc;
         cconc       += cinc;
         //double mconc = pconc + cinc * 0.5;

         double sedc  = sed_coeff( cconc, correc, meniscus, oterm,
               s->readings, npts );
         double cpjj  = (C0) * (1-boundPct)/ndivs * exp( sedc * eterm );
//qDebug() << " scn div cinc cpjj " << ii+1 << jj+1 << cinc << cpjj;
//qDebug() << "  sedc eterm eso " << sedc << eterm << (eterm*sedc);
         tcpjj       += cpjj;
         scpds.append( cpjj );
      }

      cpds << scpds;
      sdifs.append( span - tcpjj );
qDebug() << "    tcpjj span tsratio" << tcpjj << span << (tcpjj/span);
   }

   // iterate to adjust plateaus until none needed or max iters reached
   int     mxiter    = 3;
   int     iter      = 1;
   double  spdsmx    = (double)ndivs;

   while( iter <= mxiter )
   {
      double spdsum  = 0.0;
      double spdlo   = 1.0e20;
      double spdhi   = -1.0e20;
      int    count   = 0;
qDebug() << "iter mxiter " << iter << mxiter;

      // get division sed. coeff. values (intercepts)

      div_seds();

      // reset division plateaus

      for ( int ii = 0; ii < nscns; ii++ )
      {
         if ( ii < exclude  ||  excludedScans.contains( ii ) )
            continue;

         SCNDAT* s      = &d->scanData[ ii ];
         double  span   = s->plateau - baseline;
         double  bcut   = baseline + span * positPct;
         double  pcut   = bcut     + span * boundPct;
         span           = pcut - bcut;
         double  tcpjj  = 0.0;
         double  oterm  = ( s->seconds - time_correction ) * omega * omega;
         double  eterm  = -2.0e-13 * oterm;
         double  pddif  = sdifs.at( ii ) * divfac;
qDebug() << " scn pddif " << ii+1 << pddif;
         tcpds          = cpds.at( ii );
         scpds.clear();

         // split difference between divisions

         for ( int jj = 0; jj < ndivs; jj++ )
         {
            double  cinc   = tcpds.at( jj ) + pddif;
            double  sedc   = dseds[ jj ];
            double  cpjj   = (C0) * (1 - boundPct) / ndivs * exp( sedc * eterm );
            scpds.append( cpjj );
            tcpjj         += cpjj;
qDebug() << "    div " << jj+1 << "  inc1 inc2 inc3 "
   << tcpds.at(jj) << cinc << cpjj << "  term " << cpjj/cinc;
         }

         double  sdiff  = span - tcpjj;
         sdifs.replace( ii, sdiff );
         cpds.replace(  ii, scpds );
qDebug() << "   iter scn " << iter << ii+1 << " tcpjj span tsratio sdiff"
   << tcpjj << span << (tcpjj/span) << sdiff;
         spdsum        += sdiff;
         spdlo          = ( spdlo < sdiff ) ? spdlo : sdiff;
         spdhi          = ( spdhi > sdiff ) ? spdhi : sdiff;
         count++;
      }

      double  spdavg = spdsum / (double)count;
qDebug() << "      iter sum smx avg " << iter << spdsum << spdsmx << spdavg;
      iter++;

      //if ( spdsum < spdsmx )
      //   break;
   }

   int     kk        = 0;

   // Calculate the corrected sedimentation coefficients
   for ( int ii = exclude; ii < nscns; ii++ )
   {
      SCNDAT* s      = &d->scanData[ ii ];

      double  timev  = s->seconds - time_correction;
      double  timex  = 1.0 / sqrt( timev );

      if ( excludedScans.contains( ii ) )
         timex          = -1.0;       // mark excluded scan

      ptx[ ii ]      = timex;         // save corrected time and accum max
      xmax           = ( xmax > timex ) ? xmax : timex;

      double range   = s->plateau - baseline;
      double span    = range * boundPct;
      double spinc   = span * divfac;
      double spanv0  = baseline + range * positPct;
      double omega   = s->rpm * M_PI / 30.0;
      double oterm   = ( timev > 0.0 ) ? ( timev * omega * omega ) : -1.0;
//qDebug() << "   oterm eso " << oterm << exp(-2.*oterm*2.6);

      // The span is the boundary portion that is going to be analyzed (in
      // percent)

      int    points  = s->readings.size();
      double cconc   = spanv0;        // initial concentration for span
      double pconc;
      double mconc;
      scpds          = cpds.at( ii );
      double pddif   = sdifs.at( ii ) * divfac;

      for ( int jj = 0; jj < ndivs; jj++ )
      {  // walk through division points; get sed. coeff. by place in readings
         pconc       = cconc;
         spinc       = scpds.at( jj ) + pddif;
         cconc       = pconc + spinc;
         mconc       = pconc + spinc * 0.5;
         double sedc = sed_coeff( mconc, correc, meniscus, oterm,
               s->readings, points );
         pty[ kk++ ] = sedc;           // save sed. coeff. value
         ymax        = ( ymax > sedc ) ? ymax : sedc;
      }
   }

   // Draw plot
   data_plot1->clear();
   us_grid( data_plot1 );

   data_plot1->setTitle( tr( "Run " ) + d->runID + tr( ": Cell " ) + d->cell
             + " (" + d->wavelength + tr( " nm) - vHW Extrapolation Plot" ) );

   data_plot1->setAxisTitle( QwtPlot::xBottom, tr( "(Time)^-0.5" ) );
   data_plot1->setAxisTitle( QwtPlot::yLeft  , 
         tr( "Corrected Sed. Coeff. (1e-13 s)" ) );

   int nxy    = ( nscns > ndivs ) ? nscns : ndivs;
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
   for ( int ii = 0; ii < nscns; ii++ )
   {
      if ( excludedScans.contains( ii ) )
      {
         kk        += ndivs;          // excluded:  bump to next scan
         continue;
      }
      
      count      = 0;
      double xv  = ptx[ ii ];         // reciprocal square root of time value

      for ( int jj = 0; jj < ndivs; jj++ )
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
               tr( "Sed Coeff Points, scan %1" ).arg( ii+1 ) );

         curve->setStyle ( QwtPlotCurve::NoCurve );
         curve->setSymbol( sym );
         curve->setData  ( x, y, count );
      }
   }

   // fit lines for each division to all scan points

   for ( int jj = 0; jj < ndivs; jj++ )
   {  // walk thru divisions, fitting line to points from all scans
      count          = 0;
      for ( int ii = 0; ii < nscns; ii++ )
      {
         if ( excludedScans.contains( ii ) ) continue;

         kk         = ii * ndivs + jj;  // sed. coeff. index

         if ( ptx[ ii ] > 0.0  &&  pty[ kk ] > 0.0 )
         {  // points for scans in a division
            x[ count ] = ptx[ ii ];
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

         curve  = us_curve( data_plot1, tr( "Fitted Line %1" ).arg( jj ) );
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
   ndivs     = qRound( dval );

   data_plot();
}
void US_vHW_Enhanced::update_dsmoo(   double ) {}
void US_vHW_Enhanced::update_pcbound( double ) {}
void US_vHW_Enhanced::update_boundpo( double ) {}
void US_vHW_Enhanced::update_exsscan( double ) {}
void US_vHW_Enhanced::update_exscrng( double ) {}

//  find 1st concentration value in array greater than or equal to given value
int US_vHW_Enhanced::first_gteq( double concenv,
      QVector< US_DataIO::reading >& readings, int points )
{
   int index = -1;

   for ( int jj = 0; jj < points; jj++ )
   {
      if ( concenv < readings[ jj ].value )
      {
         index     = ( jj > 0 ) ? jj : -1;
         break;
      }

      else if ( concenv == readings[ jj ].value )
      {
         index     = ( jj > 0 ) ? jj : 0;
         break;
      }
   }
   return index;
}

//  get average scan plateau value for 11 points around input value
double US_vHW_Enhanced::avg_plateau( SCNDAT* s )
{
   double plato  = s->plateau;
   int    points = s->readings.size();
//qDebug() << "avg_plateau in: " << plato << "   points " << points;
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

double US_vHW_Enhanced::zone_plateau( SCNDAT* s, double baseline )
{
   double  plato  = -1.0;
   double  range  = s->plateau - baseline;
   double  startc = baseline + range * ct_boundaryPos->value() / 100.0;
   int     points = s->readings.size();
qDebug() << "zone: points=" << points;
   int     j0     = first_gteq( startc, s->readings, points );
           j0     = ( j0 < 0 ) ? 0 : j0;
qDebug() << "      j0=" << j0;
//qDebug() << "      bln v0 str " << baseline << s->readings[0].value << startc;
   int     nzp    = PZ_POINTS;
   int     j1     = 0;
   int     j2     = 0;
   int     j3     = 0;
   int     j8     = 0;
   int     j9     = 0;
   int     jj     = 0;
   int     l0     = nzp;
   double* x      = new double[ points ];
   double* y      = new double[ points ];

   // get the first potential zone and get its slope

   for ( jj = j0; jj < points; jj++ )
   {  // accumulate x,y for all readings in the scan
      x[ j9 ]       = s->readings[ jj ].d.radius;
      y[ j9++ ]     = s->readings[ jj ].value;
   }

   double  sumx;
   double  sumy;
   double  sumxy;
   double  sumxs;

   double  slope = calc_slope( x, y, nzp, sumx, sumy, sumxy, sumxs );
qDebug() << "         slope0 " << slope;

   // get slopes for sliding zone and detect where flat

   double  x0    = x[ 0 ];
   double  y0    = y[ 0 ];
   double  x1;
   double  y1;
   double  sllo1 = slope;
   double  sllo2 = slope;
   double  slhi1 = slope;
   double  slavg = 0.0;
   double  dypl  = 0.0;
   j9            = 0;
   jj            = 0;

   while ( l0 < points )
   {  // loop until zone end is at readings end or flat zone ends
      x1       = x[ l0 ];     // new values to use in slope sums
      y1       = y[ l0++ ];
      jj++;
      slope    = update_slope( nzp, x0, y0, x1, y1, sumx, sumy, sumxy, sumxs );
//qDebug() << "         jj " << jj << " slope " << slope;

      if ( slope < PZ_THRLO )
      {  // slope is below threshold, so we're in flat area
         if ( j1 == 0 )
         {  // first slope to fall below threshold (near zero)
            j1     = jj;
            j2     = jj;
            sllo1  = slope;
            sllo2  = slope;
qDebug() << "           1st flat jj " << jj;
         }

         else if ( slope < sllo2 )
         {  // slope is lowest so far
            j2     = jj;
            sllo2  = slope;
qDebug() << "           low flat jj " << jj;
         }
         slavg += slope;
      }

      else if ( j1 > 0  &&  slope > PZ_THRHI )
      {  // after flat area, first slope to get too high
         j9     = jj;
qDebug() << "           high after flat jj " << jj;
         slhi1  = slope;
         dypl   = y[ jj + nzp / 2 ] - s->plateau;
         dypl   = dypl > 0.0 ? dypl : -dypl;
         dypl  /= s->plateau;
         if ( dypl > 0.2 )
         {  // not near enough to plateau, assume another flat zone follows
qDebug() << "             reset for dypl " << dypl;
            j3     = j1;     // save indecies in case this is last found
            j8     = j9;
            j1     = 0;      // reset to search for new flat zone
            j9     = 0;
         }
         else
         {  // flat zone found near enough to end, so break out of loop
            break;
         }
      }

      x0       = x[ jj ];     // values to remove from next iteration
      y0       = y[ jj ];
   }

   if ( j1 < 1 )
   {  // no 2nd or subsequent flat zone:  use original
      j1       = j3;
      j9       = j8;
   }

   // average plateau over flat zone

   if ( j1 > j0 )
   {  // flat zone found:  get average plateau
      plato      = 0.0;
qDebug() << "        j1 j2 j9 " << j1 << j2 << j9;
      jj         = nzp / 2;            // bump start to middle of 1st gate`
      j1        += jj;
      j9         = ( j9 < j1 ) ? ( j1 + jj ) : j9;
qDebug() << "         sll1 sll2 slh1 " << sllo1 << sllo2 << slhi1;
      j2         = j9 - j1;            // size of overall flat zone
      if ( j2 > PZ_HZLO )
      {
         for ( jj = j1; jj < j9; jj++ )
            plato     += y[ jj ];      // sum y's in flat zone

         plato     /= (double)j2;      // plateau is average
qDebug() << "          plati plato " << s->plateau << plato;
         s->plateau = plato;
      }
   }

   delete [] x;                             // clean up
   delete [] y;

   return plato;
}

// calculate slope of x,y and return sums used in calculations
double US_vHW_Enhanced::calc_slope( double* x, double* y, int npts,
      double& sumx, double& sumy, double& sumxy, double& sumxs )
{
   sumx    = 0.0;
   sumy    = 0.0;
   sumxy   = 0.0;
   sumxs   = 0.0;

   for ( int ii = 0; ii < npts; ii++ )
   {
      sumx   += x[ ii ];
      sumy   += y[ ii ];
      sumxy  += ( x[ ii ] * y[ ii ] );
      sumxs  += ( x[ ii ] * x[ ii ] );
   }

   return fabs( ( (double)npts * sumxy - sumx * sumy ) /
                ( (double)npts * sumxs - sumx * sumx ) );
}

// update slope of sliding x,y by simply modifying running sums used
double US_vHW_Enhanced::update_slope( int npts,
      double x0, double y0, double x1, double y1,
      double& sumx, double& sumy, double& sumxy, double& sumxs )
{
   sumx   += ( x1 - x0 );
   sumy   += ( y1 - y0 );
   sumxy  += ( x1 * y1 - x0 * y0 );
   sumxs  += ( x1 * x1 - x0 * x0 );

   return fabs( ( (double)npts * sumxy - sumx * sumy ) /
                ( (double)npts * sumxs - sumx * sumx ) );
}

// get sedimentation coefficient for a given concentration
double US_vHW_Enhanced::sed_coeff( double cconc, double correc, double menis,
      double oterm, QVector< US_DataIO::reading >&readings, int points )
{
   int    j2   = first_gteq( cconc, readings, points );
   double rv0  = -1.0;          // mark radius excluded
   double sedc = -1.0;

   if ( j2 >= 0  &&  oterm >= 0.0 )
   {  // likely need to interpolate radius from two input values
      int j1      = j2 - 1;

      if ( j2 > 0 )
      {  // interpolate radius value
         double av1  = readings[ j1 ].value;
         double av2  = readings[ j2 ].value;
         double rv1  = readings[ j1 ].d.radius;
         double rv2  = readings[ j2 ].d.radius;
         double rra  = av2 - av1;
         rra         = ( rra == 0.0 ) ? 0.0 : ( ( rv2 - rv1 ) / rra );
         rv0         = rv1 + ( cconc - av1 ) * rra;
      }

      else
      {
         rv0         = -1.0;
      }
   }

   if ( rv0 > 0.0 )
   {  // use radius and other terms to get corrected sed. coeff. value
      sedc        = correc * log( rv0 / menis ) / oterm;
   }
   return sedc;
}

// calculate division sedimentation coefficient values (fitted line intercept)
void US_vHW_Enhanced::div_seds( )
{
   double* xx       = new double[ nscns ];
   double* yy       = new double[ nscns ];
   double* pp       = new double[ nscns ];
   int*    ll       = new int   [ nscns ];
   int     nscnu    = 0;
   int     row      = lw_triples->currentRow();
   EDTDAT* d        = &dataList[ row ];
   double  boundPct = ct_boundaryPercent->value() / 100.0;
   double  positPct = ct_boundaryPos    ->value() / 100.0;
   double  correc   = solution.correction * 1.0e13;
   double  baseline = calc_baseline();
   double  pconc;
   double  cconc;
   double  mconc;

   dseds.clear();

   for ( int jj = 0; jj < ndivs; jj++ )
   {
      double  dsed;
//qDebug() << "div_sed div " << jj+1;

      if ( jj == 0 )
      {  // we only need to calculate x values the 1st time thru

         for ( int ii = exclude; ii < nscns; ii++ )
         {
            if ( !excludedScans.contains( ii ) )
            {
               double timev  = d->scanData[ ii ].seconds - time_correction;
               xx[ nscnu ]   = 1.0 / sqrt( timev );
               ll[ nscnu++ ] = ii;
            }
         }
      }

      // accumulate y values for this division

      for ( int kk = 0; kk < nscnu; kk++ )
      {
         int     ii    = ll[ kk ];
         double  pddif = sdifs[ ii ] * divfac;
         SCNDAT* s     = &d->scanData[ ii ];
         int     npts  = s->readings.size();
         double  span  = s->plateau - baseline;
         double  bcut  = baseline + span * positPct;
         double  pcut  = bcut     + span * boundPct;
         span          = pcut - bcut;
         pconc         = bcut;
         pconc         = ( jj == 0 ) ? bcut : pp[ kk ];
         double  cinc  = cpds.at( ii ).at( jj ) + pddif;
         cconc         = pconc + cinc;
         mconc         = ( cconc + pconc ) * 0.5;
         pp[ kk ]      = cconc;
         double  omega = s->rpm * M_PI / 30.0;
         double  oterm = ( s->seconds - time_correction ) * omega * omega;
         yy[ kk ]      = sed_coeff( mconc, correc, d->meniscus, oterm,
               s->readings, npts );
      }
//qDebug() << " nscnu pp0 yy0 ppn yyn " << nscnu << yy[0] << pp[0]
//   << yy[nscnu-1] << pp[nscnu-1];

      // calculate and save the division sedcoeff

      double slope;
      double sigma;
      double corre;

      US_Math::linefit( &xx, &yy, &slope, &dsed, &sigma, &corre, nscnu );
qDebug() << "     dsed " << dsed;

      dseds.append( dsed );
   }
qDebug() << " dsed[0] " << dseds.at(0);
qDebug() << " dsed[L] " << dseds.at(ndivs-1);

   delete [] xx;                             // clean up
   delete [] yy;
   delete [] pp;
   delete [] ll;

qDebug() << "div_sed RETURN";
   return;
}

