#include "../include/us_hydrodyn_file.h"

US_Hydrodyn_File::US_Hydrodyn_File(QString *dir,
                                   QString *base,
                                   QString *ext,QWidget *p, 
                                   const char *name) : QDialog(p, name)
{
   this->dir = dir;
   this->base = base;
   this->ext = ext;
   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("File will be overwritten"));
   setupGUI();
   global_Xpos = 200;
   global_Ypos = 150;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_File::~US_Hydrodyn_File()
{
}

void US_Hydrodyn_File::setupGUI()
{
   int minHeight1 = 70;
   int minHeight2 = 30;

   lbl_info = new QLabel(tr(" Warning: The following file exists:"), this);
   lbl_info->setAlignment(AlignCenter|AlignVCenter);
   lbl_info->setMinimumHeight(minHeight2);
   lbl_info->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   lbl_info2 = new QLabel(*dir + *base + *ext, this);
   lbl_info2->setAlignment(AlignCenter|AlignVCenter);
   lbl_info2->setMinimumHeight(minHeight2);
   lbl_info2->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_info2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));


   le_dir = new QLineEdit(this, "dir Line Edit");
   le_dir->setText(*dir);
   le_dir->setReadOnly(true);
   le_dir->setMinimumWidth(200);
   le_dir->setAlignment(AlignRight|AlignVCenter);
   le_dir->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_dir->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1));

   le_base = new QLineEdit(this, "base Line Edit");
   le_base->setText(*base);
   le_base->setReadOnly(false);
   le_base->setMinimumWidth(200);
   le_base->setAlignment(AlignCenter|AlignVCenter);
   le_base->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_base->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   le_ext = new QLineEdit(this, "ext Line Edit");
   le_ext->setText(*ext);
   le_ext->setReadOnly(true);
   le_ext->setMinimumWidth(200);
   le_ext->setAlignment(AlignLeft|AlignVCenter);
   le_ext->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_ext->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1));

   pb_overwrite = new QPushButton(tr("Write over the existing file"), this);
   pb_overwrite->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_overwrite->setMinimumHeight(minHeight2);
   pb_overwrite->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_overwrite, SIGNAL(clicked()), SLOT(overwrite()));

   pb_auto_inc = new QPushButton(tr("Use next sequential integer"), this);
   pb_auto_inc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_auto_inc->setMinimumHeight(minHeight2);
   pb_auto_inc->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_auto_inc, SIGNAL(clicked()), SLOT(auto_inc()));

   pb_try_again = new QPushButton(tr("Try again"), this);
   pb_try_again->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_try_again->setMinimumHeight(minHeight2);
   pb_try_again->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_try_again, SIGNAL(clicked()), SLOT(try_again()));

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight2);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   int j = 0;

   QGridLayout *background = new QGridLayout(this, 1, 4, 4, 2);

   background->addMultiCellWidget(lbl_info, j, j, 0, 3);
   j++;
   background->addMultiCellWidget(lbl_info2, j, j, 0, 3);
   j++;
   QHBoxLayout *hbl = new QHBoxLayout;
   hbl->addWidget(le_dir);
   hbl->addWidget(le_base);
   hbl->addWidget(le_ext);
   background->addMultiCellLayout(hbl, j, j, 0, 3);
   j++;
   background->addWidget(pb_help, j, 0);
   background->addWidget(pb_overwrite, j, 1);
   background->addWidget(pb_auto_inc, j, 2);
   background->addWidget(pb_try_again, j, 3);
   j++;
}

void US_Hydrodyn_File::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_file.html");
}

void US_Hydrodyn_File::overwrite()
{
}

void US_Hydrodyn_File::auto_inc()
{
}

void US_Hydrodyn_File::try_again()
{
}
