#include "../include/us_finite2.h"

US_SetModel::US_SetModel(unsigned int *temp_components, int *temp_model, QWidget *parent, const char *name) 
: QDialog(parent, name, true)
{
	components = temp_components;
	model = temp_model;
	
	USglobal = new US_Config();
	QString str;
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	int xpos = 5;
	int ypos = 5;
	int buttonw = 300;
	int buttonh = 26;

	setCaption(tr("Model Selection"));
	lbl_model = new QLabel(tr(" Select a Model:"), this);
	lbl_model->setAlignment(AlignCenter|AlignVCenter);
	lbl_model->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	lbl_model->setGeometry(xpos, ypos, buttonw, buttonh);

	ypos += buttonh + spacing;

	lb_model = new QListBox(this, "Model");
	lb_model->insertItem( str.sprintf(tr("Non-Interacting")) );
	lb_model->insertItem( str.sprintf(tr("Monomer-Dimer Self Association")) );
	lb_model->insertItem( str.sprintf(tr("Isomerization")) );
	lb_model->setGeometry(xpos, ypos, buttonw, 55);
	lb_model->setCurrentItem(*model);
	lb_model->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	lb_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	connect(lb_model, SIGNAL(highlighted(int)), SLOT(select_model(int)));

	ypos += 55 + 2 * spacing;

	lbl_comp = new QLabel(tr(" Number of Components:"), this);
	lbl_comp->setAlignment(AlignLeft|AlignVCenter);
	lbl_comp->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_comp->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_comp->setGeometry(xpos, ypos, buttonw, buttonh+4);

	ct_comp= new QwtCounter(this);
	ct_comp->setNumButtons(2);
	ct_comp->setRange(1, 100, 1);
	ct_comp->setValue(*components);
	ct_comp->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	ct_comp->setGeometry((unsigned int) (3*buttonw/5)-5, ypos+2, (unsigned int) (2*buttonw/5)+5, buttonh);
//	connect(ct_comp, SIGNAL(buttonReleased(double)), SLOT(assign_component(double)));
	connect(ct_comp, SIGNAL(valueChanged(double)), SLOT(assign_component(double)));

	ypos += buttonh + 4 + 2 * spacing;

	pb_ok = new QPushButton( tr("Ok"), this );
	pb_ok->setAutoDefault(false);
	pb_ok->setGeometry(xpos, ypos, (unsigned int) (buttonw/3) - 1, buttonh);
	pb_ok->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_ok->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	connect(pb_ok, SIGNAL(clicked()), SLOT(accept()));

	xpos += (unsigned int) (buttonw/3) + 1;

	pb_cancel = new QPushButton( tr("Cancel"), this );
	pb_cancel->setGeometry(xpos, ypos, (unsigned int) (buttonw/3) - 2, buttonh);
	pb_cancel->setAutoDefault(false);
	pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	connect(pb_cancel, SIGNAL(clicked()), SLOT(reject()) );

	xpos += (unsigned int) (buttonw/3) + 1;

	pb_help = new QPushButton( tr("Help"), this );
	pb_help->setAutoDefault(false);
	pb_help->setGeometry(xpos, ypos, (unsigned int) (buttonw/3) - 1, buttonh);
	pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	connect(pb_help, SIGNAL(clicked()), SLOT(help()) );
}

US_SetModel::~US_SetModel()
{
}

void US_SetModel::assign_component(double val)
{
	*components = (unsigned int) val;
}

void US_SetModel::select_model(int val)
{
	*model = val;
	if (*model == 1 || *model == 2)
	{
		*components = 2;
		ct_comp->setValue(*components);
		ct_comp->setRange(2, 2, 1);
	}
	else
	{
		*components = 1;
		ct_comp->setRange(1, 100, 1);
		ct_comp->setValue(1);
	}
}

void US_SetModel::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/finite_model.html");
}
