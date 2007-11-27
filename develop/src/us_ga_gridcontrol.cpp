#include "../include/us_ga_gridcontrol.h"

US_Ga_GridControl::US_Ga_GridControl(struct ga_data *GA_Params, QWidget *p, const char *name) : QDialog( p, name)
{
	USglobal=new US_Config();
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	setCaption(tr("Genetic Algorithm Control Window"));
	QString str;
	distro_type = -1;
	k_range = 1.0;
	s_range = 1.0;
	minmax = false;
	current_solute = 0;
	this->GA_Params = GA_Params;
	sval.clear();
	frequency.clear();
	ff0.clear();
	ff0_min = 1.0;
	ff0_max = 4.0;
	textwindow = new QTextEdit(this);
	textwindow->setReadOnly(true);
	textwindow->setText("Please load a sedimentation coefficient distribution to initialize the"
							  " genetic algorithm s-value range. The distribution should have a good"
							  " resolution over the sedimentation coefficients. This distribution will" 
							  " be used to initialize all experiments used in the run, so the"
							  " distribution taken from the experiment with the highest speed is"
							  " probably the most appropriate distribution. You can use a distribution"
							  " from the van Holde - Weischet method, the C(s) method, or 2-D Spectrum"
							  " Analysis.");
	textwindow->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

	lbl_info1 = new QLabel(tr("Genetic Algorithm Control Window"), this);
	Q_CHECK_PTR(lbl_info1);
	lbl_info1->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_info1->setAlignment(AlignCenter|AlignVCenter);
	lbl_info1->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_info1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	lbl_info2 = new QLabel(tr("Distribution Data"), this);
	Q_CHECK_PTR(lbl_info2);
	lbl_info2->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_info2->setAlignment(AlignCenter|AlignVCenter);
	lbl_info2->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_info2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	lbl_genes = new QLabel(tr(" Population Size: "), this);
	Q_CHECK_PTR(lbl_genes);
	lbl_genes->setAlignment(AlignLeft|AlignVCenter);
	lbl_genes->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_genes->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_genes= new QwtCounter(this);
	Q_CHECK_PTR(cnt_genes);
	cnt_genes->setRange(50, 5000, 1);
	cnt_genes->setValue((*GA_Params).genes);
	cnt_genes->setNumButtons(3);
	cnt_genes->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_genes, SIGNAL(valueChanged(double)), SLOT(update_genes(double)));

	lbl_demes = new QLabel(tr(" Number of Demes: "), this);
	Q_CHECK_PTR(lbl_demes);
	lbl_demes->setAlignment(AlignLeft|AlignVCenter);
	lbl_demes->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_demes->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_demes= new QwtCounter(this);
	Q_CHECK_PTR(cnt_demes);
	cnt_demes->setRange(1, 100, 1);
	cnt_demes->setValue((*GA_Params).demes);
	cnt_demes->setNumButtons(3);
	cnt_demes->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_demes, SIGNAL(valueChanged(double)), SLOT(update_demes(double)));

	lbl_generations = new QLabel(tr(" Number of Generations: "), this);
	Q_CHECK_PTR(lbl_generations);
	lbl_generations->setAlignment(AlignLeft|AlignVCenter);
	lbl_generations->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_generations->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_generations= new QwtCounter(this);
	Q_CHECK_PTR(cnt_generations);
	cnt_generations->setRange(25, 1000, 1);
	cnt_generations->setValue((*GA_Params).generations);
	cnt_generations->setNumButtons(3);
	cnt_generations->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_generations, SIGNAL(valueChanged(double)), SLOT(update_generations(double)));

	lbl_crossover = new QLabel(tr(" Crossover Rate: "), this);
	Q_CHECK_PTR(lbl_crossover);
	lbl_crossover->setAlignment(AlignLeft|AlignVCenter);
	lbl_crossover->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_crossover->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_crossover= new QwtCounter(this);
	Q_CHECK_PTR(cnt_crossover);
	cnt_crossover->setRange(0, 100, 1);
	cnt_crossover->setValue((*GA_Params).crossover);
	cnt_crossover->setNumButtons(3);
	cnt_crossover->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_crossover, SIGNAL(valueChanged(double)), SLOT(update_crossover(double)));

	lbl_initial_solutes = new QLabel(tr(" Number of initial Solutes: "), this);
	Q_CHECK_PTR(lbl_initial_solutes);
	lbl_initial_solutes->setAlignment(AlignLeft|AlignVCenter);
	lbl_initial_solutes->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_initial_solutes->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_initial_solutes= new QwtCounter(this);
	Q_CHECK_PTR(cnt_initial_solutes);
	cnt_initial_solutes->setRange(1, 100, 1);
	cnt_initial_solutes->setValue((*GA_Params).initial_solutes);
	cnt_initial_solutes->setNumButtons(3);
	cnt_initial_solutes->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_initial_solutes, SIGNAL(valueChanged(double)), SLOT(update_initial_solutes(double)));

	lbl_mutation = new QLabel(tr(" Mutation Rate: "), this);
	Q_CHECK_PTR(lbl_mutation);
	lbl_mutation->setAlignment(AlignLeft|AlignVCenter);
	lbl_mutation->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_mutation->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_mutation= new QwtCounter(this);
	Q_CHECK_PTR(cnt_mutation);
	cnt_mutation->setRange(0, 100, 1);
	cnt_mutation->setValue((*GA_Params).mutation);
	cnt_mutation->setNumButtons(3);
	cnt_mutation->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_mutation, SIGNAL(valueChanged(double)), SLOT(update_mutation(double)));

	lbl_plague = new QLabel(tr(" Plague: "), this);
	Q_CHECK_PTR(lbl_plague);
	lbl_plague->setAlignment(AlignLeft|AlignVCenter);
	lbl_plague->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_plague->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_plague= new QwtCounter(this);
	Q_CHECK_PTR(cnt_plague);
	cnt_plague->setRange(0, 100, 1);
	cnt_plague->setValue((*GA_Params).plague);
	cnt_plague->setNumButtons(3);
	cnt_plague->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_plague, SIGNAL(valueChanged(double)), SLOT(update_plague(double)));

	lbl_elitism = new QLabel(tr(" Elitism: "), this);
	Q_CHECK_PTR(lbl_elitism);
	lbl_elitism->setAlignment(AlignLeft|AlignVCenter);
	lbl_elitism->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_elitism->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_elitism= new QwtCounter(this);
	Q_CHECK_PTR(cnt_elitism);
	cnt_elitism->setRange(0, 5, 1);
	cnt_elitism->setValue((*GA_Params).elitism);
	cnt_elitism->setNumButtons(3);
	cnt_elitism->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_elitism, SIGNAL(valueChanged(double)), SLOT(update_elitism(double)));

	lbl_random_seed = new QLabel(tr(" Random Seed: "), this);
	Q_CHECK_PTR(lbl_random_seed);
	lbl_random_seed->setAlignment(AlignLeft|AlignVCenter);
	lbl_random_seed->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_random_seed->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_random_seed= new QwtCounter(this);
	Q_CHECK_PTR(cnt_random_seed);
	cnt_random_seed->setRange(0, 10000, 1);
	cnt_random_seed->setValue((*GA_Params).random_seed);
	cnt_random_seed->setNumButtons(3);
	cnt_random_seed->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_random_seed, SIGNAL(valueChanged(double)), SLOT(update_random_seed(double)));

	lbl_ff0_min = new QLabel(tr(" f/f0 minimum: "), this);
	Q_CHECK_PTR(lbl_ff0_min);
	lbl_ff0_min->setAlignment(AlignLeft|AlignVCenter);
	lbl_ff0_min->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_ff0_min->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_ff0_min= new QwtCounter(this);
	Q_CHECK_PTR(cnt_ff0_min);
	cnt_ff0_min->setRange(1, 49, 0.01);
	cnt_ff0_min->setValue(ff0_min);
	cnt_ff0_min->setNumButtons(3);
	cnt_ff0_min->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_ff0_min, SIGNAL(valueChanged(double)), SLOT(update_ff0_min(double)));

	lbl_ff0_max = new QLabel(tr(" f/f0 maximum: "), this);
	Q_CHECK_PTR(lbl_ff0_max);
	lbl_ff0_max->setAlignment(AlignLeft|AlignVCenter);
	lbl_ff0_max->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_ff0_max->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_ff0_max= new QwtCounter(this);
	Q_CHECK_PTR(cnt_ff0_max);
	cnt_ff0_max->setRange(1, 50, 0.01);
	cnt_ff0_max->setValue(ff0_max);
	cnt_ff0_max->setNumButtons(3);
	cnt_ff0_max->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_ff0_max, SIGNAL(valueChanged(double)), SLOT(update_ff0_max(double)));

	lbl_regularization = new QLabel(tr(" Regularization: "), this);
	Q_CHECK_PTR(lbl_regularization);
	lbl_regularization->setAlignment(AlignLeft|AlignVCenter);
	lbl_regularization->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_regularization->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_regularization= new QwtCounter(this);
	Q_CHECK_PTR(cnt_regularization);
	cnt_regularization->setRange(1, 100, 1);
	cnt_regularization->setValue((*GA_Params).regularization);
	cnt_regularization->setNumButtons(3);
	cnt_regularization->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_regularization, SIGNAL(valueChanged(double)), SLOT(update_regularization(double)));

	lbl_meniscus_range = new QLabel(tr(" Meniscus Fit Range (cm): "), this);
	Q_CHECK_PTR(lbl_meniscus_range);
	lbl_meniscus_range->setAlignment(AlignLeft|AlignVCenter);
	lbl_meniscus_range->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_meniscus_range->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cnt_meniscus_range= new QwtCounter(this);
	Q_CHECK_PTR(cnt_meniscus_range);
	cnt_meniscus_range->setRange(0, 0.01, 1e-4);
	cnt_meniscus_range->setValue((*GA_Params).meniscus_range);
	cnt_meniscus_range->setNumButtons(3);
	cnt_meniscus_range->setEnabled((*GA_Params).fit_meniscus);
	cnt_meniscus_range->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_meniscus_range, SIGNAL(valueChanged(double)), SLOT(update_meniscus_range(double)));

	pb_load_distro = new QPushButton(tr(" Load Distribution "), this);
	Q_CHECK_PTR(pb_load_distro);
	pb_load_distro->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_load_distro->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_load_distro->setAutoDefault(false);
	connect(pb_load_distro, SIGNAL(clicked()), SLOT(load_distro()));

	pb_help = new QPushButton(tr(" Help "), this);
	Q_CHECK_PTR(pb_help);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_help->setAutoDefault(false);
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	pb_reset_peaks = new QPushButton(tr(" Reset Solute Bins "), this);
	Q_CHECK_PTR(pb_reset_peaks);
	pb_reset_peaks->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_reset_peaks->setEnabled(false);
	pb_reset_peaks->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_reset_peaks->setAutoDefault(false);
	connect(pb_reset_peaks, SIGNAL(clicked()), SLOT(reset_peaks()));

	pb_assign_peaks = new QPushButton(tr(" Autoassign Solute Bins "), this);
	Q_CHECK_PTR(pb_assign_peaks);
	pb_assign_peaks->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_assign_peaks->setEnabled(false);
	pb_assign_peaks->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_assign_peaks->setAutoDefault(false);
	connect(pb_assign_peaks, SIGNAL(clicked()), SLOT(assign_peaks()));

	pb_close = new QPushButton(tr(" Accept "), this);
	Q_CHECK_PTR(pb_close);
	pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_close->setAutoDefault(false);
	connect(pb_close, SIGNAL(clicked()), SLOT(accept()));

	pb_reject = new QPushButton(tr(" Cancel "), this);
	Q_CHECK_PTR(pb_reject);
	pb_reject->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_reject->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_reject->setAutoDefault(false);
	connect(pb_reject, SIGNAL(clicked()), SLOT(reject()));

	lb_solutes = new QListBox(this, "Solutes");
	lb_solutes->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	lb_solutes->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	connect(lb_solutes, SIGNAL(selected(int)), SLOT(edit_solute(int)));

   cb_meniscus = new QCheckBox(this);
   cb_meniscus->setText(tr(" Float Meniscus Position "));
   cb_meniscus->setChecked((*GA_Params).fit_meniscus);
   cb_meniscus->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_meniscus->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_meniscus, SIGNAL(clicked()), SLOT(set_meniscus()));

	plot = new QwtPlot(this);
	Q_CHECK_PTR(plot);
	plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
	plot->enableGridXMin();
	plot->enableGridYMin();
	plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
	plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
	plot->setCanvasBackground(USglobal->global_colors.plot);		//new version
	plot->enableOutline(true);
	plot->setAxisTitle(QwtPlot::xBottom, "s value");
	plot->setAxisTitle(QwtPlot::yLeft, "Frequency");
	plot->setTitle(tr("Distribution Data"));
	plot->setOutlinePen(white);
	plot->setOutlineStyle(Qwt::Cross);
	plot->setMinimumSize(550,300);
	plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
	plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	plot->setMargin(USglobal->config_list.margin);
	connect(plot, SIGNAL(plotMouseReleased(const QMouseEvent &)),
			  SLOT(getMouseReleased(const QMouseEvent &)));
	connect(plot, SIGNAL(plotMousePressed(const QMouseEvent &)),
			  SLOT(getMousePressed(const QMouseEvent &)));

	setup_GUI();

	global_Xpos += 30;
	global_Ypos += 30;
	move(global_Xpos, global_Ypos);

	for (unsigned int i=0; i<(*GA_Params).solute.size(); i++)
	{
		str.sprintf("Solute Bin %d: s_min=%4.2e, s_max=%4.2e, f/f0_min=%4.2f, f/f0_max=%4.2f", i+1,
						(*GA_Params).solute[i].s_min,
						(*GA_Params).solute[i].s_max,
						(*GA_Params).solute[i].ff0_min,
						(*GA_Params).solute[i].ff0_max);
		lb_solutes->insertItem(str);
		pb_reset_peaks->setEnabled(true);
	}
}

