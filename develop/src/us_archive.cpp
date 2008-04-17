#include "../include/us_archive.h"

US_Archive::US_Archive(QWidget *p , const char *name) : QFrame(p, name)
{
	USglobal = new US_Config();
	int xpos = 2, ypos = 2, spacing = 2, buttonh = 26, buttonw0 = 220;
	int buttonw = 109;
	int buttonw1 = 170;
	setCaption(tr("UltraScan Archive Manager"));
	filename = "";
	
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	QString str;

	reports_flag = true;
	ultrascan_flag = true;
	delete_flag = true;
	data_ctrl_flag = false;
	
	mle = new QTextEdit(this);
	mle->setFrameStyle(QFrame::WinPanel|Sunken);
	mle->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	mle->setReadOnly(true);
	mle->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	mle->show();

	banner1 = new QLabel(tr("Archive Information:"),this);
	banner1->setFrameStyle(QFrame::WinPanel|Raised);
	banner1->setAlignment(AlignCenter|AlignVCenter);
	banner1->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
	banner1->setGeometry(xpos, ypos, buttonw0, buttonh);
	banner1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	ypos += buttonh + spacing + 2;

	pb_view = new QPushButton(tr("View Archive Contents"), this);
	Q_CHECK_PTR(pb_view);
	pb_view->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_view->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_view->setGeometry(xpos, ypos, buttonw0, buttonh);
	pb_view->setAutoDefault(false);	
	connect(pb_view, SIGNAL(clicked()), SLOT(view()));
	
	ypos += buttonh + spacing;
	
	lbl_name1 = new QLabel(tr(" Archive:"),this);
	lbl_name1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_name1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
	lbl_name1->setGeometry(xpos, ypos, buttonw, buttonh);
	lbl_name1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));	
	lbl_name1->setAlignment(AlignLeft|AlignVCenter);

	xpos += spacing + buttonw;

	lbl_name2 = new QLabel("", this);
	lbl_name2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_name2->setGeometry(xpos, ypos, buttonw, buttonh);
	lbl_name2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	lbl_name2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_name2->setAlignment(AlignCenter|AlignVCenter);

	ypos += buttonh + spacing + 2;
	xpos = spacing;

	banner2 = new QLabel(tr("Archive Creation:"),this);
	banner2->setFrameStyle(QFrame::WinPanel|Raised);
	banner2->setAlignment(AlignCenter|AlignVCenter);
	banner2->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
	banner2->setGeometry(xpos, ypos, buttonw0, buttonh);
	banner2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	ypos += buttonh + spacing + 2;
/*	
	lbl_ultrascan = new QLabel(tr(" Include UltraScan Data:"),this);
	lbl_ultrascan->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_ultrascan->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
	lbl_ultrascan->setGeometry(xpos, ypos, buttonw1, buttonh);
	lbl_ultrascan->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));	
	lbl_ultrascan->setAlignment(AlignLeft|AlignVCenter);
*/
	xpos += spacing + buttonw1 + 18;

	cb_ultrascan = new QCheckBox(tr(" Include UltraScan Data:"),this);
	Q_CHECK_PTR(cb_ultrascan);
	cb_ultrascan->setGeometry(xpos, ypos+5, 14, 14);
	cb_ultrascan->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_ultrascan->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	if (ultrascan_flag)
	{
		cb_ultrascan->setChecked(true);
	}
	else
	{
		cb_ultrascan->setChecked(false);
	}
	connect(cb_ultrascan, SIGNAL(clicked()), SLOT(set_ultrascan()));

	ypos += buttonh + spacing;
	xpos = spacing;
/*
	lbl_reports = new QLabel(tr(" Include HTML Reports:"),this);
	lbl_reports->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_reports->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
	lbl_reports->setGeometry(xpos, ypos, buttonw1, buttonh);
	lbl_reports->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));	
	lbl_reports->setAlignment(AlignLeft|AlignVCenter);
*/
	xpos += spacing + buttonw1 + 18;

	cb_reports = new QCheckBox(tr(" Include HTML Reports:"),this);
	Q_CHECK_PTR(cb_reports);
	cb_reports->setGeometry(xpos, ypos+5, 14, 14);
	cb_reports->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_reports->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	if (reports_flag)
	{
		cb_reports->setChecked(true);
	}
	else
	{
		cb_reports->setChecked(false);
	}
	connect(cb_reports, SIGNAL(clicked()), SLOT(set_reports()));

	ypos += buttonh + spacing;
	xpos = spacing;
/*
	lbl_delete = new QLabel(tr(" Delete Original Data:"),this);
	lbl_delete->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_delete->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
	lbl_delete->setGeometry(xpos, ypos, buttonw1, buttonh);
	lbl_delete->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));	
	lbl_delete->setAlignment(AlignLeft|AlignVCenter);
*/
	xpos += spacing + buttonw1 + 18;

	cb_delete = new QCheckBox(tr(" Delete Original Data:"),this);
	Q_CHECK_PTR(cb_delete);
	cb_delete->setGeometry(xpos, ypos+5, 14, 14);
	cb_delete->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	cb_delete->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	if (delete_flag)
	{
		cb_delete->setChecked(true);
	}
	else
	{
		cb_delete->setChecked(false);
	}
	connect(cb_delete, SIGNAL(clicked()), SLOT(set_delete()));

	ypos += buttonh + spacing;
	xpos = spacing;
	
	pb_select_velocdata_create = new QPushButton(tr("Select Velocity Data"), this);
	Q_CHECK_PTR(pb_select_velocdata_create);
	pb_select_velocdata_create->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_select_velocdata_create->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_select_velocdata_create->setGeometry(xpos, ypos, buttonw0, buttonh);
	pb_select_velocdata_create->setAutoDefault(false);
	connect(pb_select_velocdata_create, SIGNAL(clicked()), SLOT(select_velocdata_create_archive()));

	ypos += buttonh + spacing;

	pb_select_equildata_create = new QPushButton(tr("Select Equilibrium Data"), this);
	Q_CHECK_PTR(pb_select_equildata_create);
	pb_select_equildata_create->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_select_equildata_create->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_select_equildata_create->setGeometry(xpos, ypos, buttonw0, buttonh);
	pb_select_equildata_create->setAutoDefault(false);
	connect(pb_select_equildata_create, SIGNAL(clicked()), SLOT(select_equildata_create_archive()));

	ypos += buttonh + spacing;

	pb_select_eqproject_create = new QPushButton(tr("Select Equilibrium Project"), this);
	Q_CHECK_PTR(pb_select_eqproject_create);
	pb_select_eqproject_create->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_select_eqproject_create->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_select_eqproject_create->setGeometry(xpos, ypos, buttonw0, buttonh);
	pb_select_eqproject_create->setAutoDefault(false);
	connect(pb_select_eqproject_create, SIGNAL(clicked()), SLOT(select_eqproject_create_archive()));

	ypos += buttonh + spacing;

	pb_select_montecarlo_create = new QPushButton(tr("Select Monte Carlo Project"), this);
	Q_CHECK_PTR(pb_select_montecarlo_create);
	pb_select_montecarlo_create->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_select_montecarlo_create->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_select_montecarlo_create->setGeometry(xpos, ypos, buttonw0, buttonh);
	pb_select_montecarlo_create->setAutoDefault(false);
	connect(pb_select_montecarlo_create, SIGNAL(clicked()), SLOT(select_montecarlo_create_archive()));

	ypos += buttonh + spacing;

	lbl_create_name1 = new QLabel(tr(" Archive:"),this);
	lbl_create_name1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_create_name1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
	lbl_create_name1->setGeometry(xpos, ypos, buttonw, buttonh);
	lbl_create_name1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));	
	lbl_create_name1->setAlignment(AlignLeft|AlignVCenter);

	xpos += spacing + buttonw;

	lbl_create_name2 = new QLabel("", this);
	lbl_create_name2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_create_name2->setGeometry(xpos, ypos, buttonw, buttonh);
	lbl_create_name2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	lbl_create_name2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_create_name2->setAlignment(AlignCenter|AlignVCenter);

	ypos += buttonh + spacing;
	xpos = spacing;

	pb_create = new QPushButton(tr("Create Archive"), this);
	Q_CHECK_PTR(pb_create);
	pb_create->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_create->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_create->setGeometry(xpos, ypos, buttonw0, buttonh);
	pb_create->setAutoDefault(false);
	pb_create->setEnabled(false);
	connect(pb_create, SIGNAL(clicked()), SLOT(create_archive()));

	ypos += buttonh + spacing + 2;

	banner3 = new QLabel(tr("Archive Extraction:"),this);
	banner3->setFrameStyle(QFrame::WinPanel|Raised);
	banner3->setAlignment(AlignCenter|AlignVCenter);
	banner3->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
	banner3->setGeometry(xpos, ypos, buttonw0, buttonh);
	banner3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	ypos += buttonh + spacing + 2;

	pb_extract = new QPushButton(tr("Extract Archive"), this);
	Q_CHECK_PTR(pb_extract);
	pb_extract->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_extract->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_extract->setGeometry(xpos, ypos, buttonw0, buttonh);
	pb_extract->setAutoDefault(false);
	connect(pb_extract, SIGNAL(clicked()), SLOT(extract_archive()));

	ypos += buttonh + spacing + 2;

	banner4 = new QLabel(tr("Module Controls:"),this);
	banner4->setFrameStyle(QFrame::WinPanel|Raised);
	banner4->setAlignment(AlignCenter|AlignVCenter);
	banner4->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
	banner4->setGeometry(xpos, ypos, buttonw0, buttonh);
	banner4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	ypos += buttonh + spacing + 2;

	pb_help = new QPushButton(tr("Help"), this);
	Q_CHECK_PTR(pb_help);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_help->setGeometry(xpos, ypos, buttonw0, buttonh);
	pb_help->setAutoDefault(false);
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	ypos += buttonh + spacing;

	pb_close = new QPushButton(tr("Close"), this);
	Q_CHECK_PTR(pb_close);
	pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_close->setGeometry(xpos, ypos, buttonw0, buttonh);
	pb_close->setAutoDefault(false);
	connect(pb_close, SIGNAL(clicked()), SLOT(close()));

	ypos += buttonh + spacing;

	global_Xpos += 30;
	global_Ypos += 30;
	
	setMinimumSize(720, ypos);
	setGeometry(global_Xpos, global_Ypos, 720, ypos);
	
	setup_GUI();
}

