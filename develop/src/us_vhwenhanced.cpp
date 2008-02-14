#include "../include/us_vhwenhanced.h"

US_vhwEnhanced::US_vhwEnhanced(QWidget *p, const char *name) : Data_Control_W(1, p, name)
{
	baseline_flag = true;
	bd_range=90;
	bd_position = 5;
	range_counter->setValue(bd_range);
	position_counter->setValue(bd_position);
	fe_completed = false;

	pm = new US_Pixmap();
	pb_second_plot->setText(tr("Distribution Plot"));
	setup_already = false;	// for garbage collection
	fe_completed = false; // do not include late datapoints on first pass
	
	delete lbl1_excluded;
	delete lbl2_excluded;
	
	xpos = border+buttonw+spacing;
	ypos = 379;

	groups_on = false;
	group_start = true;
	groups = 0;


	ypos += buttonh + spacing + 3;

	pb_groups = new QPushButton(tr("Select Groups"), this);
	Q_CHECK_PTR(pb_groups);
	pb_groups->setAutoDefault(false);
	pb_groups->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_groups->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
//	pb_groups->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_groups, SIGNAL(clicked()), SLOT(select_groups()));

	xpos =border;
	ypos += buttonh + spacing;

	pb_cofs = new QPushButton(tr("C(s) Analysis"), this);
	Q_CHECK_PTR(pb_cofs);
	pb_cofs->setAutoDefault(false);
	pb_cofs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_cofs->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
//	pb_cofs->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_cofs->setEnabled(false);
	connect(pb_cofs, SIGNAL(clicked()), SLOT(cofs()));

	xpos += buttonw + spacing;

	pb_sa2d = new QPushButton(tr("2D Spectrum Analysis"), this);
	Q_CHECK_PTR(pb_sa2d);
	pb_sa2d->setAutoDefault(false);
	pb_sa2d->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_sa2d->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
//	pb_sa2d->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_sa2d->setEnabled(false);
//	pb_sa2d->setMinimumSize(buttonw, buttonh);
	connect(pb_sa2d, SIGNAL(clicked()), SLOT(sa2d()));

	xpos =border;
	ypos += buttonh + spacing;

	tolerance_lbl = new QLabel(tr("Back Diffusion Toler.:"), this);
	Q_CHECK_PTR(tolerance_lbl);
	tolerance_lbl->setAlignment(AlignLeft|AlignVCenter);
	tolerance_lbl->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	tolerance_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
//	tolerance_lbl->setMinimumSize(buttonw, buttonh);
//	tolerance_lbl->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos += buttonw + spacing;

	tolerance = 1e-3;
	tolerance_counter= new QwtCounter(this);
	Q_CHECK_PTR(tolerance_counter);
	tolerance_counter->setRange(0, 10, 1e-3);
	tolerance_counter->setValue(tolerance);
	tolerance_counter->setNumButtons(3);
	tolerance_counter->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
//	tolerance_counter->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(tolerance_counter, SIGNAL(valueChanged(double)), SLOT(update_tolerance(double)));

	xpos =border;
	ypos += buttonh + spacing;

	division_lbl = new QLabel(tr(" Divisions:"), this);
	Q_CHECK_PTR(division_lbl);
	division_lbl->setAlignment(AlignLeft|AlignVCenter);
	division_lbl->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	division_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
//	division_lbl->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos += buttonw + spacing;

	divisions = 50;
	division_counter= new QwtCounter(this);
	Q_CHECK_PTR(division_counter);
	division_counter->setRange(2, 1000, 1);
	division_counter->setValue(50);
	division_counter->setNumButtons(3);
	division_counter->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
//	division_counter->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(division_counter, SIGNAL(valueChanged(double)), SLOT(update_divisions(double)));

	exclude_singlecurve = analysis_plot->insertCurve("Exclusion Single Candidate");
	exclude_rangecurve = analysis_plot->insertCurve("Exclusion Range Candidate");
	connect (this, SIGNAL (controls_updated()), this, SLOT (activate_reset()));
	setup_GUI();
}

US_vhwEnhanced::~US_vhwEnhanced()
{
}

void US_vhwEnhanced::setup_GUI()
{
	int j=0;
	int rows = 16, columns = 4, spacing = 2;
	
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
//	subGrid1->setColStretch(2, 1);
	
	rows = 10, columns = 4, spacing = 2, j=0;
	QGridLayout * subGrid2 = new QGridLayout(rows, columns, spacing);
	for (int i=0; i<rows; i++)
	{
		subGrid2->setRowSpacing(i, 26);
	}
	subGrid2->addMultiCellWidget(pb_reset,j,j,0,1);
	subGrid2->addMultiCellWidget(pb_groups,j,j,2,3);
	j++;
	subGrid2->addMultiCellWidget(pb_cofs,j,j,0,1);
	subGrid2->addMultiCellWidget(pb_sa2d,j,j,2,3);
	j++;
	subGrid2->addMultiCellWidget(tolerance_lbl,j,j,0,1);
	subGrid2->addMultiCellWidget(tolerance_counter,j,j,2,3);
	j++;
	subGrid2->addMultiCellWidget(division_lbl,j,j,0,1);
	subGrid2->addMultiCellWidget(division_counter,j,j,2,3);
	j++;
	subGrid2->addMultiCellWidget(smoothing_lbl,j,j,0,1);
	subGrid2->addMultiCellWidget(smoothing_counter,j,j,2,3);
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
//	subGrid2->setColStretch(1, 1);
		
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

void US_vhwEnhanced::excludeSingleSignal(unsigned int scan)
{
	double *tempx, *tempy, test;
	tempx = new double [2];
	tempy = new double [2];
	test = 1.0/sqrt((double) run_inf.time[selected_cell][selected_lambda][scan]);
	for (unsigned int i=0; i<divisions; i++)
	{
		tempx[0] = test;
		tempx[1] = test;
		tempy[0] = min_sed - (max_sed - min_sed)/20;
		tempy[1] = max_sed + (max_sed - min_sed)/20;
	}
	analysis_plot->setCurvePen(exclude_singlecurve, Qt::red);
	analysis_plot->setCurveStyle(exclude_singlecurve, QwtCurve::Lines);
	analysis_plot->setCurveData(exclude_singlecurve, tempx, tempy, 2);
	analysis_plot->replot();
	delete [] tempx;
	delete [] tempy;
}

void US_vhwEnhanced::excludeRangeSignal(unsigned int scan)
{
	double *tempx, *tempy, test;
	tempx = new double [2];
	tempy = new double [2];
	test = 1.0/sqrt((double) run_inf.time[selected_cell][selected_lambda][scan]);
	for (unsigned int i=0; i<divisions; i++)
	{
		tempx[0] = test;
		tempx[1] = test;
		tempy[0] = 0;
		tempy[1] = max_sed + max_sed/10;
	}
	analysis_plot->setCurvePen(exclude_rangecurve, Qt::red);
	analysis_plot->setCurveStyle(exclude_rangecurve, QwtCurve::Lines);
	analysis_plot->setCurveData(exclude_rangecurve, tempx, tempy, 2);
	analysis_plot->replot();
	delete [] tempx;
	delete [] tempy;
}

void US_vhwEnhanced::activate_reset()
{
	pb_reset->setEnabled(true);
}

void US_vhwEnhanced::second_plot()
{
	if (step == 0)
	{
		QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n	   Click on \"Load Data\"\n"));
		return;
	}
	QString title;
	title.sprintf(tr("Run ") + run_inf.run_id + tr(": Cell %d (%d nm) - vHW Distribution Plot"), selected_cell+1, run_inf.wavelength[selected_cell][selected_lambda]);
	dis = new Distribution(run_inf.run_id, &divisions, &boundary_fractions, &intercept, htmlDir, title, selected_cell, selected_lambda);
	dis->setCaption(tr("van Holde - Weischet Distribution Plot:"));
	dis->setMinimumSize(560,480);
	dis->setGeometry(0, 0, 560, 480);
	dis->show();
}

