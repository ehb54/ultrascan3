#include "../include/us_db_tbl_expdata.h"

//! Constructor
/*! 
	Constractor a new <var>US_ExpData_DB</var> interface, 
	with <var>parent</var> as a parent and <var>us_tblexpdata</var> as object name.
*/ 
US_ExpData_DB::US_ExpData_DB(QWidget *parent, const char *name) :US_DB(parent, name)
{
	int border=4, spacing=2;
	int xpos = border, ypos = border;
	int buttonw = 150, buttonh = 26, labelw = 150;

	cell_flag = false;		// use for cell table connect 
	query_flag = false;		// use for query listbox select
	from_query = false;		// use for Cell ID display DB data
	all_done = false;			// use for closeEvent to check the DB save, but only when called from us_expdata_db.
	cell_table_unfinished = 0;
	Steps = 0;
	for(int i=0; i<8; i++)
	{
		exp_info.Cell[i] = "";
		exp_info.CellID[i] = 0;
	}
	
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	
	data_path = USglobal->config_list.data_dir;
	
	pb_run = new QPushButton(tr("Load Run"), this);
	Q_CHECK_PTR(pb_run);
	pb_run->setAutoDefault(false);
	pb_run->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_run->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_run->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_run, SIGNAL(clicked()), SLOT(load_run()));

	xpos += buttonw + spacing;
	lbl_run= new QLabel("",this);
	lbl_run->setAlignment(AlignLeft|AlignVCenter);
	lbl_run->setGeometry(xpos, ypos, labelw, buttonh);
	lbl_run->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_run->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_run->setText(tr("not selected"));

	xpos += labelw +spacing;
	pb_help = new QPushButton(tr("Help"), this);
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_help->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	xpos += buttonw + spacing;
	pb_close = new QPushButton(tr("Close"), this);
	pb_close->setAutoDefault(false);
	pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_close->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_close->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_close, SIGNAL(clicked()), SLOT(quit()));

	xpos = border;
	ypos +=buttonh + spacing;
	pb_investigator = new QPushButton(tr("Select Investigator"), this);
	pb_investigator->setAutoDefault(false);
	pb_investigator->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_investigator->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_investigator->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_investigator, SIGNAL(clicked()), SLOT(sel_investigator()));

	xpos += buttonw + spacing;
	lbl_investigator= new QLabel("",this);
	lbl_investigator->setAlignment(AlignLeft|AlignVCenter);	
	lbl_investigator->setGeometry(xpos, ypos, labelw, buttonh);
	lbl_investigator->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_investigator->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_investigator->setText(tr("not selected"));
	
	xpos += labelw + spacing;
	pb_add = new QPushButton(tr("Add DB Entry"), this);
	pb_add->setAutoDefault(false);
	pb_add->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_add->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_add->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_add, SIGNAL(clicked()), SLOT(add_db()));

	xpos += buttonw + spacing;
	pb_delete = new QPushButton(tr("Delete DB Entry"), this);
	pb_delete->setAutoDefault(false);
	pb_delete->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_delete->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_delete->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_delete, SIGNAL(clicked()), SLOT(check_permission()));
	
	xpos = border;
	ypos += buttonh + spacing;
	pb_date = new QPushButton(tr("Date(from Calendar)"), this);
	pb_date->setAutoDefault(false);
	pb_date->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_date->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_date->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_date, SIGNAL(clicked()), SLOT(sel_date()));
	
	xpos += buttonw + spacing;
	lbl_date= new QLabel("",this);
	lbl_date->setAlignment(AlignLeft|AlignVCenter);	
	lbl_date->setGeometry(xpos, ypos, labelw, buttonh);
	lbl_date->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_date->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	QDate date = QDate::currentDate();
   current_date.sprintf( "%d/%d/%d", date.month(), date.day(), date.year() );
	lbl_date->setText(current_date);
	exp_info.Date = current_date;
	
	xpos += labelw + spacing;
	pb_reset = new QPushButton(tr("Reset"), this);
	pb_reset->setAutoDefault(false);
	pb_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_reset->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_reset->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));

	xpos += buttonw + spacing;
	pb_query = new QPushButton(tr("Query DB Entry"), this);
	pb_query->setAutoDefault(false);
	pb_query->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_query->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_query->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_query, SIGNAL(clicked()), SLOT(query_db()));

	xpos = border;
	ypos += buttonh + spacing;
	lbl_runName = new QLabel(tr(" Run Name:"),this);
	lbl_runName->setAlignment(AlignLeft|AlignVCenter);
	lbl_runName->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_runName->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_runName->setGeometry(xpos, ypos, buttonw, buttonh );

	xpos += buttonw + spacing;
	lbl_runid= new QLabel("",this);
	lbl_runid->setAlignment(AlignLeft|AlignVCenter);
	lbl_runid->setGeometry(xpos, ypos, labelw, buttonh);
	lbl_runid->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_runid->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	xpos += labelw + spacing;
	lb_query = new QListBox(this, "Query");
	lb_query->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	lb_query->setGeometry(xpos, ypos, buttonw*2+spacing, buttonh*3+spacing*2);
	connect(lb_query, SIGNAL(selected(int)), SLOT(sel_query(int)));
	
	xpos = border;
	ypos += buttonh + spacing;
	lbl_optical = new QLabel(tr(" Optical System:"),this);
	lbl_optical->setAlignment(AlignLeft|AlignVCenter);
	lbl_optical->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_optical->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_optical->setGeometry(xpos, ypos, buttonw, buttonh );

	xpos += buttonw + spacing;
	lbl_opt = new QLabel("",this);
	lbl_opt->setAlignment(AlignLeft|AlignVCenter);
	lbl_opt->setGeometry(xpos, ypos, labelw, buttonh);
	lbl_opt->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_opt->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	

	xpos = border;
	ypos += buttonh + spacing;
	lbl_experiment = new QLabel(tr(" Experiment Type:"),this);
	lbl_experiment->setAlignment(AlignLeft|AlignVCenter);
	lbl_experiment->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_experiment->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_experiment->setGeometry(xpos, ypos, buttonw, buttonh );

	xpos += buttonw + spacing;
	lbl_exp = new QLabel("",this);
	lbl_exp->setAlignment(AlignLeft|AlignVCenter);
	lbl_exp->setGeometry(xpos, ypos, labelw, buttonh);
	lbl_exp->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_exp->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	xpos = border;
	ypos += buttonh + spacing;
	lbl_duration = new QLabel(tr(" Duration:"),this);
	lbl_duration->setAlignment(AlignLeft|AlignVCenter);
	lbl_duration->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_duration->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_duration->setGeometry(xpos, ypos, buttonw, buttonh );
	
	xpos += buttonw + spacing;
	lbl_dur= new QLabel("",this);
	lbl_dur->setAlignment(AlignLeft|AlignVCenter);
	lbl_dur->setGeometry(xpos, ypos, labelw, buttonh);
	lbl_dur->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_dur->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	xpos += labelw + spacing;
	lbl_description = new QLabel(tr(" Description:"),this);
	lbl_description->setAlignment(AlignLeft|AlignVCenter);
	lbl_description->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_description->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_description->setGeometry(xpos, ypos, buttonw, buttonh );

	xpos += buttonw + spacing;
	le_description = new QLineEdit(this, "description");
	le_description->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_description->setGeometry(xpos, ypos, buttonw, buttonh);
	connect (le_description, SIGNAL(textChanged(const QString &)), SLOT(update_description(const QString &)));

	xpos = border;
	ypos += buttonh + spacing;
	lbl_rotor = new QLabel(tr(" Rotor:"),this);
	lbl_rotor->setAlignment(AlignLeft|AlignVCenter);
	lbl_rotor->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_rotor->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_rotor->setGeometry(xpos, ypos, buttonw, buttonh );
	
	xpos += buttonw + spacing;
	lbl_rt= new QLabel("",this);
	lbl_rt->setAlignment(AlignLeft|AlignVCenter);
	lbl_rt->setGeometry(xpos, ypos, labelw, buttonh);
	lbl_rt->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_rt->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
		xpos += labelw + spacing;
	lbl_temperature = new QLabel(tr(" Temperature:"),this);
	lbl_temperature->setAlignment(AlignLeft|AlignVCenter);
	lbl_temperature->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_temperature->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_temperature->setGeometry(xpos, ypos, buttonw, buttonh );

	xpos += buttonw + spacing;
	lbl_temp= new QLabel("",this);
	lbl_temp->setAlignment(AlignLeft|AlignVCenter);
	lbl_temp->setGeometry(xpos, ypos, buttonw, buttonh);
	lbl_temp->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_temp->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	

	xpos = border;
	ypos += buttonh + spacing;
	lbl_cell = new QLabel(tr(" Cell ID:"),this);
	lbl_cell->setAlignment(AlignLeft|AlignVCenter);
	lbl_cell->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_cell->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_cell->setGeometry(xpos, ypos, buttonw, buttonh );

	xpos += buttonw + spacing;
	cmbb_cell = new QComboBox(false, this, "Cell ID Select");
	cmbb_cell->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cmbb_cell->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	cmbb_cell->setGeometry(xpos, ypos, labelw + buttonw*2 + spacing*2, buttonh);
	cmbb_cell->insertItem("No Data");
	connect(cmbb_cell, SIGNAL(activated(int)), SLOT(sel_cell(int)));

	ypos += buttonh + border;
	xpos = 2 * border + 3 * buttonw + labelw + 3*spacing;

	global_Xpos += 30;
	global_Ypos += 30;

	setMinimumSize(xpos, ypos);	
	setGeometry(global_Xpos, global_Ypos, xpos, ypos);	
