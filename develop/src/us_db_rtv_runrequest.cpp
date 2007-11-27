#include "../include/us_db_rtv_runrequest.h"

//! Constructor
/*! 
	Constractor a new <var>US_DB_RtvRunRequest</var> interface, 
	with <var>p</var> as a parent and <var>us_rtvrunrequest</var> as object name. 
*/ 
US_DB_RtvRunRequest::US_DB_RtvRunRequest(QWidget *p, const char *name) : US_DB_RtvInvestigator( p, name)
{
	RunRequestID=0;
	QString str =tr("Show Investigator Submitted RunRequest\nfrom DB:");
	str.append(login_list.dbname);
	lbl_blank->setText(str);
	
	lb_name->disconnect();
	connect(lb_name, SIGNAL(highlighted(int)), SLOT(select_name(int)));
	connect(lb_name, SIGNAL(selected(int)), SLOT(check_runrequest(int)));

	pb_chkID->disconnect();
	connect(pb_chkID, SIGNAL(clicked()), SLOT(checkRunRequest()));
	lb_data->disconnect();
	connect(lb_data, SIGNAL(highlighted(int)), SLOT(select_runrequest(int)));
	connect(lb_data, SIGNAL(selected(int)), SLOT(select_runrequest(int)));

	pb_retrieve->disconnect();
	pb_retrieve->setText("Show Info");
	connect(pb_retrieve, SIGNAL(clicked()), SLOT(show_runrequest()));
}

//! Destructor
/*! destroy the US_DB_RtvRunRequest. */
US_DB_RtvRunRequest::~US_DB_RtvRunRequest()
{
}

void US_DB_RtvRunRequest::check_runrequest(int item)
{
	select_name(item);
	checkRunRequest();
}

void US_DB_RtvRunRequest::checkRunRequest()
{
	QString str, *item_description;
	int maxID = get_newID("tblRunRequest", "RunRequestID");
	int count = 0;
	item_RunRequestID = new int[maxID];
	item_description = new QString[maxID];
	display_Str = new QString[maxID];

	if(check_ID)
	{
		str.sprintf("SELECT RunRequestID, RunDescription FROM tblRunRequest WHERE InvestigatorID = %d;", InvID);
		QSqlQuery query(str);
		if(query.isActive())
		{
			while(query.next())
			{
				item_RunRequestID[count] = query.value(0).toInt();
				item_description[count] = query.value(1).toString();
				display_Str[count] = "(" + QString::number( item_RunRequestID[count] ) + "), "+item_description[count];
				count++;
			}
		}

		if(count>0)
		{
			lb_data->clear();
			for( int i=0; i<count; i++)
			{
				lb_data->insertItem(display_Str[i]);
			}	
			sel_data = true;
		}
		else
		{	
			QString str;
			str = "No database records available for: " + name;
			lb_data->clear();
			lb_data->insertItem(str);
		}
	}
	else
	{
		QMessageBox::message(tr("Attention:"), 
									tr("Please select an investigator first!\n"));
		return;
	}
}


void US_DB_RtvRunRequest::select_runrequest(int item)
{
	QString str;
	if(sel_data)
	{
		RunRequestID = item_RunRequestID[item];
		Display = display_Str[item];		
		str.sprintf("Retrieve: "+ Display);
		lbl_item->setText(str);
		retrieve_flag = true;			
	}
	else
	{
		QMessageBox::message(tr("Attention:"), 
									tr("No RunRequest Data has been selected\n"));
		return;
	}
}

void US_DB_RtvRunRequest::show_runrequest()
{
	QString str;
	if(!retrieve_flag)
	{
		QMessageBox::message(tr("Attention:"), 
									tr("You have to select a dataset first!\n"));
		return;
	}
	US_RunRequest *us_request;
	us_request = new US_RunRequest(RunRequestID);
	us_request->show();

}

/*****************************************************************************************/

