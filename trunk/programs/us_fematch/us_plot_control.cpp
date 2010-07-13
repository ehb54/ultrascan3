
//! \file us_plot_control.cpp

#include "us_fematch.h"
#include "us_plot_control.h"
#include "us_settings.h"
#include "us_gui_settings.h"

#include <qwt_legend.h>

// constructor:  enhanced plot control widget
US_PlotControl::US_PlotControl( QWidget* p, US_Model* amodel )
   : US_WidgetsDialog( 0, 0 )
{

   wparent        = p;
   model          = amodel;

   setObjectName( "US_PlotControl" );
   setAttribute( Qt::WA_DeleteOnClose, true );

   // lay out the GUI
   setWindowTitle( tr( "Enhanced Plotting Controls" ) );
   setPalette( US_GuiSettings::frameColor() );

   mainLayout      = new QVBoxLayout( this );
   controlsLayout  = new QGridLayout( );
   plotbtnLayout   = new QHBoxLayout( );
   buttonsLayout   = new QHBoxLayout( );

   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   mainLayout->addLayout( controlsLayout );
   mainLayout->addLayout( plotbtnLayout  );
   mainLayout->addLayout( buttonsLayout  );

   QLabel* lb_controls    = us_banner( tr( "3D Plotting Controls" ) );
   QLabel* lb_dimens      = us_label(  tr( "Dimension:" ) );
   QLabel* lb_molwt       = us_label(  tr( "Molecular Weight:" ) );
   QLabel* lb_sedcoeff    = us_label(  tr( "Sedimentation Coefficient:" ) );
   QLabel* lb_diffcoeff   = us_label(  tr( "Diffusion Coefficient:" ) );
   QLabel* lb_friccoeff   = us_label(  tr( "Frictional Coefficient:" ) );
   QLabel* lb_fricratio   = us_label(  tr( "Frictional Ratio:" ) );
   QLabel* lb_zscalefac   = us_label(  tr( "Z-Scaling Factor:" ) );
   QLabel* lb_gridreso    = us_label(  tr( "Grid Resolution:" ) );
   QLabel* lb_peaksmoo    = us_label(  tr( "Peak Smoothing:" ) );
   QLabel* lb_peakwidth   = us_label(  tr( "Peak Width:" ) );
   QLabel* lb_dimen1      = us_label(  tr( "1 (x)" ) );
   QLabel* lb_dimen2      = us_label(  tr( "2 (y)" ) );

   QPushButton* pb_rplot  = us_pushbutton( tr( "Residuals Plot" ) );
   QPushButton* pb_plot3d = us_pushbutton( tr( "3D Plot" ) );
   QPushButton* pb_help   = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_close  = us_pushbutton( tr( "Close" ) );

   QCheckBox*   ck_xmwt   = new QCheckBox( tr( "x=mw" ) );
   QCheckBox*   ck_ymwt   = new QCheckBox( tr( "y=mw" ) );
   QCheckBox*   ck_xsed   = new QCheckBox( tr( "x=sc" ) );
   QCheckBox*   ck_ysed   = new QCheckBox( tr( "y=sc" ) );
   QCheckBox*   ck_xdif   = new QCheckBox( tr( "x=dc" ) );
   QCheckBox*   ck_ydif   = new QCheckBox( tr( "y=dc" ) );
   QCheckBox*   ck_xfco   = new QCheckBox( tr( "x=fc" ) );
   QCheckBox*   ck_yfco   = new QCheckBox( tr( "y=fc" ) );
   QCheckBox*   ck_xfra   = new QCheckBox( tr( "x=fr" ) );
   QCheckBox*   ck_yfra   = new QCheckBox( tr( "y=fr" ) );

   QwtCounter* ct_zscalefac = us_counter( 3,  0.1,   10, 0.01 );
   QwtCounter* ct_gridreso  = us_counter( 3,   50,  300,   10 );
   QwtCounter* ct_peaksmoo  = us_counter( 3,    1,  200,    1 );
   QwtCounter* ct_peakwidth = us_counter( 3, 0.01, 10.0, 0.01 );

   controlsLayout->addWidget( lb_controls,   0, 0, 1, 4 );
   controlsLayout->addWidget( lb_dimens,     1, 0, 1, 2 );
   controlsLayout->addWidget( lb_dimen1,     1, 2, 1, 1 );
   controlsLayout->addWidget( lb_dimen2,     1, 3, 1, 1 );
   controlsLayout->addWidget( lb_molwt,      2, 0, 1, 2 );
   controlsLayout->addWidget( ck_xmwt,       2, 2, 1, 1 );
   controlsLayout->addWidget( ck_ymwt,       2, 3, 1, 1 );
   controlsLayout->addWidget( lb_sedcoeff,   3, 0, 1, 2 );
   controlsLayout->addWidget( ck_xsed,       3, 2, 1, 1 );
   controlsLayout->addWidget( ck_ysed,       3, 3, 1, 1 );
   controlsLayout->addWidget( lb_diffcoeff,  4, 0, 1, 2 );
   controlsLayout->addWidget( ck_xdif,       4, 2, 1, 1 );
   controlsLayout->addWidget( ck_ydif,       4, 3, 1, 1 );
   controlsLayout->addWidget( lb_friccoeff,  5, 0, 1, 2 );
   controlsLayout->addWidget( ck_xfco,       5, 2, 1, 1 );
   controlsLayout->addWidget( ck_yfco,       5, 3, 1, 1 );
   controlsLayout->addWidget( lb_fricratio,  6, 0, 1, 2 );
   controlsLayout->addWidget( ck_xfra,       6, 2, 1, 1 );
   controlsLayout->addWidget( ck_yfra,       6, 3, 1, 1 );
   controlsLayout->addWidget( lb_zscalefac,  7, 0, 1, 2 );
   controlsLayout->addWidget( ct_zscalefac,  7, 2, 1, 2 );
   controlsLayout->addWidget( lb_gridreso,   8, 0, 1, 2 );
   controlsLayout->addWidget( ct_gridreso,   8, 2, 1, 2 );
   controlsLayout->addWidget( lb_peaksmoo,   9, 0, 1, 2 );
   controlsLayout->addWidget( ct_peaksmoo,   9, 2, 1, 2 );
   controlsLayout->addWidget( lb_peakwidth, 10, 0, 1, 2 );
   controlsLayout->addWidget( ct_peakwidth, 10, 2, 1, 2 );

   plotbtnLayout->addWidget( pb_plot3d );
   plotbtnLayout->addWidget( pb_rplot  );
   buttonsLayout->addWidget( pb_help   );
   buttonsLayout->addWidget( pb_close  );

   ck_xmwt->setChecked( true  );
   ck_ymwt->setChecked( false );
   ck_xsed->setChecked( false );
   ck_ysed->setChecked( false );
   ck_xdif->setChecked( false );
   ck_ydif->setChecked( false );
   ck_xfco->setChecked( false );
   ck_yfco->setChecked( false );
   ck_xfra->setChecked( false );
   ck_yfra->setChecked( true  );

   ck_ymwt->setEnabled( false );
   ck_xfra->setEnabled( false );

   // set up so check boxes are like 2-d radio buttons
   xCheck.resize( 5 );
   xCheck[ 0 ] = ck_xmwt;
   xCheck[ 1 ] = ck_xsed;
   xCheck[ 2 ] = ck_xdif;
   xCheck[ 3 ] = ck_xfco;
   xCheck[ 4 ] = ck_xfra;
   yCheck.resize( 5 );
   yCheck[ 0 ] = ck_ymwt;
   yCheck[ 1 ] = ck_ysed;
   yCheck[ 2 ] = ck_ydif;
   yCheck[ 3 ] = ck_yfco;
   yCheck[ 4 ] = ck_yfra;

   ck_xmwt->setFont( pb_help->font() );
   ck_ymwt->setFont( pb_help->font() );
   ck_xsed->setFont( pb_help->font() );
   ck_ysed->setFont( pb_help->font() );
   ck_xdif->setFont( pb_help->font() );
   ck_ydif->setFont( pb_help->font() );
   ck_xfco->setFont( pb_help->font() );
   ck_yfco->setFont( pb_help->font() );
   ck_xfra->setFont( pb_help->font() );
   ck_yfra->setFont( pb_help->font() );

   ck_xmwt->setPalette( US_GuiSettings::normalColor() );
   ck_ymwt->setPalette( US_GuiSettings::normalColor() );
   ck_xsed->setPalette( US_GuiSettings::normalColor() );
   ck_ysed->setPalette( US_GuiSettings::normalColor() );
   ck_xdif->setPalette( US_GuiSettings::normalColor() );
   ck_ydif->setPalette( US_GuiSettings::normalColor() );
   ck_xfco->setPalette( US_GuiSettings::normalColor() );
   ck_yfco->setPalette( US_GuiSettings::normalColor() );
   ck_xfra->setPalette( US_GuiSettings::normalColor() );
   ck_yfra->setPalette( US_GuiSettings::normalColor() );

   ct_zscalefac->setValue( 1 );
   ct_gridreso ->setValue( 150 );
   ct_peaksmoo ->setValue( 80 );
   ct_peakwidth->setValue( 0.3 );
   ct_zscalefac->setStep(  0.01 );
   ct_gridreso ->setStep(  10   );
   ct_peaksmoo ->setStep(  1    );
   ct_peakwidth->setStep(  0.01 );

   connect( ck_xmwt, SIGNAL( toggled( bool ) ),
            this,    SLOT( xmwtCheck( bool ) ) );
   connect( ck_ymwt, SIGNAL( toggled( bool ) ),
            this,    SLOT( ymwtCheck( bool ) ) );
   connect( ck_xsed, SIGNAL( toggled( bool ) ),
            this,    SLOT( xsedCheck( bool ) ) );
   connect( ck_ysed, SIGNAL( toggled( bool ) ),
            this,    SLOT( ysedCheck( bool ) ) );
   connect( ck_xdif, SIGNAL( toggled( bool ) ),
            this,    SLOT( xdifCheck( bool ) ) );
   connect( ck_ydif, SIGNAL( toggled( bool ) ),
            this,    SLOT( ydifCheck( bool ) ) );
   connect( ck_xfco, SIGNAL( toggled( bool ) ),
            this,    SLOT( xfcoCheck( bool ) ) );
   connect( ck_yfco, SIGNAL( toggled( bool ) ),
            this,    SLOT( yfcoCheck( bool ) ) );
   connect( ck_xfra, SIGNAL( toggled( bool ) ),
            this,    SLOT( xfraCheck( bool ) ) );
   connect( ck_yfra, SIGNAL( toggled( bool ) ),
            this,    SLOT( yfraCheck( bool ) ) );

   connect( ct_zscalefac, SIGNAL( valueChanged( double ) ),
            this,         SLOT(    zscal_value( double ) ) );
   connect( ct_gridreso , SIGNAL( valueChanged( double ) ),
            this,         SLOT(    gridr_value( double ) ) );
   connect( ct_peaksmoo , SIGNAL( valueChanged( double ) ),
            this,         SLOT(    peaks_value( double ) ) );
   connect( ct_peakwidth, SIGNAL( valueChanged( double ) ),
            this,         SLOT(    peakw_value( double ) ) );

   connect( pb_rplot,  SIGNAL( clicked() ),
            this,      SLOT( rplot_btn() ) );
   connect( pb_plot3d, SIGNAL( clicked() ),
            this,      SLOT( plot3_btn() ) );
   connect( pb_help,   SIGNAL( clicked() ),
            this,      SLOT( help()      ) );
   connect( pb_close,  SIGNAL( clicked() ),
            this,      SLOT( close_all() ) );

   resplotd = 0;
   plot3d_w = 0;
   zscale   = 1.0;
   gridres  = 150.0;
   pksmooth = 80.0;
   pkwidth  = 0.3;
}

