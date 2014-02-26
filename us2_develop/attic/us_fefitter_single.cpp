// points must be # of scans * experimental datapoints per scan
// 

#include "../include/us_fefitter_single.h"

US_FeFitter_Single::US_FeFitter_Single(	bool *temp_fitting_widget, 
											bool temp_GUI, 
											unsigned int temp_components,
											int temp_model,
											vector <struct fefit_component> *temp_component_vector,
											struct fefit_run *temp_runData,
											struct runinfo *temp_run_inf,
											unsigned int temp_scanpoints,
											unsigned int temp_cell,
											unsigned int temp_lambda,
											float temp_bd_range,
											float temp_bd_position,
											float temp_density,
											float temp_density_tb,
											float temp_viscosity,
											float temp_viscosity_tb,
											float temp_bottom,
											float *temp_variance,
											double ** temp_absorbance,
											double ** temp_simulation_c,
											double ** temp_resid,
											double *temp_radius,
											QWidget *p,
											const char *name) : US_Minimize(	temp_fitting_widget,
																						temp_GUI,
																						p,
																						name)
{
	tape_construct = true;
	init_flag = false;	// keeps track if memory has been initialized already
	fitting_widget = temp_fitting_widget;
	GUI = temp_GUI;
	components = temp_components;
	model = temp_model;
	component_vector = temp_component_vector;
	runData = temp_runData;
	run_inf = temp_run_inf;
	scanpoints = temp_scanpoints;
	cell = temp_cell;
	lambda = temp_lambda;
	bd_range = temp_bd_range;
	bd_position = temp_bd_position;
	density = temp_density;
	density_tb = temp_density_tb;
	viscosity = temp_viscosity;
	viscosity_tb = temp_viscosity_tb;
	bottom = temp_bottom;
	variance = *temp_variance;
	absorbance = temp_absorbance;
	simulation_c = temp_simulation_c;
	resid = temp_resid;
	radius = temp_radius;
	USglobal = new US_Config();
	conc_dep_flag = false;
	print_bw = false;
	print_inv = true;
	createHtmlDir();
	if (GUI)
	{
		lbl_header->setText(tr("Finite Element Fitting Control"));
		bt_plotAll->setEnabled(false);
		bt_plotGroup->setEnabled(false);
		cnt_scan->setEnabled(false);
		residuals_plot = new QwtPlot(this);
		Q_CHECK_PTR(residuals_plot);
		residuals_plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
		residuals_plot->enableGridXMin();
		residuals_plot->enableGridYMin();
		residuals_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
		residuals_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
		residuals_plot->setCanvasBackground(USglobal->global_colors.plot);
		residuals_plot->enableOutline(true);
		residuals_plot->setAxisTitle(QwtPlot::xBottom, "");
		residuals_plot->setAxisTitle(QwtPlot::yLeft, "");
		residuals_plot->setTitle(" ");
		residuals_plot->setOutlinePen(white);
		residuals_plot->setOutlineStyle(Qwt::Cross);
		residuals_plot->setMargin(USglobal->config_list.margin);
		residuals_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
		residuals_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
		residuals_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
		residuals_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
		residuals_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
		residuals_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));		residuals_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));		
	
	}
}

US_FeFitter_Single::~US_FeFitter_Single()
{
	cleanup();
}

void US_FeFitter_Single::assign_orig()
{
	veloc_par.sed.clear();
	veloc_par.diff.clear();
	veloc_par.conc.clear();
	veloc_par.sigma.clear();
	veloc_par.delta.clear();
	veloc_par.vbar.clear();
	
	veloc_par.meniscus 	= (*runData).meniscus;
	veloc_par.baseline 	= (*runData).baseline;
	veloc_par.slope 		= (*runData).slope;
	veloc_par.stray 		= (*runData).stray;
	for (unsigned int i=0; i<components; i++)
	{
		veloc_par.sed.push_back((float) (*component_vector)[i].sed);
		veloc_par.diff.push_back((float) (*component_vector)[i].diff);
		veloc_par.conc.push_back((float) (*component_vector)[i].conc);
		veloc_par.sigma.push_back((float) (*component_vector)[i].sigma);
		veloc_par.delta.push_back((float) (*component_vector)[i].delta);
		veloc_par.vbar.push_back((float) (*component_vector)[i].vbar);
	}
}

void US_FeFitter_Single::resizeEvent(QResizeEvent *e)
{
	if (!GUI)
	{
		return;
	}
	int column1 = 120, plotHeight;
	int dialogw = span + 2 * border;
	int plot_width = e->size().width() - dialogw - border;
	plotHeight = (int) ((e->size().height() - 2 * buttonh - 2 * border - 3 * spacing)/2);
	data_plot->setGeometry(	dialogw, border, plot_width, plotHeight);
	residuals_plot->setGeometry(dialogw, plotHeight + border + spacing, plot_width, plotHeight);
	int ypos = e->size().height() - (2 * buttonh + spacing + border);
	int width = plot_width - column1 - spacing;
	lbl_status1->setGeometry(dialogw, ypos, column1, buttonh);
	lbl_status2->setGeometry(dialogw + column1 + spacing, ypos, width, buttonh);
	ypos += buttonh + spacing;
	lbl_status3->setGeometry(dialogw, ypos, column1, buttonh);
	lbl_status4->setGeometry(dialogw + column1 + spacing, ypos, width, buttonh);
}

void US_FeFitter_Single::cleanup()
{
	if(init_flag)
	{
		unsigned int i;
		for (i=0; i<points; i++)
		{
			delete [] jacobian[i];
		}
		for (i=0; i<parameters; i++)
		{
			delete [] information_matrix[i];
			delete [] LL_transpose[i];
		}
		delete [] sim_radius;
		delete [] y_guess;
		delete [] y_delta;
		delete [] B;
		delete [] test_guess;
		delete [] jacobian;
		delete [] information_matrix;
		delete [] LL_transpose;
		init_flag = false;
	}
}

// calculate a numerical jacobian using finite differencing
// (f(x+h) - f(x)]/h

int US_FeFitter_Single::calc_jacobian()
{
/*
	unsigned int i, j, k, count;
	double **jac;
	double *y_temp1;
	double *par_temp, h = 1.0e-5;
	par_temp = new double [parameters];
	y_temp1 = new double [all_sim_points];
	jac = new double * [all_sim_points];
	for (i=0; i<all_sim_points; i++)
	{
		jac[i] = new double [parameters];
	}
	for (i=0; i<all_sim_points; i++)
	{
		for (j=0; j<parameters; j++)
		{
			jac[i][j] = 0.0;
		}
	}
	for (i=0; i<parameters; i++)
	{
		par_temp[i] = guess[i]; //save original copy
	}
	for (i=0; i<parameters; i++)
	{
		guess[i] += guess[i] * h; //x + h
		calc_model(guess); // f(x + h)
		for (j=0; j<all_sim_points; j++)
		{
			y_temp1[j] = y_guess[j]; // assign f(x) and save it in y_temp1
		}
		guess[i] = par_temp[i] - (par_temp[i] * h); // x - h
//		cout << "\nModified parameter " << i << ": \n";
		calc_model(guess); // f(x - h)
		for (j=0; j<all_sim_points; j++)
		{
			jac[j][i] = (y_temp1[j] - y_guess[j])/(2 * par_temp[i]*h); // assign f(x) and save it in y_temp1
		}
		guess[i] = par_temp[i]; // reassign the old parameter value
	}
	
	count = 0;
	for (i=0; i<(*run_inf).scans[cell][lambda]; i++)
	{
		for(j=i*sim_points+offset1; j<i*sim_points+offset2; j++)
		{
			for (k=0; k<parameters; k++)
			{
				jacobian[count][k] = jac[j][k];
//				cout << jacobian[count][k] << " ";
			}
//			cout << "\n";
			count ++;		
		}
	}
	for (i=0; i<all_sim_points; i++)
	{
		delete [] jac[i];
	}
	delete [] jac;
	delete [] y_temp1;
	delete [] par_temp;

*/
// calculate a numerical jacobian using finite differencing
// (f(x+h) - f(x)]/h

	unsigned int i, j, k, count;
	double **jac;
	double * y_temp;
	double * par_temp;
	par_temp = new double [parameters];
	y_temp = new double [all_sim_points];
	jac = new double * [all_sim_points];
	for (i=0; i<all_sim_points; i++)
	{
		jac[i] = new double [parameters];
	}
	for (i=0; i<all_sim_points; i++)
	{
		for (j=0; j<parameters; j++)
		{
			jac[i][j] = 0.0;
		}
	}
	for (i=0; i<parameters; i++)
	{
		par_temp[i] = guess[i]; //save original copy
	}
	cout << "\nOriginal solution: \n";
	calc_model(guess); // get f(x)
	for (i=0; i<all_sim_points; i++)
	{
		y_temp[i] = y_guess[i]; // assign f(x) and save it in y_temp
	}
	float h=1e6;
	for (i=0; i<parameters; i++)
	{
		guess[i] += guess[i]/h; //x + h
		cout << "\nModified parameter " << i << ": \n";
		calc_model(guess); // f(x+h)
		for (j=0; j<all_sim_points; j++)
		{
			jac[j][i] = (y_guess[j] - y_temp[i])/(par_temp[i]/h); // assign f(x) and save it in y_temp1
		}
		guess[i] = par_temp[i]; // reassign the old parameter value
	}
	
	cout << "Finished Jacobian...\n";
	count = 0;
	for (i=0; i<(*run_inf).scans[cell][lambda]; i++)
	{
		for(j=i*sim_points+offset1; j<i*sim_points+offset2; j++)
		{
			for (k=0; k<parameters; k++)
			{
				jacobian[count][k] = jac[j][k];
//				cout << jacobian[count][k] << " ";
			}
//			cout << "\n";
			count ++;		
		}
	}
	for (i=0; i<all_sim_points; i++)
	{
		delete [] jac[i];
	}
	delete [] jac;
	delete [] y_temp;
	delete [] par_temp;
	return (0);
}

