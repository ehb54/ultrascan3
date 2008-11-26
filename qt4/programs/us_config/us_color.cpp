#include "us_color.h"
#include "us_gui_settings.h"

US_Color::US_Color( QWidget* w, Qt::WindowFlags flags ) : US_Widgets( w, flags )
{
  if ( ! g.isValid() )
  {
    // Do something for invalid global memory
    //qDebug( "us_win: invalid global memory" );
  }
 
  QPoint p = g.global_position();
  move( p );
  g.set_global_position( p + QPoint( 30, 30 ) );
 
  setWindowTitle( "Color Configuration for UltraScan" );
  setPalette( US_GuiSettings::frameColor() );


////////////////////
  save_str = tr("MyColors");
  current_widget = 0;
  current_index = USglobal->config_list.margin / 2 - 1;
  
  int xpos = 4, ypos = 4, width = 180, height = 26;
  double x[2], y[2];
  x[0] = 1.0;
  x[1] = 1.2;
  y[0] = 1;
  y[1] = 2;

  uint c1;

  lbl_background = new QLabel(tr("Frame Background"),this);
  Q_CHECK_PTR(lbl_background);
  lbl_background->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  lbl_background->setAlignment(AlignHCenter | AlignTop);
  lbl_background->setLineWidth(3);
  lbl_background->setFont(QFont( USglobal->config_list.fontFamily, 
                                 USglobal->config_list.fontSize, 
                                 QFont::Bold));
  lbl_background->setPalette(QPalette(temp_colors.cg_frame, 
                                      temp_colors.cg_frame, 
                                      temp_colors.cg_frame));


  plot = new QwtPlot(this);
  plot->enableOutline(true);
  plot->setOutlinePen(white);
  plot->setOutlineStyle(Qwt::Cross);
  plot->enableGridXMin();
  plot->enableGridYMin();
  plot->setTitle(tr("Sample Plot"));
  plot->setPalette(QPalette(temp_colors.cg_plot, temp_colors.cg_plot, temp_colors.cg_plot));
  plot->setGridMajPen(QPen(temp_colors.major_ticks, 0, DotLine));
  plot->setGridMinPen(QPen(temp_colors.minor_ticks, 0, DotLine));
  plot->setCanvasBackground(temp_colors.plot);    //new version
  plot->setAxisTitle(QwtPlot::xBottom, tr("X-axis"));
  plot->setAxisTitle(QwtPlot::yLeft, tr("Y-axis"));
  plot->setMargin(USglobal->config_list.margin);
  plot->setMinimumSize(width*2-10, height*9);
  c1 = plot->insertCurve("Sample 1");
  plot->setCurveStyle(c1, QwtCurve::Lines);
  plot->setCurvePen(c1, yellow);
  plot->setCurveData(c1, x, y, 2);
  plot->show();

  
  lbl_margin = new QLabel(tr(" Current Plot Border Margin:  "),this);
  Q_CHECK_PTR(lbl_margin);
  lbl_margin->setAlignment(AlignRight | AlignVCenter);
  lbl_margin->setFont(QFont( USglobal->config_list.fontFamily, 
                             USglobal->config_list.fontSize - 1, 
                             QFont::Bold));
  lbl_margin->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
  
  
  cmbb_margin = new QComboBox(false, this, "Margin Select");
  cmbb_margin->setPalette( QPalette(temp_colors.cg_normal, temp_colors.cg_normal, temp_colors.cg_normal));
  cmbb_margin->setFont(QFont(USglobal->config_list.fontFamily, 
                             USglobal->config_list.fontSize - 1));
  QString str_number;
  for (int i=1; i<=15; i++)   //plot margin range from 2 to 30
  {  
    str_number.sprintf("%d  pixel", i*2);
    cmbb_margin->insertItem(str_number);
  }
  cmbb_margin->setCurrentItem(current_index);
  connect(cmbb_margin, SIGNAL(activated(int)), SLOT(sel_margin(int)));
  
  lbl_example = new QLabel(tr("Label and Button Examples : "),this);
  Q_CHECK_PTR(lbl_example);
  lbl_example->setFrameStyle(QFrame::WinPanel|Raised);
  lbl_example->setAlignment(AlignCenter | AlignVCenter);
  lbl_example->setFont(QFont( USglobal->config_list.fontFamily, 
                              USglobal->config_list.fontSize - 1, QFont::Bold));
  lbl_example->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
  lbl_example->setMinimumHeight(30);
    
  lbl_banner = new QLabel(tr("Banner Label"),this);
  Q_CHECK_PTR(lbl_banner);
  lbl_banner->setFrameStyle(QFrame::WinPanel|Raised);
  lbl_banner->setAlignment(AlignCenter | AlignVCenter);
  lbl_banner->setFont(QFont( USglobal->config_list.fontFamily, 
                             USglobal->config_list.fontSize, 
                             QFont::Bold));
  lbl_banner->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
  

  lbl_progress = new QLabel(tr("Progress Bar:"),this);
  Q_CHECK_PTR(lbl_progress);
  lbl_progress->setAlignment(AlignLeft | AlignVCenter);
  lbl_progress->setFont(QFont( USglobal->config_list.fontFamily, 
                               USglobal->config_list.fontSize - 1, 
                               QFont::Bold));
  lbl_progress->setPalette(QPalette(USglobal->global_colors.cg_label, 
                                    USglobal->global_colors.cg_label, 
                                    USglobal->global_colors.cg_label));


  progress = new QProgressBar(100, this, "Analysis Progress");
  Q_CHECK_PTR(progress);
  progress->setPalette(QPalette(temp_colors.cg_normal, temp_colors.cg_normal, temp_colors.cg_normal));
  progress->setFont(QFont( USglobal->config_list.fontFamily, 
                           USglobal->config_list.fontSize - 1, 
                           QFont::Bold));
  progress->setProgress(35);

  lbl_edit = new QLabel(tr("Edit Label"),this);
  Q_CHECK_PTR(lbl_edit);
  lbl_edit->setAlignment(AlignHCenter|AlignVCenter);
  lbl_edit->setFont(QFont( USglobal->config_list.fontFamily, 
                           USglobal->config_list.fontSize - 1, 
                           QFont::Bold));
  lbl_edit->setPalette(QPalette(temp_colors.cg_edit, temp_colors.cg_edit, temp_colors.cg_edit));

  lbl_lcd = new QLabel(tr("LCD Panel:"),this);
  Q_CHECK_PTR(lbl_lcd);
  lbl_lcd->setAlignment(AlignLeft | AlignVCenter);
  lbl_lcd->setFont(QFont( USglobal->config_list.fontFamily, 
                          USglobal->config_list.fontSize - 1, 
                          QFont::Bold));
  lbl_lcd->setPalette(QPalette(USglobal->global_colors.cg_label, 
                               USglobal->global_colors.cg_label, 
                               USglobal->global_colors.cg_label));

  lcd  = new QLCDNumber( 7, this, "lcd" );
  lcd->setPalette( QPalette(temp_colors.cg_lcd, temp_colors.cg_lcd, temp_colors.cg_lcd));
  lcd->setGeometry(xpos, ypos, width - 26, height);
  lcd->setFont(QFont( USglobal->config_list.fontFamily, 
                      USglobal->config_list.fontSize - 1, 
                      QFont::Bold));
  lcd->setSegmentStyle ( QLCDNumber::Filled );
  lcd->setMode( QLCDNumber::DEC );
  lcd->display(35);

  lbl_text = new QLabel(tr("Text Label"),this);
  Q_CHECK_PTR(lbl_text);
  lbl_text->setAlignment(AlignHCenter|AlignVCenter);
  lbl_text->setFont(QFont( USglobal->config_list.fontFamily, 
                           USglobal->config_list.fontSize - 1, 
                           QFont::Bold));
  lbl_text->setPalette(QPalette(USglobal->global_colors.cg_label, 
                                USglobal->global_colors.cg_label, 
                                USglobal->global_colors.cg_label));

  lbl_counter = new QLabel(tr("Counter:"),this);
  Q_CHECK_PTR(lbl_counter);
  lbl_counter->setAlignment(AlignLeft | AlignVCenter);
  lbl_counter->setFont(QFont( USglobal->config_list.fontFamily, 
                              USglobal->config_list.fontSize - 1, 
                              QFont::Bold));
  lbl_counter->setPalette(QPalette(USglobal->global_colors.cg_label, 
                                   USglobal->global_colors.cg_label, 
                                   USglobal->global_colors.cg_label));

  cnt = new QwtCounter(this);
  Q_CHECK_PTR(cnt);
  cnt->setNumButtons(2);
  cnt->setRange(0, 100, 1);
  cnt->setPalette( QPalette(temp_colors.cg_normal, temp_colors.cg_normal, temp_colors.cg_normal));
  cnt->setValue(35);
  cnt->setFont(QFont( USglobal->config_list.fontFamily, 
                      USglobal->config_list.fontSize - 1, 
                      QFont::Bold));
  connect(cnt, SIGNAL(buttonReleased(double)), SLOT(update_widgets(double)));
  connect(cnt, SIGNAL(valueChanged(double)), SLOT(update_widgets(double)));
  

  pb_normal = new QPushButton(tr("Pushbutton"), this);
  Q_CHECK_PTR(pb_normal);
  pb_normal->setAutoDefault(false);
  pb_normal->setFont(QFont( USglobal->config_list.fontFamily, 
                            USglobal->config_list.fontSize));
  pb_normal->setPalette( QPalette(temp_colors.cg_pushb, temp_colors.cg_pushb_disabled, temp_colors.cg_pushb));


  pb_active = new QPushButton(tr("Active"), this);
  Q_CHECK_PTR(pb_active);
  pb_active->setAutoDefault(false);
  pb_active->setDown(true);
  pb_active->setFont(QFont( USglobal->config_list.fontFamily, 
                            USglobal->config_list.fontSize));
  pb_active->setPalette( QPalette(temp_colors.cg_pushb, temp_colors.cg_pushb, temp_colors.cg_pushb));
  pb_active->setGeometry(xpos, ypos, width/2, height);

  pb_disabled = new QPushButton(tr("Disabled"), this);
  Q_CHECK_PTR(pb_disabled);
  pb_disabled->setAutoDefault(false);
  pb_disabled->setFont(QFont( USglobal->config_list.fontFamily, 
                              USglobal->config_list.fontSize));
  pb_disabled->setEnabled(false);
  pb_disabled->setPalette( QPalette(temp_colors.cg_pushb, temp_colors.cg_pushb_disabled, temp_colors.cg_pushb));

  pb_reset = new QPushButton(tr("Reset"), this);
  Q_CHECK_PTR(pb_reset);
  pb_reset->setAutoDefault(false);
  pb_reset->setFont(QFont( USglobal->config_list.fontFamily, 
                           USglobal->config_list.fontSize));
  pb_reset->setPalette( QPalette(USglobal->global_colors.cg_pushb, 
                                 USglobal->global_colors.cg_pushb_disabled, 
                                 USglobal->global_colors.cg_pushb)); 
  connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));

  pb_help = new QPushButton(tr("Help"), this);
  Q_CHECK_PTR(pb_help);
  pb_help->setAutoDefault(false);
  pb_help->setFont(QFont( USglobal->config_list.fontFamily, 
                          USglobal->config_list.fontSize));
  pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, 
                                USglobal->global_colors.cg_pushb_disabled, 
                                USglobal->global_colors.cg_pushb));
  connect(pb_help, SIGNAL(clicked()), SLOT(help()));


  pb_apply = new QPushButton(tr("Apply"), this);
  Q_CHECK_PTR(pb_apply);
  pb_apply->setAutoDefault(false);
  pb_apply->setFont(QFont( USglobal->config_list.fontFamily, 
                           USglobal->config_list.fontSize));
  pb_apply->setPalette( QPalette(USglobal->global_colors.cg_pushb, 
                                 USglobal->global_colors.cg_pushb_disabled, 
                                 USglobal->global_colors.cg_pushb));
  connect(pb_apply, SIGNAL(clicked()), SLOT(apply()));

  pb_quit = new QPushButton(tr("Close"), this);
  Q_CHECK_PTR(pb_quit);
  pb_quit->setAutoDefault(false);
  pb_quit->setFont(QFont( USglobal->config_list.fontFamily, 
                          USglobal->config_list.fontSize));
  pb_quit->setPalette( QPalette(USglobal->global_colors.cg_pushb, 
                                USglobal->global_colors.cg_pushb_disabled, 
                                USglobal->global_colors.cg_pushb));
  connect(pb_quit, SIGNAL(clicked()), SLOT(quit()));

  pb_delete = new QPushButton(tr("Delete"), this);
  Q_CHECK_PTR(pb_delete);
  pb_delete->setAutoDefault(false);
  pb_delete->setFont(QFont( USglobal->config_list.fontFamily, 
                            USglobal->config_list.fontSize));
  pb_delete->setPalette( QPalette(USglobal->global_colors.cg_pushb, 
                                  USglobal->global_colors.cg_pushb_disabled, 
                                  USglobal->global_colors.cg_pushb));
  connect(pb_delete, SIGNAL(clicked()), SLOT(delete_scheme()));


  lbl_assign = new QLabel(tr("Assign new Colors:"),this);
  Q_CHECK_PTR(lbl_assign);
  lbl_assign->setFrameStyle(QFrame::WinPanel|Raised);
  lbl_assign->setAlignment(AlignHCenter|AlignVCenter);
  lbl_assign->setFont(QFont( USglobal->config_list.fontFamily, 
                             USglobal->config_list.fontSize, 
                             QFont::Bold));
  lbl_assign->setPalette(QPalette(USglobal->global_colors.cg_frame, 
                                  USglobal->global_colors.cg_frame, 
                                  USglobal->global_colors.cg_frame));
  lbl_assign->setMinimumHeight(30);


  lbl_select = new QLabel(tr("Selected Screen Element:"),this);
  Q_CHECK_PTR(lbl_select);
  lbl_select->setAlignment(AlignHCenter|AlignVCenter);
  lbl_select->setFont(QFont( USglobal->config_list.fontFamily, 
                             USglobal->config_list.fontSize - 1, 
                             QFont::Bold));
  lbl_select->setPalette(QPalette(USglobal->global_colors.cg_label, 
                                  USglobal->global_colors.cg_label, 
                                  USglobal->global_colors.cg_label));
  
  lbl_choice = new QLabel("",this);
  Q_CHECK_PTR(lbl_choice);
  lbl_choice->setAlignment(AlignLeft|AlignVCenter);
  lbl_choice->setFont(QFont( USglobal->config_list.fontFamily, 
                             USglobal->config_list.fontSize - 1, 
                             QFont::Bold));
  lbl_choice->setPalette(QPalette(USglobal->global_colors.cg_edit, 
                                  USglobal->global_colors.cg_edit, 
                                  USglobal->global_colors.cg_edit));


  lbl_color1 = new QLabel("",this);
  Q_CHECK_PTR(lbl_color1);
  lbl_color1->setAlignment(AlignLeft|AlignVCenter);
  lbl_color1->setFont(QFont( USglobal->config_list.fontFamily, 
                             USglobal->config_list.fontSize - 1, 
                             QFont::Bold));
  lbl_color1->setPalette(QPalette(USglobal->global_colors.cg_label, 
                                  USglobal->global_colors.cg_label, 
                                  USglobal->global_colors.cg_label));
  
  color_field1 = new QLabel("", this);
  Q_CHECK_PTR(color_field1);
  color_field1->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
  color_field1->setFixedSize(22,22);
  
  pb_color1 = new QPushButton(tr("Change"), this);
  Q_CHECK_PTR(pb_color1);
  pb_color1->setAutoDefault(false);
  pb_color1->setFont(QFont( USglobal->config_list.fontFamily, 
                            USglobal->config_list.fontSize));
  pb_color1->setPalette( QPalette(USglobal->global_colors.cg_pushb, 
                                  USglobal->global_colors.cg_pushb_disabled, 
                                  USglobal->global_colors.cg_pushb));
  connect(pb_color1, SIGNAL(clicked()), SLOT(pick_color1()));

  lbl_color2 = new QLabel("",this);
  Q_CHECK_PTR(lbl_color2);
  lbl_color2->setAlignment(AlignLeft|AlignVCenter);
  lbl_color2->setFont(QFont( USglobal->config_list.fontFamily, 
                             USglobal->config_list.fontSize - 1, 
                             QFont::Bold));
  lbl_color2->setPalette(QPalette(USglobal->global_colors.cg_label, 
                                  USglobal->global_colors.cg_label, 
                                  USglobal->global_colors.cg_label));

  color_field2 = new QLabel("", this);
  Q_CHECK_PTR(color_field2);
  color_field2->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
  color_field2->setFixedSize(22,22);

  pb_color2 = new QPushButton(tr("Change"), this);
  Q_CHECK_PTR(pb_color2);
  pb_color2->setAutoDefault(false);
  pb_color2->setFont(QFont( USglobal->config_list.fontFamily, 
                            USglobal->config_list.fontSize));
  pb_color2->setPalette( QPalette(USglobal->global_colors.cg_pushb, 
                                  USglobal->global_colors.cg_pushb_disabled, 
                                  USglobal->global_colors.cg_pushb));
  connect(pb_color2, SIGNAL(clicked()), SLOT(pick_color2()));

  lbl_color3 = new QLabel("",this);
  Q_CHECK_PTR(lbl_color3);
  lbl_color3->setAlignment(AlignLeft|AlignVCenter);
  lbl_color3->setFont(QFont( USglobal->config_list.fontFamily, 
                             USglobal->config_list.fontSize - 1, 
                             QFont::Bold));
  lbl_color3->setPalette(QPalette(USglobal->global_colors.cg_label, 
                                  USglobal->global_colors.cg_label, 
                                  USglobal->global_colors.cg_label));

  color_field3 = new QLabel("", this);
  Q_CHECK_PTR(color_field3);
  color_field3->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
  color_field3->setFixedSize(22,22);
  
  pb_color3 = new QPushButton(tr("Change"), this);
  Q_CHECK_PTR(pb_color3);
  pb_color3->setAutoDefault(false);
  pb_color3->setFont(QFont( USglobal->config_list.fontFamily, 
                            USglobal->config_list.fontSize));
  pb_color3->setPalette( QPalette(USglobal->global_colors.cg_pushb, 
                                  USglobal->global_colors.cg_pushb_disabled, 
                                  USglobal->global_colors.cg_pushb));
  connect(pb_color3, SIGNAL(clicked()), SLOT(pick_color3()));

  lbl_color4 = new QLabel("",this);
  Q_CHECK_PTR(lbl_color4);
  lbl_color4->setAlignment(AlignLeft|AlignVCenter);
  lbl_color4->setFont(QFont( USglobal->config_list.fontFamily, 
                             USglobal->config_list.fontSize - 1, 
                             QFont::Bold));
  lbl_color4->setPalette(QPalette(USglobal->global_colors.cg_label, 
                                  USglobal->global_colors.cg_label, 
                                  USglobal->global_colors.cg_label));

  color_field4 = new QLabel("", this);
  Q_CHECK_PTR(color_field4);
  color_field4->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
  color_field4->setFixedSize(22,22);


  pb_color4 = new QPushButton(tr("Change"), this);
  Q_CHECK_PTR(pb_color4);
  pb_color4->setAutoDefault(false);
  pb_color4->setFont(QFont( USglobal->config_list.fontFamily, 
                            USglobal->config_list.fontSize));
  pb_color4->setPalette( QPalette(USglobal->global_colors.cg_pushb, 
                                  USglobal->global_colors.cg_pushb_disabled, 
                                  USglobal->global_colors.cg_pushb));
  connect(pb_color4, SIGNAL(clicked()), SLOT(pick_color4()));

  lbl_color5 = new QLabel("",this);
  Q_CHECK_PTR(lbl_color5);
  lbl_color5->setAlignment(AlignLeft|AlignVCenter);
  lbl_color5->setFont(QFont( USglobal->config_list.fontFamily, 
                             USglobal->config_list.fontSize - 1, 
                             QFont::Bold));
  lbl_color5->setPalette(QPalette(USglobal->global_colors.cg_label, 
                                  USglobal->global_colors.cg_label, 
                                  USglobal->global_colors.cg_label));
  
  color_field5 = new QLabel("", this);
  Q_CHECK_PTR(color_field5);
  color_field5->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
  color_field5->setFixedSize(22,22);
  

  pb_color5 = new QPushButton(tr("Change"), this);
  Q_CHECK_PTR(pb_color5);
  pb_color5->setAutoDefault(false);
  pb_color5->setFont(QFont( USglobal->config_list.fontFamily, 
                            USglobal->config_list.fontSize));
  pb_color5->setPalette( QPalette(USglobal->global_colors.cg_pushb, 
                                  USglobal->global_colors.cg_pushb_disabled, 
                                  USglobal->global_colors.cg_pushb));
  connect(pb_color5, SIGNAL(clicked()), SLOT(pick_color5()));

  lbl_color6 = new QLabel("",this);
  Q_CHECK_PTR(lbl_color6);
  lbl_color6->setAlignment(AlignLeft|AlignVCenter);
  lbl_color6->setFont(QFont( USglobal->config_list.fontFamily, 
                             USglobal->config_list.fontSize - 1, 
                             QFont::Bold));
  lbl_color6->setPalette(QPalette(USglobal->global_colors.cg_label, 
                                  USglobal->global_colors.cg_label, 
                                  USglobal->global_colors.cg_label));

  color_field6 = new QLabel("", this);
  Q_CHECK_PTR(color_field6);
  color_field6->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
  color_field6->setFixedSize(22,22);
  
  pb_color6 = new QPushButton(tr("Change"), this);
  Q_CHECK_PTR(pb_color6);
  pb_color6->setAutoDefault(false);
  pb_color6->setFont(QFont( USglobal->config_list.fontFamily, 
                            USglobal->config_list.fontSize));
  pb_color6->setPalette( QPalette(USglobal->global_colors.cg_pushb, 
                                  USglobal->global_colors.cg_pushb_disabled, 
                                  USglobal->global_colors.cg_pushb));
  connect(pb_color6, SIGNAL(clicked()), SLOT(pick_color6()));

  lbl_choices = new QLabel(tr("Please Make a Selection:"),this);
  Q_CHECK_PTR(lbl_choices);
  lbl_choices->setFrameStyle(QFrame::WinPanel|Raised);
  lbl_choices->setAlignment(AlignHCenter|AlignVCenter);
  lbl_choices->setFont(QFont( USglobal->config_list.fontFamily, 
                              USglobal->config_list.fontSize, 
                              QFont::Bold));
  lbl_choices->setPalette(QPalette(USglobal->global_colors.cg_frame, 
                                   USglobal->global_colors.cg_frame, 
                                   USglobal->global_colors.cg_frame));
  lbl_choices->setMinimumHeight(30);

  lbl_select_scheme = new QLabel(tr("Select Color Scheme:"),this);
  Q_CHECK_PTR(lbl_select_scheme);
  lbl_select_scheme->setAlignment(AlignHCenter|AlignVCenter);
  lbl_select_scheme->setFont(QFont( USglobal->config_list.fontFamily, 
                                    USglobal->config_list.fontSize - 1, 
                                    QFont::Bold));
  lbl_select_scheme->setPalette(QPalette(USglobal->global_colors.cg_label, 
                                         USglobal->global_colors.cg_label, 
                                         USglobal->global_colors.cg_label));
  
  lbl_select_element = new QLabel(tr("Select a Screen Element:"),this);
  Q_CHECK_PTR(lbl_select_element);
  lbl_select_element->setAlignment(AlignHCenter|AlignVCenter);
  lbl_select_element->setFont(QFont( USglobal->config_list.fontFamily, 
                                     USglobal->config_list.fontSize - 1, 
                                     QFont::Bold));
  lbl_select_element->setPalette(QPalette(USglobal->global_colors.cg_label, 
                                          USglobal->global_colors.cg_label, 
                                          USglobal->global_colors.cg_label));
  
  QDir scheme_dir1, scheme_dir2;
  QString str, filter = "*.col";
  QStringList entries1, entries2;

  QString etc = QDir::convertSeparators( USglobal->config_list.system_dir + "/etc");

  scheme_dir1.setPath( etc ); // Look for system wide color defs
  scheme_dir2.setPath(USglobal->config_list.root_dir);      // personal color defs
  
  scheme_dir1.setNameFilter(filter);
  scheme_dir2.setNameFilter(filter);
  entries1 = scheme_dir1.entryList();
  entries2 = scheme_dir2.entryList();

  for (QStringList::Iterator it = entries2.begin(); it != entries2.end(); ++it )
  {
    str = (*it).latin1();
    entries1 += str;      // combine the personal and the system wide color defs into a single list
  }
  
  entries1.sort();
  
  lb_scheme = new QListBox(this, "Select");
  Q_CHECK_PTR(lb_scheme);
  lb_scheme->insertItem(tr("Current Scheme"));
  lb_scheme->insertItem(tr("UltraScan Default"));
  for (QStringList::Iterator it = entries1.begin(); it != entries1.end(); ++it )
  {
    str = (*it).latin1();
    str.truncate(str.findRev(".", -1, false));  // strip the extension
    if (str != tr("UltraScan Default"))
    {
      lb_scheme->insertItem(str);
    }
  }
  lb_scheme->setPalette(QPalette(USglobal->global_colors.cg_normal, 
                                 USglobal->global_colors.cg_normal, 
                                 USglobal->global_colors.cg_normal));
  lb_scheme->setCurrentItem(0);
  lb_scheme->setFont(QFont( USglobal->config_list.fontFamily, 
                            USglobal->config_list.fontSize - 1, 
                            QFont::Bold));
  current_scheme = 0;

  connect(lb_scheme, SIGNAL(highlighted(int)), this, SLOT(selected_scheme(int)));
  connect(lb_scheme, SIGNAL(selected(int)), this, SLOT(selected_scheme(int)));

  lb_item = new QListBox(this, "Select");
  Q_CHECK_PTR(lb_item);
  lb_item->insertItem(tr("Frame and Banner"));
  lb_item->insertItem(tr("Pushbutton, normal"));
  lb_item->insertItem(tr("Pushbutton, disabled"));
  lb_item->insertItem(tr("Plot Frame"));
  lb_item->insertItem(tr("Plot Background"));
  lb_item->insertItem(tr("Plot Major Tickmarks"));
  lb_item->insertItem(tr("Plot Minor Tickmarks"));
  lb_item->insertItem(tr("Edit Label"));
  lb_item->insertItem(tr("Text Label"));
  lb_item->insertItem(tr("Widget Colors"));
  lb_item->insertItem(tr("LCD Panel"));
  
  lb_item->setPalette(QPalette(USglobal->global_colors.cg_normal, 
                               USglobal->global_colors.cg_normal, 
                               USglobal->global_colors.cg_normal));
  lb_item->setFont(QFont( USglobal->config_list.fontFamily, 
                          USglobal->config_list.fontSize - 1, 
                          QFont::Bold));

  connect(lb_item, SIGNAL(highlighted(int)), this, SLOT(selected_item(int)));
  lb_item->setCurrentItem(0);
  
  pb_save_as = new QPushButton(tr("Save Color Scheme as:"), this);
  Q_CHECK_PTR(pb_save_as);
  pb_save_as->setAutoDefault(false);
  pb_save_as->setFont(QFont( USglobal->config_list.fontFamily, 
                             USglobal->config_list.fontSize));
  pb_save_as->setPalette( QPalette(USglobal->global_colors.cg_pushb, 
                                   USglobal->global_colors.cg_pushb_disabled, 
                                   USglobal->global_colors.cg_pushb));
  connect(pb_save_as, SIGNAL(clicked()), SLOT(save_as()));

  le_save_as = new QLineEdit(save_str, this);
  Q_CHECK_PTR(le_save_as);
  le_save_as->setFont(QFont( USglobal->config_list.fontFamily, 
                             USglobal->config_list.fontSize - 1, 
                             QFont::Bold));
  le_save_as->setPalette(QPalette(USglobal->global_colors.cg_normal, 
                                  USglobal->global_colors.cg_normal, 
                                  USglobal->global_colors.cg_normal));
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
  int rows = 8, columns = 3, spacing = 2;
  
  QGridLayout * background = new QGridLayout(this,1,2,spacing*2); 
  QGridLayout * subGrid1 = new QGridLayout(rows, columns, spacing);
  for (int i=0; i<rows; i++)
  {
    subGrid1->setRowSpacing(i, 26);
  }
  subGrid1->addMultiCellWidget(plot,j,j,0,2);
  j++;
  subGrid1->addMultiCellWidget(lbl_margin,j,j,0,1);
  subGrid1->addWidget(cmbb_margin,j,2);
  j++;
  subGrid1->addMultiCellWidget(lbl_example,j,j,0,2);
  j++;
  QHBoxLayout *line1 = new QHBoxLayout(4);
  line1->addWidget(lbl_banner);
  line1->addWidget(lbl_edit);
  line1->addWidget(lbl_text);
  subGrid1->addMultiCellLayout(line1,j,j,0,2);
  j++;
  QHBoxLayout *line2 = new QHBoxLayout(4);
  line2->addWidget(pb_normal);
  line2->addWidget(pb_active);
  line2->addWidget(pb_disabled);
  subGrid1->addMultiCellLayout(line2,j,j,0,2);
  j++;

  subGrid1->addMultiCellWidget(lbl_progress,j,j,0,1);
  subGrid1->addWidget(progress,j,2);
  j++;
  subGrid1->addMultiCellWidget(lbl_lcd,j,j,0,1);
  subGrid1->addWidget(lcd,j,2);
  j++;
  subGrid1->addMultiCellWidget(lbl_counter,j,j,0,1);
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
  QHBoxLayout *button = new QHBoxLayout(2);
  button->addWidget(pb_reset);
  button->addWidget(pb_help);
  button->addWidget(pb_delete);
  button->addWidget(pb_apply);
  button->addWidget(pb_quit);
