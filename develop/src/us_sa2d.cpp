#include "../include/us_sa2d.h"
#ifndef WIN32
#include <unistd.h>
#endif
#include <math.h>
#include <float.h>

#if defined(USE_PTHREADS)
# include <pthread.h>
#endif
#include <stdlib.h>

#ifndef WIN32
#define SHOW_TIMING
#if defined(SHOW_TIMING)
# include <sys/time.h>
 struct timeval start_tv, end_tv;
#endif
#endif

US_SA2D_W::US_SA2D_W(QWidget *p, const char *name) : Data_Control_W(21, p, name)
{
	additional_plots_flag = false;
	fn = "";
	ti_noise.clear();
	ri_noise.clear();
	setCaption(tr("2-D Spectrum Analysis:"));
	pm = new US_Pixmap();
	fit_status = false;
	sa2d_ctrl_vars.fit_posBaseline = false;
	sa2d_ctrl_vars.fit_negBaseline = false;
	control_window_flag = false;
	iteration = 0;
	old_variance = 0.0;
	sa2d_ctrl_vars.ff0=1.5;
	sa2d_ctrl_vars.min_s = 1.0;
	sa2d_ctrl_vars.max_s = 10.0;
	sa2d_ctrl_vars.resolution_s = 30;
	sa2d_ctrl_vars.min_ff0 = 1.0;
	sa2d_ctrl_vars.max_ff0 = 4.0;
	sa2d_ctrl_vars.resolution_ff0 = 30;
	sa2d_GUI();
	connect(this, SIGNAL(dataLoaded()), this, SLOT(enableButtons()));
	connect(pb_exsingle, SIGNAL(clicked()), this, SLOT(calc_points()));
	connect(pb_exrange, SIGNAL(clicked()), this, SLOT(calc_points()));
	sa2d = new SA2D(&fit_status);
}

US_SA2D_W::US_SA2D_W(double min_s, double max_s, QString data_fn, 
unsigned int cell, unsigned int lambda, QWidget *p, const char *name) 
: Data_Control_W(21, p, name)
{
	additional_plots_flag = false;
	ti_noise.clear();
	ri_noise.clear();
	setCaption(tr("2-D Spectrum Analysis:"));
	pm = new US_Pixmap();
	fit_status = false;
	sa2d_ctrl_vars.fit_posBaseline = false;
	sa2d_ctrl_vars.fit_negBaseline = false;
	control_window_flag = false;
	sa2d_ctrl_vars.ff0 = 1.5;
	iteration = 0;
	sa2d_ctrl_vars.min_s = min_s;
	sa2d_ctrl_vars.max_s = max_s;
	sa2d_ctrl_vars.resolution_s = 20;
	sa2d_ctrl_vars.min_ff0 = 1.0;
	sa2d_ctrl_vars.max_ff0 = 4.0;
	sa2d_ctrl_vars.resolution_ff0 = 40;
	old_variance = 0.0;
	reset_flag = true;
	sa2d_GUI();
	connect(this, SIGNAL(dataLoaded()), this, SLOT(enableButtons()));
	connect(pb_exsingle, SIGNAL(clicked()), this, SLOT(calc_points()));
	connect(pb_exrange, SIGNAL(clicked()), this, SLOT(calc_points()));
	sa2d = new SA2D(&fit_status);
	this->fn = data_fn;
	load_data(fn);
	this->selected_cell = cell;
	this->selected_lambda = lambda;
	cell_select->setSelected(cell, true);
	lambda_select->setSelected(lambda, true);
	show_cell(cell);
	show_lambda(lambda);
}

US_SA2D_W::~US_SA2D_W()
{
	delete resplot;
}

Simulation_values US_SA2D_W::Simulation_Routine(Simulation_values sv)
{
	QString str;
	s_distribution.clear();
	D_distribution.clear();
	iteration ++;
	printf("correction %.12g scan.size %d\n", correction, run_inf.scans[selected_cell][selected_lambda]);

	printf("temp %.12g vbar %g viscosity %.12g density %.12g\n", run_inf.avg_temperature, vbar, viscosity, density);
	for (unsigned int i=0; i<sv.solutes.size(); i++)
	{
// 	adjusted user-space for temperature and buffer to match expt. data
		s_distribution.push_back(sv.solutes[i].s/correction);
		D_distribution.push_back((R * (run_inf.avg_temperature + K0))/(AVOGADRO * sv.solutes[i].k 
										    * 6.0 * viscosity * 0.01 * M_PI * pow((9.0 * s_distribution[i]
											 * sv.solutes[i].k * vbar * viscosity * 0.01)/(2.0 * (1.0 - vbar 
											 * density)), 0.5)));
cout << "Solutes " << i+1 << ":  s: " <<  sv.solutes[i].s << " (" << s_distribution[i] 
<< "), k: " << sv.solutes[i].k << " (" << D_distribution[i] << ")\n";
	}
	str.sprintf("%d", iteration);
	SA_control->lbl_iteration2->setText(str);
	Simulation_values temp_values;
	ti_noise = sv.ti_noise;
	ri_noise = sv.ri_noise;
	temp_values.variance = (double) calc_residuals();
	Solute temp_solute;
// reset distribution to user-space for plotting and Emre's routine (need to check accuracy of this calculation!)
	calc_20W_distros();
	for (unsigned int i=0; i<sv.solutes.size(); i++)
	{
		if (frequency[i] > 0.0)
		{
			temp_solute.s = sv.solutes[i].s;
			temp_solute.k = sv.solutes[i].k;
			temp_solute.c = frequency[i];
			temp_values.solutes.push_back(temp_solute);
		}
	}
	return temp_values;
}

void US_SA2D_W::fit()
{
	int return_value;
	QString str;
/*
	if (fit_status)
	{
		fit_status = false;
		SA_control->pb_fit->setText(tr("Control Window"));
	}
	else
	{
*/

//cout << "res-s: " << (unsigned int) sa2d_ctrl_vars.resolution_s << ", res_f: " << (unsigned int) sa2d_ctrl_vars.resolution_ff0
//<< ", min_s: " <<  sa2d_ctrl_vars.min_s * 1.0e-13 << ", max_s: " << sa2d_ctrl_vars.max_s * 1.0e-13 << ", min_f: " << 
//sa2d_ctrl_vars.min_ff0<< ", max_f: " << sa2d_ctrl_vars.max_ff0 << endl;

		fit_status = true;
		SA_control->pb_fit->setText(tr("Stop Fit"));
		sa2d->set_params(&fit_status, 
				 (void *)this, 
				 (unsigned int) sa2d_ctrl_vars.resolution_s, 
				 (unsigned int) sa2d_ctrl_vars.resolution_ff0,
				 sa2d_ctrl_vars.min_s * 1.0e-13,
				 sa2d_ctrl_vars.max_s * 1.0e-13,
				 sa2d_ctrl_vars.min_ff0,
				 sa2d_ctrl_vars.max_ff0);
		if (SA_control->uniform)
		{
			iteration = 0;
			return_value = sa2d->uniform_run((unsigned int) SA_control->uniform_steps);
			update_result_vector();
			if (return_value >= 0)
			{
				SA_control->cb_local_uniform->setEnabled(true);
				SA_control->cb_random->setEnabled(true);
				SA_control->cb_regularize->setEnabled(true);
				SA_control->cb_clip->setEnabled(true);
				SA_control->cb_coalesce->setEnabled(true);
			}
			else
			{
				cout << "Uniform Run Returned with error value: " << return_value << endl;
			}
		}
		else if (SA_control->local_uniform)
		{
			return_value = sa2d->local_uniform_run((unsigned int) SA_control->local_uniform_steps, SA_control->scaling_factor, SA_control->scaling_factor2);
			if(return_value < 0)
			{
				cout << "Local Uniform Run Returned with error value: " << return_value << endl;
			}
			update_result_vector();
		}
		else if (SA_control->random)
		{
			return_value = sa2d->local_random_run((unsigned int) SA_control->random_steps,
															  SA_control->random_distance);
			if(return_value < 0)
			{
				cout << "Local Random Run Returned with error value: " << return_value << endl;
			}
			update_result_vector();
		}
		else if (SA_control->regularize)
		{
		  //			sa2d->fprint_solutes(stdout);
			return_value = sa2d->regularize(SA_control->regfactor);
			if(return_value < 0)
			{
				cout << "Regularize Run Returned with error value: " << return_value << endl;
			}
			//		sa2d->fprint_solutes(stdout);
			update_result_vector();
		}
		else if (SA_control->coalesce)
		{
//			return_value = sa2d->coalesce(SA_control->metric, SA_control->threshold);
//			sa2d->fprint_solutes(stdout);
			return_value = sa2d->coalesce(SA_control->metric, 0.0);
			if(return_value < 0)
			{
				cout << "Coalesce Run Returned with error value: " << return_value << endl;
			}
			//			sa2d->fprint_solutes(stdout);
			update_result_vector();
		}
		else if (SA_control->clip)
		{
			return_value = sa2d->clip_data((unsigned int) SA_control->clip_steps);
			if(return_value < 0)
			{
				cout << "Clip Data Run Returned with error value: " << return_value << endl;
			}
			update_result_vector();
		}
//		fit_status = false;
		if (SA_control->autoupdate)
		{
			SA_control->plot();
		}
		SA_control->pb_fit->setText(tr("Start Fit"));
//	}
}

