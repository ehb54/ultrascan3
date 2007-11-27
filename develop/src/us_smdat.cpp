#include "../include/us_smdat.h"

sm_dat_W::sm_dat_W(QWidget *p, const char *name) : Data_Control_W(19, p, name)
{
	baseline_flag = true;
	pm = new US_Pixmap();
	pb_second_plot->setText("Reset Data");
	fn = "";
	setup_GUI();
}

sm_dat_W::~sm_dat_W()
{
}

void sm_dat_W::setup_GUI()
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
		
	rows = 9, columns = 4, spacing = 2, j=0;
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


void sm_dat_W::second_plot()
{
	reset();
}

void sm_dat_W::save()
{
	if (step == 0)
	{
		QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n	   Click on \"Load Data\"\n"));
		return;
	}
	write_sm();
	write_res();
	QPixmap p;
	QString fileName, sys;
	QFile f;
	fileName.sprintf(htmlDir + "/sm_%d%d.", selected_cell + 1, selected_lambda +1);
	p = QPixmap::grabWidget(analysis_plot, 2, 2, analysis_plot->width() - 4, analysis_plot->height() - 4);
	pm->save_file(fileName, p);
	fileName.sprintf(htmlDir + "/sm_edited_%d%d.", selected_cell + 1, selected_lambda +1);
	p = QPixmap::grabWidget(edit_plot, 2, 2, edit_plot->width() - 4, edit_plot->height() - 4);
	pm->save_file(fileName, p);
}

void sm_dat_W::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/sm.html");
}

int sm_dat_W::plot_analysis()
{
	QwtSymbol sym;
	already = true;
	if (setups() < 0)
	{
		return(-1);
	}
	if (!plateaus_corrected)
	{
		find_plateaus();
	}
	double sum1, sum2, dcdr, span, testy;
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		sum1 = 0.0;
		sum2 = 0.0;
		count=1;
//
// the span is the boundary portion that is going to be analyzed (in percent).
//
		span = run_inf.plateau[selected_cell][selected_lambda][i] * bd_range/100;
		testy = run_inf.plateau[selected_cell][selected_lambda][i] * bd_position/100;
		while (absorbance[i][count] < testy)
		{
			count++;
		}
		while (count < points-1 && absorbance[i][count] < (testy + span))
		{
//cout << "A:[" << count << "]: " << absorbance[i][count] << ", " <<  (testy + span) << endl;
			dcdr = (absorbance[i][count] - absorbance[i][count-1])/run_inf.delta_r;
			sum1 += pow(radius[count], 2) 
			* dcdr * run_inf.delta_r;
			sum2 += dcdr * run_inf.delta_r;
			count++;		// climb up the concentration gradient until the plateau is reached
		}
		smp[i] = sqrt(sum1/sum2); //second moment points in cm
		sms[i] = 1e13 * correction * log (smp[i]/run_inf.meniscus[selected_cell]) 
		/ (omega_s * run_inf.time[selected_cell][selected_lambda][i]); //second moment s
//cout << sms[i] << ", span: " << span << ", count: " << count << ", points: " << points << ", A[" << i << "]: " << absorbance[i][count] << endl;
//
//update the progress bar:
// 
		progress->setProgress(i+1);
	}
	analysis_plot->clear();
/*
	analysis_plot->enableGridXMin();
	analysis_plot->enableGridYMin();
	analysis_plot->setGridMajPen(QPen(colorGroup().light(), 0, DotLine));
	analysis_plot->setGridMinPen(QPen(colorGroup().mid(), 0 , DotLine));
*/
	QString title;
	title.sprintf(tr("Run ") + run_inf.run_id + tr(": Cell %d (%d nm) - Second Moment Plot"),
	selected_cell + 1, run_inf.wavelength[selected_cell][selected_lambda]);
	analysis_plot->setTitle(title);
	analysis_plot->setAxisTitle(QwtPlot::xBottom, tr("Scan Number"));
	analysis_plot->setAxisTitle(QwtPlot::yLeft, tr("Corrected Sed. Coeff. (1e-13 s)"));

//
// sedimentation coefficients from all scans that have not cleared the meniscus
// form a  separate plot that will be plotted in red, and will not be included 
// in the line fit:
//
	for (i=0; i<exclude; i++)
	{
		plot1_x[i] = (double) (i + 1);
		plot1_y[i] = sms[i];
	}
//
// All others go into plot 2:
//
	avg = 0;
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]-exclude; i++)
	{
		plot2_x[i] = (double) (i + exclude + 1);
		plot2_y[i] = sms[i+exclude];
		avg += sms[i+exclude];
	}
	avg = avg/(run_inf.scans[selected_cell][selected_lambda] - exclude);