US_RunRequest::US_RunRequest(int temp_RRID, QWidget *p, const char *name) : US_DB(p, name)
{
	int border=4, spacing=2;
	int xpos = border, ypos = border;
	int buttonw = 180, buttonh = 26;
	
	s1=s2=s3=s4=s5=s6=s7=s8=0;
	
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	
	lbl_description = new QLabel(tr(" Run Description :"),this);
	lbl_description->setAlignment(AlignLeft|AlignVCenter);
	lbl_description->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_description->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_description->setGeometry(xpos, ypos, buttonw, buttonh);
	
	xpos +=buttonw+spacing;
	
	le_description= new QLineEdit(this, "description");
	le_description->setAlignment(AlignLeft|AlignVCenter);
	le_description->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_description->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_description->setGeometry(xpos, ypos, buttonw*3/2, buttonh);
	le_description->setReadOnly(true);
	
	xpos = border;
	ypos+= buttonh+spacing;
	
	lbl_request = new QLabel(tr(" Request :"),this);
	lbl_request->setAlignment(AlignLeft|AlignVCenter);
	lbl_request->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_request->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_request->setGeometry(xpos, ypos, buttonw, buttonh);
	
	xpos +=buttonw+spacing;
	
	le_request= new QLineEdit(this, "Request");
	le_request->setAlignment(AlignLeft|AlignVCenter);
	le_request->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_request->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_request->setGeometry(xpos, ypos, buttonw*3/2, buttonh);
	le_request->setReadOnly(true);
	
	xpos = border;
	ypos+= buttonh+spacing;
	
	lbl_runtype = new QLabel(tr(" Run Type :"),this);
	lbl_runtype->setAlignment(AlignLeft|AlignVCenter);
	lbl_runtype->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_runtype->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_runtype->setGeometry(xpos, ypos, buttonw, buttonh);
	
	xpos +=buttonw+spacing;
	
	le_runtype= new QLineEdit(this, "runtype");
	le_runtype->setAlignment(AlignLeft|AlignVCenter);
	le_runtype->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_runtype->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_runtype->setGeometry(xpos, ypos, buttonw*3/2, buttonh);
	le_runtype->setReadOnly(true);
	
	xpos = border;
	ypos+= buttonh+spacing;
	
	pb_sample1 = new QPushButton(tr("Sample 1 :"), this);
	pb_sample1->setAutoDefault(false);
	pb_sample1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_sample1->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_sample1->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_sample1->setEnabled(false);
	connect(pb_sample1, SIGNAL(clicked()), SLOT(view_sample1()));
	
	xpos +=buttonw+spacing;
	
	le_sample1= new QLineEdit(this, "sample1");
	le_sample1->setAlignment(AlignLeft|AlignVCenter);
	le_sample1->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_sample1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_sample1->setGeometry(xpos, ypos, buttonw*3/2, buttonh);
	le_sample1->setReadOnly(true);

	xpos = border;
	ypos+= buttonh+spacing;
	
	pb_sample2 = new QPushButton(tr("Sample 2 :"), this);
	pb_sample2->setAutoDefault(false);
	pb_sample2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_sample2->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_sample2->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_sample2->setEnabled(false);
	connect(pb_sample2, SIGNAL(clicked()), SLOT(view_sample2()));
	
	xpos +=buttonw+spacing;
	
	le_sample2= new QLineEdit(this, "sample2");
	le_sample2->setAlignment(AlignLeft|AlignVCenter);
	le_sample2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_sample2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_sample2->setGeometry(xpos, ypos, buttonw*3/2, buttonh);
	le_sample2->setReadOnly(true);
	
	xpos = border;
	ypos+= buttonh+spacing;
	
	pb_sample3 = new QPushButton(tr("Sample 3 :"), this);
	pb_sample3->setAutoDefault(false);
	pb_sample3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_sample3->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_sample3->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_sample3->setEnabled(false);
	connect(pb_sample3, SIGNAL(clicked()), SLOT(view_sample3()));
	
	xpos +=buttonw+spacing;
	
	le_sample3= new QLineEdit(this, "sample3");
	le_sample3->setAlignment(AlignLeft|AlignVCenter);
	le_sample3->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_sample3->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_sample3->setGeometry(xpos, ypos, buttonw*3/2, buttonh);
	le_sample3->setReadOnly(true);
	
	xpos = border;
	ypos+= buttonh+spacing;
	
	pb_sample4 = new QPushButton(tr("Sample 4 :"), this);
	pb_sample4->setAutoDefault(false);
	pb_sample4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_sample4->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_sample4->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_sample4->setEnabled(false);
	connect(pb_sample4, SIGNAL(clicked()), SLOT(view_sample4()));
	
	xpos +=buttonw+spacing;
	
	le_sample4= new QLineEdit(this, "sample4");
	le_sample4->setAlignment(AlignLeft|AlignVCenter);
	le_sample4->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_sample4->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_sample4->setGeometry(xpos, ypos, buttonw*3/2, buttonh);
	le_sample4->setReadOnly(true);

	xpos = border;
	ypos+= buttonh+spacing;
	
	pb_sample5 = new QPushButton(tr("Sample 5 :"), this);
	pb_sample5->setAutoDefault(false);
	pb_sample5->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_sample5->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_sample5->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_sample5->setEnabled(false);
	connect(pb_sample5, SIGNAL(clicked()), SLOT(view_sample5()));
	
	xpos +=buttonw+spacing;
	
	le_sample5= new QLineEdit(this, "sample5");
	le_sample5->setAlignment(AlignLeft|AlignVCenter);
	le_sample5->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_sample5->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_sample5->setGeometry(xpos, ypos, buttonw*3/2, buttonh);
	le_sample5->setReadOnly(true);
	
	xpos = border;
	ypos+= buttonh+spacing;
	
	pb_sample6 = new QPushButton(tr("Sample 6 :"), this);
	pb_sample6->setAutoDefault(false);
	pb_sample6->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_sample6->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_sample6->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_sample6->setEnabled(false);
	connect(pb_sample6, SIGNAL(clicked()), SLOT(view_sample6()));
	
	xpos +=buttonw+spacing;
	
	le_sample6= new QLineEdit(this, "sample6");
	le_sample6->setAlignment(AlignLeft|AlignVCenter);
	le_sample6->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_sample6->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_sample6->setGeometry(xpos, ypos, buttonw*3/2, buttonh);
	le_sample6->setReadOnly(true);
	
	xpos = border;
	ypos+= buttonh+spacing;
	
	pb_sample7 = new QPushButton(tr("Sample 7 :"), this);
	pb_sample7->setAutoDefault(false);
	pb_sample7->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_sample7->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_sample7->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_sample7->setEnabled(false);
	connect(pb_sample7, SIGNAL(clicked()), SLOT(view_sample7()));
	
	xpos +=buttonw+spacing;
	
	le_sample7= new QLineEdit(this, "sample7");
	le_sample7->setAlignment(AlignLeft|AlignVCenter);
	le_sample7->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_sample7->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_sample7->setGeometry(xpos, ypos, buttonw*3/2, buttonh);
	le_sample7->setReadOnly(true);
	
	xpos = border;
	ypos+= buttonh+spacing;
	
	pb_sample8 = new QPushButton(tr("Sample 8 :"), this);
	pb_sample8->setAutoDefault(false);
	pb_sample8->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_sample8->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_sample8->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_sample8->setEnabled(false);
	connect(pb_sample8, SIGNAL(clicked()), SLOT(view_sample8()));
	
	xpos +=buttonw+spacing;
	
	le_sample8= new QLineEdit(this, "sample8");
	le_sample8->setAlignment(AlignLeft|AlignVCenter);
	le_sample8->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_sample8->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_sample8->setGeometry(xpos, ypos, buttonw*3/2, buttonh);
	le_sample8->setReadOnly(true);



	xpos = border;
	ypos+= buttonh+spacing;
	
	pb_help = new QPushButton(tr("Help"), this);
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_help->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));
	
	xpos +=buttonw+spacing;
	
	pb_close = new QPushButton(tr("Close"), this);
	pb_close->setAutoDefault(false);
	pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_close->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_close->setGeometry(xpos, ypos, buttonw*3/2, buttonh);
	connect(pb_close, SIGNAL(clicked()), SLOT(quit()));
	
	xpos = buttonw*5/2+2*border+2*spacing;
	ypos += buttonh + border;
	setMinimumSize(xpos, ypos);
	
	global_Xpos += 30;
	global_Ypos += 30;
	setGeometry(global_Xpos, global_Ypos, xpos, ypos);
	
	load(temp_RRID);
}

