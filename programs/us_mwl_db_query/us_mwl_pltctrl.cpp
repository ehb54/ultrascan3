//! \file us_mwl_pltctrl.cpp

#include "us_mwlr_viewer.h"
#include "us_mwl_pltctrl.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_constants.h"

#include <qwt_legend.h>

#ifndef ZSCAL_FACT
#define ZSCAL_FACT 0.008
#define RXSCL_FACT 0.032
#define RYSCL_FACT 0.900
#endif

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

   QLabel*      lb_zscalefac = us_label(  tr( "Z Scale Factor:" ) );
   QLabel*      lb_rxscale   = us_label(  tr( "Relative X Scale:" ) );
   QLabel*      lb_ryscale   = us_label(  tr( "Relative Y Scale:" ) );

   QPushButton* pb_plot3d    = us_pushbutton( tr( "3D Plot" ) );
   QPushButton* pb_help      = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_close     = us_pushbutton( tr( "Close" ) );

   QLayout*     lo_yrevrs    = us_checkbox( tr( "Reverse Y" ), ck_yrevrs,
                                            false );

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
   controlsLayout->addLayout( lo_yrevrs,    row++, 2, 1, 2 );

   buttonsLayout->addWidget( pb_plot3d );
   buttonsLayout->addWidget( pb_help   );
   buttonsLayout->addWidget( pb_close  );

   zscale   = 1.0;
   rxscale  = 1.0;
   ryscale  = 1.0;
   ct_zscalefac->setValue( zscale   );
   ct_rxscale  ->setValue( rxscale  );
   ct_ryscale  ->setValue( ryscale  );
   ct_zscalefac->setSingleStep( 0.01 );
   ct_rxscale  ->setSingleStep( 0.01 );
   ct_ryscale  ->setSingleStep( 0.01 );

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

// Return caller of plot_control
QWidget* US_MwlPlotControl::caller( void )
{
   return wparent;
}

// Public slot to force (re-)plot of currently specified 3-D plot
void US_MwlPlotControl::do_3dplot()
{
   plot3_btn();
}

// Public slot to return a pointer to the 3D plot data widget
QGLWidget* US_MwlPlotControl::data_3dplot( void )
{
   QGLWidget* widgP = (QGLWidget*)0;
   widgP            = ( plot3d_w != 0 ) ? plot3d_w->dataWidgetP() : widgP;

   return widgP;
}

// Public slot to return a pointer to the 3D plot main widget
US_Plot3D* US_MwlPlotControl::widget_3dplot( void )
{
   return plot3d_w;
}

// 3D Plot button clicked
void US_MwlPlotControl::plot3_btn()
{
qDebug() << "PCtrl:  plot3_btn";
   QString wtitle  = tr( "Multiwavelength 3-Dimensional Viewer" );
   QString ptitle  = tr( "MWL 3-D Plot, Scan 1" );
   QString xatitle = tr( "Radius(cm)" );
   QString yatitle = tr( "Lambda(nm)" );
   QString zatitle = tr( "Intensity" );
   zscale          = ct_zscalefac->value() * ZSCAL_FACT;
   rxscale         = ct_rxscale  ->value() * RXSCL_FACT;
   ryscale         = ct_ryscale  ->value() * RYSCL_FACT;
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

   int nrow        = xvals.count();  // Row count is number of unique X's
   int ncol        = nidpt / nrow;   // Column count is Total/Rows

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
   plot3d_w->setTitles    ( wtitle, ptitle, xatitle, yatitle, zatitle );
   plot3d_w->setParameters( ncol, nrow, rxscale, ryscale, zscale );
   plot3d_w->replot       ( false );

   plot3d_w->setVisible( true );
}

// Close button clicked
void US_MwlPlotControl::close_all()
{
   if ( plot3d_w != 0 )      // Close 3-D plot window if need be
      plot3d_w->close();

   emit has_closed();        // Send a signal of the closing

   close();                  // And go away
}