setup_GUI();
}

//! Destructor
/*! destroy the <var>US_ExpData_DB</var>. */
US_ExpData_DB::~US_ExpData_DB()
{
	
	delete editdb;
	delete investigator_dlg;
	delete calendar_dlg;
	delete cell_dlg;
}

void US_ExpData_DB::setup_GUI()
{
	QGridLayout * Grid = new QGridLayout(this,6,4,4,2);
	Grid->setColSpacing(1,150);
	Grid->addWidget(pb_run,0,0);
	Grid->addWidget(lbl_run,0,1);
	Grid->addWidget(pb_help,0,2);
	Grid->addWidget(pb_close,0,3);
	Grid->addWidget(pb_investigator,1,0);
	Grid->addWidget(lbl_investigator,1,1);
	Grid->addWidget(pb_add,1,2);
	Grid->addWidget(pb_delete,1,3);
	Grid->addWidget(pb_date,2,0);
	Grid->addWidget(lbl_date,2,1);
	Grid->addWidget(pb_reset,2,2);
	Grid->addWidget(pb_query,2,3);
	Grid->addWidget(lbl_runName,3,0);
	Grid->addWidget(lbl_runid,3,1);
	Grid->addWidget(lbl_optical,4,0);
	Grid->addWidget(lbl_opt,4,1);
	Grid->addWidget(lbl_experiment,5,0);
	Grid->addWidget(lbl_exp,5,1);
	Grid->addMultiCellWidget(lb_query,3,5,2,3);
	
	Grid->addWidget(lbl_duration,6,0);
	Grid->addWidget(lbl_dur,6,1);
	Grid->addWidget(lbl_description,6,2);
	Grid->addWidget(le_description,6,3);
	Grid->addWidget(lbl_rotor,7,0);
	Grid->addWidget(lbl_rt,7,1);
	Grid->addWidget(lbl_temperature,7,2);
	Grid->addWidget(lbl_temp,7,3);
	Grid->addWidget(lbl_cell,8,0);
	Grid->addMultiCellWidget(cmbb_cell,8,8,1,3);
	
	Grid->activate();
}

/*!
	Open US_Edit_DB interface to load experimental data.
*/
void US_ExpData_DB::load_run()
{
	reset();
	
	editdb = new US_Edit_DB();
	editdb->setCaption("Sedimentation Data Entry");
	connect(editdb, SIGNAL(variablesUpdated()), this, SLOT(update_variables()));
	connect(editdb, SIGNAL(channel_selected(const QString &)), this, SLOT(update_flo_channel(const QString &)));
	editdb->show();
}

/*!
	Update selected fluorescence channel variable flo_channel
*/

void US_ExpData_DB::update_flo_channel(const QString &str)
{
	flo_channel = str;
}

/*!
	Update variables' value from load_run().
*/
void US_ExpData_DB::update_variables()
{

	bool flg;
	QString STR;
	QSqlQuery target;	
	STR = "INSERT INTO tblExpData (Path) VALUES ('N/A');";
	flg = target.exec(STR);
	if(!flg)
	{
		QSqlError sqlerr = target.lastError();
      QMessageBox::message(tr("Attention:"), 
                           tr("Reserving DB table 'tblExpData' failed.\n"
									"Attempted to execute this command:\n\n"
									+ STR + "\n\n"
									"Causing the following error:\n\n")
									+ sqlerr.text());

		return;
	}
	QSqlQuery query( "SELECT ExpDataID FROM tblExpData" );
	if (  query.isActive() )
	{ 
            
		query.last();                   // Moves to the last record.
      exp_info.ExpdataID=query.value(0).toInt();                
	}
	else
	{
		exp_info.ExpdataID=1;
	}

	exp_info.Path = editdb->run_inf.data_dir;
	lbl_run->setText(exp_info.Path);
	exp_info.Runid = editdb->run_inf.run_id;
	lbl_runid->setText(exp_info.Runid);
	exp_info.Temperature = editdb->run_inf.avg_temperature;
	lbl_temp->setText(QString::number(exp_info.Temperature));
	exp_info.Duration = editdb->run_inf.duration;
	lbl_dur->setText(QString::number(exp_info.Duration));
	for (int i=0; i<8; i++)
	{
		for (int j=0; j<3; j++)
		{
			exp_info.wavelength[i][j] = editdb->run_inf.wavelength[i][j];
			exp_info.scans[i][j] = editdb->run_inf.scans[i][j];
		}
		exp_info.centerpiece[i] = editdb->run_inf.centerpiece[i];
	}
	exp_info.Edit_type = editdb->edit_type;
	update_type(exp_info.Edit_type);
	exp_info.Rotor = editdb->run_inf.rotor;
	update_rotor(exp_info.Rotor);
	for(int i=0; i<8; i++)
	{
		exp_info.Cell[i] = editdb->run_inf.cell_id[i];
	}
	update_cell();
}

/*!
	Update cell tables info and show them in combobox list.
*/
void US_ExpData_DB::update_cell()
{
	cmbb_cell->clear();
	QString str;
	for (int i=0; i<8; i++) 
	{  
		if(exp_info.Cell[i].isEmpty())
		{
			str.sprintf("Cell %d : No Data", i+1);
			cmbb_cell->insertItem(str);
		}
		else
		{	
			if(from_query)
			{
				str.sprintf("Cell %d : "+ exp_info.Cell[i] + " (Cell Table ID: %d)",i+1, exp_info.CellID[i]);
				cmbb_cell->insertItem(str);
			}
			else
			{
				str.sprintf("Cell %d : "+ exp_info.Cell[i] + ", Cell Table ID not selected", i+1);
				cmbb_cell->insertItem(str);
				cell_table_unfinished++;			//total cell tables need for this experiment data table
			}
			cell_flag = true;		//now can connect to cell table
		}	
   }	
}

