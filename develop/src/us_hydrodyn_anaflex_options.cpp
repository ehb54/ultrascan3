#include "../include/us_hydrodyn_anaflex_options.h"
#include "../include/us_hydrodyn.h"

US_Hydrodyn_Anaflex_Options::US_Hydrodyn_Anaflex_Options(Anaflex_Options *anaflex_options, bool *anaflex_widget, void *us_hydrodyn, QWidget *p, const char *name) : QFrame(p, name)
{
   this->anaflex_options = anaflex_options;
   this->anaflex_widget = anaflex_widget;
   this->us_hydrodyn = us_hydrodyn;
   *anaflex_widget = true;
   USglobal=new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("Anaflex Options"));
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_Anaflex_Options::~US_Hydrodyn_Anaflex_Options()
{
   *anaflex_widget = false;
}

void US_Hydrodyn_Anaflex_Options::setupGUI()
{
   int minHeight1 = 30;
   lbl_info = new QLabel(tr("Anaflex Options:"), this);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(AlignCenter|AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   pb_cancel = new QPushButton(tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));


   QVBoxLayout *vbl_top = new QVBoxLayout;

   vbl_top->addWidget(lbl_info);
   vbl_top->addSpacing(3);

   QHBoxLayout *hbl_buttons = new QHBoxLayout;

   hbl_buttons->addWidget(pb_help);
   hbl_buttons->addWidget(pb_cancel);
   
   vbl_top->addSpacing(3);
   vbl_top->addLayout(hbl_buttons);

   QHBoxLayout *background = new QHBoxLayout(this);
   background->addLayout(vbl_top);
}

void US_Hydrodyn_Anaflex_Options::cancel()
{
   close();
}

void US_Hydrodyn_Anaflex_Options::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_anaflex_options.html");
}

void US_Hydrodyn_Anaflex_Options::closeEvent(QCloseEvent *e)
{
   *anaflex_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}
