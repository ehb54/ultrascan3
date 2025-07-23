//! \file us_plot.cpp

#include <QApplication>
#include <QtSvg>
#include "us_plot.h"
#include "us_images.h"
#if QT_VERSION > 0x050000
#include <QtPrintSupport>
#include "us_colorgradIO.h"
#include "qwt_picker_machine.h"
#include "qwt_picker.h"
#include "qwt_color_map.h"
#define canvasBackground() canvasBackground().color()
#else
#define majorPen(a)    majPen(a)
#define minorPen(a)    minPen(a)
#define setSymbol(a)   setSymbol(*a)
#define drawSymbol(a,b) draw(a,b)
#define QwtLogScaleEngine QwtLog10ScaleEngine
#endif

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
#if QT_VERSION < 0x050000
   setSelectionFlags( QwtPicker::DragSelection | QwtPicker::CornerToCorner );
   setTrackerMode   ( QwtPicker::AlwaysOff );
   setRubberBand    ( QwtPicker::NoRubberBand );
#else
   setStateMachine  ( new QwtPickerDragRectMachine() );
   setTrackerMode   ( QwtPicker::ActiveOnly );
   setRubberBand    ( QwtPicker::RectRubberBand );
#endif

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
      const QString& x_axis, const QString& y_axis, const bool cmEnab,
      const QString cmMatch, const QString cmName ) : QHBoxLayout()
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
   btnZoom->setIcon(QIcon( QPixmap( zoom_xpm ) ) );
   btnZoom->setCheckable( true );
   btnZoom->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
   btnZoom->setFont( buttonFont );
   btnZoom->setIconSize ( QSize( 20, 20 ) );
   btnZoom->setFixedSize( QSize( 40, 50 ) );
   connect( btnZoom, SIGNAL( toggled( bool ) ), SLOT( zoom( bool ) ) );

   QToolButton* btnCSV = new QToolButton( toolBar );
   btnCSV->setText( "CSV" );
   btnCSV->setIcon( US_Images::getIcon( US_Images::TABLE ) );
   btnCSV->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
   btnCSV->setFont( buttonFont );
   btnCSV->setIconSize ( QSize( 20, 20 ) );
   btnCSV->setFixedSize( QSize( 40, 50 ) );
   connect( btnCSV, SIGNAL( clicked() ), SLOT( csv() ) );

   QToolButton* btnPrint = new QToolButton( toolBar );
   btnPrint->setText( "Print" );
   btnPrint->setIcon( QIcon( QPixmap( print_xpm ) ) );
   btnPrint->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
   btnPrint->setFont( buttonFont );
   btnPrint->setIconSize ( QSize( 20, 20 ) );
   btnPrint->setFixedSize( QSize( 40, 50 ) );
   connect( btnPrint, SIGNAL( clicked() ), SLOT( print() ) );

   QToolButton* btnSVG = new QToolButton( toolBar );
   btnSVG->setText( "SVG" );
   btnSVG->setIcon( QIcon( QPixmap( vec_xpm ) ) );
   btnSVG->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
   btnSVG->setFont( buttonFont );
   btnSVG->setIconSize ( QSize( 20, 20 ) );
   btnSVG->setFixedSize( QSize( 40, 50 ) );
   connect( btnSVG, SIGNAL( clicked() ), SLOT( svg() ) );

   QToolButton* btnPNG = new QToolButton( toolBar );
   btnPNG->setText( "PNG" );
   btnPNG->setIcon( QIcon( QPixmap( ras_xpm ) ) );
   btnPNG->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
   btnPNG->setFont( buttonFont );
   btnPNG->setIconSize ( QSize( 20, 20 ) );
   btnPNG->setFixedSize( QSize( 40, 50 ) );
   connect( btnPNG, SIGNAL( clicked() ), SLOT( png() ) );

   QToolButton* btnConfig = new QToolButton( toolBar );
   btnConfig->setText( "Config" );
   btnConfig->setIcon(QIcon( QPixmap( configure_32_xpm ) ) );
   btnConfig->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
   btnConfig->setFont( buttonFont );
   btnConfig->setIconSize ( QSize( 20, 20 ) );
   btnConfig->setFixedSize( QSize( 40, 50 ) );
   connect( btnConfig, SIGNAL( clicked() ), SLOT( config() ) );

   btnCMap                = new QToolButton( toolBar );
   btnCMap  ->setText( "CMap" );
   btnCMap  ->setIcon( QIcon( QPixmap( cmap_xpm ) ) );
   btnCMap  ->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
   btnCMap  ->setFont( buttonFont );
   cmapEnab      = cmEnab;
   cmapMatch     = cmMatch;
   btnCMap  ->setVisible( cmapEnab );
   connect( btnCMap,   SIGNAL( clicked() ), SLOT( colorMap() ) );

   toolBar->addWidget( btnZoom   );
   toolBar->addWidget( btnCSV    );
   toolBar->addWidget( btnPrint  );
   toolBar->addWidget( btnSVG    );
   toolBar->addWidget( btnPNG    );
   toolBar->addWidget( btnConfig );
   toolBar->addWidget( btnCMap   );

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
#if QT_VERSION > 0x050000
   plot->setAutoDelete( false );
#endif
  
   QFont font( US_GuiSettings::fontFamily(),
               US_GuiSettings::fontSize(),
               QFont::Bold );
           
   font.setPointSizeF( US_GuiSettings::fontSize() * 1.4 );

   QwtText qwtTitle( title );
   qwtTitle.setFont( font );
   plot->setTitle( qwtTitle );

#if QT_VERSION > 0x050000
   plot->setStyleSheet( QString( "QwtPlot{ padding: %1px }" )
         .arg( US_GuiSettings::plotMargin() ) );
#else
   plot->setMargin( US_GuiSettings::plotMargin() );
#endif

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
   plot->setAxisFont( QwtPlot::yRight,  font );
   if ( plot->legend() != NULL )
      plot->legend()->setFont( font );
  
   plot->setAutoFillBackground( true );
   plot->setPalette ( US_GuiSettings::plotColor() );
   plot->setCanvasBackground( US_GuiSettings::plotCanvasBG() );

   addWidget( plot );

   // Create default color map file path if need be
   cmfpath        = QString();
qDebug() << "UP:main: cmName" << cmName;
   if ( ! cmName.isEmpty() )
   {  // Default Color Map Name is given
      cmfpath        = cmName;
      if ( ! cmfpath.startsWith( "cm-" ) )
      {  // Prepend with "cm-" if missing
         cmfpath        = "cm-" + cmfpath;
      }
      if ( ! cmfpath.endsWith( ".xml-" ) )
      {  // Append with ".xml" if missing
         cmfpath        = cmfpath + ".xml";
      }
      // Prepend with etc directory to make full path
      cmfpath        = US_Settings::etcDir() + "/" + cmfpath;
   }
qDebug() << "UP:main: cmfpath" << cmfpath;
}