void US_vhwEnhanced::save()
{
	if (step == 0)
	{
		QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n	   Click on \"Load Data\"\n"));
		return;
	}
	write_vhw();
	write_dis();
	write_res();
	QPixmap p;
	QString fileName;
	fileName.sprintf(htmlDir + "/vhw_ext_%d%d.", selected_cell + 1, selected_lambda +1);
	p = QPixmap::grabWidget(analysis_plot, 2, 2, analysis_plot->width() - 4, analysis_plot->height() - 4);
//p = QPixmap::grabWidget(analysis_plot, 0, 0, -1, -1);
	pm->save_file(fileName, p);
	fileName.sprintf(htmlDir + "/vhw_edited_%d%d.", selected_cell + 1, selected_lambda +1);
	p = QPixmap::grabWidget(edit_plot, 2, 2, edit_plot->width() - 4, edit_plot->height() - 4);
//p = QPixmap::grabWidget(position_lbl, 0, 0, -1, -1);
	pm->save_file(fileName, p);
}

void US_vhwEnhanced::help()
{
	US_Help *online_help;
	online_help = new US_Help(this);
	online_help->show_help("manual/vhw-enhanced.html");
}

void US_vhwEnhanced::update_divisions(double val)
{
	if (step == 0)
	{
		QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n	   Click on \"Load Data\"\n"));
		return;
	}
	divisions = (unsigned int) val;
	plot_analysis();
}

void US_vhwEnhanced::select_groups()
{
	if (step == 0)
	{
		QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n	   Click on \"Load Data\"\n"));
		return;
	}
	if (groups_on)	// if groups_on is "true" we are in group-selection mode.
	{
		groups_on = false;
		groups = 0;
		analysis_plot->removeMarkers();
		analysis_plot->replot();
		pb_groups->setText(tr("Select Groups"));
	}
	else
	{
		if (plot_analysis() == 0)
		{
			groups_on = true;
			pb_groups->setText(tr("Clear Groups"));
			QMessageBox::message(tr("Attention:\n"),
									  tr("Please click first above and then below the\n"
										 "intercepts on the Y-axis of the van Holde -\n"
										 "Weischet extrapolation plot to define groups\n"
										 "and to average the S-values within a group.\n\n"
										 "PLEASE NOTE:\n"
										 "This algorithm provides for multiple and also\n"
										 "overlapping groupings, such that the total\n"
										 "percentage may exceed 100%."));
		}
		else
		{
			QMessageBox::message(tr("Please note:\n"),tr("The analysis was unsuccessful.\nPlease adjust the parameters and try again!"));
			groups_on = false;
			groups = 0;
			pb_groups->setText(tr("Select Groups"));
			return;
		}
	}
}

void US_vhwEnhanced::getAnalysisPlotMousePressed(const QMouseEvent &e)
{
// store position
	p1 = e.pos();
}

void US_vhwEnhanced::getAnalysisPlotMouseReleased(const QMouseEvent &e)
{
	unsigned int count=0, j;
	float sum=0;
	QString str;

// store position:

	p2 = e.pos();
	if (groups <= divisions)
	{
		if (groups_on)
		{
			if (group_start)
			{
				group_start = false;
				groups++;
				start_y = analysis_plot->invTransform(QwtPlot::yLeft, p2.y());
			}
			else
			{
				group_start = true;
				stop_y = analysis_plot->invTransform(QwtPlot::yLeft, p2.y());
				if (stop_y > start_y)
				{
					QMessageBox::message(tr("Attention:\n"),tr("Please select the upper limit first!"));
					groups--;
				}
				else
				{
					for (j=0; j<divisions; j++)
					{
						if (intercept[j] >= stop_y && intercept[j] <= start_y)
						{
							sum += intercept[j];
							count++;
						}
					}
				/*
				int count2 = 0;
				for (j=0; j<divisions; j++)
				{
					if (intercept[j] >= stop_y && intercept[j] <= start_y)
					{
						count2 ++;
						float arg = 1.0 - (2.0 * (float) count2/ (float) count);
						cout << "Argument: " << arg << ", count2: " << count2 << ", count: " << count <<endl;
						if(arg < 0.0)
						{
							cout << "D: " << pow((double) (run_inf.meniscus[selected_cell] * -slope[j] * omega_s)/(2 * inverse_error_function(fabs(arg), 1e-5)), (double) 2) << endl;
						}
						else
						{
							cout << "D: " << pow((double) (run_inf.meniscus[selected_cell] * slope[j] * omega_s)/(2 * inverse_error_function(arg, 1e-5)), (double) 2) << endl;
						}
					}
				}
				*/
					group_sval[groups-1] = sum/count;
					group_percent[groups-1] = ((float) count/ (float) divisions) * 100;
					if (group_percent[groups-1] != 0)
					{
						str.sprintf(tr("Group %d: %6.2fs (%5.2f"), groups, group_sval[groups-1], group_percent[groups-1]);
						str += "\%)";
						sval_label[groups-1] = analysis_plot->insertMarker();
						analysis_plot->setMarkerLabelAlign(sval_label[groups-1], AlignRight);
						analysis_plot->setMarkerLabel(sval_label[groups-1], str);
						analysis_plot->setMarkerPos(sval_label[groups-1], 0, group_sval[groups-1]);
						analysis_plot->setMarkerFont(sval_label[groups-1], QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2, QFont::Bold));
						analysis_plot->setMarkerPen(sval_label[groups-1], QPen(red, 0, DashDotLine));
						analysis_plot->replot();
					}
					else
					{
						QMessageBox::message(tr("Please note:\n"),tr("There are no divisions between\nthe given limits!"));
						groups--;
					}
				}
			}
		}
	}
	else
	{
		QMessageBox::message(tr("Please note:\n"),tr("The maximum number of groups allowed\n"
														  "has been reached. If you would like to\n"
														  "define different groups, please clear\n"
														  "the currently defined groups first and\n"
														  "then start over."));
	}
}

