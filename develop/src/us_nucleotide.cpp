#include "../include/us_nucleotide.h"

US_Nucleotide::US_Nucleotide(QWidget *p, const char *name) : QFrame(p, name)
{
	USglobal = new US_Config();
	
	int buttonh	= 26;
	int column1	= 28;
	int column2	= 150;
	int border	= 4;
	int spacing	= 2;
	int xpos		= border;
	int ypos		= border;
	int span		= column1 + 3 * column2 + 3 * spacing;
	int buttonw;
	complement = false;
	doubleStranded = true;
	_3prime_oh = true;
	_5prime_oh = false;
	isDNA = true;
	sodium = 0.0;
	potassium = 0.0;
	lithium = 0.0;
	calcium = 0.0;
	magnesium = 0.0;
	
	QString str;
	
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	setCaption(tr("Nucleotide Sequence MW Calculator"));
	lbl_banner1 = new QLabel(tr("Nucleotide Sequence Molecular Weight Calculator"), this);
	Q_CHECK_PTR(lbl_banner1);
	lbl_banner1->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_banner1->setAlignment(AlignCenter|AlignVCenter);
	lbl_banner1->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_banner1->setGeometry(xpos, ypos, span, buttonh);
	lbl_banner1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	
	ypos += 2 + buttonh + spacing;
	xpos = border;

	cb_doubleStranded = new QCheckBox(this);
	Q_CHECK_PTR(cb_doubleStranded);
	cb_doubleStranded->setGeometry(xpos+7, ypos+5, 14, 14);
	cb_doubleStranded->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_doubleStranded->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_doubleStranded->setChecked(doubleStranded);
	connect(cb_doubleStranded, SIGNAL(clicked()), SLOT(update_doubleStranded()));

	xpos += column1 + spacing;

	lbl_doubleStranded = new QLabel(tr("Double Stranded"),this);
	lbl_doubleStranded->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_doubleStranded->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_doubleStranded->setGeometry(xpos, ypos, column2 - 7, buttonh);
	lbl_doubleStranded->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column2 + spacing;
	
	cb_complement = new QCheckBox(this);
	Q_CHECK_PTR(cb_complement);
	cb_complement->setGeometry(xpos+7, ypos+5, 14, 14);
	cb_complement->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_complement->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_complement->setChecked(complement);
	connect(cb_complement, SIGNAL(clicked()), SLOT(update_complement()));

	xpos += column1 + spacing;

	lbl_complement = new QLabel(tr("Calculate MW of complement only"),this);
	lbl_complement->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_complement->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_complement->setGeometry(xpos, ypos, 2 * column2 - column1, buttonh);
	lbl_complement->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos = border;
	ypos += buttonh + spacing;
	
	cb_3prime_oh = new QCheckBox(this);
	Q_CHECK_PTR(cb_3prime_oh);
	cb_3prime_oh->setGeometry(xpos+7, ypos+5, 14, 14);
	cb_3prime_oh->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_3prime_oh->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_3prime_oh->setChecked(_3prime_oh);
	connect(cb_3prime_oh, SIGNAL(clicked()), SLOT(update_3prime_oh()));

	xpos += column1 + spacing;
	
	lbl_3prime_oh = new QLabel(tr("3' -Hydroxyl"),this);
	lbl_3prime_oh->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_3prime_oh->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_3prime_oh->setGeometry(xpos, ypos, column2 - 7, buttonh);
	lbl_3prime_oh->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column2 + spacing;

	lbl_banner2 = new QLabel(tr("Counterion molar ratio/nucleotide:"), this);
	Q_CHECK_PTR(lbl_banner2);
	lbl_banner2->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_banner2->setAlignment(AlignCenter|AlignVCenter);
	lbl_banner2->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_banner2->setGeometry(xpos, ypos, 2 * column2 + spacing, buttonh);
	lbl_banner2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	xpos = border;
	ypos += buttonh + spacing;
	
	cb_3prime_po4 = new QCheckBox(this);
	Q_CHECK_PTR(cb_3prime_po4);
	cb_3prime_po4->setGeometry(xpos+7, ypos+5, 14, 14);
	cb_3prime_po4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_3prime_po4->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_3prime_po4->setChecked(false);
	connect(cb_3prime_po4, SIGNAL(clicked()), SLOT(update_3prime_po4()));

	xpos += column1 + spacing;
	
	lbl_3prime_po4 = new QLabel(tr("3'-Phosphate"),this);
	lbl_3prime_po4->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_3prime_po4->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_3prime_po4->setGeometry(xpos, ypos, column2 - 7, buttonh);
	lbl_3prime_po4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column2 + spacing;

	lbl_sodium = new QLabel(tr("Sodium, Na+"),this);
	lbl_sodium->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_sodium->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_sodium->setGeometry(xpos, ypos, column2, buttonh);
	lbl_sodium->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column2 + spacing;

	cnt_sodium= new QwtCounter(this);
	Q_CHECK_PTR(cnt_sodium);
	cnt_sodium->setRange(0, 1, .01);
	cnt_sodium->setNumButtons(2);
	cnt_sodium->setValue(sodium);
	cnt_sodium->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_sodium->setGeometry(xpos, ypos-1, column2, buttonh - 2);
//	connect(cnt_sodium, SIGNAL(buttonReleased(double)), SLOT(update_sodium(double)));
	connect(cnt_sodium, SIGNAL(valueChanged(double)), SLOT(update_sodium(double)));

	xpos = border;
	ypos += buttonh + spacing;
	
	cb_5prime_oh = new QCheckBox(this);
	Q_CHECK_PTR(cb_5prime_oh);
	cb_5prime_oh->setGeometry(xpos+7, ypos+5, 14, 14);
	cb_5prime_oh->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_5prime_oh->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_5prime_oh->setChecked(_5prime_oh);
	connect(cb_5prime_oh, SIGNAL(clicked()), SLOT(update_5prime_oh()));

	xpos += column1 + spacing;
	
	lbl_5prime_oh = new QLabel(tr("5'-Hydroxyl"),this);
	lbl_5prime_oh->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_5prime_oh->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_5prime_oh->setGeometry(xpos, ypos, column2 - 7, buttonh);
	lbl_5prime_oh->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column2 + spacing;

	lbl_potassium = new QLabel(tr("Potassium, K+"),this);
	lbl_potassium->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_potassium->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_potassium->setGeometry(xpos, ypos, column2, buttonh);
	lbl_potassium->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column2 + spacing;

	cnt_potassium= new QwtCounter(this);
	Q_CHECK_PTR(cnt_potassium);
	cnt_potassium->setRange(0, 1, .01);
	cnt_potassium->setNumButtons(2);
	cnt_potassium->setValue(potassium);
	cnt_potassium->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_potassium->setGeometry(xpos, ypos-1, column2, buttonh - 2);
//	connect(cnt_potassium, SIGNAL(buttonReleased(double)), SLOT(update_potassium(double)));
	connect(cnt_potassium, SIGNAL(valueChanged(double)), SLOT(update_potassium(double)));

	xpos = border;
	ypos += buttonh + spacing;
	
	cb_5prime_po4 = new QCheckBox(this);
	Q_CHECK_PTR(cb_5prime_po4);
	cb_5prime_po4->setGeometry(xpos+7, ypos+5, 14, 14);
	cb_5prime_po4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_5prime_po4->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_5prime_po4->setChecked(true);
	connect(cb_5prime_po4, SIGNAL(clicked()), SLOT(update_5prime_po4()));

	xpos += column1 + spacing;
	
	lbl_5prime_po4 = new QLabel(tr("5' -Phosphate"),this);
	lbl_5prime_po4->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_5prime_po4->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_5prime_po4->setGeometry(xpos, ypos, column2 - 7, buttonh);
	lbl_5prime_po4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column2 + spacing;

	lbl_lithium = new QLabel(tr("Lithium, Li+"),this);
	lbl_lithium->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_lithium->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_lithium->setGeometry(xpos, ypos, column2, buttonh);
	lbl_lithium->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column2 + spacing;

	cnt_lithium= new QwtCounter(this);
	Q_CHECK_PTR(cnt_lithium);
	cnt_lithium->setRange(0, 1, .01);
	cnt_lithium->setNumButtons(2);
	cnt_lithium->setValue(lithium);
	cnt_lithium->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_lithium->setGeometry(xpos, ypos-1, column2, buttonh - 2);
//	connect(cnt_lithium, SIGNAL(buttonReleased(double)), SLOT(update_lithium(double)));
	connect(cnt_lithium, SIGNAL(valueChanged(double)), SLOT(update_lithium(double)));

	xpos = border;
	ypos += buttonh + spacing;
	
	cb_DNA = new QCheckBox(this);
	Q_CHECK_PTR(cb_DNA);
	cb_DNA->setGeometry(xpos+7, ypos+5, 14, 14);
	cb_DNA->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_DNA->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_DNA->setChecked(true);
	connect(cb_DNA, SIGNAL(clicked()), SLOT(update_DNA()));

	xpos += column1 + spacing;
	
	lbl_DNA = new QLabel(tr("DNA"),this);
	lbl_DNA->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_DNA->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_DNA->setGeometry(xpos, ypos, column2 - 7, buttonh);
	lbl_DNA->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column2 + spacing;

	lbl_magnesium = new QLabel(tr("Magnesium, Mg++"),this);
	lbl_magnesium->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_magnesium->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_magnesium->setGeometry(xpos, ypos, column2, buttonh);
	lbl_magnesium->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column2 + spacing;

	cnt_magnesium= new QwtCounter(this);
	Q_CHECK_PTR(cnt_magnesium);
	cnt_magnesium->setRange(0, 1, .01);
	cnt_magnesium->setNumButtons(2);
	cnt_magnesium->setValue(magnesium);
	cnt_magnesium->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_magnesium->setGeometry(xpos, ypos-1, column2, buttonh - 2);
//	connect(cnt_magnesium, SIGNAL(buttonReleased(double)), SLOT(update_magnesium(double)));
	connect(cnt_magnesium, SIGNAL(valueChanged(double)), SLOT(update_magnesium(double)));

	xpos = border;
	ypos += buttonh + spacing;
	
	cb_RNA = new QCheckBox(this);
	Q_CHECK_PTR(cb_RNA);
	cb_RNA->setGeometry(xpos+7, ypos+5, 14, 14);
	cb_RNA->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_RNA->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_RNA->setChecked(false);
	connect(cb_RNA, SIGNAL(clicked()), SLOT(update_RNA()));

	xpos += column1 + spacing;
	
	lbl_RNA = new QLabel(tr("RNA"),this);
	lbl_RNA->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_RNA->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_RNA->setGeometry(xpos, ypos, column2 - 7, buttonh);
	lbl_RNA->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column2 + spacing;

	lbl_calcium = new QLabel(tr("Calcium, Ca++"),this);
	lbl_calcium->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_calcium->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_calcium->setGeometry(xpos, ypos, column2, buttonh);
	lbl_calcium->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column2 + spacing;

	cnt_calcium= new QwtCounter(this);
	Q_CHECK_PTR(cnt_calcium);
	cnt_calcium->setRange(0, 1, .01);
	cnt_calcium->setNumButtons(2);
	cnt_calcium->setValue(calcium);
	cnt_calcium->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_calcium->setGeometry(xpos, ypos-1, column2, buttonh - 2);
//	connect(cnt_calcium, SIGNAL(buttonReleased(double)), SLOT(update_calcium(double)));
	connect(cnt_calcium, SIGNAL(valueChanged(double)), SLOT(update_calcium(double)));

	ypos += buttonh + 2 * spacing;
	xpos = border;
	buttonw	= (int) (span - spacing)/3;
	
	lbl_sequence = new QLabel(tr("Sequence Name:"),this);
	lbl_sequence->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_sequence->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_sequence->setGeometry(xpos, ypos, buttonw - 40, buttonh);
	lbl_sequence->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	xpos += buttonw - 40 + spacing;

	le_sequence = new QLineEdit("",this);
	le_sequence->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_sequence->setGeometry(xpos, ypos, 2 * buttonw + spacing + 40, buttonh);
	le_sequence->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_sequence->setReadOnly(true);
	
	xpos = border;
	ypos += buttonh + spacing;

	lbl_mw = new QLabel(tr("Molecular Weight:"),this);
	lbl_mw->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_mw->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_mw->setGeometry(xpos, ypos, buttonw - 40, buttonh);
	lbl_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	xpos += buttonw - 40 + spacing;

	le_mw = new QLineEdit("",this);
	le_mw->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_mw->setGeometry(xpos, ypos, 2 * buttonw + spacing + 40, buttonh);
	le_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_mw->setReadOnly(true);

	ypos += buttonh + 2 * spacing;
	xpos = border;
	
	pb_load = new QPushButton(tr("Load Sequence"), this);
	Q_CHECK_PTR(pb_load);
	pb_load->setAutoDefault(false);
	pb_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_load->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_load->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_load, SIGNAL(clicked()), SLOT(load()));

	xpos += buttonw + spacing;

	pb_download = new QPushButton(tr("Download Sequence"), this);
	Q_CHECK_PTR(pb_download);
	pb_download->setAutoDefault(false);
	pb_download->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_download->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_download->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_download, SIGNAL(clicked()), SLOT(download()));

	xpos += buttonw + spacing;

	pb_update = new QPushButton(tr("Update"), this);
	Q_CHECK_PTR(pb_update);
	pb_update->setAutoDefault(false);
	pb_update->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_update->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_update->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_update, SIGNAL(clicked()), SLOT(update()));

	ypos += buttonh + spacing;
	xpos = border;

	pb_save = new QPushButton(tr("Save Calculation"), this);
	Q_CHECK_PTR(pb_save);
	pb_save->setAutoDefault(false);
	pb_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_save->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_save, SIGNAL(clicked()), SLOT(save()));

	xpos += buttonw + spacing;

	pb_help = new QPushButton(tr("Help"), this);
	Q_CHECK_PTR(pb_help);
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_help->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	xpos += buttonw + spacing;

	pb_quit = new QPushButton(tr("Close"), this);
	Q_CHECK_PTR(pb_quit);
	pb_quit->setAutoDefault(false);
	pb_quit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_quit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_quit->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_quit, SIGNAL(clicked()), SLOT(quit()));

	ypos += buttonh + border;

	global_Xpos += 30;
	global_Ypos += 30;
	
	setMinimumSize(span+8, ypos);
	setGeometry(global_Xpos, global_Ypos, span+8, ypos);
}

