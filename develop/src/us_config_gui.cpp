#include "../include/us_config_gui.h"
#include <qfontmetrics.h>
#include <qmessagebox.h>

US_Config_GUI::US_Config_GUI(QWidget *parent, const char *name) : QFrame(parent, name)
{
   USglobal = new US_Config();

   if (USglobal->config_list.fontFamily.isEmpty() || 
       USglobal->config_list.fontSize == 0      || 
       USglobal->config_list.margin == 0)
   {
      USglobal->config_list.fontFamily = "Helvetica";
      USglobal->config_list.fontSize = 10;
      USglobal->config_list.margin = 10;
   }

   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption("UltraScan Configuration");

   lbl_directions = new QLabel(QObject::tr("UltraScan " + US_Version + " Configuration:"),this);
   lbl_directions->setFrameStyle(QFrame::WinPanel|Raised);
   lbl_directions->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_directions->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_directions->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_paths = new QLabel(QObject::tr(" Paths and File Names:"),this);
   lbl_paths->setFrameStyle(QFrame::WinPanel|Raised);
   lbl_paths->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_paths->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_paths->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   pb_browser = new QPushButton(QObject::tr(" WWW Browser:"),this);
   pb_browser->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
   pb_browser->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   US_Config::connect(pb_browser, SIGNAL(clicked()), this, SLOT(open_browser_dir()));

   le_browser = new QLineEdit(this);
   le_browser->setFont( QFont( USglobal->config_list.fontFamily, 
                               USglobal->config_list.fontSize - 1));

   le_browser->setPalette( QPalette( USglobal->global_colors.cg_edit, 
                                     USglobal->global_colors.cg_edit, 
                                     USglobal->global_colors.cg_edit));

   US_Config::connect(le_browser, SIGNAL(textChanged(const QString &)), this, SLOT(update_browser(const QString &)));

   pb_tmp_dir = new QPushButton(QObject::tr(" Temporary Directory:"),this);
   pb_tmp_dir->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
   pb_tmp_dir->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   US_Config::connect(pb_tmp_dir, SIGNAL(clicked()), this, SLOT(open_tmp_dir()));

   le_tmp_dir = new QLineEdit(this);
   le_tmp_dir->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_tmp_dir->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   US_Config::connect(le_tmp_dir, SIGNAL(textChanged(const QString &)), this, SLOT(update_tmp_dir(const QString &)));

   pb_root_dir = new QPushButton(QObject::tr(" User's UltraScan Directory:"),this);
   pb_root_dir->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
   pb_root_dir->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   US_Config::connect(pb_root_dir, SIGNAL(clicked()), this, SLOT(open_root_dir()));

   le_root_dir = new QLineEdit(this);
   le_root_dir->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_root_dir->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   US_Config::connect(le_root_dir, SIGNAL(textChanged(const QString &)), this, SLOT(update_root_dir(const QString &)));

   pb_data_dir = new QPushButton(QObject::tr(" Data Directory:"),this);
   pb_data_dir->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
   pb_data_dir->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   US_Config::connect(pb_data_dir, SIGNAL(clicked()), this, SLOT(open_data_dir()));

   le_data_dir = new QLineEdit(this);
   le_data_dir->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_data_dir->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   US_Config::connect(le_data_dir, SIGNAL(textChanged(const QString &)), this, SLOT(update_data_dir(const QString &)));

   pb_result_dir = new QPushButton(QObject::tr(" Result Directory:"),this);
   pb_result_dir->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
   pb_result_dir->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   US_Config::connect(pb_result_dir, SIGNAL(clicked()), this, SLOT(open_result_dir()));

   le_result_dir = new QLineEdit(this);
   le_result_dir->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_result_dir->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   US_Config::connect(le_result_dir, SIGNAL(textChanged(const QString &)), this, SLOT(update_result_dir(const QString &)));

   pb_html_dir = new QPushButton(QObject::tr(" HTML Reports:"),this);
   pb_html_dir->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
   pb_html_dir->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   US_Config::connect(pb_html_dir, SIGNAL(clicked()), this, SLOT(open_html_dir()));

   le_html_dir = new QLineEdit(this);
   le_html_dir->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_html_dir->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   US_Config::connect(le_html_dir, SIGNAL(textChanged(const QString &)), this, SLOT(update_html_dir(const QString &)));

   pb_archive_dir = new QPushButton(QObject::tr(" Archive Directory:"),this);
   pb_archive_dir->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
   pb_archive_dir->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   US_Config::connect(pb_archive_dir, SIGNAL(clicked()), this, SLOT(open_archive_dir()));

   le_archive_dir = new QLineEdit(this);
   le_archive_dir->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_archive_dir->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   US_Config::connect(le_archive_dir, SIGNAL(textChanged(const QString &)), this, SLOT(update_archive_dir(const QString &)));

   pb_system_dir = new QPushButton(QObject::tr(" UltraScan System:"),this);
   pb_system_dir->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
   pb_system_dir->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   US_Config::connect(pb_system_dir, SIGNAL(clicked()), this, SLOT(open_system_dir()));

   le_system_dir = new QLineEdit(this);
   le_system_dir->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_system_dir->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   US_Config::connect(le_system_dir, SIGNAL(textChanged(const QString &)), this, SLOT(update_system_dir(const QString &)));

   pb_help_dir = new QPushButton(QObject::tr(" Help Directory:"),this);
   pb_help_dir->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
   pb_help_dir->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   US_Config::connect(pb_help_dir, SIGNAL(clicked()), this, SLOT(open_help_dir()));

   le_help_dir = new QLineEdit(this);
   le_help_dir->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_help_dir->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   US_Config::connect(le_help_dir, SIGNAL(textChanged(const QString &)), this, SLOT(update_help_dir(const QString &)));

   lbl_misc = new QLabel(QObject::tr(" Miscellaneous Settings:"),this);
   lbl_misc->setFrameStyle(QFrame::WinPanel|Raised);
   lbl_misc->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_misc->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_misc->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_temperature_tol = new QLabel(QObject::tr(" Temperature Tolerance (ºC):"),this);
   lbl_temperature_tol->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_temperature_tol->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_temperature_tol->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_temperature_tol = new QLineEdit(this);
   le_temperature_tol->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_temperature_tol->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   US_Config::connect(le_temperature_tol, SIGNAL(textChanged(const QString &)), this, SLOT(update_temperature_tol(const QString &)));

   lbl_beckman_bug = new QLabel(QObject::tr(" Beckman Time Bug Correction:"),this);
   lbl_beckman_bug->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_beckman_bug->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_beckman_bug->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   bt_on = new QRadioButton(this);
   bt_on->setText(QObject::tr("on"));
   bt_on->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   bt_on->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   US_Config::connect(bt_on, SIGNAL(clicked()), this, SLOT(update_on_button()));

   bt_off = new QRadioButton(this);
   bt_off->setText(QObject::tr("off"));
   bt_off->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   bt_off->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   US_Config::connect(bt_off, SIGNAL(clicked()), this, SLOT(update_off_button()));

   lbl_color = new QLabel(QObject::tr(" Color Preferences:"),this);
   lbl_color->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_color->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_color->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   pb_color = new QPushButton(QObject::tr("Change"), this);
   Q_CHECK_PTR(pb_color);
   pb_color->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_color->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
   US_Config::connect(pb_color, SIGNAL(clicked()), this, SLOT(update_color()));

   lbl_font = new QLabel(QObject::tr(" Font Preferences:"),this);
   lbl_font->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_font->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_font->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   pb_font = new QPushButton(QObject::tr("Change"), this);
   Q_CHECK_PTR(pb_font);
   pb_font->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_font->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
   US_Config::connect(pb_font, SIGNAL(clicked()), this, SLOT(update_font()));

   lbl_database = new QLabel(QObject::tr(" Database Preferences:"),this);
   lbl_database->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_database->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_database->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   pb_database = new QPushButton(QObject::tr("Change"), this);
   pb_database->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_database->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
   US_Config::connect(pb_database, SIGNAL(clicked()), this, SLOT(update_database()));

   lbl_numThreads = new QLabel(QObject::tr(" Number of Threads:"),this);
   lbl_numThreads->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_numThreads->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_numThreads->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   cnt_numThreads= new QwtCounter(this);
   cnt_numThreads->setRange(1, MAX_THREADS, 1);
   cnt_numThreads->setValue(USglobal->config_list.numThreads);
   cnt_numThreads->setEnabled(true);
   cnt_numThreads->setNumButtons(2);
   cnt_numThreads->setPalette( QPalette( USglobal->global_colors.cg_normal, 
                                         USglobal->global_colors.cg_normal, 
                                         USglobal->global_colors.cg_normal));

   cnt_numThreads->setFont( QFont( USglobal->config_list.fontFamily, 
                                   USglobal->config_list.fontSize - 1));

   connect(cnt_numThreads, SIGNAL(valueChanged(double)), SLOT(update_numThreads(double)));

   pb_help = new QPushButton(QObject::tr("Help"), this);
   pb_help->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
   US_Config::connect(pb_help, SIGNAL(clicked()), this, SLOT(help()));

   pb_save = new QPushButton(QObject::tr("Save"), this);
   pb_save->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
   US_Config::connect(pb_save, SIGNAL(clicked()), this, SLOT(save()));

   pb_cancel = new QPushButton(QObject::tr("Cancel"), this);
   pb_cancel->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
   US_Config::connect(pb_cancel, SIGNAL(clicked()), this, SLOT(cancel()));

   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
   update_screen();
   setup_GUI();
}