void US_SA2D_W::update_result_vector()
{
	QString str;
	str.sprintf("%6.4e", old_variance);
	SA_control->lbl_variance2->setText(str);
	str.sprintf("%6.4e", sa2d->get_variance());
	SA_control->lbl_new_variance2->setText(str);
	str.sprintf("%6.4e", old_variance - sa2d->get_variance());
	SA_control->lbl_difference2->setText(str);
	old_variance = sa2d->get_variance();
// get a copy of the fitting result (already in 20C,W space):
	vector <Solute> solute_copy;
	solute_copy = sa2d->get_solutes();
// clear the control solute vector to accept the copy:
	SA_control->solutes.clear();
	struct solute_description tmp_solute;
	//	printf("vbar20 %.12g\n", vbar20);
	//	puts("update result vector");
	for (unsigned int i=0; i<solute_copy.size(); i++)
	{
		tmp_solute.s = solute_copy[i].s;
		tmp_solute.ff0 = solute_copy[i].k;
		tmp_solute.c = solute_copy[i].c;
		tmp_solute.D = R * K20/(AVOGADRO * tmp_solute.ff0 * 9.0 * VISC_20W * M_PI
		* pow( (2.0 * tmp_solute.s * tmp_solute.ff0 * vbar20 * VISC_20W)/(1.0 - vbar20 * DENS_20W), 0.5));
		tmp_solute.mw = (tmp_solute.s/tmp_solute.D)*(R * K20)/(1.0 - vbar20 * DENS_20W);
		tmp_solute.f = (tmp_solute.mw * (1.0 - vbar20 * DENS_20W)) / (tmp_solute.s * AVOGADRO);
		SA_control->solutes.push_back(tmp_solute);
		//		printf("s %g k%g c%g\n", tmp_solute.s, tmp_solute.ff0, tmp_solute.c);
	}
}

void US_SA2D_W::sa2d_GUI()
{
	resplot = new US_ResidualPlot(0,0);
	pb_second_plot->setText(tr("Reset Data"));
	delete lbl1_excluded;
	delete lbl2_excluded;
	smoothing_counter = NULL;
	range_counter = NULL;
	position_counter = NULL;

	
	xpos = 176;
	ypos = 379;
	
	pb_control = new QPushButton(tr("Fit Control"), this);
	Q_CHECK_PTR(pb_control);
	pb_control->setAutoDefault(false);
	pb_control->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_control->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_control->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_control->setEnabled(false);
	connect(pb_control, SIGNAL(clicked()), SLOT(control_window()));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_info1 = new QLabel(tr(" Status\n Info:"),this);
	lbl_info1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
	lbl_info1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_info1->setAlignment(AlignLeft|AlignVCenter);
	lbl_info1->setGeometry(xpos, ypos, 65, 4*buttonh);
	
	xpos += 65 + spacing;

	lbl_info2 = new QLabel(tr(""),this);
	lbl_info2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_info2->setAlignment(AlignCenter|AlignVCenter);
	lbl_info2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
	lbl_info2->setGeometry(xpos, ypos, 2 * buttonw-65, 4*buttonh);
	lbl_info2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	xpos = border;
	ypos += 4*buttonh + spacing;

	pb_addplots = new QPushButton(tr("Additional Graphs"), this);
	Q_CHECK_PTR(pb_addplots);
	pb_addplots->setAutoDefault(false);
	pb_addplots->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_addplots->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_addplots->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_addplots->setEnabled(false);
	connect(pb_addplots, SIGNAL(clicked()), SLOT(addplots()));

	xpos += buttonw + spacing;

	pb_loadfit = new QPushButton(tr("Load Fit"), this);
	Q_CHECK_PTR(pb_loadfit);
	pb_loadfit->setAutoDefault(false);
	pb_loadfit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_loadfit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_loadfit->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_loadfit->setEnabled(true);
	connect(pb_loadfit, SIGNAL(clicked()), SLOT(loadfit()));
	setup_GUI();
}

void US_SA2D_W::setup_GUI()
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
		
	rows = 10, columns = 4, spacing = 2, j=0;
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
	subGrid2->addWidget(pb_vbar,j,0);
	subGrid2->addWidget(vbar_le,j,1);
	subGrid2->addMultiCellWidget(pb_control,j,j,2,3);
	j++;
	subGrid2->addMultiCellWidget(lbl_info1,j,j+3,0,0);
	subGrid2->addMultiCellWidget(lbl_info2,j,j+3,1,3);
	j=j+4;
	subGrid2->addMultiCellWidget(pb_addplots,j,j,0,1);
	subGrid2->addMultiCellWidget(pb_loadfit,j,j,2,3);
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
void US_SA2D_W::second_plot()
{
	reset();
}

void US_SA2D_W::loadfit()
{
	QString fn = QFileDialog::getOpenFileName(USglobal->config_list.result_dir, "*.sa2d_fit.??", 0);
	if ( !fn.isEmpty() ) 
	{
		loadfit(fn);
	}
}

void US_SA2D_W::loadfit(QString filename)
{
	unsigned int i, j;
	QString str;
	Q_UINT16 int16;
	Q_UINT16 temp_scans;
	Q_UINT32 int32;
	double dval;

	QFile f;
	f.setName(filename);
	if (f.open(IO_ReadOnly))
	{
		QDataStream ds(&f);
		reset_flag = true;
		ds >> str;
		ds >> int16;
		selected_cell = (unsigned int) int16;
		ds >> int16;
		selected_lambda = (unsigned int) int16;
		ds >> temp_scans;
		ds >> int16;
		points = int16;
		fn = USglobal->config_list.result_dir + "/" + str + ".us.v";
		reset_flag = true;
		if (load_data(fn) != 0)
		{
			QMessageBox::message(tr("Attention:"), tr("There was a problem loading your fit.\n\n"
			                                            "Please try again."));
			return;
		}
		details();
		for (i=0; i<temp_scans; i++)
		{
			ds >> int32;
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
		control_window(); // instantiate the control window so we can access the control variables
		ds >> int16; // number of solutes
		solute_description sd;
		SA_control->solutes.clear();
		for (i=0; i<int16; i++)
		{
			ds >> sd.c;
			ds >> sd.mw;
			ds >> sd.s;
			ds >> sd.D; 
			ds >> sd.ff0;
			ds >> sd.f;
			SA_control->solutes.push_back(sd);
		}
		ds >> int16;
		ti_noise.clear();
		if (int16 == 1)
		{
			ds >> int16;
			for (i=0; i<int16; i++)
			{
				ds >> dval;
				ti_noise.push_back(dval);
			}
		}
		else
		{
			for (i=0; i<points; i++)
			{
				ti_noise.push_back(0.0);
			}
		}
		ds >> int16;
		ri_noise.clear();
		if (int16 == 1)
		{
			ds >> int16;
			for (i=0; i<int16; i++)
			{
				ds >> dval;
				ri_noise.push_back(dval);
			}
		}
		else
		{
			for (i=0; i<temp_scans; i++)
			{
				ri_noise.push_back(0.0);
			}
		}
		ds >> sa2d_ctrl_vars.min_s;
		SA_control->cnt_min_s->setValue(sa2d_ctrl_vars.min_s);
		ds >>	sa2d_ctrl_vars.max_s;
		SA_control->cnt_max_s->setValue(sa2d_ctrl_vars.max_s);
		ds >> int16;
		sa2d_ctrl_vars.resolution_s = int16;
		SA_control->cnt_resolution_s->setValue((double) sa2d_ctrl_vars.resolution_s);
		ds >> sa2d_ctrl_vars.min_ff0;
		SA_control->cnt_min_ff0->setValue(sa2d_ctrl_vars.min_ff0);
		ds >> sa2d_ctrl_vars.max_ff0;
		SA_control->cnt_max_ff0->setValue(sa2d_ctrl_vars.max_ff0);
		ds >> int16;
		sa2d_ctrl_vars.resolution_ff0 = int16;
		SA_control->cnt_resolution_ff0->setValue((double) sa2d_ctrl_vars.resolution_ff0);
		ds >> rmsd;
		ds >> temp_scans;
		struct mfem_scan mscan;
		model.scan.clear();
		for (i=0; i<temp_scans; i++)
		{
			ds >> int16;
			ds >> mscan.time;
			mscan.conc.clear();
			for (j=0; j<int16; j++)
			{
				ds >> dval;
				mscan.conc.push_back(dval);
			}
			model.scan.push_back(mscan);
		}
		f.close();
	}
	struct mfem_scan single_scan;
	single_scan.conc.clear();
	clear_data(&residuals);
	for (i=0; i<points; i++)
	{
		residuals.radius.push_back(radius[i]);
		single_scan.conc.push_back(0.0); // this is the baseline vector to be added first
	}
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		single_scan.time = (double) run_inf.time[selected_cell][selected_lambda][i];
		residuals.scan.push_back(single_scan);
	}
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		for (j=0; j<points; j++)
		{
			residuals.scan[i].conc[j] = absorbance[i][j] - model.scan[i].conc[j] - ti_noise[j] - ri_noise[i]; 
		}
	}
	plot_edit();
	pb_save->setEnabled(true);
	pb_view->setEnabled(true);
	pb_print->setEnabled(true);
	vector <Solute> tempSolutes;
	tempSolutes.clear();
	tempSolutes.resize(SA_control->solutes.size());
	for (i=0; i<SA_control->solutes.size(); i++)
	{
		tempSolutes[i].s = SA_control->solutes[i].s;
		tempSolutes[i].k = SA_control->solutes[i].ff0;
		tempSolutes[i].c = SA_control->solutes[i].c;
	}
	sa2d->set_solutes(tempSolutes);
	update_result_vector();
	SA_control->cb_local_uniform->setEnabled(true);
	SA_control->cb_random->setEnabled(true);
	SA_control->cb_regularize->setEnabled(true);
	SA_control->cb_clip->setEnabled(true);
	SA_control->cb_coalesce->setEnabled(true);
}

