#include "../include/us_simulationparameters.h"

US_SimulationParameters::US_SimulationParameters(struct SimulationParameters *simparams, QWidget *parent, const char *name)
: QDialog( parent, name, true)
{
	USglobal = new US_Config();
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	QString str;
	int minHeight1 = 26, minHeight2 = 30;
	setCaption(tr("ASTFEM Simulation Parameters"));
	this->simparams = simparams;

	backup_simparams.speed_step.resize((*simparams).speed_step.size());
	current_speed_step = 0;
	cmb_speeds = new QComboBox(false, this, "Speed Profile Listings" );
	cmb_speeds->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cmb_speeds->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cmb_speeds->setSizeLimit(5);
	cmb_speeds->setMinimumHeight(minHeight1);
	for (unsigned int i=0; i<(*simparams).speed_step.size(); i++)
	{
		str.sprintf("Speed Profile %d: %d hr %d min, %d rpm", i+1,
		(*simparams).speed_step[i].duration_hours,
		(*simparams).speed_step[i].duration_minutes,
		(*simparams).speed_step[i].rotorspeed);
		cmb_speeds->insertItem(str, -1);
	}
	connect(cmb_speeds, SIGNAL(activated(int)), this, SLOT(select_speed_profile(int)));

	cmb_mesh = new QComboBox(false, this, "Mesh Listing" );
	cmb_mesh->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cmb_mesh->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cmb_mesh->setSizeLimit(5);
	cmb_mesh->setMinimumHeight(minHeight1);
	cmb_mesh->insertItem("Adaptive Space Mesh (ASTFEM)", -1);
	cmb_mesh->insertItem("Claverie Fixed Mesh", -1);
	cmb_mesh->insertItem("Moving Hat Mesh", -1);
	cmb_mesh->insertItem("File: \"$ULTRASCAN/mesh.dat\"", -1);
	cmb_mesh->setCurrentItem((*simparams).mesh);
	connect(cmb_mesh, SIGNAL(activated(int)), this, SLOT(update_mesh(int)));

	cmb_moving = new QComboBox(false, this, "Mesh Listing" );
	cmb_moving->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cmb_moving->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cmb_moving->setSizeLimit(5);
	cmb_moving->setMinimumHeight(minHeight1);
	cmb_moving->insertItem("Constant Time Grid (Claverie/Acceleration)", -1);
	cmb_moving->insertItem("Moving Time Grid (ASTFEM/Moving Hat)", -1);
	cmb_moving->setCurrentItem((*simparams).moving_grid);
	connect(cmb_moving, SIGNAL(activated(int)), this, SLOT(update_moving(int)));

	for (unsigned int i=0; i<(*simparams).speed_step.size(); i++)
	{
		backup_simparams.speed_step[i].duration_hours = (*simparams).speed_step[i].duration_hours;
		backup_simparams.speed_step[i].duration_minutes = (*simparams).speed_step[i].duration_minutes;
		backup_simparams.speed_step[i].delay_hours = (*simparams).speed_step[i].delay_hours;
		backup_simparams.speed_step[i].delay_minutes = (*simparams).speed_step[i].delay_minutes;
		backup_simparams.speed_step[i].rotorspeed = (*simparams).speed_step[i].rotorspeed;
		backup_simparams.speed_step[i].scans = (*simparams).speed_step[i].scans;
		backup_simparams.speed_step[i].acceleration = (*simparams).speed_step[i].acceleration;
		backup_simparams.speed_step[i].acceleration_flag = (*simparams).speed_step[i].acceleration_flag;
	}
	backup_simparams.simpoints = (*simparams).simpoints;
	backup_simparams.radial_resolution = (*simparams).radial_resolution;
	backup_simparams.meniscus = (*simparams).meniscus;
	backup_simparams.bottom = (*simparams).bottom;
	backup_simparams.rnoise = (*simparams).rnoise;
	backup_simparams.inoise = (*simparams).inoise;
	backup_simparams.rinoise = (*simparams).rinoise;

	lbl_title = new QLabel(tr("Simulation Run Parameter Setup:"), this);
	lbl_title->setAlignment(AlignHCenter|AlignVCenter);
	lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));
	lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_title->setMinimumHeight(minHeight2);
	lbl_title->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));

	cb_standard = new QCheckBox(tr(" Standard Centerpiece"), this);
	cb_standard->setMinimumHeight(minHeight1);
	cb_standard->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_standard->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cb_band = new QCheckBox(tr(" Band-forming Centerpiece"), this);
	cb_band->setMinimumHeight(minHeight1);
	cb_band->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_band->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	bg_centerpiece_selection = new QButtonGroup(2, Qt::Horizontal);
	bg_centerpiece_selection->setExclusive(true);
	connect(bg_centerpiece_selection, SIGNAL(clicked(int)), this, SLOT(select_centerpiece(int)));
	bg_centerpiece_selection->insert(cb_standard, 0);
	bg_centerpiece_selection->insert(cb_band, 1);
	if ((*simparams).band_forming)
	{
		bg_centerpiece_selection->setButton(1);
	}
	else
	{
		bg_centerpiece_selection->setButton(0);
	}

	lbl_lamella = new QLabel(tr(" Band loading volume (ml):"), this);
	lbl_lamella->setAlignment(AlignLeft|AlignVCenter);
	lbl_lamella->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_lamella->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_lamella->setMinimumHeight(minHeight1);

	cnt_lamella = new QwtCounter(this);
	cnt_lamella->setRange(0.001, 0.1, 0.0001);
	cnt_lamella->setNumButtons(3);
	cnt_lamella->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_lamella->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_lamella->setValue((*simparams).band_volume);
	cnt_lamella->setMinimumHeight(minHeight1);
	if ((*simparams).band_forming)
	{
		cnt_lamella->setEnabled(true);
	}
	else
	{
		cnt_lamella->setEnabled(false);
	}
	connect(cnt_lamella, SIGNAL(valueChanged(double)), SLOT(select_lamella(double)));

	lbl_number_of_speeds = new QLabel(tr(" Number of Speed Profiles:"), this);
	lbl_number_of_speeds->setAlignment(AlignLeft|AlignVCenter);
	lbl_number_of_speeds->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_number_of_speeds->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_number_of_speeds->setMinimumHeight(minHeight1);

	cnt_number_of_speeds = new QwtCounter(this);
	cnt_number_of_speeds->setRange(1, 100, 1);
	cnt_number_of_speeds->setNumButtons(2);
	cnt_number_of_speeds->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_number_of_speeds->setValue((*simparams).speed_step.size());
	cnt_number_of_speeds->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_number_of_speeds->setMinimumHeight(minHeight1);
	connect(cnt_number_of_speeds, SIGNAL(valueChanged(double)), SLOT(update_number_of_speeds(double)));

	lbl_selected_speed = new QLabel(tr(" Select a Speed Profile:"), this);
	lbl_selected_speed->setAlignment(AlignLeft|AlignVCenter);
	lbl_selected_speed->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_selected_speed->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_selected_speed->setMinimumHeight(minHeight1);

	cnt_selected_speed = new QwtCounter(this);
	cnt_selected_speed->setRange(1, (*simparams).speed_step.size(), 1);
	cnt_selected_speed->setNumButtons(2);
	cnt_selected_speed->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_selected_speed->setValue(current_speed_step);
	cnt_selected_speed->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_selected_speed->setMinimumHeight(minHeight1);
	connect(cnt_selected_speed, SIGNAL(valueChanged(double)), SLOT(select_speed_profile(double)));

	lbl_duration_hours = new QLabel(tr(" Length of Experiment (Hours):"), this);
	lbl_duration_hours->setAlignment(AlignLeft|AlignVCenter);
	lbl_duration_hours->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_duration_hours->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_duration_hours->setMinimumHeight(minHeight1);

	cnt_duration_hours = new QwtCounter(this);
	cnt_duration_hours->setRange(0, 5000, 1);
	cnt_duration_hours->setNumButtons(3);
	cnt_duration_hours->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_duration_hours->setValue((*simparams).speed_step[0].duration_hours);
	cnt_duration_hours->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_duration_hours->setMinimumHeight(minHeight1);
	connect(cnt_duration_hours, SIGNAL(valueChanged(double)), SLOT(update_duration_hours(double)));

	lbl_duration_minutes = new QLabel(tr(" Length of Experiment (Minutes):"), this);
	lbl_duration_minutes->setAlignment(AlignLeft|AlignVCenter);
	lbl_duration_minutes->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_duration_minutes->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_duration_minutes->setMinimumHeight(minHeight1);

	cnt_duration_minutes= new QwtCounter(this);
	cnt_duration_minutes->setRange(0, 60, 1);
	cnt_duration_minutes->setNumButtons(3);
	cnt_duration_minutes->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_duration_minutes->setValue((*simparams).speed_step[0].duration_minutes);
	cnt_duration_minutes->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_duration_minutes->setMinimumHeight(minHeight1);
	connect(cnt_duration_minutes, SIGNAL(valueChanged(double)), SLOT(update_duration_minutes(double)));

	lbl_delay_hours = new QLabel(tr(" Time Delay for Scans (Hours):"), this);
	lbl_delay_hours->setAlignment(AlignLeft|AlignVCenter);
	lbl_delay_hours->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_delay_hours->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_delay_hours->setMinimumHeight(minHeight1);

	cnt_delay_hours= new QwtCounter(this);
	cnt_delay_hours->setRange(0, 5000, 1);
	cnt_delay_hours->setValue((*simparams).speed_step[0].delay_hours);
	cnt_delay_hours->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_delay_hours->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_delay_hours->setNumButtons(3);
	cnt_delay_hours->setMinimumHeight(minHeight1);
	connect(cnt_delay_hours, SIGNAL(valueChanged(double)), SLOT(update_delay_hours(double)));

	lbl_delay_minutes = new QLabel(tr(" Time Delay for Scans (Minutes):"), this);
	lbl_delay_minutes->setAlignment(AlignLeft|AlignVCenter);
	lbl_delay_minutes->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_delay_minutes->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_delay_minutes->setMinimumHeight(minHeight1);

	cnt_delay_minutes= new QwtCounter(this);
	cnt_delay_minutes->setRange(0, 60, 1);
	cnt_delay_minutes->setNumButtons(3);
	cnt_delay_minutes->setValue((*simparams).speed_step[0].delay_minutes);
	cnt_delay_minutes->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_delay_minutes->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_delay_minutes->setMinimumHeight(minHeight1);
	connect(cnt_delay_minutes, SIGNAL(valueChanged(double)), SLOT(update_delay_minutes(double)));

	lbl_rotorspeed = new QLabel(tr(" Rotor Speed (rpm):"), this);
	lbl_rotorspeed->setAlignment(AlignLeft|AlignVCenter);
	lbl_rotorspeed->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_rotorspeed->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_rotorspeed->setMinimumHeight(minHeight1);

	cnt_rotorspeed= new QwtCounter(this);
	cnt_rotorspeed->setNumButtons(3);
	cnt_rotorspeed->setRange(1000, 100000, 100);
	cnt_rotorspeed->setValue((*simparams).speed_step[0].rotorspeed);
	cnt_rotorspeed->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_rotorspeed->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_rotorspeed->setMinimumHeight(minHeight1);
	connect(cnt_rotorspeed, SIGNAL(valueChanged(double)), SLOT(update_rotorspeed(double)));

	lbl_acceleration = new QLabel(tr(" Acceleration Profile (rpm/sec):"), this);
	lbl_acceleration->setAlignment(AlignLeft|AlignVCenter);
	lbl_acceleration->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_acceleration->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_acceleration->setMinimumHeight(minHeight1);

	cnt_acceleration= new QwtCounter(this);
	cnt_acceleration->setNumButtons(3);
	cnt_acceleration->setRange(5, 400, 1);
	cnt_acceleration->setEnabled((*simparams).speed_step[0].acceleration_flag);
	cnt_acceleration->setValue((double)(*simparams).speed_step[0].acceleration);
	cnt_acceleration->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_acceleration->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_acceleration->setMinimumHeight(minHeight1);
	connect(cnt_acceleration, SIGNAL(valueChanged(double)), SLOT(update_acceleration(double)));

	lbl_simpoints = new QLabel(tr(" Radial Discretization (points):"), this);
	lbl_simpoints->setAlignment(AlignLeft|AlignVCenter);
	lbl_simpoints->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_simpoints->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_simpoints->setMinimumHeight(minHeight1);

	cnt_simpoints= new QwtCounter(this);
	cnt_simpoints->setNumButtons(3);
	cnt_simpoints->setRange(50, 5000, 10);
	cnt_simpoints->setValue((double)(*simparams).simpoints);
	cnt_simpoints->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_simpoints->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_simpoints->setMinimumHeight(minHeight1);
	connect(cnt_simpoints, SIGNAL(valueChanged(double)), SLOT(update_simpoints(double)));

	lbl_radial_resolution = new QLabel(tr(" Radial Resolution (cm):"), this);
	lbl_radial_resolution->setAlignment(AlignLeft|AlignVCenter);
	lbl_radial_resolution->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_radial_resolution->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_radial_resolution->setMinimumHeight(minHeight1);

	cnt_radial_resolution= new QwtCounter(this);
	cnt_radial_resolution->setNumButtons(3);
	cnt_radial_resolution->setRange(1e-5, 0.01, 1e-5);
	cnt_radial_resolution->setValue((double)(*simparams).radial_resolution);
	cnt_radial_resolution->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_radial_resolution->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_radial_resolution->setMinimumHeight(minHeight1);
	connect(cnt_radial_resolution, SIGNAL(valueChanged(double)), SLOT(update_radial_resolution(double)));

	lbl_acceleration_flag = new QLabel(tr(" Simulate Rotor Acceleration:"), this);
	lbl_acceleration_flag->setAlignment(AlignLeft|AlignVCenter);
	lbl_acceleration_flag->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_acceleration_flag->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_acceleration_flag->setMinimumHeight(minHeight1);

	cb_acceleration_flag = new QCheckBox(" (Check to enable)", this);
	cb_acceleration_flag->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_acceleration_flag->setChecked((*simparams).speed_step[0].acceleration_flag);
	cb_acceleration_flag->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_acceleration_flag->setMinimumHeight(minHeight1);
	connect(cb_acceleration_flag, SIGNAL(clicked()), SLOT(acceleration_flag()));

	lbl_meniscus = new QLabel(tr(" Meniscus Position (cm):"), this);
	lbl_meniscus->setAlignment(AlignLeft|AlignVCenter);
	lbl_meniscus->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_meniscus->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_meniscus->setMinimumHeight(minHeight1);

	cnt_meniscus= new QwtCounter(this);
	cnt_meniscus->setNumButtons(3);
	cnt_meniscus->setRange(5.8, 7.2, 0.001);
	cnt_meniscus->setValue((*simparams).meniscus);
	cnt_meniscus->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_meniscus->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_meniscus->setMinimumHeight(minHeight1);
	connect(cnt_meniscus, SIGNAL(valueChanged(double)), SLOT(update_meniscus(double)));

	lbl_bottom = new QLabel(tr(" Bottom of Cell Position (cm):"), this);
	lbl_bottom->setAlignment(AlignLeft|AlignVCenter);
	lbl_bottom->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_bottom->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_bottom->setMinimumHeight(minHeight1);

	cnt_bottom= new QwtCounter(this);
	cnt_bottom->setNumButtons(3);
	cnt_bottom->setRange(5.8, 7.5, 0.001);
	cnt_bottom->setValue((*simparams).bottom);
	cnt_bottom->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_bottom->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_bottom->setMinimumHeight(minHeight1);
	connect(cnt_bottom, SIGNAL(valueChanged(double)), SLOT(update_bottom(double)));

	lbl_scans = new QLabel(tr(" Scans to be saved:"), this);
	lbl_scans->setAlignment(AlignLeft|AlignVCenter);
	lbl_scans->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_scans->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_scans->setMinimumHeight(minHeight1);

	cnt_scans= new QwtCounter(this);
	cnt_scans->setNumButtons(3);
	cnt_scans->setRange(2,1000,1);
	cnt_scans->setValue((*simparams).speed_step[0].scans);
	cnt_scans->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_scans->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_scans->setMinimumHeight(minHeight1);
	connect(cnt_scans, SIGNAL(valueChanged(double)), SLOT(update_scans(double)));

	lbl_rnoise = new QLabel(tr(" Random Noise (% Conc.):"), this);
	lbl_rnoise->setAlignment(AlignLeft|AlignVCenter);
	lbl_rnoise->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_rnoise->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_rnoise->setMinimumHeight(minHeight1);

	cnt_rnoise= new QwtCounter(this);
	cnt_rnoise->setNumButtons(3);
	cnt_rnoise->setRange(0,10,0.01);
	cnt_rnoise->setValue((*simparams).rnoise);
	cnt_rnoise->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_rnoise->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_rnoise->setMinimumHeight(minHeight1);
	connect(cnt_rnoise, SIGNAL(valueChanged(double)), SLOT(update_rnoise(double)));

	lbl_inoise = new QLabel(tr(" Time Invariant Noise (% Conc.):"), this);
	lbl_inoise->setAlignment(AlignLeft|AlignVCenter);
	lbl_inoise->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_inoise->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_inoise->setMinimumHeight(minHeight1);

	cnt_inoise= new QwtCounter(this);
	cnt_inoise->setNumButtons(3);
	cnt_inoise->setRange(0,10,0.01);
	cnt_inoise->setValue((*simparams).inoise);
	cnt_inoise->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_inoise->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_inoise->setMinimumHeight(minHeight1);
	connect(cnt_inoise, SIGNAL(valueChanged(double)), SLOT(update_inoise(double)));

	lbl_rinoise = new QLabel(tr(" Radially Invariant Noise (% Conc.):"), this);
	lbl_rinoise->setAlignment(AlignLeft|AlignVCenter);
	lbl_rinoise->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_rinoise->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_rinoise->setMinimumHeight(minHeight1);

	cnt_rinoise= new QwtCounter(this);
	cnt_rinoise->setNumButtons(3);
	cnt_rinoise->setRange(0,10,0.01);
	cnt_rinoise->setValue((*simparams).rinoise);
	cnt_rinoise->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_rinoise->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cnt_rinoise->setMinimumHeight(minHeight1);
	connect(cnt_rinoise, SIGNAL(valueChanged(double)), SLOT(update_rinoise(double)));

	pb_ok = new QPushButton( this );
	pb_ok->setText(tr("OK"));
	pb_ok->setAutoDefault(false);
	pb_ok->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_ok->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_ok->setMinimumHeight(minHeight1);
	connect(pb_ok, SIGNAL(clicked()), SLOT(check_params()));

	pb_cancel = new QPushButton( tr("Cancel"), this );
	pb_cancel->setAutoDefault(false);
	pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_cancel->setMinimumHeight(minHeight1);
	connect(pb_cancel, SIGNAL(clicked()), SLOT(revert()) );

	pb_help = new QPushButton( tr("Help"), this );
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_help->setMinimumHeight(minHeight1);
	connect(pb_help, SIGNAL(clicked()), SLOT(help()) );

	pb_load = new QPushButton( tr("Load Profile"), this );
	pb_load->setAutoDefault(false);
	pb_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_load->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_load->setMinimumHeight(minHeight1);
	connect(pb_load, SIGNAL(clicked()), SLOT(load()) );

	pb_save = new QPushButton( tr("Save"), this );
	pb_save->setAutoDefault(false);
	pb_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_save->setMinimumHeight(minHeight1);
	connect(pb_save, SIGNAL(clicked()), SLOT(save()) );

	setupGUI();

	global_Xpos += 30;
	global_Ypos += 30;
	move(global_Xpos, global_Ypos);
	select_speed_profile((int) 0);
}

