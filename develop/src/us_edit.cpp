#include "../include/us_edit.h"



EditData_Win::EditData_Win(QWidget *p , const char *name)
: QFrame(p, name)
{
	edited_channel = "";
	int spacing = 2, border = 4, column1 = 160, column2 = 200, buttonh = 26;
	int dialog_height = 8 * buttonh + 7 * spacing + 2 * border;
	int span = 2 * border + 3 * spacing + 3 * column1 + column2;
	setMinimumSize(span, (int) (2.2 * dialog_height) + 3 * border);
	setGeometry(0, 0, span, (int) (2.2 * dialog_height) + 3 * border);
	database = false;
	USglobal = new US_Config();
	step = 0;
	cell = 0;
	current_channel = 0;
	curve = new unsigned int [1];
	int i, j, k;
// initialize all static run_inf struct members:
	run_inf.rotor = 0;
	for (i=0; i<8; i++)
	{
		run_inf.centerpiece[i] = -2;
		run_inf.cell_id[i] = "";
		run_inf.wavelength_count[i] = 0;
		run_inf.meniscus[i] = 0.0;
		for (j=0; j<4; j++)
		{
			run_inf.buffer_serialnumber[i][j] = -1;
			for(int k=0; k<3; k++)
			{
				run_inf.peptide_serialnumber[i][j][k] = -1;
				run_inf.DNA_serialnumber[i][j][k] = -1;
			}
		}
		for (j=0; j<3; j++)
		{
			run_inf.wavelength[i][j] = 0;
			run_inf.scans[i][j] = 0;
			run_inf.baseline[i][j] = 0;
			for (k=0; k<4; k++)
			{
				run_inf.range_left[i][j][k] = 0.0;
				run_inf.range_right[i][j][k] = 0.0;
				run_inf.points[i][j][k] = 0;
				run_inf.point_density[i][j][k] = 0.0;
			}
		}
	}
	save_button = false;
	data_dir = new QDir();
	dataset_counter=0;
	run_inf.run_id="";
	edit_scan = 0;

	setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
	QString str;
	cp_list.clear();
	rotor_list.clear();
	if (!readCenterpieceInfo(&cp_list))
	{
		QMessageBox::critical(0, "UltraScan Fatal Error:", "There was a problem opening the\n"
									"centerpiece database file:\n\n"
									+ USglobal->config_list.system_dir + "/etc/centerpiece.dat\n\n"
									"Please install the centerpiece database file\n"
									"before proceeding.", QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		exit(-1);
	}
	if (!readRotorInfo(&rotor_list))
	{
		QMessageBox::critical(0, "UltraScan Fatal Error:", "There was a problem opening the\n"
									"rotor database file:\n\n"
									+ USglobal->config_list.system_dir + "/etc/rotor.dat\n\n"
									"Please install the rotor database file\n"
									"before proceeding.", QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		exit(-1);
	}
	current_centerpiece = 0;
	current_rotor = 0;

	pb_select_dir = new QPushButton(tr("Select Data Directory"), this);
	Q_CHECK_PTR(pb_select_dir);
	pb_select_dir->setAutoDefault(false);
	pb_select_dir->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_select_dir->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));

	lbl_directory = new QLabel(tr(" not selected"),this);
	lbl_directory->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_directory->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
	lbl_directory->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	pb_exsingle = new QPushButton(tr("Exclude Single Scan"), this);
	Q_CHECK_PTR(pb_exsingle);
	pb_exsingle->setAutoDefault(false);
	pb_exsingle->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_exsingle->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));

	cnt_exsingle = new QwtCounter(this);
	cnt_exsingle->setNumButtons(3);
	cnt_exsingle->setRange(0, 0, 1);
	cnt_exsingle->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_exsingle->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cnt_exsingle->setValue(0);

	pb_details = new QPushButton(tr("Show Run Details"), this);
	Q_CHECK_PTR(pb_details);
	pb_details->setAutoDefault(false);
	pb_details->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_details->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));

	lbl_details = new QLabel(tr(" not selected"),this);
	lbl_details->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_details->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
	lbl_details->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	pb_exrange = new QPushButton(tr("Exclude Scan Range"), this);
	Q_CHECK_PTR(pb_exrange);
	pb_exrange->setAutoDefault(false);
	pb_exrange->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_exrange->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));

	cnt_exrange = new QwtCounter(this);
	cnt_exrange->setNumButtons(3);
	cnt_exrange->setRange(0, 0, 1);
	cnt_exrange->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_exrange->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cnt_exrange->setValue(0);

	pb_accept = new QPushButton(tr("Start Editing"), this);
	Q_CHECK_PTR(pb_accept);
	pb_accept->setAutoDefault(false);
	pb_accept->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_accept->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));

	lbl_run_id1 = new QLabel(tr(" Run Id:"),this);
	lbl_run_id1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_run_id1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
	lbl_run_id1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_run_id2 = new QLabel(tr(" not selected"),this);
	lbl_run_id2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_run_id2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
	lbl_run_id2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	pb_editscan = new QPushButton(tr("Edit Single Scan"), this);
	Q_CHECK_PTR(pb_editscan);
	pb_editscan->setAutoDefault(false);
	pb_editscan->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_editscan->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));

	cnt_edit = new QwtCounter(this);
	cnt_edit->setNumButtons(3);
	cnt_edit->setRange(0, 0, 1);
	cnt_edit->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_edit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cnt_edit->setValue(0);

	pb_spikes = new QPushButton(tr("Remove Spikes"), this);
	Q_CHECK_PTR(pb_spikes);
	pb_spikes->setAutoDefault(false);
	pb_spikes->setEnabled(false);
	pb_spikes->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_spikes->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));

	pb_exclude_profile = new QPushButton(tr("Scan Exclusion Profile"), this);
	Q_CHECK_PTR(pb_exclude_profile);
	pb_exclude_profile->setAutoDefault(false);
	pb_exclude_profile->setEnabled(false);
	pb_exclude_profile->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_exclude_profile->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));

	pb_sub_baseline = new QPushButton(tr("Subtract Baseline"), this);
	Q_CHECK_PTR(pb_sub_baseline);
	pb_sub_baseline->setAutoDefault(false);
	pb_sub_baseline->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_sub_baseline->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));

	lbl_current_cell1 = new QLabel(tr(" Active Data:"),this);
	lbl_current_cell1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_current_cell1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
	lbl_current_cell1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_current_cell2 = new QLabel(tr(" not selected"),this);
	lbl_current_cell2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_current_cell2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
	lbl_current_cell2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	lbl_status1 = new QLabel(tr("  Status:"),this);
	lbl_status1->setAlignment(AlignLeft|AlignVCenter);
	lbl_status1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
	lbl_status1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	progress = new QProgressBar(100, this, "Loading Progress");
	progress->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	progress->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

	pb_invert = new QPushButton("Invert Sign", this);
	pb_invert->setAutoDefault(false);
	pb_invert->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_invert->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));

	pb_help = new QPushButton(tr("Help"), this);
	Q_CHECK_PTR(pb_help);
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));

	pb_close = new QPushButton(tr("Close"), this);
	Q_CHECK_PTR(pb_close);
	pb_close->setAutoDefault(false);
	pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));

	lbl_instructions1 = new QLabel(tr(" Step-by-Step\n Instructions:"),this);
	lbl_instructions1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
	lbl_instructions1->setAlignment(AlignLeft|AlignVCenter);
	lbl_instructions1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_instructions2 = new QLabel(tr("Please click on `Select Data Directory` and select a directory\n"
	                               "where your data is located. The data should be in a sub-\n"
	                               "directory of the Beckman Data Acquisition program in the format:\n"
	                               "/.../xlawin/data/mmddyy/hhmmss (month/day/year/hour/min/sec)"),this);
	lbl_instructions2->setAlignment(AlignCenter|AlignVCenter);
	lbl_instructions2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_instructions2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
	lbl_instructions2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	lbl_centerpiece = new QLabel(tr("Centerpiece:"), this);
	lbl_centerpiece->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_centerpiece->setAlignment(AlignCenter|AlignVCenter);
	lbl_centerpiece->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_centerpiece->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cmbb_centerpiece = new QComboBox(false, this, "CenterpieceSelector");
	cmbb_centerpiece->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cmbb_centerpiece->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	for (unsigned int i=0; i<cp_list.size(); i++)
	{
		str.sprintf(cp_list[i].material + tr(", %d channels, %3.2f mm"),  cp_list[i].channels * 2, cp_list[i].pathlength);
		cmbb_centerpiece->insertItem(str);
	}
	cmbb_centerpiece->setCurrentItem(current_centerpiece);
	centerpiece = cp_list[current_centerpiece];
	connect(cmbb_centerpiece, SIGNAL (highlighted(int)), SLOT(update_centerpiece(int)));
	update_centerpiece(current_centerpiece);

	lbl_rotor = new QLabel(tr("Rotor:"), this);
	lbl_rotor->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_rotor->setAlignment(AlignCenter|AlignVCenter);
	lbl_rotor->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_rotor->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cmbb_rotor = new QComboBox(false, this, "RotorSelector");
	cmbb_rotor->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cmbb_rotor->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	for (unsigned int i=0; i<rotor_list.size(); i++)
	{
		cmbb_rotor->insertItem(rotor_list[i].type);
	}
	cmbb_rotor->setCurrentItem(current_rotor);
	rotor = rotor_list[current_rotor];
	connect(cmbb_rotor, SIGNAL (highlighted(int)), SLOT(update_rotor(int)));
	update_rotor(current_rotor);

	edit_plt = new QwtPlot(this);
	edit_plt->enableOutline(true);
	edit_plt->setOutlinePen(white);
	edit_plt->setOutlineStyle(Qwt::VLine);
	edit_plt->setMinimumHeight(350);
	edit_plt->enableGridXMin();
	edit_plt->enableGridYMin();
	edit_plt->setPalette( QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
	edit_plt->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
	edit_plt->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
	edit_plt->setAxisTitle(QwtPlot::xBottom, tr("Radius (in cm)"));
	edit_plt->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
	edit_plt->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	edit_plt->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	edit_plt->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	edit_plt->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	edit_plt->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	edit_plt->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	edit_plt->setMargin(USglobal->config_list.margin);
	//edit_plt->setPlotBackground(USglobal->global_colors.plot);		//old version
	edit_plt->setCanvasBackground(USglobal->global_colors.plot);		//new version
	connect(edit_plt, SIGNAL(plotMouseReleased(const QMouseEvent &)),
			  SLOT(get_x(const QMouseEvent &)));
	connect(edit_plt, SIGNAL(plotMousePressed(const QMouseEvent &)),
			  SLOT(getMousePressed(const QMouseEvent &)));

	pb_accept->setEnabled(false);
	pb_details->setEnabled(false);
	pb_editscan->setEnabled(false);
	pb_exsingle->setEnabled(false);
	pb_exrange->setEnabled(false);
	pb_invert->setEnabled(false);
	pb_sub_baseline->setEnabled(false);

	connect(pb_select_dir, SIGNAL(clicked()), this, SLOT(directory()));
	connect(pb_details, SIGNAL(clicked()), this, SLOT(details()));
	connect(pb_close, SIGNAL(clicked()), this, SLOT(quitthis()));
	connect(pb_spikes, SIGNAL(clicked()), this, SLOT(remove_spikes()));
	connect(pb_accept, SIGNAL(clicked()), this, SLOT(load_dataset()));
	connect(pb_exclude_profile, SIGNAL(clicked()), this, SLOT(exclude_profile()));
	connect(pb_sub_baseline, SIGNAL(clicked()), this, SLOT(sub_baseline()));
	connect(cnt_exsingle, SIGNAL(valueChanged(double)), SLOT(set_range1(double)));
	connect(cnt_exrange, SIGNAL(valueChanged(double)), SLOT(set_range2(double)));
	connect(cnt_edit, SIGNAL(valueChanged(double)), SLOT(set_editscan(double)));
	connect(pb_exsingle, SIGNAL(clicked()), SLOT(exSingle()));
	connect(pb_exrange, SIGNAL(clicked()), SLOT(exRange()));
	connect(pb_editscan, SIGNAL(clicked()), SLOT(edSingle()));
	connect(pb_invert, SIGNAL(clicked()), SLOT(invert()));
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));
	connect(this,  SIGNAL(explain(const QString &)), lbl_instructions2,
	SLOT(setText(const QString &)));
}