// plot additional plots
void US_SA2D_W::addplots()
{
	if(!additional_plots_flag)
	{
		additional_plots = new US_PlotData(run_inf.run_id, &run_inf.scans[selected_cell][selected_lambda],
		&points, selected_cell, selected_lambda, &additional_plots_flag, absorbance, radius, &model, 
		&ti_noise, &ri_noise);
		additional_plots->show();
		connect(this, SIGNAL(excludeUpdated()), additional_plots, SLOT(update_plot()));
	}
	else
	{
		additional_plots->raise();
	}
}

void US_SA2D_W::plot_graph()
{
	resplot->setData(&residuals, global_Xpos+30, global_Ypos+30);
	resplot->show();
	resplot->repaint();
	if(additional_plots_flag)
	{
		additional_plots->update_plot();
	}
	unsigned int i, j;
	plot_edit();
	long *resids;
	resids = new long [model.scan.size()];
	for (j=0; j<model.scan.size(); j++)
	{
		resids[j] = edit_plot->insertCurve("SA2D model");
		edit_plot->setCurvePen(resids[j], QPen(Qt::red, 1, SolidLine));
		edit_plot->setCurveData(resids[j], radius, &model.scan[j].conc[0], points);
		qApp->processEvents();
	}
	delete [] resids;
	edit_plot->replot();

	analysis_plot->setAxisAutoScale(QwtPlot::yLeft);
	QString s1, s2;
	analysis_plot->clear();
	s1.sprintf(tr(": Cell %d (%d nm)\nResiduals"), selected_cell+1, run_inf.wavelength[selected_cell][selected_lambda]);
	s2 = tr("Run ");
	s2.append(run_inf.run_id);
	s2.append(s1);
	analysis_plot->setTitle(s2);
	double *x, line_x[2];
	double *y, line_y[2];
	unsigned int *curve, zeroline;
	x = new double [residuals.scan[0].conc.size()];
	y = new double [residuals.scan[0].conc.size()];
	curve = new unsigned int [residuals.scan.size()]; // one extra for the zero line
	QwtSymbol symbol;
	QPen p_raw, p_zero;
	p_raw.setWidth(1);
	p_zero.setWidth(2);
	symbol.setStyle(QwtSymbol::Ellipse);
	p_raw.setColor(Qt::yellow);
	p_zero.setColor(Qt::red);
	symbol.setPen(Qt::yellow);
	symbol.setBrush(Qt::yellow);
	analysis_plot->setAxisTitle(QwtPlot::xBottom, tr("Radius in cm"));
	analysis_plot->setAxisTitle(QwtPlot::yLeft, "OD Difference");
	line_x[0] = residuals.radius[0] - (residuals.radius[residuals.radius.size()-1] - residuals.radius[0])/30.0;
	line_x[1] = residuals.radius[residuals.radius.size()-1] + (residuals.radius[residuals.radius.size()-1] - residuals.radius[0])/30;
	analysis_plot->setAxisScale(QwtPlot::xBottom, line_x[0], line_x[1]);
	line_y[0] = 0.0;
	line_y[1] = 0.0;
	symbol.setSize(1);
	zeroline = analysis_plot->insertCurve("Zero Line");
	analysis_plot->setCurveStyle(zeroline, QwtCurve::Lines);
	analysis_plot->setCurveData(zeroline, line_x, line_y, 2);
	analysis_plot->setCurvePen(zeroline, p_zero);

	for (i=0; i<residuals.radius.size(); i++)
	{
		x[i] = residuals.radius[i];
	}
	for (i=0; i<residuals.scan.size(); i++)
	{
		for (j=0; j<residuals.scan[0].conc.size(); j++)
		{
			y[j] = residuals.scan[i].conc[j];
		}
		curve[i] = analysis_plot->insertCurve("Residual Scan");
		analysis_plot->setCurveSymbol(curve[i], symbol);
		analysis_plot->setCurveStyle(curve[i], QwtCurve::NoCurve);
		analysis_plot->setCurveData(curve[i], x, y, residuals.radius.size());
		analysis_plot->setCurvePen(curve[i], p_raw);
	}
	analysis_plot->replot();
	delete [] curve;
	delete [] x;
	delete [] y;
}

void US_SA2D_W::enableButtons()
{
	pb_save->setEnabled(false);
	pb_view->setEnabled(false);
	pb_print->setEnabled(false);
	pb_control->setEnabled(true);
	pb_addplots->setEnabled(true);
	calc_correction(run_inf.avg_temperature);
	update_distribution();
}

void US_SA2D_W::save()
{
	if (step == 0)
	{
		QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n	   Click on \"Load Data\"\n"));
		return;
	}
	write_sa2d();
	write_res();
	QPixmap p;
	QString fileName;
	plot_graph();
	fileName.sprintf(htmlDir + "/sa2d_resid_%d%d.", selected_cell + 1, selected_lambda +1);
	p = QPixmap::grabWidget(analysis_plot, 2, 2, analysis_plot->width() - 4, analysis_plot->height() - 4);
	pm->save_file(fileName, p);
	fileName.sprintf(htmlDir + "/sa2d_edited_%d%d.", selected_cell + 1, selected_lambda +1);
	p = QPixmap::grabWidget(edit_plot, 2, 2, edit_plot->width() - 4, edit_plot->height() - 4);
	pm->save_file(fileName, p);
	fileName.sprintf(htmlDir + "/sa2d_pixmap_%d%d.", selected_cell + 1, selected_lambda +1);
	p = QPixmap::grabWidget(resplot, 1, 1, resplot->width() - 2, resplot->height() - 2);
	pm->save_file(fileName, p);
	if (!SA_control->widget3d_flag)
	{
		SA_control->plot();
	}
	fileName.sprintf(htmlDir + "/sa2d_3ddis_%d%d.png", selected_cell + 1, selected_lambda +1);
	SA_control->mainwindow->dumpImage(fileName, "PNG");
}

void US_SA2D_W::update_distribution()
{
	s_distribution.clear();
	D_distribution.clear();
	unsigned int i, j;
	double tmp1, tmp2, s;
	float ff0_1;
	ff0_1 = ((double)sa2d_ctrl_vars.max_ff0 - (double) sa2d_ctrl_vars.min_ff0)/(double) (sa2d_ctrl_vars.resolution_ff0);
//	cout << "res: " << sa2d_ctrl_vars.resolution_ff0 << ", " << (double) (sa2d_ctrl_vars.resolution_ff0) << ", ff0: " << ff0_1 << endl;
	sa2d_ctrl_vars.ff0 = sa2d_ctrl_vars.min_ff0;
	for (i=0; i<sa2d_ctrl_vars.resolution_ff0; i++)
	{
		tmp1 = ((double)sa2d_ctrl_vars.max_s - (double) sa2d_ctrl_vars.min_s)/(double) (sa2d_ctrl_vars.resolution_s);
		tmp2 = sa2d_ctrl_vars.min_s;
		for (j=0; j<sa2d_ctrl_vars.resolution_s; j++)
		{
			s = tmp2 * 1.0e-13 / correction; // this distro is s20,W corrected from vhw and needs to be 
			s_distribution.push_back(s);     // adjusted for temperature and buffer to match expt. data
			D_distribution.push_back((R * (run_inf.avg_temperature + K0))/(AVOGADRO * sa2d_ctrl_vars.ff0 * 6.0 * viscosity * 0.01 * M_PI
			* pow((9.0 * s * sa2d_ctrl_vars.ff0 * vbar * viscosity * 0.01)/(2.0 * (1.0 - vbar * density)), 0.5)));
			tmp2 += tmp1;
/*
			cout << "s: " << s << ", D: " << (R * (run_inf.avg_temperature + K0))/(AVOGADRO * sa2d_ctrl_vars.ff0 * 6.0 * viscosity * 0.01 * M_PI
			* pow((9.0 * s * sa2d_ctrl_vars.ff0 * vbar * viscosity * 0.01)/(2.0 * (1.0 - vbar * density)), 0.5)) << ", f/f0: " 
			<< sa2d_ctrl_vars.ff0 << ", corr: " << correction << ", tmp2: " << tmp2 << endl;
*/
		}
		sa2d_ctrl_vars.ff0 += ff0_1;
	}
}

