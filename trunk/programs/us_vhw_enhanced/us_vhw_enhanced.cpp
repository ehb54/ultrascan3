//! \file us_vhw_enhanced.cpp

#include <QApplication>
#include <QtSvg>

#include "us_vhw_enhanced.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_matrix.h"
#include "us_constants.h"

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
US_vHW_Enhanced::US_vHW_Enhanced() : US_AnalysisBase2()
{
   // set up the GUI (mostly handled in US_AnalysisBase)

   setWindowTitle( tr( "Enhanced van Holde - Weischet Analysis:" ) );

   dbg_level     = US_Settings::us_debug();
   pb_dstrpl     = us_pushbutton( tr( "Distribution Plot" ) );
   pb_dstrpl->setEnabled( false );
   connect( pb_dstrpl, SIGNAL( clicked() ),
            this,       SLOT(  distr_plot() ) );

   pb_selegr     = us_pushbutton( tr( "Select Groups" ) );
   groupSel      = false;
   pb_selegr->setEnabled( false );
   connect( pb_selegr, SIGNAL( clicked() ),
            this,       SLOT(  sel_groups() ) );

   parameterLayout->addWidget( pb_dstrpl, 2, 0, 1, 2 );
   parameterLayout->addWidget( pb_selegr, 2, 2, 1, 2 );

   QLabel* lb_analysis     = us_banner( tr( "Analysis Controls"      ) );
   QLabel* lb_scan         = us_banner( tr( "Scan Control"           ) );
   QLabel* lb_smoothing    = us_label ( tr( "Data Smoothing:"        ) );
   QLabel* lb_boundPercent = us_label ( tr( "% of Boundary:"         ) );
   QLabel* lb_boundPos     = us_label ( tr( "Boundary Position (%):" ) );

   QLabel* lb_from         = us_label ( tr( "From:" ) );
   QLabel* lb_to           = us_label ( tr( "to:"   ) );

   lb_tolerance  = us_label( tr( "Back Diffusion Tolerance:" ) );
   lb_tolerance->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_tolerance = us_counter( 3, 0.001, 1.0, 0.001 );
   bdtoler      = 0.001;
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
   haveZone   = false;

}

// load data
void US_vHW_Enhanced::load( void )
{
   US_AnalysisBase2::load();

   if ( ! dataLoaded )
      return;

   if ( dataList[ 0 ].expType == "Equilibrium" )
   {
      QMessageBox::warning( this, tr( "Wrong Data Type" ),
            tr( "You have selected Equilibrium data, which is not\n"
                "appropriate for van Holde - Weischet analysis." ) );

      dataLoaded = false;
      lw_triples->disconnect();
      le_id     ->disconnect();
      le_temp   ->disconnect();
      te_desc   ->disconnect();

      lw_triples->clear();
      le_id     ->clear();
      le_temp   ->clear();
      te_desc   ->clear();

      data_plot1->clear();
      data_plot2->clear();

      dataList.clear();
      rawList .clear();
      triples .clear();
      pb_exclude->setEnabled( false );
      return;
   }

   connect( pb_save,    SIGNAL( clicked() ),
            this,       SLOT(   save_data() ) );
   connect( pb_view,    SIGNAL( clicked() ),
            this,       SLOT(   view_report() ) );

   data_plot1->setCanvasBackground( Qt::black );
   data_plot2->setCanvasBackground( Qt::black );
   data_plot1->setMinimumSize( 600, 500 );
   data_plot2->setMinimumSize( 600, 300 );

   gpick      = new US_PlotPicker( data_plot1 );
   gpick->setSelectionFlags( QwtPicker::PointSelection
                           | QwtPicker::ClickSelection );
   connect( gpick,    SIGNAL( mouseDown(  const QwtDoublePoint& ) ),
            this,       SLOT( groupClick( const QwtDoublePoint& ) ) );
   groupstep   = NONE;

   pb_selegr->setEnabled( true );
   pb_dstrpl->setEnabled( true );

   haveZone   = false;

   update( 0 );
}

// distribution plot
void US_vHW_Enhanced::distr_plot(  void )
{
   QList< double > bfracs;
   double  pterm    = 100.0 * positPct;
   double  bterm    = 100.0 * boundPct / (double)divsCount;
   double  bfrac;

   bfracs.clear();

   for ( int jj = 0; jj < divsCount; jj++ )
   {
      bfrac     = pterm + bterm * (double)( jj + 1 );
      bfracs.append( bfrac );
   }

   US_DistribPlot* dialog = new US_DistribPlot( bfracs, dseds );
   dialog->move( this->pos() + QPoint( 100, 100 ) );
   dialog->exec();
   delete dialog;
}