US_Nucleotide::~US_Nucleotide()
{
}

void US_Nucleotide::closeEvent(QCloseEvent *e)
{
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
}

void US_Nucleotide::update_doubleStranded()
{
	if(doubleStranded)
	{
		doubleStranded = false;
		cb_doubleStranded->setChecked(false);
	}
	else
	{
		doubleStranded = true;
		complement = false;
		cb_doubleStranded->setChecked(true);
		cb_complement->setChecked(false);
	}
}

void US_Nucleotide::update_complement()
{
	if(complement)
	{
		complement = false;
		cb_complement->setChecked(false);
	}
	else
	{
		complement = true;
		doubleStranded = false;
		cb_doubleStranded->setChecked(false);
		cb_complement->setChecked(true);
	}
}

void US_Nucleotide::update_3prime_oh()
{
	cb_3prime_oh->setChecked(true);
	cb_3prime_po4->setChecked(false);
	_3prime_oh = true;
}

void US_Nucleotide::update_3prime_po4()
{
	cb_3prime_oh->setChecked(false);
	cb_3prime_po4->setChecked(true);
	_3prime_oh = false;
}

void US_Nucleotide::update_5prime_oh()
{
	_5prime_oh = true;
	cb_5prime_oh->setChecked(true);
	cb_5prime_po4->setChecked(false);
}

