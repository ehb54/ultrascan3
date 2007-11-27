#include "../include/us_sassoc.h"

US_Sassoc::US_Sassoc(float temp_eq1, float temp_eq2, float temp_stoich1, float temp_stoich2,  
QString temp_project, int temp_model, bool temp_status, QWidget *p , const char *name)
: QFrame(p, name)
{
	USglobal = new US_Config();

	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	setCaption(tr("UltraScan: Equilibrium Concentration Distribution"));
	pm = new US_Pixmap();
	QString str;
	
	eq[0] = temp_eq1;
	eq[1] = temp_eq2;
	model = temp_model;
	stoich[0] = temp_stoich1;
	stoich[1] = temp_stoich2;
	project = temp_project;
	status = temp_status;
	
	spacing = 2;
	buttonh = 26;
	buttonw = 200;
	border = 4;
	xpos = 4;
	ypos = 4;
	
	ARRAY_SIZE = 555;
	data = new double *[4];
	for (unsigned int i=0; i<4; i++)
	{
		data[i] = new double [ARRAY_SIZE];
	}
	
	data_plot = new QwtPlot(this);
	Q_CHECK_PTR(data_plot);
	data_plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
	data_plot->enableGridXMin();
	data_plot->enableGridYMin();
	data_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
	data_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
	//data_plot->setPlotBackground(USglobal->global_colors.plot);		//old version
	data_plot->setCanvasBackground(USglobal->global_colors.plot);		//new version
	data_plot->setMargin(USglobal->config_list.margin);
	data_plot->enableOutline(true);
	data_plot->setAxisTitle(QwtPlot::xBottom, tr("Total Concentration"));
	data_plot->setAxisTitle(QwtPlot::yLeft, tr("% of Total Concentration"));
	data_plot->setAxisOptions(QwtPlot::xBottom, QwtAutoScale::Logarithmic);
	data_plot->setTitle(tr("Self-Association Profile for ") + project);
	data_plot->setOutlinePen(white);
	data_plot->setOutlineStyle(Qwt::VLine);
	data_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
	data_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	data_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	data_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	data_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	data_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	data_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));		
	
	connect(data_plot, SIGNAL(plotMouseMoved(const QMouseEvent &)), SLOT(mouseMoved(const QMouseEvent&)));
	connect(data_plot, SIGNAL(plotMousePressed(const QMouseEvent &)), SLOT(mousePressed(const QMouseEvent&)));
	connect(data_plot, SIGNAL(plotMouseReleased(const QMouseEvent &)), SLOT(mouseReleased(const QMouseEvent&)));
	curve[0] = data_plot->insertCurve("Monomer");
	curve[1] = data_plot->insertCurve("N - mer");
	curve[2] = data_plot->insertCurve("M - mer");
	data_plot->setCurvePen(curve[0], QPen(Qt::green, 2, SolidLine));
	data_plot->setCurvePen(curve[1], QPen(Qt::yellow, 2, SolidLine));
	data_plot->setCurvePen(curve[2], QPen(Qt::cyan, 2, SolidLine));

	lbl_banner1 = new QLabel(tr("Species Distribution in\nSelf-Associating System"), this);
	Q_CHECK_PTR(lbl_banner1);
	lbl_banner1->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_banner1->setAlignment(AlignCenter|AlignVCenter);
	lbl_banner1->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_banner1->setGeometry(xpos, ypos, buttonw, 2 * buttonh + spacing);
	lbl_banner1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	ypos += 2 * buttonh + 2 * spacing;
	
	pb_help = new QPushButton(tr("Help"), this);
	Q_CHECK_PTR(pb_help);
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_help->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	ypos += buttonh + spacing;
	
	pb_recalc = new QPushButton(tr("Update Graphs"), this);
	Q_CHECK_PTR(pb_recalc);
	pb_recalc->setAutoDefault(false);
	pb_recalc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_recalc->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_recalc->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_recalc, SIGNAL(clicked()), SLOT(recalc()));

	ypos += buttonh + spacing;
	
	pb_write = new QPushButton(tr("Save Data to File"), this);
	Q_CHECK_PTR(pb_write);
	pb_write->setAutoDefault(false);
	pb_write->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_write->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_write->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_write, SIGNAL(clicked()), SLOT(write_data()));

	ypos += buttonh + spacing;
	
	pb_print = new QPushButton(tr("Print Plot Window"), this);
	Q_CHECK_PTR(pb_print);
	pb_print->setAutoDefault(false);
	pb_print->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_print->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_print->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_print, SIGNAL(clicked()), SLOT(print()));

	ypos += buttonh + spacing;
	
	pb_cancel = new QPushButton(tr("Close"), this);
	Q_CHECK_PTR(pb_cancel);
	pb_cancel->setAutoDefault(false);
	pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_cancel->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

	ypos += buttonh + 2 * spacing;
	
	lbl_banner2 = new QLabel(tr("Stoichiometries:"), this);
	Q_CHECK_PTR(lbl_banner2);
	lbl_banner2->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_banner2->setAlignment(AlignCenter|AlignVCenter);
	lbl_banner2->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_banner2->setGeometry(xpos, ypos, buttonw, buttonh);
	lbl_banner2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	ypos += buttonh + 2 * spacing;

	lbl_stoich1 = new QLabel(tr(" Association 1:"), this);
	Q_CHECK_PTR(lbl_stoich1);
	lbl_stoich1->setAlignment(AlignLeft|AlignVCenter);
	lbl_stoich1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_stoich1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_stoich1->setGeometry(xpos, ypos, buttonw - 60, buttonh);

	xpos += buttonw - 90 + spacing;
	
	le_stoich1 = new QLineEdit(this, "stoich1");
	le_stoich1->setGeometry(xpos, ypos, 88, buttonh);
	le_stoich1->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_stoich1->setText(str.sprintf(" %2.4f", stoich[0]));
	if(!status)
	{
		le_stoich1->setReadOnly(true);
	}
	connect(le_stoich1, SIGNAL(textChanged(const QString &)), 
				SLOT(update_stoich1(const QString &)));	

	ypos += buttonh + spacing;
	xpos = border;

	lbl_stoich2 = new QLabel(tr(" Association 2:"), this);
	Q_CHECK_PTR(lbl_stoich2);
	lbl_stoich2->setAlignment(AlignLeft|AlignVCenter);
	lbl_stoich2->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_stoich2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_stoich2->setGeometry(xpos, ypos, buttonw - 60, buttonh);

	xpos += buttonw - 90 + spacing;
	
	le_stoich2 = new QLineEdit(this, "stoich2");
	le_stoich2->setGeometry(xpos, ypos, 88, buttonh);
	le_stoich2->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_stoich2->setText(str.sprintf(" %2.4f", stoich[1]));
	if(!status)
	{
		le_stoich2->setReadOnly(true);
	}
	connect(le_stoich2, SIGNAL(textChanged(const QString &)), 
				SLOT(update_stoich2(const QString &)));	

	xpos = border;
	ypos += buttonh + 2 * spacing;
	
	lbl_banner3 = new QLabel(tr("Equilibrium Constants:"), this);
	Q_CHECK_PTR(lbl_banner3);
	lbl_banner3->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_banner3->setAlignment(AlignCenter|AlignVCenter);
	lbl_banner3->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_banner3->setGeometry(xpos, ypos, buttonw, buttonh);
	lbl_banner3->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	ypos += buttonh + 2 * spacing;

	lbl_eq1 = new QLabel(tr(" ln(Constant 1):"), this);
	Q_CHECK_PTR(lbl_eq1);
	lbl_eq1->setAlignment(AlignLeft|AlignVCenter);
	lbl_eq1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_eq1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_eq1->setGeometry(xpos, ypos, buttonw - 60, buttonh);

	xpos += buttonw - 90 + spacing;
	
	le_eq1 = new QLineEdit(this, "eq1");
	le_eq1->setGeometry(xpos, ypos, 88, buttonh);
	le_eq1->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_eq1->setText(str.sprintf(" %2.3e", eq[0]));
	if(!status)
	{
		le_eq1->setReadOnly(true);
	}
	connect(le_eq1, SIGNAL(textChanged(const QString &)), 
				SLOT(update_eq1(const QString &)));	

	xpos = border;
	ypos += buttonh + spacing;

	cnt_eq1= new QwtCounter(this);
	Q_CHECK_PTR(cnt_eq1);
	cnt_eq1->setRange(-100, 100, 0.01);
	cnt_eq1->setNumButtons(3);
	cnt_eq1->setValue(eq[0]);
	cnt_eq1->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_eq1->setGeometry(xpos, ypos, buttonw, buttonh);
	cnt_eq1->setEnabled(status);
