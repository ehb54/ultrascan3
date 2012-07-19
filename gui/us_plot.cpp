//! \file us_plot.cpp

#include <QtSvg>

#include "us_plot.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_pixmaps.h"
#include "us_settings.h"

#include "qwt_text_label.h"
#include "qwt_plot_layout.h"
#include "qwt_legend.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_map.h"
#include "qwt_scale_widget.h"
#include "qwt_symbol.h"


US_Zoomer::US_Zoomer( int xAxis, int yAxis, QwtPlotCanvas* canvas )
   : QwtPlotZoomer( xAxis, yAxis, canvas )
{
   setSelectionFlags( QwtPicker::DragSelection | QwtPicker::CornerToCorner );
   setTrackerMode   ( QwtPicker::AlwaysOff );
   setRubberBand    ( QwtPicker::NoRubberBand );

   // RightButton: zoom out by 1
   // Ctrl+RightButton: zoom out to full size

   setMousePattern( QwtEventPattern::MouseSelect2,
                    Qt::RightButton, Qt::ControlModifier );

   setMousePattern( QwtEventPattern::MouseSelect3,
                    Qt::RightButton );
}

/*********************       US_Plot Class      *************************/

// A new plot returns a QBoxLayout
US_Plot::US_Plot( QwtPlot*& parent_plot, const QString& title,
      const QString& x_axis, const QString& y_axis ) : QHBoxLayout()
{
   zoomer = NULL;
   setSpacing( 0 );

   QFont buttonFont( US_GuiSettings::fontFamily(),
                     US_GuiSettings::fontSize() - 2 );

   // Add the tool bar 
   QToolBar* toolBar = new QToolBar;
   toolBar->setAutoFillBackground( true );
   toolBar->setPalette( US_GuiSettings::plotColor() );
   toolBar->setOrientation( Qt::Vertical );

   btnZoom = new QToolButton( toolBar );
   btnZoom->setText( "Zoom" );
   btnZoom->setIcon(QIcon( zoom_xpm ) );
   btnZoom->setCheckable( true );
   btnZoom->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
   btnZoom->setFont( buttonFont );
   connect( btnZoom, SIGNAL( toggled( bool ) ), SLOT( zoom( bool ) ) );

   QToolButton* btnPrint = new QToolButton( toolBar );
   btnPrint->setText( "Print" );
   btnPrint->setIcon( QIcon( print_xpm ) );
   btnPrint->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
   btnPrint->setFont( buttonFont );
   connect( btnPrint, SIGNAL( clicked() ), SLOT( print() ) );

   QToolButton* btnSVG = new QToolButton( toolBar );
   btnSVG->setText( "SVG" );
   btnSVG->setIcon( QIcon( vec_xpm ) );
   btnSVG->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
   btnSVG->setFont( buttonFont );
   connect( btnSVG, SIGNAL( clicked() ), SLOT( svg() ) );

   QToolButton* btnPNG = new QToolButton( toolBar );
   btnPNG->setText( "PNG" );
   btnPNG->setIcon( QIcon( ras_xpm ) );
   btnPNG->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
   btnPNG->setFont( buttonFont );
   connect( btnPNG, SIGNAL( clicked() ), SLOT( png() ) );

   QToolButton* btnConfig = new QToolButton( toolBar );
   btnConfig->setText( "Config" );
   btnConfig->setIcon(QIcon( configure_32_xpm ) );
   btnConfig->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
   btnConfig->setFont( buttonFont );
   connect( btnConfig, SIGNAL( clicked() ), SLOT( config() ) );

   toolBar->addWidget( btnZoom   );
   toolBar->addWidget( btnPrint  );
   toolBar->addWidget( btnSVG    );
   toolBar->addWidget( btnPNG    );
   toolBar->addWidget( btnConfig );

   addWidget( toolBar );

   // Add a 1 pixel black line between the tool bar and the plot
   QLabel* spacer = new QLabel;
   QPalette p;
   p.setColor( QPalette::Window, Qt::black );
   spacer->setPalette( p );
   spacer->setAutoFillBackground( true );
   spacer->setMaximumWidth( 1 );
   addWidget( spacer );

   plot        = new QwtPlot;
   parent_plot = plot;
   

   configWidget = NULL;
  
   plot->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
   plot->setAutoReplot( false );
  
   QFont font( US_GuiSettings::fontFamily(),
               US_GuiSettings::fontSize(),
               QFont::Bold );
           
   font.setPointSizeF( US_GuiSettings::fontSize() * 1.4 );

   QwtText qwtTitle( title );
   qwtTitle.setFont( font );
   plot->setTitle( qwtTitle );
  
   plot->setMargin( US_GuiSettings::plotMargin() );

   font.setPointSizeF( US_GuiSettings::fontSize() * 1.0 );
   qwtTitle.setFont( font );
   qwtTitle.setText( x_axis );  
   plot->setAxisTitle( QwtPlot::xBottom, qwtTitle );

   qwtTitle.setText( y_axis );
   plot->setAxisTitle( QwtPlot::yLeft  , qwtTitle );
  
   font.setBold ( false );
   font.setPointSizeF( US_GuiSettings::fontSize() * 0.9 );
   plot->setAxisFont( QwtPlot::xBottom, font );
   plot->setAxisFont( QwtPlot::yLeft,   font );
  
   plot->setAutoFillBackground( true );
   plot->setPalette ( US_GuiSettings::plotColor() );
   plot->setCanvasBackground( US_GuiSettings::plotCanvasBG() );

   addWidget( plot );
}

void US_Plot::zoom( bool on )
{
   if ( on )
   {
      // Set up for zooming
      zoomer = new US_Zoomer( QwtPlot::xBottom, QwtPlot::yLeft,
                              plot->canvas() );
      
      zoomer->setRubberBand   ( QwtPicker::RectRubberBand );
      zoomer->setRubberBandPen( QColor( Qt::green ) );
      zoomer->setTrackerMode  ( QwtPicker::ActiveOnly );
      zoomer->setTrackerPen   ( QColor( Qt::white ) );

      connect( zoomer, SIGNAL ( zoomed(        QwtDoubleRect ) ), 
                       SIGNAL ( zoomedCorners( QwtDoubleRect ) ) );
      
      panner = new QwtPlotPanner( plot->canvas() );
      panner->setMouseButton( Qt::MidButton );

      picker = new QwtPlotPicker( QwtPlot::xBottom, QwtPlot::yLeft,
                     QwtPicker::PointSelection | QwtPicker::DragSelection,
                     QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn,
                     plot->canvas() );

      picker->setRubberBandPen( QColor( Qt::green ) );
      picker->setRubberBand   ( QwtPicker::CrossRubberBand );
      picker->setTrackerPen   ( QColor( Qt::white ) );
   }
   
   panner->setEnabled( on );

   zoomer->setEnabled( on );
   zoomer->zoom( 0 );

   picker->setEnabled( ! on );

   if ( ! on  &&  zoomer != NULL )
   {
      delete picker;
      delete panner;
      delete zoomer;
      zoomer = NULL;
   }
}

void US_Plot::svg( void )
{
   QDir dir;
   QString reportDir = US_Settings::reportDir();
   if ( ! dir.exists( reportDir ) ) dir.mkpath( reportDir );

   QString fileName = QFileDialog::getSaveFileName( plot, 
      tr( "Export File Name" ), reportDir, 
      tr( "SVG Documents (*.svg)" ) );

   if ( ! fileName.isEmpty() )
   {
      if ( fileName.right( 4 ) != ".svg" ) fileName += ".svg";

      US_GuiUtil::save_svg( fileName, plot );
   }
}

void US_Plot::png( void )
{
   QDir dir;
   QString reportDir = US_Settings::reportDir();
   if ( ! dir.exists( reportDir ) ) dir.mkpath( reportDir );

   QString fileName = QFileDialog::getSaveFileName( plot, 
      tr( "Export File Name" ), reportDir, 
      tr( "PNG Documents (*.png)" ) );

   if ( ! fileName.isEmpty() )
   {
      if ( fileName.right( 4 ) != ".png" ) fileName += ".png";

      US_GuiUtil::save_png( fileName, plot );
   }
}

void US_Plot::print( void )
{
   QDir dir;
   QString reportDir = US_Settings::reportDir();
   if ( ! dir.exists( reportDir ) ) dir.mkpath( reportDir );

   QPrinter printer( QPrinter::HighResolution );
#ifdef Q_WS_X11
   printer.setOutputFileName( reportDir + "/ultrascan-plot.pdf" );
#endif

   QString docName = plot->title().text();
   
   if ( ! docName.isEmpty() )
   {
       docName.replace ( QRegExp( QString::fromLatin1( "\n" ) ), tr ( " -- " ) );
       printer.setDocName( docName );
   }

   printer.setCreator( "UltraScan" );
   printer.setOrientation( QPrinter::Landscape );

   QPrintDialog dialog( &printer );

   if ( dialog.exec() == QDialog::Accepted )
   {
       QwtPlotPrintFilter filter;
       if ( printer.colorMode() == QPrinter::GrayScale )
       {
           int options = QwtPlotPrintFilter::PrintAll;
           options    &= ~QwtPlotPrintFilter::PrintBackground;
           options    |= QwtPlotPrintFilter::PrintFrameWithScales;
           filter.setOptions( options );
       }
       plot->print( printer, filter );
   }
}

