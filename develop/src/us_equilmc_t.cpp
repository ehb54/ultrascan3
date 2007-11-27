#include "../include/us_equilmc_t.h"
#include <stdio.h>

//US_EquilMC_T::US_EquilMC_T(QWidget *p , const char *name) : QFrame(p, name)
US_EquilMC_T::US_EquilMC_T()
{
	USglobal = new US_Config();
	dataset = 0;
	scans_in_list = 0;
	od_limit = 0.9;
	scans_have_problems = false;
	show_messages = false;
	scans_initialized = false;
	projectName = "";
	cp_list.clear();
	rotor_list.clear();
	if (!readCenterpieceInfo(&cp_list))
	{
		QString str	= tr("UltraScan Fatal Error:")
						+ tr("There was a problem opening the\ncenterpiece database file:\n\n")
						+ USglobal->config_list.system_dir + "/etc/centerpiece.dat\n\n"
						+ tr("Please install the centerpiece database file\nbefore proceeding.");
		cout << str.latin1() << endl;
		exit(-1);
	}
	if (!readRotorInfo(&rotor_list))
	{
		QString str	= tr("UltraScan Fatal Error: ") 
						+ tr("There was a problem opening the\nrotor database file:\n\n")
						+ USglobal->config_list.system_dir + "/etc/rotor.dat\n\n"
						+ tr("Please install the rotor database file\nbefore proceeding.");
		cout << str.latin1() << endl;
		exit(-1);
	}
}

US_EquilMC_T::~US_EquilMC_T()
{
}

void US_EquilMC_T::read_init(const QString &filename)
{
//	QString str=USglobal->config_list.root_dir + "/beowulf.start";
	QFile f(filename);
	QString str;
	mc.parameter.clear();
	if(f.open(IO_ReadOnly))
	{
		QTextStream ts(&f);
		
		mc.fitName = ts.readLine();
		
		str = ts.readLine(); // 0=gaussian, 1=bootstrap, 2=mixed 
		mc.data_type = str.toInt(); // 0=gaussian, 1=bootstrap, 2=mixed 
		
		str = ts.readLine();
		mc.iterations = str.toUInt();
		
		mc.filename = ts.readLine();
		
		str = ts.readLine();
		mc.append = (bool) str.toInt();
		
		str = ts.readLine();		// true = run, false = stop
		mc.status = (bool) str.toInt();
		
		str = ts.readLine();
		mc.addNoise = str.toInt();
		
		str = ts.readLine();
		mc.percent_bootstrap = str.toInt();
		
		str = ts.readLine();
		mc.percent_gaussian = str.toInt();
		
		str = ts.readLine();
		mc.rule = str.toInt();
		
		str = ts.readLine();
		mc.varianceThreshold = str.toFloat();

		str = ts.readLine();
		mc.parameterNoise = str.toFloat();

		mc.run_id = ts.readLine();

		str = ts.readLine();
		mc.parameters = str.toUInt();
		for (unsigned int i=0; i<mc.parameters; i++)
		{
			str = ts.readLine();
			mc.parameter.push_back(str);
		}
		f.close();
		loadfit(mc.fitName);
		//cout<<"Init Finish"<<endl;
	}
	else
	{
		QString str1 = tr("Attention: Unable to read the Beowulf Start File:\n\n")
		+ str + tr("\n\nPlease make sure you have write permission.\n");
		cout << str1.latin1();
	}
}

