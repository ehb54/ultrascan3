//! \file us_mlplot.cpp

#include "us_mlplot.h"
#include "us_1dsa.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_math2.h"

#include <qwt_legend.h>

// constructor:  model lines plot widget
US_MLinesPlot::US_MLinesPlot( double& flo, double& fhi, double& fin,
      double& slo, double& shi, int& nlp, int& bmx, int& nkp, int &typ )
   : US_WidgetsDialog( 0, 0 ), fmin( flo ), fmax( fhi ), finc( fin ),
   smin( slo ), smax( shi ), nlpts( nlp ), bmndx( bmx ), nkpts( nkp ),
   ctype( typ )
{
   // lay out the GUI
   setWindowTitle( tr( "1-D Spectrum Analysis Model Lines Viewer" ) );
   setPalette( US_GuiSettings::frameColor() );

   QSize p1size( 560, 480 );

   dbg_level       = US_Settings::us_debug();
   model           = 0;

   mainLayout      = new QHBoxLayout( this );
   leftLayout      = new QVBoxLayout();
   rightLayout     = new QVBoxLayout();
   pltctrlsLayout  = new QGridLayout();
   buttonsLayout   = new QVBoxLayout();

   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   QLabel* lb_datctrls    = us_banner( tr( "1DSA Model Parameters" ) );
   QLabel* lb_mtype       = us_label(  tr( "Model Type:" ) );
   QLabel* lb_nlines      = us_label(  tr( "Lines (Models):" ) );
   QLabel* lb_npoints     = us_label(  tr( "Points per Line:" ) );
   QLabel* lb_kincr       = us_label(  tr( "f/f0 Increment:" ) );

   QPushButton* pb_close  = us_pushbutton( tr( "Close" ) );

   if ( ctype == 0 )
      nkpts = qRound( ( fmax - fmin ) / finc ) + 1;
   int    nline = nkpts * nkpts;
   le_mtype     = us_lineedit( tr( "Straight Line" ), -1, true );
   le_nlines    = us_lineedit( QString::number( nline ), -1, true );
   le_npoints   = us_lineedit( QString::number( nlpts ), -1, true );
   le_kincr     = us_lineedit( QString::number( finc  ), -1, true );
   if ( ctype == 1  ||  ctype == 2 )
   {
      lb_kincr->setText( tr( "Variation Count" ) );
      le_kincr->setText( QString::number( nkpts ) );
   }

   int row      = 0;
   pltctrlsLayout->addWidget( lb_datctrls, row++, 0, 1, 2 );
   pltctrlsLayout->addWidget( lb_mtype,    row,   0, 1, 1 );
   pltctrlsLayout->addWidget( le_mtype,    row++, 1, 1, 1 );
   pltctrlsLayout->addWidget( lb_nlines,   row,   0, 1, 1 );
   pltctrlsLayout->addWidget( le_nlines,   row++, 1, 1, 1 );
   pltctrlsLayout->addWidget( lb_npoints,  row,   0, 1, 1 );
   pltctrlsLayout->addWidget( le_npoints,  row++, 1, 1, 1 );
   pltctrlsLayout->addWidget( lb_kincr,    row,   0, 1, 1 );
   pltctrlsLayout->addWidget( le_kincr,    row++, 1, 1, 1 );
   if      ( ctype == 0 ) le_mtype->setText( tr( "Straight Line" ) );
   else if ( ctype == 1 ) le_mtype->setText( tr( "Increasing Sigmoid" ) );
   else if ( ctype == 2 ) le_mtype->setText( tr( "Decreasing Sigmoid" ) );

   buttonsLayout ->addWidget( pb_close );

   plotLayout1 = new US_Plot( data_plot1,
         tr( "Model Lines" ),
         tr( "Sedimentation Coefficient (x 1e13)" ),
         tr( "Frictional Ratio (f/f0)" ) );

   data_plot1->setCanvasBackground( Qt::black );
   data_plot1->setMinimumSize( p1size );

   rightLayout->addLayout( plotLayout1    );

   leftLayout ->addLayout( pltctrlsLayout );
   leftLayout ->addStretch();
   leftLayout ->addLayout( buttonsLayout  );

   mainLayout->addLayout( leftLayout  );
   mainLayout->addLayout( rightLayout );
   mainLayout->setStretchFactor( leftLayout,  2 );
   mainLayout->setStretchFactor( rightLayout, 5 );

   connect( pb_close,  SIGNAL( clicked()   ),
            this,      SLOT( close_all()   ) );

DbgLv(1) << "RP:  p1size" << p1size;

   data_plot1->resize( p1size );
}

