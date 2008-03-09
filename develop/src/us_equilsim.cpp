#include "../include/us_equilsim.h"

US_EquilSim::US_EquilSim(QWidget *p, const char *name) : QFrame( p, name)
{
	int xpos, ypos, column1 = 170, column2 = 150, column3 = 170, column4 = 150, buttonh = 26, span;
	span = column1 + column2 + column3 + column4 + 3 * spacing;
	int buttonw = (int) (span - 4 * spacing)/5;
	QString str;
	xpos = border;
	ypos = border;
	USglobal = new US_Config();
	USglobal->setModelString();
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	model = 0;
	species = 1;
	current_species = 0;
	assoc1 = true;
	assoc2 = true;
	assoc3 = true;
	export_format = true; 	// export ultrascan format
	export_flag = false;		// don't export
	plot_widget = false; 	// plot isn't open yet
	histogram_widget = false; 	// histogram isn't open yet
	equil1 = 1.0;
	equil2 = 1.0;
	equil3 = 1.0;
	lambda = 280.0;

	mw.clear();
	concentration.clear();
	vbar20.clear();
	extinction.clear();
	od.clear();
	pathlength = (float) 1.2;
	bin_number = 50;

	mw.push_back(50000);
	concentration.push_back( (float) 0.05 );
	vbar20.push_back( (float) 0.72 );
	extinction.push_back( (float) 4000 );
	od.push_back((concentration[0]/1000) * pathlength * extinction[0]);

	baseline       = 0;
	conc_increment = (float) 0.1;
	conc_steps     = 3;
	speed_start    = 10000;
	speed_stop     = 30000;
	speed_steps    = 4;
	temperature    = 20;
	density        = (float) DENS_20W;
	max_od         = 1.0;
	column         = 3;
	bottom         = (float) 7.2;
	meniscus       = bottom - column/10;
	datapoints     = 180;
	delta_r        = column/(10 * (datapoints-1));
	noise          = (float) 2.0e-3;
	nonlin_noise   = (float) 1.5e-2;

	pb_model = new QPushButton(tr("Select Model"), this);
	pb_model->setAutoDefault(false);
	pb_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_model->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_model->setGeometry(xpos, ypos, column1, buttonh);
	connect(pb_model, SIGNAL(clicked()), SLOT(select_model()));

	xpos += column1 + spacing;

	lbl_model = new QLabel(modelString[model],this);
	lbl_model->setAlignment(AlignLeft|AlignVCenter);
	lbl_model->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_model->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_model->setGeometry(xpos, ypos, column2 + column3 + column4 + 2 * spacing, buttonh);
	lbl_model->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

	ypos += buttonh + 5 * spacing;
	xpos = border;

	lbl_blank1 = new QLabel(tr(" Component Information:"), this);
	lbl_blank1->setAlignment(AlignCenter|AlignVCenter);
	lbl_blank1->setFrameStyle(QFrame::WinPanel|Raised);
	lbl_blank1->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_blank1->setGeometry(xpos, ypos, column1 + column2 + spacing, buttonh);
	lbl_blank1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	xpos += column1 + column2 + spacing;

	lbl_blank2 = new QLabel(tr(" Model Parameters"), this);
	lbl_blank2->setAlignment(AlignCenter|AlignVCenter);
	lbl_blank2->setFrameStyle(QFrame::WinPanel|Raised);
	lbl_blank2->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_blank2->setGeometry(xpos, ypos, column3 + column4 + spacing, buttonh);
	lbl_blank2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	ypos += buttonh + 2 * spacing;
	xpos = border;

	lbl_species = new QLabel(tr(" Species:"),this);
	lbl_species->setAlignment(AlignLeft|AlignVCenter);
	lbl_species->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_species->setGeometry(xpos, ypos, column1, buttonh);
	lbl_species->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	cnt_species= new QwtCounter(this);
	cnt_species->setRange(1, 1, 1);
	cnt_species->setNumButtons(1);
	cnt_species->setValue(1);
	cnt_species->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_species->setGeometry(xpos, ypos, column2, buttonh);
//	connect(cnt_species, SIGNAL(buttonReleased(double)), SLOT(select_species(double)));
	connect(cnt_species, SIGNAL(valueChanged(double)), SLOT(select_species(double)));

	xpos += column2 + spacing;

	pb_equil1 = new QPushButton(tr("Ln(Assoc. Const. 1):"), this);
	pb_equil1->setAutoDefault(false);
	pb_equil1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_equil1->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_equil1->setGeometry(xpos, ypos, column3, buttonh);
	pb_equil1->setEnabled(false);
	connect(pb_equil1, SIGNAL(clicked()), SLOT(change_equil1()));

	xpos += column3 + spacing;

	le_equil1 = new QLineEdit(this, "equil1");
	le_equil1->setGeometry(xpos, ypos, column4, buttonh);
	le_equil1->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_equil1->setText(str.sprintf(" %5.3e", equil1));
	le_equil1->setEnabled(false);
	connect(le_equil1, SIGNAL(textChanged(const QString &)),
				SLOT(update_equil1(const QString &)));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_mw = new QLabel(tr(" Mol. Weight (dalton):"),this);
	lbl_mw->setAlignment(AlignLeft|AlignVCenter);
	lbl_mw->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_mw->setGeometry(xpos, ypos, column1, buttonh);
	lbl_mw->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	le_mw = new QLineEdit(this, "mw");
	le_mw->setGeometry(xpos, ypos, column2, buttonh);
	le_mw->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_mw->setText(str.sprintf(" %5.3e", mw[0]));
	connect(le_mw, SIGNAL(textChanged(const QString &)),
				SLOT(update_mw(const QString &)));

	xpos += column2 + spacing;

	pb_equil2 = new QPushButton(tr("Ln(Assoc. Const. 2):"), this);
	pb_equil2->setAutoDefault(false);
	pb_equil2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_equil2->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_equil2->setGeometry(xpos, ypos, column3, buttonh);
	pb_equil2->setEnabled(false);
	connect(pb_equil2, SIGNAL(clicked()), SLOT(change_equil2()));

	xpos += column3 + spacing;

	le_equil2 = new QLineEdit(this, "equil2");
	le_equil2->setGeometry(xpos, ypos, column4, buttonh);
	le_equil2->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_equil2->setText(str.sprintf(" %5.3e", equil2));
	le_equil2->setEnabled(false);
	connect(le_equil2, SIGNAL(textChanged(const QString &)),
				SLOT(update_equil2(const QString &)));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_concentration = new QLabel(tr(" Concentration (mM):"),this);
	lbl_concentration->setAlignment(AlignLeft|AlignVCenter);
	lbl_concentration->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_concentration->setGeometry(xpos, ypos, column1, buttonh);
	lbl_concentration->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	le_concentration = new QLineEdit(this, "concentration");
	le_concentration->setGeometry(xpos, ypos, column2, buttonh);
	le_concentration->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_concentration->setText(str.sprintf(" %5.3e", concentration[0]));
	connect(le_concentration, SIGNAL(textChanged(const QString &)),
				SLOT(update_concentration(const QString &)));

	xpos += column2 + spacing;

	pb_equil3 = new QPushButton(tr("Ln(Assoc. Const. 3):"), this);
	pb_equil3->setAutoDefault(false);
	pb_equil3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_equil3->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_equil3->setGeometry(xpos, ypos, column3, buttonh);
	pb_equil3->setEnabled(false);
	connect(pb_equil3, SIGNAL(clicked()), SLOT(change_equil3()));

	xpos += column3 + spacing;

	le_equil3 = new QLineEdit(this, "equil3");
	le_equil3->setGeometry(xpos, ypos, column4, buttonh);
	le_equil3->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_equil3->setText(str.sprintf(" %5.3e", equil3));
	le_equil3->setEnabled(false);
	connect(le_equil3, SIGNAL(textChanged(const QString &)),
				SLOT(update_equil3(const QString &)));

	xpos = border;
	ypos += buttonh + spacing;

	pb_vbar = new QPushButton(tr("V-bar (ccm/g, 20ºC):"), this);
	pb_vbar->setAutoDefault(false);
	pb_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_vbar->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_vbar->setGeometry(xpos, ypos, column3, buttonh);
	pb_vbar->setEnabled(true);
	connect(pb_vbar, SIGNAL(clicked()), SLOT(change_vbar()));

	xpos += column1 + spacing;

	le_vbar = new QLineEdit(this, "vbar");
	le_vbar->setGeometry(xpos, ypos, column2, buttonh);
	le_vbar->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_vbar->setText(str.sprintf(" %5.3e", vbar20[0]));
	connect(le_vbar, SIGNAL(textChanged(const QString &)),
				SLOT(update_vbar(const QString &)));

	xpos += column2 + spacing;

	lbl_baseline = new QLabel(tr(" Baseline OD:"),this);
	lbl_baseline->setAlignment(AlignLeft|AlignVCenter);
	lbl_baseline->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_baseline->setGeometry(xpos, ypos, column3, buttonh);
	lbl_baseline->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column3 + spacing;

	le_baseline = new QLineEdit(this, "baseline");
	le_baseline->setGeometry(xpos, ypos, column4, buttonh);
	le_baseline->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_baseline->setText(str.sprintf(" %5.3e", baseline));
	connect(le_baseline, SIGNAL(textChanged(const QString &)),
				SLOT(update_baseline(const QString &)));

	xpos = border;
	ypos += buttonh + spacing;

	pb_extinction = new QPushButton(tr("Ext. Coeff. (OD/cm*M):"), this);
	pb_extinction->setAutoDefault(false);
	pb_extinction->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_extinction->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_extinction->setGeometry(xpos, ypos, column3, buttonh);
	pb_extinction->setEnabled(true);
	connect(pb_extinction, SIGNAL(clicked()), SLOT(change_extinction()));

	xpos += column1 + spacing;

	le_extinction = new QLineEdit(this, "extinction");
	le_extinction->setGeometry(xpos, ypos, column2, buttonh);
	le_extinction->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_extinction->setText(str.sprintf(" %5.3e", extinction[0]));
	connect(le_extinction, SIGNAL(textChanged(const QString &)),
				SLOT(update_extinction(const QString &)));

	xpos += column2 + spacing;

	lbl_od = new QLabel(tr(" Loading OD Contrib.:"),this);
	lbl_od->setAlignment(AlignLeft|AlignVCenter);
	lbl_od->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_od->setGeometry(xpos, ypos, column3, buttonh);
	lbl_od->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column3 + spacing;

	lbl_od_value = new QLabel(this, "od");
	lbl_od_value->setGeometry(xpos, ypos, column4, buttonh);
	lbl_od_value->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_od_value->setText(str.sprintf(" %5.2f", od[0]));

	ypos += buttonh + 5 * spacing;
	xpos = border;

	lbl_blank3 = new QLabel(tr(" Run Information:"), this);
	lbl_blank3->setFrameStyle(QFrame::WinPanel|Raised);
	lbl_blank3->setAlignment(AlignCenter|AlignVCenter);
	lbl_blank3->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_blank3->setGeometry(xpos, ypos, span, buttonh);
	lbl_blank3->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	ypos += buttonh + 2 * spacing;
	xpos = border;

	lbl_conc_increment = new QLabel(tr(" Conc. Increment (OD):"),this);
	lbl_conc_increment->setAlignment(AlignLeft|AlignVCenter);
	lbl_conc_increment->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_conc_increment->setGeometry(xpos, ypos, column1, buttonh);
	lbl_conc_increment->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	le_conc_increment = new QLineEdit(this, "conc_increment");
	le_conc_increment->setGeometry(xpos, ypos, column2, buttonh);
	le_conc_increment->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_conc_increment->setText(str.sprintf(" %5.3f", conc_increment));
	connect(le_conc_increment, SIGNAL(textChanged(const QString &)),
				SLOT(update_conc_increment(const QString &)));

	xpos += column2 + spacing;

	lbl_pathlength = new QLabel(tr(" Pathlength (cm):"),this);
	lbl_pathlength->setAlignment(AlignLeft|AlignVCenter);
	lbl_pathlength->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_pathlength->setGeometry(xpos, ypos, column3, buttonh);
	lbl_pathlength->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column3 + spacing;

	le_pathlength = new QLineEdit(this, "pathlength");
	le_pathlength->setGeometry(xpos, ypos, column4, buttonh);
	le_pathlength->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_pathlength->setText(str.sprintf(" %5.3f", pathlength));
	connect(le_pathlength, SIGNAL(textChanged(const QString &)),
				SLOT(update_pathlength(const QString &)));

	ypos += buttonh + spacing;
	xpos = border;

	lbl_conc_steps = new QLabel(tr(" # of Conc. Steps:"),this);
	lbl_conc_steps->setAlignment(AlignLeft|AlignVCenter);
	lbl_conc_steps->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_conc_steps->setGeometry(xpos, ypos, column1, buttonh);
	lbl_conc_steps->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	cnt_conc_steps= new QwtCounter(this);
	cnt_conc_steps->setRange(1, 100, 1);
	cnt_conc_steps->setNumButtons(2);
	cnt_conc_steps->setValue(conc_steps);
	cnt_conc_steps->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_conc_steps->setGeometry(xpos, ypos, column2, buttonh);
//	connect(cnt_conc_steps, SIGNAL(buttonReleased(double)), SLOT(select_conc_steps(double)));
	connect(cnt_conc_steps, SIGNAL(valueChanged(double)), SLOT(select_conc_steps(double)));

	xpos += column2 + spacing;

	lbl_speed_start = new QLabel(tr(" Speed Start:"),this);
	lbl_speed_start->setAlignment(AlignLeft|AlignVCenter);
	lbl_speed_start->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_speed_start->setGeometry(xpos, ypos, column3, buttonh);
	lbl_speed_start->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column3 + spacing;

	cnt_speed_start= new QwtCounter(this);
	cnt_speed_start->setRange(100, speed_stop, 100);
	cnt_speed_start->setNumButtons(3);
	cnt_speed_start->setValue(speed_start);
	cnt_speed_start->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_speed_start->setGeometry(xpos, ypos, column4, buttonh);
//	connect(cnt_speed_start, SIGNAL(buttonReleased(double)), SLOT(select_speed_start(double)));
	connect(cnt_speed_start, SIGNAL(valueChanged(double)), SLOT(select_speed_start(double)));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_temperature = new QLabel(tr(" Temperature (ºC):"),this);
	lbl_temperature->setAlignment(AlignLeft|AlignVCenter);
	lbl_temperature->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_temperature->setGeometry(xpos, ypos, column1, buttonh);
	lbl_temperature->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	le_temperature = new QLineEdit(this, "temperature");
	le_temperature->setGeometry(xpos, ypos, column2, buttonh);
	le_temperature->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_temperature->setText(str.sprintf(" %5.2f", temperature));
	connect(le_temperature, SIGNAL(textChanged(const QString &)),
				SLOT(update_temperature(const QString &)));

	xpos += column2 + spacing;

	lbl_speed_stop = new QLabel(tr(" Speed Stop:"),this);
	lbl_speed_stop->setAlignment(AlignLeft|AlignVCenter);
	lbl_speed_stop->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_speed_stop->setGeometry(xpos, ypos, column3, buttonh);
	lbl_speed_stop->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column3 + spacing;

	cnt_speed_stop= new QwtCounter(this);
	cnt_speed_stop->setRange(speed_start, 65000, 100);
	cnt_speed_stop->setNumButtons(3);
	cnt_speed_stop->setValue(speed_stop);
	cnt_speed_stop->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_speed_stop->setGeometry(xpos, ypos, column4, buttonh);
//	connect(cnt_speed_stop, SIGNAL(buttonReleased(double)), SLOT(select_speed_stop(double)));
	connect(cnt_speed_stop, SIGNAL(valueChanged(double)), SLOT(select_speed_stop(double)));

	xpos = border;
	ypos += buttonh + spacing;

	pb_density = new QPushButton(tr("Density (g/ccm, 20ºC):"), this);
	pb_density->setAutoDefault(false);
	pb_density->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_density->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_density->setGeometry(xpos, ypos, column3, buttonh);
	pb_density->setEnabled(true);
	connect(pb_density, SIGNAL(clicked()), SLOT(change_density()));

	xpos += column1 + spacing;

	le_density = new QLineEdit(this, "density");
	le_density->setGeometry(xpos, ypos, column2, buttonh);
	le_density->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_density->setText(str.sprintf(" %7.5e", density));
	connect(le_density, SIGNAL(textChanged(const QString &)),
				SLOT(update_density(const QString &)));

	xpos += column2 + spacing;

	lbl_speed_steps = new QLabel(tr(" Speed Steps:"),this);
	lbl_speed_steps->setAlignment(AlignLeft|AlignVCenter);
	lbl_speed_steps->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_speed_steps->setGeometry(xpos, ypos, column3, buttonh);
	lbl_speed_steps->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column3 + spacing;

	cnt_speed_steps= new QwtCounter(this);
	cnt_speed_steps->setRange(1, 100, 1);
	cnt_speed_steps->setNumButtons(2);
	cnt_speed_steps->setValue(speed_steps);
	cnt_speed_steps->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_speed_steps->setGeometry(xpos, ypos, column4, buttonh);
//	connect(cnt_speed_steps, SIGNAL(buttonReleased(double)), SLOT(select_speed_steps(double)));
	connect(cnt_speed_steps, SIGNAL(valueChanged(double)), SLOT(select_speed_steps(double)));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_column = new QLabel(tr(" Column Height (mm):"),this);
	lbl_column->setAlignment(AlignLeft|AlignVCenter);
	lbl_column->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_column->setGeometry(xpos, ypos, column1, buttonh);
	lbl_column->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	cnt_column= new QwtCounter(this);
	cnt_column->setRange(1, 14, 0.01);
	cnt_column->setNumButtons(3);
	cnt_column->setValue(column);
	cnt_column->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_column->setGeometry(xpos, ypos, column2, buttonh);
//	connect(cnt_column, SIGNAL(buttonReleased(double)), SLOT(select_column(double)));
	connect(cnt_column, SIGNAL(valueChanged(double)), SLOT(select_column(double)));

	xpos += column2 + spacing;

	lbl_bottom = new QLabel(tr(" Cell Bottom (cm):"), this);
	lbl_bottom->setAlignment(AlignLeft|AlignVCenter);
	lbl_bottom->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_bottom->setGeometry(xpos, ypos, column3, buttonh);
	lbl_bottom->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column3 + spacing;

	cnt_bottom= new QwtCounter(this);
	cnt_bottom->setRange(5.8, 7.5, 0.001);
	cnt_bottom->setNumButtons(3);
	cnt_bottom->setValue(bottom);
	cnt_bottom->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_bottom->setGeometry(xpos, ypos, column4, buttonh);
//	connect(cnt_bottom, SIGNAL(buttonReleased(double)), SLOT(select_bottom(double)));
	connect(cnt_bottom, SIGNAL(valueChanged(double)), SLOT(select_bottom(double)));

	xpos = border;
	ypos += buttonh + spacing;

	lbl_meniscus = new QLabel(tr(" Meniscus:"),this);
	lbl_meniscus->setAlignment(AlignLeft|AlignVCenter);
	lbl_meniscus->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_meniscus->setGeometry(xpos, ypos, column1, buttonh);
	lbl_meniscus->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	lbl_meniscus_value = new QLabel(this, "meniscus");
	lbl_meniscus_value->setGeometry(xpos, ypos, column4, buttonh);
	lbl_meniscus_value->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_meniscus_value->setText(str.sprintf(" %6.3f cm", meniscus));

	xpos += column2 + spacing;

	lbl_max_od = new QLabel(tr(" Maximum OD:"),this);
	lbl_max_od->setAlignment(AlignLeft|AlignVCenter);
	lbl_max_od->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_max_od->setGeometry(xpos, ypos, column3, buttonh);
	lbl_max_od->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column3 + spacing;

	le_max_od = new QLineEdit(this, "max_od");
	le_max_od->setGeometry(xpos, ypos, column4, buttonh);
	le_max_od->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_max_od->setText(str.sprintf(" %5.3e", max_od));
	connect(le_max_od, SIGNAL(textChanged(const QString &)),
				SLOT(update_max_od(const QString &)));

	ypos += buttonh + 5 * spacing;
	xpos = border;

	lbl_blank4 = new QLabel(tr(" Simulation Settings:"), this);
	lbl_blank4->setFrameStyle(QFrame::WinPanel|Raised);
	lbl_blank4->setAlignment(AlignCenter|AlignVCenter);
	lbl_blank4->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_blank4->setGeometry(xpos, ypos, span, buttonh);
	lbl_blank4->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	xpos = border;
	ypos += buttonh + 2 * spacing;

	lbl_datapoints = new QLabel(tr(" # of Datapoints:"),this);
	lbl_datapoints->setAlignment(AlignLeft|AlignVCenter);
	lbl_datapoints->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_datapoints->setGeometry(xpos, ypos, column1, buttonh);
	lbl_datapoints->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	cnt_datapoints= new QwtCounter(this);
	cnt_datapoints->setRange(30, 2000, 1);
	cnt_datapoints->setNumButtons(3);
	cnt_datapoints->setValue(datapoints);
	cnt_datapoints->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_datapoints->setGeometry(xpos, ypos, column2, buttonh);
//	connect(cnt_datapoints, SIGNAL(buttonReleased(double)), SLOT(select_datapoints(double)));
	connect(cnt_datapoints, SIGNAL(valueChanged(double)), SLOT(select_datapoints(double)));

	xpos += column2 + spacing;

	lbl_delta_r = new QLabel(tr(" Radial Increment:"),this);
	lbl_delta_r->setAlignment(AlignLeft|AlignVCenter);
	lbl_delta_r->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_delta_r->setGeometry(xpos, ypos, column3, buttonh);
	lbl_delta_r->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column3 + spacing;

	lbl_delta_r_value = new QLabel(this, "delta_r");
	lbl_delta_r_value->setGeometry(xpos, ypos, column4, buttonh);
	lbl_delta_r_value->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_delta_r_value->setText(str.sprintf(" %6.4e cm", delta_r));

	ypos += buttonh + spacing;
	xpos = border;

	lbl_noise = new QLabel(tr(" Gaussian Noise (% OD):"),this);
	lbl_noise->setAlignment(AlignLeft|AlignVCenter);
	lbl_noise->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_noise->setGeometry(xpos, ypos, column1, buttonh);
	lbl_noise->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	cnt_noise= new QwtCounter(this);
	cnt_noise->setRange(0, 10, 0.01);
	cnt_noise->setNumButtons(3);
	cnt_noise->setValue(noise * 100);
	cnt_noise->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_noise->setGeometry(xpos, ypos, column2, buttonh);
//	connect(cnt_noise, SIGNAL(buttonReleased(double)), SLOT(select_noise(double)));
	connect(cnt_noise, SIGNAL(valueChanged(double)), SLOT(select_noise(double)));

	xpos += column2 + spacing;

	lbl_lambda = new QLabel(tr(" Simulated Wavelength:"),this);
	lbl_lambda->setAlignment(AlignLeft|AlignVCenter);
	lbl_lambda->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_lambda->setGeometry(xpos, ypos, column3, buttonh);
	lbl_lambda->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column3 + spacing;

	le_lambda = new QLineEdit(this, "lambda");
	le_lambda->setGeometry(xpos, ypos, column4, buttonh);
	le_lambda->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_lambda->setText(str.sprintf(" %3.1f", lambda));
	connect(le_lambda, SIGNAL(textChanged(const QString &)),
				SLOT(update_lambda(const QString &)));

	ypos += buttonh + spacing;
	xpos = border;

	lbl_nonlin_noise = new QLabel(tr(" Nonlinear Noise (% OD):"),this);
	lbl_nonlin_noise->setAlignment(AlignLeft|AlignVCenter);
	lbl_nonlin_noise->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_nonlin_noise->setGeometry(xpos, ypos, column1, buttonh);
	lbl_nonlin_noise->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos += column1 + spacing;

	cnt_nonlin_noise= new QwtCounter(this);
	cnt_nonlin_noise->setRange(0, 10, 0.01);
	cnt_nonlin_noise->setNumButtons(3);
	cnt_nonlin_noise->setValue(nonlin_noise * 100);
	cnt_nonlin_noise->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_nonlin_noise->setGeometry(xpos, ypos, column2, buttonh);
//	connect(cnt_nonlin_noise, SIGNAL(buttonReleased(double)), SLOT(select_nonlin_noise(double)));
	connect(cnt_nonlin_noise, SIGNAL(valueChanged(double)), SLOT(select_nonlin_noise(double)));

	xpos += column2 + spacing;
/*
	lbl_xla = new QLabel(tr(" Export as:   XL-A:"),this);
	lbl_xla->setAlignment(AlignLeft|AlignVCenter);
	lbl_xla->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_xla->setGeometry(xpos, ypos, column3, buttonh);
	lbl_xla->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
*/
	cb_xla = new QCheckBox(tr(" Export as:   XL-A"), this);
	cb_xla->setGeometry(xpos+column3-20, ypos+5, 14, 14);
	cb_xla->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_xla->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	cb_xla->setChecked(!export_format);
	connect(cb_xla, SIGNAL(clicked()), SLOT(select_xla()));

	xpos += column3 + spacing;
/*
	lbl_ultrascan = new QLabel(tr(" UltraScan:"),this);
	lbl_ultrascan->setAlignment(AlignLeft|AlignVCenter);
	lbl_ultrascan->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_ultrascan->setGeometry(xpos, ypos, column4, buttonh);
	lbl_ultrascan->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
*/
	cb_ultrascan = new QCheckBox(tr(" UltraScan"),this);
	cb_ultrascan->setGeometry(xpos+column4-20, ypos+5, 14, 14);
	cb_ultrascan->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_ultrascan->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	cb_ultrascan->setChecked(export_format);
	connect(cb_ultrascan, SIGNAL(clicked()), SLOT(select_ultrascan()));

	ypos += buttonh + 5 * spacing;
	xpos = border;

	pb_help = new QPushButton(tr("Help"), this);
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_help->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_help->setEnabled(true);
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	xpos += buttonw + spacing;

	pb_plot = new QPushButton(tr("Plot"), this);
	pb_plot->setAutoDefault(false);
	pb_plot->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_plot->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_plot->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_plot->setEnabled(true);
	connect(pb_plot, SIGNAL(clicked()), SLOT(plot()));

	xpos += buttonw + spacing;

	pb_histogram = new QPushButton(tr("Histogram"), this);
	pb_histogram->setAutoDefault(false);
	pb_histogram->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_histogram->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_histogram->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_histogram->setEnabled(true);
	connect(pb_histogram, SIGNAL(clicked()), SLOT(histogram()));

	xpos += buttonw + spacing;

	pb_export = new QPushButton(tr("Export"), this);
	pb_export->setAutoDefault(false);
	pb_export->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_export->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_export->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_export->setEnabled(true);
	connect(pb_export, SIGNAL(clicked()), SLOT(export_data()));

	xpos += buttonw + spacing;

	pb_close = new QPushButton(tr("Close"), this);
	pb_close->setAutoDefault(false);
	pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_close->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_close->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_close->setEnabled(true);
	connect(pb_close, SIGNAL(clicked()), SLOT(quit()));

	ypos += buttonh + border;

	global_Xpos += 30;
	global_Ypos += 30;
	setGeometry(global_Xpos, global_Ypos, 2 * border + 3 * spacing + column1 + column2 + column3 + column4, ypos);

	setup_GUI();
}

