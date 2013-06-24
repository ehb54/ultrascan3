//! \file us_rpscan.cpp

#include "us_rpscan.h"
#include "us_pcsa.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_math2.h"

#include <qwt_double_interval.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>

// Constructor:  Regularization Parameter Scan widget
US_RpScan::US_RpScan( QList< US_SolveSim::DataSet* >&dsets,
      ModelRecord& mr, QWidget* p )
   : US_WidgetsDialog( p, 0 ), dsets( dsets ), mrec( mr )
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
   ct_stralpha  = us_counter( 3, 0.000, 1.000, 0.020 );
   ct_endalpha  = us_counter( 3, 0.000, 1.000, 0.800 );
   ct_incalpha  = us_counter( 3, 0.000, 0.200, 0.020 );
   le_selalpha  = us_lineedit( QString::number( alpha ), -1, false );
   ct_stralpha->setStep( 0.001 );
   ct_endalpha->setStep( 0.001 );
   ct_incalpha->setStep( 0.001 );

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
   pltctrlsLayout->addWidget( le_selalpha, row++, 3, 1, 3 );
   pltctrlsLayout->addWidget( pb_scan,     row++, 0, 1, 6 );
   pltctrlsLayout->addWidget( lb_status,   row,   0, 1, 1 );
   pltctrlsLayout->addWidget( b_progress,  row++, 1, 1, 5 );
   pltctrlsLayout->addWidget( le_stattext, row++, 0, 1, 6 );
//   row         += 7;
   QString ctype  = tr( "Increasing Sigmoid" );
   if ( mrec.str_k == mrec.par1 )
      ctype          = tr( "Straight Line" );
   else if ( mrec.str_k > mrec.end_k )
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
   pick->setRubberBand  ( QwtPicker::VLineRubberBand );
   pick->setMousePattern( QwtEventPattern::MouseSelect1,
                          Qt::LeftButton, Qt::ControlModifier );

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
DbgLv(1) << "TRP:  mrec.str_k" << mrec.str_k;
DbgLv(1) << "TRP:  mrec.end_k" << mrec.end_k;
DbgLv(1) << "TRP:  mrec.par1 " << mrec.par1;
DbgLv(1) << "TRP:  mrec.par2 " << mrec.par2;
DbgLv(1) << "TRP:  mrec.vari " << mrec.variance;
DbgLv(1) << "TRP:  mrec.rmsd " << mrec.rmsd;
DbgLv(1) << "TRP:  mrec.isolutes.size" << mrec.isolutes.size();
DbgLv(1) << "TRP:  mrec.csolutes.size" << mrec.csolutes.size();
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

// Get the selected alpha value
double US_RpScan::get_alpha()
{
   return alpha;
}

// Scan alphas
void US_RpScan::scan()
{
   double calpha = ct_stralpha->value();
   double ealpha = ct_endalpha->value();
   double dalpha = ct_incalpha->value();
   ealpha       += dalpha * 0.1;
   alphas.clear();
   varias.clear();
   xnorms.clear();
   b_progress->reset();
   nalpha        = qRound( ( ealpha - calpha ) / dalpha ) + 1;
   b_progress->setMaximum( nalpha );
   int jalpha    = 0;

   while( calpha <= ealpha )
   {
      alphas << calpha;

      US_SolveSim::Simulation sim_vals;
      sim_vals.alpha     = calpha;
      sim_vals.noisflag  = 0;
      sim_vals.dbg_level = dbg_level;
      sim_vals.solutes   = mrec.isolutes;

      US_SolveSim* solvesim = new US_SolveSim( dsets, 0, false );

      solvesim->calc_residuals( 0, 1, sim_vals );

      varias << sim_vals.variances[ 0 ] * 1e5;
      xnorms << sim_vals.xnormsq;

      b_progress->setValue( ++jalpha );

      QString astat      = tr( "Alpha %1  --  of %2 alphas, number %3" )
         .arg( calpha ).arg( nalpha ).arg( jalpha );
      le_stattext->setText( astat );

      calpha += dalpha;
   }

   nalpha        = alphas.size();
for(int jj=0; jj<nalpha; jj++ )
 DbgLv(1) << "a v x" << alphas[jj] << varias[jj] << xnorms[jj];

   plot_data();

}