void US_Plot::zoom( bool on )
{
   if ( on )
   {
      // Set up for zooming
      zoomer = new US_Zoomer( QwtPlot::xBottom, QwtPlot::yLeft,
                              (QwtPlotCanvas*)plot->canvas() );
      
      zoomer->setRubberBand   ( QwtPicker::RectRubberBand );
      zoomer->setRubberBandPen( QColor( Qt::green ) );
      zoomer->setTrackerMode  ( QwtPicker::ActiveOnly );
      zoomer->setTrackerPen   ( QColor( Qt::white ) );

#if QT_VERSION < 0x050000
      connect( zoomer, SIGNAL ( zoomed(        QwtDoubleRect ) ), 
                       SIGNAL ( zoomedCorners( QwtDoubleRect ) ) );
      connect( zoomer, SIGNAL ( zoomed(        QwtDoubleRect ) ),
              this  , SLOT   ( scale_yRight ( QwtDoubleRect ) ) );
#else
      connect( zoomer, SIGNAL ( zoomed(        QRectF        ) ), 
                       SIGNAL ( zoomedCorners( QRectF        ) ) );
      connect( zoomer, SIGNAL ( zoomed(        QRectF        ) ),
              this  , SLOT   ( scale_yRight ( QRectF        ) ) );
#endif
      
      panner = new QwtPlotPanner( plot->canvas() );
      panner->setMouseButton( Qt::MidButton );

#if QT_VERSION > 0x050000
      picker = new QwtPlotPicker( QwtPlot::xBottom, QwtPlot::yLeft,
                     QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn,
                     plot->canvas() );
      picker->setStateMachine ( new QwtPickerDragRectMachine() );
#else
      picker = new QwtPlotPicker( QwtPlot::xBottom, QwtPlot::yLeft,
                     QwtPicker::PointSelection | QwtPicker::DragSelection,
                     QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn,
                     plot->canvas() );
#endif

      picker->setRubberBandPen( QColor( Qt::green ) );
      picker->setRubberBand   ( QwtPicker::CrossRubberBand );
      picker->setTrackerPen   ( QColor( Qt::white ) );
      plot->setAxisAutoScale(QwtPlot::yRight, true);
      yLeftRange.fill(0, 2);
      yLeftRange[0] = plot->axisScaleDiv(QwtPlot::yLeft).lowerBound();
      yLeftRange[1] = plot->axisScaleDiv(QwtPlot::yLeft).upperBound();
      yRightRange.fill(0, 2);
      if (plot->axisEnabled(QwtPlot::yRight)){
         yRightRange[0] = plot->axisScaleDiv(QwtPlot::yRight).lowerBound();
         yRightRange[1] = plot->axisScaleDiv(QwtPlot::yRight).upperBound();
      }
   }
   
   panner->setEnabled( on );

   zoomer->setEnabled( on );
   zoomer->zoom( 0 );

   picker->setEnabled( ! on );

   if ( ! on  &&  zoomer != NULL )
   {
      zoomer->disconnect();
      delete picker;
      delete panner;
      delete zoomer;
      zoomer = NULL;
   }
}

void US_Plot::csv( void )
{
   QDir dir;
   QString reportDir = US_Settings::reportDir();
   if ( ! dir.exists( reportDir ) ) dir.mkpath( reportDir );

   QString fileName = QFileDialog::getSaveFileName( plot, 
      tr( "Export File Name" ), reportDir, 
                                                     tr( "CSV Documents (*.csv)" ) );

   if ( ! fileName.isEmpty() )
   {
        if ( fileName.right( 4 ) != ".csv" ) fileName += ".csv";

        US_GuiUtil::save_csv( fileName, plot );
   }
}