/*!
	Open US_cell_DB interface, show the selected cell information.
*/
void US_ExpData_DB::sel_cell(int i)
{
	QString new_str;
	
	if(cell_flag && !exp_info.Cell[i].isEmpty())
	{
	 	if(exp_info.Invid <= 0)
		{
			QMessageBox::message(tr("Attention:"), tr("Please 'Select Investigator' first!"));
			return;	
		}
		if(exp_info.centerpiece[i]<0)
		{
			QMessageBox::message(tr("Attention:"), tr("Please select a centerpiece during 'Load Run'"));
			return;
		}
		exp_info.cell_position = i;
		cell_dlg = new US_Cell_DB(exp_info, &cell_table_unfinished);
		cell_dlg->setCaption("Cell Information");
		
		if(from_query)		// not allow change data if you load them from DB
		{
			QSqlCursor cur( "tblCell" );
			QString chk_cellid;
			chk_cellid.sprintf("CellID = %d",exp_info.CellID[i]);
			cur.select(chk_cellid);
			
  			if(cur.next())		//check Cell Table and find CellID match the ExpData table's CellID[i] and retrieve Cell table
			{
				cell_dlg->cell_info.CellID = cur.value("CellID").toInt();
				cell_dlg->cell_info.ExperimentID = cur.value("ExperimentID").toInt();
				cell_dlg->cell_info.CenterpieceID = cur.value("CenterpieceID").toInt();
				cell_dlg->cell_info.Position = cur.value("Position").toInt();
				cell_dlg->cell_info.InvID = cur.value("InvestigatorID").toInt();
				cell_dlg->cell_info.Description = cur.value("Description").toString();
				cell_dlg->cell_info.Num_Channel = cur.value("Num_Channel").toInt();
				QString wl_str, scans_str;
				for(int i=0; i<3; i++)
				{
					wl_str = "";
					wl_str.sprintf("Wavelength_%d",i+1);
					cell_dlg->cell_info.Wavelength[i] = cur.value(wl_str).toInt();
					scans_str = "";
					scans_str.sprintf("Wl_Scans_%d",i+1);
					cell_dlg->cell_info.Scans[i] = cur.value(scans_str).toInt();	
				}
				QString DNA_Str, Buff_Str, Pep_Str;
				for(int i= 0; i<4; i++)
				{
					Buff_Str = "";
					Buff_Str.sprintf("BufferID_%d",i+1);
					cell_dlg->cell_info.BufferID[i] = cur.value(Buff_Str).toInt();
					for(int j=0; j<3; j++)
					{
						if(j==0)
						{
							DNA_Str = "";
							DNA_Str.sprintf("DNAID_%d",i+1);
							cell_dlg->cell_info.DNAID[i][j] = cur.value(DNA_Str).toInt();
							Pep_Str = "";
							Pep_Str.sprintf("PeptideID_%d",i+1);
							cell_dlg->cell_info.PepID[i][j] = cur.value(Pep_Str).toInt();
						}
						else
						{
							DNA_Str = "";
							DNA_Str.sprintf("DNA%dID_%d",j+1,i+1);
							cell_dlg->cell_info.DNAID[i][j] = cur.value(DNA_Str).toInt();
							Pep_Str = "";
							Pep_Str.sprintf("Peptide%dID_%d",j+1,i+1);
							cell_dlg->cell_info.PepID[i][j] = cur.value(Pep_Str).toInt();
						}
					}
				}
		
				cell_dlg->from_db = true;
				cell_dlg->lbl_cellid->setNum(cell_dlg->cell_info.CellID);			
				cell_dlg->lbl_invest->setNum(cell_dlg->cell_info.InvID);
				cell_dlg->lbl_ctpc->setNum(cell_dlg->cell_info.CenterpieceID);
				cell_dlg->lbl_position->setNum(cell_dlg->cell_info.Position+1);
				cell_dlg->lbl_exp->setNum(cell_dlg->cell_info.ExperimentID);
				QString str2;
				str2.sprintf("Cell Information Retrieve from Cell Table %d. ",cell_dlg->cell_info.CellID);
				cell_dlg->lb_query->insertItem(str2);
				if(cell_dlg->cell_info.Wavelength[0] == 0)
				{
					cell_dlg->lbl_wl1->setText("No Data available");
				}
				else
				{
					cell_dlg->lbl_wl1->setNum(cell_dlg->cell_info.Wavelength[0]);
				}
				if(cell_dlg->cell_info.Wavelength[1] == 0)
				{
					cell_dlg->lbl_wl2->setText("No Data available");
				}
				else
				{
					cell_dlg->lbl_wl2->setNum(cell_dlg->cell_info.Wavelength[1]);
				}
				if(cell_dlg->cell_info.Wavelength[2] == 0)
				{
					cell_dlg->lbl_wl3->setText("No Data available");
				}
				else
				{
					cell_dlg->lbl_wl3->setNum(cell_dlg->cell_info.Wavelength[2]);
				}
				if(cell_dlg->cell_info.Scans[0] == 0)
				{
					cell_dlg->lbl_scan1->setText("No Scans");
				}
				else
				{
					cell_dlg->lbl_scan1->setNum(cell_dlg->cell_info.Scans[0]);
				}
				if(cell_dlg->cell_info.Scans[1] == 0)
				{
					cell_dlg->lbl_scan2->setText("No Scans");
				}
				else
				{
					cell_dlg->lbl_scan2->setNum(cell_dlg->cell_info.Scans[1]);
				}
				if(cell_dlg->cell_info.Scans[2] == 0)
				{
					cell_dlg->lbl_scan3->setText("No Scans");
				}
				else
				{
					cell_dlg->lbl_scan3->setNum(cell_dlg->cell_info.Scans[2]);
				}
				cell_dlg->cmbb_channel->clear();
				QString channel_Str;
				for(int i=0; i<cell_dlg->cell_info.Num_Channel; i++)
				{
					channel_Str = "";
					channel_Str.sprintf("Channel %d", i+1);
					cell_dlg->cmbb_channel->insertItem(channel_Str);
				}
				int j = cell_dlg->cmbb_channel->currentItem();
			
				cell_dlg->le_buffer->setText(show_buffer(cell_dlg->cell_info.BufferID[j]));
				if(cell_dlg->cell_info.BufferID[j] == -1)
				{
					cell_dlg->le_buffer->setText("not selected");
				}
				cell_dlg->le_peptide1->setText(show_peptide(cell_dlg->cell_info.PepID[j][0]));
				if(cell_dlg->cell_info.PepID[j][0] == -1)
				{
					cell_dlg->le_peptide1->setText("not selected");
				}
				cell_dlg->le_peptide2->setText(show_peptide(cell_dlg->cell_info.PepID[j][1]));
				if(cell_dlg->cell_info.PepID[j][1] == -1)
				{
					cell_dlg->le_peptide2->setText("not selected");
				}
				cell_dlg->le_peptide3->setText(show_peptide(cell_dlg->cell_info.PepID[j][2]));
				if(cell_dlg->cell_info.PepID[j][2] == -1)
				{
					cell_dlg->le_peptide3->setText("not selected");
				}
				cell_dlg->le_DNA1->setText(show_DNA(cell_dlg->cell_info.DNAID[j][0]));
				if(cell_dlg->cell_info.DNAID[j][0] == -1)
				{
					cell_dlg->le_DNA1->setText("not selected");
				}
				cell_dlg->le_DNA2->setText(show_DNA(cell_dlg->cell_info.DNAID[j][1]));
				if(cell_dlg->cell_info.DNAID[j][1] == -1)
				{
					cell_dlg->le_DNA2->setText("not selected");
				}
				cell_dlg->le_DNA3->setText(show_DNA(cell_dlg->cell_info.DNAID[j][2]));
				if(cell_dlg->cell_info.DNAID[j][2] == -1)
				{
					cell_dlg->le_DNA3->setText("not selected");
				}

				cell_dlg->pb_add->setEnabled(false);
				cell_dlg->pb_reset->setEnabled(false);
				cell_dlg->pb_query->setEnabled(false);
				cell_dlg->pb_buffer->setEnabled(false);
				cell_dlg->pb_peptide1->setEnabled(false);
				cell_dlg->pb_peptide2->setEnabled(false);
				cell_dlg->pb_peptide3->setEnabled(false);
				cell_dlg->pb_DNA1->setEnabled(false);
				cell_dlg->pb_DNA2->setEnabled(false);
				cell_dlg->pb_DNA3->setEnabled(false);
			}
			else //not find CellID and update Expdata table CellID to 0
			{
				exp_info.CellID[i] = 0;
				QSqlCursor cur( "tblExpData" );
				QString str0;
				str0.sprintf(" ExpdataID = %d", exp_info.ExpdataID);
				cur.select(str0);
				if(cur.next())
				{
					QSqlRecord *buffer = cur.primeUpdate();
					QString str1;
					str1.sprintf("Cell%dID", i+1);
               buffer->setValue( str1, exp_info.CellID[i] );
               cur.update();
				}
				QString str2;
				str2.sprintf("Cell %d : "+exp_info.Cell[i] + " , Cell Table Deleted", i+1);
				cmbb_cell->changeItem(str2, i);
				//QMessageBox::message(tr("Attention:"), tr("No Cell Data about this Experiment in Database!"));
				return;
			}
		}
		else		//first time load from 'Load Run' and not from DB
		{
			if(exp_info.CellID[i] == 0)		//prevent twice open same cell table to get new table ID
			{
				exp_info.CellID[i] = cell_dlg->get_newID("tblCell","CellID");
			}
			for(int j=0; j<8; j++)	//to check whether same Cell Table ID exist
			{
				if((j!=i )&&(exp_info.CellID[j] == exp_info.CellID[i]))
				{
					QString str_w;
					str_w.sprintf("Please complete the data entry for\ncell table %d first!", exp_info.CellID[i]);
					exp_info.CellID[i] = 0;
					QMessageBox::message(tr("Attention:"), str_w);
					return;
				}
			}
			new_str.sprintf("Cell %d : "+ exp_info.Cell[i] + " (Cell Table ID: %d)",i+1, exp_info.CellID[i]);
			cmbb_cell->changeItem(new_str, i);
			
		}
		cell_dlg->show();	
	}
	else if(cell_flag && exp_info.Cell[i].isEmpty())
	{
		
		QMessageBox::message(tr("Attention:"), tr("No Data in this Cell!"));
		return;
	}
	else
	{
		QMessageBox::message(tr("Attention:"), tr("You need to select 'Load Run' or 'Query DB' first!"));
		return;
	}	
}

