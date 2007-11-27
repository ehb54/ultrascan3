#include "../include/us_infopanel.h"

US_InfoPanel::US_InfoPanel(int model, float *temp_limit, QWidget *p, const char *name) : QFrame( p, name)
{
	buttonh = 26;
	xpos = 0;
	ypos = 0;
	USglobal = new US_Config();

	
	limit = temp_limit;
	
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	
	lbl_status1 = new QLabel(tr("Status/Information:"), this);
	Q_CHECK_PTR(lbl_status1);
	lbl_status1->setAlignment(AlignCenter|AlignVCenter);
	lbl_status1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_status1->setGeometry(xpos, ypos, 120, buttonh);
	lbl_status1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	xpos += 120 + spacing;
	
	lbl_status2 = new QLabel("", this);
	Q_CHECK_PTR(lbl_status2);
	lbl_status2->setAlignment(AlignCenter|AlignVCenter);
	lbl_status2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_status2->setGeometry(xpos, ypos, 410, buttonh);
	lbl_status2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	xpos = 0;
	ypos += buttonh + spacing;

	lbl_model1 = new QLabel(tr("Current Model:"), this);
	Q_CHECK_PTR(lbl_model1);
	lbl_model1->setAlignment(AlignCenter|AlignVCenter);
	lbl_model1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_model1->setGeometry(xpos, ypos, 120, buttonh);
	lbl_model1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	xpos += 120 + spacing;

	lbl_model2 = new QLabel(tr("<none selected>"), this);
	Q_CHECK_PTR(lbl_model2);
	lbl_model2->setAlignment(AlignCenter|AlignVCenter);
	lbl_model2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_model2->setGeometry(xpos, ypos, 410, buttonh);
	lbl_model2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));


	ypos += buttonh + spacing;
	xpos = 0;

	lbl_max_od1 = new QLabel(tr("Max. OD/Fringe:"), this);
	Q_CHECK_PTR(lbl_max_od1);
	lbl_max_od1->setAlignment(AlignCenter|AlignVCenter);
	lbl_max_od1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_max_od1->setGeometry(xpos, ypos, 120, buttonh);
	lbl_max_od1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	xpos += 120 + spacing;
	
	le_max_od = new QLineEdit(this, "Limit Line Edit");
	le_max_od->setGeometry(xpos, ypos, 60, buttonh);
	le_max_od->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_max_od->setText("0.90");
	connect(le_max_od, SIGNAL(textChanged(const QString &)), 
				SLOT(update_limit(const QString &)));	

	xpos += 60 + spacing;
	
	lbl_max_od2 = new QLabel(tr("(set to zero to inactivate high conc. limits)"), this);
	Q_CHECK_PTR(lbl_max_od2);
	lbl_max_od2->setAlignment(AlignCenter|AlignVCenter);
	lbl_max_od2->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_max_od2->setGeometry(xpos, ypos, 348, buttonh);
	lbl_max_od2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	set_model(model);
	setup_GUI();
}

US_InfoPanel::~US_InfoPanel()
{
}

void US_InfoPanel::setup_GUI()
{
	int j=0;
	int rows = 2, columns = 4;
	
	QGridLayout * background = new QGridLayout(this,rows, columns,spacing);	
	for (int i=0; i<rows; i++)
	{
		background->setRowSpacing(i, 26);
	}
	background->addWidget(lbl_status1,j,0);
	background->addMultiCellWidget(lbl_status2,j,j,1,3);
	j++;
	background->addWidget(lbl_model1,j,0);
	background->addMultiCellWidget(lbl_model2,j,j,1,3);
	j++;
	QGridLayout *line3 = new QGridLayout(3,1,2);
	line3->addWidget(lbl_max_od1,0,0);
	line3->addWidget(le_max_od,0,1);
	line3->addWidget(lbl_max_od2,0,2);
	background->addMultiCell(line3,j,j,0,3);
}
void US_InfoPanel::set_model(int model)
{
	if (model == -1)
	{
		lbl_model2->setText(tr("-- none selected --"));
	}
	else
	{
		lbl_model2->setText(modelString[model]);
	}
}

void US_InfoPanel::update_limit(const QString &newText)
{
	*limit = newText.toFloat();
	emit limitChanged(*limit);
}

