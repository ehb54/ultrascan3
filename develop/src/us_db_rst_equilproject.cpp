#include "../include/us_db_rst_equilproject.h"

   US_DB_RST_EquilProject::US_DB_RST_EquilProject(QWidget *p, const char *name) : US_DB(p, name)
   {
      int minHeight1 = 26;
   
      setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   
      from_HD = false;
      from_DB = false;
      query_flag = false;
      projectID =0;
      investigatorID = 0;
		DataID = 0;
      runrequestID = 0;
  	
      pb_hd = new QPushButton(tr("Load Result from HD"),this);
		pb_hd->setAutoDefault(false);
      pb_hd->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
      pb_hd->setMinimumHeight(minHeight1);
      pb_hd->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      connect(pb_hd, SIGNAL(clicked()), SLOT(load_HD()));
      
      pb_load_db = new QPushButton("Query Result from DB", this);
      pb_load_db->setAutoDefault(false);
      pb_load_db->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      pb_load_db->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
      pb_load_db->setMinimumHeight(minHeight1);
      connect(pb_load_db, SIGNAL(clicked()), SLOT(load_DB()));
   
		pb_retrieve_db = new QPushButton(tr("Retrieve Result from DB"), this);
		pb_retrieve_db->setAutoDefault(false);
		pb_retrieve_db->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
		pb_retrieve_db->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
		pb_retrieve_db->setMinimumHeight(minHeight1);
		connect(pb_retrieve_db, SIGNAL(clicked()), SLOT(retrieve_db()));
   
      pb_investigator = new QPushButton(tr("Investigator ID:"), this);
      pb_investigator->setAutoDefault(false);
      pb_investigator->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      pb_investigator->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
      pb_investigator->setMinimumHeight(minHeight1);      
		connect(pb_investigator, SIGNAL(clicked()), SLOT(sel_investigator()));
   
     
      lbl_investigator= new QLabel("",this);
      lbl_investigator->setAlignment(AlignLeft|AlignVCenter);
      lbl_investigator->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);	
      lbl_investigator->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
      lbl_investigator->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
      lbl_investigator->setMinimumHeight(minHeight1);
      lbl_investigator->setText(tr(" Not Selected"));
      
      pb_runrequest = new QPushButton(tr("Select Run Request"), this);
      pb_runrequest->setAutoDefault(false);
      pb_runrequest->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      pb_runrequest->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
      pb_runrequest->setMinimumHeight(minHeight1);
      connect(pb_runrequest, SIGNAL(clicked()), SLOT(sel_runrequest()));
      
      lbl_runrequest= new QLabel("",this);
      lbl_runrequest->setAlignment(AlignLeft|AlignVCenter);
      lbl_runrequest->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);	
      lbl_runrequest->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
      lbl_runrequest->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
      lbl_runrequest->setMinimumHeight(minHeight1);
      lbl_runrequest->setText(tr(" Not Selected"));
      
		lbl_item= new QLabel("Selected Result:",this);
      lbl_item->setAlignment(AlignLeft|AlignVCenter);	
      lbl_item->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
      lbl_item->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
      lbl_item->setMinimumHeight(minHeight1);
    
		le_item= new QLineEdit(this);
      le_item->setAlignment(AlignLeft|AlignVCenter);
      le_item->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
      le_item->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
      le_item->setMinimumHeight(minHeight1);
      le_item->setText(tr(" Not Selected"));

		
    
		lbl_instr = new QLabel(tr("   Doubleclick on result data to select:   "),this);
      lbl_instr->setAlignment(AlignCenter|AlignVCenter);
      lbl_instr->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
      lbl_instr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2, QFont::Bold));
      lbl_instr->setMinimumHeight(minHeight1);
		    
      lb_result = new QListBox(this, "result");
      lb_result->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      lb_result->setMinimumHeight(minHeight1*4);
      lb_result->setSelected(0, true);
      connect(lb_result, SIGNAL(selected(int)), SLOT(select_result(int)));
   
      pb_save_db = new QPushButton(tr("Save DB"), this);
      pb_save_db->setAutoDefault(false);
      pb_save_db->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      pb_save_db->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
      pb_save_db->setMinimumHeight(minHeight1);
      pb_save_db->setEnabled(true);
      connect(pb_save_db, SIGNAL(clicked()), SLOT(save_db()));
      
      pb_display = new QPushButton(tr("Show Selected Result"), this);
      pb_display->setAutoDefault(false);
      pb_display->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      pb_display->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
      pb_display->setMinimumHeight(minHeight1);
      pb_display->setEnabled(true);
      connect(pb_display, SIGNAL(clicked()), SLOT(display()));
      
      pb_del_db = new QPushButton(tr("Delete DB"), this);
      pb_del_db->setAutoDefault(false);
      pb_del_db->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      pb_del_db->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
      pb_del_db->setMinimumHeight(minHeight1);
      pb_del_db->setEnabled(true);
      connect(pb_del_db, SIGNAL(clicked()), SLOT(check_permission()));

      pb_reset = new QPushButton(tr("Reset"), this);
      pb_reset->setAutoDefault(false);
      pb_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      pb_reset->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
      pb_reset->setMinimumHeight(minHeight1);
      pb_reset->setEnabled(true);
      connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));
     
      pb_help = new QPushButton(tr("Help"), this);
      pb_help->setAutoDefault(false);
      pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      pb_help->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
      pb_help->setMinimumHeight(minHeight1);
      pb_help->setEnabled(true);
      connect(pb_help, SIGNAL(clicked()), SLOT(help()));
   
      pb_close = new QPushButton(tr("Close"), this);
      pb_close->setAutoDefault(false);
      pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      pb_close->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
      pb_close->setMinimumHeight(minHeight1);
      pb_close->setEnabled(true);
      connect(pb_close, SIGNAL(clicked()), SLOT(quit()));
   
      setup_GUI();
   }

   US_DB_RST_EquilProject::~US_DB_RST_EquilProject()
   {
		delete compress_proc;
   }

