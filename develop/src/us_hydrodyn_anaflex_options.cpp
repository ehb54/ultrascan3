#include "../include/us_hydrodyn_anaflex_options.h"
#include "../include/us_hydrodyn.h"

US_Hydrodyn_Anaflex_Options::US_Hydrodyn_Anaflex_Options(Anaflex_Options *anaflex_options, bool *anaflex_widget, void *us_hydrodyn, QWidget *p, const char *name) : QFrame(p, name)
{
   this->anaflex_options = anaflex_options;
   this->anaflex_widget = anaflex_widget;
   this->us_hydrodyn = us_hydrodyn;
   *anaflex_widget = true;
   USglobal=new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("Anaflex Options"));
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_Anaflex_Options::~US_Hydrodyn_Anaflex_Options()
{
   *anaflex_widget = false;
}

void US_Hydrodyn_Anaflex_Options::setupGUI()
{
   int minHeight1 = 30;

   QColorGroup cg_modes = USglobal->global_colors.cg_label;
   cg_modes.setColor(QColorGroup::Shadow, Qt::gray);
   cg_modes.setColor(QColorGroup::Dark, Qt::gray);
   cg_modes.setColor(QColorGroup::Light, Qt::white);
   cg_modes.setColor(QColorGroup::Midlight, Qt::gray);

   QFont qf_modes = QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold );
   QPalette qp_modes = QPalette( cg_modes, cg_modes, cg_modes );

   lbl_info = new QLabel(tr("Anaflex Options:"), this);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(AlignCenter|AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_run_mode = new QLabel(tr("Run Mode:"), this);
   lbl_run_mode->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_run_mode->setAlignment(AlignCenter|AlignVCenter);
   lbl_run_mode->setMinimumHeight(minHeight1);
   lbl_run_mode->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_run_mode->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   cb_run_anaflex = new QCheckBox(this);
   cb_run_anaflex->setText(tr(" Run Anaflex                  "));
   cb_run_anaflex->setChecked((*anaflex_options).run_anaflex);
   cb_run_anaflex->setEnabled(true);
   //   cb_run_anaflex->setMinimumHeight(minHeight1);
   cb_run_anaflex->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_anaflex->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_anaflex, SIGNAL(clicked()), SLOT(set_run_anaflex()));

   cb_instprofiles = new QCheckBox(this);
   cb_instprofiles->setText(tr(" Store time profiles          "));
   cb_instprofiles->setChecked((*anaflex_options).instprofiles);
   cb_instprofiles->setEnabled(true);
   //   cb_instprofiles->setMinimumHeight(minHeight1);
   cb_instprofiles->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_instprofiles->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_instprofiles, SIGNAL(clicked()), SLOT(set_instprofiles()));

   cb_run_mode_1 = new QCheckBox(this);
   cb_run_mode_1->setText(tr(" Compute steady state properties "));
   cb_run_mode_1->setChecked((*anaflex_options).run_mode_1);
   cb_run_mode_1->setEnabled(true);
   //   cb_run_mode_1->setMinimumHeight(minHeight1);
   cb_run_mode_1->setFont( qf_modes );
   cb_run_mode_1->setPalette( qp_modes );
   connect(cb_run_mode_1, SIGNAL(clicked()), SLOT(set_run_mode_1()));

   cb_run_mode_1_1 = new QCheckBox(this);
   cb_run_mode_1_1->setText(tr(" Linear end-to-end square distance "));
   cb_run_mode_1_1->setChecked((*anaflex_options).run_mode_1_1);
   cb_run_mode_1_1->setEnabled(true);
   cb_run_mode_1_1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_1_1->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_mode_1_1, SIGNAL(clicked()), SLOT(set_run_mode_1_1()));

   cb_run_mode_1_2 = new QCheckBox(this);
   cb_run_mode_1_2->setText(tr(" Square radius of gyration "));
   cb_run_mode_1_2->setChecked((*anaflex_options).run_mode_1_2);
   cb_run_mode_1_2->setEnabled(true);
   cb_run_mode_1_2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_1_2->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_mode_1_2, SIGNAL(clicked()), SLOT(set_run_mode_1_2()));

   cb_run_mode_1_3 = new QCheckBox(this);
   cb_run_mode_1_3->setText(tr(" Gzz components of gyration tensor "));
   cb_run_mode_1_3->setChecked((*anaflex_options).run_mode_1_3);
   cb_run_mode_1_3->setEnabled(true);
   cb_run_mode_1_3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_1_3->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_mode_1_3, SIGNAL(clicked()), SLOT(set_run_mode_1_3()));

   cb_run_mode_1_4 = new QCheckBox(this);
   cb_run_mode_1_4->setText(tr(" Gxx, Gxy components of gyration tensor "));
   cb_run_mode_1_4->setChecked((*anaflex_options).run_mode_1_4);
   cb_run_mode_1_4->setEnabled(true);
   cb_run_mode_1_4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_1_4->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_mode_1_4, SIGNAL(clicked()), SLOT(set_run_mode_1_4()));

   cb_run_mode_1_5 = new QCheckBox(this);
   cb_run_mode_1_5->setText(tr(" Gxy, Gxz, Gyz components of gyration tensor "));
   cb_run_mode_1_5->setChecked((*anaflex_options).run_mode_1_5);
   cb_run_mode_1_5->setEnabled(true);
   cb_run_mode_1_5->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_1_5->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_mode_1_5, SIGNAL(clicked()), SLOT(set_run_mode_1_5()));

   cb_run_mode_1_7 = new QCheckBox(this);
   cb_run_mode_1_7->setText(tr(" Extension along z, and it's square "));
   cb_run_mode_1_7->setChecked((*anaflex_options).run_mode_1_7);
   cb_run_mode_1_7->setEnabled(true);
   cb_run_mode_1_7->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_1_7->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_mode_1_7, SIGNAL(clicked()), SLOT(set_run_mode_1_7()));

   cb_run_mode_1_8 = new QCheckBox(this);
   cb_run_mode_1_8->setText(tr(" Extension along x and y, and thier squares "));
   cb_run_mode_1_8->setChecked((*anaflex_options).run_mode_1_8);
   cb_run_mode_1_8->setEnabled(true);
   cb_run_mode_1_8->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_1_8->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_mode_1_8, SIGNAL(clicked()), SLOT(set_run_mode_1_8()));

   cb_run_mode_1_12 = new QCheckBox(this);
   cb_run_mode_1_12->setText(tr(" Birefringence in FENE chains (soft springs) "));
   cb_run_mode_1_12->setChecked((*anaflex_options).run_mode_1_12);
   cb_run_mode_1_12->setEnabled(true);
   cb_run_mode_1_12->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_1_12->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_mode_1_12, SIGNAL(clicked()), SLOT(set_run_mode_1_12()));

   cb_run_mode_1_13 = new QCheckBox(this);
   cb_run_mode_1_13->setText(tr(" P_2 average over connectors (hard springs) "));
   cb_run_mode_1_13->setChecked((*anaflex_options).run_mode_1_13);
   cb_run_mode_1_13->setEnabled(true);
   cb_run_mode_1_13->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_1_13->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_mode_1_13, SIGNAL(clicked()), SLOT(set_run_mode_1_13()));

   cb_run_mode_1_14 = new QCheckBox(this);
   cb_run_mode_1_14->setText(tr(" Components of stress tensor "));
   cb_run_mode_1_14->setChecked((*anaflex_options).run_mode_1_14);
   cb_run_mode_1_14->setEnabled(true);
   cb_run_mode_1_14->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_1_14->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_mode_1_14, SIGNAL(clicked()), SLOT(set_run_mode_1_14()));

   cb_run_mode_1_18 = new QCheckBox(this);
   cb_run_mode_1_18->setText(tr(" Rigid body hydrodynamic properties "));
   cb_run_mode_1_18->setChecked((*anaflex_options).run_mode_1_18);
   cb_run_mode_1_18->setEnabled(true);
   cb_run_mode_1_18->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_1_18->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_mode_1_18, SIGNAL(clicked()), SLOT(set_run_mode_1_18()));

   // cb_run_mode_1_20 = new QCheckBox(this);
   // cb_run_mode_1_20->setText(tr(" Powers of distance Rij between beads I and J "));
   // cb_run_mode_1_20->setChecked((*anaflex_options).run_mode_1_20);
   // cb_run_mode_1_20->setEnabled(true);
   // cb_run_mode_1_20->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   // cb_run_mode_1_20->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   // connect(cb_run_mode_1_20, SIGNAL(clicked()), SLOT(set_run_mode_1_20()));

   // cb_run_mode_1_24 = new QCheckBox(this);
   // cb_run_mode_1_24->setText(tr(" Dihedral (torsion) angle (rad) subtended by beads I, J, K & L "));
   // cb_run_mode_1_24->setChecked((*anaflex_options).run_mode_1_24);
   // cb_run_mode_1_24->setEnabled(true);
   // cb_run_mode_1_24->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   // cb_run_mode_1_24->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   // connect(cb_run_mode_1_24, SIGNAL(clicked()), SLOT(set_run_mode_1_24()));

   cb_run_mode_2 = new QCheckBox(this);
   cb_run_mode_2->setText(tr(" Compute time-dependent properties "));
   cb_run_mode_2->setChecked((*anaflex_options).run_mode_2);
   cb_run_mode_2->setEnabled(true);
   //   cb_run_mode_2->setMinimumHeight(minHeight1);
   cb_run_mode_2->setFont( qf_modes );
   cb_run_mode_2->setPalette( qp_modes );
   connect(cb_run_mode_2, SIGNAL(clicked()), SLOT(set_run_mode_2()));

   cb_run_mode_2_1 = new QCheckBox(this);
   cb_run_mode_2_1->setText(tr(" Linear end-to-end square distance "));
   cb_run_mode_2_1->setChecked((*anaflex_options).run_mode_2_1);
   cb_run_mode_2_1->setEnabled(true);
   cb_run_mode_2_1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_2_1->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_mode_2_1, SIGNAL(clicked()), SLOT(set_run_mode_2_1()));

   cb_run_mode_2_2 = new QCheckBox(this);
   cb_run_mode_2_2->setText(tr(" Square radius of gyration "));
   cb_run_mode_2_2->setChecked((*anaflex_options).run_mode_2_2);
   cb_run_mode_2_2->setEnabled(true);
   cb_run_mode_2_2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_2_2->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_mode_2_2, SIGNAL(clicked()), SLOT(set_run_mode_2_2()));

   cb_run_mode_2_3 = new QCheckBox(this);
   cb_run_mode_2_3->setText(tr(" Gzz components of gyration tensor "));
   cb_run_mode_2_3->setChecked((*anaflex_options).run_mode_2_3);
   cb_run_mode_2_3->setEnabled(true);
   cb_run_mode_2_3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_2_3->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_mode_2_3, SIGNAL(clicked()), SLOT(set_run_mode_2_3()));

   cb_run_mode_2_4 = new QCheckBox(this);
   cb_run_mode_2_4->setText(tr(" Gxx, Gxy components of gyration tensor "));
   cb_run_mode_2_4->setChecked((*anaflex_options).run_mode_2_4);
   cb_run_mode_2_4->setEnabled(true);
   cb_run_mode_2_4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_2_4->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_mode_2_4, SIGNAL(clicked()), SLOT(set_run_mode_2_4()));

   cb_run_mode_2_5 = new QCheckBox(this);
   cb_run_mode_2_5->setText(tr(" Gxy, Gxz, Gyz components of gyration tensor "));
   cb_run_mode_2_5->setChecked((*anaflex_options).run_mode_2_5);
   cb_run_mode_2_5->setEnabled(true);
   cb_run_mode_2_5->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_2_5->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_mode_2_5, SIGNAL(clicked()), SLOT(set_run_mode_2_5()));

   cb_run_mode_2_7 = new QCheckBox(this);
   cb_run_mode_2_7->setText(tr(" Extension along z, and it's square "));
   cb_run_mode_2_7->setChecked((*anaflex_options).run_mode_2_7);
   cb_run_mode_2_7->setEnabled(true);
   cb_run_mode_2_7->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_2_7->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_mode_2_7, SIGNAL(clicked()), SLOT(set_run_mode_2_7()));

   cb_run_mode_2_8 = new QCheckBox(this);
   cb_run_mode_2_8->setText(tr(" Extension along x and y, and thier squares "));
   cb_run_mode_2_8->setChecked((*anaflex_options).run_mode_2_8);
   cb_run_mode_2_8->setEnabled(true);
   cb_run_mode_2_8->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_2_8->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_mode_2_8, SIGNAL(clicked()), SLOT(set_run_mode_2_8()));

   cb_run_mode_2_12 = new QCheckBox(this);
   cb_run_mode_2_12->setText(tr(" Birefringence in FENE chains (soft springs) "));
   cb_run_mode_2_12->setChecked((*anaflex_options).run_mode_2_12);
   cb_run_mode_2_12->setEnabled(true);
   cb_run_mode_2_12->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_2_12->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_mode_2_12, SIGNAL(clicked()), SLOT(set_run_mode_2_12()));

   cb_run_mode_2_13 = new QCheckBox(this);
   cb_run_mode_2_13->setText(tr(" P_2 average over connectors (hard springs) "));
   cb_run_mode_2_13->setChecked((*anaflex_options).run_mode_2_13);
   cb_run_mode_2_13->setEnabled(true);
   cb_run_mode_2_13->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_2_13->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_mode_2_13, SIGNAL(clicked()), SLOT(set_run_mode_2_13()));

   cb_run_mode_2_14 = new QCheckBox(this);
   cb_run_mode_2_14->setText(tr(" Components of stress tensor "));
   cb_run_mode_2_14->setChecked((*anaflex_options).run_mode_2_14);
   cb_run_mode_2_14->setEnabled(true);
   cb_run_mode_2_14->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_2_14->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_mode_2_14, SIGNAL(clicked()), SLOT(set_run_mode_2_14()));

   cb_run_mode_2_18 = new QCheckBox(this);
   cb_run_mode_2_18->setText(tr(" Rigid body hydrodynamic properties "));
   cb_run_mode_2_18->setChecked((*anaflex_options).run_mode_2_18);
   cb_run_mode_2_18->setEnabled(true);
   cb_run_mode_2_18->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_2_18->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_mode_2_18, SIGNAL(clicked()), SLOT(set_run_mode_2_18()));

   // cb_run_mode_2_20 = new QCheckBox(this);
   // cb_run_mode_2_20->setText(tr(" Powers of distance Rij between beads I and J "));
   // cb_run_mode_2_20->setChecked((*anaflex_options).run_mode_2_20);
   // cb_run_mode_2_20->setEnabled(true);
   // cb_run_mode_2_20->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   // cb_run_mode_2_20->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   // connect(cb_run_mode_2_20, SIGNAL(clicked()), SLOT(set_run_mode_2_20()));

   // cb_run_mode_2_24 = new QCheckBox(this);
   // cb_run_mode_2_24->setText(tr(" Dihedral (torsion) angle (rad) subtended by beads I, J, K & L "));
   // cb_run_mode_2_24->setChecked((*anaflex_options).run_mode_2_24);
   // cb_run_mode_2_24->setEnabled(true);
   // cb_run_mode_2_24->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   // cb_run_mode_2_24->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   // connect(cb_run_mode_2_24, SIGNAL(clicked()), SLOT(set_run_mode_2_24()));

   cb_run_mode_3 = new QCheckBox(this);
   cb_run_mode_3->setText(tr(" Compute correlation function "));
   cb_run_mode_3->setChecked((*anaflex_options).run_mode_3);
   cb_run_mode_3->setEnabled(true);
   //   cb_run_mode_3->setMinimumHeight(minHeight1);
   cb_run_mode_3->setFont( qf_modes );
   cb_run_mode_3->setPalette( qp_modes );
   connect(cb_run_mode_3, SIGNAL(clicked()), SLOT(set_run_mode_3()));

   cb_run_mode_3_1 = new QCheckBox(this);
   cb_run_mode_3_1->setText(tr(" Overall translational diffusion, Dt "));
   cb_run_mode_3_1->setChecked((*anaflex_options).run_mode_3_1);
   cb_run_mode_3_1->setEnabled(true);
   //   cb_run_mode_3_1->setMinimumHeight(minHeight1);
   cb_run_mode_3_1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_3_1->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_mode_3_1, SIGNAL(clicked()), SLOT(set_run_mode_3_1()));

   // cb_run_mode_3_5 = new QCheckBox(this);
   // cb_run_mode_3_5->setText(tr(" < P2 cos( theta ) > "));
   // cb_run_mode_3_5->setChecked((*anaflex_options).run_mode_3_5);
   // cb_run_mode_3_5->setEnabled(true);
   // cb_run_mode_3_5->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   // cb_run_mode_3_5->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   // connect(cb_run_mode_3_5, SIGNAL(clicked()), SLOT(set_run_mode_3_5()));

   cb_run_mode_3_9 = new QCheckBox(this);
   cb_run_mode_3_9->setText(tr(" Depolarized dynamic light scattering "));
   cb_run_mode_3_9->setChecked((*anaflex_options).run_mode_3_9);
   cb_run_mode_3_9->setEnabled(true);
   cb_run_mode_3_9->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_3_9->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_mode_3_9, SIGNAL(clicked()), SLOT(set_run_mode_3_9()));

   // cb_run_mode_3_10 = new QCheckBox(this);
   // cb_run_mode_3_10->setText(tr(" Polarized dynamic light scattering "));
   // cb_run_mode_3_10->setChecked((*anaflex_options).run_mode_3_10);
   // cb_run_mode_3_10->setEnabled(true);
   // cb_run_mode_3_10->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   // cb_run_mode_3_10->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   // connect(cb_run_mode_3_10, SIGNAL(clicked()), SLOT(set_run_mode_3_10()));

   // cb_run_mode_3_14 = new QCheckBox(this);
   // cb_run_mode_3_14->setText(tr(" < Rij dot Rij > "));
   // cb_run_mode_3_14->setChecked((*anaflex_options).run_mode_3_14);
   // cb_run_mode_3_14->setEnabled(true);
   // cb_run_mode_3_14->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   // cb_run_mode_3_14->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   // connect(cb_run_mode_3_14, SIGNAL(clicked()), SLOT(set_run_mode_3_14()));
   
   cb_run_mode_3_15 = new QCheckBox(this);
   cb_run_mode_3_15->setText(tr(" First Rouse mode  "));
   cb_run_mode_3_15->setChecked((*anaflex_options).run_mode_3_15);
   cb_run_mode_3_15->setEnabled(true);
   cb_run_mode_3_15->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_3_15->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_mode_3_15, SIGNAL(clicked()), SLOT(set_run_mode_3_15()));

   cb_run_mode_3_16 = new QCheckBox(this);
   cb_run_mode_3_16->setText(tr(" Second Rouse mode "));
   cb_run_mode_3_16->setChecked((*anaflex_options).run_mode_3_16);
   cb_run_mode_3_16->setEnabled(true);
   cb_run_mode_3_16->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_3_16->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_mode_3_16, SIGNAL(clicked()), SLOT(set_run_mode_3_16()));

   cb_run_mode_4 = new QCheckBox(this);
   cb_run_mode_4->setText(tr(" Graphical display "));
   cb_run_mode_4->setChecked((*anaflex_options).run_mode_4);
   cb_run_mode_4->setEnabled(true);
   //   cb_run_mode_4->setMinimumHeight(minHeight1);
   cb_run_mode_4->setFont( qf_modes );
   cb_run_mode_4->setPalette( qp_modes );
   connect(cb_run_mode_4, SIGNAL(clicked()), SLOT(set_run_mode_4()));

   cb_run_mode_4_1 = new QCheckBox(this);
   cb_run_mode_4_1->setText(tr(" PDB format "));
   cb_run_mode_4_1->setChecked((*anaflex_options).run_mode_4_1);
   cb_run_mode_4_1->setEnabled(true);
   //   cb_run_mode_4_1->setMinimumHeight(minHeight1);
   cb_run_mode_4_1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_4_1->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_mode_4_1, SIGNAL(clicked()), SLOT(set_run_mode_4_1()));

   cb_run_mode_4_6 = new QCheckBox(this);
   cb_run_mode_4_6->setText(tr(" Static VRML format "));
   cb_run_mode_4_6->setChecked((*anaflex_options).run_mode_4_6);
   cb_run_mode_4_6->setEnabled(true);
   //   cb_run_mode_4_6->setMinimumHeight(minHeight1);
   cb_run_mode_4_6->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_4_6->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_mode_4_6, SIGNAL(clicked()), SLOT(set_run_mode_4_6()));

   cb_run_mode_4_7 = new QCheckBox(this);
   cb_run_mode_4_7->setText(tr(" Animated VRML format "));
   cb_run_mode_4_7->setChecked((*anaflex_options).run_mode_4_7);
   cb_run_mode_4_7->setEnabled(true);
   //   cb_run_mode_4_7->setMinimumHeight(minHeight1);
   cb_run_mode_4_7->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_4_7->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_mode_4_7, SIGNAL(clicked()), SLOT(set_run_mode_4_7()));

   cb_run_mode_4_8 = new QCheckBox(this);
   cb_run_mode_4_8->setText(tr(" MOVIE format "));
   cb_run_mode_4_8->setChecked((*anaflex_options).run_mode_4_8);
   cb_run_mode_4_8->setEnabled(true);
   //   cb_run_mode_4_8->setMinimumHeight(minHeight1);
   cb_run_mode_4_8->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_mode_4_8->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_mode_4_8, SIGNAL(clicked()), SLOT(set_run_mode_4_8()));

   cb_run_mode_9 = new QCheckBox(this);
   cb_run_mode_9->setText(tr(" Create text file "));
   cb_run_mode_9->setChecked((*anaflex_options).run_mode_9);
   cb_run_mode_9->setEnabled(true);
   //   cb_run_mode_9->setMinimumHeight(minHeight1);
   cb_run_mode_9->setFont( qf_modes );
   cb_run_mode_9->setPalette( qp_modes );
   connect(cb_run_mode_9, SIGNAL(clicked()), SLOT(set_run_mode_9()));

   lbl_nfrec = new QLabel(tr(" Trajectory sampling frequency:         "), this);
   lbl_nfrec->setAlignment(AlignLeft|AlignVCenter);
   lbl_nfrec->setMinimumHeight(minHeight1);
   lbl_nfrec->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_nfrec->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_nfrec = new QLineEdit(this, "Nfrec Line Edit");
   le_nfrec->setText(QString("%1").arg((*anaflex_options).nfrec));
   le_nfrec->setAlignment(AlignVCenter);
   le_nfrec->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_nfrec->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_nfrec->setEnabled(true);
   connect(le_nfrec, SIGNAL(textChanged(const QString &)), SLOT(update_nfrec(const QString &)));

   lbl_ntimc = new QLabel(tr(" Number of points of the\n correlation function: "), this);
   lbl_ntimc->setAlignment(AlignLeft|AlignVCenter);
   lbl_ntimc->setMinimumHeight(minHeight1);
   lbl_ntimc->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_ntimc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_ntimc = new QLineEdit(this, "Ntimc Line Edit");
   le_ntimc->setText(QString("%1").arg((*anaflex_options).ntimc));
   le_ntimc->setAlignment(AlignVCenter);
   le_ntimc->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_ntimc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_ntimc->setEnabled(true);
   le_ntimc->setMinimumWidth(75);
   connect(le_ntimc, SIGNAL(textChanged(const QString &)), SLOT(update_ntimc(const QString &)));

   lbl_tmax = new QLabel(tr(" Maximum time reached in the\n calculation of the correlation function (s): "), this);
   lbl_tmax->setAlignment(AlignLeft|AlignVCenter);
   lbl_tmax->setMinimumHeight(minHeight1);
   lbl_tmax->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_tmax->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_tmax = new QLineEdit(this, "Tmax Line Edit");
   le_tmax->setText(QString("%1").arg((*anaflex_options).tmax));
   le_tmax->setAlignment(AlignVCenter);
   le_tmax->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_tmax->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_tmax->setMinimumWidth(75);
   le_tmax->setEnabled(true);
   connect(le_tmax, SIGNAL(textChanged(const QString &)), SLOT(update_tmax(const QString &)));

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

   update_enables();

   QVBoxLayout *vbl_top = new QVBoxLayout;

   vbl_top->addWidget(lbl_info);
   vbl_top->addSpacing(3);

   //   QHBoxLayout *hbl_opts = new QHBoxLayout;
   //   QVBoxLayout *vbl_opts = new QVBoxLayout;

   //   vbl_opts->addWidget(cb_run_anaflex);
   //   vbl_opts->addWidget(cb_instprofiles);

   // hbl_opts->addLayout(vbl_opts);

   QGridLayout *gl_ana_1 = new QGridLayout;
   int j = 0;
   gl_ana_1->addWidget(cb_run_anaflex, j, 0);
   gl_ana_1->addWidget(cb_instprofiles, j, 1);
   gl_ana_1->addWidget(lbl_nfrec, j, 2);
   gl_ana_1->addWidget(le_nfrec, j, 3); j++;

   //   hbl_opts->addSpacing(3);
   //   hbl_opts->addLayout(gl_ana_1);

   vbl_top->addLayout(gl_ana_1);

   vbl_top->addWidget(lbl_run_mode);

   QVBoxLayout *vbl_1 = new QVBoxLayout;

   vbl_1->addWidget(cb_run_mode_1);
   vbl_1->addWidget(cb_run_mode_1_1);
   vbl_1->addWidget(cb_run_mode_1_2);
   vbl_1->addWidget(cb_run_mode_1_3);
   vbl_1->addWidget(cb_run_mode_1_4);
   vbl_1->addWidget(cb_run_mode_1_5);
   vbl_1->addWidget(cb_run_mode_1_7);
   vbl_1->addWidget(cb_run_mode_1_8);
   vbl_1->addWidget(cb_run_mode_1_12);
   vbl_1->addWidget(cb_run_mode_1_13);
   vbl_1->addWidget(cb_run_mode_1_14);
   vbl_1->addWidget(cb_run_mode_1_18);

   QVBoxLayout *vbl_2 = new QVBoxLayout;

   vbl_2->addWidget(cb_run_mode_2);
   vbl_2->addWidget(cb_run_mode_2_1);
   vbl_2->addWidget(cb_run_mode_2_2);
   vbl_2->addWidget(cb_run_mode_2_3);
   vbl_2->addWidget(cb_run_mode_2_4);
   vbl_2->addWidget(cb_run_mode_2_5);
   vbl_2->addWidget(cb_run_mode_2_7);
   vbl_2->addWidget(cb_run_mode_2_8);
   vbl_2->addWidget(cb_run_mode_2_12);
   vbl_2->addWidget(cb_run_mode_2_13);
   vbl_2->addWidget(cb_run_mode_2_14);
   vbl_2->addWidget(cb_run_mode_2_18);
   
   QVBoxLayout *vbl_3 = new QVBoxLayout;
   vbl_3->addWidget(cb_run_mode_3);
   vbl_3->addSpacing(2);

   QGridLayout *gl_3 = new QGridLayout;
   gl_3->addWidget(lbl_ntimc, 0, 0);
   gl_3->addWidget(le_ntimc, 0, 1);
   gl_3->addWidget(lbl_tmax, 1, 0);
   gl_3->addWidget(le_tmax, 1, 1);

   vbl_3->addLayout(gl_3);
   vbl_3->addSpacing(2);
   vbl_3->addWidget(cb_run_mode_3_1);
   vbl_3->addWidget(cb_run_mode_3_9);
   vbl_3->addWidget(cb_run_mode_3_15);
   vbl_3->addWidget(cb_run_mode_3_16);

   vbl_3->addSpacing(5);

   vbl_3->addWidget(cb_run_mode_4);
   vbl_3->addWidget(cb_run_mode_4_1);
   vbl_3->addWidget(cb_run_mode_4_6);
   vbl_3->addWidget(cb_run_mode_4_7);
   vbl_3->addWidget(cb_run_mode_4_8);

   vbl_3->addSpacing(5);

   vbl_3->addWidget(cb_run_mode_9);

   QHBoxLayout *hbl_modes = new QHBoxLayout;

   hbl_modes->addLayout(vbl_1);
   hbl_modes->addSpacing(3);
   hbl_modes->addLayout(vbl_2);
   hbl_modes->addSpacing(3);
   hbl_modes->addLayout(vbl_3);

   vbl_top->addLayout(hbl_modes);

   QHBoxLayout *hbl_buttons = new QHBoxLayout;

   hbl_buttons->addWidget(pb_help);
   hbl_buttons->addWidget(pb_cancel);
   
   vbl_top->addSpacing(5);
   vbl_top->addLayout(hbl_buttons);

   QHBoxLayout *background = new QHBoxLayout(this);
   background->addLayout(vbl_top);
}

