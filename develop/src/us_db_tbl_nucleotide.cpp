#include "../include/us_db_tbl_nucleotide.h"

//! Constructor
/*! 
	Constractor a new <var>US_Nucleotide_DB</var> interface, 
	\param from_cell <tt>true</tt> called from US_Cell_DB Table, <tt>false</tt> otherwise.
	\param p Parent widget.
	\param name Widget name.
*/	
US_Nucleotide_DB::US_Nucleotide_DB(bool from_cell, int temp_InvID, QWidget *p, const char *name) : US_DB(p, name)
{
	cell_flag = from_cell;
	DNA_info.DNAID = -1;
	DNA_info.InvID = temp_InvID;
	DNA_info.Description = "";
	DNA_info.Sequence = "";
	select_flag = false;
	from_HD = false;
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
	vbar = 0.0;
	e260 = 0.0;
	e280 = 0.0;
	
	QString str;
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	
	setCaption(tr("Nucleotide Sequence MW Calculator"));
	lbl_banner1 = new QLabel(tr("Nucleotide Sequence Molecular Weight Calculator"), this);
	Q_CHECK_PTR(lbl_banner1);
	lbl_banner1->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_banner1->setAlignment(AlignCenter|AlignVCenter);
	lbl_banner1->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_banner1->setMinimumHeight(50);
	lbl_banner1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	
	cb_doubleStranded = new QCheckBox(tr("Double Stranded"), this);
	Q_CHECK_PTR(cb_doubleStranded);
	cb_doubleStranded->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_doubleStranded->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_doubleStranded->setChecked(doubleStranded);
	connect(cb_doubleStranded, SIGNAL(clicked()), SLOT(update_doubleStranded()));

	cb_complement = new QCheckBox(tr("Calc. MW of complement only"),this);
	Q_CHECK_PTR(cb_complement);
	cb_complement->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_complement->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_complement->setChecked(complement);
	connect(cb_complement, SIGNAL(clicked()), SLOT(update_complement()));

	cb_3prime_oh = new QCheckBox(tr("3' -Hydroxyl"),this);
	Q_CHECK_PTR(cb_3prime_oh);
	cb_3prime_oh->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_3prime_oh->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_3prime_oh->setChecked(_3prime_oh);
	connect(cb_3prime_oh, SIGNAL(clicked()), SLOT(update_3prime_oh()));

	lbl_banner2 = new QLabel(tr("Counterion molar ratio/nucleotide:"), this);
	Q_CHECK_PTR(lbl_banner2);
	lbl_banner2->setMinimumHeight(26);
	lbl_banner2->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_banner2->setAlignment(AlignCenter|AlignVCenter);
	lbl_banner2->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_banner2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	lbl_banner3 = new QLabel(tr("Measured Sequence Properties:"), this);
	Q_CHECK_PTR(lbl_banner3);
	lbl_banner3->setMinimumHeight(26);
	lbl_banner3->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_banner3->setAlignment(AlignCenter|AlignVCenter);
	lbl_banner3->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_banner3->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	cb_3prime_po4 = new QCheckBox(tr("3'-Phosphate"),this);
	Q_CHECK_PTR(cb_3prime_po4);
	cb_3prime_po4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_3prime_po4->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_3prime_po4->setChecked(false);
	connect(cb_3prime_po4, SIGNAL(clicked()), SLOT(update_3prime_po4()));

	lbl_sodium = new QLabel(tr("Sodium, Na+"),this);
	lbl_sodium->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_sodium->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_sodium->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_sodium= new QwtCounter(this);
	Q_CHECK_PTR(cnt_sodium);
	cnt_sodium->setRange(0, 1, .01);
	cnt_sodium->setNumButtons(2);
	cnt_sodium->setValue(sodium);
	cnt_sodium->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_sodium, SIGNAL(valueChanged(double)), SLOT(update_sodium(double)));

	cb_5prime_oh = new QCheckBox(tr("5'-Hydroxyl"),this);
	Q_CHECK_PTR(cb_5prime_oh);
	cb_5prime_oh->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_5prime_oh->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_5prime_oh->setChecked(_5prime_oh);
	connect(cb_5prime_oh, SIGNAL(clicked()), SLOT(update_5prime_oh()));

	lbl_potassium = new QLabel(tr("Potassium, K+"),this);
	lbl_potassium->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_potassium->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_potassium->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_potassium= new QwtCounter(this);
	Q_CHECK_PTR(cnt_potassium);
	cnt_potassium->setRange(0, 1, .01);
	cnt_potassium->setNumButtons(2);
	cnt_potassium->setValue(potassium);
	cnt_potassium->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_potassium, SIGNAL(valueChanged(double)), SLOT(update_potassium(double)));

	cb_5prime_po4 = new QCheckBox(tr("5' -Phosphate"), this);
	Q_CHECK_PTR(cb_5prime_po4);
	cb_5prime_po4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_5prime_po4->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_5prime_po4->setChecked(true);
	connect(cb_5prime_po4, SIGNAL(clicked()), SLOT(update_5prime_po4()));

	lbl_lithium = new QLabel(tr("Lithium, Li+"),this);
	lbl_lithium->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_lithium->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_lithium->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_lithium= new QwtCounter(this);
	Q_CHECK_PTR(cnt_lithium);
	cnt_lithium->setRange(0, 1, .01);
	cnt_lithium->setNumButtons(2);
	cnt_lithium->setValue(lithium);
	cnt_lithium->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_lithium, SIGNAL(valueChanged(double)), SLOT(update_lithium(double)));

	cb_DNA = new QCheckBox(tr("DNA"), this);
	Q_CHECK_PTR(cb_DNA);
	cb_DNA->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_DNA->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_DNA->setChecked(true);
	connect(cb_DNA, SIGNAL(clicked()), SLOT(update_DNA()));

	lbl_magnesium = new QLabel(tr("Magnesium, Mg++"),this);
	lbl_magnesium->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_magnesium->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_magnesium->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_magnesium= new QwtCounter(this);
	Q_CHECK_PTR(cnt_magnesium);
	cnt_magnesium->setRange(0, 1, .01);
	cnt_magnesium->setNumButtons(2);
	cnt_magnesium->setValue(magnesium);
	cnt_magnesium->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_magnesium, SIGNAL(valueChanged(double)), SLOT(update_magnesium(double)));

	cb_RNA = new QCheckBox(tr("RNA"),this);
	Q_CHECK_PTR(cb_RNA);
	cb_RNA->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_RNA->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_RNA->setChecked(false);
	connect(cb_RNA, SIGNAL(clicked()), SLOT(update_RNA()));

	lbl_calcium = new QLabel(tr("Calcium, Ca++"),this);
	lbl_calcium->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_calcium->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_calcium->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_calcium= new QwtCounter(this);
	Q_CHECK_PTR(cnt_calcium);
	cnt_calcium->setRange(0, 1, .01);
	cnt_calcium->setNumButtons(2);
	cnt_calcium->setValue(calcium);
	cnt_calcium->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_calcium, SIGNAL(valueChanged(double)), SLOT(update_calcium(double)));

	lbl_mw = new QLabel(tr("Molecular Weight:"),this);
	lbl_mw->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_mw->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	le_mw = new QLineEdit("",this);
	le_mw->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_mw->setReadOnly(true);

	lbl_vbar = new QLabel(tr("Part. Spec. Vol. (ccm/g):"),this);
	lbl_vbar->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_vbar->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	le_vbar = new QLineEdit("",this);
	le_vbar->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_vbar->setText("0.0");
	connect(le_vbar, SIGNAL(textChanged(const QString &)), SLOT(update_vbar(const QString &)));

	lbl_e260 = new QLabel(tr("Molar Extinction (260 nm):"),this);
	lbl_e260->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_e260->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_e260->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	le_e260 = new QLineEdit("",this);
	le_e260->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_e260->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_e260->setText("0.0");
	connect(le_e260, SIGNAL(textChanged(const QString &)), SLOT(update_e260(const QString &)));

	lbl_e280 = new QLabel(tr("Molar Extinction (280 nm):"),this);
	lbl_e280->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_e280->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_e280->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	le_e280 = new QLineEdit("",this);
	le_e280->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_e280->setText("0.0");
	le_e280->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	connect(le_e280, SIGNAL(textChanged(const QString &)), SLOT(update_e280(const QString &)));

	lbl_sequence = new QLabel(tr("Sequence Name:"),this);
	lbl_sequence->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_sequence->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_sequence->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	le_sequence = new QLineEdit("",this);
	le_sequence->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_sequence->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_sequence->setReadOnly(true);

	pb_investigator = new QPushButton(tr("Select Investigator"), this);
	pb_investigator->setAutoDefault(false);
	pb_investigator->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_investigator->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	if(DNA_info.InvID>0)
	{
		pb_investigator->setEnabled(false);
	}
	connect(pb_investigator, SIGNAL(clicked()), SLOT(sel_investigator()));

	lbl_investigator= new QLabel("",this);
	lbl_investigator->setAlignment(AlignLeft|AlignVCenter);
	lbl_investigator->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_investigator->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_investigator->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	if(DNA_info.InvID>0)
	{
		lbl_investigator->setText(show_investigator(DNA_info.InvID));
	}
	else
	{
		lbl_investigator->setText(tr(" Not Selected"));
	}

	pb_load = new QPushButton(tr("Load Sequence from HD"), this);
	Q_CHECK_PTR(pb_load);
	pb_load->setAutoDefault(false);
	pb_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_load->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_load, SIGNAL(clicked()), SLOT(load()));

	pb_download = new QPushButton(tr("Download Sequence"), this);
	Q_CHECK_PTR(pb_download);
	pb_download->setAutoDefault(false);
	pb_download->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_download->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_download, SIGNAL(clicked()), SLOT(download()));

	pb_load_db = new QPushButton(tr("Query Sequence from DB"), this);
	Q_CHECK_PTR(pb_load_db);
	pb_load_db->setAutoDefault(false);
	pb_load_db->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_load_db->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_load_db, SIGNAL(clicked()), SLOT(read_db()));

	pb_enter_DNA = new QPushButton(tr("Enter new Sequence"), this);
	Q_CHECK_PTR(pb_enter_DNA);
	pb_enter_DNA->setAutoDefault(false);
	pb_enter_DNA->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_enter_DNA->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_enter_DNA, SIGNAL(clicked()), SLOT(enter_DNA()));

	lbl_banner4 = new QLabel(tr("Doubleclick on sequence data to select:"),this);
	Q_CHECK_PTR(lbl_banner4);
	lbl_banner4->setMinimumHeight(26);
	lbl_banner4->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_banner4->setAlignment(AlignCenter|AlignVCenter);
	lbl_banner4->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_banner4->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	pb_update = new QPushButton(tr("Calculate"), this);
	Q_CHECK_PTR(pb_update);
	pb_update->setAutoDefault(false);
	pb_update->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_update->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_update, SIGNAL(clicked()), SLOT(update()));

	lb_DNA = new QListBox(this, "DNA files");
	lb_DNA->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	lb_DNA->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(lb_DNA, SIGNAL(selected(int)), SLOT(select_DNA(int)));

	pb_save = new QPushButton(tr("Save Result to HD"), this);
	Q_CHECK_PTR(pb_save);
	pb_save->setAutoDefault(false);
	pb_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_save, SIGNAL(clicked()), SLOT(save()));

	pb_del_db = new QPushButton(tr("Delete Sequence from DB"), this);
	Q_CHECK_PTR(pb_del_db);
	pb_del_db->setAutoDefault(false);
	pb_del_db->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_del_db->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_del_db, SIGNAL(clicked()), SLOT(check_permission()));

	pb_reset = new QPushButton(tr("Reset"), this);
	Q_CHECK_PTR(pb_reset);
	pb_reset->setAutoDefault(false);
	pb_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_reset->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));

	pb_help = new QPushButton(tr("Help"), this);
	Q_CHECK_PTR(pb_help);
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	pb_quit = new QPushButton(tr("Close"), this);
	Q_CHECK_PTR(pb_quit);
	pb_quit->setAutoDefault(false);
	pb_quit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_quit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_quit, SIGNAL(clicked()), SLOT(quit()));

	setup_GUI();
}