// return caller of plot_control
QWidget* US_PlotControl::caller( void )
{
   return wparent;
}

// mw x box checked
void US_PlotControl::xmwtCheck( bool chkd )
{
   checkSet( chkd, true,  0 );
}
// mw y box checked
void US_PlotControl::ymwtCheck( bool chkd )
{
   checkSet( chkd, false, 0 );
}
// sedcoeff x box checked
void US_PlotControl::xsedCheck( bool chkd )
{
   checkSet( chkd, true,  1 );
}
// sedcoeff y box checked
void US_PlotControl::ysedCheck( bool chkd )
{
   checkSet( chkd, false, 1 );
}
// diffcoeff x box checked
void US_PlotControl::xdifCheck( bool chkd )
{
   checkSet( chkd, true,  2 );
}
// diffcoeff y box checked
void US_PlotControl::ydifCheck( bool chkd )
{
   checkSet( chkd, false, 2 );
}
// friccoeff x box checked
void US_PlotControl::xfcoCheck( bool chkd )
{
   checkSet( chkd, true,  3 );
}
// friccoeff y box checked
void US_PlotControl::yfcoCheck( bool chkd )
{
   checkSet( chkd, false, 3 );
}
// fricratio x box checked
void US_PlotControl::xfraCheck( bool chkd )
{
   checkSet( chkd, true,  4 );
}
// fricratio y box checked
void US_PlotControl::yfraCheck( bool chkd )
{
   checkSet( chkd, false, 4 );
}

