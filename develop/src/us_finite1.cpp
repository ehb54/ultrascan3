#include "../include/us_finite1.h"

US_Finite_W::US_Finite_W(QWidget *p, const char *name) : Data_Control_W(13, p, name)
{
	delete smoothing_lbl;
	delete smoothing_counter;
	smoothing_counter = NULL;
	delete pb_vbar;
	delete vbar_le;
	delete lbl1_excluded;
	delete lbl2_excluded;
	completed = false;
	aborted = false;
	converged = false;
	fitting_widget = false;
	model_widget = false;
	monte_carlo_widget = false;
	autoconverge = true;
	step = 0;
	model_defined = false;
	suspend_flag = false;
	constrained_fit = true;
	model = 0;
	components = 1;
	pm = new US_Pixmap();
	extraCounter = -1;
	
	pb_save->setEnabled(false);
	pb_create_model = new QPushButton(tr("Create new Model"), this);
	Q_CHECK_PTR(pb_create_model);
	pb_create_model->setAutoDefault(false);
	pb_create_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_create_model->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_create_model->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_create_model->setEnabled(false);
	connect(pb_create_model, SIGNAL(clicked()), SLOT(create_model()));

	xpos += buttonw + spacing;

	pb_fit_control = new QPushButton(tr("Fitting Control"), this);
	Q_CHECK_PTR(pb_fit_control);
	pb_fit_control->setAutoDefault(false);
	pb_fit_control->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_fit_control->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_fit_control->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_fit_control->setEnabled(false);
	connect(pb_fit_control, SIGNAL(clicked()), SLOT(select_fit_control()));
	
	xpos = border + buttonw + spacing;
	ypos += buttonh + spacing;

	pb_model_control = new QPushButton(tr("Model Control"), this);
	Q_CHECK_PTR(pb_model_control);
	pb_model_control->setAutoDefault(false);
	pb_model_control->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_model_control->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_model_control->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_model_control->setEnabled(false);
	connect(pb_model_control, SIGNAL(clicked()), SLOT(select_model_control()));

	xpos = border;
	ypos += buttonh + spacing;
	
	pb_load_fit = new QPushButton(tr("Load Fit"), this);
	Q_CHECK_PTR(pb_load_fit);
	pb_load_fit->setAutoDefault(false);
	pb_load_fit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_load_fit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_load_fit->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_load_fit, SIGNAL(clicked()), SLOT(load_fit()));

	xpos += buttonw + spacing;

	pb_save_fit = new QPushButton(tr("Save Fit"), this);
	Q_CHECK_PTR(pb_save_fit);
	pb_save_fit->setAutoDefault(false);
	pb_save_fit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_save_fit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_save_fit->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_save_fit->setEnabled(false);
	connect(pb_save_fit, SIGNAL(clicked()), SLOT(save_fit()));
	
	xpos = border;
	ypos += buttonh + spacing;
//	ypos = 466;

	residuals_lbl = new QLabel(tr(" Residuals:"), this);
	Q_CHECK_PTR(residuals_lbl);
	residuals_lbl->setAlignment(AlignLeft|AlignVCenter);
	residuals_lbl->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	residuals_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	residuals_lbl->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos += buttonw + spacing;

	residuals_counter= new QwtCounter(this);
	Q_CHECK_PTR(residuals_counter);
	residuals_counter->setRange(1, 1, 1);
	residuals_counter->setNumButtons(2);
	residuals_counter->setValue(1);
	residuals_counter->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	residuals_counter->setGeometry(xpos, ypos, buttonw, buttonh);
//	connect(residuals_counter, SIGNAL(buttonReleased(double)), SLOT(update_residuals(double)));
	connect(residuals_counter, SIGNAL(valueChanged(double)), SLOT(update_residuals(double)));


	cp_list.clear();
	rotor_list.clear();
	if (!readCenterpieceInfo(&cp_list))
	{
		QString msg_str = "There was a problem opening the\n"
								"centerpiece database file:\n\n"
								+ USglobal->config_list.system_dir + "/etc/centerpiece.dat\n\n"
								"Please install the centerpiece database file\n"
								"before proceeding.";
		QMessageBox::critical(0, "UltraScan Fatal Error:", msg_str, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		exit(-1);
	}
	if (!readRotorInfo(&rotor_list))
	{
		QString msg_str = "There was a problem opening the\n"
								"rotor database file:\n\n"
								+ USglobal->config_list.system_dir + "/etc/rotor.dat\n\n"
								"Please install the rotor database file\n"
								"before proceeding.";
		QMessageBox::critical(0, "UltraScan Fatal Error:", msg_str, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		exit(-1);
	}
	pb_second_plot->setText(tr("Monte Carlo"));
	pb_second_plot->setEnabled(false);
	
	setup_GUI();
}

US_Finite_W::~US_Finite_W()
{
}

void US_Finite_W::setup_GUI()
{
	int j=0;
	int rows = 13, columns = 4, spacing = 2;
	
	QGridLayout * background = new QGridLayout(this,2,2,spacing);	
	QGridLayout * subGrid1 = new QGridLayout(rows, columns, spacing);
	for (int i=0; i<rows; i++)
	{
		subGrid1->setRowSpacing(i, 26);
	}
	subGrid1->addMultiCellWidget(pb_load,j,j,0,1);
	subGrid1->addMultiCellWidget(pb_details,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(pb_second_plot,j,j,0,1);
	subGrid1->addMultiCellWidget(pb_save,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(pb_print,j,j,0,1);
	subGrid1->addMultiCellWidget(pb_view,j,j,2,3);
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
		
	rows = 11, columns = 4, spacing = 2, j=0;
	QGridLayout * subGrid2 = new QGridLayout(rows, columns, spacing);
	for (int i=0; i<rows; i++)
	{
		subGrid2->setRowSpacing(i, 26);
	}

	subGrid2->addMultiCellWidget(banner2,j,j,0,3);
	j++;
	subGrid2->addWidget(pb_density,j,0);
	subGrid2->addWidget(density_le,j,1);
	subGrid2->addWidget(pb_viscosity,j,2);
	subGrid2->addWidget(viscosity_le,j,3);
	j++;
	subGrid2->addMultiCellWidget(pb_create_model,j,j,0,1);
	subGrid2->addMultiCellWidget(pb_fit_control,j,j,2,3);
	j++;
	subGrid2->addMultiCellWidget(pb_reset,j,j,0,1);
	subGrid2->addMultiCellWidget(pb_model_control,j,j,2,3);
	j++;
	subGrid2->addMultiCellWidget(pb_load_fit,j,j,0,1);
	subGrid2->addMultiCellWidget(pb_save_fit,j,j,2,3);
	j++;
	subGrid2->addMultiCellWidget(residuals_lbl,j,j,0,1);
	subGrid2->addMultiCellWidget(residuals_counter,j,j,2,3);
	j++;
	subGrid2->addMultiCellWidget(range_lbl,j,j,0,1);
	subGrid2->addMultiCellWidget(range_counter,j,j,2,3);
	j++;
	subGrid2->addMultiCellWidget(position_lbl,j,j,0,1);
	subGrid2->addMultiCellWidget(position_counter,j,j,2,3);
	j++;
	subGrid2->addMultiCellWidget(pb_exsingle,j,j,0,1);
	subGrid2->addMultiCellWidget(cnt_exsingle,j,j,2,3);
	j++;
	subGrid2->addMultiCellWidget(pb_exrange,j,j,0,1);
	subGrid2->addMultiCellWidget(cnt_exrange,j,j,2,3);
	j++;
	subGrid2->addWidget(lbl_status,j,0);
	subGrid2->addMultiCellWidget(progress,j,j,1,3);
		
	background->addLayout(subGrid1,0,0);
	background->addWidget(analysis_plot,0,1);
	background->addLayout(subGrid2,1,0);
	background->addWidget(edit_plot,1,1);
	background->setColStretch(0,1);
	background->setColStretch(1,4);
	background->setColSpacing(0,350);
	background->setColSpacing(1,550);

	qApp->processEvents();
	QRect r = background->cellGeometry(0, 0);

	global_Xpos += 30;
	global_Ypos += 30;
	
	this->setGeometry(global_Xpos, global_Ypos, r.width()+550+spacing*3, this->height());

}
void US_Finite_W::updateButtons()
{
	pb_save_fit->setEnabled(false);
	pb_save->setEnabled(false);
	pb_fit_control->setEnabled(false);
	pb_model_control->setEnabled(false);
	pb_create_model->setEnabled(true);
	pb_second_plot->setEnabled(false);	
	pb_view->setEnabled(false);
}

void US_Finite_W::closeEvent(QCloseEvent *e)
{
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
	if (model_widget)
	{
		model_control->close();
	}
	if (fitting_widget)
	{
		fit_control->close();
	}
	if (monte_carlo_widget)
	{
		monte_carlo_window->close();
	}
}

void US_Finite_W::reset()
{
	if (step == 0)
	{
		QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n	   Click on \"Load Data\"\n"));
		return;
	}
	reset_flag = true;
	update_boundary_range(100.0);
	bd_range=100;
	range_counter->setValue(bd_range);
}

// This virtual function doubles as the Monte Carlo calling routine:
void US_Finite_W::second_plot()
{
	QString str;
	mc.parameter.clear();
	mc.fitName = fitName;
	mc.experiment = 2; //velocity (DUD) experiment
	mc.run_id = run_inf.run_id	+ str.sprintf("-%d%d", selected_cell + 1, selected_lambda + 1);
	mc.random_seed = 0;
	mc.iterations = 10000;
	mc.filename = USglobal->config_list.result_dir + "/" + mc.run_id + ".mc";
	mc.append = true;
	mc.status = false;
	mc.rule = 0;	// use residual as standard deviation
	mc.percent_bootstrap = 100;
	mc.data_type = 0;
	mc.percent_gaussian = 50;
	mc.parameters = 1;
	mc.parameterNoise = 8.0;	// add noise to the parameter value with a STDD of 10 percent of absolute parameter value
	mc.varianceThreshold = (float) 1.0e-4;
	mc.addNoise = 0; // use parameters from previous fit as initial guess
	str.sprintf(tr("%d: Variance"), mc.parameters);
	mc.parameter.push_back(str);
	if (run_vector->meniscus_fit)
	{
		mc.parameters++;
		str.sprintf(tr("%d: Meniscus"), mc.parameters);
		mc.parameter.push_back(str);
	}
	if (run_vector->baseline_fit)
	{
		mc.parameters++;
		str.sprintf(tr("%d: Baseline"), mc.parameters);
		mc.parameter.push_back(str);
	}
	if (run_vector->slope_fit)
	{
		mc.parameters++;
		str.sprintf(tr("%d: Linear Slope"), mc.parameters);
		mc.parameter.push_back(str);
	}
	if (run_vector->stray_fit)
	{
		mc.parameters++;
		str.sprintf(tr("%d: Stray Light"), mc.parameters);
		mc.parameter.push_back(str);
	}
	for (unsigned int m=0; m<components; m++)
	{
		if (component_vector[m].conc_fit != 0)
		{
			mc.parameters++;
			str.sprintf(tr("%d: Partial Concentration (%d)"), mc.parameters, m+1);
			mc.parameter.push_back(str);
		}
		if (component_vector[m].sed_fit != 0)
		{
			mc.parameters++;
			str.sprintf(tr("%d: Sedimentation Coefficient (%d)"), mc.parameters, m+1);
			mc.parameter.push_back(str);
		}
		if (component_vector[m].diff_fit != 0)
		{
			mc.parameters++;
			str.sprintf("%d: Diffusion Coefficient (%d)", mc.parameters, m+1);
			mc.parameter.push_back(str);
		}
		if (component_vector[m].sigma_fit != 0)
		{
			mc.parameters++;
			str.sprintf(tr("%d: Sigma (%d)"), mc.parameters, m+1);
			mc.parameter.push_back(str);
		}
		if (component_vector[m].delta_fit != 0)
		{
			mc.parameters++;
			str.sprintf(tr("%d: Delta (%d)"), mc.parameters, m+1);
			mc.parameter.push_back(str);
		}
		if (component_vector[m].mw_fit != 0)
		{
			mc.parameters++;
			str.sprintf(tr("%d: Molecular Weight (%d)"), mc.parameters, m+1);
			mc.parameter.push_back(str);
		}
		if (component_vector[m].vbar_fit != 0)
		{
			mc.parameters++;
			str.sprintf(tr("%d: Partial Spec. Vol. (%d)"), mc.parameters, m+1);
			mc.parameter.push_back(str);
		}
	}
	monte_carlo_window = new US_MonteCarlo(&mc, &monte_carlo_widget);
	connect(monte_carlo_window, SIGNAL(iterate()), SLOT(monte_carlo()));
	monte_carlo_window->show();
}

void US_Finite_W::monte_carlo()
{
	QString str;
	int now = 0;
	unsigned int count = 0, current_point, i, j, k=0;
	QTime t;
	t = QTime::currentTime();
	now = t.msec() + t.second() * 1000 + t.minute() * 60000 + t.hour() * 3600000;
	if (mc.random_seed == 0)
	{
		mc.random_seed = 	now;
	}
#ifndef WIN32
	mc.random_seed -= (int) getpid();
#endif
	str.sprintf(" %d", mc.random_seed);
	monte_carlo_window->le_seed->setText(str);
	residuals_v.clear();
	float **temp_absorbance, **fit_absorbance, sigma = 0, *std_dev;
	temp_absorbance = new float *[run_inf.scans[selected_cell][selected_lambda]];
	fit_absorbance = new float *[run_inf.scans[selected_cell][selected_lambda]];

	for (unsigned int i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		temp_absorbance[i] = new float [run_inf.points[selected_cell][selected_lambda][0]];
		fit_absorbance[i] = new float [run_inf.points[selected_cell][selected_lambda][0]];
	}
	for (unsigned int i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		for (unsigned int j=0; j<run_inf.points[selected_cell][selected_lambda][0]; j++)
		{
			fit_absorbance[i][j] = F[parameters][count];
			temp_absorbance[i][j] = absorbance[i][j];
			residuals_v.push_back(fit_absorbance[i][j] - temp_absorbance[i][j]);
			count++;
		}
	}
	srand(mc.random_seed);	//randomize the box muller function
	if (!fitting_widget)
	{
		select_fit_control();
	}
	
	unsigned int iteration = 1;
	QFile mc_f(mc.filename);
	if (!mc.append)
	{
		mc_f.remove();
	}
	QTextStream ts(&mc_f);
	ts.width(14);
	ts.flags(0x1000);
	if(mc.rule > 2) // then we need to initialize a temporary array with the smoothed SD's
	{
		j = residuals_v.size();
		std_dev = new float [j];
		switch (mc.rule)
		{
			case 3:
			{
				k = 5;
				break;
			}
			case 4:
			{
				k = 10;
				break;
			}
			case 5:
			{
				k = 15;
				break;
			}
			case 6:
			{
				k = 20;
				break;
			}
			case 7:
			{
				k = 25;
				break;
			}
			case 8:
			{
				k = 30;
				break;
			}
		}
		for (i=0; i<j; i++)
		{
			std_dev[i] = fabs(residuals_v[i]);	// only send in the absolute values of the std. deviations
		}
		gaussian_smoothing(&std_dev, k, j);
		for (i=0; i<j; i++)
		{
			if (residuals_v[i] < 0)
			{
				std_dev[i] *= -1.0;	// correct the sign of the residuals after averaging
			}
		}
	}
	while (mc.status && iteration < mc.iterations)
	{
		/* in the finite element analysis, always use the last parameters for initialization
		this will be changed once the fitting routine "dud" has been replaced.
		// re-initialize the parameters with the best-fit result and overwrite the parameters from the
		// last Monte Carlo iteration:
		switch (mc.addNoise)
		{
			case 0:
			{
				// do nothing, use parameters from last fit
				break;
			}
			case 1:
			{
				eqFitter->parameter_addRandomNoise(temp_parameters, mc.parameterNoise);
				break;
			}
			case 2:
			{
				eqFitter->parameter_mapBackward(temp_parameters);
				break;
			}
		}
		*/
		iteration ++;
		str.sprintf(" %d", iteration);
		monte_carlo_window->lbl_current_iteration2->setText(str);

		switch (mc.data_type)
		{
			case 0:	// use normal Gaussian random residuals
			{
				count = 0;
				for (unsigned int j=0; j<run_inf.scans[selected_cell][selected_lambda]; j++)
				{
					for (unsigned int k=0; k<run_inf.points[selected_cell][selected_lambda][0]; k++)
					{
						switch(mc.rule)
						{
							case 0:
							{
								sigma = residuals_v[count];
								break;
							}
							case 1:
							{
								sigma = max(standard_deviation, residuals_v[count]);
								break;
							}
							case 2:
							{
								sigma = standard_deviation;
								break;
							}
						}
						if (mc.rule > 2 && mc.rule < 9)
						{
							sigma = std_dev[count];
						}
						absorbance[j][k] = fit_absorbance[j][k] + box_muller(0, sigma);
						count ++;
					}
				}
				break;
			}
			case 1:	// use original residuals reordered in bootstrapped fashion
			{
				count = 0;
				for (unsigned int j=0; j<run_inf.scans[selected_cell][selected_lambda]; j++)
				{
					for (unsigned int k=0; k<run_inf.points[selected_cell][selected_lambda][0]; k++)
					{

// This algorithm randomizes the order of the residuals of the original fit and applies the absolute
// magnitude of the residuals to different points than where they were originally.

// First, find a random point between the first point of the first scan and the last point of the last scan

						current_point = (unsigned int) (rand()/(RAND_MAX + 1.0) * residuals_v.size());
						
// use the magnitude of the residual of the original fit and add it to the fitted solution:

						absorbance[j][k] = fit_absorbance[j][k] + residuals_v[current_point];

// if the point doesn't fall within the percentage of the bootstrapped points, use the original residual instead:

						if (rand()/(RAND_MAX + 1.0) > (mc.percent_bootstrap/100.0))
						{
							absorbance[j][k] = temp_absorbance[j][k];
						}
						count ++;
					}
				}
				break;
			}
			case 2:	// use a mixture of random Gaussian residuals and bootstrapped residuals:
			{
				count = 0;
				for (unsigned int j=0; j<run_inf.scans[selected_cell][selected_lambda]; j++)
				{
					for (unsigned int k=0; k<run_inf.points[selected_cell][selected_lambda][0]; k++)
					{
						if (rand()/(RAND_MAX + 1.0) > (mc.percent_gaussian/100.0))
						{
							switch(mc.rule)
							{
								case 0:
								{
									sigma = residuals_v[count];
									break;
								}
								case 1:
								{
									sigma = max(standard_deviation, residuals_v[count]);
									break;
								}
								case 2:
								{
									sigma = standard_deviation;
									break;
								}
							}
							if (mc.rule > 2 && mc.rule < 9)
							{
								sigma = std_dev[count];
							}
							absorbance[j][k] = fit_absorbance[j][k] + box_muller(0, sigma);
						}
						else
						{
							current_point = (unsigned int) (rand()/(RAND_MAX + 1.0) * residuals_v.size());
							absorbance[j][k] = fit_absorbance[j][k] + residuals_v[current_point];
							if (rand()/(RAND_MAX + 1.0) > (mc.percent_bootstrap/100.0))
							{
								absorbance[j][k] = temp_absorbance[j][k];
							}
						}
						count ++;
					}
				}
				break;
			}
		}
		dud();
		mc_f.open(IO_WriteOnly | IO_Append);
		ts << tr("Iteration ") << iteration << " (" << mc.random_seed << "): " << variance[parameters] << " ";
		if (run_vector->meniscus_fit)
		{
			ts << run_vector->meniscus << " ";
		}
		if (run_vector->baseline_fit)
		{
			ts << run_vector->baseline << " ";
		}
		if (run_vector->slope_fit)
		{
			ts << run_vector->slope << " ";
		}
		if (run_vector->stray_fit)
		{
			ts << run_vector->stray << " ";
		}
		for (unsigned int m=0; m<components; m++)
		{
			if (component_vector[m].conc_fit != 0)
			{
				ts << component_vector[m].conc << " ";
			}
			if (component_vector[m].sed_fit != 0)
			{
				ts << component_vector[m].s20w << " ";
			}
			if (component_vector[m].diff_fit != 0)
			{
				ts << component_vector[m].d20w << " ";
			}
			if (component_vector[m].sigma_fit != 0)
			{
				ts << component_vector[m].sigma << " ";
			}
			if (component_vector[m].delta_fit != 0)
			{
				ts << component_vector[m].delta << " ";
			}
			if (component_vector[m].mw_fit != 0)
			{
				ts << component_vector[m].mw << " ";
			}
			if (component_vector[m].vbar_fit != 0)
			{
				ts << component_vector[m].vbar << " ";
			}
		}
		ts << endl;
		mc_f.flush();
		mc_f.close();
		if(monte_carlo_widget)
		{
			monte_carlo_window->update_iteration();
		}
	}
	monte_carlo_window->pb_start->setEnabled(true);
	monte_carlo_window->pb_stop->setEnabled(false);
	mc.random_seed = 0;
	str.sprintf(" %d", mc.random_seed);
	monte_carlo_window->le_seed->setText(str);
	for (unsigned int k=0; k<run_inf.scans[selected_cell][selected_lambda]; k++)
	{
		delete [] temp_absorbance[k];
	}
	delete [] temp_absorbance;
}

void US_Finite_W::select_model_control()
{
	if (model_widget)
	{
		if (model_control->isVisible())
		{
			model_control->raise();
		}
		else
		{
			model_control->show();
		}
		return;
	}
	if ((!model_defined) && (step == 0))
	{
		QMessageBox::message(tr("Attention:\n"),tr("First, you need to load some data\n"
														"and then define a model!\n\n"
														"Click on \"Load Data\"\n"
														"and then on \"Create Model\""));
		return;
	}
	if (!model_defined)
	{
		QMessageBox::message(tr("Attention:\n"),tr("Please create a Model first!\n\n"
														"Click on \"Create Model\""));
		return;
	}
	vbar_model.clear();
	for (i=0; i<components; i++) // in case there is only one peptide defined, all components should have the same vbar
	{
		vbar_model.push_back(Vbar20[selected_cell][selected_channel][0]);
	}
	for (i=0; i<components; i++)
	{
		if ( i>0 && fabs(Vbar20[selected_cell][selected_channel][i] - 0.72) > 1e-6)
		{
			vbar_model[i] = Vbar20[selected_cell][selected_channel][i];
		}
	}
	model_id.sprintf(tr(modelString[model]));
	model_control = new US_VelocModelControl(&model_widget, components, model, &component_vector, &run_vector, 
	&viscosity_tb, &density_tb, run_inf, selected_cell, selected_lambda, selected_channel, vbar_model);
	model_control->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	model_control->show();
}

void US_Finite_W::select_fit_control()
{
	if (fitting_widget)
	{
		if (fit_control->isVisible())
		{
			fit_control->raise();
		}
		else
		{
			fit_control->show();
		}
		return;
	}
	if ((!model_defined) && (step == 0))
	{
		QMessageBox::message(tr("Attention:\n"),tr("First, you need to load some data\n"
														"and then define a model!\n\n"
														"Click on \"Load Data\"\n"
														"and then on \"Create Model\""));
		return;
	}
	if (!model_defined)
	{
		QMessageBox::message(tr("Attention:\n"),tr("Please create a Model first!\n\n"
														"Click on \"Create Model\""));
		return;
	}
	fit_control = new US_Fefit_FitControl(&constrained_fit, &fitting_widget, &autoconverge);
	fit_control->setCaption(tr("Fitting Control"));
	fit_control->lbl_header->setText(tr("Finite Element Fitting Control"));
	connect(fit_control->pb_fit, SIGNAL(clicked()), SLOT(dud()));
	connect(fit_control->pb_pause, SIGNAL(clicked()), SLOT(suspend_fit()));
	connect(fit_control->pb_resume, SIGNAL(clicked()), SLOT(resume_fit()));
	connect(fit_control->pb_cancel, SIGNAL(clicked()), SLOT(cancel_fit()));
	fit_control->show();
}

void US_Finite_W::suspend_fit()
{
	suspend_flag = true;
	model_control->cb_meniscus_float->setEnabled(false);
	model_control->cb_meniscus_fix->setEnabled(false);
	model_control->cb_baseline_float->setEnabled(false);
	model_control->cb_baseline_fix->setEnabled(false);
	model_control->cb_slope_float->setEnabled(false);
	model_control->cb_slope_fix->setEnabled(false);
	model_control->cb_stray_float->setEnabled(false);
	model_control->cb_stray_fix->setEnabled(false);
	model_control->cb_sed_float->setEnabled(false);
	model_control->cb_sed_fix->setEnabled(false);
	model_control->cb_diff_float->setEnabled(false);
	model_control->cb_diff_fix->setEnabled(false);
	model_control->cb_conc_float->setEnabled(false);
	model_control->cb_conc_fix->setEnabled(false);
	model_control->cb_sigma_float->setEnabled(false);
	model_control->cb_sigma_fix->setEnabled(false);
	model_control->cb_delta_float->setEnabled(false);
	model_control->cb_delta_fix->setEnabled(false);
}

void US_Finite_W::resume_fit()
{
	suspend_flag = false;
	model_control->cb_meniscus_float->setEnabled(true);
	model_control->cb_meniscus_fix->setEnabled(true);
	model_control->cb_baseline_float->setEnabled(true);
	model_control->cb_baseline_fix->setEnabled(true);
	model_control->cb_slope_float->setEnabled(true);
	model_control->cb_slope_fix->setEnabled(true);
	model_control->cb_stray_float->setEnabled(true);
	model_control->cb_stray_fix->setEnabled(true);
	model_control->cb_sed_float->setEnabled(true);
	model_control->cb_sed_fix->setEnabled(true);
	model_control->cb_diff_float->setEnabled(true);
	model_control->cb_diff_fix->setEnabled(true);
	model_control->cb_conc_float->setEnabled(true);
	model_control->cb_conc_fix->setEnabled(true);
	model_control->cb_sigma_float->setEnabled(true);
	model_control->cb_sigma_fix->setEnabled(true);
	model_control->cb_delta_float->setEnabled(true);
	model_control->cb_delta_fix->setEnabled(true);
}

void US_Finite_W::cancel_fit()
{
	if (aborted || converged)
	{
		fit_control->cancel();
	}
	else
	{	
		suspend_flag = true;
		fit_control->pgb_progress->reset();
		fit_control->pb_cancel->setText(tr("Close"));
		fit_control->lbl_iteration2->setText("0");
		fit_control->lbl_variance2->setText("0");
		fit_control->lbl_variance3->setText("0");
		fit_control->lbl_stddev2->setText("0");
		fit_control->lbl_difference2->setText("0");
		fit_control->lbl_evaluations2->setText("0");
		fit_control->lbl_status2->setText(tr("Aborted"));
		fit_control->lbl_status3->setText("");
		fit_control->lbl_status4->setText("");
		aborted = true;
	}
}

void US_Finite_W::create_model()
{
	if (step == 0)
	{
		QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n	   Click on \"Load Data\"\n"));
		return;
	}
	if (run_inf.wavelength[selected_cell][selected_lambda] == 999)
	{
		QMessageBox mb(tr("UltraScan - Attention:"),tr("This is simulated data!\n\n"
							"It was generated with the finite element\n"
							"method in the	in the first place. You really\n"
							"could just check the corresponding model\n"
							"for the run parameters.\n\n"
							"Do you still want to fit this data?"),
		QMessageBox::Information,
		QMessageBox::Yes,
		QMessageBox::No | QMessageBox::Default, 0);
		mb.setButtonText(QMessageBox::Yes, "Yes");
		mb.setButtonText(QMessageBox::No, "Cancel");
		if (mb.exec() == QMessageBox::No)
		{
			return;
		}
	}
	run_vector = new fefit_run;
	US_SelectModel *SelectModel;
	SelectModel = new US_SelectModel(&model, false);
	SelectModel->setCaption("Velocity Model Selection");
	if (SelectModel->exec())
	{
		switch (model)
		{
			case 0:
			{
				model_id.sprintf(tr("Non-Interacting, 1 Component"));
				components = 1;
				start_fit();
				break;
			}
			case 1:
			{
				model_id.sprintf(tr("Non-Interacting, 2 Component"));
				components = 2;
				start_fit();
				break;
			}
			case 2:
			{
				model_id.sprintf(tr("Non-Interacting, 3 Component"));
				components = 3;
				
				start_fit();
				break;
			}
			case 3:
			{
				model_id.sprintf(tr("Fixed Molecular Weight Distribution"));
				US_SelectModel3 *SelectModel3;
				SelectModel3 = new US_SelectModel3(&components);
				if (!SelectModel3->exec())
				{
					model = 0;
					return;
				}
				start_fit();
				break;
			}
			/*
			case 4:
			{
				components = 2;
				model_id.sprintf(tr("Monomer-Dimer"));
				start_fit();
				break;
			}
			case 5:
			{
				components = 2;
				model_id.sprintf(tr("Monomer-Trimer"));
				start_fit();
				break;
			}
			case 6:
			{
				components = 2;
				model_id.sprintf(tr("Monomer-Tetramer"));
				start_fit();
				break;
			}
			case 7:
			{
				components = 2;
				model_id.sprintf(tr("Monomer-Pentamer"));
				start_fit();
				break;
			}
			case 8:
			{
				components = 2;
				model_id.sprintf(tr("Monomer-Hexamer"));
				start_fit();
				break;
			}
			case 9:
			{
				components = 2;
				model_id.sprintf(tr("Monomer-Heptamer"));
				start_fit();
				break;
			}
			case 10:
			{
				components = 2;
				model_id.sprintf(tr("User-defined Monomer-N-mer"));
				start_fit();
				break;
			}
			case 11:
			{
				components = 3;
				model_id.sprintf(tr("Monomer-Dimer-Trimer"));
				start_fit();
				break;
			}
			case 12:
			{
				components = 3;
				model_id.sprintf(tr("Monomer-Dimer-Tetramer"));
				start_fit();
				break;
			}
			case 13:
			{
				components = 3;
				model_id.sprintf(tr("User-defined Monomer-N-mer-M-mer"));
				start_fit();
				break;
			}
			// 14, 15m for heteroassociation models
			case 16:
			{
				components = 3;
				model_id.sprintf(tr("User-defined Monomer-N-mer with some incompetent Monomer"));
				start_fit();
				break;
			}
			case 17:
			{
				components = 3;
				model_id.sprintf(tr("User-defined Monomer-N-mer with some incompetent N-mer"));
				start_fit();
				break;
			}
			case 18:
			{
				components = 2;
				model_id.sprintf(tr("User-defined irreversible Monomer-N-mer"));
				start_fit();
				break;
			}
			case 19:
			{
				components = 3;
				model_id.sprintf(tr("User-defined Monomer-N-mer with contaminant"));
				start_fit();
				break;
			}
			*/
			default:
			{
				QMessageBox::message(tr("UltraScan - Attention:"),
											tr("This model is currently not available in this module.\n"
												"Please only select models 1-4 from the above choices."));
				model = 0;
				return;
			}
		}
	}
	else // use single component ideal system as default
	{
		model = 0;
	}
	pb_fit_control->setEnabled(true);
	pb_model_control->setEnabled(true);
}

