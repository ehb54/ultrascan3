#include "../include/us_db_veloc.h"

US_DB_Veloc::US_DB_Veloc(QWidget *p, const char *name) : US_DB(p, name)
{
	exp_rst.expRstID = -1;
	exp_rst.invID = -1;
	from_HD = false;
	from_DB = false;
	query_flag = false;
	retrieve_flag = false;
	del_flag = false;
	run_id = "";
	runrequestID = 0;
	DataID =0;


	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	
	pb_load = new QPushButton(tr("Load Result from HD"), this);
	Q_CHECK_PTR(pb_load);
	pb_load->setAutoDefault(false);
	pb_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_load->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_load, SIGNAL(clicked()), SLOT(load_HD()));
	
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

	pb_retrieve_db = new QPushButton(tr("Retrieve Result from DB"), this);
	Q_CHECK_PTR(pb_retrieve_db);
	pb_retrieve_db->setAutoDefault(false);
	pb_retrieve_db->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_retrieve_db->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_retrieve_db, SIGNAL(clicked()), SLOT(retrieve_db()));

	lbl_instr = new QLabel(tr("Doubleclick on result data to select:"),this);
	lbl_instr->setAlignment(AlignHCenter|AlignVCenter);
	lbl_instr->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_instr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2, QFont::Bold));
	
	pb_del_db = new QPushButton(tr("Delete Result from DB"), this);
	Q_CHECK_PTR(pb_del_db);
	pb_del_db->setAutoDefault(false);
	pb_del_db->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_del_db->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_del_db, SIGNAL(clicked()), SLOT(check_permission()));
		
	lb_result = new QListBox(this, "Result files");
	lb_result->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lb_result->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(lb_result, SIGNAL(selected(int)), SLOT(select_result(int)));
	
	pb_reset = new QPushButton(tr("Reset"), this);
	Q_CHECK_PTR(pb_reset);
	pb_reset->setAutoDefault(false);
	pb_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_reset->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));
	
	pb_display = new QPushButton(tr("Show Selected Result"), this);
   pb_display->setAutoDefault(false);
   pb_display->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_display->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_display->setEnabled(true);
   connect(pb_display, SIGNAL(clicked()), SLOT(display()));

	pb_runrequest = new QPushButton(tr("Select Run Request"), this);
	pb_runrequest->setAutoDefault(false);
	pb_runrequest->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_runrequest->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_runrequest, SIGNAL(clicked()), SLOT(sel_runrequest()));
	
	lbl_runrequest= new QLabel("",this);
	lbl_runrequest->setAlignment(AlignLeft|AlignVCenter);
	lbl_runrequest->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);	
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

	pb_investigator = new QPushButton(tr("Investigator ID:"), this);
	pb_investigator->setAutoDefault(false);
	pb_investigator->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_investigator->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_investigator, SIGNAL(clicked()), SLOT(sel_investigator()));

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
	
	lbl_description = new QLabel(tr(" Run ID:"),this);
	lbl_description->setAlignment(AlignLeft|AlignVCenter);
	lbl_description->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_description->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	le_dscrpt = new QLabel("",this);
	le_dscrpt->setAlignment(AlignLeft|AlignVCenter);
	le_dscrpt->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	le_dscrpt->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_dscrpt->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	
	setup_GUI();
}

US_DB_Veloc::~US_DB_Veloc()
{
}