// handle any x or y box checked
void US_PlotControl::checkSet( bool chkd, bool isX, int row )
{
   if ( !chkd )                                // only respond if checked
      return;

   if ( isX )
   {  // this is a X box
      for ( int ii = 0; ii < 5; ii++ )
      {  // review all check boxes
         if ( ii != row )
         {  // in other rows, uncheck X, enable Y
            xCheck[ ii ]->setChecked( false );
            yCheck[ ii ]->setEnabled( true );
         }

         else
         {  // in this row, disable the Y
            yCheck[ ii ]->setEnabled( false );
         }
      }
   }

   else
   {  // this is a Y box
      for ( int ii = 0; ii < 5; ii++ )
      {  // review all check boxes
         if ( ii != row )
         {  // in other rows, uncheck Y, enable X
            yCheck[ ii ]->setChecked( false );
            xCheck[ ii ]->setEnabled( true );
         }

         else
         {  // in this row, disable the X
            xCheck[ ii ]->setEnabled( false );
         }
      }
   }
}

// z scale factor changed
void US_PlotControl::zscal_value( double value )
{
   zscale   = value;
}
// grid resolution changed
void US_PlotControl::gridr_value( double value )
{
   gridres  = value;
}
// peak smoothing changed
void US_PlotControl::peaks_value( double value )
{
   pksmooth = value;
}
// peak width changed
void US_PlotControl::peakw_value( double value )
{
   pkwidth  = value;
}

// residual plot button clicked
void US_PlotControl::rplot_btn()
{
   resplotd = new US_ResidPlot( this );
   resplotd->setVisible( true );
   //rplotd->exec();
   //qApp->processEvents();
}

// 3d plot button clicked
void US_PlotControl::plot3_btn()
{
   int typex = dimensionType( xCheck );
   int typey = dimensionType( yCheck );
   int typez = 1;

   if ( plot3d_w == 0 )
   {
      plot3d_w = new US_Plot3D( this, model );
   }

   plot3d_w->setTypes     ( typex, typey, typez );
   plot3d_w->setParameters( zscale, gridres, pksmooth, pkwidth );
   plot3d_w->replot       ( );

   plot3d_w->setVisible( true );
}

// close button clicked
void US_PlotControl::close_all()
{
   if ( resplotd )
      resplotd->close();

   close();
}

int US_PlotControl::dimensionType( QVector< QCheckBox* >& xycheck )
{
   int dimType = 1;

   for ( int ii = 0; ii < xycheck.size(); ii++ )
   {
      if ( xycheck[ ii ]->isChecked() )
      {
         dimType = ii + 1;
         break;
      }
   }

   return dimType;
}

