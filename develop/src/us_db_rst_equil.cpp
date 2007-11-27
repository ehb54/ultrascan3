#include "../include/us_db_rst_equil.h"

US_DB_RST_Equil::US_DB_RST_Equil(QWidget *p, const char *name) : US_DB(p, name)
{
	exp_rst.expRstID = -1;
	load_flag = false;
	query_flag = false;
	cell_flag = false;
	del_flag = false;
	runinf_flag = false;
	vhw_flag = false;
	run_id = "";
	source_type = -1;
	runrequestID = 0;
	for(int i=0; i<8;i++)
	{
		for(int j=0; j<4; j++)
		{
			exp_rst.EDid[i][j] = -1;
		}
	}
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	
	pb_load = new QPushButton(tr("Load Result from HD"), this);
	Q_CHECK_PTR(pb_load);
	pb_load->setAutoDefault(false);
	pb_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_load->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_load, SIGNAL(clicked()), SLOT(load()));
	
	pb_help = new QPushButton(tr("Help"), this);
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	pb_close = new QPushButton(tr("Close"), this);
	pb_close->setAutoDefault(false);
	pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_close->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_close, SIGNAL(clicked()), SLOT(quit()));

	pb_load_db = new QPushButton(tr("Query Result from DB"), this);
	Q_CHECK_PTR(pb_load_db);	
	pb_load_db->setAutoDefault(false);
	pb_load_db->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_load_db->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_load_db, SIGNAL(clicked()), SLOT(query_db()));
		
	pb_save_db = new QPushButton(tr("Backup Result to DB"), this);
	Q_CHECK_PTR(pb_save_db);
	pb_save_db->setAutoDefault(false);
	pb_save_db->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_save_db->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_save_db, SIGNAL(clicked()), SLOT(save_db()));

	instr_lbl = new QLabel(tr("Instruction"),this);
	instr_lbl->setAlignment(AlignHCenter|AlignVCenter);
	instr_lbl->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	instr_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2, QFont::Bold));
	
	pb_del_db = new QPushButton(tr("Delete Result from DB"), this);
	Q_CHECK_PTR(pb_del_db);
	pb_del_db->setAutoDefault(false);
	pb_del_db->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_del_db->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_del_db, SIGNAL(clicked()), SLOT(check_permission()));
		
	lb_result = new QListBox(this, "Result files");
	lb_result->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lb_result->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	lb_result->insertItem(tr("Click one of above buttons to load result from"));
	lb_result->insertItem(tr("Hard Driver or Database"));
	connect(lb_result, SIGNAL(selected(int)), SLOT(select_result(int)));
	
	pb_reset = new QPushButton(tr("Reset"), this);
	Q_CHECK_PTR(pb_reset);
	pb_reset->setAutoDefault(false);
	pb_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_reset->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));
	
	pb_runrequest = new QPushButton(tr("Select Run Request"), this);
	pb_runrequest->setAutoDefault(false);
	pb_runrequest->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_runrequest->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_runrequest, SIGNAL(clicked()), SLOT(sel_runrequest()));
	
	lbl_runrequest= new QLabel("",this);
	lbl_runrequest->setAlignment(AlignLeft|AlignVCenter);
//	lbl_runrequest->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);	
	lbl_runrequest->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_runrequest->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_runrequest->setText(tr(" Not Selected"));
		
	lbl_expdata = new QLabel(tr(" Experimental Data ID:"),this);
	lbl_expdata->setAlignment(AlignLeft|AlignVCenter);
	lbl_expdata->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_expdata->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_expdt = new QLabel("",this);
	lbl_expdt->setAlignment(AlignLeft|AlignVCenter);
	lbl_expdt->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_expdt->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_expdt->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_investigator = new QLabel(tr(" Investigator ID:"),this);
	lbl_investigator->setAlignment(AlignLeft|AlignVCenter);
	lbl_investigator->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_investigator->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	lbl_invst = new QLabel("",this);
	lbl_invst->setAlignment(AlignLeft|AlignVCenter);
	lbl_invst->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_invst->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_invst->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	lbl_date = new QLabel(tr(" Date:"),this);
	lbl_date->setAlignment(AlignLeft|AlignVCenter);
	lbl_date->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_date->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_dt= new QLabel("",this);
	lbl_dt->setAlignment(AlignLeft|AlignVCenter);
	lbl_dt->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_dt->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_dt->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	lbl_description = new QLabel(tr(" Description:"),this);
	lbl_description->setAlignment(AlignLeft|AlignVCenter);
	lbl_description->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_description->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	le_dscrpt= new QLineEdit("",this);
	le_dscrpt->setAlignment(AlignLeft|AlignVCenter);
	le_dscrpt->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_dscrpt->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	le_dscrpt->setReadOnly(true);
	
	lbl_runinf = new QLabel(tr(" Information for this run:"),this);
	lbl_runinf->setAlignment(AlignHCenter|AlignVCenter);