void US_DB_Veloc::setup_GUI()
{
	int j=0;
	int rows = 9, columns = 1, spacing = 2;
	
	QGridLayout * background = new QGridLayout(this,2,1,spacing);	
	QGridLayout * subGrid1 = new QGridLayout(rows, columns, spacing);
	for (int i=0; i<rows; i++)
	{
		subGrid1->setRowSpacing(i, 26);
	}
	subGrid1->addWidget(pb_load,j,0);
	j++;
	subGrid1->addWidget(pb_load_db,j,0);
	j++;
	subGrid1->addWidget(pb_retrieve_db,j,0);
	j++;
	subGrid1->addWidget(lbl_instr,j,0);
	j++;
	subGrid1->addMultiCellWidget(lb_result,j,j+4,0,0);
	subGrid1->setRowStretch(j,5);
	
	rows = 9, columns = 4, spacing = 2, j=0;
	QGridLayout * subGrid2 = new QGridLayout(rows, columns, spacing);
	for (int i=0; i<rows; i++)
	{
		subGrid2->setRowSpacing(i, 26);
	}
	subGrid2->addMultiCellWidget(pb_save_db,j,j,0,1);
	j++;
	subGrid2->addMultiCellWidget(pb_del_db,j,j,0,1);
	j++;
	subGrid2->addWidget(pb_runrequest,j,0);
	subGrid2->addWidget(lbl_runrequest,j,1);
	j++;
	subGrid2->addWidget(pb_investigator,j,0);
	subGrid2->addWidget(lbl_invst,j,1);
	j++;
	subGrid2->addWidget(lbl_expdata,j,0);
	subGrid2->addWidget(lbl_expdt,j,1);
	j++;
	subGrid2->addWidget(lbl_date,j,0);
	subGrid2->addWidget(lbl_dt,j,1);
	j++;
	subGrid2->addWidget(lbl_description,j,0);
	subGrid2->addWidget(le_dscrpt,j,1);
	j++;
	subGrid2->addWidget(pb_display,j,0);
	subGrid2->addWidget(pb_reset,j,1);
	j++;
	subGrid2->addWidget(pb_help,j,0);
	subGrid2->addWidget(pb_close,j,1);
	
	background->addLayout(subGrid1,0,0);
	background->setColSpacing(0,450);
	background->addLayout(subGrid2,0,1);
	background->setColStretch(0,3);

	background->activate();
}

void US_DB_Veloc::sel_runrequest()
{
	US_DB_RunRequest *runrequest_dlg;
	runrequest_dlg = new US_DB_RunRequest();
	runrequest_dlg->setCaption("Run Request Information");
	runrequest_dlg->show();
	connect(runrequest_dlg, SIGNAL(issue_RRID(int)), SLOT(update_runrequest_lbl(int)));
}

void US_DB_Veloc::update_runrequest_lbl (int RRID)
{
	runrequestID = RRID;
	lbl_runrequest->setNum(runrequestID);
	if(runrequestID == 0)
	{
		lbl_runrequest->setText(" Not Selected");
	}
}

void US_DB_Veloc::load_HD()
{
	reset();
	data_control = new Data_Control_W(7);	//velocity data
	data_control->load_data();
	run_id = data_control->run_inf.run_id;
	htmlDir = data_control->htmlDir;

	exp_rst.expRstID = data_control->run_inf.expdata_id;
	lbl_expdt->setNum(exp_rst.expRstID);
	exp_rst.invID = data_control->run_inf.investigator;
	lbl_invst->setNum(exp_rst.invID);
	exp_rst.date = data_control->run_inf.date;
	lbl_dt->setText(exp_rst.date);
	le_dscrpt->setText(run_id);	
	lb_result->clear();
	lb_result->insertItem(tr("Data shown is loaded from Hard Drive"));
	from_HD = true;
	
	if(htmlDir !="")
	{
		US_Report_Veloc *report_veloc;
		report_veloc = new US_Report_Veloc(false);
		report_veloc->write_file(data_control);
	}

}

/*!
	Open US_DB_TblInvestigator interface for selecting investigator.
*/
void US_DB_Veloc::sel_investigator()
{
	US_DB_TblInvestigator *investigator_dlg;
	investigator_dlg = new US_DB_TblInvestigator();
	investigator_dlg->setCaption("Investigator Information");
	investigator_dlg->pb_exit->setText("Accept");
	connect(investigator_dlg, SIGNAL(valueChanged(QString, int)), SLOT(update_investigator_lbl(QString, int)));
	investigator_dlg->exec();
}

void US_DB_Veloc::update_investigator_lbl (QString Display, int InvID)
{
	QString str;
	exp_rst.invID = InvID;
	str = Display;
	lbl_invst->setText(str);
	if(str == "")
	{
		lbl_invst->setText("");
	}
}

