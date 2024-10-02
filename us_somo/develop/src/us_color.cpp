#include "../include/us3_defines.h"
#include "../include/us_color.h"
#include "../include/us_hydrodyn.h"
//Added by qt3to4:
#include <QHBoxLayout>
#include <QLabel>
#include <QGridLayout>
#include <QFrame>
#include <QCloseEvent>

US_Color::US_Color(QWidget *p, const char* ) : QFrame( p )
{
  USglobal = new US_Config();
  set_default();
  save_str = us_tr("MyColors");
  current_widget = 0;
  current_index = USglobal->config_list.margin / 2 - 1;
  
  int xpos = 4, ypos = 4, width = 180, height = 26;
  double x[2], y[2];
  x[0] = 1.0;
  x[1] = 1.2;
  y[0] = 1;
  y[1] = 2;

#if QT_VERSION < 0x040000
  uint c1;
#endif

  setPalette( PALET_FRAME );
  setWindowTitle(us_tr("Color Configuration for UltraScan"));
  
  lbl_background = new QLabel(us_tr("Frame Background"),this);
  Q_CHECK_PTR(lbl_background);
  lbl_background->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  lbl_background->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
  lbl_background->setLineWidth(3);
  lbl_background->setFont(QFont( USglobal->config_list.fontFamily, 
                                 USglobal->config_list.fontSize, 
                                 QFont::Bold));
  lbl_background->setPalette( PALET_FRAME );
  AUTFBACK( lbl_background );


  plot = new QwtPlot(this);
#if QT_VERSION < 0x040000
  plot->enableOutline(true);
  plot->setOutlinePen(Qt::white);
  plot->setOutlineStyle(Qwt::Cross);
  plot->enableGridXMin();
  plot->enableGridYMin();
#else
  grid = new QwtPlotGrid;
  grid->enableXMin( true );
  grid->enableYMin( true );
#endif
  plot->setTitle(us_tr("Sample Plot"));
  plot->setPalette( PALET_NORMAL );
  AUTFBACK( plot );
#if QT_VERSION < 0x040000
  plot->setGridMajPen(QPen(temp_colors.major_ticks, 0, DotLine));
  plot->setGridMinPen(QPen(temp_colors.minor_ticks, 0, DotLine));
#else
  grid->setMajorPen( QPen( temp_colors.major_ticks, 0, Qt::DotLine ) );
  grid->setMinorPen( QPen( temp_colors.minor_ticks, 0, Qt::DotLine ) );
  grid->attach( plot );
#endif
  plot->setCanvasBackground(temp_colors.plot);    //new version
  plot->setAxisTitle(QwtPlot::xBottom, us_tr("X-axis"));
  plot->setAxisTitle(QwtPlot::yLeft, us_tr("Y-axis"));
//   plot->setMargin(USglobal->config_list.margin);
  plot->setMinimumSize(width*2-10, height*9);
#if QT_VERSION < 0x040000
  c1 = plot->insertCurve("Sample 1");
  plot->setCurveStyle(c1, QwtCurve::Lines);
  plot->setCurvePen(c1, Qt::yellow);
  plot->setCurveData(c1, x, y, 2);
#else
  curve = new QwtPlotCurve( "Sample 1" );
  curve->setStyle( QwtPlotCurve::Lines );
  curve->setPen( QPen( Qt::yellow ) );
  curve->setSamples( x, y, 2 );
  curve->attach( plot );
  plot->replot();
#endif
  plot->show();

  
  lbl_margin = new QLabel(us_tr(" Current Plot Border Margin:  "),this);
  Q_CHECK_PTR(lbl_margin);
  lbl_margin->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  lbl_margin->setFont(QFont( USglobal->config_list.fontFamily, 
                             USglobal->config_list.fontSize - 1, 
                             QFont::Bold));
  lbl_margin->setPalette( PALET_FRAME );
  AUTFBACK( lbl_margin );
  
  
  cmbb_margin = new QComboBox(  this );   cmbb_margin->setObjectName( "Margin Select" );
  cmbb_margin->setPalette( PALET_NORMAL );
  AUTFBACK( cmbb_margin );
  cmbb_margin->setFont(QFont(USglobal->config_list.fontFamily, 
                             USglobal->config_list.fontSize - 1));
  QString str_number;
  for (int i=1; i<=15; i++)   //plot margin range from 2 to 30
  {  
    str_number.sprintf("%d  pixel", i*2);
    cmbb_margin->addItem(str_number);
  }
  cmbb_margin->setCurrentIndex(current_index);
  connect(cmbb_margin, SIGNAL(activated(int)), SLOT(sel_margin(int)));
  
  lbl_example = new QLabel(us_tr("Label and Button Examples : "),this);
  Q_CHECK_PTR(lbl_example);
  lbl_example->setFrameStyle(QFrame::WinPanel|Raised);
  lbl_example->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  lbl_example->setFont(QFont( USglobal->config_list.fontFamily, 
                              USglobal->config_list.fontSize - 1, QFont::Bold));
  lbl_example->setPalette( PALET_FRAME );
  AUTFBACK( lbl_example );
  lbl_example->setMinimumHeight(30);
    
  lbl_banner = new QLabel(us_tr("Banner Label"),this);
  Q_CHECK_PTR(lbl_banner);
  lbl_banner->setFrameStyle(QFrame::WinPanel|Raised);
  lbl_banner->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  lbl_banner->setFont(QFont( USglobal->config_list.fontFamily, 
                             USglobal->config_list.fontSize, 
                             QFont::Bold));
  lbl_banner->setPalette( PALET_FRAME );
  AUTFBACK( lbl_banner );
  

  lbl_progress = new QLabel(us_tr("Progress Bar:"),this);
  Q_CHECK_PTR(lbl_progress);
  lbl_progress->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_progress->setFont(QFont( USglobal->config_list.fontFamily, 
                               USglobal->config_list.fontSize - 1, 
                               QFont::Bold));
  lbl_progress->setPalette( PALET_LABEL );
  AUTFBACK( lbl_progress );


  progress = new QProgressBar( this ); progress->setMaximum( 100 );
  Q_CHECK_PTR(progress);
  progress->setPalette( PALET_NORMAL );
  AUTFBACK( progress );
  progress->setFont(QFont( USglobal->config_list.fontFamily, 
                           USglobal->config_list.fontSize - 1, 
                           QFont::Bold));
  progress->setValue(35);

  lbl_edit = new QLabel(us_tr("Edit Label"),this);
  Q_CHECK_PTR(lbl_edit);
  lbl_edit->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
  lbl_edit->setFont(QFont( USglobal->config_list.fontFamily, 
                           USglobal->config_list.fontSize - 1, 
                           QFont::Bold));
  lbl_edit->setPalette( PALET_EDIT );
  AUTFBACK( lbl_edit );

  lbl_lcd = new QLabel(us_tr("LCD Panel:"),this);
  Q_CHECK_PTR(lbl_lcd);
  lbl_lcd->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_lcd->setFont(QFont( USglobal->config_list.fontFamily, 
                          USglobal->config_list.fontSize - 1, 
                          QFont::Bold));
  lbl_lcd->setPalette( PALET_LABEL );
  AUTFBACK( lbl_lcd );

  lcd = new QLCDNumber(  7, this );   lcd->setObjectName( "lcd" );
  lcd->setPalette( temp_colors.cg_lcd );
  lcd->setGeometry(xpos, ypos, width - 26, height);
  lcd->setFont(QFont( USglobal->config_list.fontFamily, 
                      USglobal->config_list.fontSize - 1, 
                      QFont::Bold));
  lcd->setSegmentStyle ( QLCDNumber::Filled );
  lcd->setMode( QLCDNumber::Dec );
  lcd->display(35);

  lbl_text = new QLabel(us_tr("Text Label"),this);
  Q_CHECK_PTR(lbl_text);
  lbl_text->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
  lbl_text->setFont(QFont( USglobal->config_list.fontFamily, 
                           USglobal->config_list.fontSize - 1, 
                           QFont::Bold));
  lbl_text->setPalette( PALET_LABEL );
  AUTFBACK( lbl_text );

  lbl_counter = new QLabel(us_tr("Counter:"),this);
  Q_CHECK_PTR(lbl_counter);
  lbl_counter->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_counter->setFont(QFont( USglobal->config_list.fontFamily, 
                              USglobal->config_list.fontSize - 1, 
                              QFont::Bold));
  lbl_counter->setPalette( PALET_LABEL );
  AUTFBACK( lbl_counter );

  cnt = new QwtCounter(this);
  US_Hydrodyn::sizeArrows( cnt );
  Q_CHECK_PTR(cnt);
  cnt->setNumButtons(2);
  cnt->setRange(0, 100); cnt->setSingleStep( 1);
  cnt->setPalette( PALET_NORMAL );
  AUTFBACK( cnt );
  cnt->setValue(35);
  cnt->setFont(QFont( USglobal->config_list.fontFamily, 
                      USglobal->config_list.fontSize - 1, 
                      QFont::Bold));
  connect(cnt, SIGNAL(buttonReleased(double)), SLOT(update_widgets(double)));
  connect(cnt, SIGNAL(valueChanged(double)), SLOT(update_widgets(double)));
  

  pb_normal = new QPushButton(us_tr("Pushbutton"), this);
  Q_CHECK_PTR(pb_normal);
  pb_normal->setAutoDefault(false);
  pb_normal->setFont(QFont( USglobal->config_list.fontFamily, 
                            USglobal->config_list.fontSize));
  pb_normal->setPalette( PALET_PUSHB );


  pb_active = new QPushButton(us_tr("Active"), this);
  Q_CHECK_PTR(pb_active);
  pb_active->setAutoDefault(false);
  pb_active->setDown(true);
  pb_active->setFont(QFont( USglobal->config_list.fontFamily, 
                            USglobal->config_list.fontSize));
  pb_active->setPalette( PALET_PUSHB );
  pb_active->setGeometry(xpos, ypos, width/2, height);

  pb_disabled = new QPushButton(us_tr("Disabled"), this);
  Q_CHECK_PTR(pb_disabled);
  pb_disabled->setAutoDefault(false);
  pb_disabled->setFont(QFont( USglobal->config_list.fontFamily, 
                              USglobal->config_list.fontSize));
  pb_disabled->setEnabled(false);
  pb_disabled->setPalette( PALET_PUSHB );

  pb_reset = new QPushButton(us_tr("Reset"), this);
  Q_CHECK_PTR(pb_reset);
  pb_reset->setAutoDefault(false);
  pb_reset->setFont(QFont( USglobal->config_list.fontFamily, 
                           USglobal->config_list.fontSize));
  pb_reset->setPalette( PALET_PUSHB );
  connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));

  pb_help = new QPushButton(us_tr("Help"), this);
  Q_CHECK_PTR(pb_help);
  pb_help->setAutoDefault(false);
  pb_help->setFont(QFont( USglobal->config_list.fontFamily, 
                          USglobal->config_list.fontSize));
  pb_help->setPalette( PALET_PUSHB );
  connect(pb_help, SIGNAL(clicked()), SLOT(help()));


  pb_apply = new QPushButton(us_tr("Apply"), this);
  Q_CHECK_PTR(pb_apply);
  pb_apply->setAutoDefault(false);
  pb_apply->setFont(QFont( USglobal->config_list.fontFamily, 
                           USglobal->config_list.fontSize));
  pb_apply->setPalette( PALET_PUSHB );
  connect(pb_apply, SIGNAL(clicked()), SLOT(apply()));

  pb_quit = new QPushButton(us_tr("Close"), this);
  Q_CHECK_PTR(pb_quit);
  pb_quit->setAutoDefault(false);
  pb_quit->setFont(QFont( USglobal->config_list.fontFamily, 
                          USglobal->config_list.fontSize));
  pb_quit->setPalette( PALET_PUSHB );
  connect(pb_quit, SIGNAL(clicked()), SLOT(quit()));

  pb_delete = new QPushButton(us_tr("Delete"), this);
  Q_CHECK_PTR(pb_delete);
  pb_delete->setAutoDefault(false);
  pb_delete->setFont(QFont( USglobal->config_list.fontFamily, 
                            USglobal->config_list.fontSize));
  pb_delete->setPalette( PALET_PUSHB );
  connect(pb_delete, SIGNAL(clicked()), SLOT(delete_scheme()));


  lbl_assign = new QLabel(us_tr("Assign new Colors:"),this);
  Q_CHECK_PTR(lbl_assign);
  lbl_assign->setFrameStyle(QFrame::WinPanel|Raised);
  lbl_assign->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
  lbl_assign->setFont(QFont( USglobal->config_list.fontFamily, 
                             USglobal->config_list.fontSize, 
                             QFont::Bold));
  lbl_assign->setPalette( PALET_FRAME );
  AUTFBACK( lbl_assign );
  lbl_assign->setMinimumHeight(30);


  lbl_select = new QLabel(us_tr("Selected Screen Element:"),this);
  Q_CHECK_PTR(lbl_select);
  lbl_select->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
  lbl_select->setFont(QFont( USglobal->config_list.fontFamily, 
                             USglobal->config_list.fontSize - 1, 
                             QFont::Bold));
  lbl_select->setPalette( PALET_LABEL );
  AUTFBACK( lbl_select );
  
  lbl_choice = new QLabel("",this);
  Q_CHECK_PTR(lbl_choice);
  lbl_choice->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
  lbl_choice->setFont(QFont( USglobal->config_list.fontFamily, 
                             USglobal->config_list.fontSize - 1, 
                             QFont::Bold));
  lbl_choice->setPalette( PALET_EDIT );
  AUTFBACK( lbl_choice );


  lbl_color1 = new QLabel("",this);
  Q_CHECK_PTR(lbl_color1);
  lbl_color1->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
  lbl_color1->setFont(QFont( USglobal->config_list.fontFamily, 
                             USglobal->config_list.fontSize - 1, 
                             QFont::Bold));
  lbl_color1->setPalette( PALET_LABEL );
  AUTFBACK( lbl_color1 );
  
  color_field1 = new QLabel("", this);
  Q_CHECK_PTR(color_field1);
  color_field1->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
  color_field1->setFixedSize(22,22);
  
  pb_color1 = new QPushButton(us_tr("Change"), this);
  Q_CHECK_PTR(pb_color1);
  pb_color1->setAutoDefault(false);
  pb_color1->setFont(QFont( USglobal->config_list.fontFamily, 
                            USglobal->config_list.fontSize));
  pb_color1->setPalette( PALET_PUSHB );
  connect(pb_color1, SIGNAL(clicked()), SLOT(pick_color1()));

  lbl_color2 = new QLabel("",this);
  Q_CHECK_PTR(lbl_color2);
  lbl_color2->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
  lbl_color2->setFont(QFont( USglobal->config_list.fontFamily, 
                             USglobal->config_list.fontSize - 1, 
                             QFont::Bold));
  lbl_color2->setPalette( PALET_LABEL );
  AUTFBACK( lbl_color2 );

  color_field2 = new QLabel("", this);
  Q_CHECK_PTR(color_field2);
  color_field2->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
  color_field2->setFixedSize(22,22);

  pb_color2 = new QPushButton(us_tr("Change"), this);
  Q_CHECK_PTR(pb_color2);
  pb_color2->setAutoDefault(false);
  pb_color2->setFont(QFont( USglobal->config_list.fontFamily, 
                            USglobal->config_list.fontSize));
  pb_color2->setPalette( PALET_PUSHB );
  connect(pb_color2, SIGNAL(clicked()), SLOT(pick_color2()));

  lbl_color3 = new QLabel("",this);
  Q_CHECK_PTR(lbl_color3);
  lbl_color3->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
  lbl_color3->setFont(QFont( USglobal->config_list.fontFamily, 
                             USglobal->config_list.fontSize - 1, 
                             QFont::Bold));
  lbl_color3->setPalette( PALET_LABEL );
  AUTFBACK( lbl_color3 );

  color_field3 = new QLabel("", this);
  Q_CHECK_PTR(color_field3);
  color_field3->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
  color_field3->setFixedSize(22,22);
  
  pb_color3 = new QPushButton(us_tr("Change"), this);
  Q_CHECK_PTR(pb_color3);
  pb_color3->setAutoDefault(false);
  pb_color3->setFont(QFont( USglobal->config_list.fontFamily, 
                            USglobal->config_list.fontSize));
  pb_color3->setPalette( PALET_PUSHB );
  connect(pb_color3, SIGNAL(clicked()), SLOT(pick_color3()));

  lbl_color4 = new QLabel("",this);
  Q_CHECK_PTR(lbl_color4);
  lbl_color4->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
  lbl_color4->setFont(QFont( USglobal->config_list.fontFamily, 
                             USglobal->config_list.fontSize - 1, 
                             QFont::Bold));
  lbl_color4->setPalette( PALET_LABEL );
  AUTFBACK( lbl_color4 );

  color_field4 = new QLabel("", this);
  Q_CHECK_PTR(color_field4);
  color_field4->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
  color_field4->setFixedSize(22,22);


  pb_color4 = new QPushButton(us_tr("Change"), this);
  Q_CHECK_PTR(pb_color4);
  pb_color4->setAutoDefault(false);
  pb_color4->setFont(QFont( USglobal->config_list.fontFamily, 
                            USglobal->config_list.fontSize));
  pb_color4->setPalette( PALET_PUSHB );
  connect(pb_color4, SIGNAL(clicked()), SLOT(pick_color4()));

  lbl_color5 = new QLabel("",this);
  Q_CHECK_PTR(lbl_color5);
  lbl_color5->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
  lbl_color5->setFont(QFont( USglobal->config_list.fontFamily, 
                             USglobal->config_list.fontSize - 1, 
                             QFont::Bold));
  lbl_color5->setPalette( PALET_LABEL );
  AUTFBACK( lbl_color5 );
  
  color_field5 = new QLabel("", this);
  Q_CHECK_PTR(color_field5);
  color_field5->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
  color_field5->setFixedSize(22,22);
  

  pb_color5 = new QPushButton(us_tr("Change"), this);
  Q_CHECK_PTR(pb_color5);
  pb_color5->setAutoDefault(false);
  pb_color5->setFont(QFont( USglobal->config_list.fontFamily, 
                            USglobal->config_list.fontSize));
  pb_color5->setPalette( PALET_PUSHB );
  connect(pb_color5, SIGNAL(clicked()), SLOT(pick_color5()));

  lbl_color6 = new QLabel("",this);
  Q_CHECK_PTR(lbl_color6);
  lbl_color6->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
  lbl_color6->setFont(QFont( USglobal->config_list.fontFamily, 
                             USglobal->config_list.fontSize - 1, 
                             QFont::Bold));
  lbl_color6->setPalette( PALET_LABEL );
  AUTFBACK( lbl_color6 );

  color_field6 = new QLabel("", this);
  Q_CHECK_PTR(color_field6);
  color_field6->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
  color_field6->setFixedSize(22,22);
  
  pb_color6 = new QPushButton(us_tr("Change"), this);
  Q_CHECK_PTR(pb_color6);
  pb_color6->setAutoDefault(false);
  pb_color6->setFont(QFont( USglobal->config_list.fontFamily, 
                            USglobal->config_list.fontSize));
  pb_color6->setPalette( PALET_PUSHB );
  connect(pb_color6, SIGNAL(clicked()), SLOT(pick_color6()));

  lbl_choices = new QLabel(us_tr("Please Make a Selection:"),this);
  Q_CHECK_PTR(lbl_choices);
  lbl_choices->setFrameStyle(QFrame::WinPanel|Raised);
  lbl_choices->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
  lbl_choices->setFont(QFont( USglobal->config_list.fontFamily, 
                              USglobal->config_list.fontSize, 
                              QFont::Bold));
  lbl_choices->setPalette( PALET_FRAME );
  AUTFBACK( lbl_choices );
  lbl_choices->setMinimumHeight(30);

  lbl_select_scheme = new QLabel(us_tr("Select Color Scheme:"),this);
  Q_CHECK_PTR(lbl_select_scheme);
  lbl_select_scheme->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
  lbl_select_scheme->setFont(QFont( USglobal->config_list.fontFamily, 
                                    USglobal->config_list.fontSize - 1, 
                                    QFont::Bold));
  lbl_select_scheme->setPalette( PALET_LABEL );
  AUTFBACK( lbl_select_scheme );
  
  lbl_select_element = new QLabel(us_tr("Select a Screen Element:"),this);
  Q_CHECK_PTR(lbl_select_element);
  lbl_select_element->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
  lbl_select_element->setFont(QFont( USglobal->config_list.fontFamily, 
                                     USglobal->config_list.fontSize - 1, 
                                     QFont::Bold));
  lbl_select_element->setPalette( PALET_LABEL );
  AUTFBACK( lbl_select_element );
  
  QDir scheme_dir1, scheme_dir2;
  QString str, filter = "*.col";
  QStringList entries1, entries2;

  QString etc = QDir::toNativeSeparators( USglobal->config_list.system_dir + "/etc");

  scheme_dir1.setPath( etc ); // Look for system wide color defs
  scheme_dir2.setPath(USglobal->config_list.root_dir);      // personal color defs
  
  scheme_dir1.setNameFilters( QStringList() << filter );
  scheme_dir2.setNameFilters( QStringList() << filter );
  entries1 = scheme_dir1.entryList();
  entries2 = scheme_dir2.entryList();

  for (QStringList::Iterator it = entries2.begin(); it != entries2.end(); ++it )
  {
    str = (*it).toLatin1().data();
    entries1 += str;      // combine the personal and the system wide color defs into a single list
  }
  
  entries1.sort();
  
  lb_scheme = new QListWidget( this );
  Q_CHECK_PTR(lb_scheme);
  lb_scheme->addItem(us_tr("Current Scheme"));
  lb_scheme->addItem(us_tr("UltraScan Default"));
  for (QStringList::Iterator it = entries1.begin(); it != entries1.end(); ++it )
  {
    str = (*it).toLatin1().data();
    str.truncate(str.lastIndexOf(".", -1, Qt::CaseInsensitive ));  // strip the extension
    if (str != us_tr("UltraScan Default"))
    {
      lb_scheme->addItem(str);
    }
  }
  lb_scheme->setPalette( PALET_NORMAL );
  AUTFBACK( lb_scheme );
  lb_scheme->setCurrentItem( lb_scheme->item(0) );
  lb_scheme->setFont(QFont( USglobal->config_list.fontFamily, 
                            USglobal->config_list.fontSize - 1, 
                            QFont::Bold));
  current_scheme = 0;

  connect(lb_scheme, SIGNAL(highlighted(int)), this, SLOT(selected_scheme(int)));
  connect(lb_scheme, SIGNAL(currentRowChanged(int)), this, SLOT(selected_scheme(int)));

  lb_item = new QListWidget( this );
  Q_CHECK_PTR(lb_item);
  lb_item->addItem(us_tr("Frame and Banner"));
  lb_item->addItem(us_tr("Pushbutton, normal"));
  lb_item->addItem(us_tr("Pushbutton, disabled"));
  lb_item->addItem(us_tr("Plot Frame"));
  lb_item->addItem(us_tr("Plot Background"));
  lb_item->addItem(us_tr("Plot Major Tickmarks"));
  lb_item->addItem(us_tr("Plot Minor Tickmarks"));
  lb_item->addItem(us_tr("Edit Label"));
  lb_item->addItem(us_tr("Text Label"));
  lb_item->addItem(us_tr("Widget Colors"));
  lb_item->addItem(us_tr("LCD Panel"));
  
  lb_item->setPalette( PALET_NORMAL );
  AUTFBACK( lb_item );
  lb_item->setFont(QFont( USglobal->config_list.fontFamily, 
                          USglobal->config_list.fontSize - 1, 
                          QFont::Bold));

  connect(lb_item, SIGNAL(highlighted(int)), this, SLOT(selected_item(int)));
  lb_item->setCurrentItem( lb_item->item(0) );
  
  pb_save_as = new QPushButton(us_tr("Save Color Scheme as:"), this);
  Q_CHECK_PTR(pb_save_as);
  pb_save_as->setAutoDefault(false);
  pb_save_as->setFont(QFont( USglobal->config_list.fontFamily, 
                             USglobal->config_list.fontSize));
  pb_save_as->setPalette( PALET_PUSHB );
  connect(pb_save_as, SIGNAL(clicked()), SLOT(save_as()));

  le_save_as = new QLineEdit(save_str, this);
  Q_CHECK_PTR(le_save_as);
  le_save_as->setFont(QFont( USglobal->config_list.fontFamily, 
                             USglobal->config_list.fontSize - 1, 
                             QFont::Bold));
  le_save_as->setPalette( PALET_NORMAL );
  AUTFBACK( le_save_as );
  connect(le_save_as, SIGNAL(textChanged(const QString &)), SLOT(update_save_str(const QString &)));
  connect(le_save_as, SIGNAL(returnPressed(void)), SLOT(save_as(void)));
  
  setup_GUI();
}

