//! \file us_exclude_profile.cpp

#include "us_edit_scan.h"
#include "us_settings.h"
#include "us_gui_settings.h"

US_EditScan::US_EditScan( US_DataIO::scan& s, 
                          double           invertValue,
                          double           left,
                          double           right )
   : US_WidgetsDialog( 0, 0 ), originalScan( s ), invert( invertValue ), 
                               range_left( left ), range_right( right )
{
   dragging    = false;
   workingScan = originalScan;
   changes.clear();

   setWindowTitle( tr( "Scan Editor" ) );
   setPalette( US_GuiSettings::frameColor() );

   QVBoxLayout* main = new QVBoxLayout( this );
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );

   // Plot Rows
   QBoxLayout* plot = new US_Plot( data_plot,
      tr( "Edit Single Scan" ),
      tr( "Radius (cm)" ),
      tr( "Absorbance" ) );
   
   data_plot->setMinimumSize( 500, 300 );
   main->addLayout( plot );

   us_grid( data_plot );
   pick = new US_PlotPicker( data_plot );

   // Draw the curve
   int  size = originalScan.values.size();
   radii     = new double[ size ];
   values    = new double[ size ];

   curve = us_curve( data_plot, tr( "Scan Curve" ) );

   fgPen = QPen( US_GuiSettings::plotCurve()    );
   bgPen = QPen( US_GuiSettings::plotCanvasBG() );

   fgSym.setStyle( QwtSymbol::Ellipse );
   fgSym.setBrush( US_GuiSettings::plotCurve() );
   fgSym.setPen  ( fgPen );
   fgSym.setSize ( 6 );


   bgSym = fgSym;
   bgSym.setBrush( US_GuiSettings::plotCanvasBG() );
   bgSym.setPen  ( bgPen );

   //curve->setRawData( radii, values, count );
   curve->setSymbol ( fgSym );
   curve->attach    ( data_plot );

   //data_plot->replot();
   redraw();

   // Instructions
   QHBoxLayout* instructions = new QHBoxLayout;
   
   QLabel* lb_instructions = us_label( tr( "Step-by-Step\nInstructions:" ) );
   instructions->addWidget( lb_instructions );

   QTextEdit* te_instructions = us_textedit();
   te_instructions->setReadOnly( true );
   
   QFont        f = te_instructions->font();
   QFontMetrics fm( f );

   te_instructions->setMaximumHeight( fm.lineSpacing() * 5 );
   
   te_instructions->setText( 
         tr( "1. Use the zoom function as needed to focus on a point.\n"
             "2. Use Control-Left Press to select the point desired.\n"
             "3. Drag the point to the value desired." ) );
           
   instructions->addWidget( te_instructions );
   main        ->addLayout( instructions );

   // Button Row
   QHBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_reset = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset, SIGNAL( clicked() ), SLOT( reset() ) );
   buttons->addWidget( pb_reset );

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   buttons->addWidget( pb_help );

   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   connect( pb_cancel, SIGNAL( clicked() ), SLOT( close() ) );
   buttons->addWidget( pb_cancel );

   QPushButton* pb_accept = us_pushbutton( tr( "Accept" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( done() ) );
   buttons->addWidget( pb_accept );

   main->addLayout( buttons );

   connect( pick, SIGNAL( cMouseDownRaw( QMouseEvent* ) ),
                  SLOT  ( start_drag   ( QMouseEvent* ) ) );

   connect( pick, SIGNAL( cMouseUp     ( const QwtDoublePoint& ) ),
                  SLOT  ( end_drag     ( const QwtDoublePoint& ) ) );
   
   connect( pick, SIGNAL( cMouseDrag   ( const QwtDoublePoint& ) ),
                  SLOT  ( drag         ( const QwtDoublePoint& ) ) );
}

void US_EditScan::done( void )
{
   emit scan_updated( changes );
   close();
}

void US_EditScan::drag( const QwtDoublePoint& p )
{
   // Ignore drag events after Mouse Up
   if ( ! dragging ) return;

   values[ point ] = p.y();
   data_plot->replot();
}

void US_EditScan::start_drag( QMouseEvent* e )
{
   dragging = true;

   // Find the nearest point
   point = curve->closestPoint( e->pos() );
}

void US_EditScan::end_drag( const QwtDoublePoint& p )
{
   dragging = false;

   // Save the change
   values[ point ] = p.y();
   // Use offset to provide an index to the full data set
   changes << QPointF( (double)( point + offset ), values[ point ] );

   data_plot->replot();   
}

void US_EditScan::reset( void )
{
   dragging    = false;
   workingScan = originalScan;
   changes.clear();
   redraw();
}

void US_EditScan::redraw( void )
{
   offset     = 0;
   int  count = 0;

   for ( int j = 0; j < workingScan.values.size(); j++ ) 
   { 
      double r = workingScan.values[ j ].d.radius;
      if ( r < range_left  )
      {
         offset = j;
         continue;
      }

      if ( r > range_right ) break;

      radii [ count ] = r;
      values[ count ] = workingScan.values[ j ].value * invert;
      count++;
   }

   offset++;  // Fix off by 1 issue

   curve->setRawData( radii, values, count );
   data_plot->replot();
}