void US_DB_Veloc::save_db()
{
	if(!from_HD)
	{
		QMessageBox::message(tr("Attention:"), 
									tr("Please load result data from harddrive first.\n"));
		return;
	}
	if(exp_rst.expRstID<=0)
	{
		QMessageBox::message(tr("Attention:"), 
									tr("No database information for this run!"));
		return;

	}

	//check whether the raw data exist
	db_connect();
/*
	QString str="SELECT ExpdataID FROM tblRawExpData WHERE ExpdataID = '"+ QString::number(exp_rst.expRstID)+"';";
	QSqlQuery query(str);
	if(query.isActive())
	{
		if(!query.next() )
		{

			QMessageBox::message(tr("Attention:"), 
			tr("Cannot find any raw experimental data from DB!\n"));
			return;
		}
	}
*/
//Insert Compress data into tblVelocResult and tblVelocResultData
	if(!insertCompressData())
	{
		QMessageBox::message(tr("Attention:"), tr("Insert compress data failed.\n"));
      exit(-1);
	}
	
}

bool US_DB_Veloc::insertCompressData()
{
	pd = new QProgressDialog( "Please wait while your data is saved to the database...", 0, 8, this,"pd", TRUE );
	pd->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
//	pd->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));	
	pd->setProgress(0);
	pd->setMinimumDuration(0);
	
	compress_proc = new QProcess(this);
	c_step = 0;   
	c_flag = true;
	connect(compress_proc, SIGNAL(processExited()), this, SLOT(endCompressProcess()));
	endCompressProcess();
//	disconnect(compress_proc);             	
	return c_flag;
}

