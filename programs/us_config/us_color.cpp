//! \file us_color.cpp
#include "us_color.h"
#include "us_gui_settings.h"
#include "us_theme.h"
#include "us_plot.h"

US_Color::US_Color( QWidget* w, const Qt::WindowFlags flags )
   : US_Widgets( true, w, flags )
{
  setWindowTitle( "Color Configuration for UltraScan" );
  setAttribute( Qt::WA_DeleteOnClose );

  auto* mainLayout = new QGridLayout( this );
  mainLayout->setSpacing( 2 );
  mainLayout->setColumnStretch( 0, 99 ); // Stretch the plot horizontally
  mainLayout->setColumnStretch( 1,  0 ); // Minimum stretch of this column

  auto* leftColumn = new QGridLayout;
  leftColumn->setSpacing( 2 );
  leftColumn->setRowStretch( 1, 99 ); // Stretch the plot vertically

  int row = 0;

  // Frame banner
  lbl_background = us_banner( tr( "Frame Background" ) );
  leftColumn->addWidget( lbl_background, row++, 0, 1, 2 );

  // Plot example
  us_plot  = new US_Plot( plot, tr( "Sample Plot" ),
        tr( "X-axis" ), ( "Y-axis" ) );

  grid  = us_grid  ( plot );
  curve = us_curve ( plot, tr( "Sample Curve 1" ) );
  pick  = us_picker( plot );

  constexpr int ArraySize = 21;
  double    x[ ArraySize ];
  double    y[ ArraySize ];

  for (int i = 0; i < ArraySize; i++)
  {
     x[ i ] = 1.0 + i * 0.2    / ( ArraySize - 1 );  // 1.0 to 1.2
     y[ i ] = 1.0 + static_cast<double>(i) / ( ArraySize - 1 );  // 1.0 to 2.0
  }

  curve->setSamples( x, y, ArraySize );

  leftColumn->addLayout( us_plot, row++, 0, 3, 2 );

  // Plot margin

  lbl_margin = us_banner( tr( "Current Plot Border Margin:") );
  leftColumn->addWidget( lbl_margin, row, 0 );

  cmbb_margin = us_comboBox();

  for ( int i = 1; i <= 15; i++ )   //set margin range from 2 to 30
  {
    QString s = QString::asprintf("%d  pixels", i * 2);
    cmbb_margin->insertItem( i - 1, s );
  }

  connect( cmbb_margin, SIGNAL( activated( int ) ), SLOT( selMargin( int ) ) );
  leftColumn->addWidget( cmbb_margin, row++, 1 );

  // Labels
  lbl_example = us_banner( tr( "Label and Button Examples:" ) );
  leftColumn->addWidget( lbl_example, row++, 0, 1, 2 );

  auto* labelLine = new QHBoxLayout();
  lbl_banner = us_banner( tr( "Banner Label" ) );
  labelLine->addWidget( lbl_banner );

  le_edit = us_lineedit( tr( "Line Edit" ), 1, true );
  le_edit->setAlignment( Qt::AlignCenter );
  le_edit->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
  labelLine->addWidget( le_edit );

  lbl_text = us_label( tr( "Text Label" ) );
  lbl_text->setAlignment( Qt::AlignCenter );
  labelLine->addWidget( lbl_text );
  leftColumn->addLayout( labelLine, row++, 0, 1, 2 );

  // Pushbuttons

  auto* pbLine = new QHBoxLayout();

  pb_normal = us_pushbutton( tr( "Pushbutton" ) );
  pbLine->addWidget( pb_normal );

  pb_active = us_pushbutton( tr( "Inactive" ) );
  pb_active->setToolTip( tr( "Click on another window for inactive colors" ) );
  pbLine->addWidget( pb_active );

  pb_disabled = us_pushbutton( tr( "Disabled" ), false );
  pbLine->addWidget( pb_disabled );

  leftColumn->addLayout( pbLine, row++, 0, 1, 2 );

  constexpr int width  = 120;

  // Progress bar
  auto* progressLine = new QHBoxLayout();

  lbl_progress = us_banner( tr("Progress Bar:" ) );
  lbl_progress->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
  lbl_progress->setMinimumWidth( width );
  progressLine->addWidget( lbl_progress );

  progress = us_progressBar( 0, 100, 35 );
  progress->setMinimumWidth( 2 * width );
  progressLine->addWidget( progress );

  leftColumn->addLayout( progressLine, row++, 0, 1, 2 );

  // LCD Panel

  auto* lcdLine = new QHBoxLayout;
  lbl_lcd = us_banner( tr( "LCD Panel:" ) );
  lbl_lcd->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
  lbl_lcd->setMinimumWidth( width );
  lbl_lcd->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
  lcdLine->addWidget( lbl_lcd );

  lcd  = us_lcd( 3, 35 );
  lcd->setMinimumWidth( 2 * width );
  lcdLine->addWidget( lcd );

  leftColumn->addLayout( lcdLine, row++, 0, 1, 2 );

  // Counter
  auto* counterLine = new QHBoxLayout();
  lbl_counter = us_banner( tr( "Counter:" ) );
  lbl_counter->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
  lbl_counter->setMinimumWidth( width );
  counterLine->addWidget( lbl_counter );

  cnt = us_counter( 2, 0.0, 100.0, 35 );
  cnt->setIncSteps    ( QwtCounter::Button1,  1 );
  cnt->setIncSteps    ( QwtCounter::Button2, 20 );
  cnt->setMinimumWidth( 2 * width );
  connect( cnt, SIGNAL( valueChanged( double ) ), SLOT( updateWidgets( double ) ) );
  counterLine->addWidget( cnt );

  leftColumn->addLayout( counterLine, row++, 0, 1, 2 );

  // Right side
  auto* rightColumn = new QGridLayout;
  rightColumn->setSpacing( 2 );

  row = 0;

  // Style
  lbl_style = us_label( tr( "Style:" ) );
  rightColumn->addWidget( lbl_style, row, 0 );

  QStringList styles = QStyleFactory::keys();
  styles.sort();

  cmbb_style = us_comboBox();
  cmbb_style->addItems( styles );
  current.guiStyle = US_GuiSettings::guiStyle();

  cmbb_style->setCurrentIndex(
        cmbb_style->findText( current.guiStyle, Qt::MatchFixedString ) );

  // activated( const QString& ) no longer exists in Qt6, so go through the
  // index overload, which both Qt5 and Qt6 provide.
  connect( cmbb_style, SIGNAL( activated  ( int ) ),
                       SLOT  ( selectStyle( int ) ) );

  rightColumn->addWidget( cmbb_style, row++, 1 );

  // Color scheme
  lbl_scheme = us_label( tr( "Color Scheme:" ) );
  rightColumn->addWidget( lbl_scheme, row, 0 );

  cmbb_scheme = us_comboBox();
  cmbb_scheme->addItem( tr( "Follow the desktop" ), QString( "auto"  ) );
  cmbb_scheme->addItem( tr( "Light"             ), QString( "light" ) );
  cmbb_scheme->addItem( tr( "Dark"              ), QString( "dark"  ) );
  cmbb_scheme->setToolTip( tr( "The UltraScan default colors come in a light"
                               " and a dark variant.  \"Follow the desktop\""
                               " picks the one the desktop asks for." ) );

  connect( cmbb_scheme, SIGNAL( activated   ( int ) ),
                        SLOT  ( selectScheme( int ) ) );

  rightColumn->addWidget( cmbb_scheme, row++, 1 );

  // Colors
  lbl_assign = us_banner( tr( "Assign new Colors" ) );
  rightColumn->addWidget( lbl_assign, row++, 0, 1, 2 );

  lbl_select = us_label( tr( "Selected Screen Element:" ) );
  rightColumn->addWidget( lbl_select, row, 0 );

  le_choice = us_lineedit( "", -1, true );
  rightColumn->addWidget( le_choice, row++, 1 );

  // Change color boxes

  auto* color1 = new QHBoxLayout;
  lbl_color1 = us_label( "Color1" );
  color_field1 = us_label( "" );
  color_field1->setFixedSize( 22, 22 );
  color_field1->setFrameStyle( QFrame::Box | QFrame::Plain );
  color1->addWidget( lbl_color1 );
  color1->addWidget( color_field1 );

  pb_color1 = us_pushbutton( tr( "Change" ) );
  connect( pb_color1, SIGNAL( clicked() ), SLOT( pick_color1() ) );

  rightColumn->addLayout( color1   , row  , 0 );
  rightColumn->addWidget( pb_color1, row++, 1 );

  auto* color2 = new QHBoxLayout;
  lbl_color2 = us_label( "Color2" );
  color_field2 = us_label( "" );
  color_field2->setFixedSize( 22, 22 );
  color_field2->setFrameStyle( QFrame::Box | QFrame::Plain );
  color2->addWidget( lbl_color2 );
  color2->addWidget( color_field2 );

  pb_color2 = us_pushbutton( tr( "Change" ) );
  connect( pb_color2, SIGNAL( clicked() ), SLOT( pick_color2() ) );

  rightColumn->addLayout( color2   , row  , 0 );
  rightColumn->addWidget( pb_color2, row++, 1 );

  auto* color3 = new QHBoxLayout;
  lbl_color3 = us_label( "Color3" );
  color_field3 = us_label( "" );
  color_field3->setFixedSize( 22, 22 );
  color_field3->setFrameStyle( QFrame::Box | QFrame::Plain );
  color3->addWidget( lbl_color3 );
  color3->addWidget( color_field3 );

  pb_color3 = us_pushbutton( tr( "Change" ) );
  connect( pb_color3, SIGNAL( clicked() ), SLOT( pick_color3() ) );

  rightColumn->addLayout( color3   , row  , 0 );
  rightColumn->addWidget( pb_color3, row++, 1 );

  auto* color4 = new QHBoxLayout;
  lbl_color4 = us_label( "Color4" );
  color_field4 = us_label( "" );
  color_field4->setFixedSize( 22, 22 );
  color_field4->setFrameStyle( QFrame::Box | QFrame::Plain );
  color4->addWidget( lbl_color4 );
  color4->addWidget( color_field4 );

  pb_color4 = us_pushbutton( tr( "Change" ) );
  connect( pb_color4, SIGNAL( clicked() ), SLOT( pick_color4() ) );

  rightColumn->addLayout( color4   , row  , 0 );
  rightColumn->addWidget( pb_color4, row++, 1 );

  auto* color5 = new QHBoxLayout;
  lbl_color5 = us_label( "Color5" );
  color_field5 = us_label( "" );
  color_field5->setFixedSize( 22, 22 );
  color_field5->setFrameStyle( QFrame::Box | QFrame::Plain );
  color5->addWidget( lbl_color5 );
  color5->addWidget( color_field5 );

  pb_color5 = us_pushbutton( tr( "Change" ) );
  connect( pb_color5, SIGNAL( clicked() ), SLOT( pick_color5() ) );

  rightColumn->addLayout( color5   , row  , 0 );
  rightColumn->addWidget( pb_color5, row++, 1 );

  auto* color6 = new QHBoxLayout;
  lbl_color6 = us_label( "Color6" );
  color_field6 = us_label( "" );
  color_field6->setFixedSize( 22, 22 );
  color_field6->setFrameStyle( QFrame::Box | QFrame::Plain );
  color6->addWidget( lbl_color6 );
  color6->addWidget( color_field6 );

  pb_color6 = us_pushbutton( tr( "Change" ) );
  connect( pb_color6, SIGNAL( clicked() ), SLOT( pick_color6() ) );

  rightColumn->addLayout( color6   , row  , 0 );
  rightColumn->addWidget( pb_color6, row++, 1 );

  // Scheme/Element selection

  lbl_choices = us_banner( tr( "Please Make a Selection:" ) );
  rightColumn->addWidget( lbl_choices, row++, 0, 1, 2 );

  lbl_select_scheme  = us_label( tr( "Select Color Scheme:"     ) );
  lbl_select_element = us_label( tr( "Select a Screen Element:" ) );
  rightColumn->addWidget( lbl_select_scheme , row  , 0 );
  rightColumn->addWidget( lbl_select_element, row++, 1 );

  schemes = us_listwidget();
  schemes->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Expanding );
  schemes->setMaximumWidth( 160 );
  schemes->setMinimumWidth( 160 );

  // Delay population of schemes until after delete pushbutton is created
  //updateSchemeList();

  // For some reason, this needs to be after the first call to updateSchemeList();
  //connect( schemes, SIGNAL( itemSelectionChanged() ),
  //                  SLOT  ( selected_scheme     () ) );

  rightColumn->setRowStretch( row, 99 );  // High number is arbitrary
  rightColumn->addWidget( schemes, row, 0 );

  elements = us_listwidget();
  elements->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Expanding );
  elements->setMaximumWidth( 160 );
  elements->setMinimumWidth( 160 );
  elements->insertItem( FRAME        , tr( "Window Background"    ) );
  elements->insertItem( BANNER       , tr( "Section Banner"       ) );
  elements->insertItem( NORMAL_PB    , tr( "Pushbutton, normal"   ) );
  elements->insertItem( DISABLED_PB  , tr( "Pushbutton, disabled" ) );
  elements->insertItem( PLOT_FRAME   , tr( "Plot, frame"          ) );
  elements->insertItem( PLOT_CANVAS  , tr( "Plot, canvas"         ) );
  elements->insertItem( LABELS       , tr( "Labels"               ) );
  elements->insertItem( OTHER_WIDGETS, tr( "Other Widgets"        ) );
  elements->insertItem( LCD          , tr( "LCD Panel"            ) );
  elements->insertItem( EDIT_BOXES   , tr( "Edit Boxes"           ) );

  connect( elements, SIGNAL( currentRowChanged( int ) ),
           this,     SLOT  ( selectedElement  ( int ) ) );

  rightColumn->addWidget( elements, row++, 1 );

  pb_save_as = us_pushbutton( tr( "Save Color Scheme as:" ) );
  connect( pb_save_as, SIGNAL( clicked() ), SLOT( save_as() ) );
  rightColumn->addWidget( pb_save_as, row, 0 );

  le_save_as = us_lineedit( tr( "MyColors" ), 0 );

  rightColumn->addWidget( le_save_as, row++, 1 );

  // Bottom row of buttons

  auto* buttonBar = new QHBoxLayout;

  pb_apply = us_pushbutton( tr( "Set as Current" ) );
  buttonBar->addWidget( pb_apply );
  connect( pb_apply, SIGNAL( clicked() ), SLOT( apply() ) );

  pb_reset = us_pushbutton( tr( "Reset" ) );
  buttonBar->addWidget( pb_reset );
  connect( pb_reset, SIGNAL( clicked() ), SLOT( reset() ) );

  pb_delete = us_pushbutton( tr( "Delete" ) );
  buttonBar->addWidget( pb_delete );
  connect( pb_delete, SIGNAL( clicked() ), SLOT( delete_scheme() ) );

  updateSchemeList();
  connect( schemes, SIGNAL( itemSelectionChanged() ),
                    SLOT  ( selected_scheme     () ) );

  pb_help = us_pushbutton( tr( "Help" ) );
  buttonBar->addWidget( pb_help );
  connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

  pb_quit = us_pushbutton( tr( "Close" ) );
  buttonBar->addWidget( pb_quit );
  connect( pb_quit, SIGNAL( clicked() ), SLOT( close() ) );

  mainLayout->addLayout( leftColumn , 0, 0 );
  mainLayout->addLayout( rightColumn, 0, 1 );
  mainLayout->addLayout( buttonBar  , 1, 0, 1, 2 );

  reset();
}
void US_Color::getCurrentSettings( void )
{
  current.plotMargin    = US_GuiSettings::plotMargin();
  current.guiStyle      = US_GuiSettings::guiStyle();
  current.colorScheme   = US_Theme::schemeSetting();

  current.plotCurve     = US_GuiSettings::plotCurve();
  current.plotBg        = US_GuiSettings::plotCanvasBG();
  current.plotMajorGrid = US_GuiSettings::plotMajGrid();
  current.plotMinorGrid = US_GuiSettings::plotMinGrid();
  current.plotPicker    = US_GuiSettings::plotPicker();

  current.frameColor    = US_GuiSettings::frameColor();
  current.bannerColor   = US_GuiSettings::bannerColor();
  current.pushbColor    = US_GuiSettings::pushbColor();
  current.labelColor    = US_GuiSettings::labelColor();
  current.editColor     = US_GuiSettings::editColor();
  current.normalColor   = US_GuiSettings::normalColor();
  current.lcdColor      = US_GuiSettings::lcdColor();
  current.plotColor     = US_GuiSettings::plotColor();
}