void US_DB_RST_EquilProject::setup_GUI()
{
	int j=0;
	int rows = 8, columns = 1, spacing = 2;
	
	QGridLayout * background = new QGridLayout(this,2,1,spacing);	
	
	QGridLayout * subGrid1 = new QGridLayout(rows, columns, spacing);
	subGrid1->addWidget(pb_hd,j,0);
	j++;
	subGrid1->addWidget(pb_load_db,j,0);
	j++;
	subGrid1->addWidget(pb_retrieve_db,j,0);
	j++;
	subGrid1->addWidget(lbl_instr,j,0);
	j++;
	subGrid1->addMultiCellWidget(lb_result,j,j+4,0,0);
	subGrid1->setRowStretch(j,4);
	
	rows = 9, columns = 2, spacing = 2, j=0;
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
	subGrid2->addWidget(lbl_investigator,j,1);
	j++;
	subGrid2->addWidget(lbl_item,j,0);
	subGrid2->addWidget(le_item,j,1);
	j++;
	subGrid2->addMultiCellWidget(pb_display,j,j,0,1);
	j++;
	subGrid2->addMultiCellWidget(pb_reset,j,j,0,1);
	j++;
	subGrid2->addMultiCellWidget(pb_help,j,j,0,1);
	j++;
	subGrid2->addMultiCellWidget(pb_close,j,j,0,1);
	j++;
	subGrid2->setRowStretch(j,3);
	
	background->addLayout(subGrid1,0,0);
	background->setColSpacing(0,450);
	background->addLayout(subGrid2,0,1);
	background->setColStretch(0,3);
	
	background->activate();
}
/*!
	Open US_DB_TblInvestigator interface for selecting investigator.
*/
   void US_DB_RST_EquilProject::sel_investigator()
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
   void US_DB_RST_EquilProject::update_investigator_lbl (QString Display, int InvID)
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

   void US_DB_RST_EquilProject::sel_runrequest()
   {
      US_DB_RunRequest *runrequest_dlg;
      runrequest_dlg = new US_DB_RunRequest();
      runrequest_dlg->setCaption("Run Request Information");
      runrequest_dlg->show();
      connect(runrequest_dlg, SIGNAL(issue_RRID(int)), SLOT(update_runrequest_lbl(int)));
   }

   void US_DB_RST_EquilProject::update_runrequest_lbl (int RRID)
   {
      runrequestID = RRID;
      lbl_runrequest->setNum(runrequestID);
      if(runrequestID == 0)
      {
         lbl_runrequest->setText(" Not Selected");
      }
   }

   void US_DB_RST_EquilProject::load_HD()
   {
		QString trashcan,InvID;
      reset();
      fileName = QFileDialog::getOpenFileName(USglobal->config_list.result_dir, "*.eq-project", 0);
   
      QFile projectFile(fileName);
      projectFile.open(IO_ReadOnly);
      QTextStream project_ts(&projectFile);
      projectName = project_ts.readLine();
		trashcan = project_ts.readLine();
		trashcan = project_ts.readLine();
		trashcan = project_ts.readLine();
		trashcan = project_ts.readLine();
		InvID = project_ts.readLine();
      projectFile.close();
      htmlDir = USglobal->config_list.html_dir + "/" + projectName;
      baseName = USglobal->config_list.result_dir + "/" + projectName;
   
 		if (fileName.isEmpty())
      {
         QMessageBox::message(tr("Attention:"), 
                             tr("No file Selected.\n"));
         return;
      }
      else
      {	

			lb_result->insertItem(tr("Data is loaded from Hard Drive"));
   		le_item->setText(projectName);

         US_Report_EquilProject *report_equilproject;
         report_equilproject = new US_Report_EquilProject(false);
         report_equilproject->generate(fileName);

			if(InvID.toInt()>0)
			{
				lbl_investigator->setText(show_investigator(InvID.toInt()));
				pb_investigator->setEnabled(false);
				investigatorID = InvID.toInt();
			}

         from_HD = true;
      }
   
   }

   void US_DB_RST_EquilProject::save_db()
   {
      QString num,str, str1, temp_file;
      if(!from_HD)
      {
         QMessageBox::message(tr("Attention:"), 
                             tr("Please load result data from harddrive first.\n"));
         return;
      }
      if(investigatorID<=0)
      {
         QMessageBox::message(tr("Attention:"), 
                             tr("Please select investigator first.\n"));
         return;
      
      }
//Insert into EquilProjectData

		if(!insertCompressData())
		{			
			QMessageBox::message(tr("Attention:"), tr("Insert compress equilproject data failed.\n"));
         exit(0);
		}
   }