US_EquilSim::~US_EquilSim()
{
}

void US_EquilSim::setup_GUI()
{
	int j=0;
	int rows = 13, columns = 4, spacing = 2;

	QGridLayout * background = new QGridLayout(this, rows,columns,spacing);
	background->setMargin(spacing);
	for (int i=0; i<rows; i++)
	{
		background->setRowSpacing(i, 26);
	}
	background->addWidget(pb_model,j,0);
	background->addMultiCellWidget(lbl_model,j,j,1,3);
	j++;
	background->addMultiCellWidget(lbl_blank1,j,j,0,1);
	background->addMultiCellWidget(lbl_blank2,j,j,2,3);
	j++;
	background->addWidget(lbl_species,j,0);
	background->addWidget(cnt_species,j,1);
	background->addWidget(pb_equil1,j,2);
	background->addWidget(le_equil1,j,3);
	j++;
	background->addWidget(lbl_mw,j,0);
	background->addWidget(le_mw,j,1);
	background->addWidget(pb_equil2,j,2);
	background->addWidget(le_equil2,j,3);
	j++;
	background->addWidget(lbl_concentration,j,0);
	background->addWidget(le_concentration,j,1);
	background->addWidget(pb_equil3,j,2);
	background->addWidget(le_equil3,j,3);
	j++;
	background->addWidget(pb_vbar,j,0);
	background->addWidget(le_vbar,j,1);
	background->addWidget(lbl_baseline,j,2);
	background->addWidget(le_baseline,j,3);
	j++;
	background->addWidget(pb_extinction,j,0);
	background->addWidget(le_extinction,j,1);
	background->addWidget(lbl_od,j,2);
	background->addWidget(lbl_od_value,j,3);
	j++;
	background->addMultiCellWidget(lbl_blank3,j,j,0,3);
	j++;
	background->addWidget(lbl_conc_increment,j,0);
	background->addWidget(le_conc_increment,j,1);
	background->addWidget(lbl_pathlength,j,2);
	background->addWidget(le_pathlength,j,3);
	j++;
	background->addWidget(lbl_conc_steps,j,0);
	background->addWidget(cnt_conc_steps,j,1);
	background->addWidget(lbl_speed_start,j,2);
	background->addWidget(cnt_speed_start,j,3);
	j++;
	background->addWidget(lbl_temperature,j,0);
	background->addWidget(le_temperature,j,1);
	background->addWidget(lbl_speed_stop,j,2);
	background->addWidget(cnt_speed_stop,j,3);
	j++;
	background->addWidget(pb_density,j,0);
	background->addWidget(le_density,j,1);
	background->addWidget(lbl_speed_steps,j,2);
	background->addWidget(cnt_speed_steps,j,3);
	j++;
	background->addWidget(lbl_column,j,0);
	background->addWidget(cnt_column,j,1);
	background->addWidget(lbl_bottom,j,2);
	background->addWidget(cnt_bottom,j,3);
	j++;
	background->addWidget(lbl_meniscus,j,0);
	background->addWidget(lbl_meniscus_value,j,1);
	background->addWidget(lbl_max_od,j,2);
	background->addWidget(le_max_od,j,3);
	j++;
	background->addMultiCellWidget(lbl_blank4,j,j,0,3);
	j++;
	background->addWidget(lbl_datapoints,j,0);
	background->addWidget(cnt_datapoints,j,1);
	background->addWidget(lbl_delta_r,j,2);
	background->addWidget(lbl_delta_r_value,j,3);
	j++;
	background->addWidget(lbl_noise,j,0);
	background->addWidget(cnt_noise,j,1);
	background->addWidget(lbl_lambda,j,2);
	background->addWidget(le_lambda,j,3);
	j++;
	background->addWidget(lbl_nonlin_noise,j,0);
	background->addWidget(cnt_nonlin_noise,j,1);
	background->addWidget(cb_xla,j,2);
	background->addWidget(cb_ultrascan,j,3);
	j++;
	QGridLayout *buttons = new QGridLayout(1,5,2);
	buttons->addWidget(pb_help,0,0);
	buttons->addWidget(pb_plot,0,1);
	buttons->addWidget(pb_histogram,0,2);
	buttons->addWidget(pb_export,0,3);
	buttons->addWidget(pb_close,0,4);
	background->addMultiCell(buttons,j,j,0,3);

	//background->activate();

}
void US_EquilSim::select_model()
{
	QString str;
	pb_equil1->setEnabled(false);
	pb_equil2->setEnabled(false);
	pb_equil3->setEnabled(false);
	le_equil1->setEnabled(false);
	le_equil2->setEnabled(false);
	le_equil3->setEnabled(false);
	equil1 = 1;
	equil2 = 1;
	equil3 = 1;
	assoc1 = false;
	assoc2 = false;
	assoc3 = false;
	change_equil1();
	change_equil2();
	change_equil3();
	SelectModel = new US_SelectModel(&model, true);
	if (SelectModel->exec())
	{
		mw.clear();
		concentration.clear();
		vbar20.clear();
		extinction.clear();
		od.clear();
		stoich1 = 0;
		stoich2 = 0;
		switch (model)
		{
			case 0:
			{
				vbar20.push_back( (float) 0.72 );
				mw.push_back(50000);
				species = 1;
				cnt_species->setRange(1, species, 1);
				concentration.push_back( (float) 0.05 );
				extinction.push_back(4000);
				od.push_back((concentration[0]/1000) * pathlength * extinction[0]);
				break;
			}
			case 1:
			{
				vbar20.push_back( (float) 0.72);
				vbar20.push_back( (float) 0.72);
				mw.push_back(50000);
				mw.push_back(75000);
				species = 2;
				cnt_species->setRange(1, species, 1);
				concentration.push_back( (float) 0.05 );
				concentration.push_back( (float) 0.05 );
				extinction.push_back(4000);
				extinction.push_back(6000);
				od.push_back((concentration[0]/1000) * pathlength * extinction[0]);
				od.push_back((concentration[1]/1000) * pathlength * extinction[1]);
				break;
			}
			case 2:
			{
				vbar20.push_back( (float) 0.72 );
				vbar20.push_back( (float) 0.72 );
				vbar20.push_back( (float) 0.72 );
				mw.push_back(50000);
				mw.push_back(80000);
				mw.push_back(120000);
				species = 3;
				cnt_species->setRange(1, species, 1);
				concentration.push_back( (float) 0.03 );
				concentration.push_back( (float) 0.05 );
				concentration.push_back( (float) 0.07 );
				extinction.push_back(3000);
				extinction.push_back(6000);
				extinction.push_back(9000);
				od.push_back((concentration[0]/1000) * pathlength * extinction[0]);
				od.push_back((concentration[1]/1000) * pathlength * extinction[1]);
				od.push_back((concentration[2]/1000) * pathlength * extinction[2]);
				break;
			}
			case 3:	// fixed molecular weight distribution, we need some addtl. info from the user
			{
				US_SelectModel3 *SelectModel3;
				model3_vbar =  (float) 0.72;
				model3_vbar_flag = true;
				SelectModel3 = new US_SelectModel3(&mw_upperLimit, &mw_lowerLimit, &species,
				&model3_vbar, &model3_vbar_flag, -1);
				if (!SelectModel3->exec())
				{
					model = 0;
					break;
				}
				cnt_species->setRange(1, species, 1);
				mw.push_back(mw_lowerLimit);
				vbar20.push_back(model3_vbar);
				concentration.push_back( (float) 0.03 );
				extinction.push_back(3000);
				od.push_back((concentration[0]/1000) * pathlength * extinction[0]);
				float step = (mw_upperLimit - mw_lowerLimit)/(species - 1);
				for (unsigned int i=1; i<species; i++)
				{
					mw.push_back(mw[i-1] + step);
					vbar20.push_back( (float) 0.72 );
					concentration.push_back( (float) 0.03 );
					extinction.push_back(3000);
					od.push_back((concentration[i]/1000) * pathlength * extinction[i]);
				}
				break;
			}
			case 4: // Monomer-Dimer Equilibrium
			{
				stoich1 = 2;
				vbar20.push_back( (float) 0.72 );
				mw.push_back(50000);
				species = 1;
				concentration.push_back( (float) 0.05 );
				extinction.push_back(4000);
				od.push_back((concentration[0]/1000) * pathlength * extinction[0]);
				equil1 = 13;
				pb_equil1->setEnabled(true);
				le_equil1->setEnabled(true);
				le_equil1->setText(str.sprintf(" %5.3e", equil1));
				break;
			}
			case 5: // Monomer-Trimer Equilibrium
			{
				stoich1 = 3;
				vbar20.push_back( (float) 0.72 );
				mw.push_back(50000);
				species = 1;
				concentration.push_back( (float) 0.05 );
				extinction.push_back(4000);
				od.push_back((concentration[0]/1000) * pathlength * extinction[0]);
				pb_equil1->setEnabled(true);
				equil1 = 26;
				le_equil1->setEnabled(true);
				le_equil1->setText(str.sprintf(" %5.3e", equil1));
				break;
			}
			case 6: // Monomer-Tetramer Equilibrium
			{
				stoich1 = 4;
				vbar20.push_back( (float) 0.72 );
				mw.push_back(50000);
				species = 1;
				concentration.push_back( (float) 0.05 );
				extinction.push_back(4000);
				od.push_back((concentration[0]/1000) * pathlength * extinction[0]);
				pb_equil1->setEnabled(true);
				equil1 = 39;
				le_equil1->setEnabled(true);
				le_equil1->setText(str.sprintf(" %5.3e", equil1));
				break;
			}
			case 7: // Monomer-Pentamer Equilibrium
			{
				stoich1 = 5;
				vbar20.push_back( (float) 0.72 );
				mw.push_back(50000);
				species = 1;
				concentration.push_back( (float) 0.05 );
				extinction.push_back(4000);
				od.push_back((concentration[0]/1000) * pathlength * extinction[0]);
				pb_equil1->setEnabled(true);
				equil1 = 52;
				le_equil1->setEnabled(true);
				le_equil1->setText(str.sprintf(" %5.3e", equil1));
				break;
			}
			case 8: // Monomer-Hexamer Equilibrium
			{
				stoich1 = 6;
				vbar20.push_back( (float) 0.72 );
				mw.push_back(50000);
				species = 1;
				concentration.push_back( (float) 0.05 );
				extinction.push_back(4000);
				od.push_back((concentration[0]/1000) * pathlength * extinction[0]);
				pb_equil1->setEnabled(true);
				equil1 = 65;
				le_equil1->setEnabled(true);
				le_equil1->setText(str.sprintf(" %5.3e", equil1));
				break;
			}
			case 9: // Monomer-Heptamer Equilibrium
			{
				stoich1 = 7;
				vbar20.push_back( (float) 0.72 );
				mw.push_back(50000);
				species = 1;
				concentration.push_back( (float) 0.05 );
				extinction.push_back(4000);
				od.push_back((concentration[0]/1000) * pathlength * extinction[0]);
				pb_equil1->setEnabled(true);
				equil1 = 78;
				le_equil1->setEnabled(true);
				le_equil1->setText(str.sprintf(" %5.3e", equil1));
				break;
			}
			case 10:	// User-Defined Monomer - N-mer Equilibrium, we need some addtl. info from the user
			{
				US_SelectModel10 *SelectModel10;
				SelectModel10 = new US_SelectModel10(&stoich1);
				if (!SelectModel10->exec())
				{
					model = 0;
					break;
				}
				vbar20.push_back( (float) 0.72 );
				mw.push_back(50000);
				species = 1;
				concentration.push_back( (float) 0.05 );
				extinction.push_back(4000);
				od.push_back((concentration[0]/1000) * pathlength * extinction[0]);
				pb_equil1->setEnabled(true);
				equil1 = 13 * (stoich1 - 1);  // puts equil in micromolar range
				le_equil1->setEnabled(true);
				le_equil1->setText(str.sprintf(" %5.3e", equil1));
				break;
			}
			case 11: // Monomer-Dimer-Trimer Equilibrium
			{
				stoich1 = 2;
				stoich2 = 3;
				vbar20.push_back( (float) 0.72 );
				mw.push_back(50000);
				species = 1;
				concentration.push_back( (float) 0.05 );
				extinction.push_back(4000);
				od.push_back((concentration[0]/1000) * pathlength * extinction[0]);
				pb_equil1->setEnabled(true);
				pb_equil2->setEnabled(true);
				equil1 = 13 * (stoich1 - 1);  // puts equil in micromolar range
				equil2 = 13 * (stoich2 - 1);  // puts equil in micromolar range

				le_equil1->setEnabled(true);
				le_equil1->setText(str.sprintf(" %5.3e", equil1));
				le_equil2->setEnabled(true);
				le_equil2->setText(str.sprintf(" %5.3e", equil2));
				break;
			}
			case 12: // Monomer-Dimer-Tetramer Equilibrium
			{
				stoich1 = 2;
				stoich2 = 4;
				vbar20.push_back( (float) 0.72 );
				mw.push_back(50000);
				species = 1;
				concentration.push_back( (float) 0.05 );
				extinction.push_back(4000);
				od.push_back((concentration[0]/1000) * pathlength * extinction[0]);
				pb_equil1->setEnabled(true);
				pb_equil2->setEnabled(true);
				equil1 = 13 * (stoich1 - 1);  // puts equil in micromolar range
				equil2 = 13 * (stoich2 - 1);  // puts equil in micromolar range

				le_equil1->setEnabled(true);
				le_equil1->setText(str.sprintf(" %5.3e", equil1));
				le_equil2->setEnabled(true);
				le_equil2->setText(str.sprintf(" %5.3e", equil2));
				break;
			}
			case 13:	// User-Defined Monomer - N-mer - M-mer Equilibrium, we need some addtl. info from the user
			{
				US_SelectModel13 *SelectModel13;
				SelectModel13 = new US_SelectModel13(&stoich1, &stoich2);
				if (!SelectModel13->exec())
				{
					model = 0;
					break;
				}
				vbar20.push_back( (float) 0.72 );
				mw.push_back(50000);
				species = 1;
				concentration.push_back( (float) 0.05 );
				extinction.push_back(4000);
				od.push_back((concentration[0]/1000) * pathlength * extinction[0]);
				pb_equil1->setEnabled(true);
				pb_equil2->setEnabled(true);
				equil1 = 13 * (stoich1 - 1);  // puts equil in micromolar range
				equil2 = 13 * (stoich2 - 1);  // puts equil in micromolar range

				le_equil1->setEnabled(true);
				le_equil1->setText(str.sprintf(" %5.3e", equil1));
				le_equil2->setEnabled(true);
				le_equil2->setText(str.sprintf(" %5.3e", equil2));
				break;
			}
			default:
			{
				model = 0;
				break;
			}
		}
	}
	else // use single component ideal system as default
	{
		model = 0;
		species = 1;
	}
	lbl_model->setText(modelString[model]);
	select_species((double) 1);
}

