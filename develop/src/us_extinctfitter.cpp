#include "../include/us_extinctfitter.h"

US_ExtinctionFitter::US_ExtinctionFitter(vector <struct WavelengthScan> *temp_wls_v, 
									double *temp_guess,
									unsigned int temp_order,
									unsigned int temp_parameters,
									QString temp_projectName,
									bool *temp_fitting_widget,
									QWidget *p, 
									const char *name) : US_Minimize(temp_fitting_widget, true, p, name)
{
	guess = temp_guess;
	parameters = temp_parameters;
	order = temp_order;
	wls_v = temp_wls_v;
	runs = 0;
	runs_percent = 0;
	USglobal = new US_Config();
	projectName = temp_projectName;
	lbl_header->setText(tr("Global Nonlinear Least Squares\nExtinction Profile Fitting Control"));
	createHtmlDir();
}

US_ExtinctionFitter::~US_ExtinctionFitter()
{
}

void US_ExtinctionFitter::saveFit()
{
	QMessageBox::message(tr("Extinction Fitter:"), tr("Please use the save function from\n"
															 "the main extinction calculation\n"
															 "module. It will allow you to save the\n"
															 "coefficients of the fit as well as a\n"
															 "copy of the plot window."));																
}

void US_ExtinctionFitter::startFit()
{
	if (autoconverge)
	{
		iteration = 1000;
		while (iteration > 1) //
		{
			return_value = Fit(); // don't bother checking return values...
			if (nlsMethod == 3)	// switch between Levenberg Marquardt and Quasi Newton
			{
				nlsMethod = 0;
				if (GUI)
				{
					lbl_status4->setText("Switching to Levenberg-Marquardt...");
					cbb_nlsMethod->setCurrentItem(0);
				}
			}
			else
			{
				nlsMethod = 3;
				if (GUI)
				{
					lbl_status4->setText("Switching to Quasi-Newton...");
					cbb_nlsMethod->setCurrentItem(3);
				}
			}
		}
	}
	else // just do a single fit
	{
		return_value = Fit();
		if (return_value != 0)
		{
			QString str;
			if (GUI)
			{
				QMessageBox::message(tr("Extinction Fitter:"), tr("The program exited with a return\n"
											 "value of ") + str.sprintf("%d.", return_value));
			}
			else
			{
				cout << str.latin1() << endl;
			}
		}
	}
}

bool US_ExtinctionFitter::fit_init()
{
	unsigned int i, j, point_counter;
//
// Calculate how many points there are in each dataset, sum them up for "points" and
// leave out datasets that aren't fitted to keep the vectors/matrices as small as possible.
//
	points = 0;
	datasets = (*wls_v).size();

	for (i=0; i<(*wls_v).size(); i++)
	{
		points_per_dataset.push_back((*wls_v)[i].lambda.size());
		points += (*wls_v)[i].lambda.size();
	}
	if ((*wls_v).size() == 0)
	{
		return(false);
	}
	
	iteration = 0;

	y_raw						= new float    [points];		// experimental data (absorbance)
	y_guess					= new double   [points];		// simulated solution
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

// initialize y_raw:
	point_counter = 0;
	for (i=0; i<(*wls_v).size(); i++)
	{
		for (j=0; j<points_per_dataset[i]; j++)
		{
			y_raw[point_counter] = (*wls_v)[i].od[j];
			point_counter++;
		}
	}
	return(true);
}

int US_ExtinctionFitter::calc_model(double *guess_par)
{
	QString str;
	unsigned int i, j, k, point_counter=0;
	float gaussian;
	for (i=0; i<(*wls_v).size(); i++)
	{
		for (j=0; j<points_per_dataset[i]; j++)
		{
			gaussian = 0.0;
			for (k=0; k<order; k++)
			{
				gaussian += exp(guess_par[(*wls_v).size() + (3 * k)]
				- (pow(((*wls_v)[i].lambda[j] - guess_par[(*wls_v).size() + (3 * k) + 1]), 2)
				/ ( 2 * pow(guess_par[(*wls_v).size() + (3 * k) + 2], 2))));
			}
			y_guess[point_counter] = guess_par[i] * gaussian;
			point_counter++;
		}
	}
	function_evaluations++;
	lbl_evaluations2->setText(str.sprintf(" %d", function_evaluations));
	qApp->processEvents();
	if (aborted)
	{
		return(-1);
	}
	return(0);
/* //Polynomial fit:
	QString str;
	unsigned int i, j, k, point_counter=0;
	float polynomial;
	for (i=0; i<(*wls_v).size(); i++)
	{
		for (j=0; j<points_per_dataset[i]; j++)
		{
			polynomial = 0.0;
			for (k=0; k<order; k++)
			{
				polynomial += guess_par[(*wls_v).size() + k] * pow((*wls_v)[i].lambda[j], k);
			}
			y_guess[point_counter] = guess_par[i] * polynomial;
//cout << "dataset: " << i << ", point: " << j << ", guessed y-value: " << y_guess[point_counter] << endl;
			point_counter++;
		}
	}
	function_evaluations++;
	lbl_evaluations2->setText(str.sprintf(" %d", function_evaluations));
	qApp->processEvents();
	if (aborted)
	{
		return(-1);
	}
	return(0);
*/

}

