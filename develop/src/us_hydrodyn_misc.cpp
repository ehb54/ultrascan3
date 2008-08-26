#include "../include/us_hydrodyn_misc.h"

US_Hydrodyn_Misc::US_Hydrodyn_Misc(struct misc_options *misc,
bool *misc_widget, QWidget *p, const char *name) : QFrame(p, name)
{
	this->misc = misc;
	this->misc_widget = misc_widget;
	*misc_widget = true;
	USglobal=new US_Config();
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	setCaption(tr("Misceallaneous SOMO Options"));
	setupGUI();
	global_Xpos += 30;
	global_Ypos += 30;
	setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_Misc::~US_Hydrodyn_Misc()
{
	*misc_widget = false;
}

void US_Hydrodyn_Misc::setupGUI()
{
	int minHeight1 = 30;
	
	lbl_info = new QLabel(tr("Miscellaneous SOMO Options:"), this);
	Q_CHECK_PTR(lbl_info);
	lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_info->setAlignment(AlignCenter|AlignVCenter);
	lbl_info->setMinimumHeight(minHeight1);
	lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	lbl_hydrovol = new QLabel(tr(" Hydration Water Vol. (A^3): "), this);
	Q_CHECK_PTR(lbl_hydrovol);
	lbl_hydrovol->setAlignment(AlignLeft|AlignVCenter);
	lbl_hydrovol->setMinimumWidth(200);
	lbl_hydrovol->setMinimumHeight(minHeight1);
	lbl_hydrovol->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_hydrovol->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	cnt_hydrovol= new QwtCounter(this);
	Q_CHECK_PTR(cnt_hydrovol);
	cnt_hydrovol->setRange(0, 100, 0.001);
	cnt_hydrovol->setValue((*misc).hydrovol);
	cnt_hydrovol->setMinimumHeight(minHeight1);
	cnt_hydrovol->setMinimumWidth(200);
	cnt_hydrovol->setEnabled(true);
	cnt_hydrovol->setNumButtons(3);
	cnt_hydrovol->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_hydrovol->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_hydrovol, SIGNAL(valueChanged(double)), SLOT(update_hydrovol(double)));

	lbl_vbar = new QLabel(tr(" Enter a vbar value: "), this);
	Q_CHECK_PTR(lbl_vbar);
	lbl_vbar->setAlignment(AlignLeft|AlignVCenter);
	lbl_vbar->setMinimumHeight(minHeight1);
	lbl_vbar->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	cb_vbar = new QCheckBox(this);
	cb_vbar->setText(tr(" Calculate vbar "));
	cb_vbar->setChecked((*misc).compute_vbar);
	cb_vbar->setMinimumHeight(minHeight1);
	cb_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_vbar->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_vbar, SIGNAL(clicked()), SLOT(set_vbar()));

	pb_vbar = new QPushButton(tr("Select vbar"), this);
	Q_CHECK_PTR(pb_vbar);
	pb_vbar->setEnabled(!(*misc).compute_vbar);
	pb_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_vbar->setMinimumHeight(minHeight1);
	pb_vbar->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_vbar, SIGNAL(clicked()), SLOT(select_vbar()));

	le_vbar = new QLineEdit(this, "vbar Line Edit");
	le_vbar->setMinimumHeight(minHeight1);
	le_vbar->setEnabled(!(*misc).compute_vbar);
	le_vbar->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	connect(le_vbar, SIGNAL(textChanged(const QString &)), SLOT(update_vbar(const QString &)));

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
	background->addWidget(cb_vbar, j, 0);
	background->addWidget(pb_vbar, j, 1);
	j++;
	background->addWidget(lbl_vbar, j, 0);
	background->addWidget(le_vbar, j, 1);
	j++;
	background->addWidget(lbl_hydrovol, j, 0);
	background->addWidget(cnt_hydrovol, j, 1);
	j++;
	background->addWidget(pb_help, j, 0);
	background->addWidget(pb_cancel, j, 1);
}

void US_Hydrodyn_Misc::update_vbar(const QString &str)
{
	(*misc).vbar = str.toDouble();
}

void US_Hydrodyn_Misc::update_vbar_signal(float val1, float val2)
{
	QString str;
	float vbar20;
	vbar20 = val1;
	(*misc).vbar = val2;
	le_vbar->setText(str.sprintf("%7.4f", (*misc).vbar));
}

void US_Hydrodyn_Misc::select_vbar()
{
	float val;
	vbar_dlg = new US_Vbar_DB(20.0, &val, &val, true, false, 0);
	vbar_dlg->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	vbar_dlg->setCaption(tr("V-bar Calculation"));
	connect(vbar_dlg, SIGNAL(valueChanged(float, float)), SLOT(update_vbar_signal(float, float)));
	vbar_dlg->exec();
}

void US_Hydrodyn_Misc::set_vbar()
{
	(*misc).compute_vbar = cb_vbar->isChecked();
	le_vbar->setEnabled(!(*misc).compute_vbar);
	pb_vbar->setEnabled(!(*misc).compute_vbar);
}

void US_Hydrodyn_Misc::cancel()
{
	close();
}

void US_Hydrodyn_Misc::help()
{
	US_Help *online_help;
	online_help = new US_Help(this);
	online_help->show_help("manual/somo_misc.html");
}

void US_Hydrodyn_Misc::update_hydrovol(double val)
{
	(*misc).hydrovol = val;
}

void US_Hydrodyn_Misc::closeEvent(QCloseEvent *e)
{
	*misc_widget = false;
	global_Xpos -= 30;
	global_Ypos -= 30;
	e->accept();
}