void US_SA2D_W::help()
{
	US_Help *online_help;
	online_help = new US_Help(this);
	online_help->show_help("manual/sa2d.html");
}

void US_SA2D_W::view()
{
	if (step == 0)
	{
		QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n	   Click on \"Load Data\"\n"));
		return;
	}
	QString filestr, filename, temp;
	filename = USglobal->config_list.result_dir.copy();
	filename.append("/");
	filename.append(run_inf.run_id);
	temp.sprintf(".%d%d", selected_cell+1, selected_lambda+1);
	filestr = filename.copy();
	filestr.append(".sa2d_res");
	filestr.append(temp);
	write_res();
	TextEdit *e;
	e = new TextEdit();
	e->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	e->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
	e->load(filestr);
	e->show();
}

void US_SA2D_W::write_res()
{
	int count=0;
	QString filestr, filename, temp, str;
	filename = USglobal->config_list.result_dir.copy();
	filename.append("/");
	filename.append(run_inf.run_id);
	temp.sprintf(".%d%d", selected_cell+1, selected_lambda+1);
	filestr = filename.copy();
	filestr.append(".sa2d_res");
	filestr.append(temp);
	QFile res_f(filestr);
	if (res_f.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&res_f);
		ts <<    "***************************************************\n";
		ts << tr("*        2-dimensional Spectrum Analysis          *\n");
		ts <<    "***************************************************\n\n\n";
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
		ts << tr("Correction Factor:       ") << correction << "\n\n\n";
		ts << tr("Data Analysis Settings:\n\n");
		ts << tr("Residual Mean Square Deviation: ") << rmsd << "\n";
		ts << tr("Variance:                       ") << rmsd*rmsd << "\n";
		if (sa2d_ctrl_vars.fit_posBaseline)
		{
			ts << tr("Fitted Baseline:        +") << frequency[frequency.size()-1] << "\n";
		}
		if (sa2d_ctrl_vars.fit_negBaseline)
		{
			ts << tr("Fitted Baseline:        -") << frequency[frequency.size()-1] << "\n";
		}
		ts << tr("\n\nWeight-Average sedimentation coefficient:\n\n");
		float sum_mw=0.0;
		float sum_s=0.0;
		float sum_freq=0.0;
		float sum_D=0.0;
		count = 0;
		for (i=0; i<SA_control->solutes.size(); i++)
		{
			sum_mw += SA_control->solutes[i].c * SA_control->solutes[i].mw;
			sum_s += SA_control->solutes[i].c * SA_control->solutes[i].s;
			sum_D += SA_control->solutes[i].c * SA_control->solutes[i].D;
			sum_freq += SA_control->solutes[i].c;
		}
		ts << str.sprintf("Weight Average S20,W: %6.4e\n", sum_s/sum_freq);
		ts << str.sprintf("Weight Average D20,W: %6.4e\n", sum_D/sum_freq);
		ts << str.sprintf("Weight Average Molecular Weight: %6.4e\n", sum_mw/sum_freq);
		ts << "\n\n";
		ts << tr("Distribution Information:\n\n");
	   ts << tr("Molecular Weight:    S 20,W:       D 20,W:        f/f0:              f:       Relative Conc.:\n\n");
		for (i=0; i<SA_control->solutes.size(); i++)
		{
			ts << str.sprintf("%6.4e           %6.4e    %6.4e   %6.4e       %6.4e     (%7.3f", 
			SA_control->solutes[i].mw, 
			SA_control->solutes[i].s,
			SA_control->solutes[i].D,
			SA_control->solutes[i].ff0,
			SA_control->solutes[i].f,
			100.0 * SA_control->solutes[i].c/sum_freq);
			ts << " %)\n";
		}
		res_f.close();
	}
}

void US_SA2D_W::write_sa2d()
{
	QString filename1, filename2, temp;
	temp.sprintf(".%d%d", selected_cell+1, selected_lambda+1);
	filename1 = USglobal->config_list.result_dir + "/" + run_inf.run_id + ".sa2d_dis" + temp;
	QFile f(filename1);
	unsigned int i, j;
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
		float sum_freq=0.0;
		for (i=0; i<SA_control->solutes.size(); i++)
		{
			sum_freq += SA_control->solutes[i].c;
		}
//	   ts << tr("C\tMW\ts20,W\tD20,W\tf/f0\tf\n");
		ts << "S_apparent\tS_20,W    \tD_apparent\tD_20,W    \tMW        \tFrequency\tf/f0(20,W)\n"; 
		for (i=0; i<SA_control->solutes.size(); i++)
		{
			ts << str.sprintf("%6.4e\t%6.4e\t%6.4e\t%6.4e\t%6.4e\t%6.4e\t%6.4e\n", 
			SA_control->solutes[i].s/correction,
			SA_control->solutes[i].s,
			SA_control->solutes[i].D,
			SA_control->solutes[i].D * (K20 * viscosity_tb)/((100.0 * VISC_20W) * (run_inf.avg_temperature + K0)),
			SA_control->solutes[i].mw, 
			100.0 * SA_control->solutes[i].c/sum_freq,
			SA_control->solutes[i].ff0);
//			SA_control->solutes[i].f);
		}
		f.close();
	}
	QFile f1(filename1);
	filename2 = USglobal->config_list.result_dir + "/" + run_inf.run_id + ".model" + temp;
	f.setName(filename2);
	if (f.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&f);
		QString str;
		ts << "SA2D" << endl;
		ts << run_inf.run_id << endl;
		ts << "0" << endl; // noninteracting model string identifier
		ts << SA_control->solutes.size() << endl;		
		for (i=0; i<SA_control->solutes.size(); i++)
		{
			ts << SA_control->solutes[i].c << endl;
			ts << SA_control->solutes[i].s << endl;
			ts << SA_control->solutes[i].D << endl;
			ts << "0" << endl;
			ts << "0" << endl;
		}
		f.close();
	}


	filename1 = USglobal->config_list.result_dir.copy() + "/" + run_inf.run_id + ".sa2d_fit" +
					temp.sprintf(".%d%d", selected_cell+1, selected_lambda+1);
	f.setName(filename1);
	if (f.open(IO_WriteOnly))
	{
		QDataStream ds(&f);
		ds << (QString) run_inf.run_id;
		ds << (Q_UINT16) selected_cell;
		ds << (Q_UINT16) selected_lambda;
		ds << (Q_UINT16) run_inf.scans[selected_cell][selected_lambda];
		ds << (Q_UINT16) points;
		for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
		{
			ds << (Q_UINT32) run_inf.time[selected_cell][selected_lambda][i];
		}
		ds << (Q_UINT16) SA_control->solutes.size();
		for (i=0; i<SA_control->solutes.size(); i++)
		{// all doubles:
			ds << 
			SA_control->solutes[i].c <<
			SA_control->solutes[i].mw <<
			SA_control->solutes[i].s <<
			SA_control->solutes[i].D << 
			SA_control->solutes[i].ff0 <<
			SA_control->solutes[i].f;
		}
		Q_UINT16 m;
		if (SA_control->fit_ti)
		{
			m = 1;
			ds << m;
			ds << (Q_UINT16) ti_noise.size();
			for (i=0; i<ti_noise.size(); i++)
			{
				ds << ti_noise[i];
			}
		}
		else
		{
			m = 0;
			ds << m;
		}
		if (SA_control->fit_ri)
		{
			m = 1;
			ds << m;
			ds << (Q_UINT16) ri_noise.size();
			for (i=0; i<ri_noise.size(); i++)
			{
				ds << ri_noise[i];
			}
		}
		else
		{
			m = 0;
			ds << m;
		}
		ds << sa2d_ctrl_vars.min_s;
		ds <<	sa2d_ctrl_vars.max_s;
		ds << (Q_UINT16) sa2d_ctrl_vars.resolution_s;
		ds << sa2d_ctrl_vars.min_ff0;
		ds << sa2d_ctrl_vars.max_ff0;
		ds << (Q_UINT16) sa2d_ctrl_vars.resolution_ff0;
		ds << rmsd; // float
		ds << (Q_UINT16) model.scan.size();
		for (i=0; i<model.scan.size(); i++)
		{
			ds << (Q_UINT16) model.scan[i].conc.size();
			ds << model.scan[i].time; // double
			for (j=0; j<model.scan[i].conc.size(); j++)
			{
				ds << model.scan[i].conc[j]; // double
			}
		}
		f.close();
	}
}

