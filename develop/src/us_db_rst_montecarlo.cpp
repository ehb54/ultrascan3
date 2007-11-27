#include "../include/us_db_rst_montecarlo.h"

US_DB_RST_Montecarlo::US_DB_RST_Montecarlo(QWidget *p, const char *name) : US_DB(p, name)
{
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	pd = NULL;
	compress_proc = NULL;
	from_HD = false;
	from_DB = false;
	query_flag = false;
	for(int i=0; i<15; i++)
	{
		modelID[i] = 0;
	}
	projectID =0;
	investigatorID = 0;
	runrequestID = 0;

	lbl_blank = new QLabel(tr(" Monte Carlo Project Result: "), this);
	lbl_blank->setAlignment(AlignCenter|AlignVCenter);
//		lbl_blank->setFrameStyle(QFrame::WinPanel|Raised);
	lbl_blank->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_blank->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	pb_hd = new QPushButton(tr("Load from HD"),this);
	pb_hd->setAutoDefault(false);
	pb_hd->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_hd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	connect(pb_hd, SIGNAL(clicked()), SLOT(load_HD()));

	pb_db = new QPushButton("Load from DB", this);
	pb_db->setAutoDefault(false);
	pb_db->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_db->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	connect(pb_db, SIGNAL(clicked()), SLOT(load_DB()));

	pb_retrieve = new QPushButton("Retrieve from DB", this);
	pb_retrieve->setAutoDefault(false);
	pb_retrieve->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_retrieve->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_retrieve->setEnabled(false);
	connect(pb_retrieve, SIGNAL(clicked()), SLOT(retrieve()));

	pb_investigator = new QPushButton(tr("Select Investigator"), this);
	pb_investigator->setAutoDefault(false);
	pb_investigator->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_investigator->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_investigator, SIGNAL(clicked()), SLOT(sel_investigator()));

	lbl_investigator= new QLabel("",this);
	lbl_investigator->setAlignment(AlignLeft|AlignVCenter);
//		lbl_investigator->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);	
	lbl_investigator->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_investigator->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	lbl_investigator->setText(tr(" Not Selected"));

	pb_runrequest = new QPushButton(tr("Select Run Request"), this);
	pb_runrequest->setAutoDefault(false);
	pb_runrequest->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_runrequest->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_runrequest, SIGNAL(clicked()), SLOT(sel_runrequest()));

	lbl_runrequest= new QLabel("",this);
	lbl_runrequest->setAlignment(AlignLeft|AlignVCenter);
//		lbl_runrequest->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);	
	lbl_runrequest->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_runrequest->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	lbl_runrequest->setText(tr(" Not Selected"));

	lbl_instr = new QLabel(tr("Doubleclick on item to select:"),this);
	lbl_instr->setAlignment(AlignCenter|AlignVCenter);
	lbl_instr->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_instr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lb_result = new QListBox(this, "result");
	lb_result->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	lb_result->setSelected(0, true);
	connect(lb_result, SIGNAL(selected(int)), SLOT(select_result(int)));

	lbl_item = new QLabel(" not selected",this);
	lbl_item->setAlignment(AlignLeft|AlignVCenter);
//		lbl_item->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_item->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_item->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	pb_save = new QPushButton(tr("Save to DB"), this);
	pb_save->setAutoDefault(false);
	pb_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_save->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_save->setEnabled(true);
	connect(pb_save, SIGNAL(clicked()), SLOT(save_db()));

	pb_display = new QPushButton(tr("Show Selected Result"), this);
	pb_display->setAutoDefault(false);
	pb_display->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_display->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_display->setEnabled(true);
	connect(pb_display, SIGNAL(clicked()), SLOT(display()));

	pb_delete = new QPushButton(tr("Delete DB"), this);
	pb_delete->setAutoDefault(false);
	pb_delete->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_delete->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_delete->setEnabled(true);
	connect(pb_delete, SIGNAL(clicked()), SLOT(check_permission()));

	pb_reset = new QPushButton(tr("Reset"), this);
	pb_reset->setAutoDefault(false);
	pb_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_reset->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_reset->setEnabled(true);
	connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));

	pb_help = new QPushButton(tr("Help"), this);
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_help->setEnabled(true);
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	pb_close = new QPushButton(tr("Close"), this);
	pb_close->setAutoDefault(false);
	pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_close->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_close->setEnabled(true);
	connect(pb_close, SIGNAL(clicked()), SLOT(quit()));

	global_Xpos += 30;
	global_Ypos += 30;
	setup_GUI();
}