void US_Finite_W::setup_model()
{
	run_vector = new fefit_run;
	switch (model)
	{
		case 0:
		{
			model_id.sprintf(tr("Non-Interacting, 1 Component"));
			components = 1;
			start_fit();
			break;
		}
		case 1:
		{
			model_id.sprintf(tr("Non-Interacting, 2 Component"));
			components = 2;
			start_fit();
			break;
		}
		case 2:
		{
			model_id.sprintf(tr("Non-Interacting, 3 Component"));
			components = 3;
			start_fit();
			break;
		}
		case 3:
		{
			model_id.sprintf(tr("Fixed Molecular Weight Distribution"));
			US_SelectModel3 *SelectModel3;
			SelectModel3 = new US_SelectModel3(&components);
			if (!SelectModel3->exec())
			{
				model = 0;
				break;
			}
			start_fit();
			break;
		}
		/*
		case 4:
		{
			components = 2;
			model_id.sprintf(tr("Monomer-Dimer"));
			start_fit();
			break;
		}
		*/
		default:
		{
			QMessageBox::message(tr("UltraScan - Attention:"),
										tr("This model is currently not available in this module.\n"
											"Please only select models 1-4 from the above choices."));
			model = 0;
			return;
		}
	}
	pb_fit_control->setEnabled(true);
	pb_model_control->setEnabled(true);
}

void US_Finite_W::start_fit()
{
	unsigned int i;
	struct fefit_component temp_component;
	component_vector.clear();
	temp_component.conc = 0.0;
	temp_component.sed = 0.0;
	temp_component.s20w = 0.0;
	temp_component.diff = 0.0;
	temp_component.d20w = 0.0;
	temp_component.sigma = 0.0;
	temp_component.delta = 0.0;
	temp_component.mw = 0.0;
	temp_component.vbar = 0.0;
	temp_component.conc_range = 0.0;
	temp_component.sed_range = 0.0;
	temp_component.diff_range = 0.0;
	temp_component.sigma_range = 0.0;
	temp_component.delta_range = 0.0;
	temp_component.conc_fit = 0;
	temp_component.sed_fit = 0;
	temp_component.diff_fit = 0;
	temp_component.sigma_fit = 0;
	temp_component.delta_fit = 0;
	temp_component.mw_fit = 0;
	temp_component.vbar_fit = 0;
	vbar_model.clear();
	for (i=0; i<components; i++) // in case there is only one peptide defined, all components should have the same vbar
	{
		vbar_model.push_back(Vbar20[selected_cell][selected_channel][0]);
	}
	for (i=0; i<components; i++)
	{
		if ( i>0 && fabs(Vbar20[selected_cell][selected_channel][i] - 0.72) > 1e-6)
		{
			vbar_model[i] = Vbar20[selected_cell][selected_channel][i];
		}
		component_vector.push_back(temp_component);
	}
	if (run_type == 1) // we already subtracted the baseline from the scans during loading for absorbance velocity runs
	{
		run_inf.baseline[selected_cell][selected_lambda] = 0.0; 
	}
	model_control = new US_VelocModelControl(&model_widget, components, model, &component_vector, &run_vector, 
	&viscosity_tb, &density_tb, run_inf, selected_cell, selected_lambda, selected_channel, vbar_model);
	model_control->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	model_control->show();
	model_defined = true;
}

void US_Finite_W::update_residuals(double val)
{
	if (step == 0)
	{
		QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n	   Click on \"Load Data\"\n"));
		return;
	}
	if (!converged)
	{
		QMessageBox::message(tr("Attention:\n"),tr("There are no residuals to display...\n\n"
														"Please fit your data first until it is\n"
														"converged before trying to display results.\n"));
		return;
	}
	unsigned int first_resid = (unsigned int) val;
	unsigned int i, num_scans, linecount;
	QString s1, s2, s3;
	first_plot = false;	//we already did the first plot, now we need to keep it off.
	analysis_plot->clear();
	if ((unsigned int) run_inf.scans[selected_cell][selected_lambda] - first_resid == 0)
	{
		num_scans = 1;
		s1.sprintf(tr(": Cell %d - Residuals for scan %d"), selected_cell+1, first_resid);
	}
	else if (run_inf.scans[selected_cell][selected_lambda] - first_resid < 5)
	{
		num_scans = run_inf.scans[selected_cell][selected_lambda] - first_resid + 1;
		s1.sprintf(tr(": Cell %d - Residuals from scans %d - %d"), selected_cell+1, first_resid,
		run_inf.scans[selected_cell][selected_lambda]);
	}
	else
	{
		num_scans = 5;
		s1.sprintf(tr(": Cell %d - Residuals from scans %d - %d"), selected_cell+1, first_resid, first_resid+4);
	}
	s2 = tr("Run ");
	s2.append(run_inf.run_id);
	s2.append(s1);
	s3.sprintf(tr("Delta OD (%d nm)"), run_inf.wavelength[selected_cell][selected_lambda]);
	analysis_plot->setTitle(s2);
	analysis_plot->setAxisTitle(QwtPlot::xBottom, tr("Radius in cm"));
	analysis_plot->setAxisTitle(QwtPlot::yLeft, s3);

	line_x[0] = radius[0]-0.02;
	line_x[1] = radius[points-1]+0.02;
	linecount = 0;
	for (i=(unsigned int) first_resid-1; i<(unsigned int) first_resid+num_scans-1; i++)
	{
		line[linecount]  = analysis_plot->insertCurve("Baselines");
		curve[i] = analysis_plot->insertCurve("Residuals");
		analysis_plot->setCurvePen(curve[i], green);
		analysis_plot->setCurveData(curve[i], radius, residuals[i], points);
		analysis_plot->setCurvePen(line[linecount], white);
		analysis_plot->setCurveData(line[linecount], line_x, line_y[i], 2);
		linecount++;
	}
	if (constrained_fit)
	{
		update_simulation_parameters_constrained(parameters);
	}
	else
	{
		update_simulation_parameters_unconstrained(parameters);
	}
	analysis_plot->replot();
	//analysis_plot->updatePlot();		//no updatePlot() in new version
	update_plot(first_resid);	//need to paint the proper scans in red.
}