void US_Plot::config( void )
{
   if ( configWidget ) 
   {
      //configWidget->close();
   }
   configWidget = new US_PlotConfig( plot ); 
   //configWidget->setAttribute( Qt::WA_DeleteOnClose );
   //connect( configWidget, SIGNAL( plotConfigClosed  () ), 
   //                       SLOT  ( plotConfigFinished() ) );
   //configWidget->show();
   configWidget->exec();
   qApp->processEvents();
   delete configWidget;
}

/*
void US_Plot::plotConfigFinished( void )
{
   configWidget = NULL;
}

void US_Plot::quit( void )
{
   if ( configWidget ) 
   {
      //configWidget->close();
   }
}
*/

////////////////////////////////////////

US_PlotPushbutton::US_PlotPushbutton( const QString& labelString, 
      QWidget* w, int index ) : QPushButton( labelString.toAscii(), w )
{
   setFont( QFont( US_GuiSettings::fontFamily(),
                   US_GuiSettings::fontSize() ) );

   setPalette( US_GuiSettings::pushbColor() );

   setAutoDefault( false );
   setEnabled( true );
   pb_index = index;

   connect( this, SIGNAL( clicked() ), SLOT( us_plotClicked() ) );
}

void US_PlotPushbutton::us_plotClicked( void )
{
   emit US_PlotPbPushed( pb_index );
};

/*********************       QwtPlotConfig Class      *************************/

/*! 
   \brief Constructor
   \param current_plot QwtPlot widget
   \param p            Parent widget
   \param f            Window Flags
*/
US_PlotConfig::US_PlotConfig( QwtPlot* current_plot, QWidget* p, 
  Qt::WindowFlags f ) : US_WidgetsDialog( p, f ) //( false, p, f )
{
   setWindowTitle( "Local Plot Configuration" );
   setPalette( US_GuiSettings::frameColor() );
   
   axisWidget  = NULL;
   gridWidget  = NULL;
   curveWidget = NULL;
   plot        = current_plot;

   // Move this frame to get out of the way
   //move( pos() + QPoint( plot->rect().width(), 0 ) );

   int row = 0;

   QGridLayout* main = new QGridLayout( this );
   main->setContentsMargins( 2, 2, 2, 2 );
   main->setSpacing( 2 );
    
   // Row 0
   QLabel* lb_elements = us_banner( tr( "----- Plot Elements -----" ) );
   lb_elements->setFixedHeight( 40 );
   main->addWidget( lb_elements, row++, 0, 1, 3 );

   // Row 1
   QLabel* lb_title = us_label( tr( "Title Text:" ) );
       
   le_titleText = us_lineedit( plot->title().text(), 1 );
   le_titleText->setMinimumWidth( 500 );
   connect ( le_titleText, SIGNAL( textChanged    ( const QString& ) ), 
                           SLOT  ( updateTitleText( const QString& ) ) ); 
   
   main->addWidget( lb_title,     row,   0 );
   main->addWidget( le_titleText, row++, 1, 1, 2 );

   // Row 2
   QLabel* lb_titleFont = us_label( tr( "Title Font:" ) );
  
   QFont titleFont = plot->title().font();

   le_titleFont = us_lineedit( titleFont.family() + ", " 
          + QString::number( titleFont.pointSize() ) + tr( " points" ), 1 );
   le_titleFont->setReadOnly( true );

   QPushButton* pb_titleFont = us_pushbutton( tr("Update Font") );
   connect( pb_titleFont, SIGNAL( clicked() ), SLOT( updateTitleFont() ) );

   main->addWidget( lb_titleFont, row,   0 );
   main->addWidget( le_titleFont, row,   1 );
   main->addWidget( pb_titleFont, row++, 2 );

   // Row 3
   QLabel* lb_frameColor = us_label( tr( "Frame Color: " ) );
 
   lb_showFrameColor = us_label( "" , 1 );
   lb_showFrameColor->setPalette( plot->palette() ); 
     
   QPushButton* pb_showFrameColor = us_pushbutton ( tr( "Update Color" ) );
   connect( pb_showFrameColor, SIGNAL( clicked() ), SLOT( selectFrameColor() ) );

   main->addWidget( lb_frameColor,     row,   0 );
   main->addWidget( lb_showFrameColor, row,   1 );
   main->addWidget( pb_showFrameColor, row++, 2 );

   // Row 4
   QLabel* lb_canvasColor = us_label( tr( "Canvas Color: " ) );
 
   lb_showCanvasColor = us_label( "" , 1 );

   // set bg color
   QColor   c = plot->canvasBackground();
   QPalette palette = US_GuiSettings::plotColor();
   palette.setColor( QPalette::Active  , QPalette::Window, c );
   palette.setColor( QPalette::Inactive, QPalette::Window, c );
   lb_showCanvasColor->setPalette( palette ); 
     
   QPushButton* pb_showCanvasColor = us_pushbutton( tr( "Update Color" ) );
   connect( pb_showCanvasColor, SIGNAL( clicked() ), SLOT( selectCanvasColor() ) );

   main->addWidget( lb_canvasColor,     row,   0 );
   main->addWidget( lb_showCanvasColor, row,   1 );
   main->addWidget( pb_showCanvasColor, row++, 2 );

   // Row 5
   QLabel* lb_margin      = us_label( tr( "Border Margin:" ) );
   QComboBox* cmbb_margin = us_comboBox();
   
   for ( int i = 2; i <= 30; i += 2 )  
   {  
      cmbb_margin->addItem( QString::number( i ) + " pixels" );
   }
   
   cmbb_margin->setCurrentIndex( plot->margin() / 2 - 1 );
   connect( cmbb_margin, SIGNAL( activated   ( int ) ), 
                         SLOT  ( selectMargin( int ) ) );

   main->addWidget( lb_margin  , row,   0 );
   main->addWidget( cmbb_margin, row++, 1, 1, 2 );
   
   // Row 6
   QLabel* lb_legendPos = us_label( tr( "Legend Position:" ) );

   cmbb_legendPos = us_comboBox();

   cmbb_legendPos->addItem( "None"   );
   cmbb_legendPos->addItem( "Left"   );
   cmbb_legendPos->addItem( "Right"  );
   cmbb_legendPos->addItem( "Top"    );
   cmbb_legendPos->addItem( "Bottom" );
   
   int position = 0;

   switch ( plot->plotLayout()->legendPosition() )
   {
      case QwtPlot::LeftLegend     : position = 1; break;
      case QwtPlot::RightLegend    : position = 2; break;
      case QwtPlot::TopLegend      : position = 3; break;
      case QwtPlot::BottomLegend   : position = 4; break;
      case QwtPlot::ExternalLegend : break; // Shouldn't happen
   }

   // Check to see it there is actually a legend in the plot
   if ( plot->legend() == NULL ) position = 0;

   cmbb_legendPos->setCurrentIndex( position );
   
   connect( cmbb_legendPos, SIGNAL( activated      ( int ) ), 
                            SLOT  ( selectLegendPos( int ) ) );

   main->addWidget( lb_legendPos  , row,   0 );
   main->addWidget( cmbb_legendPos, row++, 1, 1, 2 );

   // Row 7
   QLabel* lb_legendFont = us_label( tr( "Legend Font:" ) );

   le_legendFont = us_lineedit( "", 1 );
   setLegendFontString();
   le_legendFont->setReadOnly( true );

   QPushButton* pb_legendFont = us_pushbutton( tr( "Update Font" ) );
   connect( pb_legendFont, SIGNAL( clicked         () ), 
                           SLOT  ( updateLegendFont() ) );
  
   main->addWidget( lb_legendFont, row,   0 );
   main->addWidget( le_legendFont, row,   1 );
   main->addWidget( pb_legendFont, row++, 2 );

   // Row 8
   US_PlotPushbutton* pb_leftAxis = 
      new US_PlotPushbutton( tr( "Update Left Axis"   ), this, QwtPlot::yLeft );
   
   US_PlotPushbutton* pb_rightAxis = 
      new US_PlotPushbutton( tr( "Update Right Axis"  ), this, QwtPlot::yRight );

   US_PlotPushbutton* pb_topAxis = 
      new US_PlotPushbutton( tr( "Update Top Axis"    ), this, QwtPlot::xTop );

   US_PlotPushbutton* pb_bottomAxis = 
      new US_PlotPushbutton( tr( "Update Bottom Axis" ), this, QwtPlot::xBottom );
   
   connect( pb_leftAxis,   SIGNAL( US_PlotPbPushed( int ) ), 
                           SLOT  ( updateAxis     ( int ) ) );

   connect( pb_rightAxis,  SIGNAL( US_PlotPbPushed( int ) ), 
                           SLOT  ( updateAxis     ( int ) ) );

   connect( pb_topAxis,    SIGNAL( US_PlotPbPushed( int ) ), 
                           SLOT  ( updateAxis     ( int ) ) );

   connect( pb_bottomAxis, SIGNAL( US_PlotPbPushed( int ) ), 
                           SLOT  ( updateAxis     ( int ) ) );

   QBoxLayout* axes = new QHBoxLayout();
   axes->addWidget( pb_leftAxis   );
   axes->addWidget( pb_rightAxis  );
   axes->addWidget( pb_topAxis    );
   axes->addWidget( pb_bottomAxis );

   main->addLayout( axes, row++, 0, 1, 3 );

   // Row 9
   QPushButton* pb_grid = us_pushbutton( tr( "Update Gridlines" ) );
   connect( pb_grid, SIGNAL( clicked() ), SLOT( updateGrid() ) );
   main->addWidget( pb_grid, row++, 0, 1, 3 );

   // Row 10
   QLabel* lb_curve = us_banner( tr( "----- Curve Elements -----" ) );
   lb_curve->setFixedHeight( 40 );
   
   main->addWidget( lb_curve, row++, 0, 1, 3 );

   // Row 11
   lw_curves = us_listwidget( 1 );
   lw_curves->setMinimumHeight( 26 * 5 );
   lw_curves->setSelectionMode( QAbstractItemView::ExtendedSelection );
   
   // Populate list widget
   QwtPlotItemList list = plot->itemList();

   int j = 1;

   for ( int i = 0; i < list.size(); i++ )
   {
      if ( list[ i ]->rtti() == QwtPlotItem::Rtti_PlotCurve )
      {
         QString label = "(" + QString::number( j++ ) + ") ";   
         lw_curves->addItem( label + list[ i ]->title().text() );
      }
   }

   main->addWidget( lw_curves, row++, 0, 1, 3 );
   
   // Row 12
   QPushButton* pb_curve = us_pushbutton( tr( "Update Selected Curve(s)" ) );
   connect( pb_curve, SIGNAL( clicked() ), SLOT( updateCurve() ) );
   
   main->addWidget( pb_curve, row++, 0, 1, 3 );

   // Row 13
   QPushButton* pb_close = us_pushbutton( tr( "Close" ) );
   connect( pb_close, SIGNAL( clicked() ), SLOT( close() ) );
   
   main->addWidget( pb_close, row++, 0, 1, 3 );
}

