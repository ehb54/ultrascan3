#include "../include/us_pseudoabs.h"

US_PseudoAbs::US_PseudoAbs(QWidget *parent, const char* name) : QFrame(parent, name)
{

	USglobal = new US_Config();
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	setCaption(tr("Intensity Data Converter"));
	unsigned int i, minHeight1 = 26, minHeight2 = 30;
	QString item;
	selected_channel = 0;
	review_channel1 = false;
	review_channel2 = false;
	average_adjusted = false;
	ready_for_averaging = false;

	lbl_header = new QLabel(tr("Convert Intensity Data\nto Pseudo-Absorbance Data"), this);
	lbl_header->setAlignment(AlignCenter|AlignVCenter);
	lbl_header->setFrameStyle(QFrame::WinPanel|Raised);
	lbl_header->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_header->setMinimumHeight(minHeight2);
	lbl_header->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	pb_select_dir = new QPushButton(tr("Select Directory"), this);
	Q_CHECK_PTR(pb_select_dir);
	pb_select_dir->setAutoDefault(false);
	pb_select_dir->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_select_dir->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_select_dir->setMinimumHeight(minHeight1);
	connect(pb_select_dir, SIGNAL(clicked()), SLOT(select_dir()));

	lbl_directory = new QLabel(tr("<not selected>"),this);
	lbl_directory->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_directory->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_directory->setMinimumHeight(minHeight1);
	lbl_directory->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	pb_help = new QPushButton(tr("Help"), this);
	Q_CHECK_PTR(pb_help);
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_help->setMinimumHeight(minHeight1);
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	lbl_cells1 = new QLabel(tr("Total Cells:"),this);
	lbl_cells1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_cells1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_cells1->setMinimumHeight(minHeight1);
	lbl_cells1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_cells2 = new QLabel(tr("<not selected>"),this);
	lbl_cells2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_cells2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_cells2->setMinimumHeight(minHeight1);
	lbl_cells2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	lbl_line1 = new QLabel("", this);
	lbl_line1->setFrameStyle(QFrame::WinPanel|Raised);
	lbl_line1->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_line1->setMinimumHeight(minHeight1);

	lbl_cell_details = new QLabel(tr("Please Select a Cell:"),this);
	lbl_cell_details->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_cell_details->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_cell_details->setMinimumHeight(minHeight1);
	lbl_cell_details->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_progress = new QLabel(tr("Status:"),this);
	lbl_progress->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_progress->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_progress->setMinimumHeight(minHeight1);
	lbl_progress->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	pgb_progress = new QProgressBar(this, "iteration progress");
	pgb_progress->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	pgb_progress->setMinimumHeight(minHeight2);

	selected_cell = 0;
	lb_cells = new QListBox(this, "Cell");
	lb_cells->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	for (i=0; i<8; i++)
	{
		item.sprintf(tr("Cell %d is empty"), i+1);
		lb_cells->insertItem(item);
	}
	lb_cells->setSelected(0, TRUE);
	lb_cells->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	connect(lb_cells, SIGNAL(highlighted(int)), SLOT(show_cell(int)));

	lbl_message = new QLabel(tr("Please select a Directory by\nclicking on \"Select Directory\" now."),this);
	lbl_message->setAlignment(AlignCenter|AlignVCenter);
	lbl_message->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_message->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_message->setMinimumHeight(minHeight1);
	lbl_message->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	lbl_info1 = new QLabel(tr("Cell Contents:"),this);
	lbl_info1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_info1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_info1->setMinimumHeight(minHeight1);
	lbl_info1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_info2 = new QLabel(tr("<not selected>"),this);
	lbl_info2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_info2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_info2->setMinimumHeight(minHeight1);
	lbl_info2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	lbl_ch1txt = new QLabel(tr("Channel 1 Text:"),this);
	lbl_ch1txt->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_ch1txt->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_ch1txt->setMinimumHeight(minHeight1);
	lbl_ch1txt->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_ch2txt = new QLabel(tr("Channel 2 Text:"),this);
	lbl_ch2txt->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_ch2txt->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_ch2txt->setMinimumHeight(minHeight1);
	lbl_ch2txt->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	le_ch1txt = new QLineEdit( this, "ch1txt" );
	le_ch1txt->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_ch1txt->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
	connect(le_ch1txt, SIGNAL(textChanged(const QString &)), SLOT(update_ch1txt(const QString &)));

	le_ch2txt = new QLineEdit( this, "ch2txt" );
	le_ch2txt->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_ch2txt->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
	connect(le_ch2txt, SIGNAL(textChanged(const QString &)), SLOT(update_ch2txt(const QString &)));

	lbl_line2 = new QLabel("", this);
	lbl_line2->setFrameStyle(QFrame::WinPanel|Raised);
	lbl_line2->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_line2->setMinimumHeight(minHeight1);

	pb_show1 = new QPushButton(tr("Show Channel 1"), this);
	Q_CHECK_PTR(pb_show1);
	pb_show1->setAutoDefault(false);
	pb_show1->setEnabled(false);
	pb_show1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_show1->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_show1->setMinimumHeight(minHeight1);
	connect(pb_show1, SIGNAL(clicked()), SLOT(show1()));

	pb_markref = new QPushButton(tr("Mark this Channel as Reference"), this);
	Q_CHECK_PTR(pb_markref);
	pb_markref->setAutoDefault(false);
	pb_markref->setEnabled(false);
	pb_markref->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_markref->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_markref->setMinimumHeight(minHeight1);
	connect(pb_markref, SIGNAL(clicked()), SLOT(markref()));

	pb_show2 = new QPushButton(tr("Show Channel 2"), this);
	Q_CHECK_PTR(pb_show2);
	pb_show2->setEnabled(false);
	pb_show2->setAutoDefault(false);
	pb_show2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_show2->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_show2->setMinimumHeight(minHeight1);
	connect(pb_show2, SIGNAL(clicked()), SLOT(show2()));

	pb_convert_cell = new QPushButton(tr("Convert This Cell"), this);
	Q_CHECK_PTR(pb_convert_cell);
	pb_convert_cell->setAutoDefault(false);
	pb_convert_cell->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_convert_cell->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_convert_cell->setEnabled(false);
	pb_convert_cell->setMinimumHeight(minHeight1);
	connect(pb_convert_cell, SIGNAL(clicked()), SLOT(convert_cell()));

	lbl_scans1 = new QLabel(tr("Total Scans:"),this);
	lbl_scans1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_scans1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_scans1->setMinimumHeight(minHeight1);
	lbl_scans1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_scans2 = new QLabel(tr("<not selected>"),this);
	lbl_scans2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_scans2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_scans2->setMinimumHeight(minHeight1);
	lbl_scans2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	lbl_first_scan1 = new QLabel(tr("First Scanfile:"),this);
	lbl_first_scan1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_first_scan1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_first_scan1->setMinimumHeight(minHeight1);
	lbl_first_scan1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_first_scan2 = new QLabel(tr("<not selected>"),this);
	lbl_first_scan2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_first_scan2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_first_scan2->setMinimumHeight(minHeight1);
	lbl_first_scan2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	pb_quit = new QPushButton(tr("Quit"), this);
	Q_CHECK_PTR(pb_quit);
	pb_quit->setAutoDefault(false);
	pb_quit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_quit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_quit->setMinimumHeight(minHeight1);
	connect(pb_quit, SIGNAL(clicked()), SLOT(quit()));

	lbl_last_scan1 = new QLabel(tr("Last Scanfile:"),this);
	lbl_last_scan1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_last_scan1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_last_scan1->setMinimumHeight(minHeight1);
	lbl_last_scan1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_last_scan2 = new QLabel(tr("<not selected>"),this);
	lbl_last_scan2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_last_scan2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_last_scan2->setMinimumHeight(minHeight1);
	lbl_last_scan2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	int_plot = new QwtPlot(this);
	Q_CHECK_PTR(int_plot);
	int_plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
	int_plot->enableGridXMin();
	int_plot->enableGridYMin();
	int_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
	int_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
	int_plot->setCanvasBackground(USglobal->global_colors.plot);		//new version
	int_plot->enableOutline(true);
	int_plot->setAxisTitle(QwtPlot::xBottom, "Radius (cm)");
	int_plot->setAxisTitle(QwtPlot::yLeft, "Intensity");
	int_plot->setTitle(tr("Intensity data"));
	int_plot->setOutlinePen(white);
	int_plot->setOutlineStyle(Qwt::Rect);
	int_plot->setMinimumSize(580,260);
	int_plot->resize(580, 260);
	int_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
	int_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	int_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	int_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	int_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	int_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	int_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	int_plot->setMargin(USglobal->config_list.margin);
	connect(int_plot, SIGNAL(plotMousePressed(const QMouseEvent &)),
			  SLOT(plotMousePressed( const QMouseEvent&)));
	connect(int_plot, SIGNAL(plotMouseReleased(const QMouseEvent &)),
			  SLOT(plotMouseReleased(const QMouseEvent &)));

	abs_plot = new QwtPlot(this);
	Q_CHECK_PTR(abs_plot);
	abs_plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
	abs_plot->enableGridXMin();
	abs_plot->enableGridYMin();
	abs_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
	abs_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
	abs_plot->setCanvasBackground(USglobal->global_colors.plot);		//new version
	abs_plot->enableOutline(true);
	abs_plot->setAxisTitle(QwtPlot::xBottom, "Radius(cm)");
	abs_plot->setAxisTitle(QwtPlot::yLeft, "Absorbance");
	abs_plot->setTitle(tr("Pseudo Absorbance Data"));
	abs_plot->setOutlinePen(white);
	abs_plot->setOutlineStyle(Qwt::Cross);
	abs_plot->setMinimumSize(580,260);
	abs_plot->resize(580,260);
	abs_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
	abs_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	abs_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	abs_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	abs_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	abs_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	abs_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	abs_plot->setMargin(USglobal->config_list.margin);

	global_Xpos += 30;
	global_Ypos += 30;
	setup_GUI();
	this->setMaximumWidth(1920);

	move(global_Xpos, global_Ypos);
}

