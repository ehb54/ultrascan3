#include "../include/us_merge.h"

US_Merge::US_Merge(QWidget *parent, const char* name) : QFrame(parent, name)
{
	
	USglobal = new US_Config();
	QString item;
	int i;
	int buttonh = 26;

	ready = false;
	move_data = true;

	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));

	for (i=0; i<8; i++)
	{
		scan_count1[i] = 0;
		scan_count2[i] = 0;
		out_of_sequence1[i] = false;
		out_of_sequence2[i] = false;
	}
	data_type = 1;
	setCaption(tr("Scanfile Ordering and Merging Utility"));

	lbl_header = new QLabel(tr("Scanfile Ordering and Merging Utility"), this);
	lbl_header->setAlignment(AlignCenter|AlignVCenter);
	lbl_header->setFrameStyle(QFrame::WinPanel|Raised);
	lbl_header->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_header->setMinimumHeight(30);
	lbl_header->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	bt_abs = new QRadioButton(this);
	bt_abs->setText(tr(" Absorbance"));
	bt_abs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1)); //, QFont::Bold));
	bt_abs->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	bt_abs->setChecked(true);
	connect(bt_abs, SIGNAL(clicked()), SLOT(update_abs_button()));


	bt_if = new QRadioButton(this);
	bt_if->setText(tr(" Interference"));
	bt_if->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1)); //, QFont::Bold));
	bt_if->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	bt_if->setChecked(false);
	connect(bt_if, SIGNAL(clicked()), SLOT(update_if_button()));

	bt_wl = new QRadioButton(this);
	bt_wl->setText(tr(" Wavelength"));
	bt_wl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1)); //, QFont::Bold));
	bt_wl->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	bt_wl->setChecked(false);
	connect(bt_wl, SIGNAL(clicked()), SLOT(update_wl_button()));

	pb_help = new QPushButton(tr("Help"), this);
	Q_CHECK_PTR(pb_help);
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	pb_quit = new QPushButton(tr("Quit"), this);
	Q_CHECK_PTR(pb_quit);
	pb_quit->setAutoDefault(false);
	pb_quit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_quit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_quit, SIGNAL(clicked()), SLOT(quit()));

	bt_move = new QRadioButton(this);
	bt_move->setText(tr("Move"));
	bt_move->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1)); //, QFont::Bold));
	bt_move->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	bt_move->setChecked(true);
	connect(bt_move, SIGNAL(clicked()), SLOT(update_move_button()));

	bt_copy = new QRadioButton(this);
	bt_copy->setText(tr("Copy"));
	bt_copy->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1)); //, QFont::Bold));
	bt_copy->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	bt_copy->setChecked(false);
	connect(bt_copy, SIGNAL(clicked()), SLOT(update_copy_button()));

	lbl_progress = new QLabel(tr("Status:"),this);
	lbl_progress->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_progress->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_progress->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	pgb_progress = new QProgressBar(this, "iteration progress");
	pgb_progress->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	pgb_progress->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));


	pb_merge = new QPushButton(tr("Merge Cell (append dir. 2 data to dir. 1)"), this);
	Q_CHECK_PTR(pb_merge);
	pb_merge->setAutoDefault(false);
	pb_merge->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_merge->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_merge->setEnabled(false);
	connect(pb_merge, SIGNAL(clicked()), SLOT(merge()));

	pb_select_dir1 = new QPushButton(tr("Select Directory 1"), this);
	Q_CHECK_PTR(pb_select_dir1);
	pb_select_dir1->setAutoDefault(false);
	pb_select_dir1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_select_dir1->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_select_dir1, SIGNAL(clicked()), SLOT(select_dir1()));

	lbl_directory1 = new QLabel(tr("<not selected>"),this);
	lbl_directory1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_directory1->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_directory1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	pb_select_dir2 = new QPushButton(tr("Select Directory 2"), this);
	Q_CHECK_PTR(pb_select_dir2);
	pb_select_dir2->setAutoDefault(false);
	pb_select_dir2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_select_dir2->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_select_dir2, SIGNAL(clicked()), SLOT(select_dir2()));

	lbl_directory2 = new QLabel(tr("<not selected>"),this);
	lbl_directory2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_directory2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_directory2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));


	selected_cell1 = 0;
	lb_dir1_cells = new QListBox(this, "Cell");
	lb_dir1_cells->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	lb_dir1_cells->setMinimumHeight(3*buttonh + 2*spacing);
	for (i=0; i<8; i++)
	{
		item.sprintf(tr("Cell %d is empty"), i+1);
		lb_dir1_cells->insertItem(item);
	}
	lb_dir1_cells->setSelected(0, TRUE);
	lb_dir1_cells->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	lb_dir1_cells->setEnabled(false);
	connect(lb_dir1_cells, SIGNAL(highlighted(int)), SLOT(show_dir1_cell(int)));

	lbl_message1 = new QLabel(tr("Please select the 1. Directory by\nclicking on \"Select Directory 1\" "
	"now.\nThen select the cell to be merged."), this);
	lbl_message1->setAlignment(AlignCenter|AlignVCenter);
	lbl_message1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_message1->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_message1->setMinimumHeight(3*buttonh + 2*spacing);
	lbl_message1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	selected_cell2 = 0;
	lb_dir2_cells = new QListBox(this, "Cell");
	lb_dir2_cells->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	lb_dir2_cells->setMinimumHeight(3*buttonh + 2*spacing);
	for (i=0; i<8; i++)
	{
		item.sprintf(tr("Cell %d is empty"), i+1);
		lb_dir2_cells->insertItem(item);
	}
	lb_dir2_cells->setSelected(0, TRUE);
	lb_dir2_cells->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	lb_dir2_cells->setEnabled(false);
	connect(lb_dir2_cells, SIGNAL(highlighted(int)), SLOT(show_dir2_cell(int)));

	lbl_message2 = new QLabel(tr("Please select the 2. Directory by\nclicking on \"Select Directory 2\" "
	"now.\nThen select the cell to be appended\nto the selected cell of Directory 1."), this);
	lbl_message2->setAlignment(AlignCenter|AlignVCenter);
	lbl_message2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_message2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_message2->setMinimumHeight(3*buttonh + 2*spacing);
	lbl_message2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));


	lbl_dir1_info1 = new QLabel(tr("Cell Contents:"),this);
	lbl_dir1_info1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_dir1_info1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_dir1_info1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_dir1_info2 = new QLabel(tr("<not selected>"),this);
	lbl_dir1_info2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_dir1_info2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_dir1_info2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	lbl_dir2_info1 = new QLabel(tr("Cell Contents:"),this);
	lbl_dir2_info1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_dir2_info1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_dir2_info1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_dir2_info2 = new QLabel(tr("<not selected>"),this);
	lbl_dir2_info2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_dir2_info2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_dir2_info2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));


	lbl_dir1_cells1 = new QLabel(tr("Total Cells:"),this);
	lbl_dir1_cells1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_dir1_cells1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_dir1_cells1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_dir1_cells2 = new QLabel(tr("<not selected>"),this);
	lbl_dir1_cells2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_dir1_cells2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_dir1_cells2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));


	lbl_dir2_cells1 = new QLabel(tr("Total Cells:"),this);
	lbl_dir2_cells1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_dir2_cells1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_dir2_cells1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_dir2_cells2 = new QLabel(tr("<not selected>"),this);
	lbl_dir2_cells2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_dir2_cells2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_dir2_cells2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	pb_check_dir1_lambda = new QPushButton(tr("Check Wavelength(s)"), this);
	Q_CHECK_PTR(pb_check_dir1_lambda);
	pb_check_dir1_lambda->setAutoDefault(false);
	pb_check_dir1_lambda->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_check_dir1_lambda->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_check_dir1_lambda->setEnabled(false);
	connect(pb_check_dir1_lambda, SIGNAL(clicked()), SLOT(check_dir1_lambda()));

	lbl_dir1_lambda1 = new QLabel(tr("Wavelength(s):"),this);
	lbl_dir1_lambda1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_dir1_lambda1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_dir1_lambda1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_dir1_lambda2 = new QLabel(tr("<not checked>"),this);
	lbl_dir1_lambda2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_dir1_lambda2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_dir1_lambda2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	pb_check_dir2_lambda = new QPushButton(tr("Check Wavelength(s)"), this);
	Q_CHECK_PTR(pb_check_dir2_lambda);
	pb_check_dir2_lambda->setAutoDefault(false);
	pb_check_dir2_lambda->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_check_dir2_lambda->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_check_dir2_lambda->setEnabled(false);
	connect(pb_check_dir2_lambda, SIGNAL(clicked()), SLOT(check_dir2_lambda()));

	lbl_dir2_lambda1 = new QLabel(tr("Wavelength(s):"),this);
	lbl_dir2_lambda1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_dir2_lambda1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_dir2_lambda1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_dir2_lambda2 = new QLabel(tr("<not checked>"),this);
	lbl_dir2_lambda2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_dir2_lambda2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_dir2_lambda2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	
	pb_order_dir1_cell = new QPushButton(tr("Order This Cell"), this);
	Q_CHECK_PTR(pb_order_dir1_cell);
	pb_order_dir1_cell->setAutoDefault(false);
	pb_order_dir1_cell->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_order_dir1_cell->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_order_dir1_cell->setEnabled(false);
	connect(pb_order_dir1_cell, SIGNAL(clicked()), SLOT(order_dir1_cell()));

	lbl_dir1_scans1 = new QLabel(tr("Total Scans:"),this);
	lbl_dir1_scans1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_dir1_scans1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_dir1_scans1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_dir1_scans2 = new QLabel(tr("<not selected>"),this);
	lbl_dir1_scans2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_dir1_scans2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_dir1_scans2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	
	pb_order_dir2_cell = new QPushButton(tr("Order This Cell"), this);
	Q_CHECK_PTR(pb_order_dir2_cell);
	pb_order_dir2_cell->setAutoDefault(false);
	pb_order_dir2_cell->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_order_dir2_cell->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_order_dir2_cell->setEnabled(false);
	connect(pb_order_dir2_cell, SIGNAL(clicked()), SLOT(order_dir2_cell()));

	lbl_dir2_scans1 = new QLabel(tr("Total Scans:"),this);
	lbl_dir2_scans1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_dir2_scans1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_dir2_scans1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_dir2_scans2 = new QLabel(tr("<not selected>"),this);
	lbl_dir2_scans2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_dir2_scans2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_dir2_scans2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	
	
	pb_order_dir1_all = new QPushButton(tr("Order All Cells"), this);
	Q_CHECK_PTR(pb_order_dir1_all);
	pb_order_dir1_all->setAutoDefault(false);
	pb_order_dir1_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_order_dir1_all->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_order_dir1_all->setEnabled(false);
	connect(pb_order_dir1_all, SIGNAL(clicked()), SLOT(order_dir1_all()));

	lbl_dir1_first_scan1 = new QLabel(tr("First Scanfile:"),this);
	lbl_dir1_first_scan1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_dir1_first_scan1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_dir1_first_scan1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_dir1_first_scan2 = new QLabel(tr("<not selected>"),this);
	lbl_dir1_first_scan2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_dir1_first_scan2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_dir1_first_scan2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	
	pb_order_dir2_all = new QPushButton(tr("Order All Cells"), this);
	Q_CHECK_PTR(pb_order_dir2_all);
	pb_order_dir2_all->setAutoDefault(false);
	pb_order_dir2_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_order_dir2_all->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_order_dir2_all->setEnabled(false);
	connect(pb_order_dir2_all, SIGNAL(clicked()), SLOT(order_dir2_all()));

	lbl_dir2_first_scan1 = new QLabel(tr("First Scanfile:"),this);
	lbl_dir2_first_scan1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_dir2_first_scan1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_dir2_first_scan1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_dir2_first_scan2 = new QLabel(tr("<not selected>"),this);
	lbl_dir2_first_scan2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_dir2_first_scan2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_dir2_first_scan2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	lbl_dir1_last_scan1 = new QLabel(tr("Last Scanfile:"),this);
	lbl_dir1_last_scan1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_dir1_last_scan1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_dir1_last_scan1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_dir1_last_scan2 = new QLabel(tr("<not selected>"),this);
	lbl_dir1_last_scan2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_dir1_last_scan2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_dir1_last_scan2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));


	lbl_dir2_last_scan1 = new QLabel(tr("Last Scanfile:"),this);
	lbl_dir2_last_scan1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_dir2_last_scan1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_dir2_last_scan1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));


	lbl_dir2_last_scan2 = new QLabel(tr("<not selected>"),this);
	lbl_dir2_last_scan2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_dir2_last_scan2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_dir2_last_scan2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	setup_GUI();
}