//! Destructor
/*! destroy the <var>US_Nucleotide_DB</var>. */
US_Nucleotide_DB::~US_Nucleotide_DB()
{
	global_Xpos -= 30;
	global_Ypos -= 30;
}

void US_Nucleotide_DB::setup_GUI()
{
	int j=0;
	int rows = 23, columns = 4, spacing = 2;
	
	QGridLayout * background = new QGridLayout(this, rows, columns, spacing);
	for (int i=1; i<rows; i++)
	{
		background->setRowSpacing(i, 26);
		background->setRowStretch(i, 0);
	}
	background->setRowSpacing(22, 0); // a stretching row
	background->setRowStretch(22, 1); // a stretching row
	background->setColStretch(0, 0);
	background->setColStretch(1, 0);
	background->setColStretch(2, 0);
	background->setColStretch(3, 1);
	background->addMultiCellWidget(lbl_banner1, j, j, 0, 3);
	j++;
	background->addMultiCellWidget(cb_doubleStranded, j, j, 0, 1);
	background->addMultiCellWidget(lbl_banner2, j, j, 2, 3);
	j++;	
	background->addMultiCellWidget(cb_complement, j, j, 0, 1);
	background->addWidget(lbl_sodium, j, 2);
	background->addWidget(cnt_sodium, j, 3);
	j++;
	background->addMultiCellWidget(cb_3prime_oh, j, j, 0, 1);
	background->addWidget(lbl_potassium, j, 2);
	background->addWidget(cnt_potassium, j, 3);
	j++;
	background->addMultiCellWidget(cb_3prime_po4, j, j, 0, 1);
	background->addWidget(lbl_lithium, j, 2);
	background->addWidget(cnt_lithium, j, 3);
	j++;
	background->addMultiCellWidget(cb_5prime_oh, j, j, 0, 1);
	background->addWidget(lbl_magnesium, j, 2);
	background->addWidget(cnt_magnesium, j, 3);
	j++;
	background->addMultiCellWidget(cb_5prime_po4, j, j, 0, 1);
	background->addWidget(lbl_calcium, j, 2);
	background->addWidget(cnt_calcium, j, 3);
	j++;
	background->addMultiCellWidget(cb_DNA, j, j, 0, 1);
	background->addMultiCellWidget(lbl_banner3, j, j, 2, 3);
	j++;
	background->addMultiCellWidget(cb_RNA, j, j, 0, 1);
	background->addWidget(lbl_vbar, j, 2);
	background->addWidget(le_vbar, j, 3);
	j++;
	background->addMultiCellWidget(pb_help, j, j, 0, 1);
	background->addWidget(lbl_e260, j, 2);
	background->addWidget(le_e260, j, 3);
	j++;
	background->addMultiCellWidget(pb_reset, j, j, 0, 1);
	background->addWidget(lbl_e280, j, 2);
	background->addWidget(le_e280, j, 3);
	j++;
	background->addWidget(lbl_mw, j, 0);
	background->addMultiCellWidget(le_mw, j, j, 1, 3);
	j++;
	background->addWidget(lbl_sequence, j, 0);
	background->addMultiCellWidget(le_sequence, j, j, 1, 3);
	j++;
	background->addMultiCellWidget(pb_investigator, j, j, 0, 1);
	background->addMultiCellWidget(lbl_investigator, j, j, 2, 3);
	j++;
	background->addMultiCellWidget(pb_download, j, j, 0, 1);
	background->addMultiCellWidget(lbl_banner4, j, j, 2, 3);
	j++;
	background->addMultiCellWidget(pb_load_db, j, j, 0, 1);
	background->addMultiCellWidget(lb_DNA, j, j+7, 2, 3);
	j++;
	background->addMultiCellWidget(pb_load, j, j, 0, 1);
	j++;
	background->addMultiCellWidget(pb_enter_DNA, j, j, 0, 1);
	j++;
	background->addMultiCellWidget(pb_save, j, j, 0, 1);
	j++;
	background->addMultiCellWidget(pb_del_db, j, j, 0, 1);
	j++;
	background->addMultiCellWidget(pb_update, j, j, 0, 1);
	j++;
	background->addMultiCellWidget(pb_quit, j, j, 0, 1);

	qApp->processEvents();
	QRect r = background->cellGeometry(0, 0);
	
	global_Xpos += 30;
	global_Ypos += 30;
	this->setGeometry(global_Xpos, global_Ypos, r.width(), r.height());
}

