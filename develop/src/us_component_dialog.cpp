#include "../include/us_component_dialog.h"

US_Component_Dialog::US_Component_Dialog(struct ModelSystem *system, 
QWidget *parent, const char *name) : QDialog( parent, name, false )
{
	this->system = system;

	USglobal = new US_Config();
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	setCaption("Model Selection");

	int minHeight1 = 30, minHeight2 = 26;
	current_component = 0;
	shape = 1; // prolate is selected by default

	lbl_model = new QLabel(this);
	lbl_model->setAlignment(AlignHCenter|AlignVCenter);
	lbl_model->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	lbl_model->setText(modelString[(*system).model]);
	lbl_model->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_model->setMinimumHeight(minHeight1);

	lbl_current = new QLabel(tr(" Current Component:"), this);
	lbl_current->setAlignment(AlignLeft|AlignVCenter);
	lbl_current->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_current->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_current->setMinimumHeight(minHeight2);

	cmb_component1 = new QComboBox( FALSE, this, "Component Listings" );
	cmb_component1->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cmb_component1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cmb_component1->setSizeLimit(5);
	cmb_component1->setMinimumHeight(minHeight2);
	for (unsigned int i=0; i<(*system).component_vector.size(); i++)
	{
		cmb_component1->insertItem((*system).component_vector[i].name);
	}
	connect(cmb_component1, SIGNAL(activated(int)), this, SLOT(select_component(int)));
	connect(cmb_component1, SIGNAL(highlighted(int)), this, SLOT(select_component(int)));
		
	lbl_linked = new QLabel(tr(" This component is linked to:"), this);
	lbl_linked->setAlignment(AlignLeft|AlignVCenter);
	lbl_linked->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_linked->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_linked->setMinimumHeight(minHeight2);

	cmb_component2 = new QComboBox( FALSE, this, "Linked Component Listing" );
	cmb_component2->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cmb_component2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cmb_component2->setSizeLimit(3);
	cmb_component2->setMinimumHeight(minHeight2);

	lbl_conc = new QLabel(tr(" Partial Conc. (in OD):"), this);
	lbl_conc->setAlignment(AlignLeft|AlignVCenter);
	lbl_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_conc->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_conc->setMinimumHeight(minHeight2);

	le_conc = new QLineEdit(this, " Concentration Line Edit");
	le_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_conc->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_conc, SIGNAL(textChanged(const QString &)), 
				SLOT(update_conc(const QString &)));	
	le_conc->setMinimumHeight(minHeight2);
	le_conc->setEnabled(false);
	
	lbl_keq = new QLabel(tr(" Equilibrium Const. (in OD):"), this);
	lbl_keq->setAlignment(AlignLeft|AlignVCenter);
	lbl_keq->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_keq->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_keq->setMinimumHeight(minHeight2);

	le_keq = new QLineEdit(this, " Equilibrium Constant Line Edit");
	le_keq->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_keq->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_keq, SIGNAL(textChanged(const QString &)), 
				SLOT(update_keq(const QString &)));	
	le_keq->setMinimumHeight(minHeight2);
	le_keq->setEnabled(false);
	
	lbl_sed = new QLabel(tr(" Sedimentation Coeff. (sec):"), this);
	lbl_sed->setAlignment(AlignLeft|AlignVCenter);
	lbl_sed->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_sed->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_sed->setMinimumHeight(minHeight2);

	le_sed = new QLineEdit(this, "Sedimentation Line Edit");
	le_sed->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_sed->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	connect(le_sed, SIGNAL(textChanged(const QString &)), 
				SLOT(update_sed(const QString &)));	
	le_sed->setMinimumHeight(minHeight2);

	lbl_diff = new QLabel(tr(" Diffusion Coeff. (cm^2/sec):"), this);
	lbl_diff->setAlignment(AlignLeft|AlignVCenter);
	lbl_diff->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_diff->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_diff->setMinimumHeight(minHeight2);

	le_diff = new QLineEdit(this, "Diffusion Line Edit");
	le_diff->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_diff->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	connect(le_diff, SIGNAL(textChanged(const QString &)), 
				SLOT(update_diff(const QString &)));	
	le_diff->setMinimumHeight(minHeight2);

	lbl_koff = new QLabel(tr(" K_off Rate Constant (1/sec):"), this);
	lbl_koff->setAlignment(AlignLeft|AlignVCenter);
	lbl_koff->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_koff->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_koff->setMinimumHeight(minHeight2);

	le_koff = new QLineEdit(this, "Rate Constant Line Edit");
	le_koff->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_koff->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	connect(le_koff, SIGNAL(textChanged(const QString &)), 
				SLOT(update_koff(const QString &)));
	le_koff->setMinimumHeight(minHeight2);
	le_koff->setEnabled(false);

	lbl_f_f01 = new QLabel(tr(" Frictional Ratio (f/f0):"), this);
	lbl_f_f01->setAlignment(AlignLeft|AlignVCenter);
	lbl_f_f01->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_f_f01->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_f_f01->setMinimumHeight(minHeight2);

	lbl_f_f02 = new QLabel(this);
	lbl_f_f02->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_f_f02->setAlignment(AlignLeft|AlignVCenter);
	lbl_f_f02->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_f_f02->setMinimumHeight(minHeight2);
	lbl_f_f02->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	lbl_vbar1 = new QLabel(tr(" Vbar (ccm/g):"), this);
	lbl_vbar1->setAlignment(AlignLeft|AlignVCenter);
	lbl_vbar1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_vbar1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_vbar1->setMinimumHeight(minHeight2);

	lbl_vbar2 = new QLabel(this);
	lbl_vbar2->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_vbar2->setAlignment(AlignLeft|AlignVCenter);
	lbl_vbar2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_vbar2->setMinimumHeight(minHeight2);
	lbl_vbar2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	lbl_mw1 = new QLabel(tr(" Molecular Weight (Da):"), this);
	lbl_mw1->setAlignment(AlignLeft|AlignVCenter);
	lbl_mw1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_mw1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_mw1->setMinimumHeight(minHeight2);

	lbl_mw2 = new QLabel(this);
	lbl_mw2->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_mw2->setAlignment(AlignLeft|AlignVCenter);
	lbl_mw2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_mw2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	lbl_mw2->setMinimumHeight(minHeight2);

	lbl_stoich = new QLabel(tr(" Stoichiometry:"), this);
	lbl_stoich->setAlignment(AlignLeft|AlignVCenter);
	lbl_stoich->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_stoich->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_stoich->setMinimumHeight(minHeight2);

	le_stoich = new QLineEdit(this, "Stoichiometry");
	le_stoich->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_stoich->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_stoich->setMinimumHeight(minHeight2);
	le_stoich->setEnabled(false);
	le_stoich->setReadOnly(true);

	pb_simulateComponent = new QPushButton( tr("Simulate s and D"), this );
	pb_simulateComponent->setAutoDefault(false);
	pb_simulateComponent->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_simulateComponent->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	connect(pb_simulateComponent, SIGNAL(clicked()), SLOT(simulate_component()) );
	pb_simulateComponent->setMinimumHeight(minHeight1);

	cb_prolate = new QCheckBox("Prolate Ellipsoid", this);
	cb_prolate->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_prolate->setChecked(false);
	cb_prolate->setEnabled(false);
	cb_prolate->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_prolate->setMinimumHeight(minHeight2);
	connect(cb_prolate, SIGNAL(clicked()), SLOT(select_prolate()));

	cb_oblate = new QCheckBox("Oblate Ellipsoid", this);
	cb_oblate->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_oblate->setEnabled(false);
	cb_oblate->setChecked(false);
	cb_oblate->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_oblate->setMinimumHeight(minHeight2);
	connect(cb_oblate, SIGNAL(clicked()), SLOT(select_oblate()));

	cb_rod = new QCheckBox("Long Rod", this);
	cb_rod->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_rod->setChecked(false);
	cb_rod->setEnabled(false);
	cb_rod->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_rod->setMinimumHeight(minHeight2);
	connect(cb_rod, SIGNAL(clicked()), SLOT(select_rod()));

	cb_sphere = new QCheckBox("Sphere", this);
	cb_sphere->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_sphere->setChecked(false);
	cb_sphere->setEnabled(false);
	cb_sphere->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_sphere->setMinimumHeight(minHeight2);
	connect(cb_sphere, SIGNAL(clicked()), SLOT(select_sphere()));

	lbl_sigma = new QLabel(tr(" Conc. Dependency of s (sigma):"), this);
	lbl_sigma->setAlignment(AlignLeft|AlignVCenter);
	lbl_sigma->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_sigma->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_sigma->setMinimumHeight(minHeight2);

	le_sigma = new QLineEdit(this, "Sigma Line Edit");
	le_sigma->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_sigma->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_sigma->setMinimumHeight(minHeight2);
	connect(le_sigma, SIGNAL(textChanged(const QString &)), 
				SLOT(update_sigma(const QString &)));
	le_sigma->setEnabled(false);

	lbl_delta = new QLabel(tr(" Conc. Dependency of D (delta):"), this);
	lbl_delta->setAlignment(AlignLeft|AlignVCenter);
	lbl_delta->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_delta->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_delta->setMinimumHeight(minHeight2);

	le_delta = new QLineEdit(this, "Delta Line Edit");
	le_delta->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_delta->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_delta->setMinimumHeight(minHeight2);
	connect(le_delta, SIGNAL(textChanged(const QString &)), 
				SLOT(update_delta(const QString &)));	
	le_delta->setEnabled(false);

	cnt_item= new QwtCounter(this);
	cnt_item->setRange(1, (*system).component_vector.size(), 1);
	cnt_item->setNumButtons(2);
	cnt_item->setValue(1);
	cnt_item->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_item->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	connect(cnt_item, SIGNAL(valueChanged(double)), SLOT(select_component(double)));

	pb_load_model = new QPushButton( tr("Load Model File"), this );
	pb_load_model->setAutoDefault(false);
	pb_load_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_load_model->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_load_model->setMinimumHeight(minHeight1);
	connect(pb_load_model, SIGNAL(clicked()), SLOT(load_model()) );

	pb_load_c0 = new QPushButton( tr("Load C0 File"), this );
	pb_load_c0->setAutoDefault(false);
	pb_load_c0->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_load_c0->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_load_c0->setMinimumHeight(minHeight1);
	pb_load_c0->setEnabled(false);
	connect(pb_load_c0, SIGNAL(clicked()), SLOT(load_c0()) );

	lbl_load_c0 = new QLabel(this);
	lbl_load_c0->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_load_c0->setAlignment(AlignLeft|AlignVCenter);
	lbl_load_c0->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_load_c0->setMinimumHeight(minHeight2);
	lbl_load_c0->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	lbl_load_c0->setText(tr("Nothing loaded"));

	pb_cancel = new QPushButton(tr("Cancel"), this );
	pb_cancel->setAutoDefault(false);
	pb_cancel->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_cancel->setMinimumHeight(minHeight2);
	connect(pb_cancel, SIGNAL(clicked()), SLOT(reject()));

	pb_save = new QPushButton(tr("Save Model"), this );
	pb_save->setAutoDefault(false);
	pb_save->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_save->setMinimumHeight(minHeight2);
	pb_save->setEnabled(true);
	connect(pb_save, SIGNAL(clicked()), SLOT(save_model()));

	pb_accept = new QPushButton(tr("Accept Model"), this );
	pb_accept->setAutoDefault(false);
	pb_accept->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_accept->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_accept->setMinimumHeight(minHeight2);
	pb_accept->setEnabled(true);
	connect(pb_accept, SIGNAL(clicked()), SLOT(accept_model()));

	pb_help = new QPushButton(tr("Help"), this );
	pb_help->setAutoDefault(false);
	pb_help->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setMinimumHeight(minHeight2);
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	setup_GUI();
	select_component((int) 0);
	
	global_Xpos += 30;
	global_Ypos += 30;

	move(global_Xpos, global_Ypos);
}