US_Merge::~US_Merge()
{
}
void US_Merge::setup_GUI()
{
	int j=0;
	int rows = 10, columns = 6, spacing = 2;
	
	QGridLayout * background = new QGridLayout(this, rows, columns, spacing);
	for (int i=0; i<rows; i++)
	{
		background->setRowSpacing(i, 26);
	}
	background->addMultiCellWidget(lbl_header,j,j,0,5);
	j++;
	QHBoxLayout *cb1 = new QHBoxLayout(0);
	cb1->addWidget(bt_abs);
	cb1->addWidget(bt_if);
	cb1->addWidget(bt_wl);
	background->addMultiCellLayout(cb1,j,j,0,2);
	background->addWidget(pb_help,j,3);
	background->addWidget(pb_quit,j,4);
	QVBoxLayout *cb2 = new QVBoxLayout(0);
	cb2->addWidget(bt_move);
	cb2->addWidget(bt_copy);
	background->addMultiCellLayout(cb2,j,j+1,5,5);
	j++;
	background->addWidget(lbl_progress,j,0);
	background->addMultiCellWidget(pgb_progress,j,j,1,2);
	background->addMultiCellWidget(pb_merge,j,j,3,4);
	j++;
	background->addWidget(pb_select_dir1,j,0);
	background->addMultiCellWidget(lbl_directory1,j,j,1,2);
	background->addWidget(pb_select_dir2,j,3);
	background->addMultiCellWidget(lbl_directory2,j,j,4,5);
	j++;
	background->addMultiCellWidget(lb_dir1_cells,j,j,0,0);
	background->addMultiCellWidget(lbl_message1,j,j,1,2);
	background->addMultiCellWidget(lb_dir2_cells,j,j,3,3);
	background->addMultiCellWidget(lbl_message2,j,j,4,5);
	j++;
	background->addWidget(lbl_dir1_info1,j,0);
	background->addMultiCellWidget(lbl_dir1_info2,j,j,1,2);
	background->addWidget(lbl_dir2_info1,j,3);
	background->addMultiCellWidget(lbl_dir2_info2,j,j,4,5);
	j++;
	background->addWidget(lbl_dir1_cells1,j,0);
	background->addMultiCellWidget(lbl_dir1_cells2,j,j,1,2);
	background->addWidget(lbl_dir2_cells1,j,3);
	background->addMultiCellWidget(lbl_dir2_cells2,j,j,4,5);
	j++;
	background->addWidget(pb_check_dir1_lambda,j,0);
	background->addWidget(lbl_dir1_lambda1,j,1);
	background->addWidget(lbl_dir1_lambda2,j,2);
	background->addWidget(pb_check_dir2_lambda,j,3);
	background->addWidget(lbl_dir2_lambda1,j,4);
	background->addWidget(lbl_dir2_lambda2,j,5);
	j++;
	background->addWidget(pb_order_dir1_cell,j,0);
	background->addWidget(lbl_dir1_scans1,j,1);
	background->addWidget(lbl_dir1_scans2,j,2);
	background->addWidget(pb_order_dir2_cell,j,3);
	background->addWidget(lbl_dir2_scans1,j,4);
	background->addWidget(lbl_dir2_scans2,j,5);
	j++;
	background->addWidget(pb_order_dir1_all,j,0);
	background->addWidget(lbl_dir1_first_scan1,j,1);
	background->addWidget(lbl_dir1_first_scan2,j,2);
	background->addWidget(pb_order_dir2_all,j,3);
	background->addWidget(lbl_dir2_first_scan1,j,4);
	background->addWidget(lbl_dir2_first_scan2,j,5);
	j++;
	background->addWidget(lbl_dir1_last_scan1,j,1);
	background->addWidget(lbl_dir1_last_scan2,j,2);
	background->addWidget(lbl_dir2_last_scan1,j,4);
	background->addWidget(lbl_dir2_last_scan2,j,5);

	/*qApp->processEvents();
	QRect r = background->cellGeometry(0, 0);
	
	global_Xpos += 30;
	global_Ypos += 30;
	
	this->setGeometry(global_Xpos, global_Ypos, r.width(), r.height());
*/
}


