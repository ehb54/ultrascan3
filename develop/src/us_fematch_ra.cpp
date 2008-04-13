#include "../include/us_fematch_ra.h"

US_FeMatchRa_W::US_FeMatchRa_W(QWidget *p, const char *name) : Data_Control_W(13, p, name)
{
	setCaption(tr("Finite Element Analysis:"));
	pm = new US_Pixmap();
	setup_GUI();
	connect(this, SIGNAL(dataLoaded()), this, SLOT(enableButtons()));
	connect(this, SIGNAL(datasetChanged()), this, SLOT(clearDisplay()));
	resplot = NULL;
	stopFlag = false;
	movieFlag = false;
	astfem_rsa = new US_Astfem_RSA(&stopFlag, false, &movieFlag);
}

US_FeMatchRa_W::~US_FeMatchRa_W()
{
	delete resplot;
}

void US_FeMatchRa_W::clearDisplay()
{
	if (resplot != NULL)
	{
		resplot->close();
		resplot = NULL;
	}
	analysis_plot->clear();
	analysis_plot->replot();
}

void US_FeMatchRa_W::setup_GUI()
{
	if (resplot == NULL)
	{
		resplot = new US_ResidualPlot(0,0);
	}

	pb_second_plot->setText(tr("s20,W distribution"));
	delete pb_reset;
	delete smoothing_lbl;
	delete smoothing_counter;
	smoothing_counter = NULL;
	delete range_lbl;
	delete range_counter;
	range_counter = NULL;
	delete position_lbl;
	delete position_counter;
	delete pb_vbar;
	delete vbar_le;
	position_counter = NULL;

	lbl1_excluded->setText(tr("RMSD:"));

	pb_loadModel = new QPushButton(tr("Load Model"), this);
	Q_CHECK_PTR(pb_loadModel);
	pb_loadModel->setAutoDefault(false);
	pb_loadModel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_loadModel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_loadModel->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_loadModel->setEnabled(false);
	connect(pb_loadModel, SIGNAL(clicked()), SLOT(load_model()));

	pb_fit = new QPushButton(tr("Simulate Model"), this);
	Q_CHECK_PTR(pb_fit);
	pb_fit->setAutoDefault(false);
	pb_fit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_fit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_fit->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_fit->setEnabled(false);
	connect(pb_fit, SIGNAL(clicked()), SLOT(fit()));

	lbl_variance = new QLabel(tr(" Variance:"),this);
	lbl_variance->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
	lbl_variance->setGeometry(xpos, ypos, buttonw, buttonh);
	lbl_variance->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_variance2 = new QLabel(this);
	lbl_variance2->setGeometry(xpos, ypos, buttonw, buttonh);
	lbl_variance2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_variance2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	lbl_variance2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
	lbl_variance2->setText("0.0");
	lbl_variance2->setAlignment(AlignCenter|AlignVCenter);

	analysis_plot->setTitle(tr("Fitting Residuals"));

	int j=0;
	int rows = 15, columns = 4, spacing = 2;

	QGridLayout * background = new QGridLayout(this,2,2,spacing);
	QGridLayout * subGrid1 = new QGridLayout(rows, columns, spacing);
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
	subGrid1->addMultiCellWidget(pb_loadModel,j,j,0,1);
	subGrid1->addMultiCellWidget(pb_fit,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(lbl_variance, j, j, 0, 1);
	subGrid1->addMultiCellWidget(lbl_variance2, j, j, 2, 3);
	j++;
	subGrid1->addMultiCellWidget(lbl1_excluded, j, j, 0, 1);
	subGrid1->addMultiCellWidget(lbl2_excluded, j, j, 2, 3);
	j++;
	subGrid1->addMultiCellWidget(pb_exsingle,j,j,0,1);
	subGrid1->addMultiCellWidget(cnt_exsingle,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(pb_exrange,j,j,0,1);
	subGrid1->addMultiCellWidget(cnt_exrange,j,j,2,3);
	j++;
	subGrid1->addWidget(lbl_status,j,0);
	subGrid1->addMultiCellWidget(progress,j,j,1,3);
	for (int i=0; i<j; i++)
	{
		subGrid1->setRowSpacing(i, 26);
	}

	background->addMultiCellLayout(subGrid1, 0, 1, 0, 0);
	background->addWidget(analysis_plot, 0, 1);
	background->addWidget(edit_plot, 1, 1);
	background->setColStretch(0, 1);
	background->setColStretch(1, 3);
	background->setColSpacing(0, 350);
	background->setColSpacing(1, 550);

	qApp->processEvents();
	QRect r = background->cellGeometry(0, 0);

	global_Xpos += 30;
	global_Ypos += 30;

	this->setGeometry(global_Xpos, global_Ypos, r.width()+550+spacing*3, this->height());
}

void US_FeMatchRa_W::enableButtons()
{
	pb_second_plot->setEnabled(false);
	pb_save->setEnabled(false);
	pb_view->setEnabled(false);
	pb_print->setEnabled(false);
	pb_loadModel->setEnabled(true);
}

void US_FeMatchRa_W::save()
{
}

void US_FeMatchRa_W::help()
{
	US_Help *online_help;
	online_help = new US_Help(this);
	online_help->show_help("manual/fematch_ra.html");
}

void US_FeMatchRa_W::view()
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
	filestr.append("." + analysis_type + "_res");
	filestr.append(temp);
	write_res();
	TextEdit *e;
	e = new TextEdit();
	e->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	e->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
	e->load(filestr);
	e->show();
}

void US_FeMatchRa_W::write_res()
{
}

float US_FeMatchRa_W::fit()
{
	QString str;
	unsigned int i, j;
	long *s_curve, *r_curve;
	double **sim, **res;
	struct mfem_scan single_scan;

	sim = new double * [run_inf.scans[selected_cell][selected_lambda]];
	res = new double * [run_inf.scans[selected_cell][selected_lambda]];
	s_curve = new long   [run_inf.scans[selected_cell][selected_lambda]];
	r_curve = new long   [run_inf.scans[selected_cell][selected_lambda]];
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		sim[i] = new double [points];
		res[i] = new double [points];
	}
	simdata.resize(1);
	single_scan.conc.clear();
	for (i=0; i<points; i++)
	{
		simdata[0].radius.push_back(radius[i]);
		single_scan.conc.push_back(0.0); // populate with zeros
	}
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		single_scan.time = (double) run_inf.time[selected_cell][selected_lambda][i];
		simdata[0].scan.push_back(single_scan);
	}
	astfem_rsa->calculate(&ms, &sp, &simdata);
	rmsd = 0.0;
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		for (j=0; j<points; j++)
		{
			sim[i][j] = simdata[0].scan[i].conc[j];
			res[i][j] = absorbance[i][j] - sim[i][j];
			rmsd += pow(res[i][j], 2.0);
		}
		s_curve[i] = edit_plot->insertCurve("Simulated Model Data");
		r_curve[i] = analysis_plot->insertCurve("Residual Data");
		edit_plot->setCurvePen(s_curve[i], QPen(Qt::red, 1, SolidLine));
		edit_plot->setCurveData(s_curve[i], radius, sim[i], points);
		analysis_plot->setCurvePen(r_curve[i], QPen(Qt::yellow, 1, SolidLine));
		analysis_plot->setCurveData(r_curve[i], radius, res[i], points);
	}
	edit_plot->replot();
	analysis_plot->replot();
	rmsd /= (run_inf.scans[selected_cell][selected_lambda] * points);
	str.sprintf("%6.4e", rmsd);
	lbl_variance2->setText(str);
	rmsd = pow((double)rmsd, 0.5);
	str.sprintf("%6.4e", rmsd);
	lbl2_excluded->setText(str);
	for (i=0; i<run_inf.scans[selected_cell][selected_lambda]; i++)
	{
		delete [] sim[i];
		delete [] res[i];
	}
	delete [] sim;
	delete [] res;
	delete [] s_curve;
	delete [] r_curve;
	return rmsd;
}