void US_EquilMC_T::load(const QString &filename)
{
	unsigned int i, j, k, l;
	QString version, str;
	QFile f;
	int et;
	if ( !filename.isEmpty() ) 
	{
		QFile f(filename);
		if (!f.open(IO_ReadOnly))
		{
			cout << "\n\nFatal Error: The equilibrium file:\n            " << filename << 
			"\ncannot be found. Please check to make sure the file is available...\n\nAborting...\n";
			exit(-1);
		}
		QDataStream ts (&f);
		ts >> version;
		if (version.toFloat() < 6.0)
		{
			cout << tr("These data were edited with a previous release\n"
							"of UltraScan (version < " + US_Version + "), which is not\n"
							"binary compatible with the current version\n\n"
							"Please re-edit the experimental data before\n"
							"using the data for data analysis.\n\n");
			f.close();
			return;
		}
		ts >> run_inf.data_dir;
		ts >> run_inf.run_id;
		ts >> run_inf.duration;
		ts >> run_inf.total_scans;
		ts >> run_inf.delta_r;
		ts >> run_inf.expdata_id;
		ts >> run_inf.investigator;
		ts >> run_inf.date;
		ts >> run_inf.description;
		ts >> run_inf.dbname;
		ts >> run_inf.dbhost;
		ts >> run_inf.dbdriver;
		ts >> et;
		run_inf.exp_type.velocity = (bool) et;
		ts >> et;
		run_inf.exp_type.equilibrium = (bool) et;
		ts >> et;
		run_inf.exp_type.diffusion = (bool) et;
		ts >> et;
		run_inf.exp_type.simulation = (bool) et;
		ts >> et;
		run_inf.exp_type.interference = (bool) et;
		ts >> et;
		run_inf.exp_type.absorbance = (bool) et;
		ts >> et;
		run_inf.exp_type.fluorescence = (bool) et;
		ts >> et;
		run_inf.exp_type.intensity = (bool) et;
		ts >> et;
		run_inf.exp_type.wavelength = (bool) et;

		for (i=0; i<8; i++)
		{
			ts >> run_inf.centerpiece[i];
			ts >> run_inf.cell_id[i];
			ts >> run_inf.wavelength_count[i];
		}
		for (i=0; i<8; i++)
		{
			if (version.toFloat() < 7.0)
			{
				for (j=0; j<4; j++)	// one for each channel
				{
					ts >> run_inf.DNA_serialnumber[i][j][0];
					ts >> run_inf.buffer_serialnumber[i][j];
					ts >> run_inf.peptide_serialnumber[i][j][0];
				}
			}
			else
			{
				for (j=0; j<4; j++)	
				{
					ts >> run_inf.buffer_serialnumber[i][j];
					for(int k=0; k<3; k++)
					{
						ts >> run_inf.peptide_serialnumber[i][j][k];
						ts >> run_inf.DNA_serialnumber[i][j][k];
					}
				}
			}

			for (j=0; j<3; j++)
			{
				ts >> run_inf.wavelength[i][j];
				ts >> run_inf.scans[i][j];
				if (run_inf.centerpiece[i] >= 0 && run_inf.scans[i][j] > 0)
				{
					for (k=0; k<cp_list[run_inf.centerpiece[i]].channels; k++)
					{
						ts >> run_inf.range_left[i][j][k];	// we don't need individual scan ranges
						ts >> run_inf.range_right[i][j][k];	// because scan limits where picked together
						ts >> run_inf.points[i][j][k];
						ts >> run_inf.point_density[i][j][k];
					}
				}
			}
		}
		run_inf.temperature = new float** [8];
		run_inf.rpm = new unsigned int** [8];
		run_inf.time		= new unsigned int** [8];
		run_inf.omega_s_t   = new float** [8];
		for (i=0; i<8; i++)
		{
			run_inf.temperature[i] = new float* [run_inf.wavelength_count[i]];
			run_inf.rpm[i] = new unsigned int* [run_inf.wavelength_count[i]];
			run_inf.time[i]		= new unsigned int* [run_inf.wavelength_count[i]];
			run_inf.omega_s_t[i]   = new float* [run_inf.wavelength_count[i]];
			for (j=0; j<run_inf.wavelength_count[i]; j++)
			{
				run_inf.temperature[i][j] = new float [run_inf.scans[i][j]];
				run_inf.rpm[i][j] = new unsigned int [run_inf.scans[i][j]];
				run_inf.time[i][j]		= new unsigned int [run_inf.scans[i][j]];
				run_inf.omega_s_t[i][j]   = new float [run_inf.scans[i][j]];
			}
		}
		for (i=0; i<8; i++)
		{
			for (j=0; j<run_inf.wavelength_count[i]; j++)
			{
				for (k=0; k<run_inf.scans[i][j]; k++)
				{
					ts >> run_inf.rpm[i][j][k];
					ts >> run_inf.temperature[i][j][k];
					ts >> run_inf.time[i][j][k];
					ts >> run_inf.omega_s_t[i][j][k];
/*
cout << "i: " << i << ", j: " << j << ", k: " << k << ", rpm: " <<  
run_inf.rpm[i][j][k] << ", temperature: " << run_inf.temperature[i][j][k] << ", time: " << 
run_inf.time[i][j][k] << ", omega: " << run_inf.omega_s_t[i][j][k] << endl;
*/
				}
			}
		}
		ts >> run_inf.rotor;
		f.close();
		for (i=0; i<8; i++)
		{
			bottom[i] = 0;
			has_data[i]=false;
			if (run_inf.scans[i][0] != 0)		// check which cell has data in it
			{
				has_data[i] = true;
				for (j=0; j<3; j++)
				{
					if (run_inf.scans[i][j] != 0 && run_inf.centerpiece[i] >= 0)		// for each cell and each wavelength, check if there are 
					{											// multiple channels
						for (l=0; l<cp_list[run_inf.centerpiece[i]].channels; l++)
						{
							int result = assign_scanfit(i, j, l);
//	cout << "result: " << result << endl;
							if (result < 0)
							{
								return;
							}
						}
					}
				}
			}
		}
		update_limit(od_limit);
		dataset++;										// we got another dataset that can be unloaded
		filenames.push_back(filename);			// add the file name of the dataset
		run_information.push_back(run_inf);		// save the dataset's run information
	}
	else
	{
		str = tr("The requested datafile:\n\n\"") + filename + tr("\"\n\n"
				"is not available.\n\nPlease make sure all associated datafiles\n"
				"are located in the result directory\nbefore proceeding.");
		cout << str.latin1() << endl;
		return;
	}
}