//  subGrid1->addMultiCellLayout(button,j,j,0,2);
  
  j=0;
  rows = 8, columns = 2, spacing = 2;

  QGridLayout * subGrid2 = new QGridLayout(rows, columns, spacing);
  for (int i=0; i<rows; i++)
  {
    subGrid2->setRowSpacing(i, 26);
  }
  subGrid2->addMultiCellWidget(lbl_assign,j,j,0,1);
  j++;
  subGrid2->addWidget(lbl_select,j,0);
  subGrid2->addWidget(lbl_choice,j,1);
  j++;
  QHBoxLayout *color1 = new QHBoxLayout(0);
  color1->addWidget(lbl_color1);
  color1->addWidget(color_field1);
  subGrid2->addLayout(color1,j,0);
  subGrid2->addWidget(pb_color1,j,1);
  j++;
  QHBoxLayout *color2 = new QHBoxLayout(0);
  color2->addWidget(lbl_color2);
  color2->addWidget(color_field2);
  subGrid2->addLayout(color2,j,0);
  subGrid2->addWidget(pb_color2,j,1);
  j++;
  QHBoxLayout *color3 = new QHBoxLayout(0);
  color3->addWidget(lbl_color3);
  color3->addWidget(color_field3);
  subGrid2->addLayout(color3,j,0);
  subGrid2->addWidget(pb_color3,j,1);
  j++;
  QHBoxLayout *color4 = new QHBoxLayout(0);
  color4->addWidget(lbl_color4);
  color4->addWidget(color_field4);
  subGrid2->addLayout(color4,j,0);
  subGrid2->addWidget(pb_color4,j,1);
  j++;
  QHBoxLayout *color5 = new QHBoxLayout(0);
  color5->addWidget(lbl_color5);
  color5->addWidget(color_field5);
  subGrid2->addLayout(color5,j,0);
  subGrid2->addWidget(pb_color5,j,1);
  j++;
  QHBoxLayout *color6 = new QHBoxLayout(0);
  color6->addWidget(lbl_color6);
  color6->addWidget(color_field6);
  subGrid2->addLayout(color6,j,0);
  subGrid2->addWidget(pb_color6,j,1);
  j++;
  subGrid2->addMultiCellWidget(lbl_choices,j,j,0,1);
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
  background->setColStretch(0,2);
  background->addLayout(subGrid2,0,1);