int US_vhwEnhanced::plot_analysis()
{
	double left;
	if (setup_already)
	{
		cleanup();
	}
	double *tempx, *tempy;

	if (!plateaus_corrected)
	{
		find_plateaus();
	}

	QString s1, s2, s3;
	int first=0;
	max_sed = -1;
	min_sed = 1e6;
	already = true;
	QwtSymbol sym;
	sym.setStyle(QwtSymbol::Ellipse);
	sym.setSize(8);
	if (print_bw)
	{
		sym.setPen(black);
		sym.setBrush(white);
	}
	else
	{
		sym.setPen(blue);
		sym.setBrush(white);
	}
	bool *show_plot;
	show_plot = new bool [divisions]; //set to true if the 
	float correlation_sum;
	unsigned int step=0, i, j, count;
	if (setups() < 0)
	{
		return(-1);
	}
	for (j=0; j<divisions; j++)
	{
		boundary_fractions[j] = 100 * (bd_position/100 + (float) ((j + 1)*bd_range/100) / (float)(divisions + 1));
	}
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		radius_x[i] = 0;
		radius_y[i] = 0;
		for (j=0; j<divisions; j++)
		{
			sed_app[i][j] = 0;
			show_plot[j] = false;
		}
	}
	if (tolerance != 0.0 && !fe_completed)
	{
		float bottom = calc_bottom(rotor_list, cp_list, run_inf.rotor, run_inf.centerpiece[selected_cell], 
				0, run_inf.rpm[selected_cell][0][0]);
		stop_point.clear();
		float Davg = 5e-7, radD;
		for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
		{
// tolerance is defined as 2 * the concentration gradient divided by the initial concentration
// a smaller value means greater sensitivity, because the gradient (the slope) is smaller
/*
// this is the old version of calculating diffusion radii based on free-flow diffusion equation (Fick's law):

			radD = bottom - pow ((double) (log(pow ((double) (M_PI * Davg * run_inf.time[selected_cell][selected_lambda][i]), (double) 0.5)
				 * tolerance) * Davg * run_inf.time[selected_cell][selected_lambda][i] * -4), (double) 0.5);
			radius_x[i] = radD;
*/

// use D=5e-7 and s=3e-13 as a first approximation for finding the back-diffusion radius point:

			left = tolerance * pow((double) Davg, 0.5)/(2 * 3e-13 * omega_s * (bottom + run_inf.meniscus[selected_cell])/2 
			* pow((double) run_inf.time[selected_cell][selected_lambda][i], 0.5));
			if (run_inf.time[selected_cell][selected_lambda][i] == 0)
			{
				update_single((double) i+1);
				ex_single(); // delete the first scan if the data were simulated with 0 minutes time delay for first scan
				return (-2);
			}
//cout << "left1: " << left << ", Davg: " << Davg << ", bottom: " << bottom << ", time: " << run_inf.time[selected_cell][selected_lambda][i] << endl;
			radD = bottom - (2 * find_root(left) * pow((double) (Davg * run_inf.time[selected_cell][selected_lambda][i]), 0.5));
			radius_x[i] = radD;
//cout << "radD1: " << radD << endl;

			j = 0;
			while (radius[j] < radD && j < points-1) // find the last radius index suitable for inclusion
			{
				j++;
			}
			stop_point.push_back(j);
			if (absorbance[i][j] > run_inf.plateau[selected_cell][selected_lambda][i])
			{
				radius_y[i] = run_inf.plateau[selected_cell][selected_lambda][i];
			}
			else
			{
				radius_y[i] = absorbance[i][j];
			}
//			cout << radius_x[i] << "\t" << radius_y[i] << "\t" << absorbance[i][j] << endl;
//			cout << i << ": stop: " << stop_point[i] << " (" << radius[stop_point[i]] << "), radD: " 
//			<< radD <<  ", stopradius-dr: " << radius[stop_point[i]]-radD << endl;
		}
	}
	if (tolerance == 0.0 && !fe_completed)
	{
		for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
		{
			stop_point[i] = points-1;
		}		
	}
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		offset[i] = run_inf.plateau[selected_cell][selected_lambda][i] * bd_position/100;
//		cout << run_inf.plateau[selected_cell][selected_lambda][i] << endl;
	}		
	for (i=0; i<divisions; i++)
	{
		if (calc_sed(i) < 0)
		{
//			cout << "calc_sed of division " << i << " failed with value: " << calc_sed(i) << endl;
			return(-1);
		}
//		cout << "calc_sed of division " << i << " is: " << calc_sed(i) << endl;
		step++;
		progress->setProgress(step);
	}
	if (exclude_single == 0.0 || exclude_range == 0.0)
	{
		analysis_plot->removeCurve(exclude_singlecurve);
		analysis_plot->removeCurve(exclude_rangecurve);
	}
	analysis_plot->clear();
	s1.sprintf(tr(": Cell %d (%d nm) - vHW Extrapolation Plot"), selected_cell+1, run_inf.wavelength[selected_cell][selected_lambda]);
	s2 = tr("Run ");
	s2.append(run_inf.run_id);
	s2.append(s1);
	analysis_plot->setTitle(s2);
	analysis_plot->setAxisTitle(QwtPlot::xBottom, tr("(Time)^-0.5"));
	analysis_plot->setAxisTitle(QwtPlot::yLeft, tr("Corrected Sed. Coeff. (1e-13 s)"));
	correlation_sum  =  0;
	for (j=0; j<divisions; j++)
	{
		for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
		{
			sed_app[i][j] *= correction * 1e13;
			max_sed = (float) max((float) max_sed, (float) sed_app[i][j]);
			if (sed_app[i][j] != 0.0)
			{
				min_sed = (float) min((float) min_sed, (float) sed_app[i][j]);
			}
		}
	}
//cout << "Total divisions: " << divisions << endl;
	for (j=0; j<divisions; j++)
	{
// find out which scan is the first scan with nonzero entries
		count = 0;
		for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
		{
			if(sed_app[i][j] != 0)
			{
				count ++;
			}
			if (count > 0)
			{
				first = i;
				break;
			}
		}
// find out how many nonzero entries there are in sed_app:
		count = 0;
		for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
		{
			if(sed_app[i][j] != 0)
			{
				count ++;
			}
		}
		tempx = new double [count];
		tempy = new double [count];
		count = 0;
// assign all apparent sedimentation coefficients for a single division to a temporary set of variables:
		for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
		{
		
			if(sed_app[i][j] != 0)
			{
				tempx[count] = 1.0/sqrt((double) run_inf.time[selected_cell][selected_lambda][i]);
				tempy[count] = sed_app[i][j];
				count ++;
			}
		}
		if(count < 2) // we need at least two points to make a line
		{
			QString temp_str;
			temp_str.sprintf("There is insufficient data to create an extrapolation\n"
									"for division %d.\n", j+1);
			
			QMessageBox::message(tr("Warning:"),
			tr(temp_str + "Try to reduce the number of divisions and the percentage\n"
								"of analyzed boundary. Also, shifting the baseline position\n"
								"may help."));
			return (-1);
		}
		show_plot[j] = true;
		linefit(&tempx, &tempy, &slope[j], &intercept[j], &sigma[j], &correl[j], count);
		correlation_sum += fabs(correl[j]); 
//cout << "j: " << j << ", correlation: " << correl[j] << ", intercept: " << intercept[j] << endl;
		sa_points[j] = analysis_plot->insertCurve("Apparent Sedimentation Coefficients");
		analysis_plot->setCurveStyle(sa_points[j], QwtCurve::NoCurve);
		analysis_plot->setCurveSymbol(sa_points[j], QwtSymbol(sym));
		analysis_plot->setCurveData(sa_points[j], tempx, tempy, count);
		delete [] tempx;
		delete [] tempy;
	}
	float max_intercept = -1e6, min_intercept = 1e6, sed_bin;
	int *bin_count;
	bin_count = new int [divisions];
	average_s = 0;
	for (j=0; j<divisions; j++)
	{
		max_intercept = max((float) max_intercept, (float) intercept[j]);
		min_intercept = min((float) min_intercept, (float) intercept[j]);
		bin_count[j] = 0;
		average_s += intercept[j];
	}
	average_s = average_s/divisions;
	sed_bin = (max_intercept - min_intercept)/divisions;
	for (j=0; j<divisions; j++)
	{
		for (k=0; k<divisions; k++)
		{
			if ((intercept[j] >= min_intercept + k * sed_bin) && (intercept[j] < min_intercept + (1 + k) * sed_bin))
			{
				bin_count[k] ++;
			}
		}
	}
	fe1.sed.clear();
	fe1.diff.clear();
	fe1.conc.clear();
	fe1.times.clear();
	fe1.radius.clear();
	fe2.sed.clear();
	fe2.diff.clear();
	fe2.conc.clear();
	fe2.times.clear();
	fe2.radius.clear();
	float diff, sed, conc;