// sets up Scan fit vector with all scans that are loaded
int US_EquilMC_T::assign_scanfit(unsigned int i, unsigned int j, unsigned int l)
{
	float tempy;
	QString filename, str;
	QFile datafile;
	QDataStream s;
	filename = USglobal->config_list.result_dir.copy();
	filename.append("/");
	filename.append(run_inf.run_id);
	str.sprintf(".equil.%d%d%d",i+1, j+1, l+1);
	filename.append(str);
	datafile.setName(filename);
	if (!datafile.open(IO_ReadOnly))
	{
		QString str1;
		str1.sprintf(tr("Sorry, a requested file is missing:\n\n"));
		str1.append(filename);
		str1.append(tr("\n\nPlease restore the file, then try again."));
		cout << tr("Attention:\n") << str1.latin1() << endl;
		return(-1);
	}
	s.setDevice(&datafile);
	for (unsigned int k=0; k<run_inf.scans[i][j]; k++)
	{
		struct EquilScan_FitProfile scanfit;
		scans_in_list++;
		QString temp1, temp2;
		temp1 = temp2.sprintf("%d. ", scans_in_list) + run_inf.run_id 
		+ temp2.sprintf(tr(": Cell %d, Channel %d, %d nm, %4.2f ºC, %u rpm, "), i+1, l+1, 
		run_inf.wavelength[i][j], run_inf.temperature[i][j][k], run_inf.rpm[i][j][k]) + run_inf.cell_id[i];
		scan_info.push_back(temp1);
		scanfit.points =(unsigned int) ((run_inf.range_right[i][j][l] - run_inf.range_left[i][j][l])/run_inf.delta_r + 1);
		scanfit.start_index = 0;
		scanfit.stop_index = scanfit.points - 1;
		scanfit.cell = i;
		scanfit.lambda = j;
		scanfit.channel = l;
		s >> scanfit.meniscus;
		for (unsigned int m=0; m<scanfit.points; m++)
		{
			s >> tempy;
			scanfit.y.push_back(tempy);
		}
		for (unsigned int m=0; m<scanfit.points; m++)
		{
			scanfit.x.push_back(run_inf.range_left[i][j][l] + m * run_inf.delta_r);
		}
		scanfit.wavelength = run_inf.wavelength[i][j];
		scanfit.rpm = run_inf.rpm[i][j][k];
		scanfit.temperature = run_inf.temperature[i][j][k];
		scanfit.run_id = run_inf.run_id;
		scanfit.description = run_inf.cell_id[i];
		scanfit.rotor = run_inf.rotor;
		scanfit.centerpiece = run_inf.centerpiece[i];
		scanfit.FitScanFlag = false;
		scanfit.limits_modified = false;
		scanfit_vector.push_back(scanfit);
	}
	datafile.close();
	return(scans_in_list);
} 