US_PseudoAbs::~US_PseudoAbs()
{
}

void US_PseudoAbs::setup_GUI()
{
	int j=0, spacing=2;

	QGridLayout * panel = new QGridLayout(this, 2, 2, spacing);
	QGridLayout * background = new QGridLayout(14, 3, 2);
	background->addMultiCellWidget(lbl_header,j,j,0,2);
	j++;
	background->addWidget(pb_select_dir,j,0);
	background->addMultiCellWidget(lbl_directory,j,j,1,2);
	j++;
	background->addWidget(pb_help,j,0);
	background->addWidget(lbl_cells1,j,1);
	background->addWidget(lbl_cells2,j,2);
	j++;
	background->addMultiCellWidget(lbl_line1,j,j,0,2);
	j++;
	background->addWidget(lbl_cell_details,j,0);
	background->addWidget(lbl_progress,j,1);
	background->addWidget(pgb_progress,j,2);
	j++;
	background->addMultiCellWidget(lb_cells,j,j+3,0,0);
	background->addMultiCellWidget(lbl_message,j,j+3,1,2);
	j=j+4;
	background->addWidget(lbl_info1,j,0);
	background->addMultiCellWidget(lbl_info2,j,j,1,2);
	j++;
	background->addWidget(lbl_ch1txt, j, 0);
	background->addMultiCellWidget(le_ch1txt, j, j, 1, 2);
	j++;
	background->addWidget(lbl_ch2txt, j, 0);
	background->addMultiCellWidget(le_ch2txt, j, j, 1, 2);
	j++;
	background->addWidget(pb_show1,j,0);
	background->addMultiCellWidget(pb_markref, j, j, 1, 2);
	background->addWidget(lbl_line2, j, 2);
	j++;
	background->addWidget(pb_show2, j, 0);
	background->addWidget(lbl_first_scan1,j,1);
	background->addWidget(lbl_first_scan2,j,2);
	j++;
	background->addWidget(pb_convert_cell,j,0);
	background->addWidget(lbl_scans1,j,1);
	background->addWidget(lbl_scans2,j,2);
	j++;
	background->addWidget(pb_quit,j,0);
	background->addWidget(lbl_last_scan1,j,1);
	background->addWidget(lbl_last_scan2,j,2);

	QSize s = background->sizeHint();
	panel->addMultiCellLayout(background, 0, 1, 0, 0);
	panel->addWidget(int_plot, 0, 1);
	panel->addWidget(abs_plot, 1, 1);
	panel->setRowSpacing(0, 260);
	panel->setRowSpacing(1, 260);
	panel->setColStretch(0, 1);
	panel->setColStretch(1, 5);
	this->setGeometry(global_Xpos, global_Ypos, s.width()+580, 520+3*spacing);
}