void US_EquilSim::select_species(double num)
{
	QString str;
	current_species = (int) (num - 1);
	le_mw->setText(str.sprintf(" %5.3e", mw[current_species]));
	le_concentration->setText(str.sprintf(" %5.3e", concentration[current_species]));
	le_vbar->setText(str.sprintf(" %5.3e", vbar20[current_species]));
	le_extinction->setText(str.sprintf(" %5.3e", extinction[current_species]));
}

void US_EquilSim::change_equil1()
{
	QString str;
	if (assoc1)
	{
		assoc1 = false;
		float tmp = equil1;
		pb_equil1->setText(tr("Dissoc. Const. 1:"));
		le_equil1->setText(str.sprintf("%5.3e", 1.0/exp(equil1)));
		qApp->processEvents();
		equil1 = tmp;
	}
	else
	{
		assoc1 = true;
		pb_equil1->setText(tr("Ln(Assoc. Const. 1):"));
		le_equil1->setText(str.sprintf("%5.3e", equil1));
	}
}

void US_EquilSim::update_equil1(const QString & str)
{
	if (assoc1)
	{
		equil1 = str.toFloat();
	}
	else
	{
		equil1 = 1.0/(str.toFloat());
	}
}

void US_EquilSim::change_equil2()
{
	QString str;
	if (assoc2)
	{
		assoc2 = false;
		float tmp = equil2;
		pb_equil2->setText(tr("Dissoc. Const. 2:"));
		le_equil2->setText(str.sprintf(" %5.3e", 1.0/exp(equil2)));
		qApp->processEvents();
		equil2 = tmp;
	}
	else
	{
		assoc2 = true;
		pb_equil2->setText(tr("Ln(Assoc. Const. 2):"));
		le_equil2->setText(str.sprintf(" %5.3e", equil2));
	}
}