void US_FeMatchRa_W::clear_data(mfem_data *d)
{
	unsigned int i;
	for (i=0; i<(*d).scan.size(); i++)
	{
		(*d).scan[i].conc.clear();
	}
	(*d).radius.clear();
	(*d).scan.clear();
}

void US_FeMatchRa_W::load_model()
{
	QString fn = QFileDialog::getOpenFileName(USglobal->config_list.result_dir, "*.us_system", 0);
	if ( !fn.isEmpty() )
	{
		US_FemGlobal *fg;
		int error_code;
		QString str;
		fg = new US_FemGlobal(this);
		error_code = fg->read_experiment(&ms, &sp, fn);
		cout << "s1: " << ms.component_vector[0].s
		<< ", s2: " << ms.component_vector[1].s
		<< ", D1: " << ms.component_vector[0].D
				<< ", D2: " << ms.component_vector[1].D << endl;
				delete fg;
		if (error_code < 0)
		{
			str.sprintf("Unable to load System: " + fn + "\n\nError code: %d", error_code);
			QMessageBox::information(this, tr("Simulation Module"), tr(str), QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			return;
		}
		else
		{
			printError(4); // successfully loaded a new model
			pb_fit->setEnabled(true);
		}
	}
}

void US_FeMatchRa_W::printError(const int &ival)
{
	switch (ival)
	{
		case 0:
		{
			QMessageBox::warning(this, tr("UltraScan Warning"),
			tr("Please note:\n\nThere was an error reading\nthe selected Model File!\n\nThis file appears to be corrupted"),
			QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			break;
		}
		case 1:
		{
			QMessageBox::warning(this, tr("UltraScan Warning"),
			tr("Sorry, for old-style model files only\nnon-interacting model loading is supported.\n\n") +
			tr("Please recreate this model by clicking on:\n\n\"New Model\""),
			QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			break;
		}
		case 2:
		{
			QMessageBox::warning(this, tr("UltraScan Warning"), tr("Please note:\n\n") +
			tr("UltraScan could not open\nthe selected Model File!"),
			QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			break;
		}
		case 3:
		{
			QMessageBox::warning(this, tr("UltraScan Warning"),
			tr("Please note:\n\nUltraScan could not read\nthe selected Model File!"),
			QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			break;
		}
		case 4:
		{
			QMessageBox::information(this, tr("Simulation Module"), tr("Successfully loaded Model:\n\n")
			+ ms.description, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			break;
		}
		case 5:
		{
			QMessageBox::warning(this, tr("UltraScan Warning"),
			tr("Please note:\n\nYou provided an invalid entry!\n\nPlease try again..."),
			QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			break;
		}
		case 6:
		{
			QMessageBox::warning(this, tr("UltraScan Warning"),
										tr("Sorry, old-style models are no longer supported.\n\n") +
												tr("Please load a different model or create a new Model"),
										QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			break;
		}
	}
}

