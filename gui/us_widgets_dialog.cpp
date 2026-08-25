//! \file us_widgets_dialog.cpp
#include "us_widgets_dialog.h"
#include "us_widgets.h"
#include "us_gui_settings.h"
#include "us_theme.h"
#include "us_images.h"


US_WidgetsDialog::US_WidgetsDialog( QWidget* w, Qt::WindowFlags f, bool set_style ) 
   : QDialog( w, f )
{
  if (set_style)
      US_Theme::apply();

  if ( ! g.isValid() )
  {
    // Do something for invalid global memory
    qDebug( "us_win: invalid global memory" );
  }

  vlgray = US_GuiSettings::readonlyColor();

  US_Theme::tag( this, US_Theme::Frame );

  QIcon us3_icon = US_Images::getIcon( US_Images::US3_ICON );
  setWindowIcon( us3_icon );
}

QLabel* US_WidgetsDialog::us_label( const QString& labelString, int fontAdjust, 
                              int weight )
{
  QLabel* newLabel = new QLabel( labelString, this );

  newLabel->setFrameStyle( QFrame::NoFrame );
  newLabel->setAlignment ( Qt::AlignVCenter | Qt::AlignLeft );
  newLabel->setMargin    ( 2 );
  newLabel->setAutoFillBackground( true );

  newLabel->setFont(
      QFont( US_GuiSettings::fontFamily(), 
             US_GuiSettings::fontSize() + fontAdjust, 
             weight ) );

  US_Theme::tag( newLabel, US_Theme::Label );

  return newLabel;
}

// textlabel ( defaults to smaller font and changes text colors )
QLabel* US_WidgetsDialog::us_textlabel( const QString& labelString, int fontAdjust, 
                                  int weight )
{
  QLabel* newLabel = us_label( labelString, fontAdjust, weight );

  US_Theme::tag( newLabel, US_Theme::Edit );

  return newLabel;
}

// banner ( defaults to Bold and changes text colors )
QLabel* US_WidgetsDialog::us_banner( const QString& labelString, int fontAdjust, 
                            int weight )
{
  QLabel* newLabel = us_label( labelString, fontAdjust, weight );

  newLabel->setAlignment ( Qt::AlignCenter );
  newLabel->setFrameStyle( QFrame::NoFrame );
  newLabel->setMargin    ( 5 );

  // Tags the label as a section header (hook for site specific style sheets)
  newLabel->setProperty( US_Theme::bannerProperty(), "banner" );

  // Set label colors
  US_Theme::tag( newLabel, US_Theme::Banner );

  return newLabel;
}

// pushbutton
QPushButton* US_WidgetsDialog::us_pushbutton( const QString& labelString, bool enabled,
                                        int fontAdjust )
{
  QPushButton* button =  new QPushButton( tr( labelString.toLatin1() ), this );

  button->setFont( QFont( US_GuiSettings::fontFamily(), 
                          US_GuiSettings::fontSize() + fontAdjust ) );

  US_Theme::tag( button, US_Theme::Pushbutton );

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
  
  US_Theme::tag( te, US_Theme::Normal );
  te->setFrameStyle    ( QFrame::StyledPanel | QFrame::Plain );
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
     US_Theme::tag( le, US_Theme::ReadOnly );
     le->setReadOnly( true );
  }
  else
  {
     US_Theme::tag( le, US_Theme::Edit );
     le->setReadOnly( false );
  }
}

// Set ReadOnly and corresponding color for us_textedit
void US_WidgetsDialog::us_setReadOnly( QTextEdit* te, bool readonly )
{
  if ( readonly )
  {
     US_Theme::tag( te, US_Theme::ReadOnly );
     te->setReadOnly( true );
  }
  else
  {
     US_Theme::tag( te, US_Theme::Normal );
     te->setReadOnly( false );
  }
}

// List Widget
QListWidget* US_WidgetsDialog::us_listwidget ( int fontAdjust )
{
  QListWidget* lw = new QListWidget;

  lw->setAutoFillBackground( true );
  US_Theme::tag( lw, US_Theme::Edit );
  lw->setFont   ( QFont( US_GuiSettings::fontFamily(), 
                         US_GuiSettings::fontSize() + fontAdjust ) );

  return lw;
}

