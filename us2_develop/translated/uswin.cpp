#include "../include/uswin.h"
#include <qtimer.h>
#include <qtranslator.h>
#include <qmotifstyle.h>

UsWin::UsWin(QWidget *parent, const char *name)
        : QWidget(parent, name)
{
	config_check();
	QTranslator translate(0);
	translate.load("/usr/local/src/ultrascan/ultrascan_de.qm");
	qApp->installTranslator(&translate);
	QMotifStyle *local_style = new QMotifStyle;


	int width=710, height=532;
//	setMinimumSize(width,height);
	setGeometry(50, 50, width, height);
	QTimer *splash_time = new QTimer(this);
	bigframe = new QLabel(this);
	bigframe->setStyle(local_style);
	bigframe->setFrameStyle(QFrame::Box | QFrame::Raised);
	bigframe->setPalette(QPalette(global_colors.cg_frame, global_colors.cg_frame, global_colors.cg_frame));
	smallframe = new QLabel(this);
	smallframe->setStyle(local_style);
	smallframe->setGeometry((unsigned int) ((width/2)-210) , 130, 460, 276);
	smallframe->setPalette(QPalette(global_colors.cg_pushb, global_colors.cg_pushb_disabled, global_colors.cg_pushb_active));
	splash_b = new QLabel(this);
	QPixmap splash;
	if (splash.load("/usr/local/bin/us.gif"))
	{
		splash_b->setGeometry((unsigned int) ((width/2)-230), 110, 460, 276);
		splash_b->setPixmap(splash);
	}

	splash_time->start(6000, true);
	connect (splash_time, SIGNAL(timeout()), this, SLOT(close_splash()));

	QPopupMenu *file_info = new QPopupMenu;
	CHECK_PTR(file_info);
	file_info->setStyle(local_style);
	file_info->setLineWidth(1);
	int veloc_infoID = file_info->insertItem(tr("&Velocity Data"), this, SLOT(open_veloc()));
	file_info->setItemEnabled(veloc_infoID, true);
	int equil_infoID = file_info->insertItem(tr("&Equilibrium Data"), this, SLOT(open_equil()));
	file_info->setItemEnabled(equil_infoID, true);
	file_info->setFont(QFont( "Helvetica", 12));

	QPopupMenu *export_data = new QPopupMenu;
	CHECK_PTR(export_data);
	export_data->setStyle(local_style);
	export_data->setLineWidth(1);
	int export_velocID = export_data->insertItem(tr("&Velocity Data"), this, SLOT(export_V()));
	export_data->setItemEnabled(export_velocID, true);
	int export_equilID = export_data->insertItem(tr("&Equilibrium Data"), this, SLOT(export_E()));
	export_data->setItemEnabled(export_equilID, true);
	export_data->setFont(QFont( "Helvetica", 12));

	QPopupMenu *print_data = new QPopupMenu;
	CHECK_PTR(print_data);
	print_data->setStyle(local_style);
	print_data->setLineWidth(1);
	int print_velocID = print_data->insertItem(tr("&Velocity Data"), this, SLOT(print_V()));
	print_data->setItemEnabled(print_velocID, true);
	int print_equilID = print_data->insertItem(tr("&Equilibrium Data"), this, SLOT(print_E()));
	print_data->setItemEnabled(print_equilID, true);
	print_data->setFont(QFont( "Helvetica", 12));

	QPopupMenu *file = new QPopupMenu;
	CHECK_PTR(file);
	file->setLineWidth(1);
	file->setStyle(local_style);
	int infoID = file->insertItem(tr("&Dataset Info"), file_info);
	file->setItemEnabled(infoID, true);
	int exportID = file->insertItem(tr("&Export Data"), export_data);
	file->setItemEnabled(exportID, true);
	int printID = file->insertItem(tr("&Print Data"), print_data);
	file->setItemEnabled(printID, true);
	int configID = file->insertItem(tr("&Configuration"), this, SLOT(config()));
	file->setItemEnabled(configID, true);
	file->insertSeparator();
	file->insertItem(tr("E&xit"), this, SLOT(quit()));
	file->setFont(QFont( "Helvetica", 12));

	QPopupMenu *type1 = new QPopupMenu;
	CHECK_PTR(type1);
	type1->setStyle(local_style);
	type1->setLineWidth(1);
	type1->insertItem(tr("&Absorbance Data"), this, SLOT(veloc_absorbance()));
	type1->insertItem(tr("&Interference Data"), this, SLOT(veloc_interference()));
	type1->setFont(QFont( "Helvetica", 12));

	QPopupMenu *type2 = new QPopupMenu;
	CHECK_PTR(type2);
	type2->setLineWidth(1);
	type2->setStyle(local_style);
	int abs2ID = type2->insertItem(tr("&Absorbance Data"), this, SLOT(equil_absorbance()));
	type2->setItemEnabled(abs2ID, true);
	int if2ID = type2->insertItem(tr("&Interference Data"), this, SLOT(equil_interference()));
	type2->setItemEnabled(if2ID, false);
	type2->setFont(QFont( "Helvetica", 12));

	QPopupMenu *edit = new QPopupMenu;
	CHECK_PTR(edit);
	edit->setStyle(local_style);
	edit->setLineWidth(1);
	edit->insertItem(tr("&Velocity Data"), type1);
	int editequilID = edit->insertItem(tr("&Equilibrium Data"), type2);
	edit->setItemEnabled(editequilID, true);
	edit->setFont(QFont( "Helvetica", 12));

	QPopupMenu *veloc = new QPopupMenu;
	CHECK_PTR(veloc);
	veloc->setStyle(local_style);
	veloc->setLineWidth(1);
	veloc->insertItem(tr("&van Holde - Weischet"), this, SLOT(vhw()));
	int finiteID = veloc->insertItem(tr("&Finite Element"), this, SLOT(finite_element()));
	veloc->setItemEnabled(finiteID, true);
	int sec_momID = veloc->insertItem(tr("&Second Moment"), this, SLOT(secm()));
	veloc->setItemEnabled(sec_momID, true);
	int dcdtID = veloc->insertItem(tr("&Time Derivative"), this, SLOT(dcdt()));
	veloc->setItemEnabled(dcdtID, true);
	int dcdrID = veloc->insertItem(tr("&Radial Derivative"), this, SLOT(dcdr()));
	veloc->setItemEnabled(dcdrID, true);
	veloc->setFont(QFont( "Helvetica", 12));

	QPopupMenu *sim = new QPopupMenu;
	CHECK_PTR(sim);
	sim->setStyle(local_style);
	sim->setLineWidth(1);
	sim->insertItem(tr("&Finite Element Simulation"), this, SLOT(simulate()));
	int kirkID = sim->insertItem(tr("&Kirkwood Theory"), this, SLOT(kirkwood()));
	sim->setItemEnabled(kirkID, false);
	int equilID = sim->insertItem(tr("&Equilibrium"), this, SLOT(equilibrium()));
	sim->setItemEnabled(equilID, false);
	sim->insertItem(tr("&Self-Association Equil."), this, SLOT(sassoc()));
	int model1ID = sim->insertItem(tr("&Model s, D and f from MW for 4 basic shapes"), this, SLOT(model1()));
	sim->setItemEnabled(model1ID, true);
	int model2ID = sim->insertItem(tr("&Predict f and axial ratios for 4 basic shapes"), this, SLOT(model2()));
	sim->setItemEnabled(model2ID, true);
	sim->setFont(QFont( "Helvetica", 12));

	QPopupMenu *equil = new QPopupMenu;
	CHECK_PTR(equil);
	equil->setStyle(local_style);
	equil->setLineWidth(1);
	int suggestID = equil->insertItem(tr("&Suggest Best Speed"), this, SLOT(predict()));
	equil->setItemEnabled(suggestID, true);
	int globalfitID = equil->insertItem(tr("&Global Fit"), this, SLOT(global_fit()));
	equil->setItemEnabled(globalfitID, true);
	int logID = equil->insertItem(tr("&Log(C) vs. r^2"), this, SLOT(lnc()));
	equil->setItemEnabled(logID, true);
	int mwcID = equil->insertItem(tr("&MW vs. Concentration"), this, SLOT(mw_conc()));
	equil->setItemEnabled(mwcID, false);
	int mwrID = equil->insertItem(tr("MW vs. &Radius"), this, SLOT(mw_rad()));
	equil->setItemEnabled(mwrID, false);
	equil->setFont(QFont( "Helvetica", 12));

	QPopupMenu *laser = new QPopupMenu;
	CHECK_PTR(laser);
	laser->setStyle(local_style);
	laser->setLineWidth(1);
	laser->insertItem(tr("&Load Data"), this, SLOT(laser_load()));
	laser->insertItem(tr("&Global Fitting"), this, SLOT(laser_global()));
	laser->insertItem(tr("&Suggest Sample Times"), this, SLOT(laser_predict()));
	laser->setFont(QFont( "Helvetica", 12));

	QPopupMenu *global_menu = new QPopupMenu;
	CHECK_PTR(global_menu);
	global_menu->setStyle(local_style);
	global_menu->setLineWidth(1);
	global_menu->setFont(QFont( "Helvetica", 12));

	QPopupMenu *util = new QPopupMenu;
	CHECK_PTR(util);
	util->setStyle(local_style);
	util->setLineWidth(1);
	int archiveID = util->insertItem(tr("&Archive Database"), this, SLOT(database()));
	util->setItemEnabled(archiveID, false);
	int reorderID = util->insertItem(tr("&Re-order Scanfile Sequence"), this, SLOT(reorder()));
	util->setItemEnabled(reorderID, true);
	int combineID = util->insertItem(tr("&Combine Distribution Plots"), this, SLOT(combine()));
	util->setItemEnabled(combineID, true);
	int diagID = util->insertItem(tr("&Diagnostics"), this, SLOT(diagnostics()));
	util->setItemEnabled(diagID, false);
	int calcID = util->insertItem(tr("&Calculate DNA/RNA MW"), this, SLOT(calc_nucleic()));
	util->setItemEnabled(calcID, false);
	int calcHydro = util->insertItem(tr("&Buffer Corrections"), this, SLOT(calc_hydro()));
	util->setItemEnabled(calcHydro, true);
	int vbarID = util->insertItem(tr("Calculate &Protein MW and vbar"), this, SLOT(calc_protein()));
	util->setItemEnabled(vbarID, true);
	util->setFont(QFont( "Helvetica", 12));

	QPopupMenu *sys = new QPopupMenu;
	CHECK_PTR(sys);
	sys->setStyle(local_style);
	sys->setLineWidth(1);
	sys->insertItem(tr("&CPU Load"), this, SLOT(cpuload()));
	sys->insertItem(tr("&Memory Info"), this, SLOT(meminfo()));
	sys->insertItem(tr("&System Load"), this, SLOT(sysload()));
	sys->setFont(QFont(tr("Helvetica"), 12));

	QPopupMenu *help = new QPopupMenu;
	CHECK_PTR(help);
	help->setStyle(local_style);
	help->setLineWidth(1);
	help->insertItem(tr("&UltraScan Home"), this, SLOT(us_home()));
	help->insertItem(tr("&UltraScan Manual"), this, SLOT(help()));
	help->insertItem(tr("&Register Software"), this, SLOT(us_register()));
	help->insertItem(tr("&Upgrade UltraScan"), this, SLOT(us_upgrade()));
	help->insertItem(tr("&About"), this, SLOT(about()));
	help->setFont(QFont( "Helvetica", 12));

	menu = new QMenuBar(this);
	CHECK_PTR(menu);
	menu->setStyle(local_style);
	menu->setFont(QFont("Helvetica", 12, QFont::Bold));
	menu->insertItem(tr("&File"), file);
	menu->insertItem(tr("&Edit"), edit);
	menu->insertItem(tr("&Velocity"), veloc);
	menu->insertItem(tr("E&quilibrium"), equil);
/*
	int laserID = menu->insertItem(tr("&Laser DLS"), laser);
	menu->setItemEnabled(laserID, false);
	int globalID = menu->insertItem(tr("&Global Fit"), global_menu);
	menu->setItemEnabled(globalID, false);
*/
	menu->insertItem(tr("&Utilities"), util);
	menu->insertItem(tr("&System"), sys);
	menu->insertItem(tr("S&imulation"), sim);
	menu->insertSeparator();
	menu->insertItem(tr("&Help"), help);
	menu->setGeometry(2, 2, width-4, 22);

	stat_bar = new QLabel(this);
	CHECK_PTR(stat_bar);
	stat_bar->setStyle(local_style);
	stat_bar->setFrameStyle(QFrame::Box | QFrame::Sunken);
	stat_bar->setFont(QFont( "Helvetica", 12));
	stat_bar->setLineWidth(1);
	stat_bar->setPalette(QPalette(global_colors.cg_edit, global_colors.cg_edit, global_colors.cg_edit));
	stat_bar->setAlignment(AlignLeft|AlignVCenter);

	connect(this,  SIGNAL(explain(const QString &)),
	stat_bar, SLOT(setText(const QString &)));
}