void US_Merge::closeEvent(QCloseEvent *e)
{
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
}

void US_Merge::select_dir1()
{
	int i, j, cell_count, count;
	QFileDialog *fd;
	QString filter;
	QFile f;
	QString trashcan, item, message;
	QString str, str1, str2, str3;
	bool flag = false;
	switch (data_type)
	{
		case 1: 
		{
			filter = "*.RA?, *.ra?";
			break;
		}
		case 2: 
		{
			filter = "*.IP?, *.ip?";
			break;
		}
		case 3: 
		{
			filter = "*.WA?, *.wa?";
			break;
		}
	}
	fd = new QFileDialog(USglobal->config_list.data_dir, filter, 0, 0, TRUE);
	directory1 = fd->getExistingDirectory(USglobal->config_list.data_dir, 0, 0, 
	"Please select the directory with the 1. set of AUC data files:", false, true);
	if (directory1.isNull())
	{
		return;
	}
	data_dir1.setPath(directory1);
	if (data_type == 1)
	{
		filter = "*.RA?";
		data_dir1.setNameFilter(filter);
		if (data_dir1.count() == 0) // doesn't contain any upper case absorbance data, lets check for lower
		{
			filter = "*.ra?";
			data_dir1.setNameFilter(filter);
			if (data_dir1.count() == 0) // doesn't contain any lower case absorbance data either - give up
			{
				directory1 = "";
				for (i=0; i<8; i++)
				{
					item.sprintf(tr("Cell %d is empty"), i+1);
					lb_dir1_cells->changeItem(item, i);
					scan_count1[i] = 0;
				}
				lb_dir1_cells->setCurrentItem(0);
				lb_dir1_cells->setEnabled(false);
				lbl_message1->setText(tr("This Directory doesn't contain any\n"
				"absorbance data. Please select a different\ndirectory - click on \"Select Directory 1\"."));
				lbl_dir1_scans2->setText(tr("<not selected>"));
				lbl_dir1_cells2->setText(tr("<not selected>"));
				lbl_directory1->setText(tr("<not selected>"));
				lbl_dir1_lambda2->setText(tr("<not checked>"));
				lbl_dir1_info2->setText(tr("<not selected>"));
				lbl_dir1_first_scan2->setText(tr("<not selected>"));
				lbl_dir1_last_scan2->setText(tr("<not selected>"));
				return;
			}
			else
			{
				extension1 = "ra";
			}
		}
		else
		{
			extension1 = "RA";
		}
	}
	if (data_type == 2)
	{
		filter = "*.IP?";
		data_dir1.setNameFilter(filter);
		if (data_dir1.count() == 0) // doesn't contain any upper case absorbance data, lets check for lower
		{
			filter = "*.ip?";
			data_dir1.setNameFilter(filter);
			if (data_dir1.count() == 0) // doesn't contain any lower case absorbance data either - give up
			{
				directory1 = "";
				for (i=0; i<8; i++)
				{
					item.sprintf(tr("Cell %d is empty"), i+1);
					lb_dir1_cells->changeItem(item, i);
					scan_count1[i] = 0;
				}
				lb_dir1_cells->setCurrentItem(0);
				lb_dir1_cells->setEnabled(false);
				lbl_message1->setText(tr("This Directory doesn't contain any\n"
				"interference data. Please select a different\ndirectory - click on \"Select Directory 1\"."));
				lbl_dir1_cells2->setText(tr("<not selected>"));
				lbl_directory1->setText(tr("<not selected>"));
				lbl_dir1_lambda2->setText(tr("<not checked>"));
				lbl_dir1_info2->setText(tr("<not selected>"));
				lbl_dir1_scans2->setText(tr("<not selected>"));
				lbl_dir1_first_scan2->setText(tr("<not selected>"));
				lbl_dir1_last_scan2->setText(tr("<not selected>"));
				return;
			}
			else
			{
				extension1 = "ip";
			}
		}
		else
		{
			extension1 = "IP";
		}
	}
	if (data_type == 3)
	{
		filter = "*.WA?";
		data_dir1.setNameFilter(filter);
		if (data_dir1.count() == 0) // doesn't contain any upper case wavelength data, lets check for lower
		{
			filter = "*.wa?";
			data_dir1.setNameFilter(filter);
			if (data_dir1.count() == 0) // doesn't contain any lower case wavelength data either - give up
			{
				directory1 = "";
				for (i=0; i<8; i++)
				{
					item.sprintf(tr("Cell %d is empty"), i+1);
					lb_dir1_cells->changeItem(item, i);
					scan_count1[i] = 0;
				}
				lb_dir1_cells->setCurrentItem(0);
				lb_dir1_cells->setEnabled(false);
				lbl_message1->setText(tr("This Directory doesn't contain any\n"
				"wavelength data. Please select a different\ndirectory - click on \"Select Directory 1\"."));
				lbl_dir1_scans2->setText(tr("<not selected>"));
				lbl_dir1_cells2->setText(tr("<not selected>"));
				lbl_directory1->setText(tr("<not selected>"));
				lbl_dir1_lambda2->setText(tr("<not checked>"));
				lbl_dir1_info2->setText(tr("<not selected>"));
				lbl_dir1_first_scan2->setText(tr("<not selected>"));
				lbl_dir1_last_scan2->setText(tr("<not selected>"));
				return;
			}
			else
			{
				extension1 = "wa";
			}
		}
		else
		{
			extension1 = "WA";
		}
	}

	lb_dir1_cells->setEnabled(true);
	message = tr("The following cells of this run\n contain out-of-sequence scans:\nCell ");
	lbl_directory1->setText(directory1);
	for (i=0; i<8; i++)
	{
		str = "*.";
		str.append(extension1);
		str1.sprintf("%d", i+1);
		str.append(str1);
		data_dir1.setPath(directory1);
		data_dir1.setNameFilter(str);
		scan_count1[i] = data_dir1.count();
	}
	cell_count = 0;
	for (i=0; i<8; i++)
	{
		if(scan_count1[i] > 0)
		{
			item.sprintf(tr("Cell %d contains data"), i+1);
			lb_dir1_cells->changeItem(item, i);
			cell_count++;
		}
		else
		{
			item.sprintf(tr("Cell %d is empty"), i+1);
			lb_dir1_cells->changeItem(item, i);
		}
		str.sprintf(tr("%d Cells with Data"), cell_count);
		lbl_dir1_cells2->setText(str);
	}
	i=0;
	while (scan_count1[i] < 1)
	{
		i++;
	}
	if (i > 7)
	{
		directory1 = "";
		for (i=0; i<8; i++)
		{
			item.sprintf(tr("Cell %d is empty"), i+1);
			lb_dir1_cells->changeItem(item, i);
			scan_count1[i] = 0;
		}
		lbl_message1->setText(tr("Sorry - this directory doesn't contain\nany scanfiles.\nPlease try again..."));
		lbl_dir1_scans2->setText(tr("<not selected>"));
		lbl_dir1_cells2->setText(tr("<not selected>"));
		lbl_directory1->setText(tr("<not selected>"));
		lbl_dir1_lambda2->setText(tr("<not checked>"));
		lbl_dir1_info2->setText(tr("<not selected>"));
		lbl_dir1_first_scan2->setText(tr("<not selected>"));
		lbl_dir1_last_scan2->setText(tr("<not selected>"));
		return;
	}
	else
	{
		lb_dir1_cells->setCurrentItem(i);
		for (j=0; j<8; j++)
		{
			if (scan_count1[j] > 0)
			{
				str = "*.";
				str.append(extension1);
				str1.sprintf("%d", j+1);
				str.append(str1);
				data_dir1.setNameFilter(str);
				data_dir1.setSorting(QDir::Name);
				str2 = data_dir1[scan_count1[j]-1];
				str2.truncate(5);
				count = str2.toInt();
				if (count != scan_count1[j])
				{
					flag = true;
					out_of_sequence1[j] = true;
					pb_check_dir1_lambda->setEnabled(true);
					pb_order_dir1_cell->setEnabled(true);
					pb_order_dir1_all->setEnabled(true);
					str3.sprintf("%d, ", j+1);
					message.append(str3);
				}
				else
				{
					out_of_sequence1[j] = false;
					pb_check_dir1_lambda->setEnabled(false);
					pb_order_dir1_cell->setEnabled(false);
					pb_order_dir1_all->setEnabled(false);
				}
			}
		}
		lb_dir1_cells->setCurrentItem(i);
		show_dir1_cell(i);
		if (flag)
		{
			message.truncate(message.length() - 2);
			lbl_message1->setText(message);
		}
		else
		{
			lbl_message1->setText(tr("All scanfiles of this run are in\n the proper sequence.\nNo ordering is necessary."));
		}
	}
}