/*!  \brief Change the plot's title
     \param newText new title
*/
void US_PlotConfig::updateTitleText( const QString &newText )
{
   plot->setTitle( newText );
   plot->updateLayout();
}

/*!  \brief Open Font dialog for changing the title font */
void US_PlotConfig::updateTitleFont( void )
{
   bool ok;
   QFont currentFont = plot->title().font();
   QFont newFont     = QFontDialog::getFont( &ok, currentFont, this,
                          tr( "Set Title Font" ), 0 );

   if ( ok )
   {
      QwtText title = plot->title();
      title.setFont( newFont );
      plot->setTitle( title );

      le_titleFont->setText( newFont.family() + ", " 
             + QString::number( newFont.pointSize() ) + tr( " points" ) );
   }
}

/*!  \brief Change the plot canvas color */
void US_PlotConfig::selectCanvasColor( void )
{
   QPalette p       = lb_showCanvasColor->palette();
   QColor   current = p.color( QPalette::Active, QPalette::Window );
   QColor   c       = QColorDialog::getColor( current, this, tr( "Select canvas color" ) );
   
   if ( c.isValid() )
   {
     p.setColor( QPalette::Active  , QPalette::Window, c );
     p.setColor( QPalette::Inactive, QPalette::Window, c );
     lb_showCanvasColor->setPalette( p );
     plot->setCanvasBackground( c );
     plot->replot();
   }
}

/*!  \brief Change the frame color */
void US_PlotConfig::selectFrameColor( void )
{
   QPalette p       = plot->palette();
   QColor   current = p.color( QPalette::Active, QPalette::Window );
   QColor   c       = QColorDialog::getColor( current, this, tr( "Select canvas color" ) );
   
   if ( c.isValid() )
   {
     p.setColor( QPalette::Active  , QPalette::Window, c );
     p.setColor( QPalette::Inactive, QPalette::Window, c );
     lb_showFrameColor->setPalette( p );
     plot->setPalette( p );
   }
}

/*!  \brief Change the plot margin
     \param index - The combobox index 
*/
void US_PlotConfig::selectMargin( int index )
{
   plot->setMargin( ( index + 1 ) * 2 );
}

/*!  \brief Change the plot legend position
     \param index - The index of the plot position combobox
*/
void US_PlotConfig::selectLegendPos( int index )
{
   QwtLegend* legend = NULL;

   if ( index > 0 )
   {
      legend = new QwtLegend;
      legend->setFrameStyle( QFrame::Box | QFrame::Sunken );
   }

   plot->insertLegend( NULL, QwtPlot::BottomLegend );

   switch ( index )
   {
      case 1: plot->insertLegend( legend, QwtPlot::LeftLegend   ); break;
      case 2: plot->insertLegend( legend, QwtPlot::RightLegend  ); break;
      case 3: plot->insertLegend( legend, QwtPlot::TopLegend    ); break;
      case 4: plot->insertLegend( legend, QwtPlot::BottomLegend ); break;
   }

   setLegendFontString();
   plot->replot();
}

void US_PlotConfig::setLegendFontString( void )
{
   QString legendFontString;

   // Update font description
   if ( cmbb_legendPos->currentIndex() == 0 )
   {
      legendFontString = tr( "No legend for this plot" );
   }
   else
   {
      QFont legendFont = plot->legend()->font();
      legendFontString = legendFont.family() + ", " 
          + QString::number( legendFont.pointSize() ) + tr( " points" );
   }

   le_legendFont->setText( legendFontString );
}

/*!  \brief Change the legend font */
void US_PlotConfig::updateLegendFont( void )
{
   if ( cmbb_legendPos->currentIndex() == 0 )
   {
      QMessageBox::warning( this,
         tr( "Plog Config Warning" ),
         tr( "The layout does not exist.  Set the legend position first." ) );
      return;
   }

   QFont font = plot->legend()->font();

   bool ok;
   QFont newFont = QFontDialog::getFont( &ok, font, this,
                       tr( "Set Legend Font" ), 0 );

   plot->legend()->setFont( newFont );
   setLegendFontString();
   plot->replot();
}

void US_PlotConfig::updateAxis( int axis )
{
   // Prevent opening more than one axis configuration
   if ( axisWidget != NULL )
   {
      axisWidget->close();
   }
   
   axisWidget = new US_PlotAxisConfig( axis, plot );
   //axisWidget->setAttribute( Qt::WA_DeleteOnClose );
   //connect( axisWidget, SIGNAL( axisConfigClosed( void ) ),
   //                     SLOT  ( axisConfigFinish( void ) ) );
   //axisWidget->show();
   axisWidget->exec();
   qApp->processEvents();
   delete axisWidget;
   axisWidget = NULL;

}

/*!  \brief Open US_PlotConfig dialog for changing the 
            selected curves elements
*/
void US_PlotConfig::updateCurve( void )
{
   if ( lw_curves->selectedItems().count() == 0 )
   {
      QMessageBox::information( this,
        tr( "Attention" ), 
        tr( "No curves have been selected." ) );

      return;
   }

   if ( curveWidget != NULL )
   {
      curveWidget->close();
   }
   
   QStringList               selected;
   QList< QListWidgetItem* > items = lw_curves->selectedItems();

   for ( int i = 0; i < items.count(); i++ )
      selected << items[i]->text();

   curveWidget = new US_PlotCurveConfig( plot, selected );
   //curveWidget->setAttribute( Qt::WA_DeleteOnClose );
   //connect( curveWidget, SIGNAL( curveConfigClosed( void ) ),
   //                      SLOT  ( curveConfigFinish( void ) ) );
   //curveWidget->show();
   curveWidget->exec();
   qApp->processEvents();
   delete curveWidget;
   curveWidget = NULL;
}

void US_PlotConfig::updateGrid( void )
{
   // Prevent opening more than one grid configuration
   if ( gridWidget != NULL )
   {
      //gridWidget->close();
   }
   
   gridWidget = new US_PlotGridConfig( plot );
   //gridWidget->setAttribute( Qt::WA_DeleteOnClose );
   //connect( gridWidget, SIGNAL( gridConfigClosed( void ) ),
   //                     SLOT  ( gridConfigFinish( void ) ) );
   //gridWidget->show();
   gridWidget->exec();
   qApp->processEvents();
   delete gridWidget;
}
/*
void US_PlotConfig::closeEvent( QCloseEvent* e )
{
   if ( axisWidget  != NULL ) axisWidget->close(); 
   if ( gridWidget  != NULL ) gridWidget->close();
   //if ( curveWidget != NULL ) curveWidget->close();

   //emit plotConfigClosed();
   e->accept();
}

void US_PlotConfig::axisConfigFinish( void )
{
   axisWidget = NULL;
}

void US_PlotConfig::curveConfigFinish( void )
{
   curveWidget = NULL;
}

void US_PlotConfig::gridConfigFinish( void )
{
   gridWidget = NULL;
}
*/
/*******************      QwtPlotCurveConfig Class      ***********************/