EditData_Win::~EditData_Win()
{
	delete [] radius;
	delete [] absorbance;
	delete [] temp_radius;
	delete [] temp_absorbance;
}

void EditData_Win::closeEvent(QCloseEvent *e)
{
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
}

void EditData_Win::update_centerpiece(int index)
{
	current_centerpiece = index;
	cmbb_centerpiece->setCurrentItem(current_centerpiece);
	centerpiece = cp_list[current_centerpiece];
	run_inf.centerpiece[cell] = index;
}

void EditData_Win::remove_spikes()
{
	if (step < 7)
	{
		step_check();
		return;
	}
	float threshold = 10;
	float *x, *y, slope, intercept, sigma, correlation;
	x = new float [10];
	y = new float [10];
	unsigned int count, probed_point;
	for (unsigned int i=0; i<run_inf.scans[cell][lambda]; i++) // check all scans
	{
		for (unsigned int j=0; j<5; j++) // beginning points
		{
			count = 0;
			for (unsigned int k=0; k<10; k++) // test bottom 10 points
			{
				if (k != j) //exclude the probed point from fit
				{
					x[count] = radius[i][j+k];
					y[count] = absorbance[i][j+k];
					count ++;
				}
			}
			linefit (&x, &y, &slope, &intercept, &sigma, &correlation, 10);
			if (fabs(slope * radius[i][j] + intercept - absorbance[i][j]) > threshold * sigma) // if there is more than a 3-fold difference, it is a spike
			{
				absorbance[i][j] = slope * radius[i][j] + intercept; // interpolate
			}
		}
		for (unsigned int j=5; j<points[i]-4; j++) // center points
		{
			count = 0;
			for (int k=-5; k<6; k++) // test 5 points below and 5 points above
			{
				if (k != 0) //exclude the center point (to be probed) from fit
				{
					x[count] = radius[i][j+k];
					y[count] = absorbance[i][j+k];
					count ++;
				}
			}
			linefit (&x, &y, &slope, &intercept, &sigma, &correlation, 10);
			if (fabs(slope * radius[i][j] + intercept - absorbance[i][j]) > threshold * sigma) // if there is more than a 3-fold difference, it is a spike
			{
				absorbance[i][j] = slope * radius[i][j] + intercept; // interpolate
			}
		}
		probed_point = 0;
		for (unsigned int j=points[i]-10; j<points[i]; j++) // end points
		{
			count = 0;
			for (unsigned int k=0; k<10; k++) // test top 10 points
			{
				if (k != probed_point) //exclude the probed point from fit
				{
					x[count] = radius[i][j+k];
					y[count] = absorbance[i][j+k];
					count ++;
				}
			}
			linefit (&x, &y, &slope, &intercept, &sigma, &correlation, 10);
			if (fabs(slope * radius[i][j] + intercept - absorbance[i][j]) > threshold * sigma) // if there is more than a 3-fold difference, it is a spike
			{
				absorbance[i][j] = slope * radius[i][j] + intercept; // interpolate
			}
			probed_point ++;
		} // loop over end points
	} // loop over all scans
	delete [] x;
	delete [] y;
	plot_dataset();
	emit absorbance_changed();
}

void EditData_Win::update_rotor(int index)
{
	current_rotor = index;
	cmbb_rotor->setCurrentItem(current_rotor);
	rotor = rotor_list[current_rotor];
	run_inf.rotor = index;
}

void EditData_Win::exSingle()
{
cout << "Exclude_single: " << exclude_single << endl;
	unsigned int i;
	if (exclude_single == 0)
	{
		return;
	}
	/*
	if (edit_type == 1)
	{
		oldscan.erase(oldscan.begin() + (unsigned int) (exclude_single-1));
	}
	*/
	for (i=(unsigned int) exclude_single-1; i<(unsigned int) run_inf.scans[cell][lambda]-1; i++)
	{
		radius[i] 										= radius[i+1];
		absorbance[i]									= absorbance[i+1];
		points[i]										= points[i+1];
		run_inf.time[cell][lambda][i] 			= run_inf.time[cell][lambda][i+1];
		run_inf.plateau[cell][lambda][i]			= run_inf.plateau[cell][lambda][i+1];
		run_inf.temperature[cell][lambda][i]	= run_inf.temperature[cell][lambda][i+1];
		run_inf.omega_s_t[cell][lambda][i]		= run_inf.omega_s_t[cell][lambda][i+1];
		run_inf.rpm[cell][lambda][i]				= run_inf.rpm[cell][lambda][i+1];;
	}
	run_inf.scans[cell][lambda] --;
	run_inf.total_scans --;
	plot_dataset();
	cnt_edit->setValue(0);
	cnt_exrange->setValue(0);
	cnt_exsingle->setValue(0);
	exclude_single = 0;
	exclude_range = 0;
}

void EditData_Win::invert()
{
	if (step < 3)
	{
		step_check();
		return;
	}

	for (unsigned int i=0; i<run_inf.scans[cell][lambda]; i++)
	{
		for (unsigned int j=0; j<points[i]; j++)
		{
			absorbance[i][j] = -1.0 * absorbance[i][j];
		}
	}
	plot_dataset();
}

void EditData_Win::exRange()
{
	unsigned int i, excluded_scans;
	if (exclude_single == 0)
	{
		return;
	}
	if (exclude_range <= exclude_single)
	{
		return;
	}
	excluded_scans = exclude_range - exclude_single + 1;
	/*
	if (edit_type == 1)
	{
		oldscan.erase(oldscan.begin() + (unsigned int) (exclude_single-1), oldscan.begin() + (unsigned int) (exclude_range-1));
	}
	*/
	for (i=(unsigned int) exclude_single-1; i<(unsigned int) run_inf.scans[cell][lambda]-excluded_scans; i++)
	{
		radius[i] 										= radius[i+excluded_scans];
		absorbance[i]									= absorbance[i+excluded_scans];
		points[i]										= points[i+excluded_scans];
		run_inf.time[cell][lambda][i] 			= run_inf.time[cell][lambda][i+excluded_scans];
		run_inf.plateau[cell][lambda][i]			= run_inf.plateau[cell][lambda][i+excluded_scans];
		run_inf.temperature[cell][lambda][i]	= run_inf.temperature[cell][lambda][i+excluded_scans];
		run_inf.omega_s_t[cell][lambda][i]		= run_inf.omega_s_t[cell][lambda][i+excluded_scans];
		run_inf.rpm[cell][lambda][i]				= run_inf.rpm[cell][lambda][i+excluded_scans];;
	}
	run_inf.scans[cell][lambda] -= excluded_scans;
	run_inf.total_scans -= excluded_scans;
	plot_dataset();
	cnt_edit->setValue(0);
	cnt_exrange->setValue(0);
	cnt_exsingle->setValue(0);
}

void EditData_Win::edSingle()
{

	if (edit_scan == 0)
	{
		return;
	}
	edit_win = new edit_single_Win(&radius[edit_scan-1], &absorbance[edit_scan-1], points[edit_scan-1]);
	edit_win->setCaption(tr("Data Editing Window"));
	edit_win->show();
	connect(edit_win->frm->pb_accept, SIGNAL(clicked()), SLOT(plot_dataset()));
	connect(edit_win->frm->pb_accept, SIGNAL(clicked()), SIGNAL(absorbance_changed()));
}

void EditData_Win::set_range1(const double value)
{
	unsigned int i;
	cnt_exrange->setRange(value, cnt_exrange->maxValue(), 1);
	cnt_exrange->setValue(value);
	exclude_single = (unsigned int) value;
	edit_plt->clear();
	for (i=0; i<run_inf.scans[cell][lambda]; i++)
	{
		curve[i] = edit_plt->insertCurve("Optical Density");
		edit_plt->setCurvePen(curve[i], yellow);
		edit_plt->setCurveData(curve[i], radius[i], absorbance[i], points[i]);
	}
	if (exclude_single > 0)
	{
		edit_plt->setCurvePen(curve[exclude_single-1], red);
	}
	edit_plt->setAxisScale(QwtPlot::yLeft, ymin, ymax, 0);
	edit_plt->setAxisScale(QwtPlot::xBottom, xmin, xmax, 0);
	edit_plt->replot();
}

void EditData_Win::set_range2(double value)
{
	unsigned int i;
	cnt_exrange->setValue(value);
	exclude_range = (unsigned int) value;
	edit_plt->clear();
	for (i=0; i<run_inf.scans[cell][lambda]; i++)
	{
		curve[i] = edit_plt->insertCurve("Optical Density");
		edit_plt->setCurvePen(curve[i], yellow);
		edit_plt->setCurveData(curve[i], radius[i], absorbance[i], points[i]);
	}
	if (exclude_single > 0)
	{
		for (i=(unsigned int) exclude_single-1; i<exclude_range; i++)
		{
			edit_plt->setCurvePen(curve[i], red);
		}
	}
	edit_plt->setAxisScale(QwtPlot::yLeft, ymin, ymax, 0);
	edit_plt->setAxisScale(QwtPlot::xBottom, xmin, xmax, 0);
	edit_plt->replot();
}

void EditData_Win::set_editscan(double value)
{
	unsigned int i;
	edit_scan =	(unsigned int) value;
	edit_plt->clear();
	for (i=0; i<run_inf.scans[cell][lambda]; i++)
	{
		curve[i] = edit_plt->insertCurve("Optical Density");
		edit_plt->setCurvePen(curve[i], yellow);
		edit_plt->setCurveData(curve[i], radius[i], absorbance[i], points[i]);
	}
	if (edit_scan > 0)
	{
		edit_plt->setCurvePen(curve[edit_scan-1], magenta);
	}
	edit_plt->setAxisScale(QwtPlot::yLeft, ymin, ymax, 0);
	edit_plt->setAxisScale(QwtPlot::xBottom, xmin, xmax, 0);
	edit_plt->replot();
}

void EditData_Win::getMousePressed(const QMouseEvent &e)
{
// store position
    p1 = e.pos();
}

