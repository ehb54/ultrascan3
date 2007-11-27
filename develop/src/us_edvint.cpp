#include "../include/us_edvint.h"

EditIntVeloc_Win::EditIntVeloc_Win(QWidget *p , const char *name) 
: EditData_Win(p, name)
{
	absorbance_integral = NULL;
	residuals = NULL;
	edit_type = 3;
	extension1 = "IP";
	extension2 = "ip";
	edit_plt->setTitle(tr("Interference Velocity Data"));
	edit_plt->setAxisTitle(QwtPlot::yLeft, tr("Fringes"));
	residuals_defined = false; //if true, residuals can be subtracted
	run_inf.exp_type.velocity = true;
	run_inf.exp_type.equilibrium = false;
	run_inf.exp_type.diffusion = false;
	run_inf.exp_type.simulation = false;
	run_inf.exp_type.interference = true;
	run_inf.exp_type.absorbance = false;
	run_inf.exp_type.fluorescence = false;
	run_inf.exp_type.intensity = false;
	run_inf.exp_type.wavelength = false;
	delete pb_spikes;
	pb_subtract = new QPushButton(tr("Subtract Residuals"), this);
	Q_CHECK_PTR(pb_subtract);
	pb_subtract->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_subtract->setAutoDefault(false);
	pb_subtract->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_subtract->setEnabled(false);
	connect(pb_subtract, SIGNAL(clicked()), SLOT(subtract_residuals()));
	pb_subtract->setEnabled(false);
			
	pb_movie = new QPushButton(tr("Movie"), this);
	Q_CHECK_PTR(pb_movie);
	pb_movie->setAutoDefault(false);
	pb_movie->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_movie->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_movie->setEnabled(false);
	connect(pb_movie, SIGNAL(clicked()), SLOT(movie()));

	pb_continue = new QPushButton(tr("Continue"), this);
	Q_CHECK_PTR(pb_continue);
	pb_continue->setAutoDefault(false);
	pb_continue->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_continue->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_continue->setEnabled(false);
	connect(pb_continue, SIGNAL(clicked()), SLOT(next_step()));

	fringe_tolerance = 0.4;
	ct_fringe_tolerance = new QwtCounter(this);
	ct_fringe_tolerance->setNumButtons(3);
	ct_fringe_tolerance->setRange(0.0, 20.0, 0.001);
	ct_fringe_tolerance->setValue(fringe_tolerance);
	ct_fringe_tolerance->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	ct_fringe_tolerance->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	ct_fringe_tolerance->setEnabled(false);
//	connect(ct_fringe_tolerance, SIGNAL(buttonReleased(double)), SLOT(setFringeTolerance(double)));
	connect(ct_fringe_tolerance, SIGNAL(valueChanged(double)), SLOT(setFringeTolerance(double)));

	ct_order = new QwtCounter(this);
	ct_order->setNumButtons(1);
	ct_order->setRange(0, 20, 1);
	ct_order->setValue(5);
	ct_order->setEnabled(false);
	ct_order->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	ct_order->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
//	connect(ct_order, SIGNAL(buttonReleased(double)), SLOT(fit_ls(double)));
	connect(ct_order, SIGNAL(valueChanged(double)), SLOT(fit_ls(double)));
	
	lbl_fringe_tolerance = new QLabel(tr("Fringe Tolerance:"), this);
	lbl_fringe_tolerance->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_fringe_tolerance->setAlignment(AlignCenter|AlignVCenter);
	lbl_fringe_tolerance->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_fringe_tolerance->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	setup_GUI();
}

EditIntVeloc_Win::~EditIntVeloc_Win()
{
}