void US_EquilSim::update_equil2(const QString & str)
{
	if (assoc2)
	{
		equil2 = str.toFloat();
	}
	else
	{
		equil2 = 1.0/(str.toFloat());
	}
}

void US_EquilSim::change_equil3()
{
	QString str;
	if (assoc3)
	{
		assoc3 = false;
		float tmp = equil3;
		pb_equil3->setText(tr("Dissoc. Const. 3:"));
		le_equil3->setText(str.sprintf(" %5.3e", 1.0/exp(equil3)));
		qApp->processEvents();
		equil3 = tmp;
	}
	else
	{
		assoc3 = true;
		pb_equil3->setText(tr("Ln(Assoc. Const. 3):"));
		le_equil3->setText(str.sprintf(" %5.3e", equil3));
	}
}

void US_EquilSim::update_equil3(const QString & str)
{
	if (assoc3)
	{
		equil3 = str.toFloat();
	}
	else
	{
		equil3 = 1.0/(str.toFloat());
	}
}

void US_EquilSim::update_mw(const QString & str)
{
	QString str1;
	mw[current_species] = str.toFloat();
}

void US_EquilSim::update_concentration(const QString & str)
{
	QString str1;
	concentration[current_species] = str.toFloat();
	od[current_species] = pathlength * extinction[current_species] * concentration[current_species]/1000;
	lbl_od_value->setText(str1.sprintf(" %5.2f", od[current_species]));
}

