#include "../include/us_hydrodyn_misc.h"
#include "../include/us_hydrodyn.h"

US_Hydrodyn_Misc::US_Hydrodyn_Misc(struct misc_options *misc,
                                   bool *misc_widget, void *us_hydrodyn, QWidget *p, const char *name) : QFrame(p, name)
{
   this->misc = misc;
   this->misc_widget = misc_widget;
   *misc_widget = true;
   this->us_hydrodyn = us_hydrodyn;
   USglobal=new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("Misceallaneous SOMO Options"));
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_Misc::~US_Hydrodyn_Misc()
{
   *misc_widget = false;
}

void US_Hydrodyn_Misc::setupGUI()
{
   int minHeight1 = 30;
   QString str;
   lbl_info = new QLabel(tr("Miscellaneous SOMO Options:"), this);
   Q_CHECK_PTR(lbl_info);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(AlignCenter|AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_hydrovol = new QLabel(tr(" Hydration Water Vol. (A^3): "), this);
   Q_CHECK_PTR(lbl_hydrovol);
   lbl_hydrovol->setAlignment(AlignLeft|AlignVCenter);
   lbl_hydrovol->setMinimumWidth(200);
   lbl_hydrovol->setMinimumHeight(minHeight1);
   lbl_hydrovol->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_hydrovol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_hydrovol= new QwtCounter(this);
   Q_CHECK_PTR(cnt_hydrovol);
   cnt_hydrovol->setRange(0, 100, 0.001);
   cnt_hydrovol->setValue((*misc).hydrovol);
   cnt_hydrovol->setMinimumHeight(minHeight1);
   cnt_hydrovol->setMinimumWidth(200);
   cnt_hydrovol->setEnabled(true);
   cnt_hydrovol->setNumButtons(3);
   cnt_hydrovol->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_hydrovol->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_hydrovol, SIGNAL(valueChanged(double)), SLOT(update_hydrovol(double)));

   lbl_vbar = new QLabel(tr(" Enter a vbar value: "), this);
   Q_CHECK_PTR(lbl_vbar);
   lbl_vbar->setAlignment(AlignLeft|AlignVCenter);
   lbl_vbar->setMinimumHeight(minHeight1);
   lbl_vbar->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cb_vbar = new QCheckBox(this);
   cb_vbar->setText(tr(" Calculate vbar "));
   cb_vbar->setChecked((*misc).compute_vbar);
   cb_vbar->setMinimumHeight(minHeight1);
   cb_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_vbar->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_vbar, SIGNAL(clicked()), SLOT(set_vbar()));

   pb_vbar = new QPushButton(tr("Select vbar"), this);
   Q_CHECK_PTR(pb_vbar);
   pb_vbar->setEnabled(!(*misc).compute_vbar);
   pb_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_vbar->setMinimumHeight(minHeight1);
   pb_vbar->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_vbar, SIGNAL(clicked()), SLOT(select_vbar()));

   le_vbar = new QLineEdit(this, "vbar Line Edit");
   le_vbar->setMinimumHeight(minHeight1);
   le_vbar->setEnabled(!(*misc).compute_vbar);
   le_vbar->setText(str.sprintf("%5.3f", (*misc).vbar));
   le_vbar->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_vbar, SIGNAL(textChanged(const QString &)), SLOT(update_vbar(const QString &)));

   lbl_avg_banner = new QLabel(tr("Average Parameters for Automatic Bead Builder:"), this);
   Q_CHECK_PTR(lbl_avg_banner);
   lbl_avg_banner->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_avg_banner->setAlignment(AlignCenter|AlignVCenter);
   lbl_avg_banner->setMinimumHeight(minHeight1);
   lbl_avg_banner->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_avg_banner->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_avg_radius = new QLabel(tr(" Average atomic radius (A): "), this);
   Q_CHECK_PTR(lbl_avg_radius);
   lbl_avg_radius->setAlignment(AlignLeft|AlignVCenter);
   lbl_avg_radius->setMinimumWidth(200);
   lbl_avg_radius->setMinimumHeight(minHeight1);
   lbl_avg_radius->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_avg_radius->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_avg_radius = new QwtCounter(this);
   Q_CHECK_PTR(cnt_avg_radius);
   cnt_avg_radius->setRange(0.5, 5.0, 0.001);
   cnt_avg_radius->setValue((*misc).avg_radius);
   cnt_avg_radius->setMinimumHeight(minHeight1);
   cnt_avg_radius->setMinimumWidth(200);
   cnt_avg_radius->setEnabled(true);
   cnt_avg_radius->setNumButtons(3);
   cnt_avg_radius->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_avg_radius->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_avg_radius, SIGNAL(valueChanged(double)), SLOT(update_avg_radius(double)));

   lbl_avg_mass = new QLabel(tr(" Average atomic mass (Da): "), this);
   Q_CHECK_PTR(lbl_avg_mass);
   lbl_avg_mass->setAlignment(AlignLeft|AlignVCenter);
   lbl_avg_mass->setMinimumWidth(200);
   lbl_avg_mass->setMinimumHeight(minHeight1);
   lbl_avg_mass->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_avg_mass->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_avg_mass = new QwtCounter(this);
   Q_CHECK_PTR(cnt_avg_mass);
   cnt_avg_mass->setRange(0.0, 200.0, 0.01);
   cnt_avg_mass->setValue((*misc).avg_mass);
   cnt_avg_mass->setMinimumHeight(minHeight1);
   cnt_avg_mass->setMinimumWidth(200);
   cnt_avg_mass->setEnabled(true);
   cnt_avg_mass->setNumButtons(3);
   cnt_avg_mass->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_avg_mass->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_avg_mass, SIGNAL(valueChanged(double)), SLOT(update_avg_mass(double)));

   lbl_avg_hydration = new QLabel(tr(" Average atomic hydration: "), this);
   Q_CHECK_PTR(lbl_avg_hydration);
   lbl_avg_hydration->setAlignment(AlignLeft|AlignVCenter);
   lbl_avg_hydration->setMinimumWidth(200);
   lbl_avg_hydration->setMinimumHeight(minHeight1);
   lbl_avg_hydration->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_avg_hydration->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_avg_hydration = new QwtCounter(this);
   Q_CHECK_PTR(cnt_avg_hydration);
   cnt_avg_hydration->setRange(0.0, 10.0, 0.001);
   cnt_avg_hydration->setValue((*misc).avg_hydration);
   cnt_avg_hydration->setMinimumHeight(minHeight1);
   cnt_avg_hydration->setMinimumWidth(200);
   cnt_avg_hydration->setEnabled(true);
   cnt_avg_hydration->setNumButtons(3);
   cnt_avg_hydration->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_avg_hydration->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_avg_hydration, SIGNAL(valueChanged(double)), SLOT(update_avg_hydration(double)));

   lbl_avg_volume = new QLabel(tr(" Average bead/atom volume (A^3): "), this);
   Q_CHECK_PTR(lbl_avg_volume);
   lbl_avg_volume->setAlignment(AlignLeft|AlignVCenter);
   lbl_avg_volume->setMinimumWidth(200);
   lbl_avg_volume->setMinimumHeight(minHeight1);
   lbl_avg_volume->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_avg_volume->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_avg_volume = new QwtCounter(this);
   Q_CHECK_PTR(cnt_avg_volume);
   cnt_avg_volume->setRange(1.0, 50.0, 0.01);
   cnt_avg_volume->setValue((*misc).avg_volume);
   cnt_avg_volume->setMinimumHeight(minHeight1);
   cnt_avg_volume->setMinimumWidth(200);
   cnt_avg_volume->setEnabled(true);
   cnt_avg_volume->setNumButtons(3);
   cnt_avg_volume->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_avg_volume->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_avg_volume, SIGNAL(valueChanged(double)), SLOT(update_avg_volume(double)));

   lbl_avg_vbar = new QLabel(tr(" Average Residue vbar: "), this);
   Q_CHECK_PTR(lbl_avg_vbar);
   lbl_avg_vbar->setAlignment(AlignLeft|AlignVCenter);
   lbl_avg_vbar->setMinimumWidth(200);
   lbl_avg_vbar->setMinimumHeight(minHeight1);
   lbl_avg_vbar->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_avg_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_avg_vbar = new QwtCounter(this);
   Q_CHECK_PTR(cnt_avg_vbar);
   cnt_avg_vbar->setRange(0.01, 2.0, 0.001);
   cnt_avg_vbar->setValue((*misc).avg_vbar);
   cnt_avg_vbar->setMinimumHeight(minHeight1);
   cnt_avg_vbar->setMinimumWidth(200);
   cnt_avg_vbar->setEnabled(true);
   cnt_avg_vbar->setNumButtons(3);
   cnt_avg_vbar->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_avg_vbar->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_avg_vbar, SIGNAL(valueChanged(double)), SLOT(update_avg_vbar(double)));

   pb_cancel = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   int rows=6, columns = 2, spacing = 2, j=0, margin=4;
   QGridLayout *background=new QGridLayout(this, rows, columns, margin, spacing);

   background->addMultiCellWidget(lbl_info, j, j, 0, 1);
   j++;
   background->addWidget(cb_vbar, j, 0);
   background->addWidget(pb_vbar, j, 1);
   j++;
   background->addWidget(lbl_vbar, j, 0);
   background->addWidget(le_vbar, j, 1);
   j++;
   background->addWidget(lbl_hydrovol, j, 0);
   background->addWidget(cnt_hydrovol, j, 1);
   j++;
   background->addMultiCellWidget(lbl_avg_banner, j, j, 0, 1);
   j++;
   background->addWidget(lbl_avg_radius, j, 0);
   background->addWidget(cnt_avg_radius, j, 1);
   j++;
   background->addWidget(lbl_avg_mass, j, 0);
   background->addWidget(cnt_avg_mass, j, 1);
   j++;
   background->addWidget(lbl_avg_hydration, j, 0);
   background->addWidget(cnt_avg_hydration, j, 1);
   j++;
   background->addWidget(lbl_avg_volume, j, 0);
   background->addWidget(cnt_avg_volume, j, 1);
   j++;
   background->addWidget(lbl_avg_vbar, j, 0);
   background->addWidget(cnt_avg_vbar, j, 1);
   j++;
   background->addWidget(pb_help, j, 0);
   background->addWidget(pb_cancel, j, 1);
}

