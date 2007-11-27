#include "../include/us_gridcontrol.h"

US_GridControl::US_GridControl(QWidget *p, const char *name) : Data_Control_W(1, p, name)
{
	GUI = true;
	timestamp = QDateTime::currentDateTime();	
	timestamp_string = timestamp.toString("yyMMddhhmmss");
	SA2D_Params.ff0_min = 1.0;
	SA2D_Params.ff0_max = 4.0;
	SA2D_Params.ff0_resolution = 100;
	SA2D_Params.s_min = 1.0;
	SA2D_Params.s_max = 10.0;
	SA2D_Params.s_resolution = 100;
	SA2D_Params.uniform_grid_repetition = 1;
	SA2D_Params.regularization = 0.95;
	SA2D_Params.meniscus_range = 0.001;
	SA2D_Params.meniscus_gridpoints = 10;
	SA2D_Params.fit_meniscus = false;
	SA2D_Params.max_iterations = 3;
	SA2D_Params.use_iterative = false;
	GA_Params.genes = 500;
	GA_Params.demes = 10;
	GA_Params.generations = 100;
	GA_Params.crossover = 50;
	GA_Params.initial_solutes = 5;
	GA_Params.mutation = 50;
	GA_Params.plague = 0;
	GA_Params.elitism = 2;
	GA_Params.random_seed = 0;
	GA_Params.regularization = 0.95;
	GA_Params.meniscus_range = 0.001;
	GA_Params.fit_meniscus = false;
	GA_Params.solute.clear();
	meniscus_range = 0.001;
	fit_meniscus = false;
	meniscus_gridpoints = 10;
	use_iterative = false;
	max_iterations = 3;
	data_loaded = false;
	analysis_defined = false;
	Control_Params.int_params.clear();
	Control_Params.float_params.clear();
	cluster = new US_Cluster(); 
	int xpos, ypos, spacing=2;
	setCaption("UltraScan Cluster Control");
	
	delete smoothing_lbl;
	delete smoothing_counter;
	smoothing_counter = NULL;
	delete range_lbl;
	delete range_counter;
	range_counter = NULL;
	delete position_lbl;
	delete position_counter;
	position_counter = NULL;
	delete analysis_plot;
	delete lbl1_excluded;
	delete lbl2_excluded;
	pb_second_plot->hide();
	pb_save->hide();
	pb_view->hide();
	delete lbl_status;
	delete progress;
	
	email = "";
	pb_print->setText(tr("Add Dataset"));
	pb_print->disconnect();
	connect(pb_print, SIGNAL(clicked()), SLOT(add_experiment()));
	pb_reset->setText("Run Experiment");
	pb_reset->disconnect();
	connect(pb_reset, SIGNAL(clicked()), SLOT(write_experiment()));
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
	xpos = spacing;
	ypos = 293;

	pb_ga_control = new QPushButton(tr("GA Control"), this);
	Q_CHECK_PTR(pb_ga_control);
	pb_ga_control->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_ga_control->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_ga_control->setAutoDefault(false);
//	pb_ga_control->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_ga_control, SIGNAL(clicked()), SLOT(ga_control()));

	xpos = buttonw + spacing;
	
	pb_sa2d_control = new QPushButton(tr("2D-SA Control"), this);
	Q_CHECK_PTR(pb_sa2d_control);
	pb_sa2d_control->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_sa2d_control->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_sa2d_control->setAutoDefault(false);
//	pb_sa2d_control->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_sa2d_control, SIGNAL(clicked()), SLOT(sa2d_control()));
	
	xpos = spacing;
	ypos += buttonh + spacing;

	bt_tinoise = new QRadioButton(this);
	bt_tinoise->setText(tr(" Fit T-I Noise"));
	bt_tinoise->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1)); //, QFont::Bold));
	bt_tinoise->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	bt_tinoise->setChecked(true);
//	bt_tinoise->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(bt_tinoise, SIGNAL(clicked()), SLOT(update_tinoise()));
	fit_tinoise = true;

	xpos = buttonw + spacing;
	
	bt_rinoise = new QRadioButton(this);
	bt_rinoise->setText(tr(" Fit R-I Noise"));
	bt_rinoise->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1)); //, QFont::Bold));
	bt_rinoise->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	bt_rinoise->setChecked(false);
//	bt_rinoise->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(bt_rinoise, SIGNAL(clicked()), SLOT(update_rinoise()));
	fit_rinoise = false;
	
	xpos = spacing;
	ypos += buttonh + spacing;

	lbl_email = new QLabel(tr(" Notification E-mail:"),this);
	lbl_email->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
//	lbl_email->setGeometry(xpos, ypos, buttonw, buttonh);
	lbl_email->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += spacing + buttonw;

	le_email = new QLineEdit(this);
//	le_email->setGeometry(xpos, ypos, buttonw, buttonh);
	le_email->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_email->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_email, SIGNAL(textChanged(const QString &)), SLOT(update_email(const QString &)));

	xpos = spacing;
	ypos += buttonh + spacing;

	lb_experiments = new QListBox(this, "Scans");
	lb_experiments->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	lb_experiments->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	connect(lb_experiments, SIGNAL(selected(int)), SLOT(delete_experiment(int)));

	global_Xpos += 30;
	global_Ypos += 30;
//	setGeometry(global_Xpos, global_Ypos, this->width(), this->height());
	setup_GUI();
}