void US_EquilSim::change_vbar()
{
	float vbarTemp;	// not used
	US_Vbar_DB *vbar_dlg;
	vbar_dlg = new US_Vbar_DB(temperature, &vbarTemp, &vbar20[current_species],true,false, -1);
	vbar_dlg->setCaption(tr("V-bar Calculation"));
	vbar_dlg->pb_ok->setText(tr(" Close "));
	connect(vbar_dlg, SIGNAL(e280Changed(float)), SLOT(update_e280(float)));
	connect(vbar_dlg, SIGNAL(valueChanged(float, float)), SLOT(update_vbar(float, float)));
	vbar_dlg->show();
}

void US_EquilSim::update_vbar(const QString & str)
{
	vbar20[current_species] = str.toFloat();
}

void US_EquilSim::update_vbar(float val1, float val2)
{
	QString str;
	float vbarTemp;
	vbarTemp = val1;	// not used
	vbar20[current_species] = val2;
	le_vbar->setText(str.sprintf(" %5.3e", vbar20[current_species]));
}

void US_EquilSim::update_baseline(const QString & str)
{
	baseline = str.toFloat();
}

void US_EquilSim::update_extinction(const QString & str)
{
	QString str1;
	extinction[current_species] = str.toFloat();
	od[current_species] = pathlength * extinction[current_species] * concentration[current_species]/1000;
	lbl_od_value->setText(str1.sprintf(" %5.2f", od[current_species]));
}

void US_EquilSim::change_extinction()
{
	float vbarTemp;	// not used
	US_Vbar_DB *vbar_dlg;
	vbar_dlg = new US_Vbar_DB(temperature, &vbarTemp, &vbar20[current_species],true,false, -1);
	vbar_dlg->setCaption(tr("V-bar and E280 Calculation"));
	vbar_dlg->pb_ok->setText(tr(" Close "));
	connect(vbar_dlg, SIGNAL(e280Changed(float)), SLOT(update_e280(float)));
	connect(vbar_dlg, SIGNAL(valueChanged(float, float)), SLOT(update_vbar(float, float)));
	vbar_dlg->show();
}

void US_EquilSim::update_e280(float val)
{
	QString str;
	extinction[current_species] = val;
	od[current_species] = pathlength * extinction[current_species] * concentration[current_species]/1000;
	lbl_od_value->setText(str.sprintf(" %5.2f", od[current_species]));
}

void US_EquilSim::update_conc_increment(const QString & str)
{
	QString str1;
	conc_increment = str.toFloat();
}

void US_EquilSim::update_pathlength(const QString & str)
{
	QString str1;
	pathlength = str.toFloat();
}

void US_EquilSim::select_conc_steps(double num)
{
	conc_steps = (unsigned int) num;
}

void US_EquilSim::select_speed_start(double num)
{
	speed_start = (unsigned int) num;
	cnt_speed_stop->setRange(speed_start, 65000, 100);
	if (speed_start == speed_stop)
	{
		speed_steps = 1;
		cnt_speed_steps->setRange(1, 1, 1);
	}
	else
	{
		cnt_speed_steps->setRange(1, 100, 1);
	}
}

void US_EquilSim::update_temperature(const QString & str)
{
	QString str1;
	temperature = str.toFloat();
}

void US_EquilSim::select_speed_stop(double num)
{
	speed_stop = (unsigned int) num;
	cnt_speed_start->setRange(100, speed_stop, 100);
	if (speed_start == speed_stop)
	{
		speed_steps = 1;
		cnt_speed_steps->setRange(1, 1, 1);
	}
	else
	{
		cnt_speed_steps->setRange(1, 100, 1);
	}
}

void US_EquilSim::change_density()
{
	QString dummy = "";
	buffer_dlg = new US_Buffer_DB(false, -1);
	buffer_dlg->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	buffer_dlg->setCaption(tr("Buffer Calculation"));
	buffer_dlg->show();
	connect(buffer_dlg, SIGNAL(valueChanged(float, float)), SLOT(update_density_label(float, float)));
}

