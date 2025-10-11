//! \file us_rpscan.cpp

#include "us_rpscan.h"
#include "us_pcsa.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_math2.h"
#include "us_sleep.h"

#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#if QT_VERSION < 0x050000
#define setSamples(a,b,c)  setData(a,b,c)
#define setSymbol(a)       setSymbol(*a)
#endif

// Constructor:  Regularization Parameter Scan widget
US_RpScan::US_RpScan( QList< US_SolveSim::DataSet* >&dsets,
      US_ModelRecord& mr, int& thr, double& alf, QWidget* p )
   : US_WidgetsDialog( p, 0 ), dsets( dsets ), mrec( mr ), nthr( thr ),
   alpha( alf )
{
   alpha           = 0.0;

DbgLv(1) << "TRP:  RpScan IN";
   // lay out the GUI
   setObjectName( "US_RpScan" );
   setAttribute( Qt::WA_DeleteOnClose, false );
   setWindowTitle( tr( "Regularization Parameter (Alpha) Scan" ) );
   setPalette( US_GuiSettings::frameColor() );

   QSize p1size( 560, 480 );

   dbg_level       = US_Settings::us_debug();
   v_line          = NULL;

   mainLayout      = new QHBoxLayout( this );
   leftLayout      = new QVBoxLayout();
   rightLayout     = new QVBoxLayout();
   pltctrlsLayout  = new QGridLayout();
   buttonsLayout   = new QHBoxLayout();

   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   QLabel* lb_datctrls    = us_banner( tr( "PCSA Model Parameters" ) );
   QLabel* lb_mtype       = us_label(  tr( "Model Type:" ) );
   QLabel* lb_npoints     = us_label(  tr( "Points per Line:" ) );
   QLabel* lb_mdlpar1     = us_label(  tr( "Best Model Par 1:" ) );
   QLabel* lb_mdlpar2     = us_label(  tr( "Best Model Par 2:" ) );
   QLabel* lb_scnctrls    = us_banner( tr( "Alpha Scan Parameters" ) );
   QLabel* lb_stralpha    = us_label(  tr( "Starting Alpha:" ) );
   QLabel* lb_endalpha    = us_label(  tr( "Ending Alpha:" ) );
   QLabel* lb_incalpha    = us_label(  tr( "Alpha Increment:" ) );
   QLabel* lb_selalpha    = us_label(  tr( "Selected Alpha:" ) );
   QLabel* lb_status      = us_label(  tr( "Status:" ) );

   QPushButton* pb_scan   = us_pushbutton( tr( "Start Scan" ) );
   QPushButton* pb_help   = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_accept = us_pushbutton( tr( "Accept" ) );
   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );

   int nppln    = mrec.isolutes.size();
DbgLv(1) << "TRP:  nppln" << nppln;
   // lay out the GUI
   double par1  = mrec.par1;
   double par2  = mrec.par2;
   le_mtype     = us_lineedit( tr( "Straight Line" ),    -1, true );
   le_npoints   = us_lineedit( QString::number( nppln ), -1, true );
   le_mdlpar1   = us_lineedit( QString::number( par1  ), -1, true );
   le_mdlpar2   = us_lineedit( QString::number( par2  ), -1, true );
   ct_stralpha  = us_counter( 3, 0.000, 1000.0, 0.020 );
   ct_endalpha  = us_counter( 3, 0.000, 1000.0, 0.800 );
   ct_incalpha  = us_counter( 3, 0.000, 1000.0, 0.020 );
   le_selalpha  = us_lineedit( QString::number( alpha ), -1, false );
   ct_stralpha->setSingleStep( 0.001 );
   ct_endalpha->setSingleStep( 0.001 );
   ct_incalpha->setSingleStep( 0.001 );

   b_progress   = us_progressBar( 0, 100, 0 );
   le_stattext  = us_lineedit( "", -1, true );

   // Adjust the size of line counters and rmsd text
   lb_mtype ->adjustSize();
   QFont        font( US_GuiSettings::fontFamily(),
                      US_GuiSettings::fontSize() );
   QFontMetrics fmet( font );
   int  fwidth   = fmet.maxWidth();
   int  rheight  = lb_mtype->height();
   int  cminw    = fwidth * 9;
   int  csizw    = cminw + fwidth;
   ct_stralpha->resize( csizw, rheight );
   ct_endalpha->resize( csizw, rheight );
   ct_incalpha->resize( csizw, rheight );
   ct_stralpha->setMinimumWidth( cminw );
   ct_endalpha->setMinimumWidth( cminw );
   ct_incalpha->setMinimumWidth( cminw );
