//! \file us_exclude_profile.cpp

#include "us_edit_scan.h"
#include "us_settings.h"
#include "us_gui_settings.h"

US_EditScan::US_EditScan( scan& s, const double invertValue )
   : US_WidgetsDialog( 0, 0 ), originalScan( s ), invert( invertValue )
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
   uint    size  = originalScan.values.size();
   int     count = 0;
   radii         = new double[ size ];
   values        = new double[ size ];

   for ( uint j = 0; j < size; j++ ) 
   { 
      radii [ count ] = workingScan.values[ j ].d.radius;
      values[ count ] = workingScan.values[ j ].value * invert;
      count++;
   }

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

   curve->setRawData( radii, values, count );
   curve->setSymbol ( fgSym );
   curve->attach    ( data_plot );

   data_plot->replot();

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

   //qDebug() << "Drag" << p; 

   // Draw old curve with background color
   curve->setPen   ( bgPen );
   curve->setSymbol( bgSym );
   curve->draw( point - 1, point + 1 );

   // Redraw the relevant part of the graph
   curve->setPen   ( fgPen );
   curve->setSymbol( fgSym );
   
   values[ point ] = p.y();
   curve->draw( point - 1, point + 1 );
}

void US_EditScan::start_drag( QMouseEvent* e )
{
   dragging = true;

   // Find the nearest point
   point = curve->closestPoint( e->pos() );
   //qDebug() << "Down"  << point << radii[ point ] << values[ point ]; 
}

void US_EditScan::end_drag( const QwtDoublePoint& p )
{
   dragging = false;

   // Save the change
   values[ point ] = p.y();
   changes << QPointF( (double) point, values[ point ] );
   
   data_plot->replot();   
   //qDebug() << "Up" << changes; 
}

void US_EditScan::reset( void )
{
   dragging    = false;
   workingScan = originalScan;
   changes.clear();

   for ( uint j = 0; j < workingScan.values.size(); j++ ) 
   { 
      radii [ j ] = workingScan.values[ j ].d.radius;
      values[ j ] = workingScan.values[ j ].value * invert;
   }

   data_plot->replot();
}
