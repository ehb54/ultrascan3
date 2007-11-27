#include "../include/us_vhwdat.h"
#ifndef WIN32
#include <unistd.h>
#endif
#include <math.h>
#include <float.h>

vhw_dat_W::vhw_dat_W(QWidget *p, const char *name) : Data_Control_W(1, p, name)
{
	baseline_flag = true;
	bd_range=90;
	bd_position = 5;
	range_counter->setValue(bd_range);
	position_counter->setValue(bd_position);

	pm = new US_Pixmap();
	pb_second_plot->setText(tr("Distribution Plot"));
	setup_already = false;	// for garbage collection
	xpos = border+buttonw+spacing;
	ypos = 410;

	groups_on = false;
	group_start = true;
	groups = 0;
	pb_groups = new QPushButton(tr("Select Groups"), this);
	Q_CHECK_PTR(pb_groups);
	pb_groups->setAutoDefault(false);
	pb_groups->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_groups->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
//	pb_groups->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_groups, SIGNAL(clicked()), SLOT(select_groups()));

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
	
	setup_GUI();
}

void vhw_dat_W::setup_GUI()
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
	subGrid2->addWidget(pb_vbar,j,0);
	subGrid2->addWidget(vbar_le,j,1);
	subGrid2->addWidget(lbl1_excluded,j,2);
	subGrid2->addWidget(lbl2_excluded,j,3);
	j++;
	subGrid2->addMultiCellWidget(pb_reset,j,j,0,1);
	subGrid2->addMultiCellWidget(pb_groups,j,j,2,3);
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

vhw_dat_W::~vhw_dat_W()
{
}

void vhw_dat_W::excludeSingleSignal(unsigned int scan)
{
	if (excluded[scan])
	{
		return;
	}
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
	analysis_plot->setCurvePen(exclude_singlecurve, Qt::red);
	analysis_plot->setCurveStyle(exclude_singlecurve, QwtCurve::Lines);
	analysis_plot->setCurveData(exclude_singlecurve, tempx, tempy, 2);
	analysis_plot->replot();
	delete [] tempx;
	delete [] tempy;
}

void vhw_dat_W::excludeRangeSignal(unsigned int scan)
{
	if (excluded[scan])
	{
		return;
	}
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

void vhw_dat_W::second_plot()
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

void vhw_dat_W::save()
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

void vhw_dat_W::help()
{
	US_Help *online_help;
	online_help = new US_Help(this);
	online_help->show_help("manual/vhw.html");
}

void vhw_dat_W::update_divisions(double val)
{
	if (step == 0)
	{
		QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n	   Click on \"Load Data\"\n"));
		return;
	}
	divisions = (unsigned int) val;
	plot_analysis();
}

void vhw_dat_W::select_groups()
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

void vhw_dat_W::getAnalysisPlotMousePressed(const QMouseEvent &e)
{
// store position
	p1 = e.pos();
}

void vhw_dat_W::getAnalysisPlotMouseReleased(const QMouseEvent &e)
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