background->addMultiCellLayout(button,1,1,0,1);
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
  selected_scheme = lb_scheme->text(scheme);

  if (selected_scheme == "UltraScan Default")
  {
    USglobal->color_defaults();
    temp_colors = USglobal->global_colors;
    apply();
    return;
  }
  
  if ( scheme == 0 )
  {
    QString colfile = US_Config::get_home_dir(  ) + USCOLORS;
    f.setName(colfile);
  }
  else
  {
    colfile = USglobal->config_list.system_dir + "/etc/" + selected_scheme + ".col";
    colfile = QDir::convertSeparators( colfile);

    f.setName(colfile);
    
    if (! f.exists())    // if we can't find it in the system wide 
                         // shared directory, check the home directory
    {
      colfile = USglobal->config_list.root_dir + "/" + selected_scheme + ".col";
      colfile = QDir::convertSeparators( colfile);
      f.setName(colfile);
    }
  }
  if (f.exists())
  {
    if(f.open(IO_ReadOnly))
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
      lcd->setPalette(QPalette(temp_colors.cg_lcd, temp_colors.cg_lcd, temp_colors.cg_lcd));
      cnt->setPalette(QPalette(temp_colors.cg_normal, temp_colors.cg_normal, temp_colors.cg_normal));
      lbl_text->setPalette(QPalette(temp_colors.cg_label, temp_colors.cg_label, temp_colors.cg_label));
      lbl_edit->setPalette(QPalette(temp_colors.cg_edit, temp_colors.cg_edit, temp_colors.cg_edit));
      cmbb_margin->setPalette(QPalette(temp_colors.cg_normal, temp_colors.cg_normal, temp_colors.cg_normal));
      progress->setPalette(QPalette(temp_colors.cg_normal, temp_colors.cg_normal, temp_colors.cg_normal));
      plot->setGridMinPen(QPen(temp_colors.minor_ticks, 0, DotLine));
      plot->setGridMajPen(QPen(temp_colors.major_ticks, 0, DotLine));
      //plot->setPlotBackground(temp_colors.plot);    //old version
      plot->setCanvasBackground(temp_colors.plot);    //new version
      plot->setPalette(QPalette(temp_colors.cg_plot, temp_colors.cg_plot, temp_colors.cg_plot));
      lbl_background->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_margin->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_progress->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_lcd->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_counter->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_banner->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      pb_normal->setPalette(QPalette(temp_colors.cg_pushb, temp_colors.cg_pushb_disabled, temp_colors.cg_pushb));
      pb_active->setPalette(QPalette(temp_colors.cg_pushb, temp_colors.cg_pushb, temp_colors.cg_pushb));
      pb_disabled->setPalette(QPalette(temp_colors.cg_pushb_disabled, temp_colors.cg_pushb_disabled, temp_colors.cg_pushb_disabled));
      selected_item(current_widget);
    }
  }
}