US_Component_Dialog::~US_Component_Dialog()
{
}

void US_Component_Dialog::closeEvent(QCloseEvent *e)
{
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
}

void US_Component_Dialog::setup_GUI()
{
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
	grid->addMultiCellWidget(pb_load_model, j, j, 0, 1, 0);
	grid->addWidget(pb_save, j, 2, 0);
	grid->addWidget(pb_accept, j, 3, 0);
/*
	for (unsigned int i=0; i<11; i++)
	{
		grid->setRowSpacing(i, minHeight1);
	}
*/
}

void US_Component_Dialog::load_model()
{
	QString fn = QFileDialog::getOpenFileName(USglobal->config_list.result_dir, "*.model.?? *.model-?.?? *model-??.??", 0);
	if ( !fn.isEmpty() ) 
	{
		load_model(fn);
	}
}

void US_Component_Dialog::load_model(const QString &fileName)
{

	QFile f(fileName);
	QString str;
	bool flag;
	unsigned int i, j;
	if (f.open(IO_ReadOnly | IO_Translate))
	{
		flag = true;
		QTextStream ts(&f);
		ts.readLine(); // FE, SA2D, COFS, SIM or GA
		(*system).description = ts.readLine();
		if ((*system).description.isNull())
		{
			printError(3);
			return;
		}
		str = ts.readLine();
		if (str.find("#", 0, true) == 0) // a new model has a comment line in the second line starting with "#"
		{
			float fval;
			ts >> fval; // UltraScan version
			ts.readLine(); // read rest of line
			ts >> str;
			ts.readLine(); // read rest of line
			if (str.isNull())
			{
				printError(0);
				return;
			}
			(*system).model = str.toInt();
			ts >> str;
			if (str.isNull())
			{
				printError(0);
				return;
			}
			ts.readLine();
			(*system).component_vector.resize(str.toInt());
			for (i=0; i<(*system).component_vector.size(); i++)
			{
				str = ts.readLine();
				if (str.isNull())
				{
					printError(0);
					return;
				}
				int pos = str.find("#", 0, true);
				str.truncate(pos);
				(*system).component_vector[i].name = str.stripWhiteSpace();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).component_vector[i].concentration = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).component_vector[i].s = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).component_vector[i].D = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).component_vector[i].sigma = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).component_vector[i].delta = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).component_vector[i].mw = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).component_vector[i].vbar20 = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).component_vector[i].shape = str;
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).component_vector[i].f_f0 = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).component_vector[i].extinction = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).component_vector[i].show_conc = (bool) str.toInt();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).component_vector[i].show_stoich = str.toInt();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).component_vector[i].show_keq = (bool) str.toInt();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).component_vector[i].show_koff = (bool) str.toInt();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).component_vector[i].show_component.resize(str.toUInt());
				for (j=0; j<(*system).component_vector[i].show_component.size(); j++)
				{
					ts >> str;
					if (str.isNull())
					{
						printError(0);
						return;
					}
					ts.readLine();
					(*system).component_vector[i].show_component[j] = str.toInt();
				}
				if ((*system).component_vector[i].concentration < 0)
				{
					(*system).component_vector[i].c0.radius.clear();
					(*system).component_vector[i].c0.concentration.clear();
					ts >> str;
					if (str.isNull())
					{
						printError(0);
						return;
					}
					ts.readLine();
					unsigned int ival = str.toUInt();
					for (j=0; j<ival; j++)
					{
						ts >> str;
						if (str.isNull())
						{
							printError(0);
							return;
						}
						(*system).component_vector[i].c0.radius.push_back(str.toDouble());
						ts >> str;
						if (str.isNull())
						{
							printError(0);
							return;
						}
						(*system).component_vector[i].c0.concentration.push_back(str.toDouble());
					}
					ts.readLine(); //read the rest of the last linee
				}			}
			ts >> str;
			if (str.isNull())
			{
				printError(0);
				return;
			}
			ts.readLine();
			(*system).assoc_vector.resize(str.toUInt());
			for (i=0; i<(*system).assoc_vector.size(); i++)
			{
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).assoc_vector[i].keq = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).assoc_vector[i].units = str;
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).assoc_vector[i].k_off = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).assoc_vector[i].component1 = str.toInt();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).assoc_vector[i].component2 = str.toInt();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).assoc_vector[i].component3 = str.toInt();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).assoc_vector[i].stoichiometry1 = str.toUInt();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).assoc_vector[i].stoichiometry2 = str.toUInt();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).assoc_vector[i].stoichiometry3 = str.toUInt();
			}
		}
		else // load an old-style model file for noninteracting models
		{
			(*system).model = str.toInt();
			if ((*system).model > 3) // we can only read noninteracting models
			{
				printError(1);
				return;
			}
			(*system).model = 3; // set to fixed molecular weight distribution by default
			str = ts.readLine();
			if (str.isNull())
			{
				printError(0);
				return;
			}
			(*system).component_vector.resize(str.toInt()); // number of components
			for (i=0; i<(*system).component_vector.size(); i++)
			{
				str = ts.readLine();
				if (str.isNull())
				{
					printError(0);
					return;
				}
				(*system).component_vector[i].concentration = str.toFloat();
				str = ts.readLine();
				if (str.isNull())
				{
					printError(0);
					return;
				}
				(*system).component_vector[i].s = str.toFloat();
				str = ts.readLine();
				if (str.isNull())
				{
					printError(0);
					return;
				}
				(*system).component_vector[i].D = str.toFloat();
				str = ts.readLine();
				if (str.isNull())
				{
					printError(0);
					return;
				}
				(*system).component_vector[i].sigma = str.toFloat();
				str = ts.readLine();
				if (str.isNull())
				{
					printError(0);
					return;
				}
				(*system).component_vector[i].delta = str.toFloat();
				(*system).component_vector[i].vbar20 = 0.72;
				(*system).component_vector[i].extinction = 1.0;
				(*system).component_vector[i].name = str.sprintf("Component %d", i+1);
				
				(*system).component_vector[i].mw = ((*system).component_vector[i].s/(*system).component_vector[i].D)
				*((R*K20)/(1.0 - (*system).component_vector[i].vbar20 * DENS_20W));
				
				(*system).component_vector[i].f_f0 = (((*system).component_vector[i].mw * 
				(1.0 - (*system).component_vector[i].vbar20 * DENS_20W))/((*system).component_vector[i].s * AVOGADRO))
				/(6 * VISC_20W * pow(((*system).component_vector[i].mw * M_PI * M_PI * 3.0 
				* (*system).component_vector[i].vbar20)/(4.0 * AVOGADRO), 1.0/3.0));
				(*system).component_vector[i].show_conc = true;
				(*system).component_vector[i].show_keq = false;
				(*system).component_vector[i].show_koff = false;
				(*system).component_vector[i].show_stoich = 0;
			}
		}
		lbl_model->setText(modelString[(*system).model]);
		cmb_component1->clear();
		cmb_component2->clear();
		for (unsigned int i=0; i<(*system).component_vector.size(); i++)
		{
			cmb_component1->insertItem((*system).component_vector[i].name);
		}
		cnt_item->setValue(1);
		select_component((int) 0);
		printError(4);
	}
	else
	{
		printError(2);
	}
}

