#include "../include/us_hydrodyn_bd_options.h"
#include "../include/us_hydrodyn.h"

// note: this program uses cout and/or cerr and this should be replaced

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
   QFont qf;
   QString str;

   int minHeight1 = 30;
   lbl_info = new QLabel(tr("BD Options:"), this);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_info_model_creation = new QLabel(tr("Connector Discovery:"), this);
   lbl_info_model_creation->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info_model_creation->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info_model_creation->setMinimumHeight(minHeight1);
   lbl_info_model_creation->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info_model_creation->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_info_simulation_opts = new QLabel(tr("Simulation Parameters:"), this);
   lbl_info_simulation_opts->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info_simulation_opts->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info_simulation_opts->setMinimumHeight(minHeight1);
   lbl_info_simulation_opts->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info_simulation_opts->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_springs = new QLabel(tr("Connector Spring Definitions (spring constant in erg/cm^2, distance in cm):"), this);
   lbl_springs->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_springs->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_springs->setMinimumHeight(minHeight1);
   lbl_springs->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_springs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_bd_threshold_pb_pb = new QLabel(tr(" Threshold PB-PB (A): "), this);
   lbl_bd_threshold_pb_pb->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_bd_threshold_pb_pb->setMinimumHeight(minHeight1);
   lbl_bd_threshold_pb_pb->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_bd_threshold_pb_pb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_bd_threshold_pb_pb= new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_bd_threshold_pb_pb );
   cnt_bd_threshold_pb_pb->setRange(0, 100, 0.1);
   cnt_bd_threshold_pb_pb->setValue((*bd_options).threshold_pb_pb);
   cnt_bd_threshold_pb_pb->setMinimumHeight(minHeight1);
   cnt_bd_threshold_pb_pb->setMinimumWidth(150);
   cnt_bd_threshold_pb_pb->setEnabled(true);
   cnt_bd_threshold_pb_pb->setNumButtons(3);
   cnt_bd_threshold_pb_pb->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_bd_threshold_pb_pb->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_bd_threshold_pb_pb, SIGNAL(valueChanged(double)), SLOT(update_bd_threshold_pb_pb(double)));

   lbl_bd_threshold_pb_sc = new QLabel(tr(" Threshold PB-SC (A): "), this);
   lbl_bd_threshold_pb_sc->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_bd_threshold_pb_sc->setMinimumHeight(minHeight1);
   lbl_bd_threshold_pb_sc->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_bd_threshold_pb_sc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_bd_threshold_pb_sc = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_bd_threshold_pb_sc );
   cnt_bd_threshold_pb_sc->setRange(0, 100, 0.1);
   cnt_bd_threshold_pb_sc->setValue((*bd_options).threshold_pb_sc);
   cnt_bd_threshold_pb_sc->setMinimumHeight(minHeight1);
   cnt_bd_threshold_pb_sc->setMinimumWidth(150);
   cnt_bd_threshold_pb_sc->setEnabled(true);
   cnt_bd_threshold_pb_sc->setNumButtons(3);
   cnt_bd_threshold_pb_sc->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_bd_threshold_pb_sc->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_bd_threshold_pb_sc, SIGNAL(valueChanged(double)), SLOT(update_bd_threshold_pb_sc(double)));

   lbl_bd_threshold_sc_sc = new QLabel(tr(" Threshold SC-SC (A): "), this);
   lbl_bd_threshold_sc_sc->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_bd_threshold_sc_sc->setMinimumHeight(minHeight1);
   lbl_bd_threshold_sc_sc->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_bd_threshold_sc_sc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_bd_threshold_sc_sc= new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_bd_threshold_sc_sc );
   cnt_bd_threshold_sc_sc->setRange(0, 100, 0.1);
   cnt_bd_threshold_sc_sc->setValue((*bd_options).threshold_sc_sc);
   cnt_bd_threshold_sc_sc->setMinimumHeight(minHeight1);
   cnt_bd_threshold_sc_sc->setMinimumWidth(150);
   cnt_bd_threshold_sc_sc->setEnabled(true);
   cnt_bd_threshold_sc_sc->setNumButtons(3);
   cnt_bd_threshold_sc_sc->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_bd_threshold_sc_sc->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_bd_threshold_sc_sc, SIGNAL(valueChanged(double)), SLOT(update_bd_threshold_sc_sc(double)));

   bg_bead_size_type = new QButtonGroup(3, Qt::Vertical, " Bead sizes determined ", this);
   Q_CHECK_PTR(bg_bead_size_type);
   bg_bead_size_type->setExclusive(true);
   bg_bead_size_type->setAlignment(Qt::AlignHCenter);
   bg_bead_size_type->setInsideMargin(3);
   bg_bead_size_type->setInsideSpacing(0);
   connect(bg_bead_size_type, SIGNAL(clicked(int)), this, SLOT(set_bead_size_type(int)));

   cb_bead_size_type_1st = new QCheckBox(bg_bead_size_type);
   cb_bead_size_type_1st->setText(tr(" First model's beads "));
   cb_bead_size_type_1st->setEnabled(true);
   //   cb_bead_size_type_1st->setMinimumHeight(minHeight1);
   cb_bead_size_type_1st->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_bead_size_type_1st->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_bead_size_type_min = new QCheckBox(bg_bead_size_type);
   cb_bead_size_type_min->setText(tr(" Minimum size "));
   cb_bead_size_type_min->setEnabled(true);
   //   cb_bead_size_type_min->setMinimumHeight(minHeight1);
   cb_bead_size_type_min->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_bead_size_type_min->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_bead_size_type_avg = new QCheckBox(bg_bead_size_type);
   cb_bead_size_type_avg->setText(tr(" Average size "));
   cb_bead_size_type_avg->setEnabled(true);
   //   cb_bead_size_type_avg->setMinimumHeight(minHeight1);
   cb_bead_size_type_avg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_bead_size_type_avg->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   bg_bead_size_type->setButton(bd_options->bead_size_type);

   lbl_npadif = new QLabel(tr(" Number of consecutive steps without recalculating: "), this);
   lbl_npadif->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_npadif->setMinimumHeight(minHeight1);
   lbl_npadif->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_npadif->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_npadif = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_npadif );
   cnt_npadif->setRange(1, 999, 1);
   cnt_npadif->setValue((*bd_options).npadif);
   cnt_npadif->setMinimumHeight(minHeight1);
   cnt_npadif->setMinimumWidth(150);
   cnt_npadif->setEnabled(true);
   cnt_npadif->setNumButtons(3);
   cnt_npadif->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_npadif->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_npadif, SIGNAL(valueChanged(double)), SLOT(update_npadif(double)));

   lbl_nmol = new QLabel(tr(" Number of subtrajectories: "), this);
   lbl_nmol->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_nmol->setMinimumHeight(minHeight1);
   lbl_nmol->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_nmol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_nmol = new QLineEdit(this, "Nmol Line Edit");
   le_nmol->setText(str.sprintf("%d",(*bd_options).nmol));
   le_nmol->setAlignment(Qt::AlignVCenter);
   le_nmol->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_nmol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_nmol->setEnabled(true);
   connect(le_nmol, SIGNAL(textChanged(const QString &)), SLOT(update_nmol(const QString &)));

   lbl_nconf = new QLabel(tr(" Number of conformations to store: "), this);
   lbl_nconf->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_nconf->setMinimumHeight(minHeight1);
   lbl_nconf->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_nconf->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_nconf = new QLineEdit(this, "Nconf Line Edit");
   le_nconf->setText(str.sprintf("%d",(*bd_options).nconf));
   le_nconf->setAlignment(Qt::AlignVCenter);
   le_nconf->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_nconf->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_nconf->setEnabled(true);
   connect(le_nconf, SIGNAL(textChanged(const QString &)), SLOT(update_nconf(const QString &)));


   lbl_iseed = new QLabel(tr(" Random seed: "), this);
   lbl_iseed->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_iseed->setMinimumHeight(minHeight1);
   lbl_iseed->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_iseed->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_iseed = new QLineEdit(this, "Iseed Line Edit");
   le_iseed->setText(str.sprintf("%d",(*bd_options).iseed));
   le_iseed->setAlignment(Qt::AlignVCenter);
   le_iseed->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_iseed->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_iseed->setEnabled(true);
   connect(le_iseed, SIGNAL(textChanged(const QString &)), SLOT(update_iseed(const QString &)));

   cb_do_rr = new QCheckBox(this);
   cb_do_rr->setText(tr(" Use radial reduction "));
   cb_do_rr->setChecked((*bd_options).do_rr);
   cb_do_rr->setEnabled(true);
   //   cb_do_rr->setMinimumHeight(minHeight1);
   cb_do_rr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_do_rr->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_do_rr, SIGNAL(clicked()), SLOT(set_do_rr()));

   cb_force_chem = new QCheckBox(this);
   cb_force_chem->setText(tr(" Force chemical bonds as connectors "));
   cb_force_chem->setChecked((*bd_options).force_chem);
   cb_force_chem->setEnabled(true);
   // cb_force_chem->setMinimumHeight(minHeight1);
   cb_force_chem->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_force_chem->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_force_chem, SIGNAL(clicked()), SLOT(set_force_chem()));

   cb_icdm = new QCheckBox(this);
   cb_icdm->setText(tr(" Center of mass reference frame "));
   cb_icdm->setChecked((*bd_options).icdm);
   cb_icdm->setEnabled(true);
   cb_icdm->setMinimumHeight(minHeight1);
   cb_icdm->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_icdm->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_icdm, SIGNAL(clicked()), SLOT(set_icdm()));

   cb_show_pdb = new QCheckBox(this);
   cb_show_pdb->setText(tr(" Show connection PDB "));
   cb_show_pdb->setChecked((*bd_options).show_pdb);
   cb_show_pdb->setEnabled(true);
   //   cb_show_pdb->setMinimumHeight(minHeight1);
   cb_show_pdb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_show_pdb->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_show_pdb, SIGNAL(clicked()), SLOT(set_show_pdb()));

   cb_run_browflex = new QCheckBox(this);
   cb_run_browflex->setText(tr(" Run BrowFlex "));
   cb_run_browflex->setChecked((*bd_options).run_browflex);
   cb_run_browflex->setEnabled(true);
   //   cb_run_browflex->setMinimumHeight(minHeight1);
   cb_run_browflex->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_run_browflex->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_run_browflex, SIGNAL(clicked()), SLOT(set_run_browflex()));

   lbl_tprev = new QLabel(tr(" Previous heating time (s): "), this);
   lbl_tprev->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_tprev->setMinimumHeight(minHeight1);
   lbl_tprev->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_tprev->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_tprev = new QLineEdit(this, "Tprev Line Edit");
   le_tprev->setText(str.sprintf("%4.2g",(*bd_options).tprev));
   le_tprev->setAlignment(Qt::AlignVCenter);
   le_tprev->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_tprev->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_tprev->setEnabled(true);
   connect(le_tprev, SIGNAL(textChanged(const QString &)), SLOT(update_tprev(const QString &)));

   lbl_ttraj = new QLabel(tr(" Total time duration (s): "), this);
   lbl_ttraj->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_ttraj->setMinimumHeight(minHeight1);
   lbl_ttraj->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_ttraj->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_ttraj = new QLineEdit(this, "Ttraj Line Edit");
   le_ttraj->setText(str.sprintf("%4.2g",(*bd_options).ttraj));
   le_ttraj->setAlignment(Qt::AlignVCenter);
   le_ttraj->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_ttraj->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_ttraj->setEnabled(true);
   connect(le_ttraj, SIGNAL(textChanged(const QString &)), SLOT(update_ttraj(const QString &)));

   lbl_deltat = new QLabel(tr(" Duration of each simulation step (s): "), this);
   lbl_deltat->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_deltat->setMinimumHeight(minHeight1);
   lbl_deltat->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_deltat->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_deltat = new QLineEdit(this, "Deltat Line Edit");
   le_deltat->setText(str.sprintf("%4.6g",(*bd_options).deltat));
   le_deltat->setAlignment(Qt::AlignVCenter);
   le_deltat->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_deltat->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_deltat->setEnabled(true);
   connect(le_deltat, SIGNAL(textChanged(const QString &)), SLOT(update_deltat(const QString &)));

   lbl_chem_pb_pb = new QLabel(tr(" Chemical PB-PB: "), this);
   lbl_chem_pb_pb->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
   lbl_chem_pb_pb->setMinimumHeight(minHeight1);
   lbl_chem_pb_pb->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_chem_pb_pb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   lbl_chem_pb_sc = new QLabel(tr(" Chemical PB-SC: "), this);
   lbl_chem_pb_sc->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
   lbl_chem_pb_sc->setMinimumHeight(minHeight1);
   lbl_chem_pb_sc->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_chem_pb_sc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   lbl_chem_sc_sc = new QLabel(tr(" Chemical SC-SC: "), this);
   lbl_chem_sc_sc->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
   lbl_chem_sc_sc->setMinimumHeight(minHeight1);
   lbl_chem_sc_sc->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_chem_sc_sc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   lbl_pb_pb = new QLabel(tr(" PB-PB: "), this);
   lbl_pb_pb->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
   lbl_pb_pb->setMinimumHeight(minHeight1);
   lbl_pb_pb->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_pb_pb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   lbl_pb_sc = new QLabel(tr(" PB-SC: "), this);
   lbl_pb_sc->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
   lbl_pb_sc->setMinimumHeight(minHeight1);
   lbl_pb_sc->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_pb_sc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   lbl_sc_sc = new QLabel(tr(" SC-SC: "), this);
   lbl_sc_sc->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
   lbl_sc_sc->setMinimumHeight(minHeight1);
   lbl_sc_sc->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_sc_sc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cb_compute_chem_pb_pb_force_constant = new QCheckBox(this);
   cb_compute_chem_pb_pb_force_constant->setText(tr(" Compute "));
   cb_compute_chem_pb_pb_force_constant->setChecked((*bd_options).compute_chem_pb_pb_force_constant);
   cb_compute_chem_pb_pb_force_constant->setEnabled(true);
   cb_compute_chem_pb_pb_force_constant->setMinimumHeight(minHeight1);
   cb_compute_chem_pb_pb_force_constant->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_chem_pb_pb_force_constant->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_compute_chem_pb_pb_force_constant, SIGNAL(clicked()), SLOT(set_compute_chem_pb_pb_force_constant()));

   cb_compute_chem_pb_pb_equilibrium_dist = new QCheckBox(this);
   cb_compute_chem_pb_pb_equilibrium_dist->setText(tr(" Compute "));
   cb_compute_chem_pb_pb_equilibrium_dist->setChecked((*bd_options).compute_chem_pb_pb_equilibrium_dist);
   cb_compute_chem_pb_pb_equilibrium_dist->setEnabled(true);
   cb_compute_chem_pb_pb_equilibrium_dist->setMinimumHeight(minHeight1);
   cb_compute_chem_pb_pb_equilibrium_dist->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_chem_pb_pb_equilibrium_dist->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_compute_chem_pb_pb_equilibrium_dist, SIGNAL(clicked()), SLOT(set_compute_chem_pb_pb_equilibrium_dist()));

   cb_compute_chem_pb_pb_max_elong = new QCheckBox(this);
   cb_compute_chem_pb_pb_max_elong->setText(tr(" Compute "));
   cb_compute_chem_pb_pb_max_elong->setChecked((*bd_options).compute_chem_pb_pb_max_elong);
   cb_compute_chem_pb_pb_max_elong->setEnabled(true);
   cb_compute_chem_pb_pb_max_elong->setMinimumHeight(minHeight1);
   cb_compute_chem_pb_pb_max_elong->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_chem_pb_pb_max_elong->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_compute_chem_pb_pb_max_elong, SIGNAL(clicked()), SLOT(set_compute_chem_pb_pb_max_elong()));

   cb_compute_chem_pb_sc_force_constant = new QCheckBox(this);
   cb_compute_chem_pb_sc_force_constant->setText(tr(" Compute "));
   cb_compute_chem_pb_sc_force_constant->setChecked((*bd_options).compute_chem_pb_sc_force_constant);
   cb_compute_chem_pb_sc_force_constant->setEnabled(true);
   cb_compute_chem_pb_sc_force_constant->setMinimumHeight(minHeight1);
   cb_compute_chem_pb_sc_force_constant->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_chem_pb_sc_force_constant->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_compute_chem_pb_sc_force_constant, SIGNAL(clicked()), SLOT(set_compute_chem_pb_sc_force_constant()));

   cb_compute_chem_pb_sc_equilibrium_dist = new QCheckBox(this);
   cb_compute_chem_pb_sc_equilibrium_dist->setText(tr(" Compute "));
   cb_compute_chem_pb_sc_equilibrium_dist->setChecked((*bd_options).compute_chem_pb_sc_equilibrium_dist);
   cb_compute_chem_pb_sc_equilibrium_dist->setEnabled(true);
   cb_compute_chem_pb_sc_equilibrium_dist->setMinimumHeight(minHeight1);
   cb_compute_chem_pb_sc_equilibrium_dist->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_chem_pb_sc_equilibrium_dist->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_compute_chem_pb_sc_equilibrium_dist, SIGNAL(clicked()), SLOT(set_compute_chem_pb_sc_equilibrium_dist()));

   cb_compute_chem_pb_sc_max_elong = new QCheckBox(this);
   cb_compute_chem_pb_sc_max_elong->setText(tr(" Compute "));
   cb_compute_chem_pb_sc_max_elong->setChecked((*bd_options).compute_chem_pb_sc_max_elong);
   cb_compute_chem_pb_sc_max_elong->setEnabled(true);
   cb_compute_chem_pb_sc_max_elong->setMinimumHeight(minHeight1);
   cb_compute_chem_pb_sc_max_elong->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_chem_pb_sc_max_elong->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_compute_chem_pb_sc_max_elong, SIGNAL(clicked()), SLOT(set_compute_chem_pb_sc_max_elong()));

   cb_compute_chem_sc_sc_force_constant = new QCheckBox(this);
   cb_compute_chem_sc_sc_force_constant->setText(tr(" Compute "));
   cb_compute_chem_sc_sc_force_constant->setChecked((*bd_options).compute_chem_sc_sc_force_constant);
   cb_compute_chem_sc_sc_force_constant->setEnabled(true);
   cb_compute_chem_sc_sc_force_constant->setMinimumHeight(minHeight1);
   cb_compute_chem_sc_sc_force_constant->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_chem_sc_sc_force_constant->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_compute_chem_sc_sc_force_constant, SIGNAL(clicked()), SLOT(set_compute_chem_sc_sc_force_constant()));

   cb_compute_chem_sc_sc_equilibrium_dist = new QCheckBox(this);
   cb_compute_chem_sc_sc_equilibrium_dist->setText(tr(" Compute "));
   cb_compute_chem_sc_sc_equilibrium_dist->setChecked((*bd_options).compute_chem_sc_sc_equilibrium_dist);
   cb_compute_chem_sc_sc_equilibrium_dist->setEnabled(true);
   cb_compute_chem_sc_sc_equilibrium_dist->setMinimumHeight(minHeight1);
   cb_compute_chem_sc_sc_equilibrium_dist->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_chem_sc_sc_equilibrium_dist->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_compute_chem_sc_sc_equilibrium_dist, SIGNAL(clicked()), SLOT(set_compute_chem_sc_sc_equilibrium_dist()));

   cb_compute_chem_sc_sc_max_elong = new QCheckBox(this);
   cb_compute_chem_sc_sc_max_elong->setText(tr(" Compute "));
   cb_compute_chem_sc_sc_max_elong->setChecked((*bd_options).compute_chem_sc_sc_max_elong);
   cb_compute_chem_sc_sc_max_elong->setEnabled(true);
   cb_compute_chem_sc_sc_max_elong->setMinimumHeight(minHeight1);
   cb_compute_chem_sc_sc_max_elong->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_chem_sc_sc_max_elong->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_compute_chem_sc_sc_max_elong, SIGNAL(clicked()), SLOT(set_compute_chem_sc_sc_max_elong()));

   cb_compute_pb_pb_force_constant = new QCheckBox(this);
   cb_compute_pb_pb_force_constant->setText(tr(" Compute "));
   cb_compute_pb_pb_force_constant->setChecked((*bd_options).compute_pb_pb_force_constant);
   cb_compute_pb_pb_force_constant->setEnabled(true);
   cb_compute_pb_pb_force_constant->setMinimumHeight(minHeight1);
   cb_compute_pb_pb_force_constant->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_pb_pb_force_constant->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_compute_pb_pb_force_constant, SIGNAL(clicked()), SLOT(set_compute_pb_pb_force_constant()));

   cb_compute_pb_pb_equilibrium_dist = new QCheckBox(this);
   cb_compute_pb_pb_equilibrium_dist->setText(tr(" Compute "));
   cb_compute_pb_pb_equilibrium_dist->setChecked((*bd_options).compute_pb_pb_equilibrium_dist);
   cb_compute_pb_pb_equilibrium_dist->setEnabled(true);
   cb_compute_pb_pb_equilibrium_dist->setMinimumHeight(minHeight1);
   cb_compute_pb_pb_equilibrium_dist->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_pb_pb_equilibrium_dist->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_compute_pb_pb_equilibrium_dist, SIGNAL(clicked()), SLOT(set_compute_pb_pb_equilibrium_dist()));

   cb_compute_pb_pb_max_elong = new QCheckBox(this);
   cb_compute_pb_pb_max_elong->setText(tr(" Compute "));
   cb_compute_pb_pb_max_elong->setChecked((*bd_options).compute_pb_pb_max_elong);
   cb_compute_pb_pb_max_elong->setEnabled(true);
   cb_compute_pb_pb_max_elong->setMinimumHeight(minHeight1);
   cb_compute_pb_pb_max_elong->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_pb_pb_max_elong->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_compute_pb_pb_max_elong, SIGNAL(clicked()), SLOT(set_compute_pb_pb_max_elong()));

   cb_compute_pb_sc_force_constant = new QCheckBox(this);
   cb_compute_pb_sc_force_constant->setText(tr(" Compute "));
   cb_compute_pb_sc_force_constant->setChecked((*bd_options).compute_pb_sc_force_constant);
   cb_compute_pb_sc_force_constant->setEnabled(true);
   cb_compute_pb_sc_force_constant->setMinimumHeight(minHeight1);
   cb_compute_pb_sc_force_constant->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_pb_sc_force_constant->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_compute_pb_sc_force_constant, SIGNAL(clicked()), SLOT(set_compute_pb_sc_force_constant()));

   cb_compute_pb_sc_equilibrium_dist = new QCheckBox(this);
   cb_compute_pb_sc_equilibrium_dist->setText(tr(" Compute "));
   cb_compute_pb_sc_equilibrium_dist->setChecked((*bd_options).compute_pb_sc_equilibrium_dist);
   cb_compute_pb_sc_equilibrium_dist->setEnabled(true);
   cb_compute_pb_sc_equilibrium_dist->setMinimumHeight(minHeight1);
   cb_compute_pb_sc_equilibrium_dist->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_pb_sc_equilibrium_dist->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_compute_pb_sc_equilibrium_dist, SIGNAL(clicked()), SLOT(set_compute_pb_sc_equilibrium_dist()));

   cb_compute_pb_sc_max_elong = new QCheckBox(this);
   cb_compute_pb_sc_max_elong->setText(tr(" Compute "));
   cb_compute_pb_sc_max_elong->setChecked((*bd_options).compute_pb_sc_max_elong);
   cb_compute_pb_sc_max_elong->setEnabled(true);
   cb_compute_pb_sc_max_elong->setMinimumHeight(minHeight1);
   cb_compute_pb_sc_max_elong->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_pb_sc_max_elong->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_compute_pb_sc_max_elong, SIGNAL(clicked()), SLOT(set_compute_pb_sc_max_elong()));

   cb_compute_sc_sc_force_constant = new QCheckBox(this);
   cb_compute_sc_sc_force_constant->setText(tr(" Compute "));
   cb_compute_sc_sc_force_constant->setChecked((*bd_options).compute_sc_sc_force_constant);
   cb_compute_sc_sc_force_constant->setEnabled(true);
   cb_compute_sc_sc_force_constant->setMinimumHeight(minHeight1);
   cb_compute_sc_sc_force_constant->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_sc_sc_force_constant->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_compute_sc_sc_force_constant, SIGNAL(clicked()), SLOT(set_compute_sc_sc_force_constant()));

   cb_compute_sc_sc_equilibrium_dist = new QCheckBox(this);
   cb_compute_sc_sc_equilibrium_dist->setText(tr(" Compute "));
   cb_compute_sc_sc_equilibrium_dist->setChecked((*bd_options).compute_sc_sc_equilibrium_dist);
   cb_compute_sc_sc_equilibrium_dist->setEnabled(true);
   cb_compute_sc_sc_equilibrium_dist->setMinimumHeight(minHeight1);
   cb_compute_sc_sc_equilibrium_dist->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_sc_sc_equilibrium_dist->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_compute_sc_sc_equilibrium_dist, SIGNAL(clicked()), SLOT(set_compute_sc_sc_equilibrium_dist()));

   cb_compute_sc_sc_max_elong = new QCheckBox(this);
   cb_compute_sc_sc_max_elong->setText(tr(" Compute "));
   cb_compute_sc_sc_max_elong->setChecked((*bd_options).compute_sc_sc_max_elong);
   cb_compute_sc_sc_max_elong->setEnabled(true);
   cb_compute_sc_sc_max_elong->setMinimumHeight(minHeight1);
   cb_compute_sc_sc_max_elong->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_sc_sc_max_elong->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_compute_sc_sc_max_elong, SIGNAL(clicked()), SLOT(set_compute_sc_sc_max_elong()));

   bg_inter = new QButtonGroup(3, Qt::Vertical, "Type of simulation algorithm:", this);
   Q_CHECK_PTR(bg_inter);
   bg_inter->setExclusive(true);
   bg_inter->setAlignment(Qt::AlignHCenter);
   bg_inter->setInsideMargin(3);
   bg_inter->setInsideSpacing(0);
   connect(bg_inter, SIGNAL(clicked(int)), this, SLOT(set_inter(int)));

   cb_inter_no_hi = new QCheckBox(bg_inter);
   cb_inter_no_hi->setText(tr(" No hydrodynamic interaction (HI) "));
   cb_inter_no_hi->setEnabled(true);
   //   cb_inter_no_hi->setMinimumHeight(minHeight1);
   cb_inter_no_hi->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_inter_no_hi->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_inter_os = new QCheckBox(bg_inter);
   cb_inter_os->setText(tr(" HI Oseen "));
   cb_inter_os->setEnabled(true);
   //   cb_inter_os->setMinimumHeight(minHeight1);
   cb_inter_os->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_inter_os->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_inter_mos = new QCheckBox(bg_inter);
   cb_inter_mos->setText(tr(" HI modified Oseen "));
   cb_inter_mos->setEnabled(true);
   //   cb_inter_mos->setMinimumHeight(minHeight1);
   cb_inter_mos->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_inter_mos->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   bg_inter->setButton(bd_options->inter);

   bg_iorder = new QButtonGroup(3, Qt::Vertical, "Order of the simulation algorithm:", this);
   Q_CHECK_PTR(bg_iorder);
   bg_iorder->setExclusive(true);
   bg_iorder->setAlignment(Qt::AlignHCenter);
   bg_iorder->setInsideMargin(3);
   bg_iorder->setInsideSpacing(0);
   connect(bg_iorder, SIGNAL(clicked(int)), this, SLOT(set_iorder(int)));

   cb_iorder_em = new QCheckBox(bg_iorder);
   cb_iorder_em->setText(tr(" Ermak-McCammon 1st order "));
   cb_iorder_em->setEnabled(true);
   //   cb_iorder_em->setMinimumHeight(minHeight1);
   cb_iorder_em->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_iorder_em->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_iorder_igt = new QCheckBox(bg_iorder);
   cb_iorder_igt->setText(tr(" Iniesta-Garcia de la Torre predictor-corrector"));
   cb_iorder_igt->setEnabled(true);
   //   cb_iorder_igt->setMinimumHeight(minHeight1);
   cb_iorder_igt->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_iorder_igt->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   bg_iorder->setButton(bd_options->iorder);

   bg_chem_pb_pb_bond_types = new QButtonGroup(4, Qt::Vertical, "Bond type:", this);
   qf = bg_chem_pb_pb_bond_types->font();
   qf.setPointSize(qf.pointSize() - 1);
   bg_chem_pb_pb_bond_types->setFont(qf);
   bg_chem_pb_pb_bond_types->setExclusive(true);
   bg_chem_pb_pb_bond_types->setAlignment(Qt::AlignHCenter);
   bg_chem_pb_pb_bond_types->setInsideMargin(3);
   bg_chem_pb_pb_bond_types->setInsideSpacing(0);
   connect(bg_chem_pb_pb_bond_types, SIGNAL(clicked(int)), this, SLOT(set_chem_pb_pb_bond_types(int)));

   cb_chem_pb_pb_bond_type_fraenkel = new QCheckBox(bg_chem_pb_pb_bond_types);
   cb_chem_pb_pb_bond_type_fraenkel->setText(tr(" Fraenkel (hard Hookean) "));
   cb_chem_pb_pb_bond_type_fraenkel->setEnabled(true);
   //   cb_chem_pb_pb_bond_type_fraenkel->setMinimumHeight(minHeight1);
   cb_chem_pb_pb_bond_type_fraenkel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_chem_pb_pb_bond_type_fraenkel->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   pb_dup_fraenkel = new QPushButton(tr("Replicate"), this);
   pb_dup_fraenkel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_dup_fraenkel->setMinimumHeight(minHeight1);
   pb_dup_fraenkel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_dup_fraenkel, SIGNAL(clicked()), SLOT(dup_fraenkel()));

   cb_chem_pb_pb_bond_type_hookean = new QCheckBox(bg_chem_pb_pb_bond_types);
   cb_chem_pb_pb_bond_type_hookean->setText(tr(" Hookean,Gaussian (soft) "));
   cb_chem_pb_pb_bond_type_hookean->setEnabled(true);
   //   cb_chem_pb_pb_bond_type_hookean->setMinimumHeight(minHeight1);
   cb_chem_pb_pb_bond_type_hookean->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_chem_pb_pb_bond_type_hookean->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_chem_pb_pb_bond_type_fene = new QCheckBox(bg_chem_pb_pb_bond_types);
   cb_chem_pb_pb_bond_type_fene->setText(tr(" FENE "));
   cb_chem_pb_pb_bond_type_fene->setEnabled(true);
   //   cb_chem_pb_pb_bond_type_fene->setMinimumHeight(minHeight1);
   cb_chem_pb_pb_bond_type_fene->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_chem_pb_pb_bond_type_fene->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_chem_pb_pb_bond_type_hard_fene = new QCheckBox(bg_chem_pb_pb_bond_types);
   cb_chem_pb_pb_bond_type_hard_fene->setText(tr(" Hard-FENE "));
   cb_chem_pb_pb_bond_type_hard_fene->setEnabled(true);
   //   cb_chem_pb_pb_bond_type_hard_fene->setMinimumHeight(minHeight1);
   cb_chem_pb_pb_bond_type_hard_fene->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_chem_pb_pb_bond_type_hard_fene->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   bg_chem_pb_pb_bond_types->setButton(bd_options->chem_pb_pb_bond_type);

   bg_chem_pb_sc_bond_types = new QButtonGroup(4, Qt::Vertical, "Bond type:", this);
   qf = bg_chem_pb_sc_bond_types->font();
   qf.setPointSize(qf.pointSize() - 1);
   bg_chem_pb_sc_bond_types->setFont(qf);
   bg_chem_pb_sc_bond_types->setExclusive(true);
   bg_chem_pb_sc_bond_types->setAlignment(Qt::AlignHCenter);
   bg_chem_pb_sc_bond_types->setInsideMargin(3);
   bg_chem_pb_sc_bond_types->setInsideSpacing(0);
   connect(bg_chem_pb_sc_bond_types, SIGNAL(clicked(int)), this, SLOT(set_chem_pb_sc_bond_types(int)));

   cb_chem_pb_sc_bond_type_fraenkel = new QCheckBox(bg_chem_pb_sc_bond_types);
   cb_chem_pb_sc_bond_type_fraenkel->setText(tr(" Fraenkel (hard Hookean) "));
   cb_chem_pb_sc_bond_type_fraenkel->setEnabled(true);
   //   cb_chem_pb_sc_bond_type_fraenkel->setMinimumHeight(minHeight1);
   cb_chem_pb_sc_bond_type_fraenkel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_chem_pb_sc_bond_type_fraenkel->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_chem_pb_sc_bond_type_hookean = new QCheckBox(bg_chem_pb_sc_bond_types);
   cb_chem_pb_sc_bond_type_hookean->setText(tr(" Hookean,Gaussian (soft) "));
   cb_chem_pb_sc_bond_type_hookean->setEnabled(true);
   //   cb_chem_pb_sc_bond_type_hookean->setMinimumHeight(minHeight1);
   cb_chem_pb_sc_bond_type_hookean->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_chem_pb_sc_bond_type_hookean->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_chem_pb_sc_bond_type_fene = new QCheckBox(bg_chem_pb_sc_bond_types);
   cb_chem_pb_sc_bond_type_fene->setText(tr(" FENE "));
   cb_chem_pb_sc_bond_type_fene->setEnabled(true);
   //   cb_chem_pb_sc_bond_type_fene->setMinimumHeight(minHeight1);
   cb_chem_pb_sc_bond_type_fene->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_chem_pb_sc_bond_type_fene->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_chem_pb_sc_bond_type_hard_fene = new QCheckBox(bg_chem_pb_sc_bond_types);
   cb_chem_pb_sc_bond_type_hard_fene->setText(tr(" Hard-FENE "));
   cb_chem_pb_sc_bond_type_hard_fene->setEnabled(true);
   //   cb_chem_pb_sc_bond_type_hard_fene->setMinimumHeight(minHeight1);
   cb_chem_pb_sc_bond_type_hard_fene->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_chem_pb_sc_bond_type_hard_fene->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   bg_chem_pb_sc_bond_types->setButton(bd_options->chem_pb_sc_bond_type);

   bg_chem_sc_sc_bond_types = new QButtonGroup(4, Qt::Vertical, "Bond type:", this);
   qf = bg_chem_sc_sc_bond_types->font();
   qf.setPointSize(qf.pointSize() - 1);
   bg_chem_sc_sc_bond_types->setFont(qf);
   bg_chem_sc_sc_bond_types->setExclusive(true);
   bg_chem_sc_sc_bond_types->setAlignment(Qt::AlignHCenter);
   bg_chem_sc_sc_bond_types->setInsideMargin(3);
   bg_chem_sc_sc_bond_types->setInsideSpacing(0);
   connect(bg_chem_sc_sc_bond_types, SIGNAL(clicked(int)), this, SLOT(set_chem_sc_sc_bond_types(int)));

   cb_chem_sc_sc_bond_type_fraenkel = new QCheckBox(bg_chem_sc_sc_bond_types);
   cb_chem_sc_sc_bond_type_fraenkel->setText(tr(" Fraenkel (hard Hookean) "));
   cb_chem_sc_sc_bond_type_fraenkel->setEnabled(true);
   //   cb_chem_sc_sc_bond_type_fraenkel->setMinimumHeight(minHeight1);
   cb_chem_sc_sc_bond_type_fraenkel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_chem_sc_sc_bond_type_fraenkel->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_chem_sc_sc_bond_type_hookean = new QCheckBox(bg_chem_sc_sc_bond_types);
   cb_chem_sc_sc_bond_type_hookean->setText(tr(" Hookean,Gaussian (soft) "));
   cb_chem_sc_sc_bond_type_hookean->setEnabled(true);
   //   cb_chem_sc_sc_bond_type_hookean->setMinimumHeight(minHeight1);
   cb_chem_sc_sc_bond_type_hookean->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_chem_sc_sc_bond_type_hookean->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_chem_sc_sc_bond_type_fene = new QCheckBox(bg_chem_sc_sc_bond_types);
   cb_chem_sc_sc_bond_type_fene->setText(tr(" FENE "));
   cb_chem_sc_sc_bond_type_fene->setEnabled(true);
   //   cb_chem_sc_sc_bond_type_fene->setMinimumHeight(minHeight1);
   cb_chem_sc_sc_bond_type_fene->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_chem_sc_sc_bond_type_fene->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_chem_sc_sc_bond_type_hard_fene = new QCheckBox(bg_chem_sc_sc_bond_types);
   cb_chem_sc_sc_bond_type_hard_fene->setText(tr(" Hard-FENE "));
   cb_chem_sc_sc_bond_type_hard_fene->setEnabled(true);
   //   cb_chem_sc_sc_bond_type_hard_fene->setMinimumHeight(minHeight1);
   cb_chem_sc_sc_bond_type_hard_fene->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_chem_sc_sc_bond_type_hard_fene->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   bg_chem_sc_sc_bond_types->setButton(bd_options->chem_sc_sc_bond_type);

   bg_pb_pb_bond_types = new QButtonGroup(4, Qt::Vertical, "Bond type:", this);
   qf = bg_pb_pb_bond_types->font();
   qf.setPointSize(qf.pointSize() - 1);
   bg_pb_pb_bond_types->setFont(qf);
   bg_pb_pb_bond_types->setExclusive(true);
   bg_pb_pb_bond_types->setAlignment(Qt::AlignHCenter);
   bg_pb_pb_bond_types->setInsideMargin(3);
   bg_pb_pb_bond_types->setInsideSpacing(0);
   connect(bg_pb_pb_bond_types, SIGNAL(clicked(int)), this, SLOT(set_pb_pb_bond_types(int)));

   cb_pb_pb_bond_type_fraenkel = new QCheckBox(bg_pb_pb_bond_types);
   cb_pb_pb_bond_type_fraenkel->setText(tr(" Fraenkel (hard Hookean) "));
   cb_pb_pb_bond_type_fraenkel->setEnabled(true);
   //   cb_pb_pb_bond_type_fraenkel->setMinimumHeight(minHeight1);
   cb_pb_pb_bond_type_fraenkel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pb_pb_bond_type_fraenkel->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_pb_pb_bond_type_hookean = new QCheckBox(bg_pb_pb_bond_types);
   cb_pb_pb_bond_type_hookean->setText(tr(" Hookean,Gaussian (soft) "));
   cb_pb_pb_bond_type_hookean->setEnabled(true);
   //   cb_pb_pb_bond_type_hookean->setMinimumHeight(minHeight1);
   cb_pb_pb_bond_type_hookean->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pb_pb_bond_type_hookean->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_pb_pb_bond_type_fene = new QCheckBox(bg_pb_pb_bond_types);
   cb_pb_pb_bond_type_fene->setText(tr(" FENE "));
   cb_pb_pb_bond_type_fene->setEnabled(true);
   //   cb_pb_pb_bond_type_fene->setMinimumHeight(minHeight1);
   cb_pb_pb_bond_type_fene->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pb_pb_bond_type_fene->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_pb_pb_bond_type_hard_fene = new QCheckBox(bg_pb_pb_bond_types);
   cb_pb_pb_bond_type_hard_fene->setText(tr(" Hard-FENE "));
   cb_pb_pb_bond_type_hard_fene->setEnabled(true);
   //   cb_pb_pb_bond_type_hard_fene->setMinimumHeight(minHeight1);
   cb_pb_pb_bond_type_hard_fene->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pb_pb_bond_type_hard_fene->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   bg_pb_pb_bond_types->setButton(bd_options->pb_pb_bond_type);

   bg_pb_sc_bond_types = new QButtonGroup(4, Qt::Vertical, "Bond type:", this);
   qf = bg_pb_sc_bond_types->font();
   qf.setPointSize(qf.pointSize() - 1);
   bg_pb_sc_bond_types->setFont(qf);
   bg_pb_sc_bond_types->setExclusive(true);
   bg_pb_sc_bond_types->setAlignment(Qt::AlignHCenter);
   bg_pb_sc_bond_types->setInsideMargin(3);
   bg_pb_sc_bond_types->setInsideSpacing(0);
   connect(bg_pb_sc_bond_types, SIGNAL(clicked(int)), this, SLOT(set_pb_sc_bond_types(int)));

   cb_pb_sc_bond_type_fraenkel = new QCheckBox(bg_pb_sc_bond_types);
   cb_pb_sc_bond_type_fraenkel->setText(tr(" Fraenkel (hard Hookean) "));
   cb_pb_sc_bond_type_fraenkel->setEnabled(true);
   //   cb_pb_sc_bond_type_fraenkel->setMinimumHeight(minHeight1);
   cb_pb_sc_bond_type_fraenkel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pb_sc_bond_type_fraenkel->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_pb_sc_bond_type_hookean = new QCheckBox(bg_pb_sc_bond_types);
   cb_pb_sc_bond_type_hookean->setText(tr(" Hookean,Gaussian (soft) "));
   cb_pb_sc_bond_type_hookean->setEnabled(true);
   //   cb_pb_sc_bond_type_hookean->setMinimumHeight(minHeight1);
   cb_pb_sc_bond_type_hookean->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pb_sc_bond_type_hookean->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_pb_sc_bond_type_fene = new QCheckBox(bg_pb_sc_bond_types);
   cb_pb_sc_bond_type_fene->setText(tr(" FENE "));
   cb_pb_sc_bond_type_fene->setEnabled(true);
   //   cb_pb_sc_bond_type_fene->setMinimumHeight(minHeight1);
   cb_pb_sc_bond_type_fene->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pb_sc_bond_type_fene->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_pb_sc_bond_type_hard_fene = new QCheckBox(bg_pb_sc_bond_types);
   cb_pb_sc_bond_type_hard_fene->setText(tr(" Hard-FENE "));
   cb_pb_sc_bond_type_hard_fene->setEnabled(true);
   //   cb_pb_sc_bond_type_hard_fene->setMinimumHeight(minHeight1);
   cb_pb_sc_bond_type_hard_fene->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pb_sc_bond_type_hard_fene->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   bg_pb_sc_bond_types->setButton(bd_options->pb_sc_bond_type);

   bg_sc_sc_bond_types = new QButtonGroup(4, Qt::Vertical, "Bond type:", this);
   qf = bg_sc_sc_bond_types->font();
   qf.setPointSize(qf.pointSize() - 1);
   bg_sc_sc_bond_types->setFont(qf);
   bg_sc_sc_bond_types->setExclusive(true);
   bg_sc_sc_bond_types->setAlignment(Qt::AlignHCenter);
   bg_sc_sc_bond_types->setInsideMargin(3);
   bg_sc_sc_bond_types->setInsideSpacing(0);
   connect(bg_sc_sc_bond_types, SIGNAL(clicked(int)), this, SLOT(set_sc_sc_bond_types(int)));

   cb_sc_sc_bond_type_fraenkel = new QCheckBox(bg_sc_sc_bond_types);
   cb_sc_sc_bond_type_fraenkel->setText(tr(" Fraenkel (hard Hookean) "));
   cb_sc_sc_bond_type_fraenkel->setEnabled(true);
   //   cb_sc_sc_bond_type_fraenkel->setMinimumHeight(minHeight1);
   cb_sc_sc_bond_type_fraenkel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_sc_sc_bond_type_fraenkel->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_sc_sc_bond_type_hookean = new QCheckBox(bg_sc_sc_bond_types);
   cb_sc_sc_bond_type_hookean->setText(tr(" Hookean,Gaussian (soft) "));
   cb_sc_sc_bond_type_hookean->setEnabled(true);
   //   cb_sc_sc_bond_type_hookean->setMinimumHeight(minHeight1);
   cb_sc_sc_bond_type_hookean->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_sc_sc_bond_type_hookean->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_sc_sc_bond_type_fene = new QCheckBox(bg_sc_sc_bond_types);
   cb_sc_sc_bond_type_fene->setText(tr(" FENE "));
   cb_sc_sc_bond_type_fene->setEnabled(true);
   //   cb_sc_sc_bond_type_fene->setMinimumHeight(minHeight1);
   cb_sc_sc_bond_type_fene->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_sc_sc_bond_type_fene->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_sc_sc_bond_type_hard_fene = new QCheckBox(bg_sc_sc_bond_types);
   cb_sc_sc_bond_type_hard_fene->setText(tr(" Hard-FENE "));
   cb_sc_sc_bond_type_hard_fene->setEnabled(true);
   //   cb_sc_sc_bond_type_hard_fene->setMinimumHeight(minHeight1);
   cb_sc_sc_bond_type_hard_fene->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_sc_sc_bond_type_hard_fene->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   bg_sc_sc_bond_types->setButton(bd_options->sc_sc_bond_type);

   lbl_chem_pb_pb_force_constant = new QLabel(tr(" Hookean spring constant: "), this);
   lbl_chem_pb_pb_force_constant->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_chem_pb_pb_force_constant->setMinimumHeight(minHeight1);
   lbl_chem_pb_pb_force_constant->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_chem_pb_pb_force_constant->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_chem_pb_pb_force_constant = new QLineEdit(this, "Chem_Pb_Pb_Force_Constant Line Edit");
   le_chem_pb_pb_force_constant->setText(str.sprintf("%4.2f",(*bd_options).chem_pb_pb_force_constant));
   le_chem_pb_pb_force_constant->setAlignment(Qt::AlignVCenter);
   le_chem_pb_pb_force_constant->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_chem_pb_pb_force_constant->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_chem_pb_pb_force_constant->setEnabled(true);
   connect(le_chem_pb_pb_force_constant, SIGNAL(textChanged(const QString &)), SLOT(update_chem_pb_pb_force_constant(const QString &)));

   lbl_chem_pb_pb_equilibrium_dist = new QLabel(tr(" Equilibrium distance: "), this);
   lbl_chem_pb_pb_equilibrium_dist->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_chem_pb_pb_equilibrium_dist->setMinimumHeight(minHeight1);
   lbl_chem_pb_pb_equilibrium_dist->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_chem_pb_pb_equilibrium_dist->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_chem_pb_pb_equilibrium_dist = new QLineEdit(this, "Chem_Pb_Pb_Equilibrium_Dist Line Edit");
   le_chem_pb_pb_equilibrium_dist->setText(str.sprintf("%4.2f",(*bd_options).chem_pb_pb_equilibrium_dist));
   le_chem_pb_pb_equilibrium_dist->setAlignment(Qt::AlignVCenter);
   le_chem_pb_pb_equilibrium_dist->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_chem_pb_pb_equilibrium_dist->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_chem_pb_pb_equilibrium_dist->setEnabled(true);
   connect(le_chem_pb_pb_equilibrium_dist, SIGNAL(textChanged(const QString &)), SLOT(update_chem_pb_pb_equilibrium_dist(const QString &)));

   lbl_chem_pb_pb_max_elong = new QLabel(tr(" Maximum elongation: "), this);
   lbl_chem_pb_pb_max_elong->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_chem_pb_pb_max_elong->setMinimumHeight(minHeight1);
   lbl_chem_pb_pb_max_elong->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_chem_pb_pb_max_elong->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_chem_pb_pb_max_elong = new QLineEdit(this, "Chem_Pb_Pb_Max_Elong Line Edit");
   le_chem_pb_pb_max_elong->setText(str.sprintf("%4.2f",(*bd_options).chem_pb_pb_max_elong));
   le_chem_pb_pb_max_elong->setAlignment(Qt::AlignVCenter);
   le_chem_pb_pb_max_elong->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_chem_pb_pb_max_elong->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_chem_pb_pb_max_elong->setEnabled(true);
   connect(le_chem_pb_pb_max_elong, SIGNAL(textChanged(const QString &)), SLOT(update_chem_pb_pb_max_elong(const QString &)));

   lbl_chem_pb_sc_force_constant = new QLabel(tr(" Hookean spring constant: "), this);
   lbl_chem_pb_sc_force_constant->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_chem_pb_sc_force_constant->setMinimumHeight(minHeight1);
   lbl_chem_pb_sc_force_constant->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_chem_pb_sc_force_constant->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_chem_pb_sc_force_constant = new QLineEdit(this, "Chem_Pb_Sc_Force_Constant Line Edit");
   le_chem_pb_sc_force_constant->setText(str.sprintf("%4.2f",(*bd_options).chem_pb_sc_force_constant));
   le_chem_pb_sc_force_constant->setAlignment(Qt::AlignVCenter);
   le_chem_pb_sc_force_constant->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_chem_pb_sc_force_constant->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_chem_pb_sc_force_constant->setEnabled(true);
   connect(le_chem_pb_sc_force_constant, SIGNAL(textChanged(const QString &)), SLOT(update_chem_pb_sc_force_constant(const QString &)));

   lbl_chem_pb_sc_equilibrium_dist = new QLabel(tr(" Equilibrium distance: "), this);
   lbl_chem_pb_sc_equilibrium_dist->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_chem_pb_sc_equilibrium_dist->setMinimumHeight(minHeight1);
   lbl_chem_pb_sc_equilibrium_dist->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_chem_pb_sc_equilibrium_dist->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_chem_pb_sc_equilibrium_dist = new QLineEdit(this, "Chem_Pb_Sc_Equilibrium_Dist Line Edit");
   le_chem_pb_sc_equilibrium_dist->setText(str.sprintf("%4.2f",(*bd_options).chem_pb_sc_equilibrium_dist));
   le_chem_pb_sc_equilibrium_dist->setAlignment(Qt::AlignVCenter);
   le_chem_pb_sc_equilibrium_dist->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_chem_pb_sc_equilibrium_dist->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_chem_pb_sc_equilibrium_dist->setEnabled(true);
   connect(le_chem_pb_sc_equilibrium_dist, SIGNAL(textChanged(const QString &)), SLOT(update_chem_pb_sc_equilibrium_dist(const QString &)));

   lbl_chem_pb_sc_max_elong = new QLabel(tr(" Maximum elongation: "), this);
   lbl_chem_pb_sc_max_elong->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_chem_pb_sc_max_elong->setMinimumHeight(minHeight1);
   lbl_chem_pb_sc_max_elong->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_chem_pb_sc_max_elong->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_chem_pb_sc_max_elong = new QLineEdit(this, "Chem_Pb_Sc_Max_Elong Line Edit");
   le_chem_pb_sc_max_elong->setText(str.sprintf("%4.2f",(*bd_options).chem_pb_sc_max_elong));
   le_chem_pb_sc_max_elong->setAlignment(Qt::AlignVCenter);
   le_chem_pb_sc_max_elong->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_chem_pb_sc_max_elong->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_chem_pb_sc_max_elong->setEnabled(true);
   connect(le_chem_pb_sc_max_elong, SIGNAL(textChanged(const QString &)), SLOT(update_chem_pb_sc_max_elong(const QString &)));

   lbl_chem_sc_sc_force_constant = new QLabel(tr(" Hookean spring constant: "), this);
   lbl_chem_sc_sc_force_constant->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_chem_sc_sc_force_constant->setMinimumHeight(minHeight1);
   lbl_chem_sc_sc_force_constant->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_chem_sc_sc_force_constant->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_chem_sc_sc_force_constant = new QLineEdit(this, "Chem_Sc_Sc_Force_Constant Line Edit");
   le_chem_sc_sc_force_constant->setText(str.sprintf("%4.2f",(*bd_options).chem_sc_sc_force_constant));
   le_chem_sc_sc_force_constant->setAlignment(Qt::AlignVCenter);
   le_chem_sc_sc_force_constant->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_chem_sc_sc_force_constant->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_chem_sc_sc_force_constant->setEnabled(true);
   connect(le_chem_sc_sc_force_constant, SIGNAL(textChanged(const QString &)), SLOT(update_chem_sc_sc_force_constant(const QString &)));

   lbl_chem_sc_sc_equilibrium_dist = new QLabel(tr(" Equilibrium distance: "), this);
   lbl_chem_sc_sc_equilibrium_dist->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_chem_sc_sc_equilibrium_dist->setMinimumHeight(minHeight1);
   lbl_chem_sc_sc_equilibrium_dist->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_chem_sc_sc_equilibrium_dist->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_chem_sc_sc_equilibrium_dist = new QLineEdit(this, "Chem_Sc_Sc_Equilibrium_Dist Line Edit");
   le_chem_sc_sc_equilibrium_dist->setText(str.sprintf("%4.2f",(*bd_options).chem_sc_sc_equilibrium_dist));
   le_chem_sc_sc_equilibrium_dist->setAlignment(Qt::AlignVCenter);
   le_chem_sc_sc_equilibrium_dist->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_chem_sc_sc_equilibrium_dist->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_chem_sc_sc_equilibrium_dist->setEnabled(true);
   connect(le_chem_sc_sc_equilibrium_dist, SIGNAL(textChanged(const QString &)), SLOT(update_chem_sc_sc_equilibrium_dist(const QString &)));

   lbl_chem_sc_sc_max_elong = new QLabel(tr(" Maximum elongation: "), this);
   lbl_chem_sc_sc_max_elong->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_chem_sc_sc_max_elong->setMinimumHeight(minHeight1);
   lbl_chem_sc_sc_max_elong->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_chem_sc_sc_max_elong->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_chem_sc_sc_max_elong = new QLineEdit(this, "Chem_Sc_Sc_Max_Elong Line Edit");
   le_chem_sc_sc_max_elong->setText(str.sprintf("%4.2f",(*bd_options).chem_sc_sc_max_elong));
   le_chem_sc_sc_max_elong->setAlignment(Qt::AlignVCenter);
   le_chem_sc_sc_max_elong->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_chem_sc_sc_max_elong->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_chem_sc_sc_max_elong->setEnabled(true);
   connect(le_chem_sc_sc_max_elong, SIGNAL(textChanged(const QString &)), SLOT(update_chem_sc_sc_max_elong(const QString &)));

   lbl_pb_pb_force_constant = new QLabel(tr(" Hookean spring constant: "), this);
   lbl_pb_pb_force_constant->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_pb_pb_force_constant->setMinimumHeight(minHeight1);
   lbl_pb_pb_force_constant->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_pb_pb_force_constant->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_pb_pb_force_constant = new QLineEdit(this, "Pb_Pb_Force_Constant Line Edit");
   le_pb_pb_force_constant->setText(str.sprintf("%4.2f",(*bd_options).pb_pb_force_constant));
   le_pb_pb_force_constant->setAlignment(Qt::AlignVCenter);
   le_pb_pb_force_constant->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_pb_pb_force_constant->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_pb_pb_force_constant->setEnabled(true);
   connect(le_pb_pb_force_constant, SIGNAL(textChanged(const QString &)), SLOT(update_pb_pb_force_constant(const QString &)));

   lbl_pb_pb_equilibrium_dist = new QLabel(tr(" Equilibrium distance: "), this);
   lbl_pb_pb_equilibrium_dist->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_pb_pb_equilibrium_dist->setMinimumHeight(minHeight1);
   lbl_pb_pb_equilibrium_dist->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_pb_pb_equilibrium_dist->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_pb_pb_equilibrium_dist = new QLineEdit(this, "Pb_Pb_Equilibrium_Dist Line Edit");
   le_pb_pb_equilibrium_dist->setText(str.sprintf("%4.2f",(*bd_options).pb_pb_equilibrium_dist));
   le_pb_pb_equilibrium_dist->setAlignment(Qt::AlignVCenter);
   le_pb_pb_equilibrium_dist->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_pb_pb_equilibrium_dist->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_pb_pb_equilibrium_dist->setEnabled(true);
   connect(le_pb_pb_equilibrium_dist, SIGNAL(textChanged(const QString &)), SLOT(update_pb_pb_equilibrium_dist(const QString &)));

   lbl_pb_pb_max_elong = new QLabel(tr(" Maximum elongation: "), this);
   lbl_pb_pb_max_elong->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_pb_pb_max_elong->setMinimumHeight(minHeight1);
   lbl_pb_pb_max_elong->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_pb_pb_max_elong->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_pb_pb_max_elong = new QLineEdit(this, "Pb_Pb_Max_Elong Line Edit");
   le_pb_pb_max_elong->setText(str.sprintf("%4.2f",(*bd_options).pb_pb_max_elong));
   le_pb_pb_max_elong->setAlignment(Qt::AlignVCenter);
   le_pb_pb_max_elong->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_pb_pb_max_elong->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_pb_pb_max_elong->setEnabled(true);
   connect(le_pb_pb_max_elong, SIGNAL(textChanged(const QString &)), SLOT(update_pb_pb_max_elong(const QString &)));

   lbl_pb_sc_force_constant = new QLabel(tr(" Hookean spring constant: "), this);
   lbl_pb_sc_force_constant->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_pb_sc_force_constant->setMinimumHeight(minHeight1);
   lbl_pb_sc_force_constant->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_pb_sc_force_constant->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_pb_sc_force_constant = new QLineEdit(this, "Pb_Sc_Force_Constant Line Edit");
   le_pb_sc_force_constant->setText(str.sprintf("%4.2f",(*bd_options).pb_sc_force_constant));
   le_pb_sc_force_constant->setAlignment(Qt::AlignVCenter);
   le_pb_sc_force_constant->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_pb_sc_force_constant->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_pb_sc_force_constant->setEnabled(true);
   connect(le_pb_sc_force_constant, SIGNAL(textChanged(const QString &)), SLOT(update_pb_sc_force_constant(const QString &)));

   lbl_pb_sc_equilibrium_dist = new QLabel(tr(" Equilibrium distance: "), this);
   lbl_pb_sc_equilibrium_dist->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_pb_sc_equilibrium_dist->setMinimumHeight(minHeight1);
   lbl_pb_sc_equilibrium_dist->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_pb_sc_equilibrium_dist->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_pb_sc_equilibrium_dist = new QLineEdit(this, "Pb_Sc_Equilibrium_Dist Line Edit");
   le_pb_sc_equilibrium_dist->setText(str.sprintf("%4.2f",(*bd_options).pb_sc_equilibrium_dist));
   le_pb_sc_equilibrium_dist->setAlignment(Qt::AlignVCenter);
   le_pb_sc_equilibrium_dist->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_pb_sc_equilibrium_dist->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_pb_sc_equilibrium_dist->setEnabled(true);
   connect(le_pb_sc_equilibrium_dist, SIGNAL(textChanged(const QString &)), SLOT(update_pb_sc_equilibrium_dist(const QString &)));

   lbl_pb_sc_max_elong = new QLabel(tr(" Maximum elongation: "), this);
   lbl_pb_sc_max_elong->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_pb_sc_max_elong->setMinimumHeight(minHeight1);
   lbl_pb_sc_max_elong->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_pb_sc_max_elong->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_pb_sc_max_elong = new QLineEdit(this, "Pb_Sc_Max_Elong Line Edit");
   le_pb_sc_max_elong->setText(str.sprintf("%4.2f",(*bd_options).pb_sc_max_elong));
   le_pb_sc_max_elong->setAlignment(Qt::AlignVCenter);
   le_pb_sc_max_elong->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_pb_sc_max_elong->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_pb_sc_max_elong->setEnabled(true);
   connect(le_pb_sc_max_elong, SIGNAL(textChanged(const QString &)), SLOT(update_pb_sc_max_elong(const QString &)));

   lbl_sc_sc_force_constant = new QLabel(tr(" Hookean spring constant: "), this);
   lbl_sc_sc_force_constant->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_sc_sc_force_constant->setMinimumHeight(minHeight1);
   lbl_sc_sc_force_constant->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_sc_sc_force_constant->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_sc_sc_force_constant = new QLineEdit(this, "Sc_Sc_Force_Constant Line Edit");
   le_sc_sc_force_constant->setText(str.sprintf("%4.2f",(*bd_options).sc_sc_force_constant));
   le_sc_sc_force_constant->setAlignment(Qt::AlignVCenter);
   le_sc_sc_force_constant->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_sc_sc_force_constant->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_sc_sc_force_constant->setEnabled(true);
   connect(le_sc_sc_force_constant, SIGNAL(textChanged(const QString &)), SLOT(update_sc_sc_force_constant(const QString &)));

   lbl_sc_sc_equilibrium_dist = new QLabel(tr(" Equilibrium distance: "), this);
   lbl_sc_sc_equilibrium_dist->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_sc_sc_equilibrium_dist->setMinimumHeight(minHeight1);
   lbl_sc_sc_equilibrium_dist->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_sc_sc_equilibrium_dist->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_sc_sc_equilibrium_dist = new QLineEdit(this, "Sc_Sc_Equilibrium_Dist Line Edit");
   le_sc_sc_equilibrium_dist->setText(str.sprintf("%4.2f",(*bd_options).sc_sc_equilibrium_dist));
   le_sc_sc_equilibrium_dist->setAlignment(Qt::AlignVCenter);
   le_sc_sc_equilibrium_dist->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_sc_sc_equilibrium_dist->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_sc_sc_equilibrium_dist->setEnabled(true);
   connect(le_sc_sc_equilibrium_dist, SIGNAL(textChanged(const QString &)), SLOT(update_sc_sc_equilibrium_dist(const QString &)));

   lbl_sc_sc_max_elong = new QLabel(tr(" Maximum elongation: "), this);
   lbl_sc_sc_max_elong->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_sc_sc_max_elong->setMinimumHeight(minHeight1);
   lbl_sc_sc_max_elong->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_sc_sc_max_elong->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_sc_sc_max_elong = new QLineEdit(this, "Sc_Sc_Max_Elong Line Edit");
   le_sc_sc_max_elong->setText(str.sprintf("%4.2f",(*bd_options).sc_sc_max_elong));
   le_sc_sc_max_elong->setAlignment(Qt::AlignVCenter);
   le_sc_sc_max_elong->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_sc_sc_max_elong->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_sc_sc_max_elong->setEnabled(true);
   connect(le_sc_sc_max_elong, SIGNAL(textChanged(const QString &)), SLOT(update_sc_sc_max_elong(const QString &)));

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

   label_font_ok = QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label);
   label_font_warning = QPalette(USglobal->global_colors.cg_label_warn, USglobal->global_colors.cg_label_warn, USglobal->global_colors.cg_label_warn);

   update_enables();
   update_labels();

   QVBoxLayout *vbl_top = new QVBoxLayout;

   vbl_top->addWidget(lbl_info);
   vbl_top->addSpacing(3);

   QHBoxLayout *hbl_method = new QHBoxLayout;

   QGridLayout *gl_thresh = new QGridLayout( 0, 0 );

   int j = 0;
   gl_thresh->addMultiCellWidget(lbl_info_model_creation, j, j, 0, 1); j++;
   gl_thresh->addWidget(lbl_bd_threshold_pb_pb, j, 0);
   gl_thresh->addWidget(cnt_bd_threshold_pb_pb, j, 1); j++;
   gl_thresh->addWidget(lbl_bd_threshold_pb_sc, j, 0);
   gl_thresh->addWidget(cnt_bd_threshold_pb_sc, j, 1); j++;
   gl_thresh->addWidget(lbl_bd_threshold_sc_sc, j, 0);
   gl_thresh->addWidget(cnt_bd_threshold_sc_sc, j, 1); j++;
   gl_thresh->addMultiCellWidget(cb_do_rr, j, j, 0, 1); j++;
   gl_thresh->addMultiCellWidget(cb_force_chem, j, j, 0, 1); j++;
   gl_thresh->addMultiCellWidget(cb_show_pdb, j, j, 0, 1); j++;
   gl_thresh->addMultiCellWidget(cb_run_browflex, j, j, 0, 1); j++;
   gl_thresh->addMultiCellWidget(bg_bead_size_type, j, j+4, 0, 1); j++;

   hbl_method->addLayout(gl_thresh);
   hbl_method->addSpacing(3);

   QVBoxLayout *vbl_method_group = new QVBoxLayout;
   vbl_method_group->addWidget(lbl_info_simulation_opts);

   QHBoxLayout *hbl_method_group = new QHBoxLayout;

   QVBoxLayout *vbl_method = new QVBoxLayout;
   vbl_method->addWidget(bg_inter);
   vbl_method->addSpacing(10);
   vbl_method->addWidget(bg_iorder);
   vbl_method->addSpacing(10);
   vbl_method->addWidget(cb_icdm);

   hbl_method_group->addLayout(vbl_method);
   hbl_method_group->addSpacing(3);

   QGridLayout *gl_simu = new QGridLayout( 0, 0 );
   j = 0;
   gl_simu->addWidget(lbl_nmol, j, 0);
   gl_simu->addWidget(le_nmol, j, 1); j++;
   gl_simu->addWidget(lbl_tprev, j, 0);
   gl_simu->addWidget(le_tprev, j, 1); j++;
   gl_simu->addWidget(lbl_ttraj, j, 0);
   gl_simu->addWidget(le_ttraj, j, 1); j++;
   gl_simu->addWidget(lbl_deltat, j, 0);
   gl_simu->addWidget(le_deltat, j, 1); j++;
   gl_simu->addWidget(lbl_npadif, j, 0);
   gl_simu->addWidget(cnt_npadif, j, 1); j++;
   gl_simu->addWidget(lbl_nconf, j, 0);
   gl_simu->addWidget(le_nconf, j, 1); j++;
   gl_simu->addWidget(lbl_iseed, j, 0);
   gl_simu->addWidget(le_iseed, j, 1); j++;
   //   gl_simu->addMultiCellWidget(cb_icdm, j, j, 0, 1); j++;

   hbl_method_group->addLayout(gl_simu);

   vbl_method_group->addLayout(hbl_method_group);

   hbl_method->addLayout(vbl_method_group);

   vbl_top->addLayout(hbl_method);
   vbl_top->addSpacing(3);
   vbl_top->addWidget(lbl_springs);

   QHBoxLayout *hbl_bonds = new QHBoxLayout;

   QVBoxLayout *vbl_chem_pb_pb = new QVBoxLayout;
   vbl_chem_pb_pb->addWidget(lbl_chem_pb_pb);
   vbl_chem_pb_pb->addWidget(bg_chem_pb_pb_bond_types);
   vbl_chem_pb_pb->addWidget(lbl_chem_pb_pb_force_constant);
   QHBoxLayout *hbl_chem_pb_pb_force_constant = new QHBoxLayout;
   hbl_chem_pb_pb_force_constant->addWidget(cb_compute_chem_pb_pb_force_constant);
   hbl_chem_pb_pb_force_constant->addWidget(pb_dup_fraenkel);
   vbl_chem_pb_pb->addLayout(hbl_chem_pb_pb_force_constant);
   vbl_chem_pb_pb->addWidget(le_chem_pb_pb_force_constant);
   vbl_chem_pb_pb->addWidget(lbl_chem_pb_pb_equilibrium_dist);
   vbl_chem_pb_pb->addWidget(cb_compute_chem_pb_pb_equilibrium_dist);
   vbl_chem_pb_pb->addWidget(le_chem_pb_pb_equilibrium_dist);
   vbl_chem_pb_pb->addWidget(lbl_chem_pb_pb_max_elong);
   vbl_chem_pb_pb->addWidget(cb_compute_chem_pb_pb_max_elong);
   vbl_chem_pb_pb->addWidget(le_chem_pb_pb_max_elong);
   hbl_bonds->addLayout(vbl_chem_pb_pb);
   hbl_bonds->addSpacing(3);

   QVBoxLayout *vbl_chem_pb_sc = new QVBoxLayout;
   vbl_chem_pb_sc->addWidget(lbl_chem_pb_sc);
   vbl_chem_pb_sc->addWidget(bg_chem_pb_sc_bond_types);
   vbl_chem_pb_sc->addWidget(lbl_chem_pb_sc_force_constant);
   vbl_chem_pb_sc->addWidget(cb_compute_chem_pb_sc_force_constant);
   vbl_chem_pb_sc->addWidget(le_chem_pb_sc_force_constant);
   vbl_chem_pb_sc->addWidget(lbl_chem_pb_sc_equilibrium_dist);
   vbl_chem_pb_sc->addWidget(cb_compute_chem_pb_sc_equilibrium_dist);
   vbl_chem_pb_sc->addWidget(le_chem_pb_sc_equilibrium_dist);
   vbl_chem_pb_sc->addWidget(lbl_chem_pb_sc_max_elong);
   vbl_chem_pb_sc->addWidget(cb_compute_chem_pb_sc_max_elong);
   vbl_chem_pb_sc->addWidget(le_chem_pb_sc_max_elong);
   hbl_bonds->addLayout(vbl_chem_pb_sc);
   hbl_bonds->addSpacing(3);

   QVBoxLayout *vbl_chem_sc_sc = new QVBoxLayout;
   vbl_chem_sc_sc->addWidget(lbl_chem_sc_sc);
   vbl_chem_sc_sc->addWidget(bg_chem_sc_sc_bond_types);
   vbl_chem_sc_sc->addWidget(lbl_chem_sc_sc_force_constant);
   vbl_chem_sc_sc->addWidget(cb_compute_chem_sc_sc_force_constant);
   vbl_chem_sc_sc->addWidget(le_chem_sc_sc_force_constant);
   vbl_chem_sc_sc->addWidget(lbl_chem_sc_sc_equilibrium_dist);
   vbl_chem_sc_sc->addWidget(cb_compute_chem_sc_sc_equilibrium_dist);
   vbl_chem_sc_sc->addWidget(le_chem_sc_sc_equilibrium_dist);
   vbl_chem_sc_sc->addWidget(lbl_chem_sc_sc_max_elong);
   vbl_chem_sc_sc->addWidget(cb_compute_chem_sc_sc_max_elong);
   vbl_chem_sc_sc->addWidget(le_chem_sc_sc_max_elong);
   hbl_bonds->addLayout(vbl_chem_sc_sc);
   hbl_bonds->addSpacing(3);

   QVBoxLayout *vbl_pb_pb = new QVBoxLayout;
   vbl_pb_pb->addWidget(lbl_pb_pb);
   vbl_pb_pb->addWidget(bg_pb_pb_bond_types);
   vbl_pb_pb->addWidget(lbl_pb_pb_force_constant);
   vbl_pb_pb->addWidget(cb_compute_pb_pb_force_constant);
   vbl_pb_pb->addWidget(le_pb_pb_force_constant);
   vbl_pb_pb->addWidget(lbl_pb_pb_equilibrium_dist);
   vbl_pb_pb->addWidget(cb_compute_pb_pb_equilibrium_dist);
   vbl_pb_pb->addWidget(le_pb_pb_equilibrium_dist);
   vbl_pb_pb->addWidget(lbl_pb_pb_max_elong);
   vbl_pb_pb->addWidget(cb_compute_pb_pb_max_elong);
   vbl_pb_pb->addWidget(le_pb_pb_max_elong);
   hbl_bonds->addLayout(vbl_pb_pb);
   hbl_bonds->addSpacing(3);

   QVBoxLayout *vbl_pb_sc = new QVBoxLayout;
   vbl_pb_sc->addWidget(lbl_pb_sc);
   vbl_pb_sc->addWidget(bg_pb_sc_bond_types);
   vbl_pb_sc->addWidget(lbl_pb_sc_force_constant);
   vbl_pb_sc->addWidget(cb_compute_pb_sc_force_constant);
   vbl_pb_sc->addWidget(le_pb_sc_force_constant);
   vbl_pb_sc->addWidget(lbl_pb_sc_equilibrium_dist);
   vbl_pb_sc->addWidget(cb_compute_pb_sc_equilibrium_dist);
   vbl_pb_sc->addWidget(le_pb_sc_equilibrium_dist);
   vbl_pb_sc->addWidget(lbl_pb_sc_max_elong);
   vbl_pb_sc->addWidget(cb_compute_pb_sc_max_elong);
   vbl_pb_sc->addWidget(le_pb_sc_max_elong);
   hbl_bonds->addLayout(vbl_pb_sc);
   hbl_bonds->addSpacing(3);

   QVBoxLayout *vbl_sc_sc = new QVBoxLayout;
   vbl_sc_sc->addWidget(lbl_sc_sc);
   vbl_sc_sc->addWidget(bg_sc_sc_bond_types);
   vbl_sc_sc->addWidget(lbl_sc_sc_force_constant);
   vbl_sc_sc->addWidget(cb_compute_sc_sc_force_constant);
   vbl_sc_sc->addWidget(le_sc_sc_force_constant);
   vbl_sc_sc->addWidget(lbl_sc_sc_equilibrium_dist);
   vbl_sc_sc->addWidget(cb_compute_sc_sc_equilibrium_dist);
   vbl_sc_sc->addWidget(le_sc_sc_equilibrium_dist);
   vbl_sc_sc->addWidget(lbl_sc_sc_max_elong);
   vbl_sc_sc->addWidget(cb_compute_sc_sc_max_elong);
   vbl_sc_sc->addWidget(le_sc_sc_max_elong);
   hbl_bonds->addLayout(vbl_sc_sc);

   vbl_top->addSpacing(3);
   vbl_top->addLayout(hbl_bonds);

   QHBoxLayout *hbl_buttons = new QHBoxLayout;

   hbl_buttons->addWidget(pb_help);
   hbl_buttons->addWidget(pb_cancel);
   
   vbl_top->addSpacing(3);
   vbl_top->addLayout(hbl_buttons);

   QHBoxLayout *background = new QHBoxLayout(this);
   background->addLayout(vbl_top);
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