// to get the diffusion coeficient from the sedimentation coefficient
// when the shape is assumed to be a sphere (N is Avogadro's number, and 
// R is the gas constant, and T is the temperature in Kelvin:
// (1) M = vbar * 4/3 * M_PI * r^3
// (2) s = M * (1 - vbar * rho)/N * f
// (3) f = 6 * M_PI * eta * r
// (4) D = R * T/N * f
// substitute (1) into (2), substitute (3) into (2), solve for r from (2), solve for f in (3), substitute
// (3) into (4) to solve for D.
	for (j=0; j<divisions; j++)
	{
		if (bin_count[j] > 0)
		{
			sed = 1e-13 * (min_intercept + (1 + j) * sed_bin - sed_bin/2);
			diff = (R * (K0 + run_inf.avg_temperature))/(AVOGADRO * 0.06 * M_PI * viscosity_tb 
			* pow((double) (0.045 * sed * vbar * viscosity_tb/(1 - vbar * density_tb)), (double) 0.5));
			conc = bin_count[j] * initial_concentration/divisions;
			fe1.diff.push_back(diff);
			fe1.sed.push_back(sed);
			fe1.conc.push_back(conc);
			fe2.diff.push_back(diff);
			fe2.sed.push_back(sed);
			fe2.conc.push_back(conc);
		}
	}
	pb_cofs->setEnabled(true);
	pb_sa2d->setEnabled(true);
	for (j=0; j<divisions; j++)
	{
//cout << "Division: " << j << ", show_plot: " << show_plot[j] << endl;
		if (show_plot[j])
		{
			sa_lines[j] = analysis_plot->insertCurve("Sedimentation Extrapolations");
			analysis_plot->setCurveStyle(sa_lines[j], QwtCurve::Lines);
			lines_x[0] = 0.0;
			lines_x[1] = 1.0/sqrt((double) run_inf.time[selected_cell][selected_lambda][first]) + 1.0
				/ (20.0 * sqrt((double) run_inf.time[selected_cell][selected_lambda][first]));
			if (lines_x[1] <0)
			{
				QMessageBox::message(tr("Warning:"),tr("This data appears to have been edited with the\n"
																  "incorrect setting for the Beckman Data Acquisition\n"
																  "Time-Bug. When editing the data for this run, the\n"
																  "setting for this switch should be \"off\"\n\n"
																  "Please re-edit the data and try again..."));
			}            
			lines_y[0] = intercept[j];
			lines_y[1] = slope[j] * lines_x[1] + lines_y[0];
//cout << "Line 1: " << lines_y[0] << "Line 2: " << lines_y[1] << endl;
			if (print_bw)
			{
				analysis_plot->setCurvePen(sa_lines[j], white);
			}
			else
			{
				analysis_plot->setCurvePen(sa_lines[j], yellow);
			}
			analysis_plot->setCurveData(sa_lines[j], lines_x, lines_y, 2);
		}
	}
	unsigned int diff_plot;
	QPen diff_pen;
	diff_pen.setWidth(3);
	diff_pen.setColor(red);
	diff_plot = edit_plot->insertCurve("Diffusion Boundary");
	edit_plot->setCurveStyle(diff_plot, QwtCurve::Lines);
	edit_plot->setCurvePen(diff_plot, diff_pen);
	edit_plot->setCurveData(diff_plot, radius_x, radius_y, run_inf.scans[selected_cell][selected_lambda]);
	edit_plot->replot();
	analysis_plot->replot();
	delete [] show_plot;
	delete [] bin_count;
	exclude_singlecurve = analysis_plot->insertCurve("Exclusion Single Candidate");
	exclude_rangecurve = analysis_plot->insertCurve("Exclusion Range Candidate");
	return(0);
}

int US_vhwEnhanced::calc_sed(unsigned int div)
{
//cout << "INitC: " << initial_concentration << endl;
// cout << "Division: " << div << endl;
	unsigned int i, j, count, iter;
	float testy, span, temp_r=0.0, a, b, force, diff, offset_correction, ratio;
	double *tempx, *tempy, radD=0.0, left;
	float bottom = calc_bottom(rotor_list, cp_list, run_inf.rotor, run_inf.centerpiece[selected_cell], 
						0, run_inf.rpm[selected_cell][0][0]);

	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		span = run_inf.plateau[selected_cell][selected_lambda][i] * bd_range/100;
//cout << "Plateau[" << i << "]: " << run_inf.plateau[selected_cell][selected_lambda][i] << ", span: " << span << endl;
//cout << i << "\t" << run_inf.plateau[selected_cell][selected_lambda][i] << endl;
		increment[i] = span/(divisions + 1);
//cout << "Increment top[" << i << "]: " << increment[i] << ", division: " << div << ", plateau: " << run_inf.plateau[selected_cell][selected_lambda][i] << endl;
	}