void US_DB_Veloc::endCompressProcess()
{
	QDir work_dir;
	QString str, tarfile, tardir; 
	             
	switch(c_step)
	{
		case 0:			//tar report file	
		{
			work_dir.setPath(USglobal->config_list.html_dir);
			tarfile = run_id + "_report.tar";
			tardir = run_id + "/";
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
			pd->setProgress(c_step);
			if(!compress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to tar velocity report data archive.");
				c_flag = false;
				return;
			}
			break;

		}
		case 1:			//gzip report tar file	
		{
			work_dir.setPath(USglobal->config_list.html_dir);
			tarfile = run_id + "_report.tar";
			compress_proc->clearArguments();
         compress_proc->setWorkingDirectory(work_dir);
			QStringList cmd;
			cmd.append("gzip");
			cmd.append("-9");
			cmd.append(tarfile);
			compress_proc->setArguments(cmd);
			c_step=4;
			pd->setProgress(c_step);
			if(!compress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to gzip velocity report data archive.");
				c_flag = false;
				return;
			}
			break;

		}
/*
		case 2:			//tar raw data tar file 
		{
						
			work_dir.setPath(USglobal->config_list.data_dir);
			tarfile = run_id + "_rawdata.tar";
			tardir = run_id + "/";
			compress_proc->clearArguments();
         compress_proc->setWorkingDirectory(work_dir);
			QStringList cmd;
			cmd.append("tar");
			cmd.append("--mode=u+X");
			cmd.append("-cvf");
			cmd.append(tarfile);
			cmd.append(tardir);			
			compress_proc->setArguments(cmd);
			c_step=3;
			pd->setProgress(c_step);
			if(!compress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to tar veloc raw data archive.");
				return;
			}
			break;
		}
		case 3:			//gzip raw data tar file 
		{
						
			work_dir.setPath(USglobal->config_list.data_dir);
			tarfile = run_id + "_rawdata.tar";
			compress_proc->clearArguments();
         compress_proc->setWorkingDirectory(work_dir);
			QStringList cmd;
			cmd.append("gzip");
			cmd.append("-9");
			cmd.append(tarfile);			
			compress_proc->setArguments(cmd);
			c_step=4;
			pd->setProgress(c_step);
			if(!compress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to gzip veloc raw data archive.");
				return;
			}
			break;
		}
*/		
		case 4:			//tar result tar file
		{
			work_dir.setPath(USglobal->config_list.result_dir);
			tarfile = run_id + "_result.tar";
								
			compress_proc->clearArguments();
         compress_proc->setWorkingDirectory(work_dir);
			QStringList cmd;
			cmd.append("tar");
			cmd.append("-cvf");
			cmd.append(tarfile);
#ifdef WIN32
			QString filter=run_id + "\*";
			cmd.append(filter);
#endif
#ifdef UNIX
			QString filter=run_id + "*";
			work_dir.setNameFilter(filter);	
			QStringList entries = work_dir.entryList();
			for (QStringList::Iterator it = entries.begin(); it  != entries.end(); ++it ) 
			{
				cmd.append((*it).latin1());
			}

#endif
			compress_proc->setArguments(cmd);
			c_step=5;
			pd->setProgress(c_step);
			if(!compress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to tar veloc result archive.");
				c_flag = false;
				return;
			}
			break;
		}


		case 5:			//gzip result tar file
		{
			work_dir.setPath(USglobal->config_list.result_dir);
			tarfile = run_id + "_result.tar";
						
			compress_proc->clearArguments();
         compress_proc->setWorkingDirectory(work_dir);
			QStringList cmd;
			cmd.append("gzip");
			cmd.append("-9");
			cmd.append(tarfile);
			compress_proc->setArguments(cmd);
			c_step=6;
			pd->setProgress(c_step);
			if(!compress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to gzip veloc result archive.");
				c_flag = false;
				return;
			}
			break;
		}

		case 6:	//insert to DB table tblVelocResultData
		{
			db_connect();
			QSqlCursor cur("tblVelocResultData");
   		cur.setMode( QSqlCursor::Insert);
   		QSqlRecord *buffer = cur.primeInsert();
			DataID = get_newID("tblVelocResultData","tableID");
   		buffer->setValue("tableID", DataID);
			fileName = USglobal->config_list.html_dir +"/"+ run_id + "_report.tar.gz";
			write_blob(fileName, buffer, "Report_Tar");
		//	fileName = USglobal->config_list.data_dir +"/"+ run_id+"_rawdata.tar.gz";
		//	write_blob(fileName, buffer, "Rawdata_tar");
			fileName = USglobal->config_list.result_dir +"/"+ run_id + "_result.tar.gz";
			write_blob(fileName, buffer, "Result_Tar");
			int result = cur.insert();
			if(result<=0)
			{
				QSqlError err = cur.lastError();
				QMessageBox::message(tr("Attention:"), 
                             tr("Saving to DB table 'tblVelocResultData' failed.\n"
										 	"Error message from MySQL:\n\n")
										  + err.text());
				c_flag = false;
				return;
			}
			c_step=7;
			pd->setProgress(c_step);
			if(!compress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable insert data to DB table tblVelocResultData.");
				return;
			}
			break;
		}
		case 7: //Insert data to DB table tblVelocResult
		{
			QString STR = "INSERT INTO tblVelocResult (VelocRstID, Date, RunID, InvestigatorID, DataID";
      	if(runrequestID > 0)
      	{
         	STR += ", RunRequestID";
      	}
      	STR +=") VALUES(";
      	STR += QString::number(exp_rst.expRstID);
			STR += ", '"+ exp_rst.date;
      	STR += "', '"+ run_id; 
			STR += "', "+QString::number(exp_rst.invID); 
			STR += ", "+QString::number(DataID);
      	if(runrequestID >0)
      	{
         	STR += ", " + QString::number(runrequestID);
      	}
      	STR +=");";
      	QSqlQuery target;
      	bool finished = target.exec(STR);
      	if(!finished)
      	{
				QSqlError sqlerr = target.lastError();
         	QMessageBox::message(tr("Attention:"), 
                             tr("Saving to DB table 'tblVelocResult' failed.\n"
										  "Attempted to execute this command:\n\n"
										  + STR + "\n\n"
										  "Causing the following error:\n\n")
										  + sqlerr.text());
				//delete inserted	tblVelocResultData data
				QSqlQuery del_data;
				str.sprintf("DELETE FROM tblVelocResultData WHERE tableID = %d;", DataID);
				del_data.exec(str);
				// clean the temp compress file	
				cleanCompressFile();
				c_flag = false;
				pd->close();
         	return;
      	}
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
				if(resultID > 0)		// row exists and update velocprojectID
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
									tr("Uploading data to the Result Database table failed!"));
						c_flag = false;
						return;
					}
				}
			}
			pd->setProgress(5);	
			// clean the temp compress file	
			cleanCompressFile();
			pd->close();

			QMessageBox::message(tr("Congratulation:"),  tr("Successfully save data to database.\n"));	
			
			delete compress_proc;
						
		}//end case 7
	}
}