US_DB_RST_Montecarlo::~US_DB_RST_Montecarlo()
{
}

void US_DB_RST_Montecarlo::setup_GUI()
{
	QGridLayout * Grid = new QGridLayout(this,12, 2, 4, 2);
	unsigned int j=0;
	Grid->addMultiCellWidget(lbl_blank, j, j, 0, 1);
	j++;
	Grid->addMultiCellWidget(pb_hd, j, j, 0, 1);
	j++;
	Grid->addMultiCellWidget(pb_db, j, j, 0, 1);
	j++;
	Grid->addMultiCellWidget(pb_retrieve, j, j, 0, 1);
	j++;
	Grid->addWidget(pb_investigator, j, 0);
	Grid->addWidget(lbl_investigator, j, 1);
	j++;
	Grid->addWidget(pb_runrequest, j, 0);
	Grid->addWidget(lbl_runrequest, j, 1);
	j++; 
	Grid->addMultiCellWidget(lbl_instr, j, j, 0, 1);
	j++;
	Grid->addMultiCellWidget(lb_result, j, j, 0, 1);
	j++;
	Grid->addMultiCellWidget(lbl_item, j, j, 0, 1);
	j++;
	Grid->addWidget(pb_save, j, 0);
	Grid->addWidget(pb_display, j, 1);
	j++;
	Grid->addWidget(pb_delete, j, 0);
	Grid->addWidget(pb_reset, j, 1);
	j++;
	Grid->addWidget(pb_help, j, 0);
	Grid->addWidget(pb_close, j, 1);
	Grid->activate();
}

