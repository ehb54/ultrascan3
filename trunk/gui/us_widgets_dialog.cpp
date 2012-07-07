//! \file us_widgets_dialog.cpp
#include "us_widgets_dialog.h"
#include "us_gui_settings.h"
#include "us_images.h"

US_WidgetsDialog::US_WidgetsDialog( QWidget* w, Qt::WindowFlags f ) 
   : QDialog( w, f )
{
  QApplication::setStyle( QStyleFactory::create( US_GuiSettings::guiStyle() ) );

  if ( ! g.isValid() )
  {
    // Do something for invalid global memory
    qDebug( "us_win: invalid global memory" );
  }

  vlgray = US_GuiSettings::editColor();
  vlgray.setColor( QPalette::Base, QColor( 0xe0, 0xe0, 0xe0 ) );

  QIcon us3_icon = US_Images::getIcon( US_Images::US3_ICON );
  setWindowIcon( us3_icon );
}

QLabel* US_WidgetsDialog::us_label( const QString& labelString, int fontAdjust, 
                              int weight )
{
  QLabel* newLabel = new QLabel( labelString, this );

//  newLabel->setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
  newLabel->setAlignment ( Qt::AlignVCenter | Qt::AlignLeft );
  newLabel->setMargin    ( 2 );
  newLabel->setAutoFillBackground( true );

  newLabel->setFont(
      QFont( US_GuiSettings::fontFamily(), 
             US_GuiSettings::fontSize() + fontAdjust, 
             weight ) );

  newLabel->setPalette( US_GuiSettings::labelColor() );

  return newLabel;
}

// textlabel ( defaults to smaller font and changes text colors )
QLabel* US_WidgetsDialog::us_textlabel( const QString& labelString, int fontAdjust, 
                                  int weight )
{
  QLabel* newLabel = us_label( labelString, fontAdjust, weight );

  newLabel->setPalette( US_GuiSettings::editColor() );

  return newLabel;
}

// banner ( defaults to Bold and changes text colors )
QLabel* US_WidgetsDialog::us_banner( const QString& labelString, int fontAdjust, 
                            int weight )
{
  QLabel* newLabel = us_label( labelString, fontAdjust, weight );

  newLabel->setAlignment ( Qt::AlignCenter );
  newLabel->setFrameStyle( QFrame::WinPanel | QFrame::Raised );
  newLabel->setMidLineWidth( 2 );

  // Set label colors
  newLabel->setPalette( US_GuiSettings::frameColor() );

  return newLabel;
}

// pushbutton
QPushButton* US_WidgetsDialog::us_pushbutton( const QString& labelString, bool enabled,
                                        int fontAdjust )
{
  QPushButton* button =  new QPushButton( tr( labelString.toAscii() ), this );

  button->setFont( QFont( US_GuiSettings::fontFamily(), 
                          US_GuiSettings::fontSize() + fontAdjust ) );

  button->setPalette( US_GuiSettings::pushbColor() );

  button->setAutoDefault( false );
  button->setEnabled( enabled );

  return button;
}

// textedit
QTextEdit* US_WidgetsDialog::us_textedit( void )
{
  QTextEdit* te = new QTextEdit( this );

  te->setFont          ( QFont( US_GuiSettings::fontFamily(), 
                                US_GuiSettings::fontSize() - 1 ) );
  
  te->setPalette       ( US_GuiSettings::normalColor() );
  te->setFrameStyle    ( QFrame::WinPanel | QFrame::Sunken );
  te->setAcceptRichText( true );
  te->setReadOnly      ( true );
  te->show();

  return te;
}

// lineedit
QLineEdit* US_WidgetsDialog::us_lineedit( const QString& text, int fontAdjust,
      bool readonly )
{
  QLineEdit* le = new QLineEdit( this );


  le->setFont    ( QFont( US_GuiSettings::fontFamily(), 
                          US_GuiSettings::fontSize() + fontAdjust ) );
  
  le->insert     ( text );
  le->setAutoFillBackground( true );
  us_setReadOnly ( le, readonly );
  le->show();

  return le;
}

// Set ReadOnly and corresponding color for us_lineedit
void US_WidgetsDialog::us_setReadOnly( QLineEdit* le, bool readonly )
{
  if ( readonly )
  {
     le->setPalette ( vlgray );
     le->setReadOnly( true );
  }
  else
  {
     le->setPalette ( US_GuiSettings::editColor() );
     le->setReadOnly( false );
  }
}