US_Ga_GridControl::~US_Ga_GridControl()
{
}

void US_Ga_GridControl::setup_GUI()
{
	int rows = 18, columns = 2, spacing = 2, j=0;
	QBoxLayout *background=new QHBoxLayout(this, 2);
	QGridLayout *leftbox=new QGridLayout(background, 3, 1, spacing);
	QGridLayout *rightbox=new QGridLayout(background, 3, 1, spacing);
	leftbox->setRowSpacing(0, 40);
	leftbox->setRowSpacing(2, 60);
	rightbox->setRowSpacing(0, 40);
	rightbox->setRowSpacing(1, 334);
	rightbox->setRowSpacing(2, 60);
	leftbox->addWidget(lbl_info1, 0, 0);
	rightbox->addWidget(lbl_info2, 0, 0);
	rightbox->addWidget(plot, 1, 0);
	QGridLayout *controlGrid = new QGridLayout(leftbox, rows, columns, spacing);

	for (int i=0; i<rows-1; i++)
	{
		controlGrid->setRowSpacing(i, 26);
	}
	controlGrid->setColSpacing(0, 120);
	controlGrid->setColSpacing(1, 120);

	controlGrid->addWidget(lbl_genes, j, 0);
	controlGrid->addWidget(cnt_genes, j, 1);
	j++;
	controlGrid->addWidget(lbl_demes, j, 0);
	controlGrid->addWidget(cnt_demes, j, 1);
	j++;
	controlGrid->addWidget(lbl_crossover, j, 0);
	controlGrid->addWidget(cnt_crossover, j, 1);
	j++;
	controlGrid->addWidget(lbl_mutation, j, 0);
	controlGrid->addWidget(cnt_mutation, j, 1);
	j++;
	controlGrid->addWidget(lbl_plague, j, 0);
	controlGrid->addWidget(cnt_plague, j, 1);
	j++;
	controlGrid->addWidget(lbl_elitism, j, 0);
	controlGrid->addWidget(cnt_elitism, j, 1);
	j++;
	controlGrid->addWidget(lbl_random_seed, j, 0);
	controlGrid->addWidget(cnt_random_seed, j, 1);
	j++;
	controlGrid->addWidget(lbl_regularization, j, 0);
	controlGrid->addWidget(cnt_regularization, j, 1);
	j++;
	controlGrid->addWidget(lbl_generations, j, 0);
	controlGrid->addWidget(cnt_generations, j, 1);
	j++;
	controlGrid->addWidget(lbl_initial_solutes, j, 0);
	controlGrid->addWidget(cnt_initial_solutes, j, 1);
	j++;
	controlGrid->addWidget(lbl_ff0_min, j, 0);
	controlGrid->addWidget(cnt_ff0_min, j, 1);
	j++;
	controlGrid->addWidget(lbl_ff0_max, j, 0);
	controlGrid->addWidget(cnt_ff0_max, j, 1);
	j++;
	controlGrid->addWidget(cb_meniscus, j, 0);
	j++;
	controlGrid->addWidget(lbl_meniscus_range, j, 0);
	controlGrid->addWidget(cnt_meniscus_range, j, 1);
	j++;
	controlGrid->addWidget(pb_help, j, 0);
	controlGrid->addWidget(pb_load_distro, j, 1);
	j++;
	controlGrid->addWidget(pb_assign_peaks, j, 0);
	controlGrid->addWidget(pb_reset_peaks, j, 1);
	j++;
	controlGrid->addWidget(pb_reject, j, 0);
	controlGrid->addWidget(pb_close, j, 1);

	leftbox->addMultiCellWidget(lb_solutes, 2, 2, 0, 1);
	rightbox->addWidget(textwindow, 2, 0);
}