//
// Plot 3 is a constant line of the average of all scans included
//
	plot3_x[0] = 0.0;
	plot3_x[1] = run_inf.scans[selected_cell][selected_lambda];
	plot3_y[0]=avg;
	plot3_y[1]=avg;
	sym.setStyle(QwtSymbol::Ellipse);
	sym.setPen(white);
	sym.setBrush(red);
	sym.setSize(8);
	plot1 = analysis_plot->insertCurve("non-cleared Sedimentation Coefficients");
	analysis_plot->setCurveStyle(plot1, QwtCurve::Lines);
	analysis_plot->setCurveSymbol(plot1, QwtSymbol(sym));
	analysis_plot->setCurveData(plot1, plot1_x, plot1_y, exclude);

	sym.setStyle(QwtSymbol::Ellipse);
	sym.setPen(blue);
	sym.setBrush(white);
	sym.setSize(8);
	plot2 = analysis_plot->insertCurve("cleared Sedimentation Coefficients");
	analysis_plot->setCurveStyle(plot2, QwtCurve::Lines);
	analysis_plot->setCurveSymbol(plot2, QwtSymbol(sym));
	analysis_plot->setCurvePen(plot2, yellow);
	analysis_plot->setCurveData(plot2, plot2_x, plot2_y, run_inf.scans[selected_cell][selected_lambda]-exclude);

	plot3 = analysis_plot->insertCurve("Average");
	analysis_plot->setCurveStyle(plot3, QwtCurve::Lines);
	analysis_plot->setCurvePen(plot3, yellow);
	analysis_plot->setCurveData(plot3, plot3_x, plot3_y, 2);
	analysis_plot->replot();
	//analysis_plot->updatePlot();		//no updatePlot() in new version
	return(0);
}

int sm_dat_W::setups()
{
	QString str;
	double testy;
// 
// make sure that the first datapoint of each scan has a lower concentration than 
// what would be needed for the first division:
//
	exclude=0;
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		testy = run_inf.plateau[selected_cell][selected_lambda][i] * bd_position/100;
		if (absorbance[i][0] > testy)
		{
			exclude++;
		}
	}
	if (exclude > 0)
/*	{
		QMessageBox::message( "Warning", "Attention:\n"
									 "One or more scans have not cleared the meniscus (as defined\n"
									 "by 1/2000 of the total concentration).\n\n"
									 "Scans that do not clear the meniscus will provide unreliable\n"
									 "sedimentation coeffficients (generally too high). These\n"
									 "scans will be plotted in red color.\n\n"
									 "You can try to increase the length of the experiment\n"
									 "or use a higher speed for the same sedimentation coefficient.\n"
									 "Alternatively, you can increase the sedimentation coefficient\n"
									 "and decrease the diffusion coefficient for the slowest component(s).");
	}
*/
	lbl2_excluded->setText(str.sprintf("%d", exclude));

	progress->setTotalSteps(run_inf.scans[selected_cell][selected_lambda]);
	progress->reset();
	progress->setProgress(0);
	smp = new double [run_inf.scans[selected_cell][selected_lambda]];
	sms = new double [run_inf.scans[selected_cell][selected_lambda]];
	plot1_x = new double [exclude];
	plot1_y = new double [exclude];
	plot2_x = new double [run_inf.scans[selected_cell][selected_lambda]-exclude];
	plot2_y = new double [run_inf.scans[selected_cell][selected_lambda]-exclude];
	plot3_x = new double [2];
	plot3_y = new double [2];
	return(0);
}

void sm_dat_W::view()
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
	filestr.append(".sm_res");
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

void sm_dat_W::write_sm()
{
	QString filestr, filename, temp;
	filename = USglobal->config_list.result_dir.copy();
	filename.append("/");
	filename.append(run_inf.run_id);
	temp.sprintf(".%d%d", selected_cell+1, selected_lambda+1);
	filestr = filename.copy();
	filestr.append(".sm_dat");
	filestr.append(temp);
	QFile sm_f(filestr);
	unsigned int i;	
	if (sm_f.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&sm_f);
//
// Arrange the file as follows: First, write to the file all apparent sedimentation
// values, for each scan time one row which contains all divisions, starting at the 
// lowest concentration. The first column has the times of each scan, then follow
// the sedimentation coefficients for each division for that time:
//
		for (i=exclude; i<run_inf.scans[selected_cell][selected_lambda]; i++)
		{	
			ts << i << "\t";
			ts << sms[i-exclude] << "\t";
			ts << smp[i-exclude] << "\n";
		}
		sm_f.close();
	}
}

void sm_dat_W::write_res()
{
	QString filestr, filename, temp;
	filename = USglobal->config_list.result_dir.copy();
	filename.append("/");
	filename.append(run_inf.run_id);
	temp.sprintf(".%d%d", selected_cell+1, selected_lambda+1);
	filestr = filename.copy();
	filestr.append(".sm_res");
	filestr.append(temp);
	QFile res_f(filestr);
	if (res_f.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&res_f);
		ts << "***************************************************\n";
		ts << tr("*             Second Moment Analysis              *\n");
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
		ts << tr("Early Scans skipped:     ") << exclude << " Scans \n";
		ts << tr("Average Second Moment S: ") << avg << "s * 10e-13\n";
		if(plateaus_corrected)
		{
			ts << tr("Weight-average S from plateau fit: ") << weight_average_s << " s\n";
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
		res_f.close();
	}
}