void US_Color::updateScreen( void )
{
  QApplication::setStyle( US_Theme::createStyle( current.guiStyle ) );

  // setStyle() installs the style's own palette, so the UltraScan chrome has
  // to be put back on top of it.
  QApplication::setPalette  ( US_Theme::applicationPalette() );
  qApp->setStyleSheet       ( US_Theme::styleSheet() );
  US_Theme::invalidate();

  cmbb_style->setCurrentIndex(
      cmbb_style->findText( current.guiStyle, Qt::MatchFixedString ) );

  const int scheme_index = cmbb_scheme->findData( current.colorScheme );
  cmbb_scheme->setCurrentIndex( scheme_index < 0 ? 0 : scheme_index );

  selectedElement( elements->currentRow() );
  resetFrames();
  resetBanners();
  resetButtons();
  resetLabels();
  resetWidgets();
  resetEditBoxes();

  plot ->setPalette         ( current.plotColor );
  plot ->setCanvasBackground( current.plotBg );
  grid ->setMajorPen        ( QPen( current.plotMajorGrid ) );
  grid ->setMinorPen        ( QPen( current.plotMinorGrid ) );
  curve->setPen             ( QPen( current.plotCurve     ) );
  pick ->setRubberBandPen   ( QPen( current.plotPicker    ) );
  pick ->setTrackerPen      ( QPen( current.plotPicker    ) );

  lcd ->setPalette          ( current.lcdColor  );
}