DbgLv(1) << "TRP:  csizw cminw" << csizw << cminw;

   // Add elements to the controls layout
   int  row      = 0;
   pltctrlsLayout->addWidget( lb_datctrls, row++, 0, 1, 6 );
   pltctrlsLayout->addWidget( lb_mtype,    row,   0, 1, 3 );
   pltctrlsLayout->addWidget( le_mtype,    row++, 3, 1, 3 );
   pltctrlsLayout->addWidget( lb_npoints,  row,   0, 1, 3 );
   pltctrlsLayout->addWidget( le_npoints,  row++, 3, 1, 3 );
   pltctrlsLayout->addWidget( lb_mdlpar1,  row,   0, 1, 3 );
   pltctrlsLayout->addWidget( le_mdlpar1,  row++, 3, 1, 3 );
   pltctrlsLayout->addWidget( lb_mdlpar2,  row,   0, 1, 3 );
   pltctrlsLayout->addWidget( le_mdlpar2,  row++, 3, 1, 3 );
   pltctrlsLayout->addWidget( lb_scnctrls, row++, 0, 1, 6 );
   pltctrlsLayout->addWidget( lb_stralpha, row,   0, 1, 3 );
   pltctrlsLayout->addWidget( ct_stralpha, row++, 3, 1, 3 );
   pltctrlsLayout->addWidget( lb_endalpha, row,   0, 1, 3 );
   pltctrlsLayout->addWidget( ct_endalpha, row++, 3, 1, 3 );
   pltctrlsLayout->addWidget( lb_incalpha, row,   0, 1, 3 );
   pltctrlsLayout->addWidget( ct_incalpha, row++, 3, 1, 3 );
   pltctrlsLayout->addWidget( lb_selalpha, row,   0, 1, 3 );
   pltctrlsLayout->addWidget( le_selalpha, row,   3, 1, 2 );
   pltctrlsLayout->addWidget( pb_scan,     row++, 5, 1, 1 );
   pltctrlsLayout->addWidget( lb_status,   row,   0, 1, 1 );
   pltctrlsLayout->addWidget( b_progress,  row++, 1, 1, 5 );
   pltctrlsLayout->addWidget( le_stattext, row++, 0, 1, 6 );
//   row         += 7;
   QString ctype  = tr( "Increasing Sigmoid" );
   if ( mrec.str_y == mrec.par1 )
   {
      ctype          = mrec.str_y != mrec.end_y 
                     ? tr( "Straight Line" )
                     : tr( "Horizontal Line" );
   }
   else if ( mrec.str_y > mrec.end_y )
      ctype          = tr( "Decreasing Sigmoid" );
   le_mtype->setText( ctype );

   buttonsLayout ->addWidget( pb_help   );
   buttonsLayout ->addWidget( pb_cancel );
   buttonsLayout ->addWidget( pb_accept );

   // Complete layouts and set up signals/slots
   plotLayout1 = new US_Plot( data_plot1,
         tr( "Alpha Scan Points" ),
         tr( "Variance (x 1e-5)" ),
         tr( "Norm of X (solute concentrations)" ) );

   data_plot1->setCanvasBackground( Qt::black );
   data_plot1->setMinimumSize( p1size );
   data_plot1->enableAxis( QwtPlot::xBottom, true );
   data_plot1->enableAxis( QwtPlot::yLeft,   true );

   pick          = new US_PlotPicker( data_plot1 );
   pick->setRubberBand  ( QwtPicker::CrossRubberBand );
   pick->setMousePattern( QwtEventPattern::MouseSelect1,
                          Qt::LeftButton );

   rightLayout->addLayout( plotLayout1    );

   leftLayout ->addLayout( pltctrlsLayout );
   leftLayout ->addStretch();
   leftLayout ->addLayout( buttonsLayout  );

   mainLayout->addLayout( leftLayout  );
   mainLayout->addLayout( rightLayout );
   mainLayout->setStretchFactor( leftLayout,  3 );
   mainLayout->setStretchFactor( rightLayout, 5 );

   connect( pb_scan,   SIGNAL( clicked()        ),
            this,      SLOT  ( scan()           ) );
   connect( pb_help,   SIGNAL( clicked()        ),
            this,      SLOT  ( help()           ) );
   connect( pb_cancel, SIGNAL( clicked()        ),
            this,      SLOT  ( reject_it()      ) );
   connect( pb_accept, SIGNAL( clicked()        ),
            this,      SLOT  ( accept_it()      ) );