/*!
	Open US_DB_TblInvestigator interface for selecting investigator.
*/
void US_Nucleotide_DB::sel_investigator()
{
	US_DB_TblInvestigator *investigator_dlg;
	investigator_dlg = new US_DB_TblInvestigator();
	investigator_dlg->setCaption("Investigator Information");
	investigator_dlg->pb_exit->setText("Accept");
	connect(investigator_dlg, SIGNAL(valueChanged(QString, int)), SLOT(update_investigator_lbl(QString, int)));
	investigator_dlg->exec();
}

/*!
	Update display with the selected investigator information.
	\param Display a string variable for show investigator info.
	\param InvID a integer variable for DB table: <tt>tblInvestigators</tt> index.
*/
void US_Nucleotide_DB::update_investigator_lbl (QString Display, int InvID)
{
	QString str;
	DNA_info.InvID = InvID;
	str = Display;
	lbl_investigator->setText(str);
	if(str == "")
	{
		lbl_investigator->setText(" Not Selected");
	}
}

void US_Nucleotide_DB::update_doubleStranded()
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

void US_Nucleotide_DB::update_complement()
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

void US_Nucleotide_DB::update_3prime_oh()
{
	cb_3prime_oh->setChecked(true);
	cb_3prime_po4->setChecked(false);
	_3prime_oh = true;
}

