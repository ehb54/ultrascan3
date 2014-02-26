#include "../include/us_hydrodyn_file2.h"
//Added by qt3to4:
#include <Q3GridLayout>
#include <QLabel>
#include <QCloseEvent>

void US_Hydrodyn_File2::closeEvent( QCloseEvent *event )
{
   if ( ok_to_close )
   {
      event->accept();
   } else {
      event->ignore();
   }
}

US_Hydrodyn_File2::US_Hydrodyn_File2(QString *dir,
                                     QString *base,
                                     QString *ext,
                                     bool    &cancel,
                                     bool    &overwrite_all,
                                     int *result,
                                     QWidget *p,
                                     const char *name) : QDialog(p, name)
{
   this->dir = dir;
   this->base = base;
   this->ext = ext;
   this->cancel = &cancel;
   this->overwrite_all = &overwrite_all;
   this->result = result;
   *this->cancel = false;
   *this->overwrite_all = false;
   ok_to_close = false;
   *result = -1;
   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption("File will be overwritten");
   setupGUI();
   global_Xpos = 200;
   global_Ypos = 150;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_File2::~US_Hydrodyn_File2()
{
}

void US_Hydrodyn_File2::setupGUI()
{
   // int minHeight1 = 70;
   int minHeight2 = 30;

   lbl_info = new QLabel(" WARNING: The following file exists:", this);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight2);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   lbl_filename = new QLabel(*dir + *base + *ext, this);
   lbl_filename->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_filename->setMinimumHeight(minHeight2);
   lbl_filename->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   lbl_dir = new QLabel(" Path and project fixed base: ", this);
   lbl_dir->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_dir->setMinimumHeight(minHeight2);
   lbl_dir->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_dir->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   le_dir = new QLineEdit(this, "dir Line Edit");
   le_dir->setText(*dir);
   le_dir->setReadOnly(true);
   le_dir->setMinimumWidth(100);
   le_dir->setMinimumHeight(minHeight2);
   le_dir->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_dir->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_dir->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1));
   le_dir->setDisabled(true);

   lbl_update = new QLabel(" Update the filename: ", this);
   lbl_update->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
   lbl_update->setMinimumHeight(minHeight2);
   lbl_update->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_update->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   le_base = new QLineEdit(this, "base Line Edit");
   le_base->setText(*base);
   le_base->setReadOnly(false);
   le_base->setMinimumWidth(200);
   le_base->setMinimumHeight(minHeight2);
   le_base->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_base->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_base->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1));
   connect(le_base, SIGNAL(textChanged(const QString &)), SLOT(update_base(const QString &)));

   if ( ext->length() )
   {
      lbl_ext = new QLabel(" Fixed extension: ", this);
      lbl_ext->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
      lbl_ext->setMinimumHeight(minHeight2);
      lbl_ext->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      lbl_ext->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

      le_ext = new QLineEdit(this, "ext Line Edit");
      le_ext->setText(*ext);
      le_ext->setReadOnly(true);
      le_ext->setMinimumWidth(200);
      le_ext->setMinimumHeight(minHeight2);
      le_ext->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
      le_ext->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      le_ext->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1));
      le_ext->setDisabled(true);
   }

   pb_overwrite = new QPushButton("Overwrite existing file", this);
   pb_overwrite->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_overwrite->setMinimumHeight(minHeight2);
   pb_overwrite->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_overwrite, SIGNAL(clicked()), SLOT(overwrite()));

   pb_auto_inc = new QPushButton("Use next sequential integer", this);
   pb_auto_inc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_auto_inc->setMinimumHeight(minHeight2);
   pb_auto_inc->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_auto_inc, SIGNAL(clicked()), SLOT(auto_inc()));

   pb_try_again = new QPushButton("Try again", this);
   pb_try_again->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_try_again->setMinimumHeight(minHeight2);
   pb_try_again->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_try_again, SIGNAL(clicked()), SLOT(try_again()));

   pb_overwrite_all = new QPushButton("Overwrite all", this);
   pb_overwrite_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_overwrite_all->setMinimumHeight(minHeight2);
   pb_overwrite_all->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_overwrite_all, SIGNAL(clicked()), SLOT(do_overwrite_all()));

   pb_cancel = new QPushButton("Cancel saving", this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight2);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(do_cancel()));

   pb_help = new QPushButton("Help", this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight2);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   int j = 0;

   Q3GridLayout *background = new Q3GridLayout(this, 1, 4, 6, 2);

   background->addMultiCellWidget(lbl_info, j, j, 0, 1);
   background->addMultiCellWidget(lbl_filename, j, j, 2, 5);
   j++;
   background->addWidget(lbl_dir, j, 0);
   background->addMultiCellWidget(le_dir, j, j, 1, 6);
   j++;
   background->addWidget(lbl_update, j, 0);
   background->addMultiCellWidget(le_base, j, j, 1, 6);
   j++;
   if ( ext->length() )
   {
      background->addWidget(lbl_ext, j, 0);
      background->addMultiCellWidget(le_ext, j, j, 1, 6);
      j++;
   }
   background->addWidget(pb_help, j, 0);
   background->addWidget(pb_overwrite, j, 1);
   background->addWidget(pb_overwrite_all, j, 2);
   background->addWidget(pb_auto_inc, j, 3);
   background->addWidget(pb_try_again, j, 4);
   background->addWidget(pb_cancel, j, 5);
   j++;
   pb_try_again->setDefault(true);
}

void US_Hydrodyn_File2::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_file.html");
}

void US_Hydrodyn_File2::overwrite()
{
   *result = 0;
   ok_to_close = true;
   close();
}

void US_Hydrodyn_File2::do_overwrite_all()
{
   *result = 0;
   *overwrite_all = true;
   ok_to_close = true;
   close();
}

void US_Hydrodyn_File2::do_cancel()
{
   *result = 0;
   *cancel = true;
   ok_to_close = true;
   close();
}

void US_Hydrodyn_File2::auto_inc()
{
   *result = 1;
   ok_to_close = true;
   close();
}

void US_Hydrodyn_File2::try_again()
{
   if ( !QFile::exists(*dir + *base + *ext) )
   {
      *result = 0;
      ok_to_close = true;
      close();
   }
   lbl_info->setText(" WARNING: The file still exists:");
   lbl_filename->setText(*dir + *base + *ext);
}

void US_Hydrodyn_File2::update_base(const QString &str)
{
   *base = str;
}
