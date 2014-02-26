#include "../include/us_hydrodyn_saxs_mw.h"
#include "../include/us_hydrodyn.h"

US_Hydrodyn_Saxs_Mw::US_Hydrodyn_Saxs_Mw(
                                         QString msg,
                                         float *mw,
                                         float *last_mw,
                                         bool *remember,
                                         bool *use_partial,
                                         QString *partial,
                                         QWidget *p,
                                         const char *name
                                         ) : QDialog(p, name)
{
   this->msg = msg;
   this->mw = mw;
   this->last_mw = last_mw;
   this->remember = remember;
   this->use_partial = use_partial;
   this->partial = partial;

   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption("Set MW");
   setupGUI();
   global_Xpos = 200;
   global_Ypos = 150;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_Saxs_Mw::~US_Hydrodyn_Saxs_Mw()
{
}

void US_Hydrodyn_Saxs_Mw::setupGUI()
{
   int minHeight1 = 30;
   int minHeight2 = 30;

   lbl_info = new QLabel(msg, this);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight2);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   lbl_mw = new QLabel(tr(" Molecular Weight (Daltons):"), this);
   lbl_mw->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_mw->setMinimumHeight(minHeight2);
   lbl_mw->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   le_mw = new QLineEdit(this, "mw Line Edit");
   le_mw->setText(QString("").sprintf("%5.3f", *mw));
   le_mw->setReadOnly(false);
   le_mw->setMinimumWidth(100);
   le_mw->setMinimumHeight(minHeight2);
   le_mw->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_mw->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1));
   connect(le_mw, SIGNAL(textChanged(const QString &)), SLOT(update_mw(const QString &)));

   pb_set_to_last_used_mw = new QPushButton(tr("Set to last used weight:"), this);
   pb_set_to_last_used_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_set_to_last_used_mw->setMinimumHeight(minHeight1);
   pb_set_to_last_used_mw->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_set_to_last_used_mw->setEnabled(*last_mw > 0.0);
   connect(pb_set_to_last_used_mw, SIGNAL(clicked()), SLOT(set_to_last_used_mw()));

   lbl_last_used_mw = new QLabel(QString("").sprintf("%5.3f", *last_mw), this);
   lbl_last_used_mw->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_last_used_mw->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_remember = new QCheckBox(this);
   cb_remember->setText(tr(" Remember these values ?"));
   cb_remember->setChecked(*remember);
   cb_remember->setMinimumHeight(minHeight1);
   cb_remember->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_remember->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_remember, SIGNAL(clicked()), SLOT(set_remember()));

   cb_use_partial = new QCheckBox(this);
   cb_use_partial->setText(tr(" Remember for all files that contain: "));
   cb_use_partial->setChecked(*use_partial);
   cb_use_partial->setMinimumHeight(minHeight1);
   cb_use_partial->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_use_partial->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_use_partial, SIGNAL(clicked()), SLOT(set_use_partial()));

   le_partial = new QLineEdit(this, "partial Line Edit");
   le_partial->setText(*partial);
   le_partial->setReadOnly(false);
   le_partial->setMinimumWidth(250);
   le_partial->setMinimumHeight(minHeight2);
   le_partial->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_partial->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_partial->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1));
   connect(le_partial, SIGNAL(textChanged(const QString &)), SLOT(update_partial(const QString &)));

   pb_cancel = new QPushButton(tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton("Help", this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight2);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   int j = 0;

   QGridLayout *background = new QGridLayout(this, 1, 2, 4, 2);

   background->addMultiCellWidget(lbl_info, j, j, 0, 1);
   j++;
   background->addWidget(lbl_mw, j, 0);
   background->addWidget(le_mw, j, 1);
   j++;
   background->addWidget(pb_set_to_last_used_mw, j, 0);
   background->addWidget(lbl_last_used_mw, j, 1);
   j++;
   background->addMultiCellWidget(cb_remember, j, j, 0, 1);
   j++;
   background->addWidget(cb_use_partial, j, 0);
   background->addWidget(le_partial, j, 1);
   j++;
   background->addWidget(pb_help, j, 0);
   background->addWidget(pb_cancel, j, 1);

   US_Hydrodyn::fixWinButtons( this );
}

void US_Hydrodyn_Saxs_Mw::set_to_last_used_mw()
{
   le_mw->setText(QString("").sprintf("%5.3f", *last_mw));
   *mw = *last_mw;
}

void US_Hydrodyn_Saxs_Mw::cancel()
{
   close();
}

void US_Hydrodyn_Saxs_Mw::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_saxs_mw.html");
}

void US_Hydrodyn_Saxs_Mw::update_mw(const QString &str)
{
   *mw = str.toDouble();
}

void US_Hydrodyn_Saxs_Mw::update_partial(const QString &str)
{
   *partial = str;
}

void US_Hydrodyn_Saxs_Mw::set_remember()
{
   *remember = cb_remember->isChecked();
}

void US_Hydrodyn_Saxs_Mw::set_use_partial()
{
   *use_partial = cb_use_partial->isChecked();
}