void US_Nucleotide::update_5prime_po4()
{
	_5prime_oh = false;
	cb_5prime_oh->setChecked(false);
	cb_5prime_po4->setChecked(true);
}

void US_Nucleotide::update_DNA()
{
	isDNA = true;
	cb_RNA->setChecked(false);
	cb_DNA->setChecked(true);
	if (MW > 0 && T == 0 && U > 0)
	{
		QMessageBox::message(tr("Attention:"), tr("Are you sure?\n"
									"There don't appear to be any\n"
									"thymine residues present, instead\n"
									"there are uracil residues in this\n"
									"sequence."));
	}
}

void US_Nucleotide::update_RNA()
{
	isDNA = false;
	cb_DNA->setChecked(false);
	cb_RNA->setChecked(true);
	if (MW > 0 && T > 0 && U == 0)
	{
		QMessageBox::message(tr("Attention:"), tr("Are you sure?\n"
									"There don't appear to be any\n"
									"uracil residues present, instead\n"
									"there are thymine residues in this\n"
									"sequence."));
	}
}

void US_Nucleotide::update_sodium(double val)
{
	sodium = (float) val;
}

void US_Nucleotide::update_potassium(double val)
{
	potassium = (float) val;
}

void US_Nucleotide::update_lithium(double val)
{
	lithium = (float) val;
}