/*! 
   \brief Constructor
   \param currentPlot QwtPlot widget
   \param selected    A list of curve names to be configured
   \param parent      Parent widget
   \param f           Widget flags
*/
US_PlotCurveConfig::US_PlotCurveConfig( QwtPlot* currentPlot, 
      const QStringList& selected, QWidget* parent, Qt::WindowFlags f ) 
      : US_WidgetsDialog( parent, f ) //( false, parent, f )
{
   plot          = currentPlot;
   selectedItems = selected;

   // Keep out of the way
   //move( pos() + QPoint( plot->rect().width(), 0 ) );

   setWindowTitle( tr( "Curve Configuration" ) );
   setPalette( US_GuiSettings::frameColor() );

   // Find the first curve selected
   QwtPlotItemList list = plot->itemList();
   
   // Remove passed '(number) '
   QString firstSelectedText = selected[ 0 ];
   firstSelectedText.replace( QRegExp( "^\\(\\d+\\) " ), "" );
   
   firstSelectedCurve = NULL;

   for ( int i = 0; i < list.size(); i++ )
   {
      if ( list[ i ]->rtti()         == QwtPlotItem::Rtti_PlotCurve  &&
           list[ i ]->title().text() == firstSelectedText )
      {
         firstSelectedCurve = dynamic_cast< QwtPlotCurve* >( list[ i ] );
         break;
      }
   }

   if ( firstSelectedCurve == NULL )
   {
      qDebug() << "Could not find first selected curve";
      return;
   }

   // Set up the dialog
   int row = 0;

   QGridLayout* main = new QGridLayout( this );
   main->setContentsMargins( 2, 2, 2, 2 );
   main->setSpacing( 2 );
    
   // Row 0
   QLabel* lb_sample1 = us_label( tr( "Sample:" ) );
   
   lb_sample2 = new US_PlotLabel( this );
   QPalette p = lb_sample2->palette();
   p.setColor(  QPalette::Window, plot->canvasBackground() );
   lb_sample2->setPalette( p );
   
   main->addWidget( lb_sample1, row  , 0 );
   main->addWidget( lb_sample2, row++, 1, 1, 2 );

   // Row 1
   QLabel* lb_curve  = us_banner( tr( "---- Update Curve Elements ----" ) );
   main->addWidget( lb_curve, row++, 0, 1, 3 );

   // Row 2
   QLabel* lb_curveTitle = us_label( tr( "Title:" ) );
   le_curveTitle         = us_lineedit( "" );

   if ( selected.size() > 1 )
   {
      le_curveTitle->insert( tr( "Multiple curves selected - not changeable" ) );
      le_curveTitle->setReadOnly( true );
   }
   else
      le_curveTitle->insert( firstSelectedText );

   main->addWidget( lb_curveTitle, row  , 0 );
   main->addWidget( le_curveTitle, row++, 1, 1, 2 );

   // Row 3 - Style and width
   QLabel* lb_curveStyle = us_label( tr( "Style:" ) );
   cmbb_curveStyle = us_comboBox();
   cmbb_curveStyle->addItem( tr( "None"   ), QwtPlotCurve::NoCurve );
   cmbb_curveStyle->addItem( tr( "Lines"  ), QwtPlotCurve::Lines   );
   cmbb_curveStyle->addItem( tr( "Sticks" ), QwtPlotCurve::Sticks  );
   cmbb_curveStyle->addItem( tr( "Steps"  ), QwtPlotCurve::Steps   );
   cmbb_curveStyle->addItem( tr( "Dots"   ), QwtPlotCurve::Dots    );

   QList< int > curveStyles;
   curveStyles << QwtPlotCurve::NoCurve << QwtPlotCurve::Lines 
               << QwtPlotCurve::Sticks  << QwtPlotCurve::Steps
               << QwtPlotCurve::Dots;

   curveStyle = firstSelectedCurve->style();

   for ( int i = 0; i < curveStyles.size(); i++ )
      if ( curveStyles [ i ] == firstSelectedCurve->style() )
      {
         cmbb_curveStyle->setCurrentIndex ( i );
         break;
      }

   connect( cmbb_curveStyle, SIGNAL( currentIndexChanged( int ) ), 
                             SLOT  ( curveStyleChanged  ( int ) ) );

   QBoxLayout* stylewidth = new QHBoxLayout();
   stylewidth->addWidget( cmbb_curveStyle );
   
   QLabel* lb_curveWidth = us_label( tr( "Width (pixels):" ) );
   stylewidth->addWidget( lb_curveWidth );

   sb_curveWidth = new QSpinBox;
   sb_curveWidth->setRange( 1, 10 );
   sb_curveWidth->setValue( firstSelectedCurve->pen().width() );
   connect( sb_curveWidth, SIGNAL( valueChanged( int ) ), 
                           SLOT  ( updateSample( int ) ) );

   stylewidth->addWidget( sb_curveWidth );

   main->addWidget( lb_curveStyle, row  , 0 );
   main->addLayout( stylewidth   , row++, 1, 1, 2 );
   // Row 4 - Color
   QLabel* lb_curveColor = us_label( tr( "Color:" ) );
   
   lb_showCurveColor = us_label( "" , 1 );
   p = lb_showCurveColor->palette();
   p.setColor( QPalette::Window, firstSelectedCurve->pen().color() );
   lb_showCurveColor->setPalette( p );

   QPushButton* pb_showCurveColor = us_pushbutton ( tr( "Update Color" ) );
   connect( pb_showCurveColor, SIGNAL( clicked         () ), 
                               SLOT  ( selectCurveColor() ) );

   main->addWidget( lb_curveColor    , row  , 0 );
   main->addWidget( lb_showCurveColor, row  , 1 );
   main->addWidget( pb_showCurveColor, row++, 2 );

   // Row 5
   QLabel* lb_symbol = us_banner( tr( "---- Update Symbol Elements ----" ) );
   main->addWidget( lb_symbol, row++, 0, 1, 3 );

   // Row 6  Symbol Style
   QLabel* lb_symbolStyle = us_label( tr( "Style:" ) );
   cmbb_symbolStyle = us_comboBox();
   cmbb_symbolStyle->addItem( tr( "No Symbol"       ), QwtSymbol::NoSymbol  );
   cmbb_symbolStyle->addItem( tr( "Ellipse"         ), QwtSymbol::Ellipse   );
   cmbb_symbolStyle->addItem( tr( "Rectangle"       ), QwtSymbol::Rect      );
   cmbb_symbolStyle->addItem( tr( "Diamond"         ), QwtSymbol::Diamond   );
   cmbb_symbolStyle->addItem( tr( "Down Triangle"   ), QwtSymbol::DTriangle );
   cmbb_symbolStyle->addItem( tr( "Up Triangle"     ), QwtSymbol::UTriangle );
   cmbb_symbolStyle->addItem( tr( "Left Triangle"   ), QwtSymbol::LTriangle );
   cmbb_symbolStyle->addItem( tr( "Right Triangle"  ), QwtSymbol::RTriangle );
   cmbb_symbolStyle->addItem( tr( "Cross"           ), QwtSymbol::Cross     );
   cmbb_symbolStyle->addItem( tr( "X Cross"         ), QwtSymbol::XCross    );
   cmbb_symbolStyle->addItem( tr( "Horizontal Line" ), QwtSymbol::HLine     );
   cmbb_symbolStyle->addItem( tr( "Vertical Line"   ), QwtSymbol::VLine     );
   cmbb_symbolStyle->addItem( tr( "Star 1"          ), QwtSymbol::Star1     );
   cmbb_symbolStyle->addItem( tr( "Star 2"          ), QwtSymbol::Star2     );
   cmbb_symbolStyle->addItem( tr( "Hexagon"         ), QwtSymbol::Hexagon   );

   QList< int > symbolStyles;
   symbolStyles 
      << QwtSymbol::NoSymbol  << QwtSymbol::Rect      << QwtSymbol::Diamond  
      << QwtSymbol::Triangle  << QwtSymbol::DTriangle << QwtSymbol::UTriangle 
      << QwtSymbol::LTriangle << QwtSymbol::RTriangle << QwtSymbol::Cross    
      << QwtSymbol::XCross    << QwtSymbol::HLine     << QwtSymbol::VLine    
      << QwtSymbol::Star1     << QwtSymbol::Star2     << QwtSymbol::Hexagon;

   symbolStyle = firstSelectedCurve->symbol().style();

   for ( int i = 0; i < symbolStyles.size(); i++ )
      if ( symbolStyles [ i ] == symbolStyle )
      {
         cmbb_symbolStyle->setCurrentIndex ( i );
         break;
      }

   connect( cmbb_symbolStyle, SIGNAL( currentIndexChanged( int ) ), 
                              SLOT  ( symbolStyleChanged ( int ) ) );

   main->addWidget( lb_symbolStyle  , row  , 0 );
   main->addWidget( cmbb_symbolStyle, row++, 1, 1, 2 );

   // Row 7  Symbol width and height
   QLabel* lb_symbolWidth  = us_label( tr( "Width (pixels):" ) );

   sb_symbolWidth = new QSpinBox;
   sb_symbolWidth->setRange( 1, 30 );
   sb_symbolWidth->setValue( firstSelectedCurve->symbol().size().width() );
   connect( sb_symbolWidth, SIGNAL( valueChanged( int ) ), 
                            SLOT  ( updateSample( int ) ) );

   QLabel* lb_symbolHeight = us_label( tr( "Height (pixels):" ) );

   sb_symbolHeight = new QSpinBox;
   sb_symbolHeight->setRange( 1, 30 );
   sb_symbolHeight->setValue( firstSelectedCurve->symbol().size().height() );
   connect( sb_symbolWidth, SIGNAL( valueChanged( int ) ), 
                            SLOT  ( updateSample( int ) ) );

   QBoxLayout* heightwidth = new QHBoxLayout();
   heightwidth->addWidget( sb_symbolWidth  );
   heightwidth->addWidget( lb_symbolHeight );
   heightwidth->addWidget( sb_symbolHeight );

   main->addWidget( lb_symbolWidth, row  , 0 );
   main->addLayout( heightwidth   , row++, 1, 1, 2 );

   // Row 8  Symbol Color -- Interior and Exterior
   QLabel* lb_symbolOutlineColor  = us_label( tr( "Outline Color:" ) );

   lb_showSymbolOutlineColor = us_label( "" , 1 );
   p = lb_showSymbolOutlineColor->palette();
   p.setColor( QPalette::Window, firstSelectedCurve->symbol().pen().color() );
   lb_showSymbolOutlineColor->setPalette( p );

   QPushButton* pb_symOutlineColor = us_pushbutton ( tr( "Update Color" ) );
   connect( pb_symOutlineColor, SIGNAL( clicked                 () ), 
                                SLOT  ( selectSymbolOutlineColor() ) );
   
   QLabel* lb_symbolInteriorColor = us_label( tr( "Interior Color:" ) );

   lb_showSymbolInteriorColor = us_label( "" , 1 );
   p = lb_showSymbolInteriorColor->palette();
   p.setColor( QPalette::Window, firstSelectedCurve->symbol().brush().color() );
   lb_showSymbolInteriorColor->setPalette( p );

   QPushButton* pb_symInteriorColor = us_pushbutton ( tr( "Update Color" ) );
   connect( pb_symInteriorColor, SIGNAL( clicked                  () ), 
                                 SLOT  ( selectSymbolInteriorColor() ) );

   QBoxLayout* symbolcolor = new QHBoxLayout();
   symbolcolor->addWidget( lb_showSymbolOutlineColor  );
   symbolcolor->addWidget( pb_symOutlineColor         );
   symbolcolor->addWidget( lb_symbolInteriorColor     );
   symbolcolor->addWidget( lb_showSymbolInteriorColor );
   symbolcolor->addWidget( pb_symInteriorColor        );

   main->addWidget( lb_symbolOutlineColor, row  , 0 );
   main->addLayout( symbolcolor          , row++, 1, 1, 2 );

   // Row 9
   QPushButton* pb_apply = us_pushbutton( tr( "Apply" ) );
   connect( pb_apply, SIGNAL( clicked() ), SLOT( apply() ) );

   QPushButton* pb_close = us_pushbutton( tr( "Close" ) );
   connect( pb_close, SIGNAL( clicked() ), SLOT( close() ) );
   
   QBoxLayout* applyClose = new QHBoxLayout();
   applyClose->addWidget( pb_apply );
   applyClose->addWidget( pb_close );

   main->addLayout( applyClose, row++, 0, 1, 3 );
}
/*
void US_PlotCurveConfig::closeEvent( QCloseEvent* e )
{
   emit curveConfigClosed();
   e->accept();
}
*/
void US_PlotCurveConfig::updateSample( int )
{
   lb_sample2->update(); 
}