void US_Color::selected_item(int item)
{
  current_widget = item;
  lbl_choice->setText(" " + lb_item->text(item));
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
      temp_color1 = temp_colors.cg_frame.foreground();
      temp_color2 = temp_colors.cg_frame.background();
      temp_color3 = temp_colors.cg_frame.light();
      temp_color4 = temp_colors.cg_frame.dark();
      temp_color5 = temp_colors.cg_frame.midlight();
      temp_color6 = temp_colors.cg_frame.shadow();
      lbl_color1->setText(tr(" Text Color:"));
      color_field1->setBackgroundColor(temp_colors.cg_frame.foreground());
      lbl_color2->setText(tr(" Background Color:"));
      color_field2->setBackgroundColor(temp_colors.cg_frame.background());
      lbl_color3->setText(tr(" Highlight Color:"));
      color_field3->setBackgroundColor(temp_colors.cg_frame.light());
      lbl_color4->setText(tr(" Medium Shadow Color:"));
      color_field4->setBackgroundColor(temp_colors.cg_frame.dark());
      lbl_color5->setText(tr(" Medium Highlight Color:"));
      color_field5->setBackgroundColor(temp_colors.cg_frame.midlight());
      lbl_color6->setText(tr(" Dark Shadow Color:"));
      color_field6->setBackgroundColor(temp_colors.cg_frame.shadow());
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
      temp_color1 = temp_colors.cg_pushb.buttonText();
      temp_color2 = temp_colors.cg_pushb.button();
      temp_color3 = temp_colors.cg_pushb.light();
      temp_color4 = temp_colors.cg_pushb.dark();
      temp_color5 = temp_colors.cg_pushb.mid();
      lbl_color1->setText(tr(" Text Color:"));
      color_field1->setBackgroundColor(temp_colors.cg_pushb.text());
      lbl_color2->setText(tr(" Normal Button Color:"));
      color_field2->setBackgroundColor(temp_colors.cg_pushb.button());
      lbl_color3->setText(tr(" Highlight Color:"));
      color_field3->setBackgroundColor(temp_colors.cg_pushb.light());
      lbl_color4->setText(tr(" Shadow Color:"));
      color_field4->setBackgroundColor(temp_colors.cg_pushb.dark());
      lbl_color5->setText(tr(" Active Button Color:"));
      color_field5->setBackgroundColor(temp_colors.cg_pushb.mid());
      lbl_color6->setText("");
      color_field6->setBackgroundColor(Qt::gray);
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
      temp_color1 = temp_colors.cg_pushb_disabled.buttonText();
      temp_color2 = temp_colors.cg_pushb_disabled.button();
      temp_color3 = temp_colors.cg_pushb_disabled.light();
      temp_color4 = temp_colors.cg_pushb_disabled.dark();
      lbl_color1->setText(tr(" Text Color:"));
      color_field1->setBackgroundColor(temp_colors.cg_pushb_disabled.buttonText());
      lbl_color2->setText(tr(" Background Color:"));
      color_field2->setBackgroundColor(temp_colors.cg_pushb_disabled.button());
      lbl_color3->setText(tr(" Highlight Color:"));
      color_field3->setBackgroundColor(temp_colors.cg_pushb_disabled.light());
      lbl_color4->setText(tr(" Shadow Color:"));
      color_field4->setBackgroundColor(temp_colors.cg_pushb_disabled.dark());
      lbl_color5->setText("");
      color_field5->setBackgroundColor(Qt::gray);
      lbl_color6->setText("");
      color_field6->setBackgroundColor(Qt::gray);
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
      temp_color1 = temp_colors.cg_plot.text();
      temp_color2 = temp_colors.cg_plot.background();
      temp_color3 = temp_colors.cg_plot.light();
      temp_color4 = temp_colors.cg_plot.dark();
      lbl_color1->setText(tr(" Text Color:"));
      color_field1->setBackgroundColor(temp_colors.cg_plot.text());
      lbl_color2->setText(tr(" Background Color:"));
      color_field2->setBackgroundColor(temp_colors.cg_plot.background());
      lbl_color3->setText(tr(" Highlight Color:"));
      color_field3->setBackgroundColor(temp_colors.cg_plot.light());
      lbl_color4->setText(tr(" Shadow Color:"));
      color_field4->setBackgroundColor(temp_colors.cg_plot.dark());
      lbl_color5->setText("");
      color_field5->setBackgroundColor(Qt::gray);
      lbl_color6->setText("");
      color_field6->setBackgroundColor(Qt::gray);
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
      lbl_color1->setText(tr(" Plot Background Color:"));
      color_field1->setBackgroundColor(temp_colors.plot);
      lbl_color2->setText("");
      color_field2->setBackgroundColor(Qt::gray);
      lbl_color3->setText("");
      color_field3->setBackgroundColor(Qt::gray);
      lbl_color4->setText("");
      color_field4->setBackgroundColor(Qt::gray);
      lbl_color5->setText("");
      color_field5->setBackgroundColor(Qt::gray);
      lbl_color6->setText("");
      color_field6->setBackgroundColor(Qt::gray);
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
      lbl_color1->setText(tr(" Major Tick Color:"));
      color_field1->setBackgroundColor(temp_colors.major_ticks);
      lbl_color2->setText("");
      color_field2->setBackgroundColor(Qt::gray);
      lbl_color3->setText("");
      color_field3->setBackgroundColor(Qt::gray);
      lbl_color4->setText("");
      color_field4->setBackgroundColor(Qt::gray);
      lbl_color5->setText("");
      color_field5->setBackgroundColor(Qt::gray);
      lbl_color6->setText("");
      color_field6->setBackgroundColor(Qt::gray);
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
      lbl_color1->setText(tr(" Minor Tick Color:"));
      color_field1->setBackgroundColor(temp_colors.minor_ticks);
      lbl_color2->setText("");
      color_field2->setBackgroundColor(Qt::gray);
      lbl_color3->setText("");
      color_field3->setBackgroundColor(Qt::gray);
      lbl_color4->setText("");
      color_field4->setBackgroundColor(Qt::gray);
      lbl_color5->setText("");
      color_field5->setBackgroundColor(Qt::gray);
      lbl_color6->setText("");
      color_field6->setBackgroundColor(Qt::gray);
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
      temp_color1 = temp_colors.cg_edit.text();
      temp_color2 = temp_colors.cg_edit.background();
      lbl_color1->setText(tr(" Text Color:"));
      color_field1->setBackgroundColor(temp_colors.cg_edit.foreground());
      lbl_color2->setText(tr(" Background Color:"));
      color_field2->setBackgroundColor(temp_colors.cg_edit.background());
      lbl_color3->setText("");
      color_field3->setBackgroundColor(Qt::gray);
      lbl_color4->setText("");
      color_field4->setBackgroundColor(Qt::gray);
      lbl_color5->setText("");
      color_field5->setBackgroundColor(Qt::gray);
      lbl_color6->setText("");
      color_field6->setBackgroundColor(Qt::gray);
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
      temp_color1 = temp_colors.cg_label.text();
      temp_color2 = temp_colors.cg_label.background();
      lbl_color1->setText(tr(" Text Color:"));
      color_field1->setBackgroundColor(temp_colors.cg_label.foreground());
      lbl_color2->setText(tr(" Background Color:"));
      color_field2->setBackgroundColor(temp_colors.cg_label.background());
      lbl_color3->setText("");
      color_field3->setBackgroundColor(Qt::gray);
      lbl_color4->setText("");
      color_field4->setBackgroundColor(Qt::gray);
      lbl_color5->setText("");
      color_field5->setBackgroundColor(Qt::gray);
      lbl_color6->setText("");
      color_field6->setBackgroundColor(Qt::gray);
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
      temp_color1 = temp_colors.cg_normal.text();
      temp_color2 = temp_colors.cg_normal.background();
      temp_color3 = temp_colors.cg_normal.light();
      temp_color4 = temp_colors.cg_normal.dark();
      temp_color5 = temp_colors.cg_normal.highlight();
      temp_color6 = temp_colors.cg_normal.button();
      lbl_color1->setText(tr(" Text Color:"));
      color_field1->setBackgroundColor(temp_colors.cg_normal.text());
      lbl_color2->setText(tr(" Background Color:"));
      color_field2->setBackgroundColor(temp_colors.cg_normal.background());
      lbl_color3->setText(tr(" Highlight Color:"));
      color_field3->setBackgroundColor(temp_colors.cg_normal.light());
      lbl_color4->setText(tr(" Shadow Color:"));
      color_field4->setBackgroundColor(temp_colors.cg_normal.dark());
      lbl_color5->setText(tr(" Medium Highlight Color:"));
      color_field5->setBackgroundColor(temp_colors.cg_normal.highlight());
      lbl_color6->setText(tr(" Button Color:"));
      color_field6->setBackgroundColor(temp_colors.cg_normal.button());
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
      temp_color1 = temp_colors.cg_lcd.foreground();
      temp_color2 = temp_colors.cg_lcd.background();
      temp_color3 = temp_colors.cg_lcd.light();
      temp_color4 = temp_colors.cg_lcd.dark();
      lbl_color1->setText(tr(" Text Center Color:"));
      color_field1->setBackgroundColor(temp_colors.cg_lcd.foreground());
      lbl_color2->setText(tr(" Background Color:"));
      color_field2->setBackgroundColor(temp_colors.cg_lcd.background());
      lbl_color3->setText(tr(" Highlight Color:"));
      color_field3->setBackgroundColor(temp_colors.cg_lcd.light());
      lbl_color4->setText(tr(" Shadow Color:"));
      color_field4->setBackgroundColor(temp_colors.cg_lcd.dark());
      lbl_color5->setText("");
      color_field5->setBackgroundColor(Qt::gray);
      lbl_color6->setText("");
      color_field6->setBackgroundColor(Qt::gray);
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
  color_field1->setBackgroundColor(color1);
  temp_color1 = color1;
//  color_field1->setBackgroundColor(temp_colors.cg_pushb.buttonText());
  switch (current_widget)
  {
    case 0:
    {
      temp_colors.cg_frame.setColor(QColorGroup::Foreground, color1);
      float v = (int) (0.2125 * color1.red() + 0.0721 * color1.blue() + 0.7154 * color1.green());
      if (v > 128)
      {
        temp_colors.cg_frame.setColor(QColorGroup::Base, black);
      }
      else
      {
        temp_colors.cg_frame.setColor(QColorGroup::Base, Qt::white);
      }
      lbl_background->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_margin->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_progress->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_lcd->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_counter->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_banner->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      break;
    }
    case 1:
    {
      temp_colors.cg_pushb.setColor(QColorGroup::ButtonText, color1);
      pb_normal->setPalette(QPalette(temp_colors.cg_pushb, temp_colors.cg_pushb_disabled, temp_colors.cg_pushb));
      pb_active->setPalette(QPalette(temp_colors.cg_pushb, temp_colors.cg_pushb, temp_colors.cg_pushb));
      break;
    }
    case 2:
    {
      temp_colors.cg_pushb_disabled.setColor(QColorGroup::Text, color1);
      pb_disabled->setPalette(QPalette(temp_colors.cg_pushb_disabled, temp_colors.cg_pushb_disabled, temp_colors.cg_pushb_disabled));
      break;
    }
    case 3:
    {
      temp_colors.cg_plot.setColor(QColorGroup::Text, color1);
      plot->setPalette(QPalette(temp_colors.cg_plot, temp_colors.cg_plot, temp_colors.cg_plot));
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
      plot->setGridMajPen(QPen(temp_colors.major_ticks, 0, DotLine));
      plot->replot();
      break;
    }
    case 6:
    {
      temp_colors.minor_ticks = color1;
      plot->setGridMinPen(QPen(temp_colors.minor_ticks, 0, DotLine));
      plot->replot();
      break;
    }
    case 7:
    {
      temp_colors.cg_edit.setColor(QColorGroup::Foreground, color1);
      lbl_edit->setPalette(QPalette(temp_colors.cg_edit, temp_colors.cg_edit, temp_colors.cg_edit));
      break;
    }
    case 8:
    {
      temp_colors.cg_label.setColor(QColorGroup::Foreground, color1);
      lbl_text->setPalette(QPalette(temp_colors.cg_label, temp_colors.cg_label, temp_colors.cg_label));
      break;
    }
    case 9:
    {
      temp_colors.cg_normal.setColor(QColorGroup::Text, color1);
      cmbb_margin->setPalette(QPalette(temp_colors.cg_normal, temp_colors.cg_normal, temp_colors.cg_normal));
      progress->setPalette(QPalette(temp_colors.cg_normal, temp_colors.cg_normal, temp_colors.cg_normal));
      cnt->setPalette(QPalette(temp_colors.cg_normal, temp_colors.cg_normal, temp_colors.cg_normal));
      break;
    }
    case 10:
    {
      temp_colors.cg_lcd.setColor(QColorGroup::Foreground, color1);
      lcd->setPalette(QPalette(temp_colors.cg_lcd, temp_colors.cg_lcd, temp_colors.cg_lcd));
      break;
    }
  }
}