void US_Ga_GridControl::load_distro()
{
	QFile f;
	unsigned int index;
	float temp1, temp2, temp3;
	QString filename, str;
	sval.clear();
	frequency.clear();
	ff0.clear();
	distro_solute.clear();
	Solute temp_solute;
	filename = QFileDialog::getOpenFileName(USglobal->config_list.result_dir, "*.vhw_his.* *.fe_dis.* *.cofs_dis.* *.sa2d_dis.*", 0);
	index = filename.findRev(".", -1, true);
	str = filename.right(filename.length() - index);
	index = filename.find(".", 0, true);
	id = filename.left(index) + str;
	f.setName(filename);
	if (filename.contains("vhw_his", false))
	{
		distro_type = 0;
	}
	else if (filename.contains("cofs_dis", false))
	{
		distro_type = 1;
	}
	else if (filename.contains("fe_dis", false))
	{
		distro_type = 2;
	}
	else if (filename.contains("sa2d_dis", false))
	{
		distro_type = 3;
	}
	else
	{
		distro_type = -1; // undefined
	}
	if (distro_type == 0)
	{
		if(f.open(IO_ReadOnly))
		{
			QTextStream ts(&f);
			if (!ts.atEnd())
			{
				ts.readLine(); // discard header line
			}
			while (!ts.atEnd())
			{
				ts >> temp1;
				ts >> temp2;
				ts.readLine(); // rest of line
				sval.push_back(temp1);
				frequency.push_back(temp2);
			}
		}
	}
	if (distro_type > 0)
	{
		if(f.open(IO_ReadOnly))
		{
			QTextStream ts(&f);
			if (!ts.atEnd())
			{
				ts.readLine(); // discard header line
			}
			while (!ts.atEnd())
			{
				ts >> temp1; // s_apparent
				ts >> temp1; // s_20,W
				temp1 *= 1.0e13; // change to proper scale
				ts >> temp2; // D_apparent
				ts >> temp2; // D_20,W
				ts >> temp2; // MW
				ts >> temp2; // Frequency
				ts >> temp3; // f/f0
				temp_solute.s = temp1;
				sval.push_back(temp1);
				temp_solute.c = temp2;
				frequency.push_back(temp2);
				temp_solute.k = temp3;
				distro_solute.push_back(temp_solute);
				cerr << temp1 << ", " << temp2 << ", " << temp3 << endl;
			}
		}
		distro_solute.sort();
		list <struct Solute>::iterator j;
		float low_s, high_s, val_k, low_k=1.0e6, high_k = -1.0e6;
		for (j = distro_solute.begin(); j != distro_solute.end(); j++)
		{
			if ((*j).k > high_k)
			{
				high_k = (*j).k;
			}
			if ((*j).k < low_k)
			{
				low_k = (*j).k;
			}
		}
		val_k = high_k - low_k;
		j = distro_solute.begin();
		low_s = (*j).s;
		low_k = (*j).k;
		j = distro_solute.end();
		j --;
		high_s = (*j).s;
		high_k = (*j).k;
		float val_s = high_s - low_s;
		if (val_s > 0)
		{
			s_range = 0.9 * val_s / distro_solute.size();
		}
		else
		{
			j = distro_solute.begin();
			s_range = (*j).s * 0.9;
		}
		if (val_k > 0)
		{
			k_range = 0.9 * val_k / distro_solute.size();
		}
		else
		{
			j = distro_solute.begin();
			k_range = (*j).k * 0.9;
		}
		calc_distro();
	}
	plot->clear();
	double *x, *y;
	unsigned int curve, i;
	x = new double [sval.size()];
	y = new double [sval.size()];
	for (i=0; i<sval.size(); i++)
	{
		x[i] = sval[i];
		y[i] = frequency[i];
	}
	curve = plot->insertCurve("distro");
	plot->setCurveData(curve, x, y, sval.size());
	plot->setCurvePen(curve, QPen(yellow, 3, SolidLine));
	plot->setCurveStyle(curve, QwtCurve::Sticks);
	delete [] x;
	delete [] y;
	plot->setAxisTitle(QwtPlot::xBottom, tr("Sedimentation Coefficient corrected for water at 20ºC"));
	plot->setAxisTitle(QwtPlot::yLeft, tr("Relative Frequency"));
	plot->setAxisAutoScale(QwtPlot::xBottom);
	plot->replot();
	textwindow->setText(tr("Now either auto-assign the solute bins, or manually select bins"
								  " by clicking first on the lower and then on the upper limit of"
								  " the bin range. If you auto-assign the bins you should first"
								  " select the number of solute bins you want to use. UltraScan will"
								  " space the bins proportional to the integral value of each peak,"
								  " such that each bin contains the same integral value."
								  " You can select each solute bin from the listbox on the left"
								  " and modify the frictional ratio limits by selecting them first"
								  " with the respective counters. To change the frictional ratios for"
								  " the solutes, change to the desired f/f0 values in the counter, then"
								  " double-click on the listbox item to change the selected bin accordingly."));
	pb_assign_peaks->setEnabled(true);
}

