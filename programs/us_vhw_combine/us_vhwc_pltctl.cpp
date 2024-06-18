//! \file us_vhwc_pltctl.cpp

#include "us_vhw_combine.h"
#include "us_vhwc_pltctl.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_constants.h"

#include <qwt_legend.h>

#ifndef ZSCLF_CE // Plot scale adjusts: Contour/Points Envel/Integ
#define ZSCLF_CE  0.07
#define XSCLF_CE  0.13
#define YSCLF_CE  0.70
#define ZSCLF_CI  0.50
#define XSCLF_CI  0.16
#define YSCLF_CI  0.80
#define ZSCLF_PE  0.10
#define XSCLF_PE  0.12
#define YSCLF_PE  1.00
#define ZSCLF_PI  0.45
#define XSCLF_PI  0.16
#define YSCLF_PI  1.00
#endif

// Constructor:  enhanced spectra plot control widget
US_VhwCPlotControl::US_VhwCPlotControl( QWidget* p, QVector< QVector3D >* d,
      int a_ptype )
   : US_WidgetsDialog( 0, 0 )
{
qDebug() << "PCtrl: IN";
   wparent        = p;
   xyzdat         = d;
   p_type         = a_ptype;
   envel          = ( p_type == 0 );
qDebug() << "PCtrl: xyzdat count" << xyzdat->count();

   setObjectName( "US_VhwCPlotControl" );
   setAttribute( Qt::WA_DeleteOnClose, true );
   setPalette( US_GuiSettings::frameColor() );
   setFont( QFont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() ) );

   // Lay out the GUI
   setWindowTitle( tr( "3-D Plotting Controls" ) );

   mainLayout      = new QVBoxLayout( this );
   controlsLayout  = new QGridLayout( );
   buttonsLayout   = new QHBoxLayout( );

   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   mainLayout->addLayout( controlsLayout );
   mainLayout->addLayout( buttonsLayout  );

   QLabel*      lb_zscalefac = us_label(  tr( "Z Scale Factor:"   ) );
   QLabel*      lb_rxscale   = us_label(  tr( "Relative X Scale:" ) );
   QLabel*      lb_ryscale   = us_label(  tr( "Relative Y Scale:" ) );
   QLabel*      lb_peaksmoo  = us_label(  tr( "Peak Smoothing"    ) );
   QLabel*      lb_peakwid   = us_label(  tr( "Peak Width:"       ) );
   QLabel*      lb_gridres   = us_label(  tr( "Grid Resolution:"  ) );

   QPushButton* pb_plot3d    = us_pushbutton( tr( "3D Plot" ) );
   QPushButton* pb_help      = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_close     = us_pushbutton( tr( "Close" ) );

   QLayout*     lo_yrevrs    = us_checkbox( tr( "Reverse Y"    ), ck_yrevrs,
                                            false );
   QLayout*     lo_contour   = us_checkbox( tr( "Contour Plot" ), ck_contour,
                                            true );

   ct_peaksmoo  = us_counter( 3,    1,  200,    1 );
   ct_peakwid   = us_counter( 3, 0.01, 10.0, 0.01 );
   ct_gridres   = us_counter( 3,   50,  600,   10 );
   ct_zscalefac = us_counter( 3, 0.01, 1000, 0.01 );
   ct_rxscale   = us_counter( 3, 0.01, 1000, 0.01 );
   ct_ryscale   = us_counter( 3, 0.01, 1000, 0.01 );

   int row = 0;
   controlsLayout->addWidget( lb_zscalefac, row,   0, 1, 2 );
   controlsLayout->addWidget( ct_zscalefac, row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_rxscale,   row,   0, 1, 2 );
   controlsLayout->addWidget( ct_rxscale,   row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_ryscale,   row,   0, 1, 2 );
   controlsLayout->addWidget( ct_ryscale,   row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_peaksmoo,  row,   0, 1, 2 );
   controlsLayout->addWidget( ct_peaksmoo,  row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_peakwid,   row,   0, 1, 2 );
   controlsLayout->addWidget( ct_peakwid,   row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_gridres,   row,   0, 1, 2 );
   controlsLayout->addWidget( ct_gridres,   row++, 2, 1, 2 );
   controlsLayout->addLayout( lo_contour,   row,   0, 1, 2 );
   controlsLayout->addLayout( lo_yrevrs,    row++, 2, 1, 2 );

   buttonsLayout->addWidget( pb_plot3d );
   buttonsLayout->addWidget( pb_help   );
   buttonsLayout->addWidget( pb_close  );

   zscale   = 1.0;
   rxscale  = 1.0;
   ryscale  = 1.0;
   pksmooth = 80.0;
   pkwidth  = 0.1;
   gridres  = 150;
   ct_zscalefac->setValue( zscale   );
   ct_rxscale  ->setValue( rxscale  );
   ct_ryscale  ->setValue( ryscale  );
   ct_peaksmoo ->setValue( pksmooth );
   ct_peakwid  ->setValue( pkwidth  );
   ct_gridres  ->setValue( gridres  );
   ct_zscalefac->setSingleStep(  0.01 );
   ct_rxscale  ->setSingleStep(  0.01 );
   ct_ryscale  ->setSingleStep(  0.01 );
   ct_peaksmoo ->setSingleStep(     1 );
   ct_peakwid  ->setSingleStep(  0.01 );
   ct_gridres  ->setSingleStep(    10 );
   ct_peaksmoo ->setEnabled( !ck_contour->isChecked() );
   ct_peakwid  ->setEnabled( !ck_contour->isChecked() );
   ct_gridres  ->setEnabled( !ck_contour->isChecked() );

   connect( pb_plot3d,   SIGNAL( clicked()   ),
            this,        SLOT  ( plot3_btn() ) );
   connect( ck_contour,  SIGNAL( toggled     ( bool ) ),
            this,        SLOT  ( ptype_toggle( bool ) ) );
   connect( pb_help,     SIGNAL( clicked()   ),
            this,        SLOT  ( help()      ) );
   connect( pb_close,    SIGNAL( clicked()   ),
            this,        SLOT  ( close_all() ) );

   plot3d_w = 0;

   ct_zscalefac->setMinimumWidth( lb_zscalefac->width() );
   adjustSize();
