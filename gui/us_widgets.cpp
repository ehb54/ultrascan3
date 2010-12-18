//! \file us_widgets.cpp
#include "us_widgets.h"
#include "us_gui_settings.h"
#include "us_settings.h"

US_Widgets::US_Widgets( bool set_position, QWidget* w, Qt::WindowFlags f ) : QFrame( w, f )
{
  QApplication::setStyle( QStyleFactory::create( US_GuiSettings::guiStyle() ) );

  if ( ! g.isValid() )
  {
    // Do something for invalid global memory
   qDebug( "us_win: invalid global memory" );
  }

  if ( set_position )
  {
    QPoint p = g.global_position();
    g.set_global_position( p + QPoint( 30, 30 ) );
    move( p );
  }
}

US_Widgets::~US_Widgets()
{
  QPoint p = g.global_position();
  g.set_global_position( p - QPoint( 30, 30 ) );
}

// label
QLabel* US_Widgets::us_label( const QString& labelString, int fontAdjust, 
                              int weight )
{
  QLabel* newLabel = new QLabel( labelString, this );

  newLabel->setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
  newLabel->setAlignment ( Qt::AlignVCenter | Qt::AlignLeft );
  newLabel->setMargin    ( 2 );
  newLabel->setAutoFillBackground( true );

  newLabel->setFont(
      QFont( US_GuiSettings::fontFamily(), 
             US_GuiSettings::fontSize  () + fontAdjust, 
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
                          US_GuiSettings::fontSize  () + fontAdjust ) );

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
                                US_GuiSettings::fontSize  () - 1 ) );
  
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
                          US_GuiSettings::fontSize  () + fontAdjust ) );
  
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
                         US_GuiSettings::fontSize  () + fontAdjust ) );

  return lw;
}

// checkbox
QGridLayout* US_Widgets::us_checkbox( 
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
  cb->setFont              ( font  ); 
  cb->setPalette           ( p     );
  cb->setChecked           ( state );
  cb->setAutoFillBackground( true  );

  QGridLayout* layout = new QGridLayout;
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->setSpacing        ( 0 );

  layout->addWidget( lb_spacer, 0, 0 );
  layout->addWidget( cb       , 0, 1 );

  return layout;
}

// radiobutton
QGridLayout* US_Widgets::us_radiobutton( 
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
  rb->setAutoFillBackground( true  );
  rb->setFont              ( font  );
  rb->setPalette           ( p     );
  rb->setChecked           ( state );

  QGridLayout* layout = new QGridLayout;
  layout->setSpacing        ( 0 );
  layout->setContentsMargins( 0, 0, 0, 0 );

  layout->addWidget( lb_spacer, 0, 0 );
  layout->addWidget( rb       , 0, 1 );

  return layout;
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
                      US_GuiSettings::fontSize  (),
                      QFont::Bold ) );

  return pb;
}

// Combo Box
QComboBox* US_Widgets::us_comboBox( void )
{
  QComboBox* cb = new QComboBox( this );

  cb->setPalette( US_GuiSettings::normalColor() );
  cb->setAutoFillBackground( true );
  cb->setFont( QFont( US_GuiSettings::fontFamily(), 
                      US_GuiSettings::fontSize  () ) );

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
  counter->setNumButtons( buttons );
  counter->setRange     ( low, high );
  counter->setValue     ( value );
  counter->setPalette   ( US_GuiSettings::normalColor() );
  counter->setFont      ( QFont( US_GuiSettings::fontFamily(), 
                                 US_GuiSettings::fontSize  () ) );
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

QwtPlotCurve* US_Widgets::us_curve( QwtPlot* plot, const QString& title )
{
  QwtPlotCurve* curve = new QwtPlotCurve( title );
  //curve->setRenderHint( QwtPlotItem::RenderAntialiased );
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

void US_ListWidget::mousePressEvent( QMouseEvent* event )
{
   if ( event->button() == Qt::RightButton )
   {
      emit rightClick();
      event->ignore();
      return;
   }

   QListWidget::mousePressEvent( event );
}


// find this system's best fixedPitch font
QFont US_Widgets::fixedFont()
{
   //QFontDataBase database;
   int           fsize  =  US_GuiSettings::fontSize();
   QFont         ffont( "monospace", fsize );
   QFont         tfont( "monospace", fsize );
   QFontInfo     finfo( tfont );
   QString       family;
   bool          fmatch;
   bool          ffixed;
   const char*   preffam[] = {
      "Liberation Mono",
      "FreeMono",
      "DejaVu Sans Mono",
      "DejaVu LGC San Mono",
      "Monaco",
      "Andale Mono",
      "Nimbus Mono L",
      "Luxi Mono",
      "QuickType mono",
      "Courier New",
      "Courier 10 Pitch",
      "Courier"
   };
   const int     pfsize = sizeof( preffam ) / sizeof( preffam[ 0 ] );

   for ( int ii = 0; ii < pfsize; ii++ )
   {
      family   = QString( preffam[ ii ] );
      tfont    = QFont( family );
      finfo    = QFontInfo( tfont );
      fmatch   = finfo.exactMatch();
      ffixed   = finfo.fixedPitch();
      if ( fmatch  &&  ffixed )
      {
         ffont    = tfont;

         if ( family.contains( "New" )  ||
              family.contains( "FreeM" ) )
            ffont    = QFont( family, fsize, QFont::DemiBold );

         break;
      }
   }
   return ffont;
}

// tabWidget
QTabWidget* US_Widgets::us_tabwidget(  int fontAdjust,
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

//////////////////  New class

US_Disk_DB_Controls::US_Disk_DB_Controls( int state )
{
   US_Widgets*   w     = new US_Widgets;
   QButtonGroup* group = new QButtonGroup;

   QGridLayout* db_layout   = w->us_radiobutton( tr( "Database" ),   rb_db );
   QGridLayout* disk_layout = w->us_radiobutton( tr( "Local Disk" ), rb_disk );

   group->addButton( rb_db );
   group->addButton( rb_disk );

   if ( state == Default ) state = US_Settings::default_data_location();

   ( state == Disk ) ? rb_disk->setChecked( true ) : rb_db->setChecked( true );

   setSpacing        ( 0 );
   setContentsMargins( 0, 0, 0, 0 );

   addLayout( db_layout );
   addLayout( disk_layout );

   connect( rb_db, SIGNAL( toggled( bool ) ), SLOT( rb_changed( bool ) ) );
}

bool US_Disk_DB_Controls::db( void )
{
   return rb_db->isChecked();
}

void US_Disk_DB_Controls::rb_changed( bool /* state */ )
{
   emit changed();;
}


