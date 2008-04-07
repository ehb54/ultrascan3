#include "../include/us_fematch_ra.h"

US_FeMatchRa_W::US_FeMatchRa_W(QWidget *p, const char *name) : Data_Control_W(13, p, name)
{
	setCaption(tr("Finite Element Analysis:"));
	pm = new US_Pixmap();
	setup_GUI();
	connect(this, SIGNAL(dataLoaded()), this, SLOT(enableButtons()));
	connect(this, SIGNAL(datasetChanged()), this, SLOT(clearDisplay()));
	resplot = NULL;
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

	int j=0;
	int rows = 15, columns = 4, spacing = 2;

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

	rows = 10, columns = 4, spacing = 2, j=0;
	QGridLayout * subGrid2 = new QGridLayout(rows, columns, spacing);
	for (int i=0; i<rows; i++)
	{
		subGrid2->setRowSpacing(i, 26);
	}
	subGrid2->addMultiCellWidget(pb_loadModel,j,j,0,1);
	subGrid2->addMultiCellWidget(pb_fit,j,j,2,3);
	j++;
	subGrid2->addMultiCellWidget(lbl_variance, j, j, 0, 1);
	subGrid2->addMultiCellWidget(lbl_variance2, j, j, 2, 3);
	j++;
	subGrid2->addMultiCellWidget(lbl1_excluded, j, j, 0, 1);
	subGrid2->addMultiCellWidget(lbl2_excluded, j, j, 2, 3);
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

void US_FeMatchRa_W::update_distribution()
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

void US_FeMatchRa_W::write_cofs()
{
}

void US_FeMatchRa_W::fit()
{
	calc_residuals();
}

float US_FeMatchRa_W::calc_residuals()
{
	return rmsd;
}

void US_FeMatchRa_W::calc_distros()
{
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
	QString fn = QFileDialog::getOpenFileName(USglobal->config_list.result_dir, "*.model.??", 0);
	if ( !fn.isEmpty() )
	{
		load_model(fn);		// the user gave a file name
	}
}

void US_FeMatchRa_W::load_model(const QString &fileName)
{
}

//void US_FeMatchRa_W::updateParameters(float val1, float val2)
void US_FeMatchRa_W::updateParameters()
{
}