US_SimulationParameters::~US_SimulationParameters()
{
}

void US_SimulationParameters::closeEvent(QCloseEvent *e)
{
	global_Xpos -= 30;
	global_Ypos -= 30;
	e->accept();
}

void US_SimulationParameters::setupGUI()
{
	QGridLayout *grid = new QGridLayout(this, 15, 6, 4, 2);
	unsigned int j=0;
	grid->addMultiCellWidget(lbl_title, j, j, 0, 5, 0);
	j++;
	grid->addWidget(lbl_number_of_speeds, j, 0, 0);
	grid->addWidget(cnt_number_of_speeds, j, 1, 0);
	grid->addMultiCellWidget(cb_standard, j, j, 2, 3, 0);
	grid->addMultiCellWidget(cb_band, j, j, 4, 5, 0);
	j++;
	grid->addMultiCellWidget(cmb_speeds, j, j, 0, 1, 0);
	grid->addMultiCellWidget(lbl_lamella, j, j, 2, 3, 0);
	grid->addMultiCellWidget(cnt_lamella, j, j, 4, 5, 0);
	j++;
	grid->addWidget(lbl_duration_hours, j, 0, 0);
	grid->addWidget(cnt_duration_hours, j, 1, 0);
	grid->addMultiCellWidget(lbl_meniscus, j, j, 2, 3, 0);
	grid->addMultiCellWidget(cnt_meniscus, j, j, 4, 5, 0);
	j++;
	grid->addWidget(lbl_duration_minutes, j, 0, 0);
	grid->addWidget(cnt_duration_minutes, j, 1, 0);
	grid->addMultiCellWidget(lbl_bottom, j, j, 2, 3, 0);
	grid->addMultiCellWidget(cnt_bottom, j, j, 4, 5, 0);
	j++;
	grid->addWidget(lbl_delay_hours, j, 0, 0);
	grid->addWidget(cnt_delay_hours, j, 1, 0);
	grid->addMultiCellWidget(lbl_simpoints, j, j, 2, 3, 0);
	grid->addMultiCellWidget(cnt_simpoints, j, j, 4, 5, 0);
	j++;
	grid->addWidget(lbl_delay_minutes, j, 0, 0);
	grid->addWidget(cnt_delay_minutes, j, 1, 0);
	grid->addMultiCellWidget(lbl_radial_resolution, j, j, 2, 3, 0);
	grid->addMultiCellWidget(cnt_radial_resolution, j, j, 4, 5, 0);
	j++;
	grid->addWidget(lbl_rotorspeed, j, 0, 0);
	grid->addWidget(cnt_rotorspeed, j, 1, 0);
	grid->addMultiCellWidget(lbl_rnoise, j, j, 2, 3, 0);
	grid->addMultiCellWidget(cnt_rnoise, j, j, 4, 5, 0);
	j++;
	grid->addWidget(lbl_acceleration_flag, j, 0, 0);
	grid->addWidget(cb_acceleration_flag, j, 1, 0);
	grid->addMultiCellWidget(lbl_inoise, j, j, 2, 3, 0);
	grid->addMultiCellWidget(cnt_inoise, j, j, 4, 5, 0);
	j++;
	grid->addWidget(lbl_acceleration, j, 0, 0);
	grid->addWidget(cnt_acceleration, j, 1, 0);
	grid->addMultiCellWidget(lbl_rinoise, j, j, 2, 3, 0);
	grid->addMultiCellWidget(cnt_rinoise, j, j, 4, 5, 0);
	j++;
	grid->addWidget(lbl_scans, j, 0, 0);
	grid->addWidget(cnt_scans, j, 1, 0);
	grid->addMultiCellWidget(cmb_mesh, j, j, 2, 5, 0);
	j++;
	grid->addWidget(lbl_selected_speed, j, 0, 0);
	grid->addWidget(cnt_selected_speed, j, 1, 0);
	grid->addMultiCellWidget(cmb_moving, j, j, 2, 5, 0);
	j++;
	grid->addWidget(pb_load, j, 0, 0);
	grid->addWidget(pb_save, j, 1, 0);
	grid->addWidget(pb_help, j, 2, 0);
	grid->addWidget(pb_cancel, j, 3, 0);
	grid->addMultiCellWidget(pb_ok, j, j, 4, 5, 0);
}

