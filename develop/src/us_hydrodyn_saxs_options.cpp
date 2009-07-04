#include "../include/us_hydrodyn_asa.h"
#include "../include/us_hydrodyn.h"

US_Hydrodyn_SaxsOptions::US_Hydrodyn_SaxsOptions(struct saxs_options *saxs_options, bool *saxs_options_widget, void *us_hydrodyn, QWidget *p, const char *name) : QFrame(p, name)
{
   this->saxs_options_widget = saxs_options_widget;
   this->saxs_options = saxs_options;
   this->us_hydrodyn = us_hydrodyn;
   *saxs_options_widget = true;
   USglobal=new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("SOMO SAXS Simulation Options"));
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_SaxsOptions::~US_Hydrodyn_SaxsOptions()
{
   *saxs_options_widget = false;
}

void US_Hydrodyn_SaxsOptions::setupGUI()
{
   int minHeight1 = 30;
   lbl_info = new QLabel(tr("SOMO SAXS Simulation Options:"), this);
   Q_CHECK_PTR(lbl_info);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(AlignCenter|AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_curve = new QLabel(tr("SAXS Curve Options:"), this);
   Q_CHECK_PTR(lbl_curve);
   lbl_curve->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_curve->setAlignment(AlignCenter|AlignVCenter);
   lbl_curve->setMinimumHeight(minHeight1);
   lbl_curve->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_curve->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_pr = new QLabel(tr("SAXS P(r) vs r Options:"), this);
   Q_CHECK_PTR(lbl_pr);
   lbl_pr->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_pr->setAlignment(AlignCenter|AlignVCenter);
   lbl_pr->setMinimumHeight(minHeight1);
   lbl_pr->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_pr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   cb_hydrate_pdb = new QCheckBox(this);
   cb_hydrate_pdb->setText(tr(" Hydrate the Original Model (PDB files only)"));
   cb_hydrate_pdb->setEnabled(true);
   cb_hydrate_pdb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_hydrate_pdb->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   lbl_wavelength = new QLabel(tr(" Wavelength (Angstrom): "), this);
   Q_CHECK_PTR(lbl_wavelength);
   lbl_wavelength->setAlignment(AlignLeft|AlignVCenter);
   lbl_wavelength->setMinimumHeight(minHeight1);
   lbl_wavelength->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_wavelength->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_wavelength= new QwtCounter(this);
   Q_CHECK_PTR(cnt_wavelength);
   cnt_wavelength->setRange(0, 10, 0.01);
   cnt_wavelength->setValue((*saxs_options).wavelength);
   cnt_wavelength->setMinimumHeight(minHeight1);
   cnt_wavelength->setEnabled(true);
   cnt_wavelength->setNumButtons(3);
   cnt_wavelength->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_wavelength->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_wavelength, SIGNAL(valueChanged(double)), SLOT(update_wavelength(double)));

   lbl_start_angle = new QLabel(tr(" Starting Angle: "), this);
   Q_CHECK_PTR(lbl_start_angle);
   lbl_start_angle->setAlignment(AlignLeft|AlignVCenter);
   lbl_start_angle->setMinimumHeight(minHeight1);
   lbl_start_angle->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_start_angle->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_start_angle= new QwtCounter(this);
   Q_CHECK_PTR(cnt_start_angle);
   cnt_start_angle->setRange(0, 90, 0.001);
   cnt_start_angle->setValue((*saxs_options).start_angle);
   cnt_start_angle->setMinimumHeight(minHeight1);
   cnt_start_angle->setEnabled(true);
   cnt_start_angle->setNumButtons(3);
   cnt_start_angle->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_start_angle->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_start_angle, SIGNAL(valueChanged(double)), SLOT(update_start_angle(double)));

   lbl_end_angle = new QLabel(tr(" Ending Angle: "), this);
   Q_CHECK_PTR(lbl_end_angle);
   lbl_end_angle->setAlignment(AlignLeft|AlignVCenter);
   lbl_end_angle->setMinimumHeight(minHeight1);
   lbl_end_angle->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_end_angle->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_end_angle= new QwtCounter(this);
   Q_CHECK_PTR(cnt_end_angle);
   cnt_end_angle->setRange(0, 90, 0.001);
   cnt_end_angle->setValue((*saxs_options).end_angle);
   cnt_end_angle->setMinimumHeight(minHeight1);
   cnt_end_angle->setEnabled(true);
   cnt_end_angle->setNumButtons(3);
   cnt_end_angle->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_end_angle->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_end_angle, SIGNAL(valueChanged(double)), SLOT(update_end_angle(double)));

   lbl_water_e_density = new QLabel(tr(" Water electron density (A^3): "), this);
   Q_CHECK_PTR(lbl_water_e_density);
   lbl_water_e_density->setAlignment(AlignLeft|AlignVCenter);
   lbl_water_e_density->setMinimumHeight(minHeight1);
   lbl_water_e_density->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_water_e_density->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_water_e_density= new QwtCounter(this);
   Q_CHECK_PTR(cnt_water_e_density);
   cnt_water_e_density->setRange(0, 10, 0.001);
   cnt_water_e_density->setValue((*saxs_options).water_e_density);
   cnt_water_e_density->setMinimumHeight(minHeight1);
   cnt_water_e_density->setEnabled(true);
   cnt_water_e_density->setNumButtons(3);
   cnt_water_e_density->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_water_e_density->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_water_e_density, SIGNAL(valueChanged(double)), SLOT(update_water_e_density(double)));

   lbl_max_size = new QLabel(tr(" Maximum size (Angstrom): "), this);
   Q_CHECK_PTR(lbl_max_size);
   lbl_max_size->setAlignment(AlignLeft|AlignVCenter);
   lbl_max_size->setMinimumHeight(minHeight1);
   lbl_max_size->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_max_size->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_max_size= new QwtCounter(this);
   Q_CHECK_PTR(cnt_max_size);
   cnt_max_size->setRange(0, 100, 0.01);
   cnt_max_size->setValue((*saxs_options).max_size);
   cnt_max_size->setMinimumHeight(minHeight1);
   cnt_max_size->setEnabled(true);
   cnt_max_size->setNumButtons(3);
   cnt_max_size->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_max_size->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_max_size, SIGNAL(valueChanged(double)), SLOT(update_max_size(double)));

   lbl_bin_size = new QLabel(tr(" Bin size (Angstrom): "), this);
   Q_CHECK_PTR(lbl_bin_size);
   lbl_bin_size->setAlignment(AlignLeft|AlignVCenter);
   lbl_bin_size->setMinimumHeight(minHeight1);
   lbl_bin_size->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_bin_size->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_bin_size= new QwtCounter(this);
   Q_CHECK_PTR(cnt_bin_size);
   cnt_bin_size->setRange(0, 100, 0.01);
   cnt_bin_size->setValue((*saxs_options).bin_size);
   cnt_bin_size->setMinimumHeight(minHeight1);
   cnt_bin_size->setEnabled(true);
   cnt_bin_size->setNumButtons(3);
   cnt_bin_size->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_bin_size->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_bin_size, SIGNAL(valueChanged(double)), SLOT(update_bin_size(double)));

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

   int rows=8, columns = 2, spacing = 2, j=0, margin=4;
   QGridLayout *background=new QGridLayout(this, rows, columns, margin, spacing);

   background->addMultiCellWidget(lbl_info, j, j, 0, 1);
   j++;
   background->addMultiCellWidget(lbl_curve, j, j, 0, 1);
   j++;
   background->addWidget(lbl_wavelength, j, 0);
   background->addWidget(cnt_wavelength, j, 1);
   j++;
   background->addWidget(lbl_start_angle, j, 0);
   background->addWidget(cnt_start_angle, j, 1);
   j++;
   background->addWidget(lbl_end_angle, j, 0);
   background->addWidget(cnt_end_angle, j, 1);
   j++;
   background->addWidget(lbl_water_e_density, j, 0);
   background->addWidget(cnt_water_e_density, j, 1);
   j++;
   background->addMultiCellWidget(cb_hydrate_pdb, j, j, 0, 1);
   j++;
   background->addMultiCellWidget(lbl_pr, j, j, 0, 1);
   j++;
   background->addWidget(lbl_max_size, j, 0);
   background->addWidget(cnt_max_size, j, 1);
   j++;
   background->addWidget(lbl_bin_size, j, 0);
   background->addWidget(cnt_bin_size, j, 1);
   j++;
   background->addWidget(pb_help, j, 0);
   background->addWidget(pb_cancel, j, 1);
}

void US_Hydrodyn_SaxsOptions::cancel()
{
   close();
}

void US_Hydrodyn_SaxsOptions::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_saxs_options.html");
}

void US_Hydrodyn_SaxsOptions::closeEvent(QCloseEvent *e)
{
   *saxs_options_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_SaxsOptions::update_wavelength(double val)
{
   (*saxs_options).wavelength = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::update_start_angle(double val)
{
   (*saxs_options).start_angle = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::update_end_angle(double val)
{
   (*saxs_options).end_angle = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::update_water_e_density(double val)
{
   (*saxs_options).water_e_density = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::update_max_size(double val)
{
   (*saxs_options).max_size = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::update_bin_size(double val)
{
   (*saxs_options).bin_size = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SaxsOptions::set_hydrate_pdb()
{
   (*saxs_options).hydrate_pdb = cb_hydrate_pdb->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}