void US_Merge::select_dir2()
{
	int i, j, cell_count, count;
	QFileDialog *fd;
	QString filter;
	QFile f;
	bool flag = false;
	QString trashcan, item, message;
	QString str, str1, str2, str3;
	if (data_type == 1)
	{
		filter = "*.RA?, *.ra?";
	}
	else if (data_type == 2)
	{
		filter = "*.IP?, *.ip?";
	}
	fd = new QFileDialog(USglobal->config_list.data_dir, filter, 0, 0, TRUE);
	directory2 = fd->getExistingDirectory(USglobal->config_list.data_dir, 0, 0, 
	"Please select the directory with the 2. set of AUC data files:", false, true);
	if (directory2.isNull())
	{
		return;
	}
	data_dir2.setPath(directory2);
	if (data_type == 1)
	{
		filter = "*.RA?";
		data_dir2.setNameFilter(filter);
		if (data_dir2.count() == 0) // doesn't contain any upper case absorbance data, lets check for lower
		{
			filter = "*.ra?";
			data_dir2.setNameFilter(filter);
			if (data_dir2.count() == 0) // doesn't contain any lower case absorbance data either - give up
			{
				directory2 = "";
				for (i=0; i<8; i++)
				{
					item.sprintf(tr("Cell %d is empty"), i+1);
					lb_dir2_cells->changeItem(item, i);
					scan_count2[i] = 0;
				}
				lb_dir2_cells->setCurrentItem(0);
				lb_dir2_cells->setEnabled(false);
				lbl_message2->setText(tr("This Directory doesn't contain any\n"
				"absorbance data. Please select a different\ndirectory - click on \"Select Directory 2\"."));
				lbl_dir2_scans2->setText(tr("<not selected>"));
				lbl_dir2_cells2->setText(tr("<not selected>"));
				lbl_directory2->setText(tr("<not selected>"));
				lbl_dir2_lambda2->setText(tr("<not checked>"));
				lbl_dir2_info2->setText(tr("<not selected>"));
				lbl_dir2_first_scan2->setText(tr("<not selected>"));
				lbl_dir2_last_scan2->setText(tr("<not selected>"));
				return;
			}
			else
			{
				extension2 = "ra";
			}
		}
		else
		{
			extension2 = "RA";
		}
	}
	if (data_type == 2)
	{
		filter = "*.IP?";
		data_dir2.setNameFilter(filter);
		if (data_dir2.count() == 0) // doesn't contain any upper case absorbance data, lets check for lower
		{
			filter = "*.ip?";
			data_dir2.setNameFilter(filter);
			if (data_dir2.count() == 0) // doesn't contain any lower case absorbance data either - give up
			{
				directory2 = "";
				for (i=0; i<8; i++)
				{
					item.sprintf(tr("Cell %d is empty"), i+1);
					lb_dir2_cells->changeItem(item, i);
					scan_count2[i] = 0;
				}
				lb_dir2_cells->setCurrentItem(0);
				lb_dir2_cells->setEnabled(false);
				lbl_message2->setText(tr("This Directory doesn't contain any\n"
				"interference data. Please select a different\ndirectory - click on \"Select Directory 2\"."));
				lbl_dir2_cells2->setText(tr("<not selected>"));
				lbl_directory2->setText(tr("<not selected>"));
				lbl_dir2_lambda2->setText(tr("<not checked>"));
				lbl_dir2_info2->setText(tr("<not selected>"));
				lbl_dir2_scans2->setText(tr("<not selected>"));
				lbl_dir2_first_scan2->setText(tr("<not selected>"));
				lbl_dir2_last_scan2->setText(tr("<not selected>"));
				return;
			}
			else
			{
				extension2 = "ip";
			}
		}
		else
		{
			extension2 = "IP";
		}
	}
	if (data_type == 3)
	{
		filter = "*.WA?";
		data_dir2.setNameFilter(filter);
		if (data_dir2.count() == 0) // doesn't contain any upper case absorbance data, lets check for lower
		{
			filter = "*.wa?";
			data_dir2.setNameFilter(filter);
			if (data_dir2.count() == 0) // doesn't contain any lower case absorbance data either - give up
			{
				directory2 = "";
				for (i=0; i<8; i++)
				{
					item.sprintf(tr("Cell %d is empty"), i+1);
					lb_dir2_cells->changeItem(item, i);
					scan_count2[i] = 0;
				}
				lb_dir2_cells->setCurrentItem(0);
				lb_dir2_cells->setEnabled(false);
				lbl_message2->setText(tr("This Directory doesn't contain any\n"
				"wavelength data. Please select a different\ndirectory - click on \"Select Directory 2\"."));
				lbl_dir2_scans2->setText(tr("<not selected>"));
				lbl_dir2_cells2->setText(tr("<not selected>"));
				lbl_directory2->setText(tr("<not selected>"));
				lbl_dir2_lambda2->setText(tr("<not checked>"));
				lbl_dir2_info2->setText(tr("<not selected>"));
				lbl_dir2_first_scan2->setText(tr("<not selected>"));
				lbl_dir2_last_scan2->setText(tr("<not selected>"));
				return;
			}
			else
			{
				extension2 = "wa";
			}
		}
		else
		{
			extension2 = "WA";
		}
	}

	lb_dir2_cells->setEnabled(true);
	message = tr("The following cells of this run\n contain out-of-sequence scans:\nCell ");
	lbl_directory2->setText(directory2);
	for (i=0; i<8; i++)
	{
		str = "*.";
		str.append(extension2);
		str1.sprintf("%d", i+1);
		str.append(str1);
		data_dir2.setPath(directory2);
		data_dir2.setNameFilter(str);
		scan_count2[i] = data_dir2.count();
	}
	cell_count = 0;
	for (i=0; i<8; i++)
	{
		if(scan_count2[i] > 0)
		{
			item.sprintf(tr("Cell %d contains data"), i+1);
			lb_dir2_cells->changeItem(item, i);
			cell_count++;
		}
		else
		{
			item.sprintf(tr("Cell %d is empty"), i+1);
			lb_dir2_cells->changeItem(item, i);
		}
		str.sprintf(tr("%d Cells with Data"), cell_count);
		lbl_dir2_cells2->setText(str);
	}
	i=0;
	while (scan_count2[i] < 1)
	{
		i++;
	}
	if (i > 7)
	{
		directory2 = "";
		for (i=0; i<8; i++)
		{
			item.sprintf(tr("Cell %d is empty"), i+1);
			lb_dir2_cells->changeItem(item, i);
			scan_count2[i] = 0;
		}
		lbl_message2->setText(tr("Sorry - this directory doesn't contain\nany scanfiles.\nPlease try again..."));
		lbl_dir2_scans2->setText(tr("<not selected>"));
		lbl_dir2_cells2->setText(tr("<not selected>"));
		lbl_directory2->setText(tr("<not selected>"));
		lbl_dir2_lambda2->setText(tr("<not checked>"));
		lbl_dir2_info2->setText(tr("<not selected>"));
		lbl_dir2_first_scan2->setText(tr("<not selected>"));
		lbl_dir2_last_scan2->setText(tr("<not selected>"));
		return;
	}
	else
	{
		lb_dir2_cells->setCurrentItem(i);
		for (j=0; j<8; j++)
		{
			if (scan_count2[j] > 0)
			{
				str = "*.";
				str.append(extension2);
				str1.sprintf("%d", j+1);
				str.append(str1);
				data_dir2.setNameFilter(str);
				data_dir2.setSorting(QDir::Name);
				str2 = data_dir2[scan_count2[j]-1];
				str2.truncate(5);
				count = str2.toInt();
				if (count != scan_count2[j])
				{
					flag = true;
					out_of_sequence2[j] = true;
					pb_check_dir2_lambda->setEnabled(true);
					pb_order_dir2_cell->setEnabled(true);
					pb_order_dir2_all->setEnabled(true);
					str3.sprintf("%d, ", j+1);
					message.append(str3);
				}
				else
				{
					out_of_sequence2[j] = false;
					pb_check_dir2_lambda->setEnabled(false);
					pb_order_dir2_cell->setEnabled(false);
					pb_order_dir2_all->setEnabled(false);
				}
			}
		}
		lb_dir2_cells->setCurrentItem(i);
		show_dir2_cell(i);
		if (flag)
		{
			message.truncate(message.length() - 2);
			lbl_message2->setText(message);
		}
		else
		{
			lbl_message2->setText(tr("All scanfiles of this run are in\n the proper sequence.\nNo ordering is necessary."));
		}
	}
}