//	connect(cnt_eq1, SIGNAL(buttonReleased(double)), SLOT(update_eq1Count(double)));
	connect(cnt_eq1, SIGNAL(valueChanged(double)), SLOT(update_eq1Count(double)));

	ypos += buttonh + spacing;

	lbl_eq2 = new QLabel(tr(" ln(Constant 2):"), this);
	Q_CHECK_PTR(lbl_eq2);
	lbl_eq2->setAlignment(AlignLeft|AlignVCenter);
	lbl_eq2->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_eq2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_eq2->setGeometry(xpos, ypos, buttonw - 60, buttonh);

	xpos += buttonw - 90 + spacing;
	
	le_eq2 = new QLineEdit(this, "eq2");
	le_eq2->setGeometry(xpos, ypos, 88, buttonh);
	le_eq2->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_eq2->setText(str.sprintf(" %2.3e", eq[1]));
	if(!status)
	{
		le_eq2->setReadOnly(true);
	}
	connect(le_eq2, SIGNAL(textChanged(const QString &)), 
				SLOT(update_eq2(const QString &)));	

	xpos = border;
	ypos += buttonh + spacing;

	cnt_eq2= new QwtCounter(this);
	Q_CHECK_PTR(cnt_eq2);
	cnt_eq2->setRange(-100, 100, 0.01);
	cnt_eq2->setNumButtons(3);
	cnt_eq2->setValue(eq[1]);
	cnt_eq2->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_eq2->setGeometry(xpos, ypos, buttonw, buttonh);
	cnt_eq2->setEnabled(status);