void US_PseudoAbs::closeEvent(QCloseEvent *e)
{
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
}

void US_PseudoAbs::select_dir()
{
	int i, j, cell_count, count;
	unsigned int scans_upper = 0, scans_lower = 0;
	QFileDialog *fd;
	QFile f;
	bool out_of_sequence = false;
	QString trashcan, item, message;
	QString directory="";
	reset();
	QString str, str1, str2, str3;
	extension1 = "RI";
	extension2 = "ri";
	for (i=0; i<8; i++)
	{
		scan_count[i] = 0;
	}
	str = "*." + extension1 + "?, *." + extension2 + "?";
	fd = new QFileDialog(USglobal->config_list.data_dir, str, 0, 0, TRUE);
	directory = fd->getExistingDirectory(USglobal->config_list.data_dir, 0, 0, "AUC Data", false, true);
	lbl_directory->setText(directory);
	message = tr("The following cells of this run\n contain out-of-sequence scans:\nCell ");
	wavelength.clear();
	if (!directory.isEmpty())
	{
		data_dir.setPath(directory);
		extension = extension1; // first count the files with uppercase extensions
		for (i=0; i<8; i++)		// find the number of scan files for each cell
		{
			str.sprintf("*." + extension + "%d", i+1);
			data_dir.setNameFilter(str);
			scans_upper += data_dir.count();
		}
		extension = extension2; // next, count the files with lowercase extensions
		for (i=0; i<8; i++)		// find the number of scan files for each cell
		{
			str.sprintf("*." + extension + "%d", i+1);
			data_dir.setNameFilter(str);
			scans_lower += data_dir.count();
		}
		if (scans_upper >= scans_lower) // this system does not allow for mixed case scenarios!
		{
			extension = extension1;
		}
		else
		{
			extension = extension2;
		}
		for (i=0; i<8; i++)
		{
			str = "*." + extension;
			str1.sprintf("%d", i+1);
			str.append(str1);
			data_dir.setNameFilter(str);
			scan_count[i] = data_dir.count();
		}
		cell_count = 0;
		for (i=0; i<8; i++)
		{
			if(scan_count[i] > 0)
			{
				item.sprintf(tr("Cell %d contains data"), i+1);
				lb_cells->changeItem(item, i);
				cell_count++;
			}
			str.sprintf(tr("%d Cells with Data"), cell_count);
			lbl_cells2->setText(str);
		}
		i=0;
		while (scan_count[i] < 1)
		{
			i++;
		}
		if (i > 7)
		{
			directory = "";
			lbl_message->setText(tr("Sorry - this directory doesn't contain\nany scanfiles.\nPlease try again..."));
			lbl_first_scan2->setText(tr("<not selected>"));
			lbl_scans2->setText(tr("<not selected>"));
			lbl_cells2->setText(tr("<not selected>"));
			lbl_directory->setText(tr("<not selected>"));
			lbl_last_scan2->setText(tr("<not selected>"));
			for (i=0; i<8; i++)
			{
				item.sprintf(tr("Cell %d is empty"), i+1);
				lb_cells->changeItem(item, i);
			}
			return;
		}
		else
		{
			lb_cells->setCurrentItem(i);
			for (j=0; j<8; j++)
			{
				if (scan_count[j] > 0)
				{
					str = "*.";
					str.append(extension);
					str1.sprintf("%d", j+1);
					str.append(str1);
					data_dir.setNameFilter(str);
					data_dir.setSorting(QDir::Name);
					str2 = data_dir[scan_count[j]-1];
					QString str3;
					str3 = str2.left(5);
					str2 = str3;
					count = str2.toInt();
					if (count != scan_count[j])
					{
						out_of_sequence = true;
						str3.sprintf("%d, ", j+1);
						message.append(str3);
					}
				}
			}
			lbl_message->setText(tr("Loading all cells..."));
			pgb_progress->reset();
			qApp->processEvents();
			int total_count=0;
			for (j=0; j<8; j++)
			{
				total_count += scan_count[j];
			}
			pgb_progress->setTotalSteps(total_count);
			QStringList sl;
			QDir newdir;
			newdir.mkdir(data_dir.path() + "/channel-1", true);
			newdir.mkdir(data_dir.path() + "/channel-2", true);
			QFile scan_file;
			struct intensity_cell temp_cell;
			struct intensity_scan temp_scan;
			icell.clear();
			icell.resize(8);
			float fval;
			unsigned int count=0;
			for (j=0; j<8; j++)
			{
				if (scan_count[j] > 0)
				{
					temp_cell.scans.clear();
					str = "*." + extension + str1.sprintf("%d", j+1);
					sl = data_dir.entryList(str, QDir::Files, QDir::Name);
					temp_cell.cell_number = j+1;
					temp_cell.path = data_dir.absPath();
					for (QStringList::Iterator it = sl.begin(); it != sl.end(); it++)
					{
						temp_scan.radius.clear();
						temp_scan.intensity[0].clear();
						temp_scan.intensity[1].clear();
						temp_scan.pseudoabs[0].clear();
						temp_scan.pseudoabs[1].clear();
						scan_file.setName(data_dir.absFilePath(*it));
						temp_scan.filename = *it;
						if(scan_file.open(IO_ReadOnly))
						{
							QTextStream ts(&scan_file);
							temp_cell.header1 = ts.readLine();
							temp_scan.header2 = ts.readLine();
       					wavelength.push_back(temp_scan.header2.mid(33,3));
							while (true)
							{
								ts >> str;
								if (ts.atEnd()) break;
								temp_scan.radius.push_back(str.toFloat());
								ts >> str;
								if (ts.atEnd()) break;
								fval = str.toFloat();
								if (fval < 1.0)
								{
									fval = 1.0;
								}
								temp_scan.intensity[0].push_back(fval);
								temp_scan.pseudoabs[0].push_back(log10(10000/fval));
								ts >> str;
								if (ts.atEnd()) break;
								fval = str.toFloat();
								if (fval < 1.0)
								{
									fval = 1.0;
								}
								temp_scan.intensity[1].push_back(fval);
								temp_scan.pseudoabs[1].push_back(log10(10000/fval));
							}
							count ++;
							pgb_progress->setProgress(count);
							scan_file.close();
						}
						temp_cell.scans.push_back(temp_scan);
					}
				}
				icell[j] = temp_cell;
			}
			if (out_of_sequence)
			{
				message.truncate(message.length() - 2);
				message += tr("\n\nReorder Cell after converting");
				lbl_message->setText(message);
			}
			else
			{
				lbl_message->setText(tr("All scanfiles of this run are in\n the proper sequence.\nNo reordering is necessary."));
			}
			show_cell(i);
			QString tmp = wavelength[0];
			bool flag = false;
			for (unsigned int k=1; k<wavelength.size(); k++)
			{
     			if (wavelength[k] != tmp)
     			{
     				flag = true;
     				break;
     			}

			}
			if (flag)
			{
				QMessageBox::message("Attention", tr("This run contains one or more scans that\n"
				"were measured at different wavelengths.\n\n"
				"UltraScan expects all velocity runs acquired\n"
				"with the XLA absorbance/intensity optics to\n"
				"be measured at the SAME wavelength.\n\n"
				"If you want to edit these data with UltraScan\n"
				"please edit the intensity scan files first to\n"
				"make sure that all scans are at the same\n"
				"wavelength.\n\nAfter editing, run this utility again."));
			}
		}
	}
	else
	{
		reset();
	}
}

