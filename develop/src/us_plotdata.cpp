#include "../include/us_plotdata.h"

US_PlotData::US_PlotData(QString run_id, unsigned int *scans, unsigned int *points, 
unsigned int current_cell, unsigned int current_lambda, 
bool *widget_flag, double **absorbance, double *radius, struct mfem_data *model, 
vector <double> *ti_noise, vector <double> *ri_noise, QWidget *p, const char *name) : QFrame(p, name)
{
	this->run_id = run_id;
	this->widget_flag = widget_flag;
	this->ri_noise = ri_noise;
	this->ti_noise = ti_noise;
	this->model = model;
	this->absorbance = absorbance;
	this->radius = radius;
	this->scans = scans;
	this->points = points;
	this->current_cell = current_cell;
	this->current_lambda = current_lambda;
	*widget_flag = true;
	raw = true;
	raw_sub_ti = false;
	raw_sub_ri = false;
	sim = false;
	sim_add_ti = false;
	sim_add_ri = false;
	resid = false;
	ti = false;
	ri = false;
	random = false;
	bitmap = false;
	USglobal=new US_Config();
	resplot = new US_ResidualPlot(0,0);
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	setCaption(tr("2-D Spectrum Analysis Data Viewer"));

	lbl_info = new QLabel(tr("2D Spectrum Analysis\nData Viewer"), this);
	Q_CHECK_PTR(lbl_info);
	lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_info->setAlignment(AlignCenter|AlignVCenter);
	lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   cb_raw = new QCheckBox(this);
   cb_raw->setText("");
   cb_raw->setChecked(raw);
   cb_raw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_raw->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_raw, SIGNAL(clicked()), SLOT(plot_raw()));

   cb_raw_sub_ti = new QCheckBox(this);
   cb_raw_sub_ti->setText("");
   cb_raw_sub_ti->setChecked(raw_sub_ti);
   cb_raw_sub_ti->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_raw_sub_ti->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_raw_sub_ti, SIGNAL(clicked()), SLOT(plot_raw_sub_ti()));

   cb_raw_sub_ri = new QCheckBox(this);
   cb_raw_sub_ri->setText("");
   cb_raw_sub_ri->setChecked(raw_sub_ri);
   cb_raw_sub_ri->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_raw_sub_ri->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_raw_sub_ri, SIGNAL(clicked()), SLOT(plot_raw_sub_ri()));

   cb_sim = new QCheckBox(this);
   cb_sim->setText("");
   cb_sim->setChecked(sim);
   cb_sim->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_sim->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_sim, SIGNAL(clicked()), SLOT(plot_sim()));

   cb_sim_add_ti = new QCheckBox(this);
   cb_sim_add_ti->setText("");
   cb_sim_add_ti->setChecked(sim_add_ti);
   cb_sim_add_ti->setEnabled(false);
   cb_sim_add_ti->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_sim_add_ti->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_sim_add_ti, SIGNAL(clicked()), SLOT(plot_sim_add_ti()));

   cb_sim_add_ri = new QCheckBox(this);
   cb_sim_add_ri->setText("");
   cb_sim_add_ri->setChecked(sim_add_ri);
   cb_sim_add_ri->setEnabled(false);
   cb_sim_add_ri->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_sim_add_ri->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_sim_add_ri, SIGNAL(clicked()), SLOT(plot_sim_add_ri()));

   cb_resid = new QCheckBox(this);
   cb_resid->setText("");
   cb_resid->setChecked(resid);
   cb_resid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_resid->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_resid, SIGNAL(clicked()), SLOT(plot_resid()));

   cb_ti = new QCheckBox(this);
   cb_ti->setText("");
   cb_ti->setChecked(ti);
   cb_ti->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_ti->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_ti, SIGNAL(clicked()), SLOT(plot_ti()));

   cb_ri = new QCheckBox(this);
   cb_ri->setText("");
   cb_ri->setChecked(ri);
   cb_ri->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_ri->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_ri, SIGNAL(clicked()), SLOT(plot_ri()));

   cb_random = new QCheckBox(this);
   cb_random->setText("");
   cb_random->setChecked(random);
   cb_random->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_random->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_random, SIGNAL(clicked()), SLOT(plot_random()));

   cb_bitmap = new QCheckBox(this);
   cb_bitmap->setText("");
   cb_bitmap->setChecked(bitmap);
   cb_bitmap->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_bitmap->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_bitmap, SIGNAL(clicked()), SLOT(plot_bitmap()));

	pb_cancel = new QPushButton(tr("Close"), this);
	Q_CHECK_PTR(pb_cancel);
	pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

	pb_save = new QPushButton(tr("Write Ti/Ri Noise to File"), this);
	Q_CHECK_PTR(pb_save);
	pb_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_save, SIGNAL(clicked()), SLOT(save()));

	lbl_raw = new QLabel(tr(" Plot Experimental Data"), this);
	Q_CHECK_PTR(lbl_raw);
	lbl_raw->setAlignment(AlignLeft|AlignVCenter);
	lbl_raw->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_raw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_raw_sub_ti = new QLabel(tr(" Subtract Time Invariant Noise"), this);
	Q_CHECK_PTR(lbl_raw_sub_ti);
	lbl_raw_sub_ti->setAlignment(AlignLeft|AlignVCenter);
	lbl_raw_sub_ti->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_raw_sub_ti->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_raw_sub_ri = new QLabel(tr(" Subtract Radially Invariant Noise"), this);
	Q_CHECK_PTR(lbl_raw_sub_ri);
	lbl_raw_sub_ri->setAlignment(AlignLeft|AlignVCenter);
	lbl_raw_sub_ri->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_raw_sub_ri->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_sim = new QLabel(tr(" Plot Simulated/Modeled Data"), this);
	Q_CHECK_PTR(lbl_sim);
	lbl_sim->setAlignment(AlignLeft|AlignVCenter);
	lbl_sim->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_sim->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_sim_add_ti = new QLabel(tr(" Add Time Invariant Noise"), this);
	Q_CHECK_PTR(lbl_sim_add_ti);
	lbl_sim_add_ti->setAlignment(AlignLeft|AlignVCenter);
	lbl_sim_add_ti->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_sim_add_ti->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_sim_add_ri = new QLabel(tr(" Add Radially Invariant Noise"), this);
	Q_CHECK_PTR(lbl_sim_add_ri);
	lbl_sim_add_ri->setAlignment(AlignLeft|AlignVCenter);
	lbl_sim_add_ri->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_sim_add_ri->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_resid = new QLabel(tr(" Plot Residuals"), this);
	Q_CHECK_PTR(lbl_resid);
	lbl_resid->setAlignment(AlignLeft|AlignVCenter);
	lbl_resid->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_resid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_ti = new QLabel(tr(" Plot Time Invariant Noise"), this);
	Q_CHECK_PTR(lbl_ti);
	lbl_ti->setAlignment(AlignLeft|AlignVCenter);
	lbl_ti->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_ti->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_ri = new QLabel(tr(" Plot Radially Invariant Noise"), this);
	Q_CHECK_PTR(lbl_ri);
	lbl_ri->setAlignment(AlignLeft|AlignVCenter);
	lbl_ri->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_ri->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_random = new QLabel(tr(" Plot Random Noise"), this);
	Q_CHECK_PTR(lbl_random);
	lbl_random->setAlignment(AlignLeft|AlignVCenter);
	lbl_random->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_random->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_bitmap = new QLabel(tr(" Show Residuals Bitmap"), this);
	Q_CHECK_PTR(lbl_bitmap);
	lbl_bitmap->setAlignment(AlignLeft|AlignVCenter);
	lbl_bitmap->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_bitmap->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	data_plot = new QwtPlot(this);
	data_plot->enableOutline(true);
	data_plot->setOutlinePen(white);
	data_plot->setOutlineStyle(Qwt::VLine);
	data_plot->enableGridXMin();
	data_plot->enableGridYMin();
	data_plot->setPalette( QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
	data_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
	data_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
	data_plot->setAxisTitle(QwtPlot::xBottom, tr("Radius (in cm)"));
	data_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
	data_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	data_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	data_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	data_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	data_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	data_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	data_plot->setMargin(USglobal->config_list.margin);
	data_plot->setCanvasBackground(USglobal->global_colors.plot);
	data_plot->setTitle(tr("Experimental Data"));
	data_plot->setMinimumSize(550, 250);

	resid_plot = new QwtPlot(this);
	resid_plot->enableOutline(true);
	resid_plot->setOutlinePen(white);
	resid_plot->setOutlineStyle(Qwt::VLine);
	resid_plot->enableGridXMin();
	resid_plot->enableGridYMin();
	resid_plot->setPalette( QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
	resid_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
	resid_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
	resid_plot->setAxisTitle(QwtPlot::xBottom, tr("Radius (in cm)"));
	resid_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
	resid_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	resid_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	resid_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	resid_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	resid_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	resid_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	resid_plot->setMargin(USglobal->config_list.margin);
	resid_plot->setCanvasBackground(USglobal->global_colors.plot);
	resid_plot->setTitle(tr("Residuals"));
	resid_plot->setMinimumSize(550, 200);

	lbl_variance1 = new QLabel(tr(" Variance:"),this);
	lbl_variance1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
	lbl_variance1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_variance2 = new QLabel(this);
	lbl_variance2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_variance2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	lbl_variance2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
	lbl_variance2->setText("");

	lbl_rmsd1 = new QLabel(tr(" RMSD:"),this);
	lbl_rmsd1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
	lbl_rmsd1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_rmsd2 = new QLabel(this);
	lbl_rmsd2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_rmsd2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	lbl_rmsd2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
	lbl_rmsd2->setText("");


	setup_GUI();
	update_plot();
}

US_PlotData::~US_PlotData()
{
	global_Xpos -= 30;
	global_Ypos -= 30;
	*widget_flag = false;
	delete resplot;
}

void US_PlotData::setup_GUI()
{
	QGridLayout *background = new QGridLayout(this, 2, 2, 2);
	
	int rows = 10, columns = 3, spacing = 2, j=0;
	QGridLayout *DataGrid = new QGridLayout(rows, columns, spacing, "DataGrid");

	for (int i=0; i<rows-1; i++)
	{
		DataGrid->setRowSpacing(i, 26);
	}
	DataGrid->setRowSpacing(1, 5);
	DataGrid->setRowSpacing(5, 5);
	DataGrid->setRowStretch(9, 1);

	DataGrid->addMultiCellWidget(lbl_info, j, j, 0, 2);
	j++;
	j++;
	DataGrid->addWidget(cb_raw, j, 0);
	DataGrid->addMultiCellWidget(lbl_raw, j, j, 1, 2);
	j++;
	DataGrid->addWidget(cb_raw_sub_ti, j, 1);
	DataGrid->addWidget(lbl_raw_sub_ti, j, 2);
	j++;
	DataGrid->addWidget(cb_raw_sub_ri, j, 1);
	DataGrid->addWidget(lbl_raw_sub_ri, j, 2);
	j++;
	j++;
	DataGrid->addWidget(cb_sim, j, 0);
	DataGrid->addMultiCellWidget(lbl_sim, j, j, 1, 2);
	j++;
	DataGrid->addWidget(cb_sim_add_ti, j, 1);
	DataGrid->addWidget(lbl_sim_add_ti, j, 2);
	j++;
	DataGrid->addWidget(cb_sim_add_ri, j, 1);
	DataGrid->addWidget(lbl_sim_add_ri, j, 2);
	DataGrid->setColStretch(2, 1);
	
	rows = 2, columns = 2, spacing = 2, j=0;
	QGridLayout *infoGrid = new QGridLayout(rows, columns, spacing, "InfoGrid");
	for (int i=0; i<rows; i++)
	{
		infoGrid->setRowSpacing(i, 26);
	}
	infoGrid->addWidget(lbl_variance1, j, 0);
	infoGrid->addWidget(lbl_variance2, j, 1);
	j++;
	infoGrid->addWidget(lbl_rmsd1, j, 0);
	infoGrid->addWidget(lbl_rmsd2, j, 1);
	infoGrid->setColStretch(0, 0);
	infoGrid->setColStretch(1, 1);
	
	rows = 7, columns = 3, spacing = 2, j=0;
	QGridLayout *ResidualsGrid = new QGridLayout(rows, columns, spacing, "ResidualsGrid");

	for (int i=0; i<rows; i++)
	{
		ResidualsGrid->setRowSpacing(i, 26);
	}
	ResidualsGrid->addWidget(cb_resid, j, 0);
	ResidualsGrid->addMultiCellWidget(lbl_resid, j, j, 1, 2);
	j++;
	ResidualsGrid->addWidget(cb_ti, j, 0);
	ResidualsGrid->addMultiCellWidget(lbl_ti, j, j, 1, 2);
	j++;
	ResidualsGrid->addWidget(cb_ri, j, 0);
	ResidualsGrid->addMultiCellWidget(lbl_ri, j, j, 1, 2);
	j++;
	ResidualsGrid->addWidget(cb_random, j, 0);
	ResidualsGrid->addMultiCellWidget(lbl_random, j, j, 1, 2);
	j++;
	ResidualsGrid->addWidget(cb_bitmap, j, 0);
	ResidualsGrid->addMultiCellWidget(lbl_bitmap, j, j, 1, 2);
	j++;
	ResidualsGrid->addMultiCellLayout(infoGrid, j, j, 0, 2);
	j++;
	ResidualsGrid->addMultiCellWidget(pb_save, j, j, 0, 2);
	j++;
	ResidualsGrid->addMultiCellWidget(pb_cancel, j, j, 0, 2);
	ResidualsGrid->setColStretch(1, 1);

	background->addLayout(DataGrid, 0, 0);
	background->addWidget(data_plot, 0, 1);
	background->addLayout(ResidualsGrid, 1, 0);
	background->addWidget(resid_plot, 1, 1);
	background->setColSpacing(1, 450);
	background->setRowSpacing(0, 200);
	background->setRowSpacing(1, 200);
	background->setColStretch(1, 1);

	qApp->processEvents();
	QRect r = background->cellGeometry(0, 0);

	global_Xpos += 30;
	global_Ypos += 30;
	
	this->setGeometry(global_Xpos, global_Ypos, r.width()+6+550, this->height());
}

void US_PlotData::plot_raw()
{
	if (raw)
	{
		raw = false;
		cb_raw_sub_ti->setEnabled(false);
		cb_raw_sub_ri->setEnabled(false);
	}
	else
	{
		raw = true;
		cb_raw_sub_ti->setEnabled(true);
		cb_raw_sub_ri->setEnabled(true);
	}
	update_plot();
}

void US_PlotData::plot_raw_sub_ti()
{
	if (raw_sub_ti)
	{
		raw_sub_ti = false;
	}
	else
	{
		if ((*ti_noise).size() > 0)
		{
			raw_sub_ti = true;
		}
		else
		{
			QMessageBox::information(this, tr("UltraScan Warning"),
			tr("There is no time invariant\nnoise available for plotting!"));
			cb_raw_sub_ti->setChecked(false);
			raw_sub_ti = false;
		}
	}
	update_plot();
}

void US_PlotData::plot_raw_sub_ri()
{
	if (raw_sub_ri)
	{
		raw_sub_ri = false;
	}
	else
	{
		if ((*ri_noise).size() > 0)
		{
			raw_sub_ri = true;
		}
		else
		{
			QMessageBox::information(this, tr("UltraScan Warning"),
			tr("There is no radially invariant\nnoise available for plotting!"));
			cb_raw_sub_ri->setChecked(false);
			raw_sub_ri = false;
		}
	}
	update_plot();
}

void US_PlotData::plot_sim()
{
	if (sim)
	{
		sim = false;
		cb_sim_add_ti->setEnabled(false);
		cb_sim_add_ri->setEnabled(false);
	}
	else
	{
		if ((*model).scan.size() > 0)
		{
			sim = true;
			cb_sim_add_ti->setEnabled(true);
			cb_sim_add_ri->setEnabled(true);
		}
		else
		{
			QMessageBox::information(this, tr("UltraScan Warning"), 
			tr("There are no simulated\ndata available for plotting!"));
			cb_sim->setChecked(false);
			sim = false;
		}
	}
	update_plot();
}

void US_PlotData::plot_sim_add_ti()
{
	if (sim_add_ti)
	{
		sim_add_ti = false;
	}
	else
	{
		if ((*ti_noise).size() > 0)
		{
			sim_add_ti = true;
		}
		else
		{
			QMessageBox::information(this, tr("UltraScan Warning"),
			tr("There is no time invariant\nnoise available for plotting!"));
			cb_sim_add_ti->setChecked(false);
			sim_add_ti = false;
		}
	}
	update_plot();
}

void US_PlotData::plot_sim_add_ri()
{
	if (sim_add_ri)
	{
		sim_add_ri = false;
	}
	else
	{
		if ((*ri_noise).size() > 0)
		{
			sim_add_ri = true;
		}
		else
		{
			QMessageBox::information(this, tr("UltraScan Warning"),
			tr("There is no radially invariant\nnoise available for plotting!"));
			cb_sim_add_ri->setChecked(false);
			sim_add_ri = false;
		}
	}
	update_plot();
}

void US_PlotData::plot_resid()
{
	if (resid)
	{
		resid = false;
	}
	else
	{
		if ((*model).scan.size() > 0)
		{
			resid = true;
			ti = false;
			ri = false;
			random = false;
			cb_ti->setChecked(false);
			cb_ri->setChecked(false);
			cb_random->setChecked(false);
		}
		else
		{
			QMessageBox::information(this, tr("UltraScan Warning"), 
			tr("There are no residuals available for plotting.\n"
			"A simulated solution needs to be modeled first!"));
			cb_resid->setChecked(false);
			resid = false;
		}
	}
	update_plot();
}

void US_PlotData::plot_ti()
{
	if (ti)
	{
		ti = false;
	}
	else
	{
		if ((*ti_noise).size() > 0)
		{
			ti = true;
			resid = false;
			ri = false;
			random = false;
			cb_resid->setChecked(false);
			cb_ri->setChecked(false);
			cb_random->setChecked(false);
		}
		else
		{
			QMessageBox::information(this, tr("UltraScan Warning"), 
			tr("There is no time invariant\nnoise available for plotting!"));
			cb_ti->setChecked(false);
			ti = false;
		}
	}
	update_plot();
}

void US_PlotData::plot_ri()
{
	if (ri)
	{
		ri = false;
	}
	else
	{
		if ((*ri_noise).size() > 0)
		{
			ri = true;
			resid = false;
			ti = false;
			random = false;
			cb_resid->setChecked(false);
			cb_ti->setChecked(false);
			cb_random->setChecked(false);
		}
		else
		{
			QMessageBox::information(this, tr("UltraScan Warning"),
			tr("There is no radially invariant\nnoise available for plotting!"));
			cb_ri->setChecked(false);
			ri = false;
		}
	}
	update_plot();
}

void US_PlotData::plot_random()
{
	if (random)
	{
		random = false;
	}
	else
	{
		if ((*model).scan.size() > 0)
		{
			random = true;
			resid = false;
			ti = false;
			ri = false;
			cb_resid->setChecked(false);
			cb_ti->setChecked(false);
			cb_ri->setChecked(false);
		}
		else
		{
			QMessageBox::information(this, tr("UltraScan Warning"), 
			tr("There are no random residuals available for plotting.\n"
			"A simulated solution needs to be modeled first!"));
			cb_random->setChecked(false);
			random = false;
		}
	}
	update_plot();
}

void US_PlotData::plot_bitmap()
{
	if (bitmap)
	{
		bitmap = false;
	}
	else
	{
		if ((*model).scan.size() > 0)
		{
			bitmap = true;
		}
		else
		{
			QMessageBox::information(this, tr("UltraScan Warning"), 
			tr("There are no residuals available for plotting.\n"
			"A simulated solution needs to be modeled first!"));
			cb_bitmap->setChecked(false);
			bitmap = false;
		}
	}
	update_plot();
}

void US_PlotData::cancel()
{
	close();
}

void US_PlotData::closeEvent(QCloseEvent *e)
{
	*widget_flag = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
	delete resplot;
   e->accept();
}

void US_PlotData::save()
{
	QFile f;
	QString str1, str2, str3;
	f.setName(USglobal->config_list.result_dir + "/" + run_id + str3.sprintf(".ti_noise.%d%d", current_cell+1, current_lambda+1));
	if(f.open(IO_WriteOnly))
	{
		QTextStream ts(&f);
		for (unsigned int i=0; i<(*ti_noise).size(); i++)
		{
			ts << (*ti_noise)[i] << endl;
		}
		str1.sprintf(tr("Successfully wrote %d time-invariant noise values to:\n"), (*ti_noise).size());
		str1 += USglobal->config_list.result_dir + "/" + run_id + str3.sprintf(".ti_noise.%d%d", current_cell+1, current_lambda+1);
		f.close();
	}
	else
	{
		str1 = "Attention: Unable to write time-invariant data to disk!";
	}
	f.setName(USglobal->config_list.result_dir + "/" + run_id + str3.sprintf(".ri_noise.%d%d", current_cell+1, current_lambda+1));
	if(f.open(IO_WriteOnly))
	{
		QTextStream ts(&f);
		for (unsigned int i=0; i<(*ri_noise).size(); i++)
		{
			ts << (*ri_noise)[i] << endl;
		}
		str2.sprintf(tr("Successfully wrote %d radially-invariant noise values to:\n"), (*ri_noise).size());
		str2 += USglobal->config_list.result_dir + "/" + run_id + str3.sprintf(".ri_noise.%d%d", current_cell+1, current_lambda+1);
		f.close();
	}
	else
	{
		str2 = "Attention: Unable to write radially-invariant data to disk!";
	}
	QMessageBox::information(this, tr("UltraScan Information"), str1 + "\n\n" + str2);
}

void US_PlotData::update_plot()
{
	float resid_val = 0.0;
	data_plot->clear();
	data_plot->replot();
	resid_plot->clear();
	resid_plot->replot();
	unsigned int i, j, *c_raw, *c_model, *c_resid;
	double **raw_data, **residuals, **model_data, *yval, *xval;
	raw_data = new double * [*scans];
	model_data = new double * [*scans];
	residuals = new double * [*points];
	yval = new double [*points];
	xval = new double [*scans];
	c_raw = new unsigned int [*scans];
	c_model = new unsigned int [*scans];
	c_resid = new unsigned int [*scans];
	for (i=0; i<*scans; i++)
	{
		raw_data[i] = new double [*points];
		model_data[i] = new double [*points];
		residuals[i] = new double [*points];
	}
	
	QString str;
	QPen pen_yellow, pen_red, pen_green;
	pen_yellow.setColor(Qt::yellow);
	pen_red.setColor(Qt::red);
	pen_green.setColor(Qt::green);
	pen_green.setWidth(1);
	
	if (raw)
	{
		if(raw_sub_ti && raw_sub_ri)
		{
			for (i=0; i<*scans; i++)
			{
				for (j=0; j<*points; j++)
				{
					raw_data[i][j] = absorbance[i][j] - (*ti_noise)[j] - (*ri_noise)[i];
				}
			}
		}
		else if (raw_sub_ti && !raw_sub_ri)
		{
			for (i=0; i<*scans; i++)
			{
				for (j=0; j<*points; j++)
				{
					raw_data[i][j] = absorbance[i][j] - (*ti_noise)[j];
				}
			}
		}
		else if (!raw_sub_ti && raw_sub_ri)
		{
			for (i=0; i<*scans; i++)
			{
				for (j=0; j<*points; j++)
				{
					raw_data[i][j] = absorbance[i][j] - (*ri_noise)[i];
				}
			}
		}
		else if (!raw_sub_ti && !raw_sub_ri)
		{
			for (i=0; i<*scans; i++)
			{
				for (j=0; j<*points; j++)
				{
					raw_data[i][j] = absorbance[i][j];
				}
			}
		}
		
		for (i=0; i<*scans; i++)
		{
			c_raw[i] = data_plot->insertCurve(tr(str.sprintf("Raw Data Scan %d", i)));
			data_plot->setCurveStyle(c_raw[i], QwtCurve::Lines);
			data_plot->setCurveData(c_raw[i], radius, raw_data[i], *points);
			data_plot->setCurvePen(c_raw[i], pen_yellow);
		}
	}
	if (sim)
	{
		if(sim_add_ti && sim_add_ri)
		{
			for (i=0; i<*scans; i++)
			{
				for (j=0; j<*points; j++)
				{
					model_data[i][j] = (*model).scan[i].conc[j] + (*ti_noise)[j] + (*ri_noise)[i];
				}
			}
		}
		if(!sim_add_ti && sim_add_ri)
		{
			for (i=0; i<*scans; i++)
			{
				for (j=0; j<*points; j++)
				{
					model_data[i][j] = (*model).scan[i].conc[j] + (*ri_noise)[i];
				}
			}
		}
		if(sim_add_ti && !sim_add_ri)
		{
			for (i=0; i<*scans; i++)
			{
				for (j=0; j<*points; j++)
				{
					model_data[i][j] = (*model).scan[i].conc[j] + (*ti_noise)[j];
				}
			}
		}
		if(!sim_add_ti && !sim_add_ri)
		{
			for (i=0; i<*scans; i++)
			{
				for (j=0; j<*points; j++)
				{
					model_data[i][j] = (*model).scan[i].conc[j];
				}
			}
		}
		
		for (i=0; i<*scans; i++)
		{
			c_model[i] = data_plot->insertCurve(tr(str.sprintf("Model Data Scan %d", i)));
			data_plot->setCurveStyle(c_model[i], QwtCurve::Lines);
			data_plot->setCurveData(c_model[i], radius, model_data[i], *points);
			data_plot->setCurvePen(c_model[i], pen_red);
		}
	}
	data_plot->replot();
	if (resid && sim && raw)
	{
		resid_val = 0.0;
		for (i=0; i<*scans; i++)
		{
			for (j=0; j<*points; j++)
			{
				residuals[i][j] = raw_data[i][j] - model_data[i][j];
				resid_val += pow((double) residuals[i][j], 2.0);
			}
		}
		resid_val /= ((*scans) * (*points));
		str.sprintf("%6.4e", resid_val);
		lbl_variance2->setText(str);
		resid_val = pow((double) resid_val, 0.5);
		str.sprintf("%6.4e", resid_val);
		lbl_rmsd2->setText(str);
		
		for (i=0; i<*scans; i++)
		{
			c_resid[i] = resid_plot->insertCurve(tr(str.sprintf("Residual Scan %d", i)));
			resid_plot->setCurveStyle(c_resid[i], QwtCurve::Dots);
			resid_plot->setCurveData(c_resid[i], radius, residuals[i], *points);
			resid_plot->setCurvePen(c_resid[i], pen_green);
		}
	}
	else if (ti)
	{
		for (j=0; j<*points; j++)
		{
			yval[j] = (*ti_noise)[j];
		}
		i = resid_plot->insertCurve(tr("Time Invariant Noise"));
		resid_plot->setCurveStyle(i, QwtCurve::Lines);
		resid_plot->setCurveData(i, radius, yval, *points);
		resid_plot->setCurvePen(i, pen_green);
		lbl_variance2->setText("");
		lbl_rmsd2->setText("");
	}
	else if (ri)
	{
		delete [] yval;
		yval = new double [*scans];
		xval = new double [*scans];
		for (i=0; i<*scans; i++)
		{
			yval[i] = (*ri_noise)[i];
			xval[i] = (double) i;
		}
		i = resid_plot->insertCurve(tr("Radially Invariant Noise"));
		resid_plot->setCurveStyle(i, QwtCurve::Lines);
		resid_plot->setCurveData(i, xval, yval, *scans);
		resid_plot->setCurvePen(i, pen_green);
		lbl_variance2->setText("");
		lbl_rmsd2->setText("");
	}
	else if (random)
	{
		resid_val = 0.0;
		for (i=0; i<*scans; i++)
		{
			for (j=0; j<*points; j++)
			{
				residuals[i][j] = absorbance[i][j] - (*model).scan[i].conc[j] - (*ti_noise)[j] - (*ri_noise)[i];
				resid_val += pow((double) residuals[i][j], 2.0);
			}
		}
		for (i=0; i<*scans; i++)
		{
			c_resid[i] = resid_plot->insertCurve(tr(str.sprintf("Random Residual Scan %d", i)));
			resid_plot->setCurveStyle(c_resid[i], QwtCurve::Dots);
			resid_plot->setCurveData(c_resid[i], radius, residuals[i], *points);
			resid_plot->setCurvePen(c_resid[i], pen_green);
		}
		resid_val /= ((*scans) * (*points));
		str.sprintf("%6.4e", resid_val);
		lbl_variance2->setText(str);
		resid_val = pow((double) resid_val, 0.5);
		str.sprintf("%6.4e", resid_val);
		lbl_rmsd2->setText(str);
	}
	if (bitmap && sim && raw)
	{
		for (i=0; i<*scans; i++)
		{
			for (j=0; j<*points; j++)
			{
				residuals[i][j] = raw_data[i][j] - model_data[i][j];
				resid_val += pow((double) residuals[i][j], 2.0);
			}
		}
		resplot->setData(residuals, global_Xpos+30, global_Ypos+30, *scans, *points);
		resplot->show();
		resplot->repaint();
		resid_val /= ((*scans) * (*points));
		str.sprintf("%6.4e", resid_val);
		lbl_variance2->setText(str);
		resid_val = pow((double) resid_val, 0.5);
		str.sprintf("%6.4e", resid_val);
		lbl_rmsd2->setText(str);
	}
	else
	{
		resplot->hide();
	}
	resid_plot->replot();
	for (i=0; i<*scans; i++)
	{
		delete [] raw_data[i];
		delete [] model_data[i];
		delete [] residuals[i];
	}
	delete [] xval;
	delete [] yval;
	delete [] model_data;
	delete [] raw_data;
	delete [] residuals;
	delete [] c_raw;
	delete [] c_model;
	delete [] c_resid;
}