US_Color::~US_Color()
{
}

void US_Color::setup_GUI()
{
  int j=0;
  int rows = 8, /* columns = 3,*/ spacing = 2;
  
  QGridLayout * background = new QGridLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 ); background->setSpacing( spacing*2 ); background->setContentsMargins( spacing*2, spacing*2, spacing*2, spacing*2 ); 
  QGridLayout * subGrid1 = new QGridLayout; subGrid1->setContentsMargins( 0, 0, 0, 0 ); subGrid1->setSpacing( 0 ); subGrid1->setSpacing( spacing ); subGrid1->setContentsMargins( spacing, spacing, spacing, spacing );
  for (int i=0; i<rows; i++)
  {
    subGrid1->setRowMinimumHeight(i, 26);
  }
  subGrid1->addWidget( lbl_background , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 2 ) - ( 0 ) );
  j++;
  subGrid1->addWidget( plot , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 2 ) - ( 0 ) );
  j++;
  subGrid1->addWidget( lbl_margin , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
  subGrid1->addWidget(cmbb_margin,j,2);
  j++;
  subGrid1->addWidget( lbl_example , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 2 ) - ( 0 ) );
  j++;
  QHBoxLayout * line1 = new QHBoxLayout(); line1->setContentsMargins( 0, 0, 0, 0 ); line1->setSpacing( 0 );
  line1->addWidget(lbl_banner);
  line1->addWidget(lbl_edit);
  line1->addWidget(lbl_text);
  subGrid1->addLayout( line1 , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 2 ) - ( 0 ) );
  j++;
  QHBoxLayout * line2 = new QHBoxLayout(); line2->setContentsMargins( 0, 0, 0, 0 ); line2->setSpacing( 0 );
  line2->addWidget(pb_normal);
  line2->addWidget(pb_active);
  line2->addWidget(pb_disabled);
  subGrid1->addLayout( line2 , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 2 ) - ( 0 ) );
  j++;

  subGrid1->addWidget( lbl_progress , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
  subGrid1->addWidget(progress,j,2);
  j++;
  subGrid1->addWidget( lbl_lcd , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
  subGrid1->addWidget(lcd,j,2);
  j++;
  subGrid1->addWidget( lbl_counter , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
  subGrid1->addWidget(cnt,j,2);

/*    
  subGrid1->addWidget(lbl_banner,j,0);
  subGrid1->addWidget(lbl_progress,j,1);
  subGrid1->addWidget(progress,j,2);
  j++;
  subGrid1->addWidget(lbl_edit,j,0);
  subGrid1->addWidget(lbl_lcd,j,1);
  subGrid1->addWidget(lcd,j,2);
  j++;
  subGrid1->addWidget(lbl_text,j,0);
  subGrid1->addWidget(lbl_counter,j,1);
  subGrid1->addWidget(cnt,j,2);
  j++;
  subGrid1->addWidget(pb_normal,j,0);
  subGrid1->addWidget(pb_active,j,1);
  subGrid1->addWidget(pb_disabled,j,2);
*/  
  QHBoxLayout * button = new QHBoxLayout(); button->setContentsMargins( 0, 0, 0, 0 ); button->setSpacing( 0 );
  button->addWidget(pb_reset);
  button->addWidget(pb_help);
  button->addWidget(pb_delete);
  button->addWidget(pb_apply);
  button->addWidget(pb_quit);
//  subGrid1->addLayout( button , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 2 ) - ( 0 ) );
  
  j=0;
  rows = 8, /* columns = 2,*/ spacing = 2;

  QGridLayout * subGrid2 = new QGridLayout; subGrid2->setContentsMargins( 0, 0, 0, 0 ); subGrid2->setSpacing( 0 ); subGrid2->setSpacing( spacing ); subGrid2->setContentsMargins( spacing, spacing, spacing, spacing );
  for (int i=0; i<rows; i++)
  {
    subGrid2->setRowMinimumHeight(i, 26);
  }
  subGrid2->addWidget( lbl_assign , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
  j++;
  subGrid2->addWidget(lbl_select,j,0);
  subGrid2->addWidget(lbl_choice,j,1);
  j++;
  QHBoxLayout * color1 = new QHBoxLayout(); color1->setContentsMargins( 0, 0, 0, 0 ); color1->setSpacing( 0 );
  color1->addWidget(lbl_color1);
  color1->addWidget(color_field1);
  subGrid2->addLayout(color1,j,0);
  subGrid2->addWidget(pb_color1,j,1);
  j++;
  QHBoxLayout * color2 = new QHBoxLayout(); color2->setContentsMargins( 0, 0, 0, 0 ); color2->setSpacing( 0 );
  color2->addWidget(lbl_color2);
  color2->addWidget(color_field2);
  subGrid2->addLayout(color2,j,0);
  subGrid2->addWidget(pb_color2,j,1);
  j++;
  QHBoxLayout * color3 = new QHBoxLayout(); color3->setContentsMargins( 0, 0, 0, 0 ); color3->setSpacing( 0 );
  color3->addWidget(lbl_color3);
  color3->addWidget(color_field3);
  subGrid2->addLayout(color3,j,0);
  subGrid2->addWidget(pb_color3,j,1);
  j++;
  QHBoxLayout * color4 = new QHBoxLayout(); color4->setContentsMargins( 0, 0, 0, 0 ); color4->setSpacing( 0 );
  color4->addWidget(lbl_color4);
  color4->addWidget(color_field4);
  subGrid2->addLayout(color4,j,0);
  subGrid2->addWidget(pb_color4,j,1);
  j++;
  QHBoxLayout * color5 = new QHBoxLayout(); color5->setContentsMargins( 0, 0, 0, 0 ); color5->setSpacing( 0 );
  color5->addWidget(lbl_color5);
  color5->addWidget(color_field5);
  subGrid2->addLayout(color5,j,0);
  subGrid2->addWidget(pb_color5,j,1);
  j++;
  QHBoxLayout * color6 = new QHBoxLayout(); color6->setContentsMargins( 0, 0, 0, 0 ); color6->setSpacing( 0 );
  color6->addWidget(lbl_color6);
  color6->addWidget(color_field6);
  subGrid2->addLayout(color6,j,0);
  subGrid2->addWidget(pb_color6,j,1);
  j++;
  subGrid2->addWidget( lbl_choices , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
  j++;
  subGrid2->addWidget(lbl_select_scheme,j,0);
  subGrid2->addWidget(lbl_select_element,j,1);
  j++;
  subGrid2->addWidget(lb_scheme,j,0);
  subGrid2->addWidget(lb_item,j,1);
  j++;
  subGrid2->addWidget(pb_save_as,j,0);
  subGrid2->addWidget(le_save_as,j,1);

  background->addLayout(subGrid1,0,0);
  background->setColumnStretch(0,2);
  background->addLayout(subGrid2,0,1);
background->addLayout( button , 1 , 0 , 1 + ( 1 ) - ( 1 ) , 1 + ( 1 ) - ( 0 ) );
/*
  qApp->processEvents();
  QRect r = background->cellGeometry(0, 0);
  
  global_Xpos += 30;
  global_Ypos += 30;
  
  this->setGeometry(global_Xpos, global_Ypos, r.width(), r.height());
*/
}

void US_Color::closeEvent(QCloseEvent *e)
{
  e->accept();
  global_Xpos -= 30;
  global_Ypos -= 30;
}


void US_Color::set_default()
{
  temp_colors = USglobal->global_colors;    // called when temp_colors are 
                                            // initialized with setting from US_Config
  temp_margin = USglobal->config_list.margin;
}

void US_Color::selected_scheme(int scheme)
{
  QString colfile, selected_scheme;
  QFile f;
  current_scheme = scheme;
  selected_scheme = lb_scheme->item(scheme)->text();

  if (selected_scheme == "UltraScan Default")
  {
    USglobal->color_defaults();
    temp_colors = USglobal->global_colors;
    apply();
    return;
  }
  
  if ( scheme == 0 )
  {
    QString colfile = US_Config::get_home_dir() + USCOLORS;
    f.setFileName(colfile);
  }
  else
  {
    colfile = USglobal->config_list.system_dir + "/etc/" + selected_scheme + ".col";
    colfile = QDir::toNativeSeparators( colfile);

    f.setFileName(colfile);
    
    if (! f.exists())    // if we can't find it in the system wide 
                         // shared directory, check the home directory
    {
      colfile = USglobal->config_list.root_dir + "/" + selected_scheme + ".col";
      colfile = QDir::toNativeSeparators( colfile);
      f.setFileName(colfile);
    }
  }
  if (f.exists())
  {
    if(f.open(QIODevice::ReadOnly))
    {
      QDataStream ds(&f);
      ds >> temp_colors.pushb;
      ds >> temp_colors.frames;
      ds >> temp_colors.labels;
      ds >> temp_colors.pushb_active;
      ds >> temp_colors.dk_red;
      ds >> temp_colors.dk_green;
      ds >> temp_colors.plot;
      ds >> temp_colors.major_ticks;
      ds >> temp_colors.minor_ticks;
      ds >> temp_colors.gray;
      ds >> temp_colors.gray1;
      ds >> temp_colors.gray2;
      ds >> temp_colors.gray3;
      ds >> temp_colors.gray4;
      ds >> temp_colors.gray5;
      ds >> temp_colors.gray6;
      ds >> temp_colors.gray7;
      ds >> temp_colors.gray8;
      ds >> temp_colors.gray9;
      ds >> temp_colors.cg_frame;
      ds >> temp_colors.cg_pushb;
      ds >> temp_colors.cg_pushb_active;
      ds >> temp_colors.cg_pushb_disabled;
      ds >> temp_colors.cg_label;
      ds >> temp_colors.cg_label_disabled;
      ds >> temp_colors.cg_label_warn;
      ds >> temp_colors.cg_edit;
      ds >> temp_colors.cg_edit_warn;
      ds >> temp_colors.cg_dk_red;
      ds >> temp_colors.cg_dk_green;
      ds >> temp_colors.cg_red;
      ds >> temp_colors.cg_green;
      ds >> temp_colors.cg_gray;
      ds >> temp_colors.cg_normal;
      ds >> temp_colors.cg_plot;
      ds >> temp_colors.cg_lcd;
      ds >> temp_colors.cg_bunt;
      f.close();
      lcd->setPalette( temp_colors.cg_lcd );
      cnt->setPalette( PALET_NORMAL );
      AUTFBACK( cnt );
      lbl_text->setPalette( PALET_LABEL );
      AUTFBACK( lbl_text );
      lbl_edit->setPalette( PALET_EDIT );
      AUTFBACK( lbl_edit );
      cmbb_margin->setPalette( PALET_NORMAL );
      AUTFBACK( cmbb_margin );
      progress->setPalette( PALET_NORMAL );
      AUTFBACK( progress );
#if QT_VERSION < 0x040000
      plot->setGridMinPen(QPen(temp_colors.minor_ticks, 0, DotLine));
      plot->setGridMajPen(QPen(temp_colors.major_ticks, 0, DotLine));
#else
      grid->setMajorPen( QPen( temp_colors.major_ticks, 0, Qt::DotLine ) );
      grid->setMinorPen( QPen( temp_colors.minor_ticks, 0, Qt::DotLine ) );
#endif
      //plot->setPlotBackground(temp_colors.plot);    //old version
      plot->setCanvasBackground(temp_colors.plot);    //new version
      plot->setPalette( PALET_NORMAL );
      AUTFBACK( plot );
      lbl_background->setPalette( PALET_FRAME );
      AUTFBACK( lbl_background );
      lbl_margin->setPalette( PALET_FRAME );
      AUTFBACK( lbl_margin );
      lbl_progress->setPalette( PALET_FRAME );
      AUTFBACK( lbl_progress );
      lbl_lcd->setPalette( PALET_FRAME );
      AUTFBACK( lbl_lcd );
      lbl_counter->setPalette( PALET_FRAME );
      AUTFBACK( lbl_counter );
      lbl_banner->setPalette( PALET_FRAME );
      AUTFBACK( lbl_banner );
      pb_normal->setPalette( PALET_PUSHB );
      pb_active->setPalette( PALET_PUSHB );
      pb_disabled->setPalette( temp_colors.cg_pushb_disabled );
      selected_item(current_widget);
    }
  }
}

void US_Color::selected_item(int item)
{
  current_widget = item;
  lbl_choice->setText(" " + lb_item->item(item)->text());
  switch (item)
  {
    case 0:
    {
      pb_color1->setEnabled(true);
      pb_color2->setEnabled(true);
      pb_color3->setEnabled(true);
      pb_color4->setEnabled(true);
      pb_color5->setEnabled(true);
      pb_color6->setEnabled(true);
      temp_color1 = temp_colors.cg_frame.windowText().color();
      temp_color2 = temp_colors.cg_frame.window().color();
      temp_color3 = temp_colors.cg_frame.light().color();
      temp_color4 = temp_colors.cg_frame.dark().color();
      temp_color5 = temp_colors.cg_frame.midlight().color();
      temp_color6 = temp_colors.cg_frame.shadow().color();
      lbl_color1->setText(us_tr(" Text Color:"));
      { QPalette palette; palette.setColor(color_field1->backgroundRole(), temp_colors.cg_frame.windowText().color()); color_field1->setPalette(palette); }
      lbl_color2->setText(us_tr(" Background Color:"));
      { QPalette palette; palette.setColor(color_field2->backgroundRole(), temp_colors.cg_frame.window().color()); color_field2->setPalette(palette); }
      lbl_color3->setText(us_tr(" Highlight Color:"));
      { QPalette palette; palette.setColor(color_field3->backgroundRole(), temp_colors.cg_frame.light().color()); color_field3->setPalette(palette); }
      lbl_color4->setText(us_tr(" Medium Shadow Color:"));
      { QPalette palette; palette.setColor(color_field4->backgroundRole(), temp_colors.cg_frame.dark().color()); color_field4->setPalette(palette); }
      lbl_color5->setText(us_tr(" Medium Highlight Color:"));
      { QPalette palette; palette.setColor(color_field5->backgroundRole(), temp_colors.cg_frame.midlight().color()); color_field5->setPalette(palette); }
      lbl_color6->setText(us_tr(" Dark Shadow Color:"));
      { QPalette palette; palette.setColor(color_field6->backgroundRole(), temp_colors.cg_frame.shadow().color()); color_field6->setPalette(palette); }
      break;
    }
    case 1:
    {
      pb_color1->setEnabled(true);
      pb_color2->setEnabled(true);
      pb_color3->setEnabled(true);
      pb_color4->setEnabled(true);
      pb_color5->setEnabled(true);
      pb_color6->setEnabled(false);
      temp_color1 = temp_colors.cg_pushb.buttonText().color();
      temp_color2 = temp_colors.cg_pushb.button().color();
      temp_color3 = temp_colors.cg_pushb.light().color();
      temp_color4 = temp_colors.cg_pushb.dark().color();
      temp_color5 = temp_colors.cg_pushb.mid().color();
      lbl_color1->setText(us_tr(" Text Color:"));
      { QPalette palette; palette.setColor(color_field1->backgroundRole(), temp_colors.cg_pushb.text().color()); color_field1->setPalette(palette); }
      lbl_color2->setText(us_tr(" Normal Button Color:"));
      { QPalette palette; palette.setColor(color_field2->backgroundRole(), temp_colors.cg_pushb.button().color()); color_field2->setPalette(palette); }
      lbl_color3->setText(us_tr(" Highlight Color:"));
      { QPalette palette; palette.setColor(color_field3->backgroundRole(), temp_colors.cg_pushb.light().color()); color_field3->setPalette(palette); }
      lbl_color4->setText(us_tr(" Shadow Color:"));
      { QPalette palette; palette.setColor(color_field4->backgroundRole(), temp_colors.cg_pushb.dark().color()); color_field4->setPalette(palette); }
      lbl_color5->setText(us_tr(" Active Button Color:"));
      { QPalette palette; palette.setColor(color_field5->backgroundRole(), temp_colors.cg_pushb.mid().color()); color_field5->setPalette(palette); }
      lbl_color6->setText("");
      { QPalette palette; palette.setColor(color_field6->backgroundRole(), Qt::gray); color_field6->setPalette(palette); }
      break;
    }
    case 2:
    {
      pb_color1->setEnabled(true);
      pb_color2->setEnabled(true);
      pb_color3->setEnabled(true);
      pb_color4->setEnabled(true);
      pb_color5->setEnabled(false);
      pb_color6->setEnabled(false);
      temp_color1 = temp_colors.cg_pushb_disabled.buttonText().color();
      temp_color2 = temp_colors.cg_pushb_disabled.button().color();
      temp_color3 = temp_colors.cg_pushb_disabled.light().color();
      temp_color4 = temp_colors.cg_pushb_disabled.dark().color();
      lbl_color1->setText(us_tr(" Text Color:"));
      { QPalette palette; palette.setColor(color_field1->backgroundRole(), temp_colors.cg_pushb_disabled.text().color()); color_field1->setPalette(palette); }
      lbl_color2->setText(us_tr(" Background Color:"));
      { QPalette palette; palette.setColor(color_field2->backgroundRole(), temp_colors.cg_pushb_disabled.button().color()); color_field2->setPalette(palette); }
      lbl_color3->setText(us_tr(" Highlight Color:"));
      { QPalette palette; palette.setColor(color_field3->backgroundRole(), temp_colors.cg_pushb_disabled.light().color()); color_field3->setPalette(palette); }
      lbl_color4->setText(us_tr(" Shadow Color:"));
      { QPalette palette; palette.setColor(color_field4->backgroundRole(), temp_colors.cg_pushb_disabled.dark().color()); color_field4->setPalette(palette); }
      lbl_color5->setText("");
      { QPalette palette; palette.setColor(color_field5->backgroundRole(), Qt::gray); color_field5->setPalette(palette); }
      lbl_color6->setText("");
      { QPalette palette; palette.setColor(color_field6->backgroundRole(), Qt::gray); color_field6->setPalette(palette); }
      break;
    }
    case 3:
    {
      pb_color1->setEnabled(true);
      pb_color2->setEnabled(true);
      pb_color3->setEnabled(true);
      pb_color4->setEnabled(true);
      pb_color5->setEnabled(false);
      pb_color6->setEnabled(false);
      temp_color1 = temp_colors.cg_plot.text().color();
      temp_color2 = temp_colors.cg_plot.window().color();
      temp_color3 = temp_colors.cg_plot.light().color();
      temp_color4 = temp_colors.cg_plot.dark().color();
      lbl_color1->setText(us_tr(" Text Color:"));
      { QPalette palette; palette.setColor(color_field1->backgroundRole(), temp_colors.cg_plot.text().color()); color_field1->setPalette(palette); }
      lbl_color2->setText(us_tr(" Background Color:"));
      { QPalette palette; palette.setColor(color_field2->backgroundRole(), temp_colors.cg_plot.window().color()); color_field2->setPalette(palette); }
      lbl_color3->setText(us_tr(" Highlight Color:"));
      { QPalette palette; palette.setColor(color_field3->backgroundRole(), temp_colors.cg_plot.light().color()); color_field3->setPalette(palette); }
      lbl_color4->setText(us_tr(" Shadow Color:"));
      { QPalette palette; palette.setColor(color_field4->backgroundRole(), temp_colors.cg_plot.dark().color()); color_field4->setPalette(palette); }
      lbl_color5->setText("");
      { QPalette palette; palette.setColor(color_field5->backgroundRole(), Qt::gray); color_field5->setPalette(palette); }
      lbl_color6->setText("");
      { QPalette palette; palette.setColor(color_field6->backgroundRole(), Qt::gray); color_field6->setPalette(palette); }
      break;
    }
    case 4:
    {
      pb_color1->setEnabled(true);
      pb_color2->setEnabled(false);
      pb_color3->setEnabled(false);
      pb_color4->setEnabled(false);
      pb_color5->setEnabled(false);
      pb_color6->setEnabled(false);
      temp_color1 = temp_colors.plot;
      lbl_color1->setText(us_tr(" Plot Background Color:"));
      { QPalette palette; palette.setColor(color_field1->backgroundRole(), temp_colors.plot); color_field1->setPalette(palette); }
      lbl_color2->setText("");
      { QPalette palette; palette.setColor(color_field2->backgroundRole(), Qt::gray); color_field2->setPalette(palette); }
      lbl_color3->setText("");
      { QPalette palette; palette.setColor(color_field3->backgroundRole(), Qt::gray); color_field3->setPalette(palette); }
      lbl_color4->setText("");
      { QPalette palette; palette.setColor(color_field4->backgroundRole(), Qt::gray); color_field4->setPalette(palette); }
      lbl_color5->setText("");
      { QPalette palette; palette.setColor(color_field5->backgroundRole(), Qt::gray); color_field5->setPalette(palette); }
      lbl_color6->setText("");
      { QPalette palette; palette.setColor(color_field6->backgroundRole(), Qt::gray); color_field6->setPalette(palette); }
      break;
    }
    case 5:
    {
      pb_color1->setEnabled(true);
      pb_color2->setEnabled(false);
      pb_color3->setEnabled(false);
      pb_color4->setEnabled(false);
      pb_color5->setEnabled(false);
      pb_color6->setEnabled(false);
      temp_color1 = temp_colors.major_ticks;
      lbl_color1->setText(us_tr(" Major Tick Color:"));
      { QPalette palette; palette.setColor(color_field1->backgroundRole(), temp_colors.major_ticks); color_field1->setPalette(palette); }
      lbl_color2->setText("");
      { QPalette palette; palette.setColor(color_field2->backgroundRole(), Qt::gray); color_field2->setPalette(palette); }
      lbl_color3->setText("");
      { QPalette palette; palette.setColor(color_field3->backgroundRole(), Qt::gray); color_field3->setPalette(palette); }
      lbl_color4->setText("");
      { QPalette palette; palette.setColor(color_field4->backgroundRole(), Qt::gray); color_field4->setPalette(palette); }
      lbl_color5->setText("");
      { QPalette palette; palette.setColor(color_field5->backgroundRole(), Qt::gray); color_field5->setPalette(palette); }
      lbl_color6->setText("");
      { QPalette palette; palette.setColor(color_field6->backgroundRole(), Qt::gray); color_field6->setPalette(palette); }
      break;
    }
    case 6:
    {
      pb_color1->setEnabled(true);
      pb_color2->setEnabled(false);
      pb_color3->setEnabled(false);
      pb_color4->setEnabled(false);
      pb_color5->setEnabled(false);
      pb_color6->setEnabled(false);
      temp_color1 = temp_colors.minor_ticks;
      lbl_color1->setText(us_tr(" Minor Tick Color:"));
      { QPalette palette; palette.setColor(color_field1->backgroundRole(), temp_colors.minor_ticks); color_field1->setPalette(palette); }
      lbl_color2->setText("");
      { QPalette palette; palette.setColor(color_field2->backgroundRole(), Qt::gray); color_field2->setPalette(palette); }
      lbl_color3->setText("");
      { QPalette palette; palette.setColor(color_field3->backgroundRole(), Qt::gray); color_field3->setPalette(palette); }
      lbl_color4->setText("");
      { QPalette palette; palette.setColor(color_field4->backgroundRole(), Qt::gray); color_field4->setPalette(palette); }
      lbl_color5->setText("");
      { QPalette palette; palette.setColor(color_field5->backgroundRole(), Qt::gray); color_field5->setPalette(palette); }
      lbl_color6->setText("");
      { QPalette palette; palette.setColor(color_field6->backgroundRole(), Qt::gray); color_field6->setPalette(palette); }
      break;
    }
    case 7:
    {
      pb_color1->setEnabled(true);
      pb_color2->setEnabled(true);
      pb_color3->setEnabled(false);
      pb_color4->setEnabled(false);
      pb_color5->setEnabled(false);
      pb_color6->setEnabled(false);
      temp_color1 = temp_colors.cg_edit.text().color();
      temp_color2 = temp_colors.cg_edit.window().color();
      lbl_color1->setText(us_tr(" Text Color:"));
      { QPalette palette; palette.setColor(color_field1->backgroundRole(), temp_colors.cg_edit.windowText().color()); color_field1->setPalette(palette); }
      lbl_color2->setText(us_tr(" Background Color:"));
      { QPalette palette; palette.setColor(color_field2->backgroundRole(), temp_colors.cg_edit.window().color()); color_field2->setPalette(palette); }
      lbl_color3->setText("");
      { QPalette palette; palette.setColor(color_field3->backgroundRole(), Qt::gray); color_field3->setPalette(palette); }
      lbl_color4->setText("");
      { QPalette palette; palette.setColor(color_field4->backgroundRole(), Qt::gray); color_field4->setPalette(palette); }
      lbl_color5->setText("");
      { QPalette palette; palette.setColor(color_field5->backgroundRole(), Qt::gray); color_field5->setPalette(palette); }
      lbl_color6->setText("");
      { QPalette palette; palette.setColor(color_field6->backgroundRole(), Qt::gray); color_field6->setPalette(palette); }
      break;
    }
    case 8:
    {
      pb_color1->setEnabled(true);
      pb_color2->setEnabled(true);
      pb_color3->setEnabled(false);
      pb_color4->setEnabled(false);
      pb_color5->setEnabled(false);
      pb_color6->setEnabled(false);
      temp_color1 = temp_colors.cg_label.text().color();
      temp_color2 = temp_colors.cg_label.window().color();
      lbl_color1->setText(us_tr(" Text Color:"));
      { QPalette palette; palette.setColor(color_field1->backgroundRole(), temp_colors.cg_label.windowText().color()); color_field1->setPalette(palette); }
      lbl_color2->setText(us_tr(" Background Color:"));
      { QPalette palette; palette.setColor(color_field2->backgroundRole(), temp_colors.cg_label.window().color()); color_field2->setPalette(palette); }
      lbl_color3->setText("");
      { QPalette palette; palette.setColor(color_field3->backgroundRole(), Qt::gray); color_field3->setPalette(palette); }
      lbl_color4->setText("");
      { QPalette palette; palette.setColor(color_field4->backgroundRole(), Qt::gray); color_field4->setPalette(palette); }
      lbl_color5->setText("");
      { QPalette palette; palette.setColor(color_field5->backgroundRole(), Qt::gray); color_field5->setPalette(palette); }
      lbl_color6->setText("");
      { QPalette palette; palette.setColor(color_field6->backgroundRole(), Qt::gray); color_field6->setPalette(palette); }
      break;
    }
    case 9:
    {
      pb_color1->setEnabled(true);
      pb_color2->setEnabled(true);
      pb_color3->setEnabled(true);
      pb_color4->setEnabled(true);
      pb_color5->setEnabled(true);
      pb_color6->setEnabled(true);
      temp_color1 = temp_colors.cg_normal.text().color();
      temp_color2 = temp_colors.cg_normal.window().color();
      temp_color3 = temp_colors.cg_normal.light().color();
      temp_color4 = temp_colors.cg_normal.dark().color();
      temp_color5 = temp_colors.cg_normal.highlight().color();
      temp_color6 = temp_colors.cg_normal.button().color();
      lbl_color1->setText(us_tr(" Text Color:"));
      { QPalette palette; palette.setColor(color_field1->backgroundRole(), temp_colors.cg_normal.text().color()); color_field1->setPalette(palette); }
      lbl_color2->setText(us_tr(" Background Color:"));
      { QPalette palette; palette.setColor(color_field2->backgroundRole(), temp_colors.cg_normal.window().color()); color_field2->setPalette(palette); }
      lbl_color3->setText(us_tr(" Highlight Color:"));
      { QPalette palette; palette.setColor(color_field3->backgroundRole(), temp_colors.cg_normal.light().color()); color_field3->setPalette(palette); }
      lbl_color4->setText(us_tr(" Shadow Color:"));
      { QPalette palette; palette.setColor(color_field4->backgroundRole(), temp_colors.cg_normal.dark().color()); color_field4->setPalette(palette); }
      lbl_color5->setText(us_tr(" Medium Highlight Color:"));
      { QPalette palette; palette.setColor(color_field5->backgroundRole(), temp_colors.cg_normal.highlight().color()); color_field5->setPalette(palette); }
      lbl_color6->setText(us_tr(" Button Color:"));
      { QPalette palette; palette.setColor(color_field6->backgroundRole(), temp_colors.cg_normal.button().color()); color_field6->setPalette(palette); }
      break;
    }
    case 10:
    {
      pb_color1->setEnabled(true);
      pb_color2->setEnabled(true);
      pb_color3->setEnabled(true);
      pb_color4->setEnabled(true);
      pb_color5->setEnabled(false);
      pb_color6->setEnabled(false);
      temp_color1 = temp_colors.cg_lcd.windowText().color();
      temp_color2 = temp_colors.cg_lcd.window().color();
      temp_color3 = temp_colors.cg_lcd.light().color();
      temp_color4 = temp_colors.cg_lcd.dark().color();
      lbl_color1->setText(us_tr(" Text Center Color:"));
      { QPalette palette; palette.setColor(color_field1->backgroundRole(), temp_colors.cg_lcd.windowText().color()); color_field1->setPalette(palette); }
      lbl_color2->setText(us_tr(" Background Color:"));
      { QPalette palette; palette.setColor(color_field2->backgroundRole(), temp_colors.cg_lcd.window().color()); color_field2->setPalette(palette); }
      lbl_color3->setText(us_tr(" Highlight Color:"));
      { QPalette palette; palette.setColor(color_field3->backgroundRole(), temp_colors.cg_lcd.light().color()); color_field3->setPalette(palette); }
      lbl_color4->setText(us_tr(" Shadow Color:"));
      { QPalette palette; palette.setColor(color_field4->backgroundRole(), temp_colors.cg_lcd.dark().color()); color_field4->setPalette(palette); }
      lbl_color5->setText("");
      { QPalette palette; palette.setColor(color_field5->backgroundRole(), Qt::gray); color_field5->setPalette(palette); }
      lbl_color6->setText("");
      { QPalette palette; palette.setColor(color_field6->backgroundRole(), Qt::gray); color_field6->setPalette(palette); }
      break;
    }
  }
}

void US_Color::pick_color1()
{
  QColor black;
  black.setRgb(0x00, 0x00, 0x00);
  selected_item(current_widget);
  color1 = QColorDialog::getColor(temp_color1, this);
  { QPalette palette; palette.setColor(color_field1->backgroundRole(), color1); color_field1->setPalette(palette); }
  temp_color1 = color1;
//  { QPalette palette; palette.setColor(color_field1->backgroundRole(), temp_colors.cg_pushb.buttonText().color()); color_field1->setPalette(palette); }
  switch (current_widget)
  {
    case 0:
    {
      temp_colors.cg_frame.setColor(QPalette::WindowText, color1);
      float v = (int) (0.2125 * color1.red() + 0.0721 * color1.blue() + 0.7154 * color1.green());
      if (v > 128)
      {
        temp_colors.cg_frame.setColor(QPalette::Base, black);
      }
      else
      {
        temp_colors.cg_frame.setColor(QPalette::Base, Qt::white);
      }
      lbl_background->setPalette( PALET_FRAME );
      AUTFBACK( lbl_background );
      lbl_margin->setPalette( PALET_FRAME );
      AUTFBACK( lbl_margin );
      lbl_progress->setPalette( PALET_FRAME );
      AUTFBACK( lbl_progress );
      lbl_lcd->setPalette( PALET_FRAME );
      AUTFBACK( lbl_lcd );
      lbl_counter->setPalette( PALET_FRAME );
      AUTFBACK( lbl_counter );
      lbl_banner->setPalette( PALET_FRAME );
      AUTFBACK( lbl_banner );
      break;
    }
    case 1:
    {
      temp_colors.cg_pushb.setColor(QPalette::ButtonText, color1);
      pb_normal->setPalette( PALET_PUSHB );
      pb_active->setPalette( PALET_PUSHB );
      break;
    }
    case 2:
    {
      temp_colors.cg_pushb_disabled.setColor(QPalette::ButtonText, color1);
      temp_colors.cg_pushb_disabled.setColor(QPalette::Text, color1);
      pb_disabled->setPalette( temp_colors.cg_pushb_disabled );
      break;
    }
    case 3:
    {
      temp_colors.cg_plot.setColor(QPalette::Text, color1);
      plot->setPalette( PALET_NORMAL );
      AUTFBACK( plot );
      break;
    }
    case 4:
    {
      temp_colors.plot = color1;
      //plot->setPlotBackground(temp_colors.plot);    //old version
      plot->setCanvasBackground(temp_colors.plot);    //new version
      plot->replot();
      break;
    }
    case 5:
    {
      temp_colors.major_ticks = color1;
#if QT_VERSION < 0x040000
      plot->setGridMajPen(QPen(temp_colors.major_ticks, 0, DotLine));
#else
      grid->setMajorPen( QPen( temp_colors.major_ticks, 0, Qt::DotLine ) );
#endif
      plot->replot();
      break;
    }
    case 6:
    {
      temp_colors.minor_ticks = color1;
#if QT_VERSION < 0x040000
      plot->setGridMinPen(QPen(temp_colors.minor_ticks, 0, DotLine));
#else
      grid->setMinorPen( QPen( temp_colors.minor_ticks, 0, Qt::DotLine ) );
#endif
      plot->replot();
      break;
    }
    case 7:
    {
      temp_colors.cg_edit.setColor(QPalette::WindowText, color1);
      lbl_edit->setPalette( PALET_EDIT );
      AUTFBACK( lbl_edit );
      break;
    }
    case 8:
    {
      temp_colors.cg_label.setColor(QPalette::WindowText, color1);
      lbl_text->setPalette( PALET_LABEL );
      AUTFBACK( lbl_text );
      break;
    }
    case 9:
    {
      temp_colors.cg_normal.setColor(QPalette::Text, color1);
      cmbb_margin->setPalette( PALET_NORMAL );
      AUTFBACK( cmbb_margin );
      progress->setPalette( PALET_NORMAL );
      AUTFBACK( progress );
      cnt->setPalette( PALET_NORMAL );
      AUTFBACK( cnt );
      break;
    }
    case 10:
    {
      temp_colors.cg_lcd.setColor(QPalette::WindowText, color1);
      lcd->setPalette( temp_colors.cg_lcd );
      break;
    }
  }
}

void US_Color::pick_color2()
{
  color2 = QColorDialog::getColor(temp_color2, this);
  { QPalette palette; palette.setColor(color_field2->backgroundRole(), color2); color_field2->setPalette(palette); }
  temp_color2 = color2;
  switch (current_widget)
  {
    case 0:
    {
      temp_colors.cg_frame.setColor(QPalette::Window, color2);
      lbl_background->setPalette( PALET_FRAME );
      AUTFBACK( lbl_background );
      lbl_margin->setPalette( PALET_FRAME );
      AUTFBACK( lbl_margin );
      lbl_progress->setPalette( PALET_FRAME );
      AUTFBACK( lbl_progress );
      lbl_lcd->setPalette( PALET_FRAME );
      AUTFBACK( lbl_lcd );
      lbl_counter->setPalette( PALET_FRAME );
      AUTFBACK( lbl_counter );
      lbl_banner->setPalette( PALET_FRAME );
      AUTFBACK( lbl_banner );
      break;
    }
    case 1:
    {
      temp_colors.cg_pushb.setColor(QPalette::Button, color2);
      pb_normal->setPalette( PALET_PUSHB );
      pb_active->setPalette( PALET_PUSHB );
      break;
    }
    case 2:
    {
      temp_colors.cg_pushb_disabled.setColor(QPalette::Button, color2);
      pb_disabled->setPalette( temp_colors.cg_pushb_disabled );
      break;
    }
    case 3:
    {
      temp_colors.cg_plot.setColor(QPalette::Window, color2);
      plot->setPalette( PALET_NORMAL );
      AUTFBACK( plot );
      break;
    }
    case 7:
    {
      temp_colors.cg_edit.setColor(QPalette::Window, color2);
      lbl_edit->setPalette( PALET_EDIT );
      AUTFBACK( lbl_edit );
      break;
    }
    case 8:
    {
      temp_colors.cg_label.setColor(QPalette::Window, color2);
      lbl_text->setPalette( PALET_LABEL );
      AUTFBACK( lbl_text );
      break;
    }
    case 9:
    {
      temp_colors.cg_normal.setColor(QPalette::Window, color2);
      cmbb_margin->setPalette( PALET_NORMAL );
      AUTFBACK( cmbb_margin );
      progress->setPalette( PALET_NORMAL );
      AUTFBACK( progress );
      cnt->setPalette( PALET_NORMAL );
      AUTFBACK( cnt );
      break;
    }
    case 10:
    {
      temp_colors.cg_lcd.setColor(QPalette::Window, color2);
      lcd->setPalette( temp_colors.cg_lcd );
      break;
    }
  }
}

void US_Color::pick_color3()
{
  color3 = QColorDialog::getColor(temp_color3, this);
  { QPalette palette; palette.setColor(color_field3->backgroundRole(), color3); color_field3->setPalette(palette); }
  temp_color3 = color3;
  switch (current_widget)
  {
    case 0:
    {
      temp_colors.cg_frame.setColor(QPalette::Light, color3);
      lbl_background->setPalette( PALET_FRAME );
      AUTFBACK( lbl_background );
      lbl_margin->setPalette( PALET_FRAME );
      AUTFBACK( lbl_margin );
      lbl_progress->setPalette( PALET_FRAME );
      AUTFBACK( lbl_progress );
      lbl_lcd->setPalette( PALET_FRAME );
      AUTFBACK( lbl_lcd );
      lbl_counter->setPalette( PALET_FRAME );
      AUTFBACK( lbl_counter );
      lbl_banner->setPalette( PALET_FRAME );
      AUTFBACK( lbl_banner );
      break;
    }
    case 1:
    {
      temp_colors.cg_pushb.setColor(QPalette::Light, color3);
      pb_normal->setPalette( PALET_PUSHB );
      pb_active->setPalette( PALET_PUSHB );
      break;
    }
    case 2:
    {
      temp_colors.cg_pushb_disabled.setColor(QPalette::Light, color3);
      pb_disabled->setPalette( temp_colors.cg_pushb_disabled );
      break;
    }
    case 3:
    {
      temp_colors.cg_plot.setColor(QPalette::Light, color3);
      plot->setPalette( PALET_NORMAL );
      AUTFBACK( plot );
      break;
    }
    case 9:
    {
      temp_colors.cg_normal.setColor(QPalette::Light, color3);
      cmbb_margin->setPalette( PALET_NORMAL );
      AUTFBACK( cmbb_margin );
      progress->setPalette( PALET_NORMAL );
      AUTFBACK( progress );
      cnt->setPalette( PALET_NORMAL );
      AUTFBACK( cnt );
      break;
    }
    case 10:
    {
      temp_colors.cg_lcd.setColor(QPalette::Light, color3);
      lcd->setPalette( temp_colors.cg_lcd );
      break;
    }
  }
}

void US_Color::pick_color4()
{
  color4 = QColorDialog::getColor(temp_color4, this);
  { QPalette palette; palette.setColor(color_field4->backgroundRole(), color4); color_field4->setPalette(palette); }
  temp_color4 = color4;
  switch (current_widget)
  {
    case 0:
    {
      temp_colors.cg_frame.setColor(QPalette::Dark, color4);
      lbl_background->setPalette( PALET_FRAME );
      AUTFBACK( lbl_background );
      lbl_margin->setPalette( PALET_FRAME );
      AUTFBACK( lbl_margin );
      lbl_progress->setPalette( PALET_FRAME );
      AUTFBACK( lbl_progress );
      lbl_lcd->setPalette( PALET_FRAME );
      AUTFBACK( lbl_lcd );
      lbl_counter->setPalette( PALET_FRAME );
      AUTFBACK( lbl_counter );
      lbl_banner->setPalette( PALET_FRAME );
      AUTFBACK( lbl_banner );
      break;
    }
    case 1:
    {
      temp_colors.cg_pushb.setColor(QPalette::Dark, color4);
      pb_normal->setPalette( PALET_PUSHB );
      pb_active->setPalette( PALET_PUSHB );
      break;
    }
    case 2:
    {
      temp_colors.cg_pushb_disabled.setColor(QPalette::Dark, color4);
      pb_disabled->setPalette( temp_colors.cg_pushb_disabled );
      break;
    }
    case 3:
    {
      temp_colors.cg_plot.setColor(QPalette::Dark, color4);
      plot->setPalette( PALET_NORMAL );
      AUTFBACK( plot );
      break;
    }
    case 9:
    {
      temp_colors.cg_normal.setColor(QPalette::Dark, color4);
      cmbb_margin->setPalette( PALET_NORMAL );
      AUTFBACK( cmbb_margin );
      progress->setPalette( PALET_NORMAL );
      AUTFBACK( progress );
      cnt->setPalette( PALET_NORMAL );
      AUTFBACK( cnt );
      break;
    }
    case 10:
    {
      temp_colors.cg_lcd.setColor(QPalette::Dark, color4);
      lcd->setPalette( temp_colors.cg_lcd );
      break;
    }
  }
}

void US_Color::pick_color5()
{
  color5 = QColorDialog::getColor(temp_color5, this);
  { QPalette palette; palette.setColor(color_field5->backgroundRole(), color5); color_field5->setPalette(palette); }
  temp_color5 = color5;
  switch (current_widget)
  {
    case 0:
    {
      temp_colors.cg_frame.setColor(QPalette::Midlight, color5);
      lbl_background->setPalette( PALET_FRAME );
      AUTFBACK( lbl_background );
      lbl_margin->setPalette( PALET_FRAME );
      AUTFBACK( lbl_margin );
      lbl_progress->setPalette( PALET_FRAME );
      AUTFBACK( lbl_progress );
      lbl_lcd->setPalette( PALET_FRAME );
      AUTFBACK( lbl_lcd );
      lbl_counter->setPalette( PALET_FRAME );
      AUTFBACK( lbl_counter );
      lbl_banner->setPalette( PALET_FRAME );
      AUTFBACK( lbl_banner );
      break;
    }
    case 1:
    {
      temp_colors.cg_pushb.setColor(QPalette::Mid, color5);
      pb_active->setPalette( PALET_PUSHB );
      break;
    }
    case 9:
    {
      temp_colors.cg_normal.setColor(QPalette::Highlight, color5);
      cmbb_margin->setPalette( PALET_NORMAL );
      AUTFBACK( cmbb_margin );
      progress->setPalette( PALET_NORMAL );
      AUTFBACK( progress );
      cnt->setPalette( PALET_NORMAL );
      AUTFBACK( cnt );
      break;
    }
  }
}

void US_Color::pick_color6()
{
  color6 = QColorDialog::getColor(temp_color6, this);
  { QPalette palette; palette.setColor(color_field6->backgroundRole(), color6); color_field6->setPalette(palette); }
  temp_color6 = color6;
  switch (current_widget)
  {
    case 0:
    {
      temp_colors.cg_frame.setColor(QPalette::Shadow, color6);
      lbl_background->setPalette( PALET_FRAME );
      AUTFBACK( lbl_background );
      lbl_margin->setPalette( PALET_FRAME );
      AUTFBACK( lbl_margin );
      lbl_progress->setPalette( PALET_FRAME );
      AUTFBACK( lbl_progress );
      lbl_lcd->setPalette( PALET_FRAME );
      AUTFBACK( lbl_lcd );
      lbl_counter->setPalette( PALET_FRAME );
      AUTFBACK( lbl_counter );
      lbl_banner->setPalette( PALET_FRAME );
      AUTFBACK( lbl_banner );
      break;
    }
    case 9:
    {
      temp_colors.cg_normal.setColor(QPalette::Button, color6);
      cmbb_margin->setPalette( PALET_NORMAL );
      AUTFBACK( cmbb_margin );
      progress->setPalette( PALET_NORMAL );
      AUTFBACK( progress );
      cnt->setPalette( PALET_NORMAL );
      AUTFBACK( cnt );
      break;
    }
  }
}

void US_Color::help()
{
  US_Help *online_help; online_help = new US_Help(this);
  online_help->show_help("manual/uscol.html");
}

void US_Color::reset()
{
  temp_colors = USglobal->global_colors;
  temp_margin = USglobal->config_list.margin;
  cmbb_margin->setCurrentIndex(temp_margin/2-1);
  apply();
  selected_item(current_widget);
}

void US_Color::delete_scheme()
{
  QString str = us_tr("Do you really want to delete the\n \"") + 
               lb_scheme->item(current_scheme)->text() + us_tr("\" color scheme?");
  int result = QMessageBox::warning(0, 
        us_tr("Attention:"), str, us_tr("Yes"), us_tr("No"), QString(), 1, 1);
  if (result == 0)
  {
    if (current_scheme == 0)
    {
      US_Static::us_message(
          us_tr("Attention:"), 
          us_tr("Sorry, the current scheme cannot\n"
             "be deleted. Please try again..."));
      return;
    }
    if (current_scheme == 1)
    {
      US_Static::us_message(
          us_tr("Attention:"), 
          us_tr("Sorry, you cannot delete the\n"
             "UltraScan Default color scheme.\n"
             "Please try again..."));
      return;
    }
    QDir scheme_dir1;
    QString filter = "*.col";
    QStringList entries1;
    QString path = QDir::toNativeSeparators( USglobal->config_list.system_dir + "/etc" );

    scheme_dir1.setPath( path ); // system wide color defs
    scheme_dir1.setNameFilters( QStringList() << filter );
    entries1 = scheme_dir1.entryList();
    QStringList::Iterator it;

    // make sure it's not a system color definition  
    for ( it = entries1.begin(); it != entries1.end(); ++it ) 
    {
      str = (*it).toLatin1().data();
      str.truncate(str.lastIndexOf(".", -1, Qt::CaseInsensitive ));  // strip the extension
      if (str == lb_scheme->item(current_scheme)->text())
      {
        US_Static::us_message(
            us_tr("Attention:"), 
            us_tr("Sorry, this color definition\n"
               "is a system resource and cannot\n"
               "be deleted. Please try again..."));
        return;
      }
    }
    
    QString colfile = QDir::toNativeSeparators( USglobal->config_list.root_dir + 
                      "/" + lb_scheme->item(current_scheme)->text() + ".col" );
    QFile f( colfile);
    if (! f.remove() )
    {
      US_Static::us_message(
          us_tr("Attention:"), 
          us_tr("Sorry, couldn't delete the\n"
             "associated file for this scheme\n"
             "- please check the file permissions..."));
    }
    delete lb_scheme->takeItem(current_scheme);
  }
}

void US_Color::quit()
{
  close();
}

void US_Color::apply()
{
  USglobal->global_colors = temp_colors;
  USglobal->config_list.margin = temp_margin;

// update widgets:

  pb_quit->setPalette( PALET_PUSHB );
  pb_delete->setPalette( PALET_PUSHB );
  pb_save_as->setPalette( PALET_PUSHB );
  pb_help->setPalette( PALET_PUSHB );
  pb_reset->setPalette( PALET_PUSHB );
  pb_apply->setPalette( PALET_PUSHB );
  pb_normal->setPalette( PALET_PUSHB );
  pb_active->setPalette( PALET_PUSHB );
  pb_disabled->setPalette( USglobal->global_colors.cg_pushb_disabled );
  pb_color1->setPalette( PALET_PUSHB );
  pb_color2->setPalette( PALET_PUSHB );
  pb_color3->setPalette( PALET_PUSHB );
  pb_color4->setPalette( PALET_PUSHB );
  pb_color5->setPalette( PALET_PUSHB );
  pb_color6->setPalette( PALET_PUSHB );
  plot->setPalette( PALET_NORMAL );
  AUTFBACK( plot );
#if QT_VERSION < 0x040000
  plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
  plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
  grid->setMajorPen( QPen( temp_colors.major_ticks, 0, Qt::DotLine ) );
  grid->setMinorPen( QPen( temp_colors.minor_ticks, 0, Qt::DotLine ) );
#endif
  plot->setCanvasBackground(temp_colors.plot);
//   plot->setMargin(USglobal->config_list.margin);
  cnt->setPalette( PALET_NORMAL );
  AUTFBACK( cnt );
  cmbb_margin->setPalette( PALET_NORMAL );
  AUTFBACK( cmbb_margin );
  progress->setPalette( PALET_NORMAL );
  AUTFBACK( progress );
  lcd->setPalette( USglobal->global_colors.cg_lcd );
  lbl_background->setPalette( PALET_FRAME );
  AUTFBACK( lbl_background );
  lbl_banner->setPalette( PALET_FRAME );
  AUTFBACK( lbl_banner );
  lbl_margin->setPalette( PALET_FRAME );
  AUTFBACK( lbl_margin );
  lbl_progress->setPalette( PALET_FRAME );
  AUTFBACK( lbl_progress );
  lbl_counter->setPalette( PALET_FRAME );
  AUTFBACK( lbl_counter );
  lbl_lcd->setPalette( PALET_FRAME );
  AUTFBACK( lbl_lcd );
  lbl_select->setPalette( PALET_LABEL );
  AUTFBACK( lbl_select );
  lbl_assign->setPalette( PALET_FRAME );
  AUTFBACK( lbl_assign );
  lbl_select_scheme->setPalette( PALET_LABEL );
  AUTFBACK( lbl_select_scheme );
  lbl_select_element->setPalette( PALET_LABEL );
  AUTFBACK( lbl_select_element );
  lbl_choice->setPalette( PALET_EDIT );
  AUTFBACK( lbl_choice );
  lbl_choices->setPalette( PALET_FRAME );
  AUTFBACK( lbl_choices );
  lbl_text->setPalette( PALET_LABEL );
  AUTFBACK( lbl_text );
  lbl_edit->setPalette( PALET_EDIT );
  AUTFBACK( lbl_edit );
  lbl_color1->setPalette( PALET_LABEL );
  AUTFBACK( lbl_color1 );
  lbl_color2->setPalette( PALET_LABEL );
  AUTFBACK( lbl_color2 );
  lbl_color3->setPalette( PALET_LABEL );
  AUTFBACK( lbl_color3 );
  lbl_color4->setPalette( PALET_LABEL );
  AUTFBACK( lbl_color4 );
  lbl_color5->setPalette( PALET_LABEL );
  AUTFBACK( lbl_color5 );
  lbl_color6->setPalette( PALET_LABEL );
  AUTFBACK( lbl_color6 );
  le_save_as->setPalette( PALET_NORMAL );
  AUTFBACK( le_save_as );
  lb_scheme->setPalette( PALET_NORMAL );
  AUTFBACK( lb_scheme );
  lb_item->setPalette( PALET_NORMAL );
  AUTFBACK( lb_item );
  setPalette( PALET_FRAME );

  QFile f;

  QString colfile = US_Config::get_home_dir() + USCOLORS;

  f.setFileName( colfile );

  if ( f.open ( QIODevice::WriteOnly ) )
  {
    QDataStream ds(&f);
    ds << USglobal->global_colors.pushb;
    ds << USglobal->global_colors.frames;
    ds << USglobal->global_colors.labels;
    ds << USglobal->global_colors.pushb_active;
    ds << USglobal->global_colors.dk_red;
    ds << USglobal->global_colors.dk_green;
    ds << USglobal->global_colors.plot;
    ds << USglobal->global_colors.major_ticks;
    ds << USglobal->global_colors.minor_ticks;
    ds << USglobal->global_colors.gray;
    ds << USglobal->global_colors.gray1;
    ds << USglobal->global_colors.gray2;
    ds << USglobal->global_colors.gray3;
    ds << USglobal->global_colors.gray4;
    ds << USglobal->global_colors.gray5;
    ds << USglobal->global_colors.gray6;
    ds << USglobal->global_colors.gray7;
    ds << USglobal->global_colors.gray8;
    ds << USglobal->global_colors.gray9;
    ds << USglobal->global_colors.cg_frame;
    ds << USglobal->global_colors.cg_pushb;
    ds << USglobal->global_colors.cg_pushb_active;
    ds << USglobal->global_colors.cg_pushb_disabled;
    ds << USglobal->global_colors.cg_label;
    ds << USglobal->global_colors.cg_label_disabled;
    ds << USglobal->global_colors.cg_label_warn;
    ds << USglobal->global_colors.cg_edit;
    ds << USglobal->global_colors.cg_edit_warn;
    ds << USglobal->global_colors.cg_dk_red;
    ds << USglobal->global_colors.cg_dk_green;
    ds << USglobal->global_colors.cg_red;
    ds << USglobal->global_colors.cg_green;
    ds << USglobal->global_colors.cg_gray;
    ds << USglobal->global_colors.cg_normal;
    ds << USglobal->global_colors.cg_plot;
    ds << USglobal->global_colors.cg_lcd;
    ds << USglobal->global_colors.cg_bunt;
    ds << US_Version;
    f.close();
  }
//  WConfig = new US_Write_Config();
//  WConfig->write_config(USglobal->config_list);
//  emit marginChanged(USglobal->config_list.margin);
}

void US_Color::save_as()
{
  QDir system_dir;
  bool problem = false;
  
// Apply current setting before saving it:

  apply();

  QString str1, filter = "*.col";
  QStringList entries;
  
  QString dir = QDir::toNativeSeparators( USglobal->config_list.system_dir + "/etc");
  system_dir.setPath( dir );  // system wide color defs
  
  
  system_dir.setNameFilters( QStringList() << filter );
  entries = system_dir.entryList();
  
  QStringList::Iterator it;
  for ( it = entries.begin(); it != entries.end(); ++it )
  {
    str1 = (*it).toLatin1().data();
    if (save_str.right(4) == ".col")
    {
      save_str.truncate(save_str.length()-4);
    }
    if (str1.right(4) == ".col")
    {
      str1.truncate(str1.length()-4);
    }
    if (str1 == save_str)
    {
      problem = true;
    }
  }
  if (problem)
  {
    US_Static::us_message(
        us_tr("Attention:"), 
        us_tr("Sorry - \n\nthis name is reserved for a system-\n"
           "wide color scheme.\n\n"
           "Please select a different name."));
    return;
  }
  problem = false;
  for (int i=0; i< lb_scheme->count(); i++) 
  {
    if (save_str == lb_scheme->item(i)->text())   // if the item is already listed we don't need to list it again
    {                         // instead, we overwrite the existing color definition 
      problem = true;
    }
  }
  if (!problem)
  {
    lb_scheme->addItem(save_str);
  }
  QString filename;
  filename = QDir::toNativeSeparators(USglobal->config_list.root_dir + "/" + save_str );
  
  if (filename.right(4) != ".col")
  {
    filename.append(".col");
  }

  QFile file( filename );

  if ( file.open( QIODevice::WriteOnly ) )
  {
    QDataStream ds(&file);
    ds << USglobal->global_colors.pushb;
    ds << USglobal->global_colors.frames;
    ds << USglobal->global_colors.labels;
    ds << USglobal->global_colors.pushb_active;
    ds << USglobal->global_colors.dk_red;
    ds << USglobal->global_colors.dk_green;
    ds << USglobal->global_colors.plot;
    ds << USglobal->global_colors.major_ticks;
    ds << USglobal->global_colors.minor_ticks;
    ds << USglobal->global_colors.gray;
    ds << USglobal->global_colors.gray1;
    ds << USglobal->global_colors.gray2;
    ds << USglobal->global_colors.gray3;
    ds << USglobal->global_colors.gray4;
    ds << USglobal->global_colors.gray5;
    ds << USglobal->global_colors.gray6;
    ds << USglobal->global_colors.gray7;
    ds << USglobal->global_colors.gray8;
    ds << USglobal->global_colors.gray9;
    ds << USglobal->global_colors.cg_frame;
    ds << USglobal->global_colors.cg_pushb;
    ds << USglobal->global_colors.cg_pushb_active;
    ds << USglobal->global_colors.cg_pushb_disabled;
    ds << USglobal->global_colors.cg_label;
    ds << USglobal->global_colors.cg_label_disabled;
    ds << USglobal->global_colors.cg_label_warn;
    ds << USglobal->global_colors.cg_edit;
    ds << USglobal->global_colors.cg_edit_warn;
    ds << USglobal->global_colors.cg_dk_red;
    ds << USglobal->global_colors.cg_dk_green;
    ds << USglobal->global_colors.cg_red;
    ds << USglobal->global_colors.cg_green;
    ds << USglobal->global_colors.cg_gray;
    ds << USglobal->global_colors.cg_normal;
    ds << USglobal->global_colors.cg_plot;
    ds << USglobal->global_colors.cg_lcd;
    ds << USglobal->global_colors.cg_bunt;
    ds << US_Version;
    file.close();
  }
  else
  {
    US_Static::us_message(
        us_tr("Warning:"), 
        us_tr("Unable to open color file for output!\n"
           "Please check your write permissions.\n\n"
           "Color definitions were not saved..."));
  }
}

void US_Color::update_widgets(double newval)
{
  lcd->display(newval);
  progress->reset();
  progress->setValue((int) newval);
}

void US_Color::update_save_str(const QString &temp_str)
{
  save_str = temp_str;

}

void US_Color:: sel_margin(int newval)
{
  temp_margin = (newval+1)*2;
//   plot->setMargin(temp_margin); 
}

