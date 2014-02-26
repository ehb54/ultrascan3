#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_asa.h"
#include "../include/us_hydrodyn.h"
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <QCloseEvent>
#include <Q3GridLayout>
#include <Q3Frame>
#include <QLabel>

#define SLASH "/"
#if defined(WIN32)
#  undef SLASH
#  define SLASH "\\"
#endif

US_Hydrodyn_SasOptionsHydration::US_Hydrodyn_SasOptionsHydration(struct saxs_options *saxs_options, bool *sas_options_hydration_widget, void *us_hydrodyn, QWidget *p, const char *name) : Q3Frame(p, name)
{
   this->sas_options_hydration_widget = sas_options_hydration_widget;
   this->saxs_options = saxs_options;
   this->us_hydrodyn = us_hydrodyn;
   *sas_options_hydration_widget = true;
   USglobal=new US_Config();
   setPalette( PALET_FRAME );
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
   lbl_info->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette( PALET_FRAME );
   AUTFBACK( lbl_info );
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   cb_hydrate_pdb = new QCheckBox(this);
   cb_hydrate_pdb->setText(tr(" Hydrate the Original Model (PDB files only)"));
   cb_hydrate_pdb->setEnabled(true);
   cb_hydrate_pdb->setChecked((*saxs_options).hydrate_pdb);
   cb_hydrate_pdb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_hydrate_pdb->setPalette( PALET_NORMAL );
   AUTFBACK( cb_hydrate_pdb );
   connect(cb_hydrate_pdb, SIGNAL(clicked()), this, SLOT(set_hydrate_pdb()));
   set_hydrate_pdb();

   pb_default_rotamer_filename = new QPushButton(tr("Set Hydrated Rotamer File"), this);
   pb_default_rotamer_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_default_rotamer_filename->setMinimumHeight(minHeight1);
   pb_default_rotamer_filename->setPalette( PALET_PUSHB );
   connect(pb_default_rotamer_filename, SIGNAL(clicked()), SLOT(default_rotamer_filename()));

   le_default_rotamer_filename = new QLineEdit(this, "");
   le_default_rotamer_filename->setText(QFileInfo((*saxs_options).default_rotamer_filename).fileName());
   // le_default_rotamer_filename->setMinimumHeight(minHeight1);
   // le_default_rotamer_filename->setMaximumHeight(minHeight1);
   le_default_rotamer_filename->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   le_default_rotamer_filename->setPalette( PALET_EDIT );
   AUTFBACK( le_default_rotamer_filename );
   le_default_rotamer_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_default_rotamer_filename->setReadOnly(true);

   lbl_steric_clash_distance = new QLabel(tr(" Steric clash tolerance % : "), this);
   lbl_steric_clash_distance->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_steric_clash_distance->setMinimumHeight(minHeight1);
   lbl_steric_clash_distance->setPalette( PALET_LABEL );
   AUTFBACK( lbl_steric_clash_distance );
   lbl_steric_clash_distance->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_steric_clash_distance = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_steric_clash_distance );
   cnt_steric_clash_distance->setRange(0, 100, 1);
   cnt_steric_clash_distance->setValue((*saxs_options).steric_clash_distance);
   cnt_steric_clash_distance->setMinimumHeight(minHeight1);
   cnt_steric_clash_distance->setEnabled(true);
   cnt_steric_clash_distance->setNumButtons(2);
   cnt_steric_clash_distance->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_steric_clash_distance->setPalette( PALET_NORMAL );
   AUTFBACK( cnt_steric_clash_distance );
   connect(cnt_steric_clash_distance, SIGNAL(valueChanged(double)), SLOT(update_steric_clash_distance(double)));

   lbl_steric_clash_recheck_distance = new QLabel(tr(" Steric clash recheck tolerance % : "), this);
   lbl_steric_clash_recheck_distance->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_steric_clash_recheck_distance->setMinimumHeight(minHeight1);
   lbl_steric_clash_recheck_distance->setPalette( PALET_LABEL );
   AUTFBACK( lbl_steric_clash_recheck_distance );
   lbl_steric_clash_recheck_distance->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_steric_clash_recheck_distance = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_steric_clash_recheck_distance );
   cnt_steric_clash_recheck_distance->setRange(0, 100, 1);
   cnt_steric_clash_recheck_distance->setValue((*saxs_options).steric_clash_recheck_distance);
   cnt_steric_clash_recheck_distance->setMinimumHeight(minHeight1);
   cnt_steric_clash_recheck_distance->setEnabled(true);
   cnt_steric_clash_recheck_distance->setNumButtons(2);
   cnt_steric_clash_recheck_distance->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_steric_clash_recheck_distance->setPalette( PALET_NORMAL );
   AUTFBACK( cnt_steric_clash_recheck_distance );
   connect(cnt_steric_clash_recheck_distance, SIGNAL(valueChanged(double)), SLOT(update_steric_clash_recheck_distance(double)));

   cb_alt_hydration = new QCheckBox(this);
   cb_alt_hydration->setText(tr(" Alt hydration"));
   cb_alt_hydration->setEnabled(true);
   cb_alt_hydration->setChecked((*saxs_options).alt_hydration);
   cb_alt_hydration->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_alt_hydration->setPalette( PALET_NORMAL );
   AUTFBACK( cb_alt_hydration );
   connect(cb_alt_hydration, SIGNAL(clicked()), this, SLOT(set_alt_hydration()));

   cb_hydration_rev_asa = new QCheckBox(this);
   cb_hydration_rev_asa->setText(tr(" Alt hydration reverse ASA sorting"));
   cb_hydration_rev_asa->setEnabled(true);
   cb_hydration_rev_asa->setChecked((*saxs_options).hydration_rev_asa);
   cb_hydration_rev_asa->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_hydration_rev_asa->setPalette( PALET_NORMAL );
   AUTFBACK( cb_hydration_rev_asa );
   connect(cb_hydration_rev_asa, SIGNAL(clicked()), this, SLOT(set_hydration_rev_asa()));

   pb_cancel = new QPushButton(tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   int rows = 0, columns = 2, spacing = 2, j=0, margin=4;
   Q3GridLayout *background=new Q3GridLayout(this, rows, columns, margin, spacing);

   background->addMultiCellWidget(lbl_info, j, j, 0, 1);
   j++;

   Q3HBoxLayout *hbl_curve_opts = new Q3HBoxLayout;
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
   
   Q3HBoxLayout *hbl_various_2c = new Q3HBoxLayout;
   hbl_various_2c->addWidget( cb_alt_hydration );
   hbl_various_2c->addWidget( cb_hydration_rev_asa );
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
   QString rotamer_filename = Q3FileDialog::getOpenFileName(USglobal->config_list.system_dir + SLASH + "etc", "*.hydrated_rotamer *.HYDRATED_ROTAMER", this);
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

void US_Hydrodyn_SasOptionsHydration::set_hydration_rev_asa()
{
   (*saxs_options).hydration_rev_asa = cb_hydration_rev_asa->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}