// close button clicked
void US_MLinesPlot::close_all()
{
   close();
}

// plot the data
void US_MLinesPlot::plot_data()
{
   data_plot1->detachItems();
   data_plot1->clear();

   bool   got_best  = ( model != 0  &&  bmndx >= 0 );
DbgLv(1) << "RP:PD got_best" << got_best << "bmndx" << bmndx;

   us_grid( data_plot1 );

   QVector< double > xvec( nlpts, 0.0 );
   QVector< double > yvec( nlpts, 0.0 );
   QVector< double > bmxv( nlpts, 0.0 );
   QVector< double > bmyv( nlpts, 0.0 );

   double* xx   = xvec.data();
   double* yy   = yvec.data();
   double* bx   = bmxv.data();
   double* by   = bmyv.data();
   double  rinc = (double)( nlpts - 1 );
   double  xinc = ( smax - smin ) / rinc;
   double  ystr = fmin;
   int     mndx = 0;

   QString       title;
   QwtPlotCurve* curv;
   QPen          pen_red(  Qt::red, 3.0 );
   QPen          pen_plot( US_GuiSettings::plotCurve() );
   data_plot1->setAxisScale( QwtPlot::xBottom, smin, smax, 1.0 );
   double  y1    = fmin;
   double  y2    = fmax;

   if ( ctype == 0 )
   { // Generate and plot straight lines
      for ( int ii = 0; ii < nkpts; ii++ )
      { 
         double yend = fmin;

         for ( int jj = 0; jj < nkpts; jj++ )
         {
            double xval = smin;
            double yval = ystr;
            double yinc = ( yend - ystr ) / rinc; 

            for ( int kk = 0; kk < nlpts; kk++ )
            {
               xx[ kk ]    = xval;
               yy[ kk ]    = yval;
               xval       += xinc;
               yval       += yinc;
            } // END: points-per-line loop

            title   = tr( "Curve " ) + QString::number( mndx );
            curv    = us_curve( data_plot1, title );

//DbgLv(1) << "RP:PD    bmndx mndx" << bmndx << mndx;
            if ( ! got_best  ||  mndx != bmndx )
            { // Normal line
               curv->setPen  ( pen_plot );    // Default color (yellow)
            }
            else
            { // Best model line
               y1 = yy[ 0 ];                  // Save best model end points
               y2 = yy[ nlpts - 1 ];
               curv->setPen  ( pen_red  );    // Red
            }
            curv->setData ( xx, yy, nlpts );
            yend   += finc;
            mndx++;
         } // END: s loop
         ystr   += finc;
      } // END: k loop
   } // END: ctype==0

   if ( ctype == 1  ||  ctype == 2 )
   { // Sigmoid curves
      double p1lo  = 0.001;
      double p1up  = 0.5;
      double p2lo  = 0.0;
      double p2up  = 1.0;
      double srng  = smax - smin;
      double p1llg = log( p1lo );
      double p1ulg = log( p1up );
      double lrng  = (double)( nlpts - 1 );
      double krng  = (double)( nkpts - 1 );
      double p1inc = ( p1ulg - p1llg ) / krng;
      double p2inc = ( p2up  - p2lo  ) / krng;
      double xinc  = 1.0 / lrng;
      double kstr  = fmin;
      double kdif  = fmax - fmin;
      if ( ctype == 2 )
      {
         kstr         = fmax;
         kdif         = -kdif;
      }
      double p1vlg = p1llg;

      for ( int ii = 0; ii < nkpts; ii++ )
      { // Loop over par1 values (logarithmic progression)
         double p1val = exp( p1vlg );
         double p2val = p2lo;

         for ( int jj = 0; jj < nkpts; jj++ )
         { // Loop over par2 value (linear progression)
            double xval  = 0.0;

            for ( int kk = 0; kk < nlpts; kk++ )
            { // Loop over points on a curve
               double efac  = 0.5 * erf( ( xval - p2val )
                                         / sqrt( 2.0 * p1val ) ) + 0.5;
               double kval  = kstr + kdif * efac;
               xx[ kk ]     = smin + xval * srng;
               yy[ kk ]     = kval;
DbgLv(1) << "RP:PD    ii jj kk mx" << ii << jj << kk << mndx
 << "xv sv kv" << xval << xx[kk] << kval << "p1 p2" << p1val << p2val;
               xval        += xinc;
            } // END: points-on-curve loop

            title   = tr( "Curve " ) + QString::number( mndx );
            curv    = us_curve( data_plot1, title );

//DbgLv(1) << "RP:PD    bmndx mndx" << bmndx << mndx;
            if ( ! got_best  ||  mndx != bmndx )
            { // Normal line
               curv->setPen  ( pen_plot );    // Default color (yellow)
            }
            else
            { // Best model line
               for ( int kk = 0; kk < nlpts; kk++ )
               {                              // Save best model points
                  bx[ kk ]     = xx[ kk ];
                  by[ kk ]     = yy[ kk ];
               }
               curv->setPen  ( pen_red  );    // Red
DbgLv(1) << "RP:PD    bmndx mndx" << bmndx << mndx;
            }
            curv->setData ( xx, yy, nlpts );
            mndx++;
            p2val    += p2inc;
         } // END: par2 values loop

         p1vlg    += p1inc;
      } // END: par1 values loop

   } // END: sigmoid curves

DbgLv(1) << "RP:PD call replot";
   if ( got_best )
   { // Replot best model line, then plot points along the best model
      title    = tr( "CurveR " ) + QString::number( bmndx );
      int npts = 2;
DbgLv(1) << "RP:PD   bmndx" << bmndx;
      if ( ctype == 0 )
      { // Re-do straight line by drawing from start to end point
         bx[ 0 ]  = xx[ 0 ];
         bx[ 1 ]  = xx[ nlpts - 1 ];
         by[ 0 ]  = y1;
         by[ 1 ]  = y2;
DbgLv(1) << "RP:PD     y1 y2" << y1 << y2;
      }
      else if ( ctype == 1  ||  ctype == 2 )
      { // Re-do sigmoid by copying saved curve points
         npts     = nlpts;
DbgLv(1) << "RP:PD     npts" << npts << "x0 y0 xn yn"
 << xx[0] << yy[0] << xx[npts-1] << yy[npts-1];
      }
      curv     = us_curve( data_plot1, title );
      curv->setPen( pen_red );
      curv->setData( bx, by, npts );

      int ncomp   = model->components.size();
      for ( int kk = 0; kk < ncomp; kk++ )
      {
         xx[ kk ]    = model->components[ kk ].s * 1.0e+13;
         yy[ kk ]    = model->components[ kk ].f_f0;
      }
DbgLv(1) << "RP:PD       ncomp" << ncomp << "x0 y0 xn yn"
 << xx[0] << yy[0] << xx[ncomp-1] << yy[ncomp-1];
      title    = tr( "BestModelPoints" );
      curv     = us_curve( data_plot1, title );
      curv->setPen  ( QPen( Qt::cyan, 8.0 ) );
      curv->setStyle( QwtPlotCurve::Dots );
      curv->setData ( xx, yy, ncomp );
   } // END: replot best model

   data_plot1->replot();
DbgLv(1) << "RP:PD  return";
}

// Public slot to set a pointer to a model to use in the plot for highlights
void US_MLinesPlot::setModel( US_Model* a_model )
{
   model = a_model;
DbgLv(1) << "RP:SM  bmndx" << bmndx;
}