US_RunRequest::~US_RunRequest()
{
}

void US_RunRequest::load(int RRID)
{
	QString str;
	QString description, rtype;
	int requestID;
	
	str.sprintf("SELECT RunDescription, RequestID, RunType, SampleID1, SampleID2, SampleID3, SampleID4, SampleID5, SampleID6, SampleID7, SampleID8 FROM tblRunRequest WHERE RunRequestID = %d;", RRID);
	QSqlQuery query(str);
	if(query.isActive())
	{
		if(query.next())
		{
			description = query.value(0).toString();
			requestID=query.value(1).toInt();
			rtype=query.value(2).toString();
			s1=query.value(3).toInt();
			s2=query.value(4).toInt();
			s3=query.value(5).toInt();
			s4=query.value(6).toInt();
			s5=query.value(7).toInt();
			s6=query.value(8).toInt();
			s7=query.value(9).toInt();
			s8=query.value(10).toInt();
									
			str="("+QString::number(RRID)+"), "+description;
			le_description->setText(str);
			le_request->setText(show_request(requestID));
			le_runtype->setText(rtype);
		}
		else
		{
			QMessageBox::message(tr("Attention:"), tr("SQL query failed to retrieve Run Request"));
			exit(-1);
		}
	}
	
	if(s1>0)
	{
		pb_sample1->setEnabled(true);
		le_sample1->setText(show_sample(s1));
	}
	if(s2>0)
	{
		pb_sample2->setEnabled(true);
		le_sample2->setText(show_sample(s2));
	}
	if(s3>0)
	{
		pb_sample3->setEnabled(true);
		le_sample3->setText(show_sample(s3));
	}
	if(s4>0)
	{
		pb_sample4->setEnabled(true);
		le_sample4->setText(show_sample(s4));
	}
	if(s5>0)
	{
		pb_sample5->setEnabled(true);
		le_sample5->setText(show_sample(s5));
	}
	if(s6>0)
	{
		pb_sample6->setEnabled(true);
		le_sample6->setText(show_sample(s6));
	}
	if(s7>0)
	{
		pb_sample7->setEnabled(true);
		le_sample7->setText(show_sample(s7));
	}
	if(s8>0)
	{
		pb_sample8->setEnabled(true);
		le_sample8->setText(show_sample(s8));
	}

}