void US_Nucleotide_DB::update_3prime_po4()
{
	cb_3prime_oh->setChecked(false);
	cb_3prime_po4->setChecked(true);
	_3prime_oh = false;
}

void US_Nucleotide_DB::update_5prime_oh()
{
	_5prime_oh = true;
	cb_5prime_oh->setChecked(true);
	cb_5prime_po4->setChecked(false);
}

void US_Nucleotide_DB::update_5prime_po4()
{
	_5prime_oh = false;
	cb_5prime_oh->setChecked(false);
	cb_5prime_po4->setChecked(true);
}

void US_Nucleotide_DB::update_DNA()
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

void US_Nucleotide_DB::update_RNA()
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

void US_Nucleotide_DB::update_sodium(double val)
{
	sodium = (float) val;
}

void US_Nucleotide_DB::update_potassium(double val)
{
	potassium = (float) val;
}

void US_Nucleotide_DB::update_lithium(double val)
{
	lithium = (float) val;
}

void US_Nucleotide_DB::update_calcium(double val)
{
	calcium = (float) val;
}

void US_Nucleotide_DB::update_magnesium(double val)
{
	magnesium = (float) val;
}

void US_Nucleotide_DB::download()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/download_nucleotide.html");
}

void US_Nucleotide_DB::update()
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
	if (isDNA)
	{
		update_DNA();
	}
	else
	{
		update_RNA();
	}
}