UsWin::~UsWin()
{
}

void UsWin::simulate()
{
	emit explain( "Loading Sedimentation Simulator..." );
	sim_w = new SimWin();	
	sim_w->setCaption("Finite Element Simulator for Sedimentation Velocity and Approach to Equilibrium Experiments");
	sim_w->show();
	emit explain( " " );
}


void UsWin::sassoc()
{
	emit explain( "Starting Self-Association Simulator..." );
	sas_w = new SassocWin;	
	sas_w->setCaption("Self-Association Simulator");
	sas_w->show();
	emit explain( " " );
}

void UsWin::cpuload()
{
	emit explain( "Loading CPU Status Information..." );
	cpu_w = new CpuloadWin;	
	cpu_w->setCaption("CPU Status");
	cpu_w->show();
	emit explain( " " );
}

void UsWin::sysload()
{
	emit explain( "Loading System Load Information..." );
	sys_w = new SysloadWin;	
	sys_w->setCaption("System Load");
	sys_w->start();
	sys_w->show();
	emit explain( " " );
}

void UsWin::meminfo()
{
	emit explain( "Loading Memory Information..." );
	mem_w = new MeminfoWin;	
	mem_w->setCaption("Memory Information");
	mem_w->start();
	mem_w->show();
	emit explain( " " );
}