void US_Ga_GridControl::assign_peaks()
{
	reset_peaks();
	if (distro_type == 0)
	{
		QString str;
		float integral=0.0, range, temp, sum;
		unsigned int i, j, start, stop;
		for (i=0; i<frequency.size(); i++)
		{
			integral += frequency[i];
		}
		i=0;
		while (frequency[i]<1.0e-2)
		{
			i++;
		}
		start = i;
		i = frequency.size() - 1;
		while (frequency[i]<1.0e-2)
		{
			i--;
		}
		stop = i;
		integral = integral/(*GA_Params).initial_solutes;
		range = (sval[stop] - sval[start])/20.0;
		temp = sval[start] - range;
		if (temp < 0)
		{
			temp = 1.0e-16;
		}
		j=0;
		for (i=0; i<(*GA_Params).initial_solutes-1; i++)
		{
			(*GA_Params).solute[i].s_min = temp;
			sum = 0.0;
			while (sum < integral)
			{
				sum += frequency[j];
				j++;
			}
			j--;
			(*GA_Params).solute[i].s_max = sval[j];
			temp = (*GA_Params).solute[i].s_max;
			(*GA_Params).solute[i].s = ((*GA_Params).solute[i].s_max + (*GA_Params).solute[i].s_min)/2.0;
			(*GA_Params).solute[i].ff0_min = ff0_min;
			(*GA_Params).solute[i].ff0_max = ff0_max;
			(*GA_Params).solute[i].ff0 = ((*GA_Params).solute[i].ff0_max + (*GA_Params).solute[i].ff0_min)/2.0;
		}
		j = (*GA_Params).initial_solutes-1;
		(*GA_Params).solute[j].s_min = (*GA_Params).solute[j-1].s_max;
		(*GA_Params).solute[j].s_max = sval[stop] + range;
		(*GA_Params).solute[j].s = ((*GA_Params).solute[j].s_max + (*GA_Params).solute[j].s_min)/2.0;
		(*GA_Params).solute[j].ff0_min = ff0_min;
		(*GA_Params).solute[j].ff0_max = ff0_max;
		(*GA_Params).solute[j].ff0 = ((*GA_Params).solute[j].ff0_max + (*GA_Params).solute[j].ff0_min)/2.0;
		
		double temp_x[3], temp_y[3];
		for (i=0; i<(*GA_Params).initial_solutes; i++)
		{
			temp_x[0] = (*GA_Params).solute[i].s_min;
			temp_x[1] = (*GA_Params).solute[i].s_min;
			temp_x[2] = (*GA_Params).solute[i].s;
			temp_y[0] = 0.0;
			temp_y[1] = plot->axisScale(QwtPlot::yLeft)->hBound()/2.0;
			temp_y[2] = 3.0 * plot->axisScale(QwtPlot::yLeft)->hBound()/4.0;
			limits[i].line1 = plot->insertCurve(str.sprintf("Lower Limit for solute %d", i + 1));
			plot->setCurvePen(limits[i].line1, QPen(red, 2));
			plot->setCurveStyle(limits[i].line1, QwtCurve::Lines);
			plot->setCurveData(limits[i].line1, temp_x, temp_y, 3);
			temp_x[0] = (*GA_Params).solute[i].s_max;
			temp_x[1] = (*GA_Params).solute[i].s_max;
			limits[i].line2 = plot->insertCurve(str.sprintf("Upper Limit for solute %d", i + 1));
			plot->setCurveData(limits[i].line2, temp_x, temp_y, 3);
			plot->setCurvePen(limits[i].line2, QPen(red, 2, SolidLine));
			plot->setCurveStyle(limits[current_solute].line2, QwtCurve::Lines);
			plot->replot();
			str.sprintf("Solute Bin %d: s_min=%4.2e, s_max=%4.2e, f/f0_min=%4.2f, f/f0_max=%4.2f", i+1,
				(*GA_Params).solute[i].s_min,
				(*GA_Params).solute[i].s_max,
				(*GA_Params).solute[i].ff0_min,
				(*GA_Params).solute[i].ff0_max);
			lb_solutes->insertItem(str);
			pb_reset_peaks->setEnabled(true);
		}
	}
	if (distro_type > 0)
	{
		calc_distro();
	}
}