int US_ExtinctionFitter::calc_jacobian()
{
	unsigned int i, j, k, point_counter=0;
	float *term;
	term = new float [order];
	for (i=0; i<points; i++)
	{
		for (j=0; j<parameters; j++)
		{
			jacobian[i][j] = 0.0;
		}
	}
	for (i=0; i<(*wls_v).size(); i++)
	{
		for (j=0; j<points_per_dataset[i]; j++)
		{
			for (k=0; k<order; k++)
			{
				term[k] = exp(guess[(*wls_v).size() + (3 * k)]
				- (pow(((*wls_v)[i].lambda[j] - guess[(*wls_v).size() + (3 * k) + 1]), 2)
				/ ( 2 * pow(guess[(*wls_v).size() + (3 * k) + 2], 2))));
			}
			jacobian[point_counter][i] = 0.0;
			for (k=0; k<order; k++)
			{
				jacobian[point_counter][i] += term[k];
				
				jacobian[point_counter][(*wls_v).size() + (3 * k)] 	 = guess[i] * term[k];
				
				jacobian[point_counter][(*wls_v).size() + (3 * k) + 1] = guess[i] * term[k]
				* (((*wls_v)[i].lambda[j] - guess[(*wls_v).size() + (3 * k) + 1]) 
				/ pow(guess[(*wls_v).size() + (3 * k) + 2], 2));
				
				jacobian[point_counter][(*wls_v).size() + (3 * k) + 2] = guess[i] * term[k]
				* (pow(guess[(*wls_v).size() + (3 * k) + 2], -3) 
				* (pow(guess[(*wls_v).size() + (3 * k) + 1], 2) 
				- 2 * guess[(*wls_v).size() + (3 * k) + 1] 
				* (*wls_v)[i].lambda[j] + pow((*wls_v)[i].lambda[j], 2)));
			}
			point_counter++;
		}
	}
	qApp->processEvents();
	if (aborted)
	{
		return(-1);
	}
	return(0);
/* Jacobian for polynomial fit:
	unsigned int i, j, k, point_counter=0;
	float polynomial;
	for (i=0; i<points; i++)
	{
		for (j=0; j<parameters; j++)
		{
			jacobian[i][j] = 0.0;
		}
	}
// y[i][j] = guess[i] * (a[0] + a[1] * x[i][j] + a[2] * x[i][j]^2 + ... + a[n] * x[i][j]^n)
	for (i=0; i<(*wls_v).size(); i++)
	{
		for (j=0; j<points_per_dataset[i]; j++)
		{
			polynomial = 0.0;
			for (k=0; k<order; k++)
			{
				polynomial += guess[(*wls_v).size() + k] * pow((*wls_v)[i].lambda[j], k);
			}
// dy/dguess[i] = (a[0] + a[1] * x[i][j] + a[2] * x[i][j]^2 + ... + a[n] * x[i][j]^n)
			jacobian[point_counter][i] = polynomial;
// dy/da[n] = guess[i] * x[i][j]^n
			jacobian[point_counter][(*wls_v).size()] = guess[i];
			for (k=1; k<order; k++)
			{
				jacobian[point_counter][(*wls_v).size() + k] = guess[i] * pow((*wls_v)[i].lambda[j], k);
			}
			point_counter++;
		}
	}
	qApp->processEvents();
	if (aborted)
	{
		return(-1);
	}
	return(0);
*/
	delete [] term;
}