void US_Finite_W::save()
{
	if (step == 0)
	{
		QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n	   Click on \"Load Data\"\n"));
		return;
	}
	if (!converged)
	{
		QMessageBox::message(tr("Attention:\n"),tr("     There is nothing to save...\n\n"
														"Please fit your data first until it is\n"
														"converged before trying to save the results.\n"));
		return;
	}
	write_fef();
	write_res();
/*
	QPixmap p;
	QString fileName;
	for (unsigned int i=0; i<run_inf.scans[selected_cell][selected_lambda]; i+=5)
	{
		update_residuals((double) i+1);
		qApp->processEvents();
		fileName.sprintf(htmlDir + "/fef_%d%d_%d.", selected_cell + 1, selected_lambda + 1, i+1);
		p = QPixmap::grabWidget(analysis_plot, 2, 2, analysis_plot->width() - 4, analysis_plot->height() - 4);
		pm->save_file(fileName, p);
		fileName.sprintf(htmlDir + "/fef_edited_%d%d_%d.", selected_cell + 1, selected_lambda +1, i+1);
		p = QPixmap::grabWidget(edit_plot, 2, 2, edit_plot->width() - 4, edit_plot->height() - 4);
		pm->save_file(fileName, p);
	}
*/
}

void US_Finite_W::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/finite.html");
}

int US_Finite_W::plot_analysis()
{
	return(0);
}

void US_Finite_W::dud()
{
	QString str, str1;
	if (converged || completed || aborted)
	{
		cleanup_dud();
	}

	first_plot = true;		// reset first plot each time we do a new fit
	completed = false;
	aborted = false;
	converged = false;
	fit_control->pb_cancel->setText(tr("Abort"));
	func_eval = 0;
	float improvement=0, d, d_start=0.5;
	unsigned int i, j, loopcount = 0;
	init_simulation = false;
	dud_init();		// initialize the basis vectors
	F_init();		// initialize the corresponding function vectors
	if (aborted)
	{
		suspend_flag = false;
		return;
	}
	order_variance();			// sort the variances - theta causing largest goes into first element, theta causing best variance goes into parameters+1
	while (!aborted)
	{
		for (i=0; i<parameters; i++)
		{
			calc_deltas(i);
		}
		if (!calc_alpha())
		{
			QMessageBox::message(tr("Attention:"), tr("The Cholesky Decomposition of the\n"
														  "Hessian matrix failed due to a\n"
														  "singularity in the matrix.\n\n"
														  "You can probably achieve convergence\n"
														  "by re-fitting with the current\n"
														  "parameter estimate as initial guess."));
			aborted = true;
//			update_plot(1);
			fit_control->lbl_status2->setText(tr("Fit aborted..."));
			fit_control->lbl_status3->setText("");
			fit_control->lbl_status4->setText("");
			fit_control->pb_cancel->setText(tr("Close"));
			return;
		}

// update theta. We don't know how good the new guess is, so we temporarily place this
// guess for theta into the extra theta vector (p+2 = [parameters+1]).
// Then we calculate the residuals and reorder afterwards:

		for (i=0; i<parameters; i++)
		{
			theta[parameters+1][i] = 0.0;
			for (j=0; j<parameters; j++)
			{
				theta[parameters+1][i] += delta_theta[j][i] * alpha[j];
			}
			theta[parameters+1][i] += theta[parameters][i];
		}
		d = (float) 0.1;
		for (i=0; i<parameters; i++)
		{
			theta[parameters+1][i] = d * theta[parameters+1][i] + (1.0 - d) * theta[parameters][i];
		}
		iteration ++;
		str.sprintf("%d",iteration);
		fit_control->lbl_iteration2->setText(str);
		str.sprintf(tr("Working on Iteration %d"), iteration);
		fit_control->lbl_status3->setText(str);
		if (constrained_fit)
		{
			update_simulation_parameters_constrained(parameters+1);
		}
		else
		{
			update_simulation_parameters_unconstrained(parameters+1);
		}
		if (model >= 0 && model <= 3)
		{
			str.sprintf(tr("Fitting Non-Interacting Model..."));
			fit_control->lbl_status2->setText(str);
			completed = false;
			while (!completed)
			{
				if (non_interacting_model() != 0)
				{
					QMessageBox::message("Attention:", 
					"The finite element simulation failed.\n"
					"Please check the initialization\n"
					"parameters, and try again after\n"
					"adjusting the parameters to different\n"
					"values... Fit was aborted.");
					qApp->processEvents();
					return;
				}
				while (suspend_flag)
				{
					qApp->processEvents();
					if (aborted)
					{
						suspend_flag = false;
						return;
					}
				}
			}
		}
		if (model == 4)
		{
			str.sprintf(tr("Fitting Monomer-Dimer Model..."));
			fit_control->lbl_status2->setText(str);
			completed = false;
			while (!completed)
			{
				monomer_dimer_model();
				while (suspend_flag)
				{
					qApp->processEvents();
					if (aborted)
					{
						suspend_flag = false;
						return;
					}
				}
			}
		}
		if (model == 20)
		{
			str.sprintf(tr("Fitting Isomerizing Model..."));
			fit_control->lbl_status2->setText(str);
			completed = false;
			while (!completed)
			{
				isomerizing_model();
				while (suspend_flag)
				{
					qApp->processEvents();
					if (aborted)
					{
						suspend_flag = false;
						return;
					}
				}
			}
		}
		assign_F(parameters+1);
		calc_residuals(parameters+1);
		improvement =  variance[parameters] - variance[parameters+1];
		str.sprintf("%1.4e", variance[parameters+1]);
		fit_control->lbl_variance2->setText(str);
		if (improvement > 0.0 )
		{
			fit_control->lbl_variance3->setText(str);
		}
		str.sprintf("%1.4e", pow((double) variance[parameters+1], (double) 0.5));
		fit_control->lbl_stddev2->setText(str);
		str.sprintf("%1.4e", improvement);
		fit_control->lbl_difference2->setText(str);
		loopcount = 0;
		while (improvement <= 0.0)		// we need a step shortening procedure here
		{	//implement step shortening procedure
			str.sprintf(tr("Implementing Step-shortening..."));
			fit_control->lbl_status2->setText(str);
			loopcount++;
			str.sprintf(tr("Working on loop %d"), loopcount);
			fit_control->lbl_status3->setText(str);
			d = (-1.0) * pow((double) -d_start, (double) loopcount);
			for (i=0; i<parameters; i++)
			{
				theta[parameters+1][i] = d * theta[parameters+1][i] + (1.0 - d) * theta[parameters][i];
			}
			if (constrained_fit)	// =different GUI's depending on this flag
			{
				update_simulation_parameters_constrained(parameters+1);
			}
			else
			{
				update_simulation_parameters_unconstrained(parameters+1);
			}
			if (model >= 0 && model <= 3)
			{
				completed = false;
				while (!completed)
				{
					if (non_interacting_model() != 0)
					{
						QMessageBox::message("Attention:", 
						"The finite element simulation failed.\n"
						"Please check the initialization\n"
						"parameters, and try again after\n"
						"adjusting the parameters to different\n"
						"values... Fit was aborted.");
						qApp->processEvents();
						return;
					}
					while (suspend_flag)
					{
						qApp->processEvents();	//take care of piled up events
						if (aborted)
						{
							suspend_flag = false;
							return;
						}
					}
				}
			}
			if (model == 4)
			{
				completed = false;
				while (!completed)
				{
					monomer_dimer_model();		// calculate
					while (suspend_flag)
					{
						qApp->processEvents();	//take care of piled up events
						if (aborted)
						{
							suspend_flag = false;
							return;
						}
					}
				}
			}
			if (model == 20)
			{
				completed = false;
				while (!completed)
				{
					isomerizing_model();			// calculate
					while (suspend_flag)
					{
						qApp->processEvents();	//take care of piled up events
						if (aborted)
						{
							suspend_flag = false;
							return;
						}
					}
				}
			}
			assign_F(parameters+1);
			calc_residuals(parameters+1);
			improvement =  variance[parameters] - variance[parameters+1];
			str.sprintf("%1.4e", variance[parameters+1]);
			fit_control->lbl_variance2->setText(str);
			if (improvement > 0 )
			{
				fit_control->lbl_variance3->setText(str);
			}
			str.sprintf("%1.4e", pow((double) variance[parameters+1], (double) 0.5));
			fit_control->lbl_stddev2->setText(str);
			str.sprintf("%1.4e", improvement);
			fit_control->lbl_difference2->setText(str);
			if (improvement == 0 || loopcount > 5)
			{
				if(iteration > 1 && autoconverge)
				{
					iteration = 0;
					loopcount = 0;
					improvement = 1.0;
					model_control->reset_range();
				}
				else
				{
					converged = true;
					update_plot(1);	//show the first set of residuals
					fit_control->lbl_status2->setText(tr("Solution converged."));
					fit_control->lbl_status3->setText("");
					fit_control->lbl_status4->setText("");
					fit_control->pb_cancel->setText(tr("Close"));
					pb_save_fit->setEnabled(true);
					pb_save->setEnabled(true);
					pb_view->setEnabled(true);
					cleanup_finite();
					return; // we can't do any better, quit (this is the exit of this function)
				}
			}
		}
		if (alpha[0] >= 1.0e-5)
		{
			shift_all();
		}
		else
		{
			j = 0;
			while ((alpha[j] < 1.0e-5) && (j < parameters-1))
			{
				j ++; // find first alpha[j] that is larger/equal than 1e-5 and use that index's alpha to replace with theta new
			}
			str.sprintf(tr("Updating vectors 1 and %d"), j+1);
			fit_control->lbl_status2->setText(str);
			loopcount++;
			for (i=0; i<parameters; i++) 
			{ // make sure that old values of theta1 aren't retained forever
				theta[0][i] = (theta[0][i] + theta[parameters+1][i]) / 2.0;
				theta[j][i] = theta[parameters+1][i]; // update theta[j] with the new theta.
				variance[j] = variance[parameters+1];
			} // and replace with the average of theta1 and theta_new
			for (i=0; i<allpoints; i++)
			{
				F[j][i] = F[parameters+1][i];	// we already have the F[j] from above, no need to re-calculate
			}
			str.sprintf(tr("Working on vector 1")); // we still need to calculate theta[0], since that one is new
			fit_control->lbl_status3->setText(str);
			if (constrained_fit)
			{
				update_simulation_parameters_constrained(0);
			}
			else
			{
				update_simulation_parameters_unconstrained(0);
			}
			if (model >= 0 && model <= 3)
			{
				completed = false;
				while (!completed)
				{
					if (non_interacting_model() != 0)
					{
						QMessageBox::message("Attention:", 
						"The finite element simulation failed.\n"
						"Please check the initialization\n"
						"parameters, and try again after\n"
						"adjusting the parameters to different\n"
						"values... Fit was aborted.");
						qApp->processEvents();
						return;
					}
					while (suspend_flag)
					{
						qApp->processEvents();	//take care of piled up events
						if (aborted)
						{
							suspend_flag = false;
							return;
						}
					}
				}
			}
			if (model == 4)
			{
				completed = false;
				while (!completed)
				{
					monomer_dimer_model();		// calculate
					while (suspend_flag)
					{
						qApp->processEvents();	//take care of piled up events
						if (aborted)
						{
							suspend_flag = false;
							return;
						}
					}
				}
			}
			if (model == 20)
			{
				completed = false;
				while (!completed)
				{
					isomerizing_model();			// calculate
					while (suspend_flag)
					{
						qApp->processEvents();	//take care of piled up events
						if (aborted)
						{
							suspend_flag = false;
							return;
						}
					}
				}
			}
			assign_F(0);
			calc_residuals(0);
			improvement =  variance[parameters+1] - variance[0]; // the best one we have so far is theta[parameters+1]
			str.sprintf("%1.4e", variance[0]);
			fit_control->lbl_variance2->setText(str);
			if (improvement > 0 )
			{
				fit_control->lbl_variance3->setText(str);
			}
			str.sprintf("%1.4e", pow((double) variance[0], (double) 0.5));
			fit_control->lbl_stddev2->setText(str);
			str.sprintf("%1.4e", improvement);
			fit_control->lbl_difference2->setText(str);
			str.sprintf(tr("Working on vector %d"), j+1);
			fit_control->lbl_status3->setText(str);
			order_variance();
		}
	}
}

void US_Finite_W::update_plot(const unsigned int first_residual)
{
	unsigned int i, j, count, last_red;
	unsigned int *curve_abs;
	int current_sign = 0, last_sign = 0;
	float diff;
	curve_abs = new unsigned int [run_inf.scans[selected_cell][selected_lambda]];
	edit_plot->clear();
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		curve_abs[i] = edit_plot->insertCurve("Optical Density");
		edit_plot->setCurvePen(curve_abs[i], yellow);
		edit_plot->setCurveData(curve_abs[i], radius, absorbance[i], points);
	}

	if(first_plot) //if first plot, we need to calculate the residuals
	{
		run_vector->pos = 0;
		run_vector->neg = 0;
		run_vector->runs = 0;
		count = 0;
		for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
		{
			line_y[i][0] = i*0.03;
			line_y[i][1] = i*0.03;
			for (j=0; j<points; j++)
			{
				y[i][j] = F[parameters][count];
				residuals[i][j] = absorbance[i][j] - F[parameters][count] + i*0.03;
				diff = absorbance[i][j] - F[parameters][count];
				if (diff > 0)
				{
					run_vector->pos++;
					current_sign = 1;
				}
				else
				{
					run_vector->neg++;
					current_sign = -1;
				}
				if (j == 0)
				{
					if (diff > 0)
					{
						last_sign = 1;
					}
					else
					{
						last_sign = -1;
					}
				}
				else
				{
					if (last_sign != current_sign)
					{
						run_vector->runs++;
						last_sign = current_sign;
					}
				}
				count++;
			}
		}
		if (constrained_fit)
		{
			update_simulation_parameters_constrained(parameters);
		}
		else
		{
			update_simulation_parameters_unconstrained(parameters);
		}
	}
	if ((run_inf.scans[selected_cell][selected_lambda] - first_residual) < 5)
	{
		last_red = run_inf.scans[selected_cell][selected_lambda];
	}
	else
	{
		last_red = first_residual+4;
	}
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		curve[i] = edit_plot->insertCurve("Optical Density");
		if ((i >= first_residual - 1) && (i < last_red))
		{
			edit_plot->setCurvePen(curve[i], red);
		}
		else
		{
			edit_plot->setCurvePen(curve[i], cyan);
		}
		edit_plot->setCurveData(curve[i], radius, y[i], points);
	}
	edit_plot->replot();
	//edit_plot->updatePlot();		//no updatePlot() in new version
	if (first_plot)
	{
		unsigned int modulus = run_inf.scans[selected_cell][selected_lambda] % 5;
		switch (modulus)
		{
			case 0:
			{
				residuals_counter->setRange(1, run_inf.scans[selected_cell][selected_lambda]-4, 5);
				break;
			}
			case 1:
			{
				residuals_counter->setRange(1, run_inf.scans[selected_cell][selected_lambda], 5);
				break;
			}
			case 2:
			{
				residuals_counter->setRange(1, run_inf.scans[selected_cell][selected_lambda]-1, 5);
				break;
			}
			case 3:
			{
				residuals_counter->setRange(1, run_inf.scans[selected_cell][selected_lambda]-2, 5);
				break;
			}
			case 4:
			{
				residuals_counter->setRange(1, run_inf.scans[selected_cell][selected_lambda]-3, 5);
				break;
			}
		}
		residuals_counter->setValue(1);
		update_residuals(1);
	}
	delete [] curve_abs;
}

void US_Finite_W::shift_all()
{
	unsigned int i,j;
	for (i=1; i<parameters+2; i++)
	{
		for (j=0; j<parameters; j++)
		{
			theta[i-1][j] = theta[i][j];
		}
	}
	for (i=1; i<parameters+2; i++)
	{
		for (j=0; j<allpoints; j++)
		{
			F[i-1][j] = F[i][j];
		}
	}
	for (i=1; i<parameters+2; i++)
	{
		variance[i-1] = variance[i];
	}
}

bool US_Finite_W::calc_alpha()
{
	double **temp1_matrix, **temp2_matrix, *temp1_vector, *temp2_vector;
	unsigned int i, j, k, count;
	
	temp1_matrix = new double* [parameters];
	temp2_matrix = new double* [parameters];
	temp1_vector = new double  [allpoints];
	temp2_vector = new double  [parameters];
	for (i=0; i<parameters; i++)
	{
		temp1_matrix[i] = new double [parameters];
		temp2_matrix[i] = new double [parameters];
	}
	
	count = 0;
	for (j=0; j<run_inf.scans[selected_cell][selected_lambda]; j++)
	{
		for (k=0; k<points; k++)
		{
			temp1_vector[count] = absorbance[j][k] - F[parameters][count];	// y - f(p+1)
			count++;
		}
	}
	for (i=0; i<parameters; i++)
	{
		temp2_vector[i] = 0.0;
		count = 0;
		for (j=0; j<run_inf.scans[selected_cell][selected_lambda]; j++)
		{
			for (k=0; k<points; k++)
			{
				temp2_vector[i] += delta_F[i][count] * temp1_vector[count];
				count++;
			}
		}
		
// calculate delta_F(transpose) * delta_F, hold temporary result in temp1_matrix:

		for (j=0; j<parameters; j++)
		{
			temp1_matrix[i][j] = 0.0;
			for (k=0; k<allpoints; k++)
			{
				temp1_matrix[i][j] += delta_F[i][k] * delta_F[j][k];
			}
		}
	}

// invert delta_F(transpose) * delta_F using Cholesky Decomposition, hold inverse in temp2_matrix:
// if the cholesky decomposition fails, we need to start over with the last best estimate for theta

	if (!Cholesky_Invert(temp1_matrix, temp2_matrix, parameters))
	{
		return(false);
	}

// multiply the inverse matrix by the temp_vector to obtain alpha:

	for (i=0; i<parameters; i++)
	{
		alpha[i] = 0.0;
		for (j=0; j<parameters; j++)
		{
			alpha[i]	+= temp2_matrix[i][j] * temp2_vector[j];
		}
	}
	for (i=0; i<parameters; i++)
	{
		delete [] temp1_matrix[i];
		delete [] temp2_matrix[i];
	}
	delete [] temp1_matrix;
	delete [] temp2_matrix;
	delete [] temp1_vector;
	delete [] temp2_vector;
	return(true);
}

void US_Finite_W::calc_deltas(const unsigned int i)
{
	unsigned int j;
	for (j=0; j<parameters; j++)
	{
		delta_theta[i][j] = theta[i][j] - theta[parameters][j];
	}

	for (j=0; j<allpoints; j++)
	{
		delta_F[i][j] = F[i][j] - F[parameters][j];
//cout << delta_F[i][j] << "\n";
	}
}

