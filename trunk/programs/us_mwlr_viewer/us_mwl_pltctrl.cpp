//! \file us_mwl_pltctrl.cpp

#include "us_mwlr_viewer.h"
#include "us_mwl_pltctrl.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_constants.h"

#include <qwt_legend.h>

// constructor:  enhanced plot control widget
US_MwlPlotControl::US_MwlPlotControl( QWidget* p, QVector< QVector3D >* d )
   : US_WidgetsDialog( 0, 0 )
{
qDebug() << "PCtrl: IN";
   wparent        = p;
   xyzdat         = d;
qDebug() << "PCtrl: xyzdat count" << xyzdat->count();

   setObjectName( "US_MwlPlotControl" );
   setAttribute( Qt::WA_DeleteOnClose, true );
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

   QLabel*      lb_gridreso  = us_label(  tr( "Grid Resolution:" ) );
   QLabel*      lb_peaksmoo  = us_label(  tr( "Peak Smoothing:" ) );
   QLabel*      lb_peakwidth = us_label(  tr( "Peak Width:" ) );
   QLabel*      lb_zscalefac = us_label(  tr( "Z-Scaling Factor:" ) );
   QLabel*      lb_rxscale   = us_label(  tr( "Relative X Scale:" ) );
   QLabel*      lb_ryscale   = us_label(  tr( "Relative Y Scale:" ) );

   QPushButton* pb_plot3d    = us_pushbutton( tr( "3D Plot" ) );
   QPushButton* pb_help      = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_close     = us_pushbutton( tr( "Close" ) );

   QLayout*     lo_yrevrs    = us_checkbox( tr( "Reverse Y" ), ck_yrevrs,false);

                ct_gridreso  = us_counter( 3,   50,  300,   10 );
                ct_peaksmoo  = us_counter( 3,    1,  200,    1 );
                ct_peakwidth = us_counter( 3, 0.01, 10.0, 0.01 );
                ct_zscalefac = us_counter( 3, 0.01, 1000, 0.01 );
                ct_rxscale   = us_counter( 3, 0.01, 1000, 0.01 );
                ct_ryscale   = us_counter( 3, 0.01, 1000, 0.01 );

   int row = 0;
   controlsLayout->addWidget( lb_gridreso,  row,   0, 1, 2 );
   controlsLayout->addWidget( ct_gridreso,  row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_peaksmoo,  row,   0, 1, 2 );
   controlsLayout->addWidget( ct_peaksmoo,  row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_peakwidth, row,   0, 1, 2 );
   controlsLayout->addWidget( ct_peakwidth, row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_zscalefac, row,   0, 1, 2 );
   controlsLayout->addWidget( ct_zscalefac, row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_rxscale,   row,   0, 1, 2 );
   controlsLayout->addWidget( ct_rxscale,   row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_ryscale,   row,   0, 1, 2 );
   controlsLayout->addWidget( ct_ryscale,   row++, 2, 1, 2 );
   controlsLayout->addLayout( lo_yrevrs,    row++, 2, 1, 2 );

   buttonsLayout->addWidget( pb_plot3d );
   buttonsLayout->addWidget( pb_help   );
   buttonsLayout->addWidget( pb_close  );

   gridres  = 150.0;
   pksmooth = 80.0;
   pkwidth  = 0.1;
   zscale   = 1.0;
   rxscale  = 1.0;
   ryscale  = 1.0;
   ct_gridreso ->setValue( gridres  );
   ct_peaksmoo ->setValue( pksmooth );
   ct_peakwidth->setValue( pkwidth  );
   ct_zscalefac->setValue( zscale   );
   ct_rxscale  ->setValue( rxscale  );
   ct_ryscale  ->setValue( ryscale  );
   ct_gridreso ->setStep(  10   );
   ct_peaksmoo ->setStep(  1    );
   ct_peakwidth->setStep(  0.01 );
   ct_zscalefac->setStep(  0.01 );
   ct_rxscale  ->setStep(  0.01 );
   ct_ryscale  ->setStep(  0.01 );
lb_gridreso ->setVisible(false);
ct_gridreso ->setVisible(false);
lb_peaksmoo ->setVisible(false);
ct_peaksmoo ->setVisible(false);
lb_peakwidth->setVisible(false);
ct_peakwidth->setVisible(false);

   connect( ct_gridreso , SIGNAL( valueChanged( double ) ),
            this,         SLOT  ( gridr_value ( double ) ) );
   connect( ct_peaksmoo , SIGNAL( valueChanged( double ) ),
            this,         SLOT  ( peaks_value ( double ) ) );
   connect( ct_peakwidth, SIGNAL( valueChanged( double ) ),
            this,         SLOT  ( peakw_value ( double ) ) );
   connect( ct_zscalefac, SIGNAL( valueChanged( double ) ),
            this,         SLOT  ( zscal_value ( double ) ) );

   connect( ck_yrevrs,    SIGNAL( toggled     ( bool   ) ),
            this,         SLOT  ( yvbaCheck   ( bool   ) ) );

   connect( pb_plot3d, SIGNAL( clicked()   ),
            this,      SLOT  ( plot3_btn() ) );
   connect( pb_help,   SIGNAL( clicked()   ),
            this,      SLOT  ( help()      ) );
   connect( pb_close,  SIGNAL( clicked()   ),
            this,      SLOT  ( close_all() ) );

   plot3d_w = 0;

   ct_zscalefac->setMinimumWidth( lb_zscalefac->width() );
   adjustSize();
qDebug() << "PCtrl:  w size" << size();
}