//
// the first approximation loop runs over all scans for this division, assuming the 
// partial concentration equivalent to the interval between last baseline and plateau for
// each scan. All values found are extrapolated to infinite time for a first approximation 
// of this division's s-value.
//
	for (iter=0; iter<1; iter++) // refine each division 3 times for final answer
	{
		j = 0;
//
// start with the previous division's baseline (offset[i] - which already has been plateau corrected in the previous division)
// and add the division increment:
//
		while (absorbance[j][0] > (increment[j] + offset[j])) // if the first absorbance point of a scan 
		{                                                     // is larger than our test point, it is skipped
			j++;
			if (j == run_inf.scans[selected_cell][selected_lambda])
			{
				QString temp_str;
				temp_str.sprintf("There is insufficient data to create an extrapolation\n"
										"for division %d.\nThe increment is %f and the offset is "
										"%f concentration units\n", div+1, increment[j], offset[j]);
				QMessageBox::message(tr("Warning:"),
				tr(temp_str + "Try to reduce the number of divisions and the percentage\n"
									"of analyzed boundary. Also, shifting the baseline position\n"
									"may help."));
				return(-1);
			}
		}
		for (i=j; i<run_inf.scans[selected_cell][selected_lambda]; i++)
		{
			count = 0;
			testy = increment[i] + offset[i];
			sed_app[i][div] = -1.0; // set to minus 1 as a flag. If it is set to zero below, we want to skip later steps.

//if (div == 15)
//{
//cout << "div: " << div << ", testy: " << testy << ", increment: " << increment[i] << ", offset: " << offset[i] << 
//", stop point: " << stop_point[i] << ", absorbance: " << absorbance[i][count] << ", points: " << points <<  endl;
//}

			if(absorbance[i][0] > testy)	// sometimes the first datapoint is higher due to noise, and we should really 
			{										// search from top down to find the first lowest point
				count = points - 1;
				while (absorbance[i][count] > testy && count > 0)
				{
					count --;
				}
				if (count == 0 && absorbance[i][0] > testy) // we didn't find a point, set sed_app to zero
				{
					sed_app[i][div] = 0;
				}
			}
			else
			{
				while ((count < stop_point[i]) && (count < points - 1) && (absorbance[i][count] < testy))
				{
					count++;		// climb up the concentration gradient until one point beyond testy is reached
				}
			}
/*
if (div == 15)
{
//cout << "count: " << count << ", points: " << points << ", radius: " << radius[count] << ", abs: " << absorbance[i][count] << ", stop: " << stop_point[i] <<endl;
}
*/
//
// Now we need a linear interpolation between the last point before and the 
// first point beyond testy. The radius corresponding to the interpolated 
// concentration of testy on that line is the radius needed for the calculation
// of the apparent sedimentation coefficient. 
//
//
// Note: sometimes the second or third scan after the first usable scan that has a first absorbance
// point lower than the testy value has again a higher absorbance value and needs to be excluded, so we
// will set its sed_app to zero as well (3. condition):

			if (count >= stop_point[i] || count >= points - 1)
			{
				sed_app[i][div] = 0;  // if the sedimentation coefficient is zero, it is ignored in the extrapolation
//cout << "Stop_point: " << stop_point[i] << ", count: " << count << ", i: " << i << ", testy: " << testy << ", absorbance: " << absorbance[i][count] << endl;
			}
/*
*/
//
// If the test concentration is equal
// to a point in the concentration matrix, we don't have to interpolate and can
// use the point count instead to obtain the correct radius:
//
			else if (absorbance[i][count] == testy)
			{
				temp_r = radius[count];
			}
//cout << "scan: " << i << ", count: " << count << ", division: " << div << endl;
//
// If the test concentration is equal on the two bordering points, we use the 
// half-way point to get the radius:
//
			else if (count > 0 && absorbance[i][count] == absorbance[i][count-1])
			{
				temp_r = radius[count] - run_inf.delta_r/2.0;
//cout << "i: " << i << ", temp_r: " << temp_r << endl;
			}

// otherwise make a linear interpolation:

			else	if (sed_app[i][div] != 0)// solve: y = a*x + b:	
			{
//				cout << "Count: " << count << ", i: " << i << ", delta_R: " << run_inf.delta_r << endl;
				a = (absorbance[i][count] - absorbance[i][count-1])/(run_inf.delta_r);
				b = absorbance[i][count] - a * radius[count];
				temp_r = (testy - b)/a;
			}
			if (sed_app[i][div] < 0) // if sed_app has not been set to zero above, we can proceed....
			{
//				force = omega_s * run_inf.time[selected_cell][selected_lambda][i];
				force = run_inf.omega_s_t[selected_cell][selected_lambda][i];
				if(temp_r <= 0) // for very steep boundaries we may not have enough points, and one division may be less than the absorbance difference between two points
				{ // therefore we will fit to a straight line using the last point and the current, and the next 2 
					tempx = new double [4];
					tempy = new double [4];
					for (k=0; k<4; k++)
					{
						tempx[k] = radius[count - 1 + k];
						tempy[k] = absorbance[i][count - 1 + k];
					}
					linefit(&tempx, &tempy, &slope[div], &intercept[div], &sigma[div], &correl[div], 4);
					temp_r = (testy - intercept[div])/slope[div];
					delete [] tempx;
					delete [] tempy;
				}
				if (run_inf.meniscus[selected_cell] <=0  || force <=0)
				{
					QMessageBox::message( tr("Warning"), tr("Attention:\n\n"
																	  "The van Holde - Weischet analysis encountered\n"
																	  "a mathematical exception. This is generally caused\n"
																	  "by ill-conditioned data. \n\n"
																	  "Suggestion: Try to increase the smoothing level\n"
																	  "for this cell.\n\n"
																	  "Alternatives: Re-edit the data or exclude the data\n"
																	  "from this cell during the editing."));
					return(-1);	
				}
				sed_app[i][div] = log(temp_r/run_inf.meniscus[selected_cell]) / force;
//	cout << "i: " << i << ", div: " << div << ", temp_r: "<< temp_r << ", force " << force << ", sed_app: " << sed_app[i][div] << endl;
/*
if (sed_app[i][div] <= 0)
{
	cout << "i: " << i << ", div: " << div << ", temp_r: "<< temp_r << ", force " << force << ", sed_app: " << sed_app[i][div] << endl;
}
*/
				if (sed_app[i][div] <= 0)  // ignore negative s values in this version
				{
//	cout << "i: " << i << ", div: " << div << ", temp_r: "<< temp_r << ", force " << force << ", sed_app: " << sed_app[i][div] << endl;
					sed_app[i][div] = 0;
				}
			}
		}
		count = 0;
		for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
		{
			if(sed_app[i][div] != 0)
			{
				count ++;
			}
//cout << "Scan " << i << ": " << count << ", div: " << div << endl;
		}
		tempx = new double [count];
		tempy = new double [count];
		count = 0;
		for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
		{
			if(sed_app[i][div] != 0)
			{
				tempx[count] = 1.0/sqrt((double) run_inf.time[selected_cell][selected_lambda][i]);
				tempy[count] = sed_app[i][div];
				count ++;
			}
/*
			else
			{
				cout << "count: " << count << ", for scan " << i << " and div: " << div << ", sed: " << sed_app[i][div] << endl;
			}
*/
		}
		if(count < 2) // we need at least two points to make a line
		{
			QString temp_str;
			temp_str.sprintf("There is insufficient data to create an extrapolation\n"
									"for division %d.\n(%d points for the extrapolation)\n ", div+1, count);
			
			QMessageBox::message(tr("Warning:"),
			tr(temp_str + "Try to reduce the number of divisions and the percentage\n"
								"of analyzed boundary. Also, increasing the backdiffusion\n"
								"tolerance and shifting the baseline position may help."));
			return(-1);
		}
		linefit(&tempx, &tempy, &slope[0], &intercept[0], &sigma[0], &correl[0], count);
		delete [] tempx;
		delete [] tempy;
//
// refine/reassign the increment value based on the radial dilution equation and 
// the last s-value calculated for this division:
//
		for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
		{
			offset_correction = increment[i]; // save old value
			for (int k=0; k<1; k++)
			{
				increment[i] = (initial_concentration * bd_range/100)/(divisions+1)
				* exp(-2.0 * intercept[0] * omega_s * run_inf.time[selected_cell][selected_lambda][i]);
				ratio = offset_correction/increment[i];
//cout << "offset: " << offset_correction << ", increment[" << i << "]: " << increment[i] << ", init conc: " << initial_concentration << ", iteration: " << iter << ", intercept: " << intercept[0] << endl;
			}
		}
		if (div == 0) 	// the first division has the smallest sedimentation coefficient and the largest diffusion coefficient
		{					// use it for determining the back diffusion point, assuming the particle has a f/f0 ration of 1.0
			diff = (R * (K0 + run_inf.avg_temperature))/(AVOGADRO * 0.06 * M_PI * viscosity_tb 
			* pow((double) (0.045 * intercept[0] * vbar * viscosity_tb/(1.0 - vbar * density_tb)), (double) 0.5));
			/*
cout 	<< "Temp: " <<  run_inf.avg_temperature
		<< ", visc: " <<  viscosity_tb
		<< ", icpt: " << intercept[0]
		<< ", dens: " << density_tb 
		<< ", k0: " << K0
		<< ", vbar: " << vbar << endl;
			*/
			for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
			{
// tolerance is defined as 2 * the concentration gradient divided by the initial concentration
// a smaller value means greater sensitivity, because the gradient (the slope) is smaller

/*
// this is the old version of calculating diffusion radii based on free-flow diffusion equation (Fick's law):

				radD = bottom - pow ((double) (log(pow ((double) (M_PI * diff * run_inf.time[selected_cell][selected_lambda][i]), (double) 0.5)
					 * tolerance) * diff * run_inf.time[selected_cell][selected_lambda][i] * -4), (double) 0.5);
*/
				if (intercept[0] > 0.0)
				{
					left = tolerance * pow((double) diff, 0.5)/(2 * intercept[0] * omega_s * (bottom + run_inf.meniscus[selected_cell])/2 
					* pow((double) run_inf.time[selected_cell][selected_lambda][i], 0.5));
// cout << "left2: " << left << ", intercept: " << intercept[0] << ", diff: " << diff << endl;

					radD = bottom - (2 * find_root(left) * pow((double) (diff * run_inf.time[selected_cell][selected_lambda][i]), 0.5));
//cout << "radD2: " << radD << endl;

					radius_x[i] = radD;
				}
				else
				{
					radius_x[i] = radius[points-1];
				}
				j = 0;
				while (radius[j] < radD && j < points-1) // find the last radius index suitable for inclusion
				{
					j++;
				}
				radius_y[i] = absorbance[i][j];
				stop_point[i] = j;
			}
		}
	}
// finally add the new increment to the offset:

	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		offset[i] += increment[i];
//		cout << "Division: " << div << ", Offset: " << offset[i] << ", increment: " << increment[i] << ", plateau: " << run_inf.plateau[selected_cell][selected_lambda][i] << "\n";
	}
	return(0);
}

void US_vhwEnhanced::update_tolerance(double val)
{
	tolerance = val;
	fe_completed = false; // whenever the tolerance is changed the FE simulation has to be redone
	plot_analysis();
}