DbgLv(1) << "TRP:  p1size" << p1size;
   data_plot1 ->resize( p1size );
   ct_stralpha->resize( csizw, rheight );
   ct_endalpha->resize( csizw, rheight );
   ct_incalpha->resize( csizw, rheight );
   adjustSize();
DbgLv(1) << "TRP:   actual csizw" << ct_incalpha->width();
DbgLv(1) << "TRP:  mrec.taskx" << mrec.taskx;
DbgLv(1) << "TRP:  mrec.str_y" << mrec.str_y;
DbgLv(1) << "TRP:  mrec.end_y" << mrec.end_y;
DbgLv(1) << "TRP:  mrec.par1 " << mrec.par1;
DbgLv(1) << "TRP:  mrec.par2 " << mrec.par2;
DbgLv(1) << "TRP:  mrec.vari " << mrec.variance;
DbgLv(1) << "TRP:  mrec.rmsd " << mrec.rmsd;
DbgLv(1) << "TRP:  mrec.isolutes.size" << mrec.isolutes.size();
DbgLv(1) << "TRP:  mrec.csolutes.size" << mrec.csolutes.size();
   qApp->processEvents();
}

// Cancel button clicked
void US_RpScan::reject_it()
{
   reject();
   close();
}

// Accept button clicked
void US_RpScan::accept_it()
{
   accept();
   alpha       = le_selalpha->text().toDouble();
   close();
}