//	lbl_runinf->setFrameStyle(QFrame::WinPanel|Raised);
	lbl_runinf->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_runinf->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	
	pb_runinfo = new QPushButton(tr("Show Run Info."), this);
	pb_runinfo->setAutoDefault(false);
	pb_runinfo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_runinfo->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_runinfo->setEnabled(false);
	connect(pb_runinfo, SIGNAL(clicked()), SLOT(show_runinfo()));
	
	lbl_plot1 = new QLabel(tr(" Plot of Temperature Variation vs. Scan Number :"),this);
	lbl_plot1->setAlignment(AlignLeft|AlignVCenter);
	lbl_plot1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_plot1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	lbl_plt1= new QLabel("",this);
	lbl_plt1->setAlignment(AlignLeft|AlignVCenter);
	lbl_plt1->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_plt1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	lbl_plot2 = new QLabel(tr(" Plot of Rotorspeed vs. Scan Number :"), this);
	lbl_plot2->setAlignment(AlignLeft|AlignVCenter);
	lbl_plot2->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_plot2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	lbl_plt2 = new QLabel("",this);
	lbl_plt2->setAlignment(AlignLeft|AlignVCenter);
	lbl_plt2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_plt2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	lbl_plot3 = new QLabel(tr(" Plot of Time Differences between Scans vs. Scan Number :"), this);
	lbl_plot3->setAlignment(AlignLeft|AlignVCenter);
	lbl_plot3->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_plot3->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	lbl_plt3= new QLabel("",this);
	lbl_plt3->setAlignment(AlignLeft|AlignVCenter);
	lbl_plt3->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_plt3->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	lbl_plot4 = new QLabel(tr(" Wavelength Scan/Extinction Calculation Results :"), this);
	lbl_plot4->setAlignment(AlignLeft|AlignVCenter);
	lbl_plot4->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_plot4->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	lbl_plt4= new QLabel("",this);
	lbl_plt4->setAlignment(AlignLeft|AlignVCenter);
	lbl_plt4->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_plt4->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_plot5 = new QLabel(tr(" Extinction Profile Plot :"), this);
	lbl_plot5->setAlignment(AlignLeft|AlignVCenter);
	lbl_plot5->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_plot5->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	lbl_plt5= new QLabel("",this);
	lbl_plt5->setAlignment(AlignLeft|AlignVCenter);
	lbl_plt5->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_plt5->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_plot6 = new QLabel(tr(" ASCII Data for Extinction Profile Plot :"), this);
	lbl_plot6->setAlignment(AlignLeft|AlignVCenter);
	lbl_plot6->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_plot6->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	lbl_plt6= new QLabel("",this);
	lbl_plt6->setAlignment(AlignLeft|AlignVCenter);
	lbl_plt6->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_plt6->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));


	lbl_Cell = new QLabel(tr(" Cell Result Data :"),this);
	lbl_Cell->setAlignment(AlignHCenter|AlignVCenter);
//	lbl_Cell->setFrameStyle(QFrame::WinPanel|Raised);
	lbl_Cell->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_Cell->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	
	pb_dataset = new QPushButton(tr("Extract Dataset"), this);
   pb_dataset->setAutoDefault(false);
   pb_dataset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_dataset->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_dataset->setEnabled(false);
   connect(pb_dataset, SIGNAL(clicked()), SLOT(show_dataset()));

	lbl_cell = new QLabel(tr(" Cell ID :"), this);
	lbl_cell->setAlignment(AlignLeft|AlignVCenter);
	lbl_cell->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_cell->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cmbb_cell = new QComboBox(false, this, "Cell ID Select");
	cmbb_cell->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cmbb_cell->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	cmbb_cell->insertItem(" No Data");
	connect(cmbb_cell, SIGNAL(activated(int)), SLOT(sel_cell(int)));
	
	border=4, spacing=2;
	xpos = border, ypos = border;
	buttonw = 150, buttonh = 26;
	
	pb_load->setGeometry(xpos, ypos, buttonw*2, buttonh);
	
	xpos += buttonw*2+spacing;
	pb_help->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos += buttonw + spacing;
	pb_close->setGeometry(xpos, ypos, buttonw, buttonh);
	
	xpos = border;
	ypos += buttonh + spacing;
	pb_load_db->setGeometry(xpos, ypos, buttonw*2, buttonh);
	
	xpos += buttonw*2 + spacing;
	pb_save_db->setGeometry(xpos, ypos, buttonw*2+spacing, buttonh);
	
	xpos = border;
	ypos += buttonh + spacing;
	instr_lbl->setGeometry(xpos, ypos, buttonw*2, buttonh);
	
	xpos += buttonw*2 + spacing;
	pb_del_db->setGeometry(xpos, ypos, buttonw*2+spacing, buttonh);
	
	xpos = border;
	ypos += buttonh + spacing;	
	lb_result->setGeometry(xpos, ypos, buttonw*2, buttonh*6+5*spacing);

	xpos += buttonw*2 + spacing;
	pb_reset->setGeometry(xpos, ypos, buttonw*2+spacing, buttonh);	
	
	ypos += buttonh + spacing;
	pb_runrequest->setGeometry(xpos, ypos, buttonw, buttonh);
	
	xpos += buttonw + spacing;
	lbl_runrequest->setGeometry(xpos, ypos, buttonw, buttonh);
	
	xpos -= buttonw + spacing;
	ypos += buttonh + spacing;
	lbl_expdata->setGeometry(xpos, ypos, buttonw, buttonh);
	
	xpos += buttonw + spacing;
	lbl_expdt->setGeometry(xpos, ypos, buttonw, buttonh);
	
	xpos -= buttonw + spacing;
	ypos += buttonh + spacing;
	lbl_investigator->setGeometry(xpos, ypos, buttonw, buttonh);
	
	xpos += buttonw + spacing;
	lbl_invst->setGeometry(xpos, ypos, buttonw, buttonh);
	
	xpos -= buttonw + spacing;
	ypos += buttonh + spacing;
	lbl_date->setGeometry(xpos, ypos, buttonw, buttonh );

	xpos += buttonw + spacing;
	lbl_dt->setGeometry(xpos, ypos, buttonw, buttonh);
	
	xpos -= buttonw + spacing;
	ypos += buttonh + spacing;
	lbl_description->setGeometry(xpos, ypos, buttonw, buttonh);
	
	xpos += buttonw + spacing;
	le_dscrpt->setGeometry(xpos, ypos, buttonw, buttonh);
	
	xpos = border;
	ypos += buttonh + spacing;
	lbl_runinf->setGeometry(xpos, ypos, buttonw*3 + spacing*2, buttonh);
	
	xpos += buttonw*3 +spacing*2;
	pb_runinfo->setGeometry(xpos, ypos, buttonw , buttonh);
	
	xpos = border;
	ypos += buttonh + spacing;
	lbl_plot1->setGeometry(xpos, ypos, buttonw*3+spacing, buttonh);
	
	xpos += buttonw*3 + spacing*2;
	lbl_plt1->setGeometry(xpos, ypos, buttonw, buttonh);
	
	xpos = border;
	ypos += buttonh + spacing;
	lbl_plot2->setGeometry(xpos, ypos, buttonw*3+spacing, buttonh);
	
	xpos += buttonw*3 + spacing*2;
	lbl_plt2->setGeometry(xpos, ypos, buttonw, buttonh);
	
	xpos = border;
	ypos += buttonh + spacing;
	lbl_plot3->setGeometry(xpos, ypos, buttonw*3+spacing, buttonh);
	
	xpos += buttonw*3 + spacing*2;
	lbl_plt3->setGeometry(xpos, ypos, buttonw, buttonh);
	
	xpos = border;
	ypos += buttonh + spacing;
	lbl_plot4->setGeometry(xpos, ypos, buttonw*3 + spacing*2, buttonh);
	
	xpos += buttonw*3 + spacing*2;
	lbl_plt4->setGeometry(xpos, ypos, buttonw, buttonh);
	
	xpos = border;
	ypos += buttonh + spacing;
	lbl_plot5->setGeometry(xpos, ypos, buttonw*3 + spacing*2, buttonh);
	
	xpos += buttonw*3 + spacing*2;
	lbl_plt5->setGeometry(xpos, ypos, buttonw, buttonh);
	
	xpos = border;
	ypos += buttonh + spacing;
	lbl_plot6->setGeometry(xpos, ypos, buttonw*3 + spacing*2, buttonh);
	
	xpos += buttonw*3 + spacing*2;
	lbl_plt6->setGeometry(xpos, ypos, buttonw, buttonh);
	
	xpos = border;
	ypos += buttonh + spacing;
	lbl_Cell->setGeometry(xpos, ypos, buttonw*3 + spacing, buttonh);
	
	xpos += buttonw*3 +spacing*2;
   pb_dataset->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos = border;
	ypos += buttonh + spacing;
	lbl_cell->setGeometry(xpos, ypos, buttonw/2, buttonh);
	
	xpos += buttonw/2 + spacing;
	cmbb_cell->setGeometry(xpos, ypos, buttonw*3+buttonw/2 + spacing, buttonh);
	
	Y = ypos += buttonh + spacing;
	X = xpos = 2 * border + buttonw*4+ spacing*2;

   Xpos = geometry().x();
	Ypos = geometry().y();
	setGeometry(Xpos, Ypos, xpos, ypos);

}