bool US_DB_RST_EquilProject::insertCompressData()
{
 	pd = new QProgressDialog( "Please wait while your data is saved to the database...", 0, 6, this,"pd", TRUE );
	pd->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
//	pd->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));	
	pd->setProgress(0);
	pd->setMinimumDuration(0);
	
	compress_proc = new QProcess(this);
	c_step=0;
 	connect(compress_proc, SIGNAL(processExited()), this, SLOT(endCompressProcess()));
	endCompressProcess();
	disconnect(compress_proc);
	return true;
}

void US_DB_RST_EquilProject::endCompressProcess()
{	
	QDir work_dir;
	QString tarfile, tardir;
               
	switch(c_step)
	{
		case 0:			//Compress report tar file	
		{
			work_dir.setPath(USglobal->config_list.html_dir);
			tarfile = projectName + "_report.tar";
			tardir = projectName + "/";
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
				QMessageBox::message("UltraScan Error:", "Unable to start process to tar equil-project report archive.");
				return;
			}
			break;			
		}
		case 1:			//gzip report tar file	
		{
			work_dir.setPath(USglobal->config_list.html_dir);
			tarfile = projectName + "_report.tar";
			compress_proc->clearArguments();
         compress_proc->setWorkingDirectory(work_dir);
			QStringList cmd;
			cmd.append("gzip");
			cmd.append("-9");
			cmd.append(tarfile);
			compress_proc->setArguments(cmd);
			c_step=2;
			pd->setProgress(c_step);
			if(!compress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to gzip equil-project report data archive.");
				return;
			}
			break;

		}


		case 2:			//Compress result tar file to DB
		{
			
			work_dir.setPath(USglobal->config_list.result_dir);
			tarfile = projectName + "_result.tar";						
			compress_proc->clearArguments();
         compress_proc->setWorkingDirectory(work_dir);
			QStringList cmd;
			cmd.append("tar");
			cmd.append("-cvf");
			cmd.append(tarfile);
#ifdef WIN32
			QString filter=projectName + "\*";
			cmd.append(filter);
#endif
#ifdef UNIX
			QString filter=projectName + "*";
			work_dir.setNameFilter(filter);	
			QStringList entries = work_dir.entryList();
			for (QStringList::Iterator it = entries.begin(); it  != entries.end(); ++it ) 
			{
					
					cmd.append((*it).latin1());
			}

#endif

			compress_proc->setArguments(cmd);
			c_step=3;
			pd->setProgress(c_step);
			if(!compress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to tar equil-project result archive.");
				return;
			}
			break;
		}
		case 3:			//gzip result tar file
		{
			work_dir.setPath(USglobal->config_list.result_dir);
			tarfile = projectName + "_result.tar";
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
				QMessageBox::message("UltraScan Error:", "Unable to start process to gzip equil-project result archive.");
				return;
			}
			break;
		}

		
		case 4:			//Insert data to DB table EquilProjectData
		{
		
			QSqlCursor cur("EquilProjectData");
   		cur.setMode( QSqlCursor::Insert);
   		QSqlRecord *buffer = cur.primeInsert();
			DataID = get_newID("EquilProjectData","tableID");
   		buffer->setValue("tableID", DataID);
			fileName = USglobal->config_list.result_dir+"/"+ projectName+".eq-project";
			write_blob(fileName, buffer, "HeadFile");
			fileName = USglobal->config_list.html_dir +"/"+ projectName + "_report.tar.gz";
			write_blob(fileName, buffer, "Report_Tar");
			fileName = USglobal->config_list.result_dir +"/"+ projectName + "_result.tar.gz";
			write_blob(fileName, buffer, "Result_Tar");
			
			cur.insert();
			c_step=5;
			pd->setProgress(c_step);
			if(!compress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable insert data to DB table EquilProjectData.");
				return;
			}
			break;
		}
		case 5:		//Insert data to DB table EquilProjectResult
		{
			int id = get_newID("EquilProjectResult","EquilProjectID");
     		QString STR = "INSERT INTO EquilProjectResult (EquilProjectID, ProjectName, InvestigatorID, DataID";
      	if(runrequestID > 0)
      	{
         	STR += ", RunRequestID";
      	}
      	STR +=") VALUES(";
      	STR += QString::number(id);
      	STR += ", '"+ projectName+"', "+ QString::number(investigatorID)+", "+QString::number(DataID);
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
                             tr("Saving to DB table 'EquilProjectResult' failed.\n"
										  "Attempted to execute this command:\n\n"
										  + STR + "\n\n"
										  "Causing the following error:\n\n")
										  + sqlerr.text());
				//delete inserted	EquilProjectData data
				QString str;
				QSqlQuery del_data;
				str.sprintf("DELETE FROM EquilProjectData WHERE tableID = %d;", DataID);
				del_data.exec(str);
				// clean the temp compress file	
				cleanCompressFile();
				pd->close();
         	return;
      	}
      	if(runrequestID >0)		//update tblResult 
      	{
    			QString str;     	
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
            	str.sprintf("UPDATE tblResult SET InvestigatorID = %d, EquilProjectID = %d WHERE ResultID = %d;", investigatorID, id, resultID);
            	query.exec(str);
         	}
         	else		//create new one
         	{
           		resultID = get_newID("tblResult","ResultID");
           		str = "INSERT INTO tblResult (ResultID, InvestigatorID, RunRequestID, EquilProjectID) VALUES(";
            	str += QString::number(resultID)+", "+ QString::number(investigatorID)+", " + QString::number(runrequestID) +", " + QString::number(id)+");";
            	bool flag = query.exec(str);
            	if(!flag)
            	{
               	QMessageBox::message(tr("Attention:"), 
                                   tr("Problem occurs, save to tblResult failed.\n"));
               	return;
            	}
         	}     
			}
			pd->setProgress(6);	
			// clean the temp compress file	
			cleanCompressFile();
			pd->close();

			QMessageBox::message(tr("Congratulation:"),  tr("Successfully save data to database.\n"));							
			delete compress_proc;
		}	//end case 5
	}
}