/*! Load DNA data from Hard Drive, add warning message when this widget is called by US_Cell_DB */
void US_Nucleotide_DB::load()
{
	le_vbar->setText("0.0");
	le_e280->setText("0.0");
	le_e260->setText("0.0");
	vbar = 0.0;
	e260 = 0.0;
	e280 = 0.0;
	QString test, token;
	A=0;
	C=0;
	G=0;
	T=0;
	U=0;
	clear();
	bool sequence_found = false;
	if(cell_flag)
	{
		QMessageBox::message(tr("Attention:"), 
									tr("Selecting a nucleotide sequece from the harddrive will not\n"
										"associate the data from this channel with this file.\n\n"
										"In order to associate a nucleotide sequence with your data,\n"
										"you need to select it from the database."));
	}
	filename = QFileDialog::getOpenFileName(USglobal->config_list.root_dir, "*.seq", 0);
	DNA_info.DNAFileName = filename;
	DNA_info.DNAFileName = DNA_info.DNAFileName.remove(0,(DNA_info.DNAFileName.findRev("/", -1,false)+1));
	
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
			if (token == "DEFINITION" || test == "DEFINITION")
			{
				if (test == "DEFINITION")
				{
					title = "No Definition available";
				}
				else
				{
					test.stripWhiteSpace();
					DNA_info.Description = test;
				}
				le_sequence->setReadOnly(false);
				le_sequence->setText(DNA_info.Description);	// the rest is the sequence definition
				le_sequence->setReadOnly(true);
				break;
			}
		}
		if (ts.atEnd())
		{
			QMessageBox::message(tr("Attention:"),
			tr("The sequence file is not in the proper format!\n"
				"The description identifier \"DEFINITION\" is missing!\n\n"
				"Please fix this error and try again..."));
			return;
		}
		while (!ts.atEnd())
		{
			test = ts.readLine();
			token = getToken(&test, " ");
			token.stripWhiteSpace();
			if (token == "ORIGIN" || test ==  "ORIGIN")
			{
				sequence_found = true;
				break;
			}
		}
		if (!sequence_found)
		{
			QMessageBox::message(tr("Attention:"),
			tr("The sequence file is not in the proper format!\n"
				"The DNA/RNA sequence could not be found!\n\n"
				"Please Fix this error and try again..."));
			return;
		}
		token = "";
		while (!ts.atEnd())
		{
			test = ts.readLine();
			A += test.contains("a", false);
			G += test.contains("g", false);
			C += test.contains("c", false);
			T += test.contains("t", false);
			U += test.contains("u", false);
			DNA_info.Sequence.append(test);
			DNA_info.Sequence.append("\n");
			if (test == "//")
			{
				break;
			}
		}
		if (!ts.atEnd())
		{
			test = ts.readLine();
			vbar = test.toFloat();
			test.sprintf("%6.4f", vbar);
			le_vbar->setText(test);
		}
		if (!ts.atEnd())
		{
			test = ts.readLine();
			e260 = test.toFloat();
			test.sprintf("%10.1f", e260);
			le_e260->setText(test);
		}
		if (!ts.atEnd())
		{
			test = ts.readLine();
			e280 = test.toFloat();
			test.sprintf("%10.1f", e280);
			le_e280->setText(test);
		}
		from_HD = true;				
	}
	MW = 0.0;
	update();
}