void US_Merge::update_abs_button()
{
	bt_if->setChecked(false);
	bt_wl->setChecked(false);
	data_type = 1;
}

void US_Merge::update_if_button()
{
	bt_abs->setChecked(false);
	bt_wl->setChecked(false);
	pb_check_dir1_lambda->setEnabled(false);
	pb_check_dir2_lambda->setEnabled(false);
	data_type = 2;
}

void US_Merge::update_wl_button()
{
	bt_abs->setChecked(false);
	bt_if->setChecked(false);
	pb_check_dir1_lambda->setEnabled(false);
	pb_check_dir2_lambda->setEnabled(false);
	data_type = 3;
}

void US_Merge::update_move_button()
{
	bt_copy->setChecked(false);
	move_data = true;
}

void US_Merge::update_copy_button()
{
	bt_move->setChecked(false);
	move_data = false;
}

void US_Merge::show_dir1_cell(int cell)
{
	QString str, str1, filename;
	lbl_dir1_lambda2->setText(tr("<not checked>"));
	selected_cell1 = cell;
	str.sprintf(tr("%d Scans in Cell %d"), scan_count1[selected_cell1], selected_cell1+1);
	lbl_dir1_scans2->setText(str);
	if (scan_count1[selected_cell1] > 0 && scan_count2[selected_cell2] > 0)
	{
		pb_merge->setEnabled(true);
	}
	else
	{
		pb_merge->setEnabled(false);
	}
	if (out_of_sequence1[cell])
	{
		lbl_message1->setText(str.sprintf(tr("Cell %d contains scan files\nwhich are out of sequence."), cell + 1));
		pb_check_dir1_lambda->setEnabled(true);
		pb_order_dir1_cell->setEnabled(true);
		pb_order_dir1_all->setEnabled(true);
	}
	else
	{
		lbl_message1->setText(str.sprintf(tr("All scans in cell %d\nare in sequence"), cell + 1));
		pb_check_dir1_lambda->setEnabled(false);
		pb_order_dir1_cell->setEnabled(false);
		pb_order_dir1_all->setEnabled(false);
	}
	if(scan_count1[selected_cell1] == 0)
	{
		lbl_message1->setText("No files");
	} 
	str = "*.";
	str.append(extension1);
	str1.sprintf("%d", selected_cell1+1);
	str.append(str1);
	data_dir1.setNameFilter(str);
	data_dir1.setSorting(QDir::Name);
	filename = data_dir1[0];
	filename.prepend("/");
	filename.prepend(data_dir1.absPath());
	QFile scanfile(filename);
	if (scanfile.open(IO_ReadOnly))
	{
		QTextStream ts(&scanfile);
		if (!ts.eof())
		{
			lbl_dir1_info2->setText(ts.readLine());
		}
		scanfile.close();
	}
	else
	{
		lbl_dir1_info2->setText(tr("Could not read the first file of this cell"));
	}
	if (scan_count1[selected_cell1] > 0)
	{
		lbl_dir1_first_scan2->setText(data_dir1[0]);
		lbl_dir1_last_scan2->setText(data_dir1[scan_count1[selected_cell1]-1]);
	}
	else
	{
		lbl_dir1_info2->setText("<no contents>");
		lbl_dir1_first_scan2->setText(tr("<no Files>"));
		lbl_dir1_last_scan2->setText(tr("<no Files>"));
	}
}