void US_Hydrodyn_Misc::update_vbar(const QString &str)
{
   (*misc).vbar = str.toDouble();
}

void US_Hydrodyn_Misc::update_vbar_signal(float val1, float val2)
{
   QString str;
   float vbar20;
   vbar20 = val1;
   (*misc).vbar = val2;
   le_vbar->setText(str.sprintf("%5.3f", (*misc).vbar));
   emit vbar_changed();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::select_vbar()
{
   float val;
   vbar_dlg = new US_Vbar_DB(20.0, &val, &val, true, false, 0);
   vbar_dlg->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   vbar_dlg->setCaption(tr("V-bar Calculation"));
   connect(vbar_dlg, SIGNAL(valueChanged(float, float)), SLOT(update_vbar_signal(float, float)));
   vbar_dlg->exec();
   emit vbar_changed();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::set_vbar()
{
   (*misc).compute_vbar = cb_vbar->isChecked();
   le_vbar->setEnabled(!(*misc).compute_vbar);
   pb_vbar->setEnabled(!(*misc).compute_vbar);
   emit vbar_changed();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::cancel()
{
   close();
}

void US_Hydrodyn_Misc::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_misc.html");
}

void US_Hydrodyn_Misc::update_hydrovol(double val)
{
   (*misc).hydrovol = val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::update_avg_radius(double val)
{
   (*misc).avg_radius = val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::update_avg_mass(double val)
{
   (*misc).avg_mass = val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::update_avg_hydration(double val)
{
   (*misc).avg_hydration = val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::update_avg_volume(double val)
{
   (*misc).avg_volume = val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::update_avg_vbar(double val)
{
   (*misc).avg_vbar = val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::closeEvent(QCloseEvent *e)
{
   *misc_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