void US_DB_Veloc::cleanCompressFile()
{
		QFile tempFile1(USglobal->config_list.html_dir +"/"+ run_id + "_report.tar.gz");
		if(tempFile1.exists())
		{
			tempFile1.remove();
		}
		QFile tempFile2(USglobal->config_list.data_dir +"/"+ run_id + "_rawdata.tar.gz");
		if(tempFile2.exists())
		{
			tempFile2.remove();
		}
		QFile tempFile3(USglobal->config_list.result_dir +"/"+ run_id + "_result.tar.gz");
		if(tempFile3.exists())
		{
			tempFile3.remove();
		}
		QFile tempFile4(USglobal->config_list.html_dir +"/"+ run_id + "_report.tar");
		if(tempFile4.exists())
		{
			tempFile4.remove();
		}
		QFile tempFile5(USglobal->config_list.data_dir +"/"+ run_id + "_rawdata.tar");
		if(tempFile5.exists())
		{
			tempFile5.remove();
		}
		QFile tempFile6(USglobal->config_list.result_dir +"/"+ run_id + "_result.tar");
		if(tempFile6.exists())
		{
			tempFile6.remove();
		}

}
	
void US_DB_Veloc::query_db()
{

	QString str;
	cleanCompressFile();
	db_connect();

	int maxID = get_newID("tblVelocResult","VelocRstID");

	int count = 0;
	item_ExpdataID = new int[maxID];
	item_Description = new QString[maxID];
	display_Str = new QString[maxID];
	if(exp_rst.invID>0)
	{
		str.sprintf("SELECT VelocRstID, RunID FROM tblVelocResult WHERE InvestigatorID = %d;", exp_rst.invID);
	}

	else
	{
		str.sprintf("SELECT VelocRstID, RunID FROM tblVelocResult;");
	}
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
//		instr_lbl->setText("Doubleclick item to select");
		query_flag = true;
	}
	else
	{
		lb_result->clear();
		lb_result->insertItem("No data were found in the DB");
	}
}

void US_DB_Veloc::select_result(int item)
{	
	QString str;
	if(query_flag)
	{
		exp_rst.expRstID = item_ExpdataID[item];
		run_id = item_Description[item];
		
		QSqlCursor cur( "tblVelocResult" );
   	str.sprintf("VelocRstID = %d", exp_rst.expRstID);
		cur.select(str);		
		if(cur.next())
		{
			exp_rst.invID = cur.value("InvestigatorID").toInt();
			exp_rst.date = cur.value("Date").toString();
			runrequestID = cur.value("RunRequestID").toInt();
			DataID = cur.value("DataID").toInt();
		}
		lbl_expdt->setNum(exp_rst.expRstID);
		lbl_invst->setText(show_investigator(exp_rst.invID));
		lbl_dt->setText(exp_rst.date);
		le_dscrpt->setText(run_id);
		if(runrequestID > 0)
		{
			lbl_runrequest->setNum(runrequestID);
			pb_runrequest->setEnabled(false);
		}
		else
		{
			lbl_runrequest->setText(" N/A");
		}

		del_flag = true;
		retrieve_flag = true;
	}
	else
   {
      QMessageBox::message("Warning:", "Please Query result from DB first.");
      return;
   }	
}