void US_Merge::show_dir2_cell(int cell)
{
	QString str, str1, filename;
	lbl_dir2_lambda2->setText(tr("<not checked>"));
	selected_cell2 = cell;
	str.sprintf(tr("%d Scans in Cell %d"), scan_count2[selected_cell2], selected_cell2+1);
	lbl_dir2_scans2->setText(str);
	if (scan_count1[selected_cell1] > 0 && scan_count2[selected_cell2] > 0)
	{
		pb_merge->setEnabled(true);
	}
	else
	{
		pb_merge->setEnabled(false);
	}
	if (out_of_sequence2[cell])
	{
		lbl_message2->setText(str.sprintf(tr("Cell %d contains scan files\nwhich are out of sequence."), cell + 1));
		pb_check_dir2_lambda->setEnabled(true);
		pb_order_dir2_cell->setEnabled(true);
		pb_order_dir2_all->setEnabled(true);
	}
	else
	{
		lbl_message2->setText(str.sprintf(tr("All scans in cell %d\nare in sequence"), cell + 1));
		pb_check_dir2_lambda->setEnabled(false);
		pb_order_dir2_cell->setEnabled(false);
		pb_order_dir2_all->setEnabled(false);
	}
	if(scan_count2[selected_cell2] == 0)
	{
		lbl_message2->setText("No files");
	} 
	str = "*.";
	str.append(extension2);
	str1.sprintf("%d", selected_cell2+1);
	str.append(str1);
	data_dir2.setNameFilter(str);
	data_dir2.setSorting(QDir::Name);
	filename = data_dir2[0];
	filename.prepend("/");
	filename.prepend(data_dir2.absPath());
	QFile scanfile(filename);
	if (scanfile.open(IO_ReadOnly))
	{
		QTextStream ts(&scanfile);
		if (!ts.eof())
		{
			lbl_dir2_info2->setText(ts.readLine());
		}
		scanfile.close();
	}
	else
	{
		lbl_dir2_info2->setText(tr("Could not read the first file of this cell"));
	}
	if (scan_count2[selected_cell2] > 0)
	{
		lbl_dir2_first_scan2->setText(data_dir2[0]);
		lbl_dir2_last_scan2->setText(data_dir2[scan_count2[selected_cell2]-1]);
	}
	else
	{
		lbl_dir2_info2->setText("<no contents>");
		lbl_dir2_first_scan2->setText(tr("<no Files>"));
		lbl_dir2_last_scan2->setText(tr("<no Files>"));
	}
}