/*!
	Update <var>lbl_exp</var> and <var>lbl_opt</var> with 
	loaded data's experiment type and optical system.
	\param edit_type an integer variable with 14 different edit types.
*/
void US_ExpData_DB::update_type(int edit_type)
{
	switch (edit_type)
	{
		case 1:
		{
			lbl_exp->setText("Velocity");
			lbl_opt->setText("UV Absorbance");
			break;
		}
		case 2:
		{
			lbl_exp->setText("Equilibrium");
			lbl_opt->setText("UV Absorbance");
			break;
		}
		case 3:
		{
			lbl_exp->setText("Velocity");
			lbl_opt->setText("Raleigh Interference");
			break;
		}
		case 4:
		{
			lbl_exp->setText("Equilibrium");
			lbl_opt->setText("Raleigh Interference");
			break;
		}
		case 5:
		{
			lbl_exp->setText("Velocity");				
			lbl_opt->setText("Fluorescence");
			 break;
		}
		case 6:
		{
			lbl_exp->setText("Equilibrium");
			lbl_opt->setText("Fluorescence");
			break;
		}
		case 7:
		{
			lbl_exp->setText("Diffusion");
			lbl_opt->setText("UV Absorbance");
			break;
		}
		case 8:
		{
			lbl_exp->setText("Diffusion");
			lbl_opt->setText("Raleigh Interference");
			break;
		}
		case 9:
		{
			lbl_exp->setText("Diffusion");
			lbl_opt->setText("Fluorescence");
			break;
		}
		case 10:
		{
			lbl_exp->setText("Wavelength");				
			lbl_opt->setText("UV Absorbance");
			 break;
		}
		case 11:
		{
			lbl_exp->setText("Wavelength");				
			lbl_opt->setText("Intensity");
			 break;
		}
		case 12:
		{
			lbl_exp->setText("Velocity");				
			lbl_opt->setText("Intensity");
			 break;
		}
		case 13:
		{
			lbl_exp->setText("Equilibrium");
			lbl_opt->setText("Intensity");
			break;
		}
		case 14:
		{
			lbl_exp->setText("Diffusion");
			lbl_opt->setText("Intensity");
			break;
		}
	}
}

/*!
	Update <var>lbl_rt</var> with loaded data's rotor.
	\param rotor an integer variable with 3 different rotor types.
*/
void US_ExpData_DB::update_rotor(int rotor)
{
	switch (rotor)
	{
		case 0:
		{
			lbl_rt->setText("Simulation");
			break;
		}
		case 1:
		{
			lbl_rt->setText("AN50");
			break;
		}
		case 2:
		{
			lbl_rt->setText("AN60");
			break;
		}
	}
}

/*!
	Update description by LineEdit input.
*/
void US_ExpData_DB::update_description(const QString &newText)
{
		exp_info.Description = newText;
}

/*!
	Open US_DB_TblInvestigator interface for selecting investigator.
*/
void US_ExpData_DB::sel_investigator()
{
	investigator_dlg = new US_DB_TblInvestigator();
	investigator_dlg->setCaption("Investigator Information");
	investigator_dlg->pb_exit->setText("Accept");
	investigator_dlg->show();
	connect(investigator_dlg, SIGNAL(valueChanged(QString, int)), SLOT(update_investigator_lbl(QString, int)));
}

/*!
	Update display with the selected investigator information.
	\param Display a string variable for show investigator info.
	\param InvID a integer variable for DB table: <tt>tblInvestigators</tt> index.
*/
void US_ExpData_DB::update_investigator_lbl (QString Display, int InvID)
{
	QString str;
	exp_info.Invid = InvID;
	str = Display;
	lbl_investigator->setText(str);
	if(str == "")
	{
		lbl_investigator->setText("not selected");
	}
}

/*!
	Open US_Calendar interface for selecting date.
*/
void US_ExpData_DB::sel_date()
{
	calendar_dlg = new US_Calendar();
	calendar_dlg->setCaption("Calendar");
	calendar_dlg->show();
	connect(calendar_dlg, SIGNAL(dateChanged(QString)), SLOT(update_date_lbl(QString)));
}

/*!
	Update display with the selected date.
	\param Display_Date a string variable for show Date info.
*/
void US_ExpData_DB::update_date_lbl (QString Display_Date)
{

	exp_info.Date = Display_Date;
	lbl_date->setText(exp_info.Date);
}

/*!
	Save the experimental data into DB table: <tt>tblExpData</tt>.
*/
void US_ExpData_DB::add_db()
{
	if(exp_info.Path.isEmpty())
	{
		QMessageBox::message(tr("Attention:"), tr("Please 'Load Run' first!"));
		return;
	} 
	if(exp_info.Invid <= 0)
	{
		QMessageBox::message(tr("Attention:"), tr("Please 'Select Investigator' first!"));
		return;	
	}
	if(exp_info.Description.isEmpty())
	{
		QMessageBox::message(tr("Attention:"), tr("Please enter a description\n"
													  			" before saving!"));
		return;
	}

	if(cell_table_unfinished>0)
	{
		QMessageBox::message(tr("Attention:"), tr("Some cell datatables are still incomplete,\n"
																"please finish all cell data entries before saving."));
		return;
	}
	
	pd_add = new QProgressDialog( "Waiting for DB...", 0, 4, this,"pd_add", TRUE );
	pd_add->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	pd_add->setMinimumDuration(0);
	pd_add->setProgress(0);
	
	
	bool flg1;
	QString str, STR;
	QSqlQuery target1;
	STR = "UPDATE tblExpData SET ";
	STR +="Invid = '" + QString::number(exp_info.Invid) + "', ";
	STR +="Temperature = '" + QString::number(exp_info.Temperature) + "', ";
	STR +="Duration = '" + QString::number(exp_info.Duration) + "', "; 
	STR +="Edit_type = '" + QString::number(exp_info.Edit_type) + "', ";
	STR +="Rotor = '" + QString::number(exp_info.Rotor) + "', ";
	STR +="Path = '" + exp_info.Path + "', ";
	STR +="Date = '" + exp_info.Date + "', ";
	STR +="Runid = '" + exp_info.Runid + "', ";
	
	for(int i=0; i<8; i++)
	{	
		if(exp_info.CellID[i]>0)
		{
			STR +="Cell"+QString::number(i+1)+" = '"+ exp_info.Cell[i] + "', ";
			STR +="Cell"+QString::number(i+1)+"ID = '" + QString::number(exp_info.CellID[i]) + "', ";
		}
	}	
	STR += "Description= '" + exp_info.Description + "' ";
	STR += "WHERE ExpdataID = '"+QString::number(exp_info.ExpdataID)+"';";
	flg1 = target1.exec(STR);
	if(!flg1)
	{
		QSqlError sqlerr = target1.lastError();
      QMessageBox::message(tr("Attention:"), 
                           tr("Saving to DB table 'tblExpData' failed.\n"
									"Attempted to execute this command:\n\n"
									+ STR + "\n\n"
									"Causing the following error:\n\n")
									+ sqlerr.text());

		return;
	}
		
	QDir temp_dir;
	str = USglobal->config_list.root_dir + "/tmp";
	temp_dir.setPath(str);
	if(temp_dir.exists())
	{
		QStringList entries;
		temp_dir.setNameFilter("*.*");
		entries = temp_dir.entryList();
		for (QStringList::Iterator it = entries.begin(); it != entries.end(); ++it ) 
		{
				temp_dir.remove((*it).latin1());
		}
	}
	else
	{
		temp_dir.mkdir(str);
	}	
	
	compress_proc = new QProcess(this);
	compress_proc->clearArguments();
   QStringList cmd;
#ifdef WIN32
	cmd.append("copy");
#endif
#ifdef UNIX
	cmd.append("cp");
#endif
	cmd.append("-r");
	cmd.append(exp_info.Path);
	cmd.append(USglobal->config_list.root_dir + "/tmp/"+exp_info.Runid);		
	compress_proc->setArguments(cmd);
	pd_add->setProgress(1);
	if(!compress_proc->start())
	{
			QMessageBox::message(
        tr( "UltraScan Error:" ), 
        tr( "Unable to start process to copy data." ) );
			return;
	}
	c_step=0;   
	connect(compress_proc, SIGNAL(processExited()), this, SLOT(endCompressProcess()));
	disconnect(compress_proc);

		
}