US_DB_RST_Equil::~US_DB_RST_Equil()
{
}

void US_DB_RST_Equil::sel_runrequest()
{
	US_DB_RunRequest *runrequest_dlg;
	runrequest_dlg = new US_DB_RunRequest();
	runrequest_dlg->setCaption("Run Request Information");
	runrequest_dlg->show();
	connect(runrequest_dlg, SIGNAL(issue_RRID(int)), SLOT(update_runrequest_lbl(int)));
}

void US_DB_RST_Equil::update_runrequest_lbl (int RRID)
{
	runrequestID = RRID;
	lbl_runrequest->setNum(runrequestID);
	if(runrequestID == 0)
	{
		lbl_runrequest->setText(" Not Selected");
	}
}

void US_DB_RST_Equil::load()
{
	reset();
	data_control = new Data_Control_W(8);	//equilibrium data
	data_control->load_data();
	run_id = data_control->run_inf.run_id;
	htmlDir = data_control->htmlDir;
	baseName = USglobal->config_list.result_dir + "/" + data_control->run_inf.run_id + ".";

	exp_rst.expRstID = data_control->run_inf.expdata_id;
	lbl_expdt->setNum(exp_rst.expRstID);
	exp_rst.invID = data_control->run_inf.investigator;
	lbl_invst->setNum(exp_rst.invID);
	exp_rst.date = data_control->run_inf.date;
	lbl_dt->setText(exp_rst.date);
	exp_rst.description = data_control->run_inf.description;
	le_dscrpt->setText(exp_rst.description);
	for(int i=0; i<8;i++)
	{
		for(int j=0; j<3; j++)
		{
			exp_rst.wavelength[i][j] = data_control->run_inf.wavelength[i][j];
			exp_rst.scans[i][j] = data_control->run_inf.scans[i][j];
		}
	}
	QFile testfile;
// Run Information:
	QString dirName = USglobal->config_list.html_dir + "/temp/";
	QString runinfo_file = make_tempFile(dirName, "runinfo.html");
	QFile f_runinfo(runinfo_file);
	f_runinfo.open(IO_WriteOnly | IO_Translate);
	QTextStream ts(&f_runinfo);
	ts << tr("</ul>\n<p><b>Run Information (from hard drive) :</b>\n<ul>\n");
	fileName = htmlDir + "/temperature_data.png";
	testfile.setName(fileName);
	if (testfile.exists())
	{
		ts << "   <li><a href="<<fileName<< tr(">Plot of Temperature Variation vs. Scan Number</a>\n");
		lbl_plt1->setText(" available");
		runinf_flag = true;
	}
	else
	{
		lbl_plt1->setText(" not available");
	}
	fileName = htmlDir + "/rotorspeed_data.png";
	testfile.setName(fileName);
	if (testfile.exists())
	{
		ts << "   <li><a href="<<fileName<< tr(">Plot of Rotorspeed vs. Scan Number</a>\n");
		lbl_plt2->setText(" available");
		runinf_flag = true;
	}
	else
	{
		lbl_plt2->setText(" not available");
	}
	fileName = htmlDir + "/time_data.png";
	testfile.setName(fileName);
	if (testfile.exists())
	{
		ts << "   <li><a href="<<fileName<< tr(">Plot of Time Differences between Scans vs. Scan Number</a>\n<p>\n");			
		lbl_plt3->setText(" available");
		runinf_flag = true;
	}
	else
	{
		lbl_plt3->setText(" not available");
	}
	fileName = baseName + "extinction.res";
	testfile.setName(fileName);
	if (testfile.exists())
	{
		ts << "   <li><a href="<<fileName<< tr(">Wavelength Scan/Extinction Calculation Results</a>\n<p>\n");			
		lbl_plt4->setText(" available");
		runinf_flag = true;
	}
	else
	{
		lbl_plt4->setText(" not available");
	}
	fileName = htmlDir + "/" + data_control->run_inf.run_id + ".extinction.png";
	testfile.setName(fileName);
	if (testfile.exists())
	{
		ts << "   <li><a href="<<fileName<< tr(">Extinction Profile Plot</a>\n<p>\n");			
		lbl_plt5->setText(" available");
		runinf_flag = true;
	}
	else
	{
		lbl_plt5->setText(" not available");
	}
	fileName = baseName + "extinction.dat";
	testfile.setName(fileName);
	if (testfile.exists())
	{
		ts << "   <li><a href="<<fileName<< tr(">ASCII Data for Extinction Profile Plot</a>\n<p>\n");			
		lbl_plt6->setText(" available");
		runinf_flag = true;
	}
	else
	{
		lbl_plt6->setText(" not available");
	}

	if(runinf_flag)
	{
		pb_runinfo->setEnabled(true);
	}
	f_runinfo.close();
	source_type = 0;		//from HD
	lb_result->clear();
	lb_result->insertItem(tr("Data shown is loaded from Hard Drive"));
	load_flag = true;
	load_cell();
}
void US_DB_RST_Equil::load_cell()
{
	QString cell_Str, cellid_Str;
	cmbb_cell->clear();
db_connect();
	// get CellID and cell description from DB (search key = exp_rst.expRstID)
	QSqlCursor cur( "tblExpData" );
	QStringList orderFields = QStringList()<<"Cell1"<<"Cell2"<<"Cell3"<<"Cell4"
														<<"Cell5"<<"Cell6"<<"Cell7"<<"Cell8"
														<<"Cell1ID"<<"Cell2ID"<<"Cell3ID"<<"Cell4ID"
														<<"Cell5ID"<<"Cell6ID"<<"Cell7ID"<<"Cell8ID";
   QSqlIndex order = cur.index( orderFields );
   QSqlIndex filter = cur.index( "ExpdataID" );
   cur.setValue( "ExpdataID", exp_rst.expRstID );
	cur.select(filter, order);
	while(cur.next())
	{
		for(int i=0; i<8; i++)
		{
			cell_Str = "";
			cell_Str.sprintf("Cell%d", i+1);
			exp_rst.Cell[i] = cur.value(cell_Str).toString();
			cellid_Str = "";
			cellid_Str.sprintf("Cell%dID", i+1);
			exp_rst.CellID[i] = cur.value(cellid_Str).toInt();
		}
		update_cell();
	}
}
void US_DB_RST_Equil::update_cell()
{
	QString str;
	cmbb_cell->clear();
	for(int i=0; i<8; i++)
	{
		if(exp_rst.Cell[i].isEmpty())
		{
			str.sprintf("Cell %d : No Data", i+1);
			cmbb_cell->insertItem(str);
		}
		else
		{
			for(int j=0; j<3; j++)
			{
				if(exp_rst.wavelength[i][j]>0)
				{
					str.sprintf("Cell %d : (%d) "+ exp_rst.Cell[i]+" (WL%d: %dnm, Scans: %d)",(i+1), exp_rst.CellID[i], (j+1), exp_rst.wavelength[i][j],exp_rst.scans[i][j] );
					cmbb_cell->insertItem(str);
				}
			}
		}
	}
}
void US_DB_RST_Equil::sel_cell(int i)
{
	QString text = cmbb_cell->currentText();
	
	if(!text.isEmpty())
	{
		QString str;
		str.sprintf("Cell %d : No Data", i+1);
		if(text == str)
		{
			cell_flag = false;
		}
		else
		{
			cell_flag = true;		
			ct = new US_DB_RST_Channel(run_id, text, source_type, this);
			ct->setGeometry(border, ypos, buttonw*4, buttonh*8);
        	ct->show();
			//update();
		}
	}
	else
	{
		cell_flag = false;
	}
}