void clear_data(mfem_data *d)
{
  unsigned int i;
  for (i=0; i<(*d).scan.size(); i++) {
    (*d).scan[i].conc.clear();
  }
  (*d).radius.clear();
  (*d).scan.clear();
}

typedef struct {
  unsigned int thread;
  unsigned int start_pos;
  unsigned int end_pos;
  double *nnls_a;
  struct mfem_data experiment;
  struct runinfo *run_inf;
  vector <double> s_distribution;
  vector <double> D_distribution;
  unsigned int selected_cell;
  unsigned int selected_lambda;
  double bottom;
  double initial_concentration;
  struct mfem_initial *initCVector;
  unsigned int points;
  unsigned int b_size;
  US_SA2D_W *sa2d_w;
} thread_arg_mfem;

typedef struct {
  unsigned int thread;
  unsigned int start_pos;
  unsigned int end_pos;
  double *nnls_a;
  unsigned int points;
  unsigned int b_size;
  unsigned int experiment_scan_size;
  unsigned int s_distribution_size;
  double *L_bars;
  double *small_a;
  double *small_b;
  double **absorbance;
  double *a_bar;
  US_SA2D_W *sa2d_w;
} thread_arg_tiri;

int progress_pos;

#if defined(USE_PTHREADS)
pthread_mutex_t qApp_lock;

void *do_thread_mfem(void *s) {
  QString str;
  thread_arg_mfem *t;
  t = (thread_arg_mfem *)s;
  //  printf("hello from thread %d [%d %d] size %d\n", t->thread, t->start_pos, t->end_pos, t->end_pos - t->start_pos);
  //  printf("sizeof s_distribution %d b_size %d \n", t->s_distribution.size(), t->b_size);
  unsigned int i, j, k;
    
  struct mfem_data fem_data;
  US_MovingFEM *mfem;
  mfem = new US_MovingFEM(&fem_data, false);
  for (i=t->start_pos; i < t->end_pos; i++) {
    //    printf("thread %d processing i %d %g %g\n", t->thread, i, t->s_distribution[i], t->D_distribution[i]);
    clear_data(&fem_data);
    for (j=0; j<t->experiment.scan.size(); j++) {
      for (k=0; k<t->points; k++) {
	// reset concentration to zero:
	t->experiment.scan[j].conc[k] = 0.0;
      }
    }
    mfem->set_params(100, 
		     t->s_distribution[i],
		     t->D_distribution[i],
		     (double) t->run_inf->rpm[t->selected_cell][t->selected_lambda][0], 
		     t->experiment.scan[t->experiment.scan.size()-1].time, 
		     (double) t->run_inf->meniscus[t->selected_cell], t->bottom, t->initial_concentration, t->initCVector);

    // generate the next term of the linear combination:
    mfem->skipEvents = true;
    mfem->run();

    // interpolate model function to the experimental data so dimension 1 in A matches dimension of B:
    mfem->interpolate(&(t->experiment), &fem_data);
		
    for (j=0; j<t->experiment.scan.size(); j++) {
      for (k=0; k<t->points; k++) {
				// populate the A matrix for the NNLS routine with the model function:
	t->nnls_a[i * t->b_size + j * t->points + k] = t->experiment.scan[j].conc[k];
      }
    }
    pthread_mutex_lock(&qApp_lock);
    //    if(!t->thread) {
    //      qApp->processEvents();
    //    }
    //    str.sprintf(tr("Calculating Lamm Equation\nWorking on Term %d of %d\n"), i+1, t->s_distribution.size()); 
    //    t->sa2d_w->lbl_info2->setText(str);
    progress_pos++;
    t->sa2d_w->progress->setProgress(progress_pos);		
    t->sa2d_w->SA_control->progress->setProgress(progress_pos);
    pthread_mutex_unlock(&qApp_lock);
  }
  pthread_exit(0);
}

void *do_thread_tiri(void *s) {
  thread_arg_tiri *t;
  t = (thread_arg_tiri *)s;
  //  printf("hello from thread %d [%d %d] size %d\n", t->thread, t->start_pos, t->end_pos, t->end_pos - t->start_pos);
  //  printf("sizeof s_distribution %d b_size %d \n", t->s_distribution.size(), t->b_size);
  unsigned int i, j, k, l;

  for(l = t->start_pos; l < t->end_pos; l++) {
    //    printf("thread %d l = %d of %d\n", t->thread, l, t->s_distribution_size);
    //    fflush(stdout);
    for(j = 0; j < t->points; j++) {
      for(i = 0; i < t->experiment_scan_size; i++) {
	t->small_b[l] += (t->absorbance[i][j] - t->a_bar[j]) * (t->nnls_a[l * t->b_size + i * t->points + j] - t->L_bars[l * t->points + j]);
	for(k = 0; k < t->s_distribution_size; k++) {
	  t->small_a[k * t->s_distribution_size + l] +=
	    (t->nnls_a[k * t->b_size + i * t->points + j] - t->L_bars[k * t->points + j]) *
	    (t->nnls_a[l * t->b_size + i * t->points + j] - t->L_bars[l * t->points + j]);
	}
      }
    }		  
    pthread_mutex_lock(&qApp_lock);
    //    if(!t->thread) {
    //      qApp->processEvents();
    //    }
    progress_pos++;
    t->sa2d_w->progress->setProgress(progress_pos);		
    t->sa2d_w->SA_control->progress->setProgress(progress_pos);
    pthread_mutex_unlock(&qApp_lock);
  }
  pthread_exit(0);
}
#endif

float US_SA2D_W::calc_residuals()
{
	float variance;
	QString str;
	if (fit_status == false)
	{
		return(-1.0);
	}
	double *nnls_a, *nnls_b, *nnls_x, nnls_rnorm, *nnls_wp, *nnls_zzp, initial_concentration = 1.0;
	int *nnls_indexp, result;
	struct mfem_scan single_scan;
	struct mfem_data experiment, fem_data; 
	struct mfem_initial initCVector; 
	unsigned int i, j, k, count;
	k=0;
	if (sa2d_ctrl_vars.fit_posBaseline)
	{
		k++;
	}
	if (sa2d_ctrl_vars.fit_negBaseline)
	{
		k++;
	}
	i = points * run_inf.scans[selected_cell][selected_lambda];
//	j = sa2d_ctrl_vars.resolution_ff0 * sa2d_ctrl_vars.resolution_s + k;
	j = s_distribution.size() + k;
	nnls_a = new double [i * j]; // contains the model functions, end-to-end
	unsigned int a_size = i * j;
	nnls_b = new double [i]; // contains the experimental data
	unsigned int b_size = i;
	nnls_zzp = new double [i]; // pre-allocated working space for nnls
	nnls_x = new double [j]; // the solution vector, pre-allocated for nnls
	nnls_wp = new double [j]; // pre-allocated working space for nnls, On exit, wp[] will contain the dual solution vector, wp[i]=0.0 for all i in set p and wp[i]<=0.0 for all i in set z. */
	nnls_indexp = new int [j];
	US_MovingFEM *mfem;
	mfem = new US_MovingFEM(&fem_data, false);
	

// initialize experimental data array sizes and radius positions:

	clear_data(&experiment);
	clear_data(&residuals);
	clear_data(&model); // model will hold the simulated model for plotting
	progress->reset();
	progress->setTotalSteps(s_distribution.size());
	SA_control->progress->reset();
	SA_control->progress->setTotalSteps(s_distribution.size());
	initCVector.concentration.clear();
	initCVector.radius.clear();
	frequency.clear();
	double bottom = calc_bottom(rotor_list, cp_list, run_inf.rotor, 
	run_inf.centerpiece[selected_cell], 0, run_inf.rpm[selected_cell][selected_lambda][0]);
	single_scan.conc.clear();
	for (i=0; i<points; i++)
	{
		experiment.radius.push_back(radius[i]);
		residuals.radius.push_back(radius[i]);
		single_scan.conc.push_back(0.0); // this is the baseline vector to be added first
	}
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		single_scan.time = (double) run_inf.time[selected_cell][selected_lambda][i];
		experiment.scan.push_back(single_scan);
		residuals.scan.push_back(single_scan);
	}
	count = 0;
	printf("scan[0][0] %.12g\n", absorbance[0][0]);
	for (i=0; i<experiment.scan.size(); i++)
	{
		for (j=0; j<points; j++)
		{
			// populate the A matrix for the NNLS routine with the model function:
			nnls_b[count] = absorbance[i][j];
			count ++;
		}
	}


#if defined(SHOW_TIMING)
	gettimeofday(&start_tv, NULL);