void US_ExpData_DB::endCompressProcess()
{
	QDir work_dir;
	QString str, tarfile, tardir; 	       
	switch(c_step)
	{
		case 0:			//tar raw data tar file 
		{
			work_dir.setPath(USglobal->config_list.root_dir + "/tmp");
			tarfile = exp_info.Runid + "_rawdata.tar";
			tardir = exp_info.Runid + "/";
			compress_proc->clearArguments();
         compress_proc->setWorkingDirectory(work_dir);
			QStringList cmd;
			cmd.append("tar");
			cmd.append("--mode=u+X");
			cmd.append("-cvf");
			cmd.append(tarfile);
			cmd.append(tardir);			
			compress_proc->setArguments(cmd);
			c_step=1;
			pd_add->setProgress(2);
			if(!compress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to tar raw data archive.");
				return;
			}
			break;
		}
		case 1:			//gzip raw data tar file 
		{
						
			work_dir.setPath(USglobal->config_list.root_dir + "/tmp");
			tarfile = exp_info.Runid + "_rawdata.tar";
			compress_proc->clearArguments();
         compress_proc->setWorkingDirectory(work_dir);
			QStringList cmd;
			cmd.append("gzip");
			cmd.append("-9");
			cmd.append(tarfile);			
			compress_proc->setArguments(cmd);
			c_step=2;
			pd_add->setProgress(3);
			if(!compress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to gzip raw data archive.");
				return;
			}
			break;
		}
		case 2:		//insert compressed file into tblRawExpData
		{
			QSqlCursor cur("tblRawExpData");
   		cur.setMode( QSqlCursor::Insert);
   		QSqlRecord *buffer = cur.primeInsert();
   		buffer->setValue("ExpdataID", exp_info.ExpdataID);
			str = USglobal->config_list.root_dir + "/tmp/"+ exp_info.Runid+"_rawdata.tar.gz";
			if(!write_blob(str, buffer, "RawData"))
			{
				QMessageBox::message(tr("Error:"), 
                             "Saving file: "+str+"\n to DB table 'tblRawExpData' failed.\n");
				return;

			}
			int result = cur.insert();
			if(result<=0)
			{
				QSqlError err = cur.lastError();
				QMessageBox::message(tr("Attention:"), 
                             tr("Saving to DB table 'tblRawExpData' failed.\n"
										 	"Error message from MySQL:\n\n")
										  + err.text());
				return;
			}
			QFile tempFile(USglobal->config_list.root_dir + "/tmp/"+ exp_info.Runid + "_rawdata.tar.gz");
			if(tempFile.exists())
			{
				tempFile.remove();
			}
			pd_add->close();
			lb_query->clear();
			str.sprintf("ExpData (ID:%d) has been saved to the database. ", exp_info.ExpdataID);
			lb_query->insertItem(str);
			lb_query->insertItem("************************************************");
			lb_query->insertItem("If you want to change any selected item,");
			lb_query->insertItem("you will have to delete the existing entry first,");
			lb_query->insertItem("or you can create a new database  Entry");
			all_done=true;
			
			delete compress_proc;     
		}
	}
}

/*!
	List experimental data that stored in DB table: <tt>tblExpData</tt>.
*/
void US_ExpData_DB::query_db()
{
	QSqlCursor cur( "tblExpData" );
   QSqlIndex order = cur.index( "Description" );
   cur.select(order);
	
	maxID = get_newID("tblExpData","ExpdataID");
	int count = 0;
	item_ExpdataID = new int[maxID];
	item_Description = new QString[maxID];
	display_Str = new QString[maxID];
	
	while(cur.next() )
	{
		int id = cur.value("ExpdataID").toInt();
		display_Str[count] = "ExpDataID ("+  QString::number( id ) + "): "+
							 		cur.value( "Description" ).toString();
		item_ExpdataID[count] = id;
		item_Description[count] = cur.value("Description").toString();
		count++;					
	}
	if(count>0)
	{
		lb_query->clear();
		for( int i=0; i<count; i++)
		{
			lb_query->insertItem(display_Str[i]);
		}
		query_flag = true;
	}
	else
	{
		lb_query->clear();
		lb_query->insertItem("No data were found in the DB");
	}

}