void US_DB_RST_Equil::show_runinfo()
{
	QString display_file;
	if(source_type == 0)
	{ 
		display_file = USglobal->config_list.html_dir +"/temp/runinfo.html";
	}
	if(source_type == 1)
	{ 
		display_file = USglobal->config_list.html_dir +"/db_temp/runinfo.html";
	}
	US_Help *online_help; 
	online_help = new US_Help(this);
	online_help->show_html_file(display_file);
}

void US_DB_RST_Equil::show_dataset()
{
   QString fileName = USglobal->config_list.html_dir+"/db_temp/equilData.tar.gz";
	QDir work_dir(USglobal->config_list.result_dir);
   QProcess *proc;
   proc = new QProcess(this);
   proc->clearArguments();
   proc->setWorkingDirectory(work_dir);
   QStringList cmd;
   cmd.append("tar");
   cmd.append("-xvvzf");
   cmd.append(fileName);
   proc->setArguments(cmd);
   if(!proc->start())
	{
		QMessageBox::message("UltraScan Error:", "Unable to start process to extract dataset.");
		exit(0);
	}
	else
	{
		QMessageBox::message("UltraScan Message:", "The dataset has been successfully extracted to: \n\n "+USglobal->config_list.result_dir);
	}

}

void US_DB_RST_Equil::save_db()
{
	if(!load_flag)
	{
		QMessageBox::message(tr("Attention:"), 
									tr("Please load result data from harddrive first.\n"));
		return;
	}
	QString str, STR, cellid_str;
	QSqlCursor cur( "EquilResult");
	str.sprintf("EquilRstID=%d", exp_rst.expRstID);
	cur.select(str);
	if(cur.next())
	{
		QMessageBox::message(tr("Attention:"), 
									tr("These results have been previously committed to the DB.\n"
									"To update these results, please delete the existing data, then commit the update."));
		return;
	}
	for(int i=0; i<8; i++)
	{
			
		if(exp_rst.CellID[i] > 0)
		{
		// Save EquilCellResult table
			STR = "INSERT INTO EquilCellResult (CellRstID, EquilRstID, ";
			for(int j=0; j<4; j++)			// 1~4 channel
			{
				exp_rst.EDid[i][j] = save_db_expdata(i, j);
				if(exp_rst.EDid[i][j] > 0)
				{
					str.sprintf("EED%dID, ", j+1);
					STR += str;
				}
			}
			STR += "Description) VALUES (";
			STR += QString::number(exp_rst.CellID[i]) + ", ";
			STR += QString::number(exp_rst.expRstID) + ", ";
			for(int j=0; j<4; j++)			// 1~4 channel
			{
				if(exp_rst.EDid[i][j] > 0)
				{
					STR += QString::number(exp_rst.EDid[i][j]) + ", ";
				}
			}
			STR += "'" + exp_rst.Cell[i] + "');";
			QSqlQuery target;
			target.exec(STR);
		}
	}
	switch(QMessageBox::information(this, tr("Save this entry?"), 
										tr("Clicking 'OK' will save results to database"),
										tr("OK"), tr("CANCEL"),	0,1))
	{
		case 0:
		{
			//Insert into EquilResult
			STR = "INSERT INTO EquilResult (EquilRstID, InvID, Date, Description";
			for(int i=0; i<8; i++)
			{
				if(exp_rst.CellID[i] > 0)
				{
					str.sprintf(", Cell%dID", i+1);
					STR += str;
				}	
			}
			if(runrequestID>0)
			{
				STR += ", RunRequestID";
			}
			STR += ") VALUES (";
			STR += QString::number(exp_rst.expRstID);
			STR += ", " + QString::number(exp_rst.invID);
			STR += ", '" + exp_rst.date + "'";
			STR += ", '" + exp_rst.description + "'";
			for(int i=0; i<8; i++)
			{
				if(exp_rst.CellID[i] > 0)
				{
					STR += ", " + QString::number(exp_rst.CellID[i]);
				}	
			}
			if(runrequestID>0)
			{
				STR += ", "+ QString::number(runrequestID);
			}
			STR +=");";
			QSqlQuery target1;
			target1.exec(STR);

/**************************************************************************/
	//Insert Compress data into EquilResultFile
			if(!insertCompressData())
			{
				QMessageBox::message(tr("Attention:"), tr("Insert compress data failed.\n"));
            exit(0);
			}
			
/********************************************************************/
		
			if(runrequestID >0)		//update tblResult 
			{
				int resultID = 0;
				str.sprintf("SELECT ResultID FROM tblResult WHERE RunRequestID = %d;", runrequestID);
				QSqlQuery chk(str);
				if(chk.isActive())
				{
					if(chk.next() )
					{
						resultID = chk.value(0).toInt();
					}
				}
				QSqlQuery query;
				if(resultID > 0)		// row exists and update equilprojectID
				{
					str.sprintf("UPDATE tblResult SET InvestigatorID = %d, EquilDataID = %d WHERE ResultID = %d;", exp_rst.invID, exp_rst.expRstID, resultID);
					query.exec(str);
				}
				else		//create new one
				{
					resultID = get_newID("tblResult","ResultID");
					str = "INSERT INTO tblResult (ResultID, InvestigatorID, RunRequestID, EquilDataID) VALUES(";
					str += QString::number(resultID)+", "+ QString::number(exp_rst.invID)+", " + QString::number(runrequestID) +", " + QString::number(exp_rst.expRstID)+");";
					bool flag = query.exec(str);
					if(!flag)
					{
						QMessageBox::message(tr("Attention:"), 
									tr("Uploading data to the Result Database table failed!"));
						return;
					}
				}
			}								
			break;
		}	
		case 1:
		{
			del_subTable();
			break;
		}
	}	
	
}