// Scan alphas
void US_RpScan::scan()
{
   QTime  timer;
   int    time_sc;
   double calpha = ct_stralpha->value();
   double ealpha = ct_endalpha->value();
   double dalpha = ct_incalpha->value();
   nalpha        = qRound( ( ealpha - calpha ) / dalpha ) + 1;
   alphas.clear();
   varias.clear();
   xnorms.clear();
   b_progress->reset();
   b_progress->setRange( 1, nalpha );
   double varmx  = 0.0;
   double xnomx  = 0.0;
   double v_vari = 0.0;
   double v_xnsq = 0.0;
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   le_stattext->setText( tr( "Beginning Alpha Scan ..." ) );
   timer.start();              // Start a timer to measure run time
   qApp->processEvents();

   for ( int ja = 0; ja < nalpha; ja++ )
   {  // Loop to populate alpha scan arrays with preliminary values
      alphas << calpha;
      varias << v_vari;
      xnorms << v_xnsq;
      calpha += dalpha;
   }

int kalpha=nalpha;
   nalpha        = alphas.size();
DbgLv(1) << "ASC:  nalpha" << nalpha << kalpha << "nthr" << nthr;

   if ( nthr == 1 )
   {  // Non-multi-threaded scan
      QVector< double > csolutes;
      sv_nnls_a.clear();
      sv_nnls_b.clear();
      int    nscans      = dsets[ 0 ]->run_data.scanCount();
      int    npoints     = dsets[ 0 ]->run_data.pointCount();
      int    nisols      = mrec.isolutes.size();
      le_stattext->setText( tr( "Beginning Single-Thread Alpha Scan ..." ) );

      for ( int ja = 0; ja < nalpha; ja++ )
      {  // Loop to evaluate each alpha in the specified range
         calpha             = alphas[ ja ];

         if ( ja == 0 )
         {  // For the first one do a full compute and save the A,B matrices
            US_SolveSim::Simulation sim_vals;
            sim_vals.alpha     = calpha;
            sim_vals.noisflag  = 0;
            sim_vals.dbg_level = 0;
            sim_vals.zsolutes  = mrec.isolutes;

            US_SolveSim* solvesim = new US_SolveSim( dsets, 0, false );

            solvesim->calc_residuals( 0, 1, sim_vals, true,
                  &sv_nnls_a, &sv_nnls_b );

            v_vari             = sim_vals.variance;
            v_xnsq             = sim_vals.xnormsq;
         }

         else
         {  // After 1st, regularize by modifying A matrix diagonal, then NNLS
            apply_alpha( calpha, &sv_nnls_a, &sv_nnls_b,
                         nscans, npoints, nisols, v_vari, v_xnsq );
         }

         varias[ ja ]       = v_vari;
         xnorms[ ja ]       = v_xnsq;
         int kalf           = ja + 1;

         b_progress->setValue( kalf );

         varmx              = qMax( varmx, v_vari );
         xnomx              = qMax( xnomx, v_xnsq );
         QString astat      = tr( "Of %1 models, computed %2 (Alpha %3)" )
            .arg( nalpha ).arg( kalf ).arg( calpha );
         le_stattext->setText( astat );
DbgLv(1) << "a v x" << calpha << v_vari << v_xnsq;
         qApp->processEvents();
      }

      sv_nnls_a.clear();
      sv_nnls_b.clear();
   }

   else
   {  // Multi-threaded alpha scan
      le_stattext->setText( tr( "Beginning %1-Thread Alpha Scan ..." )
                            .arg( nthr ) );
      qApp->processEvents();
      wthreads .clear();
      sv_nnls_a.clear();
      sv_nnls_b.clear();

      // Run a complete model computation to get the A,B matrices
      US_SolveSim::Simulation sim_vals;
      calpha             = alphas[ 0 ];
      sim_vals.alpha     = calpha;
      sim_vals.noisflag  = 0;
      sim_vals.dbg_level = 0;
      sim_vals.zsolutes  = mrec.isolutes;

      US_SolveSim* solvesim = new US_SolveSim( dsets, 0, false );

      solvesim->calc_residuals( 0, 1, sim_vals, true, &sv_nnls_a, &sv_nnls_b );

      v_vari        = sim_vals.variance;
      v_xnsq        = sim_vals.xnormsq;
qDebug() << "A0: alpha" << calpha << "vari xnsq" << v_vari << v_xnsq
 << "ncsols" << sim_vals.zsolutes.size();;
      varias[ 0 ]   = v_vari;
      xnorms[ 0 ]   = v_xnsq;
      nasubm        = 1;
      nacomp        = 1;

      QString astat =
         tr( "Of %1 models, %2 done (Alpha %3, thread %4)" )
         .arg( nalpha ).arg( nacomp ).arg( alphas[ 0 ] ).arg( 0 );
      le_stattext->setText( astat );
      b_progress->setValue( 1 );

      for ( int jt = 0; jt < nthr; jt++ )
      {  // Submit the first tasks using available threads
         WorkPacketPc wtask;
         calpha               = alphas[ jt + 1 ];
         US_SolveSim::Simulation sim_vals;
         sim_vals.alpha       = calpha;
         sim_vals.noisflag    = 0;
         sim_vals.dbg_level   = qMax( 0, ( dbg_level - 1 ) );
         sim_vals.zsolutes    = mrec.isolutes;
         wtask.thrn           = jt + 1;
         wtask.taskx          = nasubm;
         wtask.str_y          = mrec.str_y;
         wtask.end_y          = mrec.end_y;
         wtask.par1           = mrec.par1;
         wtask.par2           = mrec.par2;
         wtask.sim_vals       = sim_vals;
         wtask.isolutes       = sim_vals.zsolutes;
         wtask.dsets          = dsets;
         wtask.depth          = 1;
         wtask.state          = 0;
         wtask.noisf          = sim_vals.noisflag;
         wtask.psv_nnls_a     = &sv_nnls_a;
         wtask.psv_nnls_b     = &sv_nnls_b;
         wtask.csolutes.clear();
DbgLv(1) << "ASC:   jt" << jt << "alpha" << calpha;

         WorkerThreadPc* wthr   = new WorkerThreadPc( this );

         wthr->define_work( wtask );
         wthreads << wthr;
         connect( wthr, SIGNAL( work_complete( WorkerThreadPc* ) ),
                  this, SLOT(   process_job(   WorkerThreadPc* ) ) );
         wthr->start();
         nasubm++;
DbgLv(1) << "ASC:      defined: nasubm" << nasubm;
         qApp->processEvents();
      }

      // Keep testing to see if all tasks are complete
      while( nacomp < nalpha )
      {
         US_Sleep::msleep( 500 );
         qApp->processEvents();
      }

      // Once all threads are done, determine max variance,norm
      for ( int ja = 0; ja < nalpha; ja++ )
      {
         varmx              = qMax( varmx, varias[ ja ] );
         xnomx              = qMax( xnomx, xnorms[ ja ] );
      }

      // Delete worker threads and free saved A,B matrices
      for ( int jt = 0; jt < nthr; jt++ )
         delete wthreads[ jt ];

      sv_nnls_a.clear();
      sv_nnls_b.clear();
   }

   lgv           = 0  -(int)qFloor( log10( varmx ) );
   lgx           = -1 -(int)qFloor( log10( xnomx ) );
   vscl          = qPow( 10.0, lgv );
   xscl          = qPow( 10.0, lgx );
DbgLv(1) << "Log-varia Log-xnorm" << lgv << lgx << "vscl xscl" << vscl << xscl;

   for ( int ja = 0; ja < nalpha; ja++ )
   {  // Scale the variance,normsq points
      varias[ ja ] *= vscl;
      xnorms[ ja ] *= xscl;
   }

   b_progress->setValue( nalpha );
   QApplication::restoreOverrideCursor();
   qApp->processEvents();
   // Determine elapsed time
   time_sc    = timer.elapsed();
   int ktimes = ( time_sc + 500 ) / 1000;
   int ktimeh = ktimes / 3600;
   int ktimem = ( ktimes - ktimeh * 3600 ) / 60;
   ktimes     = ktimes - ktimeh * 3600 - ktimem * 60;

   // Compose and display completed-scan status message
   QString astat = tr( "%1 alphas evaluated in" ).arg( nalpha );

   if ( ktimeh > 0 )
      astat     += tr( " %1 hr., %2 min., %3 sec." )
         .arg( ktimeh ).arg( ktimem ).arg( ktimes );

   else
      astat     += tr( " %1 min., %2 sec." ).arg( ktimem ).arg( ktimes );

   astat     += tr( ", in %1 threads" ).arg( nthr );
   le_stattext->setText( astat );
   qApp->processEvents();

   plot_data();
}

