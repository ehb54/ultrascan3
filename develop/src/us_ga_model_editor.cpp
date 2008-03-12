#include "../include/us_ga_model_editor.h"

US_GAModelEditor::US_GAModelEditor(struct ModelSystem *ms, struct ModelSystemConstraints *msc,
QWidget *parent, const char *name) : US_ModelEditor(false, ms, parent, name)
{
	this->ms = ms;
	this->msc = msc;
	setup_GUI();
	select_component((int) 0);
	
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

	c_mw.low = (*msc).component_vector_constraints[0].mw.low;
	c_mw.high = (*msc).component_vector_constraints[0].mw.high;
	c_mw.fit = (*msc).component_vector_constraints[0].mw.fit;
	c_conc.low = (*msc).component_vector_constraints[0].concentration.low;
	c_conc.high = (*msc).component_vector_constraints[0].concentration.high;
	c_conc.fit = (*msc).component_vector_constraints[0].concentration.fit;
	c_f_f0.low = (*msc).component_vector_constraints[0].f_f0.low;
	c_f_f0.high = (*msc).component_vector_constraints[0].f_f0.high;
	c_f_f0.fit = (*msc).component_vector_constraints[0].f_f0.fit;
	c_keq.low = 0.0;
	c_keq.high = 0.0;
	c_keq.fit = false;
	c_koff.low = 0.0;
	c_koff.high = 0.0;
	c_koff.fit = false;
	
	us_cc_mw = new US_ConstraintControl(0.0, &c_mw, this);
	us_cc_mw->hide();
	us_cc_f_f0 = new US_ConstraintControl(0.0, &c_conc, this);
	us_cc_f_f0->hide();
	us_cc_conc = new US_ConstraintControl(0.0, &c_f_f0, this);
	us_cc_conc->hide();
	us_cc_keq = new US_ConstraintControl(0.0, &c_keq, this);
	us_cc_keq->hide();
	us_cc_koff = new US_ConstraintControl(0.0, &c_koff, this);
	us_cc_koff->hide();

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
	grid->addWidget(us_cc_mw->le_low, j, 2, 0);
	grid->addWidget(us_cc_mw->le_high, j, 3, 0);
	grid->addWidget(us_cc_mw->cb_fit, j, 4, 0);
	j++;
	grid->addWidget(lbl_f_f0, j, 0, 0);
	grid->addWidget(le_f_f0, j, 1, 0);
	grid->addWidget(us_cc_f_f0->le_low, j, 2, 0);
	grid->addWidget(us_cc_f_f0->le_high, j, 3, 0);
	grid->addWidget(us_cc_f_f0->cb_fit, j, 4, 0);
	j++;
	grid->addWidget(lbl_conc, j, 0, 0);
	grid->addWidget(le_conc, j, 1, 0);
	grid->addWidget(us_cc_conc->le_low, j, 2, 0);
	grid->addWidget(us_cc_conc->le_high, j, 3, 0);
	grid->addWidget(us_cc_conc->cb_fit, j, 4, 0);
	j++;
	grid->addWidget(lbl_keq, j, 0, 0);
	grid->addWidget(le_keq, j, 1, 0);
	grid->addWidget(us_cc_keq->le_low, j, 2, 0);
	grid->addWidget(us_cc_keq->le_high, j, 3, 0);
	grid->addWidget(us_cc_keq->cb_fit, j, 4, 0);
	j++;
	grid->addWidget(lbl_koff, j, 0, 0);
	grid->addWidget(le_koff, j, 1, 0);
	grid->addWidget(us_cc_koff->le_low, j, 2, 0);
	grid->addWidget(us_cc_koff->le_high, j, 3, 0);
	grid->addWidget(us_cc_koff->cb_fit, j, 4, 0);
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

void US_GAModelEditor::update_constraints(unsigned int c)
{
	current_component = c;
	/*
	QString str;
	if ((*ms).component_vector[current_component].show_conc)
	{
		le_conc->setText(str.sprintf("%6.4e", (*ms).component_vector[current_component].concentration));
	}
	else
	{
		le_conc->setEnabled(false);
		le_conc->setText("");
		pb_load_c0->setEnabled(false);
		lbl_load_c0->setText("");
	}
	if ((*ms).component_vector[current_component].show_keq)
	{
		for (unsigned int i=0; i<(*ms).assoc_vector.size(); i++)
		{ // only check the dissociating species
			if ((*ms).assoc_vector[i].component2 == (int) current_component
							||  (*ms).assoc_vector[i].component3 == (int) current_component)
			{
				le_keq->setText(str.sprintf("%6.4e", (*ms).assoc_vector[i].keq));
				le_keq->setEnabled(true);
				le_koff->setText(str.sprintf("%6.4e", (*ms).assoc_vector[i].k_off));
				le_koff->setEnabled(true);
			}
		}
	}
	else
	{
		le_keq->setEnabled(false);
		le_keq->setText("");
		le_koff->setEnabled(false);
		le_koff->setText("");
	}
	if ((*ms).component_vector[current_component].show_stoich != 0)
	{
		if ((*ms).component_vector[current_component].show_stoich > 0)
		{
			le_stoich->setEnabled(true);
			le_stoich->setText(str.sprintf("%d", (*ms).component_vector[current_component].show_stoich));
		}
		else
		{
			le_stoich->setText("hetero-associating");
		}
	}
	else
	{
		le_stoich->setText("");
		le_stoich->setEnabled(false);
	}
	*/
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
	current_component = 0;
	select_component((int) current_component);
}


