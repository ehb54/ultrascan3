#include "../include/us_ga_model_editor.h"

US_GAModelEditor::US_GAModelEditor(struct ModelSystem *ms, struct ModelSystemConstraints *msc,
QWidget *parent, const char *name) : US_ModelEditor(system, parent, name)
{
	this->ms = ms;
	this->msc = msc;
	setup_GUI();
	select_component((int) 0);
	
	global_Xpos += 30;
	global_Ypos += 30;

	move(global_Xpos, global_Ypos);
}

US_GAModelEditor::US_GAModelEditor(QWidget *parent, const char *name) : US_ModelEditor(system, parent, name)
{
	setup_GUI();
	select_component((int) 0);
	
	global_Xpos += 30;
	global_Ypos += 30;

	move(global_Xpos, global_Ypos);
}

US_GAModelEditor::~US_GAModelEditor()
{
}

void US_GAModelEditor::setup_GUI()
{
	int minHeight1 = 30, minHeight2 = 26;
	
	lbl_constraints = new QLabel(tr("Constraints for Current Component:"), this);
	lbl_constraints->setAlignment(AlignLeft|AlignVCenter);
	lbl_constraints->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_constraints->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_constraints->setMinimumHeight(minHeight2);
	
	lbl_low = new QLabel(tr("Low:"), this);
	lbl_low->setAlignment(AlignLeft|AlignVCenter);
	lbl_low->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_low->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_low->setMinimumHeight(minHeight2);
	
	lbl_high = new QLabel(tr("High:"), this);
	lbl_high->setAlignment(AlignLeft|AlignVCenter);
	lbl_high->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_high->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_high->setMinimumHeight(minHeight2);
	
	lbl_fit = new QLabel(tr("Fit?"), this);
	lbl_fit->setAlignment(AlignLeft|AlignVCenter);
	lbl_fit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_fit->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_fit->setMinimumHeight(minHeight2);
	
	lbl_bandVolume = new QLabel(tr("Band-loading Volume:"), this);
	lbl_bandVolume->setAlignment(AlignLeft|AlignVCenter);
	lbl_bandVolume->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_bandVolume->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_bandVolume->setMinimumHeight(minHeight2);
	
	lbl_simpoints = new QLabel(tr("# of Simulation Points:"), this);
	lbl_simpoints->setAlignment(AlignLeft|AlignVCenter);
	lbl_simpoints->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_simpoints->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_simpoints->setMinimumHeight(minHeight2);

	us_cc_mw = new US_ConstraintControl(0.0, &c_mw.low, &c_mw.high, &c_mw.fit);
	us_cc_f_f0 = new US_ConstraintControl(0.0, &c_f_f0.low, &c_f_f0.high, &c_f_f0.fit);
	us_cc_conc = new US_ConstraintControl(0.0, &c_conc.low, &c_conc.high, &c_conc.fit);
	us_cc_keq = new US_ConstraintControl(0.0, &c_keq.low, &c_keq.high, &c_keq.fit);
	us_cc_koff = new US_ConstraintControl(0.0, &c_koff.low, &c_koff.high, &c_koff.fit);
	
	cmb_radialGrid = new QComboBox(false, this, "Radial Grid" );
	cmb_radialGrid->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cmb_radialGrid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cmb_radialGrid->setSizeLimit(5);
	cmb_radialGrid->setMinimumHeight(minHeight1);
	cmb_radialGrid->insertItem("Adaptive Space Mesh (ASTFEM)", -1);
	cmb_radialGrid->insertItem("Claverie Fixed Mesh", -1);
	cmb_radialGrid->insertItem("Moving Hat Mesh", -1);
	cmb_radialGrid->insertItem("File: \"$ULTRASCAN/mesh.dat\"", -1);
	cmb_radialGrid->setCurrentItem((*msc).mesh);
	connect(cmb_radialGrid, SIGNAL(activated(int)), this, SLOT(update_radialGrid(int)));

	cmb_timeGrid = new QComboBox(false, this, "Time Grid" );
	cmb_timeGrid->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cmb_timeGrid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cmb_timeGrid->setSizeLimit(5);
	cmb_timeGrid->setMinimumHeight(minHeight1);
	cmb_timeGrid->insertItem("Constant Time Grid (Claverie/Acceleration)", -1);
	cmb_timeGrid->insertItem("Moving Time Grid (ASTFEM/Moving Hat)", -1);
	cmb_timeGrid->setCurrentItem((*msc).moving_grid);
	connect(cmb_timeGrid, SIGNAL(activated(int)), this, SLOT(update_timeGrid(int)));

	cnt_simpoints= new QwtCounter(this);
	cnt_simpoints->setNumButtons(3);
	cnt_simpoints->setRange(50, 5000, 10);
	cnt_simpoints->setValue((double)(*msc).simpoints);
	cnt_simpoints->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_simpoints->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_simpoints->setMinimumHeight(minHeight1);
	connect(cnt_simpoints, SIGNAL(valueChanged(double)), SLOT(update_simpoints(double)));

	cnt_lamella = new QwtCounter(this);
	cnt_lamella->setRange(0.001, 0.1, 0.0001);
	cnt_lamella->setNumButtons(3);
	cnt_lamella->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_lamella->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_lamella->setValue((*msc).band_volume);
	cnt_lamella->setMinimumHeight(minHeight1);
	connect(cnt_lamella, SIGNAL(valueChanged(double)), SLOT(update_lamella(double)));

	pb_selectModel = new QPushButton( tr("Select Model"), this );
	pb_selectModel->setAutoDefault(false);
	pb_selectModel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_selectModel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_selectModel->setMinimumHeight(minHeight1);
	connect(pb_selectModel, SIGNAL(clicked()), SLOT(load_model()) );

	pb_selectModel = new QPushButton( tr("Load Initialization"), this );
	pb_selectModel->setAutoDefault(false);
	pb_selectModel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_selectModel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_selectModel->setMinimumHeight(minHeight1);
	connect(pb_selectModel, SIGNAL(clicked()), SLOT(load_model()) );

	pb_selectModel = new QPushButton( tr("Select Model"), this );
	pb_selectModel->setAutoDefault(false);
	pb_selectModel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_selectModel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_selectModel->setMinimumHeight(minHeight1);
	connect(pb_selectModel, SIGNAL(clicked()), SLOT(load_model()) );

	pb_selectModel = new QPushButton( tr("Select Model"), this );
	pb_selectModel->setAutoDefault(false);
	pb_selectModel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_selectModel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_selectModel->setMinimumHeight(minHeight1);
	connect(pb_selectModel, SIGNAL(clicked()), SLOT(load_model()) );

	
	unsigned int j=3;
	QGridLayout *grid = new QGridLayout(this, 11, 4, 4, 2);
	grid->addMultiCellWidget(lbl_model, 0, 0, 0, 3, 0);
	grid->addMultiCellWidget(lbl_current, 1, 1, 0, 1, 0);
	grid->addMultiCellWidget(lbl_linked, 1, 1, 2, 3, 0);
	grid->addMultiCellWidget(cmb_component1, 2, 2, 0, 1, 0);
	grid->addMultiCellWidget(cmb_component2, 2, 2, 2, 3, 0);
	grid->addWidget(lbl_sed, j, 0, 0);
	grid->addWidget(le_sed, j, 1, 0);
	grid->addWidget(lbl_conc, j, 2, 0);
	grid->addWidget(le_conc, j, 3, 0);
	j++;
	grid->addWidget(lbl_diff, j, 0, 0);
	grid->addWidget(le_diff, j, 1, 0);
	grid->addWidget(pb_load_c0, j, 2, 0);
	grid->addWidget(lbl_load_c0, j, 3, 0);
	j++;
	grid->addWidget(lbl_vbar1, j, 0, 0);
	grid->addWidget(lbl_vbar2, j, 1, 0);
	grid->addWidget(lbl_keq, j, 2, 0);
	grid->addWidget(le_keq, j, 3, 0);
	j++;
	grid->addWidget(lbl_mw1, j, 0, 0);
	grid->addWidget(lbl_mw2, j, 1, 0);
	grid->addWidget(lbl_koff, j, 2, 0);
	grid->addWidget(le_koff, j, 3, 0);
	j++;
	grid->addWidget(lbl_f_f01, j, 0, 0);
	grid->addWidget(lbl_f_f02, j, 1, 0);
	grid->addWidget(lbl_stoich, j, 2, 0);
	grid->addWidget(le_stoich, j, 3, 0);
	j++;
	grid->addWidget(pb_simulateComponent, j, 0, 0);
	grid->addWidget(cnt_item, j, 1, 0);
	grid->addWidget(lbl_sigma, j, 2, 0);
	grid->addWidget(le_sigma, j, 3, 0);
	j++;
	grid->addWidget(cb_prolate, j, 0, 0);
	grid->addWidget(cb_oblate, j, 1, 0);
	grid->addWidget(lbl_delta, j, 2, 0);
	grid->addWidget(le_delta, j, 3, 0);
	j++;
	grid->addWidget(cb_rod, j, 0, 0);
	grid->addWidget(cb_sphere, j, 1, 0);
	grid->addWidget(pb_help, j, 2, 0);
	grid->addWidget(pb_cancel, j, 3, 0);
	j++;
	grid->addMultiCellWidget(pb_selectModel, j, j, 0, 1, 0);
	grid->addWidget(pb_save, j, 2, 0);
	grid->addWidget(pb_accept, j, 3, 0);
}

void US_GAModelEditor::load_constraints()
{
}

bool US_GAModelEditor::verify_constraints()
{
  return true;
}

void US_GAModelEditor::save_constraints()
{
}

void US_GAModelEditor::help()
{
	US_Help *online_help;
	online_help = new US_Help(this);
	online_help->show_help("manual/ga_model_editor.html");
}

void US_GAModelEditor::update_radialGrid(int val)
{
	(*msc).moving_grid = val;
}

void US_GAModelEditor::update_timeGrid(int val)
{
	(*msc).mesh = val;
}

void US_GAModelEditor::update_simpoints(double val)
{
	(*msc).simpoints = (unsigned int) val;
}

void US_GAModelEditor::update_lamella(double val)
{
	(*msc).band_volume = (float) val;
}