void US_Nucleotide::update_calcium(double val)
{
	calcium = (float) val;
}

void US_Nucleotide::update_magnesium(double val)
{
	magnesium = (float) val;
}

void US_Nucleotide::load()
{
	QString test, token;
	A=0;
	C=0;
	G=0;
	T=0;
	U=0;
	bool sequence_found = false;
	filename = QFileDialog::getOpenFileName(USglobal->config_list.root_dir, "*.seq", 0);
	if ( !filename.isEmpty() ) 
	{
		QFile f(filename);
		f.open(IO_ReadOnly);
		QTextStream ts (&f);
		token = "";
		while (!ts.atEnd())
		{
			test = ts.readLine();
			token = getToken(&test, " ");
			token.stripWhiteSpace();
			if (token == "DEFINITION")
			{
				test.stripWhiteSpace();
				title = test;
				le_sequence->setReadOnly(false);
				le_sequence->setText(test);	// the rest is the sequence definition
				le_sequence->setReadOnly(true);
				break;
			}
		}
		if (f.atEnd())
		{
			QMessageBox::message(tr("Attention:"), tr("The Sequence File is not in the proper format!\n"
														  "The Description Line preceded by \"DEFINITION\" is missing!\n\n"
														  "Please Fix this error and try again..."));
			return;
		}
		while (!ts.atEnd())
		{
			test = ts.readLine();
			token = getToken(&test, " ");
			token.stripWhiteSpace();
			if (token == "ORIGIN")
			{
				sequence_found = true;
				break;
			}
		}
		if (!sequence_found)
		{
			QMessageBox::message(tr("Attention:"), tr("The Sequence File is not in the proper format!\n"
														  "The DNA/RNA sequence could not be found!\n\n"
														  "Please Fix this error and try again..."));
			return;
		}
		token = "";
		while (!ts.atEnd())
		{
			test = ts.readLine();
			token = getToken(&test, " ");
			token.stripWhiteSpace();
			A += test.contains("a", false);
			G += test.contains("g", false);
			C += test.contains("c", false);
			T += test.contains("t", false);
			U += test.contains("u", false);
			if (token == "//")
			{
				break;
			}
		}				
	}
	MW = 0.0;
	update();
}