void US_EquilMC_T::update_projectName(const QString &str)
{
	projectName = str.stripWhiteSpace();
}

bool US_EquilMC_T::createHtmlDir()
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

// Function will update the stop index to reflect the od_max setting, it is called
// each time the od_limit is adjusted.
void US_EquilMC_T::update_limit(float new_limit) // each time a new od_limit is entered, a signal is connected to this slot to truncate data to desired OD
{
	int i;
	if (scans_in_list > 0 && new_limit != 0)
	{
		for (vector<struct EquilScan_FitProfile>::iterator sfv_it = scanfit_vector.begin(); sfv_it != scanfit_vector.end(); sfv_it++)
		{
			vector<float>::iterator yval_it = (*sfv_it).y.begin();
			i = 0;
			while ((yval_it != (*sfv_it).y.end()-1) && (*yval_it < new_limit))
			{
				yval_it++;
				i++;
			}
			(*sfv_it).stop_index = i;
		}
	}
	else	// limit is = 0, i.e., no limits on the data, and stop_index needs to be set to the
	{		// last element which is equal to the number of points in the scan minus one.
		for (vector<struct EquilScan_FitProfile>::iterator sfv_it = scanfit_vector.begin(); sfv_it != scanfit_vector.end(); sfv_it++)
		{
			(*sfv_it).stop_index = (*sfv_it).points - 1;
		}
	}
}
void US_EquilMC_T::fitcontrol()
{
/*
// uncomment for GUI viewing of data:
	QString model_id;
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
	model_control = new US_EqModelControl(&scanfit_vector, &runfit, model, &model_widget, &selected_scan);
	model_control->show();
*/
	eqFitter = new US_EqFitter(&scanfit_vector,
										&run_information,
										&runfit,
										&model_control,
										model,
										false, //dont show GUI
										&scan_info,
										&fitting_widget, &projectName, od_limit);
	eqFitter->autoconverge = true;
	if(model == 3)
	{
		eqFitter->update_nlsMethod(5); //select General Least Squares
	}
}

void US_EquilMC_T::monte_carlo()
{
	QString str;
	unsigned int i, j, k=0;
	for (i=0; i<scanfit_vector.size(); i++)
	{	
		if (scanfit_vector[i].FitScanFlag)
		{
			for(j=scanfit_vector[i].start_index; j<scanfit_vector[i].stop_index; j++)
			{
				original_data.push_back(scanfit_vector[i].y[j]);	// needed for bootstrap
				residuals.push_back(scanfit_vector[i].y[j] - yfit[k]); // needed for bootstrap
				k++;
			}
		}
	}
}

