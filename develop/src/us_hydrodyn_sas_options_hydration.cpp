#include "../include/us_hydrodyn_asa.h"
#include "../include/us_hydrodyn.h"

#define SLASH "/"
#if defined(WIN32)
#  undef SLASH
#  define SLASH "\\"
#endif

US_Hydrodyn_SasOptionsHydration::US_Hydrodyn_SasOptionsHydration(struct saxs_options *saxs_options, bool *sas_options_hydration_widget, void *us_hydrodyn, QWidget *p, const char *name) : QFrame(p, name)
{
   this->sas_options_hydration_widget = sas_options_hydration_widget;
   this->saxs_options = saxs_options;
   this->us_hydrodyn = us_hydrodyn;
   *sas_options_hydration_widget = true;
   USglobal=new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("US-SOMO SAS Hydration Options"));
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_SasOptionsHydration::~US_Hydrodyn_SasOptionsHydration()
{
   *sas_options_hydration_widget = false;
}

void US_Hydrodyn_SasOptionsHydration::setupGUI()
{
   int minHeight1 = 30;
   lbl_info = new QLabel(tr("US-SOMO SAS Hydration Options:"), this);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   cb_hydrate_pdb = new QCheckBox(this);
   cb_hydrate_pdb->setText(tr(" Hydrate the Original Model (PDB files only)"));
   cb_hydrate_pdb->setEnabled(true);
   cb_hydrate_pdb->setChecked((*saxs_options).hydrate_pdb);
   cb_hydrate_pdb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_hydrate_pdb->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_hydrate_pdb, SIGNAL(clicked()), this, SLOT(set_hydrate_pdb()));
   set_hydrate_pdb();

   pb_default_rotamer_filename = new QPushButton(tr("Set Hydrated Rotamer File"), this);
   pb_default_rotamer_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_default_rotamer_filename->setMinimumHeight(minHeight1);
   pb_default_rotamer_filename->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_default_rotamer_filename, SIGNAL(clicked()), SLOT(default_rotamer_filename()));

   le_default_rotamer_filename = new QLineEdit(this, "");
   le_default_rotamer_filename->setText(QFileInfo((*saxs_options).default_rotamer_filename).fileName());
   // le_default_rotamer_filename->setMinimumHeight(minHeight1);
   // le_default_rotamer_filename->setMaximumHeight(minHeight1);
   le_default_rotamer_filename->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   le_default_rotamer_filename->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   le_default_rotamer_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_default_rotamer_filename->setReadOnly(true);

   lbl_steric_clash_distance = new QLabel(tr(" Steric clash tolerance % : "), this);
   lbl_steric_clash_distance->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_steric_clash_distance->setMinimumHeight(minHeight1);
   lbl_steric_clash_distance->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_steric_clash_distance->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_steric_clash_distance = new QwtCounter(this);
   cnt_steric_clash_distance->setRange(0, 100, 1);
   cnt_steric_clash_distance->setValue((*saxs_options).steric_clash_distance);
   cnt_steric_clash_distance->setMinimumHeight(minHeight1);
   cnt_steric_clash_distance->setEnabled(true);
   cnt_steric_clash_distance->setNumButtons(2);
   cnt_steric_clash_distance->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_steric_clash_distance->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_steric_clash_distance, SIGNAL(valueChanged(double)), SLOT(update_steric_clash_distance(double)));

   lbl_steric_clash_recheck_distance = new QLabel(tr(" Steric clash recheck tolerance % : "), this);
   lbl_steric_clash_recheck_distance->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_steric_clash_recheck_distance->setMinimumHeight(minHeight1);
   lbl_steric_clash_recheck_distance->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_steric_clash_recheck_distance->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_steric_clash_recheck_distance = new QwtCounter(this);
   cnt_steric_clash_recheck_distance->setRange(0, 100, 1);
   cnt_steric_clash_recheck_distance->setValue((*saxs_options).steric_clash_recheck_distance);
   cnt_steric_clash_recheck_distance->setMinimumHeight(minHeight1);
   cnt_steric_clash_recheck_distance->setEnabled(true);
   cnt_steric_clash_recheck_distance->setNumButtons(2);
   cnt_steric_clash_recheck_distance->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_steric_clash_recheck_distance->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_steric_clash_recheck_distance, SIGNAL(valueChanged(double)), SLOT(update_steric_clash_recheck_distance(double)));

   cb_alt_hydration = new QCheckBox(this);
   cb_alt_hydration->setText(tr(" Alt hydration"));
   cb_alt_hydration->setEnabled(true);
   cb_alt_hydration->setChecked((*saxs_options).alt_hydration);
   cb_alt_hydration->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_alt_hydration->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_alt_hydration, SIGNAL(clicked()), this, SLOT(set_alt_hydration()));

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

   int rows = 0, columns = 2, spacing = 2, j=0, margin=4;
   QGridLayout *background=new QGridLayout(this, rows, columns, margin, spacing);

   background->addMultiCellWidget(lbl_info, j, j, 0, 1);
   j++;

   QHBoxLayout *hbl_curve_opts = new QHBoxLayout;
   hbl_curve_opts->addWidget(cb_hydrate_pdb);
   background->addMultiCellLayout(hbl_curve_opts, j, j, 0, 1);
   j++;

   background->addWidget(pb_default_rotamer_filename, j, 0 );
   background->addWidget(le_default_rotamer_filename, j, 1 );
   j++;

   background->addWidget(lbl_steric_clash_distance, j, 0);
   background->addWidget(cnt_steric_clash_distance, j, 1);
   j++;
   background->addWidget(lbl_steric_clash_recheck_distance, j, 0);
   background->addWidget(cnt_steric_clash_recheck_distance, j, 1);
   j++;
   
   QHBoxLayout *hbl_various_2c = new QHBoxLayout;
   hbl_various_2c->addWidget( cb_alt_hydration );
   background->addMultiCellLayout(hbl_various_2c, j, j, 0, 1);
   j++;

   background->addWidget( pb_help  , j, 0 );
   background->addWidget( pb_cancel, j, 1 );

   setMinimumWidth( 500 );
}

void US_Hydrodyn_SasOptionsHydration::cancel()
{
   close();
}

void US_Hydrodyn_SasOptionsHydration::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   // online_help->show_help("manual/somo_sas_options_hydration.html");
   online_help->show_help("manual/somo_saxs_options.html");
}

void US_Hydrodyn_SasOptionsHydration::closeEvent(QCloseEvent *e)
{
   *sas_options_hydration_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_SasOptionsHydration::set_hydrate_pdb()
{
   (*saxs_options).hydrate_pdb = cb_hydrate_pdb->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsHydration::default_rotamer_filename()
{
   QString rotamer_filename = QFileDialog::getOpenFileName(USglobal->config_list.system_dir + SLASH + "etc", "*.hydrated_rotamer *.HYDRATED_ROTAMER", this);
   if (rotamer_filename.isEmpty())
   {
      return;
   }
   else
   {
      (*saxs_options).default_rotamer_filename = rotamer_filename;
      ((US_Hydrodyn *)us_hydrodyn)->rotamer_changed = true;      
      le_default_rotamer_filename->setText( QFileInfo(rotamer_filename).fileName() );
   }
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsHydration::update_steric_clash_distance(double val)
{
   (*saxs_options).steric_clash_distance = val;
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsHydration::update_steric_clash_recheck_distance(double val)
{
   (*saxs_options).steric_clash_recheck_distance = val;
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsHydration::set_alt_hydration()
{
   (*saxs_options).alt_hydration = cb_alt_hydration->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}