#endif
#if defined(USE_PTHREADS)
	if(SA_control->thread_count > 1)
	{
		unsigned int pthread_count = (unsigned int) SA_control->thread_count;
		pthread_t p_threads[pthread_count];
		thread_arg_mfem thread_args[pthread_count];
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
		unsigned int size_per = s_distribution.size() / pthread_count;
		unsigned int remainder = s_distribution.size() % pthread_count;
		pthread_mutex_init(&qApp_lock, NULL);
		progress_pos = 0;
		progress->setProgress(progress_pos);		
		SA_control->progress->setProgress(progress_pos);
		str.sprintf(tr("Calculating Lamm Equation\nUsing %d threads\n"), pthread_count); 
		lbl_info2->setText(str);
		for(i = 0; i < pthread_count; i++)
		{
	    	thread_args[i].thread = i;
	    	thread_args[i].nnls_a = nnls_a;
	    	thread_args[i].experiment = experiment;
	    	thread_args[i].run_inf = &run_inf;
	    	thread_args[i].s_distribution = s_distribution;
	    	thread_args[i].D_distribution = D_distribution;
	    	thread_args[i].selected_cell = selected_cell;
	    	thread_args[i].selected_lambda = selected_lambda;
	    	thread_args[i].bottom = bottom;
	    	thread_args[i].initial_concentration = initial_concentration;
	    	thread_args[i].initCVector = &initCVector;
	    	thread_args[i].points = points;
	    	thread_args[i].b_size = b_size;
	    	thread_args[i].sa2d_w = this;
	    	if(!i)
			{
				thread_args[i].start_pos = i * size_per;
			}
			else
			{
				thread_args[i].start_pos = thread_args[i-1].end_pos;
			}
			thread_args[i].end_pos = thread_args[i].start_pos + size_per;
			if(i < remainder)
			{
				thread_args[i].end_pos++;
			}
			if(i == pthread_count - 1)
			{
				thread_args[i].end_pos = s_distribution.size();
			}
//			if(i)
//			{
				pthread_create(&p_threads[i], &attr, do_thread_mfem, (void *)&thread_args[i]);
//			}
		}
//		do_thread_mfem((void *)&thread_args[0]);
		for(i = 0; i < pthread_count; i++)
		{
			qApp->processEvents();
			pthread_join(p_threads[i], NULL);
		}
	}
	else
#endif // USE_PTHREADS
	{
		count = 0;
//		struct mfem_data save_experiment = experiment;
//		struct runinfo save_run_inf = run_inf;
		for (i=0; i<s_distribution.size(); i++)
		{
//			experiment = save_experiment;
//			run_inf = save_run_inf;
//			delete mfem;
//			mfem = new US_MovingFEM(&fem_data, false);
			qApp->processEvents();
			if(!fit_status)
			{
				delete [] nnls_a;
				delete [] nnls_b;
				delete [] nnls_zzp;
				delete [] nnls_x;
				delete [] nnls_wp;
				delete [] nnls_indexp;
				return (-1.0);
			}
// for each term in the linear combination we need to reset the 
// simulation vectors, the experimental vector simply keeps getting overwritten:
			str.sprintf(tr("Calculating Lamm Equation\nWorking on Term %d of %d\n"), i+1, s_distribution.size()); 
			lbl_info2->setText(str);
			clear_data(&fem_data);
			for (j=0; j<experiment.scan.size(); j++)
			{
				for (k=0; k<points; k++)
				{
// reset concentration to zero:
					experiment.scan[j].conc[k] = 0.0;
				}
			}
			/*	   printf("experiment s %.12g D %.12g rpm %d dur %g men %.12g bot %.12g\n",
		  s_distribution[i],
		  D_distribution[i],
		  run_inf.rpm[selected_cell][selected_lambda][0], 
		  experiment.scan[experiment.scan.size()-1].time, 
		  run_inf.meniscus[selected_cell], 
		  bottom);
			*/
			mfem->set_params(100, s_distribution[i], D_distribution[i],
			(double) run_inf.rpm[selected_cell][selected_lambda][0], 
			experiment.scan[experiment.scan.size()-1].time, 
			(double) run_inf.meniscus[selected_cell], bottom, initial_concentration, &initCVector);

			//			mfem->fprintparams(stdout);
// generate the next term of the linear combination:
			mfem->run();

// interpolate model function to the experimental data so dimension 1 in A matches dimension of B:
			mfem->interpolate(&experiment, &fem_data);

			for (j=0; j<experiment.scan.size(); j++)
			{
				for (k=0; k<points; k++)
				{
// populate the A matrix for the NNLS routine with the model function:
					nnls_a[count] = experiment.scan[j].conc[k];
					count ++;
				}
			}
			progress->setProgress(i+1);	
			SA_control->progress->setProgress(i+1);
		}
	}
#if defined(SHOW_TIMING)
	gettimeofday(&end_tv, NULL);
	printf("mfem timing = %ld\n", 1000000l * (end_tv.tv_sec - start_tv.tv_sec) +  end_tv.tv_usec - start_tv.tv_usec);