void US_Hydrodyn_BD_Options::update_bd_threshold_pb_pb(double val)
{
   (*bd_options).threshold_pb_pb = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_bd_threshold_pb_sc(double val)
{
   (*bd_options).threshold_pb_sc = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_bd_threshold_sc_sc(double val)
{
   (*bd_options).threshold_sc_sc = (float) val;
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

void US_Hydrodyn_BD_Options::set_icdm()
{
   (*bd_options).icdm = cb_do_rr->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_show_pdb()
{
   (*bd_options).show_pdb = cb_show_pdb->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_run_browflex()
{
   (*bd_options).run_browflex = cb_run_browflex->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_bead_size_type(int val)
{
   (*bd_options).bead_size_type = val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_chem_pb_pb_force_constant()
{
   (*bd_options).compute_chem_pb_pb_force_constant = cb_compute_chem_pb_pb_force_constant->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_chem_pb_pb_equilibrium_dist()
{
   (*bd_options).compute_chem_pb_pb_equilibrium_dist = cb_compute_chem_pb_pb_equilibrium_dist->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_chem_pb_pb_max_elong()
{
   (*bd_options).compute_chem_pb_pb_max_elong = cb_compute_chem_pb_pb_max_elong->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_chem_pb_sc_force_constant()
{
   (*bd_options).compute_chem_pb_sc_force_constant = cb_compute_chem_pb_sc_force_constant->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_chem_pb_sc_equilibrium_dist()
{
   (*bd_options).compute_chem_pb_sc_equilibrium_dist = cb_compute_chem_pb_sc_equilibrium_dist->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_chem_pb_sc_max_elong()
{
   (*bd_options).compute_chem_pb_sc_max_elong = cb_compute_chem_pb_sc_max_elong->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_chem_sc_sc_force_constant()
{
   (*bd_options).compute_chem_sc_sc_force_constant = cb_compute_chem_sc_sc_force_constant->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_chem_sc_sc_equilibrium_dist()
{
   (*bd_options).compute_chem_sc_sc_equilibrium_dist = cb_compute_chem_sc_sc_equilibrium_dist->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_chem_sc_sc_max_elong()
{
   (*bd_options).compute_chem_sc_sc_max_elong = cb_compute_chem_sc_sc_max_elong->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_pb_pb_force_constant()
{
   (*bd_options).compute_pb_pb_force_constant = cb_compute_pb_pb_force_constant->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_pb_pb_equilibrium_dist()
{
   (*bd_options).compute_pb_pb_equilibrium_dist = cb_compute_pb_pb_equilibrium_dist->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_pb_pb_max_elong()
{
   (*bd_options).compute_pb_pb_max_elong = cb_compute_pb_pb_max_elong->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_pb_sc_force_constant()
{
   (*bd_options).compute_pb_sc_force_constant = cb_compute_pb_sc_force_constant->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_pb_sc_equilibrium_dist()
{
   (*bd_options).compute_pb_sc_equilibrium_dist = cb_compute_pb_sc_equilibrium_dist->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_pb_sc_max_elong()
{
   (*bd_options).compute_pb_sc_max_elong = cb_compute_pb_sc_max_elong->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_sc_sc_force_constant()
{
   (*bd_options).compute_sc_sc_force_constant = cb_compute_sc_sc_force_constant->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_sc_sc_equilibrium_dist()
{
   (*bd_options).compute_sc_sc_equilibrium_dist = cb_compute_sc_sc_equilibrium_dist->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_compute_sc_sc_max_elong()
{
   (*bd_options).compute_sc_sc_max_elong = cb_compute_sc_sc_max_elong->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_inter(int val)
{
   (*bd_options).inter = val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_iorder(int val)
{
   (*bd_options).iorder = val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_tprev(const QString &str)
{
   (*bd_options).tprev = str.toFloat();
   //   le_tprev->setText(QString("").sprintf("%4.2f",(*hydro).tprev));
   update_labels();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_ttraj(const QString &str)
{
   (*bd_options).ttraj = str.toFloat();
   //   le_ttraj->setText(QString("").sprintf("%4.2f",(*hydro).ttraj));
   update_labels();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_deltat(const QString &str)
{
   (*bd_options).deltat = str.toFloat();
   //   le_deltat->setText(QString("").sprintf("%4.2f",(*hydro).deltat));
   update_labels();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_npadif(double val)
{
   (*bd_options).npadif = (int) val;
   update_labels();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_nconf(const QString &str)
{
   (*bd_options).nconf = str.toInt();
   //   le_nconf->setText(QString("").sprintf("%4.2f",(*hydro).nconf));
   update_labels();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_nmol(const QString &str)
{
   (*bd_options).nmol = str.toInt();
   //   le_nmol->setText(QString("").sprintf("%4.2f",(*hydro).nmol));
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_iseed(const QString &str)
{
   (*bd_options).iseed = str.toInt();
   //   le_iseed->setText(QString("").sprintf("%4.2f",(*hydro).iseed));
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_chem_pb_pb_bond_types(int val)
{
   (*bd_options).chem_pb_pb_bond_type = val;
   update_enables();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_chem_pb_sc_bond_types(int val)
{
   (*bd_options).chem_pb_sc_bond_type = val;
   update_enables();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_chem_sc_sc_bond_types(int val)
{
   (*bd_options).chem_sc_sc_bond_type = val;
   update_enables();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_pb_pb_bond_types(int val)
{
   (*bd_options).pb_pb_bond_type = val;
   update_enables();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_pb_sc_bond_types(int val)
{
   (*bd_options).pb_sc_bond_type = val;
   update_enables();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::set_sc_sc_bond_types(int val)
{
   (*bd_options).sc_sc_bond_type = val;
   update_enables();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_chem_pb_pb_force_constant(const QString &str)
{
   (*bd_options).chem_pb_pb_force_constant = str.toFloat();
   //   le_chem_pb_pb_force_constant->setText(QString("").sprintf("%4.2f",(*hydro).chem_pb_pb_force_constant));
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}


void US_Hydrodyn_BD_Options::update_chem_pb_pb_equilibrium_dist(const QString &str)
{
   (*bd_options).chem_pb_pb_equilibrium_dist = str.toFloat();
   //   le_chem_pb_pb_equilibrium_dist->setText(QString("").sprintf("%4.2f",(*hydro).chem_pb_pb_equilibrium_dist));
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}


void US_Hydrodyn_BD_Options::update_chem_pb_pb_max_elong(const QString &str)
{
   (*bd_options).chem_pb_pb_max_elong = str.toFloat();
   //   le_chem_pb_pb_max_elong->setText(QString("").sprintf("%4.2f",(*hydro).chem_pb_pb_max_elong));
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}


void US_Hydrodyn_BD_Options::update_chem_pb_sc_force_constant(const QString &str)
{
   (*bd_options).chem_pb_sc_force_constant = str.toFloat();
   //   le_chem_pb_sc_force_constant->setText(QString("").sprintf("%4.2f",(*hydro).chem_pb_sc_force_constant));
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}


void US_Hydrodyn_BD_Options::update_chem_pb_sc_equilibrium_dist(const QString &str)
{
   (*bd_options).chem_pb_sc_equilibrium_dist = str.toFloat();
   //   le_chem_pb_sc_equilibrium_dist->setText(QString("").sprintf("%4.2f",(*hydro).chem_pb_sc_equilibrium_dist));
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}


void US_Hydrodyn_BD_Options::update_chem_pb_sc_max_elong(const QString &str)
{
   (*bd_options).chem_pb_sc_max_elong = str.toFloat();
   //   le_chem_pb_sc_max_elong->setText(QString("").sprintf("%4.2f",(*hydro).chem_pb_sc_max_elong));
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}


void US_Hydrodyn_BD_Options::update_chem_sc_sc_force_constant(const QString &str)
{
   (*bd_options).chem_sc_sc_force_constant = str.toFloat();
   //   le_chem_sc_sc_force_constant->setText(QString("").sprintf("%4.2f",(*hydro).chem_sc_sc_force_constant));
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}


void US_Hydrodyn_BD_Options::update_chem_sc_sc_equilibrium_dist(const QString &str)
{
   (*bd_options).chem_sc_sc_equilibrium_dist = str.toFloat();
   //   le_chem_sc_sc_equilibrium_dist->setText(QString("").sprintf("%4.2f",(*hydro).chem_sc_sc_equilibrium_dist));
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}


void US_Hydrodyn_BD_Options::update_chem_sc_sc_max_elong(const QString &str)
{
   (*bd_options).chem_sc_sc_max_elong = str.toFloat();
   //   le_chem_sc_sc_max_elong->setText(QString("").sprintf("%4.2f",(*hydro).chem_sc_sc_max_elong));
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}


void US_Hydrodyn_BD_Options::update_pb_pb_force_constant(const QString &str)
{
   (*bd_options).pb_pb_force_constant = str.toFloat();
   //   le_pb_pb_force_constant->setText(QString("").sprintf("%4.2f",(*hydro).pb_pb_force_constant));
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}


void US_Hydrodyn_BD_Options::update_pb_pb_equilibrium_dist(const QString &str)
{
   (*bd_options).pb_pb_equilibrium_dist = str.toFloat();
   //   le_pb_pb_equilibrium_dist->setText(QString("").sprintf("%4.2f",(*hydro).pb_pb_equilibrium_dist));
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}


void US_Hydrodyn_BD_Options::update_pb_pb_max_elong(const QString &str)
{
   (*bd_options).pb_pb_max_elong = str.toFloat();
   //   le_pb_pb_max_elong->setText(QString("").sprintf("%4.2f",(*hydro).pb_pb_max_elong));
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}


void US_Hydrodyn_BD_Options::update_pb_sc_force_constant(const QString &str)
{
   (*bd_options).pb_sc_force_constant = str.toFloat();
   //   le_pb_sc_force_constant->setText(QString("").sprintf("%4.2f",(*hydro).pb_sc_force_constant));
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}


void US_Hydrodyn_BD_Options::update_pb_sc_equilibrium_dist(const QString &str)
{
   (*bd_options).pb_sc_equilibrium_dist = str.toFloat();
   //   le_pb_sc_equilibrium_dist->setText(QString("").sprintf("%4.2f",(*hydro).pb_sc_equilibrium_dist));
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}


void US_Hydrodyn_BD_Options::update_pb_sc_max_elong(const QString &str)
{
   (*bd_options).pb_sc_max_elong = str.toFloat();
   //   le_pb_sc_max_elong->setText(QString("").sprintf("%4.2f",(*hydro).pb_sc_max_elong));
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}


void US_Hydrodyn_BD_Options::update_sc_sc_force_constant(const QString &str)
{
   (*bd_options).sc_sc_force_constant = str.toFloat();
   //   le_sc_sc_force_constant->setText(QString("").sprintf("%4.2f",(*hydro).sc_sc_force_constant));
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}


void US_Hydrodyn_BD_Options::update_sc_sc_equilibrium_dist(const QString &str)
{
   (*bd_options).sc_sc_equilibrium_dist = str.toFloat();
   //   le_sc_sc_equilibrium_dist->setText(QString("").sprintf("%4.2f",(*hydro).sc_sc_equilibrium_dist));
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}


void US_Hydrodyn_BD_Options::update_sc_sc_max_elong(const QString &str)
{
   (*bd_options).sc_sc_max_elong = str.toFloat();
   //   le_sc_sc_max_elong->setText(QString("").sprintf("%4.2f",(*hydro).sc_sc_max_elong));
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_BD_Options::update_enables()
{
   switch ( bd_options->chem_pb_pb_bond_type )
   {
   case 0 : // fraenkel (hard hookean)
      cb_compute_chem_pb_pb_equilibrium_dist->setEnabled(true);
      le_chem_pb_pb_equilibrium_dist->setEnabled(true);
      cb_compute_chem_pb_pb_max_elong->setEnabled(false);
      le_chem_pb_pb_max_elong->setEnabled(false);
      break;
   case 1 : // hookean, gaussian (soft)
      cb_compute_chem_pb_pb_equilibrium_dist->setEnabled(false);
      le_chem_pb_pb_equilibrium_dist->setEnabled(false);
      cb_compute_chem_pb_pb_max_elong->setEnabled(false);
      le_chem_pb_pb_max_elong->setEnabled(false);
      break;
   case 2 : // fene
      cb_compute_chem_pb_pb_equilibrium_dist->setEnabled(false);
      le_chem_pb_pb_equilibrium_dist->setEnabled(false);
      cb_compute_chem_pb_pb_max_elong->setEnabled(true);
      le_chem_pb_pb_max_elong->setEnabled(true);
      break;
   case 3 : // hard-fene
      cb_compute_chem_pb_pb_equilibrium_dist->setEnabled(true);
      le_chem_pb_pb_equilibrium_dist->setEnabled(true);
      cb_compute_chem_pb_pb_max_elong->setEnabled(true);
      le_chem_pb_pb_max_elong->setEnabled(true);
      break;
   default : 
      break;
   }
   switch ( bd_options->chem_pb_sc_bond_type )
   {
   case 0 : // fraenkel (hard hookean)
      cb_compute_chem_pb_sc_equilibrium_dist->setEnabled(true);
      le_chem_pb_sc_equilibrium_dist->setEnabled(true);
      cb_compute_chem_pb_sc_max_elong->setEnabled(false);
      le_chem_pb_sc_max_elong->setEnabled(false);
      break;
   case 1 : // hookean, gaussian (soft)
      cb_compute_chem_pb_sc_equilibrium_dist->setEnabled(false);
      le_chem_pb_sc_equilibrium_dist->setEnabled(false);
      cb_compute_chem_pb_sc_max_elong->setEnabled(false);
      le_chem_pb_sc_max_elong->setEnabled(false);
      break;
   case 2 : // fene
      cb_compute_chem_pb_sc_equilibrium_dist->setEnabled(false);
      le_chem_pb_sc_equilibrium_dist->setEnabled(false);
      cb_compute_chem_pb_sc_max_elong->setEnabled(true);
      le_chem_pb_sc_max_elong->setEnabled(true);
      break;
   case 3 : // hard-fene
      cb_compute_chem_pb_sc_equilibrium_dist->setEnabled(true);
      le_chem_pb_sc_equilibrium_dist->setEnabled(true);
      cb_compute_chem_pb_sc_max_elong->setEnabled(true);
      le_chem_pb_sc_max_elong->setEnabled(true);
      break;
   default : 
      break;
   }
   switch ( bd_options->chem_sc_sc_bond_type )
   {
   case 0 : // fraenkel (hard hookean)
      cb_compute_chem_sc_sc_equilibrium_dist->setEnabled(true);
      le_chem_sc_sc_equilibrium_dist->setEnabled(true);
      cb_compute_chem_sc_sc_max_elong->setEnabled(false);
      le_chem_sc_sc_max_elong->setEnabled(false);
      break;
   case 1 : // hookean, gaussian (soft)
      cb_compute_chem_sc_sc_equilibrium_dist->setEnabled(false);
      le_chem_sc_sc_equilibrium_dist->setEnabled(false);
      cb_compute_chem_sc_sc_max_elong->setEnabled(false);
      le_chem_sc_sc_max_elong->setEnabled(false);
      break;
   case 2 : // fene
      cb_compute_chem_sc_sc_equilibrium_dist->setEnabled(false);
      le_chem_sc_sc_equilibrium_dist->setEnabled(false);
      cb_compute_chem_sc_sc_max_elong->setEnabled(true);
      le_chem_sc_sc_max_elong->setEnabled(true);
      break;
   case 3 : // hard-fene
      cb_compute_chem_sc_sc_equilibrium_dist->setEnabled(true);
      le_chem_sc_sc_equilibrium_dist->setEnabled(true);
      cb_compute_chem_sc_sc_max_elong->setEnabled(true);
      le_chem_sc_sc_max_elong->setEnabled(true);
      break;
   default : 
      break;
   }
   switch ( bd_options->pb_pb_bond_type )
   {
   case 0 : // fraenkel (hard hookean)
      cb_compute_pb_pb_equilibrium_dist->setEnabled(true);
      le_pb_pb_equilibrium_dist->setEnabled(true);
      cb_compute_pb_pb_max_elong->setEnabled(false);
      le_pb_pb_max_elong->setEnabled(false);
      break;
   case 1 : // hookean, gaussian (soft)
      cb_compute_pb_pb_equilibrium_dist->setEnabled(false);
      le_pb_pb_equilibrium_dist->setEnabled(false);
      cb_compute_pb_pb_max_elong->setEnabled(false);
      le_pb_pb_max_elong->setEnabled(false);
      break;
   case 2 : // fene
      cb_compute_pb_pb_equilibrium_dist->setEnabled(false);
      le_pb_pb_equilibrium_dist->setEnabled(false);
      cb_compute_pb_pb_max_elong->setEnabled(true);
      le_pb_pb_max_elong->setEnabled(true);
      break;
   case 3 : // hard-fene
      cb_compute_pb_pb_equilibrium_dist->setEnabled(true);
      le_pb_pb_equilibrium_dist->setEnabled(true);
      cb_compute_pb_pb_max_elong->setEnabled(true);
      le_pb_pb_max_elong->setEnabled(true);
      break;
   default : 
      break;
   }
   switch ( bd_options->pb_sc_bond_type )
   {
   case 0 : // fraenkel (hard hookean)
      cb_compute_pb_sc_equilibrium_dist->setEnabled(true);
      le_pb_sc_equilibrium_dist->setEnabled(true);
      cb_compute_pb_sc_max_elong->setEnabled(false);
      le_pb_sc_max_elong->setEnabled(false);
      break;
   case 1 : // hookean, gaussian (soft)
      cb_compute_pb_sc_equilibrium_dist->setEnabled(false);
      le_pb_sc_equilibrium_dist->setEnabled(false);
      cb_compute_pb_sc_max_elong->setEnabled(false);
      le_pb_sc_max_elong->setEnabled(false);
      break;
   case 2 : // fene
      cb_compute_pb_sc_equilibrium_dist->setEnabled(false);
      le_pb_sc_equilibrium_dist->setEnabled(false);
      cb_compute_pb_sc_max_elong->setEnabled(true);
      le_pb_sc_max_elong->setEnabled(true);
      break;
   case 3 : // hard-fene
      cb_compute_pb_sc_equilibrium_dist->setEnabled(true);
      le_pb_sc_equilibrium_dist->setEnabled(true);
      cb_compute_pb_sc_max_elong->setEnabled(true);
      le_pb_sc_max_elong->setEnabled(true);
      break;
   default : 
      break;
   }
   switch ( bd_options->sc_sc_bond_type )
   {
   case 0 : // fraenkel (hard hookean)
      cb_compute_sc_sc_equilibrium_dist->setEnabled(true);
      le_sc_sc_equilibrium_dist->setEnabled(true);
      cb_compute_sc_sc_max_elong->setEnabled(false);
      le_sc_sc_max_elong->setEnabled(false);
      break;
   case 1 : // hookean, gaussian (soft)
      cb_compute_sc_sc_equilibrium_dist->setEnabled(false);
      le_sc_sc_equilibrium_dist->setEnabled(false);
      cb_compute_sc_sc_max_elong->setEnabled(false);
      le_sc_sc_max_elong->setEnabled(false);
      break;
   case 2 : // fene
      cb_compute_sc_sc_equilibrium_dist->setEnabled(false);
      le_sc_sc_equilibrium_dist->setEnabled(false);
      cb_compute_sc_sc_max_elong->setEnabled(true);
      le_sc_sc_max_elong->setEnabled(true);
      break;
   case 3 : // hard-fene
      cb_compute_sc_sc_equilibrium_dist->setEnabled(true);
      le_sc_sc_equilibrium_dist->setEnabled(true);
      cb_compute_sc_sc_max_elong->setEnabled(true);
      le_sc_sc_max_elong->setEnabled(true);
      break;
   default : 
      break;
   }
}

void US_Hydrodyn_BD_Options::dup_fraenkel()
{
   update_chem_pb_sc_force_constant(le_chem_pb_pb_force_constant->text());
   le_chem_pb_sc_force_constant->setText(le_chem_pb_pb_force_constant->text());
   cb_compute_chem_pb_sc_force_constant->setChecked(cb_compute_chem_pb_pb_force_constant->isChecked());
   update_chem_sc_sc_force_constant(le_chem_pb_pb_force_constant->text());
   le_chem_sc_sc_force_constant->setText(le_chem_pb_pb_force_constant->text());
   cb_compute_chem_sc_sc_force_constant->setChecked(cb_compute_chem_pb_pb_force_constant->isChecked());
   update_pb_pb_force_constant(le_chem_pb_pb_force_constant->text());
   le_pb_pb_force_constant->setText(le_chem_pb_pb_force_constant->text());
   cb_compute_pb_pb_force_constant->setChecked(cb_compute_chem_pb_pb_force_constant->isChecked());
   update_pb_sc_force_constant(le_chem_pb_pb_force_constant->text());
   le_pb_sc_force_constant->setText(le_chem_pb_pb_force_constant->text());
   cb_compute_pb_sc_force_constant->setChecked(cb_compute_chem_pb_pb_force_constant->isChecked());
   update_sc_sc_force_constant(le_chem_pb_pb_force_constant->text());
   le_sc_sc_force_constant->setText(le_chem_pb_pb_force_constant->text());
   cb_compute_sc_sc_force_constant->setChecked(cb_compute_chem_pb_pb_force_constant->isChecked());
}

#define TOLERANCE 1e-2

void US_Hydrodyn_BD_Options::update_labels()
{
   double total_steps = bd_options->ttraj / bd_options->deltat;

   int digits = (int)log10(total_steps) - 5;
   digits = digits >= 0 ? digits : 0;
   if ( fabs( total_steps * pow(0.1,digits) - (double)(int( total_steps * pow(0.1,digits) + 0.5 )) ) < TOLERANCE )
   {
      cout << QString("total steps %1 int total_steps %2\n").arg(total_steps).arg(int(total_steps));
      cout << QString("").sprintf("%f %e %g\n", total_steps, total_steps, (total_steps - int(total_steps)));
      lbl_deltat->setPalette(label_font_ok);
      lbl_deltat->setText(tr(" Duration of each simulation step (s): "));
   } else {
      cout << QString("total steps %1 int total_steps %2 diff %3\n")
         .arg(total_steps)
         .arg(int(total_steps))
         .arg(fabs( ( total_steps ) - int( total_steps + 0.5 ) ));
      cout << QString("").sprintf("%f %e %g\n", total_steps, total_steps, (total_steps - int(total_steps)));
      lbl_deltat->setPalette(label_font_warning);
      lbl_deltat->setText(tr(" Duration of each simulation step (s)\n"
                             " WARNING: does not divide total duration: "));
   }
   double max_conf = total_steps / bd_options->npadif;
   double check_nconf = max_conf / bd_options->nconf;
   if ( fabs( check_nconf  - int( check_nconf + 0.5 ) ) < TOLERANCE )
   {
      cout << QString("check_nconf %1 int check_nconf %2\n").arg(check_nconf).arg(int(check_nconf));
      if ( bd_options->nconf > max_conf )
      {
         lbl_nconf->setPalette(label_font_warning);
         lbl_nconf->setText(tr(QString(" Number of conformations to store (maximum %1)\n"
                                       " WARNING: greater than maximum: ").arg((int)max_conf)));
      } else {
         lbl_nconf->setPalette(label_font_ok);
         lbl_nconf->setText(tr(QString(" Number of conformations to store (max %1): ").arg((int)max_conf)));
      }
   } else {
      lbl_nconf->setPalette(label_font_warning);
      lbl_nconf->setText(tr(QString(" Number of conformations to store (maximum %1)\n"
                                    " WARNING: does not divide maximum").arg((int)max_conf)));
   }
}

