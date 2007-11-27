#include "../include/us_db_rtv_edittype.h"

//! Constructor
/*! 
	Constractor a new <var>US_DB_RtvEditType</var> interface, 
	with <var>p</var> as a parent and <var>us_rtvedittype</var> as object name. 
*/ 
US_DB_RtvEditType::US_DB_RtvEditType(QWidget *p, const char *name) : US_DB( p, name)
{
	int xpos, ypos, buttonw = 360,  buttonh = 26;
	
	sel_experiment = false;		// use for control 'Choose Experiment Type' combolist
	sel_optical = false;			// use for control 'Choose Optical system' combolist 
	sel_data = false;				// use for query data listbox select
	retrieve_flag = false;		// use for control 'Retrieve' pushbutton
	ExpdataID = -1;
	
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
				
	xpos = border;
	ypos = border;
	setCaption("Retrieve Data by Edit Type");
	
	QString str =tr("Retrieve Experimental Data by Type\nfrom DB: ");
	str.append(login_list.dbname);
	lbl_blank = new QLabel(str, this);
	lbl_blank->setAlignment(AlignCenter|AlignVCenter);
//	lbl_blank->setFrameStyle(QFrame::WinPanel|Raised);
	lbl_blank->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_blank->setGeometry(xpos, ypos, buttonw, buttonh*2);
	lbl_blank->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	xpos = border;
	ypos += buttonh*2 + spacing*2;

	lbl_experiment = new QLabel(tr(" Experiment Type:"),this);
//	lbl_experiment->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_experiment->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_experiment->setGeometry(xpos, ypos, buttonw/2-1, buttonh);
	lbl_experiment->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	xpos += buttonw/2+spacing;

	cmbb_experiment= new QComboBox(false, this, "Exp. Type Selection");
	cmbb_experiment->setGeometry(xpos, ypos, buttonw/2-1, buttonh);
	cmbb_experiment->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cmbb_experiment->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	cmbb_experiment->insertItem("not selected");
	cmbb_experiment->insertItem("Velocity");
	cmbb_experiment->insertItem("Equilibrium");
	cmbb_experiment->insertItem("Diffusion");
	cmbb_experiment->insertItem("Wavelength");
	connect(cmbb_experiment, SIGNAL(activated(int)), SLOT(select_experiment(int)));	

	xpos = border;
	ypos += buttonh + spacing;
	
	lbl_optical = new QLabel(tr(" Optical System:"),this);
//	lbl_optical->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_optical->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_optical->setGeometry(xpos, ypos, buttonw/2-1, buttonh);
	lbl_optical->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	xpos += buttonw/2+spacing;

	cmbb_optical= new QComboBox(false, this, "Optical System Select");
	cmbb_optical->setGeometry(xpos, ypos, buttonw/2-1, buttonh);
	cmbb_optical->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cmbb_optical->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	cmbb_optical->insertItem("not selected");
	cmbb_optical->insertItem("UV Absorbance");
	cmbb_optical->insertItem("Raleigh Interference");
	cmbb_optical->insertItem("Fluorescence");
	cmbb_optical->insertItem("Intensity");
	connect(cmbb_optical, SIGNAL(activated(int)), SLOT(select_optical(int)));	

	xpos = border;
	ypos += buttonh + spacing * 2;
	
	pb_chkET = new QPushButton(tr("Search for Experimental Data"), this);
	pb_chkET->setAutoDefault(false);
	pb_chkET->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_chkET->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_chkET->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_chkET, SIGNAL(clicked()), SLOT(checkEditType()));
	
	xpos = border;
	ypos += buttonh + spacing * 2;
	
	lbl_instr = new QLabel(tr("Doubleclick on item to select:"),this);
	lbl_instr->setAlignment(AlignCenter|AlignVCenter);
//	lbl_instr->setFrameStyle(QFrame::WinPanel|Raised);
	lbl_instr->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_instr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_instr->setGeometry(xpos, ypos, buttonw, buttonh);
	
	xpos = border;
	ypos += buttonh;
	
	lb_data = new QListBox(this, "data");
	lb_data->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	lb_data->setGeometry(xpos, ypos, buttonw, buttonh*4);
	lb_data->setSelected(0, true);
	connect(lb_data, SIGNAL(selected(int)), SLOT(select_data(int)));
	
	xpos = border;
	ypos += buttonh*4 + spacing;
	
	lbl_item = new QLabel(" Nothing selected",this);
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
}

//! Destructor
/*! destroy the US_DB_RtvDate. */
US_DB_RtvEditType::~US_DB_RtvEditType()
{
	delete item_ExpdataID;
	delete display_Str;
	delete expdata_dlg;
}

void US_DB_RtvEditType::setup_GUI()
{
	QGridLayout * Grid = new QGridLayout(this,8,2,4,2);
	Grid->addMultiCellWidget(lbl_blank,0,0,0,1);
	Grid->addWidget(lbl_experiment,1,0);
	Grid->addWidget(cmbb_experiment,1,1);
	Grid->addWidget(lbl_optical,2,0);
	Grid->addWidget(cmbb_optical,2,1);
	Grid->addMultiCellWidget(pb_chkET,3,3,0,1);
	Grid->addMultiCellWidget(lbl_instr,4,4,0,1);
	Grid->addMultiCellWidget(lb_data,5,5,0,1);
	Grid->addMultiCellWidget(lbl_item,6,6,0,1);
	Grid->addWidget(pb_reset,6,0);
	Grid->addWidget(pb_retrieve,6,1);
	Grid->addWidget(pb_help,7,0);
	Grid->addWidget(pb_close,7,1);
	Grid->activate();
}