/*!
Open US_DB_TblInvestigator interface for selecting investigator.
*/
void US_DB_RST_Montecarlo::sel_investigator()
{
	US_DB_TblInvestigator *investigator_dlg;
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
void US_DB_RST_Montecarlo::update_investigator_lbl (QString Display, int InvID)
{
	QString str;
	investigatorID = InvID;
	str = Display;
	lbl_investigator->setText(str);
	if(str == "")
	{
		lbl_investigator->setText(" Not Selected");
	}
}

void US_DB_RST_Montecarlo::sel_runrequest()
{
	US_DB_RunRequest *runrequest_dlg;
	runrequest_dlg = new US_DB_RunRequest();
	runrequest_dlg->setCaption("Run Request Information");
	runrequest_dlg->show();
	connect(runrequest_dlg, SIGNAL(issue_RRID(int)), SLOT(update_runrequest_lbl(int)));
}

void US_DB_RST_Montecarlo::update_runrequest_lbl (int RRID)
{
	runrequestID = RRID;
	lbl_runrequest->setNum(runrequestID);
	if(runrequestID == 0)
	{
		lbl_runrequest->setText(" Not Selected");
	}
}

void US_DB_RST_Montecarlo::load_HD()
{
	QString str;
	reset();
	fileName = QFileDialog::getOpenFileName(USglobal->config_list.result_dir, "*.Monte-Carlo", 0);
	parameterFile = fileName;
	parameters = 0;

	parameter_name.clear();
	QFile projectFile(fileName);
	projectFile.open(IO_ReadOnly);
	QTextStream project_ts(&projectFile);
	projectName = project_ts.readLine();
	str = project_ts.readLine();
	parameters = str.toInt();
	for (int i=0; i<parameters; i++)
	{
		parameter_name.push_back(project_ts.readLine());
	}
	projectFile.close();
	
#ifdef WIN32
	htmlDir = USglobal->config_list.html_dir + "\\" + projectName + ".mc";
	baseName = htmlDir + "\\parameter-";
#else
	htmlDir = USglobal->config_list.html_dir + "/" + projectName + ".mc";
	baseName = htmlDir + "/parameter-";
#endif

	str = "File Path : " + fileName;
	lb_result->insertItem(str);
	lbl_item->setText("Result data loaded from harddrive");
	if (fileName.isEmpty())
	{
		QMessageBox::message(tr("Attention:"), 
								  tr("No file Selected.\n"));
		return;
	}
	else
	{	
		US_Report_MonteCarlo *report_montecarlo;
		report_montecarlo = new US_Report_MonteCarlo(false);
		report_montecarlo->generate(fileName);
		from_HD = true;
	}

}

void US_DB_RST_Montecarlo::save_db()
{
	QString str, num, temp_file;
	if(!from_HD)
	{
		QMessageBox::message(tr("Attention:"), 
								  tr("Please load result data from harddrive first.\n"));
		return;
	}
	if(!investigatorID)
	{
		QMessageBox::message(tr("Attention:"), 
								  tr("Please select investigator first.\n"));
		return;
	}
	db_connect();
	if(!insertCompressData())
	{
		QMessageBox::message(tr("Attention:"), tr("There was an error during the\nuploading of the Monte Carlo data.\n"));
      exit(-1);
	}
}

void US_DB_RST_Montecarlo::showOutput()
{
	cout << "Stdout output: " << compress_proc->readLineStdout() << endl;
	cerr << "Stderr output: " << compress_proc->readLineStderr() << endl;
}

bool US_DB_RST_Montecarlo::insertCompressData()
{
	if (pd == NULL)
	{
		pd = new QProgressDialog( "Please wait while your data is saved to the database...", 0, 8, this,"pd", TRUE );
		pd->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
		pd->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));	
	}
	pd->setProgress(0);
	pd->setTotalSteps(8);
	pd->setMinimumDuration(0);
	if(compress_proc != NULL)
	{
		delete compress_proc;
	}
	compress_proc = new QProcess(this);
	c_flag = true;
	c_step = 1;   
	connect(compress_proc, SIGNAL(processExited()), this, SLOT(endCompressProcess()));
	connect(compress_proc, SIGNAL(readyReadStdout()), this, SLOT(showOutput()));
	connect(compress_proc, SIGNAL(readyReadStderr()), this, SLOT(showOutput()));
	endCompressProcess(); // enter with c_step=0
	return c_flag;
}

void US_DB_RST_Montecarlo::clearTmpDir()
{
	QDir temp_dir;
#ifdef WIN32
	temp_dir.setPath(USglobal->config_list.root_dir + "\\tmp");
#else
	temp_dir.setPath(USglobal->config_list.root_dir + "/tmp");
#endif
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
#ifdef WIN32
		temp_dir.mkdir(USglobal->config_list.root_dir + "\\tmp");
#else
		temp_dir.mkdir(USglobal->config_list.root_dir + "/tmp");
#endif
	}
}