void US_RunRequest::view_sample1()
{
	US_DB_Sample *sample_dlg;
	sample_dlg = new US_DB_Sample(s1);
	sample_dlg->show();
}
void US_RunRequest::view_sample2()
{
	US_DB_Sample *sample_dlg;
	sample_dlg = new US_DB_Sample(s2);
	sample_dlg->show();
}
void US_RunRequest::view_sample3()
{
	US_DB_Sample *sample_dlg;
	sample_dlg = new US_DB_Sample(s3);
	sample_dlg->show();
}
void US_RunRequest::view_sample4()
{
	US_DB_Sample *sample_dlg;
	sample_dlg = new US_DB_Sample(s4);
	sample_dlg->show();
}
void US_RunRequest::view_sample5()
{
	US_DB_Sample *sample_dlg;
	sample_dlg = new US_DB_Sample(s5);
	sample_dlg->show();
}
void US_RunRequest::view_sample6()
{
	US_DB_Sample *sample_dlg;
	sample_dlg = new US_DB_Sample(s6);
	sample_dlg->show();
}
void US_RunRequest::view_sample7()
{
	US_DB_Sample *sample_dlg;
	sample_dlg = new US_DB_Sample(s7);
	sample_dlg->show();
}
void US_RunRequest::view_sample8()
{
	US_DB_Sample *sample_dlg;
	sample_dlg = new US_DB_Sample(s8);
	sample_dlg->show();
}


void US_RunRequest::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/db_runrequest.html");
}

void US_RunRequest::quit()
{
	close();
}
