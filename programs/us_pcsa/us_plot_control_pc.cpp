//! \file us_plot_control_pc.cpp

#include "us_pcsa.h"
#include "us_plot_control_pc.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_constants.h"

#include <qwt_legend.h>

// constructor:  enhanced plot control widget
US_PlotControlPc::US_PlotControlPc( QWidget* p, US_Model* amodel )
   : US_WidgetsDialog( p, Qt::WindowFlags() )
{
   model          = amodel;

   setObjectName( "US_PlotControlPc" );
   setPalette( US_GuiSettings::frameColor() );
   setFont( QFont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() ) );

   // lay out the GUI
   setWindowTitle( tr( "3-D Plotting Controls" ) );

   mainLayout      = new QVBoxLayout( this );
   controlsLayout  = new QGridLayout( );
   buttonsLayout   = new QHBoxLayout( );

   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   mainLayout->addLayout( controlsLayout );
   mainLayout->addLayout( buttonsLayout  );

   QLabel* lb_dimens      = us_label(  tr( "Dimension:" ) );
   QLabel* lb_molwt       = us_label(  tr( "Molecular Weight:" ) );
   QLabel* lb_sedcoeff    = us_label(  tr( "Sedimentation Coefficient:" ) );
   QLabel* lb_diffcoeff   = us_label(  tr( "Diffusion Coefficient:" ) );
   QLabel* lb_friccoeff   = us_label(  tr( "Frictional Coefficient:" ) );
   QLabel* lb_fricratio   = us_label(  tr( "Frictional Ratio:" ) );
   QLabel* lb_vbar        = us_label(  tr( "Vbar at 20" ) + DEGC + ":" );
   QLabel* lb_zscalefac   = us_label(  tr( "Z-Scaling Factor:" ) );
   QLabel* lb_gridreso    = us_label(  tr( "Grid Resolution:" ) );
   QLabel* lb_peaksmoo    = us_label(  tr( "Peak Smoothing:" ) );
   QLabel* lb_peakwidth   = us_label(  tr( "Peak Width:" ) );
   QLabel* lb_dimen1      = us_label(  tr( "1 (x)" ) );
   QLabel* lb_dimen2      = us_label(  tr( "2 (y)" ) );
   QLabel* lb_rxscale     = us_label(  tr( "Relative X Scale:" ) );
   QLabel* lb_ryscale     = us_label(  tr( "Relative Y Scale:" ) );

   //QPushButton* pb_rplot  = us_pushbutton( tr( "Residuals Plot" ) );
   QPushButton* pb_plot3d = us_pushbutton( tr( "3D Plot" ) );
   QPushButton* pb_help   = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_close  = us_pushbutton( tr( "Close" ) );

   QCheckBox*   ck_xmwt;
   QCheckBox*   ck_ymwt;
   QCheckBox*   ck_xsed;
   QCheckBox*   ck_ysed;
   QCheckBox*   ck_xdif;
   QCheckBox*   ck_ydif;
   QCheckBox*   ck_xfco;
   QCheckBox*   ck_yfco;
   QCheckBox*   ck_xfra;
   QCheckBox*   ck_yfra;
   QCheckBox*   ck_xvba;
   QCheckBox*   ck_yvba;

   QLayout*     lo_xmwt   = us_checkbox( tr( "x=mw"   ), ck_xmwt, false );
   QLayout*     lo_ymwt   = us_checkbox( tr( "y=mw"   ), ck_ymwt, false );
   QLayout*     lo_xsed   = us_checkbox( tr( "x=s"    ), ck_xsed, true  );
   QLayout*     lo_ysed   = us_checkbox( tr( "y=s"    ), ck_ysed, false );
   QLayout*     lo_xdif   = us_checkbox( tr( "x=D"    ), ck_xdif, false );
   QLayout*     lo_ydif   = us_checkbox( tr( "y=D"    ), ck_ydif, false );
   QLayout*     lo_xfco   = us_checkbox( tr( "x=f"    ), ck_xfco, false );
   QLayout*     lo_yfco   = us_checkbox( tr( "y=f"    ), ck_yfco, false );
   QLayout*     lo_xfra   = us_checkbox( tr( "x=f/f0" ), ck_xfra, false );
   QLayout*     lo_yfra   = us_checkbox( tr( "y=f/f0" ), ck_yfra, true  );
   QLayout*     lo_xvba   = us_checkbox( tr( "x=vb"   ), ck_xvba, false );
   QLayout*     lo_yvba   = us_checkbox( tr( "y=vb"   ), ck_yvba, false );

   QwtCounter* ct_zscalefac = us_counter( 3,  0.1,   10, 0.01 );
   QwtCounter* ct_gridreso  = us_counter( 3,   50,  300,   10 );
   QwtCounter* ct_peaksmoo  = us_counter( 3,    1,  200,    1 );
   QwtCounter* ct_peakwidth = us_counter( 3, 0.01, 10.0, 0.01 );
               ct_rxscale   = us_counter( 3,  0.1,   50, 0.01 );
               ct_ryscale   = us_counter( 3,  0.1,   50, 0.01 );

   int row = 1;
   controlsLayout->addWidget( lb_dimens,     row,   0, 1, 2 );
   controlsLayout->addWidget( lb_dimen1,     row,   2, 1, 1 );
   controlsLayout->addWidget( lb_dimen2,     row++, 3, 1, 1 );
   controlsLayout->addWidget( lb_molwt,      row,   0, 1, 2 );
   controlsLayout->addLayout( lo_xmwt,       row,   2, 1, 1 );
   controlsLayout->addLayout( lo_ymwt,       row++, 3, 1, 1 );
   controlsLayout->addWidget( lb_sedcoeff,   row,   0, 1, 2 );
   controlsLayout->addLayout( lo_xsed,       row,   2, 1, 1 );
   controlsLayout->addLayout( lo_ysed,       row++, 3, 1, 1 );
   controlsLayout->addWidget( lb_diffcoeff,  row,   0, 1, 2 );
   controlsLayout->addLayout( lo_xdif,       row,   2, 1, 1 );
   controlsLayout->addLayout( lo_ydif,       row++, 3, 1, 1 );
   controlsLayout->addWidget( lb_friccoeff,  row,   0, 1, 2 );
   controlsLayout->addLayout( lo_xfco,       row,   2, 1, 1 );
   controlsLayout->addLayout( lo_yfco,       row++, 3, 1, 1 );
   controlsLayout->addWidget( lb_fricratio,  row,   0, 1, 2 );
   controlsLayout->addLayout( lo_xfra,       row,   2, 1, 1 );
   controlsLayout->addLayout( lo_yfra,       row++, 3, 1, 1 );
   controlsLayout->addWidget( lb_vbar,       row,   0, 1, 2 );
   controlsLayout->addLayout( lo_xvba,       row,   2, 1, 1 );
   controlsLayout->addLayout( lo_yvba,       row++, 3, 1, 1 );
   controlsLayout->addWidget( lb_zscalefac,  row,   0, 1, 2 );
   controlsLayout->addWidget( ct_zscalefac,  row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_gridreso,   row,   0, 1, 2 );
   controlsLayout->addWidget( ct_gridreso,   row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_peaksmoo,   row,   0, 1, 2 );
   controlsLayout->addWidget( ct_peaksmoo,   row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_peakwidth,  row,   0, 1, 2 );
   controlsLayout->addWidget( ct_peakwidth,  row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_rxscale,    row,   0, 1, 2 );
   controlsLayout->addWidget( ct_rxscale,    row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_ryscale,    row,   0, 1, 2 );
   controlsLayout->addWidget( ct_ryscale,    row++, 2, 1, 2 );

   buttonsLayout->addWidget( pb_plot3d );
   buttonsLayout->addWidget( pb_help   );
   buttonsLayout->addWidget( pb_close  );

   ck_ysed->setEnabled( false );
   ck_xfra->setEnabled( false );

   // set up so check boxes are like 2-d radio buttons
   xCheck.resize( 6 );
   xCheck[ 0 ] = ck_xmwt;
   xCheck[ 1 ] = ck_xsed;
   xCheck[ 2 ] = ck_xdif;
   xCheck[ 3 ] = ck_xfco;
   xCheck[ 4 ] = ck_xfra;
   xCheck[ 5 ] = ck_xvba;
   yCheck.resize( 6 );
   yCheck[ 0 ] = ck_ymwt;
   yCheck[ 1 ] = ck_ysed;
   yCheck[ 2 ] = ck_ydif;
   yCheck[ 3 ] = ck_yfco;
   yCheck[ 4 ] = ck_yfra;
   yCheck[ 5 ] = ck_yvba;

   zscale   = 1.0;
   gridres  = 150.0;
   pksmooth = 80.0;
   pkwidth  = 0.1;
   ct_zscalefac->setValue( zscale   );
   ct_gridreso ->setValue( gridres  );
   ct_peaksmoo ->setValue( pksmooth );
   ct_peakwidth->setValue( pkwidth  );
   ct_rxscale  ->setValue( 1.0      );
   ct_ryscale  ->setValue( 1.0      );
   ct_zscalefac->setSingleStep( 0.01 );
   ct_gridreso ->setSingleStep( 10   );
   ct_peaksmoo ->setSingleStep( 1    );
   ct_peakwidth->setSingleStep( 0.01 );
   ct_rxscale  ->setSingleStep( 0.01 );
   ct_ryscale  ->setSingleStep( 0.01 );

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
   connect( ck_xvba, SIGNAL( toggled( bool ) ),
            this,    SLOT( xvbaCheck( bool ) ) );
   connect( ck_yvba, SIGNAL( toggled( bool ) ),
            this,    SLOT( yvbaCheck( bool ) ) );

   connect( ct_zscalefac, SIGNAL( valueChanged( double ) ),
            this,         SLOT(    zscal_value( double ) ) );
   connect( ct_gridreso , SIGNAL( valueChanged( double ) ),
            this,         SLOT(    gridr_value( double ) ) );
   connect( ct_peaksmoo , SIGNAL( valueChanged( double ) ),
            this,         SLOT(    peaks_value( double ) ) );
   connect( ct_peakwidth, SIGNAL( valueChanged( double ) ),
            this,         SLOT(    peakw_value( double ) ) );

   connect( pb_plot3d, SIGNAL( clicked() ),
            this,      SLOT( plot3_btn() ) );
   connect( pb_help,   SIGNAL( clicked() ),
            this,      SLOT( help()      ) );
   connect( pb_close,  SIGNAL( clicked() ),
            this,      SLOT( close_all() ) );

   plot3d_w = 0;

   lb_sedcoeff ->adjustSize();
   ct_zscalefac->setMinimumWidth( lb_sedcoeff->width() );
   adjustSize();

   bool cnstff0 = model->constant_ff0();

   if ( cnstff0 )
   {  // f/f0 constant, so disable f/f0 X,Y check boxes
      ck_xfra->setEnabled( false );
      ck_yfra->setEnabled( false );
      ck_xfra->setChecked( false );
      ck_yfra->setChecked( false );
      ck_yvba->setChecked( true  );
   }
   if ( model->constant_vbar() )
   {  // Vbar constant, so disable vbar X,Y check boxes
      ck_xvba->setEnabled( false );
      ck_yvba->setEnabled( false );
      ck_xvba->setChecked( false );
      ck_yvba->setChecked( false );
      if ( cnstff0 )
         ck_yfco->setChecked( true  );
      else
         ck_yfra->setChecked( true  );
   }