void US_Finite_W::calc_residuals(const unsigned int i)
{

// if only a portion of the boundary is used to calculate residuals then the 

	unsigned int j, k, count=0, subcount=0;
	float start_y,stop_y;
	variance[i] = 0.0;
	if (fabs(bd_range - 100) < .01)  // exact equality with a double is unlikely
	{
		for (j=0; j<run_inf.scans[selected_cell][selected_lambda]; j++)
		{
			for (k=0; k<points; k++)
			{
				variance[i] += pow((double) (F[i][count] - absorbance[j][k]), (double) 2);
				count++;
			}
		}
		variance[i] = variance[i]/(count - parameters);
	}
	else
	{
		for (j=0; j<run_inf.scans[selected_cell][selected_lambda]; j++)
		{
			start_y = run_inf.plateau[selected_cell][selected_lambda][j] * bd_position / 100;
			stop_y  = start_y + run_inf.plateau[selected_cell][selected_lambda][j] * bd_range/100;
			for (k=0; k<points; k++)
			{
				if((absorbance[j][k] >= start_y) && (absorbance[j][k] <= stop_y))
				{
					variance[i] += pow((double) (F[i][count] - absorbance[j][k]), (double) 2);
					subcount++;
				}
				count++;
			}
		}
		variance[i] = variance[i]/(subcount - parameters);
	}
}

void US_Finite_W::F_init()
{
	unsigned int i;
	float improvement=0;
	QString str;
	for (i=0; i<parameters+1; i++)
	{ 
		str.sprintf(tr("Initializing Parameter Vectors..."));
		fit_control->lbl_status2->setText(str);
		str.sprintf(tr("Working on Vector %d of %d"), i+1, parameters+1);
		fit_control->lbl_status3->setText(str);
		if (constrained_fit)
		{
			update_simulation_parameters_constrained(i);
		}
		else
		{
			update_simulation_parameters_unconstrained(i);
		}
		if (model >= 0 && model <= 3)
		{
			completed = false;
			while (!completed)
			{
				if (non_interacting_model() != 0)
				{
					QMessageBox::message("Attention:", 
					"The finite element simulation failed.\n"
					"Please check the initialization\n"
					"parameters, and try again after\n"
					"adjusting the parameters to different\n"
					"values... Fit was aborted.");
					qApp->processEvents();
					return;
				}
				while (suspend_flag)
				{
					qApp->processEvents();	//take care of piled up events
					if (aborted)
					{
						suspend_flag = false;
						return;
					}
				}
			}
		}
		if (model == 4)
		{
			completed = false;
			while (!completed)
			{
				monomer_dimer_model();		// calculate
				while (suspend_flag)
				{
					qApp->processEvents();	//take care of piled up events
					if (aborted)
					{
						suspend_flag = false;
						return;
					}
				}
			}
		}
		if (model == 20)
		{
			completed = false;
			while (!completed)
			{
				isomerizing_model();			// calculate
				while (suspend_flag)
				{
					qApp->processEvents();	//take care of piled up events
					if (aborted)
					{
						suspend_flag = false;
						return;
					}
				}
			}
		}
		assign_F(i);
		calc_residuals(i);
		if (i > 0)
		{
			improvement =  variance[i-1] - variance[i];
		}
		str.sprintf("%1.4e", variance[i]);
		fit_control->lbl_variance2->setText(str);
		str.sprintf("%1.4e", pow((double) variance[i], (double) 0.5));
		fit_control->lbl_stddev2->setText(str);
		str.sprintf("%1.4e", improvement);
		fit_control->lbl_difference2->setText(str);
	}
}

void US_Finite_W::update_simulation_parameters_constrained(const unsigned int i)
{
//
// if any parameters are floated, the resepective component_vector member will be reassigned to 
// reflect the value stored in theta. If it isn't floated, the current value is kept unchanged.
// the parameter_count is incremented each time a floating parameter is encountered
//
	unsigned int k, parameter_count = 0;
	if (run_vector->meniscus_fit)
	{
		run_vector->meniscus = theta[i][parameter_count];
		model_control->lcd_meniscus->setPalette(QPalette(USglobal->global_colors.cg_green, USglobal->global_colors.cg_green, USglobal->global_colors.cg_green));
		if (run_vector->meniscus < (theta[parameters+2][parameter_count] - run_vector->meniscus_range))
		{
			run_vector->meniscus = theta[parameters+2][parameter_count] - run_vector->meniscus_range;
			theta[i][parameter_count] = run_vector->meniscus;
			model_control->lcd_meniscus->setPalette(QPalette(USglobal->global_colors.cg_red, USglobal->global_colors.cg_red, USglobal->global_colors.cg_red));
		}
		if (run_vector->meniscus > (theta[parameters+2][parameter_count] + run_vector->meniscus_range))
		{
			run_vector->meniscus = theta[parameters+2][parameter_count] + run_vector->meniscus_range;
			theta[i][parameter_count] = run_vector->meniscus;
			model_control->lcd_meniscus->setPalette(QPalette(USglobal->global_colors.cg_red, USglobal->global_colors.cg_red, USglobal->global_colors.cg_red));
		}
		parameter_count++;
	}
	if (run_vector->baseline_fit)
	{
		run_vector->baseline = theta[i][parameter_count];
		model_control->lcd_baseline->setPalette(QPalette(USglobal->global_colors.cg_green, USglobal->global_colors.cg_green, USglobal->global_colors.cg_green));
		if (run_vector->baseline < (theta[parameters+2][parameter_count] - run_vector->baseline_range))
		{
			run_vector->baseline = theta[parameters+2][parameter_count] - run_vector->baseline_range;
			theta[i][parameter_count] = run_vector->baseline;
			model_control->lcd_baseline->setPalette(QPalette(USglobal->global_colors.cg_red, USglobal->global_colors.cg_red, USglobal->global_colors.cg_red));
		}
		if (run_vector->baseline > (theta[parameters+2][parameter_count] + run_vector->baseline_range))
		{
			run_vector->baseline = theta[parameters+2][parameter_count] + run_vector->baseline_range;
			theta[i][parameter_count] = run_vector->baseline;
			model_control->lcd_baseline->setPalette(QPalette(USglobal->global_colors.cg_red, USglobal->global_colors.cg_red, USglobal->global_colors.cg_red));
		}
		parameter_count++;
	}
	if (run_vector->slope_fit)
	{
		run_vector->slope = theta[i][parameter_count];
		model_control->lcd_slope->setPalette(QPalette(USglobal->global_colors.cg_green, USglobal->global_colors.cg_green, USglobal->global_colors.cg_green));
		if (run_vector->slope < (theta[parameters+2][parameter_count] - run_vector->slope_range))
		{
			run_vector->slope = theta[parameters+2][parameter_count] - run_vector->slope_range;
			theta[i][parameter_count] = run_vector->slope;
			model_control->lcd_slope->setPalette(QPalette(USglobal->global_colors.cg_red, USglobal->global_colors.cg_red, USglobal->global_colors.cg_red));
		}
		if (run_vector->slope > (theta[parameters+2][parameter_count] + run_vector->slope_range))
		{
			run_vector->slope = theta[parameters+2][parameter_count] + run_vector->slope_range;
			theta[i][parameter_count] = run_vector->slope;
			model_control->lcd_slope->setPalette(QPalette(USglobal->global_colors.cg_red, USglobal->global_colors.cg_red, USglobal->global_colors.cg_red));
		}
		parameter_count++;
	}
	if (run_vector->stray_fit)
	{
		run_vector->stray = theta[i][parameter_count];
		model_control->lcd_stray->setPalette(QPalette(USglobal->global_colors.cg_green, USglobal->global_colors.cg_green, USglobal->global_colors.cg_green));
		if (run_vector->stray < (theta[parameters+2][parameter_count] - run_vector->stray_range))
		{
			run_vector->stray = theta[parameters+2][parameter_count] - run_vector->stray_range;
			theta[i][parameter_count] = run_vector->stray;
			model_control->lcd_stray->setPalette(QPalette(USglobal->global_colors.cg_red, USglobal->global_colors.cg_red, USglobal->global_colors.cg_red));
		}
		if (run_vector->stray > (theta[parameters+2][parameter_count] + run_vector->stray_range))
		{
			run_vector->stray = theta[parameters+2][parameter_count] + run_vector->stray_range;
			theta[i][parameter_count] = run_vector->stray;
			model_control->lcd_stray->setPalette(QPalette(USglobal->global_colors.cg_red, USglobal->global_colors.cg_red, USglobal->global_colors.cg_red));
		}
		parameter_count++;
	}
	for (k=0; k<components; k++)
	{
		if (component_vector[k].sed_fit)
		{
			component_vector[k].sed = theta[i][parameter_count];
			model_control->lcd_sed->setPalette(QPalette(USglobal->global_colors.cg_green, USglobal->global_colors.cg_green, USglobal->global_colors.cg_green));
			if (component_vector[k].sed < (theta[parameters+2][parameter_count] - component_vector[k].sed_range))
			{
				component_vector[k].sed = theta[parameters+2][parameter_count] - component_vector[k].sed_range;
				theta[i][parameter_count] = component_vector[k].sed;
				model_control->lcd_sed->setPalette(QPalette(USglobal->global_colors.cg_red, USglobal->global_colors.cg_red, USglobal->global_colors.cg_red));
			}
			if (component_vector[k].sed > (theta[parameters+2][parameter_count] + component_vector[k].sed_range))
			{
				component_vector[k].sed = theta[parameters+2][parameter_count] + component_vector[k].sed_range;
				theta[i][parameter_count] = component_vector[k].sed;
				model_control->lcd_sed->setPalette(QPalette(USglobal->global_colors.cg_red, USglobal->global_colors.cg_red, USglobal->global_colors.cg_red));
			}
			parameter_count++;
		}
		if (component_vector[k].diff_fit)
		{
			component_vector[k].diff = theta[i][parameter_count];
			model_control->lcd_diff->setPalette(QPalette(USglobal->global_colors.cg_green, USglobal->global_colors.cg_green, USglobal->global_colors.cg_green));
			if (component_vector[k].diff < (theta[parameters+2][parameter_count] - component_vector[k].diff_range))
			{
				component_vector[k].diff = theta[parameters+2][parameter_count] - component_vector[k].diff_range;
				theta[i][parameter_count] = component_vector[k].diff;
				model_control->lcd_diff->setPalette(QPalette(USglobal->global_colors.cg_red, USglobal->global_colors.cg_red, USglobal->global_colors.cg_red));
			}
			if (component_vector[k].diff > (theta[parameters+2][parameter_count] + component_vector[k].diff_range))
			{
				component_vector[k].diff = theta[parameters+2][parameter_count] + component_vector[k].diff_range;
				theta[i][parameter_count] = component_vector[k].diff;
				model_control->lcd_diff->setPalette(QPalette(USglobal->global_colors.cg_red, USglobal->global_colors.cg_red, USglobal->global_colors.cg_red));
			}
			parameter_count++;
		}
		if (component_vector[k].conc_fit)
		{
			component_vector[k].conc = theta[i][parameter_count];
			model_control->lcd_conc->setPalette(QPalette(USglobal->global_colors.cg_green, USglobal->global_colors.cg_green, USglobal->global_colors.cg_green));
			if (component_vector[k].conc < (theta[parameters+2][parameter_count] - component_vector[k].conc_range))
			{
				component_vector[k].conc = theta[parameters+2][parameter_count] - component_vector[k].conc_range;
				theta[i][parameter_count] = component_vector[k].conc;
				model_control->lcd_conc->setPalette(QPalette(USglobal->global_colors.cg_red, USglobal->global_colors.cg_red, USglobal->global_colors.cg_red));
			}
			if (component_vector[k].conc > (theta[parameters+2][parameter_count] + component_vector[k].conc_range))
			{
				component_vector[k].conc = theta[parameters+2][parameter_count] + component_vector[k].conc_range;
				theta[i][parameter_count] = component_vector[k].conc;
				model_control->lcd_conc->setPalette(QPalette(USglobal->global_colors.cg_red, USglobal->global_colors.cg_red, USglobal->global_colors.cg_red));
			}
			parameter_count++;
		}
		if (component_vector[k].sigma_fit)
		{
			component_vector[k].sigma = theta[i][parameter_count];
			model_control->lcd_sigma->setPalette(QPalette(USglobal->global_colors.cg_green, USglobal->global_colors.cg_green, USglobal->global_colors.cg_green));
			if (component_vector[k].sigma < (theta[parameters+2][parameter_count] - component_vector[k].sigma_range))
			{
				component_vector[k].sigma = theta[parameters+2][parameter_count] - component_vector[k].sigma_range;
				theta[i][parameter_count] = component_vector[k].sigma;
				model_control->lcd_sigma->setPalette(QPalette(USglobal->global_colors.cg_red, USglobal->global_colors.cg_red, USglobal->global_colors.cg_red));
			}
			if (component_vector[k].sigma > (theta[parameters+2][parameter_count] + component_vector[k].sigma_range))
			{
				component_vector[k].sigma = theta[parameters+2][parameter_count] + component_vector[k].sigma_range;
				theta[i][parameter_count] = component_vector[k].sigma;
				model_control->lcd_sigma->setPalette(QPalette(USglobal->global_colors.cg_red, USglobal->global_colors.cg_red, USglobal->global_colors.cg_red));
			}
			parameter_count++;
		}
		if (component_vector[k].delta_fit)
		{
			component_vector[k].delta = theta[i][parameter_count];
			model_control->lcd_delta->setPalette(QPalette(USglobal->global_colors.cg_green, USglobal->global_colors.cg_green, USglobal->global_colors.cg_green));
			if (component_vector[k].delta < (theta[parameters+2][parameter_count] - component_vector[k].delta_range))
			{
				component_vector[k].delta = theta[parameters+2][parameter_count] - component_vector[k].delta_range;
				theta[i][parameter_count] = component_vector[k].delta;
				model_control->lcd_delta->setPalette(QPalette(USglobal->global_colors.cg_red, USglobal->global_colors.cg_red, USglobal->global_colors.cg_red));
			}
			if (component_vector[k].delta > (theta[parameters+2][parameter_count] + component_vector[k].delta_range))
			{
				component_vector[k].delta = theta[parameters+2][parameter_count] + component_vector[k].delta_range;
				theta[i][parameter_count] = component_vector[k].delta;
				model_control->lcd_delta->setPalette(QPalette(USglobal->global_colors.cg_red, USglobal->global_colors.cg_red, USglobal->global_colors.cg_red));
			}
			parameter_count++;
		}
	}
	model_control->update_labels();
}

void US_Finite_W::update_simulation_parameters_unconstrained(const unsigned int i)
{
//
// if any parameters are floated, the respective component_vector member will be reassigned to 
// reflect the value stored in theta. If it isn't floated, the current value is kept unchanged.
// the parameter_count is incremented each time a floating parameter is encountered
//
	unsigned int k, parameter_count = 0;
	if (run_vector->meniscus_fit)
	{
		run_vector->meniscus = theta[i][parameter_count];
		if (run_vector->meniscus - run_inf.meniscus[selected_cell] < -0.3)
		{
			run_vector->meniscus = run_inf.meniscus[selected_cell] - 0.3;
			theta[i][parameter_count]  = run_vector->meniscus;
		}
		if (run_vector->meniscus - run_inf.meniscus[selected_cell] > 0.3)
		{
			run_vector->meniscus = run_inf.meniscus[selected_cell] + 0.3;
			theta[i][parameter_count]  = run_vector->meniscus;
		}
		parameter_count++;
	}
	if (run_vector->baseline_fit)
	{
		run_vector->baseline = theta[i][parameter_count];
		parameter_count++;
	}
	if (run_vector->slope_fit)
	{
		run_vector->slope = theta[i][parameter_count];
		parameter_count++;
	}
	if (run_vector->stray_fit)
	{
		run_vector->stray = theta[i][parameter_count];
		parameter_count++;
	}
	for (k=0; k<components; k++)
	{
		if (component_vector[k].sed_fit)
		{
			component_vector[k].sed = theta[i][parameter_count];
/*
			if (component_vector[k].sed < 1.0e-15)
			{
				component_vector[k].sed = 1.0e-15;
				theta[i][parameter_count] = 1.0e-15;
			}
*/
			parameter_count++;
		}
		if (component_vector[k].diff_fit)
		{
			component_vector[k].diff = theta[i][parameter_count];
			if (component_vector[k].diff < 5.0e-10)
			{
				component_vector[k].diff  = (float) 5.0e-10;
				theta[i][parameter_count] = (float) 5.0e-10;
			}
			parameter_count++;
		}
		if (component_vector[k].conc_fit)
		{
			component_vector[k].conc = theta[i][parameter_count];
			if (component_vector[k].conc < 5.0e-3)
			{
				component_vector[k].conc  = (float) 5.0e-3;
				theta[i][parameter_count] = (float) 5.0e-3;
			}
			parameter_count++;
		}
		if (component_vector[k].sigma_fit)
		{
			component_vector[k].sigma = theta[i][parameter_count];
			if (component_vector[k].sigma < 0.0)
			{
				component_vector[k].sigma = 0.0;
				theta[i][parameter_count] = 0.0;
			}
			parameter_count++;
		}
		if (component_vector[k].delta_fit)
		{
			component_vector[k].delta = theta[i][parameter_count];
			if (component_vector[k].delta < 0.0)
			{
				component_vector[k].delta = 0.0;
				theta[i][parameter_count] = 0.0;
			}
			parameter_count++;
		}
	}
	model_control->update_labels();
}

void US_Finite_W::order_variance()
{
	unsigned int i, j;
	for (i=0; i<parameters+1; i++)
	{
		old_variance[i] = variance[i];
	}
	for (j=0; j<parameters+1; j++)
	{
		float maxval=0.0;
		for (i=0; i<parameters+1; i++)
		{
			if (variance[i] > maxval)
			{
				maxval = variance[i];
				order[j] = i;
			}
		}
		if (j != parameters)
		{
			variance[order[j]] = 0.0;
// do not lose the best variance, since we need it for comparison for the first iteration!
		}
	}
	swap();
}

void US_Finite_W::swap()
{// reorders theta, F and variance, so they are in the proper order as determined by order_variance()
	float **matrix;
	unsigned int i, j;
	
	matrix = new float* [parameters+1];
	for (i=0; i<parameters+1; i++)
	{
		matrix[i] = new float [parameters];
	}
	for (i=0; i<parameters+1; i++)
	{
		for (j=0; j<parameters; j++)
		{
			matrix[i][j] = theta[order[i]][j];
		}
	}
	for (i=0; i<parameters+1; i++)
	{
		for (j=0; j<parameters; j++)
		{
			theta[i][j] = matrix[i][j];
		}
	}
	for (i=0; i<parameters+1; i++)
	{
		delete [] matrix[i];
	}
	delete [] matrix;
	matrix = new float* [parameters+1];
	for (i=0; i<parameters+1; i++)
	{
		matrix[i] = new float [allpoints];
	}
	for (i=0; i<parameters+1; i++)
	{
		for (j=0; j<allpoints; j++)
		{
			matrix[i][j] = F[order[i]][j];
		}
	}
	for (i=0; i<parameters+1; i++)
	{
		for (j=0; j<allpoints; j++)
		{
			F[i][j] = matrix[i][j];
		}
	}
	for (i=0; i<parameters+1; i++)
	{
		variance[i] = old_variance[order[i]];
	}
	for (i=0; i<parameters+1; i++)
	{	//update old_variance so we have the correct values in the proper indices in the next iteration when we compare
		old_variance[i] = variance[i];
	}
	for (i=0; i<parameters+1; i++)
	{
		delete [] matrix[i];
	}
	delete [] matrix;
}