void US_Color::selectedElement( int index )
{
  QPalette p;
  QColor   c;

  le_choice->setText( elements->item( index )->text() );

  switch ( index )
  {
    case FRAME:  // Window background
      pb_color1->setEnabled( true  );
      pb_color2->setEnabled( true  );
      pb_color3->setEnabled( true  );
      pb_color4->setEnabled( true  );
      pb_color5->setEnabled( true  );
      pb_color6->setEnabled( false );

      p = current.frameColor;

      /* Right now, the only colors needed are Window (background),
       * WindowText( text), and MidLight (border).  This assumes
       * a Frame style of StyledPanel.  The colors needed may change
       * for different operating systems. */


      // Set 2 before 1 so we don't overwrite p's Window Color
      lbl_color2->setText( tr( "Background:" ) );
      c = p.color( QPalette::Active, QPalette::Window );
      p.setColor( QPalette::Window, c );
      color_field2->setPalette( p );

      lbl_color1->setText( tr( "Text:" ) );
      c = p.color( QPalette::Active, QPalette::WindowText );
      p.setColor( QPalette::Window, c );
      color_field1->setPalette( p );

      lbl_color3->setText( tr( "Border:" ) );
      c = p.color( QPalette::Active, QPalette::Light );
      p.setColor( QPalette::Window, c );
      color_field3->setPalette( p );

      lbl_color4->setText( tr( "Medium Shadow:" ) );
      c = p.color( QPalette::Active, QPalette::Dark );
      p.setColor( QPalette::Window, c );
      color_field4->setPalette( p );

      lbl_color5->setText( tr( "Medium Highlight:" ) );
      c = p.color( QPalette::Active, QPalette::Midlight );
      p.setColor( QPalette::Window, c );
      color_field5->setPalette( p );

      p.setColor( QPalette::Window, Qt::gray );
      lbl_color6->setText( "" );
      color_field6->setPalette( p );

      break;

    case BANNER:  // Section banner

      pb_color1->setEnabled( true  );
      pb_color2->setEnabled( true  );
      pb_color3->setEnabled( false );
      pb_color4->setEnabled( false );
      pb_color5->setEnabled( false );
      pb_color6->setEnabled( false );

      p = current.bannerColor;

      // Set 2 before 1 so we don't overwrite p's Window Color
      lbl_color2->setText( tr( "Background:" ) );
      c = p.color( QPalette::Active, QPalette::Window );
      p.setColor( QPalette::Window, c );
      color_field2->setPalette( p );

      lbl_color1->setText( tr( "Text:" ) );
      c = p.color( QPalette::Active, QPalette::WindowText );
      p.setColor( QPalette::Window, c );
      color_field1->setPalette( p );

      p.setColor( QPalette::Window, Qt::gray );
      lbl_color3->setText( "" );
      color_field3->setPalette( p );

      lbl_color4->setText( "" );
      color_field4->setPalette( p );

      lbl_color5->setText( "" );
      color_field5->setPalette( p );

      lbl_color6->setText( "" );
      color_field6->setPalette( p );

      break;

    case NORMAL_PB:  // Pushbutton - active and inactive

      pb_color1->setEnabled( true  );
      pb_color2->setEnabled( true  );
      pb_color3->setEnabled( true  );
      pb_color4->setEnabled( true  );
      pb_color5->setEnabled( false );
      pb_color6->setEnabled( false );

      p = current.pushbColor;

      lbl_color1->setText( tr( "Active Text:" ) );
      c = p.color( QPalette::Active, QPalette::ButtonText );
      p.setColor( QPalette::Window, c );
      color_field1->setPalette( p );

      lbl_color2->setText( tr( "Active Background:" ) );
      c = p.color( QPalette::Active, QPalette::Button );
      p.setColor( QPalette::Window, c );
      color_field2->setPalette( p );

      lbl_color3->setText( tr( "Inactive Text" ) );
      c = p.color( QPalette::Inactive, QPalette::ButtonText );
      p.setColor( QPalette::Window, c );
      color_field3->setPalette( p );

      lbl_color4->setText( tr( "Inactive Background:" ) );
      c = p.color( QPalette::Inactive, QPalette::Button );
      p.setColor( QPalette::Window, c );
      color_field4->setPalette( p );

      lbl_color5->setText( "" );
      color_field5->setPalette( p );

      lbl_color6->setText( "" );
      color_field6->setPalette( p );

      break;

    case DISABLED_PB:  // Pushbutton - disabled

      pb_color1->setEnabled( true  );
      pb_color2->setEnabled( true  );
      pb_color3->setEnabled( true  );
      pb_color4->setEnabled( true  );
      pb_color5->setEnabled( false );
      pb_color6->setEnabled( false );

      p = current.pushbColor;

      lbl_color1->setText( tr( "Text:" ) );
      c = p.color( QPalette::Disabled, QPalette::ButtonText );
      p.setColor( QPalette::Window, c );
      color_field1->setPalette( p );

      lbl_color2->setText( tr( "Background:" ) );
      c = p.color( QPalette::Disabled, QPalette::Button );
      p.setColor( QPalette::Window, c );
      color_field2->setPalette( p );

      lbl_color3->setText( tr( "Drop Shadow:" ) );
      c = p.color( QPalette::Disabled, QPalette::Light );
      p.setColor( QPalette::Window, c );
      color_field3->setPalette( p );

      p.setColor( QPalette::Window, Qt::gray );
      lbl_color4->setText( "" );
      color_field4->setPalette( p );

      lbl_color5->setText( "" );
      color_field5->setPalette( p );

      lbl_color6->setText( "" );
      color_field6->setPalette( p );
      break;

    case PLOT_FRAME:   // Plot, frame

      pb_color1->setEnabled( true  );
      pb_color2->setEnabled( true  );
      pb_color3->setEnabled( true  );
      pb_color4->setEnabled( true  );
      pb_color5->setEnabled( false );
      pb_color6->setEnabled( false );

      p = current.plotColor;

      lbl_color2->setText( tr( "Background:" ) );
      c = p.color( QPalette::Active, QPalette::Window );
      p.setColor( QPalette::Window, c );
      color_field2->setPalette( p );

      lbl_color1->setText( tr( "Tick Marks:" ) );
      c = p.color( QPalette::Active, QPalette::WindowText );
      p.setColor( QPalette::Window, c );
      color_field1->setPalette( p );

      lbl_color3->setText( tr( "Text:" ) );
      c = p.color( QPalette::Active, QPalette::Shadow );
      p.setColor( QPalette::Window, c );
      color_field3->setPalette( p );

      lbl_color4->setText( "Plot Tracker:" );
      p.setColor( QPalette::Window, current.plotPicker );
      color_field4->setPalette( p );

      p.setColor( QPalette::Window, Qt::gray );
      lbl_color5->setText( "" );
      color_field5->setPalette( p );

      lbl_color6->setText( "" );
      color_field6->setPalette( p );

      break;

    case PLOT_CANVAS:  // Plot, canvas

      pb_color1->setEnabled( true  );
      pb_color2->setEnabled( true  );
      pb_color3->setEnabled( true  );
      pb_color4->setEnabled( true  );
      pb_color5->setEnabled( false );
      pb_color6->setEnabled( false );

      lbl_color1->setText( tr( "Plot Background:" ) );
      p.setColor( QPalette::Window, current.plotBg );
      color_field1->setPalette( p );

      lbl_color2->setText( tr( "Curve:" ) );
      p.setColor( QPalette::Window, current.plotCurve );
      color_field2->setPalette( p );

      lbl_color3->setText( tr( "Major Grid Lines:" ) );
      p.setColor( QPalette::Window, current.plotMajorGrid );
      color_field3->setPalette( p );

      lbl_color4->setText( tr( "Minor Grid Lines" ) );
      p.setColor( QPalette::Window, current.plotMinorGrid );
      color_field4->setPalette( p );

      p.setColor( QPalette::Window, Qt::gray );
      lbl_color5->setText( "" );
      color_field5->setPalette( p );

      lbl_color6->setText( "" );
      color_field6->setPalette( p );

      break;

    case LABELS: // Labels

      pb_color1->setEnabled( true  );
      pb_color2->setEnabled( true  );
      pb_color3->setEnabled( false );
      pb_color4->setEnabled( false );
      pb_color5->setEnabled( false );
      pb_color6->setEnabled( false );

      p = current.labelColor;

      // Set 2 before 1 so we don't overwrite p's Window Color
      lbl_color2->setText( tr( "Background:" ) );
      c = p.color( QPalette::Active, QPalette::Window );
      p.setColor( QPalette::Window, c );
      color_field2->setPalette( p );

      lbl_color1->setText( tr( "Text:" ) );
      c = p.color( QPalette::Active, QPalette::WindowText );
      p.setColor( QPalette::Window, c );
      color_field1->setPalette( p );

      p.setColor( QPalette::Window, Qt::gray );
      lbl_color3->setText( "" );
      color_field3->setPalette( p );

      lbl_color4->setText( "" );
      color_field4->setPalette( p );

      lbl_color5->setText( "" );
      color_field5->setPalette( p );

      lbl_color6->setText( "" );
      color_field6->setPalette( p );

      break;

    case OTHER_WIDGETS:  // Other Widgets

      pb_color1->setEnabled( true );
      pb_color2->setEnabled( true );
      pb_color3->setEnabled( true );
      pb_color4->setEnabled( true );
      pb_color5->setEnabled( true );
      pb_color6->setEnabled( true );

      p = current.normalColor;

      // Set 2 before 1 so we don't overwrite p's Window Color
      lbl_color2->setText( tr( "Default Background:" ) );
      c = p.color( QPalette::Active, QPalette::Base );
      p.setColor( QPalette::Window, c );
      color_field2->setPalette( p );

      lbl_color1->setText( tr( "Default Text:" ) );
      c = p.color( QPalette::Active, QPalette::Text );
      p.setColor( QPalette::Window, c );
      color_field1->setPalette( p );

      lbl_color3->setText( tr( "Default Border:" ) );
      c = p.color( QPalette::Active, QPalette::Light );
      p.setColor( QPalette::Window, c );
      color_field3->setPalette( p );

      lbl_color4->setText( tr( "Alternate Text:" ) );
      c = p.color( QPalette::Active, QPalette::ButtonText );
      p.setColor( QPalette::Window, c );
      color_field4->setPalette( p );

      lbl_color5->setText( tr( "Alternate Background:" ) );
      c = p.color( QPalette::Active, QPalette::Button );
      p.setColor( QPalette::Window, c );
      color_field5->setPalette( p );

      lbl_color6->setText( tr( "Highlight:" ) );
      c = p.color( QPalette::Active, QPalette::Midlight );
      p.setColor( QPalette::Window, c );
      color_field6->setPalette( p );
      break;

    case LCD:  // LCD Panels

      pb_color1->setEnabled( true  );
      pb_color2->setEnabled( true  );
      pb_color3->setEnabled( true  );
      pb_color4->setEnabled( true  );
      pb_color5->setEnabled( false );
      pb_color6->setEnabled( false );

      p = current.lcdColor;

      // Set 2 before 1 so we don't overwrite p's Window Color
      lbl_color2->setText( tr( "Background:" ) );
      c = p.color( QPalette::Active, QPalette::Window );
      p.setColor( QPalette::Window, c );
      color_field2->setPalette( p );

      lbl_color1->setText( tr( "Text:" ) );
      c = p.color( QPalette::Active, QPalette::WindowText );
      p.setColor( QPalette::Window, c );
      color_field1->setPalette( p );

      lbl_color3->setText( tr( "Highlight1:" ) );
      c = p.color( QPalette::Active, QPalette::Shadow );
      p.setColor( QPalette::Window, c );
      color_field3->setPalette( p );

      lbl_color4->setText( tr( "Highlight2" ) );
      c = p.color( QPalette::Active, QPalette::Light );
      p.setColor( QPalette::Window, c );
      color_field4->setPalette( p );

      p.setColor( QPalette::Window, Qt::gray );
      lbl_color5->setText( "" );
      color_field5->setPalette( p );

      lbl_color6->setText( "" );
      color_field6->setPalette( p );
      break;

    case EDIT_BOXES:

      pb_color1->setEnabled( true  );
      pb_color2->setEnabled( true  );
      pb_color3->setEnabled( true  );
      pb_color4->setEnabled( false );
      pb_color5->setEnabled( false );
      pb_color6->setEnabled( false );

      p = current.editColor;

      // Set 2 before 1 so we don't overwrite p's Window Color
      lbl_color2->setText( tr( "Background:" ) );
      c = p.color( QPalette::Active, QPalette::Base );
      p.setColor( QPalette::Window, c );
      color_field2->setPalette( p );

      lbl_color1->setText( tr( "Text:" ) );
      c = p.color( QPalette::Active, QPalette::Text );
      p.setColor( QPalette::Window, c );
      color_field1->setPalette( p );

      lbl_color3->setText( tr( "Highlight:" ) );
      c = p.color( QPalette::Active, QPalette::Shadow );
      p.setColor( QPalette::Window, c );
      color_field3->setPalette( p );

      p.setColor( QPalette::Window, Qt::gray );
      lbl_color4->setText( "" );
      color_field4->setPalette( p );

      lbl_color5->setText( "" );
      color_field5->setPalette( p );

      lbl_color6->setText( "" );
      color_field6->setPalette( p );
      break;
  }
}