void US_PlotCurveConfig::curveStyleChanged( int index )
{
   int style  = cmbb_curveStyle->itemData( index ).toInt();
   curveStyle = static_cast< QwtPlotCurve::CurveStyle > ( style );
   lb_sample2->update(); 
}

void US_PlotCurveConfig::symbolStyleChanged( int index )
{
   int style  = cmbb_symbolStyle->itemData( index ).toInt();
   symbolStyle = static_cast< QwtSymbol::Style > ( style );
   lb_sample2->update(); 
}

void US_PlotCurveConfig::selectCurveColor( void )
{
   QPalette p       = lb_showCurveColor->palette();
   QColor   current = p.color( QPalette::Active, QPalette::Window );
   QColor   c       = QColorDialog::getColor( current, this, 
                         tr( "Select curve color" ) );
   
   if ( c.isValid() )
   {
     p.setColor( QPalette::Window, c );
     lb_showCurveColor->setPalette( p );
     lb_sample2->update(); 
   }
}

void US_PlotCurveConfig::selectSymbolInteriorColor( void )
{
   QPalette p       = lb_showSymbolInteriorColor->palette();
   QColor   current = p.color( QPalette::Active, QPalette::Window );
   QColor   c       = QColorDialog::getColor( current, this, 
                         tr( "Select symbol interior color" ) );
   
   if ( c.isValid() )
   {
     p.setColor( QPalette::Window, c );
     lb_showSymbolInteriorColor->setPalette( p );
     lb_sample2->update(); 
   }
}

void US_PlotCurveConfig::selectSymbolOutlineColor( void )
{
   QPalette p       = lb_showSymbolOutlineColor->palette();
   QColor   current = p.color( QPalette::Active, QPalette::Window );
   QColor   c       = QColorDialog::getColor( current, this, 
                         tr( "Select symbol outline color" ) );
   
   if ( c.isValid() )
   {
     p.setColor( QPalette::Window, c );
     lb_showSymbolOutlineColor->setPalette( p );
     lb_sample2->update(); 
   }
}

void US_PlotCurveConfig::apply( void )
{
   QwtSymbol oldSymbol   = firstSelectedCurve->symbol();
   QPen      symbolPen   = oldSymbol.pen();
   QBrush    symbolBrush = oldSymbol.brush();
   
   QPalette  palette     = lb_showSymbolOutlineColor->palette();
   symbolPen.setColor( palette.color( QPalette::Window ) );

   palette = lb_showSymbolInteriorColor->palette();
   symbolBrush.setColor( palette.color( QPalette::Window ) );

   QSize symbolSize( sb_symbolWidth->value(), sb_symbolHeight->value() );
   
   QwtSymbol newSymbol( symbolStyle, symbolBrush, symbolPen, symbolSize );

   palette = lb_showCurveColor->palette();
   QPen      curvePen( palette.color( QPalette::Window ) );
   curvePen.setWidth( sb_curveWidth->value() );

   
   QwtPlotItemList list = plot->itemList(); // All items

   int j = 0;

   // Iterate through the selected curves
   for ( int i = 0; i < selectedItems.size(); i++ )
   {
      // Remove Numbering from passed titles
      QString title = selectedItems[ i ];
      title.replace( QRegExp( "^\\(\\d+\\) " ), "" );

      // There is no need to reiterate over the full list
      // This assumes the selected list and the full list
      // are in the same order

      for ( /* no intitalizer */; j < list.size(); j++ )
      {
         if ( list[ j ]->rtti() != QwtPlotItem::Rtti_PlotCurve ) continue;
         if ( list[ j ]->title() == title )
         {
            QwtPlotCurve* curve = dynamic_cast< QwtPlotCurve* > ( list[ j ] );
            if ( selectedItems.size() == 1 ) 
               curve->setTitle( le_curveTitle->text() );
 
            curve->setSymbol( newSymbol);
            curve->setPen   ( curvePen );
            curve->setStyle ( curveStyle );
            break;
         }
      }

      plot->replot();
   }
}

//**** Custom class to display curve configuration  ********

US_PlotLabel::US_PlotLabel( US_PlotCurveConfig* caller, 
      QWidget* p, Qt::WindowFlags f  ) : QWidget( p, f )
{
   data  = caller;
   label = new QLabel;
}

void US_PlotLabel::paintEvent( QPaintEvent* e )
{
   QRect r = e->rect();

   int   height = r.y() + r.height()    / 2;
   int   first  = r.x() + r.width()     / 10;
   int   last   = r.x() + r.width() * 9 / 10;
   int   length = last - first;
   
   QPoint p1 = QPoint( first             , height ); //   0%
   QPoint p2 = QPoint( first + length / 4, height ); //  25%    
   QPoint p3 = QPoint( first + length / 2, height ); //  50%
   QPoint p4 = QPoint( last  - length / 4, height ); //  75%
   QPoint p5 = QPoint( last              , height ); // 100%

   int y1;
   int y2;

   QPainter p( this );
   p.save();

   // Paint the background
   QBrush brush( data->plot->canvasBackground() );
   p.fillRect( e->rect(), brush );

   QPen     pen  ( data->firstSelectedCurve->pen() );
   QPalette foreground = data->lb_showCurveColor->palette();
   pen.setColor( foreground.color( QPalette::Window ) );
   pen.setWidth( data->sb_curveWidth->value() );
   p.setPen ( pen );

   // Paint the curve
   switch( data->curveStyle )
   {
      case QwtPlotCurve::NoCurve: 
         break;
         
      case QwtPlotCurve::Lines:  
         p.drawLine( p1, p5 );
         break;

      case QwtPlotCurve::Sticks:
         y1 = p1.y() - 6;
         y2 = p1.y() + 6;
            
         for( int i = p1.x(); i < p5.x(); i += 8 ) 
            p.drawLine( i, y1, i, y2 );
         break;

      case QwtPlotCurve::Steps:
         y1 = p1.y() + 3;
         y2 = p1.y() - 3;
            
         for ( int i = p1.x(); i < p5.x(); i += 10 )
         {
            p.drawLine( i     , y1, i + 10, y2 );
            p.drawLine( i + 10, y2, i + 10, y1 ); 
         } 
         break;

      case QwtPlotCurve::Dots:
         for ( int i = p1.x(); i < p5.x() ; i += 6 )
            p.drawPoint( i, p1.y() );
         break;

      default:
         break;
   }

   if ( data->symbolStyle != QwtSymbol::NoSymbol )
   {
      QSize     size( data->sb_symbolWidth->value(), 
                      data->sb_symbolHeight->value() );
      
      QPalette palette = data->lb_showSymbolOutlineColor->palette();
      pen.setColor( palette.color( QPalette::Window ) );
      
      palette = data->lb_showSymbolInteriorColor->palette();
      brush.setColor( palette.color( QPalette::Window ) );
      
      QwtSymbol s( data->symbolStyle, brush, pen, size );

      s.draw( &p, p2 );
      s.draw( &p, p3 );
      s.draw( &p, p4 );
   }

   p.restore();
}