int vhw_dat_W::plot_analysis()
{
	QString s1, s2, s3;
	double force;
	int first=0;
	max_sed = -1;
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
	if (setup_already)
	{
		cleanup();
	}
	double a , b, temp_r, testy;
	unsigned int step=0, i, j, count;
	if (setups() < 0)
	{
		return(-1);
	}
	for (int m=run_inf.scans[selected_cell][selected_lambda]-1; m>=0; m--)
	{
		if (!excluded[m])
		{
			first = m;
		}
	}
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
	//
	// the span is the boundary portion that is going to be analyzed (in percent).
	//
		if (!excluded[i])
		{
			span = run_inf.plateau[selected_cell][selected_lambda][i] * bd_range/100;
		//
		// one interval is the distance from one division to the next, we initialize the
		// 
		//
			interval = span/(divisions+1);
			testy = run_inf.plateau[selected_cell][selected_lambda][i] * bd_position/100;
			for (j=0; j<divisions; j++)
			{
				boundary_fractions[j] = 100 * (bd_position/100 + (float) ((j + 1)*bd_range/100) / (float)(divisions + 1));
				testy += interval;  // increment the division tester by one interval
				count=1;
				while (absorbance[i][count] < testy)
				{
					count++;		// climb up the concentration gradient until one 
				}					// point beyond testy is reached
			//
			// Now we need a linear interpolation between the last point before and the 
			// first point beyond testy. The radius corresponding to the interpolated 
			// concentration of testy on that line is the radius needed for the calculation
			// of the apparent sedimentation coefficient. If the test concentration is equal
			// to a point in the concentration matrix, we don't have to interpolate and can
			// use the point count instead to obtain the correct radius:
			//
				if (absorbance[i][count] == testy)
				{
					temp_r = radius[count];
				}
				else
				{
				//
				// solve: y = a*x + b:
				//
					a = (absorbance[i][count] - absorbance[i][count-1])/(run_inf.delta_r);
					b = absorbance[i][count] - a * radius[count];
					temp_r = (testy - b)/a;
				}
			//debug("Sed_app: %1.3e", sed_app[i-exclude][j]);
				force = omega_s * run_inf.time[selected_cell][selected_lambda][i];
				if(temp_r <= 0 || run_inf.meniscus[selected_cell] <=0  || force <=0)
				{
					QMessageBox::message( tr("Warning"), tr("Attention:\n\n"
																	  "The van Holde - Weischet analysis encountered\n"
																	  "a mathematical exception. This is generally caused\n"
																	  "by ill-conditioned data. \n\n"
																	  "Suggestion: Try to increase the smoothing level\n"
																	  "for this cell.\n\n"
																	  "Alternatives: Re-edit the data or exclude the data\n"
																	  "from this cell during the editing."));
					return(-2);	
				}  
				sed_app[i][j] = correction * (log(temp_r/run_inf.meniscus[selected_cell]) / force * 1.0e13);
				max_sed = (float) max((double) max_sed, sed_app[i][j]);
			
			// (first dimension for scans, second for divisions)
			// now correct for vbar, viscosity and density:
			}
			step++;
			progress->setProgress(step);
		}
	}
//
// for each division, fit the sed_app's of all scans to a straight line, the intercept
// represents the distribution, the slopes can be saved for other things (i.e, Diffusion).
//
	analysis_plot->clear();
	s1.sprintf(tr(": Cell %d (%d nm) - vHW Extrapolation Plot"), selected_cell+1, run_inf.wavelength[selected_cell][selected_lambda]);
	s2 = tr("Run ");
	s2.append(run_inf.run_id);
	s2.append(s1);
	analysis_plot->setTitle(s2);
	analysis_plot->setAxisTitle(QwtPlot::xBottom, tr("(Time)^-0.5"));
	analysis_plot->setAxisTitle(QwtPlot::yLeft, tr("Corrected Sed. Coeff. (1e-13 s)"));

//	analysis_plot->enableGridXMin();
//	analysis_plot->enableGridYMin();
//	analysis_plot->setGridMajPen(QPen(colorGroup().light(), 0, DotLine));
//	analysis_plot->setGridMinPen(QPen(colorGroup().mid(), 0 , DotLine));
	for (j=0; j<divisions; j++)
	{
		count = 0;
		for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
		{
			if(!excluded[i])
			{
				tempx[count] = 1.0/sqrt((double) run_inf.time[selected_cell][selected_lambda][i]);
				tempy[count] = sed_app[i][j];
				count ++;
			}
		}
	// debug("i: %d, time: %f, tempx: %f, tempy: %f", i, run_inf.time[selected_cell][selected_lambda][i], tempx[i-exclude], tempy[i-exclude]);
		linefit(&tempx, &tempy, &slope[j], &intercept[j], &sigma, &correl, (run_inf.scans[selected_cell][selected_lambda] - exclude));
	// debug("div: %d, ic: %f, slope: %f", j, intercept[j], slope[j]);
		sa_points[j] = analysis_plot->insertCurve("Apparent Sedimentation Coefficients");
		analysis_plot->setCurveStyle(sa_points[j], QwtCurve::NoCurve);
		analysis_plot->setCurveSymbol(sa_points[j], QwtSymbol(sym));
		analysis_plot->setCurveData(sa_points[j], tempx, tempy, run_inf.scans[selected_cell][selected_lambda]-exclude);
	}
	for (j=0; j<divisions; j++)
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
	analysis_plot->replot();
//analysis_plot->updatePlot();		//no updatePlot() in new version
	return(0);
}

