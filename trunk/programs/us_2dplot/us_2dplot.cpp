//! \file us_rotor_calibration.cpp

#include "us_2dplot.h"
#include "us_settings.h"
#include "us_license_t.h"
#include "us_math2.h"
#include "us_util.h"
#include "us_license.h"
#include "us_gui_settings.h"
#include "us_run_details2.h"
#include "us_passwd.h"

//! \brief program for plotting an arbitrary function

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_2dPlot w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

US_2dPlot::US_2dPlot() : US_Widgets()
{
   unsigned int row = 0;
   s_max = 10.0;
   s_min = 1.0;
   par1 = 4;
   par2 = -3;
   par3 = 1;
   par4 = 4;
   
   resolution = 100; //  start with 100 points in the s-domain

   setWindowTitle( tr( "Edit Rotor Calibration" ) );
   setPalette( US_GuiSettings::frameColor() );

   QGridLayout* top = new QGridLayout( this );
   top->setSpacing         ( 2 );
   top->setContentsMargins ( 2, 2, 2, 2 );
   
   QLabel* lbl_par1 = us_label( tr("Parameter 1:"), -1 );
   top->addWidget( lbl_par1, row++, 0 );

   ct_par1 = new QwtCounter(this); // set parameter 1
   ct_par1->setRange( -50, 50, 0.01 );
   ct_par1->setValue( par1 );
   ct_par1->setNumButtons( 3 );
   connect (ct_par1, SIGNAL(valueChanged (double)), this, SLOT(update_par1(double)));
   top->addWidget( ct_par1, row++, 0 );
   
   QLabel* lbl_par2 = us_label( tr("Parameter 2:"), -1 );
   top->addWidget( lbl_par2, row++, 0 );

   ct_par2 = new QwtCounter(this); // set parameter 2
   ct_par2->setRange( -50, 50, 0.01 );
   ct_par2->setValue( par2 );
   ct_par2->setNumButtons( 3 );
   connect (ct_par2, SIGNAL(valueChanged (double)), this, SLOT(update_par2(double)));
   top->addWidget( ct_par2, row++, 0 );
   
   QLabel* lbl_par3 = us_label( tr("Parameter 3:"), -1 );
   top->addWidget( lbl_par3, row++, 0 );

   ct_par3 = new QwtCounter(this); // set parameter 3
   ct_par3->setRange( -50, 50, 0.01 );
   ct_par3->setValue( par3 );
   ct_par3->setNumButtons( 3 );
   connect (ct_par3, SIGNAL(valueChanged (double)), this, SLOT(update_par3(double)));
   top->addWidget( ct_par3, row++, 0 );
   
   QLabel* lbl_par4 = us_label( tr("Parameter 4:"), -1 );
   top->addWidget( lbl_par4, row++, 0 );

   ct_par4 = new QwtCounter(this); // set parameter 4
   ct_par4->setRange( -50, 50, 0.01 );
   ct_par4->setValue( par4 );
   ct_par4->setNumButtons( 3 );
   connect (ct_par4, SIGNAL(valueChanged (double)), this, SLOT(update_par4(double)));
   top->addWidget( ct_par4, row++, 0 );
   
   QLabel* lbl_resolution = us_label( tr("S-value Resolution:"), -1 );
   top->addWidget( lbl_resolution, row++, 0 );

   ct_resolution = new QwtCounter(this); // set s-value resolution
   ct_resolution->setRange( -50, 50, 0.1 );
   ct_resolution->setValue( resolution );
   ct_resolution->setNumButtons( 3 );
   connect (ct_resolution, SIGNAL(valueChanged (double)), this, SLOT(update_resolution(double)));
   top->addWidget( ct_resolution, row++, 0 );
   
   
   // Plot layout on right side of window
   plot = new US_Plot( data_plot,
         tr( "Function Plot" ),
         tr( "sedimentation coefficient" ), tr( "f/f0" ) );

   data_plot->setMinimumSize( 700, 400 );
   data_plot->enableAxis( QwtPlot::xBottom, true );
   data_plot->enableAxis( QwtPlot::yLeft  , true );
   data_plot->setAxisScale( QwtPlot::xBottom, 0.2, 10.0 );
   data_plot->setAxisAutoScale( QwtPlot::yLeft );

   top->addLayout( plot, 0, 1, 0, 1 );

   pb_calculate = us_pushbutton( tr( "Calculate" ) );
   pb_calculate->setEnabled( true );
   connect( pb_calculate, SIGNAL( clicked() ), SLOT( calculate() ) );
   top->addWidget( pb_calculate, row++, 0 );

   QPushButton* pb_close = us_pushbutton( tr( "Close" ) );
   connect( pb_close, SIGNAL( clicked() ), SLOT( close() ) );
   top->addWidget( pb_close, row++, 0 );

   top->setColumnStretch( 0, 0 );
   top->setColumnStretch( 1, 1 );
}

void US_2dPlot::calculate()
{

   x  .resize( resolution );
   y  .resize( resolution );
   double inc = (s_max - s_min + 1.0)/resolution;

   for ( int i = 0; i < resolution; i++ )
   {
      x[ i ] = s_min + i * inc;
      y[ i ] = par1 + par2/(1.0 + par3 * x[ i ]);
   }
   
   plot->btnZoom->setChecked( false );
   data_plot->clear();
   data_plot->replot();
   QwtPlotCurve* c1;
   
   c1 = us_curve( data_plot, "Rotor Stretch" );
   c1->setData  ( x.data(), y.data(), resolution);
   c1->setStyle ( QwtPlotCurve::Lines );
   c1->setPen   ( QColor( Qt::yellow ) );

   data_plot->setAxisAutoScale( QwtPlot::xBottom );
   data_plot->setAxisAutoScale( QwtPlot::yLeft );
   data_plot->replot();
}

void US_2dPlot::update_par1( double val )
{
    par1 = val;
    calculate();
}

void US_2dPlot::update_par2( double val )
{
    par2 = val;
    calculate();
}

void US_2dPlot::update_par3( double val )
{
    par3 = val;
    calculate();
}

void US_2dPlot::update_par4( double val )
{
    par4 = val;
    calculate();
}

void US_2dPlot::update_resolution( double val )
{
    resolution = val;
}
