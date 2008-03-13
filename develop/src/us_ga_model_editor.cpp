#include "../include/us_ga_model_editor.h"

US_GAModelEditor::US_GAModelEditor(struct ModelSystem *ms, struct ModelSystemConstraints *msc,
QWidget *parent, const char *name) : US_ModelEditor(false, ms, parent, name)
{
	this->ms = ms;
	this->msc = msc;
	setup_GUI();
	current_component = 0;
	current_assoc = 0;
	select_component((int) current_component);
	
	global_Xpos += 30;
	global_Ypos += 30;

	move(global_Xpos, global_Ypos);
	connect(this, SIGNAL(componentChanged(unsigned int)), SLOT(update_constraints(unsigned int)));
}

US_GAModelEditor::~US_GAModelEditor()
{
}

void US_GAModelEditor::setup_GUI()
{
	int minHeight1 = 30, minHeight2 = 26;
	delete pb_accept;
	delete pb_save;
	delete pb_cancel;
	
	lbl_constraints = new QLabel(tr("Constraints for Current Component:"), this);
	lbl_constraints->setAlignment(AlignHCenter|AlignVCenter);
	lbl_constraints->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_constraints->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_constraints->setMinimumHeight(minHeight2);
	
	lbl_low = new QLabel(tr("Low:"), this);
	lbl_low->setAlignment(AlignHCenter|AlignVCenter);
	lbl_low->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_low->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_low->setMinimumHeight(minHeight2);
	
	lbl_high = new QLabel(tr("High:"), this);
	lbl_high->setAlignment(AlignHCenter|AlignVCenter);
	lbl_high->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_high->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_high->setMinimumHeight(minHeight2);
	
	lbl_fit = new QLabel(tr("Fit?"), this);
	lbl_fit->setAlignment(AlignHCenter|AlignVCenter);
	lbl_fit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_fit->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_fit->setMinimumHeight(minHeight2);
	
	lbl_bandVolume = new QLabel(tr(" Band-loading Volume:"), this);
	lbl_bandVolume->setAlignment(AlignLeft|AlignVCenter);
	lbl_bandVolume->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_bandVolume->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_bandVolume->setMinimumHeight(minHeight2);
	
	lbl_simpoints = new QLabel(tr(" # of Simulation Points:"), this);
	lbl_simpoints->setAlignment(AlignLeft|AlignVCenter);
	lbl_simpoints->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_simpoints->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_simpoints->setMinimumHeight(minHeight2);

	cc_mw = new US_ConstraintControl(this);
	cc_mw->setEnabled(true); // initial model is model "0" with mw fittable
	cc_mw->setDefault((*ms).component_vector[current_component].mw, (float) 0.2);
	connect(cc_mw, SIGNAL(constraintChanged(struct constraint)), this, SLOT(mw_constraintChanged(struct constraint)));
	cc_mw->hide();
	
	cc_f_f0 = new US_ConstraintControl(this);
	cc_f_f0->setEnabled(true); // initial model is model "0" with f_f0 fittable
	cc_f_f0->setDefault((*ms).component_vector[current_component].f_f0, (float) 0.2);
	connect(cc_f_f0, SIGNAL(constraintChanged(struct constraint)), this, SLOT(f_f0_constraintChanged(struct constraint)));
	cc_f_f0->hide();
	
	cc_conc = new US_ConstraintControl(this);
	cc_conc->setEnabled(true); // initial model is model "0" with conc fittable
	cc_conc->setDefault((*ms).component_vector[current_component].concentration, (float) 0.2);
	connect(cc_conc, SIGNAL(constraintChanged(struct constraint)), this, SLOT(conc_constraintChanged(struct constraint)));
	cc_conc->hide();
	
	cc_keq = new US_ConstraintControl(this);
	cc_keq->setEnabled(false); // initial model is model "0" without associations
	connect(cc_keq, SIGNAL(constraintChanged(struct constraint)), this, SLOT(keq_constraintChanged(struct constraint)));
	cc_keq->hide();
	
	cc_koff = new US_ConstraintControl(this);
	cc_koff->setEnabled(false); // initial model is model "0" without associations
	connect(cc_koff, SIGNAL(constraintChanged(struct constraint)), this, SLOT(koff_constraintChanged(struct constraint)));
	cc_koff->hide();

	cmb_radialGrid = new QComboBox(false, this, "Radial Grid" );
	cmb_radialGrid->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cmb_radialGrid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cmb_radialGrid->setSizeLimit(5);
	cmb_radialGrid->setMinimumHeight(minHeight1);
	cmb_radialGrid->insertItem("Adaptive Space Mesh (ASTFEM)", -1);
	cmb_radialGrid->insertItem("Claverie Fixed Mesh", -1);
	cmb_radialGrid->insertItem("Moving Hat Mesh", -1);
	cmb_radialGrid->insertItem("File: \"$ULTRASCAN/mesh.dat\"", -1);
	cmb_radialGrid->setCurrentItem(0);
	connect(cmb_radialGrid, SIGNAL(activated(int)), this, SLOT(update_radialGrid(int)));

	cmb_timeGrid = new QComboBox(false, this, "Time Grid" );
	cmb_timeGrid->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cmb_timeGrid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cmb_timeGrid->setSizeLimit(5);
	cmb_timeGrid->setMinimumHeight(minHeight1);
	cmb_timeGrid->insertItem("Constant Time Grid (Claverie/Acceleration)", -1);
	cmb_timeGrid->insertItem("Moving Time Grid (ASTFEM/Moving Hat)", -1);
	cmb_timeGrid->setCurrentItem(1);
	connect(cmb_timeGrid, SIGNAL(activated(int)), this, SLOT(update_timeGrid(int)));

	cnt_simpoints= new QwtCounter(this);
	cnt_simpoints->setNumButtons(3);
	cnt_simpoints->setRange(50, 5000, 10);
	cnt_simpoints->setValue(100.0);
	cnt_simpoints->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_simpoints->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_simpoints->setMinimumHeight(minHeight1);
	connect(cnt_simpoints, SIGNAL(valueChanged(double)), SLOT(update_simpoints(double)));

	cnt_lamella = new QwtCounter(this);
	cnt_lamella->setRange(0.001, 0.1, 0.0001);
	cnt_lamella->setNumButtons(3);
	cnt_lamella->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_lamella->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_lamella->setValue(0.015);
	cnt_lamella->setMinimumHeight(minHeight1);
	connect(cnt_lamella, SIGNAL(valueChanged(double)), SLOT(update_lamella(double)));

	pb_selectModel = new QPushButton( tr("Select Model"), this );
	pb_selectModel->setAutoDefault(false);
	pb_selectModel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_selectModel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_selectModel->setMinimumHeight(minHeight1);
	connect(pb_selectModel, SIGNAL(clicked()), SLOT(select_model()) );

	pb_loadInit = new QPushButton( tr("Load Initialization"), this );
	pb_loadInit->setAutoDefault(false);
	pb_loadInit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_loadInit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_loadInit->setMinimumHeight(minHeight1);
	connect(pb_loadInit, SIGNAL(clicked()), SLOT(load_constraints()) );

	pb_saveInit = new QPushButton( tr("Save Initialization"), this );
	pb_saveInit->setAutoDefault(false);
	pb_saveInit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_saveInit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_saveInit->setMinimumHeight(minHeight1);
	connect(pb_saveInit, SIGNAL(clicked()), SLOT(save_constraints()) );

	pb_close = new QPushButton( tr("Close"), this );
	pb_close->setAutoDefault(false);
	pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_close->setMinimumHeight(minHeight1);
	connect(pb_close, SIGNAL(clicked()), SLOT(accept()));

	le_sed->setReadOnly(true);
	le_diff->setReadOnly(true);

	unsigned int j;
	QGridLayout *grid = new QGridLayout(this, 17, 5, 4, 2);
	grid->addMultiCellWidget(lbl_model, 0, 0, 0, 4, 0);
	grid->addMultiCellWidget(lbl_current, 1, 1, 0, 1, 0);
	grid->addMultiCellWidget(lbl_linked, 1, 1, 2, 4, 0);
	grid->addMultiCellWidget(cmb_component1, 2, 2, 0, 1, 0);
	grid->addMultiCellWidget(cmb_component2, 2, 2, 2, 4, 0);
	j = 3;
	grid->addWidget(pb_simulateComponent, j, 0, 0);
	grid->addWidget(cnt_item, j, 1, 0);
	grid->addWidget(lbl_stoich, j, 2, 0);
	grid->addMultiCellWidget(le_stoich, j, j, 3, 4, 0);
	j++;
	grid->addWidget(cb_prolate, j, 0, 0);
	grid->addWidget(cb_oblate, j, 1, 0);
	grid->addWidget(pb_load_c0, j, 2, 0);
	grid->addMultiCellWidget(lbl_load_c0, j, j, 3, 4, 0);
	j++;	
	grid->addWidget(cb_rod, j, 0, 0);
	grid->addWidget(cb_sphere, j, 1, 0);
	grid->addWidget(pb_vbar, j, 2, 0);
	grid->addMultiCellWidget(le_vbar, j, j, 3, 4, 0);
	j++;
	grid->addWidget(lbl_sed, j, 0, 0);
	grid->addWidget(le_sed, j, 1, 0);
	grid->addWidget(lbl_sigma, j, 2, 0);
	grid->addMultiCellWidget(le_sigma, j, j, 3, 4, 0);
	j++;
	grid->addWidget(lbl_diff, j, 0, 0);
	grid->addWidget(le_diff, j, 1, 0);
	grid->addWidget(lbl_delta, j, 2, 0);
	grid->addMultiCellWidget(le_delta, j, j, 3, 4, 0);
	j++;
	grid->addMultiCellWidget(lbl_constraints, j, j, 0, 1, 0);
	grid->addWidget(lbl_low, j, 2, 0);
	grid->addWidget(lbl_high, j, 3, 0);
	grid->addWidget(lbl_fit, j, 4, 0);
	j++;
	grid->addWidget(lbl_mw, j, 0, 0);
	grid->addWidget(le_mw, j, 1, 0);
	grid->addWidget(cc_mw->le_low, j, 2, 0);
	grid->addWidget(cc_mw->le_high, j, 3, 0);
	grid->addWidget(cc_mw->cb_fit, j, 4, 0);
	j++;
	grid->addWidget(lbl_f_f0, j, 0, 0);
	grid->addWidget(le_f_f0, j, 1, 0);
	grid->addWidget(cc_f_f0->le_low, j, 2, 0);
	grid->addWidget(cc_f_f0->le_high, j, 3, 0);
	grid->addWidget(cc_f_f0->cb_fit, j, 4, 0);
	j++;
	grid->addWidget(lbl_conc, j, 0, 0);
	grid->addWidget(le_conc, j, 1, 0);
	grid->addWidget(cc_conc->le_low, j, 2, 0);
	grid->addWidget(cc_conc->le_high, j, 3, 0);
	grid->addWidget(cc_conc->cb_fit, j, 4, 0);
	j++;
	grid->addWidget(lbl_keq, j, 0, 0);
	grid->addWidget(le_keq, j, 1, 0);
	grid->addWidget(cc_keq->le_low, j, 2, 0);
	grid->addWidget(cc_keq->le_high, j, 3, 0);
	grid->addWidget(cc_keq->cb_fit, j, 4, 0);
	j++;
	grid->addWidget(lbl_koff, j, 0, 0);
	grid->addWidget(le_koff, j, 1, 0);
	grid->addWidget(cc_koff->le_low, j, 2, 0);
	grid->addWidget(cc_koff->le_high, j, 3, 0);
	grid->addWidget(cc_koff->cb_fit, j, 4, 0);
	j++;
	grid->addWidget(lbl_simpoints, j, 0, 0);
	grid->addWidget(cnt_simpoints, j, 1, 0);
	grid->addMultiCellWidget(cmb_radialGrid, j, j, 2, 3, 0);
	grid->addWidget(pb_help, j, 4, 0);
	j++;
	grid->addWidget(lbl_bandVolume, j, 0, 0);
	grid->addWidget(cnt_lamella, j, 1, 0);
	grid->addMultiCellWidget(cmb_timeGrid, j, j, 2, 3, 0);
	grid->addWidget(pb_close, j, 4, 0);
	j++;
	grid->addWidget(pb_selectModel, j, 0, 0);
	grid->addWidget(pb_load_model, j, 1, 0);
	grid->addWidget(pb_loadInit, j, 2, 0);
	grid->addMultiCellWidget(pb_saveInit, j, j, 3, 4, 0);
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

void US_GAModelEditor::mw_constraintChanged(struct constraint c)
{
	(*msc).component_vector_constraints[current_component].mw.low = c.low;
	(*msc).component_vector_constraints[current_component].mw.high = c.high;
	(*msc).component_vector_constraints[current_component].mw.fit = c.fit;
}

void US_GAModelEditor::f_f0_constraintChanged(struct constraint c)
{
	if (c.low < 1.0)
	{
		cc_f_f0->le_low->setText("1.000");
		return;
	}
	(*msc).component_vector_constraints[current_component].f_f0.low = c.low;
	(*msc).component_vector_constraints[current_component].f_f0.high = c.high;
	(*msc).component_vector_constraints[current_component].f_f0.fit = c.fit;
}

void US_GAModelEditor::conc_constraintChanged(struct constraint c)
{
	(*msc).component_vector_constraints[current_component].concentration.low = c.low;
	(*msc).component_vector_constraints[current_component].concentration.high = c.high;
	(*msc).component_vector_constraints[current_component].concentration.fit = c.fit;
}

void US_GAModelEditor::keq_constraintChanged(struct constraint c)
{
	if ((*msc).assoc_vector_constraints.size() > 0)
	{
		(*msc).assoc_vector_constraints[current_assoc].keq.low = c.low;
		(*msc).assoc_vector_constraints[current_assoc].keq.high = c.high;
		(*msc).assoc_vector_constraints[current_assoc].keq.fit = c.fit;
	}
}

void US_GAModelEditor::koff_constraintChanged(struct constraint c)
{
	if ((*msc).assoc_vector_constraints.size() > 0)
	{
		(*msc).assoc_vector_constraints[current_assoc].koff.low = c.low;
		(*msc).assoc_vector_constraints[current_assoc].koff.high = c.high;
		(*msc).assoc_vector_constraints[current_assoc].koff.fit = c.fit;
	}
}

void US_GAModelEditor::update_constraints(unsigned int c)
{
	current_component = c;
	QString str;
	le_f_f0->setReadOnly(false);
	if ((*msc).component_vector_constraints[current_component].f_f0.low <= 1.0
			  || (*msc).component_vector_constraints[current_component].f_f0.high <= 1.0)
	{ // initialize the constraints with defaults if no low/high values exist
		cc_f_f0->setDefault((*ms).component_vector[current_component].f_f0,
						  1.0 - (*ms).component_vector[current_component].f_f0 * 0.5);
	}
	if ((*ms).component_vector[current_component].show_conc)
	{
		if ((*msc).component_vector_constraints[current_component].concentration.low <= 0.0
		 || (*msc).component_vector_constraints[current_component].concentration.high <= 0.0)
		{ // initialize the constraints with defaults if no low/high values exist
			cc_conc->setDefault((*ms).component_vector[current_component].concentration, (float) 0.2);
		}
	}
	else
	{
		cc_conc->setFit(false);
		cc_conc->clear();
	}
	if ((*ms).component_vector[current_component].show_keq)
	{
		for (unsigned int i=0; i<(*ms).assoc_vector.size(); i++)
		{ // only check the dissociating species to set keq/koff
			if ((*ms).assoc_vector[i].component2 == (int) current_component
			||  (*ms).assoc_vector[i].component3 == (int) current_component)
			{
				cc_keq->setEnabled(true);
				cc_koff->setEnabled(true);
				if ((*msc).assoc_vector_constraints[i].keq.low <= 0.0
				||  (*msc).assoc_vector_constraints[i].keq.high <= 0.0)
				{ // initialize the constraints with defaults if no low/high values exist
					cc_keq->setDefault((*ms).assoc_vector[i].keq, (float) 0.9);
				}
				if ((*msc).assoc_vector_constraints[i].koff.low <= 0.0
				||  (*msc).assoc_vector_constraints[i].koff.high <= 0.0)
				{ // initialize the constraints with defaults if no low/high values exist
					cc_keq->setDefault((*ms).assoc_vector[i].k_off, (float) 0.99);
				}
			}
		}
	}
	else
	{
		cc_keq->clear();
		cc_koff->clear();
	}
	for (unsigned int i=0; i<(*ms).assoc_vector.size(); i++)
	{ // only check the dissociating species to set keq/koff
		if ((*ms).assoc_vector[i].component1 == (int) current_component)
		{
			cc_mw->setEnabled(true);
			if ((*msc).component_vector_constraints[current_component].mw.low <= 0.0
			||  (*msc).component_vector_constraints[current_component].mw.high <= 0.0)
			{ // initialize the constraints with defaults if no low/high values exist
				cc_mw->setDefault((*ms).component_vector[current_component].mw, (float) 0.2);
			}
		}
	}
}

void US_GAModelEditor::select_model()
{
	US_FemGlobal *fg;
	fg = new US_FemGlobal();
	fg->select_model(ms);
	delete fg;
	lbl_model->setText(modelString[(*ms).model]);
	cmb_component1->clear();
	for (unsigned int i=0; i<(*ms).component_vector.size(); i++)
	{
		cmb_component1->insertItem((*ms).component_vector[i].name);
	}
	cnt_item->setRange(1, (*ms).component_vector.size(), 1);
	// after selecting a model we need to allocate memory for msc in initialize_msc
	initialize_msc();
	current_component = 0;
	select_component((int) current_component);
}

void US_GAModelEditor::initialize_msc()
{
	(*msc).component_vector_constraints.resize((*ms).component_vector.size());
	(*msc).assoc_vector_constraints.resize((*ms).assoc_vector.size());
}