bool US_FeFitter_Single::fit_init()
{
	if (init_flag)
	{
		cleanup();	
	}
	emit fitStarted();
	emit meniscus_dk_green();
	emit baseline_dk_green();
	emit slope_dk_green();
	emit stray_dk_green();
	emit sed_dk_green();
	emit diff_dk_green();
	emit conc_dk_green();
	emit sigma_dk_green();
	emit delta_dk_green();
	unsigned int i;
	if (GUI)
	{
		pb_pause->setEnabled(false);
		pb_print->setEnabled(false);
	}
	assign_orig();
	datasets = (*run_inf).scans[cell][lambda];
	sim_points = (unsigned int) (1.5 + (bottom - runData->meniscus)/runData->delta_r);
	all_sim_points = sim_points * (*run_inf).scans[cell][lambda];
	points = (*run_inf).scans[cell][lambda] * scanpoints;
	/*
	for (i=0; i<(*run_inf).scans[cell][lambda]; i++)
	{
		for(unsigned int j=0; j<scanpoints; j++)
		{
			cout << "Absorbance[" << i << "][" << j << "]: " << absorbance[i][j] << endl;
		}
	}
	*/
	parameters = 0;
	ParameterIndex.conc.clear();
	ParameterIndex.sed.clear();
	ParameterIndex.diff.clear();
	ParameterIndex.sigma.clear();
	ParameterIndex.delta.clear();
	ParameterIndex.vbar.clear();
	for (i=0; i<(*component_vector).size(); i++)	// count the number of parameters, and set up the linked
	{															// list which keeps track of the indices of each
		if ((*component_vector)[i].conc_fit)		// each parameter. If the variable is not floated, use -1
		{
			ParameterIndex.conc.push_back(parameters);
			parameters += 1;
		}
		else
		{
			ParameterIndex.conc.push_back(-1);
		}
		if ((*component_vector)[i].sed_fit)
		{
			ParameterIndex.sed.push_back(parameters);
			parameters += 1;
		}
		else
		{
			ParameterIndex.sed.push_back(-1);
		}
		if ((*component_vector)[i].diff_fit)
		{
			ParameterIndex.diff.push_back(parameters);
			parameters += 1;
		}
		else
		{
			ParameterIndex.diff.push_back(-1);
		}
		if ((*component_vector)[i].sigma_fit)
		{
			ParameterIndex.sigma.push_back(parameters);
			parameters += 1;
		}
		else
		{
			ParameterIndex.sigma.push_back(-1);
		}
		if ((*component_vector)[i].delta_fit)
		{
			ParameterIndex.delta.push_back(parameters);
			parameters += 1;
		}
		else
		{
			ParameterIndex.delta.push_back(-1);
		}
		if ((*component_vector)[i].vbar_fit)
		{
			ParameterIndex.vbar.push_back(parameters);
			parameters += 1;
		}
		else
		{
			ParameterIndex.vbar.push_back(-1);
		}
	}
	if ((*runData).baseline_fit)
	{
		ParameterIndex.baseline = parameters;
		parameters += 1;
	}
	else
	{
		ParameterIndex.baseline = -1;
	}
	if ((*runData).slope_fit)
	{
		ParameterIndex.slope = parameters;
		parameters += 1;
	}
	else
	{
		ParameterIndex.slope = -1;
	}
	if ((*runData).stray_fit)
	{
		ParameterIndex.stray = parameters;
		parameters += 1;
	}
	else
	{
		ParameterIndex.stray = -1;
	}
	guess = new double [parameters];
	
	iteration = 0;
 
	y_guess					= new double   [all_sim_points];		// simulated solution
	y_delta					= new double   [points];		// y_raw - y_guess
	B							= new double   [parameters];
	test_guess				= new double   [parameters];
	jacobian					= new double  *[points];
	information_matrix	= new double  *[parameters];
	LL_transpose			= new double  *[parameters];
	for (i=0; i<points; i++)
	{
		jacobian[i] = new double [parameters];
	}
	for (i=0; i<parameters; i++)
	{
		information_matrix[i]	= new double [parameters];
		LL_transpose[i]			= new double [parameters];
	}
	sim_radius = new float [sim_points];

	// offset1 is the datapoint index that matches the simulated data radius start i
	// point on the left where the experimental data radius starts overlapping
	// offset2 is the simulated datapoint index where the data stops overlapping 
	for (i=0; i<sim_points; i++)
	{
		sim_radius[i] = runData->meniscus + i * runData->delta_r;
		if (sim_radius[i] < radius[0])  // find the index for which sim_radius and radius[0] are the same
		{
			offset1 = i;
		}
		if (sim_radius[i] <= radius[(*run_inf).points[cell][lambda][0] - 1])
		{
			offset2 = i;
		}
	}
	offset1 ++; // need to add one to be inside the data range
	parameter_map_to_guess(); // initialize parameter vector
	init_flag = true;
	return(true);
}

void US_FeFitter_Single::parameter_map_to_guess()
{
	unsigned int i, j=0;
	for (i=0; i<(*component_vector).size(); i++)
	{
		if ((*component_vector)[i].conc_fit)
		{
			guess[j] = (*component_vector)[i].conc;
			j++;
		}
		if ((*component_vector)[i].sed_fit)
		{
			guess[j] = (*component_vector)[i].sed;
			j++;
		}
		if ((*component_vector)[i].diff_fit)
		{
			guess[j] = (*component_vector)[i].diff;
			j++;
		}
		if ((*component_vector)[i].sigma_fit)
		{
			guess[j] = (*component_vector)[i].sigma;
			j++;
		}
		if ((*component_vector)[i].delta_fit)
		{
			guess[j] = (*component_vector)[i].delta;
			j++;
		}
		if ((*component_vector)[i].vbar_fit)
		{
			guess[j] = (*component_vector)[i].vbar;
			j++;
		}
	}
	if ((*runData).meniscus_fit)
	{
		guess[j] = (*runData).meniscus;
		j++;
	}
	if ((*runData).baseline_fit)
	{
		guess[j] = (*runData).baseline;
		j++;
	}
	if ((*runData).slope_fit)
	{
		guess[j] = (*runData).slope;
		j++;
	}
	if ((*runData).stray_fit)
	{
		guess[j] = (*runData).stray;
		j++;
	}
}

void US_FeFitter_Single::parameter_map_to_component_vector(double *par)
{
	unsigned int i, j=0;
	for (i=0; i<(*component_vector).size(); i++)
	{
		if ((*component_vector)[i].conc_fit)
		{
			(*component_vector)[i].conc = par[j];
			j++;
		}
		if ((*component_vector)[i].sed_fit)
		{
			(*component_vector)[i].sed = par[j];
			j++;
		}
		if ((*component_vector)[i].diff_fit)
		{
			(*component_vector)[i].diff = par[j];
			j++;
		}
		if ((*component_vector)[i].sigma_fit)
		{
			(*component_vector)[i].sigma = par[j];
			j++;
		}
		if ((*component_vector)[i].delta_fit)
		{
			(*component_vector)[i].delta = par[j];
			j++;
		}
		if ((*component_vector)[i].vbar_fit)
		{
			(*component_vector)[i].vbar = par[j];
			j++;
		}
	}
	if ((*runData).meniscus_fit)
	{
		(*runData).meniscus = par[j];
		j++;
	}
	if ((*runData).baseline_fit)
	{
		(*runData).baseline = par[j];
		j++;
	}
	if ((*runData).slope_fit)
	{
		(*runData).slope = par[j];
		j++;
	}
	if ((*runData).stray_fit)
	{
		(*runData).stray = par[j];
		j++;
	}
	emit parametersUpdated();
}

