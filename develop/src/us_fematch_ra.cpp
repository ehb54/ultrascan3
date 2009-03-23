#include "../include/us_fematch_ra.h"

US_FeMatchRa_W::US_FeMatchRa_W(QWidget *p, const char *name) : Data_Control_W(13, p, name)
{
	setCaption(tr("Finite Element Analysis:"));
	band_volume = (float) 0.015;
	mesh = 0;
	moving_grid = 1;
	simpoints = 200;
	rmsd = 0.0;
	pm = new US_Pixmap();
	connect(this, SIGNAL(dataLoaded()), this, SLOT(enableButtons()));
	connect(this, SIGNAL(datasetChanged()), this, SLOT(clearDisplay()));
	resplot = NULL;
	stopFlag = false;
	movieFlag = false;
	astfem_rsa = new US_Astfem_RSA(false);
	component_dialog = NULL;
	ga_param.clear();
	msv.clear();
	current_parameter = 0;
	current_model = 0;
	plotmode = 0;
	setup_GUI();
}

US_FeMatchRa_W::~US_FeMatchRa_W()
{
	delete resplot;
}

void US_FeMatchRa_W::clearDisplay()
{
	if (resplot != NULL)
	{
		resplot->close();
		resplot = NULL;
	}
	analysis_plot->clear();
	analysis_plot->replot();
}

