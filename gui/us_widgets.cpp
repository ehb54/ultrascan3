//! \file us_widgets.cpp
#include <QtSvg> 

#include "qwt_plot_canvas.h"

#include "us_widgets.h"
#include "us_gui_settings.h"
#include "us_theme.h"
#include "us_gui_util.h"
#include "us_settings.h"
#include "us_images.h"
#include "us_util.h"

US_Widgets::US_Widgets( bool set_position, QWidget* w, Qt::WindowFlags f ) : QFrame( w, f )
{
  // Install the UltraScan look (style, palette, font and style sheet).  This
  // is a no-op once it has been done for the current configuration.
  US_Theme::apply();

  if ( ! g.isValid() )
  {
    // Do something for invalid global memory
   qDebug( "us_win: invalid global memory" );
  }
  QPoint poit = g.global_position();
#ifndef Q_OS_WIN
  QString auto_positioning = US_Settings::debug_value("auto_positioning");
  if ( !auto_positioning.isEmpty() && auto_positioning.toLower() == "true" )
  {
     global_positioning = set_position;
  }
  else
  {
     global_positioning = false;
  }
  if ( global_positioning && !auto_positioning.isEmpty() && auto_positioning.toLower() == "true" )
  {
    QPoint p = g.global_position();
    g.set_global_position( p + QPoint( 30, 30 ) );
    move( p );
  }
#endif

  vlgray = US_GuiSettings::readonlyColor();

  US_Theme::tag( this, US_Theme::Frame );

  QIcon us3_icon = US_Images::getIcon( US_Images::US3_ICON );
  setWindowIcon( us3_icon );
}

US_Widgets::~US_Widgets()
{
   QPoint poit = g.global_position();
#ifndef Q_OS_WIN
   QString auto_positioning = US_Settings::debug_value("auto_positioning");
   if ( global_positioning && !auto_positioning.isEmpty() && auto_positioning.toLower() == "true" )
   {
      QPoint p = g.global_position();
      g.set_global_position( p - QPoint( 30, 30 ) );
   }
#endif
}

// label
QLabel* US_Widgets::us_label( const QString& labelString, int fontAdjust, 
                              int weight )
{
  QLabel* newLabel = new QLabel( labelString, this );

  newLabel->setFrameStyle( QFrame::NoFrame );
  newLabel->setAlignment ( Qt::AlignVCenter | Qt::AlignLeft );
  newLabel->setMargin    ( 2 );
  newLabel->setAutoFillBackground( true );

  newLabel->setFont(
      QFont( US_GuiSettings::fontFamily(), 
             US_GuiSettings::fontSize  () + fontAdjust, 
             weight ) );

  US_Theme::tag( newLabel, US_Theme::Label );

  return newLabel;
}

// textlabel ( defaults to smaller font and changes text colors )
QLabel* US_Widgets::us_textlabel( const QString& labelString, int fontAdjust, 
                                  int weight )
{
  QLabel* newLabel = us_label( labelString, fontAdjust, weight );

  US_Theme::tag( newLabel, US_Theme::Edit );

  return newLabel;
}

// banner ( defaults to Bold and changes text colors )
QLabel* US_Widgets::us_banner( const QString& labelString, int fontAdjust, 
                            int weight )
{
  QLabel* newLabel = us_label( labelString, fontAdjust, weight );

  newLabel->setAlignment ( Qt::AlignCenter );
  newLabel->setFrameStyle( QFrame::NoFrame );
  newLabel->setMargin    ( 5 );

  // Tags the label as a section header.  US_Theme leaves the banner colors
  // to the palette below so that they stay user configurable; the property
  // is a hook for site specific style sheets.
  newLabel->setProperty( US_Theme::bannerProperty(), "banner" );

  // Set label colors
  US_Theme::tag( newLabel, US_Theme::Banner );

  return newLabel;
}

// pushbutton
QPushButton* US_Widgets::us_pushbutton( const QString& labelString, bool enabled,
                                        int fontAdjust )
{
  QPushButton* button =  new QPushButton( tr( labelString.toLatin1() ), this );

  button->setFont( QFont( US_GuiSettings::fontFamily(), 
                          US_GuiSettings::fontSize  () + fontAdjust ) );

  US_Theme::tag( button, US_Theme::Pushbutton );

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
  
  US_Theme::tag( te, US_Theme::Normal );
  te->setFrameStyle    ( StyledPanel | Plain );
  te->setAcceptRichText( true );
  te->setReadOnly      ( true );
  te->show();

  return te;
}