int US_vhwEnhanced::setups()
{
	if ((run_inf.scans[selected_cell][selected_lambda]) < 3)
	{
		QMessageBox::message( tr("Warning"), tr("Attention:\n\n"
														  "Not enough scans have cleared the meniscus. In order to perform a\n"
														  "meaningful van Holde - Weischet Analysis, you need at least 3 scans."));
		return (-1);
	}
	QString str;
	group_sval = new float [divisions];
	group_percent = new float [divisions];
	sval_label = new uint [divisions];
	progress->setTotalSteps(divisions);
	progress->reset();
	progress->setProgress(0);
	sed_app = new double* [run_inf.scans[selected_cell][selected_lambda]];  // first dimension for scans
	offset = new float [run_inf.scans[selected_cell][selected_lambda]];
	increment = new float [run_inf.scans[selected_cell][selected_lambda]];
	radius_x = new double [run_inf.scans[selected_cell][selected_lambda]];  // diffusion boundary x
	radius_y = new double [run_inf.scans[selected_cell][selected_lambda]];  // diffusion boundary y
	for (i=0; i < (run_inf.scans[selected_cell][selected_lambda]); i++)
	{
		sed_app[i] = new double [divisions]; // second dimension for divisions
	}
	old_local_scans = run_inf.scans[selected_cell][selected_lambda];
	slope = new double [divisions];
	intercept = new double [divisions];
	sigma = new double [divisions];
	correl = new double [divisions];
	boundary_fractions = new double [divisions];
	sa_points = new uint [divisions];
	sa_lines = new uint [divisions];
	lines_y = new double [2];
	lines_x = new double [2];
	setup_already = true;
	return (0);
}

void US_vhwEnhanced::cleanup()
{
	delete [] group_sval;
	delete [] group_percent;
	delete [] sval_label;
	for (i=0; i < old_local_scans; i++)
	{
		delete [] sed_app[i]; // second dimension for divisions
	}
	delete [] sed_app; // first dimension for scans
	delete [] increment;
	delete [] offset;
	delete [] slope;
	delete [] intercept;
	delete [] sigma;
	delete [] correl;
	delete [] boundary_fractions;
	delete [] sa_points;
	delete [] sa_lines;
	delete [] lines_y;
	delete [] lines_x;
	delete [] radius_y;
	delete [] radius_x;
	setup_already = false;
}

void US_vhwEnhanced::sa2d()
{
	unsigned int i;
	double max_intercept = -1e6, min_intercept = 1e6;
	for (i=0; i<divisions; i++)
	{
		max_intercept = max((float) max_intercept, (float) intercept[i]);
		min_intercept = min((float) min_intercept, (float) intercept[i]);
	}
	max_intercept += max_intercept/4.0; 
	min_intercept -= min_intercept/4.0;
	if (min_intercept < 0.01)
	{
		min_intercept = 0.01;
	}
	if (min_intercept <= 0.001)
	{
		min_intercept = 0.001;
	} 
	US_SA2D_W *sa2d_w;
	sa2d_w = new US_SA2D_W(min_intercept, max_intercept, fn, selected_cell, selected_lambda, 0, 0);
	sa2d_w->show();
}


void US_vhwEnhanced::cofs()
{
	unsigned int i;
	double max_intercept = -1e6, min_intercept = 1e6;
	for (i=0; i<divisions; i++)
	{
		max_intercept = max((float) max_intercept, (float) intercept[i]);
		min_intercept = min((float) min_intercept, (float) intercept[i]);
	}
	max_intercept += max_intercept/4.0; 
	min_intercept -= min_intercept/4.0;
	if (min_intercept < 0.01)
	{
		min_intercept = 0.01;
	}
	if (min_intercept <= 0.001)
	{
		min_intercept = 0.001;
	} 
	US_CofS_W *cofs_w;
	cofs_w = new US_CofS_W(min_intercept, max_intercept, fn, selected_cell, selected_lambda, 0, 0);
	cofs_w->show();
}

/*




	fe1.delta_r = 0.001;
	fe1.delta_t = 20;
	fe1.rpm = run_inf.rpm[first_cell][0][0];
	fe1.meniscus = run_inf.meniscus[selected_cell];
	fe1.bottom = calc_bottom(rotor_list, cp_list, run_inf.rotor, run_inf.centerpiece[selected_cell], 
				0, run_inf.rpm[selected_cell][0][0]);
	fe1.points = (unsigned int) ((fe1.bottom - fe1.meniscus)/fe1.delta_r + 1);
	fe2.delta_r = 0.001;
	fe2.delta_t = 20;
	fe2.rpm = run_inf.rpm[first_cell][0][0];
	fe2.meniscus = run_inf.meniscus[selected_cell];
	fe2.bottom = 9;
	fe2.points = (unsigned int) ((fe2.bottom - fe2.meniscus)/fe2.delta_r + 1);
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		fe1.times.push_back(run_inf.time[selected_cell][selected_lambda][i]);
		fe2.times.push_back(run_inf.time[selected_cell][selected_lambda][i]);
	}
	for (i=0; i<fe1.points; i++)
	{
		fe1.radius.push_back(fe1.meniscus + i * fe1.delta_r);
	}
	for (i=0; i<fe2.points; i++)
	{
		fe2.radius.push_back(fe2.meniscus + i * fe2.delta_r);
	}
	fe1.absorbance = new float * [run_inf.scans[selected_cell][selected_lambda]];
	fe2.absorbance = new float * [run_inf.scans[selected_cell][selected_lambda]];
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		fe1.absorbance[i] = new float [fe1.points];
		fe2.absorbance[i] = new float [fe2.points];
	}

	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		for (j=0; j<fe1.points; j++)
		{
			fe1.absorbance[i][j] = 0.0;
			fe2.absorbance[i][j] = 0.0;
		}
	}

	progress->setTotalSteps(2 * fe1.sed.size());
	progress->reset();
	progress_counter = 0;
	progress->setProgress(progress_counter);
	non_interacting(&fe1);
	non_interacting(&fe2);
	stop_point.clear();

	float Davg=0, radD;
	
	for (i=0; i<fe1.diff.size(); i++)
	{
		Davg += fe1.diff[i];
	}
	Davg = Davg/divisions;
//	reset();
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{

		radD = fe1.bottom - pow ((double) (log(pow ((double) (M_PI * Davg * run_inf.time[selected_cell][selected_lambda][i]), (double) 0.5)
			 * tolerance) * Davg * run_inf.time[selected_cell][selected_lambda][i] * -4), (double) 0.5);

		j = 0;
		while ((fe1.absorbance[i][j] - fe2.absorbance[i][j]) < tolerance/100 && j<fe1.points)
		{
			j++;
		}
		stop_point.push_back(j);
//		cout << i << ": FEstop: " << stop_point[i] << " (" << radius[stop_point[i]] << "), radD: " 
//		<< radD <<  ", stopradius-radD: " << radius[stop_point[i]]-radD << endl;
	}
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		delete [] fe1.absorbance[i];
		delete [] fe2.absorbance[i];
	}
	delete [] fe1.absorbance;
	delete [] fe2.absorbance;
	fe_completed = true;
	plot_analysis();
}
*/
void US_vhwEnhanced::non_interacting(struct fe *temp_fe)
{
	unsigned int i, j, k, iterations, scan=0;
	float scan_timer, temp1, temp2;
	bool ldu_flag;
	iterations = (unsigned int) (0.5 + ((*temp_fe).times[(*temp_fe).times.size() - 1]/(*temp_fe).delta_t));
	init_fe(temp_fe);
	left_temp = new double* [(*temp_fe).points];
	for (i=0; i<(*temp_fe).points; i++)
	{
		left_temp[i] = new double [3];
	}
	right_old = new double [(*temp_fe).points];
	for (k=0; k<(*temp_fe).sed.size(); k++)
	{
//cout << k << " (" << (*temp_fe).sed.size() << "): s=" << (*temp_fe).sed[k] << ", D=" 
//<< (*temp_fe).diff[k] << ", c=" << (*temp_fe).conc[k] << endl;
		ldu_flag = true;
		scan_timer = 0.0;
		scan = 0;
		temp1 = (*temp_fe).delta_t * (*temp_fe).sed[k] * omega_s;
		temp2 = (*temp_fe).delta_t * (*temp_fe).diff[k];
		for (i=0; i<(*temp_fe).points; i++)
		{
			right[i]=(*temp_fe).conc[k];
			for (j=0; j<3; j++)
			{
				left[i][j] = temp2 * a1[i][j] - temp1 * a2[i][j];
			}
		}
		for (i=0; i<(*temp_fe).points; i++)
		{
			for (j=0; j<3; j++)
			{
				left_temp[i][j] = b_orig[i][j] + 0.5 * left[i][j];
			}
		}

		for (j=0; j<iterations+1; j++)
		{
			for (i=0; i<(*temp_fe).points; i++)
			{
				right_old[i] = right[i];
			}
			// Crank-Nicholson:

			m3vm_b(&b_orig, &left, &right, (*temp_fe).points, -0.5); // adds B to left and divides each element of A by 2 before multiplying with right[i]
			ldu(&left_temp, &right, (*temp_fe).points, 3, ldu_flag); // end Corrector
			ldu_flag = false;
			if (fabs((*temp_fe).times[scan] - scan_timer) <= ((*temp_fe).delta_t/2.0))
			{
				for (i=0; i<(*temp_fe).points; i++)
				{
					(*temp_fe).absorbance[scan][i] += right[i];
				}
				scan++;
			}
			scan_timer += (*temp_fe).delta_t;
			qApp->processEvents();
		}
		progress_counter++;
		progress->setProgress(progress_counter);
	}
	for (i=0; i<(*temp_fe).points; i++)
	{
		delete [] left_temp[i];
		delete [] a1[i];
		delete [] a2[i];
		delete [] b[i];
		delete [] b_orig[i];
		delete [] left[i];
	}
	delete [] left_temp;
	delete [] right_old;
	delete [] right;
	delete [] a1;
	delete [] a2;
	delete [] b;
	delete [] b_orig;
	delete [] left;
}

