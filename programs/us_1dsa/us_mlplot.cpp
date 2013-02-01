//! \file us_mlplot.cpp

#include "us_mlplot.h"
#include "us_1dsa.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_math2.h"

#include <qwt_legend.h>

// constructor:  model lines plot widget
US_MLinesPlot::US_MLinesPlot( double& flo, double& fhi, double& fin,
      double& slo, double& shi, int& nlp, int& bmx, QWidget* p = 0 )
   : US_WidgetsDialog( 0, 0 ), fmin( flo ), fmax( fhi ), finc( fin ),
   smin( slo ), smax( shi ), nlpts( nlp ), bmndx( bmx )
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

   int    nkpts = qRound( ( fmax - fmin ) / finc ) + 1;
   int    nline = nkpts * nkpts;
   le_mtype     = us_lineedit( tr( "Straight Line" ), -1, true );
   le_nlines    = us_lineedit( QString::number( nline ), -1, true );
   le_npoints   = us_lineedit( QString::number( nlpts ), -1, true );
   le_kincr     = us_lineedit( QString::number( finc  ), -1, true );

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

   plot_data();
DbgLv(1) << "RP:   PD returned";

   setVisible( true );
DbgLv(1) << "RP:    setVis returned";
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

   int    nkpts = qRound( ( fmax - fmin ) / finc ) + 1;
   bool   gotm  = ( model != 0  &&  bmndx >= 0 );
DbgLv(1) << "RP:PD gotm" << gotm << "bmndx" << bmndx;

   us_grid( data_plot1 );

   QVector< double > xvec( nlpts, 0.0 );
   QVector< double > yvec( nlpts, 0.0 );

   double* xx   = xvec.data();
   double* yy   = yvec.data();
   double  rinc = (double)( nlpts - 1 );
   double  xinc = ( smax - smin ) / rinc;
   double  ystr = fmin;
   int     mndx = 0;

   QString       title;
   QwtPlotCurve* curv;
   QPen          pen_red(  Qt::red, 4.0 );
   QPen          pen_plot( US_GuiSettings::plotCurve() );
   data_plot1->setAxisScale( QwtPlot::xBottom, smin, smax, 1.0 );
   double  y1    = fmin;
   double  y2    = fmax;

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
         }

         title   = tr( "Curve " ) + QString::number( mndx );
         curv    = us_curve( data_plot1, title );

//DbgLv(1) << "RP:PD    bmndx mndx" << bmndx << mndx;
         if ( mndx != bmndx )
            curv->setPen  ( pen_plot );
         else
         {
            y1 = yy[ 0 ];
            y2 = yy[ nlpts - 1 ];
            curv->setPen  ( pen_red  );
         }
         curv->setData ( xx, yy, nlpts );
         yend   += finc;
         mndx++;
      }
      ystr   += finc;
   }
DbgLv(1) << "RP:PD call replot";
   if ( bmndx >= 0  &&  model != 0 )
   { // Replot best model line, then plot points along the best model
      title    = tr( "CurveR " ) + QString::number( bmndx );
      xx[ 1 ]  = xx[ nlpts - 1 ];
      yy[ 0 ]  = y1;
      yy[ 1 ]  = y2;
      curv     = us_curve( data_plot1, title );
      curv->setPen( pen_red );
      curv->setData( xx, yy, 2 );

      int ncomp   = model->components.size();
      for ( int kk = 0; kk < ncomp; kk++ )
      {
         xx[ kk ]    = model->components[ kk ].s * 1.0e+13;
         yy[ kk ]    = model->components[ kk ].f_f0;
      }
      title    = tr( "BestModelPoints" );
      curv     = us_curve( data_plot1, title );
      curv->setPen  ( QPen( Qt::cyan, 8.0 ) );
      curv->setStyle( QwtPlotCurve::Dots );
      curv->setData ( xx, yy, ncomp );
   }

   data_plot1->replot();
DbgLv(1) << "RP:PD  return";
}

// Public slot to set a pointer to a model to use in the plot for highlights
void US_MLinesPlot::setModel( US_Model* a_model, int& bm_ndx )
{
   model = a_model;
   bmndx = bm_ndx;
DbgLv(1) << "RP:SM  bmndx" << bmndx;
}