//	connect(cnt_eq2, SIGNAL(buttonReleased(double)), SLOT(update_eq2Count(double)));
	connect(cnt_eq2, SIGNAL(valueChanged(double)), SLOT(update_eq2Count(double)));

	ypos += buttonh + 2 * spacing;

	lbl_banner4 = new QLabel(tr("Equilibrium Model:"), this);
	Q_CHECK_PTR(lbl_banner4);
	lbl_banner4->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_banner4->setAlignment(AlignCenter|AlignVCenter);
	lbl_banner4->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_banner4->setGeometry(xpos, ypos, buttonw, buttonh);
	lbl_banner4->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	ypos += buttonh + spacing;
	
	mle_model = new QTextEdit(this);
	mle_model->setWordWrap(QTextEdit::WidgetWidth);
	mle_model->setFrameStyle(QFrame::WinPanel|Sunken);
	mle_model->insert(modelString[model]);
	mle_model->setGeometry(xpos, ypos, buttonw, 35);
	mle_model->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	mle_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	mle_model->setReadOnly(true);

	ypos += 35 + 2 * spacing;

	lbl_banner5 = new QLabel(tr("Project Name:"), this);
	Q_CHECK_PTR(lbl_banner5);
	lbl_banner5->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_banner5->setAlignment(AlignCenter|AlignVCenter);
	lbl_banner5->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_banner5->setGeometry(xpos, ypos, buttonw, buttonh);
	lbl_banner5->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	ypos += buttonh + spacing;
	
	lbl_project = new QLabel(project, this);
	lbl_project->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_project->setGeometry(xpos, ypos, buttonw, buttonh);
	lbl_project->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_project->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	sas_l = new US_SassocLegend(this);
	sas_l->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));

	ypos += buttonh + border;

	global_Xpos += 30;
	global_Ypos += 30;

	move(global_Xpos, global_Ypos);
	setMinimumSize(812, ypos);
	recalc();
	
	setup_GUI();
}

US_Sassoc::~US_Sassoc()
{
	for (unsigned int i=0; i<4; i++)
	{
		delete [] data[i];
	}
	delete [] data;
}