US_Config_GUI::~US_Config_GUI()
{
}

void US_Config_GUI::setup_GUI()
{
   QFont*        font    = new QFont( USglobal->config_list.fontFamily, 
                                      USglobal->config_list.fontSize);

   QFontMetrics* fm      = new QFontMetrics ( *font );

   int cwidth  = fm->width( USglobal->config_list.browser );
   int w       = fm->width( USglobal->config_list.root_dir );
   
   w = ( cwidth > w ) ? cwidth : w;

   w = fm->width( USglobal->config_list.data_dir );
   w = ( cwidth > w ) ? cwidth : w;

   w = fm->width( USglobal->config_list.system_dir );
   w = ( cwidth > w ) ? cwidth : w;

   w = fm->width( USglobal->config_list.help_dir );
   w = ( cwidth > w ) ? cwidth : w;

   w = fm->width( USglobal->config_list.tmp_dir );
   w = ( cwidth > w ) ? cwidth : w;

   w = fm->width( USglobal->config_list.archive_dir );
   w = ( cwidth > w ) ? cwidth : w;

   w = fm->width( USglobal->config_list.result_dir );
   w = ( cwidth > w ) ? cwidth : w;

   w = fm->width( USglobal->config_list.html_dir );
   w = ( cwidth > w ) ? cwidth : w;


   QBoxLayout*   topbox = new QVBoxLayout(this,2);
   
   topbox->addWidget(lbl_directions);
   topbox->addWidget(lbl_paths);

   unsigned int j=0;
   
   QGridLayout* lineGrid = new QGridLayout( topbox, 10, 2, 2 );

   lineGrid->setColSpacing( 1, w + 10 );  // Set width + 10 pixels
   
   lineGrid->addWidget(pb_browser,j,0);
   lineGrid->addWidget(le_browser,j,1);
   j++;
   lineGrid->addWidget(pb_root_dir,j,0);
   lineGrid->addWidget(le_root_dir,j,1);
   j++;
   lineGrid->addWidget(pb_data_dir,j,0);
   lineGrid->addWidget(le_data_dir,j,1);
   j++;
   lineGrid->addWidget(pb_result_dir,j,0);
   lineGrid->addWidget(le_result_dir,j,1);
   j++;
   lineGrid->addWidget(pb_html_dir,j,0);
   lineGrid->addWidget(le_html_dir,j,1);
   j++;
   lineGrid->addWidget(pb_archive_dir,j,0);
   lineGrid->addWidget(le_archive_dir,j,1);
   j++;
   lineGrid->addWidget(pb_tmp_dir,j,0);
   lineGrid->addWidget(le_tmp_dir,j,1);
   j++;
   lineGrid->addWidget(pb_system_dir,j,0);
   lineGrid->addWidget(le_system_dir,j,1);
   j++;
   lineGrid->addWidget(pb_help_dir,j,0);
   lineGrid->addWidget(le_help_dir,j,1);

   topbox->addWidget(lbl_misc);

   QGridLayout *lineGrid2 = new QGridLayout(topbox,5,2);
   j=0;
   lineGrid2->addWidget(lbl_temperature_tol,j,0);
   lineGrid2->addWidget(le_temperature_tol,j,1);
   j++;
   lineGrid2->addWidget(lbl_beckman_bug,j,0);
   QBoxLayout *radiobutton = new QHBoxLayout(0);
   radiobutton->addWidget(bt_on);
   radiobutton->addWidget(bt_off);
   lineGrid2->addLayout(radiobutton,j,1);
   j++;
   lineGrid2->addWidget(lbl_color,j,0);
   lineGrid2->addWidget(pb_color,j,1);
   j++;
   lineGrid2->addWidget(lbl_font,j,0);
   lineGrid2->addWidget(pb_font,j,1);
   j++;
   lineGrid2->addWidget(lbl_database,j,0);
   lineGrid2->addWidget(pb_database,j,1);
   j++;
   lineGrid2->addWidget(lbl_numThreads,j,0);
   lineGrid2->addWidget(cnt_numThreads,j,1);

   QBoxLayout *pushbutton = new QHBoxLayout(topbox,2);
   pushbutton->addWidget(pb_help);
   pushbutton->addWidget(pb_save);
   pushbutton->addWidget(pb_cancel);
   topbox->activate();

}

