#include "../include/us_hydrodyn_bd_options.h"
#include "../include/us_hydrodyn.h"

US_Hydrodyn_BD_Options::US_Hydrodyn_BD_Options(BD_Options *bd_options, bool *bd_widget, void *us_hydrodyn, QWidget *p, const char *name) : QFrame(p, name)
{
   this->bd_options = bd_options;
   this->bd_widget = bd_widget;
   this->us_hydrodyn = us_hydrodyn;
   *bd_widget = true;
   USglobal=new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("SOMO BD Options"));
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_BD_Options::~US_Hydrodyn_BD_Options()
{
   *bd_widget = false;
}

void US_Hydrodyn_BD_Options::setupGUI()
{
   int minHeight1 = 30;
   lbl_info = new QLabel(tr("BD Options:"), this);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(AlignCenter|AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_bd_threshold = new QLabel(tr(" BD threshold (A): "), this);
   lbl_bd_threshold->setAlignment(AlignLeft|AlignVCenter);
   lbl_bd_threshold->setMinimumHeight(minHeight1);
   lbl_bd_threshold->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_bd_threshold->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_bd_threshold= new QwtCounter(this);
   cnt_bd_threshold->setRange(0, 100, 0.1);
   cnt_bd_threshold->setValue((*bd_options).threshold);
   cnt_bd_threshold->setMinimumHeight(minHeight1);
   cnt_bd_threshold->setMinimumWidth(150);
   cnt_bd_threshold->setEnabled(true);
   cnt_bd_threshold->setNumButtons(3);
   cnt_bd_threshold->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_bd_threshold->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_bd_threshold, SIGNAL(valueChanged(double)), SLOT(update_bd_threshold(double)));

   cb_do_rr = new QCheckBox(this);
   cb_do_rr->setText(tr(" Use radial reduction "));
   cb_do_rr->setChecked((*bd_options).do_rr);
   cb_do_rr->setEnabled(true);
   cb_do_rr->setMinimumHeight(minHeight1);
   cb_do_rr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_do_rr->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_do_rr, SIGNAL(clicked()), SLOT(set_do_rr()));

   cb_force_chem = new QCheckBox(this);
   cb_force_chem->setText(tr(" Force chemical bonds as connectors "));
   cb_force_chem->setChecked((*bd_options).force_chem);
   cb_force_chem->setEnabled(true);
   cb_force_chem->setMinimumHeight(minHeight1);
   cb_force_chem->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_force_chem->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_force_chem, SIGNAL(clicked()), SLOT(set_force_chem()));

   cb_include_sc = new QCheckBox(this);
   cb_include_sc->setText(tr(" Include side chains "));
   cb_include_sc->setChecked((*bd_options).include_sc);
   cb_include_sc->setEnabled(true);
   cb_include_sc->setMinimumHeight(minHeight1);
   cb_include_sc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_include_sc->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_include_sc, SIGNAL(clicked()), SLOT(set_include_sc()));

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

   int rows=1, columns = 2, spacing = 2, j=0, margin=4;
   QGridLayout *background=new QGridLayout(this, rows, columns, margin, spacing);

   background->addMultiCellWidget(lbl_info, j, j, 0, 1);
   j++;
   background->addWidget(lbl_bd_threshold, j, 0);
   background->addWidget(cnt_bd_threshold, j, 1);
   j++;
   background->addMultiCellWidget(cb_do_rr, j, j, 0, 1);
   j++;
   background->addMultiCellWidget(cb_force_chem, j, j, 0, 1);
   j++;
   background->addMultiCellWidget(cb_include_sc, j, j, 0, 1);
   j++;
   background->addWidget(pb_help, j, 0);
   background->addWidget(pb_cancel, j, 1);
}

void US_Hydrodyn_BD_Options::cancel()
{
   close();
}

void US_Hydrodyn_BD_Options::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_bd_options.html");
}

void US_Hydrodyn_BD_Options::closeEvent(QCloseEvent *e)
{
   *bd_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_BD_Options::update_bd_threshold(double val)
{
   (*bd_options).threshold = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_do_rr()
{
   (*bd_options).do_rr = cb_do_rr->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_force_chem()
{
   (*bd_options).force_chem = cb_force_chem->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_include_sc()
{
   (*bd_options).include_sc = cb_include_sc->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}