void US_Plot::svg( void )
{
   QDir dir;
   QString reportDir = US_Settings::reportDir();
   if ( ! dir.exists( reportDir ) ) dir.mkpath( reportDir );

   QString fileName = QFileDialog::getSaveFileName( plot,
      tr( "Export File Name" ), reportDir,
      tr( "SVG Documents (*.svgz)" ) );

   if ( ! fileName.isEmpty() )
   {
      if ( fileName.right( 5 ) != ".svgz" ) fileName += ".svgz";

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
#ifdef Q_OS_LINUX
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
#if QT_VERSION > 0x050000
       plot->drawCanvas( new QPainter( &printer ) );
#else
       QwtPlotPrintFilter filter;
       if ( printer.colorMode() == QPrinter::GrayScale )
       {
           int options = QwtPlotPrintFilter::PrintAll;
           options    &= ~QwtPlotPrintFilter::PrintBackground;
           options    |= QwtPlotPrintFilter::PrintFrameWithScales;
           filter.setOptions( options );
       }
       plot->print( printer, filter );
#endif
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

// Slot to set curve colors by color gradient read from file
void US_Plot::colorMap( void )
{
qDebug() << "UP:CM: colorMap chosen";
   // Get an xml file name for the color map
   QString filter = tr( "Color Map files (*cm-*.xml);;" )
         + tr( "Any XML files (*.xml);;" )
         + tr( "Any files (*)" );

   cmfpath        = QFileDialog::getOpenFileName( (QWidget*)plot,
      tr( "Load Color Map File" ),
      US_Settings::etcDir(), filter, 0, 0 );
qDebug() << "UP:CM: post-dialog cmfpath" << cmfpath;

   // Get the color gradient from the file
   QList< QColor > mcolors;
   int nmcols    = 0;
   if ( !cmfpath.isEmpty() )
   {
      US_ColorGradIO::read_color_gradient( cmfpath, mcolors );
      nmcols        = mcolors.count();
   }
   if ( nmcols == 0 )
   {
      nmcols        = 1;
      mcolors << US_GuiSettings::plotCurve();
   }
qDebug() << "UP:CM: cmfpath" << cmfpath << "nmcols" << nmcols;
   int ntcurv    = 0;
   int nmcurv    = 0;
   int mcolx     = 0;
   bool tmatch   = ! cmapMatch.isEmpty();    // Curve type filter
   QRegExp cmMatch( cmapMatch );             // Curve title match
qDebug() << "UP:CM: cmapMatch" << cmapMatch << "tmatch" << tmatch;
   QwtPlotItemList list = plot->itemList();  // All plot items

   // Examine each plot item, looking for curves and ones that match
   for ( int ii = 0; ii < list.size(); ii++ )
   {
      if ( list[ ii ]->rtti() == QwtPlotItem::Rtti_PlotCurve )
      {  // This is a curve
         ntcurv++;      // Total curves
         QwtPlotCurve* curve = dynamic_cast< QwtPlotCurve* >( list[ ii ] );
         QString ctitle      = curve->title().text();
//qDebug() << "UP:CM:   ii" << ii << "ctitle" << ctitle;
         if ( !tmatch  ||  ctitle.contains( cmMatch ) )
         {  // No matching or this curve's title matches
            nmcurv++;   // Matching curves
            // Set curve color using modulo color gradient color
//qDebug() << "UP:CM:     *MATCH* nmcurv" << nmcurv;
//qDebug() << "UP:CM:   ii" << ii << "mcolx" << mcolx << "mcolor" << mcolors[mcolx];
            curve->setPen( QPen( mcolors[ mcolx ] ) );
            mcolx++;    // Match-curve index modulo colors
            if ( mcolx >= nmcols )
               mcolx       = 0;
         }
      }
   }
   plot->replot();
qDebug() << "UP:CM: ntcurv" << ntcurv << "nmcurv" << nmcurv;
}

// Public method to return color gradient list and its count
int US_Plot::map_colors( QList< QColor >& mcolors )
{
   int nmcol   = 0;

   if ( cmfpath.isEmpty() )
   {  // No color map file was read:  revert to single color
      mcolors.clear();
      mcolors << US_GuiSettings::plotCurve();
      nmcol       = 1;
   }

   else
   {  // Get and return the colors from a map file
      US_ColorGradIO::read_color_gradient( cmfpath, mcolors );
      nmcol       = mcolors.count();
   }

   return nmcol;
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

#if QT_VERSION < 0x050000
void US_Plot::scale_yRight ( QwtDoubleRect ){}
#else
void US_Plot::scale_yRight ( QRectF rect ){
   if (! plot->axisEnabled ( QwtPlot::yRight )){
      return;
   }
   double dyL = yLeftRange.at(1) - yLeftRange.at(0);
   double ys_0 = (rect.top() - yLeftRange.at(0)) / dyL;
   double ys_1 = (rect.bottom() - yLeftRange.at(0)) / dyL;
   double dyR = yRightRange.at(1) - yRightRange.at(0);
   double y_0 = ys_0 * dyR + yRightRange.at(0);
   double y_1 = ys_1 * dyR + yRightRange.at(0);
   plot->setAxisScale( QwtPlot::yRight, y_0, y_1 );
   plot->replot();
}
#endif

////////////////////////////////////////

US_PlotPushbutton::US_PlotPushbutton( const QString& labelString, 
      QWidget* w, int index ) : QPushButton( labelString.toLatin1(), w )
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
  Qt::WindowFlags f ) : US_WidgetsDialog( p, f , false) //( false, p, f )
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
   qDebug() << "Plot Config INIT: canvas color -- " << c.name(QColor::HexRgb);
   global_canvas_color = c; //ALEXEY
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
   cmbb_margin = us_comboBox();
   
   for ( int i = 2; i <= 30; i += 2 )  
   {  
      cmbb_margin->addItem( QString::number( i ) + " pixels" );
   }
   
   QString style = plot->styleSheet();
   int     kk    = style.indexOf( "padding:" );
   if ( kk < 0 )
   {
      kk            = 0;
   }
   else
   {
     QString padding_v = style.mid( kk ).split(": ")[1];
     padding_v. simplified();
     padding_v.indexOf( "px" ); 
     // qDebug() << " padding_v --         " <<  padding_v.left( padding_v.indexOf( "px" ) );
     // qDebug() << " padding_v.toInt() -- " <<  padding_v.left( padding_v.indexOf( "px" ) ).toInt();
     
     kk            = padding_v.left( padding_v.indexOf( "px" ) ).toInt();
             
     // kk            = style.mid( kk ).toInt();
     kk            = qMax( 0, ( kk / 2 - 1 ) );
   }
   cmbb_margin->setCurrentIndex( kk );
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
      default                      : break; // Shouldn't happen
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

   //Row 12a
   QLabel* lb_plot_profile = us_banner( tr( "----- Plot Profile -----" ) );
   lb_plot_profile->setFixedHeight( 40 );
   
   main->addWidget( lb_plot_profile, row++, 0, 1, 3 );

   //Row 12b
   QPushButton* pb_loadProfile = us_pushbutton( tr( "Load Plot Profile" ) );
   connect( pb_loadProfile, SIGNAL( clicked() ), SLOT( loadPlotProfile() ) );

   QPushButton* pb_saveProfile = us_pushbutton( tr( "Save Plot Profile" ) );
   connect( pb_saveProfile, SIGNAL( clicked() ), SLOT( savePlotProfile() ) );

   QBoxLayout* profile_settings = new QHBoxLayout();
   profile_settings->addWidget( pb_loadProfile );
   profile_settings->addWidget( pb_saveProfile );

   main->addLayout( profile_settings, row++, 0, 1, 3 );
   
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
   QPalette pal     = lb_showCanvasColor->palette();
   QColor   current = pal.color( QPalette::Active, QPalette::Window );
   QColor   col     = QColorDialog::getColor( current, this, tr( "Select canvas color" ) );

   qDebug() << "Canvas Color Selected: " << col;
   
   if ( col.isValid() )
   {
      pal.setColor( QPalette::Window, col );
      lb_showCanvasColor->setPalette( pal );
      
      //global_canvas_palette = pal;
      global_canvas_color = col;   //ALEXEY
      
#if QT_VERSION > 0x050000
      plot->setCanvasBackground( QBrush( col ) );
#else
      plot->setCanvasBackground( col );
#endif
      plot->replot();
   }

   
   qDebug() << "Canvas Color Selected in Plot(): " << plot->canvasBackground();
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
////   plot->setMargin( ( index + 1 ) * 2 );
//   plot->setStyleSheet( QString( "QwtPlot{ padding: %1px }" )
//         .arg( ( index + 1 ) * 2 ) );

//   QString style = plot->styleSheet();
//   qDebug() << "In ::selectMargin() Plot's style -- " << style;

   int padding = ( index + 1 ) *  2;
   plot->setContentsMargins(padding, padding, padding, padding);
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
#if QT_VERSION > 0x050000
   else
   {
      // For Qwt6/Qt5, more than a legend setFont() is needed
      QMessageBox::warning( this,
        tr( "Legend Font Unchanged" ), 
        tr( "Currently, with Qwt6/Qt5 versions, the Legend\n"
            "Font cannot be changed." ) );
      return;
   }
#endif

   QFont font = plot->legend()->font();

   bool ok;
   QFont newFont = QFontDialog::getFont( &ok, font, this,
                       tr( "Set Legend Font" ), 0 );

   if ( ok )
   {
#if QT_VERSION > 0x050000
      // For Qwt6/Qt5, more than a legend setFont() is needed
      plot->legend()->setFont( newFont );
#else
      // For Qwt5/Qt4, only a legend setFont() is needed
      plot->legend()->setFont( newFont );
#endif
      setLegendFontString();
      plot->replot();
   }
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

/* Load/Save Plot's profile 
 */
QFont US_PlotConfig::jsonToFont(QJsonObject font_obj){
    QFont font;
    foreach (const QString& key, font_obj.keys()) {
        QString value = font_obj.value(key).toString();
        if (key.compare(tr("Family"), Qt::CaseInsensitive) == 0)
            font.setFamily(value);
        else if (key.compare(tr("Size"), Qt::CaseInsensitive) == 0)
            font.setPointSize(value.toInt());
        else if (key.compare(tr("Weight"), Qt::CaseInsensitive) == 0)
            font.setWeight(static_cast<QFont::Weight>(QMetaEnum::fromType<QFont::Weight>().
                                                      keyToValue(value.toUtf8())));
        else if (key.compare(tr("Style"), Qt::CaseInsensitive) == 0)
            font.setStyle(static_cast<QFont::Style>(QMetaEnum::fromType<QFont::Style>().
                                                    keyToValue(value.toUtf8())));
        else if (key.compare(tr("Underline"), Qt::CaseInsensitive) == 0){
            if (value.compare(tr("true"), Qt::CaseInsensitive) == 0)
                font.setUnderline(true);
            else
                font.setUnderline(false);
        } else if (key.compare(tr("Strike Out"), Qt::CaseInsensitive) == 0){
            if (value.compare(tr("true"), Qt::CaseInsensitive) == 0)
                font.setStrikeOut(true);
            else
                font.setStrikeOut(false);
        }
    }

    qDebug() << "Loaded font info:";
    qDebug() << "Size: " << font.pointSize();
    qDebug() << "Weight: " << QString::fromUtf8(QMetaEnum::fromType<QFont::Weight>().valueToKey(font.weight()));
    qDebug() << "Style: " << QString::fromUtf8(QMetaEnum::fromType<QFont::Style>().valueToKey(font.style()));
    QString font_underline = font.underline() ? "true" : "false";
    QString font_strikeout = font.strikeOut() ? "true" : "false";
    qDebug() << "Underline: " << font_underline;
    qDebug() << "Strikeout: " << font_strikeout;
    qDebug() << "Family: " << font.family();
    return font;
}

QMap<QString, bool> US_PlotConfig::parseGridJson( QJsonObject grid_obj, QPen* pen){
    QMap<QString, bool> out;
    out["Vertical"] = false;
    out["Horizontal"] = false;
    QString value;
    foreach (const QString& key, grid_obj.keys()) {
        value = grid_obj.value(key).toString();
        if (key.compare(tr("Style"), Qt::CaseInsensitive) == 0)
            pen->setStyle(static_cast<Qt::PenStyle>(QMetaEnum::fromType<Qt::PenStyle>().
                                                 keyToValue(value.toUtf8())));
        else if (key.compare(tr("Width"), Qt::CaseInsensitive) == 0)
            pen->setWidth(value.toInt());
        else if (key.compare(tr("Vertical"), Qt::CaseInsensitive) == 0){
            if (value.compare(tr("true"), Qt::CaseInsensitive) == 0)
                out["Vertical"] = true;
        }else if (key.compare(tr("Horizontal"), Qt::CaseInsensitive) == 0){
            if (value.compare(tr("true"), Qt::CaseInsensitive) == 0)
                out["Horizontal"] = true;
        }
    }
    return out;
}

void US_PlotConfig::setTitleJson(QJsonObject title_obj){
    QFont font;
    QString text;
    foreach (const QString& key, title_obj.keys()) {
        QJsonValue value = title_obj.value(key);
        if (key.compare(tr("Text"), Qt::CaseInsensitive) == 0)
            text = value.toString();
        else if (key.compare(tr("Font"), Qt::CaseInsensitive) == 0)
            font = jsonToFont(value.toObject());
    }
    le_titleText->setText(text);
    le_titleFont->setText( font.family() + ", "
                           + QString::number( font.pointSize() ) + tr( " points" ) );
    QwtText p_title = plot->title();
    p_title.setText(text);
    p_title.setFont(font);
    plot->setTitle(p_title);
}

void US_PlotConfig::setGridJson( QJsonObject grid_obj){

    int ngrid = 0;
    // Get the grid - if it exists
    QwtPlotItemList list = plot->itemList();
    QwtPlotGrid*  grid = NULL;

    for ( int i = 0; i< list.size(); i++ )
    {
       if ( list[i]->rtti() == QwtPlotItem::Rtti_PlotGrid )
       {
          ngrid++;
          if ( ngrid == 1 )
             grid = dynamic_cast<QwtPlotGrid*>( list[ i ] );
          else
             list[ i ]->detach();
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

    QJsonObject json_obj;
    if (grid_obj.contains(tr("Major"))){
        json_obj = grid_obj.value(tr("Major")).toObject();
        QPen pen = grid->majorPen();
        QPen* pp = &pen;
        QMap<QString, bool> states;
        states = parseGridJson(json_obj, pp);
        grid->enableX(states["Vertical"]);
        grid->enableY(states["Horizontal"]);
    } else if (grid_obj.contains(tr("Minor"))){
        json_obj = grid_obj.value(tr("Minor")).toObject();
        QPen pen = grid->minorPen();
        QPen* pp = &pen;
        QMap<QString, bool> states;
        states = parseGridJson(json_obj, pp);
        grid->enableXMin(states["Vertical"]);
        grid->enableYMin(states["Horizontal"]);
    }
    plot->replot();
}

void US_PlotConfig::setAxisJson( int axis_id, QJsonObject axis_obj){

    foreach (const QString& key, axis_obj.keys()) {
        QJsonValue value = axis_obj.value(key);
        if (key.compare(tr("Enable"), Qt::CaseInsensitive) == 0){
            QString axis_enable = value.toString();
            bool state = false;
            if (axis_enable.compare(tr("true"), Qt::CaseInsensitive) == 0)
                state = true;
            plot->enableAxis(axis_id, state);
        } else if (key.compare(tr("Title"), Qt::CaseInsensitive) == 0){
            QJsonObject json_obj = value.toObject();
            QwtText title = plot->axisTitle( axis_id );
            foreach (const QString& kk, json_obj.keys()) {
                QJsonValue value = json_obj.value(kk);
                if (kk.compare(tr("Text"), Qt::CaseInsensitive) == 0)
                    title.setText(value.toString());
                else if (kk.compare(tr("Color"), Qt::CaseInsensitive) == 0)
                    title.setColor(QColor(Qt::black));
                else if (kk.compare(tr("Font"), Qt::CaseInsensitive) == 0)
                    title.setFont(jsonToFont(value.toObject()));
            }
            plot->setAxisTitle( axis_id, title );
        } else if (key.compare(tr("Tick Color"), Qt::CaseInsensitive) == 0){
//            QPalette axisPalette = plot->axisWidget( axis_id )->palette();
//            axisPalette.setColor( QPalette::Text, QColor(value.toString()) );
//            plot->axisWidget( axis_id )->setPalette( axisPalette );
            continue;
        } else if (key.compare(tr("Scale"), Qt::CaseInsensitive) == 0){
            QJsonObject json_obj = value.toObject();
            int attributes = 0;
            double lower_bound = 0;
            double upper_bound = 0;
            double step_size = 0;
            double reference = 0;
            bool auto_scale = true;
//            QPalette palette = plot->axisWidget( axis_id )->palette();
//            bool linear = true;
//            bool logarithmic = false;
//            QFont font = plot->axisFont( axis_id );
            QFont font;
            foreach (const QString& kk, json_obj.keys()) {
                QJsonValue value = json_obj.value(kk);
                if (kk.compare(tr("Autoscale"), Qt::CaseInsensitive) == 0){
                    if (value.toString().compare("false", Qt::CaseInsensitive) == 0)
                        auto_scale = false;
                } else if (kk.compare(tr("Color"), Qt::CaseInsensitive) == 0){
//                    palette.setColor( QPalette::Text, QColor(value.toString()));
                    continue;
                } else if (kk.compare(tr("Floating Endpoints"), Qt::CaseInsensitive) == 0){
                    if (value.toString().compare("true", Qt::CaseInsensitive) == 0)
                        attributes |= (int)QwtScaleEngine::Floating;
                } else if (kk.compare(tr("Include Reference"), Qt::CaseInsensitive) == 0){
                    if (value.toString().compare("true", Qt::CaseInsensitive) == 0)
                        attributes |= (int)QwtScaleEngine::IncludeReference;
                } else if (kk.compare(tr("Inverted"), Qt::CaseInsensitive) == 0){
                    if (value.toString().compare("true", Qt::CaseInsensitive) == 0)
                        attributes |= (int)QwtScaleEngine::Inverted;
                } else if (kk.compare(tr("Symmetric"), Qt::CaseInsensitive) == 0){
                    if (value.toString().compare("true", Qt::CaseInsensitive) == 0)
                        attributes |= (int)QwtScaleEngine::Symmetric;
                } else if (kk.compare(tr("Lower Bound"), Qt::CaseInsensitive) == 0){
                    lower_bound = value.toString().toDouble();
                } else if (kk.compare(tr("Upper Bound"), Qt::CaseInsensitive) == 0){
                    upper_bound = value.toString().toDouble();
                } else if (kk.compare(tr("Step Size"), Qt::CaseInsensitive) == 0){
                    step_size = value.toString().toDouble();
                } else if (kk.compare(tr("Font"), Qt::CaseInsensitive) == 0){
                    font = jsonToFont(value.toObject());
                } else if (kk.compare(tr("Linear"), Qt::CaseInsensitive) == 0){
                    if (value.toString().compare("true", Qt::CaseInsensitive) == 0)
                        continue; //linear = true;
                } else if (kk.compare(tr("Logarithmic"), Qt::CaseInsensitive) == 0){
                    if (value.toString().compare("true", Qt::CaseInsensitive) == 0)
                        continue; //logarithmic = true;
                } else if (kk.compare(tr("Reference"), Qt::CaseInsensitive) == 0){
                    reference = value.toString().toDouble();
                }
            }

            plot->setAxisFont( axis_id, font );
//            plot->axisWidget( axis_id )->setPalette( palette );
            if (auto_scale)
                plot->setAxisAutoScale(axis_id);
            else
                plot->setAxisScale( axis_id, lower_bound, upper_bound, step_size);
            plot->axisScaleEngine( axis_id )->setReference( reference );
            plot->axisScaleEngine( axis_id )->setAttributes( (QwtScaleEngine::Attribute)attributes );
            plot->replot();
        }
    }

}


void US_PlotConfig::loadPlotProfile( void )
{
  qDebug() << "Loading Plot's profile -- ";

  QString canvas_color, frame_color, border_margin;
  QString legend_pos;
  QJsonObject title_obj, grid_obj, axes_obj;
  
  //open local file
  QString dirPath    = US_Settings::etcDir();
  QString p_filename = QFileDialog::getOpenFileName( this,
						     tr("Open Plot's Profile"),
						     dirPath,
						     tr("Profile Files (*.json)") );

  QFile json_plot_profile( p_filename );
  if (json_plot_profile.open(QIODevice::ReadOnly))
    {
      QByteArray bytes = json_plot_profile.readAll();
      json_plot_profile.close();
      
      QJsonParseError jsonError;
      QJsonDocument document = QJsonDocument::fromJson( bytes, &jsonError );
      if( jsonError.error != QJsonParseError::NoError )
	{
       	  qDebug() << "Reading JSON Plot profile failed: " << jsonError.errorString();//.toStdString();
	  qDebug() << "Bytes: " << bytes;
       	  return;
	}
      if( document.isObject() )
	{
	  QJsonObject json_obj = document.object();

      foreach(const QString& key, json_obj.keys()){
	      QJsonValue value = json_obj.value(key);

          if (key.compare(tr("Title"), Qt::CaseInsensitive) == 0)
               title_obj = value.toObject();
          else if (key.compare(tr("Canvas Color"), Qt::CaseInsensitive) == 0)
              canvas_color = value.toString();
          else if (key.compare(tr("Frame Color"), Qt::CaseInsensitive) == 0)
              frame_color = value.toString();
          else if (key.compare(tr("Border Margin"), Qt::CaseInsensitive) == 0)
              border_margin = value.toString();
          else if (key.compare(tr("Legend Position"), Qt::CaseInsensitive) == 0)
              legend_pos = value.toString();
          else if (key.compare(tr("Grid"), Qt::CaseInsensitive) == 0)
              grid_obj = value.toObject();
          else if (key.compare(tr("Axes"), Qt::CaseInsensitive) == 0)
              axes_obj = value.toObject();
	    }
	}
    }

  qDebug() << "Loaded Plot Profile -- "
//	   << "Title: "           << p_title
	   << ", Canvas Color: "  << canvas_color
	   << ", Frame Color: "   << frame_color
       << ", Border Margin: " << border_margin;
//	   << ", Title Font (Family, Size): (" << p_title_font_family << ", " << p_title_font_size << ")";

  
  //set plot's parameters
  //1. Title
  if (! title_obj.isEmpty())
      setTitleJson(title_obj);
  
  //2. Border Margin
  plot->setStyleSheet( QString( "QwtPlot{ padding: %1px }" )
		       .arg( border_margin ) );

  int kk = border_margin.toInt();;
  kk  = qMax( 0, ( kk / 2 - 1 ) );
  cmbb_margin->setCurrentIndex( kk );

  //3. Frame Color
  QColor col_frame   = QColor( frame_color );
  QPalette pal_frame;
  
  if ( col_frame.isValid() )
    {
      pal_frame.setColor( QPalette::Active  , QPalette::Window, col_frame );
      pal_frame.setColor( QPalette::Inactive, QPalette::Window, col_frame );
      lb_showFrameColor->setPalette( pal_frame );
      plot->setPalette( pal_frame );
    }
  
  //4. Canvas Color
  QColor col_canvas     = QColor( canvas_color );
  QPalette pal_canvas;
  
  if ( col_canvas.isValid() )
    {
      pal_canvas.setColor( QPalette::Window, col_canvas );
      lb_showCanvasColor->setPalette( pal_canvas );
      
      global_canvas_color = col_canvas;   //ALEXEY
      
#if QT_VERSION > 0x050000
      plot->setCanvasBackground( QBrush( col_canvas ) );
#else
      plot->setCanvasBackground( col_canvas );
#endif
      plot->replot();
    }

  //5. Legend Position
  cmbb_legendPos->setCurrentText(legend_pos);
  selectLegendPos(cmbb_legendPos->currentIndex());

  //6. Grid
  if (! grid_obj.isEmpty())
      setGridJson(grid_obj);

  //7. Axes Config
  foreach (const QString& key, axes_obj.keys()) {
      QJsonObject value = axes_obj.value(key).toObject();
      if (key.compare(tr("xBottom"), Qt::CaseInsensitive) == 0)
          setAxisJson(QwtPlot::xBottom, value);
      else if (key.compare(tr("xTop"), Qt::CaseInsensitive) == 0)
          setAxisJson(QwtPlot::xTop, value);
      else if (key.compare(tr("yLeft"), Qt::CaseInsensitive) == 0)
          setAxisJson(QwtPlot::yLeft, value);
      else if (key.compare(tr("yRight"), Qt::CaseInsensitive) == 0)
          setAxisJson(QwtPlot::yRight, value);

  }

}

QJsonObject US_PlotConfig::getAxisJson(int axis_id){

    QJsonObject axis_obj;

    QString axis_enable = plot->axisEnabled(axis_id) ? tr("true") : tr("false");
    axis_obj.insert(tr("Enable"), axis_enable);

    QwtText title = plot->axisTitle( axis_id );
    QJsonObject title_obj;
    title_obj.insert(tr("Text"), title.text());
    title_obj.insert("Font", getFontJson(title.font()));
    title_obj.insert("Color", title.color().name(QColor::HexRgb));
    axis_obj.insert("Title", title_obj);

    QString tick_color = plot->axisWidget(axis_id)->palette().color(QPalette::Window).name(QColor::HexRgb);
    axis_obj.insert("Tick Color", tick_color);

    QJsonObject scale_obj;
    scale_obj.insert("Font", getFontJson(plot->axisFont(axis_id)));
    scale_obj.insert("Color", plot->axisWidget(axis_id)->palette().
                     color(QPalette::Text).name());
    QString lb = QString::number(plot->axisScaleDiv(axis_id).lowerBound());
    QString ub = QString::number(plot->axisScaleDiv(axis_id).upperBound());
    QString ss = QString::number(plot->axisStepSize(axis_id));
    scale_obj.insert("Lower Bound", lb);
    scale_obj.insert("Upper Bound", ub);
    scale_obj.insert("Step Size", ss);
    double reference = plot->axisScaleEngine(axis_id)->reference();
    scale_obj.insert("Reference", QString::number(reference));

    int attributes = (int)plot->axisScaleEngine(axis_id)->attributes();
    QString floating = (attributes & QwtScaleEngine::Floating) ? tr("true") : tr("false");
    QString inverted = (attributes & QwtScaleEngine::Inverted) ? tr("true") : tr("false");
    QString inc_ref = (attributes & QwtScaleEngine::IncludeReference) ? tr("true") : tr("false");
    QString symmetric = (attributes & QwtScaleEngine::Symmetric) ? tr("true") : tr("false");
    QString autoscale = (plot->axisAutoScale(axis_id)) ? tr("true") : tr("false");
    scale_obj.insert("Floating Endpoints", floating);
    scale_obj.insert("Inverted", inverted);
    scale_obj.insert("Include Reference", inc_ref);
    scale_obj.insert("Symmetric", symmetric);
    scale_obj.insert("Autoscale", autoscale);

    QString linear, log;
    QwtScaleEngine *engine = plot->axisScaleEngine(axis_id);
    if (dynamic_cast<QwtLogScaleEngine*>(engine)) {
        log = "true";
        linear = "false";
    } else {
        log = "false";
        linear = "true";
    }
    scale_obj.insert("Linear", log);
    scale_obj.insert("Logarithmic", linear);

    axis_obj.insert("Scale", scale_obj);
    return axis_obj;

}


QJsonObject US_PlotConfig::getGridJson(){
    QJsonObject grid_obj;

    QJsonObject major_obj;
    QJsonObject minor_obj;

    int ngrid = 0;
    // Get the grid - if it exists
    QwtPlotItemList list = plot->itemList();
    QwtPlotGrid*  grid = NULL;

    for ( int i = 0; i< list.size(); i++ )
    {
       if ( list[i]->rtti() == QwtPlotItem::Rtti_PlotGrid )
       {
          ngrid++;
          if ( ngrid == 1 )
             grid = dynamic_cast<QwtPlotGrid*>( list[ i ] );
          else
             list[ i ]->detach();
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

    QString major_x_state = grid->xEnabled() ? tr("true") : tr("false");
    QString major_y_state = grid->yEnabled() ? tr("true") : tr("false");
    QString minor_x_state = grid->xMinEnabled() ? tr("true") : tr("false");
    QString minor_y_state = grid->yMinEnabled() ? tr("true") : tr("false");

    QPen pen = grid->majorPen();
    QString major_style = QString::fromUtf8(QMetaEnum::fromType<Qt::PenStyle>().
                                            valueToKey(pen.style()));
    QString major_width = QString::number(pen.width());
    pen = grid->minorPen();
    QString minor_style = QString::fromUtf8(QMetaEnum::fromType<Qt::PenStyle>().
                                            valueToKey(pen.style()));
    QString minor_width = QString::number(pen.width());

    major_obj.insert("Horizontal", major_y_state);
    major_obj.insert("Vertical", major_x_state);
    major_obj.insert("Width", major_width);
    major_obj.insert("Style", major_style);

    minor_obj.insert("horizontal", minor_y_state);
    minor_obj.insert("Vertical", minor_x_state);
    minor_obj.insert("Width", minor_width);
    minor_obj.insert("Style", minor_style);

    grid_obj.insert("Major", major_obj);
    grid_obj.insert("Minor", minor_obj);

    return grid_obj;
}

QJsonObject US_PlotConfig::getFontJson(QFont font){
    QJsonObject font_obj;
    QString font_size = QString::number( font.pointSize() );
    QString font_weight = QString::fromUtf8(QMetaEnum::fromType<QFont::Weight>().
                                              valueToKey(font.weight()));
    QString font_style = QString::fromUtf8(QMetaEnum::fromType<QFont::Style>().
                                             valueToKey(font.style()));
    QString font_underline = font.underline() ? "true" : "false";
    QString font_strikeout = font.strikeOut() ? "true" : "false";
    font_obj.insert("Family", font.family());
    font_obj.insert("Size", font_size);
    font_obj.insert("Weight", font_weight);
    font_obj.insert("Style", font_style);
    font_obj.insert("Underline", font_underline);
    font_obj.insert("Strike Out", font_strikeout);
    return font_obj;
}

void US_PlotConfig::savePlotProfile( void )
{
  qDebug() << "Saving Plot's current profile -- ";
  QJsonObject profile_obj;

  /* Sections ******************************************************/
  //title

  QJsonObject title_font_obj = getFontJson(plot->title().font());

  QJsonObject title_obj;
  title_obj.insert("Text", plot->title().text());
  title_obj.insert("Font", title_font_obj);
  profile_obj.insert("Title", title_obj);

  //frame & Canvas color
  QString  frame_color = plot->palette().color( QPalette::Active, QPalette::Window ).name(QColor::HexRgb);
  qDebug() << "Plot's Frame Color Name: "      << frame_color;
  profile_obj.insert("Frame Color", frame_color);

  QString canvas_color = plot->canvasBackground().name(QColor::HexRgb);
  qDebug() << "Plot's Canvas Color Name: "      << canvas_color;
  profile_obj.insert("Canvas Color", canvas_color);

  //Border Margin
  QString style = plot->styleSheet();
  qDebug() << "Plot's style -- " << style;
  int     kk    = style.indexOf( "padding:" );
  if ( kk < 0 )
    {
      kk            = 0;
    }
  else
    {
      QString padding_v = style.mid( kk ).split(": ")[1];
      padding_v.simplified();
      padding_v.indexOf( "px" ); 
      qDebug() << " padding_v --         " <<  padding_v.left( padding_v.indexOf( "px" ) );
      qDebug() << " padding_v.toInt() -- " <<  padding_v.left( padding_v.indexOf( "px" ) ).toInt();
      
      kk            = padding_v.left( padding_v.indexOf( "px" ) ).toInt();
    }
  profile_obj.insert("Border Margin", QString::number(kk));

  //Legend position
  profile_obj.insert("Legend Position", cmbb_legendPos->currentText());

  // Grid config
  QJsonObject grid_config = getGridJson();
  profile_obj.insert("Grid", grid_config);

  //Axes config
  QJsonObject Axes_config;

  QJsonObject yLeft_config = getAxisJson(QwtPlot::yLeft);
  Axes_config.insert(tr("yLeft"), yLeft_config);

  QJsonObject yRight_config = getAxisJson(QwtPlot::yRight);
  Axes_config.insert(tr("yRight"), yRight_config);


  QJsonObject xBottom_config = getAxisJson(QwtPlot::xBottom);
  Axes_config.insert(tr("xBottom"), xBottom_config);

  QJsonObject xTop_config = getAxisJson(QwtPlot::xTop);
  Axes_config.insert(tr("xTop"), xTop_config);

  profile_obj.insert("Axes", Axes_config);

  
  /* End of Sections ******************************************************/

  //save to local file
  QString dirPath    = US_Settings::etcDir();
  QString p_filename = QFileDialog::getSaveFileName( this,
						     tr("Save Plot's Profile"),
						     dirPath,
						     tr("Profile Files (*.json)") );
  p_filename.contains( ".json") ? p_filename += "" : p_filename += ".json";
  QFile json_plot_profile( p_filename );
  if (json_plot_profile.open(QFile::WriteOnly | QFile::Truncate))
    {
      QJsonDocument jsonDoc;
      jsonDoc.setObject(profile_obj);
      json_plot_profile.write(jsonDoc.toJson());
    }
  
}

/*!  \brief Open US_PlotConfig dialog for changing the 
            selected curves elements
*/
void US_PlotConfig::updateCurve( void )
{
  qDebug() << "1. In Update Selected Curve: plot's canvas backgound color -- " << plot->canvasBackground();
  
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

   qDebug() << "SELECTED Curves -- " << selected;
   qDebug() << "2. In Update Selected Curve: plot's canvas backgound color -- " << plot->canvasBackground();

   //curveWidget = new US_PlotCurveConfig( plot, selected );
   curveWidget = new US_PlotCurveConfig( plot, selected, this  ); //ALEXEY - call curves' Config widget with explicitly defining parent (::US_PlotConfig)

   //curveWidget->setAttribute( Qt::WA_DeleteOnClose );
   //connect( curveWidget, SIGNAL( curveConfigClosed( void ) ),
   //                      SLOT  ( curveConfigFinish( void ) ) );
   //curveWidget->show();
   curveWidget->exec();
   qApp->processEvents();
   delete curveWidget;
   curveWidget = NULL;
#ifdef Q_OS_MAC
   QMessageBox::information( this, tr( "Curve Config" ),
      tr( "Curve Update Complete" ) );
#endif
   qApp->processEvents();
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
   gridWidget = NULL;
#ifdef Q_OS_MAC
   QMessageBox::information( this, tr( "Grid Config" ),
      tr( "Grid Update Complete" ) );
   qApp->processEvents();
#endif
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
      : US_WidgetsDialog( parent, f , false) //( false, parent, f )
{
  plotConfigW = (US_PlotConfig*)parent;         //ALEXEY: access to parent's US_PlotConfig
  qDebug() << "In parent US_PlotConfig widget, plot canvas Color is: " << plotConfigW -> global_canvas_color;
   
   qDebug() << "0. In US_PlotCurveConfig() Constructor: (current)plot's canvas Color -- " << currentPlot->canvasBackground();
  
   plot          = currentPlot;
   //ALEXEY: for some reason, plot's canvas background color is lost here: needs to reset to what has been selected in US_PlotConfig::
#if QT_VERSION > 0x050000
   plot->setCanvasBackground( QBrush( plotConfigW -> global_canvas_color ) );
#else
   plot->setCanvasBackground(  plotConfigW -> global_canvas_color );
#endif
   
   selectedItems = selected;

   qDebug() << "1. In US_PlotCurveConfig() Constructor: plot's canvas Color -- " << plot->canvasBackground();
   
   // Keep out of the way
   //move( pos() + QPoint( plot->rect().width(), 0 ) );

   setWindowTitle( tr( "Curve Configuration" ) );
   setPalette( US_GuiSettings::frameColor() );

   qDebug() << "2. In US_PlotCurveConfig() Constructor: plot's canvas Color -- " << plot->canvasBackground();

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

#if QT_VERSION > 0x050000
   QwtSymbol *selSymbol  = (QwtSymbol*)firstSelectedCurve->symbol();
#else
   QwtSymbol *selSymbol  = (QwtSymbol*)&firstSelectedCurve->symbol();
#endif

   // Set up the dialog
   int row = 0;

   QGridLayout* main = new QGridLayout( this );
   main->setContentsMargins( 2, 2, 2, 2 );
   main->setSpacing( 2 );
    
   // Row 0
   QLabel* lb_sample1 = us_label( tr( "Sample:" ) );
   
   lb_sample2 = new US_PlotLabel( this );
   QPalette p = lb_sample2->palette();
   p.setColor(  QPalette::Active, QPalette::Window, plot->canvasBackground() );
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

   sb_curveWidth = us_spinbox( 0 );
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
   cmbb_symbolStyle->addItem( tr( "Rectangle"       ), QwtSymbol::Rect      );
   cmbb_symbolStyle->addItem( tr( "Diamond"         ), QwtSymbol::Diamond   );
   cmbb_symbolStyle->addItem( tr( "Ellipse"         ), QwtSymbol::Ellipse   );
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
      << QwtSymbol::Ellipse   << QwtSymbol::DTriangle << QwtSymbol::UTriangle 
      << QwtSymbol::LTriangle << QwtSymbol::RTriangle << QwtSymbol::Cross    
      << QwtSymbol::XCross    << QwtSymbol::HLine     << QwtSymbol::VLine    
      << QwtSymbol::Star1     << QwtSymbol::Star2     << QwtSymbol::Hexagon;

   if ( selSymbol != NULL )
   {
      symbolStyle = selSymbol->style();

      qDebug() << "Curves props constr.: symbolStyle -- " << symbolStyle;
      for ( int i = 0; i < symbolStyles.size(); i++ )
      {
         if ( symbolStyles [ i ] == symbolStyle )
         {
            cmbb_symbolStyle->setCurrentIndex ( i );
            break;
         }
      }
   }
   else
   {
     qDebug() << "Curves props constr.: NULL ";
     cmbb_symbolStyle->setCurrentIndex( 0 );
     selSymbol             = new QwtSymbol();
   }

   connect( cmbb_symbolStyle, SIGNAL( currentIndexChanged( int ) ), 
                              SLOT  ( symbolStyleChanged ( int ) ) );

   main->addWidget( lb_symbolStyle  , row  , 0 );
   main->addWidget( cmbb_symbolStyle, row++, 1, 1, 2 );

   // Row 7  Symbol width and height
   QLabel* lb_symbolWidth  = us_label( tr( "Width (pixels):" ) );

   sb_symbolWidth = us_spinbox( 0 );
   sb_symbolWidth->setRange( 1, 30 );
   sb_symbolWidth->setValue( selSymbol->size().width() );
   connect( sb_symbolWidth, SIGNAL( valueChanged( int ) ), 
                            SLOT  ( updateSample( int ) ) );

   QLabel* lb_symbolHeight = us_label( tr( "Height (pixels):" ) );

   sb_symbolHeight = us_spinbox( 0 );
   sb_symbolHeight->setRange( 1, 30 );
   sb_symbolHeight->setValue( selSymbol->size().height() );
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
   p.setColor( QPalette::Window, selSymbol->pen().color() );
   lb_showSymbolOutlineColor->setPalette( p );

   QPushButton* pb_symOutlineColor = us_pushbutton ( tr( "Update Color" ) );
   connect( pb_symOutlineColor, SIGNAL( clicked                 () ), 
                                SLOT  ( selectSymbolOutlineColor() ) );
   
   QLabel* lb_symbolInteriorColor = us_label( tr( "Interior Color:" ) );

   lb_showSymbolInteriorColor = us_label( "" , 1 );
   p = lb_showSymbolInteriorColor->palette();
   p.setColor( QPalette::Window, selSymbol->brush().color() );
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

   qDebug() << "Curve's SymbolStyle " << cmbb_symbolStyle->itemData( index ) << ", Type: " << cmbb_symbolStyle->itemData( index ).typeName();
   qDebug() << "Curve's SymbolStyle (int): index, style -- " << index << ", " << style; 
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
#if QT_VERSION > 0x050000
   QwtSymbol *oldSymbol  = (QwtSymbol*)firstSelectedCurve->symbol();
#else
   QwtSymbol *oldSymbol  = (QwtSymbol*)&firstSelectedCurve->symbol();
#endif

   oldSymbol             = ( oldSymbol != NULL ) ? oldSymbol : new QwtSymbol;
   QPen      symbolPen   = oldSymbol->pen();
   QBrush    symbolBrush = oldSymbol->brush();
   
   QPalette  palette     = lb_showSymbolOutlineColor->palette();
   symbolPen.setColor( palette.color( QPalette::Window ) );

   palette = lb_showSymbolInteriorColor->palette();
   symbolBrush.setColor( palette.color( QPalette::Window ) );

   QSize symbolSize( sb_symbolWidth->value(), sb_symbolHeight->value() );
   
   // QwtSymbol* newSymbol = new QwtSymbol( symbolStyle, symbolBrush,           //ALEXEY:: new Symbol object will be initialized inside the cycle !!!
   //                                       symbolPen,   symbolSize );

   palette = lb_showCurveColor->palette();
   QPen      curvePen( palette.color( QPalette::Window ) );
   curvePen.setWidth( sb_curveWidth->value() );

   
   QwtPlotItemList list = plot->itemList(); // All items

   //int j = 0;                                               //ALEXEY: that is NOT true: user can select curves in arbitrary order!!!!

   qDebug() << "Plot Curve Config: just before list() / selectedlist iteration:  ";
   // Iterate through the selected curves
   for ( int i = 0; i < selectedItems.size(); i++ )
   {
      // Remove Numbering from passed titles
      QString title = selectedItems[ i ];
      title.replace( QRegExp( "^\\(\\d+\\) " ), "" );

      qDebug() << "Plot Curve Config: for selectItem -- " << title;

      // There is no need to reiterate over the full list     //ALEXEY: that is NOT true: user can select curves in arbitrary order!!!!
      // This assumes the selected list and the full list
      // are in the same order

      //for ( /* no intitalizer */; j < list.size(); j++ )    //ALEXEY: that is NOT true: user can select curves in arbitrary order!!!!
      for ( int j = 0; j < list.size(); j++ )
      {
         if ( list[ j ]->rtti() != QwtPlotItem::Rtti_PlotCurve ) continue;
         if ( list[ j ]->title() == title )
         {
            QwtPlotCurve* curve = dynamic_cast< QwtPlotCurve* > ( list[ j ] );
            if ( selectedItems.size() == 1 ) 
               curve->setTitle( le_curveTitle->text() );

	    qDebug() << "Plot Curve Config: for allItems -- " << (list[ j ]->title()).text();
	    if ( curve->symbol() !=  NULL ) 
	      qDebug() << "Current Symbol -- " << curve->symbol()->style();

	    QwtSymbol* newSymbol = new QwtSymbol( symbolStyle, symbolBrush,             //ALEXEY: defining a new symbol must be witnin !!! caused crash before
						  symbolPen,   symbolSize );
	    
	    //if ( !symbolStyle || symbolStyle ==  QwtSymbol::NoSymbol )
	    if ( symbolStyle ==  QwtSymbol::NoSymbol )
	      {
		qDebug() << "No symbolStyle selected in the first place, setting to ::NoSymbol -- ";
		newSymbol = NULL;
	      }
	    
	    if ( newSymbol != NULL )
	      qDebug() << "newSymbol: pen, brush, size, style -- "
		       << newSymbol->pen()
		       << newSymbol->brush()
		       << newSymbol->size()
		       << newSymbol->style();
	    
	    curve->setSymbol( newSymbol );  //ALEXEY: this caused crash when plot->detachItems() from all apps (when ::NoSymbol style OR ...)
	    // Also, crashed when multiple curves selected && pressing "Apply" more than one time (even while changing the symbol type...)
	    // ATTN: crashed only when applied to the 2nd curve in a selected list! When only 1 curve selected, all good -- was wrong (heap) memory allocation!!
	    qDebug() << "Plot Curve Config: for allItems parms SET Symbol-- " << (list[ j ]->title()).text();
	    
	    curve->setPen   ( curvePen );
	    qDebug() << "Plot Curve Config: for allItems parms SET Pen-- " << (list[ j ]->title()).text();

	    curve->setStyle ( curveStyle );
	    qDebug() << "Plot Curve Config: for allItems parms SET Style-- " << (list[ j ]->title()).text();

	    qDebug() << "Plot Curve Config: for allItems parms SET ALL -- " << (list[ j ]->title()).text();
            break;
         }
      }

   }

   
   qDebug() << "Plot Curve Config: just before replot() ";
   plot->replot();
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

      s.drawSymbol( &p, p2 );
      s.drawSymbol( &p, p3 );
      s.drawSymbol( &p, p4 );
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
   QWidget* parent, Qt::WindowFlags flags ) : US_WidgetsDialog( parent, flags , false)//( false, parent, flags )
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

#if QT_VERSION > 0x050000
   le_scaleFrom = us_lineedit( 
         QString::number( plot->axisScaleDiv( axis ).lowerBound() ), 1 );
   
   le_scaleTo   = us_lineedit( 
         QString::number( plot->axisScaleDiv( axis ).upperBound() ), 1 );
#else
   le_scaleFrom = us_lineedit( 
         QString::number( plot->axisScaleDiv( axis )->lowerBound() ), 1 );
   
   le_scaleTo   = us_lineedit( 
         QString::number( plot->axisScaleDiv( axis )->upperBound() ), 1 );
#endif
   
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

#if QT_VERSION > 0x050000
#else
   if ( plot->axisScaleEngine( axis )->transformation()->type() == 
         QwtScaleTransformation::Log10 ) rb_log->setChecked( true );
#endif

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

   int attributes = (int)plot->axisScaleEngine( axis )->attributes();
   
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
#if QT_VERSION > 0x050000
#else
   int plotType = plot->axisScaleEngine( axis )->transformation()->type();
   if ( ( plotType == QwtScaleTransformation::Log10  && rb_linear->isChecked() ) 
     || ( plotType == QwtScaleTransformation::Linear && rb_log   ->isChecked() ) )
   {
      if ( rb_linear->isChecked() ) 
         plot->setAxisScaleEngine( axis, new QwtLinearScaleEngine );
      else  
         plot->setAxisScaleEngine( axis, new QwtLogScaleEngine );
   }
#endif

   // Set scale reference
   double reference = le_reference->text().toDouble();
   plot->axisScaleEngine( axis )->setReference( reference );

   int attributes = 0;
   if ( cb_refValue ->isChecked() ) attributes |= (int)QwtScaleEngine::IncludeReference; 
   if ( cb_symmetric->isChecked() ) attributes |= (int)QwtScaleEngine::Symmetric; 

   // Set other attributes

   if ( cb_floating->isChecked()  ) attributes |= (int)QwtScaleEngine::Floating; 
   if ( cb_inverted->isChecked()  ) attributes |= (int)QwtScaleEngine::Inverted;
   plot->axisScaleEngine( axis )->setAttributes( (QwtScaleEngine::Attribute)attributes );

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
   QWidget* parent, Qt::WindowFlags flags ) : US_WidgetsDialog( parent, flags , false)//( false, parent, flags )
{
   setWindowTitle( tr( "Grid Configuration" ) );
   setPalette( US_GuiSettings::frameColor() );
   
   plot = currentPlot;
   grid = NULL;
   int ngrid = 0;

   // Keep out of the way
   //move( pos() + QPoint( plot->rect().width(), 0 ) );

   // Get the grid - if it exists
   QwtPlotItemList list = plot->itemList();

   for ( int i = 0; i< list.size(); i++ )
   {
      if ( list[i]->rtti() == QwtPlotItem::Rtti_PlotGrid )
      {
         ngrid++;
         if ( ngrid == 1 )
            grid = dynamic_cast<QwtPlotGrid*>( list[ i ] );
         else
            list[ i ]->detach();
      }
   }
qDebug() << "GRID COUNT" << ngrid;

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

   QPen     gmajorPen = grid->majorPen();
   QPalette p;
   p.setColor( QPalette::Window, gmajorPen.color() );
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
   styles << tr( "Solid" ) << tr( "Dash" ) << tr( "Dot" ) << tr( "DashDot" )
          << tr( "DashDotDot" );

   QList< int > penStyle;
   penStyle << Qt::SolidLine << Qt::DashLine << Qt::DotLine << Qt::DashDotLine
            << Qt::DashDotDotLine;

   cmbb_majorStyle  = us_comboBox();
   int currentStyle = gmajorPen.style();
   
   for ( int i = 0; i < styles.size(); i++ )
   {
      cmbb_majorStyle->addItem( styles[ i ], penStyle[ i ] );

      if ( currentStyle == penStyle[ i ] )
         cmbb_majorStyle->setCurrentIndex( i );
   }

   majorStyle->addWidget( cmbb_majorStyle );

   QLabel* lb_majorWidth = us_label( tr( "Width (pixels):" ) );
   majorStyle->addWidget( lb_majorWidth );

   sb_majorWidth = us_spinbox( 0 );
   sb_majorWidth->setRange( 1, 5 );
   sb_majorWidth->setValue( gmajorPen.width() );

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

   QPen     gminorPen = grid->minorPen();
   p.setColor( QPalette::Window, gminorPen.color() );
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
   currentStyle    = gminorPen.style();

   for ( int i = 0; i < styles.size(); i++ )
   {
      cmbb_minorStyle->addItem( styles[ i ], penStyle[ i ] );
      if ( currentStyle == penStyle[ i ] )
         cmbb_minorStyle->setCurrentIndex( i );

   }

   minorStyle->addWidget( cmbb_minorStyle );

   QLabel* lb_minorWidth = us_label( tr( "Width (pixels):" ) );
   minorStyle->addWidget( lb_minorWidth );

   sb_minorWidth = us_spinbox( 0 );
   sb_minorWidth->setRange( 1, 5 ); 
   sb_minorWidth->setValue( gminorPen.width() );

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
   // Set the major pen
   QPen pen = grid->majorPen();

   pen.setColor( lb_showMajorColor->palette().color( QPalette::Window ) );
   pen.setWidth( sb_majorWidth->value() );
   
   int style;
   style = cmbb_majorStyle->itemData( cmbb_majorStyle->currentIndex() ).toInt();
   pen.setStyle( static_cast< Qt::PenStyle >( style ) );
   
   grid->setMajorPen( pen );

   // Set the minor pen
   pen = grid->minorPen();

   pen.setColor( lb_showMinorColor->palette().color( QPalette::Window ) );
   pen.setWidth( sb_minorWidth->value() );

   style = cmbb_minorStyle->itemData( cmbb_minorStyle->currentIndex() ).toInt();
   pen.setStyle( static_cast< Qt::PenStyle >( style ) );

   grid->setMinorPen( pen );

   // Enable/disable grid lines and re-plot
   grid->enableX   ( cb_enableX     ->isChecked() );   
   grid->enableY   ( cb_enableY     ->isChecked() );   
   grid->enableXMin( cb_enableXminor->isChecked() );   
   grid->enableYMin( cb_enableYminor->isChecked() );

   plot->replot();
}

/**************************    US_PlotPicker Class    *********************/

/*!  \brief Customize plot picker characteristics and mouse events
     \param plot  The plot to attach to
*/

US_PlotPicker::US_PlotPicker( QwtPlot* plot ) 
  : QwtPlotPicker( QwtPlot::xBottom, QwtPlot::yLeft, plot->canvas() )
{
#if QT_VERSION < 0x050000
   setSelectionFlags( QwtPicker::PointSelection );
#else
   setStateMachine ( new QwtPickerDragPointMachine() );
#endif
   setTrackerMode  ( QwtPicker::AlwaysOn );
   setRubberBand   ( QwtPicker::CrossRubberBand );

   QColor c = US_GuiSettings::plotPicker();
   setRubberBandPen( c );
   setTrackerPen   ( c );
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