/****************        QwtPlotAxisConfig Class        *********************/

/*! 
   \brief Constructor
   \param currentAxis Axis ot configure
   \param currentPlot QwtPlot widget
   \param parent      Parent widget
   \param flags       Frame window flags
*/
US_PlotAxisConfig::US_PlotAxisConfig( int currentAxis, QwtPlot* currentPlot, 
   QWidget* parent, Qt::WindowFlags flags ) : US_WidgetsDialog( parent, flags )//( false, parent, flags )
{
   plot = currentPlot;
   axis = currentAxis;

   // Keep out of the way
   //move( pos() + QPoint( plot->rect().width(), 0 ) );
   
   QString axisLocation;

   switch ( axis )
   {
      case QwtPlot::yLeft  : axisLocation = tr( "Left"   ); break;
      case QwtPlot::yRight : axisLocation = tr( "Right"  ); break;
      case QwtPlot::xTop   : axisLocation = tr( "Top"    ); break;
      case QwtPlot::xBottom: axisLocation = tr( "Bottom" ); break;
   }

   setWindowTitle( axisLocation + tr( " Axis Configuration" ) );
   setPalette( US_GuiSettings::frameColor() );

   int row = 0;

   QGridLayout* main = new QGridLayout( this );
   main->setContentsMargins( 2, 2, 2, 2 );
   main->setSpacing( 2 );
    
   // Row 0
   QLabel* lb_present = us_label( axisLocation + tr( " Axis Present:" ) );
   Qt::CheckState checked    = ( plot->axisEnabled( axis ) ) 
      ? Qt::Checked : Qt::Unchecked;
   
   QGridLayout* present = us_checkbox( "", cb_present, checked );

   main->addWidget( lb_present, row  , 0 );
   main->addLayout( present   , row++, 1 );

   // Row 1
   QLabel* lb_title = us_label( tr( "Axis Title:" ) );

   le_titleText = us_lineedit( plot->axisTitle( axis ).text(), 1 );
   le_titleText->setMinimumWidth( 350 );
  
   main->addWidget( lb_title    , row  , 0 );
   main->addWidget( le_titleText, row++, 1, 1, 2 );

   // Row 2
   QLabel* lb_axisTitleFont = us_label( tr( "Axis Title Font:" ) );
   
   axisTitleFont = plot->axisTitle( axis ).font();

   le_axisTitleFont = us_lineedit( axisTitleFont.family() + ", " 
     + QString::number( axisTitleFont.pointSize() ) + tr( " points" ), 1 );
   le_axisTitleFont->setReadOnly( true );

   QPushButton* pb_axisTitleFont = us_pushbutton( tr("Update Font") );
   connect( pb_axisTitleFont, SIGNAL( clicked() ), SLOT( selectTitleFont() ) );

   main->addWidget( lb_axisTitleFont, row,   0 );
   main->addWidget( le_axisTitleFont, row,   1 );
   main->addWidget( pb_axisTitleFont, row++, 2 );


   // Row 3
   QLabel* lb_axisTitleColor = us_label( tr( "Axis Title Color:" ) );

   lb_showAxisTitleColor = us_label( "" , 1 );

   QPalette p = plot->axisWidget( axis )->palette();
   p.setColor( QPalette::Window, p.color( QPalette::Text ) );
   lb_showAxisTitleColor->setPalette( p ); 
   
   QPushButton* pb_axisTitleColor = us_pushbutton ( tr( "Update Color" ) );
   connect( pb_axisTitleColor, SIGNAL( clicked             () ), 
                               SLOT  ( selectAxisTitleColor() ) );

   main->addWidget( lb_axisTitleColor    , row,   0 );
   main->addWidget( lb_showAxisTitleColor, row,   1 );
   main->addWidget( pb_axisTitleColor    , row++, 2 );

   // Row 4
   QLabel* lb_axisScaleFont = us_label( tr( "Axis Scale Font:" ) );

   axisScaleFont = plot->axisFont( axis );

   le_axisScaleFont = us_lineedit( axisScaleFont.family() + ", " 
          + QString::number( axisScaleFont.pointSize() ) + tr( " points" ), 1 );
   le_axisScaleFont->setReadOnly( true );

   QPushButton* pb_axisScaleFont = us_pushbutton( tr("Update Font") );
   connect( pb_axisScaleFont, SIGNAL( clicked() ), SLOT( selectScaleFont() ) );

   main->addWidget( lb_axisScaleFont, row,   0 );
   main->addWidget( le_axisScaleFont, row,   1 );
   main->addWidget( pb_axisScaleFont, row++, 2 );

   // Row 5
   QLabel* lb_scaleColor = us_label( tr( "Axis Scale Color:" ) );
   lb_showScaleColor = us_label( "" , 1 );

   p = plot->axisWidget( axis )->palette();
   p.setColor( QPalette::Window, p.color( QPalette::WindowText ) );
   lb_showScaleColor->setPalette( p ); 
   
   QPushButton* pb_scaleColor = us_pushbutton ( tr( "Update Color" ) );
   connect( pb_scaleColor, SIGNAL( clicked() ), SLOT( selectScaleColor() ) );

   main->addWidget( lb_scaleColor    , row,   0 );
   main->addWidget( lb_showScaleColor, row,   1 );
   main->addWidget( pb_scaleColor    , row++, 2 );

   // Row 5a
   QLabel* lb_tickColor = us_label( tr( "Axis Tick Color:" ) );
   lb_showTickColor = us_label( "" , 1 );

   p = plot->axisWidget( axis )->palette();
   p.setColor( QPalette::Window, p.color( QPalette::WindowText ) );
   lb_showTickColor->setPalette( p ); 
   
   QPushButton* pb_tickColor = us_pushbutton ( tr( "Update Color" ) );
   connect( pb_tickColor, SIGNAL( clicked() ), SLOT( selectTickColor() ) );

   main->addWidget( lb_tickColor    , row,   0 );
   main->addWidget( lb_showTickColor, row,   1 );
   main->addWidget( pb_tickColor    , row++, 2 );

   // Row 6
   QLabel* lb_scaleFrom = us_label( tr( "Scale From:" ) );
   QLabel* lb_scaleTo   = us_label( tr( "To:" ) );
   QLabel* lb_scaleStep = us_label( tr( "Step:" ) );

   le_scaleFrom = us_lineedit( 
         QString::number( plot->axisScaleDiv( axis )->lowerBound() ), 1 );
   
   le_scaleTo   = us_lineedit( 
         QString::number( plot->axisScaleDiv( axis )->upperBound() ), 1 );
   
   le_scaleStep = us_lineedit( 
         QString::number( plot->axisStepSize( axis )           ), 1 );
   
   QBoxLayout* scale = new QHBoxLayout;
   scale->addWidget( le_scaleFrom );
   scale->addWidget( lb_scaleTo   );
   scale->addWidget( le_scaleTo   );
   scale->addWidget( lb_scaleStep );
   scale->addWidget( le_scaleStep );

   main->addWidget( lb_scaleFrom, row  , 0 );
   main->addLayout( scale       , row++, 1, 1, 2 );

   // Row 7
   QLabel* lb_scaleType = us_label( tr( "Scale Type:" ) );
   
   QBoxLayout* scaleRadio = new QHBoxLayout();
   scaleRadio->setSpacing( 0 );
   
   QGridLayout* rb1 = us_radiobutton( tr( "Linear"      ), rb_linear, true  );
   QGridLayout* rb2 = us_radiobutton( tr( "Logarithmic" ), rb_log   , false );

   if ( plot->axisScaleEngine( axis )->transformation()->type() == 
         QwtScaleTransformation::Log10 ) rb_log->setChecked( true );

   scaleRadio->addLayout( rb1 );
   scaleRadio->addLayout( rb2 );

   main->addWidget( lb_scaleType, row,   0 );
   main->addLayout( scaleRadio  , row++, 1, 1, 2 );

   // Row 8
   QLabel* lb_reference = us_label( tr( "Scale Reference:" ) );
   
   le_reference = us_lineedit( 
         QString::number( plot->axisScaleEngine( axis )->reference() ), 1 );

   QGridLayout* refValue  = us_checkbox( QString( "Include"   ), cb_refValue );
   QGridLayout* symmetric = us_checkbox( QString( "Symmetric" ), cb_symmetric );
   
   QBoxLayout* reference = new QHBoxLayout;
   reference->addWidget( le_reference );
   reference->addLayout( refValue  );
   reference->addLayout( symmetric );
      
   main->addWidget( lb_reference, row,   0 );
   main->addLayout( reference   , row++, 1, 1, 2 );

   // Row 9
   QLabel* lb_options = us_label( tr( "Scale Options:" ) );

   int attributes = plot->axisScaleEngine( axis )->attributes();
   
   checked = ( attributes & QwtScaleEngine::Floating  ) 
      ? Qt::Checked : Qt::Unchecked;
   
   QGridLayout* floating  
      = us_checkbox( QString( "Floating Endpoints" ), cb_floating, checked );
   
   checked = ( attributes & QwtScaleEngine::Inverted ) 
      ? Qt::Checked : Qt::Unchecked;
   
   QGridLayout* inverted  
      = us_checkbox( QString( "Inverted Scale"), cb_inverted, checked );
   
   checked = ( plot->axisAutoScale( axis ) ) ? Qt::Checked : Qt::Unchecked;
   QGridLayout* autoscale 
      = us_checkbox( QString( "Autoscale" ), cb_autoscale, checked );

   QBoxLayout* options = new QHBoxLayout;
   options->addLayout( floating  );
   options->addLayout( inverted  );
   options->addLayout( autoscale );

   main->addWidget( lb_options, row,   0 );
   main->addLayout( options   , row++, 1, 1, 2 );
   
   // Row 10
   QPushButton* pb_apply = us_pushbutton( tr( "Apply" ) );
   connect( pb_apply, SIGNAL( clicked() ), SLOT( apply() ) );

   QPushButton* pb_close = us_pushbutton( tr( "Close" ) );
   connect( pb_close, SIGNAL( clicked() ), SLOT( close() ) );
   
   QBoxLayout* applyClose = new QHBoxLayout();
   applyClose->addWidget( pb_apply );
   applyClose->addWidget( pb_close );

   main->addLayout( applyClose, row++, 0, 1, 3 );
}
/*
void US_PlotAxisConfig::closeEvent( QCloseEvent* e )
{
   emit axisConfigClosed();
   e->accept();
}
*/
void US_PlotAxisConfig::selectTitleFont( void )
{
   bool ok;
   QFont currentFont = plot->axisTitle( axis ).font();
   QFont newFont     = QFontDialog::getFont( &ok, currentFont, this,
                          tr( "Set Axis Title Font" ), 0 );

   if ( ok )
   {
      axisTitleFont = newFont;
      le_axisTitleFont->setText( newFont.family() + ", " 
             + QString::number( newFont.pointSize() ) + tr( " points" ) );
   }
}