void US_Color::pick_color2()
{
  color2 = QColorDialog::getColor(temp_color2, this);
  color_field2->setBackgroundColor(color2);
  temp_color2 = color2;
  switch (current_widget)
  {
    case 0:
    {
      temp_colors.cg_frame.setColor(QColorGroup::Background, color2);
      lbl_background->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_margin->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_progress->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_lcd->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_counter->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_banner->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      break;
    }
    case 1:
    {
      temp_colors.cg_pushb.setColor(QColorGroup::Button, color2);
      pb_normal->setPalette(QPalette(temp_colors.cg_pushb, temp_colors.cg_pushb_disabled, temp_colors.cg_pushb));
      pb_active->setPalette(QPalette(temp_colors.cg_pushb, temp_colors.cg_pushb, temp_colors.cg_pushb));
      break;
    }
    case 2:
    {
      temp_colors.cg_pushb_disabled.setColor(QColorGroup::Button, color2);
      pb_disabled->setPalette(QPalette(temp_colors.cg_pushb_disabled, temp_colors.cg_pushb_disabled, temp_colors.cg_pushb_disabled));
      break;
    }
    case 3:
    {
      temp_colors.cg_plot.setColor(QColorGroup::Background, color2);
      plot->setPalette(QPalette(temp_colors.cg_plot, temp_colors.cg_plot, temp_colors.cg_plot));
      break;
    }
    case 7:
    {
      temp_colors.cg_edit.setColor(QColorGroup::Background, color2);
      lbl_edit->setPalette(QPalette(temp_colors.cg_edit, temp_colors.cg_edit, temp_colors.cg_edit));
      break;
    }
    case 8:
    {
      temp_colors.cg_label.setColor(QColorGroup::Background, color2);
      lbl_text->setPalette(QPalette(temp_colors.cg_label, temp_colors.cg_label, temp_colors.cg_label));
      break;
    }
    case 9:
    {
      temp_colors.cg_normal.setColor(QColorGroup::Background, color2);
      cmbb_margin->setPalette(QPalette(temp_colors.cg_normal, temp_colors.cg_normal, temp_colors.cg_normal));
      progress->setPalette(QPalette(temp_colors.cg_normal, temp_colors.cg_normal, temp_colors.cg_normal));
      cnt->setPalette(QPalette(temp_colors.cg_normal, temp_colors.cg_normal, temp_colors.cg_normal));
      break;
    }
    case 10:
    {
      temp_colors.cg_lcd.setColor(QColorGroup::Background, color2);
      lcd->setPalette(QPalette(temp_colors.cg_lcd, temp_colors.cg_lcd, temp_colors.cg_lcd));
      break;
    }
  }
}