void US_Nucleotide::download()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/download_nucleotide.html");
}

void US_Nucleotide::update()
{
	QString str;
	MW = 0;
	total = A + G + C + T + U;
	if (doubleStranded)
	{
		total *= 2;
	}
	float mw_A = (float) 313.209;
	float mw_C = (float) 289.184;
	float mw_G = (float) 329.208;
	float mw_T = (float) 304.196;
	float mw_U = (float) 274.170;
	if(isDNA)
	{
		if (doubleStranded)
		{
			MW += A * (mw_A);
			MW += G * (mw_G);
			MW += C * (mw_C);
			MW += T * (mw_T);
			MW += A * (mw_T);
			MW += G * (mw_C);
			MW += C * (mw_G);
			MW += T * (mw_A);
		}
		if (complement)
		{
			MW += A * (mw_T);
			MW += G * (mw_C);
			MW += C * (mw_G);
			MW += T * (mw_A);
		}
		if (!complement && !doubleStranded)
		{
			MW += A * (mw_A);
			MW += G * (mw_G);
			MW += C * (mw_C);
			MW += T * (mw_T);
		}
	}
	else
	{
		if (doubleStranded)
		{
			MW += A * (mw_A + 15.999);
			MW += G * (mw_G + 15.999);
			MW += C * (mw_C + 15.999);
			MW += U * (mw_U + 15.999);
			MW += A * (mw_U + 15.999);
			MW += G * (mw_C + 15.999);
			MW += C * (mw_G + 15.999);
			MW += U * (mw_A + 15.999);
		}
		if (complement)
		{
			MW += A * (mw_U + 15.999);
			MW += G * (mw_C + 15.999);
			MW += C * (mw_G + 15.999);
			MW += U * (mw_A + 15.999);
		}
		if (!complement && !doubleStranded)
		{
			MW += A * (mw_A + 15.999);
			MW += G * (mw_G + 15.999);
			MW += C * (mw_C + 15.999);
			MW += U * (mw_U + 15.999);
		}
	}
	MW += sodium * total * 22.99;
	MW += potassium * total * 39.1;
	MW += lithium * total * 6.94;
	MW += magnesium * total * 24.305;
	MW += calcium * total * 40.08;
	if (_3prime_oh)
	{
		MW += (float) 17.01;
		if (doubleStranded)
		{
			MW += (float) 17.01;
		}
	}
	else // we have phosphate
	{
		MW += (float) 94.87;
		if (doubleStranded)
		{
			MW += (float) 94.87;
		}
	}
	if (_5prime_oh)
	{
		MW -= (float) 77.96;
		if (doubleStranded)
		{
			MW -= (float) 77.96;
		}
	}
	if (doubleStranded)
	{
		str.sprintf(" %2.5e kD (%d A, %d G, %d C, %d U, %d T, %d bp)", MW/1000, A, G, C, U, T, total/2);
	}
	else
	{
		str.sprintf(" %2.5e kD (%d A, %d G, %d C, %d U, %d T, %d bases)", MW/1000, A, G, C, U, T, total);
	}
	le_mw->setReadOnly(false);
	le_mw->setText(str);
	le_mw->setReadOnly(true);
}