void US_FeMatchRa_W::setup_GUI()
{
	if (resplot == NULL)
	{
		resplot = new US_ResidualPlot(0, 0);
	}
	/*
	pb_second_plot->setText(tr("Plot Simulation"));
	pb_second_plot->setEnabled(false);
	pb_second_plot->disconnect();
	*/
	delete pb_second_plot;
	delete pb_reset;
	delete smoothing_lbl;
	delete smoothing_counter;
	smoothing_counter = NULL;
	delete range_lbl;
	delete range_counter;
	range_counter = NULL;
	delete position_lbl;
	delete position_counter;
	delete pb_vbar;
	delete vbar_le;
	position_counter = NULL;

	lbl1_excluded->setText(tr("RMSD:"));

	pb_loadModel = new QPushButton(tr("Load Model"), this);
	Q_CHECK_PTR(pb_loadModel);
	pb_loadModel->setAutoDefault(false);
	pb_loadModel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_loadModel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_loadModel->setEnabled(false);
	connect(pb_loadModel, SIGNAL(clicked()), SLOT(load_model()));

	lbl_variance = new QLabel(tr(" Variance:"),this);
	lbl_variance->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
	lbl_variance->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_variance2 = new QLabel(this);
	lbl_variance2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_variance2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	lbl_variance2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
	lbl_variance2->setText("0.0");
	lbl_variance2->setAlignment(AlignCenter|AlignVCenter);

	pb_model = new QPushButton(tr("Show Model #"), this);
	Q_CHECK_PTR(pb_model);
	pb_model->setAutoDefault(false);
	pb_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_model->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_model->setEnabled(false);
	connect(pb_model, SIGNAL(clicked()), SLOT(show_model()));

	cnt_model= new QwtCounter(this);
	cnt_model->setNumButtons(3);
	cnt_model->setRange(0, 0, 0);
	cnt_model->setValue(current_model);
	cnt_model->setEnabled(false);
	cnt_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_model->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_model, SIGNAL(valueChanged(double)), SLOT(update_model(double)));

	lbl_parameter = new QLabel(tr(" Parameter:"),this);
	lbl_parameter->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
	lbl_parameter->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_parameter= new QwtCounter(this);
	cnt_parameter->setNumButtons(3);
	cnt_parameter->setRange(0, 0, 0);
	cnt_parameter->setEnabled(false);
	cnt_parameter->setValue(current_parameter);
	cnt_parameter->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_parameter->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_parameter, SIGNAL(valueChanged(double)), SLOT(update_parameter(double)));

	lbl_bandVolume = new QLabel(tr(" Band-loading Volume:"), this);
	lbl_bandVolume->setAlignment(AlignLeft|AlignVCenter);
	lbl_bandVolume->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_bandVolume->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));

	lbl_simpoints = new QLabel(tr(" Simulation Points:"), this);
	lbl_simpoints->setAlignment(AlignLeft|AlignVCenter);
	lbl_simpoints->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_simpoints->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));

	cmb_radialGrid = new QComboBox(false, this, "Radial Grid" );
	cmb_radialGrid->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cmb_radialGrid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cmb_radialGrid->setSizeLimit(5);
	cmb_radialGrid->insertItem("Adaptive Space Mesh (ASTFEM)", -1);
	cmb_radialGrid->insertItem("Claverie Fixed Mesh", -1);
	cmb_radialGrid->insertItem("Moving Hat Mesh", -1);
	cmb_radialGrid->insertItem("File: \"$ULTRASCAN/mesh.dat\"", -1);
	cmb_radialGrid->setCurrentItem(mesh);
	connect(cmb_radialGrid, SIGNAL(activated(int)), this, SLOT(update_radialGrid(int)));

	cmb_timeGrid = new QComboBox(false, this, "Time Grid" );
	cmb_timeGrid->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cmb_timeGrid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cmb_timeGrid->setSizeLimit(5);
	cmb_timeGrid->insertItem("Constant Time Grid", -1);
	cmb_timeGrid->insertItem("Adaptive Time Grid", -1);
	cmb_timeGrid->setCurrentItem(moving_grid);
	connect(cmb_timeGrid, SIGNAL(activated(int)), this, SLOT(update_timeGrid(int)));

	cnt_simpoints= new QwtCounter(this);
	cnt_simpoints->setNumButtons(3);
	cnt_simpoints->setRange(50, 5000, 10);
	cnt_simpoints->setValue(simpoints);
	cnt_simpoints->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_simpoints->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_simpoints, SIGNAL(valueChanged(double)), SLOT(update_simpoints(double)));

	cnt_band_volume = new QwtCounter(this);
	cnt_band_volume->setRange(0.001, 0.1, 0.0001);
	cnt_band_volume->setNumButtons(3);
	cnt_band_volume->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_band_volume->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_band_volume->setValue(band_volume);
	connect(cnt_band_volume, SIGNAL(valueChanged(double)), SLOT(update_band_volume(double)));

	bg_plotmode = new QButtonGroup(4, Qt::Horizontal, "Simulate data using parameters from model or from Monte Carlo statistics:", this);
	bg_plotmode->setExclusive(true);
	bg_plotmode->setEnabled(false);
	connect(bg_plotmode, SIGNAL(clicked(int)), this, SLOT(select_plotmode(int)));

	plotmode = 0;
	cb_current_model = new QCheckBox(tr(" Current Model"), bg_plotmode);
	cb_current_model->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_current_model->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cb_mode = new QCheckBox(tr(" Mode"), bg_plotmode);
	cb_mode->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_mode->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cb_mean = new QCheckBox(tr(" Mean"), bg_plotmode);
	cb_mean->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_mean->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cb_median = new QCheckBox(tr(" Median"), bg_plotmode);
	cb_median->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_median->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	bg_plotmode->setButton(0);

	rmsd = 0.0;
	QString str;
	str.sprintf("%3.1f", rmsd);
	lbl_variance2->setText(str);
	lbl2_excluded->setText(str);

	int j=0;
	int rows = 15, columns = 4, spacing = 2;

	QGridLayout * background = new QGridLayout(this, 2, 2,spacing);
	QGridLayout * subGrid1 = new QGridLayout(rows, columns, spacing);
	subGrid1->addMultiCellWidget(pb_load, j, j, 0, 1);
	subGrid1->addMultiCellWidget(pb_details, j, j, 2, 3);
	j++;
	//subGrid1->addMultiCellWidget(pb_second_plot, j, j, 0, 1);
	subGrid1->addMultiCellWidget(pb_loadModel, j, j, 0, 1);
	subGrid1->addMultiCellWidget(pb_save, j, j, 2, 3);
	j++;
	subGrid1->addMultiCellWidget(pb_print, j, j, 0, 1);
	subGrid1->addMultiCellWidget(pb_view, j, j, 2, 3);
	j++;
	subGrid1->addMultiCellWidget(pb_help, j, j, 0, 1);
	subGrid1->addMultiCellWidget(pb_close, j, j, 2, 3);
	j++;
	subGrid1->addMultiCellWidget(banner1, j, j, 0, 3);
	j++;
	subGrid1->addMultiCellWidget(lbl_run_id1, j, j, 0, 1);
	subGrid1->addMultiCellWidget(lbl_run_id2, j, j, 2, 3);
	j++;
	subGrid1->addMultiCellWidget(lbl_temperature1, j, j, 0, 1);
	subGrid1->addMultiCellWidget(lbl_temperature2, j, j, 2, 3);
	j++;
	subGrid1->addMultiCellWidget(lbl_cell_info1, j, j, 0, 1);
	subGrid1->addMultiCellWidget(lbl_cell_info2, j, j, 2, 3);
	j++;
	subGrid1->addMultiCellWidget(lbl_cell_descr, j, j, 0, 3);
	j++;
	subGrid1->addMultiCellWidget(cell_select, j, j+2, 0, 1);
	subGrid1->addMultiCellWidget(lambda_select, j, j+2, 2, 3);
	j=j+3;
	subGrid1->addMultiCellWidget(banner2, j, j, 0, 3);
	j++;
	subGrid1->addWidget(pb_density, j, 0);
	subGrid1->addWidget(density_le, j, 1);
	subGrid1->addWidget(pb_viscosity, j, 2);
	subGrid1->addWidget(viscosity_le, j, 3);
	j++;
	subGrid1->addMultiCellWidget(lbl_simpoints, j, j, 0, 1);
	subGrid1->addMultiCellWidget(cnt_simpoints, j, j, 2, 3);
	j++;
	subGrid1->addMultiCellWidget(lbl_bandVolume, j, j, 0, 1);
	subGrid1->addMultiCellWidget(cnt_band_volume, j, j, 2, 3);
	j++;
	subGrid1->addMultiCellWidget(lbl_parameter, j, j, 0, 1);
	subGrid1->addMultiCellWidget(cnt_parameter, j, j, 2, 3);
	j++;
	subGrid1->addMultiCellWidget(pb_model, j, j, 0, 1);
	subGrid1->addMultiCellWidget(cnt_model, j, j, 2, 3);
	j++;
	subGrid1->addMultiCellWidget(cmb_radialGrid, j, j, 0, 3);
	j++;
	subGrid1->addMultiCellWidget(cmb_timeGrid, j, j, 0, 3);
	j++;
	subGrid1->addWidget(lbl_variance, j, 0);
	subGrid1->addWidget(lbl_variance2, j, 1);
	subGrid1->addWidget(lbl1_excluded, j, 2);
	subGrid1->addWidget(lbl2_excluded, j, 3);
	j++;
	subGrid1->addMultiCellWidget(pb_exsingle, j, j, 0, 1);
	subGrid1->addMultiCellWidget(cnt_exsingle, j, j, 2, 3);
	j++;
	subGrid1->addMultiCellWidget(pb_exrange, j, j, 0, 1);
	subGrid1->addMultiCellWidget(cnt_exrange, j, j, 2, 3);
	j++;
	subGrid1->addWidget(lbl_status, j, 0);
	subGrid1->addMultiCellWidget(progress, j, j, 1, 3);
	for (int i=0; i<j; i++)
	{
		subGrid1->setRowSpacing(i, 26);
	}

	background->addMultiCellLayout(subGrid1, 0, 3, 0, 0);
	background->addWidget(analysis_plot, 0, 1);
	background->addWidget(bg_plotmode, 2, 1);
	background->addWidget(edit_plot, 3, 1);
	background->setColStretch(0, 1);
	background->setColStretch(1, 3);
	background->setColSpacing(0, 350);
	background->setColSpacing(1, 550);

	qApp->processEvents();
	QRect r = background->cellGeometry(0, 0);

	global_Xpos += 30;
	global_Ypos += 30;

	this->setGeometry(global_Xpos, global_Ypos, r.width()+550+spacing*3, this->height());
}