/*!
	Select one experimental data <var>item</var> listed in listbox, 
	this function will show all its info in interface.
*/
void US_ExpData_DB::sel_query(int item)
{
	if(query_flag)
	{
		exp_info.ExpdataID = item_ExpdataID[item];
		exp_info.Description = item_Description[item];
			
		QSqlCursor cur( "tblExpData" );
   	QStringList orderFields = QStringList()<<"Invid"<<"Date";
   	QSqlIndex order = cur.index( orderFields );
   	QSqlIndex filter = cur.index( "ExpdataID" );
   	cur.setValue( "ExpdataID", exp_info.ExpdataID );
		cur.select(filter, order);
		if(cur.next())
		{
			exp_info.Invid = cur.value("Invid").toInt();
			exp_info.Path = cur.value("Path").toString();
			exp_info.Date = cur.value("Date").toString();
			exp_info.Edit_type = cur.value("Edit_type").toInt();
			exp_info.Runid = cur.value("Runid").toString();
			exp_info.Temperature = cur.value("Temperature").toString().toFloat();
			exp_info.Duration = cur.value("Duration").toString().toFloat();
			exp_info.Rotor = cur.value("Rotor").toInt();
			QString cell_Str, cellid_Str;
			for(int i=0; i<8; i++)
			{
				cell_Str = "";
				cell_Str.sprintf("Cell%d", i+1);
				exp_info.Cell[i] = cur.value(cell_Str).toString();
				cellid_Str = "";
				cellid_Str.sprintf("Cell%dID", i+1);
				exp_info.CellID[i] = cur.value(cellid_Str).toInt();
				
			}	
			QSqlCursor get( "tblCell" );
			QString get_cellid, scans_str;
			for(int i=0; i<8; i++)
			{
				get_cellid.sprintf("CellID = %d",exp_info.CellID[i]);
				get.select(get_cellid);
				if(get.next())
				{
					// query Cell Table to get the number of total scans file
					for(int j=0; j<3; j++)
					{
						scans_str = "";
						scans_str.sprintf("Wl_Scans_%d",j+1);
						Steps += get.value(scans_str).toInt();
					}
					//get centerpiece, DNA, buffer,peptide data for write_dbinfo()
					exp_info.centerpiece[i] = get.value("CenterpieceID").toInt();
					QString DNA_Str, Buff_Str, Pep_Str;
					for(int j= 0; j<4; j++)
					{
						Buff_Str = "";
						Buff_Str.sprintf("BufferID_%d",j+1);
						bufferid[i][j] = get.value(Buff_Str).toInt();
						for(int k=0; k<3; k++)
						{
							if(k==0)
							{
								Pep_Str = "";
								Pep_Str.sprintf("PeptideID_%d",j+1);
								pepid[i][j][k] = get.value(Pep_Str).toInt();
								DNA_Str = "";
								DNA_Str.sprintf("DNAID_%d",j+1);
								DNAid[i][j][k] = get.value(DNA_Str).toInt();
							}
							else
							{
								Pep_Str = "";
								Pep_Str.sprintf("Peptide%dID_%d",k+1,j+1);
								pepid[i][j][k] = get.value(Pep_Str).toInt();
								DNA_Str = "";
								DNA_Str.sprintf("DNA%dID_%d",k+1,j+1);
								DNAid[i][j][k] = get.value(DNA_Str).toInt();
							}
						}
					}
				}
			}
			QString str;	
			
// read raw data from tblExpDataFile will zlib compress file format.		
	
	/*		QSqlCursor cur_f( "tblExpDataFile" );
			str.sprintf("ExpdataID = %d",exp_info.ExpdataID);
			cur_f.select(str);
			if(cur_f.next())
			{
				int size = cur_f.value("File").toByteArray().size();
				QByteArray da(size);
				da = cur_f.value("File").toByteArray();
				QDir uncompress_dir;
				QString temp_dir = data_path +"/" + exp_info.Runid;
				if(uncompress_dir.exists(temp_dir))
				{
					QMessageBox::message(tr("Attention:"), 
									tr("The directory: \n '") + temp_dir+ "'\n" +
										tr("already exists in your data directory, \n"
										"if you want to retrieve the data data again\n"
										"from the database, you will have to remove\n"
										"this directory first"));
				 return;
			 	}
				else
				{
					uncompress_dir.mkdir(temp_dir);
					write_dbinfo(temp_dir);
					QString tempfile =temp_dir +"/expdata.zlib.gz";			
					QFile fw(tempfile);
					fw.open(IO_WriteOnly);
					fw.writeBlock(da.data(),size);
					fw.close();

					int result_flag = retrieve(tempfile, Steps);	
					if(result_flag < 0) // Error to Uncompress!
					{
						QString error_mesg;
						if (result_flag == -1)
						{
							error_mesg = tr("The source file does not exist.\n");
						}
						if (result_flag == -2)
						{
							error_mesg = tr("Can not open the source file for reading.\n");
						}
						if (result_flag == -3)
						{
							error_mesg = tr("Can not open destination file for writing.\n");
						}
						QMessageBox::message(tr("Attention:\n"), 
											tr("An error occured while trying to uncompress dataset:\n\n") 
											+ error_mesg);
						return;
					}
				}
			}
		*/	
			QSqlCursor cur_f( "tblRawExpData" );
			str.sprintf("ExpdataID = %d",exp_info.ExpdataID);
			cur_f.select(str);
			if(cur_f.next())
			{
				QDir rawdata_dir(data_path +"/" + exp_info.Runid);
				if(rawdata_dir.exists())
				{
					QMessageBox::message(tr("Attention:"), 
									tr("The directory: \n '") + data_path + exp_info.Runid+ "'\n" +
										tr("already exists in your data directory, \n"
										"if you want to retrieve the data data again\n"
										"from the database, you will have to remove\n"
										"this directory first"));
				 return;
			 	}

				pd = new QProgressDialog( "Waiting for Data Retrieval...", 0, 4, this,"pd", TRUE );
				pd->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
				pd->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));	
				pd->setProgress(0);
				pd->setMinimumDuration(0);
				
		   	QString filename = make_tempFile(data_path, exp_info.Runid+".tar.gz");
         	bool raw_flag=read_blob("Rawdata", cur_f, filename);
         	if(!raw_flag)
         	{
					QMessageBox::message("UltraScan Error:", "Unable to retrieve Raw data files.");
				}
				pd->setProgress(1);
				uncompress_proc = new QProcess(this);
				uc_step=0;   
				connect(uncompress_proc, SIGNAL(processExited()), this, SLOT(endUncompressProcess()));
				endUncompressProcess();
				disconnect(uncompress_proc);					
			}
		}
	
		from_query = true;
		lbl_run->setText(exp_info.Path);
		lbl_investigator->setNum(exp_info.Invid);
		lbl_date->setText(exp_info.Date);
		lbl_runid->setText(exp_info.Runid);
		lbl_temp->setNum(exp_info.Temperature);
		lbl_dur->setNum(exp_info.Duration);
		le_description->setText(exp_info.Description);
		update_type(exp_info.Edit_type);
		update_rotor(exp_info.Rotor);
		update_cell();
	
		pb_run->setEnabled(false);
		pb_investigator->setEnabled(false);
		pb_date->setEnabled(false);
		pb_add->setEnabled(false);
		Steps = 0;
	}
	else
	{
		QMessageBox::message(tr("Attention:"), 
									tr("This is not for selecting\n"));
		return;
	}
}

void US_ExpData_DB::endUncompressProcess()
{	
	QString tar_filename;
	switch(uc_step)
	{
		case 0:			//unzip raw data file
		{

			uncompress_proc->clearArguments();
			tar_filename = exp_info.Runid+".tar.gz";
			QDir work_dir(data_path);
			uncompress_proc->setWorkingDirectory(work_dir);
			QStringList cmd;
			cmd.append("gzip");
			cmd.append("-d");
			cmd.append(tar_filename);
			uncompress_proc->setArguments(cmd);
			uc_step=1;
			pd->setProgress(2);
			if(!uncompress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to unzip Raw data files.");
				return;
			}
			break;
		}

		case 1:			//un tar raw data file
		{
			uncompress_proc->clearArguments();
			tar_filename =exp_info.Runid+".tar";
			QDir work_dir(data_path);
			uncompress_proc->setWorkingDirectory(work_dir);

			QStringList cmd;
			cmd.append("tar");
			cmd.append("-xf");
			cmd.append(tar_filename);
			uncompress_proc->setArguments(cmd);
			uc_step=2;
			pd->setProgress(3);
			if(!uncompress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to un tar Raw data files.");
				return;
			}
			break;
		}
		case 2:
		{
			write_dbinfo(data_path + "/" + exp_info.Runid);
			QFile tempFile1(data_path +"/"+ exp_info.Runid + ".tar");
			if(tempFile1.exists())
			{
				tempFile1.remove();
			}
			pd->close();
			delete uncompress_proc;
		}	
	
	}
}

/*!
	Open US_DB_Admin to check delete permission.
*/
void US_ExpData_DB::check_permission()
{
	US_DB_Admin *db_admin;
	db_admin = new US_DB_Admin("");
	db_admin->show();
	connect(db_admin, SIGNAL(issue_permission(bool)), SLOT(delete_db(bool)));
}