bool US_DB_RST_Equil::insertCompressData()
{

	compress_proc = new QProcess(this);
	c_step=0;   
	connect(compress_proc, SIGNAL(processExited()), this, SLOT(endCompressProcess()));

   if(!compress_proc->start())
   {
       QMessageBox::message("UltraScan Error:", "Unable to start process to compress equilibrium datafile.");
       return false;
   }
	return true;             	
}

void US_DB_RST_Equil::endCompressProcess()
{
	QString str;
	//Insert into EquilResultFile
	QSqlCursor cur_f( "EquilResultFile");
	str.sprintf("EquilRstID=%d", exp_rst.expRstID);
	cur_f.setMode( QSqlCursor::Insert);
	QSqlRecord *buffer = cur_f.primeInsert();
	buffer->setValue( "EquilRstID", exp_rst.expRstID );
  	fileName = htmlDir + "/temperature_data.png";
	write_blob(fileName, buffer, "RunInfo1");
	fileName = htmlDir + "/rotorspeed_data.png";
	write_blob(fileName, buffer, "RunInfo2");
	fileName = htmlDir + "/time_data.png";
	write_blob(fileName, buffer, "RunInfo3");
	fileName = baseName + "extinction.res";
	write_blob(fileName, buffer, "RunInfo4");
	fileName = htmlDir + "/" + data_control->run_inf.run_id + ".extinction.png";
	write_blob(fileName, buffer, "RunInfo5");
	fileName = baseName + "extinction.dat";
	write_blob(fileName, buffer, "RunInfo6");

	QDir work_dir(USglobal->config_list.result_dir);
	QString tarfile = data_control->run_inf.run_id + "_equil.tar";
              
	switch(c_step)
	{
		case 0:			//create dataset tar file	
		{
			compress_proc->clearArguments();
         compress_proc->setWorkingDirectory(work_dir);
			QStringList cmd;
			cmd.append("tar");
			cmd.append("--mode=u+X");
			cmd.append("-cvvf");
			cmd.append(tarfile);
			fileName = data_control->run_inf.run_id + ".us.e";
			cmd.append(fileName);
			for(int i=0; i<8; i++)		//cell
			{
				for(int j=0; j<3; j++)		//wavelength
				{
					for(int k=0; k<4; k++)		//channel
					{
						fileName = data_control->run_inf.run_id + ".equil." + QString::number(i+1) + QString::number(j+1) + QString::number(k+1);
						QString  str = USglobal->config_list.result_dir + "/" + fileName;
						QFile testFile(str);
						if(testFile.exists())
						{
							cmd.append(fileName);
						}				
					}
				}
			}
			compress_proc->setArguments(cmd);
			c_step=1;
			if(!compress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to tar equilibrium data archive.");
				return;
			}
			break;

		}
		case 1:			//gzip tar file
		{
			compress_proc->clearArguments();
			compress_proc->setWorkingDirectory(work_dir);
			QStringList g_cmd;
			g_cmd.append("gzip");
			g_cmd.append("-9");
			g_cmd.append(tarfile);
			compress_proc->setArguments(g_cmd);
			c_step=2;
			if(!compress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to gzip equilibrium datafile.");
				return;
			}
			break;			
		}
		case 2:	//insert to DB
		{
			fileName = USglobal->config_list.result_dir + "/"+data_control->run_inf.run_id + "_equil.tar.gz";
			write_blob(fileName, buffer, "DataFile");
			cur_f.insert();

			delete compress_proc;
		}
	}
}