void US_FeMatchRa_W::enableButtons()
{
	pb_model->setEnabled(false);
	pb_save->setEnabled(false);
	pb_view->setEnabled(false);
	pb_print->setEnabled(false);
	pb_loadModel->setEnabled(true);
}

void US_FeMatchRa_W::save()
{
	if (step == 0)
	{
		QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n	   Click on \"Load Data\"\n"));
		return;
	}
	QString filename, str, mc_str;
	vector <double> val;
	str.sprintf("-%d.%d%d", msv[0].model, selected_cell+1, selected_lambda+1);
	filename = USglobal->config_list.result_dir + "/" + run_inf.run_id + ".ga_sc_res" + str;
	QFile res_f(filename);
	if (res_f.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&res_f);
		ts <<    "****************************************************\n";
		ts << tr("* Nonlinear Model Fit - Genetic Algorithm analysis *\n");
		ts << "****************************************************\n\n\n";
		ts << tr("Data Report for Run \"") << run_inf.run_id << tr("\", Cell ") << (selected_cell + 1)
			<< tr(", Wavelength ") << (selected_lambda + 1) << "\n\n";
		ts << tr("Detailed Run Information:\n\n");
		ts << tr("Cell Description:        ") << run_inf.cell_id[selected_cell] << "\n";
		ts << tr("Raw Data Directory:      ") << run_inf.data_dir << "\n";
		ts << tr("Rotor Speed:             ") << run_inf.rpm[first_cell][0][0]  << " rpm\n";
		ts << tr("Average Temperature:     ") << run_inf.avg_temperature << " ºC\n" ;
		if (run_inf.temperature_check == 1)
		{
			ts << tr("Temperature Variation:   Within Tolerance\n");
		}
		if (run_inf.temperature_check == -1)
		{
			ts << tr("Temperature Variation: (!) OUTSIDE TOLERANCE (!)\n");
		}
		i = (unsigned int) (run_inf.time_correction / 60);
		j = (unsigned int) (0.5 + (run_inf.time_correction - (i * 60)));
		ts << tr("Time Correction:         ") << i << tr(" minute(s) ") << j << tr(" second(s)\n");
		i = (unsigned int) (run_inf.duration / 3600);
		j = (unsigned int) (0.5 + (run_inf.duration - i * 3600) / 60);
		ts << tr("Run Duration:            ") << i << tr(" hour(s) ") << j << tr(" minute(s)\n");
		ts << tr("Wavelength:              ") << run_inf.wavelength[selected_cell][selected_lambda] << " nm\n";
		ts << tr("Baseline Absorbance:     ") << run_inf.baseline[selected_cell][selected_lambda] << " OD\n";
		ts << tr("Meniscus Position:       ") << run_inf.meniscus[selected_cell] << " cm\n";
		ts << tr("Edited Data starts at:   ") << run_inf.range_left[selected_cell][selected_lambda][0] << " cm\n";
		ts << tr("Edited Data stops at:    ") << run_inf.range_right[selected_cell][selected_lambda][0] << " cm\n\n\n";
		ts << tr("Hydrodynamic Settings:\n\n");
		ts << tr("Viscosity correction:    ") << viscosity << "\n" ;
		ts << tr("Viscosity (absolute):    ") << viscosity_tb << "\n" ;
		ts << tr("Density correction:      ") << density << " g/ccm\n" ;
		ts << tr("Density (absolute):      ") << density_tb << " g/ccm\n" ;
		ts << tr("Vbar:                    ") << vbar << " ccm/g\n" ;
		ts << tr("Vbar corrected for 20ºC: ") << vbar20 << " ccm/g\n" ;
		ts << tr("Buoyancy (Water, 20ºC) : ") << buoyancyw << "\n" ;
		ts << tr("Buoyancy (absolute)      ") << buoyancyb << "\n" ;
		ts << tr("Correction Factor:       ") << correction << "\n\n";
		ts << tr("Fitted Model: ") << modelString[msv[0].model] << " (" << msv[0].description << ")\n\n";
		ts << tr("Parameter Information:\n\n");
		US_GA_Initialize *gai;
		gai = new US_GA_Initialize();
		MonteCarloStats stats;
		list <Parameter>::iterator pit1;
		if (msv.size() > 1) // then we have a Monte Carlo distribution
		{
			ts << tr("Monte Carlo Iterations: ") << msv.size() << "\n\n";
			ts << tr("Monte Carlo Statistics:\n");
			mc_str = "";
			for (i=0; i<ga_param.size(); i++)
			{
				val.clear();
				for (pit1 = ga_param[i].parameter_list.begin(); pit1 != ga_param[i].parameter_list.end(); pit1++)
				{
					for (unsigned int k=0; k<(*pit1).y; k++)
					{
						val.push_back((*pit1).x); // each value must have a weight of unity. If the y value is higher than 1, add this value multiple times.
					}
				}
				bool mc_flag = false;
				double test_val = val[0];
				for (j=1; j<val.size(); j++) // check if there is a variation in the parameters over all distros, if so, call MC stats
				{
					if (test_val != val[j])
					{
						mc_flag = true;
					}
				}

				if (mc_flag)
				{
					mc_str += gai->calc_stats(&stats, val, ga_param[i].name);
					str.sprintf(tr(ga_param[i].name + ":\t %6.4e (%6.4e, %6.4e)\n"), stats.mean, stats.conf95low, stats.conf95high);
					ts << str;
				}
				else
				{
					str.sprintf(tr(ga_param[i].name + ":\t %6.4e \n"), val[0]); // all values are the same for this parameter
					ts << str;
				}
			}
			ts << mc_str;
		}
		else
		{
			for (i=0; i<ga_param.size(); i++)
			{
				pit1 = ga_param[i].parameter_list.begin();
				ts << ga_param[i].name << ": " << (*pit1).x << endl;
			}
		}
		ts << "\n\n";
		delete gai;
		ts << tr("Scan Information: \n\n");
		ts << tr("Scan:     Corrected Time:  Plateau Concentration: \n\n");
		for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
		{
			if (i+1<10)
			{
				ts << "  " << (i+1) << ":    ";
			}
			else if (i+1 < 100 && i+1 > 9)
			{
				ts << " " << (i+1) << ":    ";
			}
			else if (i+1 > 99)
			{
				ts << (i+1) << ":    ";
			}
			k = (unsigned int) (run_inf.time[selected_cell][selected_lambda][i]/ 60);
			j = (unsigned int) (0.5 + (run_inf.time[selected_cell][selected_lambda][i] - (k * 60)));
			ts.width(5);
			ts << k << tr(" min");
			ts.width(3);
			ts << j << tr(" sec     ");
			ts.width(11);
			ts << run_inf.plateau[selected_cell][selected_lambda][i] << " OD\n";
		}
		res_f.close();
	}
}