#endif
	if (sa2d_ctrl_vars.fit_posBaseline)
	{
		// add positive baseline contributions:
		for (j=0; j<experiment.scan.size(); j++)
		{
			for (k=0; k<points; k++)
			{
				nnls_a[count] = 1.0;
				count ++;
			}
		}
	}
	if (sa2d_ctrl_vars.fit_negBaseline)
	{
		// add negative baseline contributions:
		for (j=0; j<experiment.scan.size(); j++)
		{
			for (k=0; k<points; k++)
			{
				nnls_a[count] = -1.0;
				count ++;
			}
		}
	}
	i = points * run_inf.scans[selected_cell][selected_lambda];
	lbl_info2->setText(tr("Calculating NNLS solution..."));
	qApp->processEvents();
	k=0;
	if (sa2d_ctrl_vars.fit_posBaseline)
	{
		k++;
	}
	if (sa2d_ctrl_vars.fit_negBaseline)
	{
		k++;
	}
	cout << tr("Calculating NNLS solution...");
	printf("%g %g %g\n",
	       absorbance[0][0],
	       absorbance[0][5],
	       absorbance[5][0]);
	if(SA_control->fit_ti)
	{
		printf("s size %d d_size %d\n", (int) s_distribution.size(), (int) D_distribution.size());
		printf("points %d exp_scans %d b_size %d a_size %d\n", points, 
		    (int) experiment.scan.size(), b_size, a_size);
		double *L = new double [b_size]; // this is Sum(concentration * Lamm) for the models after NNLS
		double *L_bars = new double [points * s_distribution.size()]; // an average for each distribution
		double *L_tildes = new double [experiment.scan.size() * s_distribution.size()]; // an average for each distribution

		double *small_a = new double[s_distribution.size() * s_distribution.size()];
		double *small_b = new double[s_distribution.size()];
		double *small_x = new double[s_distribution.size()];
		
		double *L_bar = new double [points];  // a concentration weighted average
		double *L_tilde = new double [experiment.scan.size()];
		unsigned int l;
		unsigned int countNZ;
		unsigned int countL;
//		unsigned int iterations = 0;
		double *new_ti_noise = new double [points];
		double *new_ri_noise = new double [experiment.scan.size()];

		double *a_bar = new double [points];
		double *a_tilde = new double [experiment.scan.size()];

// start with no noise
		memset(new_ti_noise, 0, points * sizeof(double));
		memset(new_ri_noise, 0, experiment.scan.size() * sizeof(double));


// compute a_bar, the average experiment signal at each radius
// compute a_tilde, the average experiment signal at each time

		memset(a_tilde, 0, experiment.scan.size() * sizeof(double));
		if(SA_control->fit_ri)
		{
			for (i=0; i<experiment.scan.size(); i++)
			{
				for (j=0; j<points; j++)
				{
					a_tilde[i] += absorbance[i][j];
				}
				a_tilde[i] /= points;
			}
		}

		memset(a_bar, 0, points * sizeof(double));
		for (j=0; j<points; j++)
		{
			for (i=0; i<experiment.scan.size(); i++)
			{
//				a_bar[j] += absorbance[i][j];
				a_bar[j] += absorbance[i][j] - a_tilde[i];
			}
			a_bar[j] /= experiment.scan.size();
		}

		for(j = 0; j < 10; j++) {
		  printf(" %g", a_bar[j]);
		}
		puts("");

// compute the new nnls_b vector including noise
		count = 0;

// compute L_tildes, the average model signal at each radius
		memset(L_tildes, 0, experiment.scan.size() * sizeof(double) * s_distribution.size());
		count = 0;
		if(SA_control->fit_ri)
		{
			for(l = 0; l < s_distribution.size(); l++)
			{
				for (i=0; i<experiment.scan.size(); i++)
				{
					for (j=0; j<points; j++)
					{
						L_tildes[l * experiment.scan.size() + i] += nnls_a[l * b_size + i * points + j];
						count++;
					}
					L_tildes[l * experiment.scan.size() + i] /= points;
				}
			}
		}

		for(l = 0; l < 3; l++)
		{
			printf("L_tildes for eq %d :", l);
			for(i = 0; i < 10; i++)
			{
				printf(" %g", L_tildes[l * experiment.scan.size() + i]);
			}
			puts("");
		}

// compute L_bars, the average for each equation
		puts("ti nnls step 1 create L_bar");

		memset(L_bars, 0, points * s_distribution.size() * sizeof(double));
		for(l = 0; l < s_distribution.size(); l++)
		{
			for(j = 0; j < points; j++)
			{
				for(i = 0; i < experiment.scan.size(); i++)
				{
//					L_bars[l * points + j] += nnls_a[l * b_size + i * points + j];
					L_bars[l * points + j] += nnls_a[l * b_size + i * points + j] - L_tildes[l * experiment.scan.size() + i];
				}
				L_bars[l * points + j] /= experiment.scan.size();
			}
		}
		for(j = 0; j < 10; j++) {
		  printf(" %g", L_bars[j]);
		}
		puts("");

		puts("ti nnls step 2 create the nnls matrix & rhs");
// unncessary?
		{
			unsigned int k;
			double residual;
// setup small_a, small_b for the alternate nnls
			
			memset(small_a, 0, s_distribution.size() * s_distribution.size() * sizeof(double));
			memset(small_b, 0, s_distribution.size() * sizeof(double));
			memset(small_x, 0, s_distribution.size() * sizeof(double));



#if defined(SHOW_TIMING)
			gettimeofday(&start_tv, NULL);
#endif
			progress_pos = 0;
			progress->setProgress(progress_pos);		
			SA_control->progress->setProgress(progress_pos);

#if defined(USE_PTHREADS)
			if(SA_control->thread_count > 1)
			{
				unsigned int pthread_count = (unsigned int) SA_control->thread_count;
				pthread_t p_threads[pthread_count];
				thread_arg_tiri thread_args[pthread_count];
				pthread_attr_t attr;
				pthread_attr_init(&attr);
				pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
				unsigned int size_per = s_distribution.size() / pthread_count;
				unsigned int remainder = s_distribution.size() % pthread_count;
				str.sprintf(tr("Preparing for NNLS\nUsing %d threads\n"), pthread_count); 
				lbl_info2->setText(str);
				for(i = 0; i < pthread_count; i++)
				{
					thread_args[i].thread = i;
					thread_args[i].nnls_a = nnls_a;
					thread_args[i].points = points;
					thread_args[i].b_size = b_size;
					thread_args[i].experiment_scan_size = experiment.scan.size();
					thread_args[i].s_distribution_size = s_distribution.size();
					thread_args[i].L_bars = L_bars;
					thread_args[i].small_a = small_a;
					thread_args[i].small_b = small_b;
					thread_args[i].absorbance = absorbance;
					thread_args[i].a_bar = a_bar;
					thread_args[i].sa2d_w = this;
					if(!i)
					{
						thread_args[i].start_pos = i * size_per;
					}
					else
					{
						thread_args[i].start_pos = thread_args[i-1].end_pos;
					}
					thread_args[i].end_pos = thread_args[i].start_pos + size_per;
					if(i < remainder)
					{
						thread_args[i].end_pos++;
					}
					if(i == pthread_count - 1)
					{
						thread_args[i].end_pos = s_distribution.size();
					}
//					if(i)
//					{
						pthread_create(&p_threads[i], &attr, do_thread_tiri, (void *)&thread_args[i]);
//					}
				}
// do_thread_tiri((void *)&thread_args[0]);
				for(i = 0; i < pthread_count; i++)
				{
					qApp->processEvents();
					pthread_join(p_threads[i], NULL);
				}
			}
			else
#endif // USE_PTHREADS
			{
				for(l = 0; l < s_distribution.size(); l++)
				{
//					printf("l = %d of %d\r", l, s_distribution.size());
//					fflush(stdout);
					for(j = 0; j < points; j++)
					{
						for(i = 0; i < experiment.scan.size(); i++)
						{
							small_b[l] += (absorbance[i][j] - a_bar[j]) * (nnls_a[l * b_size + i * points + j] - L_bars[l * points + j]);
							for(k = 0; k < s_distribution.size(); k++)
							{
								small_a[k * s_distribution.size() + l] +=
								(nnls_a[k * b_size + i * points + j] - L_bars[k * points + j]) *
								(nnls_a[l * b_size + i * points + j] - L_bars[l * points + j]);
							}
						}
					}
					str.sprintf(tr("Preparing for NNLS\nWorking on Term %d of %d\n"), l+1, s_distribution.size()); 
					lbl_info2->setText(str);
					progress_pos++;
					progress->setProgress(progress_pos);		
					SA_control->progress->setProgress(progress_pos);
					qApp->processEvents();
				}
			}
		for(j = 0; j < 10; j++) {
		  printf(" %g ", small_a[j]);
		}
		puts("");
		for(j = 0; j < 10; j++) {
		  printf(" %g ", small_b[j]);
		}
		puts("");
#if defined(SHOW_TIMING)
			gettimeofday(&end_tv, NULL);
			printf("nnls prep time = %ld\n", 1000000l * (end_tv.tv_sec - start_tv.tv_sec) +  end_tv.tv_usec - start_tv.tv_usec);
#endif
// unncessary?
			{
				double cks1 = 0e0, cks2 = 0e0;
				for(i = 0; i < s_distribution.size(); i++)
				{
					cks1 += small_b[i];
				}
				for(i = 0; i < s_distribution.size() * s_distribution.size(); i++)
				{
					cks2 += small_a[i];
				}
				printf("cks %g %g\n", cks1, cks2);
			}
			puts("ti nnls step 3 run nnls");
			result = nnls(small_a, s_distribution.size(), s_distribution.size(), s_distribution.size(),
			small_b, small_x, &residual, NULL, NULL, NULL);
			memcpy(nnls_x, small_x, s_distribution.size() * sizeof(double));
		}
		puts("ti nnls step 4 compute ti & ri noise");
	  
// print out original nnls concentrations & the new concentrations

// compute L the sum of the equations
		memset(L, 0, b_size * sizeof(double));

// we ignore the fit_baseline stuff
		count = 0;
		countNZ = 0;
		for(l = 0; l < s_distribution.size(); l++)
		{
			if(nnls_x[l] > 0)
			{
				countNZ++;
				countL = 0;
//				printf("l %d count %d l * b_size %d\n", l, count, l * b_size);
				for(i = 0; i < experiment.scan.size(); i++)
				{
					for(j = 0; j < points; j++)
					{
						if(countL != i * points + j)
						{
							printf("countL insanity %d %d\n", countL, i * points + j);
						}
						L[countL] += nnls_x[l] * nnls_a[l * b_size + i * points + j];
						countL++;
						count++;
					} 
				}
			}
			else
			{
				count += b_size;
			}
		}

		if(countNZ == 0)
		{
			fprintf(stderr, "countNZ is zero!\n");
			exit(-1);
		}
// now L contains the best fit sum of L equations

// compute L_tilde, the average model signal at each radius
		memset(L_tilde, 0, experiment.scan.size() * sizeof(double));
		if(SA_control->fit_ri)
		{
			count = 0;
			for (i=0; i<experiment.scan.size(); i++)
			{
				for (j=0; j<points; j++)
				{
					L_tilde[i] += L[i * points + j];
					count++;
				}
				L_tilde[i] /= points;
			}
		}

// compute L_bar, the average model signal at each radius
		memset(L_bar, 0, points * sizeof(double));
		for (j=0; j<points; j++)
		{
			for (i=0; i<experiment.scan.size(); i++)
			{
//				L_bar[j] += L[i * points + j];
				L_bar[j] += L[i * points + j] - L_tilde[i];
			}
			L_bar[j] /= experiment.scan.size();
		}

// compute ti_noise
		printf("new ti noise");
		for(i = 0; i < points; i++)
		{
			new_ti_noise[i] = a_bar[i] - L_bar[i];
			if(i < 10)
			{
				printf(" %g", new_ti_noise[i]);
			}
		}
		puts("");
// unncessary?
		{
			double cks = 0e0;
			for(i = 0; i < points; i++)
			{
				cks += new_ti_noise[i];
			}
			printf("sum of b(i) = %g\n", cks);
		}

// compute ri_noise, this is not correct!
		printf("new ri noise");
		for(i = 0; i < experiment.scan.size(); i++)
		{
			new_ri_noise[i] = a_tilde[i] - L_tilde[i];
			if(i < 10)
			{
				printf(" %g", new_ti_noise[i]);
			}
		}
		puts("");
// unncessary?		
		{
			double cks = 0e0;
			for(i = 0; i < experiment.scan.size(); i++)
			{
				cks += new_ri_noise[i];
			}
			printf("sum of beta(i) = %g\n", cks);
		}

		delete [] L_bars;
		delete [] a_bar;
		delete [] a_tilde;
		delete [] L;
		delete [] L_bar;
		delete [] L_tilde;
		delete [] L_tildes;
		delete [] small_a;
		delete [] small_b;
		delete [] small_x;
		 
// copy new_ti_noise to ti_noise
		ti_noise.clear();
		for(i = 0; i < points; i++)
		{
			ti_noise.push_back(new_ti_noise[i]);
		}
		printf("ti noise");
		for(i = 0; i < 10; i++)
		{
			printf(" %g", ti_noise[i]);
		}
		puts("");
// copy new_ri_noise to ri_noise
		ri_noise.clear(); 
		for(i = 0; i < experiment.scan.size(); i++)
		{
			ri_noise.push_back(new_ri_noise[i]);
		}
		puts("ti nnls end");
// unncessary?		
		{
			unsigned int k;
			for (j=0; j<experiment.scan.size(); j++)
			{
				for (k=0; k<points; k++)
				{
					residuals.scan[j].conc[k] = 0;
				}
			}
		}
	}
	else
	{
	  //      printf("nnls_b %.12g %.12g %.12g %.12g\n", nnls_b[0], nnls_b[1], nnls_b[300], nnls_b[1000]);
		result = nnls(nnls_a, i, i, s_distribution.size() + k,
		nnls_b, nnls_x, &nnls_rnorm, nnls_wp, nnls_zzp, nnls_indexp);
// zero ti_noise
		ti_noise.clear();
		for(i = 0; i < points; i++)
		{
			ti_noise.push_back(0);
		}
// no ri_noise for now
		ri_noise.clear(); 
		for(i = 0; i < experiment.scan.size(); i++)
		{
			ri_noise.push_back(0);
		}
		printf("nnls norm %g\n", nnls_rnorm);
	}
	lbl_info2->setText(tr("Calculating Residuals..."));

	for (i=0; i<s_distribution.size(); i++)
	{
		frequency.push_back(nnls_x[i]);
		if (nnls_x[i] != 0.0)
		{
		  printf("solute %d %g %g %g\n", i, s_distribution[i], D_distribution[i],
			 nnls_x[i]);
		  
			clear_data(&fem_data);
			for (j=0; j<experiment.scan.size(); j++)
			{
				for (k=0; k<points; k++)
				{
					// reset concentration to zero:
					experiment.scan[j].conc[k] = 0.0;
				}
			}
			mfem->set_params(100, s_distribution[i], D_distribution[i],
			(double) run_inf.rpm[selected_cell][selected_lambda][0], 
			experiment.scan[experiment.scan.size()-1].time, 
			(double) run_inf.meniscus[selected_cell], bottom, initial_concentration, &initCVector);
			// generate the next term of the linear combination:
			mfem->run();

			// interpolate model function to the experimental data so dimension 1 in A matches dimension of B:
			mfem->interpolate(&experiment, &fem_data);
			double cks4 = 0e0;
			for (j=0; j<experiment.scan.size(); j++)
			{
				for (k=0; k<points; k++)
				{
					residuals.scan[j].conc[k] += nnls_x[i] * experiment.scan[j].conc[k];;
					cks4 += residuals.scan[j].conc[k];
				}
			}
			printf("nnls used %d cks %g\n", i, cks4);
			qApp->processEvents();
		}
	}
	if (sa2d_ctrl_vars.fit_posBaseline)
	{
		// add positive baseline contribution:
		frequency.push_back(nnls_x[s_distribution.size()]);
		if (nnls_x[s_distribution.size()] != 0.0)
		{
			for (j=0; j<experiment.scan.size(); j++)
			{
				for (k=0; k<points; k++)
				{
					residuals.scan[j].conc[k] += nnls_x[s_distribution.size()];
				}
			}
		}
	}
	if (sa2d_ctrl_vars.fit_negBaseline)
	{
		int dimension;
		// add negative baseline contribution:
		if (sa2d_ctrl_vars.fit_posBaseline) // if we are also fitting the positive component, we need to increment one additional dimension
		{
			dimension = s_distribution.size() + 1;
		}
		else
		{
			dimension = s_distribution.size();
		}
		frequency.push_back(nnls_x[dimension]);
		if (nnls_x[dimension] != 0.0)
		{
			for (j=0; j<experiment.scan.size(); j++)
			{
				for (k=0; k<points; k++)
				{
					residuals.scan[j].conc[k] -= nnls_x[dimension];
				}
			}
		}
	}
	model = residuals;
	rmsd = 0.0;