qDebug() << "PlCtl: cnst_vbar" << model->constant_vbar();
qDebug() << "PlCtl: cnst_ff0 " << model->constant_ff0 ();
qDebug() << "PlCtl: ck_yfra  " << ck_yfra->text();
qDebug() << "PlCtl: ncomps   " << model->components.size();
}

// mw x box checked
void US_PlotControlPc::xmwtCheck( bool chkd )
{
   checkSet( chkd, true,  0 );
}
// mw y box checked
void US_PlotControlPc::ymwtCheck( bool chkd )
{
   checkSet( chkd, false, 0 );
}
// sedcoeff x box checked
void US_PlotControlPc::xsedCheck( bool chkd )
{
   checkSet( chkd, true,  1 );
}
// sedcoeff y box checked
void US_PlotControlPc::ysedCheck( bool chkd )
{
   checkSet( chkd, false, 1 );
}
// diffcoeff x box checked
void US_PlotControlPc::xdifCheck( bool chkd )
{
   checkSet( chkd, true,  2 );
}
// diffcoeff y box checked
void US_PlotControlPc::ydifCheck( bool chkd )
{
   checkSet( chkd, false, 2 );
}
// friccoeff x box checked
void US_PlotControlPc::xfcoCheck( bool chkd )
{
   checkSet( chkd, true,  3 );
}
// friccoeff y box checked
void US_PlotControlPc::yfcoCheck( bool chkd )
{
   checkSet( chkd, false, 3 );
}
// fricratio x box checked
void US_PlotControlPc::xfraCheck( bool chkd )
{
   checkSet( chkd, true,  4 );
}
// fricratio y box checked
void US_PlotControlPc::yfraCheck( bool chkd )
{
   checkSet( chkd, false, 4 );
}
// Vbar x box checked
void US_PlotControlPc::xvbaCheck( bool chkd )
{
   checkSet( chkd, true,  5 );
}
// Vbar y box checked
void US_PlotControlPc::yvbaCheck( bool chkd )
{
   checkSet( chkd, false, 5 );
}