void US_SimulationParameters::help()
{
	US_Help *online_help;
	online_help = new US_Help(this);
	online_help->show_help("manual/simulation_parameters.html");
}

void US_SimulationParameters::check_params()
{
	QString str;
	if ((*simparams).speed_step.size() > 1)
	{
		for (unsigned int i=1; i<(*simparams).speed_step.size(); i++)
		{
			str.sprintf("speed profiles %d and %d\n", i, i+1);
			if(abs((int)((*simparams).speed_step[i].rotorspeed - (*simparams).speed_step[i-1].rotorspeed))
						< (int) (*simparams).speed_step[i].acceleration)
			{
				QMessageBox::query( tr("Warning"), tr("Attention:\nThe difference between the rotor speeds of " + str +
						"is smaller than the acceleration rate, please decrease the acceleration rate!"));
				return;
			}
		}
	}
	accept();
}

void US_SimulationParameters::save()
{
	QFileDialog fd;
	QString fn = fd.getSaveFileName(USglobal->config_list.result_dir, "*.simulation_parameters", 0);
	int k;
	if ( !fn.isEmpty() )
	{
		k = fn.findRev(".", -1, false);
		if (k != -1) 	//if an extension was given, strip it.
		{
			fn.truncate(k);
		}
		fn.append(".simulation_parameters");
		save(fn);		// the user gave a file name
	}
}