/*! 
	Clean EquilCellResult and relative tbles that already save to db.
*/
void US_DB_RST_Equil::del_subTable()
{
	for(int i=0; i<8; i++)
	{
		if(exp_rst.CellID[i] > 0)
		{
			// channel 1~4
			for(int j=0; j<4;j++)
			{
				clean_db("EquilExpData", "EEDID","EquilExpDataTable", "EEDtableID",exp_rst.EDid[i][j]);
			}
			clean_db("EquilCellResult","CellRstID","","",exp_rst.CellID[i]);
		}
	}
}

int US_DB_RST_Equil::save_db_expdata(int cell, int channel)
{
	QString str;

	bool flag = false;	// To check whether cell expdata result exist.
	int id = -1;
	int tableId[3];
	int i = cell;
	int k = channel;
	
	for(int j=0; j<3; j++)			// 3 wavelength
	{
		tableId[j] = -1;			// Initialize to -1
		if (data_control->run_inf.scans[i][j] != 0)
		{
			QSqlCursor cur_t( "EquilExpDataTable");
			cur_t.setMode( QSqlCursor::Insert);
			QSqlRecord *buffer_t = cur_t.primeInsert();
		// Raw Experimental Data Plot Image:			
			str.sprintf(htmlDir + "/raw_%d%d%d.png", i+1, j+1, k+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "EDPlot");
			else
				write_blob(str, buffer_t, "EDPlot");
		// Raw Data Ascii Data:			
			str.sprintf(baseName + "raw.%d%d%d", i+1, j+1, k+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "F_EDPlot");
			else
				write_blob(str, buffer_t, "F_EDPlot");
		// Peptide data
			str= USglobal->config_list.result_dir + "/" + QString::number(data_control->run_inf.peptide_serialnumber[i][k][0]) + ".pep_res";
			if(!flag)
				flag = write_blob(str, buffer_t, "PepFile");
			else
				write_blob(str, buffer_t, "PepFile");
		// Buffer data
			str= USglobal->config_list.result_dir + "/" + QString::number(data_control->run_inf.buffer_serialnumber[i][k]) + ".buf_res";
			if(!flag)
				flag = write_blob(str, buffer_t, "BufFile");
			else
				write_blob(str, buffer_t, "BufFile");
		//only data exists then do inserting to "EquilExpDataTable"		
			if(flag)	
			{
				tableId[j] = get_newID("EquilExpDataTable","EEDtableID");
				buffer_t->setValue("EEDtableID", tableId[j]);
				cur_t.insert();
			}
		}	
	}
	//only data exists then do inserting to "EquilExpData"
	if(flag)			
	{
		QString STR;
		id = get_newID("EquilExpData", "EEDID");
		STR = "INSERT INTO EquilExpData (EEDID" ;
		if(tableId[0]>0)
		{
			STR += ", EEDtableID_WL1";
		}
		if(tableId[1]>0)
		{
			STR += ", EEDtableID_WL2"; 
		}
		if(tableId[2]>0)
		{
			STR += ", EEDtableID_WL3";
		}
		STR +=") VALUES(";
	
		STR += QString::number(id);
		if(tableId[0]>0)
		{
			STR +=", "+ QString::number(tableId[0]);
		}
		if(tableId[1]>0)
		{
			STR +=", "+ QString::number(tableId[1]);
		}
		if(tableId[2]>0)
		{
			STR +=", "+ QString::number(tableId[2]);
		}
		STR +=");";
		QSqlQuery target;
		target.exec(STR);
	}
	return id;
}
	