void US_Color::resetFrames( void )
{
  setPalette( current.frameColor );
}

void US_Color::resetBanners( void )
{
  const QPalette p = current.bannerColor;

  lbl_background->setPalette( p );
  lbl_margin    ->setPalette( p );
  lbl_example   ->setPalette( p );
  lbl_banner    ->setPalette( p );
  lbl_progress  ->setPalette( p );
  lbl_lcd       ->setPalette( p );
  lbl_counter   ->setPalette( p );
  lbl_assign    ->setPalette( p );
  lbl_choices   ->setPalette( p );
}

void US_Color::resetButtons( void )
{
  const QPalette p = current.pushbColor;

  pb_color1  ->setPalette( p );
  pb_color2  ->setPalette( p );
  pb_color3  ->setPalette( p );
  pb_color4  ->setPalette( p );
  pb_color5  ->setPalette( p );
  pb_color6  ->setPalette( p );

  pb_normal  ->setPalette( p );
  pb_active  ->setPalette( p );
  pb_disabled->setPalette( p );

  pb_save_as ->setPalette( p );
  pb_apply   ->setPalette( p );
  pb_reset   ->setPalette( p );
  pb_delete  ->setPalette( p );
  pb_help    ->setPalette( p );
  pb_quit    ->setPalette( p );
}