void US_DB_RST_Montecarlo::endCompressProcess()
{
	QDir work_dir;
	QString str, tarfile, tardir; 
	switch(c_step)
	{
		case 1:			//clean up temp directory	
		{
			pd->setLabelText("Clearing temporary directory...");
			clearTmpDir();
			pd->setProgress(c_step);
			c_step=2;
			pd->setLabelText("Archiving the Monte Carlo report...");
			work_dir.setPath(USglobal->config_list.html_dir);
#ifdef WIN32
			tarfile = USglobal->config_list.root_dir + "\\tmp\\" + projectName + "_report.tar";
#else
			tarfile = USglobal->config_list.root_dir + "/tmp/" + projectName + "_report.tar";
#endif
			compress_proc->clearArguments();
         compress_proc->setWorkingDirectory(work_dir);
			QStringList cmd;
			cmd.append("tar");
			cmd.append("--mode=u+X");
			cmd.append("-cvf");
			cmd.append(tarfile);
			cmd.append(projectName + ".mc");
			compress_proc->setArguments(cmd);
			pd->setProgress(c_step);
			if(!compress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to build tar archive from the Monte Carlo report data.");
				c_flag = false;
				c_step=1000;
				endCompressProcess();
			}
			break;
		}
		case 2:			//gzip report tar file	
		{
			pd->setLabelText("Compressing the Monte Carlo report...");
			work_dir.setPath(USglobal->config_list.root_dir + "/tmp");
			tarfile = projectName + "_report.tar";
			compress_proc->clearArguments();
         compress_proc->setWorkingDirectory(work_dir);
			QStringList cmd;
			cmd.append("gzip");
			cmd.append("-9");
			cmd.append(tarfile);
			compress_proc->setArguments(cmd);
			c_step=3;
			pd->setProgress(c_step);
			if(!compress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to compress Monte Carlo report tar archive.");
				c_flag = false;
				c_step=1000;
				endCompressProcess();
			}
			break;
		}
		case 3:			//create Monte Carlo result tar file in temporary directory
		{
			pd->setLabelText("Archiving the Monte Carlo data...");
			work_dir.setPath(USglobal->config_list.result_dir);
#ifdef WIN32
			tarfile = USglobal->config_list.root_dir + "\\tmp\\" + projectName + "_result.tar";
#else
			tarfile = USglobal->config_list.root_dir + "/tmp/" + projectName + "_result.tar";
#endif
			compress_proc->clearArguments();
         compress_proc->setWorkingDirectory(work_dir);
			QStringList cmd;
			cmd.append("tar");
			cmd.append("-cvf");
			cmd.append(tarfile);
			cmd.append(projectName + ".mc");
			cmd.append(projectName + ".Monte-Carlo");
			compress_proc->setArguments(cmd);
			c_step=4;
			pd->setProgress(c_step);
			if(!compress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to build tar archive from the Monte Carlo result data.");
				c_flag = false;
				c_step=1000;
				endCompressProcess();
			}
			break;
		}
		case 4:			//gzip result tar file in temporary directory
		{
			pd->setLabelText("Compressing tar file with Monte Carlo Result Data...");
			work_dir.setPath(USglobal->config_list.root_dir + "/tmp");
			tarfile = projectName + "_result.tar";
			compress_proc->clearArguments();
         compress_proc->setWorkingDirectory(work_dir);
			QStringList cmd;
			cmd.append("gzip");
			cmd.append("-9");
			cmd.append(tarfile);
			compress_proc->setArguments(cmd);
			c_step=5;
			pd->setProgress(c_step);
			if(!compress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to compress Monte Carlo result tar archive.");
				c_flag = false;
				c_step=1000;
				endCompressProcess();
			}
			break;
		}
		case 5:			// creating tar archive by combining the sub-archives
		{
			pd->setLabelText("Combining Monte Carlo report and result archives...");
			work_dir.setPath(USglobal->config_list.root_dir + "/tmp");
			compress_proc->clearArguments();
         compress_proc->setWorkingDirectory(work_dir);
			QStringList cmd;
			cmd.append("tar");
			cmd.append("--mode=u+X");
			cmd.append("-cvf");
			cmd.append("montecarlo.tar");
			cmd.append(projectName + "_report.tar.gz");
			cmd.append(projectName + "_result.tar.gz");
			compress_proc->setArguments(cmd);
			c_step=6;
			pd->setProgress(c_step);
			if(!compress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to create Monte Carlo combined tar archive.");
				c_flag = false;
				c_step=1000;
				endCompressProcess();
			}
			break;
		}
		case 6:	// insert data into database
		{
			pd->setLabelText("Inserting Monte Carlo descriptions into database...");
			db_connect();
			QSqlCursor cur("MonteCarloResult");
			cur.setMode( QSqlCursor::Insert);
			QSqlRecord *buffer = cur.primeInsert();
			montecarloID = get_newID("MonteCarloResult","montecarloID");
			buffer->setValue("montecarloID", montecarloID);
			buffer->setValue("InvestigatorID", investigatorID);
			if(runrequestID>0)
			{
				buffer->setValue("RunRequestID", runrequestID);
			}
			buffer->setValue("parameter_number", parameters);
			buffer->setValue("projectName", projectName);
			cur.insert();
			c_step = 7;
			pd->setProgress(c_step);
			
			pd->setLabelText("Uploading Monte Carlo data into database...");
			qApp->processEvents();
			QSqlCursor cur_f("MonteCarloData");
			cur_f.setMode(QSqlCursor::Insert);
			QSqlRecord *buffer_f = cur_f.primeInsert();
			buffer_f->setValue("montecarloID", montecarloID);
#ifdef WIN32
			write_blob(htmlDir + "\\" + projectName + ".res", buffer_f, "report");
			write_blob(parameterFile, buffer_f, "parameterFile");
			write_blob(USglobal->config_list.root_dir + "\\tmp\\montecarlo.tar", buffer_f, "data");
#else
			write_blob(htmlDir + "/" + projectName + ".res", buffer_f, "report");
			write_blob(parameterFile, buffer_f, "parameterFile");
			write_blob(USglobal->config_list.root_dir + "/tmp/montecarlo.tar", buffer_f, "data");
#endif
			int result = cur_f.insert();
			if(result <= 0)
			{
				QSqlError err = cur.lastError();
				QMessageBox::message(tr("Attention:"), 
                             tr("Saving Monte Carlo data to DB table 'MonteCarloData' failed.\n"
										 	"Error message from MySQL:\n\n")
										  + err.text());
				c_flag = false;
				c_step=1000;
				endCompressProcess();
			}
			c_step = 8;
			pd->setProgress(c_step);
			pd->setLabelText("Updating database records...");
			qApp->processEvents();
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
					str.sprintf("UPDATE tblResult SET InvestigatorID = %d, MontecarloID = %d WHERE ResultID = %d;", investigatorID, montecarloID, resultID);
					query.exec(str);
				}
				else		//create new one
				{
					resultID = get_newID("tblResult","ResultID");
					str = "INSERT INTO tblResult (ResultID, InvestigatorID, RunRequestID, MontecarloID) VALUES(";
					str += QString::number(resultID)+", "+ QString::number(investigatorID)+", " + QString::number(runrequestID) +", " + QString::number(montecarloID)+");";
					bool flag = query.exec(str);
					if(!flag)
					{
						QMessageBox::message(tr("Attention:"), 
												  tr("Saving result data to database failed.\n"));
						c_flag = false;
						c_step=1000;
						endCompressProcess();
					}
				}
			}								
			if (c_flag) // if everything worked we can clean up, otherwise leave the contents for debugging
			{
				//clean up temp directory	
				pd->setProgress(c_step);
				QDir temp_dir;
				QString str;
#ifdef WIN32
				str = USglobal->config_list.root_dir + "\\tmp";
#else
				str = USglobal->config_list.root_dir + "/tmp";
#endif
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
				pd->close();
				QMessageBox::message(tr("Please Note:"),  tr("The Monte Carlo Data were successfully\nuploaded to the database.\n"));	
			}
			break;
		}
		case 100:	// retrieve Monte Carlo data from database
		{
			bool flag = false;
			QString str, str1, num, filename;
			clearTmpDir();
			qApp->processEvents();
			pd->setLabelText("Downloading Monte Carlo Result Data...");
			pd->setProgress(1);
			qApp->processEvents();
			if(query_flag) // database contents have been queried successfully earlier
			{
				QSqlCursor cur_f("MonteCarloData");
				str.sprintf("montecarloID = %d", montecarloID);
				cur_f.select(str);
				c_step = 101;
				if(cur_f.next())
				{
#ifdef WIN32
					filename = make_tempFile(USglobal->config_list.root_dir + "\\tmp\\", "montecarlo.tar");
#else
					filename = make_tempFile(USglobal->config_list.root_dir + "/tmp/", "montecarlo.tar");
#endif
					flag = read_blob("data", cur_f, filename);
				}
				else
				{
					QMessageBox::message("UltraScan Error:", "Cannot find the selected Monte Carlo archive in the database.");
					c_flag = false;
					c_step=1000;
					endCompressProcess();
				}
				if(flag)
				{
					pd->setLabelText("Extracting Monte Carlo master archive into temporary directory...");
					pd->setProgress(2);
					compress_proc->clearArguments();
					QDir work_dir(USglobal->config_list.root_dir + "/tmp");
					compress_proc->setWorkingDirectory(work_dir);
					QStringList cmd;
					cmd.append("tar");
					cmd.append("-xvf");
					cmd.append("montecarlo.tar");
					compress_proc->setArguments(cmd);
					c_step = 102;
					if(!compress_proc->start())
					{
						QMessageBox::message("UltraScan Error:", "Unable to uncompress the Monte Carlo tar archive.");
						c_flag = false;
						c_step=1000;
						endCompressProcess();
					}
				}
				else
				{
					QMessageBox::message("UltraScan Error:", "Unable to retrieve Mote Carlo Data from database");
					c_flag = false;
					c_step=1000;
					endCompressProcess();
				}
			}
			else
			{
				QMessageBox::message("Please note:", "Please select a Monte Carlo Analysis before retrieving.");
				c_flag = false;
				c_step=1000;
				endCompressProcess();
			}
			break;
		}
		case 102:	// uncompress results tar file
		{
			pd->setLabelText("Uncompressing Monte Carlo result tar file...");
			pd->setProgress(3);
			qApp->processEvents();
			work_dir.setPath(USglobal->config_list.root_dir + "/tmp");
			tarfile = projectName + "_result.tar.gz";
			compress_proc->clearArguments();
         compress_proc->setWorkingDirectory(work_dir);
			QStringList cmd;
			cmd.append("gzip");
			cmd.append("-d");
			cmd.append(tarfile);
			compress_proc->setArguments(cmd);
			c_step = 103;
			if(!compress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to uncompress Monte Carlo result tar archive.");
				c_flag = false;
				c_step=1000;
				endCompressProcess();
			}
			break;
		}
		case 103:	// uncompress report tar file
		{
			pd->setLabelText("Uncompressing Monte Carlo report tar file...");
			pd->setProgress(4);
			work_dir.setPath(USglobal->config_list.root_dir + "/tmp");
			tarfile = projectName + "_report.tar.gz";
			compress_proc->clearArguments();
         compress_proc->setWorkingDirectory(work_dir);
			QStringList cmd;
			cmd.append("gzip");
			cmd.append("-d");
			cmd.append(tarfile);
			compress_proc->setArguments(cmd);
			c_step = 104;
			if(!compress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to uncompress Monte Carlo report tar archive.");
				c_flag = false;
				c_step=1000;
				endCompressProcess();
			}
			break;
		}
		case 104:	// extract results
		{
			pd->setLabelText("Extracting Monte Carlo result archive...");
			pd->setProgress(5);
			work_dir.setPath(USglobal->config_list.result_dir);
#ifdef WIN32
			tarfile = USglobal->config_list.root_dir + "\\tmp\\" + projectName + "_result.tar";
#else
			tarfile = USglobal->config_list.root_dir + "/tmp/" + projectName + "_result.tar";
#endif
			compress_proc->clearArguments();
         compress_proc->setWorkingDirectory(work_dir);
			QStringList cmd;
			cmd.append("tar");
			cmd.append("-xvf");
			cmd.append(tarfile);
			compress_proc->setArguments(cmd);
			c_step = 105;
			if(!compress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to uncompress Monte Carlo report tar archive.");
				c_flag = false;
				c_step=1000;
				endCompressProcess();
			}
			break;
		}
		case 105:	// extract results
		{
			pd->setLabelText("Extracting Monte Carlo report archive...");
			pd->setProgress(6);
			work_dir.setPath(USglobal->config_list.html_dir);
#ifdef WIN32
			tarfile = USglobal->config_list.root_dir + "\\tmp\\" + projectName + "_report.tar";
#else
			tarfile = USglobal->config_list.root_dir + "/tmp/" + projectName + "_report.tar";
#endif
			compress_proc->clearArguments();
         compress_proc->setWorkingDirectory(work_dir);
			QStringList cmd;
			cmd.append("tar");
			cmd.append("-xvf");
			cmd.append(tarfile);
			compress_proc->setArguments(cmd);
			c_step = 1000;
			if(!compress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to uncompress Monte Carlo report tar archive.");
				c_flag = false;
				endCompressProcess();
			}
			QMessageBox::message(tr("Please Note:"),  tr("The Monte Carlo Data were successfully\nretrieved from the database\nand can now be displayed.\n"));	
			break;
		}
		case 1000:	// Delete process
		{
			clearTmpDir();
cout << "Cleaning up in case 1000...\n";
			delete compress_proc;
			delete pd;
			compress_proc = NULL;
			pd = NULL;
		}
	}
}