void US_Sassoc::setup_GUI()
{
	int j=0;
	int rows = 19, columns = 2, spacing = 2;
	
	QGridLayout * background = new QGridLayout(this,2,2,spacing);
	background->setMargin(spacing);	
	QGridLayout * subGrid1 = new QGridLayout(rows, columns, spacing);
	for (int i=0; i<rows; i++)
	{
		subGrid1->setRowSpacing(i, 26);
	}
	subGrid1->addMultiCellWidget(lbl_banner1,j,j+1,0,1);
	j=j+2;
	subGrid1->addMultiCellWidget(pb_help,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(pb_recalc,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(pb_write,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(pb_print,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(pb_cancel,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(lbl_banner2,j,j,0,1);
	j++;
	subGrid1->addWidget(lbl_stoich1,j,0);
	subGrid1->addWidget(le_stoich1,j,1);
	j++;
	subGrid1->addWidget(lbl_stoich2,j,0);
	subGrid1->addWidget(le_stoich2,j,1);
	j++;
	subGrid1->addMultiCellWidget(lbl_banner3,j,j,0,1);
	j++;
	subGrid1->addWidget(lbl_eq1,j,0);
	subGrid1->addWidget(le_eq1,j,1);
	j++;
	subGrid1->addMultiCellWidget(cnt_eq1,j,j,0,1);
	j++;
	subGrid1->addWidget(lbl_eq2,j,0);
	subGrid1->addWidget(le_eq2,j,1);
	j++;
	subGrid1->addMultiCellWidget(cnt_eq2,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(lbl_banner4,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(mle_model,j,j+1,0,1);
	j++;
	subGrid1->addMultiCellWidget(lbl_banner5,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(lbl_project,j,j,0,1);
	
	background->addMultiCell(subGrid1,0,1,0,0);
	background->addWidget(data_plot,0,1);
	background->addWidget(sas_l,1,1);
	background->setColStretch(0,1);
	background->setColStretch(1,4);
	background->setColSpacing(1,800);

	qApp->processEvents();
	QRect r = background->cellGeometry(0, 0);
	
	global_Xpos += 30;
	global_Ypos += 30;
	
	this->setGeometry(global_Xpos, global_Ypos, r.width()+650+spacing*3, r.height());

}

void US_Sassoc::closeEvent(QCloseEvent *e)
{
	global_Xpos -= 30;
	global_Ypos -= 30;
	e->accept();
}

/*
void US_Sassoc::resizeEvent(QResizeEvent *e)
{
	int legendheight = 2 * buttonh + 2 * border + spacing;
	int panelwidth = 2 * border + buttonw;
	QRect r(panelwidth, border, e->size().width() - (panelwidth + border),
	e->size().height() - legendheight);
	data_plot->setGeometry(r);
	sas_l->setGeometry(panelwidth, e->size().height()-legendheight, e->size().width() - (panelwidth + border),
	legendheight);
}
*/
void US_Sassoc::print()
{
	QPrinter printer;
	bool print_bw =  false;
	bool print_inv =  false;
	US_SelectPlot *sp;
	sp = new US_SelectPlot(&print_bw, &print_inv);
	sp->exec();
	if	(printer.setup(0))
	{
		if (print_bw)
		{
			PrintFilter pf;
			pf.setOptions(QwtPlotPrintFilter::PrintTitle
							 |QwtPlotPrintFilter::PrintMargin
							 |QwtPlotPrintFilter::PrintLegend
							 |QwtPlotPrintFilter::PrintGrid);
			data_plot->print(printer, pf);
		}
		else if (print_inv)
		{
			PrintFilterDark pf;
			pf.setOptions(QwtPlotPrintFilter::PrintTitle
							 |QwtPlotPrintFilter::PrintMargin
							 |QwtPlotPrintFilter::PrintLegend
							 |QwtPlotPrintFilter::PrintGrid);
			data_plot->print(printer, pf);
		}
		else
		{
			data_plot->print(printer);
		}
	}
}

void US_Sassoc::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/sassoc.html");
}

void US_Sassoc::write_data()
{
	QString str;
	QString fileName = USglobal->config_list.result_dir + "/" + project + "-" + str.sprintf("%d", model) + ".dis";
	QFile f(fileName);
	if (f.exists())
	{
		if(!QMessageBox::query( tr("Warning"), tr("Attention:\nThis file exists already!\n\nDo you want to overwrite it?"), tr("Yes"), tr("No")))
		{
			f.close();
			return;
		}
	}
	if(f.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&f);
		ts << tr("\"Total Concentration\"\t\"\% Monomer\"\t\"\% ") << stoich[0] << "-mer\"\t\"\% " 
		<< stoich[1] << "-mer\"\t\"\n";
		unsigned int i;
		for (i=0; i<ARRAY_SIZE; i++)
		{
			ts << data[0][i] << "\t";
			ts << data[1][i] << "\t";
			ts << data[2][i] << "\t";
			ts << data[3][i] << "\n";
		}
	}
	f.close();
	createHtmlDir();
	QPixmap p;
	fileName = htmlDir + "/" + project + "-" + str.sprintf("%d", model) + ".distribution.";
	p = QPixmap::grabWidget(data_plot, 2, 2, data_plot->width() - 4, data_plot->height() - 4);
	pm->save_file(fileName, p);
}

void US_Sassoc::cancel()
{
	close();
}

void US_Sassoc::recalc()
{
/*
	data[0][i] = total concentration (x variable, logarithmic)
	data[1][i] = monomer concentration
	data[2][i] = n-mer concentration (stoichiometry1)
	data[3][i] = m-mer concentration (stoichiometry2)
*/
	unsigned int i, inc = 20;
	double c = 1.0e-10;	// Lower concentration range limit
	for (i=0; i<ARRAY_SIZE; i++)
	{
		data[0][i] = (double) c;
		c += c/inc;
		data[1][i] = monomer_root(data[0][i]);
		data[2][i] = stoich[0] * pow((double) data[1][i], (double) stoich[0]) * exp(eq[0]);
		data[3][i] = stoich[1] * pow((double) data[1][i], (double) stoich[1]) * exp(eq[1]);
		data[1][i] *= 100/data[0][i];
		data[2][i] *= 100/data[0][i];
		data[3][i] *= 100/data[0][i];
	}
	data_plot->setCurveData(curve[0], data[0], data[1], ARRAY_SIZE);
	data_plot->setCurveData(curve[1], data[0], data[2], ARRAY_SIZE);
	data_plot->setCurveData(curve[2], data[0], data[3], ARRAY_SIZE);
	data_plot->replot();
}

void US_Sassoc::update_stoich1(const QString &str)
{
	stoich[0] = str.toFloat();
	recalc();
}

void US_Sassoc::update_stoich2(const QString &str)
{
	stoich[1] = str.toFloat();
	recalc();
}

void US_Sassoc::update_eq1(const QString &str)
{
	eq[0] = str.toFloat();
	cnt_eq1->setValue(eq[0]);
	recalc();
}

void US_Sassoc::update_eq2(const QString &str)
{
	eq[1] = str.toFloat();
	cnt_eq2->setValue(eq[1]);
	recalc();
}

void US_Sassoc::update_eq1Count(double val)
{
	QString str;
	eq[0] = (float) val;
	str.sprintf(" %2.3e", val);
	le_eq1->setText(str); // no need to call recalc(), since this will signal textChanged() in lineedit
}

void US_Sassoc::update_eq2Count(double val)
{
	QString str;
	eq[1] = (float) val;
	str.sprintf(" %2.3e", val);
	le_eq2->setText(str); // no need to call recalc(), since this will signal textChanged() in lineedit
}

double US_Sassoc::monomer_root(double total)
{
   double monomer, test, diff1, diff, monomer_old;
   monomer=total/2;
	diff=monomer/2;
	diff1=monomer-diff;
   test=polynomial(monomer, total);
   while (fabs(diff1) > 1e-15)
   {
      monomer_old=monomer;
      if (test > 0) 
      {
         monomer=monomer-diff;
      }
      else
      {   
         monomer=monomer+diff;
      }
      test=polynomial(monomer, total);
      diff=fabs(monomer-monomer_old)/2;
      diff1=monomer_old-monomer;
   }
   return (monomer);
}

double US_Sassoc::polynomial(double monomer, double total)
{
	return(monomer + stoich[0] * pow((double) monomer, (double) stoich[0]) * exp(eq[0]) + stoich[1] * pow((double) monomer, (double) stoich[1]) * exp(eq[1]) -  total);
}

void US_Sassoc::mousePressed(const QMouseEvent &e)
{
    // store position
    point = e.pos();

    // update cursor pos display
    mouseMoved(e);
}


void US_Sassoc::mouseReleased(const QMouseEvent &e)
{
	float xval;
	xval = e.x();
	sas_l->lbl_species1b->setText(" 0 %");
	sas_l->lbl_species2b->setText(" 0 %");
	sas_l->lbl_species3b->setText(" 0 %");
	sas_l->lbl_species4b->setText(" 0 M");
}

void US_Sassoc::mouseMoved(const QMouseEvent &e)
{
	double total;
	QString str;
	unsigned int index = 0;
	total = (data_plot->invTransform(QwtPlot::xBottom, e.x()));
	if (total >= 1e-10 && total <=100 )
	{
		while ((index < ARRAY_SIZE) && (data[0][index] < total))
		{
			index++;
		}
		str.sprintf(" %3.3f ", data[1][index]);
		str += "%";
		sas_l->lbl_species1b->setText(str);
		str.sprintf(" %3.3f ", data[2][index]);
		str += "%";
		sas_l->lbl_species2b->setText(str);
		str.sprintf(" %3.3f ", data[3][index]);
		str += "%";
		sas_l->lbl_species3b->setText(str);
		str.sprintf(" %1.2e ", data[0][index]);
		str += "M";
		sas_l->lbl_species4b->setText(str);
	}
	else
	{
		sas_l->lbl_species1b->setText(" 0 %");
		sas_l->lbl_species2b->setText(" 0 %");
		sas_l->lbl_species3b->setText(" 0 %");
		sas_l->lbl_species4b->setText(" 0 M");
	}

}

bool US_Sassoc::createHtmlDir()
{
	htmlDir = USglobal->config_list.html_dir + "/" + project;
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

US_SassocLegend::US_SassocLegend(QWidget *p, const char *name) : QFrame(p, name)
{
	spacing = 2;
	buttonh = 26;
	column1 = 106;
	column2 = 90;
	border = 4;
	xpos = 4;
	ypos = 4;

	USglobal = new US_Config();
	QColorGroup colgrp1( Qt::green, Qt::black, Qt::white, Qt::black, Qt::black, Qt::white, Qt::black );
	QColorGroup colgrp2( Qt::yellow, Qt::black, Qt::white, Qt::black, Qt::black, Qt::white, Qt::black );
	QColorGroup colgrp3( Qt::cyan, Qt::black, Qt::white, Qt::black, Qt::black, Qt::white, Qt::black );

	lbl_species1a = new QLabel(tr("Species 1:"),this);
	lbl_species1a->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_species1a->setPalette(QPalette(colgrp1, colgrp1, colgrp1));
	lbl_species1a->setGeometry(xpos, ypos, column1, buttonh);
	lbl_species1a->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	xpos += column1 + spacing;

	lbl_species1b = new QLabel(" 0 %",this);
	lbl_species1b->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_species1b->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_species1b->setGeometry(xpos, ypos, column2, buttonh);
	lbl_species1b->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	xpos += column2 + spacing;

	lbl_species2a = new QLabel(tr("Species 2:"),this);
	lbl_species2a->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_species2a->setPalette(QPalette(colgrp2, colgrp2, colgrp2));
	lbl_species2a->setGeometry(xpos, ypos, column1, buttonh);
	lbl_species2a->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	xpos += column1 + spacing;

	lbl_species2b = new QLabel(" 0 %",this);
	lbl_species2b->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_species2b->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_species2b->setGeometry(xpos, ypos, column2, buttonh);
	lbl_species2b->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	xpos += column2 + spacing;
	lbl_species3a = new QLabel(tr("Species 3:"),this);
	lbl_species3a->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_species3a->setPalette(QPalette(colgrp3, colgrp3, colgrp3));
	lbl_species3a->setGeometry(xpos, ypos, column1, buttonh);
	lbl_species3a->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	xpos += column1 + spacing;

	lbl_species3b = new QLabel(" 0 %",this);
	lbl_species3b->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_species3b->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_species3b->setGeometry(xpos, ypos, column2, buttonh);
	lbl_species3b->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_species4a = new QLabel(tr("Total Conc.:"),this);
	lbl_species4a->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_species4a->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_species4a->setGeometry(xpos, ypos, column1, buttonh);
	lbl_species4a->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	xpos += column1 + spacing;

	lbl_species4b = new QLabel(" 0 M",this);
	lbl_species4b->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_species4b->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_species4b->setGeometry(xpos, ypos, column2, buttonh);
	lbl_species4b->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	xpos += column2 + spacing;

	lbl_instructions = new QLabel(tr("Drag mouse through plot area to see relative concentrations"),this);
	lbl_instructions->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_instructions->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_instructions->setGeometry(xpos, ypos, 2 * column2 + 2* column1 + 3 * spacing, buttonh);
	lbl_instructions->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	setup_GUI();
};

US_SassocLegend::~US_SassocLegend()
{
}
void US_SassocLegend::setup_GUI()
{
	int j=0;
	int rows = 2, columns = 6, spacing = 2;
	
	QGridLayout * background = new QGridLayout(this,rows, columns,spacing);	
	for (int i=0; i<rows; i++)
	{
		background->setRowSpacing(i, 26);
	}
	background->addWidget(lbl_species1a,j,0);
	background->addWidget(lbl_species1b,j,1);
	background->addWidget(lbl_species2a,j,2);
	background->addWidget(lbl_species2b,j,3);
	background->addWidget(lbl_species3a,j,4);
	background->addWidget(lbl_species3b,j,5);
	j++;
	background->addWidget(lbl_species4a,j,0);
	background->addWidget(lbl_species4b,j,1);
	background->addMultiCellWidget(lbl_instructions,j,j,2,5);

}