void US_ExtinctionFitter::cleanup()
{
	bt_plotGroup->setEnabled(false);

	unsigned int i;
	delete [] y_raw;
	delete [] y_guess;
	delete [] y_delta;
	delete [] B;
	delete [] test_guess;
	
	for (i=0; i<points; i++)
	{
		delete [] jacobian[i];
	}
	for (i=0; i<parameters; i++)
	{
		delete [] information_matrix[i];
		delete [] LL_transpose[i];
	}
	delete [] jacobian;
	delete [] information_matrix;
	delete [] LL_transpose;
}

void US_ExtinctionFitter::view_report()
{
	QString fileName, str;
	fileName = USglobal->config_list.result_dir + "/" + projectName + "-extinction.res";
	//view_file(fileName);
	TextEdit *e;
	e = new TextEdit();
	e->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	e->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
	e->load(fileName);
	e->show();

}

void US_ExtinctionFitter::write_report()
{
}

void US_ExtinctionFitter::plot_overlays()
{
	double **xplot = 0, **yplot_fit = 0, **yplot_raw = 0;
	unsigned int *curve_raw, *curve_fit, numScans = 0;
	long unsigned int point_counter = 0;
	plotResiduals = false;
	QString s1, s2;
	xplot = new double* [datasets];
	yplot_fit = new double* [datasets];
	yplot_raw = new double* [datasets];
	curve_fit = new unsigned int [datasets];
	curve_raw = new unsigned int [datasets];
	if (plotGroup)
	{
		if (datasets - firstScan == 0)
		{
			numScans = 1;
			s1.sprintf(tr("Overlays for fitted Scan %ld"), firstScan);
		}
		else if (datasets - firstScan < 5)
		{
			numScans = datasets - firstScan + 1;
			s1.sprintf(tr("Overlays for fitted Scans %ld - %ld"), firstScan, firstScan + numScans - 1);
		}
		else
		{
			numScans = 5;
			s1.sprintf(tr("Overlays for fitted Scans %ld - %ld"),  firstScan, firstScan+4);
		}
	}
	else
	{
		s1 = tr("Overlays");
	}
	s2.sprintf(tr("Optical Density"));
	point_counter = 0;
	for (unsigned int i=0; i<(*wls_v).size(); i++)
	{
		xplot[i] = new double [points_per_dataset[i]];
		yplot_fit[i] = new double [points_per_dataset[i]];
		yplot_raw[i] = new double [points_per_dataset[i]];
		for (unsigned int j=0; j<points_per_dataset[i]; j++)
		{
			xplot[i][j] = (*wls_v)[i].lambda[j];
			yplot_fit[i][j] = y_guess[point_counter];
			yplot_raw[i][j] = y_raw[point_counter];
			point_counter++;
		}
	}
	QwtSymbol symbol;
	QPen p;
	p.setColor(Qt::red);
	p.setWidth(2);
	symbol.setSize(10);
	symbol.setPen(Qt::blue);
	symbol.setBrush(Qt::yellow);
	symbol.setStyle(QwtSymbol::Ellipse);
	data_plot->clear();
	data_plot->setTitle(s1);
	data_plot->setAxisTitle(QwtPlot::xBottom, tr("Wavelength (nm)"));
	data_plot->setAxisTitle(QwtPlot::yLeft, s2);
	if (plotGroup)
	{
		for (unsigned int i = firstScan - 1; i< numScans + firstScan - 1; i++)
		{
			curve_raw[i] = data_plot->insertCurve("raw data");
			curve_fit[i] = data_plot->insertCurve("fitted data");
			data_plot->setCurveStyle(curve_raw[i], QwtCurve::NoCurve);
			data_plot->setCurveData(curve_raw[i], xplot[i], yplot_raw[i], points_per_dataset[i]);
			data_plot->setCurveSymbol(curve_raw[i], symbol);
			data_plot->setCurveData(curve_fit[i], xplot[i], yplot_fit[i], points_per_dataset[i]);
			data_plot->setCurveStyle(curve_fit[i], QwtCurve::Lines);
			data_plot->setCurvePen(curve_fit[i], p);
		}
	}
	else
	{
		for (unsigned int i=0; i<datasets; i++)
		{
			curve_raw[i] = data_plot->insertCurve("raw data");
			curve_fit[i] = data_plot->insertCurve("fitted data");
			data_plot->setCurveStyle(curve_raw[i], QwtCurve::NoCurve);
			data_plot->setCurveData(curve_raw[i], xplot[i], yplot_raw[i], points_per_dataset[i]);
			data_plot->setCurveSymbol(curve_raw[i], symbol);
			data_plot->setCurveData(curve_fit[i], xplot[i], yplot_fit[i], points_per_dataset[i]);
			data_plot->setCurveStyle(curve_fit[i], QwtCurve::Lines);
			data_plot->setCurvePen(curve_fit[i], p);
		}
	}
//	data_plot->setAxisScale(QwtPlot::xBottom, -xmax/30.0, xmax + xmax/30.0, 0);
	data_plot->replot();
	//data_plot->updatePlot();		//no updatePlot() in new version
	for (unsigned int i=0; i<datasets; i++)
	{
		delete [] xplot[i];
		delete [] yplot_raw[i];
		delete [] yplot_fit[i];
	}
	delete [] xplot;
	delete [] yplot_raw;
	delete [] yplot_fit;
	delete [] curve_fit;
	delete [] curve_raw;
	pb_print->setEnabled(true);
}