void US_Ga_GridControl::reset_peaks()
{
	current_solute = 0;
	for (unsigned int i=0; i<limits.size(); i++)
	{
		plot->removeCurve(limits[i].line1);
		plot->removeCurve(limits[i].line2);
	}
	plot->replot();
	limits.clear();
	(*GA_Params).solute.clear();
	struct bucket temp_bucket;
	temp_bucket.s = 0.0;
	temp_bucket.s_min = 0.0;
	temp_bucket.s_max = 0.0;
	temp_bucket.ff0 = 0.0;
	temp_bucket.ff0_min = 0.0;
	temp_bucket.ff0_max = 0.0;
	struct line temp_line;
	temp_line.line1 = 0;
	temp_line.line2 = 0;
	for (unsigned int i=0; i<(*GA_Params).initial_solutes; i++)
	{
		limits.push_back(temp_line);
		(*GA_Params).solute.push_back(temp_bucket);
	}
	cnt_initial_solutes->setValue((*GA_Params).initial_solutes);
	lb_solutes->clear();
}

void US_Ga_GridControl::help()
{
	US_Help *online_help;
	online_help = new US_Help(this);
	online_help->show_help("manual/ga_control.html");
}

void US_Ga_GridControl::update_genes(double val)
{
	(*GA_Params).genes = (unsigned int) val;
}

void US_Ga_GridControl::update_demes(double val)
{
	(*GA_Params).demes = (unsigned int) val;
}

void US_Ga_GridControl::update_generations(double val)
{
	(*GA_Params).generations = (unsigned int) val;
}

void US_Ga_GridControl::update_crossover(double val)
{
	(*GA_Params).crossover = (unsigned int) val;
}

void US_Ga_GridControl::update_mutation(double val)
{
	(*GA_Params).mutation = (unsigned int) val;
}

void US_Ga_GridControl::update_plague(double val)
{
	(*GA_Params).plague = (unsigned int) val;
}

void US_Ga_GridControl::update_elitism(double val)
{
	(*GA_Params).elitism = (unsigned int) val;
}

void US_Ga_GridControl::update_random_seed(double val)
{
	(*GA_Params).random_seed = (unsigned long) val;
}

void US_Ga_GridControl::update_regularization(double val)
{
	(*GA_Params).regularization = (float) val;
}

void US_Ga_GridControl::update_initial_solutes(double val)
{
	(*GA_Params).initial_solutes = (unsigned int) val;
	
	struct bucket temp_bucket;
	temp_bucket.s = 0.0;
	temp_bucket.s_min = 0.0;
	temp_bucket.s_max = 0.0;
	temp_bucket.ff0 = 0.0;
	temp_bucket.ff0_min = 0.0;
	temp_bucket.ff0_max = 0.0;
	struct line temp_line;
	temp_line.line1 = 0;
	temp_line.line2 = 0;
	if (limits.size() > (*GA_Params).initial_solutes) // we need to get rid of the extra lines
	{
		limits.erase(limits.begin() + (*GA_Params).initial_solutes, limits.end());
		(*GA_Params).solute.erase((*GA_Params).solute.begin() + (*GA_Params).initial_solutes, (*GA_Params).solute.end());
	}
	if (limits.size() < (*GA_Params).initial_solutes) // we need to add lines
	{
		for (unsigned int i=limits.size(); i<(*GA_Params).initial_solutes; i++)
		{
			limits.push_back(temp_line);
			(*GA_Params).solute.push_back(temp_bucket);
		}
	}
}

