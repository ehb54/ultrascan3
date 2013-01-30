//! \file us_mlplot.cpp

#include "us_mlplot.h"
#include "us_1dsa.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_math2.h"

#include <qwt_legend.h>

// constructor:  residuals plot widget
US_MLinesPlot::US_MLinesPlot( double& flo, double& fhi, double& fin,
      double& slo, double& shi, int& nlp, QWidget* p = 0 )
   : US_WidgetsDialog( 0, 0 ), fmin( flo ), fmax( fhi ), finc( fin ),
   smin( slo ), smax( shi ), nlpts( nlp )
{
   // lay out the GUI
   setWindowTitle( tr( "1-D Spectrum Analysis Model Lines Viewer" ) );
   setPalette( US_GuiSettings::frameColor() );

   QSize p1size( 560, 480 );

   dbg_level       = US_Settings::us_debug();
   resbmap         = 0;

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

   // get data pointers from parent of parent

DbgLv(1) << "RP: P" << ( p != 0 );

   if ( p )
   {
      QWidget* pp     = (QWidget*)p->parent();
DbgLv(1) << "RP:  PP" << ( pp != 0 );
      US_1dsa*  mainw = (US_1dsa*)pp;
      edata           = mainw->mw_editdata();
      sdata           = mainw->mw_simdata();
      ti_noise        = mainw->mw_ti_noise();
      ri_noise        = mainw->mw_ri_noise();
   }

   else
   {
      qDebug() << "*ERROR* unable to get RP parent";
   }

DbgLv(1) << "RP:  p1size" << p1size;

   data_plot1->resize( p1size );

   plot_data();

   setVisible( true );
}

// close button clicked
void US_MLinesPlot::close_all()
{
   if ( resbmap != 0 )
   {
      resbmap->close();
   }

   close();
}

// plot the data
void US_MLinesPlot::plot_data()
{
   data_plot1->detachItems();
   data_plot1->clear();

   int    nkpts = qRound( ( fmax - fmin ) / finc ) + 1;

   us_grid( data_plot1 );

   QVector< double > xvec( nlpts, 0.0 );
   QVector< double > yvec( nlpts, 0.0 );

   double* xx   = xvec.data();
   double* yy   = yvec.data();
   double  rinc = (double)( nlpts - 1 );
   double  xinc = ( smax - smin ) / rinc;
   double  ystr = fmin;

   QString       title;
   QwtPlotCurve* curv;
   QPen          pen_red(  Qt::red );
   QPen          pen_plot( US_GuiSettings::plotCurve() );
   data_plot1->setAxisScale( QwtPlot::xBottom, smin, smax, 1.0 );

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

         title   = tr( "Curve " ) + QString::number( ii );
         curv    = us_curve( data_plot1, title );

         curv->setPen  ( pen_plot );
         curv->setData ( xx, yy, nlpts );
         yend   += finc;
      }
      ystr   += finc;
   }

   data_plot1->replot();
}