void US_ExtinctionFitter::plot_residuals()
{
	double **xplot = 0, **yplot_res = 0, line_x[2], line_y[2];
	unsigned int *curve_res, zeroLine[5], numScans = 0, l;
	unsigned long point_counter = 0;
	QString s1, s2, s3;
	float offset = 0;
	double xmax = -1e6, xmin = 1e6;
	int last_sign = 0, current_sign = 0;
	long scanMarker[5];
	plotResiduals = true;
	xplot = new double* [datasets];
	yplot_res = new double* [datasets];
	curve_res = new unsigned int [datasets];
	if (plotGroup)
	{
		if (datasets - firstScan == 0)
		{
			numScans = 1;
			s1.sprintf(tr("Residuals from fitted Scan %ld"), firstScan);
		}
		else if (datasets - firstScan < 5)
		{
			numScans = datasets - firstScan + 1;
			s1.sprintf(tr("Residuals from fitted Scans %ld - %ld"), firstScan, firstScan + numScans - 1);
		}
		else
		{
			numScans = 5;
			s1.sprintf(tr("Residuals from fitted Scans %ld - %ld"),  firstScan, firstScan+4);
		}
	}
	else
	{
		s1 = tr("Residuals");
	}
	s2.sprintf(tr("Optical Density Difference\n"));
	point_counter = 0;
	for (unsigned int i=0; i<(*wls_v).size(); i++)
	{
		(*wls_v)[i].pos = 0;
		(*wls_v)[i].neg = 0;
		(*wls_v)[i].runs = 0;
		xplot[i] = new double [points_per_dataset[i]];
		yplot_res[i] = new double [points_per_dataset[i]];
		for (unsigned int j=0; j<points_per_dataset[i]; j++)
		{
			xplot[i][j] = (*wls_v)[i].lambda[j];
			yplot_res[i][j] = y_guess[point_counter] - y_raw[point_counter];
			if (yplot_res[i][j] > 0)
			{
				(*wls_v)[i].pos++;
				current_sign = 1;
			}
			else
			{
				(*wls_v)[i].neg++;
				current_sign = -1;
			}
			if (j == 0)
			{
				if (yplot_res[i][j] > 0)
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
					(*wls_v)[i].runs++;
					last_sign = current_sign;
				}
			}
			point_counter++;
		}
		xmax = max(xplot[i][points_per_dataset[i] - 1], xmax);
		xmin = min(xplot[i][0], xmin);
	}
	QwtSymbol symbol;
	QPen p_raw, p_zero;
	p_raw.setColor(Qt::green);
	p_raw.setWidth(1);
	p_zero.setColor(Qt::red);
	p_zero.setWidth(2);
	symbol.setPen(Qt::blue);
	symbol.setBrush(Qt::yellow);
	symbol.setStyle(QwtSymbol::Ellipse);
	data_plot->clear();
	data_plot->setTitle(s1);
	data_plot->setAxisTitle(QwtPlot::xBottom, tr("Wavelength (nm)"));
	data_plot->setAxisTitle(QwtPlot::yLeft, s2);
	line_y[0] = 0;
	line_y[1] = 0;
	l = 0;
	if (plotGroup)
	{
		symbol.setSize(8);
		line_x[0] = xmin - 2;
		line_x[1] = xmax + 2;
		for (unsigned int i = firstScan - 1; i< numScans + firstScan - 1; i++)
		{
			curve_res[i] = data_plot->insertCurve("residuals");
			data_plot->setCurveStyle(curve_res[i], QwtCurve::Lines);
			if (i != firstScan - 1)
			{
				for (unsigned int j=0; j<points_per_dataset[i]; j++)
				{
					yplot_res[i][j] += offset;
				}
				line_y[0] = offset;
				line_y[1] = offset;
			}
			data_plot->setCurveData(curve_res[i], xplot[i], yplot_res[i], points_per_dataset[i]);
			data_plot->setCurveSymbol(curve_res[i], symbol);
			data_plot->setCurvePen(curve_res[i], p_raw);
			zeroLine[l] = data_plot->insertCurve("Zero Line");
			data_plot->setCurveData(zeroLine[l], line_x, line_y, 2);
			data_plot->setCurveStyle(zeroLine[l], QwtCurve::Lines);
			data_plot->setCurvePen(zeroLine[l], p_zero);
			scanMarker[l] = data_plot->insertMarker();
			data_plot->setMarkerLabel(scanMarker[l], s3.sprintf("%d", i)); 
			data_plot->setMarkerPos(scanMarker[l], line_x[1] + 7, offset);
			data_plot->setMarkerPen(scanMarker[l], p_zero);
			data_plot->setMarkerFont(scanMarker[l], QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold)); 
			offset += 0.03;
			l++;
		}
		data_plot->setAxisScale(QwtPlot::xBottom, xmin - 10, xmax + 15, 0);
	}
	else
	{
		symbol.setSize(5);
		line_x[0] = xmin - 2;
		line_x[1] = xmax + 2;
		for (unsigned int i=0; i<datasets; i++)
		{
			curve_res[i] = data_plot->insertCurve("residuals");
			data_plot->setCurveStyle(curve_res[i], QwtCurve::Lines);
			data_plot->setCurveData(curve_res[i], xplot[i], yplot_res[i], points_per_dataset[i]);
			data_plot->setCurveSymbol(curve_res[i], symbol);
			data_plot->setCurvePen(curve_res[i], p_raw);
		}
		zeroLine[0] = data_plot->insertCurve("Zero Line");
		data_plot->setCurveData(zeroLine[0], line_x, line_y, 2);
		data_plot->setCurveStyle(zeroLine[0], QwtCurve::Lines);
		data_plot->setCurvePen(zeroLine[0], p_zero);
		data_plot->setAxisScale(QwtPlot::xBottom, xmin - 10, xmax + 10, 0);
	}
	data_plot->replot();
	//data_plot->updatePlot();			//no updatePlot() in new version
	unsigned int i, total_pos = 0, total_neg = 0;
	for (i=0; i<(*wls_v).size(); i++)
	{
		total_pos += (*wls_v)[i].pos;
		total_neg += (*wls_v)[i].neg;
		runs += (*wls_v)[i].runs;
	}
	runs_percent = (float) (total_pos + total_neg);
	runs_percent = (float) runs / runs_percent;
	runs_percent *= 100.0;
	update_fitDialog();
	for (unsigned int i=0; i<datasets; i++)
	{
		delete [] xplot[i];
		delete [] yplot_res[i];
	}
	delete [] xplot;
	delete [] yplot_res;
	delete [] curve_res;
	write_report();
}