void UsWin::close_splash()
{
	delete smallframe;
	delete splash_b;
	bigframe->show();
}

void UsWin::veloc_absorbance()
{
	emit explain( "Editing Velocity Absorbance Data..." );
	editv_abs = new EditAbsVeloc_Win;	
	editv_abs->setCaption("Edit Window for Velocity Absorbance Data");
	editv_abs->show();
	emit explain( " " );
}

void UsWin::veloc_interference()
{
	emit explain( "Editing Velocity Interference Data..." );
	editv_int = new EditIntVeloc_Win;	
	editv_int->setCaption("Edit Window for Velocity Interference Data");
	editv_int->show();
	emit explain( " " );
}

void UsWin::equil_absorbance()
{
	emit explain( "Editing Equilibrium Absorbance Data..." );
	edite_abs = new EditAbsEquil_Win;	
	edite_abs->setCaption("Edit Window for Equilibrium Absorbance Data");
	edite_abs->show();
	emit explain( " " );
}

void UsWin::equil_interference()
{
}

void UsWin::vhw()
{
	emit explain( "Loading van Holde - Weischet Analysis..." );
	vhw_w = new vhw_dat_W();	
	vhw_w->setCaption("van Holde - Weischet Analysis:");
	vhw_w->show();
	emit explain( " " );
}