void US_DB_RST_Montecarlo::load_DB()
{
	QString str, *display_Str;
	clear();

	int maxID = get_newID("MonteCarloResult","montecarloID");
	int count = 0;
	item_projectID = new int[maxID];
	item_projectName = new QString[maxID];
	display_Str = new QString[maxID];

	if(investigatorID>0)
	{
		 str.sprintf("SELECT montecarloID, projectName FROM MonteCarloResult WHERE InvestigatorID = %d;",investigatorID);
	}
	else
	{
		str="SELECT montecarloID, projectName FROM MonteCarloResult;";
	}	 
	QSqlQuery query(str);
	if(query.isActive())
	{
		while(query.next() )
		{
			item_projectID[count] = query.value(0).toInt();
			item_projectName[count] = query.value(1).toString();
			display_Str[count] = "("+  QString::number( item_projectID[count] ) + "): "+item_projectName[count];							 		
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
		query_flag = true;
	}
	else
	{
		lb_result->clear();
		lb_result->insertItem("No data were found in the DB");
	}
}


void US_DB_RST_Montecarlo::select_result(int item)
{
	QString str, str1,num, filename;
	if(query_flag)
	{
		montecarloID = item_projectID[item];
		projectName = item_projectName[item];
		str = "Selected : ("+ QString::number(montecarloID) +") "+ projectName;
		lbl_item->setText(str);
#ifdef WIN32
		dirName = USglobal->config_list.root_dir + "\\tmp\\";
#else
		dirName = USglobal->config_list.root_dir + "/tmp/";
#endif
		QSqlCursor cur("MonteCarloResult");
		str.sprintf("montecarloID = %d", montecarloID);
		cur.select(str);
		if(cur.next())
		{
			investigatorID = cur.value("InvestigatorID").toInt();
			runrequestID = cur.value("RunRequestID").toInt();	 
		}
	
		if(investigatorID > 0)
		{
			lbl_investigator->setText(show_investigator(investigatorID));
			pb_investigator->setEnabled(false);
		}
		if(runrequestID > 0)
		{
			lbl_runrequest->setNum(runrequestID);
			pb_runrequest->setEnabled(false);
		}
		from_DB = true;
		pb_retrieve->setEnabled(true);
	}
	else
	{
		QMessageBox::message("UltraScan Error:", "Unable to find Monte Carlo data in database.");
	}
}

void US_DB_RST_Montecarlo::retrieve()
{
	if(montecarloID <= 0 || projectName == "")
	{
		QMessageBox::message(tr("Attention:"), 
		tr("You have to select a Monte Carlo analysis\nfrom the database before retrieving it."));
		return;
	}
	if (pd == NULL)
	{
		pd = new QProgressDialog( "Please wait while your data is saved to the database...", 0, 8, this,"pd", TRUE );
		pd->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
		pd->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));	
	}
	pd->setProgress(0);
	pd->setTotalSteps(6);
	pd->setMinimumDuration(0);
	pd->show();
	pd->setLabelText("Clearing temporary directory...");
	pd->setProgress(0);
	qApp->processEvents();
	if(compress_proc != NULL)
	{
		delete compress_proc;
	}
	compress_proc = new QProcess(this);
	c_flag = true;
	c_step = 100;   
	connect(compress_proc, SIGNAL(processExited()), this, SLOT(endCompressProcess()));
	connect(compress_proc, SIGNAL(readyReadStdout()), this, SLOT(showOutput()));
	connect(compress_proc, SIGNAL(readyReadStderr()), this, SLOT(showOutput()));
	endCompressProcess(); // enter with c_step=100
}

