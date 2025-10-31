//! \file us_exclude_profile.cpp

#include "us_edit_scan.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#if QT_VERSION < 0x050000
#define setSamples(a,b,c)  setRawData(a,b,c)
#define setSymbol(a)       setSymbol(*a)
#endif

US_EditScan::US_EditScan( US_DataIO::Scan&         s, 
                          const QVector< double >& r,
                          double invertValue, double left, double right )
   : US_WidgetsDialog( nullptr, Qt::WindowFlags() ), originalScan( s ), allRadii( r ),
                               invert( invertValue ), 
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
   int  size = originalScan.rvalues.size();
   radii     = new double[ size ];
   values    = new double[ size ];

   curve = us_curve( data_plot, tr( "Scan Curve" ) );

   fgPen = QPen( US_GuiSettings::plotCurve()    );

   QwtSymbol* fgSym = new QwtSymbol;
   fgSym->setStyle( QwtSymbol::Ellipse );
   fgSym->setBrush( US_GuiSettings::plotCurve() );
   fgSym->setPen  ( fgPen );
   fgSym->setSize ( 6 );

   curve->setSymbol ( fgSym );
   curve->attach    ( data_plot );

   redraw();

   // Instructions
   QHBoxLayout* instructions = new QHBoxLayout;
   
   QLabel* lb_instructions = us_label( tr( "Step-by-Step\nInstructions:" ) );
   instructions->addWidget( lb_instructions );

   QTextEdit* te_instructions = us_textedit();
   us_setReadOnly( te_instructions, true );
   
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

   connect( pick, SIGNAL( cMouseUp     ( const QPointF& ) ),
                  SLOT  ( end_drag     ( const QPointF& ) ) );
   
   connect( pick, SIGNAL( cMouseDrag   ( const QPointF& ) ),
                  SLOT  ( drag         ( const QPointF& ) ) );
}

void US_EditScan::done( void )
{
   emit scan_updated( changes );
   close();
}

void US_EditScan::drag( const QPointF& p )
{
//qDebug() << "drag() dragging" << dragging;
   // Ignore drag events after Mouse Up
   if ( ! dragging ) return;

   values[ point ] = p.y();
#if QT_VERSION > 0x050000
   curve->setSamples( radii, values, count );
#endif
   data_plot->replot();
}

void US_EditScan::start_drag( QMouseEvent* e )
{
   dragging = true;

   // Find the nearest point
   point = curve->closestPoint( e->pos() );
//qDebug() << " start_drag : point" << point << "v(p)" << values[point];
}

void US_EditScan::end_drag( const QPointF& p )
{
   dragging = false;

   // Save the change
   values[ point ] = p.y();
   // Use offset to provide an index to the full data set
   changes << QPointF( (double)( point + offset ), values[ point ] );

//qDebug() << " end_drag() point" << point << "values[point]" << values[point];
#if QT_VERSION > 0x050000
   curve->setSamples( radii, values, count );
#endif
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
   count      = 0;

   int indexLeft  = US_DataIO::index( allRadii, range_left );
   int indexRight = US_DataIO::index( allRadii, range_right );
   
   offset = indexLeft;

   for ( int j = indexLeft; j <= indexRight; j++ ) 
   { 
      radii [ count ] = allRadii[ j ];
      values[ count ] = workingScan.rvalues[ j ] * invert;
      count++;
   }

   curve->setSamples( radii, values, count );
   data_plot->replot();
}