void US_SimulationParameters::save(const QString &filename)
{
	QString str = QString(filename);
	QFile f(str);
	if (f.exists())
	{
		if(!QMessageBox::query( tr("Warning"), tr("Attention:\nThis file exists already!\n\nDo you want to overwrite it?"), tr("Yes"), tr("No")))
		{
			f.close();
			return;
		}
	}
	if (f.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&f);
		ts << (*simparams).speed_step.size() << "\t\t# Number of speed step profiles" << "\n";
		for (unsigned int i=0; i<(*simparams).speed_step.size(); i++)
		{
			ts << (*simparams).speed_step[i].duration_hours << str.sprintf("\t\t# run duration hours for profile %d\n", i+1);
			ts << (*simparams).speed_step[i].duration_minutes << str.sprintf("\t\t# run duration minutes for profile %d\n", i+1);
			ts << (*simparams).speed_step[i].delay_hours << str.sprintf("\t\t# run delay hours for profile %d\n", i+1);
			ts << (*simparams).speed_step[i].delay_minutes << str.sprintf("\t\t# run delay minutes for profile %d\n", i+1);
			ts << (*simparams).speed_step[i].rotorspeed << str.sprintf("\t\t# rotor speed for profile %d\n", i+1);
			ts << (*simparams).speed_step[i].acceleration << str.sprintf("\t\t# acceleration profile in revs/sec for profile %d\n", i+1);
			ts << (int) (*simparams).speed_step[i].acceleration_flag << str.sprintf("\t\t# flag for checking if rotor acceleration is used for profile %d\n", i+1);
			ts << (*simparams).speed_step[i].scans << str.sprintf("\t\t# number of scans to save for profile %d\n", i+1);
		}
		ts << (*simparams).simpoints << "\t\t# radial discretization simulation points" << "\n";
		ts << (*simparams).radial_resolution << "\t\t# radial resolution (cm)" << "\n";
		ts << (*simparams).meniscus << "\t\t# meniscus position (cm)" << "\n";
		ts << (*simparams).bottom << "\t\t# bottom of cell position (cm)" << "\n";
		ts << (*simparams).rnoise << "\t\t# random noise (in percent OD)" << "\n";
		ts << (*simparams).inoise << "\t\t# time invariant systematic noise (in percent OD)" << "\n";
		ts << (*simparams).rinoise << "\t\t# radial invariant systematic noise (in percent OD)" << "\n";
		ts << (*simparams).mesh << "\t\t# Selected simulation mesh" << "\n";
		ts << (*simparams).moving_grid << "\t\t# moving time grid (0 = Astfem, 1 = fixed)" << "\n";
		ts << (*simparams).rotor << "\t\t# Rotor serial number" << "\n";
		if ((*simparams).band_forming)
		{
			ts << "1\t\t# Band-forming centerpiece is used\n";
			ts << (*simparams).band_volume << "\t\t# Band loading volume in ml" << "\n";
		}
		else
		{
			ts << "0\t\t# Standard centerpiece is used\n";
		}
		QMessageBox::information(this, tr("UltraScan Information"),
		tr("Please note:\n\nThe Simulation Profile was successfully saved to:\n\n" + filename),
		QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		f.close();
	}
}