void US_DB_Veloc::retrieve_db()
{
	if(!query_flag)
   {
      QMessageBox::message("Warning:", "Please Query result from DB first.");
      return;
   }	
	if(!retrieve_flag)
	{
      QMessageBox::message("Warning:", "Please Double-Click to select result from list.");
      return;
   }	

	QString str, str1, str2, str3, filename;
	bool rep_flag = false;
	bool res_flag = false;
	bool raw_flag = false;
	cleanCompressFile();


	//check do there same run_id files exists
	 	QString namefilter = run_id +"*";
		QDir check_dir;
		QStringList entries;
		check_dir.setPath(USglobal->config_list.result_dir);	
		check_dir.setNameFilter(namefilter);
		entries = check_dir.entryList();
		int count =check_dir.entryList().count(); 
		if(count>0 && DataID>0)
		{
			switch(QMessageBox::information(this, "Warning", 
						"The result for '"+ run_id+
						"'\n already existed in your result directory: \n\n'"
						+USglobal->config_list.result_dir+
						"'\n\n Do you want to overwrite it?\n\n",
						"OK", "CANCEL",	0,1))
			{
				case 0:
				{
					break;
				}		
				case 1:
				{
					return;
				}		
			}
		}								
		QSqlCursor cur1( "tblVelocResultData" );
      str.sprintf("tableID = %d", DataID);
      cur1.select(str);
      if(cur1.next())
      {
			str1 = run_id +"_report.tar.gz";
         filename = make_tempFile(USglobal->config_list.html_dir+"/", str1);
         rep_flag=read_blob("Report_Tar", cur1, filename);
			if(!rep_flag)
         {               
          	QMessageBox::message("UltraScan Error:", "Unable to retrieve Report files.");
         }
			str2 = run_id +"_result.tar.gz";
         filename = make_tempFile(USglobal->config_list.result_dir+"/", str2);
         res_flag=read_blob("Result_Tar", cur1, filename);
         if(!res_flag)
         {
				QMessageBox::message("UltraScan Error:", "Unable to retrieve Result files.");
         }
		}
		QSqlCursor cur2( "tblRawExpData" );
      str.sprintf("ExpdataID = %d", exp_rst.expRstID);
      cur2.select(str);
      if(cur2.next())
      {
			str3 = run_id +"_rawdata.tar.gz";
         filename = make_tempFile(USglobal->config_list.data_dir+"/", str3);
         raw_flag=read_blob("Rawdata", cur2, filename);
         if(!raw_flag)
         {
				QMessageBox::message("UltraScan Error:", "Unable to retrieve Raw data files.");
			}
		}
		uncompress_proc = new QProcess(this);
		uc_step=0;   
		connect(uncompress_proc, SIGNAL(processExited()), this, SLOT(endUncompressProcess()));
		endUncompressProcess();
		disconnect(uncompress_proc);
		from_DB = true;													
}

void US_DB_Veloc::endUncompressProcess()
{
	QString tar_filename;
	switch(uc_step)
	{
		case 0:			//un zip report file	
		{

			uncompress_proc->clearArguments();
			tar_filename = run_id +"_report.tar.gz";
			QDir work_dir(USglobal->config_list.html_dir);
			uncompress_proc->setWorkingDirectory(work_dir);
			QStringList cmd;
			cmd.append("gzip");
			cmd.append("-d");
			cmd.append(tar_filename);
			uncompress_proc->setArguments(cmd);
			uc_step=1;
			if(!uncompress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to unzip Report files.");
				return;
			}
			break;
		}
		case 1:			//un tar report file
		{

			uncompress_proc->clearArguments();
			tar_filename = run_id +"_report.tar";
			QDir work_dir(USglobal->config_list.html_dir);
			uncompress_proc->setWorkingDirectory(work_dir);
			QStringList cmd;
			cmd.append("tar");
			cmd.append("-xvf");
			cmd.append(tar_filename);
			uncompress_proc->setArguments(cmd);
			uc_step=2;
			if(!uncompress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to un tar Report files.");
				return;
			}
			break;
		}	
		case 2:			//unzip result file
		{

			uncompress_proc->clearArguments();
			tar_filename = run_id +"_result.tar.gz";
			QDir work_dir(USglobal->config_list.result_dir);
			uncompress_proc->setWorkingDirectory(work_dir);
			QStringList cmd;
			cmd.append("gzip");
			cmd.append("-d");
			cmd.append(tar_filename);
			uncompress_proc->setArguments(cmd);
			uc_step=3;
			if(!uncompress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to unzip Report files.");
				return;
			}
			break;
		}

		case 3:			//un tar result file
		{

			uncompress_proc->clearArguments();
			tar_filename = run_id +"_result.tar";
			QDir work_dir(USglobal->config_list.result_dir);
			uncompress_proc->setWorkingDirectory(work_dir);
			QStringList cmd;
			cmd.append("tar");
			cmd.append("-xvf");
			cmd.append(tar_filename);
			uncompress_proc->setArguments(cmd);
			uc_step=4;
			if(!uncompress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to un tar Report files.");
				return;
			}
			break;
		}	
		case 4:			//unzip raw data file
		{

			uncompress_proc->clearArguments();
			tar_filename = run_id +"_rawdata.tar.gz";
			QDir work_dir(USglobal->config_list.data_dir);
			uncompress_proc->setWorkingDirectory(work_dir);
			QStringList cmd;
			cmd.append("gzip");
			cmd.append("-d");
			cmd.append(tar_filename);
			uncompress_proc->setArguments(cmd);
			uc_step=5;
			if(!uncompress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to unzip Raw data files.");
				return;
			}
			break;
		}

		case 5:			//un tar raw data file
		{

			uncompress_proc->clearArguments();
			tar_filename = run_id +"_rawdata.tar";
			QDir work_dir(USglobal->config_list.data_dir);
			uncompress_proc->setWorkingDirectory(work_dir);
			QStringList cmd;
			cmd.append("tar");
			cmd.append("-xvf");
			cmd.append(tar_filename);
			uncompress_proc->setArguments(cmd);
			uc_step=6;
			if(!uncompress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to un tar Raw data files.");
				return;
			}
			break;
		}
		case 6:
		{
			cleanCompressFile();	
			QMessageBox::message("Congratulations,", "Retrieve Data Successfully!");
			delete uncompress_proc;
		}	
	
	}
}