void US_Ga_GridControl::calc_distro()
{
	(*GA_Params).solute.clear();
	struct bucket temp_bucket;
	struct bucket temp_bucket2;
	list <struct bucket> storage; 
	int index1, index2;
	distro_solute.sort();
	vector <struct bucket>::iterator i;
	list <struct Solute>::iterator j;
	list <struct bucket>::iterator k;
	
	j = distro_solute.begin();
	k = storage.begin();
	
	temp_bucket.s = (*j).s;
	temp_bucket.s_min = temp_bucket.s - s_range;
	if (temp_bucket.s_min < 0.1)
	{
		temp_bucket.s_min = 0.1;
	}
	temp_bucket.s_max = temp_bucket.s + s_range;
	temp_bucket.ff0 = (*j).k;
	temp_bucket.ff0_min = temp_bucket.ff0 - k_range;
	if (temp_bucket.ff0_min < 1.0)
	{
		temp_bucket.ff0_min = 1.0;
	}
	temp_bucket.ff0_max = temp_bucket.ff0 + k_range;
	temp_bucket.status = 0;
	temp_bucket.conc = (*j).c;
	storage.clear();
	storage.push_back(temp_bucket);
	j++;
	index1 = 0;
	while (j != distro_solute.end())
	{
		storage.sort(); // take the filled storage vector (perhaps containing some new solutes) and sort on "s"
		(*GA_Params).solute.clear(); // empty the GA vector
		for (k = storage.begin(); k != storage.end(); k++)
		{
			(*GA_Params).solute.push_back(*k); // repopulate GA solute vector with 
		}
		i = (*GA_Params).solute.begin();
		
		// now set the default values:
		temp_bucket.s			= (*j).s;
		temp_bucket.s_min		= (*j).s - s_range;
		if (temp_bucket.s_min < 0.1)
		{
			temp_bucket.s_min = 0.1;
		}
		temp_bucket.s_max		= (*j).s + s_range;
		temp_bucket.ff0 		= (*j).k;
		temp_bucket.ff0_min	= (*j).k - k_range;
		if (temp_bucket.ff0_min < 1.0)
		{
			temp_bucket.ff0_min = 1.0;
		}
		temp_bucket.ff0_max	= (*j).k + k_range;
		temp_bucket.conc		= (*j).c;
		temp_bucket.status 	= 0;
		index1++;
		cerr << "Index 1: " << index1 << endl;
		// if there are any overlaps with lower or equal s-value buckets
		// the default values get trimmed back to the non-overlapping region. 
		// If there are s-value offsets, a new solute gets added.
		index2 = 0;
cerr << "starting with i: " << (*i).s_min << ", " << (*i).s_max << " -- "  << (*i).ff0_min << ", " << (*i).ff0_max << "\n";
cerr << "    temp_bucket: " << temp_bucket.s_min << ", " << temp_bucket.s_max << " -- "  << temp_bucket.ff0_min << ", " << temp_bucket.ff0_max << "\n\n";
		while (i != (*GA_Params).solute.end())
		{
			index2++;
			cerr << "Index 2: " << index2 << endl;
			// s values are equal, f values overlap (new solute has higher f)
cerr << "continuing with i: " << (*i).s_min << ", " << (*i).s_max << " -- "  << (*i).ff0_min << ", " << (*i).ff0_max << "\n";
cerr << "      temp_bucket: " << temp_bucket.s_min << ", " << temp_bucket.s_max << " -- "  << temp_bucket.ff0_min << ", " << temp_bucket.ff0_max << "\n\n";
			if (((*i).s_min == temp_bucket.s_min) 
			&&  ((*i).s_max == temp_bucket.s_max)
			&&  ((*i).ff0_max > temp_bucket.ff0_min)) 
			{
				temp_bucket.ff0_min	= (*i).ff0_max;
				temp_bucket.ff0 		= temp_bucket.ff0_min + (temp_bucket.ff0_max - temp_bucket.ff0_min)/2.0;
				temp_bucket.status	= 1;
cerr << "Case 1: " << temp_bucket.s_min << ", " << temp_bucket.s_max << " -- "  << temp_bucket.ff0_min << ", " << temp_bucket.ff0_max << "\n";
cerr << "        " << (*i).s_min << ", " << (*i).s_max << " -- "  << (*i).ff0_min << ", " << (*i).ff0_max << "\n\n";
			}
			else if (((*i).s_min == temp_bucket.s_min) 
					&& ((*i).s_max == temp_bucket.s_max)
					&& ((*i).ff0_min < temp_bucket.ff0_max))
			{
				temp_bucket.ff0_max	= (*i).ff0_min;
				temp_bucket.ff0 		= temp_bucket.ff0_min + (temp_bucket.ff0_max - temp_bucket.ff0_min)/2.0;
				temp_bucket.status	= 1;
cerr << "Case 2: " << temp_bucket.s_min << ", " << temp_bucket.s_max << " -- "  << temp_bucket.ff0_min << ", " << temp_bucket.ff0_max << "\n";
cerr << "        " << (*i).s_min << ", " << (*i).s_max << " -- "  << (*i).ff0_min << ", " << (*i).ff0_max << "\n\n";
			}			
			// f values are equal, s values overlap
			else if (((*i).ff0_min == temp_bucket.ff0_min) 
					&& ((*i).ff0_max == temp_bucket.ff0_max)
					&& ((*i).s_max > temp_bucket.s_min)) 
			{
				temp_bucket.s_min		= (*i).s_max;
				temp_bucket.s	 		= temp_bucket.s_min + (temp_bucket.s_max - temp_bucket.s_min)/2.0;
				temp_bucket.status	= 1;
cerr << "Case 3: " << temp_bucket.s_min << ", " << temp_bucket.s_max << " -- "  << temp_bucket.ff0_min << ", " << temp_bucket.ff0_max << "\n";
cerr << "        " << (*i).s_min << ", " << (*i).s_max << " -- "  << (*i).ff0_min << ", " << (*i).ff0_max << "\n\n";
			}
			else if (((*i).s_min == temp_bucket.s_min) 
					&& ((*i).s_max == temp_bucket.s_max)
					&& ((*i).ff0_min < temp_bucket.ff0_max))
			{
				temp_bucket.ff0_max	= (*i).ff0_min;
				temp_bucket.ff0 		= temp_bucket.ff0_min + (temp_bucket.ff0_max - temp_bucket.ff0_min)/2.0;
				temp_bucket.status	= 1;
cerr << "Case 4: " << temp_bucket.s_min << ", " << temp_bucket.s_max << " -- "  << temp_bucket.ff0_min << ", " << temp_bucket.ff0_max << "\n";
cerr << "        " << (*i).s_min << ", " << (*i).s_max << " -- "  << (*i).ff0_min << ", " << (*i).ff0_max << "\n\n";
			}
			else if ((temp_bucket.s_min   < (*i).s_max) // s-overlap lower right
					&& (temp_bucket.ff0_max < (*i).ff0_max)
					&& (temp_bucket.ff0_max >= (*i).ff0_min)
					&& (temp_bucket.ff0_min <= (*i).ff0_min))
			{
				temp_bucket2.s_min	= temp_bucket.s_min;
				if (temp_bucket2.s_min < 0.1)
				{
					temp_bucket2.s_min = 0.1;
				}
				temp_bucket2.s_max	= (*i).s_max;
				temp_bucket2.s			= temp_bucket2.s_min + (temp_bucket2.s_max - temp_bucket2.s_min)/2.0;
				temp_bucket2.ff0_min	= temp_bucket.ff0_min;
				if (temp_bucket2.ff0_min < 1.0)
				{
					temp_bucket2.ff0_min = 1.0;
				}
				temp_bucket2.ff0_max	= (*i).ff0_min;
				temp_bucket2.ff0 		= temp_bucket2.ff0_min + (temp_bucket2.ff0_max - temp_bucket2.ff0_min)/2.0;
				temp_bucket2.conc		= (*i).conc;
				temp_bucket2.status 	= 2;
				storage.push_back(temp_bucket2);
				temp_bucket.s_min		= (*i).s_max;
				temp_bucket.status	= 1;
cerr << "Case 5a: " << temp_bucket.s_min << ", " << temp_bucket.s_max << " -- "  << temp_bucket.ff0_min << ", " << temp_bucket.ff0_max << "\n";
cerr << "Case 5b: " << temp_bucket2.s_min << ", " << temp_bucket2.s_max << " -- "  << temp_bucket2.ff0_min << ", " << temp_bucket2.ff0_max << "\n";
cerr << "         " << (*i).s_min << ", " << (*i).s_max << " -- "  << (*i).ff0_min << ", " << (*i).ff0_max << "\n\n";
			}	
			else if ((temp_bucket.s_min   <  (*i).s_max) // s-overlap upper right
					&& (temp_bucket.ff0_max >  (*i).ff0_max)
					&& (temp_bucket.ff0_min >= (*i).ff0_min)
					&& (temp_bucket.ff0_min <=  (*i).ff0_max))
			{
				temp_bucket2.s_min	= temp_bucket.s_min;
				if (temp_bucket2.s_min < 0.1)
				{
					temp_bucket2.s_min = 0.1;
				}
				temp_bucket2.s_max	= (*i).s_max;
				temp_bucket2.s			= temp_bucket2.s_min + (temp_bucket2.s_max - temp_bucket2.s_min)/2.0;
				temp_bucket2.ff0_min	= (*i).ff0_max;
				if (temp_bucket2.ff0_min < 1.0)
				{
					temp_bucket2.ff0_min = 1.0;
				}
				temp_bucket2.ff0_max	= temp_bucket.ff0_max;
				temp_bucket2.ff0 		= temp_bucket2.ff0_min + (temp_bucket2.ff0_max - temp_bucket2.ff0_min)/2.0;
				temp_bucket2.conc		= (*i).conc;
				temp_bucket2.status 	= 2;
				storage.push_back(temp_bucket2);
				temp_bucket.s_min		= (*i).s_max;
				temp_bucket.status	= 1;
cerr << "Case 6a: " << temp_bucket.s_min << ", " << temp_bucket.s_max << " -- "  << temp_bucket.ff0_min << ", " << temp_bucket.ff0_max << "\n";
cerr << "Case 6b: " << temp_bucket2.s_min << ", " << temp_bucket2.s_max << " -- "  << temp_bucket2.ff0_min << ", " << temp_bucket2.ff0_max << "\n";
cerr << "         " << (*i).s_min << ", " << (*i).s_max << " -- "  << (*i).ff0_min << ", " << (*i).ff0_max << "\n\n";
			}	
			else if ((temp_bucket.s_min   < (*i).s_max) // s-overlap middle
					&& (temp_bucket.ff0_max < (*i).ff0_max)
					&& (temp_bucket.ff0_min > (*i).ff0_min))
			{
				temp_bucket.s_min		= (*i).s_max;
				temp_bucket.status	= 1;
cerr << "Case 7: " << temp_bucket.s_min << ", " << temp_bucket.s_max << " -- "  << temp_bucket.ff0_min << ", " << temp_bucket.ff0_max << "\n";
cerr << "        " << (*i).s_min << ", " << (*i).s_max << " -- "  << (*i).ff0_min << ", " << (*i).ff0_max << "\n\n";
			}
			else if ((temp_bucket.s_min   < (*i).s_max) // s-overlap, new solute bucket is larger
					&& (temp_bucket.ff0_max > (*i).ff0_max)
					&& (temp_bucket.ff0_min < (*i).ff0_min))
			{
				temp_bucket2.s_min	= temp_bucket.s_min;
				if (temp_bucket2.s_min < 0.1)
				{
					temp_bucket2.s_min = 0.1;
				}
				temp_bucket2.s_max	= (*i).s_max;
				temp_bucket2.s			= temp_bucket2.s_min + (temp_bucket2.s_max - temp_bucket2.s_min)/2.0;
				temp_bucket2.ff0_min	= temp_bucket.ff0_min;
				if (temp_bucket2.ff0_min < 1.0)
				{
					temp_bucket2.ff0_min = 1.0;
				}
				temp_bucket2.ff0_max	= (*i).ff0_min;
				temp_bucket2.ff0 		= temp_bucket2.ff0_min + (temp_bucket2.ff0_max - temp_bucket2.ff0_min)/2.0;
				temp_bucket2.conc		= (*i).conc;
				temp_bucket2.status 	= 2;
				storage.push_back(temp_bucket2);
				temp_bucket2.s_min	= temp_bucket.s_min;
				if (temp_bucket2.s_min < 0.1)
				{
					temp_bucket2.s_min = 0.1;
				}
cerr << "Case 8a: " << temp_bucket2.s_min << ", " << temp_bucket2.s_max << " -- "  << temp_bucket2.ff0_min << ", " << temp_bucket2.ff0_max << "\n";
cerr << "         " << (*i).s_min << ", " << (*i).s_max << " -- "  << (*i).ff0_min << ", " << (*i).ff0_max << "\n\n";
				temp_bucket2.s_max	= (*i).s_max;
				temp_bucket2.s			= temp_bucket2.s_min + (temp_bucket2.s_max - temp_bucket2.s_min)/2.0;
				temp_bucket2.ff0_min	= (*i).ff0_min;
				if (temp_bucket2.ff0_min < 1.0)
				{
					temp_bucket2.ff0_min = 1.0;
				}
				temp_bucket2.ff0_max	= temp_bucket.ff0_max;
				temp_bucket2.ff0 		= temp_bucket2.ff0_min + (temp_bucket2.ff0_max - temp_bucket2.ff0_min)/2.0;
				temp_bucket2.conc		= (*i).conc;
				temp_bucket2.status 	= 2;
				storage.push_back(temp_bucket2);
				temp_bucket.s_min		= (*i).s_max;
				temp_bucket.status	= 1;
cerr << "Case 8b: " << temp_bucket2.s_min << ", " << temp_bucket2.s_max << " -- "  << temp_bucket2.ff0_min << ", " << temp_bucket2.ff0_max << "\n";
cerr << "Case 7c: " << temp_bucket.s_min << ", " << temp_bucket.s_max << " -- "  << temp_bucket.ff0_min << ", " << temp_bucket.ff0_max << "\n";
cerr << "         " << (*i).s_min << ", " << (*i).s_max << " -- "  << (*i).ff0_min << ", " << (*i).ff0_max << "\n\n";
			}	
			if (temp_bucket.s_min < 0.1)
			{
				temp_bucket.s_min = 0.1;
			}
			if (temp_bucket.ff0_min < 1.0)
			{
				temp_bucket.ff0_min = 1.0;
			}
			i++;
		}
		storage.push_back(temp_bucket);
		j++;
	}	
}

