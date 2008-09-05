#include "../include/us_db_rst_veloc.h"

US_DB_RST_Veloc::US_DB_RST_Veloc(QWidget *p, const char *name) : US_DB(p, name)
{
	exp_rst.expRstID = -1;
	load_flag = false;
	query_flag = false;
	cell_flag = false;
	del_flag = false;
	runinf_flag = false;
	vhw_flag = false;
	mw_flag = false;
	source_type = -1;
	runrequestID = 0;
	for(int i=0; i<8;i++)
	{
		exp_rst.EDid[i][0] = -1;
		exp_rst.VHWid[i][0] = -1;
		exp_rst.FEid[i][0] = -1;
		exp_rst.CSid[i][0] = -1;
		exp_rst.SMid[i][0] = -1;
		exp_rst.TDid[i][0] = -1;
		exp_rst.RDid[i][0] = -1;
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
	lb_result->insertItem(tr("harddrive or Database"));
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
//      lbl_runrequest->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);	
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
//      lbl_invst->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
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
//      lbl_runinf->setFrameStyle(QFrame::WinPanel|Raised);
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

	lbl_VHW = new QLabel(tr(" van Holde - Weischet Combined Distribution :"),this);
	lbl_VHW->setAlignment(AlignHCenter|AlignVCenter);
//      lbl_VHW->setFrameStyle(QFrame::WinPanel|Raised);
	lbl_VHW->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_VHW->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	pb_vhw = new QPushButton(tr("Show vH-W"), this);
	pb_vhw->setAutoDefault(false);
	pb_vhw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_vhw->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_vhw->setEnabled(false);
	connect(pb_vhw, SIGNAL(clicked()), SLOT(show_vhw()));

	lbl_VHW1 = new QLabel(tr(" van Holde - Weischet combined Distribution Plot :"),this);
	lbl_VHW1->setAlignment(AlignLeft|AlignVCenter);
	lbl_VHW1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_VHW1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_vhw1= new QLabel("",this);
	lbl_vhw1->setAlignment(AlignLeft|AlignVCenter);
	lbl_vhw1->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_vhw1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_VHW2 = new QLabel(tr(" ASCII File for vH-W Distribution Plot Data :"), this);
	lbl_VHW2->setAlignment(AlignLeft|AlignVCenter);
	lbl_VHW2->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_VHW2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_vhw2 = new QLabel("",this);
	lbl_vhw2->setAlignment(AlignLeft|AlignVCenter);
	lbl_vhw2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_vhw2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_VHW3 = new QLabel(tr(" Text Listing for included Data sets :"), this);
	lbl_VHW3->setAlignment(AlignLeft|AlignVCenter);
	lbl_VHW3->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_VHW3->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_vhw3= new QLabel("",this);
	lbl_vhw3->setAlignment(AlignLeft|AlignVCenter);
	lbl_vhw3->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_vhw3->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_MW = new QLabel(tr(" Molecular Weight Distribution :"),this);
	lbl_MW->setAlignment(AlignHCenter|AlignVCenter);
//      lbl_MW->setFrameStyle(QFrame::WinPanel|Raised);
	lbl_MW->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_MW->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	pb_mw = new QPushButton(tr("Show MW"), this);
	pb_mw->setAutoDefault(false);
	pb_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_mw->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_mw->setEnabled(false);
	connect(pb_mw, SIGNAL(clicked()), SLOT(show_mw()));

	lbl_MW1 = new QLabel(tr(" Molecular Weight Distribution Plot :"),this);
	lbl_MW1->setAlignment(AlignLeft|AlignVCenter);
	lbl_MW1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_MW1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_mw1= new QLabel("",this);
	lbl_mw1->setAlignment(AlignLeft|AlignVCenter);
	lbl_mw1->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_mw1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_MW2 = new QLabel(tr(" ASCII File for MW Distribution Plot Data :"), this);
	lbl_MW2->setAlignment(AlignLeft|AlignVCenter);
	lbl_MW2->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_MW2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_mw2 = new QLabel("",this);
	lbl_mw2->setAlignment(AlignLeft|AlignVCenter);
	lbl_mw2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_mw2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_MW3 = new QLabel(tr(" Text Listing for included Data sets :"), this);
	lbl_MW3->setAlignment(AlignLeft|AlignVCenter);
	lbl_MW3->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_MW3->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_mw3= new QLabel("",this);
	lbl_mw3->setAlignment(AlignLeft|AlignVCenter);
	lbl_mw3->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_mw3->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	  
	lbl_Cell = new QLabel(tr(" Cell Result Data :"),this);
	lbl_Cell->setAlignment(AlignHCenter|AlignVCenter);
//      lbl_Cell->setFrameStyle(QFrame::WinPanel|Raised);
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
	lbl_VHW->setGeometry(xpos, ypos, buttonw*3 + spacing*2, buttonh);

	xpos += buttonw*3 +spacing*2;
	pb_vhw->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos = border;
	ypos += buttonh + spacing;
	lbl_VHW1->setGeometry(xpos, ypos, buttonw*3+spacing, buttonh);

	xpos += buttonw*3 + spacing*2;
	lbl_vhw1->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos = border;
	ypos += buttonh + spacing;
	lbl_VHW2->setGeometry(xpos, ypos, buttonw*3+spacing, buttonh);

	xpos += buttonw*3 + spacing*2;
	lbl_vhw2->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos = border;
	ypos += buttonh + spacing;
	lbl_VHW3->setGeometry(xpos, ypos, buttonw*3+spacing, buttonh);

	xpos += buttonw*3 + spacing*2;
	lbl_vhw3->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos = border;
	ypos += buttonh + spacing;
	lbl_MW->setGeometry(xpos, ypos, buttonw*3 + spacing*2, buttonh);

	xpos += buttonw*3 +spacing*2;
	pb_mw->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos = border;
	ypos += buttonh + spacing;
	lbl_MW1->setGeometry(xpos, ypos, buttonw*3+spacing, buttonh);

	xpos += buttonw*3 + spacing*2;
	lbl_mw1->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos = border;
	ypos += buttonh + spacing;
	lbl_MW2->setGeometry(xpos, ypos, buttonw*3+spacing, buttonh);

	xpos += buttonw*3 + spacing*2;
	lbl_mw2->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos = border;
	ypos += buttonh + spacing;
	lbl_MW3->setGeometry(xpos, ypos, buttonw*3+spacing, buttonh);

	xpos += buttonw*3 + spacing*2;
	lbl_mw3->setGeometry(xpos, ypos, buttonw, buttonh);


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

US_DB_RST_Veloc::~US_DB_RST_Veloc()
{
}

void US_DB_RST_Veloc::sel_runrequest()
{
	US_DB_RunRequest *runrequest_dlg;
	runrequest_dlg = new US_DB_RunRequest();
	runrequest_dlg->setCaption("Run Request Information");
	runrequest_dlg->show();
	connect(runrequest_dlg, SIGNAL(issue_RRID(int)), SLOT(update_runrequest_lbl(int)));
}

void US_DB_RST_Veloc::update_runrequest_lbl (int RRID)
{
	runrequestID = RRID;
	lbl_runrequest->setNum(runrequestID);
	if(runrequestID == 0)
	{
		lbl_runrequest->setText(" Not Selected");
	}
}

void US_DB_RST_Veloc::load()
{
	reset();
	data_control = new Data_Control_W(7);		//velocity data
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
	ts << tr("</ul>\n<p><b>Run Information (loaded from harddrive) :</b>\n<ul>\n");
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
	if(runinf_flag)
	{
		pb_runinfo->setEnabled(true);
	}
	f_runinfo.close();
// van Holde - Weischet Combined Distribution:
	QString vhw_file = make_tempFile(dirName, "e_vhw.html");
	QFile f_vhw(vhw_file);
	f_vhw.open(IO_WriteOnly | IO_Translate);
	QTextStream ds(&f_vhw);
	ds << tr("</ul>\n<p><b>van Holde - Weischet Combined Distribution(Load from hard drive) :</b>\n<ul>\n");	
	fileName = htmlDir + "/distribution.png";
	testfile.setName(fileName);
	if (testfile.exists())
	{
		ds << "   <li><a href="<<fileName << tr(">van Holde - Weischet combined Distribution Plot</a>\n");
		lbl_vhw1->setText(" available");
		vhw_flag = true;
	}
	else
	{
		lbl_vhw1->setText(" not available");
	}
	fileName = htmlDir + "/distrib.dat";
	testfile.setName(fileName);
	if (testfile.exists())
	{
		ds << "   <li><a href="<<fileName<< tr(">ASCII File for Distribution Plot Data</a>\n");
		lbl_vhw2->setText(" available");
		vhw_flag = true;
	}
	else
	{
		lbl_vhw2->setText(" not available");
	}
	fileName = htmlDir + "/distrib.txt";
	testfile.setName(fileName);
	if (testfile.exists())
	{
		ds << "   <li><a href="<<fileName << tr(">Text Listing for included Data sets</a>\n");
		lbl_vhw3->setText(" available");
		vhw_flag = true;
	}
	else
	{
		lbl_vhw3->setText(" not available");
	}
	f_vhw.close();
	if(vhw_flag)
	{
		pb_vhw->setEnabled(true);
	}
// Molecular Weight Distribution:
	QString mw_file = make_tempFile(dirName, "e_mw.html");
	QFile f_mw(mw_file);
	f_mw.open(IO_WriteOnly | IO_Translate);
	QTextStream mw(&f_mw);
	mw << tr("</ul>\n<p><b>Molecular Weight Distribution(Load from hard drive) :</b>\n<ul>\n");	
	fileName = htmlDir + "/mw_distribution.png";
	testfile.setName(fileName);
	if (testfile.exists())
	{
		mw << "   <li><a href="<<fileName << tr(">Molecular Weight Distribution Plot</a>\n");
		lbl_mw1->setText(" available");
		mw_flag = true;
	}
	else
	{
		lbl_mw1->setText(" not available");
	}
	fileName = htmlDir + "/mw_distrib.dat";
	testfile.setName(fileName);
	if (testfile.exists())
	{
		mw << "   <li><a href="<<fileName<< tr(">ASCII File for MW Distribution Plot Data</a>\n");
		lbl_mw2->setText(" available");
		mw_flag = true;
	}
	else
	{
		lbl_mw2->setText(" not available");
	}
	fileName = htmlDir + "/mw_distrib.txt";
	testfile.setName(fileName);
	if (testfile.exists())
	{
		mw << "   <li><a href="<<fileName << tr(">Text Listing for included Data sets</a>\n");
		lbl_mw3->setText(" available");
		mw_flag = true;
	}
	else
	{
		lbl_mw3->setText(" not available");
	}
	f_mw.close();
	if(mw_flag)
	{
		pb_mw->setEnabled(true);
	}

	source_type = 0;		//from HD
	lb_result->clear();
	lb_result->insertItem(tr("The shown data is loaded from the hard drive"));
	load_flag = true;
	load_cell();
}
void US_DB_RST_Veloc::load_cell()
{
	QString str, cell_Str, cellid_Str;
	cmbb_cell->clear();
db_connect();
// get CellID and cell description from DB (search key = exp_rst.expRstID)
	QSqlCursor cur( "tblExpData" );
	str.sprintf("ExpdataID=%d",exp_rst.expRstID);
		cur.select(str);
	if(cur.next())
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
	else
	{
		str = "The raw experimental data used for this result\n";
		str+= "file was not retrieved from the database.\n\n";
		str+= "You have to frist commit the raw data to the database,\n";
		str+= "retrieve it and edit it. Then result files can be\n";
		str+= "committed to the database\n";
		QMessageBox::message("UltraScan Error:", str);
		exit(0);
	}
}
void US_DB_RST_Veloc::update_cell()
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
void US_DB_RST_Veloc::sel_cell(int i)
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
			ct = new US_DB_RST_Cell(run_id, text, source_type, this);
				ct->setGeometry(border, ypos, buttonw*4, buttonh*11);
	  		ct->show();
		  // update();
		}
	}
	else
	{
		cell_flag = false;
	}
}

