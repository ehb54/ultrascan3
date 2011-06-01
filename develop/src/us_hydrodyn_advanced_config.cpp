#include "../include/us_hydrodyn_asa.h"
#include "../include/us_hydrodyn.h"

// #define DEBUG_CTLS

US_Hydrodyn_AdvancedConfig::US_Hydrodyn_AdvancedConfig(struct advanced_config *advanced_config, bool *advanced_config_widget, void *us_hydrodyn, QWidget *p, const char *name) : QFrame(p, name)
{
   this->advanced_config_widget = advanced_config_widget;
   this->advanced_config = advanced_config;
   this->us_hydrodyn = us_hydrodyn;
   *advanced_config_widget = true;
   USglobal=new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("SOMO Advanced Configuration"));
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_AdvancedConfig::~US_Hydrodyn_AdvancedConfig()
{
   *advanced_config_widget = false;
}

void US_Hydrodyn_AdvancedConfig::setupGUI()
{
   int minHeight1 = 30;
   lbl_info = new QLabel(tr("SOMO Advanced Configuration:"), this);
   Q_CHECK_PTR(lbl_info);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   cb_auto_view_pdb = new QCheckBox(this);
   cb_auto_view_pdb->setText(tr(" Automatically view PDB structure upon loading"));
   cb_auto_view_pdb->setEnabled(true);
   cb_auto_view_pdb->setChecked((*advanced_config).auto_view_pdb);
   cb_auto_view_pdb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_auto_view_pdb->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_auto_view_pdb, SIGNAL(clicked()), this, SLOT(set_auto_view_pdb()));

   cb_scroll_editor = new QCheckBox(this);
   cb_scroll_editor->setText(tr(" Side scroll text window"));
   cb_scroll_editor->setEnabled(true);
   cb_scroll_editor->setChecked((*advanced_config).scroll_editor);
   cb_scroll_editor->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_scroll_editor->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_scroll_editor, SIGNAL(clicked()), this, SLOT(set_scroll_editor()));

   cb_auto_calc_somo = new QCheckBox(this);
   cb_auto_calc_somo->setText(tr(" Automatically build somo bead model upon loading pdb"));
   cb_auto_calc_somo->setEnabled(true);
   cb_auto_calc_somo->setChecked((*advanced_config).auto_calc_somo);
   cb_auto_calc_somo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_auto_calc_somo->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_auto_calc_somo, SIGNAL(clicked()), this, SLOT(set_auto_calc_somo()));

   cb_auto_show_hydro = new QCheckBox(this);
   cb_auto_show_hydro->setText(tr(" Automatically show hydrodynamic calculations"));
   cb_auto_show_hydro->setEnabled(true);
   cb_auto_show_hydro->setChecked((*advanced_config).auto_show_hydro);
   cb_auto_show_hydro->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_auto_show_hydro->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_auto_show_hydro, SIGNAL(clicked()), this, SLOT(set_auto_show_hydro()));

#if defined(DEBUG_CTLS)
   cb_pbr_broken_logic = new QCheckBox(this);
   cb_pbr_broken_logic->setText(tr(" Enable logic for broken chains with peptide bond rule"));
   cb_pbr_broken_logic->setEnabled(true);
   cb_pbr_broken_logic->setChecked((*advanced_config).pbr_broken_logic);
   cb_pbr_broken_logic->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pbr_broken_logic->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_pbr_broken_logic, SIGNAL(clicked()), this, SLOT(set_pbr_broken_logic()));
#endif

   cb_use_sounds = new QCheckBox(this);
   cb_use_sounds->setText(tr(" Activate sound notifications"));
   cb_use_sounds->setEnabled(true);
   cb_use_sounds->setChecked((*advanced_config).use_sounds);
   cb_use_sounds->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_use_sounds->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_use_sounds, SIGNAL(clicked()), this, SLOT(set_use_sounds()));

   cb_expert_mode = new QCheckBox(this);
   cb_expert_mode->setText(tr(" Expert mode (skip some warning messages)"));
   cb_expert_mode->setEnabled(true);
   cb_expert_mode->setChecked((*advanced_config).expert_mode);
   cb_expert_mode->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_expert_mode->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_expert_mode, SIGNAL(clicked()), this, SLOT(set_expert_mode()));