US_Archive::~US_Archive()
{
}

void US_Archive::setup_GUI()
{
	int j=0;
	int rows = 18, columns = 2, spacing = 2;
	
	QGridLayout * background = new QGridLayout(this,1,2,spacing);
	QGridLayout * subGrid1 = new QGridLayout(rows, columns, spacing);
	for (int i=0; i<rows; i++)
	{
		subGrid1->setRowSpacing(i, 26);
	}
	subGrid1->addMultiCellWidget(banner1,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(pb_view,j,j,0,1);
	j++;
	subGrid1->addWidget(lbl_name1,j,0);
	subGrid1->addWidget(lbl_name2,j,1);
	j++;
	subGrid1->addMultiCellWidget(banner2,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(cb_ultrascan,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(cb_reports,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(cb_delete,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(pb_select_velocdata_create,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(pb_select_equildata_create,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(pb_select_eqproject_create,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(pb_select_montecarlo_create,j,j,0,1);
	j++;
	subGrid1->addWidget(lbl_create_name1,j,0);
	subGrid1->addWidget(lbl_create_name2,j,1);
	j++;
	subGrid1->addMultiCellWidget(pb_create,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(banner3,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(pb_extract,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(banner4,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(pb_help,j,j,0,1);
	j++;
	subGrid1->addMultiCellWidget(pb_close,j,j,0,1);
		
	background->addLayout(subGrid1,0,0);
	background->addWidget(mle,0,1);
	background->setColStretch(0,1);
	background->setColStretch(1,4);
	background->setColSpacing(1,480);

	qApp->processEvents();
	QRect r = background->cellGeometry(0, 0);
	
	global_Xpos += 30;
	global_Ypos += 30;
	
	this->setGeometry(global_Xpos, global_Ypos, r.width()+480, r.height());





}

void US_Archive::closeEvent(QCloseEvent *e)
{
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
}
/*
void US_Archive::resizeEvent(QResizeEvent *e)
{
	int controlPanelWidth=224;
	mle->setGeometry(controlPanelWidth, 
						  2, 
						  e->size().width()-2-controlPanelWidth, 
						  e->size().height() - 4);
}
*/
void US_Archive::view()
{
	QString fn = QFileDialog::getOpenFileName(USglobal->config_list.archive_dir, "*.tar.gz", 0);
	mle->setReadOnly(false);
	if ( !fn.isEmpty() ) 
	{
		unsigned int length=USglobal->config_list.archive_dir.length() + 1;
		lbl_name2->setText(fn.mid(length, fn.length() - 7 - length));
		mle->clear();
		qApp->processEvents();
		view(fn);
	}
	mle->setReadOnly(true);
}

void US_Archive::view(const QString &fn)
{
	QString s;
#ifdef HAS_VFS
	int return_info = statfs(USglobal->config_list.data_dir, &file_info);
	if(return_info < 0)
	{
		QMessageBox::message(tr("Attention:"), 
									tr("There was an error in function statfs() while\n"
									"processing the requested information:\n\n") +
									(QString) strerror(errno));
		return;
		
	}
#endif
	view_file.setName(fn);
	mle->insert(tr("\n\nInformation for archive \"") + fn + "\":\n");
	mle->insert(tr("(compressed size: ") + s.sprintf(tr("%f MBytes)"), (float) view_file.size()/1e6));
	mle->insert(tr("\n\nThis archive contains the following sub-archives:\n\n"));

	int s_pos=USglobal->config_list.archive_dir.length()+1;
	int e_pos=fn.length();
	view_filename = fn.mid(s_pos, e_pos);

	view_proc = new QProcess(this);

	int cr=copy(fn,USglobal->config_list.root_dir + "/temp/"+view_filename); 
	if(cr==0)
	{
		v_step=0;		
		connect(view_proc, SIGNAL(readyReadStdout()), this, SLOT(readView()));
		connect(view_proc, SIGNAL(processExited()), this, SLOT(endView()));	
		endView();
	}
	else
	{
		QMessageBox::message("UltraScan Error:", "Unable to copy view data to temp dir.");
		return;
	}
}
void US_Archive::readView()
{
	mle->insert(view_proc->readStdout());
}

void US_Archive::endView()
{
	QString lineView, temp, str;
	int k;
	QDir view_dir;
	switch(v_step)
	{
		case 0:
		{
			view_proc->clearArguments();
			view_dir.setPath(USglobal->config_list.root_dir + "/temp/");
			view_proc->setWorkingDirectory(view_dir);
			QStringList view_cmd;
			view_cmd.append("gzip");
			view_cmd.append("-d");
			view_cmd.append(view_filename);
			view_proc->setArguments(view_cmd);
			v_step=1;
			if(!view_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to list view data.");
				return;
			}
			break;
		}
		case 1:
		{
			view_filename=view_filename.left(view_filename.length()-3);
			view_proc->clearArguments();
			view_dir.setPath(USglobal->config_list.root_dir+"/temp/");
			view_proc->setWorkingDirectory(view_dir);
			QStringList view_cmd;
			view_cmd.append("tar");
			view_cmd.append("-tvvf");
			view_cmd.append(view_filename);
			view_proc->setArguments(view_cmd);
		
			v_step=2;
			if(!view_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to list view data.");
				return;
			}
			break;
		}
	
		case 2:
		{
			unsigned int diskspace = 0;
			int line = 7; 		//the data read from line 7
			while(mle->text(line) )
			{
				lineView = mle->text(line);
//the file size value position is different for Win32 and Unix
#ifdef WIN32
			k=4;
#endif
#ifdef UNIX
			k=3;
#endif
				for (int i=0; i<k; i++)
				{
					temp = getToken(&lineView, " ");
				}
				diskspace += temp.toUInt();
				line++;
			}
#ifdef HAS_VFS
			mle->insert(tr("\nTotal diskspace needed for extraction: ") + str.sprintf(tr("%f MBytes\n"), diskspace/1e6));
			mle->insert(tr("Total available diskspace on ") + USglobal->config_list.data_dir + ": " 
			+ str.sprintf(tr("%f MBytes"), (float) file_info.f_bavail * file_info.f_bsize/1e6));
			if (diskspace > file_info.f_bavail * file_info.f_bsize)
			{
				mle->insert(tr("\n\nATTENTION: You will need to free up some diskspace before extracting this archive!"));
			}
#endif
			str.sprintf("\n\nCompression Ratio: %3.2f : 1\n\n", (float) diskspace/view_file.size());
			mle->insert(str);
			clean_temp_dir();
			delete view_proc;
		}
	}
}

void US_Archive::set_ultrascan()
{
	if (!ultrascan_flag)
	{
		ultrascan_flag = true;
		cb_ultrascan->setChecked(true);
	}
	else
	{
		ultrascan_flag = false;
		cb_ultrascan->setChecked(false);
	}
}

void US_Archive::set_delete()
{
	if (!delete_flag)
	{
		delete_flag = true;
		cb_delete->setChecked(true);
	}
	else
	{
		delete_flag = false;
		cb_delete->setChecked(false);
	}
}

void US_Archive::set_reports()
{
	if (!reports_flag)
	{
		reports_flag = true;
		cb_reports->setChecked(true);
	}
	else
	{
		reports_flag = false;
		cb_reports->setChecked(false);
	}
}

void US_Archive::quit()
{	
	close();
}

void US_Archive::select_velocdata_create_archive()
{
	run_type = 0;
	clean_temp_dir();
	if (data_ctrl_flag)
	{
		delete data_control;
		data_ctrl_flag = false;
	}
	data_control = new Data_Control_W(7);
	data_ctrl_flag = true;
	int flag = data_control->load_data();
	data_control->details();
	filename = data_control->run_inf.run_id;
	lbl_create_name2->setText(filename);
	if (flag < 0)
	{
		pb_create->setEnabled(false);
	}
	else
	{
		pb_create->setEnabled(true);
	}
}

void US_Archive::select_equildata_create_archive()
{
	run_type = 1;
	clean_temp_dir();
	if (data_ctrl_flag)
	{
		delete data_control;
		data_ctrl_flag = false;
	}
	data_control = new Data_Control_W(8);
	data_ctrl_flag = true;
	int flag = data_control->load_data();
	data_control->details();
	filename = data_control->run_inf.run_id;
	lbl_create_name2->setText(data_control->run_inf.run_id);
	pb_create->setEnabled(true);
	if (flag < 0)
	{
		pb_create->setEnabled(false);
	}
	else
	{
		pb_create->setEnabled(true);
	}
}

void US_Archive::select_eqproject_create_archive()
{
	run_type = 2;
	clean_temp_dir();
	if (data_ctrl_flag)
	{
		delete data_control;
		data_ctrl_flag = false;
	}
	QString str = QFileDialog::getOpenFileName(USglobal->config_list.result_dir, "*.eq-project", 0);
	if (!str.isEmpty())
	{
		QFile projectFile(str);
		projectFile.open(IO_ReadOnly);
		QTextStream project_ts(&projectFile);
		projectName = project_ts.readLine();
		projectFile.close();
		lbl_create_name2->setText(projectName);
		pb_create->setEnabled(true);
	}
	else
	{
		pb_create->setEnabled(false);
	}
}

void US_Archive::select_montecarlo_create_archive()
{
	run_type = 3;
	clean_temp_dir();
	if (data_ctrl_flag)
	{
		delete data_control;
		data_ctrl_flag = false;
	}
	QString str = QFileDialog::getOpenFileName(USglobal->config_list.result_dir, "*.Monte-Carlo", 0);
	if (!str.isEmpty())
	{
		QFile projectFile(str);
		projectFile.open(IO_ReadOnly);
		QTextStream project_ts(&projectFile);
		projectName = project_ts.readLine();
		projectFile.close();
		lbl_create_name2->setText(projectName);
		pb_create->setEnabled(true);
	}
	else
	{
		pb_create->setEnabled(false);
	}
}
void US_Archive::clean_temp_dir()
{
	QDir temp_dir;
	QString str = USglobal->config_list.root_dir + "/temp";
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
}

void US_Archive::disable_buttons()
{
	pb_view->setEnabled(false);
	pb_extract->setEnabled(false);
	pb_select_equildata_create->setEnabled(false);
	pb_select_velocdata_create->setEnabled(false);
	pb_select_montecarlo_create->setEnabled(false);
	pb_select_eqproject_create->setEnabled(false);
	pb_create->setEnabled(false);
}
void US_Archive::enable_buttons()
{
	pb_view->setEnabled(true);
	pb_extract->setEnabled(true);
	pb_select_equildata_create->setEnabled(true);
	pb_select_velocdata_create->setEnabled(true);
	pb_select_montecarlo_create->setEnabled(true);
	pb_select_eqproject_create->setEnabled(true);
}

/**********************************************************************************/
void US_Archive::create_archive()
{	
	mle->clear();
	QString str1, str2;
	str1= filename.stripWhiteSpace();
	str2= projectName.stripWhiteSpace();
	if (str1.isEmpty() && str2.isEmpty())
	{
		QMessageBox::message(tr("Please note:"), 
									tr("You will need to select some Data before you\n"
									"can create an Archive\n\n"
									"Click on \"Select Velocity Data for Archive\"\n"
									"or click on \"Select Equilibrium Data for Archive\"\n"
									"before proceeding..."));
		return;
	}
	disable_buttons();
	
	QDir temp_dir(USglobal->config_list.root_dir+"/temp");
	if(temp_dir.exists())
	{
		clean_temp_dir();
		temp_dir.rmdir(USglobal->config_list.root_dir+"/temp", true);
	}
	if(!temp_dir.mkdir(USglobal->config_list.root_dir+"/temp", true))
	{
		
		QMessageBox::message(tr("Ultrascan Warning:"), 
									tr("Can Not make a directory for temporary usage"));

	}
	
	
	create_proc = new QProcess(this);
	if(ultrascan_flag)			// result data is selected.
	{
		c_step = 0;
	}
	else			// ultrascan_flag==false, result data is NOT selected.
	{
		if((run_type == 2) || (run_type ==3))		//select equilibrium fitting or Monte Carlo project
		{
			QMessageBox::message("UltraScan Error:", "The UltraScan Data must be included for selecting project archive.");
			return;
		}
		if(reports_flag)		//report data is selected
		{
			c_step = 1;
		}
		else						//reports_flag == false, report data is NOT selected
		{
			if ((run_type == 0) || (run_type == 1))		// velocity or equilibrium data
			{	
				c_step = 2;
			}
			if ((run_type == 2) || (run_type == 3))		// equilibrium fitting or Monte Carlo project
			{	
				c_step = 3;
			}
		}
	}
	connect(create_proc, SIGNAL(readyReadStdout()), this, SLOT(readCreate()));
	connect(create_proc, SIGNAL(processExited()), this, SLOT(endCreateProcess()));
	endCreateProcess();
}

void US_Archive::readCreate()
{
	mle->insert(create_proc->readStdout());
}

void US_Archive::endCreateProcess()
{
	QDir work_dir;
	QStringList entries;
	QString str, name_filter;
	
	switch(c_step)
	{
		case 0:			//create run data tar file in result dir
		{			
			create_proc->clearArguments();			
			work_dir.setPath(USglobal->config_list.result_dir);
			create_proc->setWorkingDirectory(work_dir);
			QStringList u_cmd;
			u_cmd.append("tar");
			u_cmd.append("-cvvf");
			if ((run_type == 0) || (run_type == 1)) // velocity or equilibrium data
			{
				str = data_control->run_inf.run_id + ".ultrascan-data.tar";
	
			}
			if ((run_type == 2) || (run_type == 3)) // equilibrium fitting or Monte Carlo project
			{
				str = projectName + ".ultrascan-data.tar";
			}
			u_cmd.append(str);
			if ((run_type == 0) || (run_type == 1)) // velocity or equilibrium data
			{
				mle->insert(tr("\nCreating Archive of UltraScan Data for Run ") + data_control->run_inf.run_id + "...\n\n");
#ifdef WIN32
				u_cmd.append(filename + "/*");
#endif
#ifdef UNIX
		//		u_cmd.append(filename + "*");				
				QDir dat_dir;
				dat_dir.setPath(USglobal->config_list.result_dir);
				name_filter=filename+"*";
				dat_dir.setNameFilter(name_filter);	
				entries = dat_dir.entryList();
				for (QStringList::Iterator it = entries.begin(); it  != entries.end(); ++it ) 
				{
					
					u_cmd.append((*it).latin1());
				}
		
#endif
			}
			if (run_type == 2) // equilibrium fitting project
			{
				mle->insert(tr("\nCreating Archive of Equilibrium Fitting Project Data:\n\n ") + projectName + "...\n\n");
#ifdef WIN32				
				u_cmd.append(projectName + ".eq-project");
				u_cmd.append(projectName + "/*.dat");
				u_cmd.append(projectName + "/*.eq_fit");
				u_cmd.append(projectName + "/*.res");
				u_cmd.append(projectName + "/*.dis");
				u_cmd.append(projectName + "/*.lncr2");
				u_cmd.append(projectName + "/*.mw/*");
#endif
#ifdef UNIX	
				QDir dat_dir;
				dat_dir.setPath(USglobal->config_list.result_dir);
				name_filter = projectName + "*.eq-project; "+ projectName +"*.dat; "+ projectName +"*.eq_fit; "
								+ projectName +"*.res; " + projectName +"*.dis; "  + projectName +"*.lncr2; " + projectName +"*.mw*";			
				dat_dir.setNameFilter(name_filter);	
				entries = dat_dir.entryList();
				for (QStringList::Iterator it = entries.begin(); it  != entries.end(); ++it ) 
				{
					u_cmd.append((*it).latin1());
				}

#endif
			}
			if (run_type == 3) // Monte Carlo project
			{
				mle->insert(tr("\nCreating Archive of Monte Carlo Project Data:\n\n ") + projectName + "...\n\n");
				QString filter1 = projectName + ".Monte-Carlo ";
				QString filter2 = projectName + ".mc";
				u_cmd.append(filter1);		
				u_cmd.append(filter2);
			}
	
			create_proc->setArguments(u_cmd);
			if(reports_flag)		//ultrascan_flag == true && reports_flag == true.
			{
				c_step = 1;
			}
			else 						//ultrascan_flag == true && reports_flag == false.
			{
				if ((run_type == 0) || (run_type == 1))	// velocity or equilibrium data
				{	
					c_step = 2;
				}
				if ((run_type == 2) || (run_type == 3))	// equilibrium fitting or Monte Carlo project
				{	
					c_step = 3;
				}
			}
			if(!create_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to create data archive.");
				return;
			}
			break;
		}
		case 1:			//create report tar file
		{
			create_proc->clearArguments();		
			work_dir.setPath(USglobal->config_list.html_dir);
			create_proc->setWorkingDirectory(work_dir);
			QStringList r_cmd;
			r_cmd.append("tar");
			r_cmd.append("--mode=u+X");
			r_cmd.append("-cvvf");
			if ((run_type == 0) || (run_type == 1)) // velocity or equilibrium data
			{
				mle->insert(tr("\nCreating Archive of HTML Reports for Run ") + data_control->run_inf.run_id + "...\n\n");
				str = data_control->run_inf.run_id + ".report-files.tar";
				r_cmd.append(str);
				r_cmd.append(filename);
				
			}
			if (run_type == 2) // equilibrium fitting project
			{
				mle->insert(tr("\nCreating Archive of HTML Reports for Equilibrium Fitting Project:\n\n") + projectName + "...\n\n");
				str = projectName + ".report-files.tar";
				r_cmd.append(str);
				r_cmd.append(projectName);
			}
			if (run_type == 3) // Monte Carlo project
			{
				mle->insert(tr("\nCreating Archive of HTML Reports for Monte Carlo Project Data:\n\n ") + projectName + "...\n\n");
				str = projectName + ".report-files.tar";
				r_cmd.append(str);
				str = projectName + ".mc";
				r_cmd.append(str);
			}

			create_proc->setArguments(r_cmd);
			if ((run_type == 0) || (run_type == 1))		// velocity or equilibrium data
			{	
				c_step = 2;
			}
			if ((run_type == 2) || (run_type == 3))		// equilibrium fitting or Monte Carlo project
			{	
				c_step = 3;
			}
			if(!create_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to create report archive.");
				return;
			}
			break;
		}
		case 2:			//create raw data tar file
		{
			create_proc->clearArguments();
			QString dirname = data_control->run_inf.data_dir;		
			int position = dirname.findRev("/", -2, false);
			dirname = dirname.mid(position + 1, dirname.length()-1);		
			mle->insert(tr("\nCreating Archive of Raw Experimental Datafiles for Run ") + 
			data_control->run_inf.run_id + "...\n\n");
			str= USglobal->config_list.data_dir+"/"+dirname;
			work_dir.setPath(str);
			if(!work_dir.exists())
			{	
				QMessageBox::message("UltraScan Warning:", "The required raw data directory does not exist:\n" + str + "\n"
											"Please select the raw data directory.");
				QString new_dir = QFileDialog::getExistingDirectory(USglobal->config_list.data_dir, this, "get existing directory", "Choose a directory",true);
				if(!new_dir.isEmpty())
				{
					work_dir.setPath(new_dir);
					position = new_dir.findRev("/", -2, false);
					new_dir = new_dir.mid(position + 1, new_dir.length()-1);

					if(QString::compare(dirname, new_dir) !=0)
					{
						QMessageBox::message("UltraScan Warning:", "New directory name is different than recorded name.");
						dirname=new_dir;
					}
				}
				else
				{
					QMessageBox::message("UltraScan Error:", "No raw data be selected, progam will be terminated.");
					exit(0); 			
				}
			}
			work_dir.cdUp();
			current_data_dir=work_dir.path();
			create_proc->setWorkingDirectory(work_dir);
			QStringList c_cmd;
			c_cmd.append("tar");
			c_cmd.append("--mode=u+X");
			c_cmd.append("-cvvf");
			c_cmd.append(data_control->run_inf.run_id + ".raw-data.tar");
			c_cmd.append(dirname);
			create_proc->setArguments(c_cmd);
			c_step = 3;
			if(!create_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to create raw data archive.");
				return;
			}	
			break;
		}
		case 3:		 //move tar files from different dir to temp dirs and compress temp-dir 
		{
			QString result_tar_file, report_tar_file, rawdata_tar_file;
			QString result_dst_file, report_dst_file, rawdata_dst_file;

			if ((run_type == 0) || (run_type == 1)) // velocity or equilibrium data result tar file
			{
				result_tar_file=USglobal->config_list.result_dir + "/"+ data_control->run_inf.run_id + ".ultrascan-data.tar";
				result_dst_file=USglobal->config_list.root_dir+"/temp/"+ data_control->run_inf.run_id + ".ultrascan-data.tar";
				report_tar_file=USglobal->config_list.html_dir + "/"+ data_control->run_inf.run_id + ".report-files.tar";
				report_dst_file=USglobal->config_list.root_dir+"/temp/"+ data_control->run_inf.run_id + ".report-files.tar";
				rawdata_tar_file=current_data_dir+"/"+ data_control->run_inf.run_id + ".raw-data.tar";
				rawdata_dst_file=USglobal->config_list.root_dir+"/temp/"+ data_control->run_inf.run_id + ".raw-data.tar";	
				
			}
			if ((run_type == 2) || (run_type == 3)) // equilibrium fitting or Monte Carlo project result tar file
			{
				result_tar_file=USglobal->config_list.result_dir + "/"+ projectName + ".ultrascan-data.tar";
				result_dst_file=USglobal->config_list.root_dir+"/temp/"+ projectName + ".ultrascan-data.tar";
				report_tar_file=USglobal->config_list.html_dir + "/"+ projectName + ".report-files.tar";
				report_dst_file=USglobal->config_list.root_dir+"/temp/"+ projectName + ".report-files.tar";
			}
			QFile resultFile(result_tar_file);
			if(resultFile.exists())
			{
				int rp=Move(result_tar_file, result_dst_file);
				if(rp<0)
				{
					QMessageBox::message("UltraScan Error:", "Unable to move result tar file from result dir to temp dir.\n"
																			"the compress archive is failed.\n\n"
																			"Error Code in Move function : " + QString::number(rp));
					exit(0);
				}	
			}
			QFile reportFile(report_tar_file);
			if(reportFile.exists())
			{
				int rp=Move(report_tar_file, report_dst_file);
				if(rp<0)
				{
					QMessageBox::message("UltraScan Error:", "Unable to move report tar file from result dir to temp dir.\n"
																			"the compress archive is failed.\n\n"
																			"Error Code in Move function : " + QString::number(rp));
					exit(0);														
				}	
			}
			QFile rawdataFile(rawdata_tar_file);
			if(rawdataFile.exists())
			{
				int rp=Move(rawdata_tar_file, rawdata_dst_file);
				if(rp<0)
				{
					QMessageBox::message("UltraScan Error:", "Unable to move rawdata tar file from result dir to temp dir.\n"
																			"the compress archive is failed.\n\n"
																			"Error Code in Move function : " + QString::number(rp));
					exit(0);														
				}	
			}

			create_proc->clearArguments();
			mle->insert(tr("\nCombining Sub-Archives...\n\n"));
			work_dir.setPath(USglobal->config_list.root_dir+"/temp/");
			create_proc->setWorkingDirectory(work_dir);
			QStringList t_cmd;
			t_cmd.append("tar");
			t_cmd.append("-cvvf");
			if ((run_type == 0) || (run_type == 1)) // velocity or equilibrium data
			{
				str = data_control->run_inf.run_id + ".tar";
			}
			if (run_type == 2) // equilibrium fitting project
			{
			 	str = projectName + ".project.tar";
			}
			if (run_type == 3) // Monte Carlo project
			{
				str = projectName + ".mc.project.tar";
			}
			t_cmd.append(str);
#ifdef WIN32
			t_cmd.append(filename + "/*.tar");
#endif
#ifdef UNIX			
			QDir dat_dir;
			dat_dir.setPath(USglobal->config_list.root_dir+"/temp/");
			name_filter=filename+"*.tar";
			dat_dir.setNameFilter(name_filter);	
			entries = dat_dir.entryList();
			for (QStringList::Iterator it = entries.begin(); it  != entries.end(); ++it ) 
			{				
				t_cmd.append((*it).latin1());
			}

#endif
		
			create_proc->setArguments(t_cmd);
			c_step = 4;
			if(!create_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to compress tar file.");
				return;
			}
			break;
			
		}
		case 4:			//gzip temp-dir tar file
		{
			create_proc->clearArguments();
			mle->insert(tr("\nCompressing Archive in Background...\n"));
			work_dir.setPath(USglobal->config_list.root_dir+"/temp/");
			create_proc->setWorkingDirectory(work_dir);
			QStringList g_cmd;
			g_cmd.append("gzip");
			g_cmd.append("-f9");
			if ((run_type == 0) || (run_type == 1)) // velocity or equilibrium data
			{
				str = data_control->run_inf.run_id + ".tar";
			}
			if (run_type == 2) // equilibrium fitting project
			{
				str = projectName + ".project.tar";
			}
			if (run_type == 3) // Monte Carlo project
			{
				str = projectName + ".mc.project.tar";
			}
			g_cmd.append(str);
			create_proc->setArguments(g_cmd);
			c_step = 5;
			if(!create_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to gzip tar file on temp dir.");
				return;
			}
			break;
		}
		case 5:
		{	
			QString temp_gz_str, gz_str;	
			if ((run_type == 0) || (run_type == 1)) // velocity or equilibrium data
			{
				temp_gz_str = USglobal->config_list.root_dir+"/temp/"+ data_control->run_inf.run_id + ".tar.gz";
				gz_str = USglobal->config_list.archive_dir +"/"+ data_control->run_inf.run_id + ".tar.gz";
			}
			if (run_type == 2) // equilibrium fitting project
			{
				temp_gz_str = USglobal->config_list.root_dir+"/temp/"+ projectName + ".project.tar.gz";
				gz_str = USglobal->config_list.archive_dir +"/"+ projectName + ".project.tar.gz";
			}
			if (run_type == 3) // Monte Carlo project
			{
				temp_gz_str = USglobal->config_list.root_dir+"/temp/"+ projectName + ".mc.project.tar.gz";
				gz_str = USglobal->config_list.archive_dir +"/"+ projectName + ".mc.project.tar.gz";
			}
			QFile gzFile(gz_str);
			if(gzFile.exists())
			{
				switch( QMessageBox::information( this, "Ultrascan Warning:", "Archive File '" +
												gz_str + "'\n"
												"already exists, overwrite?",
												"&Yes", "&Cancel", "Rename",
												0, 2 ) )	  
				{ 
					case 0:
					{
						if(Move(temp_gz_str, gz_str)<0)
						{
							QMessageBox::message("UltraScan Error:", "Unable to move compressed file from temp dir to overwrite the old file.\n"
																			"the compress archive is failed.\n\n");														
						}	
						break;
					}
					case 1:  
					{
						break;
					}
					case 2: 
					{
						QString savefilename = QFileDialog::getSaveFileName(USglobal->config_list.archive_dir, "*.tar.gz", 0);
						if(!savefilename.isEmpty())
						{
								int rp = savefilename.find(".tar.gz", 1, false);
								if(rp == -1)	//not found
								{
									savefilename=savefilename+".tar.gz";
								}
								if(Move(temp_gz_str, savefilename)<0)
								{
									QMessageBox::message("UltraScan Error:", "Unable to move compressed file from temp dir to create new file.\n"
																			"the compress archive is failed.\n\n");														
								}	
						}
						break;
					}
				}
			}
			else	//new file
			{
				if(Move(temp_gz_str, gz_str)<0)
				{
					QMessageBox::message("UltraScan Error:", "Unable to move compressed file from temp dir to archive dir.\n"
																			"the compress archive is failed.\n\n");														
				}	

			}
			if(delete_flag)		//delete source file option is selected
			{
				mle->insert(tr("\nDeleting Source Files...\n"));
				//delete result file
				if ((run_type == 0) || (run_type == 1)) // velocity or equilibrium data
				{
					name_filter = filename + "*";
				}
				if (run_type == 2) // equilibrium fitting project
				{
					name_filter = projectName + "*.eq-project; "+ projectName +"*.dat; "+ projectName +"*.eq_fit; "
									+ projectName + "*.res; " + projectName + "*.dis; " + projectName + "*.lncr2; " + projectName +"*.mw*";
				}
				if (run_type == 3) // Monte Carlo project
				{
					name_filter = projectName + ".Monte-Carlo; " + projectName + ".mc";
				}
				work_dir.setPath(USglobal->config_list.result_dir);
				work_dir.setNameFilter(name_filter);
				entries = work_dir.entryList();
				for (QStringList::Iterator it = entries.begin(); it != entries.end(); ++it ) 
				{
					work_dir.remove((*it).latin1());
				}
				//delete report file
				if ((run_type == 0) || (run_type == 1)) // velocity or equilibrium data
				{
					str = USglobal->config_list.html_dir + "/" + filename;
				}
				if (run_type == 2) // equilibrium fitting project
				{
					str = USglobal->config_list.html_dir + "/" + projectName;
				}
				if (run_type == 3) // Monte Carlo project
				{
					str = USglobal->config_list.html_dir + "/" + projectName + ".mc";
				}
				work_dir.setPath(str);
				work_dir.setNameFilter("*.*");
				entries = work_dir.entryList();
				for (QStringList::Iterator it = entries.begin(); it != entries.end(); ++it ) 
				{
					work_dir.remove((*it).latin1());
				}
				work_dir.cdUp();
				if ((run_type == 0) || (run_type == 1)) // velocity or equilibrium data
				{
					work_dir.rmdir(filename);
				}
				if ((run_type == 2) || (run_type == 3)) // equilibrium fitting or Monte Carlo project
				{
					work_dir.rmdir(projectName);
				}
				//delete data file
				if ((run_type == 0) || (run_type == 1)) // velocity or equilibrium data
				{
					work_dir.setPath(data_control->run_inf.data_dir);
					work_dir.setNameFilter("*.*");
					entries = work_dir.entryList();
					for (QStringList::Iterator it = entries.begin(); it != entries.end(); ++it ) 
					{
						work_dir.remove((*it).latin1());
					}
					work_dir.cdUp();
					work_dir.rmdir(filename);
				}
			}
		//Finish the creation job
			mle->insert(tr("\nArchive Creation finished...\n"));
			enable_buttons();
			delete create_proc;
		}
	}
	pb_create->setEnabled(false);
}

/**********************************************************************************
void US_Archive::create_archive()
{
	QString str, str1, str2;
	char cstr[500];
	str1= filename.stripWhiteSpace();
	str2= projectName.stripWhiteSpace();
	if (str1.isEmpty() && str2.isEmpty())
	{
		QMessageBox::message(tr("Please note:"), 
									tr("You will need to select some Data before you\n"
									"can create an Archive\n\n"
									"Click on \"Select Velocity Data for Archive\"\n"
									"or click on \"Select Equilibrium Data for Archive\"\n"
									"before proceeding..."));
		return;
	}
	if (run_type == 2) // equilibrium fitting project
	{
		mle->setReadOnly(false);
		mle->clear();
		qApp->processEvents();
		if (ultrascan_flag)
		{
			mle->insert(tr("\nCreating Archive of Equilibrium Fitting Project Data:\n\n ") + 
			projectName + "...\n\n");
			str = "cd " + USglobal->config_list.result_dir + "; " 
			+ "tar -cvvf " + USglobal->config_list.root_dir 
			+ "/temp/" + projectName + ".ultrascan-data.tar"
			+ " " + projectName + "*.dat" + " " + projectName + "*.eq_fit" + " " + projectName + ".res" + " " + projectName + ".dis" ;

// projectName + "*.mw*"

// projectName + "*.res" etc., *.dat, *.dis, *.eq-project

			FILE *f = popen(str, "r");
			while (fgets(cstr, 500, f))
			{
				mle->insert((QString) cstr);
			}
			pclose(f);
			mle->insert("\n");
	
		}		
		if (reports_flag)
		{
			mle->insert(tr("\nCreating Archive of HTML Reports for Equilibrium Fitting Project:\n\n") + 
			projectName + "...\n\n");
			str = "cd " + USglobal->config_list.html_dir + "; " 
			+ "tar -cvvf " + USglobal->config_list.root_dir 
			+ "/temp/" + data_control->run_inf.run_id + ".report-files.tar"
			+ " " + projectName;
			char cstr[500];
			FILE *f = popen(str, "r");
			while (fgets(cstr, 500, f))
			{
				mle->insert((QString) cstr);
			}
			pclose(f);
			mle->insert("\n");
		}		
		mle->insert(tr("\nCombining Sub-Archives...\n\n"));
		qApp->processEvents();
		str = "cd " + USglobal->config_list.root_dir + "/temp;"
		+ "tar -cvvf " + USglobal->config_list.archive_dir + "/" 
		+ projectName + ".tar *; rm *";
		FILE *f1 = popen(str, "r");
		while (fgets(cstr, 500, f1))
		{
			mle->insert((QString) cstr);
		}
		pclose(f1);
		str = "gzip -f9 " + USglobal->config_list.archive_dir + "/" 
		+ projectName + ".tar &";
		mle->insert(tr("\nCompressing Archive in Background...\n"));
		system(str);
		if (delete_flag)
		{
			mle->insert(tr("\nDeleting Source Files...\n"));
			str = "cd " + USglobal->config_list.result_dir 
			+ "; rm -f " + projectName + ".dat"
			+ " " + projectName + ".eq_fit"
			+ " " + projectName + ".res"
			+ " " + projectName + ".dis" ;
			system(str);
			str = "rm -rf " + USglobal->config_list.html_dir + "/" + projectName;
			system(str);
		}
		mle->insert(tr("\nArchive Creation finished...\n"));
	}
	if (run_type == 3) // Monte Carlo project
	{
		mle->setReadOnly(false);
		mle->clear();
		qApp->processEvents();
		if (ultrascan_flag)
		{
			mle->insert(tr("\nCreating Archive of Monte Carlo Project Data:\n\n ") + 
			projectName + "...\n\n");
			str = "cd " + USglobal->config_list.result_dir + "; " 
			+ "tar -cvvf " + USglobal->config_list.root_dir 
			+ "/temp/" + projectName + ".ultrascan-data.tar"
			+ " " + projectName + ".Monte-Carlo" + " " + projectName + ".mc";
			FILE *f = popen(str, "r");
			while (fgets(cstr, 500, f))
			{
				mle->insert((QString) cstr);
			}
			pclose(f);
			mle->insert("\n");
		}		
		if (reports_flag)
		{
			mle->insert(tr("\nCreating Archive of HTML Reports for Monte Carlo Project:\n\n") + 
			projectName + "...\n\n");
			str = "cd " + USglobal->config_list.html_dir + "; " 
			+ "tar -cvvf " + USglobal->config_list.root_dir 
			+ "/temp/" + projectName + ".report-files.tar"
			+ " " + projectName + ".mc";
			char cstr[500];
			FILE *f = popen(str, "r");
			while (fgets(cstr, 500, f))
			{
				mle->insert((QString) cstr);
			}
			pclose(f);
			mle->insert("\n");
		}		
		mle->insert(tr("\nCombining Sub-Archives...\n\n"));
		qApp->processEvents();
		str = "cd " + USglobal->config_list.root_dir + "/temp;"
		+ "tar -cvvf " + USglobal->config_list.archive_dir + "/" 
		+ projectName + ".mc.tar *; rm *";
		FILE *f1 = popen(str, "r");
		while (fgets(cstr, 500, f1))
		{
			mle->insert((QString) cstr);
		}
		pclose(f1);
		str = "gzip -f9 " + USglobal->config_list.archive_dir + "/" 
		+ projectName + ".mc.tar &";
		mle->insert(tr("\nCompressing Archive in Background...\n"));
		system(str);
		if (delete_flag)
		{
			mle->insert(tr("\nDeleting Source Files...\n"));
			str = "cd " + USglobal->config_list.result_dir 
			+ "; rm -f " + projectName + ".Monte-Carlo " + projectName + ".mc";
			system(str);
			str = "rm -rf " + USglobal->config_list.html_dir + "/" + projectName + ".mc";
			system(str);
		}
		mle->insert(tr("\nArchive Creation finished...\n"));
	}
	if ((run_type == 0) || (run_type == 1)) // velocity or equilibrium data
	{
		mle->setReadOnly(false);
		mle->clear();
		qApp->processEvents();
		if (ultrascan_flag )
		{
			continue_flag = false;
			mle->insert(tr("\nCreating Archive of UltraScan Data for Run ") + 
			data_control->run_inf.run_id + "...\n\n");
			str = "cd " + USglobal->config_list.result_dir + "; " 
			+ "tar -cvvf " + USglobal->config_list.root_dir 
			+ "/temp/" + data_control->run_inf.run_id + ".ultrascan-data.tar"
			+ " " + filename + "*";
			FILE *f = popen(str, "r");
			while (fgets(cstr, 500, f))
			{
				mle->insert((QString) cstr);
			}
			pclose(f);
			mle->insert("\n");		
		}		
		if (reports_flag)
		{
			mle->insert(tr("\nCreating Archive of HTML Reports for Run ") + 
			data_control->run_inf.run_id + "...\n\n");
			str = "cd " + USglobal->config_list.html_dir + "; " 
			+ "tar -cvvf " + USglobal->config_list.root_dir 
			+ "/temp/" + data_control->run_inf.run_id + ".report-files.tar"
			+ " " + filename;
			char cstr[500];
			FILE *f = popen(str, "r");
			while (fgets(cstr, 500, f))
			{
				mle->insert((QString) cstr);
			}
			pclose(f);
			mle->insert("\n");
		}		
		QString dirname = data_control->run_inf.data_dir;
		int position = dirname.findRev("/", -2, false);
		dirname = dirname.mid(position + 1, dirname.length());
		mle->insert(tr("\nCreating Archive of Raw Experimental Datafiles for Run ") + 
		data_control->run_inf.run_id + "...\n\n");
		str = "cd " + data_control->run_inf.data_dir + "; cd .. ; " 
		+ "tar -cvvf " + USglobal->config_list.root_dir 
		+ "/temp/" + data_control->run_inf.run_id + ".raw-data.tar"
		+ " " + dirname;
		FILE *f = popen(str, "r");
		while (fgets(cstr, 500, f))
		{
			mle->insert((QString) cstr);
		}
		pclose(f);
		mle->insert(tr("\nCombining Sub-Archives...\n\n"));
		qApp->processEvents();
		str = "cd " + USglobal->config_list.root_dir + "/temp;"
		+ "tar -cvvf " + USglobal->config_list.archive_dir + "/" 
		+ data_control->run_inf.run_id + ".tar *; rm *";
		FILE *f1 = popen(str, "r");
		while (fgets(cstr, 500, f1))
		{
			mle->insert((QString) cstr);
		}
		pclose(f1);
		str = "gzip -f9 " + USglobal->config_list.archive_dir + "/" 
		+ data_control->run_inf.run_id + ".tar &";
		mle->insert(tr("\nCompressing Archive in Background...\n"));
		system(str);
		
		if (delete_flag)
		{
			mle->insert(tr("\nDeleting Source Files...\n"));
			str = "cd " + USglobal->config_list.result_dir 
			+ "; rm -f " + filename + "*";
			system(str);
			str = "rm -rf " + data_control->run_inf.data_dir;
			system(str);
			str = "rm -rf " + USglobal->config_list.html_dir + "/" + filename;
			system(str);
		}
		mle->insert(tr("\nArchive Creation finished...\n"));
	
	}
	mle->setReadOnly(true);
}
*/

/*****************************************************************************/
void US_Archive::extract_archive()
{
	fn = QFileDialog::getOpenFileName(USglobal->config_list.archive_dir, "*.tar.gz", 0);
	if ( !fn.isEmpty() ) 
	{
		int s_pos=USglobal->config_list.archive_dir.length()+1;
		int e_pos=fn.length();
		extract_filename = fn.mid(s_pos, e_pos);	
		clean_temp_dir();	
		mle->clear();
		mle->insert(tr("\nExtracting Archive: \n") + fn + ": \n\n");
		
		disable_buttons();	
		extract_proc = new QProcess(this);
//copy tar.gz fie to temp dir for uncompress		
		int cr=copy(fn,USglobal->config_list.root_dir + "/temp/"+extract_filename); 
		if(cr==0)
		{
			e_step = 0;
			connect(extract_proc, SIGNAL(readyReadStdout()), this, SLOT(readExtract()));
			connect(extract_proc, SIGNAL(processExited()), this, SLOT(endExtractProcess()));
			endExtractProcess();
		}
		else
		{
			QMessageBox::message("UltraScan Error:", "Unable to copy archive to temp dir for extraction.");
			return;
		}
	}
}

void US_Archive::readExtract()
{
	mle->insert(extract_proc->readStdout());
}

void US_Archive::endExtractProcess()
{
	QDir work_dir;
	QStringList entries;
	QString str;
//	QString str, raw_file, result_file, report_file;
	QFile rawFile, resultFile, reportFile;
	
	
	switch(e_step)
	{
		case 0:		//unzip the *.tar.gz file
		{
			work_dir.setPath(USglobal->config_list.root_dir + "/temp");
			extract_proc->setWorkingDirectory(work_dir);
			extract_proc->clearArguments();
			QStringList u_cmd;
			u_cmd.append("gzip");
			u_cmd.append("-d");
			u_cmd.append(extract_filename);
			extract_proc->setArguments(u_cmd);
			e_step = 1;
			if(!extract_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to unzip archive.");
				return;
			}
			break;
		}
		case 1:			//tar -xf *.tar file
		{	
			extract_filename=extract_filename.left(extract_filename.length()-3);
			extract_proc->clearArguments();
			str = USglobal->config_list.root_dir + "/temp";
			work_dir.setPath(str);
			extract_proc->setWorkingDirectory(work_dir);
			QStringList u_cmd;
			u_cmd.append("tar");
			u_cmd.append("-xf");
			u_cmd.append(extract_filename);
			extract_proc->setArguments(u_cmd);
			
			e_step = 2;
			if(!extract_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to extract *.tar file.");
				return;
			}

			break;
		}
		case 2:	//Move tar file to according dir and then uncompress result-data file
		{
			str = USglobal->config_list.root_dir + "/temp";
			work_dir.setPath(str);
			work_dir.setNameFilter("*.tar");
			entries = work_dir.entryList();
			for (QStringList::Iterator it = entries.begin(); it  != entries.end(); ++it ) 
			{
				str = (*it).latin1();
				if(str.contains(".ultrascan-data.tar", false)>0)
				{
					result_file = str;
					resultFile.setName(USglobal->config_list.root_dir +"/temp/"+result_file);
					if(resultFile.exists())
					{
						copy(USglobal->config_list.root_dir +"/temp/"+result_file, USglobal->config_list.result_dir+"/"+result_file);
					}
				}
				if(str.contains(".report-files.tar", false)>0)
				{
					report_file = str;
					reportFile.setName(USglobal->config_list.root_dir + "/temp/"+report_file);
					copy(USglobal->config_list.root_dir +"/temp/"+report_file, USglobal->config_list.html_dir+"/"+report_file);
				}
				if(str.contains(".raw-data.tar", false)>0)
				{
					raw_file = str;
					rawFile.setName(USglobal->config_list.root_dir + "/temp/"+raw_file);
					copy(USglobal->config_list.root_dir + "/temp/"+raw_file, USglobal->config_list.data_dir+"/"+raw_file);
				}
			}
			
			extract_proc->clearArguments();
			mle->insert(tr("\n\nRestoring Result Sub-Archive \n") + USglobal->config_list.result_dir +"/"+ result_file + ": \n\n");
			work_dir.setPath(USglobal->config_list.result_dir);
			extract_proc->setWorkingDirectory(work_dir);
			QStringList r_cmd;
			r_cmd.append("tar");
			r_cmd.append("-xf");
			r_cmd.append(result_file);
			extract_proc->setArguments(r_cmd);
			if (reportFile.exists())	//result file exists
			{
					e_step = 3;
			}
			if(!reportFile.exists() && rawFile.exists())	
			{
					e_step = 4;
			}
			if(!reportFile.exists() && !rawFile.exists())
			{
					e_step = 5;
			}
			if(!extract_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to extract raw data file.");
				return;
			}
			break;

		}
		case 3:	// For uncompressing report-data file
		{
				extract_proc->clearArguments();
			mle->insert(tr("\n\nRestoring Report Sub-Archive \n")+ USglobal->config_list.html_dir +"/"+ report_file + ": \n\n");
			work_dir.setPath(USglobal->config_list.html_dir);
			extract_proc->setWorkingDirectory(work_dir);
			QStringList r_cmd;
			r_cmd.append("tar");
			r_cmd.append("-xf");
			r_cmd.append(report_file);
			extract_proc->setArguments(r_cmd);
			
			rawFile.setName(USglobal->config_list.data_dir+"/"+raw_file);
			if (rawFile.exists())	//result file exists
			{
				e_step = 4;
			}
			else
			{
				e_step = 5;
			}

			if(!extract_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to extract report file.");
				return;
			}
			break;
		}

		case 4:	//this case is for velocity or equilibrium data, uncompress raw data .
		{
			extract_proc->clearArguments();
			mle->insert(tr("\n\nRestoring Raw Data Sub-Archive \n") + USglobal->config_list.data_dir +"/"+ raw_file + ": \n\n");
			work_dir.setPath(USglobal->config_list.data_dir);
			extract_proc->setWorkingDirectory(work_dir);
			QStringList r_cmd;
			r_cmd.append("tar");
			r_cmd.append("-xf");
			r_cmd.append(raw_file);
			extract_proc->setArguments(r_cmd);
			e_step=5;			
			if(!extract_proc->start())
			{
				QMessageBox::message("UltraScan Error:", "Unable to start process to extract raw data file.");
				return;
			}
			break;			
		}
		case 5:	//Finish extracting archive and clear tar file
		{
			QFile tarFile1(USglobal->config_list.data_dir+"/"+raw_file);
			if(tarFile1.exists())
			{
				tarFile1.remove();
			}
			QFile tarFile2(USglobal->config_list.result_dir+"/"+result_file);
			if(tarFile2.exists())
			{
				tarFile2.remove();
			}
			QFile tarFile3(USglobal->config_list.html_dir+"/"+report_file);
			if(tarFile3.exists())
			{
				tarFile3.remove();
			}
			work_dir.setPath(USglobal->config_list.root_dir+"/temp/");
			work_dir.setNameFilter("*.*");
			entries = work_dir.entryList();
			for (QStringList::Iterator it = entries.begin(); it != entries.end(); ++it ) 
			{
				work_dir.remove((*it).latin1());
			}		
			
			mle->insert(tr("\nFinished Restoring Archive ")+ fn + "...\n");
			enable_buttons();
			delete extract_proc;
		}
	}
}
/*****************************************************************************/
/*
void US_Archive::extract_archive()
{
	int return_info;
	QString fn = QFileDialog::getOpenFileName(USglobal->config_list.archive_dir, "*.tar.gz", 0);
	mle->setReadOnly(false);
	if ( !fn.isEmpty() ) 
	{
		char cstr[500];
		unsigned int position;
		position = fn.find(".tar.gz", 1, false);
		QString filename, current_file;
		filename = fn.left(position);
		position = filename.findRev("/", -1, false);
		filename = filename.mid(position + 1, filename.length() - position - 1);
		QString str = "rm -rf " + USglobal->config_list.root_dir + "/temp";
		system(str);
		str = "mkdir " + USglobal->config_list.root_dir + "/temp";
		system(str);
		mle->clear();
		mle->insert(tr("\nExtracting Archive ") + fn + ": \n\n");
		qApp->processEvents();
		str = "cd " + USglobal->config_list.root_dir + "/temp;"
		+ "tar -xvvzf " + fn;
		FILE *f1 = popen(str, "r");
		while (fgets(cstr, 500, f1))
		{
			mle->insert((QString) cstr);
		}
		return_info = pclose(f1);
		if(return_info != 0)
		{
			QMessageBox::message(tr("Attention:"), 
										tr("There was an error in function tar/gzip while\n"
										"processing the main archive.\n\n"
										"This archive may be corrupted.\n"
										"If you just created this archive, make sure\n"
										"that the background compression process has\n"
										"terminated before trying again.\n\n"
										"Check the \"top\" command from the command\n"
										"line to see if the gzip process is still\n"
										"active. If so, just wait a few minutes and\n"
										"try again..."));
			return;
		}
		QFile resultfile;
		current_file = USglobal->config_list.root_dir + "/temp/" + filename + ".ultrascan-data.tar";
		resultfile.setName(current_file);
		if (resultfile.exists())
		{
			mle->insert(tr("\n\nRestoring Sub-Archive ") + current_file + ": \n\n");
			str = "cd " + USglobal->config_list.result_dir + ";"
			+ "tar -xf " + current_file;
			FILE *f2 = popen(str, "r");
			while (fgets(cstr, 500, f2))
			{
				mle->insert((QString) cstr);
			}
			return_info = pclose(f2);
			if(return_info < 0)
			{
				QMessageBox::message(tr("Attention:"), 
											tr("There was an error in function tar/gzip while\n"
											"processing the subarchive \"" + current_file + "\".\n\n"
											"This archive may be corrupted.\n\n"
											"If you just created this archive, make sure\n"
											"that the background compression process has\n"
											"terminated before trying again."));
				return;
			}
		}
		current_file = USglobal->config_list.root_dir + "/temp/" + filename + ".report-files.tar";
		resultfile.setName(current_file);
		if (resultfile.exists())
		{
			mle->insert(tr("\n\nRestoring Sub-Archive ") + current_file + ": \n\n");
			str = "cd " + USglobal->config_list.html_dir + ";"
			+ "tar -xf " + current_file;
			FILE *f3 = popen(str, "r");
			while (fgets(cstr, 500, f3))
			{
				mle->insert((QString) cstr);
			}
			return_info = pclose(f3);
			if(return_info < 0)
			{
				QMessageBox::message(tr("Attention:"), 
											tr("There was an error in function tar/gzip while\n"
											"processing the subarchive \"" + current_file + "\".\n\n"
											"This archive may be corrupted.\n\n"
											"If you just created this archive, make sure\n"
											"that the background compression process has\n"
											"terminated before trying again."));
				return;
			}
		}
		current_file = USglobal->config_list.root_dir + "/temp/" + filename + ".raw-data.tar";
		resultfile.setName(current_file);
		if (resultfile.exists())
		{
			mle->insert(tr("\n\nRestoring Sub-Archive ") + current_file + ": \n\n");
			str = "cd " + USglobal->config_list.data_dir + ";"
			+ "tar -xf " + current_file;
			FILE *f4 = popen(str, "r");
			while (fgets(cstr, 500, f4))
			{
				mle->insert((QString) cstr);
			}
			return_info = pclose(f4);
			if(return_info < 0)
			{
				QMessageBox::message(tr("Attention:"), 
											tr("There was an error in function tar/gzip while\n"
											"processing the subarchive \"" + current_file + "\".\n\n"
											"This archive may be corrupted.\n\n"
											"If you just created this archive, make sure\n"
											"that the background compression process has\n"
											"terminated before trying again."));
				return;
			}
		}
		mle->insert(tr("\nFinished Restoring Archive ")+ fn + "...\n");
	}
	mle->setReadOnly(true);
}
*/
void US_Archive::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/archive.html");
}