void US_Config_GUI::closeEvent(QCloseEvent *e)
{
   e->accept();
   global_Xpos -= 30;
   global_Ypos -= 30;
}

void US_Config_GUI::cancel()
{
   close();
}

void US_Config_GUI::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/config.html");
}

void US_Config_GUI::save()
{
   US_Write_Config *WConfig;
   WConfig = new US_Write_Config();
   if (!WConfig->write_config(USglobal->config_list))
   {
      delete WConfig;
      return;
   }
   else
   {
      delete WConfig;
      close();
   }
}

void US_Config_GUI::update_screen()
{
   QString str;
   le_browser->setText(USglobal->config_list.browser);
   le_system_dir->setText(USglobal->config_list.system_dir);
   le_help_dir->setText(USglobal->config_list.help_dir);
   le_data_dir->setText(USglobal->config_list.data_dir);
   le_root_dir->setText(USglobal->config_list.root_dir);
   le_archive_dir->setText(USglobal->config_list.archive_dir);
   le_result_dir->setText(USglobal->config_list.result_dir);
   le_tmp_dir->setText(USglobal->config_list.tmp_dir);
   le_html_dir->setText(USglobal->config_list.html_dir);
   str.sprintf("%4.3f", USglobal->config_list.temperature_tol);
   le_temperature_tol->setText(str);
   if (USglobal->config_list.beckman_bug)
   {
      bt_on->setChecked(true);
      bt_off->setChecked(false);
   }
   else
   {
      bt_on->setChecked(false);
      bt_off->setChecked(true);
   }
}