void US_Ga_GridControl::update_k_range(double val)
{
	k_range = (float) val;
	calc_distro();
}

void US_Ga_GridControl::update_s_range(double val)
{
	s_range = (float) val;
	calc_distro();
}

void US_Ga_GridControl::update_ff0_min(double val)
{
	ff0_min = (float) val;
}

void US_Ga_GridControl::update_ff0_max(double val)
{
	ff0_max = (float) val;
}

void US_Ga_GridControl::update_meniscus_range(double val)
{
	(*GA_Params).meniscus_range = (float) val;
}

void US_Ga_GridControl::set_meniscus()
{
	if ((*GA_Params).fit_meniscus)
	{
		(*GA_Params).fit_meniscus = false;
		cnt_meniscus_range->setEnabled(false);
		(*GA_Params).meniscus_range = 0.0;
		cnt_meniscus_range->setValue(0.0);
	}
	else
	{
		(*GA_Params).fit_meniscus = true;
		cnt_meniscus_range->setEnabled(true);
	}
}

void US_Ga_GridControl::edit_solute(int index)
{
	QString str;
	(*GA_Params).solute[index].ff0_min = ff0_min;
	(*GA_Params).solute[index].ff0_max = ff0_max;
	(*GA_Params).solute[index].ff0 = ((*GA_Params).solute[index].ff0_min + (*GA_Params).solute[index].ff0_max)/2.0;
	str.sprintf("Solute Bin %d: s_min=%4.2e, s_max=%4.2e, f/f0_min=%4.2f, f/f0_max=%4.2f",
	index + 1,
	(*GA_Params).solute[index].s_min,
	(*GA_Params).solute[index].s_max,
	(*GA_Params).solute[index].ff0_min,
	(*GA_Params).solute[index].ff0_max);
	lb_solutes->changeItem(str, index);
}