void US_FeMatchRa_W::second_plot() // currently disabled
{
	fit(); // simply re-simulate the data according to current settings
}

void US_FeMatchRa_W::help()
{
	US_Help *online_help;
	online_help = new US_Help(this);
	online_help->show_help("manual/fematch_ra.html");
}

void US_FeMatchRa_W::view()
{
	save();
	if (step == 0)
	{
		return;
	}
	QString filename, str;
	str.sprintf("-%d.%d%d", msv[0].model, selected_cell+1, selected_lambda+1);
	filename = USglobal->config_list.result_dir + "/" + run_inf.run_id + ".ga_sc_res" + str;
	TextEdit *e;
	e = new TextEdit();
	e->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	e->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
	e->load(filename);
	e->show();
}

void US_FeMatchRa_W::write_res()
{
}

float US_FeMatchRa_W::fit()
{
	QString str;
	unsigned int i, j;
	long *s_curve, *r_curve;
	double **sim, **res;
	struct mfem_scan single_scan;

	sim = new double * [run_inf.scans[selected_cell][selected_lambda]];
	res = new double * [run_inf.scans[selected_cell][selected_lambda]];
	s_curve = new long   [run_inf.scans[selected_cell][selected_lambda]];
	r_curve = new long   [run_inf.scans[selected_cell][selected_lambda]];
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		sim[i] = new double [points];
		res[i] = new double [points];
	}
	simdata.clear();
	simdata.resize(1);
	single_scan.conc.clear();
	for (i=0; i<points; i++)
	{
		simdata[0].radius.push_back(radius[i]);
		single_scan.conc.push_back(0.0); // populate with zeros
	}
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		single_scan.time = (double) run_inf.time[selected_cell][selected_lambda][i];
		single_scan.omega_s_t = (double) run_inf.omega_s_t[selected_cell][selected_lambda][i];
		simdata[0].scan.push_back(single_scan);
	}
	double tmp = sp.speed_step[0].delay_minutes;
	US_Data_IO *data_io;
	data_io = new US_Data_IO(&run_inf, false); // (baseline flag can be false, we don't need it)
	data_io->assign_simparams(&sp, selected_cell, selected_lambda, selected_channel);
	delete data_io;
	sp.moving_grid = moving_grid;
	sp.mesh = mesh;
	sp.simpoints = simpoints;
	sp.band_volume = band_volume;
	sp.speed_step[0].delay_minutes = tmp;
	assign_model();
	sp.band_firstScanIsConcentration = false;
	astfem_rsa->calculate(&ms, &sp, &simdata);
	rmsd = 0.0;
	analysis_plot->clear();
	edit_plot->clear();
	plot_edit();
	QwtSymbol symbol;
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		for (j=0; j<points; j++)
		{
			sim[i][j] = simdata[0].scan[i].conc[j];
			res[i][j] = absorbance[i][j] - sim[i][j];
			rmsd += pow(res[i][j], 2.0);
		}

		s_curve[i] = edit_plot->insertCurve("Simulated Model Data");
		r_curve[i] = analysis_plot->insertCurve("Residual Data");
		edit_plot->setCurvePen(s_curve[i], QPen(Qt::red, 1, SolidLine));
		edit_plot->setCurveData(s_curve[i], radius, sim[i], points);

		symbol.setStyle(QwtSymbol::Ellipse);
		symbol.setPen(Qt::yellow);
		symbol.setBrush(Qt::yellow);
		symbol.setSize(1);

		analysis_plot->setCurveStyle(r_curve[i], QwtCurve::NoCurve);
		analysis_plot->setCurveSymbol(curve[i], symbol);
		analysis_plot->setCurvePen(r_curve[i], QPen(Qt::yellow, 1));
		analysis_plot->setCurveData(r_curve[i], radius, res[i], points);
	}
	edit_plot->replot();
	analysis_plot->setTitle(tr("Fitting Residuals"));
	analysis_plot->replot();
	rmsd /= (run_inf.scans[selected_cell][selected_lambda] * points);
	str.sprintf("%6.4e", rmsd);
	lbl_variance2->setText(str);
	rmsd = pow((double)rmsd, 0.5);
	str.sprintf("%6.4e", rmsd);
	lbl2_excluded->setText(str);
	pb_save->setEnabled(true);
	pb_view->setEnabled(true);
	pb_print->setEnabled(true);
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		delete [] sim[i];
		delete [] res[i];
	}
	delete [] sim;
	delete [] res;
	delete [] s_curve;
	delete [] r_curve;
	return rmsd;
}