void US_DB_RST_Equil::query_db()
{
	QString str;
	reset();
	
	int maxID = get_newID("EquilResult","EquilRstID");
	int count = 0;
	item_ExpdataID = new int[maxID];
	item_Description = new QString[maxID];
	display_Str = new QString[maxID];
	
	str.sprintf("SELECT EquilRstID, Description FROM EquilResult;");
	QSqlQuery query(str);
	if(query.isActive())
	{
		while(query.next() )
		{
			item_ExpdataID[count] = query.value(0).toInt();
			item_Description[count] = query.value(1).toString();
			display_Str[count] = "("+  QString::number( item_ExpdataID[count] ) + "): "+item_Description[count];							 		
			count++;					
		}
	}
	if(count>0)
	{
		lb_result->clear();
		for( int i=0; i<count; i++)
		{
			lb_result->insertItem(display_Str[i]);
		}
		instr_lbl->setText("Doubleclick item to select");
		query_flag = true;
	}
	else
	{
		lb_result->clear();
		lb_result->insertItem("No data were found in the DB");
	}
}

void US_DB_RST_Equil::select_result(int item)
{
	bool flag;
	QString str, str1, str2, filename;
	QString dirName = USglobal->config_list.html_dir + "/db_temp/";
	if(query_flag)
	{
		exp_rst.expRstID = item_ExpdataID[item];
		exp_rst.description = item_Description[item];
		/**********************************************
				To query EquilResultFile DB table
		***********************************************/
		
		QSqlCursor cur_f( "EquilResultFile" );
   	str.sprintf("EquilRstID = %d", exp_rst.expRstID);
		cur_f.select(str);
		if(cur_f.next())
		{
			//exp_rst.invID = cur.value("InvID").toInt();
			//exp_rst.date = cur.value("Date").toString();
		// Run Information:	
			QString runinfo_file = make_tempFile(dirName, "runinfo.html");
			QFile f_runinfo(runinfo_file);
			f_runinfo.open(IO_WriteOnly | IO_Translate);
			QTextStream ts(&f_runinfo);
			ts << tr("</ul>\n<p><b>Run Information (from database) :</b>\n<ul>\n");
	
			filename = make_tempFile(dirName, "temperature_data.png");
			flag = read_blob("RunInfo1", cur_f, filename);
			if(flag)
			{
				ts << "   <li><a href="<<filename<< tr(">Plot of Temperature Variation vs. Scan Number</a>\n");
				lbl_plt1->setText(" available");
				runinf_flag = true;
			}
			else
			{			
				lbl_plt1->setText(" not available");
			}
			filename = make_tempFile(dirName, "rotorspeed_data.png");
			flag = read_blob("RunInfo2", cur_f, filename);
			if(flag)
			{
				ts << "   <li><a href="<<filename<< tr(">Plot of Rotorspeed vs. Scan Number</a>\n");
				lbl_plt2->setText(" available");
				runinf_flag = true;
			}
			else
			{			
				lbl_plt2->setText(" not available");
			}
			filename = make_tempFile(dirName, "time_data.png");
			flag = read_blob("RunInfo3", cur_f, filename);
			if(flag)
			{
				ts << "   <li><a href="<<filename<< tr(">Plot of Time Differences between Scans vs. Scan Number</a>\n<p>\n");			
				lbl_plt3->setText(" available");
				runinf_flag = true;
			}
			else	
			{		
				lbl_plt3->setText(" not available");
			}
			filename = make_tempFile(dirName, "extinction.res");
			flag = read_blob("RunInfo4", cur_f, filename);
			if(flag)
			{
				ts << "   <li><a href="<<filename<< tr(">Wavelength Scan/Extinction Calculation Results</a>\n<p>\n");			
				lbl_plt4->setText(" available");
				runinf_flag = true;
			}
			else	
			{		
				lbl_plt4->setText(" not available");
			}
			filename = make_tempFile(dirName, "extinction.png");
			flag = read_blob("RunInfo5", cur_f, filename);
			if(flag)
			{
				ts << "   <li><a href="<<filename<< tr(">Extinction Profile Plot</a>\n<p>\n");			
				lbl_plt5->setText(" available");
				runinf_flag = true;
			}
			else	
			{		
				lbl_plt5->setText(" not available");
			}
			filename = make_tempFile(dirName, "extinction.dat");
			flag = read_blob("RunInfo6", cur_f, filename);
			if(flag)
			{
				ts << "   <li><a href="<<filename<< tr(">ASCII Data for Extinction Profile Plot</a>\n<p>\n");			
				lbl_plt6->setText(" available");
				runinf_flag = true;
			}
			else	
			{		
				lbl_plt6->setText(" not available");
			}

			f_runinfo.close();
			if(runinf_flag)
			{
				pb_runinfo->setEnabled(true);
			}
			//raw result data file
			filename = make_tempFile(dirName, "equilData.tar.gz");
			flag = read_blob("DataFile", cur_f, filename);
			if(flag)
			{
				pb_dataset->setEnabled(true);
			}

		}
		//get Cell 1~8 ID
		/**********************************************
					To query EquilResult DB table
		***********************************************/
		QSqlCursor cur( "EquilResult" );
   	str.sprintf("EquilRstID = %d", exp_rst.expRstID);
		cur.select(str);		
		if(cur.next())
		{
			exp_rst.invID = cur.value("InvID").toInt();
			exp_rst.date = cur.value("Date").toString();
			runrequestID = cur.value("RunRequestID").toInt();
			QString cellid_Str;
			for(int i=0; i<8; i++)
			{
				cellid_Str = "";
				cellid_Str.sprintf("Cell%dID", i+1);
				exp_rst.CellID[i] = cur.value(cellid_Str).toInt();			
			}
			
			/**********************************************
				To query EquilCellResult DB table
			***********************************************/
			QSqlCursor get( "EquilCellResult" );
			QString get_cellid, str;
			for(int i=0; i<8; i++)
			{
				get_cellid.sprintf("CellRstID = %d",exp_rst.CellID[i]);
				get.select(get_cellid);
				if(get.next())
				{
					exp_rst.Cell[i] = get.value("Description").toString();
					for(int j=0; j<4; j++)
					{
						str.sprintf("EED%dID",j+1);
						exp_rst.EDid[i][j] = get.value(str).toInt();
					}
				}
			}
			/*******************************************************
					To query tblCell for wavelength and scans
			*******************************************************/
			QSqlCursor chk( "tblCell" );
			for(int i=0; i<8; i++)
			{
 				str.sprintf("CellID = %d",exp_rst.CellID[i]);
				chk.select(str);
				if(chk.next())
				{
					for(int j=0; j<3; j++)
					{
						str.sprintf("Wavelength_%d", j+1);
						exp_rst.wavelength[i][j] = chk.value(str).toInt();
						str.sprintf("Wl_Scans_%d", j+1);
						exp_rst.scans[i][j] = chk.value(str).toInt();
					}
				}
			}
			source_type = 1;
		}
		del_flag = true;
		lbl_expdt->setNum(exp_rst.expRstID);
		lbl_invst->setText(show_investigator(exp_rst.invID));
		lbl_dt->setText(exp_rst.date);
		le_dscrpt->setText(exp_rst.description);
		if(runrequestID > 0)
		{
			lbl_runrequest->setNum(runrequestID);
			pb_runrequest->setEnabled(false);
		}
		update_cell();
	}
}