void EditIntVeloc_Win::setup_GUI()
{
		int j=0;
	int rows = 8, columns = 5, spacing = 2;
	
	QGridLayout * background = new QGridLayout(this,3,1,spacing*2);
	QGridLayout * subGrid1 = new QGridLayout(rows, columns, spacing);
	for (int i=0; i<rows; i++)
	{
		subGrid1->setRowSpacing(i, 26);
	}
	subGrid1->setColStretch(2,2);
	subGrid1->setColStretch(4,2);
	
	subGrid1->addWidget(pb_select_dir,j,0);
	subGrid1->addMultiCellWidget(lbl_directory,j,j,1,2);	
	subGrid1->addWidget(pb_exsingle,j,3);
	subGrid1->addWidget(cnt_exsingle,j,4);
	j++;
	subGrid1->addWidget(pb_details,j,0);
	subGrid1->addMultiCellWidget(lbl_details,j,j,1,2);	
	subGrid1->addWidget(pb_exrange,j,3);
	subGrid1->addWidget(cnt_exrange,j,4);
	j++;
	subGrid1->addWidget(pb_accept,j,0);
	subGrid1->addWidget(lbl_run_id1,j,1);
	subGrid1->addWidget(lbl_run_id2,j,2);	
	subGrid1->addWidget(pb_editscan,j,3);
	subGrid1->addWidget(cnt_edit,j,4);
	j++;
	subGrid1->addWidget(pb_sub_baseline,j,0);
	subGrid1->addWidget(lbl_current_cell1,j,1);
	subGrid1->addWidget(lbl_current_cell2,j,2);	
	QHBoxLayout *status_bar = new QHBoxLayout(2);
	status_bar->addWidget(lbl_status1);
	status_bar->addWidget(progress);
	subGrid1->addMultiCell(status_bar,j,j,3,4);
	j++;
	subGrid1->addWidget(pb_invert,j,0);
	subGrid1->addMultiCellWidget(lbl_instructions1,j,j+4,1,1);
	subGrid1->addMultiCellWidget(lbl_instructions2,j,j+4,2,4);
	j++;
	subGrid1->addWidget(pb_exclude_profile,j,0);
	j++;
	subGrid1->addWidget(pb_help,j,0);
	j++;
	subGrid1->addWidget(pb_close,j,0);

	QHBoxLayout * subGrid2 = new QHBoxLayout(2);
	subGrid2->addWidget(lbl_centerpiece);
	subGrid2->addWidget(cmbb_centerpiece);
	subGrid2->addWidget(lbl_rotor);
	subGrid2->addWidget(cmbb_rotor);
	
	QHBoxLayout * subGrid3 = new QHBoxLayout(2);
	subGrid3->addWidget(lbl_fringe_tolerance);
	subGrid3->addWidget(ct_fringe_tolerance);
	subGrid3->addWidget(pb_subtract);
	subGrid3->addWidget(ct_order);
	subGrid3->addWidget(pb_movie);
	subGrid3->addWidget(pb_continue);

	
	background->addWidget(edit_plt,0,0);
	background->addLayout(subGrid1,1,0);
	background->addLayout(subGrid2,2,0);	
	background->addLayout(subGrid3,3,0);
		
	qApp->processEvents();
	QRect r = background->cellGeometry(0, 0);

	global_Xpos += 30;
	global_Ypos += 30;
	
	this->setGeometry(global_Xpos, global_Ypos, r.width(), r.height()+280);
}


void EditIntVeloc_Win::setFringeTolerance(double val)
{
	fringe_tolerance = val;
	calc_integral();
}

void EditIntVeloc_Win::calc_integral()
{
	float *integral, diff1, diff2;
	int position;
	unsigned i, scan;
	integral = new float [run_inf.scans[cell][lambda]];
	for (scan=0; scan<run_inf.scans[cell][lambda]; scan++)
	{
		integral[scan] = 0;
		for (i=0; i<points[scan]; i++)
		{ // arbitrarily add 1000 fringes to each absorbance value
			integral[scan] += absorbance[scan][i] + 1000;
		} // to make sure we don't sum negatives
	}
// integral fringe shifts contribute exactly points[scan] to integral, since we use unity stepsize in integral calculation. 

	for (scan=1; scan<run_inf.scans[cell][lambda]; scan++)
	{
		position = 0;
		while (integral[scan] <= integral[scan-1])
		{
			integral[scan] += points[scan];
			position ++;
		}
		while (integral[scan] > integral[scan-1])
		{
			integral[scan] -= points[scan];
			position --;
		}
		for (i=0; i<points[scan]; i++)
		{ // add the integral steps (which may be negative!) to each datapoint
			absorbance[scan][i] += position; 
		}

		diff1 = integral[scan-1] - integral[scan];
		diff2 = integral[scan-1] - (integral[scan] + points[scan]);
		if (fabs(diff2)/fabs(diff1) < fringe_tolerance) // then the scan is one fringe too low
		{
			for (i=0; i<points[scan]; i++)
			{ // add one fringe to all absorbance values:
				absorbance[scan][i] += 1;
			}
			// update integral for this scan
			integral[scan] += points[scan];
		}
	}
	delete [] integral;
	edit_plt->clear();
	plot_dataset();
}