// unncessary?		
	{
		double cks1 = 0e0, cks2 = 0e0, cks3 = 0e0;
		for (j=0; j<experiment.scan.size(); j++)
		{
			for (k=0; k<points; k++)
			{
				residuals.scan[j].conc[k] = absorbance[j][k] - residuals.scan[j].conc[k] - ti_noise[k] - ri_noise[j]; 
				cks1 += absorbance[j][k];
				cks2 += residuals.scan[j].conc[k];
				cks3 += ti_noise[k];
//				cout << "C[" << j << "][" << k << "]: " << residuals.scan[j].conc[k] << endl;
				rmsd += residuals.scan[j].conc[k] * residuals.scan[j].conc[k];
			}
		}
		printf("cks %g %g %g\n", cks1, cks2, cks3);
	}
	rmsd /= points * experiment.scan.size();
	variance = rmsd;
	rmsd = pow((double)rmsd, 0.5);
	lbl_info2->setText(str.sprintf(tr("Solution converged...\nRMSD: %8.6e,\nVariance: %8.6e\nIterations: %d"), rmsd, rmsd*rmsd, iteration));
    cout << str.
	sprintf(tr
		("Solution converged...\nRMSD: %8.6e,\nVariance: %8.6e\n"),
		rmsd, rmsd * rmsd);
	calc_20W_distros();
	delete [] nnls_a;
	delete [] nnls_b;
	delete [] nnls_zzp;
	delete [] nnls_x;
	delete [] nnls_wp;
	delete [] nnls_indexp;
	pb_save->setEnabled(true);
	pb_view->setEnabled(true);
	pb_print->setEnabled(true);
	delete mfem;
	return variance;
}

void US_SA2D_W::control_window()
{
	QString model_id;
	if (control_window_flag)
	{
		if (SA_control->isVisible())
		{
			SA_control->raise();
		}
		else
		{
			SA_control->show();
		}
		return;
	}
	calc_points();
	SA_control = new US_Sa2d_Control(&sa2d_ctrl_vars, &control_window_flag);
	connect(SA_control, SIGNAL(mem_changed()), SLOT(calc_points())); // recalculate points if parameters change
	connect(SA_control, SIGNAL(distribution_changed()), SLOT(update_distribution())); // recalculate points if parameters change
	connect(SA_control->pb_fit, SIGNAL(clicked()), SLOT(fit())); // recalculate points if parameters change
	connect(SA_control->pb_plot, SIGNAL(clicked()), SLOT(plot_graph())); // recalculate points if parameters change
	connect(SA_control->pb_save, SIGNAL(clicked()), SLOT(save())); // recalculate points if parameters change
	SA_control->show();
}

void US_SA2D_W::calc_points()
{
	sa2d_ctrl_vars.points = points * run_inf.scans[selected_cell][selected_lambda];
	if (control_window_flag)
	{
		SA_control->calc_memory();
	}
}

void US_SA2D_W::calc_20W_distros()
{
	mw.clear();
	SA_control->solutes.clear();
	float f0, vol, rad_sphere, frict;
	struct solute_description tmp_solute;
	printf("viscosity_tb %.12g avg temp %.12g\n", viscosity_tb, run_inf.avg_temperature);
	printf("D20 correction %.12g\n", (K20 * viscosity_tb)/((100.0 * VISC_20W) * (run_inf.avg_temperature + K0)));
	for (unsigned int i=0; i< s_distribution.size(); i++)
	{
		cout << frequency[i] << endl;
		tmp_solute.s = s_distribution[i] * correction;
		tmp_solute.D = D_distribution[i] * (K20 * viscosity_tb)/((100.0 * VISC_20W) * (run_inf.avg_temperature + K0));
		tmp_solute.mw = (tmp_solute.s/tmp_solute.D)*(R * K20)/(1.0 - vbar20 * DENS_20W);
		tmp_solute.c = frequency[i];

		// take an s/MW pair and calculate an f/f0 corrected for 20,W:
		vol = vbar20 * tmp_solute.mw / AVOGADRO;
		rad_sphere = pow((double) (vol * (3.0/4.0))/M_PI, (double) (1.0/3.0));
		f0 = rad_sphere * 6.0 * M_PI * VISC_20W;
		frict = (tmp_solute.mw * (1.0 - vbar20 * DENS_20W)) / (tmp_solute.s * AVOGADRO);
		tmp_solute.f = frict;
		tmp_solute.ff0 = frict/f0;
		SA_control->solutes.push_back(tmp_solute);
	}
}

void US_SA2D_W::clear_data(mfem_data *d)
{
	unsigned int i;
	for (i=0; i<(*d).scan.size(); i++)
	{
		(*d).scan[i].conc.clear();
	}
	(*d).radius.clear();
	(*d).scan.clear();
}