void US_Component_Dialog::load_c0()
{
	QMessageBox::information(this, tr("UltraScan Information"),
	tr("Please note:\n\nThe initial concentration file should have\n"
		"the following format:\n\n"
		"radius_value1 concentration_value1\n"
		"radius_value2 concentration_value2\n"
		"radius_value3 concentration_value3\n"
		"etc...\n\n"
		"radius values smaller than the meniscus or\n"
		"larger than the bottom of the cell will be\n"
		"excluded from the concentration vector."),
	QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
	QString fn = QFileDialog::getOpenFileName(USglobal->config_list.result_dir, "*", 0);
	if (!fn.isEmpty()) 
	{
		lbl_load_c0->setText(fn);
		QFile f;
		f.setName(fn);
		if (f.open(IO_ReadOnly | IO_Translate))
		{
			c0_file = fn;
			QTextStream ts(&f);
			(*system).component_vector[current_component].c0.radius.clear();
			(*system).component_vector[current_component].c0.concentration.clear();
			le_conc->setText("from file"); //sets concentration for this component to -1 to signal that we are using a concentration vector
			double val1, val2;
			QString line;
			while (!ts.atEnd())
			{
				ts >> val1;
				ts >> val2;
				if (val1 > 0.0) // ignore radius pairs that aren't positive
				{
					(*system).component_vector[current_component].c0.radius.push_back(val1);
					(*system).component_vector[current_component].c0.concentration.push_back(val2);
				}
			}
			f.close();
		}
		else
		{
			QMessageBox::warning(this, tr("UltraScan Warning"),
			tr("Please note:\n\nUltraScan could not open the file with the initial concentration data!\nThe file:\n\n" + USglobal->config_list.root_dir + "/mesh.dat\n\ncould not be opened."),
			QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		}
//		for (unsigned int i=0; i<(*simparams).c0.radius.size(); i++)
//		{
//			cout << i << ": " << (*simparams).c0.radius[i] << ", " << (*simparams).c0.concentration[i] << endl;
//		}
	}
}

void US_Component_Dialog::simulate_component()
{
	US_Hydro1 *hydro1;
	hydro1 = new US_Hydro1(&simcomp);
	connect(hydro1, SIGNAL(updated()), SLOT(update_component()));
	hydro1->exec();
	cb_prolate->setEnabled(true);
	cb_oblate->setEnabled(true);
	cb_rod->setEnabled(true);
	cb_sphere->setEnabled(true);
}

void US_Component_Dialog::select_prolate()
{
	shape = 1;
	cb_prolate->setChecked(true);
	cb_oblate->setChecked(false);
	cb_rod->setChecked(false);
	cb_sphere->setChecked(false);
	update_component();
}

void US_Component_Dialog::select_oblate()
{
	shape = 2;
	cb_prolate->setChecked(false);
	cb_oblate->setChecked(true);
	cb_rod->setChecked(false);
	cb_sphere->setChecked(false);
	update_component();
}

void US_Component_Dialog::select_rod()
{
	shape = 3;
	cb_prolate->setChecked(false);
	cb_oblate->setChecked(false);
	cb_rod->setChecked(true);
	cb_sphere->setChecked(false);
	update_component();
}

void US_Component_Dialog::select_sphere()
{
	shape = 4;
	cb_prolate->setChecked(false);
	cb_oblate->setChecked(false);
	cb_rod->setChecked(false);
	cb_sphere->setChecked(true);
	update_component();
}

void US_Component_Dialog::update_component()
{
	switch(shape)
	{
		case 1:
		{
			(*system).component_vector[current_component].s = simcomp.prolate.sedcoeff;
			(*system).component_vector[current_component].D = simcomp.prolate.diffcoeff;
			(*system).component_vector[current_component].f_f0 = simcomp.prolate.f_f0;
			(*system).component_vector[current_component].shape = "prolate";
			break;
		}
		case 2:
		{
			(*system).component_vector[current_component].s = simcomp.oblate.sedcoeff;
			(*system).component_vector[current_component].D = simcomp.oblate.diffcoeff;
			(*system).component_vector[current_component].f_f0 = simcomp.oblate.f_f0;
			(*system).component_vector[current_component].shape = "oblate";
			break;
		}
		case 3:
		{
			(*system).component_vector[current_component].s = simcomp.rod.sedcoeff;
			(*system).component_vector[current_component].D = simcomp.rod.diffcoeff;
			(*system).component_vector[current_component].f_f0 = simcomp.rod.f_f0;
			(*system).component_vector[current_component].shape = "rod";
			break;
		}
		case 4:
		{
			(*system).component_vector[current_component].s = simcomp.sphere.sedcoeff;
			(*system).component_vector[current_component].D = simcomp.sphere.diffcoeff;
			(*system).component_vector[current_component].f_f0 = simcomp.sphere.f_f0;
			(*system).component_vector[current_component].shape = "sphere";
			break;
		}
	}
	(*system).component_vector[current_component].mw = simcomp.mw;
	(*system).component_vector[current_component].vbar20 = simcomp.vbar;
	select_component((int) current_component);
	cb_prolate->setEnabled(true);
	cb_oblate->setEnabled(true);
	cb_rod->setEnabled(true);
	cb_sphere->setEnabled(true);
}

void US_Component_Dialog::select_component(double val)
{
	int i = (int) (val - 1);
	select_component(i);
}

void US_Component_Dialog::select_component(int val)
{
	current_component = (unsigned int) val;
	cnt_item->setValue(current_component + 1);
	cmb_component1->setCurrentItem(current_component);
	cb_prolate->setEnabled(false);
	cb_oblate->setEnabled(false);
	cb_rod->setEnabled(false);
	cb_sphere->setEnabled(false);
	QString str;

	le_sed->setText(str.sprintf("%6.4e", (*system).component_vector[current_component].s));
	le_diff->setText(str.sprintf("%6.4e", (*system).component_vector[current_component].D));
	lbl_vbar2->setText(str.sprintf("%6.4e", (*system).component_vector[current_component].vbar20));
	lbl_mw2->setText(str.sprintf("%6.4e", (*system).component_vector[current_component].mw));
	lbl_f_f02->setText(str.sprintf("%6.4e", (*system).component_vector[current_component].f_f0));
	
	if((*system).component_vector[current_component].shape == "sphere")
	{
		cb_prolate->setChecked(false);
		cb_oblate->setChecked(false);
		cb_rod->setChecked(false);
		cb_sphere->setChecked(true);
	}
	else if ((*system).component_vector[current_component].shape == "prolate")
	{
		cb_prolate->setChecked(true);
		cb_oblate->setChecked(false);
		cb_rod->setChecked(false);
		cb_sphere->setChecked(false);
	}
	else if ((*system).component_vector[current_component].shape == "oblate")
	{
		cb_prolate->setChecked(false);
		cb_oblate->setChecked(true);
		cb_rod->setChecked(false);
		cb_sphere->setChecked(false);
	}
	else if ((*system).component_vector[current_component].shape == "rod")
	{
		cb_prolate->setChecked(false);
		cb_oblate->setChecked(false);
		cb_rod->setChecked(true);
		cb_sphere->setChecked(false);
	}
	else
	{
		cb_prolate->setChecked(false);
		cb_oblate->setChecked(false);
		cb_rod->setChecked(false);
		cb_sphere->setChecked(false);
	}
	// find the associated components for component val and enter them into the
	// linked component list:

	cmb_component2->clear();
	for (unsigned int i=0; i<(*system).component_vector[current_component].show_component.size(); i++)
	{
		cmb_component2->insertItem((*system).component_vector[(*system).component_vector[current_component].show_component[i]].name, 0);
	}
	if ((*system).component_vector[current_component].show_conc)
	{
		le_conc->setEnabled(true);
		pb_load_c0->setEnabled(true);
		if ((*system).component_vector[current_component].concentration == -1.0)
		{
			le_conc->setText("from file");
			lbl_load_c0->setText(c0_file);
		}
		else
		{
			le_conc->setText(str.sprintf("%6.4e", (*system).component_vector[current_component].concentration));
			(*system).component_vector[current_component].c0.radius.clear();
			(*system).component_vector[current_component].c0.concentration.clear();
		}
	}
	else
	{
		le_conc->setEnabled(false);
		le_conc->setText("");
		pb_load_c0->setEnabled(false);
		lbl_load_c0->setText("");
	}
	if ((*system).component_vector[current_component].show_keq)
	{
		for (unsigned int i=0; i<(*system).assoc_vector.size(); i++)
		{ // only check the dissociating species
			if ((*system).assoc_vector[i].component2 == (int) current_component
			||  (*system).assoc_vector[i].component3 == (int) current_component)
			{
				le_keq->setText(str.sprintf("%6.4e", (*system).assoc_vector[i].keq));
				le_keq->setEnabled(true);
				le_koff->setText(str.sprintf("%6.4e", (*system).assoc_vector[i].k_off));
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
	if ((*system).component_vector[current_component].show_stoich != 0)
	{
		if ((*system).component_vector[current_component].show_stoich > 0)
		{
			le_stoich->setEnabled(true);
			le_stoich->setText(str.sprintf("%d", (*system).component_vector[current_component].show_stoich));
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
}

void US_Component_Dialog::update_conc(const QString &newText)
{
	if (newText == "")
	{
		return;
	}
	if (newText == "from file")
	{
		(*system).component_vector[current_component].concentration = -1.0;
	}
	else
	{
		bool ok;
		float val;
		val = newText.toFloat(&ok);
		if (!ok)
		{
			printError(5);
		}
		else
		{
			(*system).component_vector[current_component].concentration = val;
			(*system).component_vector[current_component].c0.radius.clear();
			(*system).component_vector[current_component].c0.concentration.clear();
			lbl_load_c0->setText("");
		}
	}
}

void US_Component_Dialog::printError(const int &ival)
{
	switch (ival)
	{
		case 0:
		{
			QMessageBox::warning(this, tr("UltraScan Warning"),
			tr("Please note:\n\nThere was an error reading\nthe selected Model File!\n\nThis file appears to be corrupted"),
			QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			break;
		}
		case 1:
		{
			QMessageBox::warning(this, tr("UltraScan Warning"), 
			tr("Sorry, for old-style model files only\nnon-interacting model loading is supported.\n\n") +
			tr("Please recreate this model by clicking on:\n\n\"New Model\""),
			QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			break;
		}
		case 2:
		{
			QMessageBox::warning(this, tr("UltraScan Warning"), tr("Please note:\n\n") +
			tr("UltraScan could not open\nthe selected Model File!"),
			QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			break;
		}
		case 3:
		{
			QMessageBox::warning(this, tr("UltraScan Warning"),
			tr("Please note:\n\nUltraScan could not read\nthe selected Model File!"),
			QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			break;
		}
		case 4:
		{
			QMessageBox::information(this, tr("Simulation Module"), tr("Successfully loaded Model:\n\n")
			+ (*system).description, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			break;
		}
		case 5:
		{
			QMessageBox::warning(this, tr("UltraScan Warning"),
			tr("Please note:\n\nYou provided an invalid entry!\n\nPlease try again..."),
			QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			break;
		}
	}
}

void US_Component_Dialog::update_sed(const QString &newText)
{
	if (newText == "")
	{
		return;
	}
	(*system).component_vector[current_component].s = newText.toFloat();
}

void US_Component_Dialog::update_diff(const QString &newText)
{
	if (newText == "")
	{
		return;
	}
	(*system).component_vector[current_component].D = newText.toFloat();
}

void US_Component_Dialog::update_sigma(const QString &newText)
{
	if (newText == "")
	{
		return;
	}
	(*system).component_vector[current_component].sigma = newText.toFloat();
}

void US_Component_Dialog::update_keq(const QString &newText)
{
	if (newText == "")
	{
		return;
	}
	// check to see if the current component is a dissociation component
	for (unsigned int i=0; i<(*system).assoc_vector.size(); i++)
	{
		// check to see if there is an dissociation linked to this component
		if ((*system).assoc_vector[i].component2 == (int) current_component)
		{ 	
			// check to make sure this component is not an irreversible component
			if ((*system).assoc_vector[i].stoichiometry1 != (*system).assoc_vector[i].stoichiometry2 // self-association
			|| ((*system).assoc_vector[i].stoichiometry1 == 0 && (*system).assoc_vector[i].stoichiometry2 == 0))  // hetero-association
			{
				(*system).assoc_vector[i].keq = newText.toFloat();
			}
		}
	}
}

void US_Component_Dialog::update_koff(const QString &newText)
{
	if (newText == "")
	{
		return;
	}
	// check to see if the current component is a dissociation component
	for (unsigned int i=0; i<(*system).assoc_vector.size(); i++)
	{
		// check to see if there is an dissociation linked to this component
		if ((*system).assoc_vector[i].component2 == (int) current_component)
		{ 	
			// check to make sure this component is not an irreversible component 
			if ((*system).assoc_vector[i].stoichiometry1 != (*system).assoc_vector[i].stoichiometry2 // self-association
			|| ((*system).assoc_vector[i].stoichiometry1 == 0 && (*system).assoc_vector[i].stoichiometry2 == 0))  // hetero-association
			{
				(*system).assoc_vector[i].k_off = newText.toFloat();
			}
		}
	}
}

void US_Component_Dialog::update_delta(const QString &newText)
{
	if (newText == "")
	{
		return;
	}
	(*system).component_vector[current_component].delta = newText.toFloat();
}

void US_Component_Dialog::help()
{
	US_Help *online_help;
	online_help = new US_Help(this);
	online_help->show_help("manual/astfem_component.html");
}

void US_Component_Dialog::accept_model()
{
	if (!verify_model())
	{
		return;
	}
	this->accept();
}

bool US_Component_Dialog::verify_model()
{
	bool flag = true;
	QString str1, str2;
	for (unsigned int i=0; i<(*system).assoc_vector.size(); i++)
	{
		if ((*system).assoc_vector[i].stoichiometry2 > 0 && (*system).assoc_vector[i].stoichiometry3 != 1) // then we need to check if the MWs match
		{
			if (fabs	((*system).component_vector[(*system).assoc_vector[i].component2].mw
			 - ((*system).component_vector[(*system).assoc_vector[i].component1].mw * (*system).assoc_vector[i].stoichiometry2 / (*system).assoc_vector[i].stoichiometry1
				)) > 1.0) // MWs don't match within 1 dalton
			{
				str2.sprintf(tr("The molecular weights of the reacting species\nin reaction %d do not agree:\n\n"), i+1);
				str1 = str2;
				str2.sprintf(tr("Molecular weight of species 1: %6.4e\n"), (*system).component_vector[(*system).assoc_vector[i].component1].mw);
				str1 += str2;
				str2.sprintf(tr("Molecular weight of species 2: %6.4e\n"), (*system).component_vector[(*system).assoc_vector[i].component2].mw);
				str1 += str2;
				str2.sprintf(tr("Stoichiometry of reaction %d: MW(1) * %d = MW(2)\n\n"), i+1, (*system).assoc_vector[i].stoichiometry2);
				str1 += str2;
				str1 += tr("Please adjust either MW(1) or MW(2) before proceeding...");
				QMessageBox::warning(this, "Model Definition Error", str1, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
				flag = false;
			}
		}
		if ((*system).assoc_vector[i].stoichiometry3 == 1) // then we need to check if the sum of MW(1) + MW(2) = MW(3)
		{
			if (fabs	((*system).component_vector[(*system).assoc_vector[i].component3].mw
			 - (*system).component_vector[(*system).assoc_vector[i].component2].mw 
			 - (*system).component_vector[(*system).assoc_vector[i].component1].mw) > 1.0) // MWs don't match within 10 dalton
			{
				str2.sprintf(tr("The molecular weights of the reacting species\nin reaction %d do not agree:\n\n"), i+1);
				str1 = str2;
				str2.sprintf(tr("Molecular weight of species 1: %6.4e\n"), (*system).component_vector[(*system).assoc_vector[i].component1].mw);
				str1 += str2;
				str2.sprintf(tr("Molecular weight of species 2: %6.4e\n"), (*system).component_vector[(*system).assoc_vector[i].component2].mw);
				str1 += str2;
				str2.sprintf(tr("Molecular weight of species 3: %6.4e\n"), (*system).component_vector[(*system).assoc_vector[i].component3].mw);
				str1 += str2;
				str2.sprintf(tr("Stoichiometry of reaction %d: MW(1) + MW(2) = MW(3)\n\n"), i+1, (*system).assoc_vector[i].stoichiometry2);
				str1 += str2;
				str1 += tr("Please adjust the molecular weight of the appropriate\ncomponent before proceeding...");
				QMessageBox::warning(this, "Model Definition Error", str1, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
				flag = false;
			}
		}
	}
	return (flag);
}

void US_Component_Dialog::save_model()
{
	if (!verify_model())
	{
		return;
	}
	savefile();
}

void US_Component_Dialog::savefile()
{
	QString str, fn = QFileDialog::getSaveFileName(USglobal->config_list.result_dir, "*.model.?? *.model-?.?? *model-??.??", 0);
	int k;
	
	if ( !fn.isEmpty() ) 
	{
		k = fn.findRev(".", -1, false);
		if (k != -1) 	//if an extension was given, strip it.
		{
			fn.truncate(k);
		}
		fn.append(str.sprintf(".model-%d.11", (*system).model));	
		savefile(fn);		// the user gave a file name
	}
}

void US_Component_Dialog::savefile(const QString &fileName)
{
	QFile f(fileName);
	QString str;
	unsigned int i, j;
	if (f.exists())
	{
		if(!QMessageBox::query( tr("Warning"), tr("Attention:\nThis file exists already!\n\nDo you want to overwrite it?"), tr("Yes"), tr("No")))
		{
			f.close();
			return;
		}
	}
	if (f.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&f);
		QString message = tr("Please enter a description\nfor your model:");
		OneLiner ol_descr(message);
		ol_descr.parameter1->setText((*system).description);
		ol_descr.show();
		if (ol_descr.exec())
		{
			if (ol_descr.string.isEmpty())
			{
				ol_descr.string = "not specified";
			}
			ts << "SIM" << "\n";
			ts << ol_descr.string << "\n";
			ts << "# This file is computer-generated, please do not edit unless you know what you are doing\n";
		}
		ts << US_Version << "\t\t# UltraScan Version Number\n";
		ts << (*system).model << "\t\t# model number/identifier\n";
		ts << (*system).component_vector.size() << "\t\t# number of components in the model\n";
		for (i=0; i<(*system).component_vector.size(); i++)
		{
			ts << (*system).component_vector[i].name << "\t\t# name of component\n";
			ts << (*system).component_vector[i].concentration << "\t\t# concentration\n";
			ts << (*system).component_vector[i].s << "\t\t# sedimentation coefficient\n";
			ts << (*system).component_vector[i].D << "\t\t# diffusion coefficient\n";
			ts << (*system).component_vector[i].sigma << "\t\t# sigma\n";
			ts << (*system).component_vector[i].delta << "\t\t# delta\n";
			ts << (*system).component_vector[i].mw << "\t\t# molecular Weight \n";
			ts << (*system).component_vector[i].vbar20 << "\t\t# vbar at 20C \n";
			ts << (*system).component_vector[i].shape << "\t\t# shape \n";
			ts << (*system).component_vector[i].f_f0 << "\t\t# frictional ratio \n";
			ts << (*system).component_vector[i].extinction << "\t\t# extinction\n";
			ts << (int) (*system).component_vector[i].show_conc << "\t\t# show concentration?\n";
			ts << (*system).component_vector[i].show_stoich << "\t\t# show Stoichiometry?\n";
			ts << (int) (*system).component_vector[i].show_keq << "\t\t# show k equilibrium?\n";
			ts << (int) (*system).component_vector[i].show_koff << "\t\t# show k_off?\n";
			ts << (*system).component_vector[i].show_component.size() << "\t\t# number of linked components\n";
			for (j=0; j<(*system).component_vector[i].show_component.size(); j++)
			{
				ts << (*system).component_vector[i].show_component[j] << str.sprintf("\t\t# linked component (%d)\n", j+1);
			}
			if((*system).component_vector[i].concentration < 0)
			{
				ts << (*system).component_vector[i].c0.radius.size() << "\t\t# number of initial concentration points\n";
				for (j=0; j<(*system).component_vector[i].c0.radius.size(); j++)
				{
					ts << (*system).component_vector[i].c0.radius[j] << " " 
					<< (*system).component_vector[i].c0.concentration[j] << endl;
				}
			}
		}
		ts << (*system).assoc_vector.size() << "\t\t# number of association reactions in the model\n";
		for (i=0; i<(*system).assoc_vector.size(); i++)
		{
			ts << (*system).assoc_vector[i].keq << "\t\t# equilibrium constant\n";
			ts << (*system).assoc_vector[i].units << "\t\t# units for equilibrium constant\n";
			ts << (*system).assoc_vector[i].k_off << "\t\t# rate constant\n";
			ts << (*system).assoc_vector[i].component1 << "\t\t# component 1 in this association\n";
			ts << (*system).assoc_vector[i].component2 << "\t\t# component 2 in this association\n";
			ts << (*system).assoc_vector[i].component3 << "\t\t# component 3 in this association\n";
			ts << (*system).assoc_vector[i].stoichiometry1 << "\t\t# stoichiometry for component 1 in this association\n";
			ts << (*system).assoc_vector[i].stoichiometry2 << "\t\t# stoichiometry for component 2 in this association\n";
			ts << (*system).assoc_vector[i].stoichiometry3 << "\t\t# stoichiometry for component 3 in this association\n";
		}
		f.close();
	}
}