// handle any x or y box checked
void US_PlotControlPc::checkSet( bool chkd, bool isX, int row )
{
   if ( !chkd )                                // only respond if checked
      return;

   if ( isX )
   {  // this is a X box
      for ( int ii = 0; ii < 6; ii++ )
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
      for ( int ii = 0; ii < 6; ii++ )
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
   if ( model->constant_ff0() )
   {  // f/f0 constant, so disable f/f0 X,Y check boxes
      xCheck[ 4 ]->setEnabled( false );
      yCheck[ 4 ]->setEnabled( false );
      xCheck[ 4 ]->setChecked( false );
      yCheck[ 4 ]->setChecked( false );
   }
   if ( model->constant_vbar() )
   {  // Vbar constant, so disable vbar X,Y check boxes
      xCheck[ 5 ]->setEnabled( false );
      yCheck[ 5 ]->setEnabled( false );
      xCheck[ 5 ]->setChecked( false );
      yCheck[ 5 ]->setChecked( false );
   }
}

// z scale factor changed
void US_PlotControlPc::zscal_value( double value )
{
   zscale   = value;
}
// grid resolution changed
void US_PlotControlPc::gridr_value( double value )
{
   gridres  = value;
}
// peak smoothing changed
void US_PlotControlPc::peaks_value( double value )
{
   pksmooth = value;
}
// peak width changed
void US_PlotControlPc::peakw_value( double value )
{
   pkwidth  = value;
}

// 3d plot button clicked
void US_PlotControlPc::plot3_btn()
{
   int    typex  = dimensionType( xCheck );
   int    typey  = dimensionType( yCheck );
   int    typez  = 1;
   double rxscl  = ct_rxscale->value();
   double ryscl  = ct_ryscale->value();

   if ( plot3d_w == 0 )
   {
      if ( model == 0  ||  model->components.size() < 1 )
         return;

      plot3d_w = new US_Plot3D( this, model );
      connect( plot3d_w, SIGNAL( has_closed() ),
               this,     SLOT(   plot_close() ) );
   }

   plot3d_w->setTypes     ( typex, typey, typez );
   plot3d_w->setParameters( zscale, gridres, pksmooth, pkwidth, rxscl, ryscl );
   plot3d_w->replot       ( );

   plot3d_w->setVisible( true );
}

// close button clicked
void US_PlotControlPc::close_all()
{
   close();
}

// 3d plot closed
void US_PlotControlPc::plot_close()
{
   plot3d_w = 0;
}

int US_PlotControlPc::dimensionType( QVector< QCheckBox* >& xycheck )
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