void UsWin::finite_element()
{
	emit explain( "Loading Finite Element Analysis for a single experiment..." );
	finite1_w = new US_Finite_W();	
	finite1_w->setCaption("Finite Element Analysis:");
	finite1_w->show();
	emit explain( " " );
}

void UsWin::secm()
{
	emit explain( "Loading Second Moment Analysis..." );
	sm_w = new sm_dat_W();	
	sm_w->setCaption("Second Moment Analysis:");
	sm_w->show();
	emit explain( " " );
}

void UsWin::dcdt()
{
	emit explain( "Loading Time Derivative - dC/dt Analysis..." );
	dcdt_w = new dcdt_dat_W();	
	dcdt_w->setCaption("Time Derivative - dC/dt Analysis:");
	dcdt_w->show();
	emit explain( " " );
}

void UsWin::dcdr()
{
	emit explain( "Loading Radial Derivative - dC/dr Analysis..." );
	dcdr_w = new dcdr_dat_W();	
	dcdr_w->setCaption("Radial Derivative - dC/dr Analysis:");
	dcdr_w->show();
	emit explain( " " );
}

void UsWin::kirkwood()
{
}

void UsWin::equilibrium()
{
}

void UsWin::model1()
{
	emit explain( "Loading Molecular Modeling Simulator..." );
	hydro1_w = new US_Hydro1();	
	hydro1_w->setCaption("Modeling s, D and f from MW for 4 basic shapes:");
	hydro1_w->show();
	emit explain( " " );	
}

