//! \file us_widgets.cpp
#include "us_widgets.h"
#include "us_gui_settings.h"

// label
QLabel* US_Widgets::us_label( const QString& labelString, int fontAdjust, 
                              int weight )
{
  QLabel* newLabel = new QLabel( labelString, this );

  newLabel->setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
  newLabel->setAlignment ( Qt::AlignVCenter | Qt::AlignLeft );
  newLabel->setMargin    ( 5 );
  newLabel->setAutoFillBackground( true );

  newLabel->setFont(
      QFont( US_GuiSettings::fontFamily(), 
             US_GuiSettings::fontSize() + fontAdjust, 
             weight ) );

  newLabel->setPalette( US_GuiSettings::labelColor() );

  return newLabel;
}

// textlabel ( defaults to smaller font and changes text colors )
QLabel* US_Widgets::us_textlabel( const QString& labelString, int fontAdjust, 
                                  int weight )
{
  QLabel* newLabel = us_label( labelString, fontAdjust, weight );

  newLabel->setPalette( US_GuiSettings::editColor() );

  return newLabel;
}

// banner ( defaults to Bold and changes text colors )
QLabel* US_Widgets::us_banner( const QString& labelString, int fontAdjust, 
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
QPushButton* US_Widgets::us_pushbutton( const QString& labelString, bool enabled,
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
QTextEdit* US_Widgets::us_textedit( void )
{
  QTextEdit* te = new QTextEdit( this );

  te->setFont          ( QFont( US_GuiSettings::fontFamily(), 
                                US_GuiSettings::fontSize() - 1 ) );
  
  te->setPalette       ( US_GuiSettings::normalColor() );
  te->setFrameStyle    ( WinPanel | Sunken );
  te->setAcceptRichText( true );
  te->setReadOnly      ( true );
  te->show();

  return te;
}

// lineedit
QLineEdit* US_Widgets::us_lineedit( const QString& text, int fontAdjust )
{
  QLineEdit* le = new QLineEdit( this );


  le->setFont    ( QFont( US_GuiSettings::fontFamily(), 
                          US_GuiSettings::fontSize() + fontAdjust ) );
  
  le->insert     ( text );
  le->setAutoFillBackground( true );
  le->setPalette ( US_GuiSettings::editColor() );
  le->setReadOnly( false );
  le->show();

  return le;
}

// List Widget
QListWidget* US_Widgets::us_listwidget ( int fontAdjust )
{
  QListWidget* lw = new QListWidget;

  lw->setAutoFillBackground( true );
  lw->setPalette( US_GuiSettings::editColor() );
  lw->setFont   ( QFont( US_GuiSettings::fontFamily(), 
                         US_GuiSettings::fontSize() + fontAdjust ) );

  return lw;
}

// checkbox
QCheckBox* US_Widgets::us_checkbox( const QString& text, Qt::CheckState state )
{
  QCheckBox* cb = new QCheckBox( text.toAscii(), this );
  
  cb->setFont( QFont ( US_GuiSettings::fontFamily(), 
                       US_GuiSettings::fontSize(),
                       QFont::Bold ) );
  
  cb->setPalette( US_GuiSettings::normalColor() );

  cb->setCheckState( state );
  return cb;
}

// radiobutton
QRadioButton* US_Widgets::us_radiobutton( const QString& text, Qt::CheckState state )
{
  QRadioButton* rb = new QRadioButton( text.toAscii(), this );
  
  rb->setFont( QFont ( US_GuiSettings::fontFamily(), 
                       US_GuiSettings::fontSize(),
                       QFont::Bold ) );
  
  rb->setPalette( US_GuiSettings::editColor() );

  rb->setChecked( state );
  return rb;
}

// Progress Bar
QProgressBar* US_Widgets::us_progressBar( int low, int high, int value )
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
QComboBox* US_Widgets::us_comboBox( void )
{
  QComboBox* cb = new QComboBox;

  cb->setPalette( US_GuiSettings::normalColor() );
  cb->setAutoFillBackground( true );
  cb->setFont( QFont( US_GuiSettings::fontFamily(), 
                      US_GuiSettings::fontSize() ) );

  return cb;
}

// LCD
QLCDNumber* US_Widgets::us_lcd( int digits, int value )
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
QwtCounter* US_Widgets::us_counter( int buttons, double low, double high, 
                                    double value )
{
  QwtCounter* counter = new QwtCounter;
  counter->setNumButtons( buttons );
  counter->setRange     ( low, high );
  counter->setValue     ( value );
  counter->setPalette   ( US_GuiSettings::normalColor() );
  counter->setAutoFillBackground( true );

  return counter;
}

QwtPlot* US_Widgets::us_plot( const QString& title, const QString& x_axis,
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

QwtPlotGrid* US_Widgets::us_grid( QwtPlot* plot )
{
  QwtPlotGrid* grid = new QwtPlotGrid;
  grid->enableXMin    ( true );
  grid->setMajPen(QPen( US_GuiSettings::plotMajGrid(), 0, Qt::DotLine ) );
  grid->setMinPen(QPen( US_GuiSettings::plotMinGrid(), 0, Qt::DotLine ) );
  grid->attach        ( plot );

  return grid;
}

QwtPlotCurve* US_Widgets::us_curve( QwtPlot* plot )
{
  QwtPlotCurve* curve = new QwtPlotCurve;
  curve->setRenderHint( QwtPlotItem::RenderAntialiased );
  curve->setPen       ( QPen( US_GuiSettings::plotCurve() ) );
  curve->setYAxis     ( QwtPlot::yLeft );
  curve->attach       ( plot );

  return curve;
}

QwtPlotPicker* US_Widgets::us_picker( QwtPlot* plot )
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