void US_DB_RST_Veloc::show_runinfo()
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
void US_DB_RST_Veloc::show_vhw()
{
	QString display_file;
	if(source_type == 0)
	{ 
		display_file = USglobal->config_list.html_dir +"/temp/e_vhw.html";
	}
	if(source_type == 1)
	{ 
		display_file = USglobal->config_list.html_dir +"/db_temp/e_vhw.html";
	}
	US_Help *online_help; 
	online_help = new US_Help(this);
	online_help->show_html_file(display_file);
}
	void US_DB_RST_Veloc::show_mw()
{
	QString display_file;
	if(source_type == 0)
	{ 
		display_file = USglobal->config_list.html_dir +"/temp/e_mw.html";
	}
	if(source_type == 1)
	{ 
		display_file = USglobal->config_list.html_dir +"/db_temp/e_mw.html";
	}
	US_Help *online_help; 
	online_help = new US_Help(this);
	online_help->show_html_file(display_file);
}

void US_DB_RST_Veloc::show_dataset()
{
	QString fileName = USglobal->config_list.html_dir+"/db_temp/velocData.tar.gz";
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

void US_DB_RST_Veloc::save_db()
{
	if(!load_flag)
	{
		QMessageBox::message(tr("Attention:"), 
								  tr("Please load result data from harddrive.\n"));
		return;
	}
	QString str, STR, cellid_str;
	QSqlCursor cur( "VelocResult");
	str.sprintf("VelocRstID=%d", exp_rst.expRstID);
	cur.select(str);
	if(cur.next())
	{
		QMessageBox::message(tr("Attention:"), 
								  tr("The result data are already stored in the database."));
		return;
	}
	for(int i=0; i<8; i++)
	{
		if(exp_rst.CellID[i] > 0)
		{	
			exp_rst.EDid[i][0] = save_db_expdata(i);
			exp_rst.VHWid[i][0] = save_db_vhw(i);
			exp_rst.CSid[i][0] = save_db_cs(i);
			exp_rst.FEid[i][0] = save_db_fe(i);
			exp_rst.SMid[i][0] = save_db_sm(i);
			exp_rst.TDid[i][0] = save_db_td(i);
			exp_rst.RDid[i][0] = save_db_rd(i);
		// Save VelocCellResult table			
			STR = "INSERT INTO VelocCellResult (CellRstID, VelocRstID, ";
			if(exp_rst.EDid[i][0] > 0)
			{
				STR += "EDID, ";
			}
			if(exp_rst.VHWid[i][0] > 0)
			{
				STR += "VHWID, ";
			}
			 	if(exp_rst.CSid[i][0] > 0)
			{
				STR += "CSID, ";
			}
			if(exp_rst.FEid[i][0] > 0)
			{
				STR += "FEID, ";
			}
			if(exp_rst.SMid[i][0] > 0)
			{
				STR += "SMID, ";
			}
			if(exp_rst.TDid[i][0] > 0)
			{
				STR += "TDID, ";
			}
			if(exp_rst.RDid[i][0] > 0)
			{
				STR += "RDID, ";
			}
			STR += "Description) VALUES (";
			STR += QString::number(exp_rst.CellID[i]) + ", ";
			STR += QString::number(exp_rst.expRstID) + ", ";
			if(exp_rst.EDid[i][0] > 0)
			{
				STR += QString::number(exp_rst.EDid[i][0]) + ", ";
			}
			if(exp_rst.VHWid[i][0] > 0)
			{
				STR += QString::number(exp_rst.VHWid[i][0]) + ", ";
			}
				if(exp_rst.CSid[i][0] > 0)
			{
				STR += QString::number(exp_rst.CSid[i][0]) + ", ";
			}
			if(exp_rst.FEid[i][0] > 0)
			{
				STR += QString::number(exp_rst.FEid[i][0]) + ", ";
			}
			if(exp_rst.SMid[i][0] > 0)
			{
				STR += QString::number(exp_rst.SMid[i][0]) + ", ";
			}
			if(exp_rst.TDid[i][0] > 0)
			{
				STR += QString::number(exp_rst.TDid[i][0]) + ", ";
			}
			if(exp_rst.RDid[i][0] > 0)
			{
				STR += QString::number(exp_rst.RDid[i][0]) + ", ";
			}
			STR += "'" + exp_rst.Cell[i] + "');";
			QSqlQuery target;
			target.exec(STR);
		}
	}
	switch(QMessageBox::information(this, tr("Save this entry?"), 
									tr("Clicking 'OK' will save result data to database"),
									tr("OK"), tr("CANCEL"),	0,1))
	{
		case 0:
		{
			//Insert into VelocResult
			STR = "INSERT INTO VelocResult (VelocRstID, InvID, Date, Description";
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
		
			/***********************************************************/	
			//Insert into VelocResultFile
		 	if(!insertCompressData())
			{
				QMessageBox::message(tr("Attention:"), tr("Insert compress velocity data failed.\n"));
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
					str.sprintf("UPDATE tblResult SET InvestigatorID = %d, VelocDataID = %d WHERE ResultID = %d;", exp_rst.invID, exp_rst.expRstID, resultID);
					query.exec(str);
				}
				else		//create new one
				{
					resultID = get_newID("tblResult","ResultID");
					str = "INSERT INTO tblResult (ResultID, InvestigatorID, RunRequestID, VelocDataID) VALUES(";
					str += QString::number(resultID)+", "+ QString::number(exp_rst.invID)+", " + QString::number(runrequestID) +", " + QString::number(exp_rst.expRstID)+");";
					bool flag = query.exec(str);
					if(!flag)
					{
						QMessageBox::message(tr("Attention:"), 
											     tr("Saving data to the result database table failed.\n"));
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

bool US_DB_RST_Veloc::insertCompressData()
{
	compress_proc = new QProcess(this);
	c_step=0;
	connect(compress_proc, SIGNAL(processExited()), this, SLOT(endCompressProcess()));
	if(!compress_proc->start())
	{
		QMessageBox::message("UltraScan Error:", "Unable to start process to compress velocity datafile.");
		return false;
	}
  	return true;
}

void US_DB_RST_Veloc::endCompressProcess()
{
	QDir work_dir(USglobal->config_list.result_dir);
	QString tarfile = data_control->run_inf.run_id + "_veloc.tar";
				
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
			fileName = data_control->run_inf.run_id + ".us.v";
			cmd.append(fileName);
			for(int i=0; i<8; i++)		//cell
			{
				for(int j=0; j<3; j++)		//wavelength
				{
					fileName = data_control->run_inf.run_id + ".veloc." + QString::number(i+1) + QString::number(j+1);
					QString  str = USglobal->config_list.result_dir + "/" + fileName;                    
					QFile testFile(str);
					if(testFile.exists())
					{
						cmd.append(fileName);
					}				
				}
			}
			compress_proc->setArguments(cmd);
			c_step=1;
			if(!compress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to tar data archive.");
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
				QMessageBox::message("UltraScan Error:", "Unable to start process to gzip velocity datafile.");
				return;
			}	
			break;		
		}
		case 2:		//insert to DB
		{
			//Insert into VelocResultFile
			QSqlCursor cur_f( "VelocResultFile");
			cur_f.setMode( QSqlCursor::Insert);
			QSqlRecord *buffer_f = cur_f.primeInsert();
			buffer_f->setValue( "VelocRstID", exp_rst.expRstID );
			fileName = htmlDir + "/temperature_data.png";
			write_blob(fileName, buffer_f, "RunInfo1");
			fileName = htmlDir + "/rotorspeed_data.png";
			write_blob(fileName, buffer_f, "RunInfo2");
			fileName = htmlDir + "/time_data.png";
			write_blob(fileName, buffer_f, "RunInfo3");
			fileName = htmlDir + "/distribution.png";
			write_blob(fileName, buffer_f, "VHW1");
			fileName = htmlDir + "/distrib.dat";
			write_blob(fileName, buffer_f, "VHW2");
			fileName = htmlDir + "/distrib.txt";
			write_blob(fileName, buffer_f, "VHW3");
			fileName = htmlDir + "/mw_distribution.png";
			write_blob(fileName, buffer_f, "MW1");
			fileName = htmlDir + "/mw_distrib.dat";
			write_blob(fileName, buffer_f, "MW2");
			fileName = htmlDir + "/mw_distrib.txt";
			write_blob(fileName, buffer_f, "MW3");
			fileName = USglobal->config_list.result_dir + "/"+data_control->run_inf.run_id + "_veloc.tar.gz";
			write_blob(fileName, buffer_f, "DataFile");
			cur_f.insert();
			QFile tempFile(fileName);
			if(tempFile.exists())
			{
				tempFile.remove();
			}
			delete compress_proc;
		}
	}
}

/*! 
	Clean CellResult and other six cell result tables that already save to db.
*/
void US_DB_RST_Veloc::del_subTable()
{
	for(int i=0; i<8; i++)
	{
		if(exp_rst.CellID[i] > 0)
		{
		// channel 1~4
			for(int j=0; j<4;j++)
			{
				clean_db("VelocExpData", "EDID","VelocExpDataTable", "EDtableID",exp_rst.EDid[i][j]);
				clean_db("VelocVHW","VHWID","VelocVHWTable","VHWtableID",exp_rst.VHWid[i][j]);
					clean_db("VelocCS","CSID","VelocCSTable","CStableID",exp_rst.CSid[i][j]);
				clean_db("VelocFiniteElement","FEID","VelocFiniteElementTable","FEtableID",exp_rst.FEid[i][j]);
				clean_db("VelocSecondMoment","SMID","VelocSecondMomentTable","SMtableID",exp_rst.SMid[i][j]);
				clean_db("VelocTimeDerivative","TDID","VelocTimeDerivativeTable","TDtableID",exp_rst.TDid[i][j]);
				clean_db("VelocRadialDerivative","RDID","VelocRadialDerivativeTable","RDtableID",exp_rst.RDid[i][j]);
			}
			clean_db("VelocCellResult","CellRstID","","",exp_rst.CellID[i]);
		}
	}
}

int US_DB_RST_Veloc::insert_upTable(QString tablename, QString key, int *value)
{
	int id, pos;
	QString str, STR;
	if(key == "VHWID")
	{
		pos = 3;
	}
	else
	{
		pos = 2;
	}
	id = get_newID(tablename, key);
	STR = "INSERT INTO " + tablename +" (" + key;
	if(value[0]>0)
	{
		str = key.insert(pos, "table");
		str+= "_WL1";
		STR += ", " + str;
	}
	if(value[1]>0)
	{
		str = key.insert(pos, "table");
		str+= "_WL2";
		STR += ", " + str; 
	}
	if(value[2]>0)
	{
		str = key.insert(pos, "table");
		str+= "_WL3";
		STR += ", " + str; 
	}
	STR +=") VALUES(";

	STR += QString::number(id);
	if(value[0]>0)
	{
		STR +=", "+ QString::number(value[0]);
	}
	if(value[1]>0)
	{
		STR +=", "+ QString::number(value[1]);
	}
	if(value[2]>0)
	{
		STR +=", "+ QString::number(value[2]);
	}
	STR +=");";
	QSqlQuery target;
	target.exec(STR);
	return id;
}

int US_DB_RST_Veloc::save_db_expdata(int cell)
{
	QString str;

	bool flag = false;	// To check whether cell expdata result exist.
	int id = -1;
	int tableId[3];
	int i = cell;

	for  (int j=0; j<3; j++)			// 3 wavelength
	{
		tableId[j] = -1;			// Initialize to -1
		if (data_control->run_inf.scans[i][j] != 0)
		{
			QSqlCursor cur_t( "VelocExpDataTable");
			cur_t.setMode( QSqlCursor::Insert);
			QSqlRecord *buffer_t = cur_t.primeInsert();
		// Raw Experimental Data Plot Image:			
			str.sprintf(htmlDir + "/raw_%d%d.png", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "EDPlot");
			else
				write_blob(str, buffer_t, "EDPlot");
		// Raw Data Ascii Data:			
			str.sprintf(baseName + "raw.%d%d1", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "F_EDPlot");
			else
				write_blob(str, buffer_t, "F_EDPlot");
		// Peptide data
			str.sprintf(USglobal->config_list.result_dir + "/" + QString::number(data_control->run_inf.peptide_serialnumber[i][0][0]) + ".pep_res");
			if(!flag)
				flag = write_blob(str, buffer_t, "PepFile");
			else
				write_blob(str, buffer_t, "PepFile");
		// Buffer data
			str.sprintf(USglobal->config_list.result_dir + "/" + QString::number(data_control->run_inf.buffer_serialnumber[i][0]) + ".buf_res");
			if(!flag)
				flag = write_blob(str, buffer_t, "BufFile");
			else
				write_blob(str, buffer_t, "BufFile");
		//only data exists then do inserting to "VelocExpDataTable"		
			if(flag)	
			{
				tableId[j] = get_newID("VelocExpDataTable","EDtableID");
				buffer_t->setValue("EDtableID", tableId[j]);
				cur_t.insert();
			}
		}
	}
//only data exists then do inserting to "VelocExpData"
	if(flag)			
	{
		id = insert_upTable("VelocExpData", "EDID", tableId);
	}
	return id;
}

int US_DB_RST_Veloc::save_db_vhw(int cell)
{
	QString str;
	bool flag = false;	// To check whether cell VHW result exist.
	int id = -1;
	int tableId[3];
	int i = cell;
	for  (int j=0; j<3; j++)
	{
		tableId[j] = -1;			// Initialize to -1
		if (data_control->run_inf.scans[i][j] != 0)
		{
			QSqlCursor cur_t( "VelocVHWTable");
			cur_t.setMode( QSqlCursor::Insert);
			QSqlRecord *buffer_t = cur_t.primeInsert();
		// vHW Anaysis Report:
			str.sprintf(baseName + "vhw_res.%d%d", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "Report");
			else
				write_blob(str, buffer_t, "Report");
		// Extrapolation Plot Image:		
			str.sprintf(htmlDir + "/vhw_ext_%d%d.png", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "ExtraPlot");
			else
				write_blob(str, buffer_t, "ExtraPlot");
		// Extrapolation Plot Data:		
			str.sprintf(baseName + "vhw_ext.%d%d", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "F_ExtraPlot");
			else
				write_blob(str, buffer_t, "F_ExtraPlot");
		// Distribution Plot Image:
			str.sprintf(htmlDir + "/vhw_dis_%d%d.png", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "DistrPlot");
			else
				write_blob(str, buffer_t, "DistrPlot");
		// Distribution Plot Data:
			str.sprintf(baseName + "vhw_dis.%d%d", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "F_DistrPlot");
			else
				write_blob(str, buffer_t, "F_DistrPlot");
		// Histogram Plot Image:
			str.sprintf(htmlDir + "/vhw_his_%d%d.png", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "HisPlot");
			else
				write_blob(str, buffer_t, "HisPlot");
		// Histogram Plot Data:
			str.sprintf(htmlDir + "/vhw_his_%d%d.dat", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "F_HisPlot");
			else
				write_blob(str, buffer_t, "F_HisPlot");
		// Edited Data Plot Image:
			str.sprintf(htmlDir + "/vhw_edited_%d%d.png", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "EditedPlot");
			else
				write_blob(str, buffer_t, "EditedPlot");
			if(flag)		//only data exists then do inserting to "VelocVHWTable"
			{
				tableId[j] = get_newID("VelocVHWTable","VHWtableID");
				buffer_t->setValue("VHWtableID", tableId[j]);
				cur_t.insert();
			}		
		}
	}
	if(flag)		//only data exists then do inserting to "VelocVHW"
	{
		id = 	insert_upTable("VelocVHW", "VHWID", tableId);
	}
	return id;
}

	int US_DB_RST_Veloc::save_db_cs(int cell)
{
		 QString str;
	bool flag = false;	// To check whether cell VHW result exist.
	int id = -1;
	int tableId[3];
	int i = cell;

		for  (int j=0; j<3; j++)
	{
		tableId[j] = -1;			// Initialize to -1
		if (data_control->run_inf.scans[i][j] != 0)
		{
			QSqlCursor cur_t( "VelocCSTable");
			cur_t.setMode( QSqlCursor::Insert);
			QSqlRecord *buffer_t = cur_t.primeInsert();

			// C(s) Anaysis Report:
			str.sprintf(baseName + "cofs_res.%d%d", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "Report");
			else
				write_blob(str, buffer_t, "Report");
		// C(s) Distrubution Plot Image:		
			str.sprintf(htmlDir + "/cofs_dis_%d%d.png", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "DistrPlot");
			else
				write_blob(str, buffer_t, "DistrPlot");
		// C(s) Molecular Weight Distrubution Plot Image:		
			str.sprintf(htmlDir + "/cofs_mw_%d%d.png", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "MW_DistrPlot");
			else
				write_blob(str, buffer_t, "MW_DistrPlot");
		// Residuals Plot Image:
			str.sprintf(htmlDir + "/cofs_resid_%d%d.png", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "ResidPlot");
			else
				write_blob(str, buffer_t, "ResidPlot");
		// Residuals Pixel Map:
			str.sprintf(htmlDir + "/cofs_pixmap_%d%d.png", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "ResidPixmap");
			else
				write_blob(str, buffer_t, "ResidPixmap");
		// ASCII File of Distrubution Plot Data:
			str.sprintf(baseName + "cofs_dis.%d%d", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "F_DistrPlot");
			else
				write_blob(str, buffer_t, "F_DistrPlot");
		// Edited Data Plot Image:
			str.sprintf(htmlDir + "/cofs_edited_%d%d.png", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "EditedPlot");
			else
				write_blob(str, buffer_t, "EditedPlot");
			if(flag)		//only data exists then do inserting to "VelocCSTable"
			{
				tableId[j] = get_newID("VelocCSTable","CStableID");
				buffer_t->setValue("CStableID", tableId[j]);
				cur_t.insert();
			}		
			}
		}
		if(flag)		//only data exists then do inserting to "VelocCS"
	{
		id = 	insert_upTable("VelocCS", "CSID", tableId);
	}
	return id;

	}

int US_DB_RST_Veloc::save_db_fe(int cell)
{
	QString str;
		QFile testfile;
	QFileInfo f_info;
	bool flag = false;	// To check whether Veloc FE result exist.
		bool r_flag = false; //To check whether Residuals png file exist
		bool o_flag = false; //To check whether Overlay png file exist

	int id = -1;
	int tableId[3];
	int i = cell;

	for  (int j=0; j<3; j++)
	{
		tableId[j] = -1;			// Initialize to -1
		if (data_control->run_inf.scans[i][j] != 0)
		{
				
			QSqlCursor cur_t( "VelocFiniteElementTable");
			cur_t.setMode( QSqlCursor::Insert);
			QSqlRecord *buffer_t = cur_t.primeInsert();
		// Finite Element Anaysis Report:
			str.sprintf(baseName + "fef_res.%d%d", i+1, j+1);
				if(!flag)
				flag = write_blob(str, buffer_t, "Report");
			else
			  	write_blob(str, buffer_t, "Report");
		// Residuals Plot Image:
			QDir work_dir;
			QProcess *r_proc;
			r_proc = new QProcess(this);
			r_proc->clearArguments();
			work_dir.setPath(htmlDir);
			r_proc->setWorkingDirectory(work_dir);
			QStringList r_cmd;
			r_cmd.append("tar");
			r_cmd.append("-cvvf");
			r_cmd.append("fe_residuals.tar");
			for(unsigned int k=0; k<data_control->run_inf.scans[i][j]; k+=5)
			{
				str.sprintf("fef_%d%d_%d.png", i+1, j+1, k+1);
					testfile.setName(str);
					if (testfile.exists())
					{
						r_cmd.append(str);
						r_flag=true;
					}
			}
				if(r_flag) //at least one fef_xx_x.png file exists
				{
				r_proc->setArguments(r_cmd);
				if(!r_proc->start())
				{
			  	 	QMessageBox::message("UltraScan Error:", "Unable to start process to compress residuals image files.");
					exit(0);
				}
				int Size = 0;
				residual_file = htmlDir + "/fe_residuals.tar";
				while(!Size)	//wait qprocess finish compressing residual files
				{		
			  	 	f_info.setFile(residual_file);
					Size = f_info.size();
					//sleep(1);
				}
				if(!flag)
					flag = write_blob(residual_file, buffer_t, "Residuals");
				else
					write_blob(residual_file, buffer_t, "Residuals");
				}
		
		// Overlays Plot Image:
			QProcess *o_proc;
			o_proc = new QProcess(this);
			o_proc->clearArguments();
			work_dir.setPath(htmlDir);
			o_proc->setWorkingDirectory(work_dir);
			QStringList o_cmd;
			o_cmd.append("tar");
			o_cmd.append("-cvvf");
			o_cmd.append("fe_overlays.tar");
			for(unsigned int k=0; k<data_control->run_inf.scans[i][j]; k+=5)
			{
				str.sprintf("fef_edited_%d%d_%d.png", i+1, j+1, k+1);
					testfile.setName(str);
					if (testfile.exists())
					{
						o_cmd.append(str);
						o_flag=true;
					}
			}
				if(o_flag) //at least one fef_edited_xx_x.png file exists
				{

				o_proc->setArguments(o_cmd);
				if(!o_proc->start())
				{
					QMessageBox::message("UltraScan Error:", "Unable to start process to compress overlays image files.");
					exit(0);
				}
				overlay_file = htmlDir + "/fe_overlays.tar";
				int Size = 0;
				while(!Size)	//wait qprocess finish compressing overlay files
				{		
					f_info.setFile(overlay_file);
					Size = f_info.size();
					//sleep(1);
				}
				if(!flag)
					flag = write_blob(overlay_file, buffer_t, "Overlays");
				else
					write_blob(overlay_file, buffer_t, "Overlays");		
			}
		// Residuals ASCII Data:
			str.sprintf(baseName + "fef_residuals.%d%d", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "F_Residuals");
			else
				write_blob(str, buffer_t, "F_Residuals");
		// Raw ASCII Data:
			str.sprintf(baseName + "fef_raw.%d%d", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "F_Raw");
			else
				write_blob(str, buffer_t, "F_Raw");
		// Finite Element ASCII Data:
			str.sprintf(baseName + "fef_dat.%d%d", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "F_Solution");
			else
				write_blob(str, buffer_t, "F_Solution");
			if(flag)		//only data exists then do inserting to "VelocFiniteElementTable"
			{
				tableId[j] = get_newID("VelocFiniteElementTable","FEtableID");
				buffer_t->setValue("FEtableID", tableId[j]);
				cur_t.insert();
			}		
		}
	}
	if(flag)		//only data exists then do inserting to "VelocFiniteElement"
	{
		id = 	insert_upTable("VelocFiniteElement", "FEID", tableId);
	}
	return id;
}

	
int US_DB_RST_Veloc::save_db_sm(int cell)
{
	QString str;
	bool flag = false;	// To check whether Veloc second moment result exist.
	int id = -1;
	int tableId[3];
	int i = cell;

	for  (int j=0; j<3; j++)
	{
		tableId[j] = -1;			// Initialize to -1
		if (data_control->run_inf.scans[i][j] != 0)
		{
			QSqlCursor cur_t( "VelocSecondMomentTable");
			cur_t.setMode( QSqlCursor::Insert);
			QSqlRecord *buffer_t = cur_t.primeInsert();
		// Second Moment Anaysis Report:
			str.sprintf(baseName + "sm_res.%d%d", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "Report");
			else
				write_blob(str, buffer_t, "Report");
		// Second Moment Plot Image:
			str.sprintf(htmlDir + "/sm_%d%d.png", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "SMPlot");
			else
				write_blob(str, buffer_t, "SMPlot");
		// Second Moment Plot Data:
			str.sprintf(baseName + "sm_dat.%d%d", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "F_SMPlot");
			else
				write_blob(str, buffer_t, "F_SMPlot");
		// Second Moment Edited Data Plot Image:
			str.sprintf(htmlDir + "/sm_edited_%d%d.png", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "EditedPlot");
			else
				write_blob(str, buffer_t, "EditedPlot");
			if(flag)		//only data exists then do inserting to "VelocSecondMomentTable"
			{
				tableId[j] = get_newID("VelocSecondMomentTable","SMtableID");
				buffer_t->setValue("SMtableID", tableId[j]);
				cur_t.insert();
			}		
		}
	}
	if(flag)		//only data exists then do inserting to "VelocSecondMoment"
	{
		id = 	insert_upTable("VelocSecondMoment", "SMID", tableId);	
	}
	return id;
}

int US_DB_RST_Veloc::save_db_td(int cell)
{
	QString str;
	bool flag = false;	// To check whether Veloc time derivative result exist.
	int id = -1;
	int tableId[3];
	int i = cell;
	for  (int j=0; j<3; j++)
	{
		tableId[j] = -1;			// Initialize to -1
		if (data_control->run_inf.scans[i][j] != 0)
		{
			QSqlCursor cur_t( "VelocTimeDerivativeTable");
			cur_t.setMode( QSqlCursor::Insert);
			QSqlRecord *buffer_t = cur_t.primeInsert();
		// Time Derivative Anaysis Report:
			str.sprintf(baseName + "dcdt_res.%d%d", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "Report");
			else
				write_blob(str, buffer_t, "Report");
		// Time Derivative Plot Image:
			str.sprintf(htmlDir + "/dcdt_%d%d.png", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "TDPlot");
			else
				write_blob(str, buffer_t, "TDPlot");
		// Time Derivative Scan Plot Data:
			str.sprintf(baseName + "dcdt_scans.%d%d", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "F_ScanPlot");
			else
				write_blob(str, buffer_t, "F_ScanPlot");
		// Time Derivative S-value Plot Data:
			str.sprintf(baseName + "dcdt_sval.%d%d", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "F_SvaluePlot");
			else
				write_blob(str, buffer_t, "F_SvaluePlot");
		// Time Derivative Average Plot Image:
			str.sprintf(htmlDir + "/dcdt_avg_%d%d.png", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "AvgPlot");
			else
				write_blob(str, buffer_t, "AvgPlot");
		// Time Derivative S-value Average Plot Data:
			str.sprintf(baseName + "dcdt_avg.%d%d", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "F_AvgPlot");
			else
				write_blob(str, buffer_t, "F_AvgPlot");
		// Time Derivative Edited Data Plot Image:
			str.sprintf(htmlDir + "/dcdt_edited_%d%d.png", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "EditedPlot");
			else
				write_blob(str, buffer_t, "EditedPlot");
			if(flag)		//only data exists then do inserting to "VelocTimeDerivativeTable"
			{
				tableId[j] = get_newID("VelocTimeDerivativeTable","TDtableID");
				buffer_t->setValue("TDtableID", tableId[j]);
				cur_t.insert();
			}		
		}
	}
	if(flag)		//only data exists then do inserting to "VelocTimeDerivative"
	{
		id = 	insert_upTable("VelocTimeDerivative", "TDID", tableId);
	}
	return id;
}

int US_DB_RST_Veloc::save_db_rd(int cell)
{
	QString str;
	bool flag = false;	// To check whether Veloc radial derivative result exist.
	int id = -1;
	int tableId[3];
	int i = cell;

	for  (int j=0; j<3; j++)
	{
		tableId[j] = -1;			// Initialize to -1
		if (data_control->run_inf.scans[i][j] != 0)
		{
			QSqlCursor cur_t( "VelocRadialDerivativeTable");
			cur_t.setMode( QSqlCursor::Insert);
			QSqlRecord *buffer_t = cur_t.primeInsert();
		
		// Radial Derivative Anaysis Report:
			str.sprintf(baseName + "dcdr_res.%d%d", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "Report");
			else
				write_blob(str, buffer_t, "Report");
		// Radial Derivative Plot Image:
			str.sprintf(htmlDir + "/dcdr_%d%d.png", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "RDPlot");
			else
				write_blob(str, buffer_t, "RDPlot");
		// Radial Derivative Scan Plot Data:
			str.sprintf(baseName + "dcdr_scans.%d%d", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "F_ScanPlot");
			else
				write_blob(str, buffer_t, "F_ScanPlot");
		// Radial Derivative S-value Plot Data:
			str.sprintf(baseName + "dcdr_sval.%d%d", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "F_SvaluePlot");
			else
				write_blob(str, buffer_t, "F_SvaluePlot");
		// Radial Derivative Average Plot Image:
			str.sprintf(htmlDir + "/dcdr_avg_%d%d.png", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "AvgPlot");
			else
				write_blob(str, buffer_t, "AvgPlot");
		// Radial Derivative S-value Average Plot Data:
			str.sprintf(baseName + "dcdr_avg.%d%d", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "F_AvgPlot");
			else
				write_blob(str, buffer_t, "F_AvgPlot");
		// Radial Derivative Edited Data Plot Image:
			str.sprintf(htmlDir + "/dcdr_edited_%d%d.png", i+1, j+1);
			if(!flag)
				flag = write_blob(str, buffer_t, "EditedPlot");
			else
				write_blob(str, buffer_t, "EditedPlot");
			if(flag)		//only data exists then do inserting to "VelocRadialDerivativeTable"
			{		
				tableId[j] = get_newID("VelocRadialDerivativeTable","RDtableID");
				buffer_t->setValue("RDtableID", tableId[j]);
				cur_t.insert();	
			}		
		}
	}
	if(flag)	//only data exists then do inserting to "VelocRadialDerivative"
	{
		id = 	insert_upTable("VelocRadialDerivative", "RDID", tableId);
	}
	return id;
}

void US_DB_RST_Veloc::query_db()
{
	reset();
	QSqlCursor cur( "VelocResult" );
	QSqlIndex order = cur.index( "VelocRstID" );
	cur.select(order);

	int maxID = get_newID("VelocResult","VelocRstID");
	int count = 0;
	item_ExpdataID = new int[maxID];
	item_Description = new QString[maxID];
	display_Str = new QString[maxID];

	while(cur.next() )
	{
		int id = cur.value("VelocRstID").toInt();
		display_Str[count] = "VelocRstID ("+  QString::number( id ) + "): "+
			cur.value( "Description" ).toString();
		item_ExpdataID[count] = id;
		item_Description[count] = cur.value("Description").toString();
		count++;					
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

void US_DB_RST_Veloc::select_result(int item)
{
	bool flag;
	QString str, str1, str2, filename;
	QString dirName = USglobal->config_list.html_dir + "/db_temp/";
	if(query_flag)
	{
		exp_rst.expRstID = item_ExpdataID[item];
		exp_rst.description = item_Description[item];
	/**********************************************
			To query VelocResultFile DB table
	***********************************************/
		QSqlCursor cur_f( "VelocResultFile" );
		str.sprintf("VelocRstID = %d", exp_rst.expRstID);
		cur_f.select(str);		
		if(cur_f.next())
		{
		// Run Information:	
			QString runinfo_file = make_tempFile(dirName, "runinfo.html");
			QFile f_runinfo(runinfo_file);
			f_runinfo.open(IO_WriteOnly | IO_Translate);
			QTextStream ts(&f_runinfo);
			ts << tr("</ul>\n<p><b>Run Information(Load from database) :</b>\n<ul>\n");
		
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
			f_runinfo.close();
			if(runinf_flag)
			{
				pb_runinfo->setEnabled(true);
			}
		
		// van Holde - Weischet Combined Distribution:
			QString vhw_file = make_tempFile(dirName, "e_vhw.html");	
			QFile f_vhw(vhw_file);
			f_vhw.open(IO_WriteOnly | IO_Translate);
			QTextStream ds(&f_vhw);
			ds << tr("</ul>\n<p><b>Molecular Weight Distribution(Load from database) :</b>\n<ul>\n");
			filename = make_tempFile(dirName, "distribution.png");
			flag = read_blob("VHW1", cur_f, filename);
			if(flag)
			{
				ds << "   <li><a href="<<filename << tr(">van Holde - Weischet combined Distribution Plot</a>\n");
				lbl_vhw1->setText(" available");
				vhw_flag = true;
			}
			else
			{			
				lbl_vhw1->setText(" not available");
			}
			filename = make_tempFile(dirName, "distrib.dat");
			flag = read_blob("VHW2", cur_f, filename);
			if(flag)
			{
				ds << "   <li><a href="<<filename<< tr(">ASCII File for Distribution Plot Data</a>\n");
				lbl_vhw2->setText(" available");
				vhw_flag = true;
			}
			else
			{			
				lbl_vhw2->setText(" not available");
			}
			filename = make_tempFile(dirName, "distrib.txt");
			flag = read_blob("VHW3", cur_f, filename);
			if(flag)
			{
				ds << "   <li><a href="<<filename << tr(">Text Listing for included Data sets</a>\n");
				lbl_vhw3->setText(" available");
				vhw_flag = true;
			}
			else
			{			
				lbl_vhw3->setText(" not available");
			}
			f_vhw.close();
			if(vhw_flag)
			{
				pb_vhw->setEnabled(true);
			}

			// Molecular Weight Distribution:
			QString mw_file = make_tempFile(dirName, "e_mw.html");	
			QFile f_mw(mw_file);
			f_mw.open(IO_WriteOnly | IO_Translate);
			QTextStream mw(&f_mw);
			mw << tr("</ul>\n<p><b>Molecular Weight Distribution(Load from database) :</b>\n<ul>\n");
			filename = make_tempFile(dirName, "distribution.png");
			flag = read_blob("MW1", cur_f, filename);
			if(flag)
			{
				mw << "   <li><a href="<<filename << tr(">Molecular Weight Distribution Plot</a>\n");
				lbl_mw1->setText(" available");
				mw_flag = true;
			}
			else
			{			
				lbl_mw1->setText(" not available");
			}
			filename = make_tempFile(dirName, "distrib.dat");
			flag = read_blob("MW2", cur_f, filename);
			if(flag)
			{
				mw << "   <li><a href="<<filename<< tr(">ASCII File for MW Distribution Plot Data</a>\n");
				lbl_mw2->setText(" available");
				mw_flag = true;
			}
			else
			{			
				lbl_mw2->setText(" not available");
			}
			filename = make_tempFile(dirName, "distrib.txt");
			flag = read_blob("MW3", cur_f, filename);
			if(flag)
			{
				mw << "   <li><a href="<<filename << tr(">Text Listing for included Data sets</a>\n");
				lbl_mw3->setText(" available");
				mw_flag = true;
			}
			else
			{			
				lbl_mw3->setText(" not available");
			}
			f_mw.close();
			if(mw_flag)
			{
				pb_mw->setEnabled(true);
			}
				//raw result data file
				filename = make_tempFile(dirName, "velocData.tar.gz");
				flag = read_blob("DataFile", cur_f, filename);
				if(flag)
				{
					pb_dataset->setEnabled(true);
				}
		}
	//get Cell 1~8 ID
	/**********************************************
			To query VelocResult DB table
	***********************************************/
		QSqlCursor cur( "VelocResult" );
		str.sprintf("VelocRstID = %d", exp_rst.expRstID);
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
			To query VelocCellResult DB table
		***********************************************/
			QSqlCursor get( "VelocCellResult" );
			QString get_cellid, str;
			for(int i=0; i<8; i++)
			{
				get_cellid.sprintf("CellRstID = %d",exp_rst.CellID[i]);
				get.select(get_cellid);
				if(get.next())
				{
					exp_rst.EDid[i][0] = get.value("EDID").toInt();
					exp_rst.VHWid[i][0] = get.value("VHWID").toInt();
						exp_rst.CSid[i][0] = get.value("CSID").toInt();
					exp_rst.FEid[i][0] = get.value("FEID").toInt();
					exp_rst.SMid[i][0] = get.value("SMID").toInt();
					exp_rst.TDid[i][0] = get.value("TDID").toInt();
					exp_rst.RDid[i][0] = get.value("RDID").toInt();
					exp_rst.Cell[i] = get.value("Description").toString();
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
void US_DB_RST_Veloc::check_permission()
{
	US_DB_Admin *db_admin;
	db_admin = new US_DB_Admin("");
	db_admin->show();
	connect(db_admin, SIGNAL(issue_permission(bool)), SLOT(delete_db(bool)));
}

/*!
	Delete one selected experimental result table entry.
*/
void US_DB_RST_Veloc::delete_db(bool permission)
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
								  tr("Please select the record to\n"
									 "be deleted from the database"));
	}
	else
	{
		QSqlCursor cur( "VelocResult");
		cur.setMode( QSqlCursor::Delete);
		QSqlIndex filter = cur.index("VelocRstID");
		cur.setValue("VelocRstID", exp_rst.expRstID);
		cur.primeDelete();
	
		switch(QMessageBox::information(this, tr("Delete this record?"), 
										tr("Clicking 'OK' will delete the selected result from the database"),
										tr("OK"), tr("CANCEL"),	0,1))
		{
			case 0:
				{
					del_subTable();
					QSqlQuery query;
					str.sprintf("UPDATE tblResult SET VelocDataID = NULL WHERE VelocDataID = %d;", exp_rst.expRstID);
					query.exec(str);
					QSqlQuery del_file;
					str.sprintf("DELETE FROM VelocResultFile WHERE VelocRstID = %d;", exp_rst.expRstID);
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
void US_DB_RST_Veloc::reset()
{
	load_flag = false;
	query_flag = false;
	cell_flag = false;
	del_flag = false;
	runinf_flag = false;
	vhw_flag = false;
		mw_flag = false;
	source_type = -1;
	runrequestID = 0;
	exp_rst.expRstID = -1;
	exp_rst.invID = -1;
	exp_rst.date = "";
	exp_rst.description = "";
	for(int i=0; i<8; i++)
	{
		exp_rst.CellID[i] = 0;
	//	exp_rst.Cell[i] ="";
		for(int j=0; j<4; j++)
		{
			exp_rst.EDid[i][j] = -1;
			exp_rst.VHWid[i][j] = -1;
			exp_rst.FEid[i][j] = -1;
			exp_rst.SMid[i][j] = -1;
			exp_rst.TDid[i][j] = -1;
			exp_rst.RDid[i][j] = -1;
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
	lbl_vhw1->setText("");
	lbl_vhw2->setText("");
	lbl_vhw3->setText("");
		lbl_mw1->setText("");
	lbl_mw2->setText("");
	lbl_mw3->setText("");
	pb_runinfo->setEnabled(false);
	pb_vhw->setEnabled(false);
		pb_mw->setEnabled(false);
		pb_dataset->setEnabled(false);
	instr_lbl->setText("Instruction");
	lb_result->clear();
	lb_result->insertItem(tr("Click one of above buttons to load result from"));
	lb_result->insertItem(tr("harddrive or Database"));
	cmbb_cell->clear();
	cmbb_cell->insertItem(" No Data");
	update();
}

/*! Open a netscape browser to load help page.*/
void US_DB_RST_Veloc::help()
{
	US_Help *online_help; 
	online_help = new US_Help(this);
	online_help->show_help("manual/us_db_result_veloc.html");

}

/*! Close the interface.*/
void US_DB_RST_Veloc::quit()
{
	remove_temp_dir("temp");
	remove_temp_dir("db_temp");
	close();
}

/*!
	This event handler, for event <var>e</var>, to receive widget close events.
*/
void US_DB_RST_Veloc::closeEvent(QCloseEvent *e)
{
	remove_temp_dir("temp");
	remove_temp_dir("db_temp");
	e->accept();
}


void US_DB_RST_Veloc::paintEvent( QPaintEvent *)
{
 /*     xpos = X;
	ypos = Y;

	if(cell_flag)
	{	
		ct->setGeometry(border, ypos, buttonw*4, buttonh*11);
		ct->show();
		ypos += buttonh*11 + border;
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
 */  }