void US_DB_RST_EquilProject::load_DB()
{
      QString str, *display_Str;
      clear();
   
      int maxID = get_newID("EquilProjectResult","EquilProjectID");
      int count = 0;
      item_projectID = new int[maxID];
      item_projectName = new QString[maxID];
      display_Str = new QString[maxID];
   
		if(investigatorID>0)
		{
   	   str.sprintf("SELECT EquilProjectID, ProjectName FROM EquilProjectResult WHERE InvestigatorID = %d;",investigatorID);
		}
		else
		{
			str="SELECT EquilProjectID, ProjectName FROM EquilProjectResult;";
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


void US_DB_RST_EquilProject::select_result(int item)
{ 
   QString str, num, filename;
   if(query_flag)
   {
      projectID = item_projectID[item];
      projectName = item_projectName[item];
      str = "("+ QString::number(projectID) +") "+ projectName;
      le_item->setText(str);
        
      str = "SELECT DataID, InvestigatorID, RunRequestID";
      str += " FROM EquilProjectResult WHERE EquilProjectID = ";
      str += QString::number(projectID);
      QSqlQuery query(str);
      if(query.isActive())
      {
         if(query.next() )
         {
               
				DataID = query.value(0).toInt();
            investigatorID = query.value(1).toInt();
				runrequestID = query.value(2).toInt();          
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
				else
				{
					  lbl_runrequest->setText(" N/A");
				}
     		}      
       }
       else
       {
            QMessageBox::message("UltraScan Error:", "Unable load data from DB table 'EquilProjectResult'.");
            exit(0);
       }	
       from_DB = true;		
    }
}

void US_DB_RST_EquilProject::retrieve_db()
{
	bool rep_flag = false;
	bool res_flag = false;
	QString str, str1;
	
	if(DataID>0)
	{
		QSqlCursor cur( "EquilProjectData" );
      str.sprintf("tableID = %d", DataID);
      cur.select(str);             
		if(cur.next())
      {
    	       str1 = projectName +"_report.tar.gz";         
			    QString filename = make_tempFile(USglobal->config_list.html_dir+"/", str1);
             rep_flag=read_blob("Report_Tar", cur, filename);
           	 if(!rep_flag)
             {
                	QMessageBox::message("UltraScan Error:", "Unable to retrieve Report files.");
             }

				str1 = projectName +"_result.tar.gz";
            filename = make_tempFile(USglobal->config_list.result_dir+"/", str1);   
            res_flag=read_blob("Result_Tar", cur, filename);
            if(!res_flag)
            {       
		        QMessageBox::message("UltraScan Error:", "Unable to retrieve Result files.");                     
				}
				uncompress_proc = new QProcess(this);
				uc_step=0;   
				connect(uncompress_proc, SIGNAL(processExited()), this, SLOT(endUncompressProcess()));
				endUncompressProcess();
				disconnect(uncompress_proc);
      }
	}
}

void US_DB_RST_EquilProject::endUncompressProcess()
{
	QString tar_filename;
	switch(uc_step)
	{
		case 0:			//unzip result file
		{
			uncompress_proc->clearArguments();
			tar_filename = projectName +"_result.tar.gz";
			QDir work_dir(USglobal->config_list.result_dir);
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
		case 1:			//un tar result file
		{
			uncompress_proc->clearArguments();
			tar_filename = projectName +"_result.tar";
			QDir work_dir(USglobal->config_list.result_dir);
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

	
		case 2:			//un zip report file	
		{
			uncompress_proc->clearArguments();
			tar_filename = projectName +"_report.tar.gz";
			QDir work_dir(USglobal->config_list.html_dir);
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
		case 3:			//un tar report file
		{
			uncompress_proc->clearArguments();
			tar_filename = projectName +"_report.tar";
			QDir work_dir(USglobal->config_list.html_dir);
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
		case 4:
		{	
			QMessageBox::message("Congratulations,", "Retrieve Data Successfully!");
			delete uncompress_proc;
		}
	}	
}


void US_DB_RST_EquilProject::display()
{
   if(from_HD||from_DB)
   {
      indexFile = USglobal->config_list.html_dir + "/" + projectName + "/index.html";
   }
	else
	{
		QMessageBox::message(tr("Attention:"), tr("Please first select the result\n"));									
		return;
	}
   
   US_Help *online_help; 
   online_help = new US_Help();
   online_help->show_html_file(indexFile);
}


/*!
	Open US_DB_Admin to check delete permission.
*/

   void US_DB_RST_EquilProject::check_permission()
   {
      US_DB_Admin *db_admin;
      db_admin = new US_DB_Admin("");
      db_admin->show();
      connect(db_admin, SIGNAL(issue_permission(bool)), SLOT(delete_db(bool)));
   }

/*!
	Delete one selected experimental result table entry.
*/

   void US_DB_RST_EquilProject::delete_db(bool permission)
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
                             tr("Please select the record to\n"
                               "be deleted from the database"));
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
                  str.sprintf("UPDATE tblResult SET EquilProjectID = NULL WHERE EquilProjectID = %d;", projectID);
                  del_res.exec(str);
                  str.sprintf("DELETE FROM EquilProjectResult WHERE EquilProjectID = %d;", projectID);
                  bool flag = del_res.exec(str);
               //delete all relative equilproject data
                  if(flag)
                  {
                      if(DataID>0)
                      {
                           QSqlQuery del_model;
                           str.sprintf("DELETE FROM EquilProjectData WHERE tableID = %d;", DataID);
                           del_model.exec(str);
                     }
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


void US_DB_RST_EquilProject::clear()
{
  		cleanCompressFile();    
		from_HD = false;
      from_DB = false;
      query_flag = false;
      projectID = 0;
      DataID = 0;
		runrequestID = 0;
      lb_result->clear();
      pb_investigator->setEnabled(true);
      pb_runrequest->setEnabled(true);
      lbl_runrequest->setText(" Not Selected");
		le_item->setText(" Not Selected");
}

 void US_DB_RST_EquilProject::reset()
 {
	clear();
	investigatorID=0;
	lbl_investigator->setText(" Not Selected");
 }

/*! Open a netscape browser to load help page.*/
   void US_DB_RST_EquilProject::help()
   {
      US_Help *online_help; 
      online_help = new US_Help(this);
      online_help->show_help("manual/us_db_result_equilproject.html");
   
   }

void US_DB_RST_EquilProject::cleanCompressFile()
{
	QFile tempFile1(USglobal->config_list.html_dir +"/"+ projectName + "_report.tar.gz");
	if(tempFile1.exists())
	{
		tempFile1.remove();
	}
	QFile tempFile2(USglobal->config_list.result_dir +"/"+ projectName + "_result.tar.gz");
	if(tempFile2.exists())
	{
		tempFile2.remove();
	}
	QFile tempFile3(USglobal->config_list.html_dir +"/"+ projectName + "_report.tar");
	if(tempFile3.exists())
	{
		tempFile3.remove();
	}
	QFile tempFile4(USglobal->config_list.result_dir +"/"+ projectName + "_result.tar");
	if(tempFile4.exists())
	{
		tempFile4.remove();
	}

}

/*! Close the interface.*/
void US_DB_RST_EquilProject::quit()
{
	cleanCompressFile();     
	close();
}
/*!
	This event handler, for event <var>e</var>, to receive widget close events.
*/
void US_DB_RST_EquilProject::closeEvent(QCloseEvent *e)
{
	cleanCompressFile();
	e->accept();
}