#if defined(DEBUG_CTLS)
   cb_experimental_threads = new QCheckBox(this);
   cb_experimental_threads->setText(tr(" Use threads (experimental)"));
   cb_experimental_threads->setEnabled(true);
   cb_experimental_threads->setChecked((*advanced_config).experimental_threads);
   cb_experimental_threads->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_experimental_threads->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_experimental_threads, SIGNAL(clicked()), this, SLOT(set_experimental_threads()));

   cb_debug_1 = new QCheckBox(this);
   cb_debug_1->setText(tr(" Debug molecular weight/volume adjustments"));
   cb_debug_1->setEnabled(true);
   cb_debug_1->setChecked((*advanced_config).debug_1);
   cb_debug_1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_debug_1->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_debug_1, SIGNAL(clicked()), this, SLOT(set_debug_1()));

   cb_debug_2 = new QCheckBox(this);
   cb_debug_2->setText(tr(" Debug automatic bead builder"));
   cb_debug_2->setEnabled(true);
   cb_debug_2->setChecked((*advanced_config).debug_2);
   cb_debug_2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_debug_2->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_debug_2, SIGNAL(clicked()), this, SLOT(set_debug_2()));

   cb_debug_3 = new QCheckBox(this);
   cb_debug_3->setText(tr(" Debug COG calculation"));
   cb_debug_3->setEnabled(true);
   cb_debug_3->setChecked((*advanced_config).debug_3);
   cb_debug_3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_debug_3->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_debug_3, SIGNAL(clicked()), this, SLOT(set_debug_3()));

   cb_debug_4 = new QCheckBox(this);
   cb_debug_4->setText(tr(" Debug save controls"));
   cb_debug_4->setEnabled(true);
   cb_debug_4->setChecked((*advanced_config).debug_4);
   cb_debug_4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_debug_4->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_debug_4, SIGNAL(clicked()), this, SLOT(set_debug_4()));
#endif

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

   int rows = 8
#if defined(DEBUG_CTLS)   
      + 5
#endif
      ;

   int columns = 2;
   int spacing = 2;
   int j=0;
   int margin=4;

   QGridLayout *background=new QGridLayout(this, rows, columns, margin, spacing);

   background->addMultiCellWidget(lbl_info, j, j, 0, 1);
   j++;
   background->addMultiCellWidget(cb_auto_view_pdb, j, j, 0, 1);
   j++;
   background->addMultiCellWidget(cb_scroll_editor, j, j, 0, 1);
   j++;
   background->addMultiCellWidget(cb_auto_calc_somo, j, j, 0, 1);
   j++;
   background->addMultiCellWidget(cb_auto_show_hydro, j, j, 0, 1);
   j++;
#if defined(DEBUG_CTLS)   
   background->addMultiCellWidget(cb_pbr_broken_logic, j, j, 0, 1);
   j++;
#endif
   background->addMultiCellWidget(cb_use_sounds, j, j, 0, 1);
   j++;
   background->addMultiCellWidget(cb_expert_mode, j, j, 0, 1);
   j++;
#if defined(DEBUG_CTLS)   
   background->addMultiCellWidget(cb_experimental_threads, j, j, 0, 1);
   j++;
   background->addMultiCellWidget(cb_debug_1, j, j, 0, 1);
   j++;
   background->addMultiCellWidget(cb_debug_2, j, j, 0, 1);
   j++;
   background->addMultiCellWidget(cb_debug_3, j, j, 0, 1);
   j++;
   background->addMultiCellWidget(cb_debug_4, j, j, 0, 1);
   j++;
#endif
   background->addWidget(pb_help, j, 0);
   background->addWidget(pb_cancel, j, 1);
}

void US_Hydrodyn_AdvancedConfig::cancel()
{
   close();
}

void US_Hydrodyn_AdvancedConfig::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_advanced_config.html");
}

void US_Hydrodyn_AdvancedConfig::closeEvent(QCloseEvent *e)
{
   *advanced_config_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_AdvancedConfig::set_auto_view_pdb()
{
   (*advanced_config).auto_view_pdb = cb_auto_view_pdb->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_AdvancedConfig::set_scroll_editor()
{
   (*advanced_config).scroll_editor = cb_scroll_editor->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->editor->setWordWrap (cb_scroll_editor->isChecked() ? QTextEdit::NoWrap : QTextEdit::WidgetWidth );
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_AdvancedConfig::set_auto_calc_somo()
{
   (*advanced_config).auto_calc_somo = cb_auto_calc_somo->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_AdvancedConfig::set_auto_show_hydro()
{
   (*advanced_config).auto_show_hydro = cb_auto_show_hydro->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_AdvancedConfig::set_pbr_broken_logic()
{
   (*advanced_config).pbr_broken_logic = cb_pbr_broken_logic->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_AdvancedConfig::set_use_sounds()
{
   (*advanced_config).use_sounds = cb_use_sounds->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_AdvancedConfig::set_expert_mode()
{
   (*advanced_config).expert_mode = cb_expert_mode->isChecked();
   if ( cb_expert_mode->isChecked() )
   {
      cb_auto_view_pdb->setChecked(false);
      set_auto_view_pdb();
      cb_auto_calc_somo->setChecked(true);
      set_auto_calc_somo();
      cb_auto_calc_somo->setChecked(true);
      set_auto_calc_somo();
      cb_auto_show_hydro->setChecked(true);
      set_auto_show_hydro();
      cb_scroll_editor->setChecked(true);
      set_scroll_editor();
   }

   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_AdvancedConfig::set_experimental_threads()
{
   (*advanced_config).experimental_threads = cb_experimental_threads->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_AdvancedConfig::set_debug_1()
{
   (*advanced_config).debug_1 = cb_debug_1->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_AdvancedConfig::set_debug_2()
{
   (*advanced_config).debug_2 = cb_debug_2->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_AdvancedConfig::set_debug_3()
{
   (*advanced_config).debug_3 = cb_debug_3->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_AdvancedConfig::set_debug_4()
{
   (*advanced_config).debug_4 = cb_debug_4->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