/*!
	Delete one selected experimental data table entry.
*/
void US_ExpData_DB::delete_db(bool permission)
{
	QString str;
	if(!permission)
	{
		QMessageBox::message(tr("Attention:"), 
									tr("Permission denied, please contact your system administrator for help"));
		return;
	}
	int id = exp_info.ExpdataID;
	if(from_query == false)
	{
		QMessageBox::message(tr("Attention:"), 
									tr("Please first select the record\n"
										"to be deleted from the database"));
										
	}
	else
	{
		/*
		QSqlCursor check( "tblCell");
		QString condition;
		condition.sprintf("ExperimentID = %d", id);
		check.select(condition);		
		if(check.next())
		{
			QMessageBox::message(tr("Attention:"), 
									tr("Cell data is associated with this experimental data!\n"
										"You need to delete this experiment's cell data first!"));
			return;
		}	
		
	
		QSqlCursor cur( "tblExpData");
		cur.setMode( QSqlCursor::Delete);
		QSqlIndex filter = cur.index("ExpdataID");
		cur.setValue("ExpdataID", id);
		cur.primeDelete();
		*/
		switch(QMessageBox::information(this, tr("Delete this record?"), 
										tr("Clicking 'OK' will delete the selected data from the database"),
										tr("OK"), tr("CANCEL"),	0,1))
		{
			case 0:
			{
				//cur.del();
				QSqlQuery del, del_exp, del_cell;
				QString str;
				str.sprintf("DELETE FROM tblExpData WHERE ExpdataID = %d;", id);
				del.exec(str);
			//delete binary file in tblExpDataFile
			//	str.sprintf("DELETE FROM tblExpDataFile WHERE ExpdataID = %d;", id);
			//delete binary file in tblRawExpData
				str.sprintf("DELETE FROM tblRawExpData WHERE ExpdataID = %d;", id);
				del_exp.exec(str);
			//delete all relative cell data
				str.sprintf("DELETE FROM tblCell WHERE ExperimentID = %d;", id);
				del_cell.exec(str);
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

/*! Reset all variables to start values. */
void US_ExpData_DB::reset()
{
	query_flag = false;
	from_query = false;
	cell_flag = false;
	all_done = false;
	Steps = 0;
	cell_table_unfinished = 0;
	exp_info.ExpdataID = -1;
	exp_info.Path = "";
	exp_info.Invid = -1;
	exp_info.Date = current_date;
	exp_info.Edit_type = -1;
	exp_info.Runid = "";
	exp_info.Temperature = -1;
	exp_info.Duration = -1;
	exp_info.Rotor = -1;
	for(int i=0; i<8; i++)
	{
		exp_info.Cell[i] = "";
		exp_info.CellID[i] = 0;
	}
	lbl_run->setText("not selected");
	lbl_investigator->setText("not selected");
	lbl_date->setText(current_date);
	lbl_runid->setText("");
	lbl_temp->setText("");
	lbl_dur->setText("");
	le_description->setText("");
	lbl_exp->setText("");
	lbl_opt->setText("");
	lbl_rt->setText("");
	lb_query->clear();
	cmbb_cell->clear();
	cmbb_cell->insertItem("No Data");
	pb_run->setEnabled(true);
	pb_investigator->setEnabled(true);
	pb_date->setEnabled(true);
	pb_add->setEnabled(true);
}

/*!
	This event handler, for event <var>e</var>, to receive widget close events.
*/
void US_ExpData_DB::closeEvent(QCloseEvent *e)
{
	QSqlQuery del_cell, del_expdata, del_rawdata;
		QString str;
//delete all relative cell data if the expData is not finally saved
	if(!all_done && !from_query)
	{
		str.sprintf("DELETE FROM tblCell WHERE ExperimentID = %d;", exp_info.ExpdataID);
		del_cell.exec(str);
		str.sprintf("DELETE FROM tblExpData WHERE ExpdataID = %d;", exp_info.ExpdataID);
		del_expdata.exec(str);
		str.sprintf("DELETE FROM tblRawExpData WHERE ExpdataID = %d;", exp_info.ExpdataID);
		del_rawdata.exec(str);

	}
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
}

/*! Open a netscape browser to load help page.*/
void US_ExpData_DB::help()
{
	US_Help *online_help; 
	online_help = new US_Help(this);
	online_help->show_help("manual/expdata_db.html");

}

/*! Close the interface.*/
void US_ExpData_DB::quit()
{
	close();
}

/*!
	Use <tt>ZLIB</tt> functions to compress a regular text file.
	\param path the location of file.
	\param wholeFile the directory of a bunch of files need to be compressed.
	\param filter the condition for selecting files to compress.
*/
int US_ExpData_DB::create(QString path, QString wholeFile, QString filter)
{
	QString filename, file, END, str,temp;
	END = "$%^";
	
	QDir compress_dir;
	QStringList entries;
	compress_dir.setPath(path);	
	compress_dir.setNameFilter(filter);
	entries = compress_dir.entryList();
	int count =compress_dir.entryList().count(); 
	double total_steps = (int)count*1.1;
	pd_add = new QProgressDialog( "Waiting for DB...", 0, (int) total_steps, this,"pd_add", TRUE );
	pd_add->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	pd_add->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));	
	pd_add->setMinimumDuration(0);
	double steps = 0;
	pd_add->setProgress(0);
	QFile fr, fw, fin;
	compFile = path + wholeFile; 
	fw.setName(compFile);
	if(!fw.open(IO_WriteOnly | IO_Translate))
	{
		return -1;
	}
	for (QStringList::Iterator it = entries.begin(); it != entries.end(); ++it ) 
	{
		filename = (*it).latin1();
		file = path + filename;
//cout<<file<<endl;
		fr.setName(file);
		if (fr.exists())
		{	
			if(!fr.open(IO_ReadOnly))
			{
				return -3;
			}
			QTextStream dsr(&fr);
			QTextStream dsw(&fw);
			dsw<<filename<<"\n";
			while (!dsr.atEnd())
			{
				temp = dsr.readLine();
				dsw<<temp<<"\n";	
			}
			dsw<<END<<"\n";
			fr.close();
			steps++;
			pd_add->setProgress((int) steps);
		}
		else
		{
			return -2;
		}
	}
	fw.close();
	
	fin.setName(compFile);
	fin.open(IO_ReadOnly);
	gzFile outfile;
	char buf[4096];
	int size;
	QString out = compFile + ".gz";
 	outfile = gzopen(out, "wb");
 	while((size = fin.readBlock(buf, 4096))!= 0)
	{
		gzwrite(outfile, buf, size);
	}
	gzclose(outfile);
	fin.close();
	fw.remove();
	pd_add->setProgress((int) total_steps);
	pd_add->close();
	return count;
}

/*!
	Use <tt>ZLIB</tt> functions to uncompress and restore a compressed file.
	\param compFile the compressed filename.
	\param Steps a progress bar count for showing the uncompress progress.
*/
int US_ExpData_DB::retrieve(QString compFile, int Steps)
{
	double TotalSteps = (int)Steps*1.1;
	pd = new QProgressDialog( "Waiting for Data Retrieval...", 0, (int) TotalSteps, this,"pd", TRUE );
	pd->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	pd->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));	
	pd->setProgress(0);
	pd->setMinimumDuration(0);
	QString file, filename, wholeFile, temp, begin;
	QFile fin, fout, fr, fw;
	wholeFile = compFile;
	wholeFile.truncate(wholeFile.findRev(".", -1,false));	//erase .gz extension
	QString path = wholeFile;
	path.truncate(path.findRev("/", -1,false)); //erase the filename to get the path
/*	proc = new QProcess(this);
	proc->addArgument("gzip");
	proc->addArgument("-d");
	proc->addArgument(compFile);
	if(!proc->start()) //error
	{
			QMessageBox::message("Compress Error:", "Unable to uncompress data");
			return -1;
	}
	while(!proc->normalExit())	//wait for finishing to uncompress the file
	{
	};
*/
	gzFile infile;
	fin.setName(compFile);
	fout.setName(wholeFile);

	if (!fout.open(IO_WriteOnly))
	{
		QMessageBox::message("Attention", "The file:\n\n" 
		+ wholeFile + "\n\ncannot be opened for writing!\nPlease check and try again.");
		return(-1);
	}
	char buf[4096];
	infile = gzopen(compFile, "rb");
	int size;
 	while((size = gzread(infile, buf, 4096))!= 0)
	{
		fout.writeBlock(buf,size);
	}
	gzclose(infile);
	fout.close();
	fin.remove();
	
	double steps = (int) Steps*.1;
	pd->setProgress((int) steps);		//retrieve finish the uncompress job
	fr.setName(wholeFile);
	if (fr.exists())
	{	
		if(!fr.open(IO_ReadOnly))
		{
			return -2;
		}
		QTextStream dsr(&fr);
		
		while(!dsr.atEnd())
		{
			dsr>> filename;
			dsr.readLine();
			file = path + "/" + filename;
			//cout<<file<<endl;
			fw.setName(file);
			if(!fw.open(IO_WriteOnly | IO_Translate))
			{
				return -3;
			}
			QTextStream dsw(&fw);
			while((temp = dsr.readLine()) != "$%^")
			{
				dsw << temp<<"\n";
			}
			fw.close();
			steps++;
			pd->setProgress((int) steps);
		}
		pd->close();
		fr.close();
		fr.remove();
		return 0;	
	}
	else
	{
		//Wholefile not exist.
		return -1;
	}
}

/*!
 	Create a file: <var>db_info.dat</var> to store centerpiece, DNA, buffer, peptide info.
	\param dir the same directory as uncompressed files' directory.
*/
void US_ExpData_DB::write_dbinfo(QString dir)
{
	QString db_file = dir + "/db_info.dat";
cerr << "Directory: " << db_file << endl;
	//QString db_file=make_tempFile(dir, "db_info.dat");
	QFile f(db_file);
	if(!f.open(IO_WriteOnly | IO_Translate))
	{
		cout<<"CAN NOT OPEN"<<endl;
		QMessageBox::message(tr("Attention:"), 
		tr("The file: \n '") + db_file+ "'\n" +
		tr("cannot be opened.\n"
		"Please check if the disk is write protected,\n"
		"if the directory exists, or has enough diskspace!"));
		return;
	}
	QTextStream ts (&f);
	ts << exp_info.ExpdataID <<"	# Experimental Data ID \n";
	ts << exp_info.Invid<<"   # Investigator ID \n";
	ts << exp_info.Date <<"	  # The date of experiment \n";
	ts << exp_info.Description << "\n";
	ts << login_list.dbname << "	# database name\n";
	ts << login_list.host << "	# database host\n";
	ts << login_list.driver << "	# database driver\n";
	ts << exp_info.Rotor<<"   # rotor serial number \n";
	for(int i=0; i<8; i++)
	{
		if(exp_info.CellID[i] != 0)
		{
			ts<<exp_info.centerpiece[i]<<"   # centerpiece serial number for cell "<<(i+1)<<"\n";
			for(int j=0; j<4; j++)
			{
				ts<<bufferid[i][j]<<"\t"<<" # buffer serial number for cell "<<(i+1)<<", channel "<<(j+1)<<"\n";
				for(int k=0; k<3; k++)
				{
					ts<<pepid[i][j][k]<<"\t"<<" # peptide"<<(k+1)<<" serial number for cell "<<(i+1)<<", channel "<<(j+1)<<"\n";
					ts<<DNAid[i][j][k]<<"\t"<<" # DNA"<<(k+1)<<" serial number for cell "<<(i+1)<<", channel "<<(j+1)<<"\n";
				}
			}			
		}
	}
	f.close();
}