void US_EquilSim::update_density_label(float table_density, float)
{
	QString str;
	density = calc_density_tb(table_density);
	le_density->setText(str.sprintf(" %7.5f", density));
}

float US_EquilSim::calc_density_tb(float table_density)
{
// density of parameter list is density of buffer at 20C
// t of parameter list is temperature in Celsius
	float t = temperature;
	float density_wt, density_wt_20, density_tb;
	density_wt	 = 1.000028e-3 * ((999.83952 + 16.945176 * t) / (1.0 + 16.879850e-3 * t))
					 - 1.000028e-3 * ((7.9870401e-3 * pow((double) t, (double) 2.0) + 46.170461e-6 * pow((double) t, (double) 3.0))
										/ (1.0 + 16.87985e-3 * t))
					 + 1.000028e-3 * ((105.56302e-9 * pow((double) t, (double) 4.0) - 280.54253e-12 * pow((double) t, (double) 5.0))
										/ (1.0 + 16.87985e-3 * t));
	density_wt_20	 = 1.000028e-3 * ((999.83952 + 16.945176 * 20.0) / (1.0 + 16.879850e-3 * 20.0))
					 - 1.000028e-3 * ((7.9870401e-3 * pow((double) 20.0, (double) 2.0) + 46.170461e-6 * pow((double) 20.0, (double) 3.0))
										/ (1.0 + 16.87985e-3 * 20.0))
					 + 1.000028e-3 * ((105.56302e-9 * pow((double) 20.0, (double) 4.0) - 280.54253e-12 * pow((double) 20.0, (double) 5.0))
										/ (1.0 + 16.87985e-3 * 20.0));
	density_tb = table_density * density_wt / density_wt_20;
	return(density_tb);
}

void US_EquilSim::update_density(const QString & str)
{
	density = str.toFloat();
}

void US_EquilSim::update_max_od(const QString & str)
{
	max_od = str.toFloat();
}

void US_EquilSim::update_lambda(const QString & str)
{
	lambda = str.toFloat();
}

void US_EquilSim::select_speed_steps(double num)
{
	speed_steps = (unsigned int) num;
}

void US_EquilSim::select_column(double num)
{
	QString str;
	column = (float) num;
	meniscus = bottom - column/10;
	lbl_meniscus_value->setText(str.sprintf(" %6.3f cm", meniscus));
	delta_r = column/(10 * (datapoints-1));
	lbl_delta_r_value->setText(str.sprintf(" %6.4e cm", delta_r));
}


void US_EquilSim::select_bottom(double num)
{
	QString str;
	bottom = (float) num;
	meniscus = bottom - column/10;
	lbl_meniscus_value->setText(str.sprintf(" %6.3f cm", meniscus));
}

void US_EquilSim::select_datapoints(double num)
{
	QString str;
	datapoints = (unsigned int) num;
	delta_r = column/(10 * (datapoints-1));
	lbl_delta_r_value->setText(str.sprintf(" %6.4e cm", delta_r));
}

void US_EquilSim::select_noise(double num)
{
	noise = (float) num/100;
}

void US_EquilSim::select_nonlin_noise(double num)
{
	nonlin_noise = (float) num/100;
}

void US_EquilSim::select_ultrascan()
{
	export_format = true;
	cb_ultrascan->setChecked(true);
	cb_xla->setChecked(false);
}

void US_EquilSim::select_xla()
{
	export_format = false;
	cb_ultrascan->setChecked(false);
	cb_xla->setChecked(true);
}

void US_EquilSim::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/equilsim.html");
}

void US_EquilSim::plot()
{
	double **x, **y;
	float ***amplitude, sum=0;
	float *partial_od, time_data; // what fraction does each component contribute to the total OD
	unsigned int *curve, steps, i, j, k, count;
	int n;
	QwtSymbol symbol;
	symbol.setSize(8);
	symbol.setStyle(QwtSymbol::Ellipse);
	symbol.setPen(Qt::blue);
	symbol.setBrush(Qt::yellow);
	QString str1, str2;
	QFile f;
	scans.clear();
	partial_od = new float [species];
	for (i=0; i<species; i++)
	{
		sum += od[i];
	}
	for (i=0; i< species; i++)
	{
		partial_od[i] = od[i]/sum;
	}
	steps = conc_steps * speed_steps;
	if (!plot_widget)
	{
		data_plot = new US_2dPlot();
		data_plot->setCaption("Equilibrium Scans");
		data_plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
		data_plot->enableGridXMin();
		data_plot->enableGridYMin();
		data_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
		data_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
		data_plot->setCanvasBackground(USglobal->global_colors.plot);
		data_plot->enableOutline(true);
		data_plot->setAxisTitle(QwtPlot::xBottom, tr("Radius (cm)"));
		data_plot->setAxisTitle(QwtPlot::yLeft, tr("Optical Density"));
		data_plot->setTitle(modelString[model]);
		data_plot->setOutlinePen(white);
		data_plot->setOutlineStyle(Qwt::Cross);
		data_plot->setGeometry(global_Xpos + 30, global_Ypos + 30, 700, 603);
		data_plot->show();
		connect(data_plot, SIGNAL(plotClosed()), SLOT(plot_closeEvent()));
		plot_widget = true;
		pb_plot->setText(tr("Update Plot"));
	}
	else
	{
		data_plot->removeCurves();
		data_plot->replot();
	}
	speeds.clear();
	if (speed_steps == 1)
	{
		speeds.push_back(speed_start);
	}
	if (speed_steps == 2)
	{
		speeds.push_back(speed_start);
		speeds.push_back(speed_stop);
	}
	if (speed_steps > 2)
	{
		speeds.push_back(speed_start);
		float temp;
		temp = (speed_stop - speed_start)/(speed_steps - 1);
		for (i=1; i<speed_steps-1; i++)
		{
			speeds.push_back(speed_start + (unsigned int) (temp * i));
		}
		speeds.push_back(speed_stop);
	}
	concs.clear();
	for (i=0; i<conc_steps; i++)
	{
		concs.push_back(i * conc_increment);	// calculate the concentration steps,
	}														// first step i=0 means starting concentration is used

	x = new double * [steps];
	y = new double * [steps];
	amplitude = new float ** [species];
	for (i=0; i<species; i++)
	{
		amplitude[i] = new float * [speed_steps];
		for (j=0; j<speed_steps; j++)
		{
			amplitude[i][j] = new float [conc_steps];
		}
	}
	curve = new unsigned int [steps];
	for (i=0; i<steps; i++)
	{
		x[i] = new double [datapoints];
		y[i] = new double [datapoints];
	}
	calc_conc(amplitude, partial_od);
	count=0;
	for (i=0; i<speed_steps; i++)
	{
		for (j=0; j<conc_steps; j++)
		{
			calc_function(x, y, i, j, count, amplitude);
			count ++;
		}
	}
	bin.clear();
	frequency.clear();
	for (i=0; i<bin_number; i++)
	{
		bin.push_back((i + 1) * max_od/bin_number);
		frequency.push_back(0);
	}

	for (i=0; i<steps; i++)
	{
		k=0; //bin number
		for (j=0; j<datapoints; j++)
		{
			if (y[i][j] < bin[k] && k < bin_number)	//if the y-value is below the bin cutoff, increment,
			{														//otherwise increment to next bin and add the value to the next
				frequency[k] ++;
			}
			else
			{
				k++;
				if (k == bin_number)
				{
					break;
				}
				frequency[k] ++;
			}
		}
	}
	if (export_flag)
	{
		if(export_format)	// UltraScan format
		{
			f.setName(fileName + ".us.e");
			int et;
			if (f.open(IO_WriteOnly))
			{
				QDataStream ds(&f);
				ds << US_Version;
				ds << USglobal->config_list.data_dir;
				n = fileName.findRev("/", -1, false);
				str1 = fileName.right(fileName.length() - n - 1); // run ID
				ds << str1;
				time_data =  (float) (345600 + 200 * steps);	// time
				ds << time_data;
				ds << steps;	// total scans
				ds << delta_r;
				ds << (int) -1;		//experimental data ID
				ds << (int) -1;		//Investigator ID
				QDate today = QDate::currentDate();
				QString current_date;
				current_date.sprintf( "%d/%d/%d", today.month(), today.day(), today.year() );
				ds << current_date;
				ds << (QString) "Simulated Equilibrium Data";
				ds << (QString) "ultrascan";
				ds << (QString) "192.168.0.1";
				ds << (QString) "QMYSQL3";
				et = 0;
				ds << et; // run_inf.exp_type.velocity;
				et = 1;
				ds << et; // run_inf.exp_type.equilibrium;
				et = 0;
				ds << et; // run_inf.exp_type.diffusion;
				et = 1;
				ds << et; // run_inf.exp_type.simulation;
				et = 0;
				ds << et; // run_inf.exp_type.interference;
				et = 1;
				ds << et; // run_inf.exp_type.absorbance;
				et = 0;
				ds << et; // run_inf.exp_type.fluorescence;
				ds << et; // run_inf.exp_type.intensity;
				ds << et; // run_inf.exp_type.wavelength;
				ds << (int) 0; // centerpiece serial number
				ds << (QString) tr("Simulated Data"); // cell contents
				ds << (unsigned int) 1; // wavelength count
				for (i=0; i<7; i++)
				{
					ds << (int) -2; // centerpiece is unused, therefore negative
					ds << (QString) "not used"; // cell contents
					ds << (unsigned int) 0; // wavelength count
				}
				int serial_number = -1;
				for (i=0; i<8; i++)
				{
					for (j=0; j<4; j++)
					{
						ds << serial_number;	//buffer serial number
						for (int l=0; l<3; l++)
						{
							ds << serial_number;	//peptide serial number
							ds << serial_number;	//DNA serial number
						}
					}
					for (j=0; j<3; j++)
					{
						if (i == 0 && j == 0)
						{
							ds << scans[0].lambda;	// wavelength
							ds << steps; // number of scans
							ds << meniscus;
							ds << bottom;
							ds << datapoints;
							ds << delta_r; // point density
						}
						else
						{
							ds << (unsigned int) 0;	// wavelength
							ds << (unsigned int) 0;	// number of scans
						}
					}
				}

				for (i=0; i<steps; i++)
				{
					ds << scans[i].speed;
					ds << scans[i].temperature;
					ds << scans[i].time;
					ds << scans[i].omega;
				}
				ds << (int) 0;
				f.flush();
				f.close();
			}
			f.setName(fileName + ".equil.111");
			if (f.open(IO_WriteOnly))
			{
				QDataStream ds(&f);
				for (i=0; i<steps; i++)
				{
					ds << meniscus;
					for (j=0; j<datapoints; j++)
					{
						ds << scans[i].absorbance[j];
					}
				}
			}
			f.flush();
			f.close();
		}
		else	// XLA format
		{
			for (i=0; i<steps; i++)
			{
				if (i < 9)
				{
					str2 = fileName + "/0000" + str1.sprintf("%d", i+1) + ".ra1";
				}
				else if (i >= 9 && i < 99)
				{
					str2 = fileName + "/000" + str1.sprintf("%d", i+1) + ".ra1";
				}
				else if (i >= 99 && i < 999)
				{
					str2 = fileName + "/00" + str1.sprintf("%d", i+1) + ".ra1";
				}
				else if (i >= 999 && i < 9999)
				{
					str2 = fileName + "/0" + str1.sprintf("%d", i+1) + ".ra1";
				}
				else if (i >= 9999)
				{
					str2 = fileName + "/" + str1.sprintf("%d", i+1) + ".ra1";
				}
				f.setName(str2);
				if (f.open(IO_WriteOnly | IO_Translate))
				{
					QTextStream ts(&f);
					ts << "Simulated Equilibrium Data" << endl;
					ts << scans[i].mode << " 1 " << str1.sprintf("%2.1f %5d %7d %e %d %d\n",
																			scans[i].temperature,
																			scans[i].speed,
																			scans[i].time,
																			scans[i].omega,
																			scans[i].lambda,
																			scans[i].averages);
					for (j=0; j<scans[i].radius.size(); j++)
					{
						ts << str1.sprintf("%9.4f %12.5e %12.5e", scans[i].radius[j], scans[i].absorbance[j], scans[i].sigma[j]) << endl;
					}
				}
				f.close();
			}
		}
	}
	for (i=0; i<steps; i++)
	{
		curve[i] = data_plot->insertCurve("equil_scan");
		data_plot->setCurveData(curve[i], x[i], y[i], datapoints);
		data_plot->setCurvePen(curve[i], QPen(Qt::blue, 1, SolidLine));
		data_plot->setCurveSymbol(curve[i], symbol);
	}
	data_plot->setAxisScale(QwtPlot::yLeft, baseline, max_od);
	data_plot->replot();

	for (i=0; i<steps; i++)
	{
		delete [] x[i];
		delete [] y[i];
	}
	for (i=0; i<species; i++)
	{
		for (j=0; j<speed_steps; j++)
		{
			delete [] amplitude[i][j];
		}
		delete [] amplitude[i];
	}
	delete [] amplitude;
	delete [] x;
	delete [] y;
	delete [] curve;
	delete [] partial_od;
}

