#include "../include/us_hydrodyn_bd_load_results_opts.h"

US_Hydrodyn_BD_Load_Results_Opts::US_Hydrodyn_BD_Load_Results_Opts(
                                                 QString msg,
                                                 float *psv,
                                                 float *mw,
                                                 bool *write_bead_model,
                                                 QWidget *p,
                                                 const char *name
                                                 ) : QDialog(p, name)
{
   this->msg = msg;
   this->psv = psv;
   this->mw = mw;
   this->write_bead_model = write_bead_model;
   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption("Set PSV and MW for DAMMIN/DAMMIF files");
   setupGUI();
   global_Xpos = 200;
   global_Ypos = 150;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_BD_Load_Results_Opts::~US_Hydrodyn_BD_Load_Results_Opts()
{
}

void US_Hydrodyn_BD_Load_Results_Opts::setupGUI()
{
   int minHeight1 = 30;
   int minHeight2 = 30;

   lbl_info = new QLabel(msg, this);
   lbl_info->setAlignment(AlignCenter|AlignVCenter);
   lbl_info->setMinimumHeight(minHeight2);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   lbl_psv = new QLabel(tr(" Enter a vbar value:"), this);
   lbl_psv->setAlignment(AlignCenter|AlignVCenter);
   lbl_psv->setMinimumHeight(minHeight2);
   lbl_psv->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_psv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   le_psv = new QLineEdit(this, "psv Line Edit");
   le_psv->setText(QString("").sprintf("%5.3f", *psv));
   le_psv->setReadOnly(false);
   le_psv->setMinimumWidth(100);
   le_psv->setMinimumHeight(minHeight2);
   le_psv->setAlignment(AlignCenter|AlignVCenter);
   le_psv->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_psv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1));
   connect(le_psv, SIGNAL(textChanged(const QString &)), SLOT(update_psv(const QString &)));

   lbl_mw = new QLabel(tr(" Molecular Weight (Daltons):"), this);
   lbl_mw->setAlignment(AlignCenter|AlignVCenter);
   lbl_mw->setMinimumHeight(minHeight2);
   lbl_mw->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   le_mw = new QLineEdit(this, "mw Line Edit");
   le_mw->setText(QString("").sprintf("%5.3f", *mw));
   le_mw->setReadOnly(false);
   le_mw->setMinimumWidth(100);
   le_mw->setMinimumHeight(minHeight2);
   le_mw->setAlignment(AlignCenter|AlignVCenter);
   le_mw->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1));
   connect(le_mw, SIGNAL(textChanged(const QString &)), SLOT(update_mw(const QString &)));

   cb_write_bead_model = new QCheckBox(this);
   cb_write_bead_model->setText(tr(" Create a bead model file ?"));
   cb_write_bead_model->setChecked(*write_bead_model);
   cb_write_bead_model->setMinimumHeight(minHeight1);
   cb_write_bead_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_write_bead_model->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_write_bead_model, SIGNAL(clicked()), SLOT(set_write_bead_model()));

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
   background->addWidget(lbl_psv, j, 0);
   background->addWidget(le_psv, j, 1);
   j++;
   background->addWidget(lbl_mw, j, 0);
   background->addWidget(le_mw, j, 1);
   j++;
   background->addMultiCellWidget(cb_write_bead_model, j, j, 0, 1);
   j++;
   background->addWidget(pb_help, j, 0);
   background->addWidget(pb_cancel, j, 1);
}

void US_Hydrodyn_BD_Load_Results_Opts::cancel()
{
   close();
}

void US_Hydrodyn_BD_Load_Results_Opts::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_dammin_opts.html");
}

void US_Hydrodyn_BD_Load_Results_Opts::update_psv(const QString &str)
{
   *psv = str.toDouble();
}

void US_Hydrodyn_BD_Load_Results_Opts::update_mw(const QString &str)
{
   *mw = str.toDouble();
}

void US_Hydrodyn_BD_Load_Results_Opts::set_write_bead_model()
{
   *write_bead_model = cb_write_bead_model->isChecked();
}