/*!
	Retrieve the selected experimental data from database. 
	used by US_DB_RtvDate(), US_DB_RtvDescription(), US_DB_RtvEditType(), US_DB_RtvInvestigator().
	\return true retrieve sucessfully.
	\return false otherwise.
*/
bool US_ExpData_DB::retrieve_all(int ExpdataID, QString Display)
{
		int Step = 0;
		QSqlCursor cur( "tblExpData" );
		
		QStringList orderFields = QStringList()<<"Path"<<"Invid"<<"Date"<<"Edit_type"<<"Runid"
														<<"Temperature"<<"Duration"<<"Rotor"<<"Description"
														<<"Cell1"<<"Cell2"<<"Cell3"<<"Cell4"
														<<"Cell5"<<"Cell6"<<"Cell7"<<"Cell8";
   	QSqlIndex order = cur.index( orderFields );
   	QSqlIndex filter = cur.index( "ExpdataID" );
   	cur.setValue( "ExpdataID", ExpdataID );
		cur.select(filter, order);
		while(cur.next())
		{
			exp_info.ExpdataID = ExpdataID;
			exp_info.Description = cur.value("Description").toString();
			exp_info.Path = cur.value("Path").toString();
			exp_info.Invid = cur.value("Invid").toInt();
			exp_info.Date = cur.value("Date").toString();
			exp_info.Edit_type = cur.value("Edit_type").toInt();
			exp_info.Runid = cur.value("Runid").toString();
			exp_info.Temperature = cur.value("Temperature").toString().toFloat();
			exp_info.Duration = cur.value("Duration").toString().toFloat();
			exp_info.Rotor = cur.value("Rotor").toInt();
			QString cell_Str, cellid_Str;
			for(int i=0; i<8; i++)
			{
				cell_Str = "";
				cell_Str.sprintf("Cell%d", i+1);
				exp_info.Cell[i] = cur.value(cell_Str).toString();
				cellid_Str = "";
				cellid_Str.sprintf("Cell%dID", i+1);
				exp_info.CellID[i] = cur.value(cellid_Str).toInt();
			}
			QSqlCursor get( "tblCell" );
			QString get_cellid, scans_str;
			for(int i=0; i<8; i++)
			{
				get_cellid.sprintf("CellID = %d",exp_info.CellID[i]);
				get.select(get_cellid);
				if(get.next())
				{
					// query Cell Table to get the number of total scans file
					for(int j=0; j<3; j++)
					{
						scans_str = "";
						scans_str.sprintf("Wl_Scans_%d",j+1);
						Step += get.value(scans_str).toInt();
					}
					//get centerpiece, DNA, buffer,peptide data for write_dbinfo()
					exp_info.centerpiece[i] = get.value("CenterpieceID").toInt();
					QString DNA_Str, Buff_Str, Pep_Str;
					for(int j= 0; j<4; j++)
					{
						Buff_Str = "";
						Buff_Str.sprintf("BufferID_%d",j+1);
						bufferid[i][j] = get.value(Buff_Str).toInt();
						for(int k=0; k<3; k++)
						{
							if(k==0)
							{
								Pep_Str = "";
								Pep_Str.sprintf("PeptideID_%d",j+1);
								pepid[i][j][k] = get.value(Pep_Str).toInt();
								DNA_Str = "";
								DNA_Str.sprintf("DNAID_%d",j+1);
								DNAid[i][j][k] = get.value(DNA_Str).toInt();
							}
							else
							{
								Pep_Str = "";
								Pep_Str.sprintf("Peptide%dID_%d",k+1,j+1);
								pepid[i][j][k] = get.value(Pep_Str).toInt();
								DNA_Str = "";
								DNA_Str.sprintf("DNA%dID_%d",k+1,j+1);
								DNAid[i][j][k] = get.value(DNA_Str).toInt();
							}
						}
					}
				}
			}
/*			QString str;
			QSqlCursor cur_f( "tblExpDataFile" );
			str.sprintf("ExpdataID = %d",exp_info.ExpdataID);
			cur_f.select(str);
			if(cur_f.next())
			{
				int size = cur_f.value("File").toByteArray().size();
				QByteArray da(size);
				da = cur_f.value("File").toByteArray();
				QDir uncompress_dir;
				QString temp_dir = data_path +"/" + exp_info.Runid;
				if(uncompress_dir.exists(temp_dir))
				{
					QMessageBox::message(tr("Attention:"), 
										tr("The directory: \n '") + temp_dir+ "'\n" +
										tr("already exists in your data directory.\n"
										"If you want to retrieve this dataset you\n"
										"have to remove the existing directory first!"));
					return (false);
				}
				else
				{
					uncompress_dir.mkdir(temp_dir);
					write_dbinfo(temp_dir);
					QString tempfile = temp_dir +"/expdata.zlib.gz";
					QFile fw(tempfile);
					fw.open(IO_WriteOnly);
					fw.writeBlock(da.data(),size);
					fw.close();
					int result = retrieve(tempfile, Step);
					if(result<0)
					{
						if(result==-1)
						{
							str = "Can not open a temp file for writing";
						}
						if(result==-2)
						{
							str=" Can not open the temp file for reading";
						}
						if(result==-3)
						{
							str="Can not write the data to seperated files";
						}
						QMessageBox::message("Attention:", str);
						return (false);
					}
					QMessageBox::message(tr("Please note:"),
									tr("The selected dataset has been retrieved to the directory:\n '")
									 + temp_dir+ "'\n" + 
									 tr("You can review the associated data\n"
										"in the experimental data table.\n"));
				}	
			}
		*/	
			QString str;
			QSqlCursor cur_f( "tblRawExpData" );
			str.sprintf("ExpdataID = %d",exp_info.ExpdataID);
			cur_f.select(str);
			if(cur_f.next())
			{
				QDir rawdata_dir(data_path +"/" + exp_info.Runid);
				if(rawdata_dir.exists())
				{
					QMessageBox::message(tr("Attention:"), 
									tr("The directory: \n '") + data_path + exp_info.Runid+ "'\n" +
										tr("already exists in your data directory, \n"
										"if you want to retrieve the data data again\n"
										"from the database, you will have to remove\n"
										"this directory first"));
				 	return (false);
			 	}

				pd = new QProgressDialog( "Waiting for Data Retrieval...", 0, 4, this,"pd", TRUE );
				pd->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
				pd->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));	
				pd->setProgress(0);
				pd->setMinimumDuration(0);
				
		   	QString filename = make_tempFile(data_path, exp_info.Runid+".tar.gz");
         	bool raw_flag=read_blob("RawData", cur_f, filename);
         	if(!raw_flag)
         	{
					QMessageBox::message("UltraScan Error:", "Unable to retrieve Raw data files.");
					return (false);
				}
				pd->setProgress(1);
				uncompress_proc = new QProcess(this);
				uc_step=0;   
				connect(uncompress_proc, SIGNAL(processExited()), this, SLOT(endUncompressProcess()));
				endUncompressProcess();
				disconnect(uncompress_proc);					
			}
			else
			{
				QMessageBox::message(tr("Attention:"), 
										tr("No Raw Data found in Database for ExpdataID=")+QString::number(exp_info.ExpdataID));
				return (false);

			}
		}
	
		from_query = true;
		query_flag = false;
		lb_query->insertItem(Display);
		lbl_run->setText(exp_info.Path);
		lbl_investigator->setNum(exp_info.Invid);
		lbl_date->setText(exp_info.Date);
		lbl_runid->setText(exp_info.Runid);
		lbl_temp->setNum(exp_info.Temperature);
		lbl_dur->setNum(exp_info.Duration);
		le_description->setText(exp_info.Description);
		update_type(exp_info.Edit_type);
		update_rotor(exp_info.Rotor);
		update_cell();
		pb_run->setEnabled(false);
		pb_investigator->setEnabled(false);
		pb_date->setEnabled(false);
		pb_add->setEnabled(false);
		pb_query->setEnabled(false);
		pb_reset->setEnabled(false);
		return (true);
}