// Plot the data
void US_RpScan::plot_data()
{
   dataPlotClear( data_plot1 );

   if ( nalpha > 1 )
   {
      int    je     = nalpha - 1;
      double ainc   = alphas[ 1 ] - alphas[ 0 ];
      data_plot1->setTitle(
            tr( "Alpha Scan Points\nAlphas from %1 to %2 by %3" )
            .arg( alphas[ 0 ] ).arg( alphas[ je ] ).arg( ainc ) );
   }
   else
   {
      data_plot1->setTitle( tr( "Alpha Scan Points" ) );
   }

   data_plot1->setAxisTitle( QwtPlot::xBottom,
         tr( "Variance (x 1e%1)" ).arg( lgv ) ),
   data_plot1->setAxisTitle( QwtPlot::yLeft,
         ( lgx == 0 ) ?
         tr( "Norm of X (solute concentrations)" ) :
         tr( "Norm of X (solute concentrations x 1e%1)" ).arg( lgx ) );

   grid          = us_grid( data_plot1 );

   QwtPlotCurve* curvln;
   QwtPlotCurve* curvpt;

   double* xx    = varias.data();
   double* yy    = xnorms.data();
   QPen    pen_plot( US_GuiSettings::plotCurve(), 1 );

   // Draw the variance,xnorm line
   curvln        = us_curve( data_plot1, tr( "Curve V-X" ) );
   curvln->setPen    ( pen_plot );
   curvln->setSamples( xx, yy, nalpha );

   // Show the variance,xnorm points
   curvpt        = us_curve( data_plot1, tr( "Alpha Points" ) );
   QwtSymbol* sym = new QwtSymbol;
   sym->setStyle( QwtSymbol::Ellipse );
   sym->setPen  ( QPen( Qt::blue ) );
   sym->setBrush( QBrush( Qt::white ) );
   sym->setSize ( 8 );
   curvpt->setStyle  ( QwtPlotCurve::NoCurve );
   curvpt->setSymbol ( sym );
   curvpt->setSamples( xx, yy, nalpha );

   // Compute and show lines that hint at the elbow point of the curve
   if ( nalpha > 6 )
   {
      double  xh[ 2 ];
      double  yh[ 2 ];
      double  xa[ 5 ];
      double  ya[ 5 ];
      double* xe = (double*)xa;
      double* ye = (double*)ya;
      QwtPlotCurve* curvh1;
      QwtPlotCurve* curvh2;
      QwtPlotCurve* curvh3;
      double  slope; double slop2;
      double  intcp; double intc2;
      double  sigma; double  corre;
      double  xl1p1; double xl1p2; double yl1p1; double yl1p2;
      double  xl2p1; double xl2p2; double yl2p1; double yl2p2;
      double  xl3p1; double xl3p2; double yl3p1; double yl3p2;
      QPen    pen_red ( Qt::red,  0, Qt::DashLine );
      QPen    pen_cyan( Qt::cyan, 0, Qt::DashLine );

      // Compute a line fitted to the first few main curve points
      int  nlp  = 5;

      while ( nlp < nalpha )
      {
         double avg  = US_Math2::linefit( &xx, &yy, &slope, &intcp,
                                          &sigma, &corre, nlp );
DbgLv(1) << "TRP:H1:  avg" << avg << "nlp" << nlp;
DbgLv(1) << "TRP:H1:   sl" << slope << "in" << intcp << "sg" << sigma
   << "co" << corre;
         if ( slope < 1e99  &&  slope > (-1e99) )
            break;

         nlp      += 2;
      }

DbgLv(1) << "TRP:H1:  intcp slope" << intcp << slope;
      yl1p1     = yy[ 0 ];
      xl1p1     = ( slope == 0.0 ) ? xx[ 0 ] : ( yl1p1 - intcp ) / slope;
      yl1p2     = yy[ nalpha - 5 ];
      xl1p2     = ( slope == 0.0 ) ? xl1p1 : ( yl1p2 - intcp ) / slope;
DbgLv(1) << "TRP:H1:   l1: x1,y1,x2,y2" << xl1p1 << yl1p1 << xl1p2 << yl1p2;

      // Compute a line fitted to the last few main curve points
      int je    = nalpha - 1;
      for ( int jj = 0; jj < 5; jj++, je-- )
      {
         xe[ jj ]  = xx[ je ];
         ye[ jj ]  = yy[ je ];
DbgLv(1) << "TRP:H2: jj x y" << jj << xe[jj] << ye[jj] << "je" << je;
      }
      US_Math2::linefit( &xe, &ye, &slop2, &intc2, &sigma, &corre, 5 );
DbgLv(1) << "TRP:H2:  intcp slope" << intc2 << slop2;
      xl2p1     = xe[ 0 ];
      yl2p1     = slop2 * xl2p1 + intc2;
      xl2p2     = xx[ 4 ];
      yl2p2     = slop2 * xl2p2 + intc2;
DbgLv(1) << "TRP:H2:   l2: x1,y1,x2,y2" << xl2p1 << yl2p1 << xl2p2 << yl2p2;

      // Find the intersection point for the 2 fitted lines
      US_Math2::intersect( slope, intcp, slop2, intc2, &xl3p1, &yl3p1 );
DbgLv(1) << "TRP:H3:   l3: x1,y1" << xl3p1 << yl3p1;

      // Find the curve point nearest to the intersection point;
      //  then compute a line from intersection to nearest curve point.
      US_Math2::nearest_curve_point( xx, yy, nalpha, true, xl3p1, yl3p1,
            &xl3p2, &yl3p2, alphas.data(), &alpha );

      // Do a sanity check. If the intersection point is outside the
      // rectangle that encloses the curve, we likely have an aberrant curve.
      // So, skip plotting hint lines that are probably just confusing.
      double xcvp1  = xx[ 0 ];
      double ycvp1  = yy[ 0 ];
      double xcvp2  = xx[ je ];
      double ycvp2  = yy[ je ];
      bool do_plot  = ( xl3p1 >= xcvp1  &&  yl3p1 <= ycvp1  &&
                        xl3p1 <= xcvp2  &&  yl3p1 >= ycvp2 );
DbgLv(1) << "TRP:T4:   cv: x1,y1" << xcvp1 << ycvp1
 << "x2,y2" << xcvp2 << ycvp2 << " do_plot" << do_plot;

      if ( do_plot )
      {
         // Plot the alpha hint lines
         curvh1        = us_curve( data_plot1, tr( "Curve Hint 1" ) );
         curvh1->setPen( pen_red );
         xh[ 0 ]   = xl1p1;    // Line fitted to first few points
         yh[ 0 ]   = yl1p1;
         xh[ 1 ]   = xl3p1;
         yh[ 1 ]   = yl3p1;
         curvh1->setSamples( xh, yh, 2 );

         curvh2        = us_curve( data_plot1, tr( "Curve Hint 2" ) );
         curvh2->setPen( pen_red );
         xh[ 0 ]   = xl2p1;    // Line fitted to last few points
         yh[ 0 ]   = yl2p1;
         xh[ 1 ]   = xl3p1;
         yh[ 1 ]   = yl3p1;
         curvh2->setSamples( xh, yh, 2 );

         curvh3        = us_curve( data_plot1, tr( "Curve Hint 3" ) );
         curvh3->setPen( pen_cyan );
         xh[ 0 ]   = xl3p1;    // Line between hint intersection and main curve
         yh[ 0 ]   = yl3p1;
         xh[ 1 ]   = xl3p2;
         yh[ 1 ]   = yl3p2;
DbgLv(1) << "TRP:H3: x1,y1,x2,y2" << xl3p1 << yl3p1 << yl3p2 << yl3p2;
         curvh3->setSamples( xh, yh, 2 );
      }

      else   // If no good intersection point, default the half-way alpha
         alpha     = alphas[ nalpha / 2 ];

      // Use nearest curve point for default alpha
      le_selalpha->setText( QString::asprintf( "%.3f", alpha ) );
   }

   connect( pick, SIGNAL( cMouseUp( const QwtDoublePoint& ) ),
            this, SLOT  ( mouse   ( const QwtDoublePoint& ) ) );

   data_plot1->replot();
}

