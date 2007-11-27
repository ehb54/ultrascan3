#include "../include/us_db_rtv_date.h"

//! Constructor
/*! 
	Constractor a new <var>US_DB_RtvDate</var> interface, 
	with <var>p</var> as a parent and <var>us_rtvdate</var> as object name. 
*/ 
US_DB_RtvDate::US_DB_RtvDate(QWidget *p, const char *name) : US_DB( p, name)
{
	int xpos, ypos, buttonw = 480,  buttonh = 26;
	
	sel_data = false;		//use for query data listbox select
	retrieve_flag = false;//use for control 'Retrieve' pushbutton
	Date = "";
	ExpdataID = -1;
	
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	
 	xpos = border;
	ypos = border;	
	
	QString str =tr("Retrieve Experimental Data by Date\nfrom DB: ");
	str.append(login_list.dbname);
	lbl_blank = new QLabel(str, this);
	lbl_blank->setAlignment(AlignCenter|AlignVCenter);
//	lbl_blank->setFrameStyle(QFrame::WinPanel|Raised);
	lbl_blank->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_blank->setGeometry(xpos, ypos, buttonw, buttonh*2);
	lbl_blank->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	xpos = border;
	ypos += buttonh*2 + spacing*2;

	pb_date = new QPushButton(tr("Calendar"),this);
	pb_date->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_date->setGeometry(xpos, ypos, buttonw/2, buttonh);
	pb_date->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	connect(pb_date, SIGNAL(clicked()), SLOT(select_date()));
	
	xpos += buttonw/2;

	lbl_date= new QLabel("",this);
	lbl_date->setAlignment(AlignLeft|AlignVCenter);	
	lbl_date->setGeometry(xpos, ypos, buttonw/2, buttonh);
	lbl_date->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_date->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_date->setText(tr(" not selected"));
	

	xpos = border;
	ypos += buttonh + spacing;
	
	pb_chkDate = new QPushButton("Check for Experimental Data", this);
	pb_chkDate->setAutoDefault(false);
	pb_chkDate->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_chkDate->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_chkDate->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_chkDate, SIGNAL(clicked()), SLOT(checkDate()));
	
	xpos = border;
	ypos += buttonh;
	
	lbl_instr = new QLabel(tr("Doubleclick on item to select:"),this);
	lbl_instr->setAlignment(AlignCenter|AlignVCenter);
	lbl_instr->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_instr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_instr->setGeometry(xpos, ypos, buttonw, buttonh);
	
	xpos = border;
	ypos += buttonh;
	
	lb_data = new QListBox(this, "data");
	lb_data->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	lb_data->setGeometry(xpos, ypos, buttonw, buttonh*5);
	lb_data->setSelected(0, true);
	connect(lb_data, SIGNAL(selected(int)), SLOT(select_data(int)));
	
	xpos = border;
	ypos += buttonh*5 + spacing;
	
	lbl_item = new QLabel("Retrieve: not selected",this);
	lbl_item->setAlignment(AlignLeft|AlignVCenter);
//	lbl_item->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_item->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_item->setGeometry(xpos, ypos, buttonw+spacing, buttonh);
	lbl_item->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos = border;
	ypos += buttonh + spacing;
	
	pb_reset = new QPushButton(tr("Reset"), this);
	pb_reset->setAutoDefault(false);
	pb_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_reset->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_reset->setGeometry(xpos, ypos, buttonw/2-1, buttonh);
	pb_reset->setEnabled(true);
	connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));

	xpos +=buttonw/2 + spacing;

	pb_retrieve = new QPushButton(tr("Retrieve"), this);
	pb_retrieve->setAutoDefault(false);
	pb_retrieve->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_retrieve->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_retrieve->setGeometry(xpos, ypos, buttonw/2-1, buttonh);
	pb_retrieve->setEnabled(true);
	connect(pb_retrieve, SIGNAL(clicked()), SLOT(retrieve()));


	xpos = border;
	ypos += buttonh + spacing;
		
	pb_help = new QPushButton(tr("Help"), this);
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_help->setGeometry(xpos, ypos, buttonw/2-1, buttonh);
	pb_help->setEnabled(true);
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	xpos += buttonw/2 + spacing;

	pb_close = new QPushButton(tr("Close"), this);
	pb_close->setAutoDefault(false);
	pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_close->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_close->setGeometry(xpos, ypos, buttonw/2-1, buttonh);
	pb_close->setEnabled(true);
	connect(pb_close, SIGNAL(clicked()), SLOT(quit()));
	
	xpos = buttonw +2*border;
	ypos += buttonh + spacing;
	setMinimumSize(xpos, ypos);
	
	global_Xpos += 30;
	global_Ypos += 30;
	setGeometry(global_Xpos, global_Ypos, xpos, ypos);
	
	setup_GUI();
	reset();
}

//! Destructor
/*! destroy the US_DB_RtvDate. */
US_DB_RtvDate::~US_DB_RtvDate()
{
	delete item_ExpdataID;
	delete display_Str;
	delete expdata_dlg;
}