void EditIntVeloc_Win::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/edit_veloc_if.html");
}

void EditIntVeloc_Win::get_x(const QMouseEvent &e)
{
	unsigned int scan=0, temp_points = 0;
	QwtSymbol sym;
	sym.setStyle(QwtSymbol::Ellipse);
	sym.setPen(blue);
	sym.setBrush(white);
	sym.setSize(5);
	QwtSymbol sym1;
	sym1.setStyle(QwtSymbol::None);
		
	int x1, y1, x2, y2;

	uint line;
	unsigned int i, j, k, count=0, index1, index2;
	int  ik, jk, kk;
	double r[2], y[2], sum;
	

	current_R = (edit_plt->invTransform(QwtPlot::xBottom, e.x()));
	r[0] = current_R;
	r[1] = current_R;
	y[0] = ymin + ymax/20;
	y[1] = ymax - ymax/20;
	line = edit_plt->insertCurve("Line");
	edit_plt->setCurvePen(line, QPen(white, 2));
	edit_plt->setCurveData(line, r, y, 2);
	edit_plt->replot();
	switch (step)
	{
		case 10:	// pick baseline
		{
			scan = run_inf.scans[cell][lambda]-1;
			j=find_indexf(current_R, &radius[scan], points[scan]);
			sum = 0;
			if (j > points[scan]+4)
			{
				str.sprintf	(tr("There appears to be a problem with scan %d of your\n"), scan+1);
				str1.sprintf(tr("data that showed up in module edvint.cpp:get_x::CASE %d.\n\n"), step);
				str.append(str1);
				str.append 	(tr("Please manually inspect the file for:\n\n"));
				str1.sprintf(tr("   Scan:			%d\n"), scan+1);
				str.append(str1);
				str1.sprintf(tr("   Wavelength:	%d\n"), lambda);
				str.append(str1);
				str1.sprintf(tr("   Cell:			%d\n\n"), cell+1);
				str.append(str1);
				str.append 	(tr("to make sure it doesn't contain any corruption.\n\n"));
				str.append 	(tr("If the file doesn't have any corruption, you can try\n"));
				str.append 	(tr("to re-edit your data and pick a point that is closer\n"));
				str.append 	(tr("to the meniscus."));
				QMessageBox::message(tr("Attention:"), str);
				return;
			}
			if (j < 4)
			{
				str.sprintf	(tr("There appears to be a problem with scan %d of your\n"), scan+1);
				str1.sprintf(tr("data that showed up in module edvint.cpp:get_x::CASE %d.\n\n"), step);
				str.append(str1);
				str.append 	(tr("Please manually inspect the file for:\n\n"));
				str1.sprintf(tr("   Scan:			%d\n"), scan+1);
				str.append(str1);
				str1.sprintf(tr("   Wavelength:	%d\n"), lambda);
				str.append(str1);
				str1.sprintf(tr("   Cell:			%d\n\n"), cell+1);
				str.append(str1);
				str.append 	(tr("to make sure it doesn't contain any corruption.\n\n"));
				str.append 	(tr("If the file doesn't have any corruption, you can try\n"));
				str.append 	(tr("to re-edit your data and pick a point that is a little\n"));
				str.append 	(tr("further away from the meniscus."));
				QMessageBox::message(tr("Attention:"), str);
				return;
			}
			for (i=j-5; i<=j+5; i++)	// average 5 points to the left and 5 points to the right
			{
				sum += absorbance[scan][i];
			}
			run_inf.baseline[cell][lambda] = sum/11;
			run_inf.centerpiece[cell] = centerpiece.serial_number;
			QString filename;
			filename = USglobal->config_list.result_dir.copy();
			filename.append("/");
			filename.append(run_inf.run_id);
			str.sprintf(".veloc.%d%d",cell+1, lambda+1);
			filename.append(str);
			QFile f(filename);
			f.open( IO_WriteOnly );
			QDataStream s( &f );
/*
			float minval = 9.9e6;
			for (scan=0; scan<run_inf.scans[cell][lambda]; scan++)
			{
				for (i=0; i<points[0]; i++)
				{
					minval = min(minval, absorbance[scan][i]);
				}
			}
*/
			for (scan=0; scan<run_inf.scans[cell][lambda]; scan++)
			{
				run_inf.plateau[cell][lambda][scan] -= run_inf.baseline[cell][lambda];
				for (i=0; i<points[0]; i++)
				{
					s << (float) (absorbance[scan][i] - run_inf.baseline[cell][lambda]);
				}
			}
			run_inf.baseline[cell][lambda] = 0.0;
			f.flush();
			f.close();
			cleanup_dataset();
			load_dataset();
			return;
		}
		case 9:	// pick plateau x
		{
			for (scan=0; scan<run_inf.scans[cell][lambda]; scan++)
			{
				j=find_indexr(current_R, &radius[scan], points[scan]);
				sum = 0;
				if (j > points[scan]+4)
				{
					str.sprintf	(tr("There appears to be a problem with scan %d of your\n"), scan+1);
					str1.sprintf(tr("data that showed up in module edvint.cpp:get_x::CASE %d.\n\n"), step);
					str.append(str1);
					str.append 	(tr("Please manually inspect the file for:\n\n"));
					str1.sprintf(tr("   Scan:			%d\n"), scan+1);
					str.append(str1);
					str1.sprintf(tr("   Wavelength:	%d\n"), lambda);
					str.append(str1);
					str1.sprintf(tr("   Cell:			%d\n\n"), cell+1);
					str.append(str1);
					str.append 	(tr("to make sure it doesn't contain any corruption.\n\n"));
					str.append 	(tr("If the file doesn't have any corruption, you can try\n"));
					str.append 	(tr("to re-edit your data and pick a point that is closer\n"));
					str.append 	(tr("to the meniscus."));
					QMessageBox::message(tr("Attention:"), str);
					return;
				}
				if (j < 4)
				{
					str.sprintf	(tr("There appears to be a problem with scan %d of your\n"), scan+1);
					str1.sprintf(tr("data that showed up in module edvint.cpp:get_x::CASE %d.\n\n"), step);
					str.append(str1);
					str.append 	(tr("Please manually inspect the file for:\n\n"));
					str1.sprintf(tr("   Scan:			%d\n"), scan+1);
					str.append(str1);
					str1.sprintf(tr("   Wavelength:	%d\n"), lambda);
					str.append(str1);
					str1.sprintf(tr("   Cell:			%d\n\n"), cell+1);
					str.append(str1);
					str.append 	(tr("to make sure it doesn't contain any corruption.\n\n"));
					str.append 	(tr("If the file doesn't have any corruption, you can try\n"));
					str.append 	(tr("to re-edit your data and pick a point that is a little\n"));
					str.append 	(tr("further away from the meniscus."));
					QMessageBox::message(tr("Attention:"), str);
					return;
				}
				for (i=j-5; i<=j+5; i++)	// average 5 points to the left and 5 points to the right
				{
					sum += absorbance[scan][i];
				}
				run_inf.plateau[cell][lambda][scan] = sum/11;
//				cout << "Plateau of scan " << scan << ": " << run_inf.plateau[cell][lambda][scan] << endl;
			}
			step = 10;
			for (scan = 0; scan < run_inf.scans[cell][lambda]-1; scan++)
			{
				edit_plt->removeCurve(curve[scan]);
			}
			edit_plt->replot();
			lbl_instructions2->setText(tr("Please click on a point that describes best the baseline absorbance\n"));
			break;
		}
		case 8:
		{
			// do nothing, if user accidentally clicks in plot area, it doesn't do anything.
			break;
		}
		case 7:  // pick range-right
		{
			run_inf.range_right[cell][lambda][current_channel] = current_R;
			limit_right = current_R;
			temp_points = 0;
			for (scan=0; scan<run_inf.scans[cell][lambda]; scan++)
			{
				temp_points += points[scan];
			}
			run_inf.point_density[cell][lambda][current_channel] = run_inf.scans[cell][lambda] * (limit_right - limit_left) / temp_points;
			for (scan=0; scan<run_inf.scans[cell][lambda]; scan++)
			{
				left_index = find_indexf(limit_left, &radius[scan], points[scan]);
				right_index = find_indexr(limit_right, &radius[scan], points[scan]);
				if ((left_index < 0) || (right_index < 0))
				{
					str.sprintf	(tr("There appears to be a problem with scan %d of your\n"), scan+1);
					str1.sprintf(tr("data that showed up in module edvint.cpp:get_x::CASE %d.\n\n"), step);
					str.append(str1);
					str.append 	(tr("Please manually inspect the file for:\n\n"));
					str1.sprintf(tr("   Scan:			%d\n"), scan+1);
					str.append(str1);
					str1.sprintf(tr("   Wavelength:	%d\n"), lambda);
					str.append(str1);
					str1.sprintf(tr("   Cell:			%d\n\n"), cell+1);
					str.append(str1);
					str.append 	(tr("to make sure it doesn't contain any corruption.\n\n"));
					str.append 	(tr("If the file doesn't have any corruption, you can try\n"));
					str.append 	(tr("to re-edit your data and pick a point that is closer\n"));
					str.append 	(tr("to the meniscus."));
					QMessageBox::message(tr("Attention:"), str);
					return;
				}
				if (right_index < left_index)
				{
					QMessageBox::message(tr("Attention:"), tr("You need to pick first the left limit,\n"
																  "then the right. Please try again now..."));
					step = 6;
					return;
				}
				count=0;
				for (kk=left_index; kk<right_index; kk++)
				{
					radius[scan][count] = radius[scan][kk]; // throw away points outside the limits
					absorbance[scan][count] = absorbance[scan][kk];
					count++;
				}
//
// we probably don't need to carry a different "points" variable for each scan like in the absorbance
// editing routine, but for simplicity we duplicate the scans[i] stuff for interference
//
				points[scan]=count-1;
			}
//
// We'll use the number of points for the first scan (of this cell, wavelength and channel) to 
// be representative for the number of points in all scans in the global variable:
//

			run_inf.points[cell][lambda][current_channel] = points[0];

// find the proper integral fringe shift offsets:

			calc_integral();

// in interference scans all radial increments are equal, and we don't have to worry about
// missing datapoints:

			run_inf.delta_r = (radius[0][points[0]] - radius[0][0])/(points[0]-1);

// now set up the plot window with the appropriate buttons and counters and plot the integrals 
// and a polynomial fit to the integrals:			

			ct_order->setEnabled(true);
			ct_fringe_tolerance->setEnabled(true);
			pb_continue->setEnabled(true);
			pb_movie->setEnabled(true);
			pb_subtract->setEnabled(true);

			lbl_instructions2->setText(tr("Now Play a movie of your interference data by clicking on \"Movie\"\n"
																"(optional). If the data doesn't move with smooth transitions, select\n"
																"the order of a polynomial fit to the scan integrals. Once you found\n"
																"a smooth polynomial to fit your integrals, subtract the residuals from.\n"
																"each scan. After subtracting, you can run another movie."));
			step=8;
			break;
		}
		case 6:	// pick range-left
		{
//			i = (int) (current_R * 1000);	//make sure we only have 3 significant digits
//			limit_left = 1e-3 * (float) i;
			run_inf.range_left[cell][lambda][current_channel] = current_R;
			limit_left = current_R;
			lbl_instructions2->setText(tr("Please click near the bottom of the cell to define the right limit\n"
																"for the included data..."));
			step = 7;
			break;
		}
		case 5:	// pick air-to-air region, right limit
		{
			float sum;
			int count;
			limit_right = current_R;
			ik = find_indexf(limit_left, &radius[0], points[0]);
			jk = find_indexf(limit_right, &radius[0], points[0]);
			for (scan=1; scan<run_inf.scans[cell][lambda]; scan++)
			{

				if ((ik < 0) || (jk < 0))
				{
					str.sprintf	(tr("There appears to be a problem with scan %d of your\n"), scan+1);
					str1.sprintf(tr("data that showed up in module edvint.cpp:get_x::CASE %d.\n\n"), step);
					str.append(str1);
					str.append 	(tr("Please manually inspect the file for:\n\n"));
					str1.sprintf(tr("   Scan:			%d\n"), scan+1);
					str.append(str1);
					str1.sprintf(tr("   Wavelength:	%d\n"), lambda);
					str.append(str1);
					str1.sprintf(tr("   Cell:			%d\n\n"), cell+1);
					str.append(str1);
					str.append 	(tr("to make sure it doesn't contain any corruption.\n\n"));
					str.append 	(tr("If the file doesn't have any corruption, you can try\n"));
					str.append 	(tr("to re-edit your data and pick a point that is closer\n"));
					str.append 	(tr("to the meniscus."));
					QMessageBox::message(tr("Attention:"), str);
					return;
				}
				if (jk < ik)
				{
					QMessageBox::message(tr("Attention:"), tr("You need to pick first the left limit,\n"
																  "then the right. Please try again now..."));
					step = 4;
					return;
				}
				sum = 0;
				count = 0;
				for (kk=ik; kk<jk; kk++)
				{
					sum += absorbance[scan][kk] - absorbance[0][kk];
					count++;
				}
				sum = sum/count;
				for (k=0; k<points[scan]; k++)
				{
					absorbance[scan][k] -= sum;
				}
			}
			edit_plt->clear();
			plot_dataset();
			step = 6;
			str.sprintf(tr("Please define the upper (left) limit of the useful data range now..."));
			lbl_instructions2->setText(str);
			break;
		}
		case 4:	//pick air-to-air region, left limit
		{
			limit_left = current_R;
			str.sprintf(tr("Please define the right limit of the air-to-air region now..."));
			lbl_instructions2->setText(str);
			step = 5;
			break;
		}
		case 3:	//pick meniscus
		{
			if (zoomflag)
			{
				x1 = qwtMin(p1.x(), e.x());
				x2 = qwtMax(p1.x(), e.x());
				y1 = qwtMin(p1.y(), e.y());
				y2 = qwtMax(p1.y(), e.y());
		
				r1 = edit_plt->invTransform(QwtPlot::xBottom, x1);
				r2 = edit_plt->invTransform(QwtPlot::xBottom, x2);
				index1 = find_indexf(r1, &radius[0], points[0]);
				index2 = find_indexf(r2, &radius[0], points[0]);
				get_2d_limits(&absorbance, &od1, &od2, run_inf.scans[cell][lambda], index2, 0, index1);
				edit_plt->setAxisScale(QwtPlot::xBottom, r1, r2);
				edit_plt->setAxisScale(QwtPlot::yLeft, od1, od2);
				unsigned int temp_scan=10;
				if (temp_scan > run_inf.scans[cell][lambda])
				{
					temp_scan = run_inf.scans[cell][lambda];
				}
				edit_plt->clear();
				for (i=0; i<temp_scan; i++)
				{
					curve[i]= edit_plt->insertCurve("Zoomed Data");
					edit_plt->setCurveData(curve[i], radius[i], absorbance[i], points[i]);
					edit_plt->setCurveSymbol(curve[i], QwtSymbol(sym));
				}
				edit_plt->replot();
			}
			else
			{	
				int index;
				uint vert, horiz;
				double xv[2], yv[2], xh[2], yh[2], lengthx, lengthy, xval, yval;
//				edit_plt->closestCurve(e.x(), e.y(), dist, xval, yval, index);
				index=find_indexf(current_R, &radius[0], points[0]);				
				xval = radius[0][index];
				run_inf.meniscus[cell] = (float) radius[0][index];
				yval = od2  - (od2 - od1) / 17;
				lengthx = (r2 - r1) / 40;
				lengthy = (od2 - od1) / 17;
				xh[0] = xval - lengthx;		//horizontal
				xh[1] = xval + lengthx;
				yh[0] = yval;
				yh[1] = yval;
				horiz = edit_plt->insertCurve("horizontal");
				edit_plt->setCurvePen(horiz, QPen(red, 2));
				edit_plt->setCurveData(horiz, xh, yh, 2);
				xv[0] = xval;
				xv[1] = xval;
				yv[0] = yval - lengthy;
				yv[1] = yval + lengthy;
				vert = edit_plt->insertCurve("vertical");
				edit_plt->setCurvePen(vert, QPen(red, 2));
				edit_plt->setCurveData(vert, xv, yv, 2);
				edit_plt->replot();
			}
			break;
		}
	}
}