void US_ExtinctionFitter::updateRange(double scan)
{
	firstScan = (int) scan;
	if (firstScan > datasets)
	{
		firstScan = 1;
	}
	unsigned int modulus = datasets % 5;
	switch (modulus)
	{
		case 0:
		{
			cnt_scan->setRange(1, datasets-4, 5);
			break;
		}
		case 1:
		{
			cnt_scan->setRange(1, datasets, 5);
			break;
		}
		case 2:
		{
			cnt_scan->setRange(1, datasets-1, 5);
			break;
		}
		case 3:
		{
			cnt_scan->setRange(1, datasets-2, 5);
			break;
		}
		case 4:
		{
			cnt_scan->setRange(1, datasets-3, 5);
			break;
		}
	}
	cnt_scan->setValue(firstScan);
	if (plotResiduals)
	{
		plot_residuals();
	}
	else
	{
		plot_overlays();
	}
}

void US_ExtinctionFitter::endFit()
{
	emit currentStatus("Converged");
	emit hasConverged();
	converged = true;
	pb_cancel->setText(tr("Close"));
	pb_fit->setEnabled(true);
	lbl_status2->setText(tr("The fitting process converged..."));
	plot_residuals();
	pgb_progress->setProgress(totalSteps);
	pb_residuals->setEnabled(true);
	pb_overlays->setEnabled(true);
	pb_saveFit->setEnabled(true);
}

bool US_ExtinctionFitter::createHtmlDir()
{
	htmlDir = USglobal->config_list.html_dir + "/" + projectName;
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
