#include "../include/us_clipdata.h"

US_ClipData::US_ClipData(float *temp_conc, float *temp_rad, float temp_meniscus, float temp_loading, QWidget *p, const char *name) : QDialog(p, name, true)
{
	int buttonh	= 26;
	int buttonw	= 200;
	int border	= 4;
	int spacing	= 2;
	int width	= 402;
	int xpos		= border;
	int ypos		= border;
	QString str;
	conc	= temp_conc;
	rad	= temp_rad;
	meniscus = temp_meniscus;
	loading = temp_loading;

	USglobal = new US_Config();


	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	setCaption(tr("Data Range Selection - UltraScan Finite Element Simulation"));

	lbl_info = new QLabel(tr("Please select the Cropping Range\nfor the Data to be saved:"), this);
	Q_CHECK_PTR(lbl_info);
	lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_info->setAlignment(AlignCenter|AlignVCenter);
	lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_info->setGeometry(xpos, ypos, width, 2 * buttonh + spacing);
	lbl_info->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	
	ypos += 2 + 2 * buttonh + 2 * spacing;
	xpos = border;

	lbl_conc = new QLabel(tr("Maximum Concentration\n(0 = entire range):"),this);
	lbl_conc->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_conc->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_conc->setGeometry(xpos, ypos, buttonw, buttonh*2);
	lbl_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	xpos += buttonw + spacing;

	str.sprintf(tr("(Loading conc.: %4.2f)"), loading);
	lbl_loading = new QLabel(str, this);
	lbl_loading->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_loading->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_loading->setGeometry(xpos, ypos, buttonw, buttonh);
	lbl_loading->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	ypos += buttonh;

	ct_conc= new QwtCounter(this);
	Q_CHECK_PTR(ct_conc);
	ct_conc->setRange(loading, (*conc), 0.1);
	ct_conc->setNumButtons(3);
	ct_conc->setValue(2.0*loading);
	(*conc) = 2.0*loading;
	ct_conc->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	ct_conc->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(ct_conc, SIGNAL(valueChanged(double)), SLOT(update_conc(double)));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_rad = new QLabel(tr("Maximum Radius:"),this);
	lbl_rad->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_rad->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_rad->setGeometry(xpos, ypos, buttonw, buttonh);
	lbl_rad->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	xpos += buttonw + spacing;

	ct_rad= new QwtCounter(this);
	Q_CHECK_PTR(ct_rad);
	ct_rad->setRange(meniscus, (*rad), 0.01);
	ct_rad->setNumButtons(3);
	ct_rad->setValue((*rad));
	ct_rad->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	ct_rad->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(ct_rad, SIGNAL(valueChanged(double)), SLOT(update_rad(double)));

	ypos += 2 + buttonh + spacing;
	xpos = border;

	pb_ok = new QPushButton(tr("OK"), this);
	Q_CHECK_PTR(pb_ok);
	pb_ok->setAutoDefault(false);
	pb_ok->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_ok->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_ok->setGeometry(xpos, ypos, (int) (width-4)/3, buttonh);
	connect(pb_ok, SIGNAL(clicked()), SLOT(check()));

	xpos += (int) (width-4)/3 + spacing;

	pb_cancel = new QPushButton(tr("Cancel"), this);
	Q_CHECK_PTR(pb_cancel);
	pb_cancel->setAutoDefault(false);
	pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_cancel->setGeometry(xpos, ypos, (int) (width-4)/3, buttonh);
	connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

	xpos += (int) (width-4)/3 + spacing;

	pb_help = new QPushButton(tr("Help"), this);
	Q_CHECK_PTR(pb_help);
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_help->setGeometry(xpos, ypos, (int) (width-4)/3, buttonh);
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	ypos += buttonh + border;

	global_Xpos += 30;
	global_Ypos += 30;
	
	setMinimumSize(width+8, ypos);
	setGeometry(global_Xpos, global_Ypos, width+8, ypos);
}

US_ClipData::~US_ClipData()
{
}

void US_ClipData::closeEvent(QCloseEvent *e)
{
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
}

void US_ClipData::cancel()
{
	*conc = 2;
	reject();
}

void US_ClipData::check()
{
	accept();
}

void US_ClipData::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/equil2.html");
}

void US_ClipData::update_conc(double val)
{
	*conc = (float) val;
}

void US_ClipData::update_rad(double val)
{
	*rad = (float) val;
}