void US_EquilMC_T::monte_carlo_iterate()
{
//cout << "Monte Carlo rule: " << mc.rule << endl;
	QString str;
	float sigma = 0, *std_dev;
	double *temp_parameters;
	temp_parameters = new double [parameters];
	int now = 0;
	unsigned int point_counter=0, current_point, i, j, k=0;
	float error_count = 1, success_count = 1;
	
	QTime t;
	t = QTime::currentTime();
	now = t.msec() + t.second() * 1000 + t.minute() * 60000 + t.hour() * 3600000;
	mc.random_seed = 	now;
	mc.random_seed -= (int) getpid();
	srand(mc.random_seed);	//randomize the box muller function
	if (!fitting_widget)
	{
		fitcontrol();
	}
	// keep a copy of the original fit parameters to be used as initial guesses for each new 
	// Monte Carlo iteration:
	eqFitter->guess_mapForward(temp_parameters);
	unsigned int iteration = 1;
	QString seed_str;
	QFile mc_f(mc.filename + "." + seed_str.sprintf("%d", mc.random_seed));
	if (!mc.append)
	{
		mc_f.remove();
	}
	QTextStream ts(&mc_f);
	ts.width(14);
	ts.flags(0x1000);

	if(mc.rule > 2) // then we need to initialize a temporary array with the smoothed SD's
	{
		j = residuals.size();
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
			std_dev[i] = fabs(residuals[i]);	// only send in the absolute values of the std. deviations
		}
//	cout << "Residuals size: " << j << endl;
		gaussian_smoothing(&std_dev, k, j);
		for (i=0; i<j; i++)
		{
			if (residuals[i] < 0)
			{
				std_dev[i] *= -1.0;	// correct the sign of the residuals after averaging
			}
		}
	}
	//cout<<"iteration= "<<iteration<<" mc.iterations= "<<mc.iterations<<endl;
	while (iteration <= mc.iterations)
	{
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
		switch (mc.data_type)
		{

			case 0:	// use normal Gaussian random residuals
			{
				point_counter = 0;
				for (i=0; i<scanfit_vector.size(); i++)
				{
					if (scanfit_vector[i].FitScanFlag)
					{
						for(j=scanfit_vector[i].start_index; j<scanfit_vector[i].stop_index; j++)
						{
							switch(mc.rule)
							{
								case 0:
								{
									sigma = residuals[point_counter];
									break;
								}
								case 1:
								{
									sigma = max(std_deviation, residuals[point_counter]);
									break;
								}
								case 2:
								{
									sigma = std_deviation;
									break;
								}
							}
							if (mc.rule > 2 && mc.rule < 9)
							{
								sigma = std_dev[point_counter];
							}
							scanfit_vector[i].y[j] = yfit[point_counter] + box_muller(0, sigma);
							point_counter++;
						}
					}
				}
				break;
			}
			case 1:	// use original residuals reordered in bootstrapped fashion
			{
// This algorithm randomizes the order of the residuals of the original fit and applies the absolute
// magnitude of the residuals to different points than where they were originally.
				point_counter = 0;
				for (i=0; i<scanfit_vector.size(); i++)
				{
					if (scanfit_vector[i].FitScanFlag)
					{
						for(j=scanfit_vector[i].start_index; j<scanfit_vector[i].stop_index; j++)
						{
// First, find a random point between the first point of the first scan and the last point of the last scan
							current_point = (unsigned int) (rand()/(RAND_MAX + 1.0) * residuals.size());
						
// use the magnitude of the residual of the original fit and add it to the fitted solution:
							scanfit_vector[i].y[j] = yfit[point_counter] + residuals[current_point];

// if the point doesn't fall within the percentage of the bootstrapped points, use the original residual instead:
							if (rand()/(RAND_MAX + 1.0) > (mc.percent_bootstrap/100.0))
							{
								scanfit_vector[i].y[j] = original_data[point_counter];
							}
							point_counter++;
						}
					}
				}
				break;
			}
			case 2:	// use a mixture of random Gaussian residuals and bootstrapped residuals:
			{
				point_counter = 0;
				for (i=0; i<scanfit_vector.size(); i++)
				{
					if (scanfit_vector[i].FitScanFlag)
					{
						for(j=scanfit_vector[i].start_index; j<scanfit_vector[i].stop_index; j++)
						{
							if (rand()/(RAND_MAX + 1.0) > (mc.percent_gaussian/100.0))
							{
								switch(mc.rule)
								{
									case 0:
									{
										sigma = residuals[point_counter];
										break;
									}
									case 1:
									{
										sigma = max(std_deviation, residuals[point_counter]);
										break;
									}
									case 2:
									{
										sigma = std_deviation;
										break;
									}
								}
								if (mc.rule > 2 && mc.rule < 9)
								{
									sigma = std_dev[point_counter];
								}
								scanfit_vector[i].y[j] = yfit[point_counter] + box_muller(0, sigma);
							}
							else
							{
// First, find a random point between the first point of the first scan and the last point of the last scan
								current_point = (unsigned int) (rand()/(RAND_MAX + 1.0) * residuals.size());
						
// use the magnitude of the residual of the original fit and add it to the fitted solution:
								scanfit_vector[i].y[j] = yfit[point_counter] + residuals[current_point];

// if the point doesn't fall within the percentage of the bootstrapped points, use the original residual instead:
								if (rand()/(RAND_MAX + 1.0) > (mc.percent_bootstrap/100.0))
								{
									scanfit_vector[i].y[j] = original_data[point_counter];
								}
							}
							point_counter++;
						}
					}
				}
				break;
			}
		}
		if (model == 3)
		{
			eqFitter->nlsMethod = 5;
		}
		else
		{
			eqFitter->nlsMethod = 0;
		}
		eqFitter->startFit();
		if(eqFitter->return_value < 0)
		{
			error_count++;
			if(success_count/error_count < 0.01)
			{
				cout<<"There are problems in Fit()"<<endl;
				cout << "There were " << error_count << " errors and " << success_count << " successes.\n";
				exit(-1);
			}
		}
		else
		{
			success_count ++;
		}
		if(eqFitter->return_value == 0)
		{
			error_count=0;
			iteration ++;
			mc_f.open(IO_WriteOnly | IO_Append);
			ts << tr("Iteration ") << iteration << " (" << mc.random_seed << "): " << eqFitter->variance << " ";
			for (k=0; k<runfit.components; k++)
			{
				if (runfit.mw_fit[k])
				{
					ts << runfit.mw[k] << " ";
				}
				if (runfit.vbar20_fit[k])
				{
					ts << runfit.vbar20[k] << " ";
				}
				if (runfit.virial_fit[k])
				{
					ts << runfit.virial[k] << " ";
				}
			}
			for (j=0; j<scanfit_vector.size(); j++)
			{
				if (scanfit_vector[j].FitScanFlag)
				{
					for (k=0; k<runfit.components; k++)
					{
						if (scanfit_vector[j].amplitude_fit[k])
						{
							ts << scanfit_vector[j].amplitude[k] << " ";
						}
					}
					if (scanfit_vector[j].baseline_fit)
					{
						ts << scanfit_vector[j].baseline << " ";
					}
				}
			}
			for (k=0; k<runfit.association_constants; k++)
			{
				if (runfit.eq_fit[k])
				{
					ts << runfit.eq[k] << " ";
				}
			}
		
			ts << endl;
			mc_f.flush();
			mc_f.close();
		}
	}
	mc.random_seed = 0;
	delete [] temp_parameters;
	if(mc.rule > 2) // then we need to delete the temporary array with the smoothed SD's
	{
		delete [] std_dev;
	}
	exit(0);
}