/*!
	Open US_DB_Admin to check delete permission.
*/
void US_DB_RST_Equil::check_permission()
{
	US_DB_Admin *db_admin;
	db_admin = new US_DB_Admin("");
	db_admin->show();
	connect(db_admin, SIGNAL(issue_permission(bool)), SLOT(delete_db(bool)));
}

/*!
	Delete one selected experimental result table entry.
*/
void US_DB_RST_Equil::delete_db(bool permission)
{
	QString str;
	if(!permission)
	{
		QMessageBox::message(tr("Attention:"), 
									tr("Permission denied, please contact your system administrator for help"));
		return;
	}
	if(!del_flag)
	{
		QMessageBox::message(tr("Attention:"), 
									tr("Please select a record\n"
										"to be deleted from the database"));
	}
	else
	{
		QSqlCursor cur( "EquilResult");
		cur.setMode( QSqlCursor::Delete);
		QSqlIndex filter = cur.index("EquilRstID");
		cur.setValue("EquilRstID", exp_rst.expRstID);
		cur.primeDelete();

		switch(QMessageBox::information(this, tr("Delete this record?"), 
										tr("Clicking 'OK' will delete the selected result data from the database"),
										tr("OK"), tr("CANCEL"),	0,1))
		{
			case 0:
			{
				del_subTable();
				QSqlQuery query;
				str.sprintf("UPDATE tblResult SET EquilDataID = NULL WHERE EquilDataID = %d;", exp_rst.expRstID);
				query.exec(str);
				QSqlQuery del_file;
				str.sprintf("DELETE FROM EquilResultFile WHERE EquilRstID = %d;", exp_rst.expRstID);
				del_file.exec(str);
				cur.del();
				reset();
				break;
			}	
			case 1:
			{
				break;
			}
		}
	}
		
}
void US_DB_RST_Equil::reset()
{
	load_flag = false;
	query_flag = false;
	cell_flag = false;
	del_flag = false;
	runinf_flag = false;
	vhw_flag = false;
	
	run_id = "";
	source_type = -1;
	runrequestID = 0;
	exp_rst.expRstID = -1;
	exp_rst.invID = -1;
	exp_rst.date = "";
	exp_rst.description = "";
	for(int i=0; i<8; i++)
	{
		exp_rst.CellID[i] = 0;
		for(int j=0; j<4; j++)
		{
			exp_rst.EDid[i][j] = -1;
		}
	}
	pb_runrequest->setEnabled(true);
	lbl_runrequest->setText(" Not Selected");
	lbl_expdt->setText("");
	lbl_invst->setText("");
	lbl_dt->setText("");
	le_dscrpt->setText("");
	lbl_plt1->setText("");
	lbl_plt2->setText("");
	lbl_plt3->setText("");
	lbl_plt4->setText("");
	lbl_plt5->setText("");
	lbl_plt6->setText("");
	pb_runinfo->setEnabled(false);
	pb_dataset->setEnabled(false);
	instr_lbl->setText("Instruction");
	lb_result->clear();
	lb_result->insertItem(tr("Select above buttons to load result"));
	lb_result->insertItem(tr("data from harddrive or database"));
	cmbb_cell->clear();
	cmbb_cell->insertItem(" No Data");
	update();	
}

/*! Open a netscape browser to load help page.*/
void US_DB_RST_Equil::help()
{
	US_Help *online_help; 
	online_help = new US_Help(this);
	online_help->show_help("manual/us_db_result_veloc.html");

}

/*! Close the interface.*/
void US_DB_RST_Equil::quit()
{
	remove_temp_dir("temp");
	remove_temp_dir("db_temp");
	close();
}

/*!
	This event handler, for event <var>e</var>, to receive widget close events.
*/
void US_DB_RST_Equil::closeEvent(QCloseEvent *e)
{
	remove_temp_dir("temp");
	remove_temp_dir("db_temp");
	e->accept();
}

/*! 
	Processes the paint event.\n
*/  
void US_DB_RST_Equil::paintEvent( QPaintEvent * )
{
/*	xpos = X;
	ypos = Y;

	if(cell_flag)
	{	
		ct->setGeometry(border, ypos, buttonw*4, buttonh*8);
		ct->show();
		ypos += buttonh*8 + border;
		Xpos = geometry().x();
		Ypos = geometry().y();
		setGeometry(Xpos, Ypos, xpos, ypos);
	}
	if(!cell_flag)
	{
		Xpos = geometry().x();
		Ypos = geometry().y();
		setGeometry(Xpos, Ypos, xpos, ypos);
	}
*/
}