void US_Color::pick_color3()
{
  color3 = QColorDialog::getColor(temp_color3, this);
  color_field3->setBackgroundColor(color3);
  temp_color3 = color3;
  switch (current_widget)
  {
    case 0:
    {
      temp_colors.cg_frame.setColor(QColorGroup::Light, color3);
      lbl_background->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_margin->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_progress->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_lcd->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_counter->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_banner->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      break;
    }
    case 1:
    {
      temp_colors.cg_pushb.setColor(QColorGroup::Light, color3);
      pb_normal->setPalette(QPalette(temp_colors.cg_pushb, temp_colors.cg_pushb_disabled, temp_colors.cg_pushb));
      pb_active->setPalette(QPalette(temp_colors.cg_pushb, temp_colors.cg_pushb, temp_colors.cg_pushb));
      break;
    }
    case 2:
    {
      temp_colors.cg_pushb_disabled.setColor(QColorGroup::Light, color3);
      pb_disabled->setPalette(QPalette(temp_colors.cg_pushb_disabled, temp_colors.cg_pushb_disabled, temp_colors.cg_pushb_disabled));
      break;
    }
    case 3:
    {
      temp_colors.cg_plot.setColor(QColorGroup::Light, color3);
      plot->setPalette(QPalette(temp_colors.cg_plot, temp_colors.cg_plot, temp_colors.cg_plot));
      break;
    }
    case 9:
    {
      temp_colors.cg_normal.setColor(QColorGroup::Light, color3);
      cmbb_margin->setPalette(QPalette(temp_colors.cg_normal, temp_colors.cg_normal, temp_colors.cg_normal));
      progress->setPalette(QPalette(temp_colors.cg_normal, temp_colors.cg_normal, temp_colors.cg_normal));
      cnt->setPalette(QPalette(temp_colors.cg_normal, temp_colors.cg_normal, temp_colors.cg_normal));
      break;
    }
    case 10:
    {
      temp_colors.cg_lcd.setColor(QColorGroup::Light, color3);
      lcd->setPalette(QPalette(temp_colors.cg_lcd, temp_colors.cg_lcd, temp_colors.cg_lcd));
      break;
    }
  }
}