// Handle a mouse click near to a curve point location
void US_RpScan::mouse( const QwtDoublePoint& p )
{
   double xloc    = p.x();
   double yloc    = p.y();
   double xcurv;
   double ycurv;
   double xlabl;
   double ylabl;

   US_Math2::nearest_curve_point( varias.data(), xnorms.data(), nalpha, true,
         xloc, yloc, &xcurv, &ycurv, alphas.data(), &alpha );

   alpha          = (double)qRound( alpha * 1000.0 ) / 1000.0;
   QString salpha = QString::asprintf( "%.3f", alpha );
   le_selalpha->setText( salpha );

   // Mark selected curve point and give it a label
   xlabl          = xcurv + qAbs( varias[ 0 ] - varias[ nalpha - 1 ] ) / 10.0;
   ylabl          = ycurv + qAbs( xnorms[ 0 ] - xnorms[ nalpha - 1 ] ) / 10.0;

   data_plot1->detachItems( QwtPlotItem::Rtti_PlotMarker );
   QwtPlotMarker* msymbo = new QwtPlotMarker;
   QBrush sbrush( Qt::cyan );
   QPen   spen  ( sbrush, 2.0 );
   QwtSymbol* sym = new QwtSymbol( QwtSymbol::Cross, sbrush, spen, QSize( 8, 8 ) );
   msymbo->setValue( xcurv, ycurv );
   msymbo->setSymbol( sym );
   msymbo->attach  ( data_plot1 );
   QwtPlotMarker* marker = new QwtPlotMarker;
   QwtText label;
   label.setText ( salpha );
   label.setColor( Qt::cyan );
   marker->setValue( xlabl, ylabl );
   marker->setLabel( label );
   marker->setLabelAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
   marker->attach  ( data_plot1 );

   data_plot1->replot();
}