// return caller of plot_control
QWidget* US_MwlPlotControl::caller( void )
{
   return wparent;
}

// public slot to force (re-)plot of currently specified 3-D plot
void US_MwlPlotControl::do_3dplot()
{
   plot3_btn();
}

// Public slot to return a pointer to the 3D plot data widget
QGLWidget* US_MwlPlotControl::data_3dplot( void )
{
   QGLWidget* widgP = (QGLWidget*)0;

   if ( plot3d_w != 0 )
   {
      //plot3_btn();
      widgP = plot3d_w->dataWidgetP();
      //widgP = NULL;
   }

   return widgP;
}

// Public slot to return a pointer to the 3D plot main widget
US_Plot3Dxyz* US_MwlPlotControl::widget_3dplot( void )
{
//   return (US_Plot3D*)plot3d_w;
   return plot3d_w;
}

// mw x box checked
void US_MwlPlotControl::xmwtCheck( bool chkd )
{
   checkSet( chkd, true,  0 );
}
// mw y box checked
void US_MwlPlotControl::ymwtCheck( bool chkd )
{
   checkSet( chkd, false, 0 );
}
// sedcoeff x box checked
void US_MwlPlotControl::xsedCheck( bool chkd )
{
   checkSet( chkd, true,  1 );
}
// sedcoeff y box checked
void US_MwlPlotControl::ysedCheck( bool chkd )
{
   checkSet( chkd, false, 1 );
}
// diffcoeff x box checked
void US_MwlPlotControl::xdifCheck( bool chkd )
{
   checkSet( chkd, true,  2 );
}
// diffcoeff y box checked
void US_MwlPlotControl::ydifCheck( bool chkd )
{
   checkSet( chkd, false, 2 );
}
// friccoeff x box checked
void US_MwlPlotControl::xfcoCheck( bool chkd )
{
   checkSet( chkd, true,  3 );
}
// friccoeff y box checked
void US_MwlPlotControl::yfcoCheck( bool chkd )
{
   checkSet( chkd, false, 3 );
}
// fricratio x box checked
void US_MwlPlotControl::xfraCheck( bool chkd )
{
   checkSet( chkd, true,  4 );
}
// fricratio y box checked
void US_MwlPlotControl::yfraCheck( bool chkd )
{
   checkSet( chkd, false, 4 );
}
// Vbar x box checked
void US_MwlPlotControl::xvbaCheck( bool chkd )
{
   checkSet( chkd, true,  5 );
}
// Vbar y box checked
void US_MwlPlotControl::yvbaCheck( bool chkd )
{
   checkSet( chkd, false, 5 );
}

// handle any x or y box checked
void US_MwlPlotControl::checkSet( bool chkd, bool isX, int row )
{
   if ( !chkd )                                // only respond if checked
      return;

   if ( isX )
   {  // this is a X box
      for ( int ii = 0; ii < 6; ii++ )
      {  // review all check boxes
         if ( ii != row )
         {  // in other rows, uncheck X, enable Y
         }

         else
         {  // in this row, disable the Y
         }
      }
   }
}

// z scale factor changed
void US_MwlPlotControl::zscal_value( double value )
{
   zscale   = value;
}
// grid resolution changed
void US_MwlPlotControl::gridr_value( double value )
{
   gridres  = value;
}
// peak smoothing changed
void US_MwlPlotControl::peaks_value( double value )
{
   pksmooth = value;
}
// peak width changed
void US_MwlPlotControl::peakw_value( double value )
{
   pkwidth  = value;
}

// 3d plot button clicked
void US_MwlPlotControl::plot3_btn()
{
qDebug() << "PCtrl:  plot3_btn";
   QString wtitle  = tr( "Multiwavelength 3-Dimensional Viewer" );
   QString ptitle  = tr( "MWL 3-D Plot" );
   QString xatitle = tr( "Radius(cm)" );
   QString yatitle = tr( "Lambda(nm)" );
   QString zatitle = tr( "Intensity" );
   rxscale         = ct_rxscale->value();
   ryscale         = ct_ryscale->value();
   ryscale         = ( ck_yrevrs->isChecked() ) ? -ryscale : ryscale;
qDebug() << "PCtrl:  plot3_btn: scales" << rxscale << ryscale << zscale;
   int nidpt       = xyzdat->count();
   QList< double > xvals;

   for ( int ii = 0; ii < nidpt; ii++ )
   {  // Accumulate unique X values so that we can count rows
      double xval     = xyzdat->at( ii ).x();
      if ( ! xvals.contains( xval ) )
         xvals << xval;
   }

   int nrow        = xvals.count();
   int ncol        = nidpt / nrow;

   if ( plot3d_w == 0 )
   {
      plot3d_w = new US_Plot3Dxyz( this, xyzdat );
   }

   plot3d_w->setTitles    ( wtitle, ptitle, xatitle, yatitle, zatitle );
   plot3d_w->setParameters( ncol, nrow, rxscale, ryscale, zscale );
   plot3d_w->replot       ( );

   plot3d_w->setVisible( true );
}

// close button clicked
void US_MwlPlotControl::close_all()
{
   if ( plot3d_w != 0 )
      plot3d_w->close();

   emit has_closed();

   close();
}

int US_MwlPlotControl::dimensionType( QVector< QCheckBox* >& xycheck )
{
   int dimType = 1;

   for ( int ii = 0; ii < xycheck.size(); ii++ )
   {
      dimType = ii + 1;
      break;
   }

   return dimType;
}