void US_PseudoAbs::reset()
{
	unsigned int i;
	QString item;
	lbl_message->setText(tr("Please select a Directory by\nclicking on \"Select Directory\" now."));
	lbl_first_scan2->setText(tr("<not selected>"));
	lbl_scans2->setText(tr("<not selected>"));
	lbl_cells2->setText(tr("<not selected>"));
	lbl_directory->setText(tr("<not selected>"));
	lbl_last_scan2->setText(tr("<not selected>"));
	for (i=0; i<8; i++)
	{
		item.sprintf(tr("Cell %d is empty"), i+1);
		lb_cells->changeItem(item, i);
	}
	reference_channel = -1;
	pb_markref->setEnabled(false);
	pb_convert_cell->setEnabled(false);
	pb_show1->setEnabled(false);
	pb_show2->setEnabled(false);
	average_adjusted = false;
	review_channel1 = false;
	review_channel2 = false;
	average_adjusted = false;
	ready_for_averaging = false;

}

void US_PseudoAbs::show_cell(int cell)
{
	pb_show1->setEnabled(true);
	pb_show2->setEnabled(true);
	review_channel1 = false;
	review_channel2 = false;
	pb_convert_cell->setEnabled(false);
	QString str, str1, filename;
	selected_cell = cell;
	str.sprintf(tr("%d Scans in Cell %d"), scan_count[selected_cell], selected_cell+1);
	lbl_scans2->setText(str);
	str = "*.";
	str.append(extension);
	str1.sprintf("%d", selected_cell+1);
	str.append(str1);
	data_dir.setNameFilter(str);
	data_dir.setSorting(QDir::Name);
	filename = data_dir[0];
	filename.prepend("/");
	filename.prepend(data_dir.absPath());
	QFile scanfile(filename);
	if (scanfile.open(IO_ReadOnly))
	{
		QTextStream ts(&scanfile);
		if (!ts.eof())
		{
			QString tmp_str = ts.readLine();
			lbl_info2->setText(tmp_str);
			int channel = selected_cell * 2;
			if (channel_text[channel] == "")
			{
				channel_text[channel] = tmp_str;
			}
			if (channel_text[channel+1] == "")
			{
				channel_text[channel+1] = tmp_str;
			}
			le_ch1txt->setText(channel_text[channel]);
			le_ch2txt->setText(channel_text[channel+1]);
		}
		scanfile.close();
	}
	else
	{
		lbl_info2->setText(tr("Could not read the first file of this cell"));
	}
	if (scan_count[selected_cell] > 0)
	{
		lbl_first_scan2->setText(data_dir[0]);
		lbl_last_scan2->setText(data_dir[scan_count[selected_cell]-1]);
	}
	else
	{
		lbl_first_scan2->setText(tr("no Files"));
		lbl_last_scan2->setText(tr("no Files"));
	}
}