void US_FeFitter_Single::check_constraints(double *par)
{
	unsigned int i, j=0;
	for (i=0; i<(*component_vector).size(); i++)
	{
		if ((*component_vector)[i].conc_fit)
		{
			if(par[j] > veloc_par.conc[i] + (*component_vector)[i].conc_range)
			{
				par[j] = veloc_par.conc[i] + (*component_vector)[i].conc_range;
				emit conc_red();
			}
			else if(par[j] < veloc_par.conc[i] - (*component_vector)[i].conc_range)
			{
				par[j] = veloc_par.conc[i] - (*component_vector)[i].conc_range;
				emit conc_red();
			}
			else
			{
				emit conc_green();
			}
			j++;
		}
		if ((*component_vector)[i].sed_fit)
		{
			if(par[j] > veloc_par.sed[i] + (*component_vector)[i].sed_range)
			{
				par[j] = veloc_par.sed[i] + (*component_vector)[i].sed_range;
				emit sed_red();
			}
			else if(par[j] < veloc_par.sed[i] - (*component_vector)[i].sed_range)
			{
				par[j] = veloc_par.sed[i] - (*component_vector)[i].sed_range;
				emit sed_red();
			}
			else
			{
				emit sed_green();
			}
			j++;
		}
		if ((*component_vector)[i].diff_fit)
		{
			if(par[j] > veloc_par.diff[i] + (*component_vector)[i].diff_range)
			{
				par[j] = veloc_par.diff[i] + (*component_vector)[i].diff_range;
				emit diff_red();
			}
			else if(par[j] < veloc_par.diff[i] - (*component_vector)[i].diff_range)
			{
				par[j] = veloc_par.diff[i] - (*component_vector)[i].diff_range;
				emit diff_red();
			}
			else
			{
				emit diff_green();
			}
			j++;
		}
		if ((*component_vector)[i].sigma_fit)
		{
			if(par[j] > veloc_par.sigma[i] + (*component_vector)[i].sigma_range)
			{
				par[j] = veloc_par.sigma[i] + (*component_vector)[i].sigma_range;
				emit sigma_red();
			}
			else if(par[j] < veloc_par.sigma[i] - (*component_vector)[i].sigma_range)
			{
				par[j] = veloc_par.sigma[i] - (*component_vector)[i].sigma_range;
				emit sigma_red();
			}
			else
			{
				emit sigma_green();
			}
			j++;
		}
		if ((*component_vector)[i].delta_fit)
		{
			if(par[j] > veloc_par.delta[i] + (*component_vector)[i].delta_range)
			{
				par[j] = veloc_par.delta[i] + (*component_vector)[i].delta_range;
				emit delta_red();
			}
			else if(par[j] < veloc_par.delta[i] - (*component_vector)[i].delta_range)
			{
				par[j] = veloc_par.delta[i] - (*component_vector)[i].delta_range;
				emit delta_red();
			}
			else
			{
				emit delta_green();
			}
			j++;
		}
		if ((*component_vector)[i].vbar_fit)
		{
			j++;
		}
	}
	if ((*runData).meniscus_fit)
	{
		if(par[j] > veloc_par.meniscus + (*runData).meniscus_range)
		{
			par[j] = veloc_par.meniscus + (*runData).meniscus_range;
			emit meniscus_red();
		}
		else if(par[j] < veloc_par.meniscus - (*runData).meniscus_range)
		{
			par[j] = veloc_par.meniscus - (*runData).meniscus_range;
			emit meniscus_red();
		}
		else
		{
			emit meniscus_green();
		}
		j++;
	}
	if ((*runData).baseline_fit)
	{
		if(par[j] > veloc_par.baseline + (*runData).baseline_range)
		{
			par[j] = veloc_par.baseline + (*runData).baseline_range;
			emit baseline_red();
		}
		else if(par[j] < veloc_par.baseline - (*runData).baseline_range)
		{
			par[j] = veloc_par.baseline - (*runData).baseline_range;
			emit baseline_red();
		}
		else
		{
			emit baseline_green();
		}
		j++;
	}
	if ((*runData).slope_fit)
	{
		if(par[j] > veloc_par.slope + (*runData).slope_range)
		{
			par[j] = veloc_par.slope + (*runData).slope_range;
			emit slope_red();
		}
		else if(par[j] < veloc_par.slope - (*runData).slope_range)
		{
			par[j] = veloc_par.slope - (*runData).slope_range;
			emit slope_red();
		}
		else
		{
			emit slope_green();
		}
		j++;
	}
	if ((*runData).stray_fit)
	{
		if(par[j] > veloc_par.stray + (*runData).stray_range)
		{
			par[j] = veloc_par.stray + (*runData).stray_range;
			emit stray_red();
		}
		else if(par[j] < veloc_par.stray - (*runData).stray_range)
		{
			par[j] = veloc_par.stray - (*runData).stray_range;
			emit stray_red();
		}
		else
		{
			emit stray_green();
		}
		j++;
	}
}

int US_FeFitter_Single::calc_model(double *par)
{
	QString str;
	bool ldu_flag = true;
	if (GUI)
	{
		parameter_map_to_component_vector(par);	// update component_vector with the latest parameters, 
																// so the model control can be updated
		lbl_status2->setText(tr("Fitting Finite Element Solution..."));
	}
	if (constrained)
	{
		check_constraints(par);
	}
	unsigned int i, j, k, time_steps, scan=0, count=0;
	float scan_timer, omega_s;
	omega_s = pow((double) (M_PI * (*run_inf).rpm[cell][0][0]/30.0), (double) 2.0);
	time_steps = (unsigned int) (0.5 + ((*run_inf).time[cell][lambda][(*run_inf).scans[cell][lambda] - 1]/runData->delta_t));
	for (i=0; i<components; i++)
	{
		if ((*component_vector)[i].sigma != 0 || (*component_vector)[i].delta != 0)
		{
			conc_dep_flag = true;
		}
	}
//	cout << "time: " << (*run_inf).time[cell][lambda][(*run_inf).scans[cell][lambda] - 1]<<", Time steps: " <<time_steps << ", rpm: " << (*run_inf).rpm[cell][0][0]<<endl;
	init_finite_element();
	concentration = new double *[(*run_inf).scans[cell][lambda]];
	for (i=0; i<(*run_inf).scans[cell][lambda]; i++)
	{
		concentration[i] = new double [sim_points];
	}
	for (i=0; i<(*run_inf).scans[cell][lambda]; i++)
	{
		for (j=0; j<sim_points; j++)
		{
			concentration[i][j]=0.0;
		}
	}
	double *temp_val;
	temp_val = new double [2];
	right = new double [sim_points];
	right_old = new double [sim_points];
	left = new double* [sim_points];
	left_temp = new double* [sim_points];
	for (i=0; i<parameters; i++)
	{
		cout << "Parameter " << i << ": " << guess[i] << endl;
	}
	for (i=0; i<sim_points; i++)
	{
		left[i] = new double [3];
		left_temp[i] = new double [3];
	}
	for (k=0; k<components; k++)
	{
		ldu_flag = true;
		if (GUI)
		{
			lbl_status2->setText(tr("Fitting Finite Element Solution, Component ") + str.sprintf("%d", k+1));
		}
		scan_timer=0.0;
		if (ParameterIndex.sed[k] == -1)
		{
			temp_val[0] = runData->delta_t * (*component_vector)[k].sed * omega_s;
		}
		else
		{
			temp_val[0] = runData->delta_t * par[ParameterIndex.sed[k]] * omega_s;
		}
		if (ParameterIndex.diff[k] == -1)
		{
			temp_val[1] = runData->delta_t * (*component_vector)[k].diff;
		}
		else
		{
			temp_val[1] = runData->delta_t * par[ParameterIndex.diff[k]];
		}
		if (ParameterIndex.conc[k] == -1)
		{
			for (i=0; i<sim_points; i++)
			{
				right[i] = (*component_vector)[k].conc;
				for (j=0; j<3; j++)
				{
					left[i][j] = temp_val[1] * a1[i][j] - temp_val[0] * a2[i][j];
				}
			}
		}
		else
		{
			for (i=0; i<sim_points; i++)
			{
				right[i] = par[ParameterIndex.conc[k]];
				for (j=0; j<3; j++)
				{
					left[i][j] = temp_val[1] * a1[i][j] - temp_val[0] * a2[i][j];
				}
			}
		}
		for (i=0; i< sim_points; i++)
		{
			for (j=0; j<3; j++)
			{
				left_temp[i][j] = b_orig[i][j] + 0.5 * left[i][j];
			}
		}
		for (j=0; j<time_steps; j++)
		{
			qApp->processEvents();
			count++;
			scan_timer += runData->delta_t;
			if ((*component_vector)[k].sigma != 0 || (*component_vector)[k].delta != 0) // we don't need to check
			{	// if sigma or delta are floated, because if either is nonzero c_current is needed anyway
				for (i=0; i<sim_points; i++)
				{
					c_current[i] = right[i];
				}
			}
			for (i=0; i<sim_points; i++)
			{
				right_old[i] = right[i];
			}
			

// Crank-Nicholson:

			m3vm_b(&b_orig, &left, &right, sim_points, -0.5); // adds B to left and divides each element of A by 2 before multiplying with right[i]
			qApp->processEvents();
			ldu(&left_temp, &right, sim_points, 3, ldu_flag); // end Corrector
			qApp->processEvents();
			ldu_flag = false;
			if (fabs((*run_inf).time[cell][lambda][scan] - scan_timer) <= (runData->delta_t/2.0))
			{
				for (i=0; i<sim_points; i++)
				{
					concentration[scan][i] += right[i];
				}
				scan++;
				if (scan == (*run_inf).scans[cell][lambda])
				{
					scan = 0;
				}
			}
			if (aborted) // if we abort, we will delete all local and global memory and return
			{
				for (i=0; i<sim_points; i++)
				{
					delete [] left[i];
					delete [] left_temp[i];
				}
				delete [] left;
				delete [] left_temp;
				delete [] right;
				delete [] right_old;
				delete [] temp_val;
				if (conc_dep_flag)
				{
					delete [] c_current;
				}
				emit currentStatus(tr("Aborted"));
				emit fitAborted();
				if (GUI)
				{
					pb_cancel->setText(tr("Close"));
					pb_fit->setEnabled(true);
					lbl_status2->setText(tr("The fit was aborted..."));
					pb_saveFit->setEnabled(false);
					pb_report->setEnabled(false);
					pb_residuals->setEnabled(false);
					pb_overlays->setEnabled(false);
					pb_pause->setEnabled(false);
					qApp->processEvents(); // wait until the residuals plots
				}
				cleanup_finite_element();
				cleanup();
				return(-1);
			}
		}
	}

	for (i=0; i<sim_points; i++)
	{
		delete [] left[i];
		delete [] left_temp[i];
	}
	delete [] left;
	delete [] left_temp;
	delete [] right;
	delete [] right_old;
	delete [] temp_val;
	if (conc_dep_flag)
	{
		delete [] c_current;
	}
	function_evaluations++;
	if (GUI)
	{
		QString str;
		str.sprintf("%d", function_evaluations);
		lbl_evaluations2->setText(str);
	}

// before exiting this function, add the baseline

	if (ParameterIndex.baseline == -1)
	{
		for (i=0; i<(*run_inf).scans[cell][lambda]; i++)
		{
			for (j=0; j<sim_points; j++)
			{
				concentration[i][j] += runData->baseline;
			}
		}
	}
	else
	{
		for (i=0; i<(*run_inf).scans[cell][lambda]; i++)
		{
			for (j=0; j<sim_points; j++)
			{
				concentration[i][j] += par[ParameterIndex.baseline];
			}
		}
	}
	if (ParameterIndex.slope == -1)
	{
		for (i=0; i<(*run_inf).scans[cell][lambda]; i++)
		{
			for (j=0; j<sim_points; j++)
			{
				concentration[i][j] += runData->slope * (sim_radius[j] - (*run_inf).range_left[cell][lambda][0]);
			}
		}
	}
	else
	{
		for (i=0; i<(*run_inf).scans[cell][lambda]; i++)
		{
			for (j=0; j<sim_points; j++)
			{
				concentration[i][j] += par[ParameterIndex.slope] * (sim_radius[j] - (*run_inf).range_left[cell][lambda][0]);
			}
		}
	}
	if (ParameterIndex.stray == -1)
	{
		for (i=0; i<(*run_inf).scans[cell][lambda]; i++)
		{
			for (j=0; j<sim_points; j++)
			{
				concentration[i][j] += runData->stray * concentration[i][j] * (sim_radius[j] - (*run_inf).range_left[cell][lambda][0]);
			}
		}
	}
	else
	{
		for (i=0; i<(*run_inf).scans[cell][lambda]; i++)
		{
			for (j=0; j<sim_points; j++)
			{
				concentration[i][j] += par[ParameterIndex.stray] * concentration[i][j] * (sim_radius[j] - (*run_inf).range_left[cell][lambda][0]);
			}
		}
	}
	count=0;
	for (unsigned int i=0; i<(*run_inf).scans[cell][lambda]; i++)
	{
		for (unsigned int j=0; j<sim_points; j++)
		{
			y_guess[count] = concentration[i][j];
			count ++;
		}
	}
	cleanup_finite_element();
	return(0);
}