// checkbox
QGridLayout* US_WidgetsDialog::us_checkbox( 
      const QString& text, QCheckBox*& cb, bool state )
{
  QFont    font = QFont( US_GuiSettings::fontFamily(),
                         US_GuiSettings::fontSize  (),
                         QFont::Bold );

  QFontMetrics fm( font );

  QLabel* lb_spacer = new QLabel;
  lb_spacer->setFixedWidth        ( fm.horizontalAdvance( "w" ) ); // Space as wide as a 'w'
  US_Theme::tag( lb_spacer, US_Theme::Normal );

  cb = new QCheckBox( text.toLatin1(), this );
  cb->setFont              ( font );
  US_Theme::tag( cb, US_Theme::Normal );
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
  QFont    font = QFont( US_GuiSettings::fontFamily(),
                         US_GuiSettings::fontSize  (),
                         QFont::Bold );

  QFontMetrics fm( font );

  QLabel* lb_spacer = new QLabel;
  lb_spacer->setFixedWidth        ( fm.horizontalAdvance( "w" ) ); // Space as wide as a 'w'
  US_Theme::tag( lb_spacer, US_Theme::Normal );

  rb = new QRadioButton( text.toLatin1(), this );
  rb->setFont              ( font );
  US_Theme::tag( rb, US_Theme::Normal );
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
  US_Theme::tag( pb, US_Theme::Normal );
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

  US_Theme::tag( cb, US_Theme::Normal );
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

  US_Theme::tag( lcd, US_Theme::Lcd );

  return lcd;
}

//QwtCounter
QwtCounter* US_WidgetsDialog::us_counter( int buttons, double low, double high, 
                                    double value )
{
  QwtCounter* counter = new QwtCounter;
  counter->setNumButtons( buttons );
  counter->setRange     ( low, high );
  counter->setValue     ( value );
  QList< QObject* > children = counter->children();
  int totwid          = 0;
#ifdef Q_OS_MAC
  QStyle *btnstyle = QApplication::setStyle( "fusion" );

  for ( int jj = 0; jj < children.size(); jj++ )
  {
     QWidget* cwidg = (QWidget*)children.at( jj );
     QString clname = cwidg->metaObject()->className();

     if ( !clname.isEmpty()  &&  clname.contains( "Button" ) )
     {
        cwidg->setStyle( btnstyle );
     }
  }
#endif    // END: special button treatment for Mac

  for ( int jj = 0; jj < children.size(); jj++ )
  {  // Accumulate total width of button widgets
     QWidget* cwidg = (QWidget*)children.at( jj );
     QString clname = cwidg->metaObject()->className();
     if ( clname.contains( "Button" ) )
     {
        cwidg->adjustSize();
        totwid        += cwidg->width();
     }
  }

  QFont vfont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );
  QFontMetrics fm( vfont );
  US_Theme::tag( counter, US_Theme::Normal );
  counter->setFont      ( vfont );
  counter->setAutoFillBackground( true );

  // Set min,curr width based on current value and high-value sizes
  int ncv    = int( log10( value ) ) + 1;
  ncv        = ( ncv > 0 ) ? ncv : ( 4 - ncv );
  int widv   = fm.horizontalAdvance( QString( "12345678901234" ).left( ncv ) );
  counter->adjustSize();
  int mwidth = widv * 2 + totwid;
  counter->setMinimumWidth( mwidth );
  counter->resize(          mwidth + widv, counter->height() );

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

  US_Widgets::us_style_plot( plot );

  return plot;
}

QwtPlotGrid* US_WidgetsDialog::us_grid( QwtPlot* plot )
{
  QwtPlotGrid* grid = new QwtPlotGrid;
  grid->enableXMin ( true );
  grid->setMajorPen( QPen( US_GuiSettings::plotMajGrid(), 0, Qt::DotLine ) );
  grid->setMinorPen( QPen( US_GuiSettings::plotMinGrid(), 0, Qt::DotLine ) );
  grid->attach     ( plot );

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

#if QT_VERSION > 0x050000
  pick->setStateMachine( new QwtPickerClickPointMachine() );
#else
  pick->setSelectionFlags( QwtPicker::PointSelection );
#endif
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

  US_Theme::tag( newtw, US_Theme::Normal );

  return newtw;
}