// Plot the data
void US_RpScan::plot_data()
{
DbgLv(1) << "TRP:  plot_data IN";
   data_plot1->detachItems();
DbgLv(1) << "TRP:  plot_data items detached";

   grid          = us_grid( data_plot1 );

   QwtPlotCurve* curvln;
   QwtPlotCurve* curvpt;

   double* xx    = varias.data();
   double* yy    = xnorms.data();
   QPen    pen_plot( US_GuiSettings::plotCurve(), 1 );

   // Draw the variance,xnorm line
   curvln        = us_curve( data_plot1, tr( "Curve V-X" ) );
   curvln->setPen( pen_plot );
   curvln->setData( xx, yy, nalpha );

   // Show the variance,xnorm points
   curvpt        = us_curve( data_plot1, tr( "Alpha Points" ) );
   QwtSymbol sym;
   sym.setStyle( QwtSymbol::Ellipse );
   sym.setPen  ( QPen( Qt::blue ) );
   sym.setBrush( QBrush( Qt::white ) );
   sym.setSize ( 8 );
   curvpt->setStyle( QwtPlotCurve::NoCurve );
   curvpt->setSymbol( sym );
   curvpt->setData( xx, yy, nalpha );

   // Compute and show lines that hint at elbow point of line
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

      // Compute and plot a line fitted to the first few main curve points
      US_Math2::linefit( &xx, &yy, &slope, &intcp, &sigma, &corre, 5 );
      xl1p1     = xx[ 0 ];
      yl1p1     = yy[ 0 ];
      yl1p2     = yy[ nalpha - 5 ];
      xl1p2     = xl1p1 + ( yl1p2 - yl1p1 ) / slope;
      curvh1        = us_curve( data_plot1, tr( "Curve Hint 1" ) );
      curvh1->setPen( pen_red );
      xh[ 0 ]   = xl1p1;    // Line fitted to first few points
      xh[ 1 ]   = xl1p2;
      yh[ 0 ]   = yl1p1;
      yh[ 1 ]   = yl1p2;
      curvh1->setData( xh, yh, 2 );

      // Compute and plot a line fitted to the last few main curve points
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
      yl2p1     = ye[ 0 ];
      xl2p2     = xx[ 4 ];
      yl2p2     = yl2p1 + ( xl2p2 - xl2p1 ) * slop2;
DbgLv(1) << "TRP:H2:   l2: x1,y1,x2,y2" << xl2p1 << yl2p1 << xl2p2 << yl2p2;
      curvh2        = us_curve( data_plot1, tr( "Curve Hint 2" ) );
      curvh2->setPen( pen_red );
      xh[ 0 ]   = xl2p1;    // Line fitted to last few points
      xh[ 1 ]   = xl2p2;
      yh[ 0 ]   = yl2p1;
      yh[ 1 ]   = yl2p2;
      curvh2->setData( xh, yh, 2 );

      // Find the intersection point for 2 fitted lines
      xl3p1     = ( intc2 - intcp ) / ( slope - slop2 );
      yl3p1     = xl3p1 * slope + intcp;
DbgLv(1) << "TRP:H3:   l3: x1,y1" << xl3p1 << yl3p1;

      // Find the curve point nearest to intersection point
      //  then draw a line from intersection to nearest point.
      double xrng = qAbs( xx[ 0 ] - xx[ nalpha - 1 ] );
      double yrng = qAbs( yy[ 0 ] - yy[ nalpha - 1 ] );
      double xdif = ( xx[ 0 ] - xl3p1 ) / xrng;
      double ydif = ( yy[ 0 ] - yl3p1 ) / yrng;
      double dmin = sqrt( sq( xdif ) + sq( ydif ) );
DbgLv(1) << "TRP:H2: x1,x2,y1,y2" << xl1p1 << xl2p1 << yl2p1 << yl1p1
   << "init.dmin" << dmin;
      int    jmin = nalpha / 2;
      for ( int jj = 3; jj < nalpha - 3; jj++ )
      {  // Find the curve point nearest to intersection point
         xdif        = ( xx[ jj ] - xl3p1 ) / xrng;
         ydif        = ( yy[ jj ] - yl3p1 ) / yrng;
         double dval = sqrt( sq( xdif ) + sq( ydif ) );
         if ( dval < dmin )
         {
            jmin     = jj;
            dmin     = dval;
DbgLv(1) << "TRP:H2:    dval dmin jmin" << dval << dmin << jmin;
         }
      }
      xl3p2     = xx[ jmin ];
      yl3p2     = yy[ jmin ];
      curvh3        = us_curve( data_plot1, tr( "Curve Hint 3" ) );
      curvh3->setPen( pen_cyan );
      xh[ 0 ]   = xl3p1;    // Line between hint intersection and main curve
      xh[ 1 ]   = xl3p2;
      yh[ 0 ]   = yl3p1;
      yh[ 1 ]   = yl3p2;
      curvh3->setData( xh, yh, 2 );

      // Use nearest curve point for default alpha
      alpha     = alphas[ jmin ];
      le_selalpha->setText( QString().sprintf( "%.3f", alpha ) );
   }

   connect( pick, SIGNAL( cMouseUp( const QwtDoublePoint& ) ),
            this, SLOT  ( mouse   ( const QwtDoublePoint& ) ) );

   data_plot1->replot();
}

// Handle a mouse click at a horizontal location
void US_RpScan::mouse( const QwtDoublePoint& p )
{
   double xloc = p.x();
   double xmin = xloc;

   if ( v_line != NULL )
   {
      v_line->detach();
      delete v_line;
      v_line = NULL;
   }

   for ( int jj = 0; jj < nalpha; jj++ )
   {
      double xdif = qAbs( xloc - varias[ jj ] );

      if ( xdif < xmin )
      {
         xmin        = xdif;
         alpha       = alphas[ jj ];
      }
   }

   QString salpha = QString().sprintf( "%.3f", alpha );
   le_selalpha->setText( salpha );

   draw_vline( xloc, salpha );
}

// Draw a vertical pick line
void US_RpScan::draw_vline( double xval, QString salpha )
{
   QwtScaleDiv* y_axis = data_plot1->axisScaleDiv( QwtPlot::yLeft );
   double ymin    = y_axis->lowerBound();
   double ymax    = y_axis->upperBound();
   double padding = ( ymax - ymin ) / 10.0;
   double ymark   = ymin + padding / 2.0;
   double xx[ 2 ];
   double yy[ 2 ];

   xx[ 0 ] = xval;
   xx[ 1 ] = xval;
   yy[ 0 ] = ymax - padding;
   yy[ 1 ] = ymin + padding;

   v_line  = us_curve( data_plot1, "V-Line" );
   v_line->setData( xx, yy, 2 );

   QPen pen = QPen( QBrush( Qt::white ), 2.0 );
   v_line->setPen( pen );

   data_plot1->detachItems( QwtPlotItem::Rtti_PlotMarker );
   QwtPlotMarker* marker = new QwtPlotMarker;
   QwtText label;
   label.setText ( salpha );
   label.setColor( Qt::cyan );
   marker->setValue( xval, ymark );
   marker->setLabel( label );
   marker->setLabelAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
   marker->attach  ( data_plot1 );

   data_plot1->replot();
}