void US_vhwEnhanced::init_fe(struct fe *temp_fe)
{
	unsigned int i,j;
	a1 = new double* [(*temp_fe).points];
	a2 = new double* [(*temp_fe).points];
	b = new double* [(*temp_fe).points];
	left = new double* [(*temp_fe).points];
	right = new double [(*temp_fe).points];
	for (i=0; i<(*temp_fe).points; i++)
	{
		a1[i] = new double [3];
		a2[i] = new double [3];
		b[i] = new double [3];
		left[i] = new double [3];
	}
	a1[0][1] = ((*temp_fe).radius[0]/(*temp_fe).delta_r) + 0.5;
	a1[0][0] = 0.0;
	for (i=1; i<(*temp_fe).points; i++)
	{
		a1[i][0] = (-((*temp_fe).radius[i])/(*temp_fe).delta_r) + 0.5;
		a1[i][1] = 2.0 * (*temp_fe).radius[i]/(*temp_fe).delta_r;
		a1[i-1][2] = a1[i][0];
	}
	a1[(*temp_fe).points-1][1] = ((*temp_fe).radius[(*temp_fe).points-1]/(*temp_fe).delta_r) - 0.5;
	a1[(*temp_fe).points-1][2] = 0.0;
	double delta2 = square((*temp_fe).delta_r);
	a2[0][1] = -1 * ((*temp_fe).radius[0] * (*temp_fe).radius[0])/2.0 - (*temp_fe).radius[0] * ((*temp_fe).delta_r/3.0) - delta2/12.0;
	a2[0][0] = 0.0;
	for (i=1; i<(*temp_fe).points; i++)
	{
		a2[i][0] = ((*temp_fe).radius[i] * (*temp_fe).radius[i])/2.0 - 2.0 * (*temp_fe).radius[i] * ((*temp_fe).delta_r/3.0) + delta2/4.0;
		a2[i][1] = -2 * (*temp_fe).radius[i] * (*temp_fe).delta_r/3.0;
		a2[i-1][2]= -1 * ((*temp_fe).radius[i-1] * (*temp_fe).radius[i-1])/2.0 - 2.0 * (*temp_fe).radius[i-1] * ((*temp_fe).delta_r/3.0) - delta2/4.0;
	}
	a2[(*temp_fe).points-1][1] = ((*temp_fe).radius[(*temp_fe).points-1] * (*temp_fe).radius[(*temp_fe).points-1])/2.0 
	- (*temp_fe).radius[(*temp_fe).points-1] * ((*temp_fe).delta_r/3.0) + delta2/12.0;
	a2[(*temp_fe).points-1][2] = 0.0;
	b[0][0] = 0.0;
	b[0][1] = (*temp_fe).radius[0] * ((*temp_fe).delta_r/3.0) + delta2/12.0;
	for (i=1; i<(*temp_fe).points; i++)
	{
		b[i][0] = (*temp_fe).radius[i] * ((*temp_fe).delta_r/6.0) - delta2/12.0;
		b[i][1] = 2.0 * (*temp_fe).radius[i]*((*temp_fe).delta_r/3.0);
		b[i-1][2] = b[i][0];
	}
	b[(*temp_fe).points-1][1] = (*temp_fe).radius[(*temp_fe).points-1] * ((*temp_fe).delta_r/3.0) - delta2/12.0;
	b[(*temp_fe).points-1][2] = 0.0;
	b_orig = new double* [(*temp_fe).points];
	for (i=0; i<(*temp_fe).points; i++)
	{
		b_orig[i] = new double [3];
	}
	for (i=0; i<(*temp_fe).points; i++)
	{
		for (j=0; j<3; j++)
		{
			b_orig[i][j] = b[i][j];
		}
	}
}

void US_vhwEnhanced::view()
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
	filestr.append(".vhw_res");
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

double US_vhwEnhanced::find_root(double goal)
{
// cout << "Goal: " << goal << endl;
   double test=0, x1=0, x2=0, x=0, xdiff=0, tolerance=1.0e-7, xsqr;
   x1 = 0;  // lower range limit
   x2 = 10; // upper range limit
   x = 5;  // half-way point
   xdiff = 2.5;
//
// Does the y-value fall above or below our goal?
//
   xsqr = x * x;
// the inverse complementary error function, test(x)=ierfc(x)=1/pi^0.5*e(-x^2) - x * erfc(x)
   test = exp(-xsqr) * pow(M_PI, -0.5) - (x * erfc(x));
//cout << "test: " << test << ", goal: " << goal << endl;
//
// iterate until the difference between subsequent x value evaluations is too small to be relevant
//
   while (fabs(test-goal) > tolerance)
   {
//
// Note: this test is only valid for monotone functions!
// If the difference between the goal and the testvalue is negative, then
// the x value was too high. We then adjust the top (x2) limit:
//
      xdiff = (x2 - x1)/2.0;
      if (test < goal)
      {
         x2 = x;
			x -= xdiff;
      }
//
// Otherwise, we adjust the lower limit:
//
      else
      {
         x1 = x;
			x += xdiff;
      }
//
// We adjust the new test x-value by adding half the stepsize from the last step to the lower limit:
//
//      x = x1 + xdiff;

//
// then we update the test y-value:
//
      xsqr = x * x;
      test = (1 + 2 * xsqr) * erfc(x) - (2 * x * exp(-xsqr))/pow(M_PI, 0.5);
//    cout << "2 goal: " << goal << ", test: " << test << ", x: " << x << endl;
   }
   return (x);
}

void US_vhwEnhanced::write_vhw()
{
	QString filestr, filename, temp;
	filename = USglobal->config_list.result_dir.copy();
	filename.append("/");
	filename.append(run_inf.run_id);
	temp.sprintf(".%d%d", selected_cell+1, selected_lambda+1);
	filestr = filename.copy();
	filestr.append(".vhw_ext");
	filestr.append(temp);
	QFile vhw_f(filestr);
	unsigned int i, j;	
	if (vhw_f.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&vhw_f);
	//
	// Arrange the file as follows: First, write to the file all apparent sedimentation
	// values, for each scan time one row which contains all divisions, starting at the 
	// lowest concentration. The first column has the times of each scan, then follow
	// the sedimentation coefficients for each division for that time:
	//
		for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
		{	
			ts << (1.0 / pow((double) run_inf.time[selected_cell][selected_lambda][i], (double) 0.5)) << "\t";
			for (j=0; j< (unsigned int) divisions-1; j++)
			{
				if(sed_app[i][j] > 0)
				{
					ts << sed_app[i][j] << "\t";
				}
				else
				{
					ts << "        \t";
				}
			}
			if(sed_app[i][j] > 0)
			{
				ts << sed_app[i][j] << "\n";
			}
			else
			{
				ts << "        \n";
			}
		}
		vhw_f.close();
	}
}