void EditIntVeloc_Win::movie()
{

	double ymax=-1e20, ymin=1e20;
	unsigned int i;
	uint *scn;
	
	scn = new uint [run_inf.scans[cell][lambda]];
	
	for (i=0; i<points[0]; i++)
	{
		ymax = max(ymax, absorbance[0][i]);
		ymin = min(ymin, absorbance[0][i]);
	}
	for (i=0; i<points[run_inf.scans[cell][lambda]-1]; i++)
	{
		ymax = max(ymax, absorbance[run_inf.scans[cell][lambda]-1][i]);
		ymin = min(ymin, absorbance[run_inf.scans[cell][lambda]-1][i]);
	}
	ymax += fabs(ymax-ymin)/10;
	ymin -= fabs(ymax-ymin)/10;
	
	edit_plt->clear();
	edit_plt->setAxisTitle(QwtPlot::xBottom, tr("Radius (cm)"));
	edit_plt->setAxisScale(QwtPlot::yLeft, ymin, ymax, 0);

	for (i=0; i<run_inf.scans[cell][lambda]; i++)
	{
		edit_plt->clear();
		scn[i] = edit_plt->insertCurve("Fringes");
		edit_plt->setCurvePen(scn[i], QPen(yellow));
		edit_plt->setCurveRawData(scn[i], radius[i], absorbance[i], points[0]);
		edit_plt->replot();
	}
}