void US_FeFitter_Single::cleanup_finite_element()
{
	unsigned int i;
	for (unsigned int i=0; i<(*run_inf).scans[cell][lambda]; i++)
	{
		delete [] concentration[i];
	}
	delete [] concentration;
	if (model == 4 || model == 20)
	{
		delete [] s_bar;
		delete [] D_bar;
		delete [] partial_c[0];
		delete [] partial_c[1];
		delete [] partial_c;
	}

	for (i=0; i<sim_points; i++)
	{
		delete [] a1[i];
		delete [] a2[i];
		delete [] b[i];
		delete [] b_orig[i];
	}
	delete [] a1;
	delete [] a2;
	delete [] b;
	delete [] b_orig;
}

void US_FeFitter_Single::init_finite_element()
{
	unsigned int i, j;
	init_simulation = true;
	if (model == 4 || model == 20)
	{
		s_bar = new double [sim_points];
		D_bar = new double [sim_points];
		partial_c = new double* [2];
		partial_c[0] = new double [sim_points];
		partial_c[1] = new double [sim_points];
	}

	a1 = new float* [sim_points];
	a2 = new float* [sim_points];
	b = new float* [sim_points];
	b_orig = new double* [sim_points];

// concentration has the current concentration for all scans

	for (i=0; i<sim_points; i++)
	{
		a1[i] = new float [3];
		a2[i] = new float [3];
		b[i] = new float [3];
		b_orig[i] = new double [3];
	}
	a1[0][1] = (sim_radius[0]/runData->delta_r) + 0.5;
	a1[0][0] = 0.0;
	for (i=1; i<sim_points; i++)
	{
		a1[i][0] = (-(sim_radius)[i]/runData->delta_r) + 0.5;
		a1[i][1] = 2.0 * sim_radius[i]/runData->delta_r;
		a1[i-1][2] = a1[i][0];
	}
	a1[sim_points-1][1] = (sim_radius[sim_points-1]/runData->delta_r) - 0.5;
	a1[sim_points-1][2] = 0.0;
	float delta2 = pow((double) runData->delta_r, (double) 2.0);
	a2[0][1] = -1 * (sim_radius[0] * sim_radius[0])/2.0 - sim_radius[0] * (runData->delta_r/3.0) - delta2/12.0;
	a2[0][0] = 0.0;
	for (i=1; i<sim_points; i++)
	{
		a2[i][0] = (sim_radius[i] * sim_radius[i])/2.0 - 2.0 * sim_radius[i] * (runData->delta_r/3.0) + delta2/4.0;
		a2[i][1] = -2 * sim_radius[i] * runData->delta_r/3.0;
		a2[i-1][2]= -1 * (sim_radius[i-1] * sim_radius[i-1])/2.0 - 2.0 * sim_radius[i-1] * (runData->delta_r/3.0) - delta2/4.0;
	}
	a2[sim_points-1][1] = (sim_radius[sim_points-1] * sim_radius[sim_points-1])/2.0 - sim_radius[sim_points-1] * (runData->delta_r/3.0) + delta2/12.0;
	a2[sim_points-1][2] = 0.0;
	b[0][0] = 0.0;
	b[0][1] = sim_radius[0] * (runData->delta_r/3.0) + delta2/12.0;
	for (i=1; i<sim_points; i++)
	{
		b[i][0] = sim_radius[i] * (runData->delta_r/6.0) - delta2/12.0;
		b[i][1] = 2.0 * sim_radius[i]*(runData->delta_r/3.0);
		b[i-1][2] = b[i][0];
	}
	b[sim_points-1][1] = sim_radius[sim_points-1] * (runData->delta_r/3.0) - delta2/12.0;
	b[sim_points-1][2] = 0.0;
	for (i=0; i<sim_points; i++)
	{
		for (j=0; j<3; j++)
		{
			b_orig[i][j] = b[i][j];
		}
	}
}

void US_FeFitter_Single::view_report()
{
	if(!GUI)
	{
		return;
	}
	QString filestr, filename, temp;
	filename = USglobal->config_list.result_dir.copy();
	filename.append("/");
	filename.append((*run_inf).run_id);
	temp.sprintf(".%d%d", cell+1, lambda+1);
	filestr = filename.copy();
	filestr.append(".fef_res");
	filestr.append(temp);
	//view_file(filestr);
	TextEdit *e;
	e = new TextEdit();
	e->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	e->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
	e->load(filestr);
	e->show();

}