void US_Ga_GridControl::getMouseReleased(const QMouseEvent &e)
{
	QString str;
	if (current_solute == 0)
	{
		(*GA_Params).initial_solutes = 1;
		cnt_initial_solutes->setValue((*GA_Params).initial_solutes);
	}
	if (current_solute >= (*GA_Params).initial_solutes) // the user wants another bucket
	{
		(*GA_Params).initial_solutes++;
		cnt_initial_solutes->setValue((*GA_Params).initial_solutes);
	}
	double s = (plot->invTransform(QwtPlot::xBottom, e.x()));
	if (current_solute > 0 && s < (*GA_Params).solute[current_solute-1].s_max)
	{
		s = (*GA_Params).solute[current_solute-1].s_max;
/*
		QMessageBox::information(this, tr("Attention:"), 
		tr("Your selected s-value overlaps the previous bin.\n"
		"Overlapping bins will cause unnecessary calculations\n"
		"in the genetic algorithm calculation.\n\n"
		"Resetting the lower s-value limit to: ") 
		+ str.sprintf("%e", (*GA_Params).solute[current_solute-1].s_max) + " s");
*/
	}
	double temp_x[3];
	double temp_y[3];
	temp_x[0] = s;
	temp_x[1] = s;
	temp_x[2] = s;
	
	temp_y[0] = 0.0;
	temp_y[1] = plot->axisScale(QwtPlot::yLeft)->hBound();
	temp_y[2] = plot->axisScale(QwtPlot::yLeft)->hBound();
	if(!minmax)
	{
		limits[current_solute].line1 = 
		plot->insertCurve(str.sprintf("Lower Limit for solute %d", current_solute + 1));
		plot->setCurvePen(limits[current_solute].line1, QPen(red, 2));
		plot->setCurveStyle(limits[current_solute].line1, QwtCurve::Lines);
		plot->setCurveData(limits[current_solute].line1, temp_x, temp_y, 3);
		(*GA_Params).solute[current_solute].s_min = s;
		(*GA_Params).solute[current_solute].ff0_min = ff0_min;
		minmax = true;
	}
	else
	{
		if (s < (*GA_Params).solute[current_solute].s_min)
		{
			QMessageBox::information(this, tr("Attention:"), tr("The upper s-value limit should"
			"\nbe higher than the lower limit,\nwhich is: ") 
			+ str.sprintf("%e", (*GA_Params).solute[current_solute].s_min) + " s");
			return;
		}
		(*GA_Params).solute[current_solute].s_max = s;
		(*GA_Params).solute[current_solute].s = (*GA_Params).solute[current_solute].s_min + (s - (*GA_Params).solute[current_solute].s_min)/2.0;
		plot->removeCurve(limits[current_solute].line1);

		temp_x[0] = (*GA_Params).solute[current_solute].s_min;
		temp_x[1] = (*GA_Params).solute[current_solute].s_min;
		temp_x[2] = (*GA_Params).solute[current_solute].s;
	
		temp_y[0] = 0.0;
		temp_y[1] = plot->axisScale(QwtPlot::yLeft)->hBound()/2.0;
		temp_y[2] = 3.0 * plot->axisScale(QwtPlot::yLeft)->hBound()/4.0;

		limits[current_solute].line1 = 
		plot->insertCurve(str.sprintf("Lower Limit for solute %d", current_solute + 1));
		plot->setCurvePen(limits[current_solute].line1, QPen(red, 2));
		plot->setCurveStyle(limits[current_solute].line1, QwtCurve::Lines);
		plot->setCurveData(limits[current_solute].line1, temp_x, temp_y, 3);

		temp_x[0] = s;
		temp_x[1] = s;

		limits[current_solute].line2 = 
		plot->insertCurve(str.sprintf("Upper Limit for solute %d", current_solute + 1));
		plot->setCurveData(limits[current_solute].line2, temp_x, temp_y, 3);
		plot->setCurvePen(limits[current_solute].line2, QPen(red, 2, SolidLine));
		plot->setCurveStyle(limits[current_solute].line2, QwtCurve::Lines);
		(*GA_Params).solute[current_solute].s_max = s;
		(*GA_Params).solute[current_solute].ff0_min = ff0_min;
		(*GA_Params).solute[current_solute].ff0_max = ff0_max;
		(*GA_Params).solute[current_solute].ff0 = ((*GA_Params).solute[current_solute].ff0_min + (*GA_Params).solute[current_solute].ff0_max)/2.0;
		(*GA_Params).solute[current_solute].s = (*GA_Params).solute[current_solute].s_min;
		minmax = false;
		str.sprintf("Solute Bin %d: s_min=%4.2e, s_max=%4.2e, f/f0_min=%4.2f, f/f0_max=%4.2f",
						current_solute + 1,
						(*GA_Params).solute[current_solute].s_min,
						(*GA_Params).solute[current_solute].s_max,
						(*GA_Params).solute[current_solute].ff0_min,
						(*GA_Params).solute[current_solute].ff0_max);
		lb_solutes->insertItem(str);
		current_solute ++;
		pb_reset_peaks->setEnabled(true);
	}
	plot->replot();
}

void US_Ga_GridControl::getMousePressed(const QMouseEvent &)
{
}

void US_Ga_GridControl::closeEvent(QCloseEvent *e)
{
	global_Xpos -= 30;
	global_Ypos -= 30;
	e->accept();
}

void US_Ga_GridControl::accept()
{

	QString str;
	str = id + ".gadistro.dat";
	QFile f(str);
	f.open(IO_WriteOnly);
	QTextStream ts(&f);
	ts << (*GA_Params).solute.size() << endl;
	for (unsigned int i=0; i<(*GA_Params).solute.size(); i++)
	{
		ts << (*GA_Params).solute[i].s_min << ", ";
		ts << (*GA_Params).solute[i].s_max << ", ";
		ts << (*GA_Params).solute[i].ff0_min << ", ";
		ts << (*GA_Params).solute[i].ff0_max << "\n";
	}
	f.close();
	global_Xpos -= 30;
	global_Ypos -= 30;
	QDialog::accept();
}

void US_Ga_GridControl::reject()
{
	global_Xpos -= 30;
	global_Ypos -= 30;
	QDialog::reject();
}