void US_DB_RtvDate::setup_GUI()
{
	QGridLayout * Grid = new QGridLayout(this,8,2,4,2);
	Grid->addMultiCellWidget(lbl_blank,0,0,0,1);
	Grid->addWidget(pb_date,1,0);
	Grid->addWidget(lbl_date,1,1);
	Grid->addMultiCellWidget(pb_chkDate,2,2,0,1);
	Grid->addMultiCellWidget(lbl_instr,3,3,0,1);
	Grid->addMultiCellWidget(lb_data,4,4,0,1);
	Grid->addMultiCellWidget(lbl_item,5,5,0,1);
	Grid->addWidget(pb_reset,6,0);
	Grid->addWidget(pb_retrieve,6,1);
	Grid->addWidget(pb_help,7,0);
	Grid->addWidget(pb_close,7,1);
	Grid->activate();
}

/*!
	Open an US_Calendar widget.
*/
void US_DB_RtvDate::select_date()
{
	calendar_dlg = new US_Calendar();
	calendar_dlg->setCaption("Calendar");
	calendar_dlg->show();
	connect(calendar_dlg, SIGNAL(dateChanged(QString)), SLOT(update_date_lbl(QString)));
}

/*!
	Update the date with your selected date from US_Calendar.
*/
void US_DB_RtvDate::update_date_lbl (QString Display_Date)
{

	Date = Display_Date;
	lbl_date->setText(Date);
	QString str = tr("Check for Data by Date: ") + Date;
	pb_chkDate->setText(str);
	lb_data->clear();
}

/*!
	If you select the date, this function will list all Experimental data done on that date.
	If no date is selected, this function will list all Experimental data stored in database.
*/
void US_DB_RtvDate::checkDate()
{
	
	QSqlCursor cur( "tblExpData" );
 	QSqlIndex order = cur.index( "ExpdataID" );
	if(!Date.isEmpty())
	{
		QSqlIndex filter = cur.index( "Date" );
		cur.setValue( "Date", Date );
   	cur.select(filter,order);
	}
	else
	{
		cur.select(order);
	}
	
			
	int maxID = get_newID("tblExpData", "ExpDataID");
	int count = 0;
	QString *item_date;
	item_date=new QString[maxID];
	item_ExpdataID = new int[maxID];
	display_Str = new QString[maxID];
/*
	QString str;
	str="SELECT ExpdataID, Description, Date FROM tblExpData WHERE Date > "+ Date;
	QSqlQuery query(str);
	if(query.isActive())
	{
		while(query.next())
		{
			item_ExpdataID[count] = query.value(0).toInt();
			display_Str[count] = "ExpDataID (" + QString::number( item_ExpdataID[count] ) + "), "+	
									query.value(1).toString();
			Date = query.value(2).toString();
			count++;
		}
	}
*/
	while(cur.next() )
	{
		int id = cur.value("ExpdataID").toInt();
		display_Str[count] = "ExpDataID (" + QString::number( id ) + "), "+	
									cur.value( "Description" ).toString();
		item_ExpdataID[count] = id;
		item_date[count] = cur.value("Date").toString();
		count++;					
	}

	if(count>0)
	{
		QString str;
		lb_data->clear();
		for( int i=0; i<count; i++)
		{
			str = "";
			str = "Date ("+ item_date[i] +"): " + display_Str[i];
			lb_data->insertItem(str);
		}	
		sel_data = true;
	}
	else
	{
		QString str1 = "No database record found on: " + Date;
		lb_data->clear();
		lb_data->insertItem(str1);
	}
}
/*!
	If you find your expected experimental data in ListBox, doubleclick it.
	This item will be selected for your retrieving.
*/
void US_DB_RtvDate::select_data(int item)
{
	QString str;
	if(sel_data)
	{
		ExpdataID = item_ExpdataID[item];
		Display = display_Str[item];		
		str.sprintf("Retrieve: "+Display);
		lbl_item->setText(str);
		retrieve_flag = true;			
	}
	else
	{
		QMessageBox::message(tr("Attention:"), 
									tr("No Experimental Data was selected\n"));
		return;
	}
}

/*!
	Retrieve the selected experimental data from database. Pop-up US_ExpData_DB interface.
*/
void US_DB_RtvDate::retrieve()
{
	if(retrieve_flag)
	{
		expdata_dlg = new US_ExpData_DB();
		if(expdata_dlg->retrieve_all(ExpdataID, Display))
		{
			expdata_dlg->show();
		}
		else 
		{
			return;
		}		
	}
	else
	{
		QMessageBox::message(tr("Attention:"), 
									tr("You must select a dataset first!\n"));
		return;
	}
}

/*!
	Reset all variables to initialization status.
*/
void US_DB_RtvDate::reset()
{	
	sel_data = false;		
	retrieve_flag = false;
	ExpdataID = -1;
	Date = "";
	lbl_date->setText(tr(" not selected"));
	lbl_item->setText(tr(" not selected"));
	pb_chkDate ->setText(tr("Check for Experimental Data"));
	lb_data->clear();
}

/*! Open a netscape browser to load help page.*/
void US_DB_RtvDate::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/rtv_date.html");
}

/*! Close the interface. */			
void US_DB_RtvDate::quit()
{
	close();
}