void US_Config_GUI::update_on_button()
{
   bt_off->setChecked(false);
   USglobal->config_list.beckman_bug = 1;
}

void US_Config_GUI::update_color()
{
   US_Color *uscol;
   uscol = new US_Color();
   US_Config::connect(uscol, SIGNAL(marginChanged(int)), this, SLOT(update_margin(int)));
   uscol->show();
}

void US_Config_GUI::update_margin(int val)
{
   USglobal->config_list.margin = val;
}

void US_Config_GUI::update_font()
{
   US_Font *usfont;
   QString temp_font = USglobal->config_list.fontFamily;
   int temp_size = USglobal->config_list.fontSize;

   usfont = new US_Font(&USglobal->config_list.fontFamily, &USglobal->config_list.fontSize);
   if (usfont->exec() != QDialog::Accepted)
   {
      USglobal->config_list.fontSize = temp_size;
      USglobal->config_list.fontFamily = temp_font;
   }
   delete usfont;
}

void US_Config_GUI::update_database()
{
#ifndef NO_DB
   US_Database *usdb;
   usdb = new US_Database();
   usdb->setCaption("Database Configuration");
   usdb->resize(330,264);
   usdb->show();
#endif
}

void US_Config_GUI::update_off_button()
{
   bt_on->setChecked(false);
   USglobal->config_list.beckman_bug = 0;
}

void US_Config_GUI::open_browser_dir()
{
   QString browser = QFileDialog::getOpenFileName(
                                                  USglobal->config_list.browser, QString::null, 0);

   if ( browser != "" )
   {
      USglobal->config_list.browser = browser;
      le_browser->setText( browser );
   }
}