void US_Color::resetLabels( void )
{
  const QPalette p = current.labelColor;

  lbl_text          ->setPalette( p );
  lbl_select        ->setPalette( p );
  lbl_color1        ->setPalette( p );
  lbl_color2        ->setPalette( p );
  lbl_color3        ->setPalette( p );
  lbl_color4        ->setPalette( p );
  lbl_color5        ->setPalette( p );
  lbl_color6        ->setPalette( p );
  lbl_select_scheme ->setPalette( p );
  lbl_select_element->setPalette( p );
  lbl_style         ->setPalette( p );
  lbl_scheme        ->setPalette( p );
}

void US_Color::resetWidgets( void )
{
  const QPalette p = current.normalColor;

  cmbb_margin->setPalette( p );
  cmbb_style ->setPalette( p );
  cmbb_scheme->setPalette( p );
  progress   ->setPalette( p );
  cnt        ->setPalette( p );
}

void US_Color::resetEditBoxes( void )
{
  const QPalette p = current.editColor;
  le_choice ->setPalette( p );
  le_edit   ->setPalette( p );
  le_save_as->setPalette( p );
  schemes   ->setPalette( p );
  elements  ->setPalette( p );
}

void US_Color::pick_color1( void )
{
  // Get the current color
  const QColor old = color_field1->palette().color( QPalette::Window );

  // Get the new color
  const QColor c  = QColorDialog::getColor( old );

  if ( ! c.isValid() || c == old ) return;

  // Set the new color in selection window
  QPalette p;

  p.setColor( QPalette::Window, c );
  color_field1->setPalette( p );

  // Figure out what to do based on current element selected

  switch ( elements->currentRow() )
  {
    case FRAME:

      current.frameColor.setColor( QPalette::Active  , QPalette::WindowText, c );
      current.frameColor.setColor( QPalette::Inactive, QPalette::WindowText, c );
      current.frameColor.setColor( QPalette::Disabled, QPalette::WindowText, c );
      resetFrames();
      break;

    case BANNER:
      current.bannerColor.setColor( QPalette::Active  , QPalette::WindowText, c );
      current.bannerColor.setColor( QPalette::Inactive, QPalette::WindowText, c );
      current.bannerColor.setColor( QPalette::Disabled, QPalette::WindowText, c );
      resetBanners();
      break;

    case NORMAL_PB:
      current.pushbColor.setColor( QPalette::Active  , QPalette::ButtonText, c );
      resetButtons();
      break;

    case DISABLED_PB:
      current.pushbColor.setColor( QPalette::Disabled, QPalette::ButtonText, c );
      resetButtons();
      break;

    case PLOT_FRAME:
      current.plotColor.setColor( QPalette::Active  , QPalette::WindowText, c );
      current.plotColor.setColor( QPalette::Inactive, QPalette::WindowText, c );
      current.plotColor.setColor( QPalette::Disabled, QPalette::WindowText, c );
      plot->setPalette( current.plotColor );
      plot->replot();
      break;

    case PLOT_CANVAS:
      current.plotBg = c;
      plot->setCanvasBackground( c );
      plot->replot();
      break;

    case LABELS:
      current.labelColor.setColor( QPalette::Active  , QPalette::WindowText, c );
      current.labelColor.setColor( QPalette::Inactive, QPalette::WindowText, c );
      current.labelColor.setColor( QPalette::Disabled, QPalette::WindowText, c );
      resetLabels();
      break;

    case OTHER_WIDGETS:
      current.normalColor.setColor( QPalette::Active  , QPalette::Text, c );
      current.normalColor.setColor( QPalette::Inactive, QPalette::Text, c );
      current.normalColor.setColor( QPalette::Disabled, QPalette::Text, c );
      resetWidgets();
      break;

    case LCD:
      current.lcdColor.setColor( QPalette::Active  , QPalette::WindowText, c );
      current.lcdColor.setColor( QPalette::Inactive, QPalette::WindowText, c );
      current.lcdColor.setColor( QPalette::Disabled, QPalette::WindowText, c );
      lcd->setPalette( current.lcdColor );
      break;

    case EDIT_BOXES:
      current.editColor.setColor( QPalette::Active  , QPalette::Text, c );
      current.editColor.setColor( QPalette::Inactive, QPalette::Text, c );
      current.editColor.setColor( QPalette::Disabled, QPalette::Text, c );
      resetEditBoxes();;
      break;
  }
}