void US_FeFitter_Single::write_report()
{
	if(!GUI)
	{
		return;
	}
	QPixmap p;
	US_Pixmap *pm;
	pm = new US_Pixmap();
	QString filestr, filename, temp, model_str, str1;
	unsigned int i, j, k;
	filename.sprintf(htmlDir + "/fef_overlays-%d%d.", cell + 1, lambda + 1);
	p = QPixmap::grabWidget(data_plot, 2, 2, data_plot->width() - 4, data_plot->height() - 4);
	pm->save_file(filename, p);
	filename.sprintf(htmlDir + "/fef_residuals-%d%d.", cell + 1, lambda + 1);
	p = QPixmap::grabWidget(residuals_plot, 2, 2, residuals_plot->width() - 4, residuals_plot->height() - 4);
	pm->save_file(filename, p);
	filename = USglobal->config_list.result_dir.copy();
	filename.append("/");
	filename.append((*run_inf).run_id);
	temp.sprintf(".%d%d", cell+1, lambda+1);
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
		ts << tr("Data Report for Run \"") << (*run_inf).run_id << tr("\", Cell ") << (cell + 1) 
		 << tr(", Wavelength ") << (lambda + 1) << "\n\n";
		ts << tr("Detailed Run Information:\n\n");
		ts << tr("Cell Description:            ") << (*run_inf).cell_id[cell] << "\n";
		ts << tr("Raw Data Directory:          ") << (*run_inf).data_dir << "\n";
		ts << tr("Rotor Speed:                 ") << (*run_inf).rpm[cell][0][0]  << " rpm\n";
		ts << tr("Average Temperature:         ") << (*run_inf).avg_temperature << " ºC\n" ;
		if ((*run_inf).temperature_check == 1)
		{
			ts << tr("Temperature Variation:       Within Tolerance\n");
		}
		if ((*run_inf).temperature_check == -1)
		{
			ts << tr("Temperature Variation: (!) OUTSIDE TOLERANCE (!)\n");
		}
		i = (unsigned int) ((*run_inf).time_correction / 60);
		j = (unsigned int) (0.5 + ((*run_inf).time_correction - (i * 60)));
		ts << tr("Time Correction:             ") << i << tr(" minute(s) ") << j << tr(" second(s)\n");
		i = (unsigned int) ((*run_inf).duration / 3600);
		j = (unsigned int) (0.5 + ((*run_inf).duration - i * 3600) / 60);
		ts << tr("Run Duration:                ") << i << tr(" hour(s) ") << j << tr(" minute(s)\n");
		ts << tr("Wavelength:                  ") << (*run_inf).wavelength[cell][lambda] << " nm\n";
		if (runData->baseline_fit)
		{
			ts << tr("Baseline Absorbance:         ") << runData->baseline << tr(" OD (fitted)\n");
		}
		else
		{
			ts << tr("Baseline Absorbance:         ") << runData->baseline << tr(" OD (fixed)\n");
		}
		if (runData->meniscus_fit)
		{
			ts << tr("Meniscus Position:           ") << runData->meniscus << tr(" cm (fitted)\n");
		}
		else
		{
			ts << tr("Meniscus Position:           ") << runData->meniscus << tr(" cm (fixed)\n");
		}
		if (runData->slope_fit)
		{
			ts << tr("Slope Error Correction(r):   ") << runData->slope << tr(" OD (fitted)\n");
		}
		else
		{
			ts << tr("Slope Error Correction(r):   ") << runData->slope << tr(" OD (fixed)\n");
		}
		if (runData->stray_fit)
		{
			ts << tr("Slope Error Correction(c,r): ") << runData->stray << tr(" OD (fitted)\n");
		}
		else
		{
			ts << tr("Slope Error Correction(c,r): ") << runData->stray << tr(" OD (fixed)\n") ;
		}
		ts << tr("Edited Data starts at:       ") << (*run_inf).range_left[cell][lambda][0] << " cm\n";
		ts << tr("Edited Data stops at:        ") << (*run_inf).range_right[cell][lambda][0] << " cm\n\n\n";
		ts << tr("Hydrodynamic Settings:\n\n");
		ts << tr("Viscosity correction:        ") << viscosity << "\n" ;
		ts << tr("Viscosity (absolute):        ") << viscosity_tb << "\n" ;
		ts << tr("Density correction:          ") << density << " g/ccm\n" ;
		ts << tr("Density (absolute):          ") << density_tb << " g/ccm\n\n\n" ;
		ts << tr("Data Analysis Settings:\n\n");
		ts << tr("Analyzed Boundary:           ") << bd_range << " %\n";
		ts << tr("Boundary Position:           ") << bd_position << " %\n\n\n";

		ts << tr("Fitting Information:\n\n");
		ts << tr("Variance:                    ") << variance << "\n";
		ts << tr("Standard Deviation           ") << pow((double) variance, (double) 0.5) << "\n";
		ts << tr("# of Iterations:             ") << iteration << "\n";
		ts << tr("# of Function Evals.:        ") << function_evaluations << "\n";		
		ts << tr("# of fitted Parameters:      ") << parameters << "\n";
		ts << tr("# of fitted Data Points:     ") << points << "\n";
		ts << tr("Average Data Point Density:  ") << (*run_inf).point_density[cell][lambda][0] << " cm/point\n";
		str1.sprintf(tr("# of Runs in the Fit:        %ld (%2.3f "), runData->runs, runData->runs_percent);
		ts << str1 << "%, ";
		runData->runs_percent *= (*run_inf).point_density[cell][lambda][0]/(*run_inf).delta_r;
		str1.sprintf("%2.3f", runData->runs_percent);
		ts << str1 << tr("% corrected)\n");
//
// Formula for expected number of runs (P = nummber of positives, N = number of Negatives):
// E(r) = 1 + 2PN/(P + N)	
// Formula for its variance:
// V(R) = [2PN (2PN - P - N)] / [(P + N)^2 (P + N - 1)]
//
		runData->runs_expected = 1.0 + (2.0 * runData->pos * runData->neg) / (runData->pos + runData->neg);
		runData->runs_expected = runData->runs_expected * ((*run_inf).delta_r/(*run_inf).point_density[cell][lambda][0]);
		runData->runs_variance = ((2.0 * runData->pos * runData->neg) *	(2.0 * runData->pos * runData->neg - runData->pos - runData->neg))
		/ ((pow((double) (runData->pos + runData->neg), (double) 2)) * (runData->pos + runData->neg - 1.0));
		str1.sprintf(tr("Expected Number of Runs:     %ld\nRun Variance:                %2.3e\n\n"), (long) runData->runs_expected, runData->runs_variance);		ts << str1;
		if (runData->runs_percent < 20.0)
		{
			ts << tr("According to these statistical tests, this model is either inappropriate for the\n");
			ts << tr("experimental data, or the fitting process has not yet converged. Please try to reduce\n");
			ts << tr("the variance by additional nonlinear least-squares minimization of the data.\n");
			ts << tr("This fit cannot be used for a Monte Carlo Analysis.\n");
		}
		if (runData->runs_percent > 20.0 && runData->runs_percent < 25.0)
		{
			ts << tr("According to these statistical tests, this model is either a poor candidate for the\n");
			ts << tr("experimental data, or the fitting process has not yet converged. Please try to reduce\n");
			ts << tr("the variance by additional nonlinear least-squares minimization of the data.\n");
			ts << tr("This fit cannot be used for a Monte Carlo Analysis.\n");
		}
		if (runData->runs_percent > 25.0 && runData->runs_percent < 30)
		{
			ts << tr("According to these statistical tests, this model is either a mediocre candidate\n");
			ts << tr("for the experimental data, or the fitting process has not yet converged. Please try to\n");
			ts << tr("reduce the variance by additional nonlinear least-squares minimization of the data.\n");
			ts << tr("This fit should not be used for a Monte Carlo Analysis.\n");
		}
		if (runData->runs_percent > 30.0 && runData->runs_percent < 35)
		{
			ts << tr("According to these statistical tests, this model is a good candidate for the experimental data.\n");
			ts << tr("This fit may be used for a Monte Carlo Analysis with reservations.\n");
		}
		if (runData->runs_percent > 35.0)
		{
			ts << tr("According to these statistical tests, this model is an excellent candidate for the experimental data.\n");
			ts << tr("Monte Carlo Analysis is recommended for this data.\n");
		}

		ts << tr("\nComponent Information for " + modelString[model] + ":\n\n");
		if (model>= 0 && model <= 3)
		{
			for (i=0; i<components; i++)
			{
				US_Hydro3 *shape;
				float vbar20 = (*component_vector)[i].vbar - (4.25e-4 * ((*run_inf).avg_temperature - 20));
				model_str.sprintf(tr("Molecular Shape Model for Component %d"), i+1);
				shape = new US_Hydro3(0, &(*component_vector)[i].s20w, &(*component_vector)[i].d20w, 
											 &(*component_vector)[i].mw, &vbar20, model_str, i);
				ts << tr("Component ") << (i+1) << ":\n";
				if ((*component_vector)[i].mw_fit)
				{
					ts << tr("Molecular Weight:            ") << (*component_vector)[i].mw << tr(" Dalton") << tr(" (fitted)\n");
				}
				else
				{
					ts << tr("Molecular Weight:            ") << (*component_vector)[i].mw << tr(" Dalton") << tr(" (fixed)\n");
				}
				if ((*component_vector)[i].sed_fit)
				{
					ts << tr("Sed. Coeff. (s20,W):         ") << (*component_vector)[i].s20w << " s" << tr(" (fitted)\n");
				}
				else
				{
					ts << tr("Sed. Coeff. (s20,W):         ") << (*component_vector)[i].s20w << " s" << tr(" (fixed)\n");
				}
				if ((*component_vector)[i].sed_fit)
				{
					ts << tr("Sedimentation Coeff.:        ") << (*component_vector)[i].sed << " s" << tr(" (fitted)\n");
				}
				else
				{
					ts << tr("Sedimentation Coeff.:        ") << (*component_vector)[i].sed << " s" << tr(" (fixed)\n");
				}
				if ((*component_vector)[i].diff_fit)
				{
					ts << tr("Diff. Coeff. (D20,W):        ") << (*component_vector)[i].d20w << " D" << tr(" (fitted)\n");
				}
				else
				{
					ts << tr("Diff. Coeff. (D20,W):        ") << (*component_vector)[i].d20w << " D" << tr(" (fixed)\n");
				}
				if ((*component_vector)[i].diff_fit)
				{
					ts << tr("Diffusion Coeff.:            ") << (*component_vector)[i].diff << " D" << tr(" (fitted)\n");
				}
				else
				{
					ts << tr("Diffusion Coeff.:            ") << (*component_vector)[i].diff << " D" << tr(" (fixed)\n");
				}
				if ((*component_vector)[i].conc_fit)
				{
					ts << tr("Partial Concentration:       ") << (*component_vector)[i].conc << " OD" << tr(" (fitted)\n");
				}
				else
				{
					ts << tr("Partial Concentration:       ") << (*component_vector)[i].conc << " OD" << tr(" (fixed)\n");
				}
				if ((*component_vector)[i].vbar_fit)
				{
					ts << tr("Partial Spec. Vol. (vbar):   ") << ((*component_vector)[i].vbar - (4.25e-4 
																 * ((*run_inf).avg_temperature - 20))) << tr(" ccm/g (fitted)\n") ;
				}
				else
				{
					ts << tr("Partial Spec. Vol. (vbar):   ") << ((*component_vector)[i].vbar - (4.25e-4 
																 * ((*run_inf).avg_temperature - 20))) << tr(" ccm/g (fixed)\n") ;
				}
				if ((*component_vector)[i].vbar_fit)
				{
					ts << tr("vbar (Corr. for 20ºC):       ") << (*component_vector)[i].vbar << tr(" ccm/g (fitted)\n") ;
				}
				else
				{
					ts << tr("vbar (Corr. for 20ºC):       ") << (*component_vector)[i].vbar << tr(" ccm/g (fixed)\n") ;
				}
				if ((*component_vector)[i].sigma_fit)
				{
					ts << tr("Conc. Dep. of s (Sigma):     ") << (*component_vector)[i].sigma << tr(" (fitted)\n") ;
				}
				else
				{
					ts << tr("Conc. Dep. of s (Sigma):     ") << (*component_vector)[i].sigma << tr(" (fixed)\n") ;
				}
				if ((*component_vector)[i].delta_fit)
				{
					ts << tr("Conc. Dep. of D (Delta):     ") << (*component_vector)[i].delta << tr(" (fitted)\n") ;
				}
				else
				{
					ts << tr("Conc. Dep. of D (Delta):     ") << (*component_vector)[i].delta << tr(" (fixed)\n") ;
				}
				ts << tr("Buoyancy (Water, 20ºC):      ") << (1 - ((*component_vector)[i].vbar - (4.25e-4 
																 * ((*run_inf).avg_temperature - 20))) * DENS_20W) << "\n" ;
				ts << tr("Buoyancy (absolute):         ") << (1 - (*component_vector)[i].vbar * density_tb) << "\n\n" ;
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
			for (i=0; i<2; i++)
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
				float vbar20 = (*component_vector)[i].vbar - (4.25e-4 * ((*run_inf).avg_temperature - 20));
				shape = new US_Hydro3(0, &(*component_vector)[i].s20w, &(*component_vector)[i].d20w, 
											 &(*component_vector)[i].mw, &vbar20, model_str, i);
				if ((*component_vector)[i].mw_fit)
				{
					ts << tr("Molecular Weight:            ") << (*component_vector)[i].mw << tr(" Dalton") << tr(" (fitted)\n");
				}
				else
				{
					ts << tr("Molecular Weight:            ") << (*component_vector)[i].mw << tr(" Dalton") << tr(" (fixed)\n");
				}
				if ((*component_vector)[i].sed_fit)
				{
					ts << tr("Sed. Coeff. (s20,W):         ") << (*component_vector)[i].s20w << " s" << tr(" (fitted)\n");
				}
				else
				{
					ts << tr("Sed. Coeff. (s20,W):         ") << (*component_vector)[i].s20w << " s" << tr(" (fixed)\n");
				}
				if ((*component_vector)[i].sed_fit)
				{
					ts << tr("Sedimentation Coeff.:        ") << (*component_vector)[i].sed << " s" << tr(" (fitted)\n");
				}
				else
				{
					ts << tr("Sedimentation Coeff.:        ") << (*component_vector)[i].sed << " s" << tr(" (fixed)\n");
				}
				if ((*component_vector)[i].diff_fit)
				{
					ts << tr("Diff. Coeff. (D20,W):        ") << (*component_vector)[i].d20w << " D" << tr(" (fitted)\n");
				}
				else
				{
					ts << tr("Diff. Coeff. (D20,W):        ") << (*component_vector)[i].d20w << " D" << tr(" (fixed)\n");
				}
				if ((*component_vector)[i].diff_fit)
				{
					ts << tr("Diffusion Coeff.:            ") << (*component_vector)[i].diff << " D" << tr(" (fitted)\n");
				}
				else
				{
					ts << tr("Diffusion Coeff.:            ") << (*component_vector)[i].diff << " D" << tr(" (fixed)\n");
				}
				if (i == 0)
				{
					if ((*component_vector)[i].conc_fit)
					{
						ts << tr("Total Concentration:         ") << (*component_vector)[i].conc << " OD" << tr(" (fitted)\n");
					}
					else
					{
						ts << tr("Total Concentration:         ") << (*component_vector)[i].conc << " OD" << tr(" (fixed)\n");
					}
				}
				else
				{
					if ((*component_vector)[i].conc_fit)
					{
						ts << tr("Equilibrium Constant:        ") << (*component_vector)[i].conc << " OD" << tr(" (fitted)\n");
					}
					else
					{
						ts << tr("Equilibrium Constant:        ") << (*component_vector)[i].conc << " OD" << tr(" (fixed)\n");
					}
				}
				if ((*component_vector)[i].vbar_fit)
				{
					ts << tr("Partial Spec. Vol. (vbar):   ") << ((*component_vector)[i].vbar - (4.25e-4 
																 * ((*run_inf).avg_temperature - 20))) << tr(" ccm/g (fitted)\n") ;
				}
				else
				{
					ts << tr("Partial Spec. Vol. (vbar):   ") << ((*component_vector)[i].vbar - (4.25e-4 
																 * ((*run_inf).avg_temperature - 20))) << tr(" ccm/g (fixed)\n") ;
				}
				if ((*component_vector)[i].vbar_fit)
				{
					ts << tr("vbar (Corr. for 20ºC):       ") << (*component_vector)[i].vbar << tr(" ccm/g (fitted)\n") ;
				}
				else
				{
					ts << tr("vbar (Corr. for 20ºC):       ") << (*component_vector)[i].vbar << tr(" ccm/g (fixed)\n") ;
				}
				if ((*component_vector)[i].sigma_fit)
				{
					ts << tr("Conc. Dep. of s (Sigma):     ") << (*component_vector)[i].sigma << tr(" (fitted)\n") ;
				}
				else
				{
					ts << tr("Conc. Dep. of s (Sigma):     ") << (*component_vector)[i].sigma << tr(" (fixed)\n") ;
				}
				if ((*component_vector)[i].delta_fit)
				{
					ts << tr("Conc. Dep. of D (Delta):     ") << (*component_vector)[i].delta << tr(" (fitted)\n") ;
				}
				else
				{
					ts << tr("Conc. Dep. of D (Delta):     ") << (*component_vector)[i].delta << tr(" (fixed)\n") ;
				}
				ts << tr("Buoyancy (Water, 20ºC):      ") << (1 - ((*component_vector)[i].vbar - (4.25e-4 
																 * ((*run_inf).avg_temperature - 20))) * DENS_20W) << "\n" ;
				ts << tr("Buoyancy (absolute):         ") << (1 - (*component_vector)[i].vbar * density_tb) << "\n\n" ;
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
			for (i=0; i<2; i++)
			{
				ts << tr("Isomer ") << (i+1) << ":\n";
				US_Hydro3 *shape;
				float vbar20 = (*component_vector)[i].vbar - (4.25e-4 * ((*run_inf).avg_temperature - 20));
				model_str.sprintf(tr("Molecular Shape Model for Isomer %d"), i+1);
				shape = new US_Hydro3(0, &(*component_vector)[i].s20w, &(*component_vector)[i].d20w, 
											 &(*component_vector)[i].mw, &vbar20, model_str, i);
				if ((*component_vector)[i].mw_fit)
				{
					ts << tr("Molecular Weight:            ") << (*component_vector)[i].mw << tr(" Dalton") << tr(" (fitted)\n");
				}
				else
				{
					ts << tr("Molecular Weight:            ") << (*component_vector)[i].mw << tr(" Dalton") << tr(" (fixed)\n");
				}
				if ((*component_vector)[i].sed_fit)
				{
					ts << tr("Sed. Coeff. (s20,W):         ") << (*component_vector)[i].s20w << " s" << tr(" (fitted)\n");
				}
				else
				{
					ts << tr("Sed. Coeff. (s20,W):         ") << (*component_vector)[i].s20w << " s" << tr(" (fixed)\n");
				}
				if ((*component_vector)[i].sed_fit)
				{
					ts << tr("Sedimentation Coeff.:        ") << (*component_vector)[i].sed << " s" << tr(" (fitted)\n");
				}
				else
				{
					ts << tr("Sedimentation Coeff.:        ") << (*component_vector)[i].sed << " s" << tr(" (fixed)\n");
				}
				if ((*component_vector)[i].diff_fit)
				{
					ts << tr("Diff. Coeff. (D20,W):        ") << (*component_vector)[i].d20w << " D" << tr(" (fitted)\n");
				}
				else
				{
					ts << tr("Diff. Coeff. (D20,W):        ") << (*component_vector)[i].d20w << " D" << tr(" (fixed)\n");
				}
				if ((*component_vector)[i].diff_fit)
				{
					ts << tr("Diffusion Coeff.:            ") << (*component_vector)[i].diff << " D" << tr(" (fitted)\n");
				}
				else
				{
					ts << tr("Diffusion Coeff.:        ") << (*component_vector)[i].diff << " D" << tr(" (fixed)\n");
				}
				if (i == 0)
				{
					if ((*component_vector)[i].conc_fit)
					{
						ts << tr("Total Concentration:     ") << (*component_vector)[i].conc << " OD" << tr(" (fitted)\n");
					}
					else
					{
						ts << tr("Total Concentration:     ") << (*component_vector)[i].conc << " OD" << tr(" (fixed)\n");
					}
				}
				else
				{
					if ((*component_vector)[i].conc_fit)
					{
						ts << tr("Equilibrium Constant:    ") << (*component_vector)[i].conc << " OD" << tr(" (fitted)\n");
					}
					else
					{
						ts << tr("Equilibrium Constant:    ") << (*component_vector)[i].conc << " OD" << tr(" (fixed)\n");
					}
				}
				if ((*component_vector)[i].vbar_fit)
				{
					ts << tr("Partial Spec. Vol. (vbar):   ") << ((*component_vector)[i].vbar - (4.25e-4 
																 * ((*run_inf).avg_temperature - 20))) << tr(" ccm/g (fitted)\n") ;
				}
				else
				{
					ts << tr("Partial Spec. Vol. (vbar):   ") << ((*component_vector)[i].vbar - (4.25e-4 
																 * ((*run_inf).avg_temperature - 20))) << tr(" ccm/g (fixed)\n") ;
				}
				if ((*component_vector)[i].vbar_fit)
				{
					ts << tr("vbar (Corr. for 20ºC):       ") << (*component_vector)[i].vbar << tr(" ccm/g (fitted)\n") ;
				}
				else
				{
					ts << tr("vbar (Corr. for 20ºC):       ") << (*component_vector)[i].vbar << tr(" ccm/g (fixed)\n") ;
				}
				if ((*component_vector)[i].sigma_fit)
				{
					ts << tr("Conc. Dep. of s (Sigma):     ") << (*component_vector)[i].sigma << tr(" (fitted)\n") ;
				}
				else
				{
					ts << tr("Conc. Dep. of s (Sigma):     ") << (*component_vector)[i].sigma << tr(" (fixed)\n") ;
				}
				if ((*component_vector)[i].delta_fit)
				{
					ts << tr("Conc. Dep. of D (Delta):     ") << (*component_vector)[i].delta << tr(" (fitted)\n") ;
				}
				else
				{
					ts << tr("Conc. Dep. of D (Delta):     ") << (*component_vector)[i].delta << tr(" (fixed)\n") ;
				}
				ts << tr("Buoyancy (Water, 20ºC):      ") << (1 - ((*component_vector)[i].vbar - (4.25e-4 
																 * ((*run_inf).avg_temperature - 20))) * DENS_20W) << "\n" ;
				ts << tr("Buoyancy (absolute):         ") << (1 - (*component_vector)[i].vbar * density_tb) << "\n\n" ;
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
		for (i=0; i<(*run_inf).scans[cell][lambda]; i++)
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
			k = (unsigned int) ((*run_inf).time[cell][lambda][i]/ 60);
			j = (unsigned int) (0.5 + ((*run_inf).time[cell][lambda][i] - (k * 60)));
			ts.width(5);
			ts << k << tr(" min");
			ts.width(3);
			ts << j << tr(" sec     ");
			ts.width(11);
			ts << (*run_inf).plateau[cell][lambda][i] << " OD\n";
		}
		res_f.close();
	}

}

void US_FeFitter_Single::endFit()
{
	if (GUI)
	{
		update_fitDialog();
		pb_cancel->setText(tr("Close"));
		pb_fit->setEnabled(true);
		lbl_status2->setText(tr("The fitting process converged..."));
		pb_saveFit->setEnabled(true);
		pb_report->setEnabled(true);
		pb_residuals->setEnabled(false);
		pb_overlays->setEnabled(false);
		pb_pause->setEnabled(false);
		pgb_progress->setProgress(totalSteps);
		write_report();

/*		pb_plottwo->setEnabled(true);
		pb_plotthree->setEnabled(true);
		pb_plotfour->setEnabled(true);
		pb_plottwo->setText(tr("Ln(C) vs R^2"));
		pb_plotthree->setText(tr("MW vs R^2"));
		pb_plotfour->setText(tr("MW vs C"));
*/
	}
	emit currentStatus(tr("Converged"));
	emit hasConverged();
	qApp->processEvents(); // wait until the residuals plots
	converged = true;
}

bool US_FeFitter_Single::createHtmlDir()
{
	htmlDir = USglobal->config_list.html_dir + "/" + (*run_inf).run_id;
	QDir d(htmlDir);
	if (d.exists())
	{
		return true;
	}
	else
	{
		if (d.mkdir(htmlDir, true))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

void US_FeFitter_Single::saveFit()
{
	QString fn = QFileDialog::getSaveFileName( USglobal->config_list.root_dir, "*.fef_fit", 0);
	int k;
	if ( !fn.isEmpty() ) 
	{
		k = fn.find(".", 0, FALSE);
		if (k != -1) //if an extension was given, strip it
		{
			fn.truncate(k);
		}
		saveFit(fn);		// the user gave a file name
	}
}

// With this function we can save the entire dataset, fit and run parameters in a binary file for later
// retrieval to do Monte Carlo, etc...

void US_FeFitter_Single::saveFit(const QString &filename)
{

	QString filestr;
	filestr = filename;
	filestr.append(".fef_fit");
	QFile fef_file(filestr);
	if (fef_file.exists())
	{
		if(!QMessageBox::query( tr("Warning"), tr("Attention:\nThis file exists already!\n\nDo you want to overwrite it?"), tr("Yes"), tr("No")))
		{
			fef_file.close();
			return;
		}
	}
	if (fef_file.open(IO_WriteOnly))
	{
		QDataStream ds(&fef_file);
		ds << (QString) (*run_inf).run_id;
		ds << (Q_UINT16) cell;
		ds << (Q_UINT16) lambda;
		ds << (Q_UINT16) parameters;
		ds << (Q_UINT16) (*run_inf).scans[cell][lambda];
		ds << (float) bd_range;
		ds << (float) bd_position;
		ds << (float) density;
		ds << (float) viscosity;
		ds << (float) pow((double) variance, (double) 0.5);
		ds << (Q_UINT32) sim_points;
		for (unsigned int i=0; i<(*run_inf).scans[cell][lambda]; i++)
		{
			ds << (Q_UINT32) (*run_inf).time[cell][lambda][i];
		}
		ds << (Q_INT16) model;
		ds << (Q_INT16) components;
		ds << (float) (*runData).meniscus;
		ds << (float) (*runData).meniscus_range;
		ds << (Q_INT16) (*runData).meniscus_fit;
		ds << (float) (*runData).baseline;
		ds << (float) (*runData).baseline_range;
		ds << (Q_INT16) (*runData).baseline_fit;
		ds << (float) (*runData).slope;
		ds << (float) (*runData).slope_range;
		ds << (Q_INT16) (*runData).slope_fit;
		ds << (float) (*runData).stray;
		ds << (float) (*runData).stray_range;
		ds << (Q_INT16) (*runData).stray_fit;
		ds << (float) (*runData).delta_t;
		ds << (float) (*runData).delta_r;
		for (unsigned int i=0; i<components; i++)
		{
			ds << (float) (*component_vector)[i].sed;
			ds << (float) (*component_vector)[i].sed_range;
			ds << (Q_INT16) (*component_vector)[i].sed_fit;
			ds << (float) (*component_vector)[i].diff;
			ds << (float) (*component_vector)[i].diff_range;
			ds << (Q_INT16) (*component_vector)[i].diff_fit;
			ds << (float) (*component_vector)[i].conc;
			ds << (float) (*component_vector)[i].conc_range;
			ds << (Q_INT16) (*component_vector)[i].conc_fit;
			ds << (float) (*component_vector)[i].sigma;
			ds << (float) (*component_vector)[i].sigma_range;
			ds << (Q_INT16) (*component_vector)[i].sigma_fit;
			ds << (float) (*component_vector)[i].delta;
			ds << (float) (*component_vector)[i].delta_range;
			ds << (Q_INT16) (*component_vector)[i].delta_fit;
			ds << (Q_INT16) (*component_vector)[i].mw_fit;
			ds << (float) (*component_vector)[i].vbar;
			ds << (Q_INT16) (*component_vector)[i].vbar_fit;
		}
		for (unsigned int i=0; i<(*run_inf).scans[cell][lambda]; i++)
		{
			for (unsigned int j=0; j<(*run_inf).points[cell][lambda][0]; j++)
			{
				ds << (float) simulation_c[i][j];
			}
		}
		fef_file.close();
		emit fitLoaded();
	}
}

void US_FeFitter_Single::print()
{
	if (!GUI)
	{
		return;
	}
	QPrinter printer;
	bool plotFlag = true;	// true for analysis plot, false for edit plot
	bool change = true;	// true if it is allowed to select between edit and analysis plot
	US_SelectPlot *sp;
	sp = new US_SelectPlot(&plotFlag, &print_bw, &print_inv, change, "Overlay Plot", "Residuals Plot" );
	sp->exec();
	calc_residuals();
	if (plotFlag)
	{
		if	(printer.setup(0))
		{
			if (print_inv)
			{
				//data_plot->print(printer, QwtFltrInv());			//old version
				data_plot->print(printer, QwtPlotPrintFilter());	//new version
			}
			else
			{
				data_plot->print(printer);
			}
		}
	}
	else
	{
		if	(printer.setup(0))
		{
			if (print_inv)
			{		
				//residuals_plot->print(printer, QwtFltrInv());				//old version
				residuals_plot->print(printer, QwtPlotPrintFilter());		//new version
			}
			else
			{
				residuals_plot->print(printer);
			}
		}
	}
	print_bw = false;
	calc_residuals();
}

float US_FeFitter_Single::calc_residuals()
{
	unsigned int i, j, count, *curve_sim = 0, *curve_exp = 0, index;
	unsigned int *curve_resid = 0, zeroline;
	int current_sign = 0, last_sign = 0;

	float residuals = 0, y_raw, maxOD = -100, minOD = 1000, margin;
	float resid_min=1000, resid_max=-1000;
	double **sim_conc = 0, *radius_exp = 0, *radius_sim = 0, *zero_x = 0, *zero_y = 0;
	QPen p_sim, p_exp;
	QColor dgrey;
	dgrey.setRgb(90, 90, 90);

	if (showGuiFit && GUI)
	{
		sim_conc = new double  *[(*run_inf).scans[cell][lambda]];
		zero_x = new double [2];
		zero_y = new double [2];
		curve_sim = new unsigned int [(*run_inf).scans[cell][lambda]];
		curve_resid = new unsigned int [(*run_inf).scans[cell][lambda]];
		curve_exp = new unsigned int [(*run_inf).scans[cell][lambda]];
		radius_sim = new double [sim_points];
		radius_exp = new double [scanpoints];
		if (print_bw)
		{
			p_sim.setColor(dgrey);
		}
		else
		{
			p_sim.setColor(Qt::red);
		}
		p_sim.setWidth(1);

		if (print_bw)
		{
			p_exp.setColor(Qt::white);
		}
		else
		{
			p_exp.setColor(Qt::yellow);
		}
		p_exp.setWidth(1);

		for (i=0; i<(*run_inf).scans[cell][lambda]; i++)
		{
			sim_conc[i] = new double [sim_points];
		}
		count = 0;
		for (i=0; i<(*run_inf).scans[cell][lambda]; i++)
		{
			for (j=0; j<sim_points; j++)
			{
				sim_conc[i][j] = y_guess[count]; // for local use only
				count ++;
			}
		}
		for (j=0; j<sim_points; j++)
		{
			radius_sim[j] = sim_radius[j];
		}
		for (j=0; j<scanpoints; j++)
		{
			radius_exp[j] = radius[j];
		}
	}
	count = 0;
	for (i=0; i<(*run_inf).scans[cell][lambda]; i++)
	{
		index = 0;
		for (j=0; j<sim_points; j++)
		{
			if(j>=offset1 && j<=offset2)
			{
				simulation_c[i][index] = y_guess[count]; // this gets exported
				index++;
			}
			count ++;
		}
	}
	
	count = 0;
	runData->pos = 0;
	runData->neg = 0;
	runData->runs = 0;
	for (i=0; i<(*run_inf).scans[cell][lambda]; i++)
	{
		index = 0;
		for (j=offset1; j<=offset2; j++)
		{
			y_raw = absorbance[i][index];
			maxOD = max(y_raw, maxOD);
			minOD = min(y_raw, minOD);
			y_delta[count] = y_raw - simulation_c[i][index];
//cout << "Y_delta: " << y_delta[count] << ", y_raw: " << y_raw << ", Sim_c: " << simulation_c[i][index] << endl;
			if (y_delta[count] > 0)
			{
				runData->pos++;
				current_sign = 1;
			}
			else
			{
				runData->neg++;
				current_sign = -1;
			}
			if (j == 0)
			{
				if (y_delta[count] > 0)
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
					runData->runs++;
					last_sign = current_sign;
				}
			}
			if(showGuiFit)
			{
				resid[i][index] = y_delta[count];
//cout << "Residuals: " <<resid[i][index] << endl;
				resid_max = max((float) resid_max, (float) resid[i][index]);
				resid_min = min((float) resid_min, (float) resid[i][index]);
			}
			residuals +=  pow((double) y_delta[count], (double) 2.0);
			index ++;
			count ++;
		}
		qApp->processEvents();
	}
	runData->runs_percent = ((float) runData->runs/ (float) points) * 100.0;
	runs = runData->runs;
	runs_percent = runData->runs_percent;
	if (GUI)
	{
		QString str;
		str.sprintf(tr("%ld Runs (%2.2f"), runs, runs_percent);
		str += " %)";
		lbl_status4->setText(str);
		pb_print->setEnabled(true);
	}
	if(residuals > FLT_MAX)
	{
		residuals = -1;
	}
	if(showGuiFit && GUI)
	{
		margin = (maxOD - minOD)/20;
		data_plot->clear();
		residuals_plot->clear();
		for (i=0; i<(*run_inf).scans[cell][lambda]; i++)
		{
			curve_exp[i] = data_plot->insertCurve("veloc data");
			data_plot->setCurveData(curve_exp[i], radius_exp, absorbance[i], scanpoints);
			data_plot->setCurveStyle(curve_exp[i], QwtCurve::Lines);
			data_plot->setCurvePen(curve_exp[i], p_exp);
			curve_sim[i] = data_plot->insertCurve("veloc data");
			data_plot->setCurveData(curve_sim[i], radius_sim, sim_conc[i], sim_points);
			data_plot->setCurveStyle(curve_sim[i], QwtCurve::Lines);
			data_plot->setCurvePen(curve_sim[i], p_sim);
			data_plot->setAxisScale(QwtPlot::yLeft, minOD - margin, maxOD + margin, 0);
			data_plot->setAxisScale(QwtPlot::xBottom, radius_exp[0] - 0.02, radius_exp[scanpoints-1] + 0.02, 0);
			curve_resid[i] = residuals_plot->insertCurve("residuals");
			residuals_plot->setCurveData(curve_resid[i], radius_exp, resid[i], scanpoints);
			residuals_plot->setCurveStyle(curve_resid[i], QwtCurve::Lines);
			residuals_plot->setCurvePen(curve_resid[i], p_exp);
			residuals_plot->setAxisScale(QwtPlot::xBottom, radius_exp[0] - 0.02, radius_exp[scanpoints-1] + 0.02, 0);
			residuals_plot->setAxisScale(QwtPlot::yLeft, resid_min, resid_max, 0);
		}	
		data_plot->replot();
		zero_x[0] = radius_exp[0] - 0.02;
		zero_x[1] = radius_exp[scanpoints-1] + 0.02;
		zero_y[0] = 0;
		zero_y[1] = 0;
		zeroline = residuals_plot->insertCurve("residuals");
		residuals_plot->setCurveData(zeroline, zero_x, zero_y, 2);
		residuals_plot->setCurveStyle(zeroline, QwtCurve::Lines);
		residuals_plot->setCurvePen(zeroline, p_sim);
		residuals_plot->replot();

		for (unsigned int i=0; i<(*run_inf).scans[cell][lambda]; i++)
		{
			delete [] sim_conc[i];
		}
		delete [] sim_conc;
		delete [] radius_exp;
		delete [] radius_sim;
		delete [] curve_sim;
		delete [] curve_exp;
		delete [] curve_resid;
	}
	return (residuals);
}