void US_RpScan::process_job( WorkerThreadPc* wthrd )
{
DbgLv(1) << "SCPJ: IN";
   WorkPacketPc wresult;
   nacomp++;                            // Bump alphas-complete count
   b_progress->setValue( nacomp );
   wthrd->get_result( wresult );        // Get results of thread task
   int    thrn     = wresult.thrn;      // Thread number of task
   int    taskx    = wresult.taskx;     // Task index of task
DbgLv(1) << "SCPJ:  thrn" << thrn << "taskx" << taskx;
   double variance = wresult.sim_vals.variance;
   double xnormsq  = wresult.sim_vals.xnormsq;
DbgLv(1) << "SCPJ:   a v x" << alphas[taskx] << variance << xnormsq;
   varias[ taskx ] = variance;
   xnorms[ taskx ] = xnormsq;
//   delete wthrd;
   QString astat      =
      tr( "Of %1 models, %2 done (Alpha %3, thread %4)" )
      .arg( nalpha ).arg( nacomp ).arg( alphas[ taskx ] ).arg( thrn );
   le_stattext->setText( astat );
DbgLv(1) << "SCPJ:    astat" << astat;

   if ( nacomp == nalpha )
   {  // We are done with all tasks
      qApp->processEvents();
DbgLv(1) << "SCPJ:    ALL-DONE";
      return;
   }

   if ( nasubm < nalpha )
   {  // We need to set up to submit another task
      WorkPacketPc wtask   = wresult;
      wtask.taskx          = nasubm;
      wtask.sim_vals.alpha = alphas[ nasubm ];
      wtask.dsets          = dsets;
      wtask.depth          = 1;
      wtask.state          = 0;
      wtask.psv_nnls_a     = &sv_nnls_a;
      wtask.psv_nnls_b     = &sv_nnls_b;
DbgLv(1) << "SCPJ:     new subm nnls_a nnls_b" << wtask.psv_nnls_a
 << wtask.psv_nnls_b << "b size" << sv_nnls_b.size();
      wtask.csolutes.clear();
DbgLv(1) << "SCPJ:     new subm taskx alpha" << nasubm << wtask.sim_vals.alpha;

//      WorkerThreadPc* wthr   = new WorkerThreadPc( this );
      wthrd->disconnect();
      WorkerThreadPc* wthr = wthrd;

      wthr->define_work( wtask );
DbgLv(1) << "SCPJ:     new subm   work defined  sv_nnls_a" << wtask.psv_nnls_b;
//      wthr->disconnect();
      connect( wthr, SIGNAL( work_complete( WorkerThreadPc* ) ),
               this, SLOT(   process_job(   WorkerThreadPc* ) ) );
      wthr->start();
      nasubm++;
DbgLv(1) << "SCPJ:     new subm   tsk started" << nasubm;
   }
   qApp->processEvents();
}