void US_EquilSim::calc_function(double **x, double **y, unsigned int i, unsigned int j,
unsigned int count, float ***amplitude)
{
	unsigned int n, k;
	struct xlaScan temp_scan;
	float sigma1, temp, vbarTemp, density_tb, arg1, arg2, arg3;
	density_tb = calc_density_tb(density);
	temp_scan.radius.clear();
	temp_scan.absorbance.clear();
	temp_scan.sigma.clear();
	for (n=0; n<datapoints; n++)
	{
		x[count][n] = meniscus + n * delta_r;
		temp_scan.radius.push_back(x[count][n]);
		y[count][n] = 0;
	}
	if (model >= 0 && model < 4)
	{
		for (k=0; k<species; k++)	// first calculate all the contributions from all components
		{
			vbarTemp = adjust_vbar20(vbar20[k], temperature);
			sigma1 = mw[k] * pow((double) ((M_PI/30) * speeds[i]), (double) 2) * (1 - vbarTemp * density_tb)/(2 * R * (K0 + temperature));
			temp_scan.speed = speeds[i];
			temp_scan.temperature = temperature;
			temp_scan.mode = "R";
			temp_scan.time = 345600 + 200 * count;	// 4 days plus some scan time
			temp_scan.omega = temp_scan.time * pow((double) (M_PI/30 * speeds[i]), (double) 2);
			temp_scan.lambda = (unsigned int) lambda;
			temp_scan.averages = 50;
			for (n=0; n<datapoints; n++)
			{
				y[count][n] += amplitude[k][i][j] * exp(sigma1 * (pow((double) x[count][n], (double) 2) - pow((double) meniscus, (double) 2)));
//cout << "Y: " << y[count][n] << ", X^2: " << (pow(x[count][n], 2) - pow(meniscus, 2)) << ", X: " << x[count][n] << endl;
			}
		}
	}
	if (model >= 4 && model < 11)	// monomer - N-mer equilibrium
	{
		vbarTemp = adjust_vbar20(vbar20[0], temperature);
		sigma1 = mw[0] * pow((double) (M_PI/30 * speeds[i]), (double) 2) * (1 - vbarTemp * density_tb)/(2 * R * (K0 + temperature));
		temp_scan.speed = speeds[i];
		temp_scan.temperature = temperature;
		temp_scan.mode = "R";
		temp_scan.time = 345600 + 200 * count;	// 4 days plus some scan time
		temp_scan.omega = temp_scan.time * pow((double) (M_PI/30 * speeds[i]), (double) 2);
		temp_scan.lambda = (unsigned int) lambda;
		temp_scan.averages = 50;
		for (n=0; n<datapoints; n++)
		{
			arg1 = log(amplitude[0][i][j]) + sigma1 * (pow((double) x[count][n], (double) 2) - pow((double) meniscus, (double) 2));
			arg2 = stoich1 * log(amplitude[0][i][j])
					 + log(stoich1/(pow((double) (extinction[0]*pathlength), (double) (stoich1-1))))
					 + equil1 + stoich1 * sigma1 * (pow((double) x[count][n], (double) 2) - pow((double) meniscus, (double) 2));
			y[count][n] = exp(arg1)	+ exp(arg2);
		}
	}
	if (model >= 11 && model < 14)	// monomer - N-mer - M-mer equilibrium
	{
		vbarTemp = adjust_vbar20(vbar20[0], temperature);
		sigma1 = mw[0] * pow((double) (M_PI/30 * speeds[i]), (double) 2) * (1 - vbarTemp * density_tb)/(2 * R * (K0 + temperature));
		temp_scan.speed = speeds[i];
		temp_scan.temperature = temperature;
		temp_scan.mode = "R";
		temp_scan.time = 345600 + 200 * count;	// 4 days plus some scan time
		temp_scan.omega = temp_scan.time * pow((double) (M_PI/30 * speeds[i]), (double) 2);
		temp_scan.lambda = (unsigned int) lambda;
		temp_scan.averages = 50;
		for (n=0; n<datapoints; n++)
		{
			arg1 = log(amplitude[0][i][j]) + sigma1 * (pow((double) x[count][n], (double) 2) - pow((double) meniscus, (double) 2));
			arg2 = stoich1 * log(amplitude[0][i][j])
					 + log(stoich1/(pow((double) (extinction[0]*pathlength), (double) (stoich1-1))))
					 + equil1 + stoich1 * sigma1 * (pow((double) x[count][n], (double) 2) - pow((double) meniscus, (double) 2));
			arg3 = stoich2 * log(amplitude[0][i][j])
					 + log(stoich2/(pow((double) (extinction[0]*pathlength), (double) (stoich2-1))))
					 + equil2 + stoich2 * sigma1 * (pow((double) x[count][n], (double) 2) - pow((double) meniscus, (double) 2));
			y[count][n] = exp(arg1)	+ exp(arg2)	+ exp(arg3);
		}
	}
	for (n=0; n<datapoints; n++) // add baseline, then all noise components
	{
		y[count][n] +=  baseline;
		temp = box_muller(0, nonlin_noise * y[count][n]) + box_muller(0, noise);
		temp_scan.sigma.push_back(nonlin_noise * y[count][n] + noise);
		y[count][n] += temp;
		temp_scan.absorbance.push_back(y[count][n]);
	}
	scans.push_back(temp_scan);
}

void US_EquilSim::calc_conc(float ***amplitude, float *partial_od) //calculate the amplitudes
{
	unsigned int i, j, k;
	float sigma, vbarTemp, density_tb, integral, temp1, temp2;
//	float taylor;
	density_tb = calc_density_tb(density);
	if (model>= 0 && model < 4)
	{
		for (k=0; k<species; k++)
		{
			vbarTemp = adjust_vbar20(vbar20[k], temperature);
			for (i=0; i<speed_steps; i++)
			{
				sigma = mw[k] * pow((double) (M_PI/30 * speeds[i]), (double) 2) * (1 - vbarTemp * density_tb)/(2 * R * (K0 + temperature));

//cout << "Sigma: " << sigma << endl;
//				temp1 = sigma * (pow(bottom, 2) - pow(meniscus, 2));
//				temp2 = pow(temp1, 0.5)/(sigma * bottom);
//cout << "argument: " << temp2 << endl;
//				taylor = calc_taylor(temp2,  100 /*number of taylor terms*/);
//cout << "taylor: " << taylor << endl;
				for (j=0; j<conc_steps; j++)
				{
					integral = (bottom - meniscus) * (od[k] + concs[j] * partial_od[k]);
					float sum=0, xval, dr=(bottom - meniscus)/500;
					temp1 = dr;
					for (int m=1; m<500; m++)
					{
						xval = meniscus + m * dr;
						temp2 = exp(sigma*(pow((double) xval, (double) 2) - pow((double) meniscus, (double) 2))) * dr;
						sum += temp1 + (temp2 - temp1)/2.0;
						temp1 = temp2;
					}
//cout << "Numeric integral: " << sum << endl;
//cout << "Integral: " << integral << ", od[" << k << "]: " << od[k] << ", Column: " << bottom-meniscus << ", concs[" << j << "]: " << concs[j] << ", partial_od: " << partial_od[k] <<endl;
//					amplitude[k][i][j] = integral/taylor;
					amplitude[k][i][j] = integral/sum;
//cout << "Amplitude[" << k << "][" << i << "][" << j << "]: " << amplitude[k][i][j] << endl;
				}
			}
		}
	}
	if (model>= 4 && model < 14)
	{
		vbarTemp = adjust_vbar20(vbar20[0], temperature);
		for (i=0; i<speed_steps; i++)
		{
			sigma = mw[0] * pow((double) (M_PI/30 * speeds[i]), (double) 2) * (1 - vbarTemp * density_tb)/(2 * R * (K0 + temperature));
			for (j=0; j<conc_steps; j++)
			{
				integral = (bottom - meniscus) * (od[0] + concs[j] * partial_od[0]);
				amplitude[0][i][j] = find_root(integral, sigma, (od[0] + concs[j] * partial_od[0]));
			}
		}
	}
}