void US_PlotAxisConfig::selectAxisTitleColor( void )
{
   QPalette p       = lb_showAxisTitleColor->palette();
   QColor   current = p.color( QPalette::Window );
   QColor   c       = QColorDialog::getColor( current, this, 
                         tr( "Select Axis Title Color" ) );
   
   if ( c.isValid() )
   {
      p.setColor( QPalette::Window, c );
      lb_showAxisTitleColor->setPalette( p );
   }
}

void US_PlotAxisConfig::selectScaleFont( void )
{
   bool ok;
   QFont currentFont = plot->axisFont( axis );
   QFont newFont     = QFontDialog::getFont( &ok, currentFont, this,
                          tr( "Set Axis Scale Font" ), 0 );

   if ( ok )
   {
      axisScaleFont = newFont;
      le_axisScaleFont->setText( newFont.family() + ", " 
             + QString::number( newFont.pointSize() ) + tr( " points" ) );
   }
}

void US_PlotAxisConfig::selectScaleColor( void )
{
   QPalette p       = lb_showScaleColor->palette();
   QColor   current = p.color( QPalette::Window );
   QColor   c       = QColorDialog::getColor( current, this, 
                         tr( "Select Scale Color" ) );

   if ( c.isValid() )
   {
      p.setColor( QPalette::Window, c );
      lb_showScaleColor->setPalette( p );
   }
}

void US_PlotAxisConfig::selectTickColor( void )
{
   QPalette p       = lb_showTickColor->palette();
   QColor   current = p.color( QPalette::Window );
   QColor   c       = QColorDialog::getColor( current, this, 
                         tr( "Select Tick Color" ) );

   if ( c.isValid() )
   {
      p.setColor( QPalette::Window, c );
      lb_showTickColor->setPalette( p );
   }
}

void US_PlotAxisConfig::apply( void )
{
   // This is where the work is done.
   // Turn the axis on or off.
   plot->enableAxis( axis, cb_present->isChecked() );
   
   // Set the title text
   plot->axisWidget( axis )->setTitle( le_titleText->text() );

   // Set the title font
   QwtText title = plot->axisTitle( axis );
   title.setFont( axisTitleFont );

   // Set the title color
   QPalette p = lb_showAxisTitleColor->palette();
   title.setColor( p.color( QPalette::Window ) );

   plot->setAxisTitle( axis, title );

   // Set the scale font
   plot->setAxisFont( axis, axisScaleFont );

   // Set the scale color
   p = lb_showScaleColor->palette();
   QPalette axisPalette = plot->axisWidget( axis )->palette();
   axisPalette.setColor( QPalette::Text, p.color( QPalette::Window ) );
   
   p = lb_showTickColor->palette();
   axisPalette.setColor( QPalette::WindowText, p.color( QPalette::Window ) );

   plot->axisWidget( axis )->setPalette( axisPalette );   

   // Set the scale From, To, Step
   double from = le_scaleFrom->text().toDouble();
   double to   = le_scaleTo  ->text().toDouble();
   double step = le_scaleStep->text().toDouble();

   if ( ! cb_autoscale->isChecked() )
     plot->setAxisScale( axis, from, to, step );

   // Scale type - Linear or Log  -- Do nothing if unchanged
   int plotType = plot->axisScaleEngine( axis )->transformation()->type();
   if ( ( plotType == QwtScaleTransformation::Log10  && rb_linear->isChecked() ) 
     || ( plotType == QwtScaleTransformation::Linear && rb_log   ->isChecked() ) )
   {
      if ( rb_linear->isChecked() ) 
         plot->setAxisScaleEngine( axis, new QwtLinearScaleEngine );
      else  
         plot->setAxisScaleEngine( axis, new QwtLog10ScaleEngine );
   }

   // Set scale reference
   double reference = le_reference->text().toDouble();
   plot->axisScaleEngine( axis )->setReference( reference );

   int attributes = 0;
   if ( cb_refValue ->isChecked() ) attributes |= QwtScaleEngine::IncludeReference; 
   if ( cb_symmetric->isChecked() ) attributes |= QwtScaleEngine::Symmetric; 

   // Set other attributes

   if ( cb_floating->isChecked()  ) attributes |= QwtScaleEngine::Floating; 
   if ( cb_inverted->isChecked()  ) attributes |= QwtScaleEngine::Inverted;
   plot->axisScaleEngine( axis )->setAttributes( attributes );

   if ( cb_autoscale->isChecked()  ) plot->setAxisAutoScale( axis );

   plot->replot();
}

/**************************    US_PlotGridConfig Class    *********************/