void EditIntVeloc_Win::fit_ls(double value)
{
	if (value <= 0)
	{
		return;
	}
	order = (unsigned int) value;
	if (absorbance_integral != NULL)
	{
		delete [] absorbance_integral;
	}
	if (residuals != NULL)
	{
		delete [] residuals;
	}
	absorbance_integral = new double [run_inf.scans[cell][lambda]];
	residuals = new double [run_inf.scans[cell][lambda]];
	unsigned int scan, i;
	double *coeffs;
	double *fit;
	double *scan_time;
	
	US_lsfit *polyfit;
	coeffs = new double [order];
	scan_time = new double [run_inf.scans[cell][lambda]];
	fit = new double [run_inf.scans[cell][lambda]];

//
// calculate the integral of each scan which is needed for the least-squares polynomial fit to correct
// for radially invariant baseline noise. We also keep track of the total integral at each point for 
// 
//
	for (scan=0; scan<run_inf.scans[cell][lambda]; scan++)
	{
		absorbance_integral[scan] = 0;
		for (i=0; i<points[scan]; i++)
		{
			absorbance_integral[scan] += absorbance[scan][i] * run_inf.delta_r;
			if (centerpiece.sector == 3) //if synthetic boundary centerpiece, we need to integrate data for vHW
			{
				absorbance[scan][i] = absorbance_integral[scan];
			}
		}
	}
//	plot against the time of the scan:
	for (i=0; i<run_inf.scans[cell][lambda]; i++)
	{
		scan_time[i] = (double) run_inf.time[cell][lambda][i];
	}
	residuals_defined = true;
	polyfit = new US_lsfit(coeffs, scan_time, absorbance_integral, order, run_inf.scans[cell][lambda], true);

	for (scan=0; scan<run_inf.scans[cell][lambda]; scan++)
	{
		fit[scan] = 0;
		for (i=0; i<order; i++)
		{
			fit[scan] +=  coeffs[i] * pow((double) run_inf.time[cell][lambda][scan], (double) i);
		}
		residuals[scan] = absorbance_integral[scan] - fit[scan];
	}	

	uint scn[2];
	edit_plt->removeCurves();
	edit_plt->setAxisAutoScale(QwtPlot::xBottom);
	edit_plt->setAxisAutoScale(QwtPlot::yLeft);
	edit_plt->setAxisTitle(QwtPlot::xBottom, tr("Scan Time (seconds)"));
	scn[0] = edit_plt->insertCurve("Integrals");
	scn[1] = edit_plt->insertCurve("Polynomial Fit");
	edit_plt->setCurvePen(scn[0], QPen(yellow));
	edit_plt->setCurveRawData(scn[0], scan_time, absorbance_integral, run_inf.scans[cell][lambda]);
	edit_plt->setCurvePen(scn[1], QPen(magenta));
	edit_plt->setCurveRawData(scn[1], scan_time, fit, run_inf.scans[cell][lambda]);
	edit_plt->replot();
	pb_subtract->setEnabled(true);
	delete [] coeffs;
	delete [] fit;
	delete [] scan_time;
}

