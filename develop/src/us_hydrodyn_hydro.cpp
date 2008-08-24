#include "../include/us_hydrodyn_hydro.h"

US_Hydrodyn_Hydro::US_Hydrodyn_Hydro(struct hydro_options *hydro,
bool *hydro_widget, QWidget *p, const char *name) : QFrame(p, name)
{
	this->hydro = hydro;
	this->hydro_widget = hydro_widget;
	*hydro_widget = true;
	USglobal=new US_Config();
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	setCaption(tr("SOMO Hydrodynamic Calculation Options"));
	setupGUI();
	global_Xpos += 30;
	global_Ypos += 30;
	setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_Hydro::~US_Hydrodyn_Hydro()
{
	*hydro_widget = false;
}

void US_Hydrodyn_Hydro::setupGUI()
{
	int minHeight1 = 30;
	
	lbl_info = new QLabel(tr("SOMO Hydrodynamic Calculation Options:"), this);
	Q_CHECK_PTR(lbl_info);
	lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_info->setAlignment(AlignCenter|AlignVCenter);
	lbl_info->setMinimumHeight(minHeight1);
	lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	pb_cancel = new QPushButton(tr("Close"), this);
	Q_CHECK_PTR(pb_cancel);
	pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_cancel->setMinimumHeight(minHeight1);
	pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

	pb_help = new QPushButton(tr("Help"), this);
	Q_CHECK_PTR(pb_help);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_help->setMinimumHeight(minHeight1);
	pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	int rows=6, columns = 2, spacing = 2, j=0, margin=4;
	QGridLayout *background=new QGridLayout(this, rows, columns, margin, spacing);

	background->addMultiCellWidget(lbl_info, j, j, 0, 1);
	j++;
	background->addWidget(pb_help, j, 0);
	background->addWidget(pb_cancel, j, 1);
}

void US_Hydrodyn_Hydro::cancel()
{
	close();
}

void US_Hydrodyn_Hydro::help()
{
	US_Help *online_help;
	online_help = new US_Help(this);
	online_help->show_help("manual/somo_hydro.html");
}

void US_Hydrodyn_Hydro::closeEvent(QCloseEvent *e)
{
	*hydro_widget = false;
	global_Xpos -= 30;
	global_Ypos -= 30;
	e->accept();
}