void US_SimulationParameters::load()
{
	QString fn = QFileDialog::getOpenFileName(USglobal->config_list.result_dir, "*.simulation_parameters", 0);
	if ( !fn.isEmpty() )
	{
		load(fn);
	}
}

void US_SimulationParameters::load(const QString &fileName)
{
	QFile f(fileName);
	int ival;
	if (f.open(IO_ReadOnly))
	{
		QTextStream ts(&f);
		if (!ts.atEnd())
		{
			unsigned int ival;
			ts >> ival;
			(*simparams).speed_step.resize(ival);
			ts.readLine();
			cnt_number_of_speeds->setValue((*simparams).speed_step.size());
		}
		else
		{
			f.close();
			printError(0);
			return;
		}
		cmb_speeds->clear();
		for (unsigned int i=0; i<(*simparams).speed_step.size(); i++)
		{
			QTextStream ts(&f);
			if (!ts.atEnd())
			{
				ts >> (*simparams).speed_step[i].duration_hours;
				ts.readLine();
			}
			else
			{
				f.close();
				printError(0);
				return;
			}
			if (!ts.atEnd())
			{
				ts >> (*simparams).speed_step[i].duration_minutes;
				ts.readLine();
			}
			else
			{
				f.close();
				printError(0);
				return;
			}
			if (!ts.atEnd())
			{
				ts >> (*simparams).speed_step[i].delay_hours;
				ts.readLine();
			}
			else
			{
				f.close();
				printError(0);
				return;
			}
			if (!ts.atEnd())
			{
				ts >> (*simparams).speed_step[i].delay_minutes;
				ts.readLine();
			}
			else
			{
				f.close();
				printError(0);
				return;
			}
			if (!ts.atEnd())
			{
				ts >> (*simparams).speed_step[i].rotorspeed;
				ts.readLine();
			}
			else
			{
				f.close();
				printError(0);
				return;
			}
			if (!ts.atEnd())
			{
				ts >> (*simparams).speed_step[i].acceleration;
				ts.readLine();
			}
			else
			{
				f.close();
				printError(0);
				return;
			}
			if (!ts.atEnd())
			{
				int ival;
				ts >> ival;
				(*simparams).speed_step[i].acceleration_flag = ival;
				ts.readLine();
			}
			else
			{
				f.close();
				printError(0);
				return;
			}
			if (!ts.atEnd())
			{
				ts >> (*simparams).speed_step[i].scans;
				ts.readLine();
			}
			else
			{
				f.close();
				printError(0);
				return;
			}
			QString str;
			str.sprintf("Speed Profile %d: %d hr %d min, %d rpm", i+1,
			(*simparams).speed_step[i].duration_hours,
			(*simparams).speed_step[i].duration_minutes,
			(*simparams).speed_step[i].rotorspeed);
			cmb_speeds->insertItem(str, -1);
		}
// initialize all counters with the first speed profile:
		cnt_duration_hours->setValue((*simparams).speed_step[0].duration_hours);
		cnt_duration_minutes->setValue((*simparams).speed_step[0].duration_minutes);
		cnt_delay_hours->setValue((*simparams).speed_step[0].delay_hours);
		cnt_delay_minutes->setValue((*simparams).speed_step[0].delay_minutes);
		cnt_rotorspeed->setValue((*simparams).speed_step[0].rotorspeed);
		cnt_acceleration->setValue((double)(*simparams).speed_step[0].acceleration);
		cb_acceleration_flag->setChecked((*simparams).speed_step[0].acceleration_flag);
		if (cb_acceleration_flag->isChecked())
		{
			cnt_acceleration->setEnabled(true);
		}
		else
		{
			cnt_acceleration->setEnabled(false);
		}
		cnt_scans->setValue((*simparams).speed_step[0].scans);
		if (!ts.atEnd())
		{
			ts >> (*simparams).simpoints;
			ts.readLine();
			cnt_simpoints->setValue((*simparams).simpoints);
		}
		else
		{
			f.close();
			printError(0);
			return;
		}
		if (!ts.atEnd())
		{
			ts >> (*simparams).radial_resolution;
			ts.readLine();
			cnt_radial_resolution->setValue((*simparams).radial_resolution);
		}
		else
		{
			f.close();
			printError(0);
			return;
		}
		if (!ts.atEnd())
		{
			ts >> (*simparams).meniscus;
			ts.readLine();
			cnt_meniscus->setValue((*simparams).meniscus);
		}
		else
		{
			f.close();
			printError(0);
			return;
		}
		if (!ts.atEnd())
		{
			ts >> (*simparams).bottom;
			ts.readLine();
			cnt_bottom->setValue((*simparams).bottom);
		}
		else
		{
			f.close();
			printError(0);
			return;
		}
		if (!ts.atEnd())
		{
			ts >> (*simparams).rnoise;
			ts.readLine();
			cnt_rnoise->setValue((*simparams).rnoise);
		}
		else
		{
			f.close();
			printError(0);
			return;
		}
		if (!ts.atEnd())
		{
			ts >> (*simparams).inoise;
			ts.readLine();
			cnt_inoise->setValue((*simparams).inoise);
		}
		else
		{
			f.close();
			printError(0);
			return;
		}
		if (!ts.atEnd())
		{
			ts >> (*simparams).rinoise;
			ts.readLine();
			cnt_rinoise->setValue((*simparams).rinoise);
		}
		else
		{
			f.close();
			printError(0);
			return;
		}
		if (!ts.atEnd())
		{
			ts >> (*simparams).mesh;
			cmb_mesh->setCurrentItem((*simparams).mesh);
		}
		else
		{
			printError(0);
			return;
		}
		if (!ts.atEnd())
		{
			ts >> (*simparams).moving_grid;
			cmb_moving->setCurrentItem((*simparams).moving_grid);
		}
		else
		{
			printError(0);
			return;
		}
		if (!ts.atEnd())
		{
			ts >> (*simparams).rotor;
		}
		else
		{
			printError(0);
			return;
		}
		if (!ts.atEnd())
		{
			ts >> ival;
			if (ival == 1)
			{
				(*simparams).band_forming = true;
				bg_centerpiece_selection->setButton(1);
				if (!ts.atEnd())
				{
					ts >> (*simparams).band_volume;
				}
				else
				{
					printError(0);
					return;
				}
			}
			else
			{
				(*simparams).band_forming = false;
				bg_centerpiece_selection->setButton(0);
			}
		}
		else
		{
			printError(0);
			return;
		}
		f.close();
		QMessageBox::information(this, tr("UltraScan Information"),
		tr("Please note:\n\nThe Simulation Profile:\n\n" + fileName + "\n\nwas successfully loaded."),
		QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
	}
	else
	{
		printError(1);
	}
}

void US_SimulationParameters::update_duration_hours(double temp_var)
{
	(*simparams).speed_step[current_speed_step].duration_hours = (unsigned int) temp_var;
	update_combobox();
}

void US_SimulationParameters::update_duration_minutes(double temp_var)
{
	(*simparams).speed_step[current_speed_step].duration_minutes = (unsigned int) temp_var;
	update_combobox();
}

void US_SimulationParameters::update_delay_hours(double temp_var)
{
	(*simparams).speed_step[current_speed_step].delay_hours = (unsigned int) temp_var;
}

void US_SimulationParameters::update_delay_minutes(double temp_var)
{
	(*simparams).speed_step[current_speed_step].delay_minutes = (float) temp_var;
}

void US_SimulationParameters::update_rotorspeed(double temp_var)
{
	(*simparams).speed_step[current_speed_step].rotorspeed = (unsigned long) temp_var;
	update_combobox();
	if (cb_acceleration_flag->isChecked())
	{
		// if there is acceleration we need to set the scan delay
		// minimum to the time it takes to accelerate:
		check_delay();
	}
}

void US_SimulationParameters::check_delay()
{
	unsigned int i, lower_limit;
	vector <unsigned int> hours;
	vector <float> minutes;
	vector <unsigned int> speed;
	speed.clear();
	speed.push_back(0);
	hours.resize((*simparams).speed_step.size());
	minutes.resize((*simparams).speed_step.size());
	for (i=0; i<(*simparams).speed_step.size(); i++)
	{
		speed.push_back((*simparams).speed_step[i].rotorspeed);
		lower_limit = (unsigned int)(1 + (abs((int)(speed[i+1] - speed[i])) + 1)/(*simparams).speed_step[i].acceleration);
		hours[i] = (unsigned int) lower_limit/3600;
		minutes[i] = (float) (1.0/60.0 + (lower_limit - hours[i] * 3600)/60.0);
		cnt_delay_minutes->setRange((double) minutes[i], 60, 0.1);
		cnt_delay_hours->setRange((double) hours[i], 5000, 1);
		//cout << "Profile: " << i+1 << ", speed1: " << speed[i] << ", speed2: " << speed[i+1] << ", lower limit: " << lower_limit << ", hours: " << hours << ", mins: " << minutes << endl;
	}
	if ((*simparams).speed_step[current_speed_step].delay_minutes < minutes[current_speed_step])
	{
		(*simparams).speed_step[current_speed_step].delay_minutes = minutes[current_speed_step];
		cnt_delay_minutes->setValue((double) minutes[current_speed_step]);
	}
	if ((*simparams).speed_step[current_speed_step].delay_hours < hours[current_speed_step])
	{
		(*simparams).speed_step[current_speed_step].delay_hours = hours[current_speed_step];
		cnt_delay_hours->setValue((double) hours[current_speed_step]);
	}
}

void US_SimulationParameters::update_acceleration(double temp_var)
{
	(*simparams).speed_step[current_speed_step].acceleration = (unsigned long) temp_var;
	if (cb_acceleration_flag->isChecked())
	{
		// if there is acceleration we need to set the scan delay
		// minimum to the time it takes to accelerate:
		check_delay();
	}
}

void US_SimulationParameters::update_simpoints(double temp_var)
{
	(*simparams).simpoints = (unsigned int) temp_var;
}

void US_SimulationParameters::update_radial_resolution(double temp_var)
{
	(*simparams).radial_resolution = (float) temp_var;
}

void US_SimulationParameters::update_meniscus(double temp_var)
{
	(*simparams).meniscus=(float) temp_var;
}

void US_SimulationParameters::update_bottom(double temp_var)
{
	(*simparams).bottom = (float) temp_var;
}

void US_SimulationParameters::update_scans(double temp_var)
{
	(*simparams).speed_step[current_speed_step].scans = (unsigned int) temp_var;
}

void US_SimulationParameters::update_rnoise(double temp_var)
{
	(*simparams).rnoise = (float) temp_var;
}

void US_SimulationParameters::update_inoise(double temp_var)
{
	(*simparams).inoise = (float) temp_var;
}

void US_SimulationParameters::update_rinoise(double temp_var)
{
	(*simparams).rinoise = (float) temp_var;
}

void US_SimulationParameters::select_speed_profile(double temp_var)
{
	int i = (int) (temp_var - 1);
	select_speed_profile(i);
}

void US_SimulationParameters::select_lamella(double temp_var)
{
	(*simparams).band_volume = (float) temp_var;
}

void US_SimulationParameters::select_speed_profile(int temp_var)
{
	current_speed_step = (unsigned int) temp_var;
	cnt_selected_speed->setValue(current_speed_step + 1);
	cmb_speeds->setCurrentItem(current_speed_step);
	cnt_duration_hours->setValue((*simparams).speed_step[current_speed_step].duration_hours);
	cnt_duration_minutes->setValue((*simparams).speed_step[current_speed_step].duration_minutes);
	cnt_delay_hours->setValue((*simparams).speed_step[current_speed_step].delay_hours);
	cnt_delay_minutes->setValue((*simparams).speed_step[current_speed_step].delay_minutes);
	cnt_rotorspeed->setValue((*simparams).speed_step[current_speed_step].rotorspeed);
	cnt_acceleration->setValue((double)(*simparams).speed_step[current_speed_step].acceleration);
	cb_acceleration_flag->setChecked((*simparams).speed_step[current_speed_step].acceleration_flag);
	cnt_scans->setValue((*simparams).speed_step[current_speed_step].scans);
	if (cb_acceleration_flag->isChecked())
	{
		cnt_acceleration->setEnabled(true);
		// if there is acceleration we need to set the scan delay
		// minimum to the time it takes to accelerate:
		check_delay();
	}
	else
	{
		cnt_acceleration->setEnabled(false);
	}
}

void US_SimulationParameters::update_number_of_speeds(double temp_var)
{
	unsigned int old_size = (*simparams).speed_step.size();
	(*simparams).speed_step.resize((unsigned int) temp_var);
	cnt_selected_speed->setRange(1, (*simparams).speed_step.size(), 1);
// only initialize the new elements, leave the previously assigned elements alone.
// New elements simply get copies of the last old element
// if old_size > new_size then we won't go through this loop and simply truncate the list
	for (unsigned int i=old_size; i<(*simparams).speed_step.size(); i++)
	{
		(*simparams).speed_step[i].duration_hours = (*simparams).speed_step[old_size - 1].duration_hours;
		(*simparams).speed_step[i].duration_minutes = (*simparams).speed_step[old_size - 1].duration_minutes;
		(*simparams).speed_step[i].delay_hours = (*simparams).speed_step[old_size - 1].delay_hours;
		(*simparams).speed_step[i].delay_minutes = (*simparams).speed_step[old_size - 1].delay_minutes;
		(*simparams).speed_step[i].rotorspeed = (*simparams).speed_step[old_size - 1].rotorspeed;
		(*simparams).speed_step[i].scans = (*simparams).speed_step[old_size - 1].scans;
		(*simparams).speed_step[i].acceleration = (*simparams).speed_step[old_size - 1].acceleration;
		(*simparams).speed_step[i].acceleration_flag = (*simparams).speed_step[old_size - 1].acceleration_flag;
	}
	update_combobox();
}

void US_SimulationParameters::acceleration_flag()
{
	if (cb_acceleration_flag->isChecked())
	{
		(*simparams).speed_step[current_speed_step].acceleration_flag = true;
		cnt_acceleration->setEnabled(true);

		// if there is acceleration we need to set the scan delay
		// minimum to the time it takes to accelerate:
		check_delay();
	}
	else
	{
		(*simparams).speed_step[current_speed_step].acceleration_flag = false;
		cnt_acceleration->setEnabled(false);
	}
}

void US_SimulationParameters::printError(const int &ival)
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
			QMessageBox::warning(this, tr("UltraScan Warning"), tr("Please note:\n\nUltraScan could not open\nthe selected File!"),
			QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			break;
		}
		case 2:
		{
			QMessageBox::warning(this, tr("UltraScan Warning"),
			tr("Please note:\n\nUltraScan could not read\nthe selected Model File!"),
			QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		}
	}
}