/*!  \brief A window to configure the plot grid
     \param currentPlot The plot to configure   
     \param parent      Parent widget
     \param flags       Window flags
*/
US_PlotGridConfig::US_PlotGridConfig( QwtPlot* currentPlot, 
   QWidget* parent, Qt::WindowFlags flags ) : US_WidgetsDialog( parent, flags )//( false, parent, flags )
{
   setWindowTitle( tr( "Grid Configuration" ) );
   setPalette( US_GuiSettings::frameColor() );
   
   plot = currentPlot;
   grid = NULL;

   // Keep out of the way
   //move( pos() + QPoint( plot->rect().width(), 0 ) );

   // Get the grid - if it exists
   QwtPlotItemList list = plot->itemList();

   for ( int i = 0; i< list.size(); i++ )
   {
      if ( list[i]->rtti() == QwtPlotItem::Rtti_PlotGrid )
      {
         grid = dynamic_cast<QwtPlotGrid*>( list[ i ] );
         break;
      }
   }

   // Add an inactive grid if necessary
   if ( grid == NULL ) 
   {
      grid = new QwtPlotGrid;
      grid->enableX   ( false );
      grid->enableY   ( false );
      grid->enableXMin( false );
      grid->enableYMin( false );
      grid->attach( plot );
   }


   QGridLayout* main = new QGridLayout( this );
   main->setContentsMargins( 2, 2, 2, 2 );
   main->setSpacing( 2 );
   int row = 0;

   // Row 0
   QLabel* lb_major = us_banner( tr( " ---- Major Gridlines ----" ) );
   lb_major->setFixedHeight( 40 );
   
   main->addWidget( lb_major, row++, 0, 1, 3 );

   // Row 1
   QLabel* lb_enableMajor = us_label( tr( "Enable:" ) );

   Qt::CheckState checked = ( grid->xEnabled() ) ? Qt::Checked : Qt::Unchecked;
   QGridLayout* enableX = us_checkbox( tr( "Vertical" ), cb_enableX, checked );

   checked = ( grid->yEnabled() ) ? Qt::Checked : Qt::Unchecked;
   QGridLayout* enableY = us_checkbox( tr( "Horizontal"   ), cb_enableY, checked );

   QBoxLayout* enableMajor = new QHBoxLayout;
   enableMajor->addLayout( enableX );
   enableMajor->addLayout( enableY );

   main->addWidget( lb_enableMajor, row  , 0 );
   main->addLayout( enableMajor   , row++, 1, 1, 2 );

   // Row 2
   QLabel* lb_majorColor = us_label( tr( "Color:" ) );

   lb_showMajorColor = us_label( "" , 1 );

   QPen     majorPen = grid->majPen();
   QPalette p;
   p.setColor( QPalette::Window, majorPen.color() );
   lb_showMajorColor->setPalette( p ); 
   
   QPushButton* pb_majorColor = us_pushbutton ( tr( "Update Color" ) );
   connect( pb_majorColor, SIGNAL( clicked() ), SLOT( selectMajorColor() ) );

   main->addWidget( lb_majorColor    , row,   0 );
   main->addWidget( lb_showMajorColor, row,   1 );
   main->addWidget( pb_majorColor    , row++, 2 );

   // Row 3 
   QLabel* lb_majorStyle = us_label( tr( "Style:" ) );

   QBoxLayout* majorStyle = new QHBoxLayout;

   QStringList styles;
   styles << tr( "Solid" ) << tr( "Dash" ) << tr( "Dot" ) << tr( "DashDotDot" )
          << tr( "DashDotDot" );

   QList< int > penStyle;
   penStyle << Qt::SolidLine << Qt::DashLine << Qt::DotLine << Qt::DashDotLine
            << Qt::DashDotDotLine;

   cmbb_majorStyle  = us_comboBox();
   int currentStyle = majorPen.style();
   
   for ( int i = 0; i < styles.size(); i++ )
   {
      cmbb_majorStyle->addItem( styles[ i ], penStyle[ i ] );

      if ( currentStyle == penStyle[ i ] )
         cmbb_majorStyle->setCurrentIndex( i );
   }

   majorStyle->addWidget( cmbb_majorStyle );

   QLabel* lb_majorWidth = us_label( tr( "Width (pixels):" ) );
   majorStyle->addWidget( lb_majorWidth );

   sb_majorWidth = new QSpinBox;
   sb_majorWidth->setRange( 1, 5 );
   sb_majorWidth->setValue( majorPen.width() );

   majorStyle->addWidget( sb_majorWidth );

   main->addWidget( lb_majorStyle, row,   0 );
   main->addLayout( majorStyle   , row++, 1, 1, 2 );

   //  Start Minor gridline settings
   // Row 4
   QLabel*  lb_minor = us_banner( tr( " ---- Minor Gridlines ----" ) );
   lb_minor->setFixedHeight( 40 );
   main->addWidget( lb_minor, row++, 0, 1, 3 );

   // Row 5
   QLabel* lb_enableMinor = us_label( tr( "Enable:" ) );

   checked = ( grid->xMinEnabled() ) ? Qt::Checked : Qt::Unchecked;
   QGridLayout* enableXminor 
      = us_checkbox( tr( "Vertical" ), cb_enableXminor, checked );

   checked = ( grid->yMinEnabled() ) ? Qt::Checked : Qt::Unchecked;
   QGridLayout* enableYminor 
      = us_checkbox( tr( "Horizontal" ), cb_enableYminor, checked );

   QBoxLayout* enableMinor = new QHBoxLayout;
   enableMinor->addLayout( enableXminor );
   enableMinor->addLayout( enableYminor );

   main->addWidget( lb_enableMinor, row  , 0 );
   main->addLayout( enableMinor   , row++, 1, 1, 2 );

   // Row 6
   QLabel* lb_minorColor = us_label( tr( "Color:" ) );

   lb_showMinorColor = us_label( "" , 1 );

   QPen     minorPen = grid->minPen();
   p.setColor( QPalette::Window, minorPen.color() );
   lb_showMinorColor->setPalette( p ); 
   
   QPushButton* pb_minorColor = us_pushbutton ( tr( "Update Color" ) );
   connect( pb_minorColor, SIGNAL( clicked() ), SLOT( selectMinorColor() ) );

   main->addWidget( lb_minorColor    , row,   0 );
   main->addWidget( lb_showMinorColor, row,   1 );
   main->addWidget( pb_minorColor    , row++, 2 );
   
   // Row 7 
   QLabel* lb_minorStyle = us_label( tr( "Style:" ) );

   QBoxLayout* minorStyle = new QHBoxLayout;

   cmbb_minorStyle = us_comboBox();
   currentStyle    = minorPen.style();

   for ( int i = 0; i < styles.size(); i++ )
   {
      cmbb_minorStyle->addItem( styles[ i ], penStyle[ i ] );
      if ( currentStyle == penStyle[ i ] )
         cmbb_minorStyle->setCurrentIndex( i );

   }

   minorStyle->addWidget( cmbb_minorStyle );

   QLabel* lb_minorWidth = us_label( tr( "Width (pixels):" ) );
   minorStyle->addWidget( lb_minorWidth );

   sb_minorWidth = new QSpinBox;
   sb_minorWidth->setRange( 1, 5 ); 
   sb_minorWidth->setValue( minorPen.width() );

   minorStyle->addWidget( sb_minorWidth );

   main->addWidget( lb_minorStyle, row,   0 );
   main->addLayout( minorStyle   , row++, 1, 1, 2 );

   // Row 8
   QPushButton* pb_apply = us_pushbutton( tr( "Apply" ) );
   connect( pb_apply, SIGNAL( clicked() ), SLOT( apply() ) );

   QPushButton* pb_close = us_pushbutton( tr( "Close" ) );
   connect( pb_close, SIGNAL( clicked() ), SLOT( close() ) );
   
   QBoxLayout* applyClose = new QHBoxLayout();
   applyClose->addWidget( pb_apply );
   applyClose->addWidget( pb_close );

   main->addLayout( applyClose, row++, 0, 1, 3 );
}

void US_PlotGridConfig::selectMajorColor( void )
{
   QPalette p       = lb_showMajorColor->palette();
   QColor   current = p.color( QPalette::Window );
   QColor   c       = QColorDialog::getColor( current, this, 
                         tr( "Select major grid line color" ) );
   if ( c.isValid() )
   {
     p.setColor( QPalette::Window, c );
     lb_showMajorColor->setPalette( p );
   }
}

void US_PlotGridConfig::selectMinorColor( void )
{
   QPalette p       = lb_showMinorColor->palette();
   QColor   current = p.color( QPalette::Window );
   QColor   c       = QColorDialog::getColor( current, this, 
                         tr( "Select minor grid line color" ) );
   if ( c.isValid() )
   {
     p.setColor( QPalette::Window, c );
     lb_showMinorColor->setPalette( p );
   }
}

void US_PlotGridConfig::apply( void )
{
   grid->enableX   ( cb_enableX     ->isChecked() );   
   grid->enableY   ( cb_enableY     ->isChecked() );   
   grid->enableXMin( cb_enableXminor->isChecked() );   
   grid->enableYMin( cb_enableYminor->isChecked() );

   // Set the major pen
   QPen pen = grid->majPen();

   pen.setColor( lb_showMajorColor->palette().color( QPalette::Window ) );
   pen.setWidth( sb_majorWidth->value() );
   
   int style;
   style = cmbb_majorStyle->itemData( cmbb_majorStyle->currentIndex() ).toInt();
   pen.setStyle( static_cast< Qt::PenStyle >( style ) );
   
   grid->setMajPen( pen );

   // Set the minor pen
   pen = grid->minPen();

   pen.setColor( lb_showMinorColor->palette().color( QPalette::Window ) );
   pen.setWidth( sb_minorWidth->value() );

   style = cmbb_minorStyle->itemData( cmbb_minorStyle->currentIndex() ).toInt();
   pen.setStyle( static_cast< Qt::PenStyle >( style ) );

   grid->setMinPen( pen );

   plot->replot();
}

/**************************    US_PlotPicker Class    *********************/

/*!  \brief Customize plot picker characteristics and mouse events
     \param plot  The plot to attach to
*/

US_PlotPicker::US_PlotPicker( QwtPlot* plot ) 
  : QwtPlotPicker( QwtPlot::xBottom, QwtPlot::yLeft, plot->canvas() )
{
   setSelectionFlags( QwtPicker::PointSelection );
   setTrackerMode   ( QwtPicker::AlwaysOn );
   setRubberBand    ( QwtPicker::CrossRubberBand );

   QColor c = US_GuiSettings::plotPicker();
   setRubberBandPen ( c );
   setTrackerPen    ( c );
}

void US_PlotPicker::widgetMousePressEvent( QMouseEvent* e )
{
   // Prevent spurious clicks
   static QTime last_click;

   if ( last_click.isNull() || last_click.elapsed() > 300 )
   {
      last_click.start();
      if ( e->button() == Qt::LeftButton ) 
         emit mouseDown( invTransform( e->pos() ) );
      if ( e->button() == Qt::LeftButton && e->modifiers() == Qt::ControlModifier )
      {
         emit cMouseDown   ( invTransform( e->pos() ) );
         emit cMouseDownRaw( e );
      }
   }
   
   QwtPlotPicker::widgetMousePressEvent( e );
}

void US_PlotPicker::widgetMouseReleaseEvent( QMouseEvent* e )
{
   static QTime last_click;

   if ( last_click.isNull() || last_click.elapsed() > 300 ) 
   {
      last_click.start();
      if ( e->button() == Qt::LeftButton )
         emit mouseUp( invTransform( e->pos() ) );
      
      if ( e->button() == Qt::LeftButton && e->modifiers() == Qt::ControlModifier )
         emit cMouseUp( invTransform( e->pos() ) );
   }

   QwtPlotPicker::widgetMouseReleaseEvent( e );
}

void US_PlotPicker::widgetMouseMoveEvent( QMouseEvent* e )
{
   static QTime last_click;

   // Slow things down a bit
   if ( last_click.isNull() || last_click.elapsed() > 100 )
   {
      last_click.start();
      if ( e->button() == Qt::LeftButton )
         emit mouseDrag( invTransform( e->pos() ) );

      if ( e->modifiers() == Qt::ControlModifier )
      {
         emit cMouseDrag( invTransform( e->pos() ) );
      }
   }

   QwtPlotPicker::widgetMouseMoveEvent( e );
}


