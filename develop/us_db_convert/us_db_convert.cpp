#include "us_db_convert.h"

//! Constructor
/*! 
	Constractor a new <var>US_Convert_DB</var> interface, 
	with <var>parent</var> as a parent and <var>us_tblexpdata</var> as object name.
*/ 

US_Convert_DB::US_Convert_DB(QWidget *parent, const char *name) :US_DB(parent, name)
{
	int border=4, spacing=2;
	int xpos = border, ypos = border;
	int buttonw = 150, buttonh = 26, labelw = 150;

	cell_flag = false;		// use for cell table connect 
	query_flag = false;		// use for query listbox select
	from_query = false;		// use for Cell ID display DB data
	all_done = true;			// use for closeEvent to check the DB save, but only when called from us_expdata_db.
	cell_table_unfinished = 0;

	for(int i=0; i<8; i++)
	{
		exp_info.Cell[i] = "";
		exp_info.CellID[i] = 0;
	}
	
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	
	data_path = USglobal->config_list.data_dir;
	


	xpos += buttonw + spacing;
	pb_close = new QPushButton(tr("Close"), this);
	pb_close->setAutoDefault(false);
	pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_close->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_close->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_close, SIGNAL(clicked()), SLOT(quit()));
	
	
	xpos += buttonw + spacing;
	pb_query = new QPushButton(tr("Query DB Entry"), this);
	pb_query->setAutoDefault(false);
	pb_query->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_query->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_query->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_query, SIGNAL(clicked()), SLOT(convert_db()));
//	connect(pb_query, SIGNAL(clicked()), SLOT(retrieve_db()));
	
	xpos += labelw + spacing;
	lb_query = new QListBox(this, "Query");
	lb_query->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	lb_query->setGeometry(xpos, ypos, buttonw*2+spacing, buttonh*3+spacing*2);
	connect(lb_query, SIGNAL(selected(int)), SLOT(sel_query(int)));
	
	setup_GUI();
}

//! Destructor
/*! destroy the <var>US_Convert_DB</var>. */
US_Convert_DB::~US_Convert_DB()
{
}

void US_Convert_DB::setup_GUI()
{
	QGridLayout * Grid = new QGridLayout(this,5,2,4,2);
	Grid->addWidget(pb_close,0,0);
	Grid->addWidget(pb_query,0,1);
	Grid->addMultiCellWidget(lb_query,1,5,0,1);
	
	Grid->activate();
}
	

/*!
	Save the experimental data into DB table: <tt>tblExpData</tt>.
*/
void US_Convert_DB::add_db()
{
 
//	int result_flag = create(exp_info.Path, wholeFile, name_filter);	
	
	compFile.append(".gz");
	QFile f(compFile);
	QByteArray myFile;
	if(f.exists())
	{
		f.open(IO_ReadOnly);
		unsigned int SIZE = f.size();
//cout<<"FILE SIZE = "<<SIZE<<endl;
		if(SIZE>24000000)
		{
			QMessageBox::message(tr("Attention:"), 
									tr("The file's size is too large to store\n"));
			return;
		}
		//QByteArray myFile(f.size());
		//int adf = f.readBlock(myFile.data(), f.size());	
		//buffer->setValue("File", myFile);
		myFile=f.readAll();
		f.close();
	}
	f.remove();
	
	bool flg1, flg2;
	QString str, STR;
	QSqlQuery target1, target2;
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
	
	STR = "INSERT INTO tblExpDataFile (ExpdataID, File) VALUES (";
	STR += QString::number(exp_info.ExpdataID) + ", :file);";
	target2.prepare(STR);
	target2.bindValue(":file", QVariant(myFile));
	flg2 = target2.exec();
	if(!flg2)
	{
		QMessageBox::message(tr("Warning:"), 
									tr("Data can NOT be saved, problem on saving tblExpdataFile. "));
		return;
	}
//	int cnt = cur.insert();
	pb_add->setEnabled(false);
	lb_query->clear();
	str.sprintf("ExpData (ID:%d) has been saved to the database. ", exp_info.ExpdataID);
	lb_query->insertItem(str);
	lb_query->insertItem("************************************************");
	lb_query->insertItem("If you want to change any selected item,");
	lb_query->insertItem("you will have to delete the existing entry first,");
	lb_query->insertItem("or you can create a new database  Entry");
	all_done=true;
}