void US_FeMatchRa_W::clear_data(mfem_data *d)
{
	unsigned int i;
	for (i=0; i<(*d).scan.size(); i++)
	{
		(*d).scan[i].conc.clear();
	}
	(*d).radius.clear();
	(*d).scan.clear();
}

void US_FeMatchRa_W::load_model()
{
	QString fn = QFileDialog::getOpenFileName(USglobal->config_list.result_dir, "*model*", 0);
	if ( !fn.isEmpty() )
	{
		US_FemGlobal *fg;
		int error_code;
		QString str;
		fg = new US_FemGlobal(this);
		error_code = fg->read_experiment(&msv, &sp, fn);
		/*
		printf("got model system size %u errorcode %d\n", (unsigned int)msv.size(), error_code); fflush(stdout);
		{
			for (unsigned int i = 0; i < msv.size(); i++)
			{
				fg->write_modelSystem(&msv[i], QString("tmp.model_system-%1").arg(i));
			}
		}
		fg->write_simulationParameters(&sp, "tmp.simulation_parameters");
		*/
		cmb_timeGrid->setCurrentItem(sp.moving_grid);
		cnt_band_volume->setValue(sp.band_volume);
		cnt_simpoints->setValue(sp.simpoints);
		cmb_radialGrid->setCurrentItem(sp.mesh);
		//fg->write_experiment(&ms, &sp, "/root/fematch_ra-output");
		delete fg;
		if (error_code < 0)
		{
			str.sprintf("Unable to load System: " + fn + "\n\nError code: %d", error_code);
			QMessageBox::information(this, tr("Simulation Module"), tr(str), QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			return;
		}
		else
		{
			printError(4); // successfully loaded a new model
			assign_parameters();
			current_model = 0;
			cnt_parameter->setEnabled(true);
			cnt_model->setRange(1, msv.size(), 1);
			cnt_model->setValue(current_model + 1);
			pb_model->setEnabled(true);
			//pb_second_plot->setEnabled(true);
			if (msv.size() > 1)
			{
				bg_plotmode->setEnabled(true);
				cnt_model->setEnabled(true);
			}
			else
			{
				cnt_model->setEnabled(false);
				bg_plotmode->setEnabled(false);
			}
			update_model(1);
		}
	}
}

void US_FeMatchRa_W::printError(const int &ival)
{
	switch (ival)
	{
		case 0:
		{
			QMessageBox::warning(this, tr("UltraScan Warning"),
										tr("Please note:\n\nThere was an error reading\nthe selected Model File!\n\nThis file appears to be corrupted"),
											QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			break;
		}
		case 1:
		{
			QMessageBox::warning(this, tr("UltraScan Warning"),
										tr("Sorry, for old-style model files only\nnon-interacting model loading is supported.\n\n") +
												tr("Please recreate this model by clicking on:\n\n\"New Model\""),
													QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			break;
		}
		case 2:
		{
			QMessageBox::warning(this, tr("UltraScan Warning"), tr("Please note:\n\n") +
					tr("UltraScan could not open\nthe selected Model File!"),
						QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			break;
		}
		case 3:
		{
			QMessageBox::warning(this, tr("UltraScan Warning"),
										tr("Please note:\n\nUltraScan could not read\nthe selected Model File!"),
											QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			break;
		}
		case 4:
		{
			QMessageBox::information(this, tr("Simulation Module"), tr("Successfully loaded Model:\n\n")
					+ modelString[msv[0].model], QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			break;
		}
		case 5:
		{
			QMessageBox::warning(this, tr("UltraScan Warning"),
										tr("Please note:\n\nYou provided an invalid entry!\n\nPlease try again..."),
											QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			break;
		}
		case 6:
		{
			QMessageBox::warning(this, tr("UltraScan Warning"),
										tr("Sorry, old-style models are no longer supported.\n\n") +
												tr("Please load a different model or create a new Model"),
													QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			break;
		}
	}
}

void US_FeMatchRa_W::update_radialGrid(int val)
{
	moving_grid = val;
}

void US_FeMatchRa_W::update_timeGrid(int val)
{
	mesh = val;
}

void US_FeMatchRa_W::update_simpoints(double val)
{
	simpoints = (unsigned int) val;
}

void US_FeMatchRa_W::update_band_volume(double val)
{
	band_volume = (float) val;
}

void US_FeMatchRa_W::update_model(double val)
{
	current_model = (unsigned int) (val - 1);
	ms = msv[current_model];
	fit();
}

void US_FeMatchRa_W::update_parameter(double val)
{
	current_parameter = (unsigned int) val - 1;
	show_parameter();
}

void US_FeMatchRa_W::assign_parameters()
{
	unsigned int j;
	QString str;
	Parameter temp_param;
	par temp_ga_param;
	ga_param.clear(); // this structure needs to be populated. Each parameter has 1 name and a reduced vector of values (identical values are added)
	for (j=0; j<msv[0].component_vector.size(); j++)
	{
		temp_ga_param.name = msv[0].component_vector[j].name + " concentration";
		for (i=0; i<msv.size(); i++)
		{
			temp_param.x = msv[i].component_vector[j].concentration;
			temp_param.y = 1.0;
			temp_ga_param.parameter_list.push_back(temp_param);
		}
		reduce(&temp_ga_param.parameter_list, &temp_ga_param.mode, &temp_ga_param.mean, &temp_ga_param.median);
		ga_param.push_back(temp_ga_param);
		temp_ga_param.parameter_list.clear();

		temp_ga_param.name = msv[0].component_vector[j].name + " sedimentation coefficient";
		for (i=0; i<msv.size(); i++)
		{
			temp_param.x = msv[i].component_vector[j].s;
			temp_param.y = 1.0;
			temp_ga_param.parameter_list.push_back(temp_param);
		}
		reduce(&temp_ga_param.parameter_list, &temp_ga_param.mode, &temp_ga_param.mean, &temp_ga_param.median); // combine like parameters
		ga_param.push_back(temp_ga_param);
		temp_ga_param.parameter_list.clear();

		temp_ga_param.name = msv[0].component_vector[j].name + " diffusion coefficient";
		for (i=0; i<msv.size(); i++)
		{
			temp_param.x = msv[i].component_vector[j].D;
			temp_param.y = 1.0;
			temp_ga_param.parameter_list.push_back(temp_param);
		}
		reduce(&temp_ga_param.parameter_list, &temp_ga_param.mode, &temp_ga_param.mean, &temp_ga_param.median);
		ga_param.push_back(temp_ga_param);
		temp_ga_param.parameter_list.clear();

		temp_ga_param.name = msv[0].component_vector[j].name + " molecular weight";
		for (i=0; i<msv.size(); i++)
		{
			temp_param.x = msv[i].component_vector[j].mw;
			temp_param.y = 1.0;
			temp_ga_param.parameter_list.push_back(temp_param);
		}
		reduce(&temp_ga_param.parameter_list, &temp_ga_param.mode, &temp_ga_param.mean, &temp_ga_param.median);
		ga_param.push_back(temp_ga_param);
		temp_ga_param.parameter_list.clear();

		temp_ga_param.name = msv[0].component_vector[j].name + " frictional ratio";
		for (i=0; i<msv.size(); i++)
		{
			temp_param.x = msv[i].component_vector[j].f_f0;
			temp_param.y = 1.0;
			temp_ga_param.parameter_list.push_back(temp_param);
		}
		reduce(&temp_ga_param.parameter_list, &temp_ga_param.mode, &temp_ga_param.mean, &temp_ga_param.median);
		ga_param.push_back(temp_ga_param);
		temp_ga_param.parameter_list.clear();
	}
	for (j=0; j<msv[0].assoc_vector.size(); j++)
	{
		temp_ga_param.name = str.sprintf("Reaction %d: equilibrium constant", j+1);
		for (i=0; i<msv.size(); i++)
		{
			temp_param.x = msv[i].assoc_vector[j].keq;
			temp_param.y = 1.0;
			temp_ga_param.parameter_list.push_back(temp_param);
		}
		reduce(&temp_ga_param.parameter_list, &temp_ga_param.mode, &temp_ga_param.mean, &temp_ga_param.median);
		ga_param.push_back(temp_ga_param);
		temp_ga_param.parameter_list.clear();

		temp_ga_param.name = str.sprintf("Reaction %d: k_off rate", j+1);
		for (i=0; i<msv.size(); i++)
		{
			temp_param.x = msv[i].assoc_vector[j].k_off;
			temp_param.y = 1.0;
			temp_ga_param.parameter_list.push_back(temp_param);
		}
		reduce(&temp_ga_param.parameter_list, &temp_ga_param.mode, &temp_ga_param.mean, &temp_ga_param.median);
		ga_param.push_back(temp_ga_param);
		temp_ga_param.parameter_list.clear();
	}
	cnt_parameter->setRange(1, ga_param.size(), 1);
	current_parameter = 0;
	cnt_parameter->setValue(current_parameter + 1);
}

void US_FeMatchRa_W::select_plotmode(int val)
{
	plotmode = val;
	fit();
}

void US_FeMatchRa_W::assign_model()
{
	unsigned int count;
	switch (plotmode)
	{
		case 0: // current model
		{
			ms = msv[current_model];
			break;
		}
		case 1: // mode
		{
			ms = msv[current_model];
			count = 0; // now update the relevant variables with the statistical averages
			for (j=0; j<msv[0].component_vector.size(); j++)
			{
				ms.component_vector[j].concentration = ga_param[count].mode;
				count ++;
				ms.component_vector[j].s = ga_param[count].mode;
				count ++;
				ms.component_vector[j].D = ga_param[count].mode;
				count ++;
				ms.component_vector[j].mw = ga_param[count].mode;
				count ++;
				ms.component_vector[j].f_f0 = ga_param[count].mode;
				count ++;
			}
			for (j=0; j<msv[0].assoc_vector.size(); j++)
			{
				ms.assoc_vector[j].keq = ga_param[count].mode;
				count ++;
				ms.assoc_vector[j].k_off = ga_param[count].mode;
			}
			break;
		}
		case 2: // mean
		{
			ms = msv[current_model];
			count = 0; // now update the relevant variables with the statistical averages
			for (j=0; j<msv[0].component_vector.size(); j++)
			{
				ms.component_vector[j].concentration = ga_param[count].mean;
				count ++;
				ms.component_vector[j].s = ga_param[count].mean;
				count ++;
				ms.component_vector[j].D = ga_param[count].mean;
				count ++;
				ms.component_vector[j].mw = ga_param[count].mean;
				count ++;
				ms.component_vector[j].f_f0 = ga_param[count].mean;
				count ++;
			}
			for (j=0; j<msv[0].assoc_vector.size(); j++)
			{
				ms.assoc_vector[j].keq = ga_param[count].mean;
				count ++;
				ms.assoc_vector[j].k_off = ga_param[count].mean;
			}
			break;
		}
		case 3: // median
		{
			ms = msv[current_model];
			count = 0; // now update the relevant variables with the statistical averages
			for (j=0; j<msv[0].component_vector.size(); j++)
			{
				ms.component_vector[j].concentration = ga_param[count].median;
				count ++;
				ms.component_vector[j].s = ga_param[count].median;
				count ++;
				ms.component_vector[j].D = ga_param[count].median;
				count ++;
				ms.component_vector[j].mw = ga_param[count].median;
				count ++;
				ms.component_vector[j].f_f0 = ga_param[count].median;
				count ++;
			}
			for (j=0; j<msv[0].assoc_vector.size(); j++)
			{
				ms.assoc_vector[j].keq = ga_param[count].median;
				count ++;
				ms.assoc_vector[j].k_off = ga_param[count].median;
			}
			break;
		}
	}
}

void US_FeMatchRa_W::show_model()
{
	if (component_dialog == NULL)
	{
		component_dialog = new US_ModelEditor(true, &msv[current_model]);
	}
	component_dialog->exec();
	delete component_dialog;
	component_dialog = NULL;
}

void US_FeMatchRa_W::show_parameter()
{
	//cout << "Number of parameters: " << ga_param[current_parameter].parameter_list.size() << endl;
	unsigned int count=0;
	double *x, *y;
	x = new double [ga_param[current_parameter].parameter_list.size()];
	y = new double [ga_param[current_parameter].parameter_list.size()];
	list <Parameter>::iterator pit1;
	for (pit1 = ga_param[current_parameter].parameter_list.begin(); pit1 != ga_param[current_parameter].parameter_list.end(); pit1++)
	{
		x[count] = (*pit1).x;
		//cout << count << ": " << x[count] << ", ";
		y[count] = (*pit1).y;
		//cout << y[count] << "\n";
		count ++;
	}
	long curve;
	analysis_plot->clear();
	curve = analysis_plot->insertCurve(ga_param[current_parameter].name);
	analysis_plot->setCurveData(curve, x, y, count);
	analysis_plot->setCurveStyle(curve, QwtCurve::Sticks);
	analysis_plot->setCurvePen(curve, QPen(Qt::yellow, 6, SolidLine));
	analysis_plot->setTitle(ga_param[current_parameter].name);

	analysis_plot->replot();
}

// this function combines like solutes and adds their frequencies, and calculates some statistics
void US_FeMatchRa_W::reduce(list <Parameter> *p, double *mode, double *mean, double *median)
{
	list <Parameter> par1, par2;
	Parameter high;
	Parameter low;
	list <Parameter>::iterator pit1, pit2;
	for (pit1 = (*p).begin(); pit1 != (*p).end(); pit1++)
	{
		par1.push_back(*pit1);
	}
	double sum = 0.0;
	(*mode) = -1.0e300;
	for (pit1 = par1.begin(); pit1 != par1.end(); pit1++)
	{
		sum += (*pit1).x;
	}
	(*mean) = sum/par1.size(); // all values have weight 1 at this point
	par1.sort();
	high = par1.back();
	low = par1.front();
	(*median) = low.x + (high.x - low.x)/2.0;
	par2 = par1; // make backup copy
	par1.clear(); // clear for repopulation with combined parameters that are identical
	pit1 = par2.begin();
	pit2 = par2.begin();
	while (pit2 != par2.end())
	{
		pit2++; // increment one so we can compare with the next higher up
		if ((*pit1).x == (*pit2).x)
		{
			(*pit1).y += (*pit2).y; // if they are the same, add frequencies
		}
		else
		{
			par1.push_back(*pit1); // if they are not the same
			pit1 = pit2; // jump to next higher point in list and set iterator to next point.
		}
	}
	(*p).clear();
	for (pit1 = par1.begin(); pit1 != par1.end(); pit1++)
	{
		(*mode) = max((*pit1).x, (*mode));
		(*p).push_back(*pit1);
	}
}