void US_vhwEnhanced::write_dis()
{
//	double *tempx, *tempy;
	QString filestr, filename, temp;
	int count=0;
	filename = USglobal->config_list.result_dir.copy();
	filename.append("/");
	filename.append(run_inf.run_id);
	temp.sprintf(".%d%d", selected_cell+1, selected_lambda+1);
	filestr = filename.copy();
	filestr.append(".vhw_dis");
	filestr.append(temp);
//
// Print the distributions, slopes and intercepts to a file and include all 
// statistical information obtained from the linefits:
//
	QFile dis_f(filestr);
	if (dis_f.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&dis_f);
		ts << tr("%Boundary:    Points:       Slope:      Intercept:      Sigma:      Correlation:\n");
		ts.setf(QTextStream::scientific);
		for (j=0; j<divisions; j++)
		{
			ts << boundary_fractions[j] << "  ";
			count = 0;
			for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
			{
				if (sed_app[i][j] > 0)
				{
					count++;
				}
			}
			ts << count << "  ";
			ts << slope[j] << "  ";
			ts << intercept[j] << "  ";
			ts << sigma[j] << "  ";
			ts << correl[j] << "\n";
		}
		dis_f.close();
	}
}

void US_vhwEnhanced::save_model()
{
	QString filestr, temp;
	float conc;
	float *x, sl, ic, sig, cor;
	x = new float [run_inf.scans[selected_cell][selected_lambda]];
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		x[i] = (float) run_inf.time[selected_cell][selected_lambda][i];
	}
	linefit(&x, &run_inf.plateau[selected_cell][selected_lambda], &sl, &ic, &sig, &cor, run_inf.scans[selected_cell][selected_lambda]);
	delete [] x;
	filestr = USglobal->config_list.result_dir +	"/" +	run_inf.run_id;
	temp.sprintf(".%d%d", selected_cell+1, selected_lambda+1);
	filestr += temp + ".fef_model";
	QFile fef_file(filestr);
	if (fef_file.open(IO_WriteOnly | IO_Translate))
	{
		unsigned int i;
		QTextStream ts(&fef_file);
		ts.setf(0x1000);
		ts << "*************************************\n";
		ts << tr("*   Please do not edit this file!   *\n");
		ts << "*************************************\n\n";
		ts << "3" << "\t# " << modelString[3].latin1() << endl;
		ts << groups << tr("\t\t# Number of Components\n");
		ts << run_inf.meniscus[selected_cell] << tr("\t# Meniscus in cm\n");
		ts << "0.01" << tr("\t# Meniscus range in cm\n");
		ts << "0" << tr("\t\t# Meniscus fitting control\n");
		ts << run_inf.baseline[selected_cell][selected_lambda] << tr("\t# Baseline in OD\n");
		ts << "0.01" << tr("\t# Baseline range in OD\n");
		ts << "1" << tr("\t\t# Baseline fitting control\n");
		ts << "0.0000" << tr("\t# Slope(r) Correction in OD\n");
		ts << "0.0000" << tr("\t# Slope(r) Correction range in OD\n");
		ts << "0" << tr("\t\t# Slope(r) Correction fitting control\n");
		ts << "0.0000" << tr("\t# Slope(c,r) Correction in OD\n");
		ts << "0.0000" << tr("\t# Slope(c,r) Correction range in OD\n");
		ts << "0" << tr("\t\t# Slope(c,r) Correction fitting control\n");
		ts << "20" << tr("\t# Delta_t in seconds\n"); 
		ts << "0.001" << tr("\t# Delta_r in cm\n");
		for (i=0; i<groups; i++)
		{
			ts << tr("\nParameters for Component ") << (i+1) << ":\n\n";
			ts << group_sval[i]*1e-13 << tr("\t# Sedimentation Coefficient in s\n");
			ts << group_sval[i]*1e-14 << tr("\t# Sedimentation Coefficient range in s\n");
			ts << "1" << tr("\t\t# Sedimentation Coefficient fitting control\n");
			ts << "7.00e-07" << tr("\t# Diffusion Coefficient in D\n");
			ts << "7.00e-8" << tr("\t# Diffusion Coefficient range in D\n");
			ts << "1" << tr("\t\t# Diffusion Coefficient fitting control\n");
			conc = (group_percent[i]/100) * (ic - run_inf.baseline[selected_cell][selected_lambda]);
			ts << conc << tr("\t# Partial Concentration in OD\n");
			ts << conc/10 << tr("\t# Partial Concentration range in OD\n");
			ts << "0" << tr("\t\t# Partial Concentration fitting control\n");
			ts << "0.0000" << tr("\t# Sigma\n");
			ts << "0.0000" << tr("\t# Sigma range\n");
			ts << "0" << tr("\t\t# Sigma fitting control\n");
			ts << "0.0000" << tr("\t# Delta\n");
			ts << "0.0000" << tr("\t# Delta range\n");
			ts << "0" << tr("\t\t# Delta fitting control\n");
			ts << "1" << tr("\t\t# Molecular Weight fitting control\n");
			ts << "0" << tr("\t\t# Part. Spec. Volume fitting control\n");
		}
		fef_file.close();
	}
}

void US_vhwEnhanced::write_res()
{
	QString filestr, filename, temp, str;
	filename = USglobal->config_list.result_dir.copy();
	filename.append("/");
	filename.append(run_inf.run_id);
	temp.sprintf(".%d%d", selected_cell+1, selected_lambda+1);
	filestr = filename.copy();
	filestr.append(".vhw_res");
	filestr.append(temp);
	QFile res_f(filestr);
	if (res_f.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&res_f);
		ts << "***************************************************\n";
		ts << tr("*    Enhanced van Holde - Weischet Analysis       *\n");
		ts << "***************************************************\n\n\n";
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
		ts << tr("Divisions:               ") << divisions << "\n";
		ts << tr("Smoothing Frame:         ") << smooth << "\n";
		ts << tr("Analyzed Boundary:       ") << bd_range << " %\n";
		ts << tr("Boundary Position:       ") << bd_position << " %\n";
		ts << tr("Selected Groups:\n\n");
		if (groups == 0)
		{
			ts << tr("No groups were selected...\n\n\n");
		}
		else
		{
			ts << tr("Group: Average S: Relative Amount:\n\n");
			for (i=0; i<groups; i++)
			{
				str.sprintf("%3d:    %6.2fs      (%5.2f", i+1, group_sval[i], group_percent[i]);
				str.append("%)\n");
				ts << str;
			}
			ts << "\n\n";
			save_model();
		}
		ts << tr("Average S:               ") << average_s << "\n";
		if(plateaus_corrected)
		{
			ts << tr("Initial concentration from plateau fit: ") << initial_concentration << " OD/fringes\n";
		}
		ts << "\n\n";
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
		float *x, sl, ic, sig, cor;
		x = new float [run_inf.scans[selected_cell][selected_lambda]];
		for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
		{
			x[i] = (float) run_inf.time[selected_cell][selected_lambda][i];
		}
		linefit(&x, &run_inf.plateau[selected_cell][selected_lambda], &sl, &ic, &sig, &cor, run_inf.scans[selected_cell][selected_lambda]);
		ts << "\nInitial Concentration: " << ic << "OD\n(Correlation. coeff.: " << cor << "\nStandard Dev.: " << sig << ")\n\n";
		ts << "\nInitial Concentration from exponential fit: " << initial_concentration << "OD\n";
		delete [] x;
		res_f.close();
	}
}