void UsWin::model2()
{
	emit explain( "Loading Molecular Modeling Simulator..." );
	hydro2_w = new US_Hydro2();	
	hydro2_w->setCaption("Modeling s, D and f from MW for 4 basic shapes:");
	hydro2_w->show();
	emit explain( " " );
}

void UsWin::predict()
{
	predict_dlg = new US_CalcEquil();
	predict_dlg->setCaption("Equilibrium Speed Prediction");
	predict_dlg->show();
}

void UsWin::global_fit()
{
	emit explain( "Loading Nonlinear Least Squares Fitting Routine for global Equilibrium Analysis..." );
	global_eq = new US_GlobalEquil();	
	global_eq->setCaption("Global Equilibrium Fitting:");
	global_eq->show();
	emit explain( " " );
}

void UsWin::lnc()
{
	emit explain( "Loading Equilibrium Analysis - ln(C) vs. r^2 Analysis..." );
	lncr2_w = new US_lncr2_W();	
	lncr2_w->setCaption("Equilibrium Analysis - ln(C) vs. r^2 Analysis");
	lncr2_w->show();
	emit explain( " " );
}

void UsWin::mw_conc()
{
}

void UsWin::mw_rad()
{
}

void UsWin::laser_load()
{
}

void UsWin::laser_global()
{
}

void UsWin::laser_predict()
{
}

void UsWin::database()
{
}

void UsWin::reorder()
{
	reorder_w = new US_ReOrder();
	reorder_w->show();
}

void UsWin::combine()
{
	combine_dlg = new US_Combine();
	combine_dlg->setCaption("Combine G(s) Distributions");
	combine_dlg->show();
}

void UsWin::diagnostics()
{
}

void UsWin::calc_nucleic()
{
}

void UsWin::calc_hydro()
{
	buffer_dlg = new US_Buffer();
	buffer_dlg->setCaption("Buffer Calculation");
	buffer_dlg->pb_accept->setText(" Close ");
	buffer_dlg->show();
}

void UsWin::calc_protein()
{
	float vbar=.72, temp=20, vbar20=.72;
	vbar_dlg = new US_Vbar(temp, &vbar, &vbar20);
	vbar_dlg->setCaption("V-bar Calculation");
	vbar_dlg->pb_ok->setText(" Close ");
	vbar_dlg->show();
}