void US_Finite_W::assign_F(const unsigned int i)
{

// synchronize the radial stepsize in the simulation with the stepsize in the raw data:

	unsigned int j=0, k, start_index, stop_index, count, radius_count;
	float m, b=0.0;
	
	while (run_inf.range_left[selected_cell][selected_lambda][0] >= sim_radius[j])
	{
		j++;
	}
	start_index = j-1;
	while (run_inf.range_right[selected_cell][selected_lambda][0] >= sim_radius[j])
	{
		j++;
	}
	stop_index = j-1;

	if (run_vector->delta_r == run_inf.delta_r)
	{
		count = 0;
		for (k=0; k<run_inf.scans[selected_cell][selected_lambda]; k++)
		{
			for (j=start_index; j<start_index+points; j++) //second dimension of F is equal to number of points*scans in raw data
			{
				F[i][count] = concentration[k][j];
				count++;
//cout << "F[" << i << "][" << count << "]: " << concentration[k][j] << endl;
			}
		}
		allpoints = count; //should be the same as points*scans
		if (allpoints != points*run_inf.scans[selected_cell][selected_lambda])
		{
			debug(tr("Problem with number of points - they don't match!"));
			debug(tr("Allpoints: %d, points*scans: %d"), allpoints, points*run_inf.scans[selected_cell][selected_lambda]);
		}
	}
	else // if the radial discretizations don't agree, we need to interpolate:
	{
		count = 0;
		for (k=0; k<run_inf.scans[selected_cell][selected_lambda]; k++)
		{
			radius_count = start_index;
			for (j=0; j<points; j++)
			{
				while (sim_radius[radius_count] < radius[j])
				{
					radius_count++;
				}
				m = (concentration[k][radius_count] - concentration[k][radius_count-1])
					/(sim_radius[radius_count] - sim_radius[radius_count-1]);
				b = concentration[k][radius_count] - m * sim_radius[radius_count];
				F[i][count] = m * radius[j] + b;
				count++;
			}
		}
		allpoints = count;
	}
}

void US_Finite_W::monomer_dimer_model()
{
	unsigned int i, j, k, time_steps, scan = 0, count = 0;
	float temp1, temp2, scan_timer;
	bool ldu_flag = true;
	QString str;

	time_steps = (unsigned int) (0.5 + (run_inf.time[selected_cell][selected_lambda][run_inf.scans[selected_cell][selected_lambda] - 1]
	/ run_vector->delta_t));
	fit_control->pgb_progress->setTotalSteps(time_steps);
	fit_control->pgb_progress->reset();
	if (!init_simulation)
	{
		init_finite_element();
	}
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		for (j=0; j<(unsigned int) sim_points; j++)
		{
			concentration[i][j]=0.0;
		}
	}
//
// allocate memory for the partial concentrations of monomer ([0]) and dimer ([1]):
//
	for (i=0; i<(unsigned int) sim_points; i++)
	{
		right[i] = component_vector[0].conc;
		partial_c[0][i] = (-1 + pow((double) (1 + 8 * right[i] * component_vector[1].conc), (double) 0.5))
								/ 4 * component_vector[1].conc;
		partial_c[1][i] = right[i] - partial_c[0][i];
	}
	func_eval++;
	str.sprintf("%d", func_eval);
	fit_control->lbl_evaluations2->setText(str);
	temp1 = run_vector->delta_t * omega_s;
	str.sprintf(tr("Simulating Equilibrium..."));
	fit_control->lbl_status4->setText(str);
	scan_timer=0.0;
	for (j=0; j<time_steps; j++)
	{
		fit_control->pgb_progress->setProgress(count);
		count++;
		scan_timer += run_vector->delta_t;
		for (i=0; i<(unsigned int) sim_points; i++)
		{
			if ((partial_c[0][i] + partial_c[1][i]) > 1e-16)
			{
				s_bar[i] = (component_vector[0].sed * partial_c[0][i]
							+ component_vector[1].sed * partial_c[1][i])
							/ (partial_c[0][i] + partial_c[1][i]);
			}
			else
			{
				s_bar[i] = (component_vector[0].sed + component_vector[1].sed)/2;
			}
		}
		for (i=0; i< (unsigned int) sim_points; i++)
		{
			if (((partial_c[0][i+1] - partial_c[0][i]) +
				 (partial_c[1][i+1] - partial_c[1][i])) > 1e-16)
			{
				D_bar[i] = ((component_vector[0].diff * (partial_c[0][i+1] - partial_c[0][i]))
							+ (component_vector[1].diff * (partial_c[1][i+1] - partial_c[1][i])))
							/ ((partial_c[0][i+1] - partial_c[0][i]) +
				 				(partial_c[1][i+1] - partial_c[1][i]));
			}
			else
			{
				D_bar[i] = (component_vector[0].diff + component_vector[1].diff)/2;
			}
		}
		D_bar[sim_points-1] = (component_vector[0].diff + component_vector[1].diff)/2;
		for (i=0; i<(unsigned int) sim_points; i++)
		{
			for (k=0; k<3; k++)
			{
				left[i][k] = b[i][k] + run_vector->delta_t * D_bar[i] * a1[i][k] -
				temp1 * s_bar[i] * a2[i][k];
			}
		}
		m3vm(&b, &right, sim_points);
		ldu(&left, &right, sim_points, 3, ldu_flag);
		ldu_flag = true;
		if (fabs(run_inf.time[selected_cell][selected_lambda][scan] - scan_timer) <= (run_vector->delta_t/2.0))
		{
			for (i=0; i<(unsigned int) sim_points; i++)
			{
				concentration[scan][i] = right[i];
			}
			scan++;
			if (scan == run_inf.scans[selected_cell][selected_lambda])
			{
				scan = 0;
			}
		}
		for (i=0; i<(unsigned int) sim_points; i++)
		{
			temp2 = 1 + 8 * right[i] * component_vector[1].conc;
			if (temp2 > 0)
			{
				partial_c[0][i] = (-1 + pow((double) temp2, (double) 0.5))
									/ (4 * component_vector[1].conc);
				partial_c[1][i] = right[i] - partial_c[0][i];
			}
			else
			{
				QMessageBox::message( tr("Warning"), tr("Attention:\n\n"
				"The solution encountered a discontinuity at the\n"
				"bottom of the cell.\n\n"
				"This is most often caused by a combination of\n"
				"small diffusion coefficients and large radial\n"
				"discretization stepsizes. Please try again with\n"
				"a smaller radial discretization stepsize."));
				return;
			}
		}
		qApp->processEvents();
		if(suspend_flag || aborted)
		{
			return;
		}
	}
	fit_control->pgb_progress->setProgress(time_steps);

// before exiting this function, add the baseline and the slope to the run:

	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		for (j=0; j<(unsigned int) sim_points; j++)
		{
			concentration[i][j] += run_vector->baseline + (sim_radius[j]-run_inf.range_left[selected_cell][selected_lambda][0]) 
									 	* run_vector->slope + (sim_radius[j]-run_inf.range_left[selected_cell][selected_lambda][0])
										* run_vector->stray * concentration[i][j];
		}
	}
	completed = true;
}

void US_Finite_W::isomerizing_model()
{
	unsigned int i, j, k, time_steps, scan = 0, count = 0;
	float temp1, temp2, scan_timer;
	bool ldu_flag = true;
	QString str;

	time_steps = (unsigned int) (0.5 + (run_inf.time[selected_cell][selected_lambda][run_inf.scans[selected_cell][selected_lambda] - 1]
	/ run_vector->delta_t));
	fit_control->pgb_progress->setTotalSteps(time_steps);
	fit_control->pgb_progress->reset();
	if (!init_simulation)
	{
		init_finite_element();
	}
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		for (j=0; j<(unsigned int) sim_points; j++)
		{
			concentration[i][j]=0.0;
		}
	}
//
// allocate memory for the partial concentrations of monomer ([0]) and dimer ([1]):
//
	for (i=0; i<(unsigned int) sim_points; i++)
	{
		right[i] = component_vector[0].conc;
		partial_c[0][i] = right[i] / (component_vector[1].conc + 1.0);
		partial_c[1][i] = right[i] - partial_c[0][i];
	}
	func_eval++;
	str.sprintf("%d", func_eval);
	fit_control->lbl_evaluations2->setText(str);
	temp1 = run_vector->delta_t * omega_s;
	str.sprintf(tr("Simulating Equilibrium..."));
	fit_control->lbl_status4->setText(str);
	scan_timer=0.0;
	for (j=0; j<time_steps; j++)
	{
		fit_control->pgb_progress->setProgress(count);
		count++;
		scan_timer += run_vector->delta_t;
		for (i=0; i<(unsigned int) sim_points; i++)
		{
			if ((partial_c[0][i] + partial_c[1][i]) > 1e-16)
			{
				s_bar[i] = (component_vector[0].sed * partial_c[0][i]
							+ component_vector[1].sed * partial_c[1][i])
							/ (partial_c[0][i] + partial_c[1][i]);
			}
			else
			{
				s_bar[i] = (component_vector[0].sed + component_vector[1].sed)/2;
			}
		}
		for (i=0; i< (unsigned int) sim_points-1; i++)
		{
			if (((partial_c[0][i+1] - partial_c[0][i]) +
				 (partial_c[1][i+1] - partial_c[1][i])) > 1e-16)
			{
				D_bar[i] = ((component_vector[0].diff * (partial_c[0][i+1] - partial_c[0][i]))
							+ (component_vector[1].diff * (partial_c[1][i+1] - partial_c[1][i])))
							/ ((partial_c[0][i+1] - partial_c[0][i]) +
				 				(partial_c[1][i+1] - partial_c[1][i]));
			}
			else
			{
				D_bar[i] = (component_vector[0].diff + component_vector[1].diff)/2;
			}
		}
		D_bar[sim_points-1] = (component_vector[0].diff + component_vector[1].diff)/2;
		for (i=0; i<(unsigned int) sim_points; i++)
		{
			for (k=0; k<3; k++)
			{
				left[i][k] = b[i][k] + run_vector->delta_t * D_bar[i] * a1[i][k] -
				temp1 * s_bar[i] * a2[i][k];
			}
		}
		m3vm(&b, &right, sim_points);
		ldu(&left, &right, sim_points, 3, ldu_flag);
		ldu_flag = true;
		if (fabs(run_inf.time[selected_cell][selected_lambda][scan] - scan_timer) <= (run_vector->delta_t/2.0))
		{
			for (i=0; i<(unsigned int) sim_points; i++)
			{
				concentration[scan][i] = right[i];
			}
			scan++;
			if (scan == run_inf.scans[selected_cell][selected_lambda])
			{
				scan = 0;
			}
		}
		for (i=0; i<(unsigned int) sim_points; i++)
		{
			temp2 = 1 + 4 * right[i] * component_vector[1].conc;
			if (temp2 > 0)
			{
				partial_c[0][i] = right[i] / (component_vector[1].conc + 1.0);
				partial_c[1][i] = right[i] - partial_c[0][i];
			}
			else
			{
				QMessageBox::message( tr("Warning"), tr("Attention:\n\n"
				"The solution encountered a discontinuity at the\n"
				"bottom of the cell.\n\n"
				"This is most often caused by a combination of\n"
				"small diffusion coefficients and large radial\n"
				"discretization stepsizes. Please try again with\n"
				"a smaller radial discretization stepsize."));
				return;
			}
		}
		qApp->processEvents();
		if(suspend_flag || aborted)
		{
			return;
		}
	}
	fit_control->pgb_progress->setProgress(time_steps);

// before exiting this function, add the baseline and the slope to the run:

	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		for (j=0; j<(unsigned int) sim_points; j++)
		{
			concentration[i][j] += run_vector->baseline + (sim_radius[j]-run_inf.range_left[selected_cell][selected_lambda][0]) 
									 	* run_vector->slope + (sim_radius[j]-run_inf.range_left[selected_cell][selected_lambda][0])
										* run_vector->stray * concentration[i][j];
		}
	}
	completed = true;
}

int US_Finite_W::non_interacting_model()
{
	QString str;
	mfem_data fem_data;
	US_MovingFEM *mfem;
	mfem = new US_MovingFEM(&fem_data, false, 0, 0);
	if (!init_simulation)
	{
		init_finite_element();
	}
	double m = run_vector->meniscus;
	double b = calc_bottom(rotor_list, cp_list, run_inf.rotor, run_inf.centerpiece[selected_cell], 0,
	run_inf.rpm[first_cell][0][0]);
	double rpm = run_inf.rpm[first_cell][0][0];
	double total_t = run_inf.time[selected_cell][selected_lambda][run_inf.scans[selected_cell][selected_lambda] - 1] + 1000;     // total_time for simulation
	double slope;
	double intercept;
	
// initialize concentration to zero:

	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		for (j=0; j<(unsigned int) sim_points; j++)
		{
			concentration[i][j]=0.0;
		}
	}

// calculate a solution for each component in the model:

	for (k=0; k<components; k++)
	{
//cout << component_vector[k].sed << ", " <<  component_vector[k].diff << endl;

		// update the GUI:
		str.sprintf(tr("Simulating Component %d"), k+1);
		fit_control->lbl_status4->setText(str);
		qApp->processEvents();

		mfem->set_params(run_vector->sim_points, component_vector[k].sed, component_vector[k].diff,
		rpm, total_t, m, b, component_vector[k].conc, &initCvector);
//cout << ", " << component_vector[k].sed << ", " << component_vector[k].diff
// << ", " << rpm << ", " << total_t << ", " << m << ", " <<  b<< ", " << component_vector[k].conc << endl;
		if(mfem->run() != 0)
		{
			delete mfem;
			aborted = true;
			cancel_fit();
			return (-1);
		}
		else
		{
/*
	 		interpolation:
			First, we need to interpolate the time. Create a new array with the same time dimensions
			as the raw data and the same radius dimensions as the simulated data. Then find the time
			steps from the simulated data that bracket the experimental data from the left and right.
			Then make a linear interpolation for the concentration values at each radius point from
			the simulated data. Then interpolate the radius points by linear interpolation.
*/
			double **ip_array;
			ip_array	= 	new double* [run_inf.scans[selected_cell][selected_lambda]];
			for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
			{
				ip_array[i] = new double [fem_data.radius.size()];
			}
			unsigned int count = 0; // counting the number of time steps of the raw data
			for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
			{
				while (count < fem_data.scan.size()-1
						&& run_inf.time[selected_cell][selected_lambda][i] >= fem_data.scan[count].time)
				{
					count++;
				}
				if (run_inf.time[selected_cell][selected_lambda][i] == fem_data.scan[count].time)
				{
					for (j=0; j<fem_data.radius.size(); j++)
					{
						ip_array[i][j] = fem_data.scan[count].conc[j];
					}
				}
				else  // else, perform a linear time interpolation:
				{
					for (j=0; j<fem_data.radius.size(); j++)
					{
						slope = (fem_data.scan[count].conc[j] - fem_data.scan[count-1].conc[j])
										  /(fem_data.scan[count].time - fem_data.scan[count-1].time);
						intercept = fem_data.scan[count].conc[j] - slope * fem_data.scan[count].time;
						ip_array[i][j] = slope * run_inf.time[selected_cell][selected_lambda][i] + intercept;
					}
				}
			}
			//interpolate radius then add to concentration vector
			for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
			{
				concentration[i][0] += ip_array[i][0]; // meniscus position is identical for all scans
			}
			// all other points may need interpolation:
			for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
			{
				count = 1;
				for (j=1; j<(unsigned int) sim_points; j++)
				{
					while (sim_radius[j] > fem_data.radius[count] && count < fem_data.radius.size()-1)
					{
						count++;
					}
					if (sim_radius[j] == fem_data.radius[count])
					{
						concentration[i][j] += ip_array[i][count];
					}
					else 
					{
						slope = (ip_array[i][count] - ip_array[i][count-1])/(fem_data.radius[count] - fem_data.radius[count-1]);
						intercept = ip_array[i][count] - fem_data.radius[count] * slope;
						concentration[i][j] += slope * sim_radius[j] + intercept;
					}
				}
			}
			for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
			{
				delete [] ip_array[i];
			}
			delete [] ip_array;

		} // end else
	} // end component loop
	delete mfem;
	// update the concentration with baseline and time invariant slopes
	ti_noise.clear();
	ti_noise.resize(sim_points);
	for (j=0; j<(unsigned int) sim_points; j++)
	{// add the constant terms (baseline and slope)
		ti_noise[j] = run_vector->baseline + (sim_radius[j]-run_inf.range_left[selected_cell][selected_lambda][0]) * run_vector->slope;
	}
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		for (j=0; j<(unsigned int) sim_points; j++)
		{
			concentration[i][j] += run_vector->baseline + (sim_radius[j]-run_inf.range_left[selected_cell][selected_lambda][0]) 
									 	* run_vector->slope + (sim_radius[j]-run_inf.range_left[selected_cell][selected_lambda][0])
										* run_vector->stray * concentration[i][j];
			// add an average of all scans from the variable slope terms to the ti noise to get an approximated baseline
		 	ti_noise[j] += ((sim_radius[j]-run_inf.range_left[selected_cell][selected_lambda][0])
									* run_vector->stray * concentration[i][j])/run_inf.scans[selected_cell][selected_lambda];
		}
	}
	completed = true;
	return(0);
}

void US_Finite_W::init_finite_element()
{
	unsigned int i;
	float bottom = calc_bottom(rotor_list, cp_list, run_inf.rotor, run_inf.centerpiece[selected_cell], 0, run_inf.rpm[first_cell][0][0]);
	init_simulation = true;
	sim_points = (unsigned int) (1.5 + (bottom - run_vector->meniscus)/run_vector->delta_r);
	sim_radius = new float [sim_points];
	sim_radius[0] = run_vector->meniscus;
	for (i=1; i<(unsigned int) sim_points; i++)
	{
		sim_radius[i] = sim_radius[i-1] + run_vector->delta_r;
	}
	concentration = new float* [run_inf.scans[selected_cell][selected_lambda]];
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		concentration[i] = new float [sim_points];
	}
}