void EditIntVeloc_Win::next_step()
{
	ct_order->setEnabled(false);
	ct_fringe_tolerance->setEnabled(false);
	pb_continue->setEnabled(false);
	pb_movie->setEnabled(false);
	pb_subtract->setEnabled(false);
	edit_plt->clear();
	plot_dataset();
	step = 9;
	lbl_instructions2->setText(tr("Now you should subtract a BASELINE scan from your dataset by\n"
														"clicking on \"Subtract Baseline\" (optional). Then edit/delete any\n"
														"scans, if necessary. Then click on a point that best describes\n"
														"a stable plateau for all scans in this dataset. Please note that some\n"
														"analysis methods do not require a stable plateau for each scan."));
	return;
}

void EditIntVeloc_Win::subtract_residuals()
{
	if (!residuals_defined)
	{
		QMessageBox::message(tr("Attention:"), 	tr("Please define some residuals first by\n"
														"calculating a non-zero polynomial fit\n"
														"to the integrals of the scans!"));
		return;
	}
	unsigned int i, j;
	for (i=0; i<run_inf.scans[cell][lambda]; i++)
	{
		absorbance_integral[i] = 0;
		for (j=0; j<points[i]; j++)
		{
			absorbance[i][j] -= residuals[i];
			absorbance_integral[i] += absorbance[i][j] * run_inf.delta_r;
			if (centerpiece.sector == 3) //if synthetic boundary centerpiece, we need to integrate data for vHW
			{
				absorbance[i][j] = absorbance_integral[i];
			}
		}
	}
	residuals_defined = false;
}