/*!
	Open US_DB_Admin to check delete permission.
*/
void US_DB_Veloc::check_permission()
{
	US_DB_Admin *db_admin;
	db_admin = new US_DB_Admin("");
	db_admin->show();
	connect(db_admin, SIGNAL(issue_permission(bool)), SLOT(delete_db(bool)));
}

/*!
	Delete one selected experimental result table entry.
*/
void US_DB_Veloc::delete_db(bool permission)
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
		QSqlCursor cur( "tblVelocResult");
		cur.setMode( QSqlCursor::Delete);
		QSqlIndex filter = cur.index("VelocRstID");
		cur.setValue("VelocRstID", exp_rst.expRstID);
		cur.primeDelete();

		switch(QMessageBox::information(this, tr("Delete this record?"), 
										tr("Clicking 'OK' will delete the selected result data from the database"),
										tr("OK"), tr("CANCEL"),	0,1))
		{
			case 0:
			{
				QSqlQuery query;
				str.sprintf("UPDATE tblResult SET VelocDataID = NULL WHERE VelocDataID = %d;", exp_rst.expRstID);
				query.exec(str);
				if(DataID>0)
				{
					QSqlQuery del_file;
					str.sprintf("DELETE FROM tblVelocResultData WHERE tableID = %d;", DataID);
					del_file.exec(str);
				}
				cur.del();
				reset();
				query_db();
				break;
			}	
			case 1:
			{
				break;
			}
		}
	}
}

void US_DB_Veloc::display()
{
	QString indexFile;
   if(from_HD||from_DB)
   {
      indexFile = USglobal->config_list.html_dir+"/"+run_id + "/index.html";
   }
	else
	{
		QMessageBox::message(tr("Attention:"), tr("Please retrieve result from DB first\n"));									
		return;
	}
   
   US_Help *online_help; 
   online_help = new US_Help();
   online_help->show_html_file(indexFile);
}

void US_DB_Veloc::reset()
{
	cleanCompressFile();
	from_HD = false;
	from_DB = false;
	query_flag = false;
	retrieve_flag = false;
	del_flag = false;
	
	run_id = "";
	source_type = -1;
	runrequestID = 0;
	exp_rst.invID = -1;
	exp_rst.expRstID = -1;
	exp_rst.date = "";
	run_id = "";
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
	lb_result->clear();
}

/*! Open a netscape browser to load help page.*/
void US_DB_Veloc::help()
{
	US_Help *online_help; 
	online_help = new US_Help(this);
	online_help->show_help("manual/us_db_result_veloc.html");

}

/*! Close the interface.*/
void US_DB_Veloc::quit()
{
	cleanCompressFile();
	close();
}

/*!
	This event handler, for event <var>e</var>, to receive widget close events.
*/
void US_DB_Veloc::closeEvent(QCloseEvent *e)
{
	cleanCompressFile();
	e->accept();
}