void US_Finite_W::cleanup_finite()
{
	unsigned int i;
	init_simulation = false;
	delete [] sim_radius;
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		delete [] concentration[i];
	}
	delete [] concentration;
}

void US_Finite_W::dud_init()
{
	unsigned int i, j, parameter_count=0;

// How many parameters need to be fit?

	parameters = 0;
	if (run_vector->meniscus_fit)
	{
		parameters++;
	}
	if (run_vector->baseline_fit)
	{
		parameters++;
	}
	if (run_vector->slope_fit)
	{
		parameters++;
	}
	if (run_vector->stray_fit)
	{
		parameters++;
	}
	for (i=0; i<components; i++)
	{
		if (component_vector[i].sed_fit)
		{
			parameters++;
		}
		if (component_vector[i].diff_fit)
		{
			parameters++;
		}
		if (component_vector[i].conc_fit)
		{
			parameters++;
		}
		if (component_vector[i].sigma_fit)
		{
			parameters++;
		}
		if (component_vector[i].delta_fit)
		{
			parameters++;
		}
	}
	model_control->lcd_meniscus->setPalette(QPalette(USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green));
	model_control->lcd_baseline->setPalette(QPalette(USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green));
	model_control->lcd_slope->setPalette(QPalette(USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green));
	model_control->lcd_stray->setPalette(QPalette(USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green));
	model_control->lcd_sed->setPalette(QPalette(USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green));
	model_control->lcd_diff->setPalette(QPalette(USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green));
	model_control->lcd_conc->setPalette(QPalette(USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green));
	model_control->lcd_sigma->setPalette(QPalette(USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green));
	model_control->lcd_delta->setPalette(QPalette(USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green));

	iteration = 0;

// About the parameters for MW and Vbar:
// if S and D are floating, either MW or vbar also must be floating
// if the molecular weight is fixed, we can still float vbar, but if
// both are fixed, we have to adjust the ratio of s/D by floating S
// and adjusting D through the ratio constraint. In that case D doesn't float
// even though it is adjusted, but the adjustment is dictated by the ratio,
// and D is not considered a fitted parameter.

// Rows (in dimension 1):

	theta			 = new float*  [parameters + 3];	//columns in dimension 1 (1 extra for theta_new, 1 for bounds checking)
	delta_theta	 = new float*  [parameters];		//columns in dimension 1
	F 				 = new float*  [parameters + 2];	//columns in dimension 1 (1 extra for theta_new)
	delta_F		 = new float*  [parameters];		//columns in dimension 1
	alpha			 = new float   [parameters];
	variance		 = new float   [parameters + 2];
	old_variance = new float   [parameters + 2];
	order 		 = new unsigned int [parameters + 1];
	residuals	 = new double* [run_inf.scans[selected_cell][selected_lambda]];
	y 				 = new double* [run_inf.scans[selected_cell][selected_lambda]];
	line_x		 = new double  [2];
	line			 = new uint		[5];
	line_y		 = new double* [run_inf.scans[selected_cell][selected_lambda]];
	for (i=0; i<parameters; i++)
	{
		delta_theta[i]	= new float [parameters];
		delta_F[i] 		= new float [points * run_inf.scans[selected_cell][selected_lambda]];
	}
	for (i=0; i<parameters+3; i++)
	{
		theta[i] 		= new float [parameters];
	}
	for (i=0; i<parameters+2; i++)
	{
		F[i] 				= new float [points * run_inf.scans[selected_cell][selected_lambda]];
	}
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		residuals[i]	= new double [points];
		y[i] 				= new double [points];
		line_y[i] 		= new double [2];
	}

// Initialize Theta with an improved scheme:

	for (i=0; i<parameters+1; i++)
	{
		parameter_count = 0;

// For each column, check if it is the last column or for any other column, check if the element
// is on the diagonal or off-diagonal

		if (run_vector->meniscus_fit)
		{
			if (i == parameters)				// this is the p+1 column of Theta, which is not offset by the range bias
			{
				theta[i][parameter_count] = run_vector->meniscus;
			}
			else
			{
				if (i == parameter_count)		//then we have a diagonal element
				{
					theta[i][parameter_count] = run_vector->meniscus + run_vector->meniscus_range;
				}
				else 		//then we have an off-diagonal element
				{
					theta[i][parameter_count] = run_vector->meniscus - run_vector->meniscus_range;
				}
			}
			parameter_count++;
		}
		if (run_vector->baseline_fit)
		{
			if (i == parameters)				// this is the p+1 column of Theta, which is not offset by the range bias
			{
				theta[i][parameter_count] = run_vector->baseline;
			}
			else
			{
				if (i == parameter_count)		//then we have a diagonal element
				{
					theta[i][parameter_count] = run_vector->baseline + run_vector->baseline_range;
				}
				else 		//then we have an off-diagonal element
				{
					theta[i][parameter_count] = run_vector->baseline - run_vector->baseline_range;
				}
			}
			parameter_count++;
		}
		if (run_vector->slope_fit)
		{
			if (i == parameters)				// this is the p+1 column of Theta, which is not offset by the range bias
			{
				theta[i][parameter_count] = run_vector->slope;
			}
			else
			{
				if (i == parameter_count)		//then we have a diagonal element
				{
					theta[i][parameter_count] = run_vector->slope + run_vector->slope_range;
				}
				else 		//then we have an off-diagonal element
				{
					theta[i][parameter_count] = run_vector->slope - run_vector->slope_range;
				}
			}
			parameter_count++;
		}
		if (run_vector->stray_fit)
		{
			if (i == parameters)				// this is the p+1 column of Theta, which is not offset by the range bias
			{
				theta[i][parameter_count] = run_vector->stray;
			}
			else
			{
				if (i == parameter_count)		//then we have a diagonal element
				{
					theta[i][parameter_count] = run_vector->stray + run_vector->stray_range;
				}
				else 		//then we have an off-diagonal element
				{
					theta[i][parameter_count] = run_vector->stray - run_vector->stray_range;
				}
			}
			parameter_count++;
		}
		for (j=0; j<components; j++)
		{
			if (component_vector[j].sed_fit)
			{
				if (i == parameters)				// this is the p+1 column of Theta, which is not offset by the range bias
				{
					theta[i][parameter_count] = component_vector[j].sed;
				}
				else
				{
					if (i == parameter_count)		//then we have a diagonal element
					{
						theta[i][parameter_count] = component_vector[j].sed + component_vector[j].sed_range;
					}
					else 		//then we have an off-diagonal element
					{
						theta[i][parameter_count] = component_vector[j].sed - component_vector[j].sed_range;
					}
				}
				parameter_count++;
			}
			if (component_vector[j].diff_fit)
			{
				if (i == parameters)				// this is the p+1 column of Theta, which is not offset by the range bias
				{
					theta[i][parameter_count] = component_vector[j].diff;
				}
				else
				{
					if (i == parameter_count)		//then we have a diagonal element
					{
						theta[i][parameter_count] = component_vector[j].diff + component_vector[j].diff_range;
					}
					else 		//then we have an off-diagonal element
					{
						theta[i][parameter_count] = component_vector[j].diff - component_vector[j].diff_range;
					}
				}
				parameter_count++;
			}
			if (component_vector[j].conc_fit)
			{
				if (i == parameters)				// this is the p+1 column of Theta, which is not offset by the range bias
				{
					theta[i][parameter_count] = component_vector[j].conc;
				}
				else
				{
					if (i == parameter_count)		//then we have a diagonal element
					{
						theta[i][parameter_count] = component_vector[j].conc + component_vector[j].conc_range;
					}
					else 		//then we have an off-diagonal element
					{
						theta[i][parameter_count] = component_vector[j].conc - component_vector[j].conc_range;
					}
				}
				parameter_count++;
			}
			if (component_vector[j].sigma_fit)
			{
				if (i == parameters)				// this is the p+1 column of Theta, which is not offset by the range bias
				{
					theta[i][parameter_count] = component_vector[j].sigma;
				}
				else
				{
					if (i == parameter_count)		//then we have a diagonal element
					{
						theta[i][parameter_count] = component_vector[j].sigma + component_vector[j].sigma_range;
					}
					else 		//then we have an off-diagonal element
					{
						theta[i][parameter_count] = component_vector[j].sigma - component_vector[j].sigma_range;
					}
				}
				parameter_count++;
			}
			if (component_vector[j].delta_fit)
			{
				if (i == parameters)				// this is the p+1 column of Theta, which is not offset by the range bias
				{
					theta[i][parameter_count] = component_vector[j].delta;
				}
				else
				{
					if (i == parameter_count)		//then we have a diagonal element
					{
						theta[i][parameter_count] = component_vector[j].delta + component_vector[j].delta_range;
					}
					else 		//then we have an off-diagonal element
					{
						theta[i][parameter_count] = component_vector[j].delta - component_vector[j].delta_range;
					}
				}
				parameter_count++;
			}
		}
	}
	
// save the p+1 column in the p+3 column for later bounds checking. The p+3 column will not get changed
// The p+2 column is reserved for new_theta. The P+3 column will be used to calculate the maximal bounds
// for the constrain checking.

	for (i=0; i<parameters; i++)
	{
		theta[parameters+2][i] = theta[parameters][i];
	}

/*
// print out initialized theta matrix:
	cout << "in dud_init:\n";
	for (i=0; i<parameters; i++)
	{
		for (j=0; j<parameters+3; j++)
		{
			cout << theta[j][i] << "\t";
		}
		cout << "\n";
	}
*/
}

void US_Finite_W::cleanup_dud()
{
	unsigned int i;
	for (i=0; i<parameters; i++)
	{
		delete [] delta_theta[i];
		delete [] delta_F[i];
	}
	for (i=0; i<parameters+3; i++)
	{
		delete [] theta[i];
	}
	for (i=0; i<parameters+2; i++)
	{
		delete [] F[i];
	}
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		delete [] residuals[i];
		delete [] y[i];
		delete [] line_y[i];
	}
	delete [] theta;
	delete [] delta_theta;
	delete [] F;
	delete [] delta_F;
	delete [] alpha;
	delete [] variance;
	delete [] old_variance;
	delete [] order;
	delete [] residuals;
	delete [] y;
	delete [] line_x;
	delete [] line;
	delete [] line_y;
}

void US_Finite_W::view()
{
	if (step == 0)
	{
		QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n	   Click on \"Load Data\"\n"));
		return;
	}
	if (!converged)
	{
		QMessageBox::message(tr("Attention:\n"),tr("     There is nothing to report...\n\n"
														"Please fit your data first until it is\n"
														"converged before trying to save the results.\n"));
		return;
	}
	QString filestr, filename, temp;
	filename = USglobal->config_list.result_dir.copy();
	filename.append("/");
	filename.append(run_inf.run_id);
	temp.sprintf(".%d%d", selected_cell+1, selected_lambda+1);
	filestr = filename.copy();
	filestr.append(".fef_res");
	filestr.append(temp);
	write_res();
	//view_file(filestr);
	TextEdit *e;
	e = new TextEdit();
	e->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	e->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
	e->load(filestr);
	e->show();

}

void US_Finite_W::load_fit()
{
	QString fn = QFileDialog::getOpenFileName(USglobal->config_list.result_dir, "*.fef_fit.??", 0);
	if ( !fn.isEmpty() ) 
	{
		load_fit(fn);
	}
}