void US_DB_RST_Montecarlo::display()
{
	US_Report_MonteCarlo *report_montecarlo;
	report_montecarlo = new US_Report_MonteCarlo(projectName);
}
/*!
Open US_DB_Admin to check delete permission.
*/

void US_DB_RST_Montecarlo::check_permission()
{
	US_DB_Admin *db_admin;
	db_admin = new US_DB_Admin("");
	db_admin->show();
	connect(db_admin, SIGNAL(issue_permission(bool)), SLOT(delete_db(bool)));
}

/*!
Delete one selected experimental result table entry.
*/

void US_DB_RST_Montecarlo::delete_db(bool permission)
{
	QString str;
	if(!permission)
	{
		QMessageBox::message(tr("Attention:"), 
								  tr("Permission denied, please contact your system administrator for help"));
		return;
	}
	if(!from_DB)
	{
		QMessageBox::message(tr("Attention:"), 
								  tr("Please first select the record\n"
									 "to be deleted from the database"));
	}
	else
	{
		switch(QMessageBox::information(this, tr("Delete this record?"), 
										tr("Clicking 'OK' will delete the selected result from the database"),
										tr("OK"), tr("CANCEL"),	0,1))
		{
			case 0:
			{
				QSqlQuery del_res;
				str.sprintf("UPDATE tblResult SET MontecarloID = NULL WHERE MontecarloID = %d;", montecarloID);
				del_res.exec(str);
				str.sprintf("DELETE FROM MonteCarloResult WHERE montecarloID = %d;", montecarloID);
				bool flag = del_res.exec(str);			
				//delete all relative data
				if(flag)
				{
					QSqlQuery del_dat;
					str.sprintf("DELETE FROM MonteCarloData WHERE montecarloID = %d;", montecarloID);
					del_dat.exec(str);	
				}
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

void US_DB_RST_Montecarlo::clear()
{
	from_HD = false;
	from_DB = false;
	query_flag = false;
	parameters = 0;
	montecarloID = 0;
	runrequestID = 0;
	pb_investigator->setEnabled(true);
	pb_runrequest->setEnabled(true);		
	lbl_runrequest->setText(" Not Selected");

	lb_result->clear();
  }

void US_DB_RST_Montecarlo::reset()
{
	clear();
	investigatorID = 0;
	lbl_investigator->setText(" Not Selected");
}


/*! Open a netscape browser to load help page.*/
void US_DB_RST_Montecarlo::help()
{
	US_Help *online_help; 
	online_help = new US_Help(this);
	online_help->show_help("manual/us_db_result_montecarlo.html");

}

/*! Close the interface.*/
void US_DB_RST_Montecarlo::quit()
{
	QString dirName = projectName+".mc_db";
	remove_temp_dir(dirName);
	close();
}
/*!
This event handler, for event <var>e</var>, to receive widget close events.
*/
void US_DB_RST_Montecarlo::closeEvent(QCloseEvent *e)
{
	QString dirName = projectName+".mc_db";
	remove_temp_dir(dirName);
	e->accept();
}

