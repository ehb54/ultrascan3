#include "../include/us_hydrodyn_results.h"
#define DOTSOMO      ""
#define DOTSOMOCAP   ""

US_Hydrodyn_Results::US_Hydrodyn_Results(struct hydro_results *results,
                                         bool *result_widget, QWidget *p, const char *name) : QFrame(p, name)
{
   this->results = results;
   this->result_widget = result_widget;
   *result_widget = true;
   USglobal=new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("SOMO Hydrodynamic Results"));
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
   somo_dir = USglobal->config_list.root_dir + "/somo";
}

US_Hydrodyn_Results::~US_Hydrodyn_Results()
{
   *result_widget = false;
}

void US_Hydrodyn_Results::setupGUI()
{
   int minHeight1 = 70;
   int minHeight2 = 30;
   QString str;

   lbl_info = new QLabel(tr(QString("SOMO Hydrodynamic Results\n"
                                    "(%1 at %2ºC)\n(Density %3 cP, Viscosity %4 g/ml)")
                            .arg(results->solvent_name)
                            .arg(results->temperature)
                            .arg(results->solvent_viscosity)
                            .arg(results->solvent_density)
                            ), this);
   Q_CHECK_PTR(lbl_info);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(AlignCenter|AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_name = new QLabel(tr(" Model: "), this);
   Q_CHECK_PTR(lbl_name);
   lbl_name->setAlignment(AlignLeft|AlignVCenter);
   lbl_name->setMinimumWidth(200);
   lbl_name->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_name = new QLineEdit(this, "name Line Edit");
   le_name->setText(results->name);
   le_name->setReadOnly(true);
   le_name->setMinimumWidth(200);
   //   le_name->setAlignment(AlignLeft|AlignVCenter);
   le_name->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_total_beads = new QLabel(tr(" Total Beads in Model: "), this);
   Q_CHECK_PTR(lbl_total_beads);
   lbl_total_beads->setAlignment(AlignLeft|AlignVCenter);
   lbl_total_beads->setMinimumWidth(200);
   lbl_total_beads->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_total_beads->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_total_beads = new QLineEdit(this, "total_beads Line Edit");
   if ( fabs(results->total_beads_sd) <= 1e-100 )
   {
      le_total_beads->setText(QString("%1").arg(results->total_beads));
   }
   else
   {
      le_total_beads->setText(QString("").sprintf("%u (%4.2e)", 
                                                  (int)(results->total_beads + .5),
                                                  results->total_beads_sd));
   }

      
   le_total_beads->setReadOnly(true);
   le_total_beads->setMinimumWidth(200);
   le_total_beads->setAlignment(AlignVCenter);
   le_total_beads->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_total_beads->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_used_beads = new QLabel(tr(" Used Beads in Model: "), this);
   Q_CHECK_PTR(lbl_used_beads);
   lbl_used_beads->setAlignment(AlignLeft|AlignVCenter);
   lbl_used_beads->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_used_beads->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_used_beads = new QLineEdit(this, "used_beads Line Edit");
   if ( fabs(results->used_beads_sd) <= 1e-100 )
   {
      le_used_beads->setText(QString("%1").arg(results->used_beads));
   }
   else
   {
      le_used_beads->setText(QString("").sprintf("%u (%4.2e)", 
                                                  (int)(results->used_beads + .5),
                                                  results->used_beads_sd));
   }
   le_used_beads->setReadOnly(true);
   le_used_beads->setAlignment(AlignVCenter);
   le_used_beads->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_used_beads->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_mass = new QLabel(tr(" Molecular Mass: "), this);
   Q_CHECK_PTR(lbl_mass);
   lbl_mass->setAlignment(AlignLeft|AlignVCenter);
   lbl_mass->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_mass->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_mass = new QLineEdit(this, "mass Line Edit");
   le_mass->setText(str.sprintf("%6.4e Da", (*results).mass));
   le_mass->setReadOnly(true);
   le_mass->setAlignment(AlignVCenter);
   le_mass->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_mass->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_vbar = new QLabel(tr(" Part. Specif. Volume: "), this);
   Q_CHECK_PTR(lbl_vbar);
   lbl_vbar->setAlignment(AlignLeft|AlignVCenter);
   lbl_vbar->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_vbar = new QLineEdit(this, "vbar Line Edit");
   le_vbar->setText(str.sprintf("%5.3f cm^3/g", (*results).vbar));
   le_vbar->setReadOnly(true);
   le_vbar->setAlignment(AlignVCenter);
   le_vbar->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_s20w = new QLabel(tr(" Sedimentation coefficient s: "), this);
   Q_CHECK_PTR(lbl_s20w);
   lbl_s20w->setAlignment(AlignLeft|AlignVCenter);
   lbl_s20w->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_s20w->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_s20w = new QLineEdit(this, "s20w Line Edit");
   if (fabs((*results).s20w_sd) <= 1e-100)
   {
      le_s20w->setText(str.sprintf("%4.2e S", (*results).s20w));
   }
   else
   {
      le_s20w->setText(str.sprintf("%4.2e S (%4.2e)", (*results).s20w, (*results).s20w_sd));
   }
   le_s20w->setReadOnly(true);
   le_s20w->setAlignment(AlignVCenter);
   le_s20w->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_s20w->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_D20w = new QLabel(tr(" Tr. diffusion coefficient D: "), this);
   Q_CHECK_PTR(lbl_D20w);
   lbl_D20w->setAlignment(AlignLeft|AlignVCenter);
   lbl_D20w->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_D20w->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_D20w = new QLineEdit(this, "D20w Line Edit");
   if (fabs((*results).D20w_sd) <= 1e-100)
   {
      le_D20w->setText(str.sprintf("%4.2e cm/sec^2", (*results).D20w));
   }
   else
   {
      le_D20w->setText(str.sprintf("%4.2e cm/sec^2 (%4.2e)", (*results).D20w, (*results).D20w_sd));
   }
   le_D20w->setReadOnly(true);
   le_D20w->setAlignment(AlignVCenter);
   le_D20w->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_D20w->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_tau = new QLabel(tr(" Relaxation Time, tau(h): "), this);
   Q_CHECK_PTR(lbl_tau);
   lbl_tau->setAlignment(AlignLeft|AlignVCenter);
   lbl_tau->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_tau->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_tau = new QLineEdit(this, "tau Line Edit");
   if (fabs((*results).tau_sd) <= 1e-100)
   {
      le_tau->setText(str.sprintf("%4.2e ns", (*results).tau));
   }
   else
   {
      le_tau->setText(str.sprintf("%4.2e ns (%4.2e)", (*results).tau, (*results).tau_sd));
   }
   le_tau->setReadOnly(true);
   le_tau->setAlignment(AlignVCenter);
   le_tau->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_tau->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_viscosity = new QLabel(tr(" Intrinsic Viscosity: "), this);
   Q_CHECK_PTR(lbl_viscosity);
   lbl_viscosity->setAlignment(AlignLeft|AlignVCenter);
   lbl_viscosity->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_viscosity->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_viscosity = new QLineEdit(this, "viscosity Line Edit");
   if (fabs((*results).viscosity_sd) <= 1e-100)
   {
      le_viscosity->setText(str.sprintf("%4.2e cm^3/g", (*results).viscosity));
   }
   else
   {
      le_viscosity->setText(str.sprintf("%4.2e cm^3/g (%4.2e)", (*results).viscosity, (*results).viscosity_sd));
   }
   le_viscosity->setReadOnly(true);
   le_viscosity->setAlignment(AlignVCenter);
   le_viscosity->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_viscosity->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_rs = new QLabel(tr(" Stokes Radius: "), this);
   Q_CHECK_PTR(lbl_rs);
   lbl_rs->setAlignment(AlignLeft|AlignVCenter);
   lbl_rs->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_rs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_rs = new QLineEdit(this, "rs Line Edit");
   if (fabs((*results).rs_sd) <= 1e-100)
   {
      le_rs->setText(str.sprintf("%4.2e nm", (*results).rs));
   }
   else
   {
      le_rs->setText(str.sprintf("%4.2e nm (%4.2e)", (*results).rs, (*results).rs_sd));
   }
   le_rs->setReadOnly(true);
   le_rs->setAlignment(AlignVCenter);
   le_rs->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_rs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_ff0 = new QLabel(tr(" Frictional Ratio: "), this);
   Q_CHECK_PTR(lbl_ff0);
   lbl_ff0->setAlignment(AlignLeft|AlignVCenter);
   lbl_ff0->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_ff0->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_ff0 = new QLineEdit(this, "ff0 Line Edit");
   if (fabs((*results).ff0_sd) <= 1e-100)
   {
      le_ff0->setText(str.sprintf("%3.2f", (*results).ff0));
   }
   else
   {
      le_ff0->setText(str.sprintf("%3.2f nm (%3.2e)", (*results).ff0, (*results).ff0_sd));
   }
   le_ff0->setReadOnly(true);
   le_ff0->setAlignment(AlignVCenter);
   le_ff0->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_ff0->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_rg = new QLabel(tr(" Radius of Gyration: "), this);
   Q_CHECK_PTR(lbl_rg);
   lbl_rg->setAlignment(AlignLeft|AlignVCenter);
   lbl_rg->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_rg = new QLineEdit(this, "rg Line Edit");
   if (fabs((*results).rg_sd) <= 1e-100)
   {
      le_rg->setText(str.sprintf("%4.2e nm", (*results).rg));
   }
   else
   {
      le_rg->setText(str.sprintf("%4.2e nm (%4.2e)", (*results).rg, (*results).rg_sd));
   }
   le_rg->setReadOnly(true);
   le_rg->setAlignment(AlignVCenter);
   le_rg->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   pb_load_asa = new QPushButton(tr("View ASA Results File"), this);
   Q_CHECK_PTR(pb_load_asa);
   pb_load_asa->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_load_asa->setMinimumHeight(minHeight2);
   pb_load_asa->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_load_asa, SIGNAL(clicked()), SLOT(load_asa()));

   pb_load_results = new QPushButton(tr("View Full Hydrodynamics Results File"), this);
   Q_CHECK_PTR(pb_load_results);
   pb_load_results->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_load_results->setMinimumHeight(minHeight2);
   pb_load_results->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_load_results, SIGNAL(clicked()), SLOT(load_results()));

   pb_load_beadmodel = new QPushButton(tr("View Bead Model File"), this);
   Q_CHECK_PTR(pb_load_beadmodel);
   pb_load_beadmodel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_load_beadmodel->setMinimumHeight(minHeight2);
   pb_load_beadmodel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_load_beadmodel, SIGNAL(clicked()), SLOT(load_beadmodel()));

   pb_cancel = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight2);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight2);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   int rows=13, columns = 2, spacing = 2, j=0, margin=4;
   QGridLayout *background=new QGridLayout(this, rows, columns, margin, spacing);

   background->addMultiCellWidget(lbl_info, j, j, 0, 1);
   j++;
   background->addWidget(lbl_name, j, 0);
   background->addWidget(le_name, j, 1);
   j++;
   background->addWidget(lbl_total_beads, j, 0);
   background->addWidget(le_total_beads, j, 1);
   j++;
   background->addWidget(lbl_used_beads, j, 0);
   background->addWidget(le_used_beads, j, 1);
   j++;
   background->addWidget(lbl_mass, j, 0);
   background->addWidget(le_mass, j, 1);
   j++;
   background->addWidget(lbl_vbar, j, 0);
   background->addWidget(le_vbar, j, 1);
   j++;
   background->addWidget(lbl_s20w, j, 0);
   background->addWidget(le_s20w, j, 1);
   j++;
   background->addWidget(lbl_D20w, j, 0);
   background->addWidget(le_D20w, j, 1);
   j++;
   background->addWidget(lbl_rs, j, 0);
   background->addWidget(le_rs, j, 1);
   j++;
   background->addWidget(lbl_ff0, j, 0);
   background->addWidget(le_ff0, j, 1);
   j++;
   background->addWidget(lbl_rg, j, 0);
   background->addWidget(le_rg, j, 1);
   j++;
   background->addWidget(lbl_tau, j, 0);
   background->addWidget(le_tau, j, 1);
   j++;
   background->addWidget(lbl_viscosity, j, 0);
   background->addWidget(le_viscosity, j, 1);
   j++;
   background->addWidget(pb_load_asa, j, 0);
   background->addWidget(pb_load_beadmodel, j, 1);
   j++;
   background->addMultiCellWidget(pb_load_results, j, j, 0, 1);
   j++;
   background->addWidget(pb_help, j, 0);
   background->addWidget(pb_cancel, j, 1);
}

void US_Hydrodyn_Results::cancel()
{
   close();
}

void US_Hydrodyn_Results::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_results.html");
}

void US_Hydrodyn_Results::load_results()
{
   QString filename = QFileDialog::getOpenFileName(somo_dir, "*.hydro_res *.HYDRO_RES", this);
   if (!filename.isEmpty())
   {
      view_file(filename);
   }
}

void US_Hydrodyn_Results::load_beadmodel()
{
   QString filename = QFileDialog::getOpenFileName(somo_dir, "*.bead_model* *.BEAD_MODEL*", this);
   if (!filename.isEmpty())
   {
      view_file(filename);
   }
}

void US_Hydrodyn_Results::load_asa()
{
   QString filename = QFileDialog::getOpenFileName(somo_dir, "*.asa_res *.ASA_RES", this);
   if (!filename.isEmpty())
   {
      view_file(filename);
   }
}

void US_Hydrodyn_Results::view_file(const QString &filename)
{
   e = new TextEdit();
   e->setFont(QFont("Courier"));
   e->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   e->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
   e->load(filename);
   e->show();
}

void US_Hydrodyn_Results::closeEvent(QCloseEvent *e)
{
   *result_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

