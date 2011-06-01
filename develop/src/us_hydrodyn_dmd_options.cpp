#include "../include/us_hydrodyn_dmd_options.h"
#include "../include/us_hydrodyn.h"

US_Hydrodyn_DMD_Options::US_Hydrodyn_DMD_Options(DMD_Options *dmd_options, bool *dmd_widget, void *us_hydrodyn, QWidget *p, const char *name) : QFrame(p, name)
{
   this->dmd_options = dmd_options;
   this->dmd_widget = dmd_widget;
   this->us_hydrodyn = us_hydrodyn;
   *dmd_widget = true;
   USglobal=new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("SOMO DMD Options"));
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_DMD_Options::~US_Hydrodyn_DMD_Options()
{
   *dmd_widget = false;
}

void US_Hydrodyn_DMD_Options::setupGUI()
{
   int minHeight1 = 30;
   lbl_info = new QLabel(tr("DMD Options:"), this);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   cb_pdb_static_pairs = new QCheckBox(this);
   cb_pdb_static_pairs->setText(tr(" Create static pairs on load PDB "));
   cb_pdb_static_pairs->setChecked((*dmd_options).pdb_static_pairs);
   cb_pdb_static_pairs->setEnabled(true);
   //   cb_pdb_static_pairs->setMinimumHeight(minHeight1);
   cb_pdb_static_pairs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pdb_static_pairs->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_pdb_static_pairs, SIGNAL(clicked()), SLOT(set_pdb_static_pairs()));

   lbl_info_model_creation = new QLabel(tr("Pair Discovery:"), this);
   lbl_info_model_creation->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info_model_creation->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info_model_creation->setMinimumHeight(minHeight1);
   lbl_info_model_creation->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info_model_creation->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_threshold_pb_pb = new QLabel(tr(" Threshold PB-PB (A): "), this);
   lbl_threshold_pb_pb->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_threshold_pb_pb->setMinimumHeight(minHeight1);
   lbl_threshold_pb_pb->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_threshold_pb_pb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_threshold_pb_pb= new QwtCounter(this);
   cnt_threshold_pb_pb->setRange(0, 100, 0.1);
   cnt_threshold_pb_pb->setValue((*dmd_options).threshold_pb_pb);
   cnt_threshold_pb_pb->setMinimumHeight(minHeight1);
   cnt_threshold_pb_pb->setMinimumWidth(150);
   cnt_threshold_pb_pb->setEnabled(true);
   cnt_threshold_pb_pb->setNumButtons(3);
   cnt_threshold_pb_pb->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_threshold_pb_pb->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_threshold_pb_pb, SIGNAL(valueChanged(double)), SLOT(update_threshold_pb_pb(double)));

   lbl_threshold_pb_sc = new QLabel(tr(" Threshold PB-SC (A): "), this);
   lbl_threshold_pb_sc->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_threshold_pb_sc->setMinimumHeight(minHeight1);
   lbl_threshold_pb_sc->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_threshold_pb_sc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_threshold_pb_sc = new QwtCounter(this);
   cnt_threshold_pb_sc->setRange(0, 100, 0.1);
   cnt_threshold_pb_sc->setValue((*dmd_options).threshold_pb_sc);
   cnt_threshold_pb_sc->setMinimumHeight(minHeight1);
   cnt_threshold_pb_sc->setMinimumWidth(150);
   cnt_threshold_pb_sc->setEnabled(true);
   cnt_threshold_pb_sc->setNumButtons(3);
   cnt_threshold_pb_sc->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_threshold_pb_sc->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_threshold_pb_sc, SIGNAL(valueChanged(double)), SLOT(update_threshold_pb_sc(double)));

   lbl_threshold_sc_sc = new QLabel(tr(" Threshold SC-SC (A): "), this);
   lbl_threshold_sc_sc->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_threshold_sc_sc->setMinimumHeight(minHeight1);
   lbl_threshold_sc_sc->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_threshold_sc_sc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_threshold_sc_sc= new QwtCounter(this);
   cnt_threshold_sc_sc->setRange(0, 100, 0.1);
   cnt_threshold_sc_sc->setValue((*dmd_options).threshold_sc_sc);
   cnt_threshold_sc_sc->setMinimumHeight(minHeight1);
   cnt_threshold_sc_sc->setMinimumWidth(150);
   cnt_threshold_sc_sc->setEnabled(true);
   cnt_threshold_sc_sc->setNumButtons(3);
   cnt_threshold_sc_sc->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_threshold_sc_sc->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_threshold_sc_sc, SIGNAL(valueChanged(double)), SLOT(update_threshold_sc_sc(double)));

   cb_force_chem = new QCheckBox(this);
   cb_force_chem->setText(tr(" Force chemical bonds as connectors "));
   cb_force_chem->setChecked((*dmd_options).force_chem);
   cb_force_chem->setEnabled(true);
   // cb_force_chem->setMinimumHeight(minHeight1);
   cb_force_chem->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_force_chem->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_force_chem, SIGNAL(clicked()), SLOT(set_force_chem()));

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

   QVBoxLayout *vbl_top = new QVBoxLayout;

   vbl_top->addWidget(lbl_info);
   vbl_top->addSpacing(3);

   QGridLayout *gl_thresh = new QGridLayout;

   int j = 0;
   gl_thresh->addMultiCellWidget(lbl_info_model_creation, j, j, 0, 1); j++;
   gl_thresh->addWidget(lbl_threshold_pb_pb, j, 0);
   gl_thresh->addWidget(cnt_threshold_pb_pb, j, 1); j++;
   gl_thresh->addWidget(lbl_threshold_pb_sc, j, 0);
   gl_thresh->addWidget(cnt_threshold_pb_sc, j, 1); j++;
   gl_thresh->addWidget(lbl_threshold_sc_sc, j, 0);
   gl_thresh->addWidget(cnt_threshold_sc_sc, j, 1); j++;
   gl_thresh->addMultiCellWidget(cb_pdb_static_pairs, j, j, 0, 1); j++;
   gl_thresh->addMultiCellWidget(cb_force_chem, j, j, 0, 1); j++;

   vbl_top->addSpacing(3);
   vbl_top->addLayout(gl_thresh);

   QHBoxLayout *hbl_buttons = new QHBoxLayout;

   hbl_buttons->addWidget(pb_help);
   hbl_buttons->addWidget(pb_cancel);
   
   vbl_top->addSpacing(3);
   vbl_top->addLayout(hbl_buttons);

   QHBoxLayout *background = new QHBoxLayout(this);
   background->addLayout(vbl_top);
}

void US_Hydrodyn_DMD_Options::cancel()
{
   close();
}

void US_Hydrodyn_DMD_Options::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_dmd_options.html");
}

void US_Hydrodyn_DMD_Options::closeEvent(QCloseEvent *e)
{
   *dmd_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_DMD_Options::update_threshold_pb_pb(double val)
{
   (*dmd_options).threshold_pb_pb = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_DMD_Options::update_threshold_pb_sc(double val)
{
   (*dmd_options).threshold_pb_sc = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_DMD_Options::update_threshold_sc_sc(double val)
{
   (*dmd_options).threshold_sc_sc = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_DMD_Options::set_pdb_static_pairs()
{
   (*dmd_options).pdb_static_pairs = cb_pdb_static_pairs->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_DMD_Options::set_force_chem()
{
   (*dmd_options).force_chem = cb_force_chem->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}