// TimeEdit
QHBoxLayout* US_WidgetsDialog::us_timeedit( 
      QTimeEdit*& tedt, const int fontAdjust, QSpinBox** sbox )
{
   QPalette   pal    = US_GuiSettings::normalColor();
   QFont      font   = QFont( US_GuiSettings::fontFamily(),
                              US_GuiSettings::fontSize  () + fontAdjust );
   tedt              = new QTimeEdit( QTime( 0, 0 ), this );
   US_Theme::tag( tedt, US_Theme::Normal );
   tedt->setAutoFillBackground( true );
   tedt->setFont( font );

   QHBoxLayout* layo = new QHBoxLayout;
   layo->setContentsMargins( 0, 0, 0, 0 );
   layo->setSpacing        ( 0 );

   if ( sbox != NULL )
   {
      *sbox             = new QSpinBox( this );
      US_Theme::tag( *sbox, US_Theme::Normal );
      (*sbox)->setAutoFillBackground( true );
      (*sbox)->setFont( font );

      layo->addWidget( *sbox );
   }

   layo->addWidget( tedt );

   return layo;
}

// day-hh-mm-ss layout
QHBoxLayout* US_WidgetsDialog::us_ddhhmmsslay( 
					      const int fontAdjust,  const int dd_hide, const int hh_hide, const int mm_hide, const int ss_hide, QSpinBox** dd, QSpinBox** hh, QSpinBox** mm, QSpinBox** ss)
{
   QPalette   pal    = US_GuiSettings::normalColor();
   QFont      font   = QFont( US_GuiSettings::fontFamily(),
                              US_GuiSettings::fontSize  () + fontAdjust );

   QHBoxLayout* layout = new QHBoxLayout;
   layout->setContentsMargins( 0, 0, 0, 0 );
   layout->setSpacing        ( 0 );
   
   if ( dd != NULL )
   {
      *dd              = new QSpinBox( this );
      (*dd)->setRange(0, 20);
      US_Theme::tag( *dd, US_Theme::Normal );
      (*dd)->setAutoFillBackground( true );
      (*dd)->setFont( font );
      QLabel*  lb_d   = us_label( tr( "D:" ) );
      layout->addWidget( lb_d );
      
      layout->addWidget( *dd );

      //ALEXEY do not show days
      if (dd_hide)
	{
	  lb_d->hide();       
	  (*dd)->hide();
	}
   }
   
   if ( hh != NULL )
   {
      *hh              = new QSpinBox( this );
      (*hh)->setRange(0, 23);
      US_Theme::tag( *hh, US_Theme::Normal );
      (*hh)->setAutoFillBackground( true );
      (*hh)->setFont( font );
      QLabel*  lb_h   = us_label( tr( "H:" ) );
      layout->addWidget( lb_h );
      
      layout->addWidget( *hh );

      //ALEXEY do not show hh
      if (hh_hide)
	{
	  lb_h->hide();
	  (*hh)->hide();
	}  
   }   
   
   if ( mm != NULL )
   {
      *mm              = new QSpinBox( this );
      (*mm)->setRange(0, 59);
      US_Theme::tag( *mm, US_Theme::Normal );
      (*mm)->setAutoFillBackground( true );
      (*mm)->setFont( font );
      QLabel*  lb_m   = us_label( tr( "M:" ) );
      layout->addWidget( lb_m );
      
      layout->addWidget( *mm );

      //ALEXEY do not show mins
      if (mm_hide)
	{
	  lb_m->hide();
	  (*mm)->hide();
	}   
   }
   
   if ( ss != NULL )
   {
      *ss              = new QSpinBox( this );
      (*ss)->setRange(0, 59);
      US_Theme::tag( *ss, US_Theme::Normal );
      (*ss)->setAutoFillBackground( true );
      (*ss)->setFont( font );
      QLabel*  lb_s   = us_label( tr( "S:" ) );
      layout->addWidget( lb_s );

      layout->addWidget( *ss );

      //ALEXEY do not show seconds
      if (ss_hide)
	{
	  lb_s->hide();
	  (*ss)->hide();
	}   
   }
   return layout;
}

// SpinBox
QSpinBox* US_WidgetsDialog::us_spinbox( const int fontAdjust )
{
   QSpinBox* sbox   = new QSpinBox( this );
   US_Theme::tag( sbox, US_Theme::Normal );
   sbox->setAutoFillBackground( true );
   sbox->setFont( QFont( US_GuiSettings::fontFamily(),
                         US_GuiSettings::fontSize() + fontAdjust ) );

   return sbox;
}