qDebug() << "PCtrl:  w size" << size();
}

// Make changes based on plot type (contour/point) toggled
void US_VhwCPlotControl::ptype_toggle( bool chkd )
{
   ct_peaksmoo ->setEnabled( !chkd );
   ct_peakwid  ->setEnabled( !chkd );
   ct_gridres  ->setEnabled( !chkd );
}

// Return caller of plot_control
QWidget* US_VhwCPlotControl::caller( void )
{
   return wparent;
}

// Public slot to force (re-)plot of currently specified 3-D plot
void US_VhwCPlotControl::do_3dplot()
{
   plot3_btn();
}

// Public slot to return a pointer to the 3D plot data widget
QGLWidget* US_VhwCPlotControl::data_3dplot( void )
{
   QGLWidget* widgP = (QGLWidget*)0;
   widgP            = ( plot3d_w != 0 ) ? plot3d_w->dataWidgetP() : widgP;

   return widgP;
}

// Public slot to return a pointer to the 3D plot main widget
US_Plot3D* US_VhwCPlotControl::widget_3dplot( void )
{
   return plot3d_w;
}

// 3D Plot button clicked
void US_VhwCPlotControl::plot3_btn()
{
qDebug() << "PCtrl:  plot3_btn";
   QString wtitle  = tr( "Multiwavelength 3-D vHW Viewer" );
   QString ptitle  = tr( "MWL 3-D Plot, vHW Distributions" );
   QString xatitle = tr( "Sed.C.(*e+13)" );
   QString yatitle = tr( "Lambda(nm)" );
   QString zatitle = envel             ? tr( "Concen."  ) :
                     ( ( p_type == 1 ) ? tr( "B.Frac."  ) :
                                         tr( "BF*Conc." ) );
   zscale          = ct_zscalefac->value();
   rxscale         = ct_rxscale  ->value();
   ryscale         = ct_ryscale  ->value();
   ryscale         = ( ck_yrevrs->isChecked() ) ? -ryscale : ryscale;
qDebug() << "PCtrl:  plot3_btn: scales" << rxscale << ryscale << zscale;
   bool contour    = ck_contour->isChecked();
   pksmooth        = contour ? 0.0 : ct_peaksmoo ->value();
   pkwidth         = contour ? 0.0 : ct_peakwid  ->value();
qDebug() << "PCtrl:  plot3_btn: smoo.." << pksmooth << pkwidth << gridres;

   int nidpt       = xyzdat->count();
qDebug() << "PCtrl:  plot3_btn: nidpt" << nidpt;
   QList< double > yvals;

   for ( int ii = 0; ii < nidpt; ii++ )
   {  // Accumulate unique Y values so that we can count columns
      double yval     = xyzdat->at( ii ).y();
      if ( ! yvals.contains( yval ) )
         yvals << yval;
   }

   int ncol        = yvals.count();  // Column count is number of unique Y's
   int nrow        = nidpt / ncol;   // Row count is Total/Columns
   gridres         = contour ? (int)ct_gridres->value() : nrow;
qDebug() << "PCtrl:  ncol nrow" << ncol << nrow << "gridres" << gridres;
   ncol            = contour ? ncol : gridres;
   nrow            = contour ? nrow : gridres;

   if ( plot3d_w == 0 )
   {  // If no 3-D plot window, bring it up now
      plot3d_w     = new US_Plot3D( this, nullptr, xyzdat );
      // Position so a corner is near a control dialog corner
      int cx       = x();
      int cy       = y();
      int ncx      = width();
      int ncy      = height();
      int nhdx     = qApp->desktop()->width()  / 2;
      int nhdy     = qApp->desktop()->height() / 2;
      int nwx      = plot3d_w->width();
      int nwy      = plot3d_w->height();
      int wx       = ( cx < nhdx ) ? ( cx + ncx + 20 ) : ( cx - nwx - 20 );
      int wy       = ( cy < nhdy ) ? ( cy + ncy + 20 ) : ( cy - nwy - 20 );
      plot3d_w->move( wx, wy );
   }

   // Set plot window parameters; do initial plot; and make it visible
   if ( contour )
   {  // Contour plot scale adjustments Envel/Integ
      zscale         *= ( envel ? ZSCLF_CE : ZSCLF_CI );
      rxscale        *= ( envel ? XSCLF_CE : XSCLF_CI );
      ryscale        *= ( envel ? YSCLF_CE : YSCLF_CI );
   }
   else
   {  // Points plot scale adjustments Envel/Integ
      zscale         *= ( envel ? ZSCLF_PE : ZSCLF_PI );
      rxscale        *= ( envel ? XSCLF_PE : XSCLF_PI );
      ryscale        *= ( envel ? YSCLF_PE : YSCLF_PI );
   }

   plot3d_w->setTitles    ( wtitle, ptitle, xatitle, yatitle, zatitle );
   plot3d_w->setParameters( ncol, nrow, rxscale, ryscale, zscale,
                            pksmooth, pkwidth );
   plot3d_w->replot       ( false );

   plot3d_w->setVisible( true );
}

// Close button clicked
void US_VhwCPlotControl::close_all()
{
   if ( plot3d_w != 0 )      // Close 3-D plot window if need be
      plot3d_w->close();

   emit has_closed();        // Send a signal of the closing

   close();                  // And go away
}