int vhw_dat_W::setups()
{
	QString str;
	group_sval = new float [divisions];
	group_percent = new float [divisions];
	sval_label = new uint [divisions];
	excluded = new bool [run_inf.scans[selected_cell][selected_lambda]];
// 
// make sure that the first datapoint of each scan has a lower concentration than 
// what would be needed for the first division:
//
	exclude=0;
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		span = run_inf.plateau[selected_cell][selected_lambda][i] * bd_range/100;
		interval = span/(divisions+1);
		if (absorbance[i][0] >= interval + run_inf.plateau[selected_cell][selected_lambda][i] * bd_position/100)
		{
			excluded[i] = true;
			exclude++;
		}
		else
		{
			excluded[i] = false;
		}
	}
	lbl2_excluded->setText(str.sprintf("%d", exclude));
	if ((run_inf.scans[selected_cell][selected_lambda] - exclude) < 3)
	{
		QMessageBox::message( tr("Warning"), tr("Attention:\n\n"
														  "Not enough scans have cleared the meniscus. In order to perform a\n"
														  "meaningful van Holde - Weischet Analysis, you need at least 3 scans\n"
														  "that have cleared the meniscus.\n\n"));
		return (-1);
	}
	progress->setTotalSteps(run_inf.scans[selected_cell][selected_lambda] - exclude);
	progress->reset();
	progress->setProgress(0);
	tempx = new double [run_inf.scans[selected_cell][selected_lambda]-exclude];
	tempy = new double [run_inf.scans[selected_cell][selected_lambda]-exclude];
	sed_app = new double* [run_inf.scans[selected_cell][selected_lambda]]; // first dimension for scans
	for (i=0; i < (run_inf.scans[selected_cell][selected_lambda]); i++)
	{
		sed_app[i] = new double [divisions]; // second dimension for divisions
	}
	old_local_scans = run_inf.scans[selected_cell][selected_lambda];
	slope = new double [divisions];
	intercept = new double [divisions];
	boundary_fractions = new double [divisions];
	sa_points = new uint [divisions];
	sa_lines = new uint [divisions];
	lines_y = new double [2];
	lines_x = new double [2];
	setup_already = true;
	return (0);
}

void vhw_dat_W::cleanup()
{
	delete [] group_sval;
	delete [] group_percent;
	delete [] sval_label;
	delete [] excluded;
	delete [] tempx;
	delete [] tempy;
	for (i=0; i < old_local_scans; i++)
	{
		delete [] sed_app[i]; // second dimension for divisions
	}
	delete [] sed_app; // first dimension for scans
	delete [] slope;
	delete [] intercept;
	delete [] boundary_fractions;
	delete [] sa_points;
	delete [] sa_lines;
	delete [] lines_y;
	delete [] lines_x;
	setup_already = false;
}

void vhw_dat_W::view()
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
//	view_file(filestr);
	TextEdit *e;
	e = new TextEdit();
	e->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	e->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
	e->load(filestr);
	e->show();
}

void vhw_dat_W::write_vhw()
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
			if (!excluded[i])
			{
				ts << (1.0 / pow((double) run_inf.time[selected_cell][selected_lambda][i], (double) 0.5)) << "\t";
				for (j=0; j< (unsigned int) divisions-1; j++)
				{
					ts << sed_app[i][j] << "\t";
				}
				ts << sed_app[i][j] << "\n";
			}
		}
		vhw_f.close();
	}
}

void vhw_dat_W::write_dis()
{
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
		ts << tr("%Boundary:    Average_S:       Slope:      Intercept:      Sigma:      Correlation:\n");
		ts.setf(QTextStream::scientific);
		for (j=0; j<divisions; j++)
		{
			ts << boundary_fractions[j] << "  ";
			count = 0;
			for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
			{
				if (!excluded[i])
				{
					tempx[count] = 1.0/sqrt((double) run_inf.time[selected_cell][selected_lambda][i]);
					tempy[count] = sed_app[i][j];
					count++;		//index variable for tempx/y so we can keep track of the excluded scans.
				}
			}
			ts << linefit(&tempx, &tempy, &slope[j], &intercept[j], &sigma, &correl, 
							 (run_inf.scans[selected_cell][selected_lambda] - exclude)) << "  ";
			ts << slope[j] << "  ";
			ts << intercept[j] << "  ";
			ts << sigma << "  ";
			ts << correl << "\n";
		}
		dis_f.close();
	}
}

void vhw_dat_W::save_model()
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



void vhw_dat_W::write_res()
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
		ts << tr("*         van Holde - Weischet Analysis           *\n");
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
		ts << tr("Early Scans skipped:     ") << exclude << tr(" Scans \n\n\n");
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
		delete [] x;
		res_f.close();
	}
}




