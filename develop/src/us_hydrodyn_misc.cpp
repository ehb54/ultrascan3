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
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_hydrovol = new QLabel(tr(" Hydration Water Vol. (A^3): "), this);
   Q_CHECK_PTR(lbl_hydrovol);
   lbl_hydrovol->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_hydrovol->setMinimumWidth(200);
   lbl_hydrovol->setMinimumHeight(minHeight1);
   lbl_hydrovol->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_hydrovol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_hydrovol= new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_hydrovol );
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


   lbl_vbar = new QLabel(tr(" Enter a vbar value: "), this);
   Q_CHECK_PTR(lbl_vbar);
   lbl_vbar->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_vbar->setMinimumHeight(minHeight1);
   lbl_vbar->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_vbar = new QLineEdit(this, "vbar Line Edit");
   le_vbar->setMinimumHeight(minHeight1);
   le_vbar->setEnabled(!(*misc).compute_vbar);
   le_vbar->setText(str.sprintf("%5.3f", (*misc).vbar));
   le_vbar->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_vbar, SIGNAL(textChanged(const QString &)), SLOT(update_vbar(const QString &)));

   lbl_vbar_temperature = new QLabel(tr(" Vbar measured/computed at T=(ºC): "), this);
   Q_CHECK_PTR(lbl_vbar_temperature);
   lbl_vbar_temperature->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_vbar_temperature->setMinimumHeight(minHeight1);
   lbl_vbar_temperature->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_vbar_temperature->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_vbar_temperature = new QLineEdit(this, "vbar_temperature Line Edit");
   le_vbar_temperature->setMinimumHeight(minHeight1);
   le_vbar_temperature->setEnabled(!(*misc).compute_vbar);
   le_vbar_temperature->setText(str.sprintf("%5.2f", (*misc).vbar_temperature));
   le_vbar_temperature->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_vbar_temperature->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_vbar_temperature, SIGNAL(textChanged(const QString &)), SLOT(update_vbar_temperature(const QString &)));

   cb_pb_rule_on = new QCheckBox(this);
   cb_pb_rule_on->setText(tr(" Enable Peptide Bond Rule "));
   cb_pb_rule_on->setChecked((*misc).pb_rule_on);
   cb_pb_rule_on->setMinimumHeight(minHeight1);
   cb_pb_rule_on->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pb_rule_on->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_pb_rule_on, SIGNAL(clicked()), SLOT(set_pb_rule_on()));

   lbl_avg_banner = new QLabel(tr("Average Parameters for Automatic Bead Builder:"), this);
   Q_CHECK_PTR(lbl_avg_banner);
   lbl_avg_banner->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_avg_banner->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_avg_banner->setMinimumHeight(minHeight1);
   lbl_avg_banner->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_avg_banner->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_avg_radius = new QLabel(tr(" Average atomic radius (A): "), this);
   Q_CHECK_PTR(lbl_avg_radius);
   lbl_avg_radius->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_avg_radius->setMinimumWidth(200);
   lbl_avg_radius->setMinimumHeight(minHeight1);
   lbl_avg_radius->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_avg_radius->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_avg_radius = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_avg_radius );
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
   lbl_avg_mass->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_avg_mass->setMinimumWidth(200);
   lbl_avg_mass->setMinimumHeight(minHeight1);
   lbl_avg_mass->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_avg_mass->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_avg_mass = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_avg_mass );
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
   lbl_avg_hydration->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_avg_hydration->setMinimumWidth(200);
   lbl_avg_hydration->setMinimumHeight(minHeight1);
   lbl_avg_hydration->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_avg_hydration->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_avg_hydration = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_avg_hydration );
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
   lbl_avg_volume->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_avg_volume->setMinimumWidth(200);
   lbl_avg_volume->setMinimumHeight(minHeight1);
   lbl_avg_volume->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_avg_volume->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_avg_volume = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_avg_volume );
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
   lbl_avg_vbar->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_avg_vbar->setMinimumWidth(200);
   lbl_avg_vbar->setMinimumHeight(minHeight1);
   lbl_avg_vbar->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_avg_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_avg_vbar = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_avg_vbar );
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

   lbl_bead_model_controls = new QLabel(tr("Bead model controls:"), this);
   lbl_bead_model_controls->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_bead_model_controls->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_bead_model_controls->setMinimumHeight(minHeight1);
   lbl_bead_model_controls->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_bead_model_controls->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_target_e_density = new QLabel(tr(" Target electron density (A^-3): "), this);
   lbl_target_e_density->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_target_e_density->setMinimumWidth(200);
   lbl_target_e_density->setMinimumHeight(minHeight1);
   lbl_target_e_density->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_target_e_density->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_target_e_density = new QLineEdit(this, "target_e_density Line Edit");
   le_target_e_density->setMinimumHeight(minHeight1);
   le_target_e_density->setEnabled(true);
   le_target_e_density->setText(QString("%1").arg((*misc).target_e_density));
   le_target_e_density->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_target_e_density->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_target_e_density, SIGNAL(textChanged(const QString &)), SLOT(update_target_e_density(const QString &)));

   lbl_target_volume = new QLabel(tr(" Target volume (A^3): "), this);
   lbl_target_volume->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_target_volume->setMinimumWidth(200);
   lbl_target_volume->setMinimumHeight(minHeight1);
   lbl_target_volume->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_target_volume->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_target_volume = new QLineEdit(this, "target_volume Line Edit");
   le_target_volume->setMinimumHeight(minHeight1);
   le_target_volume->setEnabled(true);
   le_target_volume->setText(QString("%1").arg((*misc).target_volume));
   le_target_volume->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_target_volume->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_target_volume, SIGNAL(textChanged(const QString &)), SLOT(update_target_volume(const QString &)));

   cb_set_target_on_load_pdb = new QCheckBox(this);
   cb_set_target_on_load_pdb->setText(tr(" Set targets on load PDB "));
   cb_set_target_on_load_pdb->setChecked((*misc).set_target_on_load_pdb);
   cb_set_target_on_load_pdb->setMinimumHeight(minHeight1);
   cb_set_target_on_load_pdb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_set_target_on_load_pdb->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_set_target_on_load_pdb, SIGNAL(clicked()), SLOT(set_set_target_on_load_pdb()));

   cb_equalize_radii = new QCheckBox(this);
   cb_equalize_radii->setText(tr(" Equalize radii (constant volume)"));
   cb_equalize_radii->setChecked((*misc).equalize_radii);
   cb_equalize_radii->setMinimumHeight(minHeight1);
   cb_equalize_radii->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_equalize_radii->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_equalize_radii, SIGNAL(clicked()), SLOT(set_equalize_radii()));

   lbl_hydro_method = new QLabel(tr("Hydrodynamic Method:"), this);
   lbl_hydro_method->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_hydro_method->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_hydro_method->setMinimumHeight(minHeight1);
   lbl_hydro_method->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_hydro_method->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   cb_hydro_supc = new QCheckBox(this);
   cb_hydro_supc->setText(tr(" Standard matrix inversion"));
   cb_hydro_supc->setChecked((*misc).hydro_supc);
   cb_hydro_supc->setMinimumHeight(minHeight1);
   cb_hydro_supc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_hydro_supc->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_hydro_supc, SIGNAL(clicked()), SLOT(set_hydro_supc()));

   cb_hydro_zeno = new QCheckBox(this);
   cb_hydro_zeno->setText(tr(" Zeno method"));
   cb_hydro_zeno->setChecked((*misc).hydro_zeno);
   cb_hydro_zeno->setMinimumHeight(minHeight1);
   cb_hydro_zeno->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_hydro_zeno->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_hydro_zeno, SIGNAL(clicked()), SLOT(set_hydro_zeno()));

   pb_cancel = new QPushButton(tr("Close"), this);
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

   int rows=8, columns = 2, spacing = 2, j=0, margin=4;
   QGridLayout *background=new QGridLayout(this, rows, columns, margin, spacing);

   background->addMultiCellWidget(lbl_info, j, j, 0, 1);
   j++;
   background->addWidget(cb_vbar, j, 0);
   background->addWidget(pb_vbar, j, 1);
   j++;
   background->addWidget(lbl_vbar, j, 0);
   background->addWidget(le_vbar, j, 1);
   j++;
   background->addWidget(lbl_vbar_temperature, j, 0);
   background->addWidget(le_vbar_temperature, j, 1);
   j++;
   background->addWidget(lbl_hydrovol, j, 0);
   background->addWidget(cnt_hydrovol, j, 1);
   j++;
   background->addMultiCellWidget(cb_pb_rule_on, j, j, 0, 1);
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

   background->addMultiCellWidget(lbl_bead_model_controls, j, j, 0, 1);
   j++;
   background->addWidget(lbl_target_e_density, j, 0);
   background->addWidget(le_target_e_density, j, 1);
   j++;
   background->addWidget(lbl_target_volume, j, 0);
   background->addWidget(le_target_volume, j, 1);
   j++;
   background->addWidget(cb_set_target_on_load_pdb, j, 0);
   background->addWidget(cb_equalize_radii, j, 1);
   j++;

   background->addMultiCellWidget(lbl_hydro_method, j, j, 0, 1);
   j++;
   background->addWidget(cb_hydro_supc, j, 0);
   background->addWidget(cb_hydro_zeno, j, 1);
   j++;

   background->addWidget(pb_help, j, 0);
   background->addWidget(pb_cancel, j, 1);
}