void US_SimulationParameters::update_combobox()
{
	cmb_speeds->clear();
	for (unsigned int i=0; i<(*simparams).speed_step.size(); i++)
	{
		QString str;
		str.sprintf("Speed Profile %d: %d hr %d min, %d rpm", i+1, (*simparams).speed_step[i].duration_hours,
		(*simparams).speed_step[i].duration_minutes, (*simparams).speed_step[i].rotorspeed);
		cmb_speeds->insertItem(str, -1);
	}
	cmb_speeds->setCurrentItem(current_speed_step);
}

void US_SimulationParameters::update_moving(int val)
{
	(*simparams).moving_grid = val;
}

void US_SimulationParameters::update_mesh(int val)
{
// this routine makes sure that the first and last radius points match the meniscus and bottom respectively
	(*simparams).mesh = val;
	cnt_simpoints->setEnabled(true); // by default, the simpoints can be set by the user
	if ((*simparams).mesh == 4)
	{
		QMessageBox::information(this, tr("UltraScan Information"),
		tr("Please note:\n\nThe radial mesh file should have\n"
			"the following format:\n\n"
			"radius_value1\n"
			"radius_value2\n"
			"radius_value3\n"
			"etc...\n\n"
			"radius values smaller than the meniscus or\n"
			"larger than the bottom of the cell will be\n"
			"excluded from the concentration vector."),
		QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		QFile meshfile;
		meshfile.setName(USglobal->config_list.root_dir + "/mesh.dat");
		if (meshfile.open(IO_ReadOnly | IO_Translate))
		{
			QTextStream ts(&meshfile);
			(*simparams).mesh_radius.clear();
			double val;
			bool flag=false;
			while (!ts.atEnd())
			{
				ts >> val;
				if (val >= (*simparams).meniscus && val <= (*simparams).bottom) // ignore values outside the meniscus/bottom range
				{
					if(!flag)
					{
						if (val > (*simparams).meniscus)
						{
							(*simparams).mesh_radius.push_back((double)(*simparams).meniscus);
						}
						flag = true;
					}
					(*simparams).mesh_radius.push_back(val);
				}
			}
			if ((*simparams).mesh_radius[(*simparams).mesh_radius.size() - 1] < (*simparams).bottom)
			{
				(*simparams).mesh_radius.push_back((*simparams).bottom);
			}
			meshfile.close();
			(*simparams).simpoints = (*simparams).mesh_radius.size();
			cnt_simpoints->setValue((double)(*simparams).simpoints);
			cnt_simpoints->setEnabled(false); // can't change the simulation points after defining a mesh
//			for (unsigned int i=0; i<(*simparams).mesh_radius.size(); i++)
//			{
//				cout << i << ": " << (*simparams).mesh_radius[i] << endl;
//			}
		}
		else
		{
			(*simparams).mesh = 0; // set to default ASTFEM mesh
			cnt_simpoints->setEnabled(true); // by default, the simpoints can be set by the user
			QMessageBox::warning(this, tr("UltraScan Warning"),
			tr("Please note:\n\nUltraScan could not open the mesh file!\nThe file:\n\n" + USglobal->config_list.root_dir + "/mesh.dat\n\ncould not be opened."),
			QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		}
	}
}

void US_SimulationParameters::revert()
{
	(*simparams).speed_step.resize(backup_simparams.speed_step.size());
	for (unsigned int i=0; i<(*simparams).speed_step.size(); i++)
	{
		(*simparams).speed_step[i].duration_hours = backup_simparams.speed_step[i].duration_hours;
		(*simparams).speed_step[i].duration_minutes = backup_simparams.speed_step[i].duration_minutes;
		(*simparams).speed_step[i].delay_hours = backup_simparams.speed_step[i].delay_hours;
		(*simparams).speed_step[i].delay_minutes = backup_simparams.speed_step[i].delay_minutes;
		(*simparams).speed_step[i].rotorspeed = backup_simparams.speed_step[i].rotorspeed;
		(*simparams).speed_step[i].scans = backup_simparams.speed_step[i].scans;
		(*simparams).speed_step[i].acceleration = backup_simparams.speed_step[i].acceleration;
		(*simparams).speed_step[i].acceleration_flag = backup_simparams.speed_step[i].acceleration_flag;
	}
	(*simparams).simpoints = backup_simparams.simpoints;
	(*simparams).radial_resolution = backup_simparams.radial_resolution;
	(*simparams).meniscus = backup_simparams.meniscus;
	(*simparams).bottom = backup_simparams.bottom;
	(*simparams).rnoise = backup_simparams.rnoise;
	(*simparams).inoise = backup_simparams.inoise;
	(*simparams).rinoise = backup_simparams.rinoise;
	reject();
}

void US_SimulationParameters::select_centerpiece(int cp)
{
	if (cp == 0)
	{
		cnt_lamella->setEnabled(false);
		(*simparams).band_forming = false;
	}
	else
	{
		cnt_lamella->setEnabled(true);
		(*simparams).band_forming = true;
	}
}