void US_Color::pick_color4()
{
  color4 = QColorDialog::getColor(temp_color4, this);
  color_field4->setBackgroundColor(color4);
  temp_color4 = color4;
  switch (current_widget)
  {
    case 0:
    {
      temp_colors.cg_frame.setColor(QColorGroup::Dark, color4);
      lbl_background->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_margin->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_progress->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_lcd->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_counter->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_banner->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      break;
    }
    case 1:
    {
      temp_colors.cg_pushb.setColor(QColorGroup::Dark, color4);
      pb_normal->setPalette(QPalette(temp_colors.cg_pushb, temp_colors.cg_pushb_disabled, temp_colors.cg_pushb));
      pb_active->setPalette(QPalette(temp_colors.cg_pushb, temp_colors.cg_pushb, temp_colors.cg_pushb));
      break;
    }
    case 2:
    {
      temp_colors.cg_pushb_disabled.setColor(QColorGroup::Dark, color4);
      pb_disabled->setPalette(QPalette(temp_colors.cg_pushb_disabled, temp_colors.cg_pushb_disabled, temp_colors.cg_pushb_disabled));
      break;
    }
    case 3:
    {
      temp_colors.cg_plot.setColor(QColorGroup::Dark, color4);
      plot->setPalette(QPalette(temp_colors.cg_plot, temp_colors.cg_plot, temp_colors.cg_plot));
      break;
    }
    case 9:
    {
      temp_colors.cg_normal.setColor(QColorGroup::Dark, color4);
      cmbb_margin->setPalette(QPalette(temp_colors.cg_normal, temp_colors.cg_normal, temp_colors.cg_normal));
      progress->setPalette(QPalette(temp_colors.cg_normal, temp_colors.cg_normal, temp_colors.cg_normal));
      cnt->setPalette(QPalette(temp_colors.cg_normal, temp_colors.cg_normal, temp_colors.cg_normal));
      break;
    }
    case 10:
    {
      temp_colors.cg_lcd.setColor(QColorGroup::Dark, color4);
      lcd->setPalette(QPalette(temp_colors.cg_lcd, temp_colors.cg_lcd, temp_colors.cg_lcd));
      break;
    }
  }
}