void US_EquilMC_T::loadfit(const QString &filename)
{
	parameters = 0;
	yfit.clear();
	QFile f(filename);
	QString str;
	Q_UINT16 int16;
	Q_UINT32 int32;
	show_messages = false; // when we load the data we don't want to see the run details.
	float val;
	if (f.open(IO_ReadOnly))
	{
		QDataStream ds(&f);
		ds >> int16;	// number of runs
		for (unsigned int i=0; i<(unsigned int) int16; i++)
		{
			ds >> str;
			load(USglobal->config_list.result_dir + "/" + str + ".us.e");
		}
		ds >> int16;	// model number
		model = (int) int16;
		ds >> projectName;
		ds >> val;
		std_deviation = val;
		ds >> int16;
		if (int16 != scanfit_vector.size())
		{
			cout << tr("Attention:\n") << tr("The number of scans loaded does not match\n")
			<< tr("the number of scans in the saved fit.\n\n")
			<< tr("Please make sure the original datafiles that\n")
			<< tr("were associated with this fitting project are\n")
			<< tr("available in the results directory before\n")
			<<	tr("proceeding.\n\n");
			return;
		}
		ds >> int16;
		runfit.components = (unsigned int) int16;
		ds >> int16;
		runfit.association_constants = int16;
		setup_vectors();
		float_parameters = false;  // this boolean will be toggled to "float" in the next call to float_all()
		show_messages =  false; // we don't want to see the message
		for (unsigned int i=0; i<scanfit_vector.size(); i++)
		{
			ds >> int16;
			scanfit_vector[i].FitScanFlag = (bool) int16;
			ds >> int16;
			scanfit_vector[i].autoExcluded = (bool) int16;
			ds >> int32;
			scanfit_vector[i].start_index = (unsigned int) int32;
			ds >> int32;
			scanfit_vector[i].stop_index = (unsigned int) int32;
			ds >> int16;
			scanfit_vector[i].limits_modified = (bool) int16;
			ds >> val;
			scanfit_vector[i].density = val;
			ds >> val;
			scanfit_vector[i].pathlength = val;
			ds >> int16;
			scanfit_vector[i].baseline_fit = (bool) int16;
			if (scanfit_vector[i].baseline_fit)
			{
				parameters++;
			}
			ds >> val;
			scanfit_vector[i].baseline = val;
			ds >> val;
			scanfit_vector[i].baseline_range = val;
			ds >> int16;
			scanfit_vector[i].baseline_bound = (bool) int16;
			for (unsigned int j=0; j<runfit.components; j++)
			{
				ds >> int16;
				scanfit_vector[i].amplitude_fit[j] = (bool) int16;
				if (scanfit_vector[i].amplitude_fit[j])
				{
					parameters++;
				}
				ds >> val;
				scanfit_vector[i].amplitude[j] = val;
				ds >> val;
				scanfit_vector[i].amplitude_range[j] = val;
				ds >> int16;
				scanfit_vector[i].amplitude_bound[j] = (bool) int16;
				ds >> val;
				scanfit_vector[i].extinction[j] = val;
			}
		}
		for (unsigned int i=0; i<runfit.components; i++)
		{
			ds >> int16;
			runfit.mw_fit[i] = (bool) int16;
			ds >> val;
			if (runfit.mw_fit[i])
			{
				parameters++;
			}
			runfit.mw[i] = val;
			if (model == 3)
			{
				if (i == 0)
				{
					mc.mw_lowerLimit = runfit.mw[i];
				}
				if (i == runfit.components - 1)
				{
					mc.mw_upperLimit = runfit.mw[i];
				}
			}
			ds >> val;
			runfit.mw_range[i] = val;
			ds >> int16;
			runfit.mw_bound[i] = (bool) int16;
			ds >> int16;
			runfit.vbar20_fit[i] = (bool) int16;
			if (runfit.vbar20_fit[i])
			{
				parameters++;
			}
			ds >> val;
			runfit.vbar20[i] = val;
			ds >> val;
			runfit.vbar20_range[i] = val;
			ds >> int16;
			runfit.vbar20_bound[i] = (bool) int16;
			ds >> int16;
			runfit.virial_fit[i] = (bool) int16;
			if (runfit.virial_fit[i])
			{
				parameters++;
			}
			ds >> val;
			runfit.virial[i] = val;
			ds >> val;
			runfit.virial_range[i] = val;
			ds >> int16;
			runfit.virial_bound[i] = (bool) int16;
		}
		for (unsigned int i=0; i<runfit.association_constants; i++)
		{
			ds >> int16;
			runfit.eq_fit[i] = (bool) int16;
			if (runfit.eq_fit[i])
			{
				parameters++;
			}
			ds >> val;
			runfit.eq[i] = val;
			ds >> val;
			runfit.eq_range[i] = val;
			ds >> int16;
			runfit.eq_bound[i] = (bool) int16;
		}
		ds >> int16;
		fitpoints = int16;
		for (unsigned int i=0; i<fitpoints; i++)
		{
			ds >> val;
//cout << "i: " << i << ": " << val << endl;
			yfit.push_back(val);
		}
		f.close();
		fitcontrol();
		eqFitter->startFit();
		for (unsigned int i=0; i<scanfit_vector.size(); i++)
		{
			selected_scan = i;
		}
		selected_scan = 0;
	}
	scans_initialized = true;
}