void US_Merge::merge()
{
	QString source, s1, s2, target, temp, command;
	int i, start;
	pgb_progress->setTotalSteps(scan_count2[selected_cell2]);
	pgb_progress->reset();
	temp.sprintf(tr("Merging Cell %d from:\n" + directory2 + "\n with Cell %d from:\n" 
	+ directory1), selected_cell2+1, selected_cell1+1);
	lbl_message1->setText(temp);
	s1 = "*.";
	s1.append(extension2);
	s2.sprintf("%d", selected_cell2+1);
	s1.append(s2);
	data_dir2.setNameFilter(s1);
	data_dir2.setSorting(QDir::Name);
	s1 = "*.";
	s1.append(extension1);
	s2.sprintf("%d", selected_cell1+1);
	s1.append(s2);
	data_dir1.setNameFilter(s1);
	data_dir1.setSorting(QDir::Name);
	target = data_dir1[scan_count1[selected_cell1] - 1];
	temp = target.left(5);
	start = temp.toInt();
	for (i=0; i<scan_count2[selected_cell2]; i++)
	{
		start++;
		source = data_dir2[i];
#ifdef UNIX
		source.prepend(data_dir2.absPath() + "/");
#endif
#ifdef WIN32
		source.prepend(data_dir2.absPath() + "\\");
#endif
		if (start < 10)
		{
			target.sprintf("0000%d.", start);
		}
		else if ( start > 9 && start < 100)
		{
			target.sprintf("000%d.", start);
		}
		else if (start > 99 && start < 1000)
		{
			target.sprintf("00%d.", start);
		}
		else if (start > 999 && start < 10000)
		{
			target.sprintf("0%d.", start);
		}
		else if (start > 9999)
		{
			target.sprintf("%d.", start);
		}
		target.append(extension1);
		target.append(s2);
		target.prepend("/");
		target.prepend(data_dir1.absPath());
		if (move_data)
		{
			Move(source, target);
		}
		else
		{
			copy(source, target);
		}
		pgb_progress->setProgress(i+1);
		qApp->processEvents();
	}
	lbl_message1->setText(tr("Merging completed"));
}

void US_Merge::order_dir1_cell()
{
	QString source, s1, s2, target, temp, command;
	int i;
	pgb_progress->setTotalSteps(scan_count1[selected_cell1]);
	pgb_progress->reset();
	temp.sprintf(tr("Ordering Scanfiles for:\nCell %d"), selected_cell1+1);
	lbl_message1->setText(temp);
	for (i=1; i<scan_count1[selected_cell1]+1; i++)
	{
		s1 = "*.";
		s1.append(extension1);
		s2.sprintf("%d", selected_cell1+1);
		s1.append(s2);
		data_dir1.setNameFilter(s1);
		data_dir1.setSorting(QDir::Name);
		source = data_dir1[i-1];
		source.prepend("/");
		source.prepend(data_dir1.absPath());
		if (i < 10)
		{
			target.sprintf("0000%d.", i);
		}
		else if ( i > 9 && i < 100)
		{
			target.sprintf("000%d.", i);
		}
		else if (i > 99 && i < 1000)
		{
			target.sprintf("00%d.", i);
		}
		else if (i > 999 && i < 10000)
		{
			target.sprintf("0%d.", i);
		}
		else if (i > 9999)
		{
			target.sprintf("%d.", i);
		}
		target.append(extension1);
		target.append(s2);
		target.prepend("/");
		target.prepend(data_dir1.absPath());
		if (source != target)
		{
			Move(source, target);	
		}
		pgb_progress->setProgress(i);
		qApp->processEvents();
		out_of_sequence1[selected_cell1] = false;
	}
	show_dir1_cell(selected_cell1);
	lbl_message1->setText(tr("Ordering completed"));
}

void US_Merge::order_dir1_all()
{
	QString source, s1, s2, target, temp, command;
	int i, j, all_scans=0, count;

	for (i=0; i<8; i++)
	{
		all_scans += scan_count1[i];
	}
	pgb_progress->setTotalSteps(all_scans);
	pgb_progress->reset();
	count = 0;
	for (j=0; j<8; j++)
	{
		temp.sprintf(tr("Ordering Scanfiles for:\nCell %d"), j+1);
		lbl_message1->setText(temp);
		for (i=1; i<scan_count1[j]+1; i++)
		{
			count++;
			s1 = "*.";
			s1.append(extension1);
			s2.sprintf("%d", j+1);
			s1.append(s2);
			data_dir1.setNameFilter(s1);
			data_dir1.setSorting(QDir::Name);
			source = data_dir1[i-1];
			source.prepend("/");
			source.prepend(data_dir1.absPath());
			if (i < 10)
			{
				target.sprintf("0000%d.", i);
			}
			else if ( i > 9 && i < 100)
			{
				target.sprintf("000%d.", i);
			}
			else if (i > 99 && i < 1000)
			{
				target.sprintf("00%d.", i);
			}
			else if (i > 999 && i < 10000)
			{
				target.sprintf("0%d.", i);
			}
			else if (i > 9999)
			{
				target.sprintf("%d.", i);
			}
			target.append(extension1);
			target.append(s2);
			target.prepend("/");
			target.prepend(data_dir1.absPath());
			if (source != target)
			{
				Move(source, target);
			}
			pgb_progress->setProgress(count);
			qApp->processEvents();
		}
		out_of_sequence1[j] = false;
	}
	show_dir1_cell(selected_cell1);
	lbl_message1->setText(tr("Ordering completed"));
}

void US_Merge::order_dir2_cell()
{
	QString source, s1, s2, target, temp, command;
	int i;
	pgb_progress->setTotalSteps(scan_count2[selected_cell2]);
	pgb_progress->reset();
	temp.sprintf(tr("Ordering Scanfiles for:\nCell %d"), selected_cell2+1);
	lbl_message2->setText(temp);
	for (i=1; i<scan_count2[selected_cell2]+1; i++)
	{
		s1 = "*.";
		s1.append(extension2);
		s2.sprintf("%d", selected_cell2+1);
		s1.append(s2);
		data_dir2.setNameFilter(s1);
		data_dir2.setSorting(QDir::Name);
		source = data_dir2[i-1];
		source.prepend("/");
		source.prepend(data_dir2.absPath());
		if (i < 10)
		{
			target.sprintf("0000%d.", i);
		}
		else if ( i > 9 && i < 100)
		{
			target.sprintf("000%d.", i);
		}
		else if (i > 99 && i < 1000)
		{
			target.sprintf("00%d.", i);
		}
		else if (i > 999 && i < 10000)
		{
			target.sprintf("0%d.", i);
		}
		else if (i > 9999)
		{
			target.sprintf("%d.", i);
		}
		target.append(extension2);
		target.append(s2);
		target.prepend("/");
		target.prepend(data_dir2.absPath());
		if (source != target)
		{
			Move(source, target);	
		}
		pgb_progress->setProgress(i);
		qApp->processEvents();
		out_of_sequence2[selected_cell2] = false;
	}
	show_dir2_cell(selected_cell2);
	lbl_message2->setText(tr("Ordering completed"));
}