// lineedit
QLineEdit* US_Widgets::us_lineedit( const QString& text, int fontAdjust,
      bool readonly )
{
  QLineEdit* le = new QLineEdit( this );


  le->setFont    ( QFont( US_GuiSettings::fontFamily(), 
                          US_GuiSettings::fontSize  () + fontAdjust ) );
  
  le->insert     ( text );
  le->setAutoFillBackground( true );
  us_setReadOnly ( le, readonly );
  le->show();

  return le;
}

// Set read-only flag and associated color palette for a line edit
void US_Widgets::us_setReadOnly( QLineEdit* le, bool readonly )
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

// Set read-only flag and associated color palette for a text edit
void US_Widgets::us_setReadOnly( QTextEdit* te, bool readonly )
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
QListWidget* US_Widgets::us_listwidget ( int fontAdjust )
{
  QListWidget* lw = new QListWidget;

  lw->setAutoFillBackground( true );
  US_Theme::tag( lw, US_Theme::Edit );
  lw->setFont   ( QFont( US_GuiSettings::fontFamily(), 
                         US_GuiSettings::fontSize  () + fontAdjust ) );

  return lw;
}

// checkbox
QGridLayout* US_Widgets::us_checkbox( 
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
  cb->setFont              ( font  );
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
QGridLayout* US_Widgets::us_radiobutton( 
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
  rb->setFont              ( font  );
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
QProgressBar* US_Widgets::us_progressBar( int low, int high, int value )
{
  QProgressBar* pb = new QProgressBar;

  pb->setRange( low, high );
  pb->setValue( value );

  pb->setAlignment( Qt::AlignCenter );
  US_Theme::tag( pb, US_Theme::Normal );
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

  US_Theme::tag( cb, US_Theme::Normal );
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

  US_Theme::tag( lcd, US_Theme::Lcd );

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
  QList< QObject* > children = counter->children();
  int totwid          = 0;
#ifdef Q_OS_MAC
  // The counter's up/down buttons are unusably small with the native macOS
  // style.  Give just those buttons a Fusion style - unlike the former
  // QApplication::setStyle() call this leaves the style the user selected
  // for the rest of the application alone.
  static QStyle* btnstyle = QStyleFactory::create( "Fusion" );

  if ( btnstyle != nullptr )
  {
     for ( int jj = 0; jj < children.size(); jj++ )
     {
        QWidget* cwidg = (QWidget*)children.at( jj );
        QString clname = cwidg->metaObject()->className();

        if ( !clname.isEmpty()  &&  clname.contains( "Button" ) )
        {
           cwidg->setStyle( btnstyle );
        }
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

// Apply the theme to a plot.  Called for every plot UltraScan creates, so
// that plots built outside the factory below look the same.
void US_Widgets::us_style_plot( QwtPlot* plot )
{
  if ( plot == nullptr )
    return;

  plot->setAutoFillBackground( true );
  US_Theme::tag( plot, US_Theme::PlotFrame );
  plot->setCanvasBackground( US_GuiSettings::plotCanvasBG() );

  QwtPlotCanvas* canvas = qobject_cast< QwtPlotCanvas* >( plot->canvas() );

  if ( canvas != NULL )
  {
    // Qwt defaults the canvas to a 3D sunken panel.  A flat, rounded canvas
    // matches the rest of the UltraScan widgets.  Note that the canvas
    // background lives in the canvas palette's Window role, so the palette
    // must not be replaced here - it would undo setCanvasBackground().
    canvas->setFrameStyle  ( QFrame::NoFrame );
    canvas->setBorderRadius( US_Theme::radius() );
  }
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

  us_style_plot( plot );

  return plot;
}

QwtPlotGrid* US_Widgets::us_grid( QwtPlot* plot )
{
  QwtPlotGrid* grid = new QwtPlotGrid;
  grid->enableXMin ( true );
  grid->setMajorPen( QPen( US_GuiSettings::plotMajGrid(), 0, Qt::DotLine ) );
  grid->setMinorPen( QPen( US_GuiSettings::plotMinGrid(), 0, Qt::DotLine ) );
  grid->attach     ( plot );

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

  pick->setStateMachine( new QwtPickerClickPointMachine() );
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
      "DejaVu Sans Mono",
      "Nimbus Mono L",
      "Liberation Mono",
      "FreeMono",
      "DejaVu LGC Sans Mono",
      "Andale Mono",
      "Menlo", 
      "Luxi Mono",
      "Lucida Console",
      "Fixedsys",
      "Terminal",
      "QuickType mono",
      "Monaco",
      "Courier New",
      "Courier 10 Pitch",
      "Courier",
      "Monospace"
   };
   const int     pfsize = sizeof( preffam ) / sizeof( preffam[ 0 ] );

   for ( int ii = 0; ii < pfsize; ii++ )
   {
      family   = QString( preffam[ ii ] );
      tfont    = QFont( family );
      finfo    = QFontInfo( tfont );
      fmatch   = finfo.exactMatch();
      ffixed   = finfo.fixedPitch();
qDebug() << "fixf:   ii" << ii << "family" << family
 << "fmatch" << fmatch << "ffixed" << ffixed;
      if ( fmatch  &&  ffixed )
      {
         ffont    = tfont;

         if ( family.contains( "New" )  ||
              family.contains( "FreeM" ) )
            ffont    = QFont( family, fsize, QFont::DemiBold );

         break;
      }
   }
qDebug() << "fixf:   ffont.family()" << ffont.family();
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

  US_Theme::tag( newtw, US_Theme::Normal );

  return newtw;
}

// TimeEdit
QHBoxLayout* US_Widgets::us_timeedit( 
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

// day-hh-mm-ss box
QHBoxLayout* US_Widgets::us_ddhhmmss( 
	     const int fontAdjust, QSpinBox** dd, QSpinBox** hh, QSpinBox** mm, QSpinBox** ss)
{

   QPalette   pal    = US_GuiSettings::normalColor();
   QFont      font   = QFont( US_GuiSettings::fontFamily(),
                              US_GuiSettings::fontSize  () + fontAdjust );

   QHBoxLayout* layo = new QHBoxLayout;
   layo->setContentsMargins( 0, 0, 0, 0 );
   layo->setSpacing        ( 0 );
   
   if ( dd != NULL )
   {
      *dd              = new QSpinBox( this );
      US_Theme::tag( *dd, US_Theme::Normal );
      (*dd)->setAutoFillBackground( true );
      (*dd)->setFont( font );

      layo->addWidget( *dd );
   }
   
   if ( hh != NULL )
   {
      *hh              = new QSpinBox( this );
      US_Theme::tag( *hh, US_Theme::Normal );
      (*hh)->setAutoFillBackground( true );
      (*hh)->setFont( font );

      layo->addWidget( *hh );
   }   
   
   if ( mm != NULL )
   {
      *mm              = new QSpinBox( this );
      US_Theme::tag( *mm, US_Theme::Normal );
      (*mm)->setAutoFillBackground( true );
      (*mm)->setFont( font );

      layo->addWidget( *mm );
   }
   
   if ( ss != NULL )
   {
      *ss              = new QSpinBox( this );
      US_Theme::tag( *ss, US_Theme::Normal );
      (*ss)->setAutoFillBackground( true );
      (*ss)->setFont( font );

      layo->addWidget( *ss );
   }   

   return layo;
}

// day-hh-mm-ss layout
QHBoxLayout* US_Widgets::us_ddhhmmsslay( 
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
QSpinBox* US_Widgets::us_spinbox( const int fontAdjust )
{
   QSpinBox* sbox   = new QSpinBox( this );
   US_Theme::tag( sbox, US_Theme::Normal );
   sbox->setAutoFillBackground( true );
   sbox->setFont( QFont( US_GuiSettings::fontFamily(),
                         US_GuiSettings::fontSize() + fontAdjust ) );

   return sbox;
}

void US_Widgets::write_plot( const QString& fname, const QwtPlot* plot )
{
   US_GuiUtil::save_plot( fname, plot );
}

// Clean up install and work ./etc directories
int US_Widgets::clean_etc_dir( bool report )
{
   int nfmove         = 0;
   int nfcopy         = 0;
   int nfdele         = 0;
   QString ietc_dname = US_Settings::appBaseDir() + "/etc";  // Install etc
   QString wetc_dname = US_Settings::etcDir();               // Work etc
   QString list_fname = "etc_belongs_list.txt";  // Files that belong in etc
   QDir().mkpath( wetc_dname );   // Make sure work etc directory exists
   QDir ietc_dir( ietc_dname );
   QDir wetc_dir( wetc_dname );
   ietc_dname        += "/";
   wetc_dname        += "/";
   QDir::Filters ffilt = QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot;
   QList< QFileInfo > ie_files = ietc_dir.entryInfoList( ffilt );
   QList< QFileInfo > we_files = wetc_dir.entryInfoList( ffilt );
   QStringList keep_files;        // Files to keep in install etc
   QStringList copy_files;        // Files to copy to work etc
   QStringList link_files;        // Symbolic links in install etc
   QStringList dir_names;         // Subdirectories in install etc
   QStringList ietc_files;        // All files in install etc
   QStringList wetc_files;        // All files initially in work etc
   int niefs = ie_files.size();   // Count of install etc files
   int nwefs = we_files.size();   // Count of work etc files

   for ( int ii = 0; ii < niefs; ii++ )     // Build list of install etc files
      ietc_files << ie_files[ ii ].fileName();

   for ( int ii = 0; ii < nwefs; ii++ )     // Build list of work etc files
      wetc_files << we_files[ ii ].fileName();

   QFile lfile( ietc_dname + list_fname );  // Belong in install etc

   if ( lfile.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
      bool keep_file = false;
      bool copy_file = false;
      bool link_file = false;
      bool dir_file  = false;

      QTextStream ts( &lfile );
      while( ! ts.atEnd() )
      {  // Read the list of files that belong in install etc
         QString fline = ts.readLine();

         if ( fline.startsWith( "#" ) )
         {  // Handle comment line
            keep_file = copy_file = link_file = dir_file = false;
            if ( fline.contains( "List of files" ) )
               keep_file = true;            // Keep files follow
            else if ( fline.contains( "List of directories" ) )
               dir_file  = true;            // Subdirectories follow
            else if ( fline.contains( "COPY" ) )
               copy_file = true;            // Copy files follow
            else if ( fline.contains( "LINK" ) )
               link_file = true;            // Symbolic links follow
         }

         else
         {  // Actual file name:  move it to appropriate list
            QString filename = fline.section( " ", 0, 0 ).simplified();
            if ( keep_file )
               keep_files << filename;
            else if ( copy_file )
               copy_files << filename;
            else if ( link_file )
               link_files << filename.replace( "@", "" );
            else if ( dir_file )
               dir_names << filename.replace( "/", "" );
         }

      }
   }

   int nkeepf   = keep_files.size();
   int ncopyf   = copy_files.size();
   int nlinkf   = link_files.size();
   int nsdir    = dir_names .size();
qDebug() << "niefs nwefs" << niefs << nwefs << "nkeep/copy/link/dirf"
 << nkeepf << ncopyf << nlinkf << nsdir;

   // Examine each file in ie_files (e.g.,"*/ultrascan3/etc") and operate on it
   for ( int ii = 0; ii < niefs; ii++ )
   {
      QString filename = ietc_files[ ii ];

      if ( filename.contains( "somo" ) )    // Leave SOMO files alone for now
         continue;

      bool    in_wetc  = wetc_files.contains( filename );

      if ( keep_files.contains( filename ) )
      {  // This file is to be kept in the install-etc directory
         qDebug() << "KEEP " << filename;
      }

      else if ( copy_files.contains( filename ) )
      {  // This file is to be copied to the work-etc directory
         qDebug() << "COPY " << filename;
         if ( in_wetc )
         {  // But only copy if it is not already copied
            QString icksum = US_Util::md5sum_file( ietc_dname + filename );
            QString wcksum = US_Util::md5sum_file( wetc_dname + filename );
            qDebug() << "   ietc cksum+size " << icksum;
            qDebug() << "   wetc cksum+size " << wcksum;

            if ( icksum != wcksum )
            {  // They do not match in cksum+size, so copy
               nfcopy++;
               qDebug() << "       FILE COPY" << nfcopy;
               QFile( wetc_dname + filename ).remove();

               QFile( ietc_dname + filename ).copy(
                      wetc_dname + filename );
            }
         }
         else
         {  // Not present in work-etc, so copy
            qDebug() << "   not present in" << wetc_dname;
            nfcopy++;
            qDebug() << "       FILE COPY" << nfcopy;
            QFile( ietc_dname + filename ).copy(
                   wetc_dname + filename );
         }
      }

      else if ( link_files.contains( filename ) )
      {  // This is a link, so copy the target file
         qDebug() << "LINK " << filename;
         QString sltarg = ie_files[ ii ].symLinkTarget();
         qDebug() << "   ietc sltarg " << sltarg;

         if ( in_wetc )
         {  // But only copy if not already copied
            QString icksum = US_Util::md5sum_file( sltarg );
            QString wcksum = US_Util::md5sum_file( wetc_dname + filename );
            qDebug() << "   ietc cksum+size " << icksum;
            qDebug() << "   wetc cksum+size " << wcksum;

            if ( icksum != wcksum )
            {  // They do not match in cksum+size, so copy
               nfcopy++;
               qDebug() << "       FILE COPY" << nfcopy;
               QFile( wetc_dname + filename ).remove();        // Remove first

               QFile( sltarg ).copy( wetc_dname + filename );  // Then copy
            }
         }
         else
         {  // Not present in work-etc, so copy
            qDebug() << "   not present in" << wetc_dname;
            nfcopy++;
            qDebug() << "       FILE COPY" << nfcopy;
            QFile( sltarg ).copy( wetc_dname + filename );
         }
      }

      else if ( dir_names.contains( filename ) )
      {  // This is a directory, so ignore it
         qDebug() << "SDIR " << filename;
      }

      else if ( filename.contains( "~" ) )
      {  // If name ends in tilde, delete it
         qDebug() << "DELE " << filename;
         nfdele++;
         qDebug() << "       FILE DELE" << nfdele;
         QFile( ietc_dname + filename ).remove();
      }

      else
      {  // File not in "belongs" list:  move it to work-etc
         qDebug() << "MOVE " << filename;
         nfmove++;
         qDebug() << "       FILE MOVE" << nfmove;

         if ( in_wetc )
         {  // First delete any version in work-etc
            QFile( wetc_dname + filename ).remove();
         }

         QFile( ietc_dname + filename ).rename(
                wetc_dname + filename );
      }
   }

   int nfmods         = nfmove + nfcopy + nfdele;

   if ( nfmods > 0  &&  report )
   {  // If so flagged and any exist, pop up a message on modified files
      QString msg        = tr( "%1 files were moved, copied, or deleted from"
                               "<br/>&nbsp;&nbsp; <b>%2</b>.<br/>"
                               "Examine possible new or replaced files in"
                               "<br/>&nbsp;&nbsp; <b>%3</b>." )
                           .arg( nfmods ).arg( ietc_dname ).arg( wetc_dname );
      QMessageBox::information( this, tr( "Etc Directory Cleaned" ), msg );
   }

   return nfmods;
}

//////////////////  New class

US_Disk_DB_Controls::US_Disk_DB_Controls( int state )
{
   QButtonGroup* group = new QButtonGroup;

   QGridLayout* db_layout   = us_radiobutton( tr( "Database" ),   rb_db );
   QGridLayout* disk_layout = us_radiobutton( tr( "Local Disk" ), rb_disk );

   group->addButton( rb_db );
   group->addButton( rb_disk );

   if ( state == Default ) state = US_Settings::default_data_location();

   ( state == Disk ) ? rb_disk->setChecked( true ) : rb_db->setChecked( true );

   setSpacing        ( 0 );
   setContentsMargins( 0, 0, 0, 0 );

   addLayout( db_layout );
   addLayout( disk_layout );

   connect( rb_db, &QAbstractButton::toggled, this, &US_Disk_DB_Controls::rb_changed );
}

bool US_Disk_DB_Controls::db( void )
{
   return rb_db->isChecked();
}

void US_Disk_DB_Controls::set_db( void )
{
   rb_db->disconnect();
   rb_db->setChecked( true );
   connect( rb_db, &QAbstractButton::toggled, this, &US_Disk_DB_Controls::rb_changed );
}

void US_Disk_DB_Controls::set_disk( void )
{
   rb_db  ->disconnect();
   rb_disk->setChecked( true );
   connect( rb_db, &QAbstractButton::toggled, this, &US_Disk_DB_Controls::rb_changed );
}

void US_Disk_DB_Controls::rb_changed( bool /* state */ )
{
   emit changed( rb_db->isChecked() );
}

// Copy from US_Widgets so global is not needed.
QGridLayout* US_Disk_DB_Controls::us_radiobutton( 
      const QString& text, QRadioButton*& rb, bool state )
{
  QFont    font = QFont( US_GuiSettings::fontFamily(),
                         US_GuiSettings::fontSize  (),
                         QFont::Bold );

  QFontMetrics fm( font );

  QLabel* lb_spacer = new QLabel;
  lb_spacer->setFixedWidth        ( fm.horizontalAdvance( "w" ) ); // Space as wide as a 'w'
  US_Theme::tag( lb_spacer, US_Theme::Normal );

  rb = new QRadioButton( text.toLatin1() );
  rb->setFont              ( font  );
  US_Theme::tag( rb, US_Theme::Normal );
  rb->setChecked           ( state );

  QGridLayout* layout = new QGridLayout;
  layout->setSpacing        ( 0 );
  layout->setContentsMargins( 0, 0, 0, 0 );

  layout->addWidget( lb_spacer, 0, 0 );
  layout->addWidget( rb       , 0, 1 );

  return layout;
}

US_LineEdit_RE::US_LineEdit_RE(const QString& txt, int fontAdjust, bool readonly): QLineEdit(){
    _dfltchrs = 50;
    _mytext = txt;
    _editstate = false;
    this->setFont(QFont(US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() + fontAdjust));
    this->insert(_mytext);
    this->setAutoFillBackground( true );
    QPalette vlgray = US_GuiSettings::readonlyColor();
    if (readonly){
        US_Theme::tag( this, US_Theme::ReadOnly );
        this->setReadOnly( true );
    } else {
        US_Theme::tag( this, US_Theme::Normal );
        this->setReadOnly( false );
    }
    setDefault();
    connect(this, &QLineEdit::textEdited, this, &US_LineEdit_RE::newEdit);
}

void US_LineEdit_RE::setDefault(){
    setMaxChars(_dfltchrs);
    re.setPattern("[^a-zA-Z0-9\+_-]" );
}

void US_LineEdit_RE::setMaxChars(int n){
    _maxchrs = n;
}

void US_LineEdit_RE::setText(const QString & inText) {
    QLineEdit::setText(inText);
    _mytext = inText;
    if (! _editstate){
        QString outText = inText;
        int reIdx = outText.indexOf(re, 0);
        while (reIdx >= 0) {
            outText.remove(reIdx, 1);
            reIdx = outText.indexOf(re, 0);
        }
        if (outText.size() > _maxchrs){
            outText = outText.left(_maxchrs);
        }
        if (outText != inText){
            QMessageBox::warning( this,
                  tr( "Warning!" ),
                  tr( "Special characters are removed from the input string and the "
                      "length of the string is limited to the first %1 characters.\n"
                      "input: %2\noutput: %3" ).arg(_maxchrs).arg(inText, outText));
            QLineEdit::setText(outText);
            _mytext = outText;
        }
    }
    _editstate = false;
}

void US_LineEdit_RE::setText_auto(const QString & txt){
    _editstate = true;
    this->setText(txt);
}

void US_LineEdit_RE::_set_etext(const QString & txt){
    _editstate = true;
    this->setText(txt);
}

void US_LineEdit_RE::newEdit(const QString & newText){
    int reIdx;
    int crtpos = this->cursorPosition();
    if (newText.size() < _mytext.size()){
        reIdx = newText.indexOf(re, 0);
        if (reIdx >= 0){
            this->_set_etext(_mytext);
            this->setCursorPosition(reIdx);
        }else{
            _mytext = newText;
            emit textUpdated();
        }
    }else{
        reIdx = newText.indexOf(re, 0);
        if (reIdx >= 0){
            this->_set_etext(_mytext);
            this->setCursorPosition(reIdx);
        }else{
            if (newText.size() > _maxchrs){
                QMessageBox::warning( this,
                      tr( "Warning!" ),
                      tr( "The length of the text cannot exceed %1 characters!" ).arg(_maxchrs));
                this->_set_etext(_mytext);
                this->setCursorPosition(crtpos - 1);
            } else{
                _mytext = newText;
                emit textUpdated();
            }
        }
    }
    return;
}