void US_Hydrodyn_Misc::update_vbar(const QString &str)
{
   (*misc).vbar = str.toDouble();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::update_vbar_temperature(const QString &str)
{
   (*misc).vbar_temperature = str.toDouble();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
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
#ifndef NO_DB
   float val;
   vbar_dlg = new US_Vbar_DB(20.0, &val, &val, true, false, 0);
   vbar_dlg->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   vbar_dlg->setCaption(tr("V-bar Calculation"));
   connect(vbar_dlg, SIGNAL(valueChanged(float, float)), SLOT(update_vbar_signal(float, float)));
   vbar_dlg->exec();
   emit vbar_changed();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
#endif
}

void US_Hydrodyn_Misc::set_vbar()
{
   (*misc).compute_vbar = cb_vbar->isChecked();
   le_vbar->setEnabled(!(*misc).compute_vbar);
   pb_vbar->setEnabled(!(*misc).compute_vbar);
   le_vbar_temperature->setEnabled(!(*misc).compute_vbar);
   emit vbar_changed();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::set_pb_rule_on()
{
   if( cb_pb_rule_on->isChecked() )
   {
      ((US_Hydrodyn *)us_hydrodyn)->pdb_parse.missing_atoms = 0;
      ((US_Hydrodyn *)us_hydrodyn)->pdb_parse.missing_residues = 0;
   }

   (*misc).pb_rule_on = cb_pb_rule_on->isChecked();

   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   ((US_Hydrodyn *)us_hydrodyn)->set_disabled();
}

void US_Hydrodyn_Misc::update_target_e_density(const QString &str)
{
   (*misc).target_e_density = str.toDouble();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::update_target_volume(const QString &str)
{
   (*misc).target_volume = str.toDouble();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->pb_bead_saxs->isEnabled() &&
        (*misc).target_volume != 0e0 )
   {
      ((US_Hydrodyn *)us_hydrodyn)->pb_bead_saxs->setEnabled(true);
   } else {
      ((US_Hydrodyn *)us_hydrodyn)->pb_bead_saxs->setEnabled(false);
   }
}

void US_Hydrodyn_Misc::set_set_target_on_load_pdb()
{
   (*misc).set_target_on_load_pdb = cb_set_target_on_load_pdb->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::set_equalize_radii()
{
   (*misc).equalize_radii = cb_equalize_radii->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::set_hydro_supc()
{
   (*misc).hydro_supc = cb_hydro_supc->isChecked();
   (*misc).hydro_zeno = !cb_hydro_supc->isChecked();
   cb_hydro_zeno->setChecked( (*misc).hydro_zeno );

   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::set_hydro_zeno()
{
   (*misc).hydro_zeno = cb_hydro_zeno->isChecked();
   (*misc).hydro_supc = !cb_hydro_zeno->isChecked();
   cb_hydro_supc->setChecked( (*misc).hydro_supc );
      
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