// data plot
void US_vHW_Enhanced::data_plot( void )
{
   double  xmax        = 0.0;
   double  ymax        = 0.0;
   int     count       = 0;
   int     nskip       = 0;
   int     totalCount;

   if ( !dataLoaded )
      return;

   row        = lw_triples->currentRow();
   d          = &dataList[ row ];

   data_plot2->detachItems();

   // let AnalysisBase do the lower plot
   US_AnalysisBase2::data_plot();

   // handle upper (vHW Extrapolation) plot, here

   valueCount = d->x.size();
   scanCount  = d->scanData.size();
   divsCount  = qRound( ct_division->value() );
   totalCount = scanCount * divsCount;
   divfac     = 1.0 / (double)divsCount;
   boundPct   = ct_boundaryPercent->value() / 100.0;
   positPct   = ct_boundaryPos    ->value() / 100.0;
   baseline   = calc_baseline();
   correc     = solution.s20w_correction * 1.0e13;
	C0         = 0.0;
	Swavg      = 0.0;
   omega      = d->scanData[ 0 ].rpm * M_PI / 30.0;
   plateau    = d->scanData[ 0 ].plateau;

   for ( int ii = 0; ii < scanCount; ii++ )
   {  // count the scans excluded due to position percent
      if ( excludedScans.contains( ii ) ) continue;
      
      s          = &d->scanData[ ii ];
      plateau    = avg_plateau( );
      range      = plateau - baseline;
      basecut    = baseline + range * positPct;
      
      if ( d->scanData[ ii ].readings[ 0 ].value > basecut ) nskip++;
   }
DbgLv(1) << " valueCount  totalCount" << valueCount << totalCount;
DbgLv(1) << "  scanCount  divsCount" << scanCount << divsCount;
   le_skipped->setText( QString::number( nskip ) );

   // Do first experimental plateau calcs based on horizontal zones

   int     nrelp = 0;
   int     nunrp = 0;
   QVector< double > pxvec( scanCount  );
   QVector< double > pyvec( totalCount );
   QVector< double > sxvec( scanCount  );
   QVector< double > syvec( scanCount  );
   QVector< double > fyvec( scanCount  );
   double* ptx   = pxvec.data();   // t,log(p) points
   double* pty   = pyvec.data();
   double* csx   = sxvec.data();   // count,slope points
   double* csy   = syvec.data();
   double* fsy   = fyvec.data();   // fitted slope

   QList< double > plats;
   QList< int >    isrel;
   QList< int >    isunr;

   // accumulate reliable,unreliable scans and plateaus

   scplats.fill( 0.0, scanCount );
   nrelp    = 0;
   nunrp    = 0;

DbgLv(1) << " Initial reliable (non-excluded) scan t,log(avg-plat):";
   // Initially reliable plateaus are all average non-excluded
   for ( int ii = 0; ii < scanCount; ii++ )
   {
      if ( excludedScans.contains( ii ) ) continue;

      s             = &d->scanData[ ii ];
      plateau       = avg_plateau();
      scplats[ ii ] = plateau;
      ptx[ nrelp ]  = s->seconds - time_correction;
      pty[ nrelp ]  = log( plateau );

      isrel.append( ii );
DbgLv(1) << ptx[nrelp] << pty[nrelp];
      nrelp++;
   }

   QList< double >  scpds;
   double  cconc;
   double  pconc;
   double  mconc;
   double  cinc;
   double  eterm;
   double  oterm;
   double  slope;
   double  intcp;
   double  sigma;
   double  corre;
   cpds.clear();
   int    krelp  = nrelp;
   int    jrelp  = nrelp;
   int    kf     = 0;

   // Accumulate count,slope points for line fits to the t,log(p) points

   for ( int jj = 6; jj < nrelp; jj++ )
   {
      US_Math2::linefit( &ptx, &pty, &slope, &intcp, &sigma, &corre, jj );

      csx[ kf ]   = (double)jj;
      csy[ kf++ ] = slope;
DbgLv(1) << "k,slope point: " << kf << jj << slope;
   }

   // Create a 3rd-order polynomial fit to the count,slope curve

   double coefs[ 4 ];

   US_Matrix::lsfit( coefs, csx, csy, kf, 4 );
DbgLv(1) << "3rd-ord poly fit coefs:" << coefs[0] << coefs[1] << coefs[2]
 << coefs[3];

   for ( int jj = 0; jj < kf; jj++ )
   {
      double xx = csx[ jj ];
      double yy = coefs[ 0 ] + coefs[ 1 ] * xx + coefs[ 2 ] * xx * xx
                + coefs[ 3 ] * xx * xx * xx;
      fsy[ jj ] = yy;
DbgLv(1) << " k,fit-slope point: " << jj << xx << yy << " raw slope" << csy[jj];
   }

   // Now find the spot where the derivative of the fit crosses zero
   double prevx = csx[ 1 ];
   double prevy = fsy[ 1 ];
   double maxd  = -1e-20;
   double dfac  = 1.0 / qAbs( fsy[ kf - 1 ] - prevy );  // norm. deriv. factor
   double prevd = ( prevy - fsy[ 0 ] ) * dfac;  // normalized derivative
   jrelp        = -1;

   for ( int jj = 2; jj < kf; jj++ )
   {
      double currx = csx[ jj ];
      double curry = fsy[ jj ];
      double currd = ( curry - prevy ) * dfac;  // normalized derivative
DbgLv(1) << "  k cx cy cd pd" << jj << currx << curry << currd << prevd;

      if (   currd == 0.0  ||
           ( currd > 0.0  &&  prevd < 0.0 )  ||
           ( currd < 0.0  &&  prevd > 0.0 ) )
      {  // Zero point or zero crossing
         jrelp    = jj - 1;
DbgLv(1) << "    Z-CROSS";
for ( int mm = jj + 1; mm < kf; mm++ ) {
 prevx = currx; prevy = curry; prevd = currd;
 currx = csx[mm]; curry = fsy[mm];
 currd = ( curry - prevy ) * dfac;
 DbgLv(1) << "  k cx cy cd pd" << mm << currx << curry << currd << prevd;
}
         break;
      }

      if ( currd > maxd )
      {  // Maximum derivative value
         maxd     = currd;
         jrelp    = jj;
DbgLv(1) << "    MAXD" << maxd;
      }
      prevx      = currx;
      prevy      = curry;
      prevd      = currd;
   }

   // Do final fit to the determined number of leading points
   krelp         = (int)csx[ jrelp ];

   US_Math2::linefit( &ptx, &pty, &slope, &intcp, &sigma, &corre, krelp );

   for ( int jj = krelp; jj < nrelp; jj++ )
   {  // Mark points beyond new count as unreliable
      isunr.append( isrel.at( jj ) );
   }

   nrelp          = krelp;
   nunrp          = isunr.size();
DbgLv(1) << " NRELP" << nrelp << "   slope intcp sigma"
 << slope << intcp << sigma;

   Swavg      = slope / ( -2.0 * omega * omega );  // Swavg func of slope
	C0         = exp( intcp );                      // C0 func of intercept
DbgLv(1) << "Swavg(c): " << Swavg*correc << " C0: " << C0 ;

   // Determine Cp for each of the unreliable scans
   //   y = ax + b, using "a" and "b" determined above.
   // Since y = log( Cp ), we get Cp by exponentiating
   //   the left-hand term.

   for ( int jj = 0; jj < nunrp; jj++ )
   {  // each extrapolated plateau is exp of Y for X of corrected time
      int     ii  = isunr.at( jj );
      double  tc  = d->scanData[ ii ].seconds - time_correction;

      scplats[ ii ] = exp( tc * slope + intcp );

DbgLv(1) << " jj scan plateau " << jj << ii+1 << scplats[ii];
   }

   // initialize plateau values for components of scans

   for ( int ii = 0; ii < scanCount; ii++ )
   {
      s          = &d->scanData[ ii ];
      valueCount = s->readings.size();
      range      = scplats[ ii ] - baseline;
      basecut    = baseline + range * positPct;
      platcut    = basecut  + range * boundPct;
      span       = platcut - basecut;
      cconc      = basecut;
      pconc      = basecut;
      mconc      = basecut;
      sumcpij    = 0.0;
      cinc       = span * divfac;
      omega      = s->rpm * M_PI / 30.0;
      oterm      = ( s->seconds - time_correction ) * omega * omega;
      eterm      = -2.0 * oterm / correc;
      c0term     = ( C0 - baseline ) * boundPct * divfac;

      scpds.clear();                       // clear this scan's Cp list

      if ( excludedScans.contains( ii ) )
      {
         for ( int jj = 0; jj < divsCount; jj++ )
            scpds.append( cinc );

         cpds << scpds;
         continue;
      }

      for ( int jj = 0; jj < divsCount; jj++ )
      {  // calculate partial plateaus
         pconc      = cconc;              // prev (baseline) div concentration
         cconc     += cinc;               // curr (plateau) div concentration
         mconc      = pconc + cinc * 0.5; // mid div concentration

         // get sedimentation coefficient for concentration
         sedc       = sed_coeff( mconc, oterm );

         if ( jj == 0  &&  ii == 0 )
         { // calculate back diffusion coefficient at 1st div of 1st scan
            //bdiffc     = back_diff_coeff( sedc * 1.0e-13 );
            //cpij       = sed_coeff( cconc + cinc, oterm );
            //bdiffc     = back_diff_coeff( cpij * 1.0e-13 );
            bdiffc     = back_diff_coeff( Swavg );
//DbgLv(1) << "  sedcM sedcC" << sedc << cpij;
DbgLv(1) << "  sedcM sedcC" << sedc << Swavg*correc;
         }

         // calculate the partial concentration for this division
         cpij       = c0term * exp( sedc * eterm );
//DbgLv(1) << " scn div cinc cpij " << ii+1 << jj+1 << cinc << cpij;
//DbgLv(1) << "  sedc eterm eso " << sedc << eterm << (eterm*sedc);

         // update Cpij sum and add to divisions list for scan
         sumcpij   += cpij;
         scpds.append( cpij );
      }

      // get span-minus-sum_cpij and divide by number of divisions
      sdiff    = ( span - sumcpij ) * divfac;
DbgLv(1) << "   sumcpij span " << sumcpij << span
   << " sumcpij/span " << (sumcpij/span);

      for ( int jj = 0; jj < divsCount; jj++ )
      {  // spread difference to each partial plateau concentration
         cpij     = scpds.at( jj ) + sdiff;
         scpds.replace( jj, cpij );
      }

      cpds << scpds;  // add cpij list to scan's list-of-lists
   }

   // iterate to adjust plateaus until none needed or max iters reached

   int     iter      = 1;
   int     mxiter    = 3;          // maximum iterations
   double  avdthr    = 2.0e-5;     // threshold cp-absavg-diff

   while( iter <= mxiter )
   {
      double avgdif  = 0.0;
      double adiff   = 0.0;
      count          = 0;
DbgLv(1) << "iter mxiter " << iter << mxiter;

      // get division sedimentation coefficient values (intercepts)

      div_seds();

      // reset division plateaus

      for ( int ii = 0; ii < scanCount; ii++ )
      {
         if ( excludedScans.contains( ii ) )   continue;

         s        = &d->scanData[ ii ];
         range    = scplats[ ii ] - baseline;
         basecut  = baseline + range * positPct;
         platcut  = basecut  + range * boundPct;
         span     = platcut - basecut;
         sumcpij  = 0.0;
         oterm    = ( s->seconds - time_correction ) * omega * omega;
         eterm    = -2.0 * oterm / correc;
         c0term   = ( C0 - baseline ) * boundPct * divfac;
         scpds    = cpds.at( ii );
         scpds.clear();

         // split difference between divisions

         for ( int jj = 0; jj < divsCount; jj++ )
         {  // recalculate partial concentrations based on sedcoeff intercepts
            sedc     = dseds[ jj ];
            cpij     = c0term * exp( sedc * eterm );
            scpds.append( cpij );
            sumcpij += cpij;
//DbgLv(1) << "    div " << jj+1 << "  tcdps cpij " << tcpds.at(jj) << cpij;
         }

         // set to split span-sum difference over each division
         sdiff    = ( span - sumcpij ) * divfac;

         for ( int jj = 0; jj < divsCount; jj++ )
         {  // spread difference to each partial plateau concentration
            cpij     = scpds.at( jj ) + sdiff;
            scpds.replace( jj, cpij );
         }

         cpds.replace( ii, scpds );  // replace scan's list of divison Cp vals

         adiff    = ( sdiff < 0 ) ? -sdiff : sdiff;
         avgdif  += adiff;  // sum of difference magnitudes
         count++;
DbgLv(1) << "   iter scn " << iter << ii+1 << " sumcpij span "
   << sumcpij << span << "  sdiff sumabsdif" << sdiff << avgdif;
      }

      avgdif  /= (double)count;  // average of difference magnitudes
DbgLv(1) << " iter" << iter << " avg(abs(sdiff))" << avgdif;

      if ( avgdif < avdthr )     // if differences are small, we're done
      {
DbgLv(1) << "   +++ avgdif < avdthr (" << avgdif << avdthr << ") +++";
         break;
      }

      iter++;
   }

   for ( int ii = 0; ii < totalCount; ii++ )
      pty[ ii ]   = -1.0;

   int     kk     = 0;                    // index to sed. coeff. values
   int     kl     = 0;                    // index/count of live scans

   // Calculate the corrected sedimentation coefficients

   for ( int ii = 0; ii < scanCount; ii++ )
   {
      if ( excludedScans.contains( ii ) )
      {
         kk         += divsCount;
         continue;
      }

      s              = &d->scanData[ ii ];
      double  timev  = s->seconds - time_correction;
      double  timex  = 1.0 / sqrt( timev );
      double  bdrad  = bdrads.at( kl );   // back-diffus cutoff radius for scan
      double  bdcon  = bdcons.at( kl++ ); // back-diffus cutoff concentration
      double  divrad = 0.0;               // division radius value
DbgLv(1) << "scn liv" << ii+1 << kl
   << " radius concen time" << bdrad << bdcon << timev;

      ptx[ ii ]  = timex;         // save corrected time and accum max
      xmax       = ( xmax > timex ) ? xmax : timex;

      valueCount = s->readings.size();
      range      = scplats[ ii ] - baseline;
      cconc      = baseline + range * positPct; // initial conc for span
      basecut    = cconc;
      omega      = s->rpm * M_PI / 30.0;
      oterm      = ( timev > 0.0 ) ? ( timev * omega * omega ) : -1.0;
      scpds      = cpds.at( ii );  // list of conc values of divs this scan

      for ( int jj = 0; jj < divsCount; jj++ )
      {  // walk through division points; get sed. coeff. by place in readings
         pconc       = cconc;               // div base
         cpij        = scpds.at( jj );      // div partial concentration
         cconc       = pconc + cpij;        // absolute concentration
         mconc       = pconc + cpij * 0.5;  // mid div concentration

         int rx      = first_gteq( mconc, s->readings, valueCount, 0 );
         divrad      = readings_radius( d, rx ); // radius this division pt.

         if ( divrad < bdrad  ||  mconc < bdcon )
         {  // corresponding sedimentation coefficient
            sedc        = sed_coeff( mconc, oterm );
if(sedc<0)
DbgLv(1) << " *excl* div" << jj+1 << " mconc conc0" << mconc
   << s->readings[0].value;
         }

         else
         {  // mark a point to be excluded by back-diffusion
            sedc        = -1.0;
DbgLv(1) << " *excl* div" << jj+1 << " drad dcon " << divrad << mconc;
         }

         // y value of point is sedcoeff; accumulate y max
         pty[ kk++ ] = sedc;
         ymax        = ( ymax > sedc ) ? ymax : sedc;
      }
   }

   // Draw plot
   data_plot1->clear();
   us_grid( data_plot1 );

   data_plot1->setTitle( tr( "Run " ) + runID + tr( ": Cell " ) + d->cell
             + " (" + d->wavelength + tr( " nm) - vHW Extrapolation Plot" ) );

   data_plot1->setAxisTitle( QwtPlot::xBottom, tr( "(Time)^-0.5" ) );
   data_plot1->setAxisTitle( QwtPlot::yLeft  , 
         tr( "Corrected Sed. Coeff. (1e-13 s)" ) );

   int nxy    = ( scanCount > divsCount ) ? scanCount : divsCount;
   QVector< double > xvec( nxy );
   QVector< double > yvec( nxy );
   double* x  = xvec.data();
   double* y  = yvec.data();
   
   QwtPlotCurve* curve;
   QwtSymbol     sym;
   sym.setStyle( QwtSymbol::Ellipse );
   sym.setPen  ( QPen( Qt::blue ) );
   sym.setBrush( QBrush( Qt::white ) );
   sym.setSize ( 8 );
 
   kk         = 0;                    // index to sed. coeff. values

   // Set points for each division of each scan

   for ( int ii = 0; ii < scanCount; ii++ )
   {
      if ( excludedScans.contains( ii ) )
      {
         kk        += divsCount;      // excluded:  bump to next scan
         continue;
      }
      
      count      = 0;
      double xv  = ptx[ ii ];         // reciprocal square root of time value

      for ( int jj = 0; jj < divsCount; jj++ )
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

   for ( int jj = 0; jj < divsCount; jj++ )
   {  // walk thru divisions, fitting line to points from all scans
      count          = 0;

      for ( int ii = 0; ii < scanCount; ii++ )
      {
         if ( excludedScans.contains( ii ) ) continue;

         kk         = ii * divsCount + jj;  // sed. coeff. index

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

         US_Math2::linefit( &x, &y, &slope, &intcept, &sigma, &correl, count );

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
   xmax   = (double)qRound( ( xmax + 0.0009 ) / 0.001 ) * 0.001;
   ymax   = (double)qRound( ( ymax + 0.3900 ) / 0.400 ) * 0.400;
   data_plot1->setAxisScale( QwtPlot::xBottom, 0.0, xmax, 0.005 );
   data_plot1->setAxisScale( QwtPlot::yLeft,   0.0, ymax, 0.500 );

   count  = 0;

   for ( int ii = 0; ii < scanCount; ii++ )
   {  // accumulate points of back-diffusion cutoff line

      if ( !excludedScans.contains( ii ) )
      {
         x[ count ] = bdrads.at( count );
         y[ count ] = bdcons.at( count );
DbgLv(2) << "   bd x y k " << x[count] << y[count] << count+1;
         count++;
      }
   }

   // plot the red back-diffusion cutoff line
   dcurve  = us_curve( data_plot2, tr( "Fitted Line BD" ) );
   dcurve->setPen( QPen( QBrush( Qt::red ), 3.0 ) );
   dcurve->setData( x, y, count );
DbgLv(1) << " DP: xr0 yr0 " << x[0]       << y[0];
DbgLv(1) << " DP: xrN yrN " << x[count-1] << y[count-1] << count;
   data_plot2->replot();

   // plot any upper plot vertical excluded-scan line(s)

   int frsc   = qRound( ct_from->value() );
   int tosc   = qRound( ct_to  ->value() );

   if ( tosc > 0 )
   {
      frsc       = ( frsc < 1 ) ? 1 : frsc;
      y[ 0 ]     = 0.5;
      y[ 1 ]     = ymax - 0.1;
      int ixsc   = 0;

      for ( int ii = 0; ii < scanCount; ii++ )
      { 
         if ( excludedScans.contains( ii ) )  continue;

         ixsc++;
         if ( ixsc < frsc )  continue;
         if ( ixsc > tosc )  break;

         x[ 0 ]     = 1.0 / sqrt( d->scanData[ ii ].seconds - time_correction );
         x[ 1 ]     = x[ 0 ];
         curve      = us_curve( data_plot1,
            tr( "Scan %1 Exclude Marker" ).arg( ixsc ) );
         curve->setPen( QPen( QBrush( Qt::red ), 1.0 ) );
         curve->setData( x, y, 2 );
      }
   }

   data_plot1->replot();

   // save all sedcoeff values for report files
   aseds.clear();

   for ( int ii = 0; ii < totalCount; ii++ )
   {
      aseds.append( pty[ ii ] );
   }
}

// Write the main report HTML to a stream
void US_vHW_Enhanced::write_report( QTextStream& ts )
{
   d          = &dataList[ lw_triples->currentRow() ];
   ts << html_header( "US_vHW_Enhanced",
         tr( "van Holde - Weischet Analysis" ), d );
   ts << run_details();
   ts << hydrodynamics();
   ts << analysis( "" );

   ts << "\n" + indent( 4 ) + tr( "<h3>Selected Groups:</h3>\n" )
         + indent( 4 ) + "<table>\n";
   int ngrp = groupdat.size();

   if ( ngrp == 0 )
      ts << table_row( tr( "Groups Selected:" ), tr( "NONE" ) );

   else
   {
      ts << table_row( tr( "Group:" ), tr( "Average S:" ),
                       tr( "Relative Amount:" ) );

      for ( int jj = 0; jj < ngrp; jj++ )
      {
         ts << table_row(
               QString().sprintf( "%3d:", jj + 1 ),
               QString().sprintf( "%6.2f", groupdat.at( jj ).sed ),
               QString().sprintf( "(%5.2f %%)", groupdat.at( jj ).percent ) );
      }
   }

   ts << indent( 4 ) + "</table>\n\n";

   ts << indent( 4 ) + "<br/><table>\n";
   ts << table_row( tr( "Average S:" ),
                    QString::number( Swavg * 1.0e13 ) );
   ts << table_row( tr( "Initial concentration from plateau fit:" ),
                     QString::number( C0 ) +  tr( " OD/fringes" ) );
   ts << indent( 4 ) + "</table>\n";

   double  sl;
   double  ci;
   double  sig;
   double  cor;
   QVector< double > xvec( scanCount );
   QVector< double > yvec( scanCount );
   double* x  = xvec.data();
   double* y  = yvec.data();
   QString tscn;
   QString tpla;

   ts << scan_info();

   for ( int ii = 0; ii < scanCount; ii++ )
   {  // accumulate time,plateau pairs for line fit
      s            = &d->scanData[ ii ];
      x[ ii ]      = s->seconds;
      y[ ii ]      = scplats[ ii ];
   }

   US_Math2::linefit( &x, &y, &sl, &ci, &sig, &cor, scanCount );

   ts << "\n" + indent( 4 ) + "<br/><table>\n";
   ts << table_row( tr( "Initial Concentration:   " ),
                    QString::number( ci ) + " OD" );
   ts << table_row( tr( "Correlation Coefficient: " ),
                    QString::number( cor ) );
   ts << table_row( tr( "Standard Deviation:      " ),
                    QString::number( sig ) );
   ts << table_row( tr( "Initial Concentration from exponential fit: " ),
                    QString::number( C0 ) + " OD" );
   ts << indent( 4 ) + "</table>\n";

   ts << "  </body>\n</html>\n";
}

// Write SVG plot file
void US_vHW_Enhanced::write_plot( const QString fname, const QwtPlot* plot )
{
   QSvgGenerator generator;
   generator.setSize( plot->size() );
   generator.setFileName( fname );
   plot->print( generator );
}

// save the enhanced data
void US_vHW_Enhanced::save_data( void )
{ 
   row           = lw_triples->currentRow();
   d             = &dataList[ row ];
   QString tripl = QString( triples.at( row ) ).replace( " / ", "" );
   QString basernam  = US_Settings::resultDir() + "/" + d->runID
      + "/vHW." + tripl + ".";
   QStringList files;

   // Write results files
   write_vhw();
   write_dis();
   files << basernam + "extrap.dat";
   files << basernam + "distrib.dat";

   if ( groupdat.size() > 0 )
   {
      write_model();
      files << basernam + "fef_model.rpt";
   }

   // Set up to write reports files
   QString basename  = US_Settings::reportDir() + "/" + d->runID
      + "/vHW." + tripl + ".";
   QString htmlFile  = basename + "report.html";
   QString plot1File = basename + "velocity.svg";
   QString plot2File = basename + "extrap.svg";
   QString plot3File = basename + "s-c-distrib.svg";
   QString plot4File = basename + "s-c-histo.svg";

   // Write the main report file
   QFile rpt_f( htmlFile );

   if ( !rpt_f.open( QIODevice::WriteOnly | QIODevice::Text ) )
      return;

   QTextStream ts( &rpt_f );
   write_report( ts );
   rpt_f.close();

   // Write the velocity and extrapolation plots
   write_plot( plot1File, data_plot2 );
   write_plot( plot2File, data_plot1 );

   // Save distribution and histogram plots
   QList< double > bfracs;
   double  bfrac    = 100.0 * positPct;
   double  bterm    = 100.0 * boundPct / (double)divsCount;
   bfracs.clear();

   for ( int jj = 0; jj < divsCount; jj++ )
   {
      bfrac    += bterm;
      bfracs.append( bfrac );
   }

   US_DistribPlot* dialog = new US_DistribPlot( bfracs, dseds );
   dialog->save_plots( plot3File, plot4File );
   delete dialog;

   files << " ";
   files << htmlFile;
   files << plot1File;
   files << plot2File;
   files << plot3File;
   files << plot4File;

   // Report files created to the user
   QString wmsg = tr( "Wrote:\n\n" );

   for ( int ii = 0; ii < files.size(); ii++ )
      wmsg += "  " + files.at( ii ) + "\n";

   QMessageBox::information( this, tr( "Successfully Written" ), wmsg );
}

// generate result report, pop up dialog and display the report
void US_vHW_Enhanced::view_report( void )
{
   // Generate the main report html text
   QString rtext;
   QTextStream ts( &rtext );
   write_report( ts );

   // Create the report dialog and display the text
   US_Editor* edit = new US_Editor( US_Editor::LOAD, true );
   edit->setWindowTitle( "Results:  van Holde - Weischet Analysis" );
   edit->move( this->pos() + QPoint( 100, 100 ) );
   edit->resize( 700, 600 );
   edit->e->setFont( QFont( US_GuiSettings::fontFamily(),
                            US_GuiSettings::fontSize() ) );
   edit->e->setText( rtext );
   edit->show();
}

// alternate between selecting set-up and clearing vH-W groups
void US_vHW_Enhanced::sel_groups(  void )
{
   if ( groupSel )
   {  // had been Select, now doing Clear
      groupstep = NONE;
      pb_selegr->setText( tr( "Select Groups" ) );

      data_plot1->detachItems( QwtPlotItem::Rtti_PlotMarker );
      data_plot1->replot();
      groupdat.clear();
      groupxy.clear();
   }

   else
   {  // had been Clear, now doing new Select
      groupstep = START;
      pb_selegr->setText( tr( "Clear Groups" ) );
      groupxy.clear();

      QMessageBox::information( this, tr( "Group Selection" ),
            tr( "Please click first ABOVE, then BELOW the intercepts on the\n"
                "Y-axis of the van Holde - Weischet extrapolation plot to\n"
                "define groups and to average the S-values within a group.\n\n"
                "PLEASE NOTE:\n"
                "This algorithm provides for multiple and also overlapping\n"
                "groups, such that the total percentage may exceed 100%" ) );
   }

   groupSel  = !groupSel;    // reverse Select/Clear mode
}

// update density
void US_vHW_Enhanced::update_density(  double dval )
{
   density   = dval;
}

// update viscosity
void US_vHW_Enhanced::update_viscosity( double dval )
{
   viscosity  = dval;
}

// update vbar
void US_vHW_Enhanced::update_vbar(      double  dval )
{
   vbar       = dval;
}


void US_vHW_Enhanced::update_bdtoler(    double dval )
{
   bdtoler   = dval;

   data_plot();
}

void US_vHW_Enhanced::update_divis(      double dval )
{ 
   divsCount = qRound( dval );

   data_plot();
}

// index to first readings value greater than or equal to given concentration
int US_vHW_Enhanced::first_gteq( double concenv,
      QVector< US_DataIO2::Reading >& readings, int valueCount, int defndx )
{
   int index = defndx;

   for ( int jj = 0; jj < valueCount; jj++ )
   {
      if ( readings[ jj ].value >= concenv )
      {
         index     = jj;
         break;
      }
   }
   return index;
}

int US_vHW_Enhanced::first_gteq( double concenv,
      QVector< US_DataIO2::Reading >& readings, int valueCount )
{
   return first_gteq( concenv, readings, valueCount, -1 );
}

//  get average scan plateau value for 11 points around input value
double US_vHW_Enhanced::avg_plateau( )
{
   double plato  = 0.0;
//int points=s->readings.size();
//DbgLv(1) << "avg_plateau in: " << plato << "   points " << points;
//DbgLv(1) << " rd0 rdn " << s->readings[0].value << s->readings[points-1].value;
   int j2  = US_DataIO2::index( d->x, d->plateau );
   int j1  = max( 0, ( j2 - PA_POINTS ) );
       j2  = min( valueCount, ( j2 + PA_POINTS + 1 ) );

   for ( int jj = j1; jj < j2; jj++ )    // Sum the 41 points centered at
      plato += s->readings[ jj ].value;  //  the scan plateau radial position

   plato /= (double)( j2 - j1 );         // Find and return the average

   return plato;
}

// get sedimentation coefficient for a given concentration
double US_vHW_Enhanced::sed_coeff( double cconc, double oterm )
{
   int    j2   = first_gteq( cconc, s->readings, valueCount );
   double rv0  = -1.0;          // mark radius excluded
   double sedc = -1.0;

   if ( j2 >= 0  &&  oterm >= 0.0 )
   {  // likely need to interpolate radius from two input values
      int j1      = j2 - 1;

      if ( j2 > 0 )
      {  // interpolate radius value
         double av1  = s->readings[ j1 ].value;
         double av2  = s->readings[ j2 ].value;
         double rv1  = readings_radius( d, j1 );
         double rv2  = readings_radius( d, j2 );
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
      sedc        = correc * log( rv0 / d->meniscus ) / oterm;
   }
   return sedc;
}

// calculate division sedimentation coefficient values (fitted line intercepts)
void US_vHW_Enhanced::div_seds( )
{
   QVector< double > xxv( scanCount );
   QVector< double > yyv( scanCount );
   QVector< double > ppv( scanCount );
   QVector< double > xrv( scanCount );
   QVector< double > yrv( scanCount );
   QVector< int    > llv( scanCount );
   double* xx       = xxv.data();
   double* yy       = yyv.data();
   double* pp       = ppv.data();
   double* xr       = xrv.data();
   double* yr       = yrv.data();
   int*    ll       = llv.data();
   int     nscnu    = 0;  // number used (non-excluded) scans
   int     kscnu    = 0;  // count of scans of div not affected by diffusion
   double  bdifsqr  = sqrt( bdiffc );  // sqrt( diff ) used below
   double  pconc;
   double  cconc;
   double  mconc;
   bdtoler          = ct_tolerance->value();

   dseds.clear();
   dslos.clear();
   dsigs.clear();
   dcors.clear();
   dpnts.clear();

   for ( int jj = 0; jj < divsCount; jj++ )
   {  // loop to fit points across scans in a division
      double  dsed;
      double  slope;
      double  sigma;
      double  corre;
      int     ii;
      double  oterm;
      double  timecor;
      double  timesqr;
      double  bdleft;
      double  xbdleft;
      double  bottom;
      double  radD;
      double  omegasq;
//DbgLv(1) << "div_sed div " << jj+1;

      if ( jj == 0 )
      {  // we only need to calculate x values, bcut the 1st time thru

         for ( ii = 0; ii < scanCount; ii++ )
         {
            if ( !excludedScans.contains( ii ) )
            {
               s           = &d->scanData[ ii ];
               valueCount  = s->readings.size();
               omega       = s->rpm * M_PI / 30.0;
               omegasq     = omega * omega;
               timecor     = s->seconds - time_correction;
               timesqr     = sqrt( timecor );
               mconc       = s->readings[ 0 ].value;
               pconc       = baseline + ( scplats[ ii ] - baseline) * positPct;
               xx[ nscnu ] = 1.0 / timesqr;
               ll[ nscnu ] = ii;
               pp[ nscnu ] = pconc;               // analysis baseline
               pconc       = max( pconc, mconc ); // cleared of meniscus

               // accumulate limits based on back diffusion

//left=tolerance*pow(diff,0.5)/(2*intercept[0]*omega_s*
//  (bottom+run_inf.meniscus[selected_cell])/2
//  *pow(run_inf.time[selected_cell][selected_lambda][i],0.5);
//radD=bottom-(2*find_root(left)
//  *pow((diff*run_inf.time[selected_cell][selected_lambda][i]),0.5));
               bottom      = readings_radius( d, valueCount - 1 );
               oterm       = timecor * omegasq;
               cpij        = cpds.at( ii ).at( jj );

               mconc       = pconc + cpij * 0.5;
               dsed        = sed_coeff( mconc, oterm );
               dsed       *= 1.0e-13;

               // left = tolerance * sqrt( diff )
               //        / ( 2 * intercept[0] * omega^2
               //            * ( bottom + meniscus ) / 2 * sqrt( time ) )

               bdleft      = bdtoler * bdifsqr
                  / ( dsed * omegasq * ( bottom + d->meniscus ) * timesqr );
               xbdleft     = find_root( bdleft );

               // radD = bottom - ( 2 * find_root(left) * sqrt( diff * time ) )

               radD        = bottom - ( 2.0 * xbdleft * bdifsqr * timesqr );

               int mm      = 0;
               int mmlast  = valueCount - 1;

               while ( readings_radius( d, mm ) < radD  &&  mm < mmlast )
                  mm++;

               // accumulate for this scan of this division
               //  the back diffusion limit radius and corresponding absorbance
               xr[ nscnu ] = radD;
               yr[ nscnu ] = s->readings[ mm ].value;
DbgLv(1) << "  bottom meniscus bdleft" << bottom << d->meniscus << bdleft;
DbgLv(1) << "  dsed find_root toler" << dsed << xbdleft << bdtoler;
DbgLv(1) << "  bdiffc bdifsqr mm" << bdiffc << bdifsqr << mm << mmlast;
DbgLv(1) << "BD x,y " << nscnu+1 << radD << yr[nscnu];

               nscnu++;
            }
         }
      }

      kscnu      = nscnu;

      // accumulate y values for this division

      for ( int kk = 0; kk < nscnu; kk++ )
      { // accumulate concentration, sed.coeff. for all scans, this div
         ii         = ll[ kk ];                   // scan index
         s          = &d->scanData[ ii ];         // scan info
         valueCount = s->readings.size();         // readings in this scan
         pconc      = pp[ kk ];                   // prev concen (baseline)
         cpij       = cpds.at( ii ).at( jj );     // partial concen (increment)
         cconc      = pconc + cpij;               // curr concen (plateau)
         mconc      = ( cconc + pconc ) * 0.5;    // mid div concentration
         omega      = s->rpm * M_PI / 30.0;       // omega
         oterm      = ( s->seconds - time_correction ) * omega * omega;

         pp[ kk ]   = cconc;        // division mark of concentration for scan
         yy[ kk ]   = sed_coeff( mconc, oterm );  // sedimentation coefficient

         int mm     = first_gteq( mconc, s->readings, valueCount, 0 );
         radD       = readings_radius( d, mm );   // radius this division pt.

         if ( radD > xr[ kk ]  &&  mconc > yr[ kk ] )
         {  // gone beyond back-diffusion cutoff: exit loop with truncated list
            kscnu      = kk;
//DbgLv(2) << " div kscnu" << jj+1 << kscnu
//   << " radD xrkk" << radD << xr[kk] << " mconc yrkk" << mconc << yr[kk];
//if(kscnu==0) DbgLv(2) << "   pc cc cpij mm" << pconc << cconc << cpij << mm;
            break;
         }
//if ( kk < 2 || kk > (nscnu-3) )
//DbgLv(2) << "div scn " << jj+1 << ii+1 << " pconc cconc " << pconc << cconc;

      }
//DbgLv(2) << " nscnu pp0 yy0 ppn yyn " << nscnu << yy[0] << pp[0]
//   << yy[nscnu-1] << pp[nscnu-1];

      ii         = 0;
      for ( int kk = 0; kk < kscnu; kk++ )
      {  // remove any leading points below meniscus
         if ( yy[ kk ] > 0.0 )
         { // sed coeff value positive (not from below meniscus)
            if ( kk > ii )
               yy[ ii ] = yy[ kk ];
            ii++;
         }
         else
            DbgLv(1) << "+++ SED<=0.0 div" << jj+1 << "scn" << kk+1;
      }
      kscnu      = ii;

      if ( kscnu > 0 )
      {
         // calculate and save the division sedcoeff and fitted line slope

         US_Math2::linefit( &xx, &yy, &slope, &dsed, &sigma, &corre, kscnu );
      }

      else
      {
         ii         = nscnu / 2;
         dsed       = yy[ ii ];
         slope      = 0.0;
         sigma      = 0.0;
         corre      = 0.0;
         slope      = 0.0;
ii=(nscnu<1)?1:nscnu;
DbgLv(1) << "    nscnu" << nscnu << "pp0 yy0 ppn yyn " << pp[0] << yy[0]
   << pp[ii-1] << yy[ii-1];
      }

      dseds.append( dsed );

      dslos.append( slope );
      dsigs.append( sigma );
      dcors.append( corre );
      dpnts.append( kscnu );
//if((jj&7)==0||jj==(divsCount-1))
// DbgLv(1) << "     div dsed slope " << jj+1 << dsed << slope;

   }
DbgLv(1) << " dsed[0] " << dseds.at(0);
DbgLv(1) << " dsed[L] " << dseds.at(divsCount-1);
DbgLv(1) << " D_S: xr0 yr0 " << xr[0] << yr[0];
DbgLv(1) << " D_S: xrN yrN " << xr[nscnu-1] << yr[nscnu-1] << nscnu;

   bdrads.clear();
   bdcons.clear();

   for ( int kk = 0; kk < nscnu; kk++ )
   {
      bdrads.append( xr[ kk ] );
      bdcons.append( yr[ kk ] );
   }

   return;
}

// find root X where evaluated Y is virtually equal to a goal, using a
//  calculation including the inverse complementary error function (erfc).
double US_vHW_Enhanced::find_root( double goal )
{
#ifdef WIN32
#define erfc(x) US_Math2::erfc(x)
#endif

#define _FR_MXKNT 100
   double  tolerance = 1.0e-7;
   double  x1        = 0.0;
   double  x2        = 10.0;
   double  xv        = 5.0;
   double  xdiff     = 2.5;
   double  xsqr      = xv * xv;
   double  rsqr_pi   = 1.0 / sqrt( M_PI );
   double  test      = exp( -xsqr ) * rsqr_pi - ( xv * erfc( xv ) );
           test      = ( goal != 0.0 ) ? test : 0.0;
DbgLv(1) << "      find_root: goal test" << goal << test << " xv" << xv;
//DbgLv(1) << "        erfc(x)" << erfc(xv);
   int     count     = 0;

   // iterate until the difference between subsequent x value evaluations
   //  is too small to be relevant;

   while ( fabs( test - goal ) > tolerance )
   {
      xdiff  = ( x2 - x1 ) / 2.0;

      if ( test < goal )
      { // at less than goal, adjust top (x2) limit
         x2     = xv;
         xv    -= xdiff;
      }

      else
      { // at greater than goal, adjust bottom (x1) limit
         x1     = xv;
         xv    += xdiff;
      }

      // then update the test y-value
      xsqr   = xv * xv;
      test   = ( 1.0 + 2.0 * xsqr ) * erfc( xv ) 
         - ( 2.0 * xv * exp( -xsqr ) ) * rsqr_pi;
//DbgLv(1) << "      find_root:  goal test" << goal << test << " x" << xv;

      if ( (++count) > _FR_MXKNT )
         break;
   }
DbgLv(1) << "      find_root:  goal test" << goal << test
   << " xv" << xv << "  count" << count;

   return xv;
}

// calculate back diffusion coefficient
double US_vHW_Enhanced::back_diff_coeff( double sedc )
{
   double  tempera  = le_temp->text().section( " ", 0, 0 ).toDouble();
   double  RT       = R * ( K0 + tempera );
   double  D1       = AVOGADRO * 0.06 * M_PI * viscosity;
   double  D2       = 0.045 * sedc * vbar * viscosity;
   double  D3       = 1.0 - vbar * density;

   double  bdcoef   = RT / ( D1 * sqrt( D2 / D3 ) );

   DbgLv(1) << "BackDiffusion:";
DbgLv(1) << " RT " << RT << " R K0 tempera  " << R << K0 << tempera;
DbgLv(1) << " D1 " << D1 << " viscosity AVO " << viscosity << AVOGADRO; 
DbgLv(1) << " D2 " << D2 << " sedc vbar     " << sedc << vbar;
DbgLv(1) << " D3 " << D3 << " density       " << density;
DbgLv(1) << "  bdiffc" << bdcoef << " = RT/(D1*sqrt(D2/D3))";
   return bdcoef;
}

// handle mouse clicks for selecting vH-W groups
void US_vHW_Enhanced::groupClick( const QwtDoublePoint& p )
{
   QwtPlotMarker* marker;
   QwtText        label;
   GrpInfo        cgrdata;
   QString        gbanner;
   int            ngroup;
DbgLv(1) << "groupClick: step" << groupstep
   << "x y" << p.x() << p.y();

   switch( groupstep )
   {
      case NONE :
         break;

      case START :
         groupstep = END;
         groupxy << p.x() << p.y();      // add start x,y to list
         break;

      case END :
         groupstep = START;
         groupxy << p.x() << p.y();      // add end x,y to list
         ngroup    = groupxy.size() / 4;

         add_group_info( );

         marker    = new QwtPlotMarker;
         cgrdata   = groupdat.at( ngroup-1 );

         gbanner   = tr( "Group %1: %2 (%3%)" )
               .arg( ngroup ).arg( cgrdata.sed ).arg( cgrdata.percent );
         label.setText( gbanner );
         label.setFont( QFont( US_GuiSettings::fontFamily(),
                  -1, QFont::Bold ) );
         label.setColor( Qt::magenta );
         label.setBackgroundBrush( QBrush( QColor( 8, 8, 8, 128 ) ) );

         marker->setValue( 0.0, cgrdata.sed );
         marker->setLabel( label );
         marker->setLabelAlignment( Qt::AlignRight | Qt::AlignVCenter );
         marker->attach( data_plot1 );

         data_plot1->replot();
         break;

      default :
         break;
   }
DbgLv(1) << "groupClick:  nxy val" << groupxy.size();
}

// add to selected group information
void US_vHW_Enhanced::add_group_info( )
{
   int     ngroup  = groupxy.size() / 4;
   int     jg      = ( ngroup - 1 ) * 4;
   GrpInfo grdat;

   grdat.x1        = groupxy.at( jg     ); // save pick coordinates
   grdat.y1        = groupxy.at( jg + 1 );
   grdat.x2        = groupxy.at( jg + 2 );
   grdat.y2        = groupxy.at( jg + 3 );
   grdat.sed       = 0.0;                  // initialize other variables
   grdat.percent   = 0.0;
   grdat.ndivs     = 0;
   grdat.idivs.clear();

   divsCount       = qRound( ct_division->value() );

   for ( int jj = 0; jj < divsCount; jj++ )
   {  // walk thru all division lines to see if within clicked range
      double sed      = dseds.at( jj );         // intercept sed coeff
      double slope    = dslos.at( jj );         // div line slope
      double yh       = sed + slope * grdat.x1; // line intercept w high x
      double yl       = sed + slope * grdat.x2; // line intercept w low x

      if ( yh <= grdat.y1  &&  yl >= grdat.y2 )
      {  // line is in group:  add division to group
         grdat.idivs.append( jj );  // add to div index list
         grdat.sed      += sed;     // accumulate sedcoeff sum for average
         grdat.ndivs++;             // bump included divs count
      }
   }

   // finish average-sed-coeff and percent calculations; add to groups list
   grdat.ndivs     = grdat.ndivs > 0 ? grdat.ndivs : 1;
   grdat.sed      /= (double)grdat.ndivs;
   grdat.percent   = ( (double)grdat.ndivs / (double)divsCount ) * 100.0;

   groupdat.append( grdat );
}

// write a file of vHW extrapolation data
void US_vHW_Enhanced::write_vhw()
{
   QString filename = US_Settings::resultDir() + "/" + d->runID + "/vHW."
      + QString( triples.at( row ) ).replace( " / ", "" )
      + ".extrap.dat";

   QFile   res_f( filename );
   double  sedc;
   int     lastDiv   = divsCount - 1;
   int     kk        = 0;
   QString control   = "\t";

   if ( !res_f.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      return;
   }
DbgLv(1) << "WV: filename " << filename;

   QTextStream ts( &res_f );

   for ( int ii = 0; ii < scanCount; ii++ )
   {
      // each output line begins with reciprocal square root of scan time
      control      = "\t";
      ts << 1.0 / sqrt( d->scanData[ ii ].seconds - time_correction )
         << control;

      // balance of line is a list of sedimentation coefficient values for
      //  the divisions in the scan 
      for ( int jj = 0; jj < divsCount; jj++ )
      {
         sedc         = aseds.at( kk++ );

         if ( jj == lastDiv )
            control      = "\n";

         if ( sedc > 0.0 )
            ts << sedc << control;
         else
            ts << "      " << control;
      }
   }

   res_f.close();
}

// write a file of vHW division distribution values
void US_vHW_Enhanced::write_dis()
{
   QString filename = US_Settings::resultDir() + "/" + d->runID + "/vHW."
      + QString( triples.at( row ) ).replace( " / ", "" )
      + ".distrib.dat";
   QFile   res_f( filename );
   double  pterm    = 100.0 * positPct;
   double  bterm    = 100.0 * boundPct / (double)divsCount;
   double  bfrac;
   QString dline;

   if ( !res_f.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      return;
   }
DbgLv(1) << "WD: filename " << filename;

   // write the line-fit variables for each division
   QTextStream ts( &res_f );
   ts << tr( "%Boundary: Points:       Slope:   Intercept:"
         "       Sigma: Correlation:\n" );

   for ( int jj = 0; jj < divsCount; jj++ )
   {
      bfrac     = pterm + bterm * (double)( jj + 1 );

      dline.sprintf( "%9.2f %7d %12.6f %12.6f %12.6f %12.6f\n", bfrac,
         dpnts.at( jj ), dslos.at( jj ), dseds.at( jj ),
         dsigs.at( jj ), dcors.at( jj ) );
      ts << dline;
   }

   res_f.close();
}

// write a file of vHW detailed division group model data
void US_vHW_Enhanced::write_model()
{
   QString filename = US_Settings::resultDir() + "/" + d->runID + "/vHW."
      + QString( triples.at( row ) ).replace( " / ", "" )
      + ".fef_model.rpt";
   QFile   res_f( filename );
   int     groups   = groupdat.size();

   if ( !res_f.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      return;
   }
DbgLv(1) << "WM: filename " << filename;

   QTextStream ts( &res_f );

   ts <<     "*************************************\n";
   ts << tr( "*   Please do not edit this file!   *\n" );
   ts <<     "*************************************\n\n\n";
   ts << "3            " << tr( "\t# Fixed Molecular Weight Distribution\n" );
   ts << groups << "           " << tr( "\t# Number of Components\n" );
   ts << d->meniscus << "       " << tr( "\t# Meniscus in cm\n" );
   ts << "0.01         " << tr( "\t# Meniscus range in cm\n" );
   ts << "0            " << tr( "\t# Meniscus fitting control\n" );
   ts << baseline << "     " << tr( "\t# Baseline in OD\n" );
   ts << "0.01         " << tr( "\t# Baseline range in OD\n" );
   ts << "1            " << tr( "\t# Baseline fitting control\n" );
   ts << "0.0000       " << tr( "\t# Slope(r) Correction in OD\n" );
   ts << "0.0000       " << tr( "\t# Slope(r) Correction range in OD\n" );
   ts << "0            " << tr( "\t# Slope(r) Correction fitting control\n" );
   ts << "0.0000       " << tr( "\t# Slope(c,r) Correction in OD\n" );
   ts << "0.0000       " << tr( "\t# Slope(c,r) Correction range in OD\n" );
   ts << "0            " << tr( "\t# Slope(c,r) Correction fitting control\n");
   ts << "20           " << tr( "\t# Delta_t in seconds\n" );
   ts << "0.001        " << tr( "\t# Delta_t in cm\n" );

   QString line04 = "1            \t" +
      tr( "# Sedimentation Coefficient fitting control\n" );
   QString line05 = "7.00e-07     \t" +
      tr( "# Diffusion Coefficient in D\n" );
   QString line06 = "7.00e-08     \t" +
      tr( "# Diffusion Coefficient range in D\n" );
   QString line07 = "1            \t" +
      tr( "# Diffusion Coefficient fitting control\n" );
   QString line10 = "0            \t" +
      tr( "# Partial Concentration fitting control\n" );
   QString line11 = "0.0000       \t" + tr( "# Sigma\n" );
   QString line12 = "0.0000       \t" + tr( "# Sigma range\n" );
   QString line13 = "0            \t" + tr( "# Sigma fitting control\n" );
   QString line14 = "0.0000       \t" + tr( "# Delta\n" );
   QString line15 = "0.0000       \t" + tr( "# Delta range\n" );
   QString line16 = "0            \t" + tr( "# Delta fitting control\n" );
   QString line17 = "1            \t" +
      tr( "# Molecular Weight fitting control\n" );
   QString line18 = "0            \t" +
      tr( "# Part. Spec. Volume fitting control\n" );
   double  cterm  = ( C0 - baseline ) / 100.0;

   for ( int ii = 0; ii < groups; ii++ )
   {
      double gsed = groupdat.at( ii ).sed * 1.0e-13;
      double gcon = groupdat.at( ii ).percent * cterm;
      ts << "\n";
      ts << tr( "Parameters for Component " ) << ( ii + 1 ) << ":\n\n";
      ts << gsed << "  "
         << tr( "\t# Sedimentation Coefficient in s\n" );
      ts << gsed / 10.0 << "  "
         << tr( "\t# Sedimentation Coefficient range in s\n" );
      ts << line04 << line05 << line06 << line07;
      ts << gcon << "    "
         << tr( "\t# Partial Concentration in OD\n" );
      ts << gcon / 10.0 << "   "
         << tr( "\t# Partial Concentration range in OD\n" );
      ts << line10 << line11 << line12 << line13;
      ts << line14 << line15 << line16 << line17 << line18;
   }

   res_f.close();
}

// text of minutes,seconds or hours,minutes for a given total seconds value
QString US_vHW_Enhanced::text_time( double seconds, int type )
{
   int mins = (int)( seconds / 60.0 );
   int secs = (int)( seconds - (double)mins * 60.0 );

   if ( type == 0 )
   {  // fixed-field mins,secs text
      QString tmin = QString().sprintf( "%4d", mins );
      QString tsec = QString().sprintf( "%3d", secs );
      return  tr( "%1 min %2 sec" ).arg( tmin ).arg( tsec );
   }

   else if ( type == 1 )
   {  // minutes,seconds text
      return  tr( "%1 minutes(s) %2 second(s)" ).arg( mins ).arg( secs );
   }

   else
   {  // hours,minutes text
      int hrs  = (int)( seconds / 3600.0 );
      mins     = qRound( ( seconds - (double)hrs * 3600.0 ) / 60.0 );
      return  tr( "%1 hour(s) %2 minute(s)" ).arg( hrs ).arg( mins );
   }
}

// text of minutes and seconds for a given total seconds value
QString US_vHW_Enhanced::text_time( double seconds )
{  // default mins,secs text
   return  text_time( seconds, 0 );
}


// pare down files list by including only the last-edit versions
QStringList US_vHW_Enhanced::last_edit_files( QStringList files )
{
   QStringList ofiles;
   QStringList part;
   QString     file;
   QString     test;
   QString     pfile;
   QString     ptest;
   int         nfi   = files.size();

   // if only one in list, we need do no more
   if ( nfi < 2 )
   {
      return files;
   }

   // make sure files list is in ascending alphabetical order
   files.sort();

   // get first file name and its non-editID parts
   file    = files[ 0 ];
   part    = file.split( "." );
   test    = part[ 0 ] + part[ 3 ] + part[ 4 ] + part[ 5 ];

   // skip all but last of any duplicates (differ only in editID)
   for ( int ii = 1; ii < nfi; ii++ )
   {
      pfile   = file;
      ptest   = test;
      file    = files[ ii ];
      part    = file.split( "." );
      test    = part[ 0 ] + part[ 3 ] + part[ 4 ] + part[ 5 ];

      if ( QString::compare( test, ptest ) != 0 )
      {  // differs by more than just edit, so output previous
         ofiles.append( pfile );
      }
   }

   // output the final
   ofiles.append( file );

   return ofiles;
}

// get the data readings radius value at a given index
//   ( needed because the location in structures of radius values
//     is changing to a single vector for all scans )
double US_vHW_Enhanced::readings_radius( US_DataIO2::EditedData* d, int index )
{
   return d->x[ index ].radius;
}

void US_vHW_Enhanced::new_triple( int row )
{
   US_AnalysisBase2::new_triple( row );
   haveZone   = false;
}

void US_vHW_Enhanced::update( int row )
{
   haveZone   = false;
   d          = &dataList[ row ];

   US_AnalysisBase2::update( row );
}