// Set ReadOnly and corresponding color for us_textedit
void US_WidgetsDialog::us_setReadOnly( QTextEdit* te, bool readonly )
{
  if ( readonly )
  {
     te->setPalette ( vlgray );
     te->setReadOnly( true );
  }
  else
  {
     te->setPalette ( US_GuiSettings::normalColor() );
     te->setReadOnly( false );
  }
}

// List Widget
QListWidget* US_WidgetsDialog::us_listwidget ( int fontAdjust )
{
  QListWidget* lw = new QListWidget;

  lw->setAutoFillBackground( true );
  lw->setPalette( US_GuiSettings::editColor() );
  lw->setFont   ( QFont( US_GuiSettings::fontFamily(), 
                         US_GuiSettings::fontSize() + fontAdjust ) );

  return lw;
}

// checkbox
QGridLayout* US_WidgetsDialog::us_checkbox( 
      const QString& text, QCheckBox*& cb, bool state )
{
  QPalette p    = US_GuiSettings::normalColor();
  QFont    font = QFont( US_GuiSettings::fontFamily(),
                         US_GuiSettings::fontSize  (),
                         QFont::Bold );

  QFontMetrics fm( font );
 
  QLabel* lb_spacer = new QLabel;
  lb_spacer->setFixedWidth        ( fm.width( "w" ) ); // Space as wide as a 'w'
  lb_spacer->setAutoFillBackground( true );
  lb_spacer->setPalette           ( p );
  
  cb = new QCheckBox( text.toAscii(), this );
  cb->setFont              ( font );
  cb->setPalette           ( p );
  cb->setAutoFillBackground( true );
  cb->setChecked           ( state );

  QGridLayout* layout = new QGridLayout;
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->setSpacing        ( 0 );

  layout->addWidget( lb_spacer, 0, 0 );
  layout->addWidget( cb       , 0, 1 );

  return layout;
}

// radiobutton
QGridLayout* US_WidgetsDialog::us_radiobutton( 
      const QString& text, QRadioButton*& rb, bool state )
{
  QPalette p    = US_GuiSettings::normalColor();
  QFont    font = QFont( US_GuiSettings::fontFamily(),
                         US_GuiSettings::fontSize  (),
                         QFont::Bold );

  QFontMetrics fm( font );

  QLabel* lb_spacer = new QLabel;
  lb_spacer->setFixedWidth        ( fm.width( "w" ) ); // Space as wide as a 'w'
  lb_spacer->setAutoFillBackground( true );
  lb_spacer->setPalette           ( p );

  rb = new QRadioButton( text.toAscii(), this );
  rb->setAutoFillBackground( true );
  rb->setFont              ( font );
  rb->setPalette           ( p );
  rb->setChecked           ( state );

  QGridLayout* layout = new QGridLayout;
  layout->setSpacing        ( 0 );
  layout->setContentsMargins( 0, 0, 0, 0 );

  layout->addWidget( lb_spacer, 0, 0 );
  layout->addWidget( rb       , 0, 1 );

  return layout;
}

// Progress Bar
QProgressBar* US_WidgetsDialog::us_progressBar( int low, int high, int value )
{
  QProgressBar* pb = new QProgressBar;

  pb->setRange( low, high );
  pb->setValue( value );

  pb->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
  pb->setPalette( US_GuiSettings::normalColor() );
  pb->setAutoFillBackground( true );

  pb->setFont( QFont( US_GuiSettings::fontFamily(),
                      US_GuiSettings::fontSize(),
                      QFont::Bold ) );

  return pb;
}

// Combo Box
QComboBox* US_WidgetsDialog::us_comboBox( void )
{
  QComboBox* cb = new QComboBox( this );

  cb->setPalette( US_GuiSettings::normalColor() );
  cb->setAutoFillBackground( true );
  cb->setFont( QFont( US_GuiSettings::fontFamily(), 
                      US_GuiSettings::fontSize() ) );

  return cb;
}

// LCD
QLCDNumber* US_WidgetsDialog::us_lcd( int digits, int value )
{
  QLCDNumber* lcd = new QLCDNumber( digits );

  lcd->setSegmentStyle( QLCDNumber::Filled );
  lcd->setMode        ( QLCDNumber::Dec );
  lcd->display        ( value );
  lcd->setAutoFillBackground( true );

  lcd->setPalette     ( US_GuiSettings::lcdColor() );

  return lcd;
}