void US_Merge::order_dir2_all()
{
	QString source, s1, s2, target, temp, command;
	int i, j, all_scans=0, count;

	for (i=0; i<8; i++)
	{
		all_scans += scan_count2[i];
	}
	pgb_progress->setTotalSteps(all_scans);
	pgb_progress->reset();
	count = 0;
	for (j=0; j<8; j++)
	{
		temp.sprintf(tr("Ordering Scanfiles for:\nCell %d"), j+1);
		lbl_message2->setText(temp);
		for (i=1; i<scan_count2[j]+1; i++)
		{
			count++;
			s1 = "*.";
			s1.append(extension2);
			s2.sprintf("%d", j+1);
			s1.append(s2);
			data_dir2.setNameFilter(s1);
			data_dir2.setSorting(QDir::Name);
			source = data_dir2[i-1];
			source.prepend("/");
			source.prepend(data_dir2.absPath());
			if (i < 10)
			{
				target.sprintf("0000%d.", i);
			}
			else if ( i > 9 && i < 100)
			{
				target.sprintf("000%d.", i);
			}
			else if (i > 99 && i < 1000)
			{
				target.sprintf("00%d.", i);
			}
			else if (i > 999 && i < 10000)
			{
				target.sprintf("0%d.", i);
			}
			else if (i > 9999)
			{
				target.sprintf("%d.", i);
			}
			target.append(extension2);
			target.append(s2);
			target.prepend("/");
			target.prepend(data_dir2.absPath());
			if (source != target)
			{
				Move(source, target);
			}
			pgb_progress->setProgress(count);
			qApp->processEvents();
		}
		out_of_sequence2[j] = false;
	}
	show_dir2_cell(selected_cell2);
	lbl_message2->setText(tr("Ordering completed"));
}

void US_Merge::check_dir1_lambda()
{
	QString str, str1, filename, label, trashcan;
	int lambda[3], temp, i, j, count=0;;
	bool equal;
	QFile scanfile;

	str = "*.";
	str.append(extension1);
	str1.sprintf("%d", selected_cell1+1);
	str.append(str1);
	data_dir1.setNameFilter(str);
	data_dir1.setSorting(QDir::Name);
	pgb_progress->setTotalSteps(scan_count1[selected_cell1]);
	pgb_progress->reset();

	for (i=0; i<scan_count1[selected_cell1]; i++)
	{
		filename = data_dir1[i];
		filename.prepend("/");
		filename.prepend(data_dir1.absPath());
		scanfile.setName(filename);
		if (scanfile.open(IO_ReadOnly))
		{
			QTextStream ts(&scanfile);
			if (!ts.eof())
			{
				ts.readLine();
				ts >> trashcan;	// radial or wavelength scan mode
				ts >> trashcan;	// cell number
				ts >> trashcan;	// temperature
				ts >> trashcan;	//	rotor speed
				ts >> trashcan;	// time in secs
				ts >> trashcan;	// omega-squared-t
				ts >> temp;			// wavelength
			}
			if (i == 0)
			{
				lambda[0] = temp;
				count++;
				label.sprintf("%d nm", temp);
			}
			else if (count < 3)
			{
				equal = false;
				for (j=0; j<=count; j++)
				{
			 		if (temp == lambda[j])
					{
						equal = true;
					}
				}
				if (!equal)
				{
					str.sprintf(", %d nm", temp);
					label.append(str);
					lambda[count] = temp;
					count++;
				}
			}
			scanfile.close();
		}
		pgb_progress->setProgress(i+1);
		qApp->processEvents();		// check to see if there is any other pressing business that can't wait
	}
	if (count > 1)
	{
		lbl_message1->setText(tr("This cell contains data which was\nmeasured at multiple wavelengths!\nIt is not safe to re-order this cell!"));
	}
	else
	{
		lbl_message1->setText(tr("This cell was measured at a single\nwavelength.\nIt is safe to re-order this cell."));
	}
	lbl_dir1_lambda2->setText(label);
}

void US_Merge::check_dir2_lambda()
{
	QString str, str1, filename, label, trashcan;
	int lambda[3], temp, i, j, count=0;;
	bool equal;
	QFile scanfile;

	str = "*.";
	str.append(extension2);
	str1.sprintf("%d", selected_cell2+1);
	str.append(str1);
	data_dir2.setNameFilter(str);
	data_dir2.setSorting(QDir::Name);
	pgb_progress->setTotalSteps(scan_count2[selected_cell2]);
	pgb_progress->reset();

	for (i=0; i<scan_count2[selected_cell2]; i++)
	{
		filename = data_dir2[i];
		filename.prepend("/");
		filename.prepend(data_dir2.absPath());
		scanfile.setName(filename);
		if (scanfile.open(IO_ReadOnly))
		{
			QTextStream ts(&scanfile);
			if (!ts.eof())
			{
				ts.readLine();
				ts >> trashcan;	// radial or wavelength scan mode
				ts >> trashcan;	// cell number
				ts >> trashcan;	// temperature
				ts >> trashcan;	//	rotor speed
				ts >> trashcan;	// time in secs
				ts >> trashcan;	// omega-squared-t
				ts >> temp;			// wavelength
			}
			if (i == 0)
			{
				lambda[0] = temp;
				count++;
				label.sprintf("%d nm", temp);
			}
			else if (count < 3)
			{
				equal = false;
				for (j=0; j<=count; j++)
				{
			 		if (temp == lambda[j])
					{
						equal = true;
					}
				}
				if (!equal)
				{
					str.sprintf(", %d nm", temp);
					label.append(str);
					lambda[count] = temp;
					count++;
				}
			}
			scanfile.close();
		}
		pgb_progress->setProgress(i+1);
		qApp->processEvents();		// check to see if there is any other pressing business that can't wait
	}
	if (count > 1)
	{
		lbl_message2->setText(tr("This cell contains data which was\nmeasured at multiple wavelengths!\nIt is not safe to re-order this cell!"));
	}
	else
	{
		lbl_message2->setText(tr("This cell was measured at a single\nwavelength.\nIt is safe to re-order this cell."));
	}
	lbl_dir2_lambda2->setText(label);
}

void US_Merge::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/merge.html");
}

void US_Merge::quit()
{
	close();
}