// this constructor is used for non-gui calls from the command line. It
// reads an input file with all the details assigned from a web interface
US_GridControl::US_GridControl(const QString &control_file, const QString &gridopt, QWidget *p, const char *name)  : Data_Control_W(101, p, name)
{
  gcfile = control_file;
  this->gridopt = gridopt;
  cout << "gridopt: " << gridopt << endl;
	GUI = false;
	cout << "non-gui\n";
	timestamp = QDateTime::currentDateTime();	
	timestamp_string = timestamp.toString("yyMMddhhmmss");
   SA2D_Params.ff0_min = 1.0;
   SA2D_Params.ff0_max = 4.0;
   SA2D_Params.ff0_resolution = 10;
   SA2D_Params.s_min = 1.0;
   SA2D_Params.s_max = 10.0;
   SA2D_Params.s_resolution = 10;
   SA2D_Params.regularization = 0.0;
   SA2D_Params.uniform_grid_repetition = 1;
	SA2D_Params.meniscus_range = 0.0;
	SA2D_Params.meniscus_gridpoints = 0;
	SA2D_Params.fit_meniscus = false;
	SA2D_Params.max_iterations = 3;
	SA2D_Params.use_iterative = false;
   GA_Params.genes = 500;
   GA_Params.demes = 10;
   GA_Params.generations = 100;
   GA_Params.crossover = 50;
   GA_Params.initial_solutes = 5;
   GA_Params.mutation = 50;
   GA_Params.plague = 0;
   GA_Params.elitism = 2;
   GA_Params.random_seed = 0;
   GA_Params.regularization = 0.95;
   GA_Params.meniscus_range = 0.0;
   GA_Params.fit_meniscus = false;
   GA_Params.solute.clear();
   meniscus_range = 0.0;
   fit_meniscus = false;
	use_iterative = false;
	max_iterations = 3;
   cerr << "l1\n";
	struct bucket temp_bucket;
	QString file_info;
	unsigned int count;
	Control_Params.int_params.clear();
	Control_Params.float_params.clear();

	cp_list.clear();
	rotor_list.clear();
	GA_Params.solute.clear();
	if (!readCenterpieceInfo(&cp_list))
	{
		cout << "UltraScan Fatal Error: There was a problem opening the\n"
				  "centerpiece database file:\n\n"
				  + USglobal->config_list.system_dir + "/etc/centerpiece.dat\n\n"
				  "Please install the centerpiece database file\n"
				  "before proceeding.\n\n";
		exit(-1);
	}
	if (!readRotorInfo(&rotor_list))
	{
		cout << "UltraScan Fatal Error:", "There was a problem opening the\n"
				  "rotor database file:\n\n"
				  + USglobal->config_list.system_dir + "/etc/rotor.dat\n\n"
				  "Please install the rotor database file\n"
				  "before proceeding.\n\n";
		exit(-1);
	}
	cerr << "l2\n";
	QFile f(control_file);
	if(f.open(IO_ReadOnly))
	{
	  cerr << "l2b\n";
		QTextStream ts(&f);
		db_login.driver = "QMYSQL3";
		ts >> analysis_type;
cerr << "Analysis Type: " << analysis_type << endl;
		ts >> db_login.user;
cerr << "Database user: " << db_login.user << endl;
		ts >> db_login.password;
cout << "Database password: " << db_login.password << endl;
		ts >> db_login.database;
cerr << "Database name: " << db_login.database << endl;
		ts >> db_login.host;
cout << "Database host: " << db_login.host << endl;
		ts >> count;
cerr << "Number of experiments: " << count << endl;
		for (unsigned  int i=0; i<count; i++)
		{
			ts >> file_info; // run ID
			int position = file_info.find(".veloc.", 0, false);
			file_info = file_info.left(position);
cout << "Run ID for experiment " << i+1 << ": " << file_info << endl;
			ts >> USglobal->config_list.result_dir; // place where data was extracted to
cout << "Path for experiment " << i+1 << ": " << USglobal->config_list.result_dir << endl;
			fn = USglobal->config_list.result_dir + "/" + file_info + ".us.v";
			ts >> selected_cell;
			selected_cell --;
cout << "Cell for experiment " << i+1 << ": " << selected_cell << endl;
			ts >> selected_lambda;
			selected_lambda --;
cout << "Wavelength for experiment " << i+1 << ": " << selected_lambda << endl;
			ts >> selected_channel;
			selected_channel --;
cout << "Channel for experiment " << i+1 << ": " << selected_channel << endl;
cerr << "About to load file: " << fn << endl;
			load_data(fn);

			if(load_scan() < 0)
			{
				cout << "Error: the selected scan could not be loaded:\n\n";
				cout << "selected Experiment: " << fn << "\n";
				cout << "selected cell: " << selected_cell + 1 << "\n";
				cout << "selected channel: " << selected_channel + 1 << "\n";
				cout << "selected wavelength: " << selected_lambda + 1 << "\n\n";
				cout << "Run aborted....\n\n";
			}
			density = Density[selected_cell][selected_lambda];
			viscosity = Viscosity[selected_cell][selected_lambda];
			add_experiment();
		}
		if (analysis_type == "GA")
		{
			ts >> GA_Params.demes;
			cerr << "demes:" << GA_Params.demes << endl;
			ts >> GA_Params.generations;
			cerr << "generations:" << GA_Params.generations << endl;
			ts >> GA_Params.crossover;
			cerr << "crossover:" << GA_Params.crossover << endl;
			ts >> GA_Params.mutation;
			cerr << "mutation:" << GA_Params.mutation << endl;
			ts >> GA_Params.plague;
			cerr << "plague:" << GA_Params.plague << endl;
			ts >> GA_Params.elitism;
			cerr << "elitism:" << GA_Params.elitism << endl;
			ts >> GA_Params.migration_rate;
			cerr << "Migration Rate:" << GA_Params.migration_rate << endl;
			ts >> GA_Params.genes;
			cerr << "genes:" << GA_Params.genes << endl;
			ts >> GA_Params.initial_solutes;
			cerr << "solutes:" << GA_Params.initial_solutes << endl;
			ts >> GA_Params.random_seed;
			cerr << "seed:" << GA_Params.random_seed << endl;
			ts >> GA_Params.regularization;
			cerr << "regularization:" << GA_Params.regularization << endl;
			regularization = GA_Params.regularization;
			for (unsigned int i=0; i<GA_Params.initial_solutes; i++)
			{
			  ts >>  temp_bucket.s;
			  ts >>  temp_bucket.s_min;
			  ts >>  temp_bucket.s_max;
			  ts >>  temp_bucket.ff0;
			  ts >>  temp_bucket.ff0_min;
			  ts >>  temp_bucket.ff0_max;
			  GA_Params.solute.push_back(temp_bucket);
			}
			ts >> count;
cout << "Fit meniscus?: " << count << endl;
			GA_Params.fit_meniscus = count;
			fit_meniscus = count;
			ts >> GA_Params.meniscus_range;
cout << "Meniscus range: " << GA_Params.meniscus_range << endl;
			meniscus_range = GA_Params.meniscus_range;
			ts >> count;
cerr << "Fit ti noise?: " << count << endl;
			fit_tinoise = count;
			ts >> count;
cerr << "Fit ri noise?: " << count << endl;
			fit_rinoise = count;
			ts >> email;
cerr << "Email: " << email << endl;
			{
				analysis_defined = true;
				data_loaded = true;
				write_experiment();
			}
		}
		if (analysis_type == "SA2D")
		{
			ts >> SA2D_Params.ff0_min;
cout << "ff0 min: " << SA2D_Params.ff0_min << endl;
			ts >> SA2D_Params.ff0_max;
cout << "ff0 max: " << SA2D_Params.ff0_max << endl;
			ts >> SA2D_Params.ff0_resolution;
cout << "ff0 Resolution: " << SA2D_Params.ff0_resolution << endl;
			ts >> SA2D_Params.s_min;
cout << "s min: " << SA2D_Params.s_min << endl;
			ts >> SA2D_Params.s_max;
cout << "s max: " << SA2D_Params.s_max << endl;
			ts >> SA2D_Params.s_resolution;
cout << "s Resolution: " << SA2D_Params.s_resolution<< endl;
			ts >> SA2D_Params.regularization;
			regularization = SA2D_Params.regularization;
cout << "Regularization: " << SA2D_Params.regularization << endl;
			ts >> SA2D_Params.uniform_grid_repetition;
cout << "Grid Repetitions: " << SA2D_Params.uniform_grid_repetition << endl;
			ts >> count;
cout << "Fit meniscus?: " << count << endl;
			SA2D_Params.fit_meniscus = count;
			fit_meniscus = count;
			ts >> SA2D_Params.meniscus_range;
cout << "Meniscus range: " << SA2D_Params.meniscus_range << endl;
			meniscus_range = SA2D_Params.meniscus_range;
			ts >> SA2D_Params.meniscus_gridpoints;
cout << "Meniscus Gridpoints: " << SA2D_Params.meniscus_gridpoints << endl;
			meniscus_gridpoints = SA2D_Params.meniscus_gridpoints;
			ts >> count;
cout << "Fit ti noise?: " << count << endl;
			fit_tinoise = count;
			ts >> count;
cout << "Fit ri noise?: " << count << endl;
			fit_rinoise = count;
			ts >> email;
cout << "Email: " << email << endl;
			ts >> count;
cout << "Use iterative Method?: " << count << endl;
			SA2D_Params.use_iterative = count;
			use_iterative = count;
			ts >> SA2D_Params.max_iterations;
cout << "Max iterations: " << SA2D_Params.max_iterations << endl;
			max_iterations = SA2D_Params.max_iterations;

 cerr << "px\n";
			{
				analysis_defined = true;
				data_loaded = true;
				write_experiment();
			}
		}
	}
	else
	{
		cout << "Error: cannot read input file " << control_file << endl;
		cout << "Run aborted ...\n\n";
	}
	exit(0);
}