//QwtCounter
QwtCounter* US_WidgetsDialog::us_counter( int buttons, double low, double high, 
                                    double value )
{
  QwtCounter* counter = new QwtCounter;
#ifdef Q_WS_MAC
  QList< QObject* > children = counter->children();
  QStyle *btnstyle = new QPlastiqueStyle();

  for ( int jj = 0; jj < children.size(); jj++ )
  {
     QWidget* cwidg = (QWidget*)children.at( jj );
     QString clname = cwidg->metaObject()->className();

     if ( !clname.isEmpty()  &&  clname.contains( "Button" ) )
     {
        cwidg->setStyle( btnstyle );
     }
  }
#endif
  QFont vfont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );
  QFontMetrics fm( vfont );
  counter->setNumButtons( buttons );
  counter->setRange     ( low, high );
  counter->setValue     ( value );
  counter->setPalette   ( US_GuiSettings::normalColor() );
  counter->setFont      ( vfont );
  counter->setAutoFillBackground( true );

  // Set width based on current value and high-value sizes
  int ncv    = int( log10( value ) ) + 1;
  int nch    = int( log10( high  ) ) + 1;
  ncv        = ( ncv > 0 ) ? ncv : ( 4 - ncv );
  nch        = ( nch > 0 ) ? nch : ( 4 - nch );
  nch        = qMax( nch, ncv );
  int widv   = fm.width( QString( "12345678901234" ).left( ncv ) );
  int widh   = fm.width( QString( "12345678901234" ).left( nch ) );
  counter->adjustSize();
  int mwidth = counter->width() + widh - widv;
  counter->resize         ( mwidth, counter->height() );
  counter->setMinimumWidth( mwidth - fm.width( "A" ) );

  return counter;
}

QwtPlot* US_WidgetsDialog::us_plot( const QString& title, const QString& x_axis,
                              const QString& y_axis )
{
  QwtPlot* plot = new QwtPlot;  
  plot->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
  plot->setAutoReplot( false );
  plot->setTitle     ( title );
 
  plot->setAxisTitle( QwtPlot::xBottom, x_axis );
  plot->setAxisTitle( QwtPlot::yLeft  , y_axis );

  plot->setAutoFillBackground( true );
  plot->setPalette ( US_GuiSettings::plotColor() );
  plot->setCanvasBackground( US_GuiSettings::plotCanvasBG() );

  return plot;
}

QwtPlotGrid* US_WidgetsDialog::us_grid( QwtPlot* plot )
{
  QwtPlotGrid* grid = new QwtPlotGrid;
  grid->enableXMin    ( true );
  grid->setMajPen(QPen( US_GuiSettings::plotMajGrid(), 0, Qt::DotLine ) );
  grid->setMinPen(QPen( US_GuiSettings::plotMinGrid(), 0, Qt::DotLine ) );
  grid->attach        ( plot );

  return grid;
}

QwtPlotCurve* US_WidgetsDialog::us_curve( QwtPlot* plot, const QString& title )
{
  QwtPlotCurve* curve = new QwtPlotCurve( title );
  //curve->setRenderHint( QwtPlotItem::RenderAntialiased );
  curve->setPen       ( QPen( US_GuiSettings::plotCurve() ) );
  curve->setYAxis     ( QwtPlot::yLeft );
  curve->attach       ( plot );

  return curve;
}

QwtPlotPicker* US_WidgetsDialog::us_picker( QwtPlot* plot )
{
  QwtPlotPicker* pick = new QwtPlotPicker( QwtPlot::xBottom, QwtPlot::yLeft,
                                           plot->canvas() ); 

  pick->setSelectionFlags( QwtPicker::PointSelection );
  pick->setTrackerMode   ( QwtPicker::AlwaysOn );
  pick->setRubberBand    ( QwtPicker::CrossRubberBand );

  QColor c = US_GuiSettings::plotPicker();
  pick->setRubberBandPen ( c );
  pick->setTrackerPen    ( c );

  return pick;
}

// tabWidget
QTabWidget* US_WidgetsDialog::us_tabwidget(  int fontAdjust,
                                             int weight )
{
  QTabWidget* newtw = new QTabWidget( this );

  newtw->setAutoFillBackground( true );

  newtw->setFont(
      QFont( US_GuiSettings::fontFamily(),
             US_GuiSettings::fontSize  () + fontAdjust,
             weight ) );

  newtw->setPalette( US_GuiSettings::normalColor() );

  return newtw;
}