void US_Color::pick_color2( void )
{
  // Get the current color
  const QColor old = color_field2->palette().color( QPalette::Window );

  // Get the new color
  const QColor c  = QColorDialog::getColor( old );

  if ( ! c.isValid() || c == old ) return;

  // Set the new color in selection window
  QPalette p;

  p.setColor( QPalette::Window, c );
  color_field2->setPalette( p );

  // Figure out what to do based on current element selected

  switch ( elements->currentRow() )
  {
    case FRAME:

      current.frameColor.setColor( QPalette::Active  , QPalette::Window, c );
      current.frameColor.setColor( QPalette::Inactive, QPalette::Window, c );
      current.frameColor.setColor( QPalette::Disabled, QPalette::Window, c );
      resetFrames();
      break;

    case BANNER:
      current.bannerColor.setColor( QPalette::Active  , QPalette::Window, c );
      current.bannerColor.setColor( QPalette::Inactive, QPalette::Window, c );
      current.bannerColor.setColor( QPalette::Disabled, QPalette::Window, c );
      resetBanners();
      break;

    case NORMAL_PB:
      current.pushbColor.setColor( QPalette::Active  , QPalette::Button, c );
      resetButtons();
      break;

    case DISABLED_PB:
      current.pushbColor.setColor( QPalette::Disabled, QPalette::Button, c );
      resetButtons();
      break;

    case PLOT_FRAME:
      current.plotColor.setColor( QPalette::Active  , QPalette::Window, c );
      current.plotColor.setColor( QPalette::Inactive, QPalette::Window, c );
      current.plotColor.setColor( QPalette::Disabled, QPalette::Window, c );
      plot->setPalette( current.plotColor );
      plot->replot();
      break;

    case PLOT_CANVAS:
      current.plotCurve = c;
      curve->setPen( QPen( c ) );
      plot->replot();
      break;

    case LABELS:
      current.labelColor.setColor( QPalette::Active  , QPalette::Window, c );
      current.labelColor.setColor( QPalette::Inactive, QPalette::Window, c );
      current.labelColor.setColor( QPalette::Disabled, QPalette::Window, c );
      resetLabels();
      break;

    case OTHER_WIDGETS:
      current.normalColor.setColor( QPalette::Active  , QPalette::Base, c );
      current.normalColor.setColor( QPalette::Inactive, QPalette::Base, c );
      current.normalColor.setColor( QPalette::Disabled, QPalette::Base, c );
      resetWidgets();
      break;

    case LCD:
      current.lcdColor.setColor( QPalette::Active  , QPalette::Window, c );
      current.lcdColor.setColor( QPalette::Inactive, QPalette::Window, c );
      current.lcdColor.setColor( QPalette::Disabled, QPalette::Window, c );
      lcd->setPalette( current.lcdColor );
      break;

    case EDIT_BOXES:
      current.editColor.setColor( QPalette::Active  , QPalette::Base, c );
      current.editColor.setColor( QPalette::Inactive, QPalette::Base, c );
      current.editColor.setColor( QPalette::Disabled, QPalette::Base, c );
      resetEditBoxes();
      break;
   }
}

void US_Color::pick_color3( void )
{
  // Get the current color
  const QColor old = color_field3->palette().color( QPalette::Window );

  // Get the new color
  const QColor c  = QColorDialog::getColor( old );

  if ( ! c.isValid() || c == old ) return;

  // Set the new color in selection window
  QPalette p;

  p.setColor( QPalette::Window, c );
  color_field3->setPalette( p );

  // Figure out what to do based on current element selected

  switch ( elements->currentRow() )
  {
    case FRAME:

      current.frameColor.setColor( QPalette::Active  , QPalette::Light, c );
      current.frameColor.setColor( QPalette::Inactive, QPalette::Light, c );
      current.frameColor.setColor( QPalette::Disabled, QPalette::Light, c );
      resetFrames();
      break;

    case NORMAL_PB:
      current.pushbColor.setColor( QPalette::Inactive, QPalette::ButtonText, c );
      resetButtons();
      break;

    case DISABLED_PB:
      current.pushbColor.setColor( QPalette::Disabled, QPalette::Light, c );
      resetButtons();
      break;

    case PLOT_FRAME:
      current.plotColor.setColor( QPalette::Active  , QPalette::Text, c );
      current.plotColor.setColor( QPalette::Inactive, QPalette::Text, c );
      current.plotColor.setColor( QPalette::Disabled, QPalette::Text, c );
      plot->setPalette( current.plotColor );
      plot->replot();
      break;

    case PLOT_CANVAS:
      current.plotMajorGrid = c;
      grid->setMajorPen( QPen( c ) );
      plot->replot();
      break;

    case OTHER_WIDGETS:
      current.normalColor.setColor( QPalette::Active  , QPalette::Shadow, c );
      current.normalColor.setColor( QPalette::Inactive, QPalette::Shadow, c );
      current.normalColor.setColor( QPalette::Disabled, QPalette::Shadow, c );
      resetWidgets();
      break;

    case LCD:
      current.lcdColor.setColor( QPalette::Active  , QPalette::Dark, c );
      current.lcdColor.setColor( QPalette::Inactive, QPalette::Dark, c );
      current.lcdColor.setColor( QPalette::Disabled, QPalette::Dark, c );
      lcd->setPalette( current.lcdColor );
      break;

    case EDIT_BOXES:
      current.editColor.setColor( QPalette::Active  , QPalette::Shadow, c );
      current.editColor.setColor( QPalette::Inactive, QPalette::Shadow, c );
      current.editColor.setColor( QPalette::Disabled, QPalette::Shadow, c );
      resetEditBoxes();
      break;
   }
}