void US_Nucleotide_DB::save()
{
	QString str, format;
	str = DNA_info.DNAFileName.left(DNA_info.DNAFileName.length() - 4);	//erase '.seq'
	str = USglobal->config_list.result_dir + "/" + str + ".nucleotide_res";
	QFile f(str);
	QTextStream ts(&f);
	if (f.open(IO_WriteOnly | IO_Translate))
	{
		ts << tr("Molecular Weight Calculation for Sequence \"") << filename << "\":\n\n";
		ts << tr("Description: ") << title << "\n\n";
		if(doubleStranded)
		{
			ts << tr("Molecular Weight: ") << MW << tr(" Dalton (") << total/2 << tr(" basepairs)\n\n");
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
													 tr("\n\nhave been saved in\n\n") + str);
}

/*! Load DNA data from database table: tblDNA. */
void US_Nucleotide_DB::read_db()
{
	QString str;
	le_vbar->setText("0.0");
	le_e280->setText("0.0");
	le_e260->setText("0.0");
	vbar = 0.0;
	e260 = 0.0;
	e280 = 0.0;
	int maxID = get_newID("tblDNA", "DNAID");
	int count = 0;
	item_DNAID = new int[maxID];
	item_Description = new QString[maxID];
	display_Str = new QString[maxID];
	
	if(DNA_info.InvID >0)
	{
		str.sprintf("SELECT DNAID, Description FROM tblDNA WHERE InvestigatorID = %d ORDER BY DNAID DESC;", DNA_info.InvID);
	}
	else
	{
		str.sprintf("SELECT DNAID, Description FROM tblDNA ORDER BY DNAID DESC;");
	
	}
	QSqlQuery query(str);
	if(query.isActive())
	{
		while(query.next())
		{
			item_DNAID[count] = query.value(0).toInt();
			item_Description[count] = query.value(1).toString();
			display_Str[count] = "DNAID ("+  QString::number( item_DNAID[count] ) + "): "+item_Description[count];
			count++;
		}
	}

	if(count>0)
	{
		lb_DNA->clear();
		for( int i=0; i<count; i++)
		{
			lb_DNA->insertItem(display_Str[i]);
		}
		select_flag = true;
	}
	else
	{
		lb_DNA->clear();
		lb_DNA->insertItem("No DNA/RNA sequence file found for the selected investigator,");
	}
}

/*!
	If you find the DNA file name in the ListBox by load() or read_db(). 
	Doubleclick it, you will get all DNA data about this name.
	\param item The number of items in ListBox, count start from 0.
*/
void US_Nucleotide_DB::select_DNA(int item)
{
	if(select_flag)
	{	
		Item = item;
		DNA_info.DNAID = item_DNAID[item];
		emit IdChanged(DNA_info.DNAID);
		retrieve_DNA(DNA_info.DNAID);
	}
	else
	{
		QMessageBox::message(tr("Attention:"), 
									tr("No records found in the database\n"));
		return;
	}
}
void US_Nucleotide_DB::retrieve_DNA(int DNAID)
{
	A=0;
	C=0;
	G=0;
	T=0;
	U=0;
	QString str;
	str.sprintf("SELECT DNAFileName, Description, Sequence, InvestigatorID, vbar, e260, e280  FROM tblDNA WHERE DNAID = %d", DNAID);
	QSqlQuery query(str);
	if(query.isActive())
	{
		if(query.next())
		{
				DNA_info.DNAFileName = query.value(0).toString();
				DNA_info.Description = query.value(1).toString();
				DNA_info.Sequence = query.value(2).toString();
				DNA_info.InvID = query.value(3).toInt();
				DNA_info.vbar = (float) query.value(4).toDouble();
				DNA_info.e260 = (float) query.value(5).toDouble();
				DNA_info.e280 = (float) query.value(6).toDouble();
		}
	}
	le_sequence->setText(DNA_info.Description);
	lbl_investigator->setText(show_investigator(DNA_info.InvID));
	str.sprintf("%6.4f", DNA_info.vbar);
	le_vbar->setText(str);
	str.sprintf("%6.4f", DNA_info.e260);
	le_e260->setText(str);
	str.sprintf("%6.4f", DNA_info.e280);
	le_e280->setText(str);
	pb_investigator->setEnabled(false);
	A += DNA_info.Sequence.contains("a", false);
	G += DNA_info.Sequence.contains("g", false);
	C += DNA_info.Sequence.contains("c", false);
	T += DNA_info.Sequence.contains("t", false);
	U += DNA_info.Sequence.contains("u", false);
	MW = 0.0;
	update();
	from_HD = false;

}
/*!
	Save all new DNA data to DB table: tblDNA and exit.
*/
void US_Nucleotide_DB::enter_DNA()
{
	if(DNA_info.InvID <= 0)
	{
		QMessageBox::message(tr("Attention:"), tr("Please 'Select Investigator' first!"));
		return;
	}
	enterDNADlg = new US_Enter_DNA_DB(DNA_info.InvID);
	enterDNADlg->exec();
}

/*!
	Open US_DB_Admin to check delete permission.
*/
void US_Nucleotide_DB::check_permission()
{
	US_DB_Admin *db_admin;
	db_admin = new US_DB_Admin("");
	db_admin->show();
	connect(db_admin, SIGNAL(issue_permission(bool)), SLOT(delete_db(bool)));
}

/*! Delete selected entry of DB table: <tt>tblDNA</tt>. */
void US_Nucleotide_DB::delete_db(bool permission)
{
	if(!permission)
	{
		QMessageBox::message(tr("Attention:"), 
									tr("Permission denied"));
		return;
	}
	int j = Item;
	int id = DNA_info.DNAID;
	if(id == -1)
	{
		QMessageBox::message(tr("Attention:"), 
									tr("Please first select one file which\n"
										" you like to delete from database"));
										
	}
	else
	{
	/*	QSqlCursor cur( "tblDNA");
		cur.setMode( QSqlCursor::Delete);
		QSqlIndex filter = cur.index("DNAID");
		cur.setValue("DNAID", id);
		cur.select(filter);
		cur.primeDelete();
	*/
		switch(QMessageBox::information(this, tr("Please confirm:"), 
										tr("Clicking 'OK' will delete the selected record from the database"),
										tr("OK"), tr("CANCEL"),	0,1))
		{
			case 0:
			{
				//cur.del();
				QSqlQuery del;
				QString str;
				str.sprintf("DELETE FROM tblDNA WHERE DNAID = %d;", id);
				bool done=del.exec(str);
				if(!done)
				{
					QSqlError sqlerr = del.lastError();
					QMessageBox::message(tr("Attention:"),
                           tr("Delete failed.\n"
                              "Attempted to execute this command:\n\n"
                              + str + "\n\n"
                              "Causing the following error:\n\n")
                              + sqlerr.text());
				}


				lb_DNA->removeItem(j);
				clear();
				break;
			}	
			case 1:
			{
				break;
			}
		}
	}			  	
}

/*! Initialize all variables. */
void US_Nucleotide_DB::clear()
{
	DNA_info.DNAID = -1;
	DNA_info.InvID = -1;
	DNA_info.Description = "";
	DNA_info.Sequence = "";
	doubleStranded = true;
	complement = false;
	cb_doubleStranded->setChecked(true);
	cb_complement->setChecked(false);
	_3prime_oh = true;
	cb_3prime_oh->setChecked(true);
	cb_3prime_po4->setChecked(false);
	_5prime_oh = false;
	cb_5prime_oh->setChecked(false);
	cb_5prime_po4->setChecked(true);
	isDNA = true;
	cb_RNA->setChecked(false);
	cb_DNA->setChecked(true);
	sodium = 0.0;
	cnt_sodium->setValue(sodium);
	potassium = 0.0;
	cnt_potassium->setValue(potassium);
	lithium = 0.0;
	cnt_lithium->setValue(lithium);
	magnesium = 0.0;
	cnt_magnesium->setValue(magnesium);
	calcium = 0.0;
	cnt_calcium->setValue(calcium);
	le_sequence->setText("");
	le_mw->setText("");
	lb_DNA->clear();
	pb_investigator->setEnabled(true);
	lbl_investigator->setText(" Not Selected");
	from_HD = false;
	
}

/*! Reset all variables to start values */
void US_Nucleotide_DB::reset()
{
	lb_DNA->clear();
	clear();
	emit IdChanged(DNA_info.DNAID);
}

void US_Nucleotide_DB::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/nucleotide.html");
}

/*! Add warning message when DNA data is not loaded from DB and emit <var>DNAID</var> = -1 to US_Cell_DB. */
void US_Nucleotide_DB::quit()
{
	if(cell_flag && from_HD)
	{
		emit IdChanged(-1);
		QMessageBox::message(tr("Attention:"), 
									tr("Selecting a sequence file from the harddrive will not\n"
										"associate the data from this channel with this file.\n\n"
										"In order to associate a sequence file with your data,\n"
										"you need to select it from the database."));
		return;								
	}
	close();
}

/*!
	This event handler, for event <var>e</var>, to receive widget close events.
*/
void US_Nucleotide_DB::closeEvent(QCloseEvent *e)
{
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
}


void US_Nucleotide_DB::update_vbar(const QString & str)
{
	vbar = str.toFloat();
}

void US_Nucleotide_DB::update_e280(const QString & str)
{
	e280 = str.toFloat();
}

void US_Nucleotide_DB::update_e260(const QString & str)
{
	e260 = str.toFloat();
}