double US_EquilSim::calc_taylor(double arg, unsigned int terms)
{
	double taylor=0;
	unsigned int i, exponent;
	double *factorial;
	factorial = new double [terms];

	factorial[0] = 1;
	for (i=1; i<terms; i++) // calculate the factorials
	{
		factorial[i] = factorial[i-1] * i;
//cout << "Factorial["<<i<<"]: " << factorial[i] << endl;
	}
	for (i=0; i<terms; i++) // calculate the Taylor series for e^x
	{
		exponent = 2 * i + 2;
		taylor += (pow((double) arg, (double) exponent) / (factorial[i] * exponent));
//cout << "taylor["<<i<<"]: " << taylor << ", arg: " << arg << ", expo: " << exponent <<endl;
	}
	delete [] factorial;
	return(taylor);
}

void US_EquilSim::plot_closeEvent()
{
	plot_widget = false;
	pb_plot->setText(tr("Plot"));
}

void US_EquilSim::histogram()
{
	double *x, *y;
	unsigned int curve, i;
	QFile histogramFile;
	if (export_flag)
	{
		QFileDialog fd;
		if (export_format)	// UltraScan format
		{
			fileName = fd.getSaveFileName(USglobal->config_list.result_dir, "*.us.e", this, "equil_dialog",
			"Please select a filename for the simulated data:", 0, true);
		}
		else	// XLA format
		{
			fileName = fd.getExistingDirectory(USglobal->config_list.data_dir, this, "equil_dialog",
			"Please select an output Directory for the simulated data:", 0, true);
		}
		if (fileName.stripWhiteSpace() == "" || fileName.isNull())
		{
			export_flag = false;
		}
		else // strip extension, if it exists
		{
			if (fileName.right(5) == ".us.e")
			{
				fileName.truncate(fileName.length() - 5);
			}
		}
	}
	plot();
	if (!histogram_widget)
	{
		histogram_plot = new US_2dPlot();
		histogram_plot->setCaption("Equilibrium Histogram");
		histogram_plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
		histogram_plot->enableGridXMin();
		histogram_plot->enableGridYMin();
		histogram_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
		histogram_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
		histogram_plot->setCanvasBackground(USglobal->global_colors.plot);
		histogram_plot->enableOutline(true);
		histogram_plot->setAxisTitle(QwtPlot::xBottom, tr("Concentration Bin"));
		histogram_plot->setAxisTitle(QwtPlot::yLeft, tr("Frequency"));
		histogram_plot->setTitle("Concentration Histogram");
		histogram_plot->setOutlinePen(white);
		histogram_plot->setOutlineStyle(Qwt::Cross);
		histogram_plot->setGeometry(global_Xpos + 30, global_Ypos + 30, 700, 603);
		histogram_plot->show();
		connect(histogram_plot, SIGNAL(plotClosed()), SLOT(histogram_closeEvent()));
		histogram_widget = true;
		pb_histogram->setText(tr("Update Histogram"));
	}
	else
	{
		histogram_plot->removeCurves();
		histogram_plot->replot();
	}
	x = new double [bin_number];
	y = new double [bin_number];
	for (i=0; i<bin_number; i++)
	{
		x[i] = (double) bin[i];
		y[i] = (double) frequency[i];
	}
	if (export_flag)
	{
		if(export_format)
		{
			histogramFile.setName(fileName + ".histogram.dat");
		}
		else
		{
			histogramFile.setName(fileName + "/histogram.dat");
		}
		if (histogramFile.open(IO_WriteOnly | IO_Translate))
		{
			QTextStream ts(&histogramFile);
			for (i=0; i<bin_number; i++)
			{
				ts << x[i] << "\t" << y[i] << endl;
			}
		}
		histogramFile.close();
	}
	curve = histogram_plot->insertCurve("histogram");
	histogram_plot->setCurveStyle(curve, QwtCurve::Sticks);
	histogram_plot->setCurvePen(curve, QPen(Qt::red, 7));
	histogram_plot->setCurveData(curve, x, y, bin_number);
	QwtSymbol symbol;
	symbol.setSize(12);
	symbol.setStyle(QwtSymbol::Ellipse);
	symbol.setPen(Qt::blue);
	symbol.setBrush(Qt::yellow);
	histogram_plot->setCurveSymbol(curve, symbol);
	histogram_plot->replot();

	delete [] x;
	delete [] y;
}

void US_EquilSim::histogram_closeEvent()
{
	histogram_widget = false;
	pb_histogram->setText(tr("Histogram"));
}

void US_EquilSim::export_data()
{
	export_flag = true;
	histogram();
}

void US_EquilSim::quit()
{
	close();
}

void US_EquilSim::closeEvent(QCloseEvent *e)
{
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
}

float US_EquilSim::find_root(float integral, float sigma, float od)
{
   double monomer_amplitude, test, diff, monomer_amplitude_old = 0, upper, lower;
   monomer_amplitude=od; // maximum upper limit makes a good positive bracket
   test=integral_root(monomer_amplitude, integral, sigma);
   while (test > 0) // search for a lower bracket
   {
      monomer_amplitude_old = monomer_amplitude;
		monomer_amplitude -= monomer_amplitude/2;
		test = integral_root(monomer_amplitude, integral, sigma);
	}

// now we should have a bracket (monomer_amplitude_old: test>0, and monomer_amplitude: test<0)

	upper = monomer_amplitude_old;
	lower = monomer_amplitude;
	diff = (upper - lower)/2;
	monomer_amplitude += diff; // we are coming in with a negative test, so add diff
	while (fabs(test) > 1e-6)
	{
		test = integral_root(monomer_amplitude, integral, sigma);
		if (test < 0) // reassign upper or lower bracket
		{
			lower = monomer_amplitude;
		}
		else
		{
			upper = monomer_amplitude;
		}
		diff = (upper - lower)/2; // find a new subdivision
		if (test > 0) // if the last bracket was positive, subtract difference, otherwise add
		{
			monomer_amplitude -= diff;
		}
		else
		{
			monomer_amplitude += diff;
		}
	}
   return (monomer_amplitude);
}

float US_EquilSim::integral_root(float monomer_amplitude, float integral, float sigma)
{
	float temp1, temp2, sum=0, xval, dr=(bottom - meniscus)/500, arg1, arg2, arg3;
	if (stoich2 == 0) // monomer-n-mer only
	{
		arg1 = stoich1 * log(monomer_amplitude) + log(stoich1/(pow((double) (extinction[0] * pathlength), (double) (stoich1-1)))) + equil1;
		temp1 = dr * (monomer_amplitude + exp(arg1));
		for (int m=1; m<500; m++)
		{
			xval = meniscus + m * dr;
			arg1 = log(monomer_amplitude) + sigma * (pow((double) xval, (double) 2) - pow((double) meniscus, (double) 2));
			arg2 = stoich1 * log(monomer_amplitude)
					 + log(stoich1/(pow((double) (extinction[0]*pathlength), (double) (stoich1-1))))
					 + equil1 + stoich1 * sigma * (pow((double) xval, (double) 2) - pow((double) meniscus, (double) 2));
			temp2 = dr * (exp(arg1) + exp(arg2));
			sum += temp1 + (temp2 - temp1)/2.0;
			temp1 = temp2;
		}
	}
	else // monomer-n-mer-m-mer
	{
		arg1 = stoich1 * log(monomer_amplitude)
				+ log(stoich1 / (pow((double) (extinction[0] * pathlength), (double) (stoich1-1)))) + equil1;
		arg2 = stoich2 * log(monomer_amplitude)
				+ log(stoich2 / (pow((double) (extinction[0] * pathlength), (double) (stoich2-1)))) + equil2;
		temp1 = dr * (monomer_amplitude + exp(arg1) + exp(arg2));
		for (int m=1; m<500; m++)
		{
			xval = meniscus + m * dr;
			arg1 = log(monomer_amplitude) + sigma * (pow((double) xval, (double) 2) - pow((double) meniscus, (double) 2));
			arg2 = stoich1 * log(monomer_amplitude)
					 + log(stoich1/(pow((double) (extinction[0]*pathlength), (double) (stoich1-1))))
					 + equil1 + stoich1 * sigma * (pow((double) xval, (double) 2) - pow((double) meniscus, (double) 2));
			arg3 = stoich2 * log(monomer_amplitude)
					 + log(stoich2/(pow((double) (extinction[0]*pathlength), (double) (stoich2-1))))
					 + equil2 + stoich2 * sigma * (pow((double) xval, (double) 2) - pow((double) meniscus, (double) 2));
			temp2 = dr * (exp(arg1) + exp(arg2) + exp(arg3));
			sum += temp1 + (temp2 - temp1)/2.0;
			temp1 = temp2;
		}
	}
	return (sum	- integral);
}