/*!
	Chooses desired experiment type.
*/
void US_DB_RtvEditType::select_experiment(int item)
{
	if(item ==1)
	{	
		exp_type = 1;
		sel_experiment = true;
	}
	if(item ==2)
	{
		exp_type = 2;
		sel_experiment = true;	
	}
	if(item ==3)
	{
		exp_type = 3;
		sel_experiment = true;
	}
	if(item ==4)
	{
		exp_type = 4;
		sel_experiment = true;
	}
	if(item == 0)
	{
		sel_experiment = false;
	}
	lb_data->clear();
}

/*!
	Chooses desired optical system.
*/
void US_DB_RtvEditType::select_optical(int item)
{
	if(item ==1)
	{		
		opt_type = 1;
		sel_optical = true;
	}
	if(item ==2)
	{
		opt_type = 2;
		sel_optical = true;	
	}
	if(item ==3)
	{
		opt_type = 3;
		sel_optical = true;
	}
	if(item ==4)
	{
		opt_type = 4;
		sel_optical = true;
	}
	if(item == 0)
	{
		sel_optical = false;
	}
	lb_data->clear();	
}

/*!
	If the edit type is found, this function will list all experimental data matching the edit type.
	If the edit type is not found, no experimental data will be returned.
	If no edit type is input, this function will list all experimental data stored in database.
*/
void US_DB_RtvEditType::checkEditType()
{
	if(!sel_experiment && sel_optical)
	{
		QMessageBox::message(tr("Attention:"), 
									tr("Please select an Experiment Type.\n"));
		return;
	}
	if(!sel_optical && sel_experiment)
	{
		QMessageBox::message(tr("Attention:"), 
									tr("Please select an Optical System.\n"));
		return;
	}
	else
	{
		if(exp_type == 1 && opt_type == 1)
		{
			edit_type = 1;
		}
		if(exp_type == 2 && opt_type == 1)
		{
			edit_type = 2;
		}
		if(exp_type == 1 && opt_type == 2)
		{
			edit_type = 3;
		}
		if(exp_type == 2 && opt_type == 2)
		{
			edit_type = 4;
		}
		if(exp_type == 1 && opt_type == 3)
		{
			edit_type = 5;
		}
		if(exp_type == 2 && opt_type == 3)
		{
			edit_type = 6;
		}
		if(exp_type == 3 && opt_type == 1)
		{
			edit_type = 7;
		}
		if(exp_type == 3 && opt_type == 2)
		{
			edit_type = 8;
		}
		if(exp_type == 3 && opt_type == 3)
		{
			edit_type = 9;
		}
		if(exp_type == 4 && opt_type == 1)
		{
			edit_type = 10;
		}
		if(exp_type == 4 && opt_type == 4)
		{
			edit_type = 11;
		}
		if(exp_type == 1 && opt_type == 4)
		{
			edit_type = 12;
		}
		if(exp_type == 2 && opt_type == 4)
		{
			edit_type = 13;
		}
		if(exp_type == 3 && opt_type == 4)
		{
			edit_type = 14;
		}
		QSqlCursor cur( "tblExpData" );
 		QSqlIndex order = cur.index( "Description" );
		if(sel_experiment && sel_optical)
		{
			QSqlIndex filter = cur.index( "Edit_type" );
			cur.setValue( "Edit_type", edit_type );
   		cur.select(filter,order);
		}
		if(!sel_experiment && !sel_optical)
		{
			cur.select(order);
		}
		int maxID = get_newID("tblExpData", "ExpDataID");
		int count = 0;
		item_ExpdataID = new int[maxID];
		display_Str = new QString[maxID];
	
		while(cur.next() )
		{
			int id = cur.value("ExpdataID").toInt();
			display_Str[count] = "ExpDataID (" + QString::number( id ) + "), "+	
										cur.value( "Description" ).toString();
			item_ExpdataID[count] = id;
			count++;					
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
			QString str1 = "Data matching this condition could not be found!";
			lb_data->clear();
			lb_data->insertItem(str1);
		}
	}
}

/*!
	If you find your expected experimental data in ListBox, doubleclick it.
	This item will be selected for your retrieving.
*/
void US_DB_RtvEditType::select_data(int item)
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
									tr("No experimental data has been selected\n"));
		return;
	}
}

/*!
	Retrieve the selected experimental data from database. Pop-up US_ExpData_DB interface.
*/
void US_DB_RtvEditType::retrieve()
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
									tr("You have to select a dataset first!\n"));
		return;
	}
}

/*!
	Reset all variables to initialization status.
*/
void US_DB_RtvEditType::reset()
{
	sel_experiment = false;	
	sel_optical = false;		
	sel_data = false;		
	retrieve_flag = false;
	ExpdataID = -1;
	lbl_item->setText(tr(" nothing selected"));
	cmbb_experiment->setCurrentItem(0);
	cmbb_optical->setCurrentItem(0);
	lb_data->clear();
}

/*! Open a netscape browser to load help page.*/
void US_DB_RtvEditType::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/rtv_edittype.html");
}

/*! Close the interface. */				
void US_DB_RtvEditType::quit()
{
	close();
}