void US_Nucleotide::save()
{
	QString str, format;
	if(filename.right(4) == ".seq")
	{
		str = filename.left(filename.length() - 4);
	}
	else
	{
		str = filename;
	}
	QFile f(str + ".res");
	QTextStream ts(&f);
	if (f.open(IO_WriteOnly | IO_Translate))
	{
		ts << tr("Molecular Weight Calculation for Sequence \"") << filename << "\":\n\n";
		ts << tr("Description: ") << title << "\n\n";
		if(doubleStranded)
		{
			ts << tr("Molecular Weight: ") << MW << tr(" Dalton (") << total/2 << tr(" basepairs\n\n");
		}
		else
		{
			ts << tr("Molecular Weight: ") << MW << tr(" Dalton (") << total << tr(" bases)\n\n");
		}
		ts << tr("Number of Adenine Residues: ") << A << endl;
		ts << tr("Number of Cytosin Residues: ") << C << endl;
		ts << tr("Number of Guanine Residues: ") << G << endl;
		ts << tr("Number of Thymine Residues: ") << T << endl;
		ts << tr("Number of Uracil Residues: ") << U << "\n\n";
		ts << tr("The molecular weight was calculated with the following molar ratios\nof counterions:\n\n");
		format.sprintf("%1.2f", sodium);
		ts << tr("Sodium:    ") << format.latin1() << tr(" mols/residue\n");
		format.sprintf("%1.2f", potassium);
		ts << tr("Potassium: ") << format.latin1() << tr(" mols/residue\n");
		format.sprintf("%1.2f", lithium);
		ts << tr("Lithium:   ") << format.latin1() << tr(" mols/residue\n");
		format.sprintf("%1.2f", magnesium);
		ts << tr("Magnesium: ") << format.latin1() << tr(" mols/residue\n");
		format.sprintf("%1.2f", calcium);
		ts << tr("Calcium:   ") << format.latin1() << tr(" mols/residue\n\n");
		if(_3prime_oh)
		{
			ts << tr("The 3'-terminus was calculated as a hydroxyl group.\n");
		}
		else
		{
			ts << tr("The 3'-terminus was calculated as a phosphate group.\n");
		}
		if(_5prime_oh)
		{
			ts << tr("The 5'-terminus was calculated as a hydroxyl group.\n");
		}
		else
		{
			ts << tr("The 5'-terminus was calculated as a phosphate group.\n");
		}
		if (doubleStranded)
		{
			ts << tr("The molecular weight represents a double-stranded molecule.\n");
		}
		if (complement)
		{
			ts << tr("The molecular weight represents the single-stranded complement of the sequence.\n");
		}
		if (!complement && !doubleStranded)
		{
			ts << tr("The molecular weight represents a single-stranded molecule.\n");
		}
		f.close();
	}
	QMessageBox::message(tr("Please note:"), tr("The results for the molecular weight calculation of\n\n")
													 + filename +
													 tr("\n\nhave been saved in\n\n") + str + ".res");
}

void US_Nucleotide::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/nucleotide.html");
}

void US_Nucleotide::quit()
{
	close();
}