void US_Color::pick_color5()
{
  color5 = QColorDialog::getColor(temp_color5, this);
  color_field5->setBackgroundColor(color5);
  temp_color5 = color5;
  switch (current_widget)
  {
    case 0:
    {
      temp_colors.cg_frame.setColor(QColorGroup::Midlight, color5);
      lbl_background->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_margin->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_progress->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_lcd->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_counter->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_banner->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      break;
    }
    case 1:
    {
      temp_colors.cg_pushb.setColor(QColorGroup::Mid, color5);
      pb_active->setPalette(QPalette(temp_colors.cg_pushb, temp_colors.cg_pushb_disabled, temp_colors.cg_pushb));
      break;
    }
    case 9:
    {
      temp_colors.cg_normal.setColor(QColorGroup::Highlight, color5);
      cmbb_margin->setPalette(QPalette(temp_colors.cg_normal, temp_colors.cg_normal, temp_colors.cg_normal));
      progress->setPalette(QPalette(temp_colors.cg_normal, temp_colors.cg_normal, temp_colors.cg_normal));
      cnt->setPalette(QPalette(temp_colors.cg_normal, temp_colors.cg_normal, temp_colors.cg_normal));
      break;
    }
  }
}

void US_Color::pick_color6()
{
  color6 = QColorDialog::getColor(temp_color6, this);
  color_field6->setBackgroundColor(color6);
  temp_color6 = color6;
  switch (current_widget)
  {
    case 0:
    {
      temp_colors.cg_frame.setColor(QColorGroup::Shadow, color6);
      lbl_background->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_margin->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_progress->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_lcd->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_counter->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      lbl_banner->setPalette(QPalette(temp_colors.cg_frame, temp_colors.cg_frame, temp_colors.cg_frame));
      break;
    }
    case 9:
    {
      temp_colors.cg_normal.setColor(QColorGroup::Button, color6);
      cmbb_margin->setPalette(QPalette(temp_colors.cg_normal, temp_colors.cg_normal, temp_colors.cg_normal));
      progress->setPalette(QPalette(temp_colors.cg_normal, temp_colors.cg_normal, temp_colors.cg_normal));
      cnt->setPalette(QPalette(temp_colors.cg_normal, temp_colors.cg_normal, temp_colors.cg_normal));
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
  cmbb_margin->setCurrentItem(temp_margin/2-1);
  apply();
  selected_item(current_widget);
}

void US_Color::delete_scheme()
{
  QString str = tr("Do you really want to delete the\n \"") + 
               lb_scheme->text(current_scheme) + tr("\" color scheme?");
  int result = QMessageBox::warning(0, 
        tr("Attention:"), str, tr("Yes"), tr("No"), QString::null, 1, 1);
  if (result == 0)
  {
    if (current_scheme == 0)
    {
      QMessageBox::message(
          tr("Attention:"), 
          tr("Sorry, the current scheme cannot\n"
             "be deleted. Please try again..."));
      return;
    }
    if (current_scheme == 1)
    {
      QMessageBox::message(
          tr("Attention:"), 
          tr("Sorry, you cannot delete the\n"
             "UltraScan Default color scheme.\n"
             "Please try again..."));
      return;
    }
    QDir scheme_dir1;
    QString filter = "*.col";
    QStringList entries1;
    QString path = QDir::convertSeparators( USglobal->config_list.system_dir + "/etc" );

    scheme_dir1.setPath( path ); // system wide color defs
    scheme_dir1.setNameFilter(filter);
    entries1 = scheme_dir1.entryList();
    QStringList::Iterator it;

    // make sure it's not a system color definition  
    for ( it = entries1.begin(); it != entries1.end(); ++it ) 
    {
      str = (*it).latin1();
      str.truncate(str.findRev(".", -1, false));  // strip the extension
      if (str == lb_scheme->text(current_scheme))
      {
        QMessageBox::message(
            tr("Attention:"), 
            tr("Sorry, this color definition\n"
               "is a system resource and cannot\n"
               "be deleted. Please try again..."));
        return;
      }
    }
    
    QString colfile = QDir::convertSeparators( USglobal->config_list.root_dir + 
                      "/" + lb_scheme->text(current_scheme) + ".col" );
    QFile f( colfile);
    if (! f.remove() )
    {
      QMessageBox::message(
          tr("Attention:"), 
          tr("Sorry, couldn't delete the\n"
             "associated file for this scheme\n"
             "- please check the file permissions..."));
    }
    lb_scheme->removeItem(current_scheme);
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

  pb_quit->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
  pb_delete->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
  pb_save_as->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
  pb_help->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
  pb_reset->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
  pb_apply->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
  pb_normal->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
  pb_active->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb));
  pb_disabled->setPalette(QPalette(USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_disabled));
  pb_color1->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
  pb_color2->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
  pb_color3->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
  pb_color4->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
  pb_color5->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
  pb_color6->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
  plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
  plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
  plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
  plot->setCanvasBackground(temp_colors.plot);
  plot->setMargin(USglobal->config_list.margin);
  cnt->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
  cmbb_margin->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
  progress->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
  lcd->setPalette(QPalette(USglobal->global_colors.cg_lcd, USglobal->global_colors.cg_lcd, USglobal->global_colors.cg_lcd));
  lbl_background->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
  lbl_banner->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
  lbl_margin->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
  lbl_progress->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
  lbl_counter->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
  lbl_lcd->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
  lbl_select->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
  lbl_assign->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
  lbl_select_scheme->setPalette(QPalette(temp_colors.cg_label, temp_colors.cg_label, temp_colors.cg_label));
  lbl_select_element->setPalette(QPalette(temp_colors.cg_label, temp_colors.cg_label, temp_colors.cg_label));
  lbl_choice->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
  lbl_choices->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
  lbl_text->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
  lbl_edit->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
  lbl_color1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
  lbl_color2->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
  lbl_color3->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
  lbl_color4->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
  lbl_color5->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
  lbl_color6->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
  le_save_as->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
  lb_scheme->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
  lb_item->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
  setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));

  QFile f;

  QString colfile = US_Config::get_home_dir(  ) + USCOLORS;

  f.setName( colfile );

  if ( f.open ( IO_WriteOnly ) )
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
  
  QString dir = QDir::convertSeparators( USglobal->config_list.system_dir + "/etc");
  system_dir.setPath( dir );  // system wide color defs
  
  
  system_dir.setNameFilter(filter);
  entries = system_dir.entryList();
  
  QStringList::Iterator it;
  for ( it = entries.begin(); it != entries.end(); ++it )
  {
    str1 = (*it).latin1();
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
    QMessageBox::message(
        tr("Attention:"), 
        tr("Sorry - \n\nthis name is reserved for a system-\n"
           "wide color scheme.\n\n"
           "Please select a different name."));
    return;
  }
  problem = false;
  for (int i=0; i< lb_scheme->numRows(); i++) 
  {
    if (save_str == lb_scheme->text(i))   // if the item is already listed we don't need to list it again
    {                         // instead, we overwrite the existing color definition 
      problem = true;
    }
  }
  if (!problem)
  {
    lb_scheme->insertItem(save_str);
  }
  QString filename;
  filename = QDir::convertSeparators(USglobal->config_list.root_dir + "/" + save_str );
  
  if (filename.right(4) != ".col")
  {
    filename.append(".col");
  }

  QFile file( filename );

  if ( file.open( IO_WriteOnly ) )
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
    QMessageBox::message(
        tr("Warning:"), 
        tr("Unable to open color file for output!\n"
           "Please check your write permissions.\n\n"
           "Color definitions were not saved..."));
  }
}

void US_Color::update_widgets(double newval)
{
  lcd->display(newval);
  progress->reset();
  progress->setProgress((int) newval);
}

void US_Color::update_save_str(const QString &temp_str)
{
  save_str = temp_str;

}

void US_Color:: sel_margin(int newval)
{
  temp_margin = (newval+1)*2;
  plot->setMargin(temp_margin); 
}