void US_Color::pick_color4( void )
{
  // Get the current color
  const QColor old = color_field4->palette().color( QPalette::Window );

  // Get the new color
  const QColor c  = QColorDialog::getColor( old );

  if ( ! c.isValid() || c == old ) return;

  // Set the new color in selection window
  QPalette p;

  p.setColor( QPalette::Window, c );
  color_field4->setPalette( p );

  // Figure out what to do based on current element selected

  switch ( elements->currentRow() )
  {
    case FRAME:

      current.frameColor.setColor( QPalette::Active  , QPalette::Dark, c );
      current.frameColor.setColor( QPalette::Inactive, QPalette::Dark, c );
      current.frameColor.setColor( QPalette::Disabled, QPalette::Dark, c );
      resetFrames();
      break;

    case NORMAL_PB:
      current.pushbColor.setColor( QPalette::Inactive, QPalette::Button, c );
      resetButtons();
      break;

    case PLOT_FRAME:
      current.plotPicker = c;
      pick ->setRubberBandPen( QPen( c ) );
      pick ->setTrackerPen   ( QPen( c ) );
      plot->replot();
      break;

    case PLOT_CANVAS:
      current.plotMajorGrid = c;
      grid->setMajorPen( QPen( c ) );
      plot->replot();
      break;

    case OTHER_WIDGETS:
      current.normalColor.setColor( QPalette::Active  , QPalette::ButtonText, c );
      current.normalColor.setColor( QPalette::Inactive, QPalette::ButtonText, c );
      current.normalColor.setColor( QPalette::Disabled, QPalette::ButtonText, c );
      resetWidgets();
      break;

    case LCD:
      current.lcdColor.setColor( QPalette::Active  , QPalette::Light, c );
      current.lcdColor.setColor( QPalette::Inactive, QPalette::Light, c );
      current.lcdColor.setColor( QPalette::Disabled, QPalette::Light, c );
      lcd->setPalette( current.lcdColor );
      break;
   }
}

void US_Color::pick_color5( void )
{
  // Get the current color
  const QColor old = color_field5->palette().color( QPalette::Window );

  // Get the new color
  const QColor c  = QColorDialog::getColor( old );

  if ( ! c.isValid() || c == old ) return;

  // Set the new color in selection window
  QPalette p;

  p.setColor( QPalette::Window, c );
  color_field5->setPalette( p );

  // Figure out what to do based on current element selected

  switch ( elements->currentRow() )
  {
    case FRAME:

      current.frameColor.setColor( QPalette::Active  , QPalette::Midlight, c );
      current.frameColor.setColor( QPalette::Inactive, QPalette::Midlight, c );
      current.frameColor.setColor( QPalette::Disabled, QPalette::Midlight, c );
      resetFrames();
      break;

    case OTHER_WIDGETS:
      current.normalColor.setColor( QPalette::Active  , QPalette::Button, c );
      current.normalColor.setColor( QPalette::Inactive, QPalette::Button, c );
      current.normalColor.setColor( QPalette::Disabled, QPalette::Button, c );
      resetWidgets();
      break;
   }
}

void US_Color::pick_color6( void )
{
  // Get the current color
  const QColor old = color_field6->palette().color( QPalette::Window );

  // Get the new color
  const QColor c  = QColorDialog::getColor( old );

  if ( ! c.isValid() || c == old ) return;

  // Set the new color in selection window
  QPalette p;

  p.setColor( QPalette::Window, c );
  color_field6->setPalette( p );

  // Figure out what to do based on current element selected

  switch ( elements->currentRow() )
  {
    case OTHER_WIDGETS:
      current.normalColor.setColor( QPalette::Active  , QPalette::Midlight, c );
      current.normalColor.setColor( QPalette::Inactive, QPalette::Midlight, c );
      current.normalColor.setColor( QPalette::Disabled, QPalette::Midlight, c );
      resetWidgets();
      break;
   }
}

void US_Color::help( void )
{
  showhelp.show_help( "manual/config.html#color-configuration-panel" );
}

void US_Color::reset()
{
  elements->setCurrentRow( 0 );
  schemes ->setCurrentRow( 0 );

  getCurrentSettings();
  updateScreen();

  cnt->setValue( 35 );
  updateWidgets( 35 );

  const int index = current.plotMargin / 2 - 1;
  cmbb_margin->setCurrentIndex( index );
  selMargin( index );

  pb_delete->setEnabled( false );
}

void US_Color::updateWidgets( const double newval )
{
  lcd->display( newval );
  progress->reset();
  progress->setValue( static_cast<int>(newval) );
}

void US_Color:: selMargin( const int index )
{
  current.plotMargin = ( index + 1 ) * 2;
  //plot->setMargin( current.plotMargin );
  plot->setStyleSheet( QString( "QwtPlot{ padding: %1px }" )
        .arg( current.plotMargin ) );
}

void US_Color::apply( void )
{
  US_GuiSettings::set_guiStyle    ( current.guiStyle      );
  US_GuiSettings::set_plotMargin  ( current.plotMargin    );

  US_GuiSettings::set_plotCurve   ( current.plotCurve     );
  US_GuiSettings::set_plotCanvasBG( current.plotBg        );
  US_GuiSettings::set_plotMajGrid ( current.plotMajorGrid );
  US_GuiSettings::set_plotMinGrid ( current.plotMinorGrid );
  US_GuiSettings::set_plotPicker  ( current.plotPicker    );

  US_GuiSettings::set_frameColor  ( current.frameColor    );
  US_GuiSettings::set_bannerColor ( current.bannerColor   );
  US_GuiSettings::set_pushbColor  ( current.pushbColor    );
  US_GuiSettings::set_labelColor  ( current.labelColor    );
  US_GuiSettings::set_editColor   ( current.editColor     );
  US_GuiSettings::set_normalColor ( current.normalColor   );
  US_GuiSettings::set_lcdColor    ( current.lcdColor      );
  US_GuiSettings::set_plotColor   ( current.plotColor     );

  // Rebuild the application chrome from what was just stored
  US_Theme::apply( true );
}