void US_Finite_W::load_fit(const QString &filename)
{
	unsigned int cell,lambda;
	QFile f(filename);
	fitName = filename;
	QString str;
	Q_UINT16 int16;
	Q_UINT16 temp_scans;
	Q_UINT32 int32;
	float val;
	if (f.open(IO_ReadOnly))
	{
		QDataStream ds(&f);
		ds >> str;	// run ID
		ds >> int16;
		cell = (unsigned int) int16;
		ds >> int16;
		lambda = (unsigned int) int16;
		ds >> int16;
		parameters = (unsigned int) int16;
		ds >> temp_scans;
//cout << "temp_scans: " << temp_scans << endl;
		fn = USglobal->config_list.result_dir + "/" + str + ".us.v";
		reset_flag = true;
		if (load_data(fn) != 0)
		{
			QMessageBox::message(tr("Attention:"), tr("There was a problem loading your fit.\n\n"
			                                            "Please try again."));
			return;
		}
		
//		details();
		this->selected_cell = cell;
		this->selected_lambda = lambda;
		this->selected_channel = 0;
		cell_select->setSelected(cell, true);
		lambda_select->setSelected(lambda, true);
		show_cell(cell);
		show_lambda(lambda);
		ds >> val;
		bd_range = val;
		ds >> val;
		bd_position = val;
		ds >> density;
		ds >> viscosity;
		ds >> standard_deviation;
		ds >> int32;
		sim_points = (unsigned int) int32;
		for (unsigned int i=0; i<(unsigned int) temp_scans; i++)
		{
			ds >> int32;
//cout << "scan: " << i << ", time_stored: " << int32 << ", run_inf.time: " <<  run_inf.time[selected_cell][selected_lambda][i] << endl;
			while (run_inf.time[selected_cell][selected_lambda][i] != int32)
			{
				exclude_single = i+1;
				ex_single();
			}	// exclude scans until the times match.
		}
// if there were scans excluded at the end of the run, we don't catch them with the exclusion algorithm
// above, so we need to catch them by comparing the true number of scans with the temp_scan variable

		while (run_inf.scans[selected_cell][selected_lambda] > (unsigned int) temp_scans)
		{
			exclude_single = run_inf.scans[selected_cell][selected_lambda];	// set to the last scan of the original dataset
			ex_single();
		}	// exclude scans at the end until the numbers of scans match
		
		plot_edit();
		ds >> int16;
		model = (unsigned int) int16;
		ds >> int16;
		components = (unsigned int) int16;
		step = 1;
		setup_model();
		ds >> run_vector->meniscus;
		ds >> run_vector->meniscus_range;
		ds >> int16;
		run_vector->meniscus_fit = (unsigned int) int16;
		ds >> run_vector->baseline;
		ds >> run_vector->baseline_range;
		ds >> int16;
		run_vector->baseline_fit = (unsigned int) int16;
		ds >> run_vector->slope;
		ds >> run_vector->slope_range;
		ds >> int16;
		run_vector->slope_fit = (unsigned int) int16;
		ds >> run_vector->stray;
		ds >> run_vector->stray_range;
		ds >> int16;
		run_vector->stray_fit = (unsigned int) int16;
		ds >> run_vector->delta_t;
		ds >> run_vector->delta_r;
		for (unsigned int i=0; i<components; i++)
		{
			ds >> component_vector[i].sed;
			ds >> component_vector[i].sed_range;
			ds >> int16;
			component_vector[i].sed_fit = (unsigned int) int16;
			ds >> component_vector[i].diff;
			ds >> component_vector[i].diff_range;
			ds >> int16;
			component_vector[i].diff_fit = (unsigned int) int16;
			ds >> component_vector[i].conc;
			ds >> component_vector[i].conc_range;
			ds >> int16;
			component_vector[i].conc_fit = (unsigned int) int16;
			ds >> component_vector[i].sigma;
			ds >> component_vector[i].sigma_range;
			ds >> int16;
			component_vector[i].sigma_fit = (unsigned int) int16;
			ds >> component_vector[i].delta;
			ds >> component_vector[i].delta_range;
			ds >> int16;
			component_vector[i].delta_fit = (unsigned int) int16;
			ds >> int16;
			component_vector[i].mw_fit = (unsigned int) int16;
			ds >> component_vector[i].vbar;
			ds >> int16;
			component_vector[i].vbar_fit = (unsigned int) int16;
		}
		update_buffer_lbl(density, viscosity);
		update_buffer_signal(density, viscosity);
		model_control->update_labels();
		dud_init();
		unsigned int count = 0;
		for (unsigned int i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
		{
			for (unsigned int j=0; j<run_inf.points[selected_cell][selected_lambda][0]; j++)
			{
				ds >> F[parameters][count];
				count++;
			}
		}
		converged = true;
		first_plot = true;
		update_plot(1);
		f.close();
		pb_second_plot->setEnabled(true);
		pb_save_fit->setEnabled(true);
		pb_save->setEnabled(true);
		pb_view->setEnabled(true);
	}
}

// With this function we can save the entire dataset, fit and run parameters in a binary file for later
// retrieval to do Monte Carlo, etc...

void US_Finite_W::save_fit()
{
	QString str;
	QString filename = USglobal->config_list.result_dir 
	+ "/" + run_inf.run_id + ".fef_fit." + str.sprintf("%d%d", selected_cell+1, selected_lambda+1);
	QFile f(filename);
	if (f.exists())
	{
		if(!QMessageBox::query( tr("Warning"), tr("Attention:\nThis file exists already!\n\nDo you want to overwrite it?"), tr("Yes"), tr("No")))
		{
			f.close();
			return;
		}
	}
	if (f.open(IO_WriteOnly))
	{
		QDataStream ds(&f);
		ds << (QString) run_inf.run_id;
		ds << (Q_UINT16) selected_cell;
		ds << (Q_UINT16) selected_lambda;
		ds << (Q_UINT16) parameters;
		ds << (Q_UINT16) run_inf.scans[selected_cell][selected_lambda];
		ds << (float) bd_range;
		ds << (float) bd_position;
		ds << (float) density;
		ds << (float) viscosity;
		ds << (float) pow((double) variance[parameters], (double) 0.5);
		ds << (Q_UINT32) sim_points;
		for (unsigned int i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
		{
			ds << (Q_UINT32) run_inf.time[selected_cell][selected_lambda][i];
		}
		ds << (Q_INT16) model;
		ds << (Q_INT16) components;
		ds << (float) run_vector->meniscus;
		ds << (float) run_vector->meniscus_range;
		ds << (Q_INT16) run_vector->meniscus_fit;
		ds << (float) run_vector->baseline;
		ds << (float) run_vector->baseline_range;
		ds << (Q_INT16) run_vector->baseline_fit;
		ds << (float) run_vector->slope;
		ds << (float) run_vector->slope_range;
		ds << (Q_INT16) run_vector->slope_fit;
		ds << (float) run_vector->stray;
		ds << (float) run_vector->stray_range;
		ds << (Q_INT16) run_vector->stray_fit;
		ds << (float) run_vector->delta_t;
		ds << (float) run_vector->delta_r;
		for (unsigned int i=0; i<components; i++)
		{
			ds << (float) component_vector[i].sed;
			ds << (float) component_vector[i].sed_range;
			ds << (Q_INT16) component_vector[i].sed_fit;
			ds << (float) component_vector[i].diff;
			ds << (float) component_vector[i].diff_range;
			ds << (Q_INT16) component_vector[i].diff_fit;
			ds << (float) component_vector[i].conc;
			ds << (float) component_vector[i].conc_range;
			ds << (Q_INT16) component_vector[i].conc_fit;
			ds << (float) component_vector[i].sigma;
			ds << (float) component_vector[i].sigma_range;
			ds << (Q_INT16) component_vector[i].sigma_fit;
			ds << (float) component_vector[i].delta;
			ds << (float) component_vector[i].delta_range;
			ds << (Q_INT16) component_vector[i].delta_fit;
			ds << (Q_INT16) component_vector[i].mw_fit;
			ds << (float) component_vector[i].vbar;
			ds << (Q_INT16) component_vector[i].vbar_fit;
		}
		unsigned int count=0;
		for (unsigned int i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
		{
			for (unsigned int j=0; j<run_inf.points[selected_cell][selected_lambda][0]; j++)
			{
				ds << (float) F[parameters][count];
				count++;
			}
		}
		f.close();
		pb_second_plot->setEnabled(true);
	}
	f.setName(USglobal->config_list.result_dir + "/" + run_inf.run_id
	+ str.sprintf("-fe.model.%d%d", selected_cell+1, selected_lambda+1));
	if (f.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&f);
		ts << "FE\n";
		ts << run_inf.cell_id[selected_cell] << "\n";
		ts << model << "\n";
		ts << components << "\n";
		for (i=0; i<components; i++)
		{
			ts << component_vector[i].conc << "\n";
			ts << component_vector[i].s20w << "\n";
			ts << component_vector[i].d20w << "\n";
			ts << component_vector[i].sigma << "\n";
			ts << component_vector[i].delta << "\n";
		}
		f.close();
	}
	// ti_noise already contains the constant baseline terms, now we add the average of each residual point
	ri_noise.clear();
	float fval;
	for (j=0; j<points; j++)
	{
		fval = 0.0;
		for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
		{
			fval += y[i][j] - absorbance[i][j]; 
		}
		fval /= run_inf.scans[selected_cell][selected_lambda];
		ti_noise[j] += fval;
	}
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		ri_noise.push_back(0.0);
	}
	
	f.setName(USglobal->config_list.result_dir + "/" + run_inf.run_id + str.sprintf("-fe.ti_noise.%d%d", selected_cell+1, selected_lambda+1));
	if(f.open(IO_WriteOnly))
	{
		QTextStream ts(&f);
		for (unsigned int i=0; i<ti_noise.size(); i++)
		{
			ts << ti_noise[i] << endl;
		}
		f.close();
	}
	f.setName(USglobal->config_list.result_dir + "/" + run_inf.run_id + str.sprintf("-fe.ri_noise.%d%d", selected_cell+1, selected_lambda+1));
	if(f.open(IO_WriteOnly))
	{
		QTextStream ts(&f);
		for (unsigned int i=0; i<ri_noise.size(); i++)
		{
			ts << ri_noise[i] << endl;
		}
		f.close();
	}
}

void US_Finite_W::write_fef()
{
	
	QString filename1, filename2, temp;
	temp.sprintf(".%d%d", selected_cell+1, selected_lambda+1);
	filename1 = USglobal->config_list.result_dir + "/" + run_inf.run_id + ".fe_dis" + temp;
	QFile f(filename1);
	unsigned int i;
	if (f.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&f);
	//
	// Arrange the file as follows: First, write to the file all apparent sedimentation
	// values, for each scan time one row which contains all divisions, starting at the 
	// lowest concentration. The first column has the times of each scan, then follow
	// the sedimentation coefficients for each division for that time:
	//
		QString str;
		float f_f0;
		ts << "S_apparent\tS_20,W    \tD_apparent\tD_20,W    \tMW        \tFrequency\tf/f0(20,W)\n"; 
		if (model >= 0 && model <= 3)
		{
			for (i=0; i<components; i++)
			{
				US_Hydro3 *shape;
				float vbar20 = component_vector[i].vbar - (4.25e-4 * (run_inf.avg_temperature - 20));
				shape = new US_Hydro3(0, &component_vector[i].s20w, &component_vector[i].d20w, 
											 &component_vector[i].mw, &vbar20, "", i);
				shape->calc();
				f_f0 = shape->f_ratio;
				if (f_f0 < 1) f_f0 = 1.0;
				ts << str.sprintf("%6.4e\t%6.4e\t%6.4e\t%6.4e\t%6.4e\t%6.4e\t%6.4e\n",
				component_vector[i].sed,
				component_vector[i].s20w,
				component_vector[i].diff,
				component_vector[i].d20w,
				component_vector[i].mw,
				component_vector[i].conc,
				f_f0);
				delete shape;
			}
		}
		f.close();
	}
/*
	QString filestr1, filestr2, filestr3, filename, temp, message;
	filename = USglobal->config_list.result_dir.copy();
	filename.append("/");
	filename.append(run_inf.run_id);
	temp.sprintf(".%d%d", selected_cell+1, selected_lambda+1);
	filestr1 = filename.copy();
	filestr1.append(".fef_dat");
	filestr1.append(temp);
	QFile fef_f1(filestr1);
	unsigned int i, j;
	int f1_error, f2_error, f3_error;
	if (fef_f1.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&fef_f1);
		ts << tr("Radius:\t");
		for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
		{	
			ts << tr("Scan-") << (i+1) << "\t";
		}
		ts << "\n";
		for (j=0; j<points; j++)
		{
			ts << radius[j] << "\t";
			for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
			{	
				ts << y[i][j] << "\t";
			}
			ts << "\n";
		}
		fef_f1.close();
		f1_error=0;
	}
	else
	{
		f1_error=1;
	}
	filename = "";
	filename = USglobal->config_list.result_dir.copy();
	filename.append("/");
	filename.append(run_inf.run_id);
	temp.sprintf(".%d%d", selected_cell+1, selected_lambda+1);
	filestr2 = filename.copy();
	filestr2.append(".fef_residuals");
	filestr2.append(temp);
	QFile fef_f2(filestr2);
	if (fef_f2.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&fef_f2);
		ts << tr("Radius:\t");
		for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
		{	
			ts << tr("Scan-") << (i+1) << "\t";
		}
		ts << "\n";
		for (j=0; j<points; j++)
		{
			ts << radius[j] << "\t";
			for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
			{	
				ts << residuals[i][j] << "\t";
			}
			ts << "\n";
		}
		fef_f2.close();
		f2_error=0;
	}
	else
	{
		f2_error=1;
	}
	filename = "";
	filename = USglobal->config_list.result_dir.copy();
	filename.append("/");
	filename.append(run_inf.run_id);
	temp.sprintf(".%d%d", selected_cell+1, selected_lambda+1);
	filestr3 = filename.copy();
	filestr3.append(".fef_raw");
	filestr3.append(temp);
	QFile fef_f3(filestr3);
	if (fef_f3.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&fef_f3);
		ts << tr("Radius:\t");
		for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
		{	
			ts << tr("Scan-") << (i+1) << "\t";
		}
		ts << "\n";
		for (j=0; j<points; j++)
		{
			ts << radius[j] << "\t";
			for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
			{	
				ts << absorbance[i][j] << "\t";
			}
			ts << "\n";
		}
		fef_f3.close();
		f3_error=0;
	}
	else
	{
		f3_error=1;
	}
	message = tr("The following files where saved to disk:\n\n");
	if (!f1_error)
	{
		message.append(tr("The Finite Element Simulation:\n"));
		message.append(filestr1);
		message.append("\n\n");
	}
	if (!f2_error)
	{
		message.append(tr("The Residuals of the Fit:\n"));
		message.append(filestr2);
		message.append("\n\n");
	}
	if (!f3_error)
	{
		message.append(tr("The Experimental Data:\n"));
		message.append(filestr3);
		message.append("\n");
	}
	message.append("\n");
	if (f1_error || f2_error || f3_error)
	{
		message.append(tr("There was a problem saving the following files:\n\n"));
	}
	if (f1_error)
	{
		message.append(tr("The Finite Element Simulation:\n"));
		message.append(filestr1);
		message.append("\n\n");
	}
	if (f2_error)
	{
		message.append(tr("The Residuals of the Fit:\n"));
		message.append(filestr2);
		message.append("\n\n");
	}
	if (f3_error)
	{
		message.append(tr("The Experimental Data:\n"));
		message.append(filestr3);
		message.append("\n");
	}
	QMessageBox::message(tr("Please note:\n"), message);
*/
}