void UsWin::open_veloc()
{
	emit explain( "Dataset Information for Velocity Run..." );
	dataset_i = new Data_Control_W(1);
	dataset_i->load_data();	
	emit explain( " " );
}

void UsWin::open_equil()
{
	emit explain( "Dataset Information for Equilibrium Run..." );
	dataset_i = new Data_Control_W(2);
	dataset_i->load_data();	
	emit explain( " " );
}

void UsWin::quit()
{
    QApplication::exit( 0 );
}

void UsWin::about()
{
	emit explain( "About..." );
	QMessageBox::message("About UltraScan...", "UltraScan II\n\n"
	"Version 5.0 for Unix\n"
	"Copyright 1998, 1999 UTHSCSA\n\n"
	"Author: Borries Demeler\n\n"
	"For more information, please visit:\n"
	"http://www.biochem.uthscsa.edu/UltraScan\n\n"
	"The author can be reached at:\n"
	"The University of Texas Health Science Center\n"
	"Department of Biochemistry\n"
	"7703 Floyd Curl Drive\n"
	"San Antonio, Texas 78284-7760\n"
	"voice: (210) 567-6592\n"
	"Fax:   (210) 567-6595\n"
	"E-mail: demeler@biochem.uthscsa.edu");	
	emit explain( " " );
}

void UsWin::help()
{
	emit explain( "Loading Help..." );
	US_Help *online_help;
	online_help = new US_Help("manual/index.html");
	emit explain( " " );
}

void UsWin::us_register()
{
	emit explain( "Loading Registration Information..." );
	US_Help *online_help;
	online_help = new US_Help("register.html");
	emit explain( " " );
}

void UsWin::us_home()
{
	emit explain( "Loading Registration Information..." );
	US_Help *online_help;
	online_help = new US_Help("index.html");
	emit explain( " " );
}

void UsWin::export_V()
{
	emit explain( "Loading Function for Exporting Velocity Data..." );
	export_veloc = new US_Export_Veloc();
	export_veloc->setCaption("Export Velocity Data");
	export_veloc->show();
	emit explain( " " );
}

void UsWin::export_E()
{
	emit explain( "Loading Function for Exporting Equilibrium Data..." );
	export_equil = new US_Export_Equil();
	export_equil->setCaption("Export Equilibrium Data");
	export_equil->show();
	emit explain( " " );
}

void UsWin::print_V()
{
	emit explain( "Loading Function for Printing Velocity Data..." );
	print_veloc = new US_Print_Veloc();
	print_veloc->setCaption("Printing Velocity Data");
	print_veloc->show();
	emit explain( " " );
}

void UsWin::print_E()
{
	emit explain( "Loading Function for Printing Equilibrium Data..." );
	print_equil = new US_Print_Equil();
	print_equil->setCaption("Printing Equilibrium Data");
	print_equil->show();
	emit explain( " " );
}

void UsWin::us_upgrade()
{
	emit explain( "Loading Upgrade Information..." );
	US_Help *online_help;
	online_help = new US_Help("download.html");
	emit explain( " " );
}

void UsWin::config()
{
	emit explain( "Loading Configuration..." );	
	config_w = new US_Config(1);	
	config_w->setCaption("UltraScan Configuration:");
	config_w->show();
	emit explain( " " );
}

void UsWin::config_check()
{
	config_w = new US_Config(0);	
	if (!config_w->rc_exists())
	{
		this->hide();
		emit explain( "Loading Configuration..." );	
		config_w->setCaption("UltraScan Configuration:");
		config_w->lbl_directions->setText("Please confirm your settings:");
		config_w->show();
		emit explain( " " );
	}
}

void UsWin::resizeEvent(QResizeEvent *e)
{
	QRect r(0, 0, e->size().width(), e->size().height());
	bigframe->setGeometry(2, 36, r.width()-4, r.height()-66);
	stat_bar->setGeometry(2, r.height()-28, r.width()-4, 26);
}