void US_Hydrodyn_Anaflex_Options::cancel()
{
   close();
}

void US_Hydrodyn_Anaflex_Options::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_anaflex_options.html");
}

void US_Hydrodyn_Anaflex_Options::closeEvent(QCloseEvent *e)
{
   *anaflex_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Anaflex_Options::set_run_anaflex()
{
   (*anaflex_options).run_anaflex = cb_run_anaflex->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_instprofiles()
{
   (*anaflex_options).instprofiles = cb_instprofiles->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_1()
{
   (*anaflex_options).run_mode_1 = cb_run_mode_1->isChecked();
   if ( cb_run_mode_1->isChecked() )
   {
      cb_run_mode_2->setChecked(false);
      cb_run_mode_3->setChecked(false);
      cb_run_mode_4->setChecked(false);
      cb_run_mode_9->setChecked(false);
      (*anaflex_options).run_mode_2 = false;
      (*anaflex_options).run_mode_3 = false;
      (*anaflex_options).run_mode_4 = false;
      (*anaflex_options).run_mode_9 = false;
   }
   update_enables();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_1_1()
{
   (*anaflex_options).run_mode_1_1 = cb_run_mode_1_1->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_1_2()
{
   (*anaflex_options).run_mode_1_2 = cb_run_mode_1_2->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_1_3()
{
   (*anaflex_options).run_mode_1_3 = cb_run_mode_1_3->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_1_4()
{
   (*anaflex_options).run_mode_1_4 = cb_run_mode_1_4->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_1_5()
{
   (*anaflex_options).run_mode_1_5 = cb_run_mode_1_5->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_1_7()
{
   (*anaflex_options).run_mode_1_7 = cb_run_mode_1_7->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_1_8()
{
   (*anaflex_options).run_mode_1_8 = cb_run_mode_1_8->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_1_12()
{
   (*anaflex_options).run_mode_1_12 = cb_run_mode_1_12->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_1_13()
{
   (*anaflex_options).run_mode_1_13 = cb_run_mode_1_13->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_1_14()
{
   (*anaflex_options).run_mode_1_14 = cb_run_mode_1_14->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_1_18()
{
   (*anaflex_options).run_mode_1_18 = cb_run_mode_1_18->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

// void US_Hydrodyn_Anaflex_Options::set_run_mode_1_20()
// {
//    (*anaflex_options).run_mode_1_20 = cb_run_mode_1_20->isChecked();
//    ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
// }

// void US_Hydrodyn_Anaflex_Options::set_run_mode_1_24()
// {
//    (*anaflex_options).run_mode_1_24 = cb_run_mode_1_24->isChecked();
//    ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
// }

void US_Hydrodyn_Anaflex_Options::set_run_mode_2()
{
   (*anaflex_options).run_mode_2 = cb_run_mode_2->isChecked();
   if ( cb_run_mode_2->isChecked() )
   {
      cb_run_mode_1->setChecked(false);
      cb_run_mode_3->setChecked(false);
      cb_run_mode_4->setChecked(false);
      cb_run_mode_9->setChecked(false);
      (*anaflex_options).run_mode_1 = false;
      (*anaflex_options).run_mode_3 = false;
      (*anaflex_options).run_mode_4 = false;
      (*anaflex_options).run_mode_9 = false;
   }
   update_enables();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_2_1()
{
   (*anaflex_options).run_mode_2_1 = cb_run_mode_2_1->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_2_2()
{
   (*anaflex_options).run_mode_2_2 = cb_run_mode_2_2->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_2_3()
{
   (*anaflex_options).run_mode_2_3 = cb_run_mode_2_3->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_2_4()
{
   (*anaflex_options).run_mode_2_4 = cb_run_mode_2_4->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_2_5()
{
   (*anaflex_options).run_mode_2_5 = cb_run_mode_2_5->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_2_7()
{
   (*anaflex_options).run_mode_2_7 = cb_run_mode_2_7->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_2_8()
{
   (*anaflex_options).run_mode_2_8 = cb_run_mode_2_8->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_2_12()
{
   (*anaflex_options).run_mode_2_12 = cb_run_mode_2_12->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_2_13()
{
   (*anaflex_options).run_mode_2_13 = cb_run_mode_2_13->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_2_14()
{
   (*anaflex_options).run_mode_2_14 = cb_run_mode_2_14->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_2_18()
{
   (*anaflex_options).run_mode_2_18 = cb_run_mode_2_18->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

// void US_Hydrodyn_Anaflex_Options::set_run_mode_2_20()
// {
//    (*anaflex_options).run_mode_2_20 = cb_run_mode_2_20->isChecked();
//    ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
// }

// void US_Hydrodyn_Anaflex_Options::set_run_mode_2_24()
// {
//    (*anaflex_options).run_mode_2_24 = cb_run_mode_2_24->isChecked();
//    ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
// }

void US_Hydrodyn_Anaflex_Options::set_run_mode_3()
{
   (*anaflex_options).run_mode_3 = cb_run_mode_3->isChecked();
   if ( cb_run_mode_3->isChecked() )
   {
      cb_run_mode_1->setChecked(false);
      cb_run_mode_2->setChecked(false);
      cb_run_mode_4->setChecked(false);
      cb_run_mode_9->setChecked(false);
      (*anaflex_options).run_mode_1 = false;
      (*anaflex_options).run_mode_2 = false;
      (*anaflex_options).run_mode_4 = false;
      (*anaflex_options).run_mode_9 = false;
   }
   update_enables();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_3_1()
{
   (*anaflex_options).run_mode_3_1 = cb_run_mode_3_1->isChecked();
   if ( cb_run_mode_3_1->isChecked() )
   {
      //      cb_run_mode_3_1->setChecked(false);
      // cb_run_mode_3_5->setChecked(false);
      cb_run_mode_3_9->setChecked(false);
      // cb_run_mode_3_10->setChecked(false);
      // cb_run_mode_3_14->setChecked(false);
      cb_run_mode_3_15->setChecked(false);
      cb_run_mode_3_16->setChecked(false);
      //      (*anaflex_options).run_mode_3_1 = false;
      // (*anaflex_options).run_mode_3_5 = false;
      (*anaflex_options).run_mode_3_9 = false;
      // (*anaflex_options).run_mode_3_10 = false;
      // (*anaflex_options).run_mode_3_14 = false;
      (*anaflex_options).run_mode_3_15 = false;
      (*anaflex_options).run_mode_3_16 = false;
   }
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

// void US_Hydrodyn_Anaflex_Options::set_run_mode_3_5()
// {
//    (*anaflex_options).run_mode_3_5 = cb_run_mode_3_5->isChecked();
//    if ( cb_run_mode_3_5->isChecked() )
//    {
//       cb_run_mode_3_1->setChecked(false);
//       //      // cb_run_mode_3_5->setChecked(false);
//       cb_run_mode_3_9->setChecked(false);
//       // cb_run_mode_3_10->setChecked(false);
//       // cb_run_mode_3_14->setChecked(false);
//       cb_run_mode_3_15->setChecked(false);
//       cb_run_mode_3_16->setChecked(false);
//       (*anaflex_options).run_mode_3_1 = false;
//       //      // (*anaflex_options).run_mode_3_5 = false;
//       (*anaflex_options).run_mode_3_9 = false;
//       // (*anaflex_options).run_mode_3_10 = false;
//       // (*anaflex_options).run_mode_3_14 = false;
//       (*anaflex_options).run_mode_3_15 = false;
//       (*anaflex_options).run_mode_3_16 = false;
//    }
//    ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
// }

void US_Hydrodyn_Anaflex_Options::set_run_mode_3_9()
{
   (*anaflex_options).run_mode_3_9 = cb_run_mode_3_9->isChecked();
   if ( cb_run_mode_3_9->isChecked() )
   {
      cb_run_mode_3_1->setChecked(false);
      // cb_run_mode_3_5->setChecked(false);
      //      cb_run_mode_3_9->setChecked(false);
      // cb_run_mode_3_10->setChecked(false);
      // cb_run_mode_3_14->setChecked(false);
      cb_run_mode_3_15->setChecked(false);
      cb_run_mode_3_16->setChecked(false);
      (*anaflex_options).run_mode_3_1 = false;
      // (*anaflex_options).run_mode_3_5 = false;
      //      (*anaflex_options).run_mode_3_9 = false;
      // (*anaflex_options).run_mode_3_10 = false;
      // (*anaflex_options).run_mode_3_14 = false;
      (*anaflex_options).run_mode_3_15 = false;
      (*anaflex_options).run_mode_3_16 = false;
   }
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

// void US_Hydrodyn_Anaflex_Options::set_run_mode_3_10()
// {
//    (*anaflex_options).run_mode_3_10 = cb_run_mode_3_10->isChecked();
//    if ( cb_run_mode_3_10->isChecked() )
//    {
//       cb_run_mode_3_1->setChecked(false);
//       // cb_run_mode_3_5->setChecked(false);
//       cb_run_mode_3_9->setChecked(false);
//       //      // cb_run_mode_3_10->setChecked(false);
//       // cb_run_mode_3_14->setChecked(false);
//       cb_run_mode_3_15->setChecked(false);
//       cb_run_mode_3_16->setChecked(false);
//       (*anaflex_options).run_mode_3_1 = false;
//       // (*anaflex_options).run_mode_3_5 = false;
//       (*anaflex_options).run_mode_3_9 = false;
//       //      // (*anaflex_options).run_mode_3_10 = false;
//       // (*anaflex_options).run_mode_3_14 = false;
//       (*anaflex_options).run_mode_3_15 = false;
//       (*anaflex_options).run_mode_3_16 = false;
//    }
//    ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
// }

// void US_Hydrodyn_Anaflex_Options::set_run_mode_3_14()
// {
//    (*anaflex_options).run_mode_3_14 = cb_run_mode_3_14->isChecked();
//    if ( cb_run_mode_3_14->isChecked() )
//    {
//       cb_run_mode_3_1->setChecked(false);
//       // cb_run_mode_3_5->setChecked(false);
//       cb_run_mode_3_9->setChecked(false);
//       // cb_run_mode_3_10->setChecked(false);
//       //      // cb_run_mode_3_14->setChecked(false);
//       cb_run_mode_3_15->setChecked(false);
//       cb_run_mode_3_16->setChecked(false);
//       (*anaflex_options).run_mode_3_1 = false;
//       // (*anaflex_options).run_mode_3_5 = false;
//       (*anaflex_options).run_mode_3_9 = false;
//       // (*anaflex_options).run_mode_3_10 = false;
//       //      // (*anaflex_options).run_mode_3_14 = false;
//       (*anaflex_options).run_mode_3_15 = false;
//       (*anaflex_options).run_mode_3_16 = false;
//    }
//    ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
// }

void US_Hydrodyn_Anaflex_Options::set_run_mode_3_15()
{
   (*anaflex_options).run_mode_3_15 = cb_run_mode_3_15->isChecked();
   if ( cb_run_mode_3_15->isChecked() )
   {
      cb_run_mode_3_1->setChecked(false);
      // cb_run_mode_3_5->setChecked(false);
      cb_run_mode_3_9->setChecked(false);
      // cb_run_mode_3_10->setChecked(false);
      // cb_run_mode_3_14->setChecked(false);
      //      cb_run_mode_3_15->setChecked(false);
      cb_run_mode_3_16->setChecked(false);
      (*anaflex_options).run_mode_3_1 = false;
      // (*anaflex_options).run_mode_3_5 = false;
      (*anaflex_options).run_mode_3_9 = false;
      // (*anaflex_options).run_mode_3_10 = false;
      // (*anaflex_options).run_mode_3_14 = false;
      //      (*anaflex_options).run_mode_3_15 = false;
      (*anaflex_options).run_mode_3_16 = false;
   }
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_3_16()
{
   (*anaflex_options).run_mode_3_16 = cb_run_mode_3_16->isChecked();
   if ( cb_run_mode_3_16->isChecked() )
   {
      cb_run_mode_3_1->setChecked(false);
      // cb_run_mode_3_5->setChecked(false);
      cb_run_mode_3_9->setChecked(false);
      // cb_run_mode_3_10->setChecked(false);
      // cb_run_mode_3_14->setChecked(false);
      cb_run_mode_3_15->setChecked(false);
      //      cb_run_mode_3_16->setChecked(false);
      (*anaflex_options).run_mode_3_1 = false;
      // (*anaflex_options).run_mode_3_5 = false;
      (*anaflex_options).run_mode_3_9 = false;
      // (*anaflex_options).run_mode_3_10 = false;
      // (*anaflex_options).run_mode_3_14 = false;
      (*anaflex_options).run_mode_3_15 = false;
      //      (*anaflex_options).run_mode_3_16 = false;
   }
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_4()
{
   (*anaflex_options).run_mode_4 = cb_run_mode_4->isChecked();
   if ( cb_run_mode_4->isChecked() )
   {
      cb_run_mode_1->setChecked(false);
      cb_run_mode_2->setChecked(false);
      cb_run_mode_3->setChecked(false);
      cb_run_mode_9->setChecked(false);
      (*anaflex_options).run_mode_1 = false;
      (*anaflex_options).run_mode_2 = false;
      (*anaflex_options).run_mode_3 = false;
      (*anaflex_options).run_mode_9 = false;
   }
   update_enables();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_4_1()
{
   (*anaflex_options).run_mode_4_1 = cb_run_mode_4_1->isChecked();
   if ( cb_run_mode_4_1->isChecked() )
   {
      cb_run_mode_4_6->setChecked(false);
      cb_run_mode_4_7->setChecked(false);
      cb_run_mode_4_8->setChecked(false);
      (*anaflex_options).run_mode_4_6 = false;
      (*anaflex_options).run_mode_4_7 = false;
      (*anaflex_options).run_mode_4_8 = false;
   }
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_4_6()
{
   (*anaflex_options).run_mode_4_6 = cb_run_mode_4_6->isChecked();
   if ( cb_run_mode_4_6->isChecked() )
   {
      cb_run_mode_4_1->setChecked(false);
      cb_run_mode_4_7->setChecked(false);
      cb_run_mode_4_8->setChecked(false);
      (*anaflex_options).run_mode_4_1 = false;
      (*anaflex_options).run_mode_4_7 = false;
      (*anaflex_options).run_mode_4_8 = false;
   }
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_4_7()
{
   (*anaflex_options).run_mode_4_7 = cb_run_mode_4_7->isChecked();
   if ( cb_run_mode_4_7->isChecked() )
   {
      cb_run_mode_4_1->setChecked(false);
      cb_run_mode_4_6->setChecked(false);
      cb_run_mode_4_8->setChecked(false);
      (*anaflex_options).run_mode_4_1 = false;
      (*anaflex_options).run_mode_4_6 = false;
      (*anaflex_options).run_mode_4_8 = false;
   }
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_4_8()
{
   (*anaflex_options).run_mode_4_8 = cb_run_mode_4_8->isChecked();
   if ( cb_run_mode_4_8->isChecked() )
   {
      cb_run_mode_4_1->setChecked(false);
      cb_run_mode_4_6->setChecked(false);
      cb_run_mode_4_7->setChecked(false);
      (*anaflex_options).run_mode_4_1 = false;
      (*anaflex_options).run_mode_4_6 = false;
      (*anaflex_options).run_mode_4_7 = false;
   }
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::set_run_mode_9()
{
   (*anaflex_options).run_mode_9 = cb_run_mode_9->isChecked();
   if ( cb_run_mode_9->isChecked() )
   {
      cb_run_mode_1->setChecked(false);
      cb_run_mode_2->setChecked(false);
      cb_run_mode_3->setChecked(false);
      cb_run_mode_4->setChecked(false);
      (*anaflex_options).run_mode_1 = false;
      (*anaflex_options).run_mode_2 = false;
      (*anaflex_options).run_mode_3 = false;
      (*anaflex_options).run_mode_4 = false;
   }
   update_enables();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::update_nfrec(const QString &str)
{
   (*anaflex_options).nfrec = str.toInt();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::update_ntimc(const QString &str)
{
   (*anaflex_options).ntimc = str.toInt();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::update_tmax(const QString &str)
{
   (*anaflex_options).tmax = str.toFloat();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Anaflex_Options::update_enables()
{
   cb_run_mode_1_1->setEnabled(cb_run_mode_1->isChecked());
   cb_run_mode_1_2->setEnabled(cb_run_mode_1->isChecked());
   cb_run_mode_1_3->setEnabled(cb_run_mode_1->isChecked());
   cb_run_mode_1_4->setEnabled(cb_run_mode_1->isChecked());
   cb_run_mode_1_5->setEnabled(cb_run_mode_1->isChecked());
   cb_run_mode_1_7->setEnabled(cb_run_mode_1->isChecked());
   cb_run_mode_1_8->setEnabled(cb_run_mode_1->isChecked());
   cb_run_mode_1_12->setEnabled(cb_run_mode_1->isChecked());
   cb_run_mode_1_13->setEnabled(cb_run_mode_1->isChecked());
   cb_run_mode_1_14->setEnabled(cb_run_mode_1->isChecked());
   cb_run_mode_1_18->setEnabled(cb_run_mode_1->isChecked());
   // cb_run_mode_1_20->setEnabled(cb_run_mode_1->isChecked());
   // cb_run_mode_1_24->setEnabled(cb_run_mode_1->isChecked());

   cb_run_mode_2_1->setEnabled(cb_run_mode_2->isChecked());
   cb_run_mode_2_2->setEnabled(cb_run_mode_2->isChecked());
   cb_run_mode_2_3->setEnabled(cb_run_mode_2->isChecked());
   cb_run_mode_2_4->setEnabled(cb_run_mode_2->isChecked());
   cb_run_mode_2_5->setEnabled(cb_run_mode_2->isChecked());
   cb_run_mode_2_7->setEnabled(cb_run_mode_2->isChecked());
   cb_run_mode_2_8->setEnabled(cb_run_mode_2->isChecked());
   cb_run_mode_2_12->setEnabled(cb_run_mode_2->isChecked());
   cb_run_mode_2_13->setEnabled(cb_run_mode_2->isChecked());
   cb_run_mode_2_14->setEnabled(cb_run_mode_2->isChecked());
   cb_run_mode_2_18->setEnabled(cb_run_mode_2->isChecked());
   // cb_run_mode_2_20->setEnabled(cb_run_mode_2->isChecked());
   // cb_run_mode_2_24->setEnabled(cb_run_mode_2->isChecked());

   cb_run_mode_3_1->setEnabled(cb_run_mode_3->isChecked());
   le_ntimc->setEnabled(cb_run_mode_3->isChecked());
   le_tmax->setEnabled(cb_run_mode_3->isChecked());
   // cb_run_mode_3_5->setEnabled(cb_run_mode_3->isChecked());
   cb_run_mode_3_9->setEnabled(cb_run_mode_3->isChecked());
   // cb_run_mode_3_10->setEnabled(cb_run_mode_3->isChecked());
   // cb_run_mode_3_14->setEnabled(cb_run_mode_3->isChecked());
   cb_run_mode_3_15->setEnabled(cb_run_mode_3->isChecked());
   cb_run_mode_3_16->setEnabled(cb_run_mode_3->isChecked());

   cb_run_mode_4_1->setEnabled(cb_run_mode_4->isChecked());
   cb_run_mode_4_6->setEnabled(cb_run_mode_4->isChecked());
   cb_run_mode_4_7->setEnabled(cb_run_mode_4->isChecked());
   cb_run_mode_4_8->setEnabled(cb_run_mode_4->isChecked());
}