void US_Finite_W::write_res()
{
	QString filestr, filename, temp, model_str, str1;
	unsigned int i;
	filename = USglobal->config_list.result_dir.copy();
	filename.append("/");
	filename.append(run_inf.run_id);
	temp.sprintf(".%d%d", selected_cell+1, selected_lambda+1);
	filestr = filename.copy();
	filestr.append(".fef_res");
	filestr.append(temp);
	QFile res_f(filestr);
	if (res_f.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&res_f);
		ts << "***************************************************\n";
		ts << tr("*           Finite Element Fit Analysis           *\n");
		ts << "***************************************************\n\n\n";
		ts << tr("Data Report for Run \"") << run_inf.run_id << tr("\", Cell ") << (selected_cell + 1) 
		 << tr(", Wavelength ") << (selected_lambda + 1) << "\n\n";
		ts << tr("Detailed Run Information:\n\n");
		ts << tr("Cell Description:            ") << run_inf.cell_id[selected_cell] << "\n";
		ts << tr("Raw Data Directory:          ") << run_inf.data_dir << "\n";
		ts << tr("Rotor Speed:                 ") << run_inf.rpm[first_cell][0][0]  << " rpm\n";
		ts << tr("Average Temperature:         ") << run_inf.avg_temperature << " ºC\n" ;
		if (run_inf.temperature_check == 1)
		{
			ts << tr("Temperature Variation:       Within Tolerance\n");
		}
		if (run_inf.temperature_check == -1)
		{
			ts << tr("Temperature Variation: (!) OUTSIDE TOLERANCE (!)\n");
		}
		i = (unsigned int) (run_inf.time_correction / 60);
		j = (unsigned int) (0.5 + (run_inf.time_correction - (i * 60)));
		ts << tr("Time Correction:             ") << i << tr(" minute(s) ") << j << tr(" second(s)\n");
		i = (unsigned int) (run_inf.duration / 3600);
		j = (unsigned int) (0.5 + (run_inf.duration - i * 3600) / 60);
		ts << tr("Run Duration:                ") << i << tr(" hour(s) ") << j << tr(" minute(s)\n");
		ts << tr("Wavelength:                  ") << run_inf.wavelength[selected_cell][selected_lambda] << " nm\n";
		if (run_vector->baseline_fit)
		{
			ts << tr("Baseline Absorbance:         ") << run_vector->baseline << tr(" OD (fitted)\n");
		}
		else
		{
			ts << tr("Baseline Absorbance:         ") << run_vector->baseline << tr(" OD (fixed)\n");
		}
		if (run_vector->meniscus_fit)
		{
			ts << tr("Meniscus Position:           ") << run_vector->meniscus << tr(" cm (fitted)\n");
		}
		else
		{
			ts << tr("Meniscus Position:           ") << run_vector->meniscus << tr(" cm (fixed)\n");
		}
		if (run_vector->slope_fit)
		{
			ts << tr("Slope Error Correction(r):   ") << run_vector->slope << tr(" OD (fitted)\n");
		}
		else
		{
			ts << tr("Slope Error Correction(r):   ") << run_vector->slope << tr(" OD (fixed)\n");
		}
		if (run_vector->stray_fit)
		{
			ts << tr("Slope Error Correction(c,r): ") << run_vector->stray << tr(" OD (fitted)\n");
		}
		else
		{
			ts << tr("Slope Error Correction(c,r): ") << run_vector->stray << tr(" OD (fixed)\n") ;
		}
		ts << tr("Edited Data starts at:       ") << run_inf.range_left[selected_cell][selected_lambda][0] << " cm\n";
		ts << tr("Edited Data stops at:        ") << run_inf.range_right[selected_cell][selected_lambda][0] << " cm\n\n\n";
		ts << tr("Hydrodynamic Settings:\n\n");
		ts << tr("Viscosity correction:        ") << viscosity << "\n" ;
		ts << tr("Viscosity (absolute):        ") << viscosity_tb << "\n" ;
		ts << tr("Density correction:          ") << density << " g/ccm\n" ;
		ts << tr("Density (absolute):          ") << density_tb << " g/ccm\n\n\n" ;
		ts << tr("Data Analysis Settings:\n\n");
		ts << tr("Analyzed Boundary:           ") << bd_range << " %\n";
		ts << tr("Boundary Position:           ") << bd_position << " %\n\n\n";

		ts << tr("Fitting Information:\n\n");
		ts << tr("Variance:                    ") << variance[parameters] << "\n";
		ts << tr("Standard Deviation           ") << pow((double) variance[parameters], (double) 0.5) << "\n";
		ts << tr("# of Iterations:             ") << iteration << "\n";
		ts << tr("# of Function Evals.:        ") << func_eval << "\n";		
		ts << tr("# of fitted Parameters:      ") << parameters << "\n";
		ts << tr("# of fitted Data Points:     ") << allpoints << "\n";
		
		run_vector->runs_percent = (float) (run_vector->pos + run_vector->neg);
		run_vector->runs_percent = run_vector->runs_percent * (run_inf.delta_r/run_inf.point_density[selected_cell][selected_lambda][0]);
		run_vector->runs_percent = (float) run_vector->runs / run_vector->runs_percent;
		run_vector->runs_percent *= 100.0;
		str1.sprintf(tr("# of Runs in the Fit:        %ld (%2.3f"), run_vector->runs, run_vector->runs_percent);
		ts << str1 << tr(" %, corrected)\n");
//
// Formula for expected number of runs (P = nummber of positives, N = number of Negatives):
// E(r) = 1 + 2PN/(P + N)
// Formula for its variance:
// V(R) = [2PN (2PN - P - N)] / [(P + N)^2 (P + N - 1)]
//
		run_vector->runs_expected = 1.0 + (2.0 * run_vector->pos * run_vector->neg) / (run_vector->pos + run_vector->neg);
		run_vector->runs_expected = run_vector->runs_expected * (run_inf.delta_r/run_inf.point_density[selected_cell][selected_lambda][0]);
		run_vector->runs_variance = ((2.0 * run_vector->pos * run_vector->neg) *	(2.0 * run_vector->pos * run_vector->neg - run_vector->pos - run_vector->neg))
		/ ((pow((double) (run_vector->pos + run_vector->neg), (double) 2)) * (run_vector->pos + run_vector->neg - 1.0));
		str1.sprintf(tr("Expected Number of Runs:     %ld\nRun Variance:                %2.3e\n\n"), (long) run_vector->runs_expected, run_vector->runs_variance);		ts << str1;
		if (run_vector->runs_percent < 25.0)
		{
			ts << tr("According to these statistical tests, this model is either inappropriate for the\n");
			ts << tr("experimental data, or the fitting process has not yet converged. Please try to reduce\n");
			ts << tr("the variance by additional nonlinear least-squares minimization of the data.\n");
			ts << tr("This fit cannot be used for a Monte Carlo Analysis.\n");
		}
		if (run_vector->runs_percent > 25.0 && run_vector->runs_percent < 30)
		{
			ts << tr("According to these statistical tests, this model is either a poor candidate for the\n");
			ts << tr("experimental data, or the fitting process has not yet converged. Please try to reduce\n");
			ts << tr("the variance by additional nonlinear least-squares minimization of the data.\n");
			ts << tr("This fit cannot be used for a Monte Carlo Analysis.\n");
		}
		if (run_vector->runs_percent > 30.0 && run_vector->runs_percent < 35)
		{
			ts << tr("According to these statistical tests, this model is either a mediocre candidate\n");
			ts << tr("for the experimental data, or the fitting process has not yet converged. Please try to\n");
			ts << tr("reduce the variance by additional nonlinear least-squares minimization of the data.\n");
			ts << tr("This fit should not be used for a Monte Carlo Analysis.\n");
		}
		if (run_vector->runs_percent > 35.0 && run_vector->runs_percent < 40)
		{
			ts << tr("According to these statistical tests, this model is a good candidate for the experimental data.\n");
			ts << tr("This fit may be used for a Monte Carlo Analysis with reservations.\n");
		}
		if (run_vector->runs_percent > 40.0)
		{
			ts << tr("According to these statistical tests, this model is an excellent candidate for the experimental data.\n");
			ts << tr("Monte Carlo Analysis is recommended for this data.\n");
		}

		ts << tr("\nComponent Information:\n\n");
		
		if (model >= 0 && model <= 3)
		{
			for (i=0; i<components; i++)
			{
				US_Hydro3 *shape;
				float vbar20 = component_vector[i].vbar - (4.25e-4 * (run_inf.avg_temperature - 20));
				model_str.sprintf(tr("Molecular Shape Model for Component %d"), i+1);
				shape = new US_Hydro3(0, &component_vector[i].s20w, &component_vector[i].d20w, 
											 &component_vector[i].mw, &vbar20, model_str, i);
				ts << tr("Component ") << (i+1) << ":\n";
				if (component_vector[i].mw_fit)
				{
					ts << tr("Molecular Weight:            ") << component_vector[i].mw << tr(" Dalton") << tr(" (fitted)\n");
				}
				else
				{
					ts << tr("Molecular Weight:            ") << component_vector[i].mw << tr(" Dalton") << tr(" (fixed)\n");
				}
				if (component_vector[i].sed_fit)
				{
					ts << tr("Sed. Coeff. (s20,W):         ") << component_vector[i].s20w << " s" << tr(" (fitted)\n");
				}
				else
				{
					ts << tr("Sed. Coeff. (s20,W):         ") << component_vector[i].s20w << " s" << tr(" (fixed)\n");
				}
				if (component_vector[i].sed_fit)
				{
					ts << tr("Sedimentation Coeff.:        ") << component_vector[i].sed << " s" << tr(" (fitted)\n");
				}
				else
				{
					ts << tr("Sedimentation Coeff.:        ") << component_vector[i].sed << " s" << tr(" (fixed)\n");
				}
				if (component_vector[i].diff_fit)
				{
					ts << tr("Diff. Coeff. (D20,W):        ") << component_vector[i].d20w << " D" << tr(" (fitted)\n");
				}
				else
				{
					ts << tr("Diff. Coeff. (D20,W):        ") << component_vector[i].d20w << " D" << tr(" (fixed)\n");
				}
				if (component_vector[i].diff_fit)
				{
					ts << tr("Diffusion Coeff.:            ") << component_vector[i].diff << " D" << tr(" (fitted)\n");
				}
				else
				{
					ts << tr("Diffusion Coeff.:            ") << component_vector[i].diff << " D" << tr(" (fixed)\n");
				}
				if (component_vector[i].conc_fit)
				{
					ts << tr("Partial Concentration:       ") << component_vector[i].conc << " OD" << tr(" (fitted)\n");
				}
				else
				{
					ts << tr("Partial Concentration:       ") << component_vector[i].conc << " OD" << tr(" (fixed)\n");
				}
				if (component_vector[i].vbar_fit)
				{
					ts << tr("Partial Spec. Vol. (vbar):   ") << (component_vector[i].vbar - (4.25e-4 
																 * (run_inf.avg_temperature - 20))) << tr(" ccm/g (fitted)\n") ;
				}
				else
				{
					ts << tr("Partial Spec. Vol. (vbar):   ") << (component_vector[i].vbar - (4.25e-4 
																 * (run_inf.avg_temperature - 20))) << tr(" ccm/g (fixed)\n") ;
				}
				if (component_vector[i].vbar_fit)
				{
					ts << tr("vbar (Corr. for 20ºC):       ") << component_vector[i].vbar << tr(" ccm/g (fitted)\n") ;
				}
				else
				{
					ts << tr("vbar (Corr. for 20ºC):       ") << component_vector[i].vbar << tr(" ccm/g (fixed)\n") ;
				}
				if (component_vector[i].sigma_fit)
				{
					ts << tr("Conc. Dep. of s (Sigma):     ") << component_vector[i].sigma << tr(" (fitted)\n") ;
				}
				else
				{
					ts << tr("Conc. Dep. of s (Sigma):     ") << component_vector[i].sigma << tr(" (fixed)\n") ;
				}
				if (component_vector[i].delta_fit)
				{
					ts << tr("Conc. Dep. of D (Delta):     ") << component_vector[i].delta << tr(" (fitted)\n") ;
				}
				else
				{
					ts << tr("Conc. Dep. of D (Delta):     ") << component_vector[i].delta << tr(" (fixed)\n") ;
				}
				ts << tr("Buoyancy (Water, 20ºC):      ") << (1 - (component_vector[i].vbar - (4.25e-4 
																 * (run_inf.avg_temperature - 20))) * DENS_20W) << "\n" ;
				ts << tr("Buoyancy (absolute):         ") << (1 - component_vector[i].vbar * density_tb) << "\n\n" ;
				ts << tr("Hydrodynamic Modeling Information:\n\n");
				shape->calc();
				if (shape->f_ratio < 1.0)
				{
					ts << tr("\nAttention:\n");
					ts << tr("The frictional ratio F/F0 is less than 1.0, which is physically impossible.\n");
					ts << tr("The fitting results to this component are non-sensical, and should not be\n");
					ts << tr("used for data interpretation.\n\n\n");
				}
				else
				{
					ts << tr("Radius of Minimal Sphere:    ") << shape->rad_sphere * 1.0e08 <<" Å\n";
					ts << tr("Radius of Stoke's Sphere:    ") << shape->frict_coeff * 1.0e10 / (6*M_PI*viscosity_tb) <<" Å\n";
					ts << tr("Frictional Coefficient:      ") << shape->frict_coeff <<"\n";
					ts << tr("Minimal Sphere F0:           ") << shape->f0 <<"\n";
					ts << tr("Frictional Ratio (F/F0):     ") << shape->f_ratio <<"\n";
					ts << tr("Volume of Molecule:          ") << shape->vol_per_molecule <<" Å^3\n\n";
					ts << tr("Possible Hydrodynamic Models for this Molecule (hypothetical only!):\n\n");
					ts << tr("Prolate Ellipsoid:\n");
					ts << tr("Length of Semi-Major Axis:   ") << shape->ap << " Å\n";
					ts << tr("Length of Semi-Minor Axis:   ") << shape->bp << " Å\n";
					ts << tr("Axial Ratio:                 ") << shape->ratio_p << "\n\n"; 
					ts << tr("Oblate Ellipsoid:\n");
					ts << tr("Length of Semi-Major Axis:   ") << shape->ao << " Å\n";
					ts << tr("Length of Semi-Minor Axis:   ") << shape->bo << " Å\n";
					ts << tr("Axial Ratio:                 ") << shape->ratio_o << "\n\n"; 
					ts << tr("Long Rod:\n");
					if (shape->f_ratio > 1.32)
					{
						ts << tr("Length of Semi-Major Axis:   ") << shape->ar << " Å\n";
						ts << tr("Length of Semi-Minor Axis:   ") << shape->br << " Å\n";
						ts << tr("Axial Ratio:                 ") << shape->ratio_r << "\n\n\n"; 
					}
					else
					{
						ts << tr("F/F0 < 1.32, this model is not applicable\n\n\n");
					}
				}
				delete shape;
			}
		}
		if (model == 4)
		{
			for (i=0; i<components; i++)
			{
				if (i == 0)
				{
					ts << tr("Monomer:\n");
					model_str.sprintf(tr("Molecular Shape Model for Monomer:"));
				}
				else
				{
					ts << tr("Dimer:\n");
					model_str.sprintf(tr("Molecular Shape Model for Dimer:"));
				}
				US_Hydro3 *shape;
				float vbar20 = component_vector[i].vbar - (4.25e-4 * (run_inf.avg_temperature - 20));
				shape = new US_Hydro3(0, &component_vector[i].s20w, &component_vector[i].d20w, 
											 &component_vector[i].mw, &vbar20, model_str, i);
				if (component_vector[i].mw_fit)
				{
					ts << tr("Molecular Weight:            ") << component_vector[i].mw << tr(" Dalton") << tr(" (fitted)\n");
				}
				else
				{
					ts << tr("Molecular Weight:            ") << component_vector[i].mw << tr(" Dalton") << tr(" (fixed)\n");
				}
				if (component_vector[i].sed_fit)
				{
					ts << tr("Sed. Coeff. (s20,W):         ") << component_vector[i].s20w << " s" << tr(" (fitted)\n");
				}
				else
				{
					ts << tr("Sed. Coeff. (s20,W):         ") << component_vector[i].s20w << " s" << tr(" (fixed)\n");
				}
				if (component_vector[i].sed_fit)
				{
					ts << tr("Sedimentation Coeff.:        ") << component_vector[i].sed << " s" << tr(" (fitted)\n");
				}
				else
				{
					ts << tr("Sedimentation Coeff.:        ") << component_vector[i].sed << " s" << tr(" (fixed)\n");
				}
				if (component_vector[i].diff_fit)
				{
					ts << tr("Diff. Coeff. (D20,W):        ") << component_vector[i].d20w << " D" << tr(" (fitted)\n");
				}
				else
				{
					ts << tr("Diff. Coeff. (D20,W):        ") << component_vector[i].d20w << " D" << tr(" (fixed)\n");
				}
				if (component_vector[i].diff_fit)
				{
					ts << tr("Diffusion Coeff.:            ") << component_vector[i].diff << " D" << tr(" (fitted)\n");
				}
				else
				{
					ts << tr("Diffusion Coeff.:            ") << component_vector[i].diff << " D" << tr(" (fixed)\n");
				}
				if (i == 0)
				{
					if (component_vector[i].conc_fit)
					{
						ts << tr("Total Concentration:         ") << component_vector[i].conc << " OD" << tr(" (fitted)\n");
					}
					else
					{
						ts << tr("Total Concentration:         ") << component_vector[i].conc << " OD" << tr(" (fixed)\n");
					}
				}
				else
				{
					if (component_vector[i].conc_fit)
					{
						ts << tr("Equilibrium Constant:        ") << component_vector[i].conc << " OD" << tr(" (fitted)\n");
					}
					else
					{
						ts << tr("Equilibrium Constant:        ") << component_vector[i].conc << " OD" << tr(" (fixed)\n");
					}
				}
				if (component_vector[i].vbar_fit)
				{
					ts << tr("Partial Spec. Vol. (vbar):   ") << (component_vector[i].vbar - (4.25e-4 
																 * (run_inf.avg_temperature - 20))) << tr(" ccm/g (fitted)\n") ;
				}
				else
				{
					ts << tr("Partial Spec. Vol. (vbar):   ") << (component_vector[i].vbar - (4.25e-4 
																 * (run_inf.avg_temperature - 20))) << tr(" ccm/g (fixed)\n") ;
				}
				if (component_vector[i].vbar_fit)
				{
					ts << tr("vbar (Corr. for 20ºC):       ") << component_vector[i].vbar << tr(" ccm/g (fitted)\n") ;
				}
				else
				{
					ts << tr("vbar (Corr. for 20ºC):       ") << component_vector[i].vbar << tr(" ccm/g (fixed)\n") ;
				}
				if (component_vector[i].sigma_fit)
				{
					ts << tr("Conc. Dep. of s (Sigma):     ") << component_vector[i].sigma << tr(" (fitted)\n") ;
				}
				else
				{
					ts << tr("Conc. Dep. of s (Sigma):     ") << component_vector[i].sigma << tr(" (fixed)\n") ;
				}
				if (component_vector[i].delta_fit)
				{
					ts << tr("Conc. Dep. of D (Delta):     ") << component_vector[i].delta << tr(" (fitted)\n") ;
				}
				else
				{
					ts << tr("Conc. Dep. of D (Delta):     ") << component_vector[i].delta << tr(" (fixed)\n") ;
				}
				ts << tr("Buoyancy (Water, 20ºC):      ") << (1 - (component_vector[i].vbar - (4.25e-4 
																 * (run_inf.avg_temperature - 20))) * DENS_20W) << "\n" ;
				ts << tr("Buoyancy (absolute):         ") << (1 - component_vector[i].vbar * density_tb) << "\n\n" ;
				ts << tr("Hydrodynamic Modeling Information:\n\n");
				shape->calc();
				if (shape->f_ratio < 1.0)
				{
					ts << tr("\nAttention:\n");
					ts << tr("The frictional ratio F/F0 is less than 1.0, which is physically impossible.\n");
					ts << tr("The fitting results to this component are non-sensical, and should not be\n");
					ts << tr("used for data interpretation.\n\n\n");
				}
				else
				{
					ts << tr("Radius of Minimal Sphere:    ") << shape->rad_sphere * 1.0e08 <<" Å\n";
					ts << tr("Radius of Stoke's Sphere:    ") << shape->frict_coeff * 1.0e10/(6*M_PI*viscosity_tb) <<" Å\n";
					ts << tr("Frictional Coefficient:      ") << shape->frict_coeff <<"\n";
					ts << tr("Minimal Sphere F0:           ") << shape->f0 <<"\n";
					ts << tr("Frictional Ratio (F/F0):     ") << shape->f_ratio <<"\n";
					ts << tr("Volume of Molecule:          ") << shape->vol_per_molecule <<" Å^3\n\n";
					ts << tr("Possible Hydrodynamic Models for this Molecule (hypothetical only!):\n\n");
					ts << tr("Prolate Ellipsoid:\n");
					ts << tr("Length of Semi-Major Axis:   ") << shape->ap << " Å\n";
					ts << tr("Length of Semi-Minor Axis:   ") << shape->bp << " Å\n";
					ts << tr("Axial Ratio:                 ") << shape->ratio_p << "\n\n"; 
					ts << tr("Oblate Ellipsoid:\n");
					ts << tr("Length of Semi-Major Axis:   ") << shape->ao << " Å\n";
					ts << tr("Length of Semi-Minor Axis:   ") << shape->bo << " Å\n";
					ts << tr("Axial Ratio:                 ") << shape->ratio_o << "\n\n"; 
					ts << tr("Long Rod:\n");
					if (shape->f_ratio > 1.32)
					{
						ts << tr("Length of Semi-Major Axis:   ") << shape->ar << " Å\n";
						ts << tr("Length of Semi-Minor Axis:   ") << shape->br << " Å\n";
						ts << tr("Axial Ratio:                 ") << shape->ratio_r << "\n\n\n"; 
					}
					else
					{
						ts << tr("F/F0 < 1.32, this model is not applicable\n\n\n");
					}
				}
				delete shape;
			}
		}
		if (model == 20)
		{
			for (i=0; i<components; i++)
			{
				ts << tr("Isomer ") << (i+1) << ":\n";
				US_Hydro3 *shape;
				float vbar20 = component_vector[i].vbar - (4.25e-4 * (run_inf.avg_temperature - 20));
				model_str.sprintf(tr("Molecular Shape Model for Isomer %d"), i+1);
				shape = new US_Hydro3(0, &component_vector[i].s20w, &component_vector[i].d20w, 
											 &component_vector[i].mw, &vbar20, model_str, i);
				if (component_vector[i].mw_fit)
				{
					ts << tr("Molecular Weight:            ") << component_vector[i].mw << tr(" Dalton") << tr(" (fitted)\n");
				}
				else
				{
					ts << tr("Molecular Weight:            ") << component_vector[i].mw << tr(" Dalton") << tr(" (fixed)\n");
				}
				if (component_vector[i].sed_fit)
				{
					ts << tr("Sed. Coeff. (s20,W):         ") << component_vector[i].s20w << " s" << tr(" (fitted)\n");
				}
				else
				{
					ts << tr("Sed. Coeff. (s20,W):         ") << component_vector[i].s20w << " s" << tr(" (fixed)\n");
				}
				if (component_vector[i].sed_fit)
				{
					ts << tr("Sedimentation Coeff.:        ") << component_vector[i].sed << " s" << tr(" (fitted)\n");
				}
				else
				{
					ts << tr("Sedimentation Coeff.:        ") << component_vector[i].sed << " s" << tr(" (fixed)\n");
				}
				if (component_vector[i].diff_fit)
				{
					ts << tr("Diff. Coeff. (D20,W):        ") << component_vector[i].d20w << " D" << tr(" (fitted)\n");
				}
				else
				{
					ts << tr("Diff. Coeff. (D20,W):        ") << component_vector[i].d20w << " D" << tr(" (fixed)\n");
				}
				if (component_vector[i].diff_fit)
				{
					ts << tr("Diffusion Coeff.:            ") << component_vector[i].diff << " D" << tr(" (fitted)\n");
				}
				else
				{
					ts << tr("Diffusion Coeff.:        ") << component_vector[i].diff << " D" << tr(" (fixed)\n");
				}
				if (i == 0)
				{
					if (component_vector[i].conc_fit)
					{
						ts << tr("Total Concentration:     ") << component_vector[i].conc << " OD" << tr(" (fitted)\n");
					}
					else
					{
						ts << tr("Total Concentration:     ") << component_vector[i].conc << " OD" << tr(" (fixed)\n");
					}
				}
				else
				{
					if (component_vector[i].conc_fit)
					{
						ts << tr("Equilibrium Constant:    ") << component_vector[i].conc << " OD" << tr(" (fitted)\n");
					}
					else
					{
						ts << tr("Equilibrium Constant:    ") << component_vector[i].conc << " OD" << tr(" (fixed)\n");
					}
				}
				if (component_vector[i].vbar_fit)
				{
					ts << tr("Partial Spec. Vol. (vbar):   ") << (component_vector[i].vbar - (4.25e-4 
																 * (run_inf.avg_temperature - 20))) << tr(" ccm/g (fitted)\n") ;
				}
				else
				{
					ts << tr("Partial Spec. Vol. (vbar):   ") << (component_vector[i].vbar - (4.25e-4 
																 * (run_inf.avg_temperature - 20))) << tr(" ccm/g (fixed)\n") ;
				}
				if (component_vector[i].vbar_fit)
				{
					ts << tr("vbar (Corr. for 20ºC):       ") << component_vector[i].vbar << tr(" ccm/g (fitted)\n") ;
				}
				else
				{
					ts << tr("vbar (Corr. for 20ºC):       ") << component_vector[i].vbar << tr(" ccm/g (fixed)\n") ;
				}
				if (component_vector[i].sigma_fit)
				{
					ts << tr("Conc. Dep. of s (Sigma):     ") << component_vector[i].sigma << tr(" (fitted)\n") ;
				}
				else
				{
					ts << tr("Conc. Dep. of s (Sigma):     ") << component_vector[i].sigma << tr(" (fixed)\n") ;
				}
				if (component_vector[i].delta_fit)
				{
					ts << tr("Conc. Dep. of D (Delta):     ") << component_vector[i].delta << tr(" (fitted)\n") ;
				}
				else
				{
					ts << tr("Conc. Dep. of D (Delta):     ") << component_vector[i].delta << tr(" (fixed)\n") ;
				}
				ts << tr("Buoyancy (Water, 20ºC):      ") << (1 - (component_vector[i].vbar - (4.25e-4 
																 * (run_inf.avg_temperature - 20))) * DENS_20W) << "\n" ;
				ts << tr("Buoyancy (absolute):         ") << (1 - component_vector[i].vbar * density_tb) << "\n\n" ;
				ts << tr("Hydrodynamic Modeling Information:\n\n");
				shape->calc();
				if (shape->f_ratio < 1.0)
				{
					ts << tr("\nAttention:\n");
					ts << tr("The frictional ratio F/F0 is less than 1.0, which is physically impossible.\n");
					ts << tr("The fitting results to this component are non-sensical, and should not be\n");
					ts << tr("used for data interpretation.\n\n\n");
				}
				else
				{
					ts << tr("Radius of Minimal Sphere:    ") << shape->rad_sphere * 1.0e08 <<" Å\n";
					ts << tr("Radius of Stoke's Sphere:    ") << shape->frict_coeff * 1.0e10/(6*M_PI*viscosity_tb) <<" Å\n";
					ts << tr("Frictional Coefficient:      ") << shape->frict_coeff <<"\n";
					ts << tr("Minimal Sphere F0:           ") << shape->f0 <<"\n";
					ts << tr("Frictional Ratio (F/F0):     ") << shape->f_ratio <<"\n";
					ts << tr("Volume of Molecule:          ") << shape->vol_per_molecule <<" Å^3\n\n";
					ts << tr("Possible Hydrodynamic Models for this Molecule (hypothetical only!):\n\n");
					ts << tr("Prolate Ellipsoid:\n");
					ts << tr("Length of Semi-Major Axis:   ") << shape->ap << " Å\n";
					ts << tr("Length of Semi-Minor Axis:   ") << shape->bp << " Å\n";
					ts << tr("Axial Ratio:                 ") << shape->ratio_p << "\n\n"; 
					ts << tr("Oblate Ellipsoid:\n");
					ts << tr("Length of Semi-Major Axis:   ") << shape->ao << " Å\n";
					ts << tr("Length of Semi-Minor Axis:   ") << shape->bo << " Å\n";
					ts << tr("Axial Ratio:                 ") << shape->ratio_o << "\n\n"; 
					ts << tr("Long Rod:\n");
					if (shape->f_ratio > 1.32)
					{
						ts << tr("Length of Semi-Major Axis:   ") << shape->ar << " Å\n";
						ts << tr("Length of Semi-Minor Axis:   ") << shape->br << " Å\n";
						ts << tr("Axial Ratio:                 ") << shape->ratio_r << "\n\n\n"; 
					}
					else
					{
						ts << tr("F/F0 < 1.32, this model is not applicable\n\n\n");
					}
				}
				delete shape;
			}
		}

		ts << tr("\nScan Information: \n\n");
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