float EditIntVeloc_Win::calc_sum(int fringe, int scan)
{
	float sum = 0;
	unsigned int i;
	for (i = (unsigned int) points[scan]/3; i < (unsigned int) 2*points[scan]/3; 
	i += (unsigned int) points[scan]/30)
	{
		sum += absorbance[scan-1][i] - absorbance[scan][i] - fringe;
	}
	return (sum);
}

void EditIntVeloc_Win::sub_baseline()
{
	if (step < 9)
	{
		QMessageBox::message(tr("Attention:"), tr("Please edit the data until prompted to\n"
		                                   "subtract a baseline scan"));
		return;
	}
	else
	{
		str.sprintf(tr("Please load a valid baseline scan to be subtracted from each scan..."));
		lbl_instructions2->setText(str);
		load_base();
	}
}

void EditIntVeloc_Win::load_base()
{
	QString fn = QFileDialog::getOpenFileName(USglobal->config_list.data_dir, "*", 0);
	if ( !fn.isEmpty() ) 
	{
		load_base(fn);
	}
}

void EditIntVeloc_Win::load_base(const char *fileName)
{
	unsigned int i, j;
	int ik;
	float base_rad, *base, sum=0;
	base = new float [points[0]];
	QFile f(fileName);
	if (f.open(IO_ReadOnly))
	{
		QTextStream ts(&f);
		for (ik=0; ik<left_index+1; ik++)	//read the first 2 headerlines, and then everything up to the left index (left_index+2-1)
		{
			if (!ts.eof())
			{
				ts.readLine();
			}
		}
		i = 0;
		while (!ts.eof() && i < points[0])
		{
			ts >> base_rad;
			ts >> base[i];
			i++;
		}
		f.close();
	}
	for (j=0; j<points[0]; j++)
	{
		sum += base[j];
	}
	sum = sum/points[0];
	for (j=0; j<points[0]; j++)
	{
		base[j] -= sum;
	}
	for (i=0; i<run_inf.scans[cell][lambda]; i++)
	{
		for (j=0; j<points[0]; j++)
		{
			absorbance[i][j] -= base[j];
		}
	}
	plot_dataset();
	str.sprintf(tr("Next, edit or delete any scans, if necessary. Then click on a point\n"
						"that best describes a stable plateau for all scans in this dataset.\n"
						"Please note that some analysis methods do not require a stable\n"
						"plateau for each scan."));
	lbl_instructions2->setText(str);
}