/*!
	List experimental data that stored in DB table: <tt>tblExpData</tt>.
*/
void US_Convert_DB::convert_db()
{
	QString str;
		
	int maxID = get_newID("tblExpData","ExpdataID");
	int count = 0;
	item_ExpdataID = new int[maxID];
	display_Str = new QString[maxID];
	
	QDir temp_dir(USglobal->config_list.data_dir+"/temp");
	if(temp_dir.exists())
	{
		QMessageBox::message(tr("Ultrascan Warning:"),
                           "The directory:"+USglobal->config_list.data_dir+"/temp already existed \n Please remove this directory first for ensuring this model working." );
		return;
	}
	if(!temp_dir.mkdir(USglobal->config_list.data_dir+"/temp", true))
	{
		
		QMessageBox::message(tr("Ultrascan Warning:"), 
									tr("Can Not make a directory for temporary usage"));

	}

	
	QString log_file=data_path +"/temp/convert.log";
	QFile log(log_file);
	if(!log.open(IO_WriteOnly))
	{
		QMessageBox::message("warning:", "Can NOT open log file to write");
		return;
	}
	QTextStream logstr(&log);
	
	QSqlCursor cur( "tblExpData" );
   cur.select();
	while(cur.next() )
	{
		int id = cur.value("ExpdataID").toInt();
		exp_info.Runid = cur.value("Runid").toString();
	//	if(id>2) break;
		display_Str[count] = "ExpDataID ("+  QString::number( id ) + "): "+exp_info.Runid;		
		item_ExpdataID[count] = id;
		count++;
		
		QSqlCursor cur_f( "tblExpDataFile" );
		str.sprintf("ExpdataID = %d",id);
		cur_f.select(str);
		if(cur_f.next())
		{
			int size = cur_f.value("File").toByteArray().size();
			QByteArray da(size);
			da = cur_f.value("File").toByteArray();
			logstr<<"Read "<<QString::number(size)<<" bytes data from DB for ExpDataID: "<<QString::number(id)<<"\n";
			QDir uncompress_dir;
			QString temp_dir = data_path +"/temp/" + exp_info.Runid;
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
			uncompress_dir.mkdir(temp_dir);
			QString tempfile =temp_dir +"/"+exp_info.Runid+".zlib.gz";			
			QFile fw(tempfile);
			fw.open(IO_WriteOnly);
			fw.writeBlock(da.data(),size);
			fw.close();
			Steps = 0;
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
			sleep(1);
			str="cd "+data_path +"/temp/";
			str +="; tar -cvvf "+exp_info.Runid+".tar " + exp_info.Runid;
			str+="; gzip -f9 "+exp_info.Runid+".tar ";
			str+="; rm -r "+exp_info.Runid;
			system(str);
			
			QFile f(data_path+"/temp/"+exp_info.Runid+".tar.gz");
			QByteArray myFile;
			if(f.exists())
			{
					f.open(IO_ReadOnly);
					unsigned int SIZE = f.size();
				//	cout<<"FILE SIZE = "<<SIZE<<endl;
					if(SIZE>24000000)
					{
						QMessageBox::message(tr("Attention:"), 
									tr("The file's size is too large to store\n"));
						return;
					}
					myFile=f.readAll();
					f.close();
					f.remove();
			}			
			QString STR = "INSERT INTO tblRawExpData (ExpdataID, RawData) VALUES (";
			STR += QString::number(id) + ", :file);";
			QSqlQuery target;
			target.prepare(STR);
			target.bindValue(":file", QVariant(myFile));
			bool flag = target.exec();
			if(!flag)
			{
					QMessageBox::message(tr("Warning:"), 
									tr("Data can NOT be saved, problem on saving tblRawExpdata. "));
					return;
			}
		}
		else
		{
			logstr<<"Can Not find Raw Data in tblExpDataFile for ExpDataID: "<<QString::number(id)<<"\n";
		}						
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

void US_Convert_DB::retrieve_db()
{
	QString str;
	
	int maxID = get_newID("tblExpData","ExpdataID");
	int count = 0;
	item_ExpdataID = new int[maxID];
	item_Runid = new QString[maxID];
	display_Str = new QString[maxID];
	
	
	QSqlCursor cur( "tblExpData" );
   QSqlIndex order = cur.index( "Runid" );
   cur.select(order);

	while(cur.next() )
	{
		int id = cur.value("ExpdataID").toInt();
		display_Str[count] = "ExpDataID ("+  QString::number( id ) + "): "+
							 		cur.value( "Runid" ).toString();
		item_ExpdataID[count] = id;
		item_Runid[count] = cur.value("Runid").toString();
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

void US_Convert_DB::sel_query(int item)
{	
	QString str;
	if(query_flag)
	{
		exp_info.ExpdataID = item_ExpdataID[item];
		exp_info.Runid = item_Runid[item];
	
		QSqlCursor cur_f( "tblRawExpData" );
		str.sprintf("ExpdataID = %d",exp_info.ExpdataID);
		cur_f.select(str);
		if(cur_f.next())
		{
		    QString filename = make_tempFile(USglobal->config_list.data_dir+"/temp/", exp_info.Runid+".tar.gz");
         bool raw_flag=read_blob("Rawdata", cur_f, filename);
         if(!raw_flag)
         {
				QMessageBox::message("UltraScan Error:", "Unable to retrieve Raw data files.");
			}

			uncompress_proc = new QProcess(this);
			uc_step=0;   
			connect(uncompress_proc, SIGNAL(processExited()), this, SLOT(endUncompressProcess()));
			endUncompressProcess();
			disconnect(uncompress_proc);													
		}
	}

}

void US_Convert_DB::endUncompressProcess()
{
	QString tar_filename;
	switch(uc_step)
	{
		case 0:			//unzip raw data file
		{

			uncompress_proc->clearArguments();
			tar_filename = exp_info.Runid+".tar.gz";
			QDir work_dir(USglobal->config_list.data_dir+"/temp/");
			uncompress_proc->setWorkingDirectory(work_dir);
			QStringList cmd;
			cmd.append("gzip");
			cmd.append("-d");
			cmd.append(tar_filename);
			uncompress_proc->setArguments(cmd);
			uc_step=1;
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
			QDir work_dir(USglobal->config_list.data_dir+"/temp/");
			uncompress_proc->setWorkingDirectory(work_dir);

			QStringList cmd;
			cmd.append("tar");
			cmd.append("-xvf");
			cmd.append(tar_filename);
			uncompress_proc->setArguments(cmd);
			uc_step=2;
			if(!uncompress_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to un tar Raw data files.");
				return;
			}
		}
		case 2:
		{
		//	cleanCompressFile();	
			delete uncompress_proc;
		}	
	
	}
}



/*! Close the interface.*/
void US_Convert_DB::quit()
{
	close();
}

/*!
	Use <tt>ZLIB</tt> functions to compress a regular text file.
	\param path the location of file.
	\param wholeFile the directory of a bunch of files need to be compressed.
	\param filter the condition for selecting files to compress.
*/
int US_Convert_DB::create(QString path, QString wholeFile, QString filter)
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
int US_Convert_DB::retrieve(QString compFile, int Steps)
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
	Retrieve the selected experimental data from database. 
	used by US_DB_RtvDate(), US_DB_RtvDescription(), US_DB_RtvEditType(), US_DB_RtvInvestigator().
	\return true retrieve sucessfully.
	\return false otherwise.
*/
bool US_Convert_DB::retrieve_all(int ExpdataID, QString Display)
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
			QString str;
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
	//				write_dbinfo(temp_dir);
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
		}
		from_query = true;
		query_flag = false;
		lb_query->insertItem(Display);
		pb_add->setEnabled(false);
		pb_query->setEnabled(false);
		pb_reset->setEnabled(false);
		return (true);
}