void US_Color::save_as( void )
{
  // Remove blanks from name
  const QString custom = le_save_as->text().replace( QRegularExpression( "[ \t/]" ), "" );

  if ( custom.size() == 0 )
  {
    QMessageBox::warning( this,
      tr( "Save scheme" ),
      tr( "A scheme name must be specified." ) );

    return;
  }

  le_save_as->setText( custom );

  // Get list of custom colors schemes
  QSettings settings( "UTHSCSA", "UltraScan" );
  QStringList schemeList = settings.value( "schemeList" ).toStringList();

  // Add new one if it's not there
  if ( ! schemeList.contains( custom ) )
  {
    schemeList << custom;
    settings.setValue( "schemeList", schemeList );
  }

  const QString set = "schemes/" + custom + "/";

  // Save current as scheme
  settings.setValue( set + "plotMargin"  , current.plotMargin    );
  settings.setValue( set + "plotCurve"   , current.plotCurve     );
  settings.setValue( set + "plotBg"      , current.plotBg        );
  settings.setValue( set + "plotMajGrid" , current.plotMajorGrid );
  settings.setValue( set + "plotMinGrid" , current.plotMinorGrid );
  settings.setValue( set + "plotPicker"  , current.plotPicker    );

  settings.setValue( set + "frameColor"  , current.frameColor    );
  settings.setValue( set + "bannerColor" , current.bannerColor   );
  settings.setValue( set + "pushbColor"  , current.pushbColor    );
  settings.setValue( set + "labelColor"  , current.labelColor    );
  settings.setValue( set + "editColor"   , current.editColor     );
  settings.setValue( set + "normalColor" , current.normalColor   );
  settings.setValue( set + "lcdColor"    , current.lcdColor      );
  settings.setValue( set + "plotColor"   , current.plotColor     );
  settings.sync();

  // Update scheme list
  updateSchemeList( custom );
}

void US_Color::updateSchemeList( const QString& active )
{
  schemes->clear();
  schemes->addItem( tr( "Current Scheme"    ) );
  schemes->addItem( tr( "UltraScan Default" ) );
  schemes->setCurrentRow( 0 );
  pb_delete->setEnabled( false );

  const QSettings settings( "UTHSCSA", "UltraScan" );
  QStringList schemeList = settings.value( "schemeList" ).toStringList();
  int row = 0;
  for ( const QString& scheme : schemeList )
  {
    schemes->addItem( scheme );
    if ( scheme == active )
    {
      schemes->setCurrentRow( row + 2  );
      pb_delete->setEnabled( true );
    }
    row++;
  }
}

void US_Color::delete_scheme( void )
{
  const int result = QMessageBox::warning( this,
        tr( "Attention:" ),
        tr( "Do you really want to delete the\n\"%1\" color scheme?" ).
            arg( schemes->currentItem()->text() ),
        QMessageBox::Yes, QMessageBox::No );

  // Note that the delete push button is disabled for schemes 0 and 1.
  if ( result == QMessageBox::Yes )
  {
    const QString s = schemes->currentItem()->text();

    QSettings   settings( "UTHSCSA", "UltraScan" );
    QStringList schemeList = settings.value( "schemeList" ).toStringList();
    schemeList.removeAll( s );
    settings.setValue( "schemeList", schemeList );

    const QString custom = "schemes/" + s + "/";

    settings.remove( custom + "frameColor"  );
    settings.remove( custom + "bannerColor" );
    settings.remove( custom + "pushbColor"  );
    settings.remove( custom + "labelColor"  );
    settings.remove( custom + "editColor"   );
    settings.remove( custom + "normalColor" );
    settings.remove( custom + "lcdColor"    );
    settings.remove( custom + "plotColor"   );

    settings.remove( custom + "plotMargin"  );

    settings.remove( custom + "plotCurve"   );
    settings.remove( custom + "plotBg"      );
    settings.remove( custom + "plotMajGrid" );
    settings.remove( custom + "plotMinGrid" );
    settings.remove( custom + "plotPicker"  );

    le_save_as->setText( "" );
    updateSchemeList();
  }
}

void US_Color::selected_scheme( void )
{
  switch ( schemes->currentRow() )
  {
    case 0: // Current scheme
      getCurrentSettings();
      pb_delete->setEnabled( false );
      break;

    case 1: // UltraScan default
      current.frameColor    = US_GuiSettings::frameColorDefault();
      current.bannerColor   = US_GuiSettings::bannerColorDefault();
      current.pushbColor    = US_GuiSettings::pushbColorDefault();
      current.labelColor    = US_GuiSettings::labelColorDefault();
      current.editColor     = US_GuiSettings::editColorDefault();
      current.normalColor   = US_GuiSettings::normalColorDefault();
      current.lcdColor      = US_GuiSettings::lcdColorDefault();
      current.plotColor     = US_GuiSettings::plotColorDefault();

      current.plotMargin    = 10;

      current.plotCurve     = US_GuiSettings::plotCurveDefault();
      current.plotBg        = US_GuiSettings::plotCanvasBGDefault();
      current.plotMajorGrid = US_GuiSettings::plotMajGridDefault();
      current.plotMinorGrid = US_GuiSettings::plotMinGridDefault();
      current.plotPicker    = US_GuiSettings::plotPickerDefault();
      pb_delete->setEnabled( false );
      break;

    default:
      const QString custom = "schemes/" + schemes->currentItem()->text() + "/";

      const QSettings settings( "UTHSCSA", "UltraScan" );
      current.frameColor    = settings.value( custom + "frameColor"  ).value<QPalette>();
      current.bannerColor   = settings.value( custom + "bannerColor",
                                              US_GuiSettings::bannerColorDefault()
                                            ).value<QPalette>();
      current.pushbColor    = settings.value( custom + "pushbColor"  ).value<QPalette>();
      current.labelColor    = settings.value( custom + "labelColor"  ).value<QPalette>();
      current.editColor     = settings.value( custom + "editColor"   ).value<QPalette>();
      current.normalColor   = settings.value( custom + "normalColor" ).value<QPalette>();
      current.lcdColor      = settings.value( custom + "lcdColor"    ).value<QPalette>();
      current.plotColor     = settings.value( custom + "plotColor"   ).value<QPalette>();

      current.plotMargin    = settings.value( custom + "plotMargin"  ).toInt();

      current.plotCurve     = settings.value( custom + "plotCurve"   ).value<QColor>();
      current.plotBg        = settings.value( custom + "plotBg"      ).value<QColor>();
      current.plotMajorGrid = settings.value( custom + "plotMajGrid" ).value<QColor>();
      current.plotMinorGrid = settings.value( custom + "plotMinGrid" ).value<QColor>();
      current.plotPicker    = settings.value( custom + "plotPicker"  ).value<QColor>();

      le_save_as->setText( schemes->currentItem()->text() );
      pb_delete->setEnabled( true );
  }

  updateScreen();
}

void US_Color::selectStyle( const int index )
{
  current.guiStyle = cmbb_style->itemText( index );

  const QString styleName = current.guiStyle;

  QApplication::setStyle  ( US_Theme::createStyle( styleName ) );
  QApplication::setPalette( US_Theme::applicationPalette() );
  qApp->setStyleSheet     ( US_Theme::styleSheet() );
  US_Theme::invalidate();
}

void US_Color::selectScheme( const int index )
{
  // The scheme decides which set of default colors US_GuiSettings hands out,
  // so it is stored right away and everything that is still at its default
  // is re-read from the new scheme.
  current.colorScheme = cmbb_scheme->itemData( index ).toString();

  US_Theme::set_schemeSetting( current.colorScheme );

  getCurrentSettings();
  updateScreen();
}

