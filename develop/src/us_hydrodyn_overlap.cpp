#include "../include/us_hydrodyn_overlap.h"

US_Hydrodyn_Overlap::US_Hydrodyn_Overlap(struct overlap_reduction *sidechain_overlap,
struct overlap_reduction *mainchain_overlap, struct overlap_reduction *buried_overlap,
double *overlap_tolerance, bool *overlap_widget, QWidget *p, const char *name) : QFrame(p, name)
{
	this->sidechain_overlap = sidechain_overlap;
	this->mainchain_overlap = mainchain_overlap;
	this->buried_overlap = buried_overlap;
	this->overlap_widget = overlap_widget;
	this->overlap_tolerance = overlap_tolerance;
	*overlap_widget = true;
	USglobal=new US_Config();
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	setCaption(tr("SOMO Bead Overlap Reduction Options"));
	setupGUI();
	global_Xpos += 30;
	global_Ypos += 30;
	setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_Overlap::~US_Hydrodyn_Overlap()
{
	*overlap_widget = false;
}

void US_Hydrodyn_Overlap::setupGUI()
{
	int minHeight1 = 30;

	sidechain_OR = new US_Hydrodyn_OR(sidechain_overlap, this);
	mainchain_OR = new US_Hydrodyn_OR(mainchain_overlap, this);
	buried_OR = new US_Hydrodyn_OR(buried_overlap, this);
	//buried_OR->cnt_fuse->setEnabled(false);
	//buried_OR->cb_fuse->setEnabled(false);

	lbl_info = new QLabel(tr("Bead Overlap Reduction Options:"), this);
	Q_CHECK_PTR(lbl_info);
	lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_info->setAlignment(AlignCenter|AlignVCenter);
	lbl_info->setMinimumHeight(minHeight1);
	lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	lbl_overlap_tolerance = new QLabel(tr(" Bead Overlap Tolerance: "), this);
	Q_CHECK_PTR(lbl_overlap_tolerance);
	lbl_overlap_tolerance->setAlignment(AlignLeft|AlignVCenter);
	lbl_overlap_tolerance->setMinimumHeight(minHeight1);
	lbl_overlap_tolerance->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_overlap_tolerance->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	cnt_overlap_tolerance= new QwtCounter(this);
	Q_CHECK_PTR(cnt_overlap_tolerance);
	cnt_overlap_tolerance->setRange(0, 1, 0.0001);
	cnt_overlap_tolerance->setValue(*overlap_tolerance);
	cnt_overlap_tolerance->setMinimumHeight(minHeight1);
	cnt_overlap_tolerance->setEnabled(true);
	cnt_overlap_tolerance->setNumButtons(3);
	cnt_overlap_tolerance->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_overlap_tolerance->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_overlap_tolerance, SIGNAL(valueChanged(double)), SLOT(update_overlap_tolerance(double)));

	tw_overlap = new QTabWidget(this);
	tw_overlap->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	tw_overlap->addTab(sidechain_OR, "Exposed Side chain beads");
	tw_overlap->addTab(mainchain_OR, "Exposed Main and side chain beads");
	tw_overlap->addTab(buried_OR, "Buried beads");

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

	int rows=11, columns = 2, spacing = 2, j=0, margin=4;
	QGridLayout *background=new QGridLayout(this, rows, columns, margin, spacing);

	background->addMultiCellWidget(lbl_info, j, j, 0, 1);
	j++;
	background->addWidget(lbl_overlap_tolerance, j, 0);
	background->addWidget(cnt_overlap_tolerance, j, 1);
	j++;
	background->addMultiCellWidget(tw_overlap, j, j+6, 0, 1);
	j+=8;
	background->addWidget(pb_help, j, 0);
	background->addWidget(pb_cancel, j, 1);
}

void US_Hydrodyn_Overlap::cancel()
{
	close();
}

void US_Hydrodyn_Overlap::help()
{
	US_Help *online_help;
	online_help = new US_Help(this);
	online_help->show_help("manual/somo_overlap.html");
}

void US_Hydrodyn_Overlap::closeEvent(QCloseEvent *e)
{
	*overlap_widget = false;
	global_Xpos -= 30;
	global_Ypos -= 30;
	e->accept();
}

void US_Hydrodyn_Overlap::update_overlap_tolerance(double val)
{
	*overlap_tolerance = val;
}