void US_PseudoAbs::convert_cell()
{
	QString str, str2;
	int i;
	unsigned int j;
	pgb_progress->setTotalSteps(scan_count[selected_cell]);
	pgb_progress->reset();
	str.sprintf(tr("Converting Scanfiles for:\nCell %d"), selected_cell+1);
	lbl_message->setText(str);
	QFile f;
	for (i=0; i<scan_count[selected_cell]; i++)
	{
		str = icell[selected_cell].path;
#ifdef WIN32
		str += "\\channel-1\\";
#else
		str += "/channel-1/";
#endif
		str2 = icell[selected_cell].scans[i].filename;
		str2.truncate(6);
		str += str2;
		str2.sprintf("RA%d", selected_cell + 1);
		str += str2;
		f.setName(str);
		if (f.open(IO_WriteOnly))
		{
			QTextStream ts(&f);
			ts << channel_text[selected_cell * 2] << endl;
			ts << icell[selected_cell].scans[i].header2 << endl;
			for (j=0; j<icell[selected_cell].scans[i].radius.size(); j++)
			{
				ts << str.sprintf("%9.4f %12.5e %12.5e", icell[selected_cell].scans[i].radius[j], icell[selected_cell].scans[i].pseudoabs[0][j], 0.0) << endl;
			}
			f.close();
		}
		else
		{
			QMessageBox::warning(this, tr("UltraScan Warning"),
			tr("Please note:\n\nUltraScan could not open\nthe output file " + str + "!"),
			QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		}
		str = icell[selected_cell].path;
#ifdef WIN32
		str += "\\channel-2\\";
#else
		str += "/channel-2/";
#endif
		str2 = icell[selected_cell].scans[i].filename;
		str2.truncate(6);
		str += str2;
		str2.sprintf("RA%d", selected_cell + 1);
		str += str2;
		f.setName(str);
		if (f.open(IO_WriteOnly))
		{
			QTextStream ts(&f);
			ts << channel_text[selected_cell * 2 + 1] << endl;
			ts << icell[selected_cell].scans[i].header2 << endl;
			for (j=0; j<icell[selected_cell].scans[i].radius.size(); j++)
			{
				ts << str.sprintf("%9.4f %12.5e %12.5e", icell[selected_cell].scans[i].radius[j], icell[selected_cell].scans[i].pseudoabs[1][j], 0.0) << endl;
			}
			f.close();
		}
		else
		{
			QMessageBox::warning(this, tr("UltraScan Warning"),
			tr("Please note:\n\nUltraScan could not open\nthe output file " + str + "!"),
			QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		}
		pgb_progress->setProgress(i+1);
		qApp->processEvents();
	}
	show_cell(selected_cell);
	lbl_message->setText(tr("** Conversion complete **"));
	pb_convert_cell->setEnabled(false);
}

void US_PseudoAbs::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/pseudoabsorbance.html");
}

void US_PseudoAbs::show1()
{
	show_channel(0);
	review_channel1 = true;
	if (review_channel2)
	{
		pb_convert_cell->setEnabled(true);
	}
}

void US_PseudoAbs::show2()
{
	show_channel(1);
	review_channel2 = true;
	if (review_channel1)
	{
		pb_convert_cell->setEnabled(true);
	}
}

void US_PseudoAbs::show_channel(int channel)
{
	if(!average_adjusted)
	{
		pb_markref->setEnabled(true); //only if we haven't done it already!
	}
	selected_channel = channel;
	double **radius, **intensity;
	unsigned int *curve;
	QString str;
	intensity = new double * [scan_count[selected_cell]];
	radius = new double * [scan_count[selected_cell]];
	curve = new unsigned int [scan_count[selected_cell]];
	int_plot->clear();
	abs_plot->clear();
	double maxval = 0.0;
	for (int i=0; i<scan_count[selected_cell]; i++)
	{
		intensity[i] = new double [icell[selected_cell].scans[i].radius.size()];
		radius[i]    = new double [icell[selected_cell].scans[i].radius.size()];
		for (unsigned int j=0; j<icell[selected_cell].scans[i].radius.size(); j++)
		{
			radius[i][j] = icell[selected_cell].scans[i].radius[j];
			intensity[i][j] = icell[selected_cell].scans[i].intensity[selected_channel][j];
			maxval = max(maxval, intensity[i][j]);
		}
		curve[i] = int_plot->insertCurve(tr(str.sprintf("Scan %d", i+1)));
		int_plot->setCurveStyle(curve[i], QwtCurve::Lines);
		int_plot->setCurvePen(curve[i], Qt::yellow);
		int_plot->setCurveData(curve[i], radius[i], intensity[i], icell[selected_cell].scans[i].radius.size());
	}
	int_plot->setAxisTitle(QwtPlot::xBottom, "Radius (cm)");
	int_plot->replot();
	for (int i=0; i<scan_count[selected_cell]; i++)
	{
		for (unsigned int j=0; j<icell[selected_cell].scans[i].radius.size(); j++)
		{
			intensity[i][j] = log10(maxval/intensity[i][j]);
			icell[selected_cell].scans[i].pseudoabs[selected_channel][j] = intensity[i][j];
		}
		curve[i] = abs_plot->insertCurve(tr(str.sprintf("Scan %d", i+1)));
		abs_plot->setCurveStyle(curve[i], QwtCurve::Lines);
		abs_plot->setCurvePen(curve[i], Qt::yellow);
		abs_plot->setCurveData(curve[i], radius[i], intensity[i], icell[selected_cell].scans[i].radius.size());
	}
	abs_plot->replot();
	delete [] curve;
	for (int i=0; i<scan_count[selected_cell]; i++)
	{
		delete [] radius[i];
		delete [] intensity[i];
	}
	delete [] radius;
	delete [] intensity;
}

void US_PseudoAbs::quit()
{
	close();
}

void US_PseudoAbs::plotMousePressed(const QMouseEvent &e)
{
	if (!ready_for_averaging)
	{
		return; // we haven't selected a baseline channel yet
	}
	p1.x = int_plot->invTransform(QwtPlot::xBottom, e.x());
	p1.y = int_plot->invTransform(QwtPlot::yLeft, e.y());
}

void US_PseudoAbs::plotMouseReleased(const QMouseEvent &e)
{
	if (average_adjusted)
	{
		return; // we already adjusted all scans, so don't do it again
	}
	if (!ready_for_averaging)
	{
		return; // we haven't selected a baseline channel yet
	}
	p2.x = int_plot->invTransform(QwtPlot::xBottom, e.x());
	p2.y = int_plot->invTransform(QwtPlot::yLeft, e.y());
	double *scan, *intensity, avg;
	int i;
	unsigned int curve, j, count;
	intensity = new double [scan_count[selected_cell]];
	scan = new double [scan_count[selected_cell]];
	int_plot->clear();
	average.clear();
	for (i=0; i<scan_count[selected_cell]; i++)
	{
		j=0;
		while (icell[selected_cell].scans[i].radius[j] < p1.x && j<icell[selected_cell].scans[i].radius.size())
		{
			j++;
		}
		count = 0;
		avg = 0.0;
		while (icell[selected_cell].scans[i].radius[j] < p2.x && j<icell[selected_cell].scans[i].radius.size())
		{
			avg += icell[selected_cell].scans[i].intensity[selected_channel][j];
			j++;
			count++;
		}
		scan[i] = (double) i;
		avg /= count;
		average.push_back(avg);
		intensity[i] = average[i];
	}
	curve = int_plot->insertCurve(tr("Average Intensity"));
	int_plot->setCurveStyle(curve, QwtCurve::Lines);
	int_plot->setCurvePen(curve, Qt::yellow);
	int_plot->setCurveData(curve, scan, intensity, average.size());
	int_plot->setAxisTitle(QwtPlot::xBottom, "Scan #");
	int_plot->replot();
	for (int i=0; i<scan_count[selected_cell]; i++)
	{
		for (unsigned int j=0; j<icell[selected_cell].scans[i].radius.size(); j++)
		{
			icell[selected_cell].scans[i].pseudoabs[selected_channel][j] =			log10(average[i]/icell[selected_cell].scans[i].intensity[selected_channel][j]);
		}
	}
	average_adjusted = true;
	pb_convert_cell->setEnabled(true);
	pb_show1->setEnabled(true);
	pb_show2->setEnabled(true);
	delete [] scan;
	delete [] intensity;
}

void US_PseudoAbs::markref()
{
	reference_channel = selected_channel;
	lbl_message->setText(tr("Please drag mouse\nover baseline scans\nto average intensities\nin the intensity plot"));
	pb_convert_cell->setEnabled(false);
	pb_show1->setEnabled(false);
	pb_show2->setEnabled(false);
	ready_for_averaging = true;
	pb_markref->setEnabled(false);
}

void US_PseudoAbs::update_ch1txt(const QString &str)
{
	int channel = selected_cell * 2;
	channel_text[channel] = str;
}

void US_PseudoAbs::update_ch2txt(const QString &str)
{
	int channel = selected_cell * 2 + 1;
	channel_text[channel] = str;
}