void US_EquilMC_T::setup_vectors()
{
	switch (model) 
	{
		case -1:
		{
			QMessageBox::message(tr("Attention:\n"),tr("Please create a Model first!\n\n"
										"Click on \"Create Model\""));
			return;
		}
		case 0:	// 1-Component, Ideal
		{
			runfit.components = 1;
			runfit.association_constants = 0;
			break;
		}
		case 1:	// 2-Component, Ideal, Noninteracting
		{
			runfit.components = 2;
			runfit.association_constants = 0;
			break;
		}
		case 2:	// 3-Component, Ideal, Noninteracting
		{
			runfit.components = 3;
			runfit.association_constants = 0;
			break;
		}
		case 3:	// Fixed Molecular Weight Distribution
		{			// the runfit.components are initialized in the select_model() routine
			runfit.association_constants = 0;
			break;
		}
		case 4:	// Monomer-Dimer Equilibrium
		{
			runfit.components = 1;
			runfit.association_constants = 1;
			break;
		}
		case 5:	// Monomer-Trimer Equilibrium
		{
			runfit.components = 1;
			runfit.association_constants = 1;
			break;
		}
		case 6:	// Monomer-Tetramer Equilibrium
		{
			runfit.components = 1;
			runfit.association_constants = 1;
			break;
		}
		case 7:	// Monomer-Pentamer Equilibrium
		{
			runfit.components = 1;
			runfit.association_constants = 1;
			break;
		}
		case 8:	// Monomer-Hexamer Equilibrium
		{
			runfit.components = 1;
			runfit.association_constants = 1;
			break;
		}
		case 9:	// Monomer-Heptamer Equilibrium
		{
			runfit.components = 1;
			runfit.association_constants = 1;
			break;
		}
		case 10:	// User-Defined Monomer - N-mer Equilibrium
		{
			runfit.components = 1;
			runfit.association_constants = 1;
			break;
		}
		case 11:	// Monomer-Dimer-Trimer Equilibrium
		{
			runfit.components = 1;
			runfit.association_constants = 2;
			break;
		}
		case 12:	// Monomer-Dimer-Tetramer Equilibrium
		{
			runfit.components = 1;
			runfit.association_constants = 2;
			break;
		}
		case 13:	// User-Defined Monomer - N-mer - M-mer Equilibrium
		{
			runfit.components = 1;
			runfit.association_constants = 2;
			break;
		}
		case 14:	// 2-Component Hetero-Associating Equilibrium
		{
			runfit.components = 2;
			runfit.association_constants = 1;
			break;
		}
	}
	runfit.mw.clear();
	runfit.mw_index.clear();
	runfit.mw_range.clear();
	runfit.mw_fit.clear();
	runfit.mw_bound.clear();
	runfit.vbar20.clear();
	runfit.vbar20_index.clear();
	runfit.vbar20_range.clear();
	runfit.vbar20_fit.clear();
	runfit.vbar20_bound.clear();
	runfit.virial.clear();
	runfit.virial_index.clear();
	runfit.virial_range.clear();
	runfit.virial_fit.clear();
	runfit.virial_bound.clear();
	for (unsigned int i=0; i<runfit.components; i++)
	{
		runfit.mw.push_back(0.0);
		runfit.mw_index.push_back(0);
		runfit.mw_range.push_back(0.0);
		runfit.mw_fit.push_back(false);
		runfit.mw_bound.push_back(false);
		runfit.vbar20.push_back(.72);
		runfit.vbar20_index.push_back(0);
		runfit.vbar20_range.push_back(.144);
		runfit.vbar20_fit.push_back(false);
		runfit.vbar20_bound.push_back(false);
		runfit.virial.push_back(0.0);
		runfit.virial_index.push_back(0);
		runfit.virial_range.push_back(0.0);
		runfit.virial_fit.push_back(false);
		runfit.virial_bound.push_back(false);
	}
	for (unsigned int j=0; j<runfit.association_constants; j++)
	{
		runfit.eq_fit[j] = false;
		runfit.eq_bound[j] = false;
	}
	for (unsigned int i=0; i<scanfit_vector.size(); i++)
	{
		scanfit_vector[i].baseline = 0.0;
		scanfit_vector[i].baseline_index = 0;
		scanfit_vector[i].baseline_range = 0.0;
		scanfit_vector[i].baseline_fit = false;
		scanfit_vector[i].baseline_bound = false;
		scanfit_vector[i].amplitude.clear();
		scanfit_vector[i].amplitude_index.clear();
		scanfit_vector[i].amplitude_range.clear();
		scanfit_vector[i].amplitude_fit.clear();
		scanfit_vector[i].amplitude_bound.clear();
		scanfit_vector[i].extinction.clear();
		for (unsigned int j=0; j<runfit.components; j++)
		{
			scanfit_vector[i].amplitude.push_back(0.0);
			scanfit_vector[i].amplitude_index.push_back(0);
			scanfit_vector[i].amplitude_range.push_back(0.0);
			scanfit_vector[i].amplitude_fit.push_back(false);
			scanfit_vector[i].amplitude_bound.push_back(false);
			scanfit_vector[i].extinction.push_back(1.0);
		}
	}
}