void US_Config_GUI::update_browser(const QString& newText)
{
   if ( newText != "" )
      USglobal->config_list.browser = newText;
}

void US_Config_GUI::open_system_dir()
{
   QString system = QFileDialog::getExistingDirectory(
                                                      USglobal->config_list.system_dir, 0, 0, QString::null, true);

   if ( system != "" )
   {
      le_system_dir->setText( system );
      USglobal->config_list.system_dir = system;
   }
}

void US_Config_GUI::update_system_dir( const QString& newText )
{
   if ( newText != "" )
      USglobal->config_list.system_dir = newText;
}

void US_Config_GUI::open_help_dir()
{
   QString help = QFileDialog::getExistingDirectory(
                                                    USglobal->config_list.help_dir, 0, 0, QString::null, true);

   if ( help != "" )
   {
      le_help_dir->setText( help );
      USglobal->config_list.help_dir = help;
   }
}

void US_Config_GUI::update_help_dir( const QString& newText )
{
   if ( newText != "" )
      USglobal->config_list.help_dir = newText;
}

void US_Config_GUI::update_temperature_tol( const QString& newText )
{
   if ( newText != "" )
      USglobal->config_list.temperature_tol = newText.toFloat();
}

void US_Config_GUI::open_result_dir()
{
   QString result = QFileDialog::getExistingDirectory(
                                                      USglobal->config_list.result_dir, 0, 0, QString::null, true);

   if ( result != "" )
   {
      le_result_dir->setText( result );
      USglobal->config_list.result_dir = result;
   }
}

void US_Config_GUI::update_result_dir(const QString &newText)
{
   USglobal->config_list.result_dir = newText;
}

void US_Config_GUI::open_html_dir()
{
   QString html = QFileDialog::getExistingDirectory(
                                                    USglobal->config_list.html_dir, 0, 0, QString::null, true);

   if ( html != "" )
   {
      le_html_dir->setText( html );
      USglobal->config_list.html_dir = html;
   }
}

void US_Config_GUI::update_html_dir( const QString& newText )
{
   if ( newText != "" )
      USglobal->config_list.html_dir = newText;
}

void US_Config_GUI::open_tmp_dir()
{
   QString tmp = QFileDialog::getExistingDirectory(
                                                   USglobal->config_list.tmp_dir, 0, 0, QString::null, true);

   if ( tmp != "" )
   {
      le_tmp_dir->setText( tmp );
      USglobal->config_list.tmp_dir = tmp;
   }
}

void US_Config_GUI::update_tmp_dir( const QString& newText )
{
   if ( newText != "" )
      USglobal->config_list.tmp_dir = newText;
}

void US_Config_GUI::open_data_dir()
{
   QString data = QFileDialog::getExistingDirectory(
                                                    USglobal->config_list.data_dir, 0, 0, QString::null, true);
   
   if ( data != "" )
   {
      le_data_dir->setText( data );
      USglobal->config_list.data_dir = data;
   }
}

void US_Config_GUI::update_data_dir( const QString& newText )
{
   if ( newText != "" )
      USglobal->config_list.data_dir = newText;
}

void US_Config_GUI::open_archive_dir()
{
   QString archive = QFileDialog::getExistingDirectory(
                                                       USglobal->config_list.archive_dir, 0, 0, QString::null, true);
   
   if ( archive != "" )
   {
      le_archive_dir->setText( archive );
      USglobal->config_list.archive_dir = archive ;
   }
}

void US_Config_GUI::update_archive_dir( const QString& newText )
{
   if ( newText != "" )
      USglobal->config_list.archive_dir = newText;
}

void US_Config_GUI::open_root_dir()
{
   QString root = QFileDialog::getExistingDirectory(
                                                    USglobal->config_list.root_dir, 0, 0, QString::null, true);

   if ( root != "" ) 
   {
      le_root_dir->setText( root );
      USglobal->config_list.root_dir = root;
   }
}

void US_Config_GUI::update_root_dir( const QString& newText )
{
   if ( newText != "" )
      USglobal->config_list.root_dir = newText;
}

void US_Config_GUI::update_numThreads( double val )
{
   USglobal->config_list.numThreads = (unsigned int) val;
}