void US_RpScan::apply_alpha( const double alpha, QVector< double >* psv_nnls_a,
      QVector< double >* psv_nnls_b, const int nscans, const int npoints,
      const int nisols, double& variance, double& xnormsq )
{
   int    ntotal   = nscans * npoints;
   int    narows   = ntotal + nisols;
   int    ncsols   = 0;
          variance = 0.0;
          xnormsq  = 0.0;
   QVector< double > nnls_a = *psv_nnls_a;
   QVector< double > nnls_b = *psv_nnls_b;
   QVector< double > nnls_x;
   QVector< double > simdat;
   nnls_x.fill( 0.0, nisols );
   simdat.fill( 0.0, ntotal );
qDebug() << "AA: ns np ni na" << nscans << npoints << nisols << narows;

   // Replace alpha in the diagonal of the lower square of A
   int    dx       = ntotal;
   int    dinc     = ntotal + nisols + 1;
//qDebug() << "AA:  alf alfd" << alpha << alphad << "dx dinc" << dx << dinc;
qDebug() << "AA:  alf" << alpha << "dx dinc" << dx << dinc;

   for ( int cc = 0; cc < nisols; cc++ )
   {
      nnls_a[ dx ]    = alpha;
      dx             += dinc;
   }

   // Compute the X vector using NNLS
   US_Math2::nnls( nnls_a.data(), narows, narows, nisols,
                   nnls_b.data(), nnls_x.data() );

   // Construct the output solutes and the implied simulation and xnorm-sq
   for ( int cc = 0; cc < nisols; cc++ )
   {
      double soluval  = nnls_x[ cc ];   // Computed concentration, this solute

      if ( soluval > 0.0 )
      {
         xnormsq        += sq( soluval );
         ncsols++;
         int    aa       = cc * narows;

         for ( int kk = 0; kk < ntotal; kk++ )
         {
            simdat[ kk ]   += ( soluval * (*psv_nnls_a)[ aa++ ] );
         }
      }
   }

   // Calculate the sum for the variance computation
   for ( int kk = 0; kk < ntotal; kk++ )
   {
      variance       += sq( ( (*psv_nnls_b)[ kk ] - simdat[ kk ] ) );
   }
qDebug() << "AA:    ntotal" << ntotal << "varisum" << variance;

   // Return computed variance and xnorm-sq
   variance         /= (double)ntotal;
qDebug() << "AA: alpha" << alpha << "vari xnsq" << variance << xnormsq
 << "ncsols" << ncsols;
int mm = npoints / 2;
qDebug() << "AA: mm=" << mm << "a[m] b[m] s[m]" << (*psv_nnls_a)[mm]
   << (*psv_nnls_b)[mm] << simdat[mm];
}