US_GridControl::~US_GridControl()
{
}

void US_GridControl::setup_GUI()
{
	int j=0;
	int rows = 14, columns = 4, spacing = 2;
	
	QGridLayout * background = new QGridLayout(this, 2, 2,spacing);	
	QGridLayout * subGrid1 = new QGridLayout(rows, columns, spacing);
	for (int i=0; i<rows; i++)
	{
		subGrid1->setRowSpacing(i, 26);
	}
	subGrid1->addMultiCellWidget(pb_load,j,j,0,1);
	subGrid1->addMultiCellWidget(pb_details,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(pb_print,j,j,0,1);
	subGrid1->addMultiCellWidget(pb_reset,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(pb_help,j,j,0,1);
	subGrid1->addMultiCellWidget(pb_close,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(banner1,j,j,0,3);
	j++;
	subGrid1->addMultiCellWidget(lbl_run_id1,j,j,0,1);
	subGrid1->addMultiCellWidget(lbl_run_id2,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(lbl_temperature1,j,j,0,1);
	subGrid1->addMultiCellWidget(lbl_temperature2,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(lbl_cell_info1,j,j,0,1);
	subGrid1->addMultiCellWidget(lbl_cell_info2,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(lbl_cell_descr,j,j,0,3);
	j++;
	subGrid1->addMultiCellWidget(cell_select,j,j+2,0,1);
	subGrid1->addMultiCellWidget(lambda_select,j,j+2,2,3);
	j=j+3;	
	subGrid1->addMultiCellWidget(banner2,j,j,0,3);
	j++;
	subGrid1->addWidget(pb_density,j,0);
	subGrid1->addWidget(density_le,j,1);
	subGrid1->addWidget(pb_viscosity,j,2);
	subGrid1->addWidget(viscosity_le,j,3);
	j++;
	subGrid1->addWidget(pb_vbar,j,0);
	subGrid1->addWidget(vbar_le,j,1);
		
	rows = 5, columns = 4, spacing = 2, j=0;
	QGridLayout * subGrid2 = new QGridLayout(rows, columns, spacing);
	for (int i=0; i<rows; i++)
	{
		subGrid2->setRowSpacing(i, 26);
	}

	subGrid2->addMultiCellWidget(pb_ga_control,j,j,0,1);
	subGrid2->addMultiCellWidget(pb_sa2d_control,j,j,2,3);
	j++;
	subGrid2->addMultiCellWidget(bt_tinoise,j,j,0,1);
	subGrid2->addMultiCellWidget(bt_rinoise,j,j,2,3);
	j++;
	subGrid2->addMultiCellWidget(lbl_email,j,j,0,1);
	subGrid2->addMultiCellWidget(le_email,j,j,2,3);
	j++;
	subGrid2->addMultiCellWidget(pb_exsingle,j,j,0,1);
	subGrid2->addMultiCellWidget(cnt_exsingle,j,j,2,3);
	j++;
	subGrid2->addMultiCellWidget(pb_exrange,j,j,0,1);
	subGrid2->addMultiCellWidget(cnt_exrange,j,j,2,3);
			
	background->addLayout(subGrid1,0,0);
	background->addWidget(edit_plot,0,1);
	background->addLayout(subGrid2,1,0);
	background->addWidget(lb_experiments,1,1);
	background->setColStretch(0,1);
	background->setColStretch(1,2);
//	background->setColSpacing(0,340);
//	background->setColSpacing(1,540);

	qApp->processEvents();
	QRect r = background->cellGeometry(0, 0);

	global_Xpos += 30;
	global_Ypos += 30;
	
	this->setGeometry(global_Xpos, global_Ypos, r.width()+550+spacing*3, 568);
}

void US_GridControl::update_email(const QString &str)
{
	email = str;
}

void US_GridControl::ga_control()
{
	US_Ga_GridControl *GaControl;
	GaControl = new US_Ga_GridControl(&GA_Params);
	analysis_type = "GA";
	if(GaControl->exec())
	{
		bool solutes_defined = false;
		meniscus_range = GA_Params.meniscus_range;
		fit_meniscus = GA_Params.fit_meniscus;
		for (unsigned int i=0; i<GA_Params.solute.size(); i++)
		{
			if (GA_Params.solute[i].s_max != 0.0 &&
				 GA_Params.solute[i].s_min != 0.0 &&
				 GA_Params.solute[i].s != 0.0 &&
				 GA_Params.solute[i].ff0 != 0.0 &&
				 GA_Params.solute[i].ff0_min != 0.0 &&
				 GA_Params.solute[i].ff0_max != 0.0)
			{
				solutes_defined = true;
			}
		}
		if (solutes_defined)
		{	
			QMessageBox::information(this, tr("Cluster Control:"), 
			tr("The Genetic Algorithm parameters are now defined.\n Please select all experiments to be included in this\n fit, then click on \"Write Data\""));
		}
		else
		{
			QMessageBox::information(this, tr("Cluster Control:"), 
			tr("The solute bins have not been correctly assigned.\n You will need to restart the genetic algorithm control\n	module and assign at least one solute bin before proceeding"));
		}
		analysis_defined = true;
		if (data_loaded && GUI)
		{
			pb_reset->setEnabled(true);
		}
	}
	else
	{
		analysis_defined = false;
		if (GUI)
		{
			pb_reset->setEnabled(false);
		}
	}
}

void US_GridControl::update_tinoise()
{
	if (fit_tinoise)
	{
		fit_tinoise = false;
		fit_rinoise = false;
		bt_rinoise->setChecked(false);
	}
	else
	{
		fit_tinoise = true;
	}
}

void US_GridControl::update_rinoise()
{
	if (fit_rinoise)
	{
		fit_rinoise = false;
	}
	else
	{
		fit_rinoise = true;
		fit_tinoise = true;
		bt_tinoise->setChecked(true);
	}
}

void US_GridControl::write_experiment()
{
  cerr << "write_experiment\n";
	if (!analysis_defined || !data_loaded)
	{
		return;
	}
	if (email.isEmpty())
	{
		if(GUI)
		{
			QMessageBox::critical(0, "UltraScan Error:", tr("You have to provide an e-mail address!"), 
					 QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			return;
		}
		else
		{
			cout << tr("You have to provide an e-mail address!") << endl;
			return;
		}
	}
	printf("write experiment<%s>\n", USglobal->config_list.result_dir.ascii());
	unsigned int count, i, j, k;
	bool union_results;
	QString str;
	QString user = "ultrascan";
	
	
	QFile f(USglobal->config_list.result_dir + QString("/experiments%1.dat").arg(timestamp_string));
	cout << "try to write experiment data for " << analysis_type << "\n";
	if(f.open(IO_WriteOnly))
	{
	  cout << "write experiment data for " << analysis_type << "\n";
		QDataStream ds(&f);
		ds << email;
		ds << analysis_type;
		ds << fit_tinoise;
		ds << fit_rinoise;
		union_results = false;
		if(analysis_type == "SA2D" &&
		   SA2D_Params.uniform_grid_repetition > 1) {
		  union_results = true;
		}
		ds << union_results;
		ds << meniscus_range;
		ds << fit_meniscus;
		ds << meniscus_gridpoints;
		ds << use_iterative;
		ds << max_iterations;
		ds << regularization;
		ds << (unsigned int)experiment.size();
		cerr << "experiment size is " << experiment.size() << endl;
		cerr << "sizeof(experiment size) is " << sizeof(experiment.size()) << endl;
		cerr << "sizeof(unsigned int) is " << sizeof(unsigned int) << endl;
		if(analysis_type == "SA2D") {
		  ds << SA2D_Params.ff0_min;
		  ds << SA2D_Params.ff0_max;
		  ds << SA2D_Params.ff0_resolution;
		  ds << SA2D_Params.s_min;
		  ds << SA2D_Params.s_max;
		  ds << SA2D_Params.s_resolution;
		  ds << SA2D_Params.uniform_grid_repetition;
		}
 		if(analysis_type == "GA") {
		  ds << GA_Params.demes;
		  ds << GA_Params.generations;
		  ds << GA_Params.crossover;
		  ds << GA_Params.mutation;
		  ds << GA_Params.plague;
		  ds << GA_Params.elitism;
		  ds << GA_Params.migration_rate;
		  ds << GA_Params.genes;
		  ds << GA_Params.initial_solutes;
		  ds << GA_Params.random_seed;
		  cerr << "writing demes:" << GA_Params.demes << endl;
		  cerr << "generations:" << GA_Params.generations << endl;
		  cerr << "crossover:" << GA_Params.crossover << endl;
		  cerr << "mutation:" << GA_Params.mutation << endl;
		  cerr << "plague:" << GA_Params.plague << endl;
		  cerr << "elitism:" << GA_Params.elitism << endl;
		  cerr << "migration:" << GA_Params.migration_rate << endl;
		  cerr << "genes:" << GA_Params.genes << endl;
		  cerr << "solutes:" << GA_Params.initial_solutes << endl;
		  cerr << "seed:" << GA_Params.random_seed << endl;
		  cerr << "sizeof(unsigned long):" << sizeof(unsigned long) << endl;

		  for(i = 0; i < GA_Params.solute.size(); i++) {
		    ds <<  GA_Params.solute[i].s;
		    ds <<  GA_Params.solute[i].s_min;
		    ds <<  GA_Params.solute[i].s_max;
		    ds <<  GA_Params.solute[i].ff0;
		    ds <<  GA_Params.solute[i].ff0_min;
		    ds <<  GA_Params.solute[i].ff0_max;
		    cerr << "writing solute:" << i << endl;
		    cerr << "solute s:" << GA_Params.solute[i].s << endl;
		    cerr << "solute smin:" << GA_Params.solute[i].s_min << endl;
		    cerr << "solute ff0:" << GA_Params.solute[i].ff0 << endl;
		  }
		}
		for (i=0; i<experiment.size(); i++)
		{
			ds << experiment[i].id;
			ds << experiment[i].cell;
			ds << experiment[i].channel;
			ds << experiment[i].wavelength;
			ds << experiment[i].meniscus;
			ds << experiment[i].bottom;
			ds << experiment[i].rpm;
			ds << experiment[i].s20w_correction;
			ds << experiment[i].D20w_correction;
			ds << (unsigned int) experiment[i].radius.size();
			for (j=0; j<experiment[i].radius.size(); j++)
			{
			  ds << experiment[i].radius[j];
			  //				ds << (short int)( 10000 * (experiment[i].radius[j] - 5.0));
			}
			ds << (unsigned int)experiment[i].scan.size();
			printf("scan[0][0] %.12g\n", experiment[i].scan[0].conc[0]);
			for (j=0; j<experiment[i].scan.size(); j++)
			{
				ds << experiment[i].scan[j].time;
				for (k=0; k<experiment[i].radius.size(); k++)
				{
				  ds << experiment[i].scan[j].conc[k];
				  //					ds << (short int) (10000 * experiment[i].scan[j].conc[k]);
				}
			}
			fprintf(stderr,"experiment last time %g avg_temp %g vbar %g visc %g density %g vbar %g vbar20 %g rpm %u bottom %g meniscus %g scorr %g Dcorr %g\n", 
			experiment[i].scan[experiment[i].scan.size()-1].time,
			experiment[i].avg_temperature,
			experiment[i].vbar,
			experiment[i].viscosity,
			experiment[i].density,
			experiment[i].vbar,
			experiment[i].vbar20,
			experiment[i].rpm,
			experiment[i].bottom,
			experiment[i].meniscus,
			experiment[i].s20w_correction,
			experiment[i].D20w_correction);

			ds << experiment[i].viscosity;
			ds << experiment[i].density;
			ds << experiment[i].vbar;
			ds << experiment[i].vbar20;
			ds << experiment[i].avg_temperature;
		}
		f.close();
		
/*
	how the process is parallelized:
	
	At this point we want to split the job into however many clusters exist.
	the total job should be divided into a loadbalanced share for each cluster,
	where the number of jobs depends on the available nodes in a particular cluster and
	their performance.
	
	Initially, we just want to run on the bcf cluster. So there is only one node
	to send this to, master.hydrodyn.beowulf.
	
	A job sent to a particular cluster will be sent to the MPI master, which
	allocates portions from an array of jobs to individual nodes available
	over MPI in the cluster. So the job sent to the MPI master will actually
	consist of an array of jobs, or an array of genes, where each element 
	in the gene array is a gene, which contains one or more components, or a
	list of solutes that will be simulated and then interpolated to the 
	experimental data, then NNLS is performed on the interpolated solution
	and the experimental data. Each job returns the following information:
	the amplitudes found by NNLS for each solute in the simulated solution, 
	the time- and radially invariant noise vectors, and finally the variance.
	
	Each cluster also gets its own job control file, which lists only the
	gene arrays to be compared to the experimental solution, and a list of
	float and integer arrays for any desired use. So each cluster
	gets 2 files: 1. experimental_data.dat which contains the experimental data,
	and 2. genes-<i>.dat, which contains the solute arrays or genes to be computed.
	Each MPI master is expected to be able to figure out how best to re-distribute
	all genes it receives to its own cluster slaves.
	
	The name of the experimental_data.dat file and the name of the 
	solutes-<i>.dat file	are passed as a commandline argument to the MPI 
	master, which is called	using a system call.
*/		
		QString f_name;

/*
		find out how many headnodes can be used by looking at the load of each cluster node
*/
		cerr << "b1\n";
		//		cluster->update_load(); // check out the current load and sort according to load
		//		list <cluster_node>::iterator grid_iter;
		cerr << "b2\n";
		count = 0; // count how many clusters are available so we can make some estimate on how to distribute the load
		//	if ((*grid_iter).load_average >= 0 && (*grid_iter).load_average < 2)
		//	{
		// count ++;
		//	}
		i = 0;
		cerr << "b3\n";
		//		for (grid_iter = cluster->grid.begin(); grid_iter != cluster->grid.end(); grid_iter++)
		//		{
		cerr << "b4\n";
		//			i++;
		//			if ((*grid_iter).load_average >= 0 && (*grid_iter).load_average < 2)
		//			{
/*
	Here, the job vector needs to be populated:
	
	vector <struct gene> solutions
	
	where each gene is a vector of solutes. The solutions are genes, i.e.,
	one or more components (solutes) constituting a solution. Basically, 
	we have "count" number of clusters available. For now, we can just divide
	the total workload by "count" to scale the jobs.
	
	The MPI master module (us_fe_nnls_t) will get more than one solution to distribute among its slaves.
	
	The job vector should be allocated according to the load that can be handled
	by each cluster. 
	
*/
		cerr << "p2\n";
				if (analysis_type == "GA")
				{
/*
	For the GA method, we may want to perform each deme on a different cluster,
	and split the job into the calculation of individual genes. f_name contains 
	the solute-<i>.dat file to be passed to the MPI master module. 
*/
				  solutions.clear();  // empty solute file for GA
				  //				  exit(-1);
				}
				if (analysis_type == "SA2D")
				{
				  cerr << "sa2d_2\n";
/*
	For the SA2D method, we consider the array of solutes inside one grid
	simply as the "gene" to be simulated. So each low-resolution frame is 
	one "gene". All low-resolution frames together make up the genes to be
	calculated. f_name contains the solute-<i>.dat file to be passed to the 
	MPI master module. 
*/
				  gene solutes;
				  Solute solute;
				  unsigned int s_partition = SA2D_Params.s_resolution;  
				  unsigned int f_partition = SA2D_Params.ff0_resolution;
				  unsigned int i, step_s, step_k, m, n;
				  unsigned int steps = SA2D_Params.uniform_grid_repetition;
				  double inc_s, inc_k;
				  double s, k, use_s, use_k;
				  double s_diff = 1.0e-13 * (SA2D_Params.s_max - SA2D_Params.s_min);
				  double ff0_diff = SA2D_Params.ff0_max - SA2D_Params.ff0_min;
				  printf("s %g %g %g  ff0 %g %g %g\n",
					 SA2D_Params.s_max*1.0e-13 , SA2D_Params.s_min*1.0e-13, s_diff,
					 SA2D_Params.ff0_max, SA2D_Params.ff0_min,  ff0_diff);

				  for(step_s = 0; step_s < steps; step_s++) {
				    for(step_k = 0; step_k < steps; step_k++) {
				      solutes.component.clear();
				      
				      inc_s = step_s * (s_diff) / ((s_partition - 1) * steps);
				      inc_k = step_k * (ff0_diff) / ((f_partition - 1) * steps);
#define DEBUG
#if defined(DEBUG)
				      printf("step_s %d step_k %d inc_s %g inc_k %g\n", step_s, step_k, inc_s, inc_k);
#endif    
				      i = 0;
				      for(m = 0; m < s_partition; m++) {
					s = SA2D_Params.s_min*1.0e-13 + m * (s_diff) / (s_partition - 1);
					for(n = 0; n < f_partition; n++) {
					  k = SA2D_Params.ff0_min + n * (ff0_diff) / (f_partition - 1);
					  
					  use_s = s + inc_s;
					  use_k = k + inc_k;
					  if(use_s > SA2D_Params.s_max*1.0e-13) {
					    use_s -= s_diff;
					  }
					  if(use_k > SA2D_Params.ff0_max) {
					    use_k -= ff0_diff;
					  }
					  solute.s = use_s;
					  solute.k = use_k;
					  // clipping
					  if(use_s <= (SA2D_Params.s_max*1.0e-13) && use_s >= (SA2D_Params.s_min*1.0e-13) &&
					     use_k <= SA2D_Params.ff0_max && use_k >= SA2D_Params.ff0_min) {
					    solutes.component.push_back(solute);
					  }
					}
				      }
				      printf("solute size on push back %u\n", solutes.component.size());
				      solutions.push_back(solutes);
				    }
				  }				      
				}
				f_name.sprintf(USglobal->config_list.result_dir + "/solutes%s-%d.dat", timestamp_string.ascii(), i);
				write_solutes(f_name); // write the solutions vector to a file

				QString mpifilebase = "> /tmp/gc_mpi_" + timestamp_string + ".%1";
				
				if(gridopt == "TIGRE") {
				  mpifilebase = "> /tmp/gc_tigre_" + timestamp_string + ".%1";
				}
				QString npstring;
				QString machines;
				if(analysis_type == "SA2D") {
				  npstring = "`head -1 $ULTRASCAN/etc/mpi_sa2d_proccount`";
				  machines = " -machinefile $ULTRASCAN/etc/mpi_sa2d_machines ";
				} 
				if(analysis_type == "GA") {
				  npstring = QString("%1").arg(GA_Params.demes + 1);
				  machines = " -machinefile $ULTRASCAN/etc/mpi_ga_machines ";
				}
				  
				
#ifdef BIN64
				QString syscall = 
				  "echo 'mpi_job_run [" + experiment[0].id + "][" +
				  email + "][" + + analysis_type +
				  "] nohup rsh -n -l apache bcf.uthscsa.edu \". /etc/profile; mpirun -np " +
				  npstring + machines +
				  " $ULTRASCAN/bin64/us_fe_nnls_t_mpi " +
				    //  user + "@" + *(grid_iter).node + 
				  USglobal->config_list.result_dir + QString("/experiments%1.dat ").arg(timestamp_string) 
				  + f_name + " " + 
				    mpifilebase.arg(1) + " 2" + mpifilebase.arg(2) + "\" 2>&1 " + mpifilebase.arg(0) 
				  + " ' > $ULTRASCAN/etc/us_gridpipe";
#else
				QString syscall = 
				  "echo 'mpi_job_run [" + experiment[0].id + "][" +
				  email + "][" + + analysis_type +
				  "] nohup rsh -n -l apache bcf.uthscsa.edu \". /etc/profile; mpirun -np " +
				  npstring + machines +
				  " $ULTRASCAN/bin/us_fe_nnls_t_mpi " +
				    //  user + "@" + *(grid_iter).node + 
				  USglobal->config_list.result_dir + QString("/experiments%1.dat ").arg(timestamp_string) 
				  + f_name + " " + 
				    mpifilebase.arg(1) + " 2" + mpifilebase.arg(2) + "\" 2>&1 " + mpifilebase.arg(0) 
				  + " ' > $ULTRASCAN/etc/us_gridpipe";
#endif
				if(gridopt == "TIGRE") {
				    syscall =
				      "echo 'tigre_job_run [" + experiment[0].id + "][" +
				      email + "][" + + analysis_type +
				      "] nohup perl -I$ULTRASCAN/etc $ULTRASCAN/etc/us_tigre_job.pl " + 
				      gcfile + " " +
				      email + " " +
				      timestamp_string + " " +
				      USglobal->config_list.result_dir + " " +
				      USglobal->config_list.result_dir + QString("/experiments%1.dat ").arg(timestamp_string) +
				      f_name + " " + 
				      mpifilebase.arg(1) + " 2" + mpifilebase.arg(2) + 
				      " ' > $ULTRASCAN/etc/us_gridpipe";
				}

				// #define KOZA
#if defined(KOZA)
				syscall = 
				  "nohup rsh -n -l root koza \". /etc/profile; mpirun -np 2 /root/ultrascan/bin/us_fe_nnls_t_mpi " +
				  //  user + "@" + *(grid_iter).node + 
				  USglobal->config_list.result_dir + QString("/experiments%1.dat ").arg(timestamp_string) 
				  + f_name + " > mpilog.1 2> mpilog.2\" 2>&1 > /mpilog.0";
#endif
				cout << syscall << endl;
				printf("$ %s\n", syscall.ascii());
				system(syscall); // issue the call to the remote MPI master which will
    				// be sent to each individual cluster.
				//			}
				//		}
	}
	else
	{
		if(GUI)
		{
			QMessageBox::critical(0, "UltraScan Fatal Error:", tr("There was a problem opening the output file:\n\n")
				  + USglobal->config_list.result_dir + "/us_gridcontrol.dat\n\n"
				  + tr("Please make sure you have write permission and then try again..."), 
					 QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			exit(-2);
		}
		else
		{
			cout << tr("UltraScan Fatal Error: There was a problem opening the output file:\n\n")
				  << USglobal->config_list.result_dir + "/us_gridcontrol.dat\n\n"
				  << tr("Please make sure you have write permission and then try again...");
			exit(-2);
		}
	}
}

void US_GridControl::write_solutes(const QString &filename)
{
	// write solute arrays (genes) to file:
	QFile f(filename);
	if (f.open(IO_WriteOnly))
	{
		QDataStream ds(&f);
		ds << (unsigned int) solutions.size(); // how many genes
		for (unsigned int i=0; i<solutions.size(); i++)
		{
		  printf("solute %d size %u\n", i, solutions[i].component.size());
			ds << (unsigned int) solutions[i].component.size(); // the size of each gene
			for (unsigned int j=0; j<solutions[i].component.size(); j++)
			{
				ds << solutions[i].component[j].s;
				ds << solutions[i].component[j].k;
				ds << solutions[i].component[j].c;
			}
		}
		ds << (unsigned int) Control_Params.float_params.size();
		for (unsigned int i=0; i<Control_Params.float_params.size(); i++)
		{
			ds << (float) Control_Params.float_params[i];
		}
		ds << (unsigned int) Control_Params.int_params.size();
		for (unsigned int i=0; i<Control_Params.int_params.size(); i++)
		{
			ds << (int) Control_Params.int_params[i];
		}
		f.close();
	}
}

// this function is probably not needed...
void US_GridControl::write_ga_experiment()
{
	QFile f(USglobal->config_list.result_dir + "/ga_control.dat");
	if(f.open(IO_WriteOnly))
	{
		QDataStream ds(&f);
		ds << GA_Params.demes;
		ds << GA_Params.generations;
		ds << GA_Params.crossover;
		ds << GA_Params.mutation;
		ds << GA_Params.plague;
		ds << GA_Params.elitism;
		ds << GA_Params.genes;
		ds << GA_Params.initial_solutes;
		ds << GA_Params.random_seed;
		ds << GA_Params.regularization;
		ds << GA_Params.solute.size();
		for (unsigned int i=0; i<GA_Params.solute.size(); i++)
		{
			ds << (float) GA_Params.solute[i].s;
			ds << (float) GA_Params.solute[i].s_min;
			ds << (float) GA_Params.solute[i].s_max;
			ds << (float) GA_Params.solute[i].ff0;
			ds << (float) GA_Params.solute[i].ff0_min;
			ds << (float) GA_Params.solute[i].ff0_max;
		}
		f.close();
	}
}

void US_GridControl::sa2d_control()
{
	US_SA2D_GridControl *SA2D_Control;
	SA2D_Control = new US_SA2D_GridControl(&SA2D_Params);
	analysis_type = "SA2D";
	if(SA2D_Control->exec())
	{
		meniscus_range = SA2D_Params.meniscus_range;
		fit_meniscus = SA2D_Params.fit_meniscus;
		meniscus_gridpoints = SA2D_Params.meniscus_gridpoints;
		analysis_defined = true;
		if (data_loaded && GUI)
		{
			pb_reset->setEnabled(true);
		}
	}
	else
	{
		analysis_defined = false;
		if (GUI)
		{
			pb_reset->setEnabled(false);
		}
	}
}

// this function is probably not needed...

void US_GridControl::write_sa2d_experiment()
{
	QFile f(USglobal->config_list.result_dir + "/sa2d_control.dat");
	if(f.open(IO_WriteOnly))
	{
		QDataStream ds(&f);
		ds << SA2D_Params.ff0_min;
		ds << SA2D_Params.ff0_max;
		ds << SA2D_Params.ff0_resolution;
		ds << SA2D_Params.s_min;
		ds << SA2D_Params.s_max;
		ds << SA2D_Params.s_resolution;
		f.close();
	}
}

void US_GridControl::delete_experiment(int val)
{
	QString str;
	QMessageBox mb(tr("Grid Control Warning:"), 
	tr("Experiment") + " " + experiment[val].id + " " + 
	+ str.sprintf(tr("(cell %d, channel %d, wavelength %d)"), 
	experiment[val].cell+1, experiment[val].channel+1, experiment[val].wavelength+1) 
	+ tr("\nis about to be deleted from this list.\n\nProceed?"), 
	QMessageBox::Warning, 
	QMessageBox::Yes | QMessageBox::Default, 
	QMessageBox::Cancel | QMessageBox::Escape, QMessageBox::NoButton);
	mb.setButtonText(QMessageBox::Yes, "Delete");
	mb.setButtonText(QMessageBox::Cancel, "Cancel");
	switch (mb.exec())
	{
		case QMessageBox::Cancel:
		{
			return;
		}
		case QMessageBox::Yes:
		{
			vector <struct mfem_data>::iterator exp_iter=experiment.begin() + val;
			experiment.erase(exp_iter);
			lb_experiments->removeItem(val);
			break;
		}
		default:
		{
			return;
		}
	}
	if (experiment.size() == 0)
	{
		data_loaded = false;
		if (GUI)
		{
			pb_reset->setEnabled(false);
		}
	}
}

void US_GridControl::add_experiment()
{
	if (run_inf.scans[selected_cell][selected_lambda] == 0)
	{
		return;
	}
	mfem_data temp_experiment;
	mfem_scan temp_scan;
	QString str;
	temp_experiment.radius.clear();
	temp_experiment.scan.clear();
	for (unsigned int i=0; i<experiment.size(); i++) // check to make sure this data has not already been selected
	{
		if (experiment[i].id         == run_inf.run_id
		&&  experiment[i].cell       == selected_cell
		&&  experiment[i].wavelength == selected_lambda
		&&  experiment[i].channel    == selected_channel)
		{
			if (GUI)
			{
				QMessageBox::warning(this, "Attention:", 
				tr("This dataset is already included..."),
				QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
				return;
			}
			else
			{
				cout << experiment[i].id << tr(" has already been included, skipped...\n");
			}
		}
	}
// calculate the temperature, buffer and vbar corrections
	calc_correction(run_inf.avg_temperature);
	cout << "buoyancyb: " << buoyancyb << ", buoyancyw: " << buoyancyw << ", viscosity_tb: " << viscosity_tb << endl;
	temp_experiment.s20w_correction = (buoyancyw*viscosity_tb)/(buoyancyb*(100.0 * VISC_20W));
	temp_experiment.D20w_correction = (K20 * viscosity_tb)/((100.0 * VISC_20W) * (run_inf.avg_temperature + K0));
	cerr << "Density: " << density << ", Viscosity: " << viscosity << ", vbar: " << vbar << ", vbar20: " << vbar20 << endl;
	cerr << "Buoyancy_water: " << buoyancyw << ", Buoyancy_buffer: " << buoyancyb << ", viscosity_TB: " << viscosity_tb << endl;
	printf("s20w_corr %.12g\n", temp_experiment.s20w_correction);
	printf("viscosity_tb %.12g avg temp %.12g\n", viscosity_tb, run_inf.avg_temperature);
	printf("D20w_corr %.12g\n", temp_experiment.D20w_correction);
	temp_experiment.id = run_inf.run_id;
	temp_experiment.cell = selected_cell;
	temp_experiment.channel = selected_channel;
	temp_experiment.wavelength = selected_lambda;
	temp_experiment.meniscus = run_inf.meniscus[selected_cell];
	temp_experiment.bottom = calc_bottom(rotor_list, 
	cp_list, run_inf.rotor, run_inf.centerpiece[selected_cell], 
	selected_channel, run_inf.rpm[selected_cell][selected_lambda][0]);
	temp_experiment.rpm = run_inf.rpm[selected_cell][selected_lambda][0];
	printf("absorbance[0][0] %.12g baseline %12g\n", absorbance[0][0], run_inf.baseline[selected_cell][selected_lambda]);

	for (unsigned int i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		temp_scan.conc.clear();
		temp_scan.time = run_inf.time[selected_cell][selected_lambda][i];
		for (unsigned int j=0; j<points; j++)
		{
			temp_scan.conc.push_back(absorbance[i][j]);
		}
		temp_experiment.scan.push_back(temp_scan);
	}
	for (unsigned int i=0; i<points; i++)
	{
		temp_experiment.radius.push_back(radius[i]);
	}
	printf("den %g visc %g\n", density, viscosity);
	temp_experiment.viscosity = viscosity;
	temp_experiment.density = density;
	temp_experiment.vbar = vbar;
	temp_experiment.vbar20 = vbar20;
	temp_experiment.avg_temperature = run_inf.avg_temperature;
	experiment.push_back(temp_experiment);
	data_loaded = true;
	if (GUI)
	{
		if(analysis_defined && GUI)
		{
			pb_reset->setEnabled(true);
		}
		lb_experiments->insertItem(temp_experiment.id 
		+ str.sprintf(tr(" (cell %d, channel %d, wavelength %d)"), 
		temp_experiment.cell+1, temp_experiment.channel+1, temp_experiment.wavelength+1), -1);
	}
}

void US_GridControl::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/gridcontrol.html");
}

void US_GridControl::closeEvent(QCloseEvent *e)
{
	global_Xpos -= 30;
	global_Ypos -= 30;
	e->accept();
}