void EditData_Win::details()
{
	QString str;
	unsigned int i, j, count = 0, scan1=0, scan2=0, cell1, cell2=0, lambda1, lambda2=0, rpm1, rpm2=0;
	if (step == 1)
	{
/*
		for (unsigned int i=0; i<8; i++)
		{
			cout << "Scans: " << run_inf.scans[i][0] << endl;
			for (unsigned int j=0; j<run_inf.scans[i][0]; j++)
			{
				cout	<< "Temp: " << run_inf.temperature[i][0][j]
						<< ",omega: " << run_inf.omega_s_t[i][0][j]
						<< ", time: " << run_inf.time[i][0][j]
						<< ", rpm: " << run_inf.rpm[i][0][j] << endl;
			}
		}
*/
		float fval1 = -1, fval2 = -1;
		unsigned int **scan_counter;	// 2 dimensional array which counts the scans for each cell and each wavelength
		scan_counter = new unsigned int* [8];	// keeps track of the scan number for each cell and each wavelength
		for (i=0; i<8; i++)
		{
			scan_counter[i] = new unsigned int [3];
		}
		for (i=0; i<8; i++)
		{
			for (j=0; j<3; j++)
			{
				scan_counter[i][j] = 0;
			}
		}

		while (count < run_inf.total_scans)
		{
			for (i=0; i<8; i++)	//cell
			{
				for (j=0; j<3; j++)
				{
					if (run_inf.wavelength_count[i] > j)
					{
						if (scan_counter[i][j] < run_inf.scans[i][j])
						{
							fval1 = (float) run_inf.rpm[i][j][scan_counter[i][j]];
							rpm1 = (unsigned int) fval1;
							cell1 = i+1;
							lambda1 = j+1;
							scan1 = scan_counter[i][j] + 1;
							if (fval2 > 0)
							{
								if (edit_type == 1 || edit_type == 3 || edit_type == 5)		//absorbance velocity = 1, interference velocity = 3
								{
									if (fabs(fval2 - fval1) > 100)
									{
										str.sprintf(tr("This dataset appears to have been acquired with\n"
											"multiple speeds. This should not be the case.\n\n"
											"Make sure the speed was constant throughout\n"
											"the experiment. Velocity experiments need to\n"
											"be done at constant rotor speed (+/- 100 rpm)\n\n"
											"UltraScan will not proceed until you select a\n"
											"directory that includes data with a constant\n"
											"velocity speed profile. You will have to\n"
											"delete the scans at a non-constant speed and\n"
											"re-order all scans before you can edit the\n"
											"data with UltraScan.\n\n"
											"Speeds with problems:\n\n"
											"Cell %d, Wavelength %d, Scan %d: %d rpm\n"
											"Cell %d, Wavelength %d, Scan %d: %d rpm\n"),
											cell1, lambda1, scan1, rpm1, cell2, lambda2, scan2, rpm2);
										QMessageBox::warning(this, tr("UltraScan Warning"), str,
										QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
										for (i=0; i<8; i++)
										{
											delete [] scan_counter[i];
										}
										delete [] scan_counter;
										return;
									}
								}
							}
							fval2 = fval1;
							scan2 = scan1;
							cell2 = cell1;
							lambda2 = lambda1;
							rpm2 = rpm1;
							scan_counter[i][j]++;
							count++;
						}
					}
				}
			}
		}
		edit_details = new RunDetails_F(edit_type, 0, &run_inf);
		if (edit_type == 1)
		{
			edit_details->setCaption(tr("Details for Velocity Absorbance Data"));
		}
		else if (edit_type == 2)
		{
			edit_details->setCaption(tr("Details for Equilibrium Absorbance Data"));
		}
		else if (edit_type == 3)
		{
			edit_details->setCaption(tr("Details for Velocity Interference Data"));
		}
		else if (edit_type == 4)
		{
			edit_details->setCaption(tr("Details for Equilibrium Interference Data"));
		}
		else if (edit_type == 5)
		{
			edit_details->setCaption(tr("Details for Velocity Fluorescence Data"));
		}
		else if (edit_type == 6)
		{
			edit_details->setCaption(tr("Details for Equilibrium Fluorescence Data"));
		}
		edit_details->show();
		connect(edit_details->pb_cancel, SIGNAL(clicked()), SLOT(directory()));
		connect(edit_details->pb_accept, SIGNAL(clicked()), SLOT(edit_all()));
		connect(edit_details->run_id_le, SIGNAL(returnPressed()), SLOT(edit_all()));
	}
	else if (step > 1)
	{
		delete edit_details;
		edit_details = new RunDetails_F(edit_type, 0, &run_inf);
		if (edit_type == 1)
		{
			edit_details->setCaption(tr("Details for Velocity Absorbance Data"));
		}
		else if (edit_type == 2)
		{
			edit_details->setCaption(tr("Details for Equilibrium Absorbance Data"));
		}
		else if (edit_type == 3)
		{
			edit_details->setCaption(tr("Details for Velocity Interference Data"));
		}
		else if (edit_type == 4)
		{
			edit_details->setCaption(tr("Details for Equilibrium Interference Data"));
		}
		else if (edit_type == 5)
		{
			edit_details->setCaption(tr("Details for Velocity Fluorescence Data"));
		}
		else if (edit_type == 6)
		{
			edit_details->setCaption(tr("Details for Equilibrium Fluorescence Data"));
		}		edit_details->show();
	}
	else
	{
		step_check();
	}
}

void EditData_Win::set_zoom()
{
	if (zoomflag)
	{
		zoomflag = false;
		pb_zoom->setText(tr("Continue"));
		edit_plt->setOutlineStyle(Qwt::VLine);
		str.sprintf(tr("Now click near the point that best defines the MENISCUS.\n"
						"After the MENISCUS is defined, a red cross will identify\n"
						"the MENISCUS position.\n"
						"Then click on the \"Continue\" Button to align the scans..."));
		lbl_instructions2->setText(str);

	}
	else
	{
		plot_dataset();
		uint line;
		double r[2],y[2], maximum = ymax - (ymax - ymin)/15;
		str.sprintf(tr("Meniscus at %5.3f cm"), run_inf.meniscus[cell]);
		uint meniscus_label = edit_plt->insertMarker();
		edit_plt->setMarkerLabel(meniscus_label, str);
		edit_plt->setMarkerPos(meniscus_label, run_inf.meniscus[cell], maximum + (ymax-ymin)/20);
		edit_plt->setMarkerFont(meniscus_label, QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
		edit_plt->setMarkerPen(meniscus_label, QPen(cyan, 0, DashDotLine));

		r[0] = run_inf.meniscus[cell];
		r[1] = run_inf.meniscus[cell];
		y[0] = ymin;
		y[1] = maximum - (ymax - ymin)/30;
		line = edit_plt->insertCurve("Line");
		edit_plt->setCurvePen(line, QPen(white, 2));
		edit_plt->setCurveData(line, r, y, 2);
		edit_plt->replot();
		str.sprintf(tr("The MENISCUS is now defined. It was found to be at %5.3f cm.\n\n"
						"Next, The scans need to be aligned in the air-to-air region\n"
						"Please click on the left limit of the air-to-air region now..."),
						run_inf.meniscus[cell]);
		lbl_instructions2->setText(str);
		step = 4;
		delete pb_zoom;
	}
}

void EditData_Win::fluorescence_directory()
{
	QFileDialog *fd;
	QFile f;
	QString trashcan;
	unsigned int i, j, count=0;
	unsigned int scans_upper = 0, scans_lower = 0;
	unsigned int sequence_check = 0, *channels;
	QString *flo_channel;
	flo_channel = new QString [5];

	channels = new unsigned int [5];
	// sequence check is a counting variable used to verify that all scanfiles are in sequence
	float t_temp, t_ost; 	// temporary variables for time, temperature and omega_s_t
	float t_time;

	lbl_instructions2->setText(tr("Please select a directory where your\n"
										   "fluorescence data is located"));

	run_inf.run_id=""; // we are starting with a new selection, initialize to empty
	run_inf.data_dir = USglobal->config_list.data_dir;
	QString str1, str2;
	str1 = "*." + extension1 + "*, *." + extension2 + "*";
	fd = new QFileDialog(USglobal->config_list.data_dir, str, 0, 0, TRUE);
	run_inf.data_dir = fd->getExistingDirectory(USglobal->config_list.data_dir, 0, 0, "AUC Data", false, true);
	lbl_directory->setText(run_inf.data_dir);
	if (!run_inf.data_dir.isEmpty())
	{
		step = 1;
		data_dir->setPath(run_inf.data_dir);
		run_inf.total_scans = 0;
		for (i=0; i<8; i++)		// find the number of scan files for each cell
		{
			str.sprintf("*." + extension1 + "%d", i+1);
			data_dir->setNameFilter(str);
			scans_upper += data_dir->count();
		}
		extension = extension2; // next, count the files with lowercase extensions
		for (i=0; i<8; i++)		// find the number of scan files for each cell
		{
			str.sprintf("*." + extension2 + "%d", i+1);
			data_dir->setNameFilter(str);
			scans_lower += data_dir->count();
		}
		if (scans_upper >= scans_lower) // this system does not allow for mixed case scenarios!
		{
			extension = extension1;
			flo_channel[0] = "A";
			flo_channel[1] = "B";
			flo_channel[2] = "C";
			flo_channel[3] = "D";
			flo_channel[4] = "E";
		}
		else
		{
			extension = extension2;
			flo_channel[0] = "a";
			flo_channel[1] = "b";
			flo_channel[2] = "c";
			flo_channel[3] = "d";
			flo_channel[4] = "e";
		}
		count = 0;
		for (j=0; j<5; j++) // find out how many channels
		{
			channels[j] = 0;
			str2 = flo_channel[j];
			for (i=0; i<8; i++)		// find the number of scan files for each channel
			{
				str.sprintf(str2 + "*." + extension + "%d", i+1);
				data_dir->setNameFilter(str);
				if (data_dir->count() > 2) // only count cells where a channel has 3 or more scans
				{
					channels[j] += data_dir->count();
				}
			}
			if (channels[j] > 2) // find out which channel contain data
			{
				count ++; // count how many channels contain data
			}
		}
		if (count > 1) // we have multiple channels, ask the user to pick one channel
		{
			US_SelectChannel sc(&edited_channel, &channels);
			if(sc.exec() == QDialog::Rejected)
			{
				return;
			}
			else
			{
				emit channel_selected(edited_channel);
			}
		}
		else
		{
			for (j=0; j<5; j++) // find out how many channels
			{
				if(channels[j] > 0) //find the nonzero channel (there is only one)
				{
					edited_channel = flo_channel[j];
				}
			}
		}
		run_inf.total_scans = 0;
		for (i=0; i<8; i++)		// find the number of scan files for each cell
		{
			str.sprintf(edited_channel + "*." + extension + "%d", i+1);
			data_dir->setNameFilter(str);
			run_inf.scans[i][0] = data_dir->count();
			run_inf.total_scans += run_inf.scans[i][0];
			run_inf.cell_id[i] = "";
			if (run_inf.scans[i][0] > 2)
			{
				run_inf.wavelength_count[i] = 1;
				run_inf.wavelength[i][0] = 488;
			}
			else
			{
				run_inf.wavelength_count[i] = 0;
				run_inf.wavelength[i][0] = 0;
			}
			for (j=1; j<3; j++)
			{
				run_inf.scans[i][j] = 0;
				run_inf.wavelength[i][j] = 0;
			}
		}
		progress->reset();
		progress->setTotalSteps(run_inf.total_scans);
		run_inf.temperature = new float** [8];
		run_inf.time        = new unsigned int** [8];
		run_inf.omega_s_t   = new float** [8];
		run_inf.plateau   = new float** [8];
		run_inf.rpm   = new unsigned int** [8];
		for (i=0; i<8; i++)
		{
			run_inf.temperature[i] = new float* [run_inf.wavelength_count[i]];
			run_inf.time[i]        = new unsigned int* [run_inf.wavelength_count[i]];
			run_inf.omega_s_t[i]   = new float* [run_inf.wavelength_count[i]];
			run_inf.plateau[i]   = new float* [run_inf.wavelength_count[i]];
			run_inf.rpm[i]   = new unsigned int* [run_inf.wavelength_count[i]];

			run_inf.temperature[i][0] = new float [run_inf.scans[i][0]];
			run_inf.time[i][0]        = new unsigned int [run_inf.scans[i][0]];
			run_inf.omega_s_t[i][0]   = new float [run_inf.scans[i][0]];
			run_inf.plateau[i][0]   = new float [run_inf.scans[i][0]];
			run_inf.rpm[i][0]   = new unsigned int [run_inf.scans[i][0]];
		}
		count = 0;
		for (i=0; i<8; i++)
		{
			for (j=1; j<=run_inf.scans[i][0]; j++)
			{
				if (j<10)
				{
					str.sprintf(edited_channel + "0000%d." + extension + "%d", j, i+1);
				}
				else if (j>9 && j<100)
				{
					str.sprintf(edited_channel + "000%d." + extension + "%d", j, i+1);
				}
				else if (j>99 && j<1000)
				{
					str.sprintf(edited_channel + "00%d." + extension + "%d", j, i+1);
				}
				else if (j>999 && j<10000)
				{
					str.sprintf(edited_channel + "0%d." + extension + "%d", j, i+1);
				}
				else if (j>9999 && j<100000)
				{
					str.sprintf(edited_channel + "%d." + extension + "%d", j, i+1);
				}
				str.prepend(run_inf.data_dir);
				f.setName(str);
				if (f.open(IO_ReadOnly))
				{
					QTextStream ts(&f);
					if (!ts.atEnd())
					{
						run_inf.cell_id[i] = ts.readLine();
						if (run_inf.cell_id[i] == NULL)
						{
							QMessageBox::message(tr("Attention:"), tr("The file:\n\n") + str +
														tr("\n\n appears to be truncated - please delete\n"
														"the file first, and reorder the scans,\n"
														"then try again."));
							close();
						}
						ts >> trashcan;	// radial or wavelength scan mode
						if (trashcan == NULL || trashcan.stripWhiteSpace() == "")
						{
							QMessageBox::message(tr("Attention:"), tr("The file:\n\n") + str +
														tr("\n\n appears to be truncated - please delete\n"
															"the file first, and reorder the scans,\n"
															"then try again."));
							close();
						}
						sequence_check ++;
						ts >> trashcan;		// cell number
						ts >> t_temp;			// temperature
						ts >> trashcan;			// rotor speed (sometimes starts with leading zero, needs to be converted from QString
						t_rpm = trashcan.toUInt();
						ts >> t_time;			// time in seconds
						if (USglobal->config_list.beckman_bug)
						{
								t_time = t_time * 10 + 5;
						}
						ts >> t_ost;			// omega-square-t
						run_inf.temperature[i][0][j-1] 	= t_temp;
						run_inf.time[i][0][j-1] 			= (Q_UINT32) t_time;
						run_inf.omega_s_t[i][0][j-1]		= t_ost;
						run_inf.rpm[i][0][j-1]				= t_rpm;
// cout << str << ": temp: " << run_inf.temperature[i][0][j] << ", time: " << run_inf.time[i][0][j]
// << ", omega^2t: " << run_inf.omega_s_t[i][0][j] << ", rpm: " << run_inf.rpm[i][0][j] << endl;
					}
				}
				else
				{
					QMessageBox::message(tr("Attention:"), tr("UltraScan cannot find\nthe file: ") + str +
												tr("\n\nPlease reorder the scans,\n"
													"then try again."));
					close();
				}
				f.close();
				count ++;
				progress->setProgress(count);
				qApp->processEvents();
			}
		}
		QString dbfile;
		dbfile = run_inf.data_dir + "db_info.dat";
		QFile f(dbfile);
		if(!f.exists())		//if can not find 'db_info.dat' file, then set all value to -1
		{
			run_inf.expdata_id = -1;
			run_inf.investigator = -1;
			QDate today = QDate::currentDate();
			run_inf.date.sprintf( "%d/%d/%d", today.month(), today.day(), today.year() );
			run_inf.description = run_inf.run_id;
			run_inf.dbname = "ultrascan";
			run_inf.dbhost = "192.168.0.1";
			run_inf.dbdriver = "QMYSQL3";
			for(int i=0; i<8; i++)
			{
				for(int j=0; j<4; j++)
				{
					run_inf.buffer_serialnumber[i][j] = -1;
					for(int k=0; k<3; k++)
					{
						run_inf.peptide_serialnumber[i][j][k] = -1;
						run_inf.DNA_serialnumber[i][j][k] = -1;
					}
				}
			}
		}
		else
		{
			run_inf.run_id = run_inf.data_dir.section('/',-2, -1);		//set run_id with file dir name
			if (f.open(IO_ReadOnly))
			{
				QTextStream ts(&f);
				if (!ts.atEnd())
				{
					ts >> run_inf.expdata_id;			// get experimental data id
					trashcan = ts.readLine();					//get rid of the rest of the line
					ts >> run_inf.investigator;		// get investigator info
					trashcan = ts.readLine();					//get rid of the rest of the line
					ts >> run_inf.date;					// get date info
					trashcan = ts.readLine();					//get rid of the rest of the line
					run_inf.description = ts.readLine();			// get description info
					ts >> run_inf.dbname;						// get dbname info
					trashcan = ts.readLine();				//get rid of the rest of the line
					ts >> run_inf.dbhost;						// get dbhost info
					trashcan = ts.readLine();				//get rid of the rest of the line
					ts >> run_inf.dbdriver;						// get dbdriver info
					trashcan = ts.readLine();				//get rid of the rest of the line
					ts >> run_inf.rotor;					// get rotor info
					trashcan = ts.readLine();					//get rid of the rest of the line
					update_rotor(run_inf.rotor);
					for(int i=0; i<8; i++)
					{
						if (run_inf.cell_id[i] != "")		//if this cell has data
						{
							ts >> run_inf.centerpiece[i];	// get centerpiece info
							trashcan = ts.readLine();		// get rid of the rest of line
							update_centerpiece(run_inf.centerpiece[i]);
							if ( strcmp( US_Version, "7.0" ) < 0 )
							{
								for(int j=0; j<4; j++)
								{
									ts >> run_inf.DNA_serialnumber[i][j][0];		//get the number
									trashcan = ts.readLine();						//get rid of the rest of the line
									ts >> run_inf.buffer_serialnumber[i][j];
									trashcan = ts.readLine();
									ts >> run_inf.peptide_serialnumber[i][j][0];
									trashcan = ts.readLine();
								}
							}
							else
							{
								for(int j=0; j<4; j++)
								{
									ts >> run_inf.buffer_serialnumber[i][j];
									trashcan = ts.readLine();
									for(int k=0; k<3; k++)
									{
										ts >> run_inf.peptide_serialnumber[i][j][k];
										trashcan = ts.readLine();
										ts >> run_inf.DNA_serialnumber[i][j][k];
										trashcan = ts.readLine();
									}
								}
							}
						}
					}
				}
			}
		}
		lbl_instructions2->setText(tr("Please review the details of this run in the summary window\n"
												"and enter a descriptive name for your selected dataset (no spaces)\n"));
		details();
	}
	else
	{
		step = 0;
		lbl_instructions2->setText(tr("Please click on `Select Data Directory` and select a directory\n"
	                               "where your data is located. The data should be in a sub-\n"
	                               "directory of the Beckman Data Acquisition program in the format:\n"
	                               "/.../xlawin/data/mmddyy/hhmmss (month/day/year/hour/min/sec)"));
		lbl_directory->setText(tr("not selected"));
		lbl_run_id2->setText(tr("not selected"));
		lbl_current_cell2->setText(tr("not selected"));
		lbl_details->setText(tr("not selected"));
	}
	pb_accept->setEnabled(true);
	pb_details->setEnabled(true);
}

void EditData_Win::directory()
{
	if(extension1 == "FI" || extension2 == "fi")
	{
		fluorescence_directory();
		return;
	}
	QFileDialog *fd;
	QFile f;
	QString trashcan;
	unsigned int i, j, count=0, current_lambda=0, *scan_count;
	unsigned int scans_upper = 0, scans_lower = 0;
	unsigned int sequence_check;
	// sequence check is a counting variable used to verify that all scanfiles are in sequence
	float t_temp, t_ost; 	// temporary variables for time, temperature and omega_s_t
	float t_time;
	bool veloc_flag;
	scan_count = new unsigned int [8];		// how many scans in each cell?
	lbl_instructions2->setText(tr("Please select a directory where your data is located. The data\n"
														"should be in a subdirectory of the Beckman Data Acquisition\n"
														"program in the format:\n"
	       				                     "/.../xlawin/data/mmddyy/hhmmss (month/day/year/hour/min/sec)"));
	run_inf.run_id=""; // we are starting with a new selection, initialize to empty
//	QMessageBox::message("This is the data directory:", USglobal->config_list.data_dir);
	run_inf.data_dir = USglobal->config_list.data_dir;
	QString str1, str2;
	QString s = "*." + extension + "*";
	fd = new QFileDialog(USglobal->config_list.data_dir, s, 0, 0, TRUE);
//	fd->setSelection(USglobal->config_list.data_dir);
//	fd->setSelection("/auc/data");
	run_inf.data_dir = fd->getExistingDirectory(USglobal->config_list.data_dir, 0, 0, "AUC Data", false, true);
	lbl_directory->setText(run_inf.data_dir);
//debug("String length: %d", );
//	connect(fd, SIGNAL(dirEntered(const char*)), SLOT(newdir(const char*)));
	if (!run_inf.data_dir.isEmpty())
	{
		step = 1;
		data_dir->setPath(run_inf.data_dir);
		run_inf.total_scans = 0;
		extension = extension1; // first count the files with uppercase extensions
		for (i=0; i<8; i++)		// find the number of scan files for each cell
		{
			str.sprintf("*." + extension + "%d", i+1);
			data_dir->setNameFilter(str);
			scans_upper += data_dir->count();
		}
		extension = extension2; // next, count the files with lowercase extensions
		for (i=0; i<8; i++)		// find the number of scan files for each cell
		{
			str.sprintf("*." + extension + "%d", i+1);
			data_dir->setNameFilter(str);
			scans_lower += data_dir->count();
		}
		if (scans_upper >= scans_lower) // this system does not allow for mixed case scenarios!
		{
			extension = extension1;
		}
		else
		{
			extension = extension2;
		}
		QString ext_chk = extension.left(1).lower();
		for (i=0; i<8; i++)		// find the number of scan files for each cell
		{
			str.sprintf("*." + extension + "%d", i+1);
			data_dir->setNameFilter(str);
			scan_count[i] = data_dir->count();
			run_inf.total_scans += scan_count[i];
			run_inf.cell_id[i] = "";
		}
		for (i=0; i<8; i++)			// first, we collect the wavelengths for the first 3 scans of each cell
		{
			veloc_flag = false;		// if there aren't enough scans in each cell this is FALSE, reset before each cell
/*
			bool check_flag = false;
			//if extension is WA or WI, the scan_count[i] unnessary more than 3
			if(ext_chk != "w")
			{
				if(scan_count[i] >= 3)
				{
					check_flag = true;
				}
			}
			else
			{
				check_flag = true;
			}
			if (check_flag)
*/
			if (scan_count[i] >= 3)
			{
				veloc_flag = true;	// we have a cell with at least 3 scans in it
				count = 0;
				for (j=0; j<3; j++)
				{
					run_inf.wavelength[i][j] = 0;
					str.sprintf("/0000%d." + extension + "%d", j+1, i+1);	// check first 3 scans only
					str.prepend(run_inf.data_dir);
//					cout<<str<<endl;
					f.setName(str);
					if (f.open(IO_ReadOnly))
					{
//
// If the file exists, we increment count to make sure we are starting at scan 1 and have scans in sequence,
// if count is not 3 by then of this, we have a problem with the sequence and need to exit (in order for
// this algorithm to work properly with multiple wavelengths, we need to have all scans in sequence.
//
						QTextStream ts(&f);
						if (!ts.atEnd())
						{
							trashcan = ts.readLine();
							if (trashcan == NULL)
							{
								QMessageBox::message(tr("Attention:"), tr("The file:\n\n") + str +
															tr("\n\n appears to be truncated - please delete\n"
															"the file first, and reorder the scans, if necessary,\n"
															"then try again."));
								close();
							}
							ts >> trashcan;	// radial or wavelength scan mode
							if (trashcan == NULL || trashcan.stripWhiteSpace() == "")
							{
								QMessageBox::message(tr("Attention:"), tr("The file:\n\n") + str +
															tr("\n\n appears to be truncated - please delete\n"
															"the file first, and reorder the scans, if necessary,\n"
															"then try again."));
								close();
							}
							ts >> trashcan;	// cell number
							ts >> trashcan;	// temperature
							ts >> trashcan;	//	rotor speed
							ts >> trashcan;	// time in secs
							ts >> trashcan;	// omega-squared-t
							ts >> run_inf.wavelength[i][j]; // if wavelength data, this variable contains the radius of the measurement
							count ++;
						}
						f.close();
					}
					qApp->processEvents();		// check to see if there is any other pressing business that can't wait
				}
				if (count != 3)	// this test doesn't check if any other files except the first three are
				{						// out of sequence. This is done later in the editing program during loading.
					str1.sprintf(tr("The scans for cell %d do not start with scanfile 00001"), i+1);
					str2 = tr("or are out\nof sequence.\n\nIn order to properly distinguish between multiple wavelengths\n"
							 "in your experiment the scanfiles have to start with scan 00001\n"
							 "and be in sequence.\n\n"
							 "If this run has been acquired with a single wavelength you can\n"
							 "use the \"Scanfile Ordering Utility\" from the UltraScan Utility\n"
							 "Menu on the main screen of UltraScan to recreate the proper\n"
							 "scanfile sequence and after ordering, edit this data.\n\n"
							 "Another possibility for this error is that no files were found.");
					str1.append(str2);
					step = 0;
					lbl_instructions2->setText(tr("Please click on `Select Data Directory` and select a directory\n"
	                              						 	"where your data is located. The data should be in a sub-\n"
																		"directory of the Beckman Data Acquisition program in the format:\n"
	   																"/.../xlawin/data/mmddyy/hhmmss (month/day/year/hour/min/sec)"));
					lbl_directory->setText(tr("not selected"));
					lbl_run_id2->setText(tr("not selected"));
					lbl_current_cell2->setText(tr("not selected"));
					lbl_details->setText(tr("not selected"));
					QMessageBox::message(tr("Attention:"), str1);
					return;
				}
			}
			if (!veloc_flag && scan_count[i] > 0)		//there are only one or two scans, inform user and quit now.
			{
				if (edit_type == 1 || edit_type == 3 || edit_type == 5)
				{
					str1.sprintf(tr("There are not enough data scans in cell %d\n"), i+1);
					str2 = 		 tr("to do a meaningful velocity analysis.\n"
									 "Also: UltraScan needs at least 3 scans/cell\n"
									 "in order to determine wavelength information.\n\n"
									 "Please select a different data directory\n"
									 "or move/delete the scanfiles associated\n"
									 "with this cell.");
					str1.append(str2);
					QMessageBox::message(tr("Attention:"), str1);
				}
				else if (edit_type == 2 || edit_type == 4)
				{
					str1.sprintf(tr("There are not enough data scans in cell %d\n\n"), i+1);
					str2 =		 tr("UltraScan needs at least 3 scans/cell in\n"
									 "order to determine wavelength information.\n\n"
									 "Please select a different data directory\n"
									 "or make a copy of the existing scan(s) to\n"
									 "at least create files for scans 2 and 3\n"
									 "which you can delete later during editing.");
					str1.append(str2);
					QMessageBox::message(tr("Attention:"), str1);
				}
				lbl_instructions2->setText(tr("Please click on `Select Data Directory` and select a directory\n"
	                               "where your data is located. The data should be in a sub-\n"
	                               "directory of the Beckman Data Acquisition program in the format:\n"
	                               "/.../xlawin/data/mmddyy/hhmmss (month/day/year/hour/min/sec)"));
				step = 0;	// go back to the beginning
				return;
			}
		}
		if (run_inf.total_scans == 0)		//there are no scans in any cells, inform user and quit now.
		{
			str1 = (tr("There are no data scans in:\n\n "));
			str1.append(run_inf.data_dir);
			str1.append("\n\n");
			str2 = 		 tr("UltraScan needs at least 3 scans/cell in\n"
							 "order to determine wavelength information.\n\n"
							 "Please select a different data directory\n"
							 "or move/delete the scanfiles associated\n"
							 "with this cell.");
			str1.append(str2);
			QMessageBox::message(tr("Attention:"), str1);
			lbl_instructions2->setText(tr("Please click on `Select Data Directory` and select a directory\n"
                               "where your data is located. The data should be in a sub-\n"
                               "directory of the Beckman Data Acquisition program in the format:\n"
                               "/.../xlawin/data/mmddyy/hhmmss (month/day/year/hour/min/sec)"));
			step = 0;	// go back to the beginning
			return;
		}
		progress->reset();
		progress->setTotalSteps(run_inf.total_scans);
		lbl_instructions2->setText(tr("Please wait...\nCurrently analyzing "
															"the file structure and contents of the\n"
		                                       "selected directory"));
		for (i=0; i<8; i++)		// see which ones are different
		{
			run_inf.wavelength_count[i] = 0;	//initialize with 0 wavelengths, increase later if necessary
			if ((run_inf.wavelength[i][0] != 0) &&
				 (run_inf.wavelength[i][1] != 0) &&
				 (run_inf.wavelength[i][2] != 0))	// OK, there are at least 3 scans
			{    // if they are less than 3 nm apart, they are considered to be the same wavelength
				if (abs((int) (run_inf.wavelength[i][0] - run_inf.wavelength[i][1])) < 3 &&
					 abs((int) (run_inf.wavelength[i][0] - run_inf.wavelength[i][2])) < 3)
				{ // all three are the same, set 2nd and 3rd to zero
					run_inf.wavelength[i][1] = 0;
					run_inf.wavelength[i][2] = 0;
					run_inf.wavelength_count[i] = 1;
				}
				else if (abs((int) (run_inf.wavelength[i][0] - run_inf.wavelength[i][1])) > 2 &&
					 abs((int) (run_inf.wavelength[i][0] - run_inf.wavelength[i][2])) < 3)
				{ // the third is the same as the first, set the third to zero
					run_inf.wavelength[i][2] = 0;
					run_inf.wavelength_count[i] = 2;
				}
				else // we have three different wavelengths
				{
					run_inf.wavelength_count[i] = 3;
				}
			}
		}
/*
In order to identify each time-, omega_s_t- and temperature entry with the correct cell
and wavelength, and in order to allocate the appropriate amount of memory for those variables,
we need to know the number of cells, number of different wavelengths and the number of scans
for the run. The number of wavelengths for each cell are saved in wavelength[cell], the number
of cells is fixed at 8, and the number of scans needs to be figured out from the total number
of scans for each cell and the number of different wavelengths available for each cell. Since
we know both, we can calculate how many scans there are for each. We then

***********assume*************

that  for an uneven division of total_scans[cell]/wavelengths[cell], the last one or two
wavelengths are not available, and in that order.
*/

		for (i=0; i<8; i++)
		{
			switch (run_inf.wavelength_count[i])
	      {
		      case 1:
	         {
					run_inf.scans[i][0] = scan_count[i];
					run_inf.scans[i][1] = 0;
					run_inf.scans[i][2] = 0;
					break;
		      }
		      case 2:
	         {
					run_inf.scans[i][0] = (unsigned int) (0.5 + scan_count[i] / 2);
					run_inf.scans[i][1] = scan_count[i] - run_inf.scans[i][0];
					run_inf.scans[i][2] = 0;
					break;
		      }
		      case 3:
	         {
					run_inf.scans[i][0] = (unsigned int) (0.7 + scan_count[i] / 3);
					run_inf.scans[i][1] = (unsigned int) (0.5 + (scan_count[i] - run_inf.scans[i][0]) / 2);
					run_inf.scans[i][2] = scan_count[i] - run_inf.scans[i][0] - run_inf.scans[i][1];
					break;
		      }
			}
		}
//
// Now allocate the appropriate memory for the three, 3-dimensional variables temperature,
//	time and omega_s_t:
//
		run_inf.temperature = new float** [8];
		run_inf.time        = new unsigned int** [8];
		run_inf.omega_s_t   = new float** [8];
		run_inf.plateau   = new float** [8];
		run_inf.rpm   = new unsigned int** [8];
		for (i=0; i<8; i++)
		{
			run_inf.temperature[i] = new float* [run_inf.wavelength_count[i]];
			run_inf.time[i]        = new unsigned int* [run_inf.wavelength_count[i]];
			run_inf.omega_s_t[i]   = new float* [run_inf.wavelength_count[i]];
			run_inf.plateau[i]   = new float* [run_inf.wavelength_count[i]];
			run_inf.rpm[i]   = new unsigned int* [run_inf.wavelength_count[i]];
			for (j=0; j<run_inf.wavelength_count[i]; j++)
			{
				run_inf.temperature[i][j] = new float [run_inf.scans[i][j]];
				run_inf.time[i][j]        = new unsigned int [run_inf.scans[i][j]];
				run_inf.omega_s_t[i][j]   = new float [run_inf.scans[i][j]];
				run_inf.plateau[i][j]   = new float [run_inf.scans[i][j]];
				run_inf.rpm[i][j]   = new unsigned int [run_inf.scans[i][j]];
			}
		}
		count = 0;
//
// re-initialize the scan numbers for each cell & wavelength back to 0 since they will be used as
// counting variables for each cell and wavelength
//
		for (i=0; i<8; i++)
		{
			for (j=0; j<3; j++)
			{
				run_inf.scans[i][j] = 0;
			}
		}
		for (i=0; i<8; i++)		// look at the first 2 lines in each file
		{
			sequence_check = 0;
			for (j=1; j<=scan_count[i]; j++)
			{
				qApp->processEvents();
				if (j<10)
				{
					str.sprintf("0000%d." + extension + "%d", j, i+1);
				}
				else if (j>9 && j<100)
				{
					str.sprintf("000%d." + extension + "%d", j, i+1);

				}
				else if (j>99 && j<1000)
				{
					str.sprintf("00%d." + extension + "%d", j, i+1);

				}
				else if (j>999 && j<10000)
				{
					str.sprintf("0%d." + extension + "%d", j, i+1);

				}
				else if (j>9999 && j<100000)
				{
					str.sprintf("%d." + extension + "%d", j, i+1);

				}
				str.prepend(run_inf.data_dir);
				f.setName(str);
				if (f.open(IO_ReadOnly))
				{
					QTextStream ts(&f);
					if (!ts.atEnd())
					{
						run_inf.cell_id[i] = ts.readLine();
						if (run_inf.cell_id[i] == NULL)
						{
							QMessageBox::message(tr("Attention:"), tr("The file:\n\n") + str +
														tr("\n\n appears to be truncated - please delete\n"
														"the file first, and reorder the scans, if necessary,\n"
														"then try again."));
							close();
						}
						ts >> trashcan;	// radial or wavelength scan mode
						if (trashcan == NULL || trashcan.stripWhiteSpace() == "")
						{
							QMessageBox::message(tr("Attention:"), tr("The file:\n\n") + str +
														tr("\n\n appears to be truncated - please delete\n"
														"the file first, and reorder the scans, if necessary,\n"
														"then try again."));
							close();
						}
						sequence_check ++;
						ts >> trashcan;		// cell number
						ts >> t_temp;			// temperature
						ts >> t_rpm;			// rotor speed
						ts >> t_time;			// time in seconds
						if (USglobal->config_list.beckman_bug)
						{
							t_time = t_time * 10 + 5;
						}
						ts >> t_ost;			// omega-square-t
						switch (run_inf.wavelength_count[i])	// what we do depends on how many different
				      {													// wavelengths we have for each cell
					      case 1:
				         {
								run_inf.temperature[i][0][run_inf.scans[i][0]] 	= t_temp;
								run_inf.time[i][0][run_inf.scans[i][0]] 			= (Q_UINT32) t_time;
								run_inf.omega_s_t[i][0][run_inf.scans[i][0]]		= t_ost;
								run_inf.rpm[i][0][run_inf.scans[i][0]]				= t_rpm;
								run_inf.scans[i][0] ++;
								break;
							}
					      case 2:
				         {
								if (current_lambda == 2)
								{
									current_lambda = 0;
								}
								run_inf.temperature[i][current_lambda][run_inf.scans[i][current_lambda]] 	= t_temp;
								run_inf.time[i][current_lambda][run_inf.scans[i][current_lambda]] 			= (Q_UINT32) t_time;
								run_inf.omega_s_t[i][current_lambda][run_inf.scans[i][current_lambda]] 		= t_ost;
								run_inf.rpm[i][current_lambda][run_inf.scans[i][current_lambda]]				= t_rpm;
								run_inf.scans[i][current_lambda] ++;
								current_lambda++;
								break;
							}
					      case 3:
				         {
								if (current_lambda == 3)
								{
									current_lambda = 0;
								}
								run_inf.temperature[i][current_lambda][run_inf.scans[i][current_lambda]] 	= t_temp;
								run_inf.time[i][current_lambda][run_inf.scans[i][current_lambda]] 			= (Q_UINT32) t_time;
								run_inf.omega_s_t[i][current_lambda][run_inf.scans[i][current_lambda]]	 	= t_ost;
								run_inf.rpm[i][current_lambda][run_inf.scans[i][current_lambda]]				= t_rpm;
								run_inf.scans[i][current_lambda] ++;
								current_lambda++;
								break;
							}
						}
					}
					f.close();
					count ++;
					progress->setProgress(count);
				}
			}
			if (sequence_check != scan_count[i])	// this test makes sure that the scanfiles are in sequence
			{
				str1.sprintf(tr("The scans for cell %d are not in sequence!"), i+1);
				str2 = tr("\n\nIn order to properly distinguish between multiple wavelengths\n"
						 "in your experiment the scanfiles have to start with scan 00001\n"
						 "and be in sequence.\n\n"
						 "If this run has been acquired with a single wavelength you can\n"
						 "use the \"Scanfile Ordering Utility\" from the UltraScan Utility\n"
						 "Menu on the main screen of UltraScan to recreate the proper\n"
						 "scanfile sequence and after ordering, edit this data.");
				str1.append(str2);
				progress->reset();
				step = 0;
				lbl_instructions2->setText(tr("Please click on `Select Data Directory` and select a directory\n"
	                               "where your data is located. The data should be in a sub-\n"
	                               "directory of the Beckman Data Acquisition program in the format:\n"
	                               "/.../xlawin/data/mmddyy/hhmmss (month/day/year/hour/min/sec)"));
				lbl_directory->setText(tr("not selected"));
				lbl_run_id2->setText(tr("not selected"));
				lbl_current_cell2->setText(tr("not selected"));
				lbl_details->setText(tr("not selected"));
				QMessageBox::message(tr("Attention:"), str1);
				return;
			}
		}
/****************************************************************************************************************/
/******* get rotor, centerpiece, DNA, Buffer and Peptide serial number by reading 'db_info.dat' file************/
		QString dbfile;
		dbfile = run_inf.data_dir + "db_info.dat";
		QFile f(dbfile);
		if(!f.exists())		//if can not find 'db_info.dat' file, then set all value to -1
		{
			run_inf.expdata_id = -1;
			run_inf.investigator = -1;
			QDate today = QDate::currentDate();
			run_inf.date.sprintf( "%d/%d/%d", today.month(), today.day(), today.year() );
			run_inf.description = run_inf.run_id;
			run_inf.dbname = "ultrascan";
			run_inf.dbhost = "192.168.0.1";
			run_inf.dbdriver = "QMYSQL3";
			//run_inf.rotor = 0;
			for(int i=0; i<8; i++)
			{
				for(int j=0; j<4; j++)
				{
					run_inf.buffer_serialnumber[i][j] = -1;
					for(int k=0; k<3; k++)
					{
						run_inf.peptide_serialnumber[i][j][k] = -1;
						run_inf.DNA_serialnumber[i][j][k] = -1;
					}
				}
			}
		}
		else
		{
			run_inf.run_id = run_inf.data_dir.section('/',-2, -1);		//set run_id with file dir name
			if (f.open(IO_ReadOnly))
			{
				QTextStream ts(&f);
				if (!ts.atEnd())
				{
					ts >> run_inf.expdata_id;			// get experimental data id
					trashcan = ts.readLine();					//get rid of the rest of the line
					ts >> run_inf.investigator;		// get investigator info
					trashcan = ts.readLine();					//get rid of the rest of the line
					ts >> run_inf.date;					// get date info
					trashcan = ts.readLine();					//get rid of the rest of the line
					run_inf.description = ts.readLine();			// get description info
					ts >> run_inf.dbname;						// get dbname info
					trashcan = ts.readLine();				//get rid of the rest of the line
					ts >> run_inf.dbhost;						// get dbhost info
					trashcan = ts.readLine();				//get rid of the rest of the line
					ts >> run_inf.dbdriver;						// get dbdriver info
					trashcan = ts.readLine();				//get rid of the rest of the line
					ts >> run_inf.rotor;					// get rotor info
					trashcan = ts.readLine();					//get rid of the rest of the line
					update_rotor(run_inf.rotor);
					for(int i=0; i<8; i++)
					{
						if (run_inf.cell_id[i] != "")		//if this cell has data
						{
							ts >> run_inf.centerpiece[i];	// get centerpiece info
							trashcan = ts.readLine();		// get rid of the rest of line
							update_centerpiece(run_inf.centerpiece[i]);
							if ( strcmp( US_Version,  "7.0" ) < 0 )
							{
								for(int j=0; j<4; j++)
								{
									ts >> run_inf.DNA_serialnumber[i][j][0];		//get the number
									trashcan = ts.readLine();						//get rid of the rest of the line
									ts >> run_inf.buffer_serialnumber[i][j];
									trashcan = ts.readLine();
									ts >> run_inf.peptide_serialnumber[i][j][0];
									trashcan = ts.readLine();
								}
							}
							else
							{
								for(int j=0; j<4; j++)
								{
									ts >> run_inf.buffer_serialnumber[i][j];
									trashcan = ts.readLine();
									for(int k=0; k<3; k++)
									{
										ts >> run_inf.peptide_serialnumber[i][j][k];
										trashcan = ts.readLine();
										ts >> run_inf.DNA_serialnumber[i][j][k];
										trashcan = ts.readLine();
									}
								}
							}
						}
					}
				}
			}
		}

/***********************************************************************************************/
		lbl_instructions2->setText(tr("Please review the details of this run in the summary window\n"
															"and enter a descriptive name for your selected dataset (no spaces)\n"));
		details();
	}
	else
	{
		step = 0;
		lbl_instructions2->setText(tr("Please click on `Select Data Directory` and select a directory\n"
	                               "where your data is located. The data should be in a sub-\n"
	                               "directory of the Beckman Data Acquisition program in the format:\n"
	                               "/.../xlawin/data/mmddyy/hhmmss (month/day/year/hour/min/sec)"));
		lbl_directory->setText(tr("not selected"));
		lbl_run_id2->setText(tr("not selected"));
		lbl_current_cell2->setText(tr("not selected"));
		lbl_details->setText(tr("not selected"));
	}
	pb_accept->setEnabled(true);
	pb_details->setEnabled(true);
}

void EditData_Win::edit_all()
{
	int i;
	step = 2;
	count1=0;
	count2=0;
	dataset_counter=0;
	if (database)
	{
		lbl_instructions2->setText(tr("To review the first dataset of your run, click on:\n\n"
									         "REVIEW DATASET"));
	}
	else
	{
		lbl_instructions2->setText(tr("To edit the first dataset of your run, click on:\n\n"
									         "START EDITING"));
	}

/*
In order to find out which cells and wavelengths actually have data in them
we create a variable active_set[24] that contains the cell and wavelength count
of actual datasets. Cells are multiplied by 10, (0 -> 70), and wavelengths are
added as 0, 1 and 2. For example, active_set[3]=22 means that the fourth active set
is cell 3, wavelength 3. The variable "dataset_counter" will keep track of which
dataset we are working on.
NOTE: in most cases the full array size of 24 is not used. By assuming the
maximum possible array size, we avoid having to go through the following loop
twice to find out how many datasets there really are
*/

	for (i=0; i<8; i++)
	{
		if (run_inf.cell_id[i] != "")
		{
			count1 ++;
			lbl_run_id2->setText(run_inf.run_id);
			if (run_inf.scans[i][0] != 0)
			{
				active_set[count2] = (i)*10;
				count2 ++;
				if (run_inf.scans[i][1] != 0)
				{
					active_set[count2] = (i)*10 + 1;
					count2 ++;
					if (run_inf.scans[i][2] != 0)
					{
						active_set[count2] = (i)*10 + 2;
						count2 ++;
					}
				}
			}
		}
	}
	if (count2 == 1)
	{
		if(count1 == 1)
		{
			str.sprintf(tr("%d Cell, %d Dataset total"), count1, count2);
		}
		else
		{
			str.sprintf(tr("%d Cells, %d Dataset total"), count1, count2);
		}
	}
	else
	{
		if(count1 == 1)
		{
			str.sprintf(tr("%d Cell, %d Datasets total"), count1, count2);
		}
		else
		{
			str.sprintf(tr("%d Cells, %d Datasets total"), count1, count2);
		}
	}
	lbl_details->setText(str);
}

void EditData_Win::load_dataset()
{
	unsigned int i, j, k, scan, scan_counter=0;
	cmbb_centerpiece->setEnabled(true);
	cmbb_rotor->setEnabled(true);
	pb_editscan->setEnabled(true);
	pb_exsingle->setEnabled(true);
	pb_exrange->setEnabled(true);
	pb_invert->setEnabled(true);
	cnt_exsingle->setEnabled(true);
	cnt_exrange->setEnabled(true);
	pb_exclude_profile->setEnabled(true);
	if (edit_type == 3 || edit_type == 4)
	{
		pb_sub_baseline->setEnabled(true);
	}

	if (step == 0)
	{
		step_check();
		return;
	}
	else if (step == 1)
	{
		step_check();
		return;
	}

//
// Now we need to check if we have defined the last dataset. that condition is
// true if we have dataset_counter == count2, except for the case of multiple channels,
// where we also have to wait for the last channel:
//
	else if (((centerpiece.channels == 1) && (dataset_counter == count2))
           || ((centerpiece.channels > 1) && (dataset_counter == count2)
			  && (current_channel == centerpiece.channels - 1)))	// we're all done editing!
	{
		QString filename;
		filename = USglobal->config_list.result_dir.copy();
		filename.append("/");
		filename.append(run_inf.run_id);
		if (edit_type == 1 || edit_type == 3 || edit_type == 5)		//absorbance velocity = 1, interference velocity = 3
		{
/*
			if (edited_channel != "")
			{
				filename.append("-channel_" + edited_channel + ".us.v");
			}
			else
			{
				filename.append(".us.v");
			}
*/

			filename.append(".us.v");
			QFile f(filename);
			f.open( IO_WriteOnly );
			QDataStream ts(&f);
			ts << US_Version;
			ts << run_inf.data_dir;
			ts << run_inf.run_id;
			ts << run_inf.avg_temperature;
			ts << run_inf.temperature_check;
			ts << run_inf.time_correction;
			ts << run_inf.duration;
			ts << run_inf.total_scans;
			ts << run_inf.delta_r;
			ts << run_inf.expdata_id;
			ts << run_inf.investigator;
			ts << run_inf.date;
			ts << run_inf.description;
			ts << run_inf.dbname;
			ts << run_inf.dbhost;
			ts << run_inf.dbdriver;
			ts << (int) run_inf.exp_type.velocity;
			ts << (int) run_inf.exp_type.equilibrium;
			ts << (int) run_inf.exp_type.diffusion;
			ts << (int) run_inf.exp_type.simulation;
			ts << (int) run_inf.exp_type.interference;
			ts << (int) run_inf.exp_type.absorbance;
			ts << (int) run_inf.exp_type.fluorescence;
			ts << (int) run_inf.exp_type.intensity;
			ts << (int) run_inf.exp_type.wavelength;
			for (i=0; i<8; i++)
			{
				ts << run_inf.centerpiece[i];
				ts << run_inf.meniscus[i];
				ts << run_inf.cell_id[i];
				ts << run_inf.wavelength_count[i];
			}
			for (i=0; i<8; i++)
			{
				for (j=0; j<4; j++)
				{
					ts << run_inf.buffer_serialnumber[i][j];
					for(int k=0; k<3; k++)
					{
						ts << run_inf.peptide_serialnumber[i][j][k];
						ts << run_inf.DNA_serialnumber[i][j][k];
					}
				}
				for (j=0; j<3; j++)
				{
					ts << run_inf.wavelength[i][j];
					ts << run_inf.scans[i][j];
					ts << run_inf.baseline[i][j];
					if (run_inf.centerpiece[i] >= 0)
					{
						for (k=0; k<cp_list[run_inf.centerpiece[i]].channels; k++)
						{
							ts << run_inf.range_left[i][j][k];
							ts << run_inf.range_right[i][j][k];
							ts << run_inf.points[i][j][k];
							ts << run_inf.point_density[i][j][k];
						}
					}
				}
			}
			for (i=0; i<8; i++)
			{
				for (j=0; j<run_inf.wavelength_count[i]; j++)
				{
					for (k=0; k<run_inf.scans[i][j]; k++)
					{
						ts << run_inf.rpm[i][j][k];
						ts << run_inf.temperature[i][j][k];
						ts << run_inf.time[i][j][k];
						ts << run_inf.omega_s_t[i][j][k];
						ts << run_inf.plateau[i][j][k];
					}
				}
			}
			ts << rotor.serial_number;
			f.flush();
			f.close();
		}
		else if (edit_type == 2 || edit_type == 4)
		{
			write_equil_file();
			save_button = false;
			filename.append(".us.e");
			QFile f(filename);
			f.open( IO_WriteOnly );
			QDataStream ts(&f);
			ts << US_Version;
			ts << run_inf.data_dir ;
			ts << run_inf.run_id;
			ts << run_inf.duration;
			ts << run_inf.total_scans;
			ts << run_inf.delta_r;
			ts << run_inf.expdata_id;
			ts << run_inf.investigator;
			ts << run_inf.date;
			ts << run_inf.description;
			ts << run_inf.dbname;
			ts << run_inf.dbhost;
			ts << run_inf.dbdriver;
			ts << (int) run_inf.exp_type.velocity;
			ts << (int) run_inf.exp_type.equilibrium;
			ts << (int) run_inf.exp_type.diffusion;
			ts << (int) run_inf.exp_type.simulation;
			ts << (int) run_inf.exp_type.interference;
			ts << (int) run_inf.exp_type.absorbance;
			ts << (int) run_inf.exp_type.fluorescence;
			ts << (int) run_inf.exp_type.intensity;
			ts << (int) run_inf.exp_type.wavelength;
			for (i=0; i<8; i++)
			{
				ts << run_inf.centerpiece[i];
				ts << run_inf.cell_id[i];
				ts << run_inf.wavelength_count[i];
			}
			for (i=0; i<8; i++)
			{
				for (j=0; j<4; j++)	// one for each channel
				{
					ts << run_inf.buffer_serialnumber[i][j];
					for(int k=0; k<3; k++)
					{
						ts << run_inf.peptide_serialnumber[i][j][k];
						ts << run_inf.DNA_serialnumber[i][j][k];
					}
				}
				for (j=0; j<3; j++)
				{
					ts << run_inf.wavelength[i][j];
					ts << run_inf.scans[i][j];
					if (run_inf.centerpiece[i] >= 0 && run_inf.scans[i][j] != 0)
					{
						for (k=0; k<cp_list[run_inf.centerpiece[i]].channels; k++)
						{
							ts << run_inf.range_left[i][j][k];
							ts << run_inf.range_right[i][j][k];
							ts << run_inf.points[i][j][k];
							ts << run_inf.point_density[i][j][k];
//cout << "Point_dens: " << run_inf.point_density[i][j][k] << ", cell " << i+1 << ", wavelength " << j+1
//	  << ", channel: " << k+1 << endl;
						}
					}
				}
			}
			for (i=0; i<8; i++)
			{
				for (j=0; j<run_inf.wavelength_count[i]; j++)
				{
					for (k=0; k<run_inf.scans[i][j]; k++)
					{
						ts << run_inf.rpm[i][j][k];
						ts << run_inf.temperature[i][j][k];
						ts << run_inf.time[i][j][k];
						ts << run_inf.omega_s_t[i][j][k];
					}
				}
			}
			ts << rotor.serial_number;
			f.flush();
			f.close();
		}
		str = tr("Information for Run ");
		str.append(run_inf.run_id);
		str.append(":\n\n");
		str.append(tr("       Data editing complete\n\n"));
		str.append(tr("----------------------------------\n\n"));
      str.append(tr(" All data has been written to one\n"));
		str.append(tr("   binary file for each dataset"));
		str1 = tr("Information for ");
		str1.append(run_inf.run_id);
		QMessageBox::message(str1, str);
     // close();
		quitthis();
		return;
	}
	else if(save_button)
	{
		write_equil_file();
		save_button = false;
		return;
	}
	QFile f;
//
// what's our current cell?
// the extension of the filename has this in the second digit (of 2)
//
	cell = active_set[dataset_counter]/10;
//
// what's our current wavelength?
// the extension of the filename has this in the second digit (of 2)
//
	lambda = active_set[dataset_counter] - (cell * 10);
	dataset_counter++;	//get ready for the next data set (we need that counter)
	radius     = new double* [run_inf.scans[cell][lambda]];
	absorbance = new double* [run_inf.scans[cell][lambda]];
	points     = new unsigned int [run_inf.scans[cell][lambda]];
	temp_radius     = new double* [run_inf.scans[cell][lambda]];
	temp_absorbance = new double* [run_inf.scans[cell][lambda]];
	temp_points     = new unsigned int [run_inf.scans[cell][lambda]];
/*
	if (dataset_counter == 1)
	{
		radius     = new double* [run_inf.scans[cell][lambda]];
		absorbance = new double* [run_inf.scans[cell][lambda]];
		points     = new unsigned int [run_inf.scans[cell][lambda]];
		temp_radius     = new double* [run_inf.scans[cell][lambda]];
		temp_absorbance = new double* [run_inf.scans[cell][lambda]];
		temp_points     = new unsigned int [run_inf.scans[cell][lambda]];
	}
*/
	progress->setTotalSteps(run_inf.scans[cell][lambda]);
	progress->reset();
	str.sprintf(tr("Please wait...\n\nLoading Data for Dataset %d"), dataset_counter);
	lbl_instructions2->setText(str);
	str.sprintf(tr("Dataset %d"), dataset_counter);
	lbl_current_cell2->setText(str);

// start with the correct wavelength scan index
// (need to use +1 because this is our index for the file names.
	scan = lambda+1;
//
// Note: "scan" is a variable that serves as the scan number index for the files
// scan_counter counts the actual files. Those numbers are different by one for cases
// where there is a single wavlength, otherwise the difference is larger
//
	run_inf.centerpiece[cell] = current_centerpiece;	// we need to set the centerpiece for each cell at this point
//cout << "scan_counter: " << scan_counter << ", run_inf.scans[cell][lambda]: " << run_inf.scans[cell][lambda] << endl;
	while (scan_counter < run_inf.scans[cell][lambda])
	{
		str = "";
		radius[scan_counter]= new double [2048];
		absorbance[scan_counter]= new double [2048];
		temp_radius[scan_counter]= new double [2048];
		temp_absorbance[scan_counter]= new double [2048];
		if (extension == "FI" || extension == "fi")
		{
			if (scan<10)
			{
				str.sprintf(edited_channel + "0000%d." + extension + "%d", scan, cell+1);
			}
			else if (scan>9 && scan<100)
			{
				str.sprintf(edited_channel + "000%d." + extension + "%d", scan, cell+1);
			}
			else if (scan>99 && scan<1000)
			{
				str.sprintf(edited_channel + "00%d." + extension + "%d", scan, cell+1);
			}
			else if (scan>999 && scan<10000)
			{
				str.sprintf(edited_channel + "0%d." + extension + "%d", scan, cell+1);
			}
			else if (scan>9999 && scan<100000)
			{
				str.sprintf(edited_channel + "%d." + extension + "%d", scan, cell+1);
			}
		}
		else
		{
			if (scan<10)
			{
				str.sprintf("0000%d." + extension + "%d", scan, cell+1);
			}
			else if (scan>9 && scan<100)
			{
				str.sprintf("000%d." + extension + "%d", scan, cell+1);
			}
			else if (scan>99 && scan<1000)
			{
				str.sprintf("00%d." + extension + "%d", scan, cell+1);
			}
			else if (scan>999 && scan<10000)
			{
				str.sprintf("0%d." + extension + "%d", scan, cell+1);
			}
			else if (scan>9999 && scan<100000)
			{
				str.sprintf("%d." + extension + "%d", scan, cell+1);
			}
		}
		str.prepend(run_inf.data_dir);
		f.setName(str);
		if (f.open(IO_ReadOnly))
		{
			QTextStream ts(&f);
			if (!ts.eof())
			{
				for (i=0; i<2; i++)	// i is a row counter
				{
					ts.readLine();	// read the 2 header lines
				}
			}
			i=0;	// i is a row counter
			while (!ts.eof() && i<2048)	//load maximally 2048 points (limit of XLA)
			{
				ts >> radius[scan_counter][i];
				ts >> absorbance[scan_counter][i];
				temp_radius[scan_counter][i] = radius[scan_counter][i];
				temp_absorbance[scan_counter][i] = absorbance[scan_counter][i];
				if (edit_type == 1 || edit_type == 2 || edit_type == 5)	// get rid of third column (std.dev)
				{
					ts >> trashcan;
				}
				i++;
			}
			points[scan_counter] = i - 1;
			temp_points[scan_counter] = (unsigned int) i - 1;
			f.close();
			qApp->processEvents();
			progress->setProgress(scan_counter+1);
		}
//
// increment scan according to wavelength count, if more than
//	one wavelength is present, we need to skip scans. Also, we need to add one.
//
		scan += run_inf.wavelength_count[cell];
//
// scan_counter simply is incremented to denote the next scan (used as index to arrays
// radius, absorbance and points).
//
		scan_counter++;
	}
	if (edit_type == 1 || edit_type == 5) // absorbance velocity
	{
		str.sprintf(tr("First, confirm the Centerpiece and Rotor Settings below,\n"
						"then select the MENISCUS for dataset %d by clicking to the left\n"
						"and to the right of the MENISCUS. The radius at the maximal\n"
						"absorbance between those points is the position of the MENISCUS"), dataset_counter);
	}
	else if (edit_type == 2) //absorbance equilibrium
	{
		str.sprintf(tr("Please confirm the correct centerpiece setting.\n"
						"Next, identify the Meniscus region for dataset %d by clicking to the\n"
						"left of all menisci and then to the right of all menisci. The program\n"
						"will automatically identify the exact position for each scan's meniscus.\n"
						"First, click left, then right."), dataset_counter);
	}
	else if (edit_type == 3) // interference velocity
	{
		str.sprintf(tr("Please zoom the MENISCUS for dataset %d by dragging the mouse\n"
						"over the MENISCUS region. You can zoom repeatedly until the\n"
						"MENISCUS is sufficiently zoomed.\n"
						"Then turn zooming off by clicking on the \"Turn Zoom off\" button.\n"), dataset_counter);
		edit_plt->setOutlineStyle(Qwt::Rect);
		zoomflag = true;
		pb_zoom = new QPushButton(tr("Turn Zoom Off"), this);
		Q_CHECK_PTR(pb_zoom);
		pb_zoom->setAutoDefault(false);
		pb_zoom->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
		pb_zoom->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
		pb_zoom->setGeometry(5, 5, 110, 26);
		pb_zoom->show();
		connect(pb_zoom, SIGNAL(clicked()), SLOT(set_zoom()));
	}
	else if (edit_type == 4) // interference equilibrium
	{
		str = tr("Please confirm the correct rotor and centerpiece setting.\n"
				"Next, click to the left of the meniscus region for channel 1.\n"
				"The program will then allow you to define the meniscus and the\n"
				"data range for each scan.");

	}
	lbl_instructions2->setText(str);
	pb_accept->setEnabled(false);
	plot_dataset();
	step=3;
}

void EditData_Win::cleanup_dataset()
{
	unsigned int i;
	for (i=0; i<run_inf.scans[cell][lambda]; i++)
	{
		delete [] radius[i];
		delete [] absorbance[i];
		delete [] temp_radius[i];
		delete [] temp_absorbance[i];
	}
	delete [] radius;
	delete [] absorbance;
	delete [] points;
	delete [] temp_radius;
	delete [] temp_absorbance;
	delete [] temp_points;
}

void EditData_Win::plot_dataset()
{
	QString header;
	unsigned int i;
	double *temp_max, *temp_min, dummy;
	cnt_exrange->setRange(0, run_inf.scans[cell][lambda], 1);
	cnt_exsingle->setRange(0, run_inf.scans[cell][lambda], 1);
	cnt_edit->setRange(0, run_inf.scans[cell][lambda], 1);
	cnt_exrange->setValue(0);
	cnt_exsingle->setValue(0);
	if (step < 3)
	{
		cnt_edit->setValue(0);
	}
	if (edit_type == 1 || edit_type == 3 || edit_type == 5)
	{
		header = tr("Velocity Data for ");
	}
	else if (edit_type == 2 || edit_type == 4)
	{
		header = tr("Equilibrium Data for ");
	}
	header += run_inf.run_id + ": " + run_inf.cell_id[cell];
	edit_plt->setTitle(header);
	if (edit_type == 1 || edit_type == 2)
	{
		str.sprintf(tr("Absorbance at %d nm"), run_inf.wavelength[cell][lambda]);
	}
	if (edit_type == 3 || edit_type == 4)
	{
//		str.sprintf("Fringes", run_inf.wavelength[cell][lambda]);
		str.sprintf(tr("Fringes"));
	}
	if (edit_type == 5 || edit_type == 6)
	{
		str.sprintf(tr("Fluorescence Intensity"));
	}
	edit_plt->setAxisTitle(QwtPlot::yLeft, str);

	temp_max = new double [run_inf.scans[cell][lambda]];
	temp_min = new double [run_inf.scans[cell][lambda]];

//
// find the absorbance maximum and minimum in each scan and collect them in
// temp_max and temp_min:
//

	for (i=0; i<run_inf.scans[cell][lambda]; i++)
	{
		get_1d_limits(&absorbance[i], &temp_min[i], &temp_max[i], points[i]);
	}
	get_1d_limits(&temp_max, &dummy, &ymax, run_inf.scans[cell][lambda]);
	get_1d_limits(&temp_min, &ymin, &dummy, run_inf.scans[cell][lambda]);
	ymax = ymax + (ymax - ymin)/5;
	for (i=0; i<run_inf.scans[cell][lambda]; i++)
	{
		temp_max[i] = radius[i][points[i]-1];
	}
	get_1d_limits(&temp_max, &dummy, &xmax, run_inf.scans[cell][lambda]);

	xmin = radius[0][0];
	delete [] curve;
	curve = new unsigned int [run_inf.scans[cell][lambda]];
	edit_plt->clear();
	for (i=0; i<run_inf.scans[cell][lambda]; i++)
	{
		curve[i] = edit_plt->insertCurve("Optical Density");
		edit_plt->setCurvePen(curve[i], yellow);
		edit_plt->setCurveData(curve[i], radius[i], absorbance[i], points[i]);
	}
	edit_plt->setAxisScale(QwtPlot::yLeft, ymin, ymax, 0);
	edit_plt->setAxisScale(QwtPlot::xBottom, xmin, xmax, 0);
	edit_plt->replot();

	delete [] temp_max;
	delete [] temp_min;
}

void EditData_Win::step_check()
{
	switch (step)
	{
		case 0:
		{
			QMessageBox::message(tr("Attention:"),tr("Please select a Data Directory First\n"
	                           "(Hint: read the instructions)"));
			break;
		}
		case 1:
		{
			QMessageBox::message(tr("Attention:"),tr("Please pick a Run Identification before\n"
	                           "proceeding\n(Hint: read the instructions)"));
			break;
		}
		case 2:
		{
			break;
		}
		case 3:
		{
			break;
		}
		case 4:
		{
			break;
		}
	}
}

void EditData_Win::newdir(const QString &temp_dir)
{
	run_inf.data_dir = temp_dir;
	lbl_directory->setText(run_inf.data_dir);
}

void EditData_Win::help()
{
}

void EditData_Win::write_equil_file()
{
	unsigned int i, j, scan;
	float rad, first_point;
	double m, b, newpoint = 0;
	unsigned int *count;	 // write all scans from this dataset to file
	QString filename;
	run_inf.centerpiece[cell] = centerpiece.serial_number;
	filename = USglobal->config_list.result_dir.copy();
	filename.append("/");
	filename.append(run_inf.run_id);
	run_inf.delta_r = (float) 0.001;
//cout << "cell: " << cell << ", channel: " << current_channel << endl;
	str.sprintf(".equil.%d%d%d", cell+1, lambda+1, current_channel+1);
	filename.append(str);
	QFile f(filename);
	if (!f.open( IO_WriteOnly ))
	{
		QString s1 = "There was an error opening the output file:\n\n"
					  + filename
					  + "\n\nPlease check to make sure that your Result\n"
					  + "Directory is writable, then try again...";
		QMessageBox::message(tr("Attention:"),tr(s1));
		close();
		return;
	}
	QDataStream s( &f );
	count = new unsigned int [run_inf.scans[cell][lambda]];
	for (scan=0; scan<run_inf.scans[cell][lambda]; scan++)
	{
		count[scan]=0;	// keeps track of the datapoint index in each scan
	}
	if (edit_type == 4)
	{
//just check the delta_r for the first scan, the others will be the same
		run_inf.delta_r = (radius[0][points[0]-1] - radius[0][0])/points[0];
//cout << "index1: " << index1 << ", index2: " << index2 << ", points: " << run_inf.points[cell][lambda][current_channel] << ", delta_r: " << run_inf.delta_r << endl;
	}
	else
	{
// for absorbance scans, the left and right range is the same for all scans
// delta_r is fixed, therefore the number of points is the same for all scans as well,
// but we still have to enter them individually for each scan:
		run_inf.points[cell][lambda][current_channel] = (unsigned int)
		((run_inf.range_right[cell][lambda][current_channel] -
		 run_inf.range_left[cell][lambda][current_channel])/run_inf.delta_r) + 1;
	}
	for (scan=0; scan<run_inf.scans[cell][lambda]; scan++)
	{
		s << run_inf.equil_meniscus[scan];	// the first point in each dataset is always the meniscus
		if (edit_type == 4) // for interference the point spacing is regular and doesn't need to be regularized
		{
// needed for interference where we set first point arbitrarily to zero:
			first_point = (float) absorbance[scan][index1];
			unsigned int tmp_1 = (unsigned int) index1;
			unsigned int tmp_2 = (unsigned int) index2;
			for (i=tmp_1; i<=tmp_2; i++)
			{
				s << (float) (absorbance[scan][i] - first_point); //set first point to zero
			}
		}
		else
		{
			for (i=0; i<run_inf.points[cell][lambda][current_channel]; i++)
			{
//	rad is the radius for the current point - equally spaced 0.001 cm:
				rad = run_inf.range_left[cell][lambda][current_channel] + i * run_inf.delta_r;
// only compare the first 3 significant digits
// Sometimes there are increments smaller than 0.001, so we need to skip those
				while ((unsigned int) (radius[scan][count[scan]] * 1000) < (unsigned int) (rad * 1000))
				{
					count[scan] ++;
				}
				if ((unsigned int) (radius[scan][count[scan]] * 1000) == (unsigned int) (rad * 1000))
				{
					s << (float) (absorbance[scan][count[scan]]);
					count[scan] ++;
				}
				else
				{
// if we are at the beginning of the scan, and the first datapoints is past "rad", fill in with
// the first data point's value:
					if (count[scan] == 0)
					{
						newpoint = absorbance[scan][0];
					}
// if we are in the middle of the scan, we linearly interpolate missing datapoints (newpoint = m * rad + b)
// make sure that we are not outside bounds
					else if ((count[scan] < points[scan] - 1) && (count[scan] > 0))
					{
						m = (absorbance[scan][count[scan]] - absorbance[scan][count[scan]-1]) /
					 	    (radius[scan][count[scan]] - radius[scan][count[scan]-1]);
						b = absorbance[scan][count[scan]] - m * radius[scan][count[scan]];
						newpoint = m * rad + b;
					}
					else if (count[scan] == points[scan] - 1)
					{
						newpoint = absorbance[scan][points[scan] - 1]; // we are at the end of the scan
					}
//cout << "Scan: " << scan << ", Count: " << count[scan] << ", points: " << points[scan] << ", newpoint: " << newpoint << endl;
					s << (float) newpoint;
				}
			}
		}
	}
	run_inf.equil_meniscus.clear(); // clear this variable so it can be filled again in next dataset
	f.flush();
	f.close();
//	qDebug(tr("Centerpiece channels: %d"), centerpiece.channels);
	if (centerpiece.channels > 1)	// we have a multi-channel centerpiece & need to go on to the next channel
	{
		current_channel ++;	// we just wrote the previous channel to disk, so increment the channel
		if (current_channel < centerpiece.channels ) // there are more channels in this cell to process, so replot the data so the user can pick the next channel
		{
			if (current_channel == 1)
			{
				str.sprintf(tr("Please select the menisci for dataset %d by clicking to the\n"
								"left and to the right of all menisci of channel 2\n"
								"(from the left). First, click left."), dataset_counter);
				lbl_instructions2->setText(str);
			}
			else if (current_channel == 2)
			{
				str.sprintf(tr("Please select the menisci for dataset %d by clicking to the\n"
								"left and to the right of all menisci of channel 3\n"
								"(from the left). First, click left."), dataset_counter);
				lbl_instructions2->setText(str);
			}
			else if (current_channel == 3)
			{
				str.sprintf(tr("Please select the menisci for dataset %d by clicking to the\n"
								"left and to the right of all menisci of channel 4\n"
								"(from the left). First, click left."), dataset_counter);
				lbl_instructions2->setText(str);
			}
//
// if we have multiple channels and we aren't working on the last channel, we have to
// replot the entire cell to allow the user to pick the next channel:
//
			for (i=0; i<run_inf.scans[cell][lambda]; i++)
			{
				points[i] = temp_points[i];
				for (j=0; j<temp_points[i]; j++)
				{
					absorbance[i][j] = temp_absorbance[i][j];
					radius[i][j]	  = temp_radius[i][j];
				}
			}
			edit_plt->clear();
			plot_dataset(); // no cleanup_dataset call needed since we reuse the existing arrays for the other channels
			return;
		}
		else if (dataset_counter != count2) 	// no more channels in this cell, so if we aren't working on
		{													//the last dataset, load the next dataset and set channel to zero
			save_button = false;
			current_channel = 0;
			cleanup_dataset();
			load_dataset();
			return;
		}
	}
//
// if we have a 2-channel centerpiece, we need to load the next dataset, but only if we aren't
// working already on the last dataset:
//
	else if (dataset_counter != count2)
	{
		save_button = false;
		current_channel = 0;
		cleanup_dataset();
		load_dataset();
		return;
	}
}

void EditData_Win::exclude_profile()
{
	US_ExcludeProfile *exclude;
	exclude = new US_ExcludeProfile(run_inf.scans[cell][lambda]);
	exclude->show();
	connect(exclude, SIGNAL(update_exclude_profile(vector <bool>)), this, SLOT(update_exclude_profile(vector <bool>)));
	connect(exclude, SIGNAL(final_exclude_profile(vector <bool>)), this, SLOT(final_exclude_profile(vector <bool>)));
}

void EditData_Win::update_exclude_profile(vector <bool> include_flag)
{
	unsigned int i;
	edit_plt->clear();
	for (i=0; i<run_inf.scans[cell][lambda]; i++)
	{
		curve[i] = edit_plt->insertCurve("Optical Density");
		if (include_flag[i])
		{
			edit_plt->setCurvePen(curve[i], yellow);
		}
		else
		{
			edit_plt->setCurvePen(curve[i], red);
		}
		edit_plt->setCurveData(curve[i], radius[i], absorbance[i], points[i]);
	}
	edit_plt->setAxisScale(QwtPlot::yLeft, ymin, ymax, 0);
	edit_plt->setAxisScale(QwtPlot::xBottom, xmin, xmax, 0);
	edit_plt->replot();
}

void EditData_Win::final_exclude_profile(vector <bool> include_flag)
{
	unsigned int i, count=0;
	for (i=0; i<include_flag.size(); i++)
	{
		/*
		if(!include_flag[i])
		{
			if (edit_type == 1)
			{
				oldscan.erase(oldscan.begin() + i-1);
			}
		}
		*/
		if(include_flag[i])
		{
			radius[count] 										= radius[i];
			absorbance[count]									= absorbance[i];
			points[count]										= points[i];
			run_inf.time[cell][lambda][count] 			= run_inf.time[cell][lambda][i];
			run_inf.plateau[cell][lambda][count]		= run_inf.plateau[cell][lambda][i];
			run_inf.temperature[cell][lambda][count]	= run_inf.temperature[cell][lambda][i];
			run_inf.omega_s_t[cell][lambda][count]		= run_inf.omega_s_t[cell][lambda][i];
			run_inf.rpm[cell][lambda][count]				= run_inf.rpm[cell][lambda][i];
			count++;
		}
	}
	run_inf.scans[cell][lambda] = count;
	run_inf.total_scans -= (include_flag.size() - count);
	plot_dataset();
	cnt_edit->setValue(0);
	cnt_exrange->setValue(0);
	cnt_exsingle->setValue(0);
}

//Virtual Member Functions below:

void EditData_Win::sub_baseline()
{
}

void EditData_Win::get_x(const QMouseEvent &)
{
}

void EditData_Win::quitthis()
{
	close();
}
